/*
 * Modified version, originally from Samuli Laine's and Tero Karras' CudaRaster.
 * (http://code.google.com/p/cudaraster/)
 * 
 * 04-2012 - Thibault Coppex
 * 
 * ---------------------------------------------------------------------------
 * 
 *  Copyright 2009-2010 NVIDIA Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once
#include <cudaraster/cuda/PixelPipe.hpp>


namespace FW {
//------------------------------------------------------------------------
// Texture specification.
//------------------------------------------------------------------------

struct TextureSpec
{
    Vec2f       size;
    U32         pad[2];
    Vec4f       mipLevels[16];       // scaleX, scaleY, biasX, biasY
};

//------------------------------------------------------------------------
// Material parameters.
//------------------------------------------------------------------------

struct Material
{
    Vec4f       diffuseColor;
    Vec3f       specularColor;
    F32         glossiness;
    TextureSpec diffuseTexture;
    TextureSpec alphaTexture;
};

//------------------------------------------------------------------------
// Constants.
//------------------------------------------------------------------------

struct Constants
{
    Mat4f       posToClip;
    Mat4f       posToCamera;
    Mat3f       normalToCamera;
    CUdeviceptr materials;              // numMaterials * Material
    CUdeviceptr vertexMaterialIdx;      // numVertices * S32
    CUdeviceptr triangleMaterialIdx;    // numTriangles * S32
};

//------------------------------------------------------------------------
// Vertex attributes.
//------------------------------------------------------------------------

struct InputVertex
{
    Vec3f       modelPos;
    Vec3f       modelNormal;
    Vec2f       texCoord;
};

//------------------------------------------------------------------------
// Varyings.
//------------------------------------------------------------------------

typedef GouraudVertex ShadedVertex_gouraud;

//------------------------------------------------------------------------

struct ShadedVertex_texPhong : ShadedVertexBase
{
    Vec4f       cameraPos;      // Varying 0.
    Vec4f       cameraNormal;   // Varying 1.
    Vec4f       texCoord;       // Varying 2.
};

//------------------------------------------------------------------------
// Globals.
//------------------------------------------------------------------------

#if FW_CUDA

extern "C" __constant__ Constants c_constants;
extern "C"  texture<float4, 2> t_textureAtlas;

__device__              Vec3f   evaluateLighting        (Vec3f cameraPos, Vec3f cameraNormal, const Material& material, Vec3f diffuseColor);
extern "C" __global__   void    vertexShader_gouraud    (const InputVertex* inPtr, ShadedVertex_gouraud*  outPtr, int numVertices);
extern "C" __global__   void    vertexShader_texPhong   (const InputVertex* inPtr, ShadedVertex_texPhong* outPtr, int numVertices);

// CR_DEFINE_PIXEL_PIPE(PixelPipe_gouraud, ...)
// CR_DEFINE_PIXEL_PIPE(PixelPipe_texPhong, ...)

#endif

//------------------------------------------------------------------------
}
