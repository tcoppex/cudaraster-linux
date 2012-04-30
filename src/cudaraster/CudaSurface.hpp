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

#ifndef CUDARASTER_CUDASURFACE_HPP_
#define CUDARASTER_CUDASURFACE_HPP_

#include <cuda.h>
#include "gpu/GLContext.hpp"
#include "base/Math.hpp"

namespace FW {
//------------------------------------------------------------------------
// Render target for CudaRaster, visible in OpenGL as a 2D texture.
//------------------------------------------------------------------------

class CudaSurface
{
  public:
    enum Format
    {
      FORMAT_RGBA8  = 0,          // U8 red, U8 green, U8 blue, U8 alpha
      FORMAT_DEPTH32,             // U32 depth

      NUM_FORMAT
    };

  private:
    Vec2i               m_size;
    Vec2i               m_roundedSize;
    Vec2i               m_textureSize;
    Format              m_format;
    S32                 m_numSamples;

    GLuint              m_glTexture;
    CUgraphicsResource  m_cudaResource;
    bool                m_isMapped;
    CUarray             m_cudaArray;
  
  public:
    CudaSurface(const Vec2i& size, Format format, int numSamples = 1);
    ~CudaSurface(void);

    const Vec2i&        getSize         (void) const    { return m_size; }          // Original size specified in the constructor.
    const Vec2i&        getRoundedSize  (void) const    { return m_roundedSize; }   // Rounded to full 8x8 tiles.
    const Vec2i&        getTextureSize  (void) const    { return m_textureSize; }   // 8x8 tiles are replicated horizontally for MSAA.
    Format              getFormat       (void) const    { return m_format; }
    int                 getNumSamples   (void) const    { return m_numSamples; }
    int                 getSamplesLog2  (void) const    { return popc8(m_numSamples - 1); } // log2(numSamples)

    GLuint              getGLTexture    (void);             // Invalidates the CUDA array.
    CUarray             getCudaArray    (void);             // Invalidates the GL texture.

    //void                resolveToScreen (GLContext* gl);    // Resolves MSAA and writes pixels into the current GL render target.

private:
	CudaSurface                         (const CudaSurface&); // forbidden
	CudaSurface&        operator=       (const CudaSurface&); // forbidden
};

} //namespace FW

#endif //CUDARASTER_CUDASURFACE_HPP_
