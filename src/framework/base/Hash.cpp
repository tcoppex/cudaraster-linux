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
 
#include "base/Hash.hpp"

using namespace FW;

//------------------------------------------------------------------------

U32 FW::hashBuffer(const void* ptr, int size)
{
    FW_ASSERT(size >= 0);
    FW_ASSERT(ptr || !size);

    if ((((S32)(UPTR)ptr | size) & 3) == 0)
        return hashBufferAlign(ptr, size);

    const U8*   src     = (const U8*)ptr;
    U32         a       = FW_HASH_MAGIC;
    U32         b       = FW_HASH_MAGIC;
    U32         c       = FW_HASH_MAGIC;

    while (size >= 12)
    {
        a += src[0] + (src[1] << 8) + (src[2] << 16) + (src[3] << 24);
        b += src[4] + (src[5] << 8) + (src[6] << 16) + (src[7] << 24);
        c += src[8] + (src[9] << 8) + (src[10] << 16) + (src[11] << 24);
        FW_JENKINS_MIX(a, b, c);
        src += 12;
        size -= 12;
    }

    switch (size)
    {
    case 11: c += src[10] << 16;
    case 10: c += src[9] << 8;
    case 9:  c += src[8];
    case 8:  b += src[7] << 24;
    case 7:  b += src[6] << 16;
    case 6:  b += src[5] << 8;
    case 5:  b += src[4];
    case 4:  a += src[3] << 24;
    case 3:  a += src[2] << 16;
    case 2:  a += src[1] << 8;
    case 1:  a += src[0];
    case 0:  break;
    }

    c += size;
    FW_JENKINS_MIX(a, b, c);
    return c;
}

//------------------------------------------------------------------------

U32 FW::hashBufferAlign(const void* ptr, int size)
{
    FW_ASSERT(size >= 0);
    FW_ASSERT(ptr || !size);
    FW_ASSERT(((UPTR)ptr & 3) == 0);
    FW_ASSERT((size & 3) == 0);

    const U32*  src     = (const U32*)ptr;
    U32         a       = FW_HASH_MAGIC;
    U32         b       = FW_HASH_MAGIC;
    U32         c       = FW_HASH_MAGIC;

    while (size >= 12)
    {
        a += src[0];
        b += src[1];
        c += src[2];
        FW_JENKINS_MIX(a, b, c);
        src += 3;
        size -= 12;
    }

    switch (size)
    {
    case 8: b += src[1];
    case 4: a += src[0];
    case 0: break;
    }

    c += size;
    FW_JENKINS_MIX(a, b, c);
    return c;
}

//------------------------------------------------------------------------
