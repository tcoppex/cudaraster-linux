
#ifndef SHADER_PASSTHROUGH_HPP_
#define SHADER_PASSTHROUGH_HPP_

#include <cudaraster/cuda/PixelPipe.hpp>


namespace FW {

//------------------------------------------------------------------------
// Constants.
//------------------------------------------------------------------------

struct Constants
{
  Mat4f posToClip;  // ModelViewProj matrix
};


//------------------------------------------------------------------------
// Vertex attributes.
//------------------------------------------------------------------------

struct InputVertex
{
  Vec3f modelPos;
};


//------------------------------------------------------------------------
// Varyings.
//------------------------------------------------------------------------

typedef ShadedVertexBase ShadedVertex_passthrough;


//------------------------------------------------------------------------
// Globals.
//------------------------------------------------------------------------

#if FW_CUDA

extern "C" __constant__ Constants c_constants;
//extern "C"  texture<float4, 2> t_textureAtlas;//

extern "C" __global__
void vertexShader_passthrough( const InputVertex *inPtr,
                               ShadedVertex_passthrough *outPtr,
                               int numVertices);

#endif


} // namespace FW

#endif //SHADER_PASSTHROUGH_HPP_
