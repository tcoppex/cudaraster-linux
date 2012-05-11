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
 

#include "CudaSurface.hpp"

#include <cassert>
#include <cudaGL.h>             // CUDA driver API OpenGL interoperability
#include "base/Defs.hpp"
#include "gpu/CudaModule.hpp"

#include "cuda/Constants.hpp"


namespace FW {

//------------------------------------------------------------------------

CudaSurface::CudaSurface(const Vec2i& size, Format format, int numSamples)
  : m_isMapped  (false),
    m_cudaArray (0)
{
  // Check parameters.
  if (min(size) <= 0) {
    fail("CudaSurface: Size must be positive!");
  }

  if (max(size) > CR_MAXVIEWPORT_SIZE) {
    fail("CudaSurface: CR_MAXVIEWPORT_SIZE exceeded!");
  }
  
  if (format < 0 || format >= NUM_FORMAT) {
    fail("CudaSurface: Invalid format!");
  }
  
  if (numSamples > 8) {
    fail("CudaSurface: numSamples cannot exceed 8!");
  }
  
  if (numSamples < 1 || popc8(numSamples) != 1) {
    fail("CudaSurface: numSamples must be a power of two!");
  }

  // Identify format.
  int glInternal, glFormat, glType;

  switch (format)
  {
    case FORMAT_RGBA8:    
      glInternal = GL_RGBA; 
      glFormat = GL_RGBA; 
      glType = GL_UNSIGNED_BYTE; 
    break;
    
    case FORMAT_DEPTH32:        
      glInternal = GL_LUMINANCE32UI_EXT; 
      glFormat = GL_LUMINANCE_INTEGER_EXT; 
      glType = GL_UNSIGNED_INT; 
    break;
    
    default:
      FW_ASSERT(false); 
    return;
  }

  // Initialize.
  m_size        = size;
  m_roundedSize = (size + CR_TILE_SIZE - 1) & -CR_TILE_SIZE;
  m_textureSize = m_roundedSize * Vec2i(numSamples, 1);
  m_format      = format;
  m_numSamples  = numSamples;

  // Create GL texture.
  glGenTextures(1, &m_glTexture);
  glBindTexture(GL_TEXTURE_2D, m_glTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, glInternal, m_textureSize.x, m_textureSize.y, 0, 
                              glFormat, glType, NULL);
  GLContext::checkErrors();

  // Register to CUDA.
  CudaModule::staticInit(); // 

  CUresult res = cuGraphicsGLRegisterImage( &m_cudaResource, 
                                            m_glTexture, 
                                            GL_TEXTURE_2D, 
                                            CU_GRAPHICS_REGISTER_FLAGS_SURFACE_LDST);
  CudaModule::checkError("cuGraphicsGLRegisterImage", res);
}

//------------------------------------------------------------------------

CudaSurface::~CudaSurface(void)
{
  getGLTexture(); // unmap
  cuGraphicsUnregisterResource(m_cudaResource);
  glDeleteTextures(1, &m_glTexture);
}

//------------------------------------------------------------------------

GLuint CudaSurface::getGLTexture(void)
{
  if (m_isMapped)
  {
    CUresult res = cuGraphicsUnmapResources(1u, &m_cudaResource, NULL);
    CudaModule::checkError("cuGraphicsUnmapResources", res);
    m_isMapped = false;
  }
  return m_glTexture;
}

//------------------------------------------------------------------------

CUarray CudaSurface::getCudaArray(void)
{
  if (!m_isMapped)
  {
    CUresult res = cuGraphicsMapResources(1u, &m_cudaResource, NULL);
    CudaModule::checkError("cuGraphicsMapResources", res);
    
    res = cuGraphicsSubResourceGetMappedArray(&m_cudaArray, m_cudaResource, 0, 0);
    CudaModule::checkError("cuGraphicsSubResourceGetMappedArray", res);
    
    m_isMapped = true;
  }
  return m_cudaArray;
}


} // namespace FW
