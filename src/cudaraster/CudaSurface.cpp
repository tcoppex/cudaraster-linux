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
#include <cudaGL.h>         // CUDA driver API OpenGL interoperability
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
    CUresult res = cuGraphicsUnmapResources(1, &m_cudaResource, NULL);
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
    CUresult res = cuGraphicsMapResources(1, &m_cudaResource, NULL);
    CudaModule::checkError("cuGraphicsMapResources", res);
    
    res = cuGraphicsSubResourceGetMappedArray(&m_cudaArray, m_cudaResource, 0, 0);
    CudaModule::checkError("cuGraphicsSubResourceGetMappedArray", res);
    
    m_isMapped = true;
  }
  return m_cudaArray;
}

//------------------------------------------------------------------------

/*
void CudaSurface::resolveToScreen(GLContext* gl)
{
  fprintf( stderr, "%s : not implemented yet.\n", __FUNCTION__ );
  

  S8 s_vertices[] = { -1, -1, +1, -1, +1, +1, -1, +1, };
  FW_ASSERT(gl);

  // Choose fragment shader.
  // Depth => convert to color.
  // 1spp => direct blit.
  // MSAA => average over samples.

  const char* programID;
  const char* fragmentShader;

  if (m_format == FORMAT_DEPTH32)
  {
    programID = "CudaSurface::resolveToScreen_depth";
    fragmentShader =
        "#extension GL_EXT_gpu_shader4 : enable\n"
        "#extension GL_ARB_gpu_shader_fp64 : enable\n"
        FW_GL_SHADER_SOURCE(
            uniform usampler2D texture;
            uniform vec2 texScale;
            uniform int numSamples;
            uniform int tileMask;
            uniform double depthMin;
            uniform double invDepthRange;

            void main()
            {
                int x = int(gl_FragCoord.x);
                x = (x & tileMask) | ((x & ~tileMask) * numSamples);
                vec2 texCoord = vec2(float(x) + 0.5, gl_FragCoord.y) * texScale;
                uint depth = texture2D(texture, texCoord).r;

                double v = double(depth & 0x0000FFFF) + double(depth & 0xFFFF0000); // retain full precision
                v -= depthMin;
                v *= invDepthRange;
                v = clamp(v, 0.0, 1.0);
                v *= double(65536.0 * 65536.0) - 1.0;

                for (int i = 0; i < 4; i++)
                {
                    v = floor(v) * (1.0 / 256.0);
                    gl_FragColor[i] = float((v - floor(v)) * (256.0 / 255.0));
                }
            }
        );
  }
  else if (m_numSamples == 1)
  {
    programID = "CudaSurface::resolveToScreen_1spp";
    fragmentShader =
        FW_GL_SHADER_SOURCE(
            uniform sampler2D texture;
            uniform vec2 texScale;
            void main()
            {
                gl_FragColor = texture2D(texture, gl_FragCoord.xy * texScale);
            }
        );
  }
  else
  {
    programID = "CudaSurface::resolveToScreen_msaa";
    fragmentShader =
        "#extension GL_EXT_gpu_shader4 : enable\n"
        FW_GL_SHADER_SOURCE(
            uniform sampler2D texture;
            uniform vec2 texScale;
            uniform int numSamples;
            uniform float invSamples;
            uniform int tileMask;
            uniform float tileStep;

            void main()
            {
                int x = int(gl_FragCoord.x);
                x = (x & tileMask) | ((x & ~tileMask) * numSamples);
                vec2 texCoord = vec2(float(x) + 0.5, gl_FragCoord.y) * texScale;

                vec4 sum = vec4(0.0);
                for (int i = 0; i < numSamples; i++)
                {
                    sum += texture2D(texture, texCoord);
                    texCoord.x += tileStep;
                }
                gl_FragColor = sum * invSamples;
            }
        );
  }

  // Create shader program.
  GLContext::Program* prog = gl->getProgram(programID);
  if (!prog)
  {
    prog = new GLContext::Program(
        FW_GL_SHADER_SOURCE(
            attribute vec2 pos;
            void main()
            {
                gl_Position = vec4(pos, 0.0, 1.0);
            }
        ),
        fragmentShader);
    gl->setProgram(programID, prog);
  }

  // Draw.

  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, getGLTexture());

  prog->use();
  gl->setUniform(prog->getUniformLoc("texture"), 0);
  gl->setUniform(prog->getUniformLoc("texScale"), 1.0f / Vec2f(m_textureSize));
  gl->setUniform(prog->getUniformLoc("numSamples"), m_numSamples);
  gl->setUniform(prog->getUniformLoc("invSamples"), 1.0f / (F32)m_numSamples);
  gl->setUniform(prog->getUniformLoc("tileMask"), CR_TILE_SIZE - 1);
  gl->setUniform(prog->getUniformLoc("tileStep"), (F32)CR_TILE_SIZE / (F32)m_textureSize.x);
  gl->setUniform(prog->getUniformLoc("depthMin"), (F64)CR_DEPTH_MIN);
  gl->setUniform(prog->getUniformLoc("invDepthRange"), 1.0 / (F64)(CR_DEPTH_MAX - CR_DEPTH_MIN));
  gl->setAttrib(prog->getAttribLoc("pos"), 2, GL_BYTE, 0, s_vertices);

  glDrawArrays(GL_QUADS, 0, 4);

  gl->resetAttribs();
  glPopAttrib();
  GLContext::checkErrors();
}
*/

//------------------------------------------------------------------------
} // namespace FW
