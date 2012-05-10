

#include "SceneCR.hpp"

#include "App.hpp"
#include "Data.hpp"
#include "shader/PassThrough.hpp" // kernel shader


// -----------------------------------------------
// Anonymous declaration
// -----------------------------------------------

namespace {

// translate GLUT key as generic Camera key
void glmToFW_matrix4f( const glm::mat4 &in, FW::Mat4f &out);

}

// -----------------------------------------------
// Constructor / Destructor
// -----------------------------------------------

SceneCR::~SceneCR()
{  
  if (m_colorBuffer)
  {
    delete m_colorBuffer;
    m_colorBuffer = NULL;
  }
  
  if (m_depthBuffer)
  {
    delete m_depthBuffer;
    m_depthBuffer = NULL;
  }
}


// -----------------------------------------------
// Initializers
// -----------------------------------------------

void SceneCR::init(const Data& data)
{
  /// TODO: use more specific directories   
  
  // rasterizer's shaders sources
  m_cudaCompiler.setSourceFile( "../test/shader/PassThrough.cu" );
  
  // Uses the same include directories as the CMake script (for consistency)
  m_cudaCompiler.include( "../src/framework" );
  m_cudaCompiler.include( "../src/" );
  
  m_cudaRaster.init();
  
  // TODO: call only if no cached version exists
  firstTimeInit();
  
  initPipe();
  
  initGeometry(data);
}


void SceneCR::initGeometry(const Data& data)
{ 
  fprintf(stderr, "SceneCR::initGeometry\n");
  
  assert( !data.indices.empty() );
  assert( !data.positions.empty() );
  
   
  m_numVertices = data.numVertices;
  m_numTriangles = data.numIndices / 3;
  
  // Allocate buffers.
  m_inVertices.resizeDiscard( m_numVertices  * sizeof(FW::InputVertex) );
  m_outVertices.resizeDiscard( m_numVertices * sizeof(FW::ShadedVertex_passthrough) );
  
  
  // Copy vertex attributes.
  FW::InputVertex* inputVertexPtr = (FW::InputVertex*)m_inVertices.getMutablePtr();
  for (int i = 0; i < data.numVertices; ++i)
  {
    //FW::InputVertex& in = inputVertexPtr[i];
    FW::Vec3f &vertex = inputVertexPtr[i].modelPos;
    
    vertex.x = data.positions[3*i+0];
    vertex.y = data.positions[3*i+1];
    vertex.z = data.positions[3*i+2];
  }
  
  
  // Copy vertex indices.
  m_indices.resizeDiscard( m_numTriangles * sizeof(FW::Vec3i) );
  
  FW::Vec3i* vertexIndexPtr = (FW::Vec3i*)m_indices.getMutablePtr();
  
  for (int i = 0; i < m_numTriangles; ++i)
  {
    FW::Vec3i &id = vertexIndexPtr[i];
    id.x = data.indices[3*i+0];
    id.y = data.indices[3*i+1];
    id.z = data.indices[3*i+2];
  }
}


void SceneCR::initPipe(void)
{
  // Create surfaces.
  FW::Vec2i screenResolution = FW::Vec2i( App::kScreenWidth, App::kScreenHeight);  
  
  if (m_colorBuffer) delete m_colorBuffer;
  m_colorBuffer = new FW::CudaSurface( screenResolution, 
                                       FW::CudaSurface::FORMAT_RGBA8, 
                                       App::kState.numSamples);

  if (m_depthBuffer) delete m_depthBuffer;
  m_depthBuffer = new FW::CudaSurface( screenResolution, 
                                       FW::CudaSurface::FORMAT_DEPTH32, 
                                       App::kState.numSamples);

  // Compile CUDA code.
  FW::U32 renderModeFlags = 0;
  if (App::kState.bDepth) renderModeFlags |= FW::RenderModeFlag_EnableDepth;
  if (App::kState.bLerp)  renderModeFlags |= FW::RenderModeFlag_EnableLerp;
  if (App::kState.bQuads) renderModeFlags |= FW::RenderModeFlag_EnableQuads;
  
  m_cudaCompiler.clearDefines();
  m_cudaCompiler.define( "SAMPLES_LOG2", m_colorBuffer->getSamplesLog2());
  m_cudaCompiler.define( "RENDER_MODE_FLAGS", renderModeFlags);
  m_cudaCompiler.define( "BLEND_SHADER", 
                         (!App::kState.bBlend) ? "BlendReplace" : "BlendSrcOver" );

  
  m_cudaModule = m_cudaCompiler.compile();
    
  if (NULL == m_cudaModule) {
    exit( EXIT_FAILURE );
  }

  // Setup CudaRaster.  
  std::string pipePostfix = "passthrough";
  
  std::string kernelName( "vertexShader_" + pipePostfix );
  size_t paramSize = 1 * sizeof(FW::U32) + 2 * sizeof(CUdeviceptr);
  m_vertexShaderKernel = m_cudaModule->getKernel( kernelName, paramSize);

  std::string pipeName( "PixelPipe_" + pipePostfix );
  m_cudaRaster.setSurfaces( m_colorBuffer, m_depthBuffer);
  m_cudaRaster.setPixelPipe( m_cudaModule, pipeName);
}


void SceneCR::firstTimeInit(void)
{
  printf("Performing first-time initialization.\n");
  printf("This will take a while.\n");
  printf("\n");
  
  // Populate CudaCompiler cache.
  //  int numMSAA = 4, numModes = 8, numBlends = 2; // all variants
  int numMSAA = 1, numModes = 2, numBlends = 2; // first 3 toggles

  int progress = 0;
  for (int msaa = 0; msaa < numMSAA; msaa++)
  for (int mode = 0; mode < numModes; mode++)
  for (int blend = 0; blend < numBlends; blend++)
  {
    printf( "Populating CudaCompiler cache... %d/%d\n", 
            ++progress, numMSAA * numModes * numBlends);

    m_cudaCompiler.clearDefines();
    
    m_cudaCompiler.define("SAMPLES_LOG2", msaa);
    
    m_cudaCompiler.define("RENDER_MODE_FLAGS", 
                          mode ^ 
                          FW::RenderModeFlag_EnableDepth ^ 
                          FW::RenderModeFlag_EnableLerp);

    m_cudaCompiler.define("BLEND_SHADER", (blend == 0) ? "BlendReplace" : 
                                                         "BlendSrcOver" );
    m_cudaCompiler.compile(false);
    //failIfError();
  }
  printf("\rPopulating CudaCompiler cache... Done.\n");
}


// -----------------------------------------------
// Renderer
// -----------------------------------------------

void SceneCR::render( const Camera& camera )
{
  FW::Vec2i windowSize( App::kScreenWidth, App::kScreenWidth);
  
  // Parameters changed => reinitialize pipe.  
  if (m_colorBuffer && m_colorBuffer->getSize() != windowSize) {
    m_pipeDirty = true;
  }  
  if (m_colorBuffer && m_colorBuffer->getNumSamples() != App::kState.numSamples) {
    m_pipeDirty = true;
  }  
  if (m_pipeDirty) {
    initPipe();
  }  
  m_pipeDirty = false;
  
  
  //----------------------------------
    
  /// ========== 1) Custom VertexShader to transform the vertices ==========
  
  // Set globals. (here, it can be seen as GLSL uniforms)
  FW::Constants& c = *(FW::Constants*)m_cudaModule->getGlobal("c_constants").getMutablePtrDiscard();
    
  
  // Translate glm matrix as CudaRaster Framework matrix
  glmToFW_matrix4f( camera.getViewProjMatrix(), c.posToClip);
  
  
  int ofs = 0;
  ofs += m_cudaModule->setParamPtr( m_vertexShaderKernel, ofs, 
                                    m_inVertices.getCudaPtr());
  ofs += m_cudaModule->setParamPtr( m_vertexShaderKernel, ofs, 
                                    m_outVertices.getMutableCudaPtrDiscard());
  ofs += m_cudaModule->setParami( m_vertexShaderKernel, ofs, m_numVertices);
  
  FW::Vec2i blockSize(32, 4);
  int numBlocks = (m_numVertices - 1) / (blockSize.x * blockSize.y) + 1;
  m_cudaModule->launchKernel(m_vertexShaderKernel, blockSize, numBlocks);
  
  
  /// ========== 2) Run CudaRaster ==========

  m_cudaRaster.deferredClear( FW::Vec4f(0.2f, 0.4f, 0.8f, 1.0f) );  
  
  m_cudaRaster.setVertexBuffer( &m_outVertices, 0);
  m_cudaRaster.setIndexBuffer( &m_indices, 0, m_numTriangles);    
    
  m_cudaRaster.drawTriangles();


  /// ========== 3) Render the buffer as an OpenGL screenquad ==========

  // XXX XXX XXX
  /// Render the texture as a Quad mapping the screen's corners
  /// TODO to visualize the result
  //m_colorBuffer->resolveToScreen();
  // XXX XXX XXX
  
  
  
  // Show CudaRaster statistics.
  if (false) //m_showStats
  {
    FW::CudaRaster::Stats s = m_cudaRaster.getStats();
    fprintf( stderr,  "CudaRaster: setup = %.2fms, bin = %.2fms, "\
                      "coarse = %.2fms, fine = %.2fms, total = %.2fms",
                      s.setupTime   * 1.0e3f,
                      s.binTime     * 1.0e3f,
                      s.coarseTime  * 1.0e3f,
                      s.fineTime    * 1.0e3f,
                      (s.setupTime + s.binTime + s.coarseTime + s.fineTime) * 1.0e3f
    );
  }
}


// -----------------------------------------------
// Namespace definition
// -----------------------------------------------

namespace {
  
void glmToFW_matrix4f( const glm::mat4 &in, FW::Mat4f &out)
{
  //memcpy( &out.m00, &(in[0][0]), 16*sizeof(float));
  
  #define COPY_MAT(i,j)  out.m##i##j = in[i][j]
  
  COPY_MAT(0, 0); COPY_MAT(0, 1); COPY_MAT(0, 2); COPY_MAT(0, 3);
  COPY_MAT(1, 0); COPY_MAT(1, 1); COPY_MAT(1, 2); COPY_MAT(1, 3);
  COPY_MAT(2, 0); COPY_MAT(2, 1); COPY_MAT(2, 2); COPY_MAT(2, 3);
  COPY_MAT(3, 0); COPY_MAT(3, 1); COPY_MAT(3, 2); COPY_MAT(3, 3);
  
  #undef COPY_MAT  
}

}
