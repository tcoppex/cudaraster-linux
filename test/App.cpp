
#include "App.hpp"

#include <cstdio>
#include <cstdlib>
#include <GL/freeglut.h>

#include "engine/Context.hpp"
#include "shader/PassThrough.hpp"


//==============================================================================

App::~App()
{
  if (!m_bInitialized) {
    return;
  }
  
  delete m_colorBuffer;
  m_colorBuffer = NULL;
  
  delete m_depthBuffer;
  m_depthBuffer = NULL;
}


// -----------------------------------------------


void App::_initContext( int argc, char *argv[])
{
  m_Context = new Context( this, argc, argv);
  
  m_Context->setGLVersion( 3, 3);
  m_Context->setFlags( GLUT_FORWARD_COMPATIBLE );
  m_Context->setProfile( GLUT_CORE_PROFILE );
  
  
  const int flags = GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL;
      
  int h = m_Context->createWindow( App::kScreenWidth, App::kScreenHeight, 
                                   flags, 
                                   "CudaRaster test");
  
  if (h < 1)
  {
    fprintf( stderr, "Error: Context creation has failed.\n");
    exit( EXIT_FAILURE );
  }
}

void App::_initObject( int argc, char *argv[])
{ 
  // TODO: use more specific directories 
  
  m_cudaCompiler.setSourceFile( "../test/shader/PassThrough.cu" );

  // Uses same include directories as the CMake script (for consistency)
  m_cudaCompiler.include( "../src/framework" );
  m_cudaCompiler.include( "../src/" );
  
  m_cudaRaster.init();
 
  //firstTimeInit(); 
  initPipe();
}


// -----------------------------------------------


void App::idle()
{
  glutPostRedisplay();
}

void App::display()
{
#if 0
  render_cudaraster();
#else
  if (MODE_CUDARASTER==m_mode) {
    render_cudaraster();
  } else {
    render_opengl();
  }
#endif
  m_Context->flush();
}


// -----------------------------------------------


void App::initPipe(void)
{  
  // Create surfaces.
  FW::Vec2i screenResolution = FW::Vec2i( App::kScreenWidth, App::kScreenHeight);  
  
  m_colorBuffer = new FW::CudaSurface( screenResolution, 
                                       FW::CudaSurface::FORMAT_RGBA8, 
                                       m_numSamples);

  m_depthBuffer = new FW::CudaSurface( screenResolution, 
                                       FW::CudaSurface::FORMAT_DEPTH32, 
                                       m_numSamples);

  // Compile CUDA code.
  FW::U32 renderModeFlags = 0;
  if (m_enableDepth) renderModeFlags |= FW::RenderModeFlag_EnableDepth;
  if (m_enableLerp)  renderModeFlags |= FW::RenderModeFlag_EnableLerp;
  if (m_enableQuads) renderModeFlags |= FW::RenderModeFlag_EnableQuads;

  m_cudaCompiler.clearDefines();
  m_cudaCompiler.define("SAMPLES_LOG2", m_colorBuffer->getSamplesLog2());
  m_cudaCompiler.define("RENDER_MODE_FLAGS", renderModeFlags);
  m_cudaCompiler.define("BLEND_SHADER", (!m_enableBlend) ? "BlendReplace" : 
                                                           "BlendSrcOver" );

  m_cudaModule = m_cudaCompiler.compile();
    
  if (NULL == m_cudaModule) {
    exit( EXIT_FAILURE );
  }
  
  std::string pipePostfix = "passthrough";
  
  std::string kernelName("vertexShader_" + pipePostfix);
  size_t paramSize = 1 * sizeof(FW::U32) + 2 * sizeof(CUdeviceptr);
  m_vertexShaderKernel = m_cudaModule->getKernel( kernelName, paramSize);

  // Setup CudaRaster.
  std::string pipeName("PixelPipe_" + pipePostfix);
  m_cudaRaster.setSurfaces( m_colorBuffer, m_depthBuffer);
  m_cudaRaster.setPixelPipe( m_cudaModule, pipeName);
}

void App::firstTimeInit(void)
{
#if 0
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

  // Setup default state.
  /*
  printf("Loading mesh...\n");
  loadMesh("scenes/fairyforest/fairyforest.obj");    
  */
#endif
}


// -----------------------------------------------


void App::render_cudaraster()
{
  /*
  Mat4f posToCamera = m_cameraCtrl.getWorldToCamera();
  Mat4f projection = gl->xformFitToView(-1.0f, 2.0f) * m_cameraCtrl.getCameraToClip();
  */
  
  /*
  // Parameters changed => reinitialize pipe.  
  if (m_colorBuffer && m_colorBuffer->getSize() != m_window.getSize()) {
    m_pipeDirty = true;
  }
  */
  if (m_colorBuffer && m_colorBuffer->getNumSamples() != m_numSamples) {
    m_pipeDirty = true;
  }
  if (m_pipeDirty) {
    initPipe();
  }
  m_pipeDirty = false;
  /**/
  
  //----------------------------------

    
  /// ========== 1) Custom VertexShader to transform the vertices ==========
    
  // Set globals. (here, it can be seen as GLSL's uniforms)
  FW::Constants& c = *(FW::Constants*)m_cudaModule->getGlobal("c_constants").getMutablePtrDiscard();
  
  c.posToClip;
  // TODO
  //c.posToClip = projection * posToCamera;
  
  int ofs = 0;
  ofs += m_cudaModule->setParamPtr( m_vertexShaderKernel, ofs, m_inputVertices.getCudaPtr());
  ofs += m_cudaModule->setParamPtr( m_vertexShaderKernel, ofs, m_shadedVertices.getMutableCudaPtrDiscard());
  ofs += m_cudaModule->setParami( m_vertexShaderKernel, ofs, m_numVertices);
  
  FW::Vec2i blockSize(32, 4);
  int numBlocks = (m_numVertices - 1) / (blockSize.x * blockSize.y) + 1;
  m_cudaModule->launchKernel(m_vertexShaderKernel, blockSize, numBlocks);

  /// ========== 2) Run CudaRaster ==========

  m_cudaRaster.deferredClear( FW::Vec4f(0.2f, 0.4f, 0.8f, 1.0f) );
  
  m_cudaRaster.setVertexBuffer(&m_shadedVertices, 0);
  m_cudaRaster.setIndexBuffer(&m_vertexIndices, 0, m_numTriangles);
  fprintf( stderr, "%d\n", __LINE__);
  m_cudaRaster.drawTriangles();
  fprintf( stderr, "%d\n", __LINE__);

  /// ========== 3) Render the buffer as an OpenGL screenquad ==========

  // Render the texture as a Quad mapping the screen's corners
  //m_colorBuffer->resolveToScreen();
  
  /*  
  // Show CudaRaster statistics.
  if (m_showStats)
  {
    CudaRaster::Stats s = m_cudaRaster.getStats();
    m_commonCtrl.message(
        sprintf( "CudaRaster: setup = %.2fms, bin = %.2fms, "\
                 "coarse = %.2fms, fine = %.2fms, total = %.2fms",
        s.setupTime * 1.0e3f,
        s.binTime * 1.0e3f,
        s.coarseTime * 1.0e3f,
        s.fineTime * 1.0e3f,
        (s.setupTime + s.binTime + s.coarseTime + s.fineTime) * 1.0e3f
    ), "cudaRasterStats");
  }
  */
}


void App::render_opengl()
{
  /*
  Mat4f posToCamera = m_cameraCtrl.getWorldToCamera();
  Mat4f projection = gl->xformFitToView(-1.0f, 2.0f) * m_cameraCtrl.getCameraToClip();
  */  
  
  glClearColor( 0.2f, 0.4f, 0.8f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glEnable(GL_CULL_FACE);
  
  glDisable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  
  glDisable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  if (m_enableDepth)  glEnable(GL_DEPTH_TEST);
  if (m_enableBlend)  glEnable(GL_BLEND);

  //m_mesh->draw(gl, posToCamera, projection, NULL, (!m_enableTexPhong));  
  
}

//==============================================================================
