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
 

#ifndef FRAMEWORK_GPU_CUDAMODULE_HPP_
#define FRAMEWORK_GPU_CUDAMODULE_HPP_

#include <map>
#include <string>
#include <vector>

#include <cuda.h>
#include "base/Defs.hpp"
#include "base/Math.hpp"


namespace FW {

class Buffer;

class CudaModule
{
  private:
    typedef std::map<std::string, Buffer*> GlobalMap_t;
    
    typedef std::map<std::string, CUtexref> TexrefMap_t;    
    
    
  private:
    static bool      s_inited;
    static bool      s_available;
    static CUdevice  s_device;
    static CUcontext s_context;
    static CUevent   s_startEvent;
    static CUevent   s_endEvent;
    static bool      s_preferL1;

    CUmodule m_module;
    
    GlobalMap_t m_globalHash;
    
    TexrefMap_t m_texrefHash; // store pointer, not ref
    
    
  public:
    explicit CudaModule(const void* cubin);
    explicit CudaModule(const std::string& cubinFile);
    ~CudaModule(void);

    //++++++++++++
    inline CUmodule getHandle(void) { return m_module; }

    //++++++++++++
    Buffer& getGlobal(const std::string& name);
    
    // copy to the device if modified
    void updateGlobals(bool async = false, CUstream stream = NULL);
    
    void destroysGlobals();

    //++++++++++++
    CUfunction getKernel(const std::string& name, int paramSize = 0);
    int setParami(CUfunction kernel, int offset, S32 value); // returns sizeof(value)
    int setParamf(CUfunction kernel, int offset, F32 value);
    int setParamPtr(CUfunction kernel, int offset, CUdeviceptr value);


    //++++++++++++
    CUtexref getTexRef(const std::string& name);
    
    void setTexRef(const std::string& name, Buffer& buf, CUarray_format format, 
                   int numComponents);
    
    void setTexRef(const std::string& name, CUdeviceptr ptr, S64 size, 
                   CUarray_format format, int numComponents);
    
    void setTexRef(const std::string& name, CUarray cudaArray, bool wrap = true, 
                   bool bilinear = true, bool normalizedCoords = true, 
                   bool readAsInt = false);

    void unsetTexRef(const std::string& name);
    void updateTexRefs(CUfunction kernel);
    

    //++++++++++++
    CUsurfref getSurfRef(const std::string& name);
    void setSurfRef(const std::string& name, CUarray cudaArray);
    
  
    //++++++++++++
    void launchKernel(CUfunction kernel, const Vec2i& blockSize, 
                      const Vec2i& gridSize, bool async = false, 
                      CUstream stream = NULL);
    
    inline
    void launchKernel(CUfunction kernel, const Vec2i& blockSize, int numBlocks, 
                      bool async = false, CUstream stream = NULL) 
    { 
      launchKernel(kernel, blockSize, selectGridSize(numBlocks), async, stream); 
    }

    
    F32 launchKernelTimed(CUfunction kernel, const Vec2i& blockSize, 
                          const Vec2i& gridSize, bool async = false, 
                          CUstream stream = NULL, bool yield = true);

    inline
    F32 launchKernelTimed(CUfunction kernel, const Vec2i& blockSize, 
                          int numBlocks, bool async = false, CUstream stream = NULL) 
    { 
      return launchKernelTimed( kernel, blockSize, selectGridSize(numBlocks), async, stream);
    }


    //++++++++++++
    static void staticInit(void);
    
    static void staticDeinit(void);    
    
    
    static bool isAvailable(void) { staticInit(); return s_available; }
    
    static S64 getMemoryUsed(void);
    
    static void sync(bool yield = true);    
    
    static void checkError(const char* funcName, CUresult res);    
    
    static const char* decodeError(CUresult res);
    

    //++++++++++++
    static CUdevice getDeviceHandle(void) { staticInit(); return s_device; }
    
    static int getDriverVersion(void); // e.g. 23 = 2.3
    
    static int getComputeCapability(void); // e.g. 13 = 1.3
    
    static int getDeviceAttribute(CUdevice_attribute attrib);
    
    static bool setPreferL1OverShared(bool preferL1) 
    { 
      bool old = s_preferL1; 
      s_preferL1 = preferL1; 
      return old; 
    }


  private:
    static CUdevice selectDevice(void);
    
    static void printDeviceInfo(CUdevice device);
    
    static Vec2i selectGridSize(int numBlocks);
    
    CudaModule(const CudaModule&);              // forbidden
    CudaModule& operator= (const CudaModule&);  // forbidden
};

} //namespace FW

#endif //FRAMEWORK_GPU_CUDAMODULE_HPP_
