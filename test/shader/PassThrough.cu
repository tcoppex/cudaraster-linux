
#include "PassThrough.hpp"
#include <cudaraster/cuda/PixelPipe.inl>


using namespace FW;


//==============================================================================

//------------------------------------------------------------------------
// Vertex shaders.
//------------------------------------------------------------------------

// test of a simple 'passthrough' vertex shader
extern "C" __global__ 
void FW::vertexShader_passthrough( const InputVertex* inPtr,              // IN
                                   ShadedVertex_passthrough* outPtr,      // OUT
                                   int numVertices)                       // IN
{
  int VertexID = threadIdx.x + blockDim.x * (threadIdx.y + blockDim.y * 
                 (blockIdx.x + gridDim.x * blockIdx.y));

  if (VertexID >= numVertices) {
    return;
  }

  const InputVertex&        in  = inPtr[VertexID];
  ShadedVertex_passthrough& out = outPtr[VertexID];

  Vec4f inPosition      = Vec4f( in.modelPos, 1.0f);
  Mat4f &uModelViewProj = c_constants.posToClip;
  
  out.clipPos = uModelViewProj * inPosition;
}


//==============================================================================

//------------------------------------------------------------------------
// Fragment shaders.
//------------------------------------------------------------------------

class FragmentShader_passthrough : public FragmentShaderBase
{
  public:
    // Override
    __device__ __inline__ void run(void)
    {
      Vec3f red_color = Vec3f( 1.0f, 0.0f, 0.0f);
      
      m_color = toABGR( Vec4f( red_color, 1.0f) );
    }
};


//==============================================================================

//------------------------------------------------------------------------
// Pixel pipes.
//------------------------------------------------------------------------


CR_DEFINE_PIXEL_PIPE( PixelPipe_passthrough,
                      ShadedVertex_passthrough, 
                      FragmentShader_passthrough, BLEND_SHADER, 
                      SAMPLES_LOG2, RENDER_MODE_FLAGS)


//==============================================================================

