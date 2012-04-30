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


#ifndef CUDARASTER_CUDARASTER_HPP_
#define CUDARASTER_CUDARASTER_HPP_

#include <string>
#include <gpu/Buffer.hpp>
#include <gpu/CudaModule.hpp>

#include "CudaSurface.hpp"
#include "cuda/PixelPipe.hpp"
#include "cuda/PrivateDefs.hpp"


namespace FW {
//------------------------------------------------------------------------
// CudaRaster host-side public interface.
//------------------------------------------------------------------------

class CudaRaster
{
  public:
    struct Stats // Statistics for the previous call to drawTriangles().
    {
      F32 setupTime;  // Seconds spent in TriangleSetup.
      F32 binTime;    // Seconds spent in BinRaster.
      F32 coarseTime; // Seconds spent in CoarseRaster.
      F32 fineTime;   // Seconds spent in FineRaster.
    };

    struct DebugParams // Host-side emulation of individual stages, for debugging purposes.
    {
      bool emulateTriangleSetup;
      bool emulateBinRaster;
      bool emulateCoarseRaster;
      bool emulateFineRaster;      // Only supports GouraudShader, BlendReplace, and BlendSrcOver.

      DebugParams(void)
      {
        emulateTriangleSetup = false;
        emulateBinRaster     = false;
        emulateCoarseRaster  = false;
        emulateFineRaster    = false;
      }
    };

  private:
    bool m_bInitialized;
    
    // State.
    CudaSurface* m_colorBuffer;
    CudaSurface* m_depthBuffer;

    bool    m_deferredClear;
    U32     m_clearColor;
    U32     m_clearDepth;

    Buffer* m_vertexBuffer;
    S64     m_vertexOfs;
    Buffer* m_indexBuffer;
    S64     m_indexOfs;
    S32     m_numTris;

    // Surfaces.
    Vec2i   m_viewportSize;
    Vec2i   m_sizePixels;
    Vec2i   m_sizeBins;
    S32     m_numBins;
    Vec2i   m_sizeTiles;
    S32     m_numTiles;
    S32     m_numSamples;
    S32     m_samplesLog2;


    // Pixel pipe.
    CudaModule*    m_module;
    CUfunction     m_setupKernel;
    CUfunction     m_binKernel;
    CUfunction     m_coarseKernel;
    CUfunction     m_fineKernel;
    PixelPipeSpec  m_pipeSpec;    
    
    S32 m_numSMs;
    S32 m_numFineWarps;


    // Buffers.
    S32     m_binBatchSize;

    S32     m_maxSubtris;
    Buffer  m_triSubtris;
    Buffer  m_triHeader;
    Buffer  m_triData;

    S32     m_maxBinSegs;
    Buffer  m_binFirstSeg;
    Buffer  m_binTotal;
    Buffer  m_binSegData;
    Buffer  m_binSegNext;
    Buffer  m_binSegCount;

    S32     m_maxTileSegs;
    Buffer  m_activeTiles;
    Buffer  m_tileFirstSeg;
    Buffer  m_tileSegData;
    Buffer  m_tileSegNext;
    Buffer  m_tileSegCount;


    // Stats, profiling, debug.
    CUevent m_evSetupBegin;
    CUevent m_evBinBegin;
    CUevent m_evCoarseBegin;
    CUevent m_evFineBegin;
    CUevent m_evFineEnd;
    Buffer  m_profData;
    
    DebugParams m_debug;
    

  public:
    CudaRaster(void);
    ~CudaRaster(void);
    
    void init();

    // Set before calling other methods.
    void setSurfaces(CudaSurface* color, CudaSurface* depth);
    
    // Clear surfaces on the next call to drawTriangles().
    void deferredClear(const Vec4f& color = 0.0f, F32 depth = 1.0f);

    // See CR_DEFINE_PIXEL_PIPE() in PixelPipe.hpp.
    void setPixelPipe(CudaModule* module, const std::string& name);
    void setVertexBuffer(Buffer* buf, S64 ofs);
    void setIndexBuffer(Buffer* buf, S64 ofs, int numTris);
    
    // Draw all triangles specified by the current index buffer.
    void drawTriangles(void);

    Stats getStats (void);
    
    // See CR_PROFILING_MODE in PixelPipe.hpp.
    std::string getProfilingInfo(void);
    
    void setDebugParams(const DebugParams& p);

  private:
    void launchStages(void);

    Vec3i setupPleq(const Vec3f& values, const Vec2i& v0, const Vec2i& d1, 
                    const Vec2i& d2, S32 area, int samplesLog2);

    bool setupTriangle( int triIdx, 
                        const Vec4f& v0, const Vec4f& v1, const Vec4f& v2, 
                        const Vec2f& b0, const Vec2f& b1, const Vec2f& b2,
                        const Vec3i& vidx);

    void emulateTriangleSetup(void);
    void emulateBinRaster(void);
    void emulateCoarseRaster(void);
    void emulateFineRaster(void);

  private:
    CudaRaster (const CudaRaster&);             // forbidden
    CudaRaster& operator= (const CudaRaster&);  // forbidden
  
};

} // namespace FW

#endif //CUDARASTER_CUDARASTER_HPP_
