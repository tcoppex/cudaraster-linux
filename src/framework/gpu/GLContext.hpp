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


// TODO : remove it completely

#pragma once

#include <cassert>
#include <GL/glew.h>

#include "base/Defs.hpp"


namespace FW {


class GLContext
{
  public:
    static void staticInit(void)
    {
      fprintf( stderr, "%s %s : [TODO] remove", __FILE__, __FUNCTION__ );
      /*
      static bool bInit=false;
      
      if (bInit) return;      
      bInit = true;
      
      GLenum err = glewInit();
      assert(err == GLEW_OK);
      */
    }

    static void checkErrors(void)
    {
      GLenum err = glGetError();
      const char* name;
      
      switch (err)
      {
        case GL_NO_ERROR:                       name = NULL; break;
        case GL_INVALID_ENUM:                   name = "GL_INVALID_ENUM"; break;
        case GL_INVALID_VALUE:                  name = "GL_INVALID_VALUE"; break;
        case GL_INVALID_OPERATION:              name = "GL_INVALID_OPERATION"; break;
        case GL_STACK_OVERFLOW:                 name = "GL_STACK_OVERFLOW"; break;
        case GL_STACK_UNDERFLOW:                name = "GL_STACK_UNDERFLOW"; break;
        case GL_OUT_OF_MEMORY:                  name = "GL_OUT_OF_MEMORY"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:  name = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
        default:                                name = "unknown"; break;
      }

      if (name) {
        fail("Caught GL error 0x%04x (%s)!", err, name);
      }
    }
};

//------------------------------------------------------------------------
}
