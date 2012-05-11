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
 
#include "CudaModule.hpp"

#include <GL/glew.h>
#include <cuda.h>
#include <cudaGL.h>

#include "gpu/Buffer.hpp"
#include "gpu/CudaCompiler.hpp"



namespace FW {

bool        CudaModule::s_inited        = false;
bool        CudaModule::s_available     = false;
CUdevice    CudaModule::s_device        = 0;
CUcontext   CudaModule::s_context       = NULL;
CUevent     CudaModule::s_startEvent    = NULL;
CUevent     CudaModule::s_endEvent      = NULL;
bool        CudaModule::s_preferL1      = true;


//------------------------------------------------------------------------

CudaModule::CudaModule(const void* cubin)
{
  staticInit();
  checkError("cuModuleLoadData", cuModuleLoadData(&m_module, cubin));
}

CudaModule::CudaModule(const std::string& cubinFile)
{
  staticInit();
  checkError("cuModuleLoad", cuModuleLoad(&m_module, cubinFile.c_str()));
}

CudaModule::~CudaModule(void)
{
  destroysGlobals();  
  checkError("cuModuleUnload", cuModuleUnload(m_module));
}

//------------------------------------------------------------------------

Buffer& CudaModule::getGlobal(const std::string& name)
{
  Buffer* found = m_globalHash[name];
  
  if (found) {
    return *found;
  }

  CUdeviceptr ptr;
  size_t size;
  
  checkError( "cuModuleGetGlobal", 
              cuModuleGetGlobal(&ptr, &size, m_module, name.c_str()));

  Buffer* buffer = new Buffer;  
  buffer->wrapCuda(ptr, size);

  m_globalHash[name] = buffer;
  return *buffer;
}

void CudaModule::updateGlobals(bool async, CUstream stream)
{ 
  GlobalMap_t::iterator it;
  for (it=m_globalHash.begin(); it != m_globalHash.end(); ++it) {
    it->second->setOwner( Buffer::Cuda, true, async, stream);
  }
}

void CudaModule::destroysGlobals()
{
  GlobalMap_t::iterator it;
  for (it=m_globalHash.begin(); it!=m_globalHash.end(); ++it) {
    delete it->second;
  }
}


//------------------------------------------------------------------------

CUfunction CudaModule::getKernel(const std::string& name, int paramSize)
{
  CUfunction kernel = NULL;
  cuModuleGetFunction(&kernel, m_module, name.c_str());
  
  if (!kernel) {
    std::string funcName(std::string("__globfunc_") + name);    
    cuModuleGetFunction( &kernel, m_module, funcName.c_str() );
  }
  
  if (kernel) {
    checkError( "cuParamSetSize", cuParamSetSize(kernel, paramSize));
  }
  return kernel;
}

int CudaModule::setParami(CUfunction kernel, int offset, S32 value)
{
  if (kernel) {
    checkError( "cuParamSeti", cuParamSeti(kernel, offset, value));
  }
  return sizeof(S32);
}

int CudaModule::setParamf(CUfunction kernel, int offset, F32 value)
{
  if (kernel) {
    checkError( "cuParamSetf", cuParamSetf(kernel, offset, value));
  }
  return sizeof(F32);
}

int CudaModule::setParamPtr(CUfunction kernel, int offset, CUdeviceptr value)
{
  if (kernel) {
    checkError( "cuParamSetv", cuParamSetv(kernel, offset, &value, sizeof(CUdeviceptr)));
  }
  return sizeof(CUdeviceptr);
}

//------------------------------------------------------------------------

CUtexref CudaModule::getTexRef(const std::string& name)
{
  CUtexref &texref = m_texrefHash[name];

  if (0 == texref) {
    checkError("cuModuleGetTexRef", cuModuleGetTexRef( &texref, m_module, name.c_str()));  
  } 

  return texref;
}

void CudaModule::setTexRef( const std::string& name, 
                            Buffer& buf, 
                            CUarray_format format, 
                            int numComponents)
{  
  setTexRef( name, buf.getCudaPtr(), buf.getSize(), format, numComponents);
}


void CudaModule::setTexRef( const std::string& name, 
                            CUdeviceptr ptr, 
                            S64 size, 
                            CUarray_format format, 
                            int numComponents)
{
  CUtexref texRef = getTexRef(name);
  
  checkError("cuTexRefSetFormat", cuTexRefSetFormat(texRef, format, numComponents));
  checkError("cuTexRefSetAddress", cuTexRefSetAddress(NULL, texRef, ptr, (U32)size));
}

void CudaModule::setTexRef( const std::string& name, 
                            CUarray cudaArray, 
                            bool wrap, 
                            bool bilinear, 
                            bool normalizedCoords, 
                            bool readAsInt)
{
  U32 flags = 0;
  if (normalizedCoords) {
    flags |= CU_TRSF_NORMALIZED_COORDINATES;
  }
  if (readAsInt) {
    flags |= CU_TRSF_READ_AS_INTEGER;
  }

  CUaddress_mode addressMode;
  CUfilter_mode filterMode;
  
  addressMode = (wrap) ? CU_TR_ADDRESS_MODE_WRAP : CU_TR_ADDRESS_MODE_CLAMP;
  filterMode = (bilinear) ? CU_TR_FILTER_MODE_LINEAR : CU_TR_FILTER_MODE_POINT;
  
  CUtexref texRef = getTexRef(name);
  for (int dim=0; dim<3; ++dim) 
  {
    checkError( "cuTexRefSetAddressMode", 
                 cuTexRefSetAddressMode(texRef, dim, addressMode));
  }
  
  checkError("cuTexRefSetFilterMode", cuTexRefSetFilterMode(texRef, filterMode));
  checkError("cuTexRefSetFlags", cuTexRefSetFlags(texRef, flags));
  checkError("cuTexRefSetArray", cuTexRefSetArray(texRef, cudaArray, CU_TRSA_OVERRIDE_FORMAT));
}

void CudaModule::unsetTexRef(const std::string& name)
{
  CUtexref texRef = getTexRef(name);
  checkError("cuTexRefSetAddress", cuTexRefSetAddress( 0, texRef, 0, 0));
}

void CudaModule::updateTexRefs(CUfunction kernel)
{
  if (getDriverVersion() >= 32) {
    return;
  }

  TexrefMap_t::iterator it;
  for (it=m_texrefHash.begin(); it!=m_texrefHash.end(); ++it)
  {
    checkError("cuParamSetTexRef", 
               cuParamSetTexRef( kernel, CU_PARAM_TR_DEFAULT, it->second));
  }
}

//------------------------------------------------------------------------

CUsurfref CudaModule::getSurfRef(const std::string& name)
{
  
#if (CUDA_VERSION >= 3010)
  CUsurfref surfRef;
  checkError( "cuModuleGetSurfRef", 
              cuModuleGetSurfRef(&surfRef, m_module, name.c_str()) );  
  return surfRef;
#else
  FW_UNREF(name);
  fail("CudaModule: getSurfRef() requires CUDA 3.1 or later!");
  return NULL;
#endif
}

void CudaModule::setSurfRef(const std::string& name, CUarray cudaArray)
{
#if (CUDA_VERSION >= 3010)
  checkError("cuSurfRefSetArray", cuSurfRefSetArray(getSurfRef(name), cudaArray, 0));
#else
  FW_UNREF(name);
  FW_UNREF(cudaArray);
  fail("CudaModule: setSurfRef() requires CUDA 3.1 or later!");
#endif
}

//------------------------------------------------------------------------

void CudaModule::launchKernel(CUfunction kernel, const Vec2i& blockSize, 
                              const Vec2i& gridSize, bool async, 
                              CUstream stream)
{
  if (!kernel) {
    fail("CudaModule: No kernel specified!");
  }

#if (CUDA_VERSION >= 3000)
  if (NULL != cuFuncSetCacheConfig)
  {
    CUfunc_cache cache = (s_preferL1)? CU_FUNC_CACHE_PREFER_L1 : 
                                       CU_FUNC_CACHE_PREFER_SHARED;  
    checkError("cuFuncSetCacheConfig", cuFuncSetCacheConfig( kernel, cache) );
  }
#endif

  updateGlobals();
  updateTexRefs(kernel);
  checkError("cuFuncSetBlockShape", cuFuncSetBlockShape(kernel, blockSize.x, blockSize.y, 1));

  if (async && (NULL != cuLaunchGridAsync)) 
  {
    checkError("cuLaunchGridAsync", 
                cuLaunchGridAsync(kernel, gridSize.x, gridSize.y, stream));
  } 
  else 
  {
    checkError("cuLaunchGrid", 
                cuLaunchGrid(kernel, gridSize.x, gridSize.y));
  }
}

F32 CudaModule::launchKernelTimed(CUfunction kernel, const Vec2i& blockSize, 
                                  const Vec2i& gridSize, bool async, 
                                  CUstream stream, bool yield)
{
  // Update globals before timing.
  updateGlobals();
  updateTexRefs(kernel);
  sync(false);


  // Events not supported => use CPU-based timer.
  if (!s_startEvent)
  {
    assert(0);//
#if 0
    Timer timer(true);
    launchKernel(kernel, blockSize, gridSize, async, stream);
    sync(false); // spin for more accurate timing
    return timer.getElapsed();
#endif
  }


  // Use events.
  checkError("cuEventRecord", cuEventRecord(s_startEvent, NULL));
  launchKernel(kernel, blockSize, gridSize, async, stream);
  checkError("cuEventRecord", cuEventRecord(s_endEvent, NULL));
  sync(yield);

  F32 time;
  checkError("cuEventElapsedTime", cuEventElapsedTime(&time, s_startEvent, s_endEvent));
  return time * 1.0e-3f;
}

//------------------------------------------------------------------------

void CudaModule::staticInit(void)
{
  if (s_inited) {
    return;
  }
  
  s_inited = true;
  s_available = false;

  checkError("cuInit", cuInit(0));
  s_available = true;
  
  s_device = selectDevice();
  printDeviceInfo(s_device);

  U32 flags = 0;
  flags |= CU_CTX_SCHED_SPIN; // use sync() if you want to yield
  
#if (CUDA_VERSION >= 2030)
  if (getDriverVersion() >= 23) 
  {
    // reduce launch overhead with large localmem
    flags |= CU_CTX_LMEM_RESIZE_TO_MAX; 
  }
#endif

  // OpenGL & window context must have been initialized !
  checkError("cuGLCtxCreate", cuGLCtxCreate( &s_context, flags, s_device));

  checkError("cuEventCreate", cuEventCreate(&s_startEvent, 0));
  checkError("cuEventCreate", cuEventCreate(&s_endEvent, 0));
}

void CudaModule::staticDeinit(void)
{
  if (!s_inited) {
    return;
  }  
  s_inited = false;

  if (s_startEvent) {
    checkError("cuEventDestroy", cuEventDestroy(s_startEvent));
  }
  s_startEvent = NULL;

  if (s_endEvent) {
    checkError("cuEventDestroy", cuEventDestroy(s_endEvent));
  }
  s_endEvent = NULL;

  if (s_context) {
    checkError("cuCtxDestroy", cuCtxDestroy(s_context));
  }
  s_context = NULL;
  
  s_device = 0;
}

S64 CudaModule::getMemoryUsed(void)
{
  staticInit();

  if (!s_available) {
    return 0;
  }

  size_t free = 0;
  size_t total = 0;
  cuMemGetInfo(&free, &total);
  return total - free;
}

//------------------------------------------------------------------------

void CudaModule::sync(bool yield)
{
  if (!s_inited) {
    return;
  }

  if (!yield || !s_endEvent) {
    checkError("cuCtxSynchronize", cuCtxSynchronize());
    return;
  }
}

//------------------------------------------------------------------------

const char* CudaModule::decodeError(CUresult res)
{
  const char* error;
  switch (res)
  {
  default:                                        error = "Unknown CUresult"; break;
  case CUDA_SUCCESS:                              error = "No error"; break;
  case CUDA_ERROR_INVALID_VALUE:                  error = "Invalid value"; break;
  case CUDA_ERROR_OUT_OF_MEMORY:                  error = "Out of memory"; break;
  case CUDA_ERROR_NOT_INITIALIZED:                error = "Not initialized"; break;
  case CUDA_ERROR_DEINITIALIZED:                  error = "Deinitialized"; break;
  case CUDA_ERROR_NO_DEVICE:                      error = "No device"; break;
  case CUDA_ERROR_INVALID_DEVICE:                 error = "Invalid device"; break;
  case CUDA_ERROR_INVALID_IMAGE:                  error = "Invalid image"; break;
  case CUDA_ERROR_INVALID_CONTEXT:                error = "Invalid context"; break;
  case CUDA_ERROR_CONTEXT_ALREADY_CURRENT:        error = "Context already current"; break;
  case CUDA_ERROR_MAP_FAILED:                     error = "Map failed"; break;
  case CUDA_ERROR_UNMAP_FAILED:                   error = "Unmap failed"; break;
  case CUDA_ERROR_ARRAY_IS_MAPPED:                error = "Array is mapped"; break;
  case CUDA_ERROR_ALREADY_MAPPED:                 error = "Already mapped"; break;
  case CUDA_ERROR_NO_BINARY_FOR_GPU:              error = "No binary for GPU"; break;
  case CUDA_ERROR_ALREADY_ACQUIRED:               error = "Already acquired"; break;
  case CUDA_ERROR_NOT_MAPPED:                     error = "Not mapped"; break;
  case CUDA_ERROR_INVALID_SOURCE:                 error = "Invalid source"; break;
  case CUDA_ERROR_FILE_NOT_FOUND:                 error = "File not found"; break;
  case CUDA_ERROR_INVALID_HANDLE:                 error = "Invalid handle"; break;
  case CUDA_ERROR_NOT_FOUND:                      error = "Not found"; break;
  case CUDA_ERROR_NOT_READY:                      error = "Not ready"; break;
  case CUDA_ERROR_LAUNCH_FAILED:                  error = "Launch failed"; break;
  case CUDA_ERROR_LAUNCH_OUT_OF_RESOURCES:        error = "Launch out of resources"; break;
  case CUDA_ERROR_LAUNCH_TIMEOUT:                 error = "Launch timeout"; break;
  case CUDA_ERROR_LAUNCH_INCOMPATIBLE_TEXTURING:  error = "Launch incompatible texturing"; break;
  case CUDA_ERROR_UNKNOWN:                        error = "Unknown error"; break;

#if (CUDA_VERSION >= 4000) // TODO: Some of these may exist in earlier versions, too.
  case CUDA_ERROR_PROFILER_DISABLED:              error = "Profiler disabled"; break;
  case CUDA_ERROR_PROFILER_NOT_INITIALIZED:       error = "Profiler not initialized"; break;
  case CUDA_ERROR_PROFILER_ALREADY_STARTED:       error = "Profiler already started"; break;
  case CUDA_ERROR_PROFILER_ALREADY_STOPPED:       error = "Profiler already stopped"; break;
  case CUDA_ERROR_NOT_MAPPED_AS_ARRAY:            error = "Not mapped as array"; break;
  case CUDA_ERROR_NOT_MAPPED_AS_POINTER:          error = "Not mapped as pointer"; break;
  case CUDA_ERROR_ECC_UNCORRECTABLE:              error = "ECC uncorrectable"; break;
  case CUDA_ERROR_UNSUPPORTED_LIMIT:              error = "Unsupported limit"; break;
  case CUDA_ERROR_CONTEXT_ALREADY_IN_USE:         error = "Context already in use"; break;
  case CUDA_ERROR_SHARED_OBJECT_SYMBOL_NOT_FOUND: error = "Shared object symbol not found"; break;
  case CUDA_ERROR_SHARED_OBJECT_INIT_FAILED:      error = "Shared object init failed"; break;
  case CUDA_ERROR_OPERATING_SYSTEM:               error = "Operating system error"; break;
  case CUDA_ERROR_PEER_ACCESS_ALREADY_ENABLED:    error = "Peer access already enabled"; break;
  case CUDA_ERROR_PEER_ACCESS_NOT_ENABLED:        error = "Peer access not enabled"; break;
  case CUDA_ERROR_PRIMARY_CONTEXT_ACTIVE:         error = "Primary context active"; break;
  case CUDA_ERROR_CONTEXT_IS_DESTROYED:           error = "Context is destroyed"; break;
#endif
  }
  return error;
}

//------------------------------------------------------------------------

void CudaModule::checkError(const char* funcName, CUresult res)
{
  if (res != CUDA_SUCCESS) {
    fail( "%s() failed: %s!", funcName, decodeError(res));
  }
}

//------------------------------------------------------------------------

int CudaModule::getDriverVersion(void)
{
  int version = 2010;
#if (CUDA_VERSION >= 2020)
  cuDriverGetVersion(&version);
#endif
  version /= 10;
  return version / 10 + version % 10;
}

int CudaModule::getComputeCapability(void)
{
  staticInit();
  
  if (!s_available) {
    return 0;
  }

  int major, minor;
  checkError( "cuDeviceComputeCapability", 
              cuDeviceComputeCapability(&major, &minor, s_device));
              
  return major * 10 + minor;
}

int CudaModule::getDeviceAttribute(CUdevice_attribute attrib)
{
  staticInit();

  if (!s_available) {
    return 0;
  }

  int value;
  checkError( "cuDeviceGetAttribute", 
              cuDeviceGetAttribute(&value, attrib, s_device));
  
  return value;
}

//------------------------------------------------------------------------

CUdevice CudaModule::selectDevice(void)
{  
  CUresult res = CUDA_SUCCESS;
  
  int numDevices;
  checkError("cuDeviceGetCount", cuDeviceGetCount(&numDevices));

  CUdevice device = 0;
  S32 bestScore = FW_S32_MIN;
  
  for (int i=0; i<numDevices; ++i)
  {
    CUdevice dev;
    checkError("cuDeviceGet", cuDeviceGet(&dev, i));

    int clockRate;
    res = cuDeviceGetAttribute(&clockRate, CU_DEVICE_ATTRIBUTE_CLOCK_RATE, dev);
    checkError("cuDeviceGetAttribute", res);

    int numProcessors;
    res = cuDeviceGetAttribute(&numProcessors, 
                               CU_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT, dev);
    checkError("cuDeviceGetAttribute", res);
    
    S32 score = clockRate * numProcessors;
    if (score > bestScore)
    {
      device = dev;
      bestScore = score;
    }
  }

  if (bestScore == FW_S32_MIN) {
    fail("No appropriate CUDA device found!");
  }
  
  return device;
}

void CudaModule::printDeviceInfo(CUdevice device)
{
    static const struct
    {
        CUdevice_attribute  attrib;
        const char*         name;
    } attribs[] =
    {
#define A21(ENUM, NAME) { CU_DEVICE_ATTRIBUTE_ ## ENUM, NAME },
#if (CUDA_VERSION >= 4000)
#   define A40(ENUM, NAME) A21(ENUM, NAME)
#else
#   define A40(ENUM, NAME) // TODO: Some of these may exist in earlier versions, too.
#endif

        A21(CLOCK_RATE,                         "Clock rate")
        A40(MEMORY_CLOCK_RATE,                  "Memory clock rate")
        A21(MULTIPROCESSOR_COUNT,               "Number of SMs")
//      A40(GLOBAL_MEMORY_BUS_WIDTH,            "DRAM bus width")
//      A40(L2_CACHE_SIZE,                      "L2 cache size")

        A21(MAX_THREADS_PER_BLOCK,              "Max threads per block")
        A40(MAX_THREADS_PER_MULTIPROCESSOR,     "Max threads per SM")
        A21(REGISTERS_PER_BLOCK,                "Registers per block")
//      A40(MAX_REGISTERS_PER_BLOCK,            "Max registers per block")
        A21(SHARED_MEMORY_PER_BLOCK,            "Shared mem per block")
//      A40(MAX_SHARED_MEMORY_PER_BLOCK,        "Max shared mem per block")
        A21(TOTAL_CONSTANT_MEMORY,              "Constant memory")
//      A21(WARP_SIZE,                          "Warp size")

        A21(MAX_BLOCK_DIM_X,                    "Max blockDim.x")
//      A21(MAX_BLOCK_DIM_Y,                    "Max blockDim.y")
//      A21(MAX_BLOCK_DIM_Z,                    "Max blockDim.z")
        A21(MAX_GRID_DIM_X,                     "Max gridDim.x")
//      A21(MAX_GRID_DIM_Y,                     "Max gridDim.y")
//      A21(MAX_GRID_DIM_Z,                     "Max gridDim.z")
//      A40(MAXIMUM_TEXTURE1D_WIDTH,            "Max tex1D.x")
//      A40(MAXIMUM_TEXTURE2D_WIDTH,            "Max tex2D.x")
//      A40(MAXIMUM_TEXTURE2D_HEIGHT,           "Max tex2D.y")
//      A40(MAXIMUM_TEXTURE3D_WIDTH,            "Max tex3D.x")
//      A40(MAXIMUM_TEXTURE3D_HEIGHT,           "Max tex3D.y")
//      A40(MAXIMUM_TEXTURE3D_DEPTH,            "Max tex3D.z")
//      A40(MAXIMUM_TEXTURE1D_LAYERED_WIDTH,    "Max layerTex1D.x")
//      A40(MAXIMUM_TEXTURE1D_LAYERED_LAYERS,   "Max layerTex1D.y")
//      A40(MAXIMUM_TEXTURE2D_LAYERED_WIDTH,    "Max layerTex2D.x")
//      A40(MAXIMUM_TEXTURE2D_LAYERED_HEIGHT,   "Max layerTex2D.y")
//      A40(MAXIMUM_TEXTURE2D_LAYERED_LAYERS,   "Max layerTex2D.z")
//      A40(MAXIMUM_TEXTURE2D_ARRAY_WIDTH,      "Max array.x")
//      A40(MAXIMUM_TEXTURE2D_ARRAY_HEIGHT,     "Max array.y")
//      A40(MAXIMUM_TEXTURE2D_ARRAY_NUMSLICES,  "Max array.z")

//      A21(MAX_PITCH,                          "Max memcopy pitch")
//      A21(TEXTURE_ALIGNMENT,                  "Texture alignment")
//      A40(SURFACE_ALIGNMENT,                  "Surface alignment")

        A40(CONCURRENT_KERNELS,                 "Concurrent launches supported")
        A21(GPU_OVERLAP,                        "Concurrent memcopy supported")
        A40(ASYNC_ENGINE_COUNT,                 "Max concurrent memcopies")
//      A40(KERNEL_EXEC_TIMEOUT,                "Kernel launch time limited")
//      A40(INTEGRATED,                         "Integrated with host memory")
        A40(UNIFIED_ADDRESSING,                 "Unified addressing supported")
        A40(CAN_MAP_HOST_MEMORY,                "Can map host memory")
        A40(ECC_ENABLED,                        "ECC enabled")

//      A40(TCC_DRIVER,                         "Driver is TCC")
//      A40(COMPUTE_MODE,                       "Compute exclusivity mode")

//      A40(PCI_BUS_ID,                         "PCI bus ID")
//      A40(PCI_DEVICE_ID,                      "PCI device ID")
//      A40(PCI_DOMAIN_ID,                      "PCI domain ID")

#undef A21
#undef A40
    };

    char name[256];
    int major;
    int minor;
    size_t memory;

    checkError("cuDeviceGetName", cuDeviceGetName(name, FW_ARRAY_SIZE(name) - 1, device));
    checkError("cuDeviceComputeCapability", cuDeviceComputeCapability(&major, &minor, device));
    checkError("cuDeviceTotalMem", cuDeviceTotalMem(&memory, device));
    name[FW_ARRAY_SIZE(name) - 1] = '\0';

    printf("\n");
    char deviceIdStr[16];
    sprintf( deviceIdStr, "CUDA device %d", device);
    printf("%-32s%s\n",deviceIdStr, name);
        
    printf("%-32s%s\n", "---", "---");
    
    int version = getDriverVersion();
    printf("%-32s%d.%d\n", "CUDA driver API version", version/10, version%10);
    printf("%-32s%d.%d\n", "Compute capability", major, minor);
    printf("%-32s%.0f megs\n", "Total memory", (F32)memory * exp2(-20));

    for (int i = 0; i < (int)FW_ARRAY_SIZE(attribs); i++)
    {
        int value;
        if (cuDeviceGetAttribute(&value, attribs[i].attrib, device) == CUDA_SUCCESS)
            printf("%-32s%d\n", attribs[i].name, value);
    }
    printf("\n");
}

Vec2i CudaModule::selectGridSize(int numBlocks)
{
  CUresult res = CUDA_SUCCESS;
  int maxWidth;

  res = cuDeviceGetAttribute(&maxWidth, CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_X, s_device);
  checkError("cuDeviceGetAttribute", res);

  Vec2i size(numBlocks, 1);
  while (size.x > maxWidth)
  {
    size.x = (size.x + 1) >> 1;
    size.y <<= 1;
  }
  return size;
}

} // namespace FW
