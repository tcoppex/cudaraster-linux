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

#include <cstring>
#include <string>
#include <vector>

#include "base/Defs.hpp"
#include "base/Math.hpp"

namespace FW
{

//------------------------------------------------------------------------
// Helpers for equals() and hash().
//------------------------------------------------------------------------

#define FW_HASH_MAGIC   (0x9e3779b9u)

// By Bob Jenkins, 1996. bob_jenkins@burtleburtle.net.
#define FW_JENKINS_MIX(a, b, c)   \
    a -= b; a -= c; a ^= (c>>13); \
    b -= c; b -= a; b ^= (a<<8);  \
    c -= a; c -= b; c ^= (b>>13); \
    a -= b; a -= c; a ^= (c>>12); \
    b -= c; b -= a; b ^= (a<<16); \
    c -= a; c -= b; c ^= (b>>5);  \
    a -= b; a -= c; a ^= (c>>3);  \
    b -= c; b -= a; b ^= (a<<10); \
    c -= a; c -= b; c ^= (b>>15);

inline U32                          hashBits        (U32 a, U32 b = FW_HASH_MAGIC, U32 c = 0)                   { c += FW_HASH_MAGIC; FW_JENKINS_MIX(a, b, c); return c; }
inline U32                          hashBits        (U32 a, U32 b, U32 c, U32 d, U32 e = 0, U32 f = 0)          { c += FW_HASH_MAGIC; FW_JENKINS_MIX(a, b, c); a += d; b += e; c += f; FW_JENKINS_MIX(a, b, c); return c; }

inline bool                         equalsBuffer    (const void* ptrA, const void* ptrB, int size)              { return (memcmp(ptrA, ptrB, size) == 0); }
inline bool                         equalsBuffer    (const void* ptrA, int sizeA, const void* ptrB, int sizeB)  { return (sizeA == sizeB && memcmp(ptrA, ptrB, sizeA) == 0); }
U32                                 hashBuffer      (const void* ptr, int size);
U32                                 hashBufferAlign (const void* ptr, int size);

//------------------------------------------------------------------------
// Base templates.
//------------------------------------------------------------------------

template <class T>  inline bool equalsArray     (const T* ptrA, int sizeA, const T* ptrB, int sizeB);
template <class T>  inline U32  hashArray       (const T* ptr, int size);

template <class T>  inline bool equals          (const T& a, const T& b)                { return equalsBuffer(&a, &b, sizeof(T)); }
template <class T>  inline U32  hash            (const T& value)                        { return hashBuffer(&value, sizeof(T)); }

//------------------------------------------------------------------------
// Specializations for primitive types.
//------------------------------------------------------------------------

template <> inline bool equalsArray<S8> (const S8* ptrA, int sizeA, const S8* ptrB, int sizeB)      { return equalsBuffer(ptrA, sizeA * (int)sizeof(S8), ptrB, sizeB * (int)sizeof(S8)); }
template <> inline bool equalsArray<U8> (const U8* ptrA, int sizeA, const U8* ptrB, int sizeB)      { return equalsBuffer(ptrA, sizeA * (int)sizeof(U8), ptrB, sizeB * (int)sizeof(U8)); }
template <> inline bool equalsArray<S16>(const S16* ptrA, int sizeA, const S16* ptrB, int sizeB)    { return equalsBuffer(ptrA, sizeA * (int)sizeof(S16), ptrB, sizeB * (int)sizeof(S16)); }
template <> inline bool equalsArray<U16>(const U16* ptrA, int sizeA, const U16* ptrB, int sizeB)    { return equalsBuffer(ptrA, sizeA * (int)sizeof(U16), ptrB, sizeB * (int)sizeof(U16)); }
template <> inline bool equalsArray<S32>(const S32* ptrA, int sizeA, const S32* ptrB, int sizeB)    { return equalsBuffer(ptrA, sizeA * (int)sizeof(S32), ptrB, sizeB * (int)sizeof(S32)); }
template <> inline bool equalsArray<U32>(const U32* ptrA, int sizeA, const U32* ptrB, int sizeB)    { return equalsBuffer(ptrA, sizeA * (int)sizeof(U32), ptrB, sizeB * (int)sizeof(U32)); }
template <> inline bool equalsArray<F32>(const F32* ptrA, int sizeA, const F32* ptrB, int sizeB)    { return equalsBuffer(ptrA, sizeA * (int)sizeof(F32), ptrB, sizeB * (int)sizeof(F32)); }
template <> inline bool equalsArray<S64>(const S64* ptrA, int sizeA, const S64* ptrB, int sizeB)    { return equalsBuffer(ptrA, sizeA * (int)sizeof(S64), ptrB, sizeB * (int)sizeof(S64)); }
template <> inline bool equalsArray<U64>(const U64* ptrA, int sizeA, const U64* ptrB, int sizeB)    { return equalsBuffer(ptrA, sizeA * (int)sizeof(U64), ptrB, sizeB * (int)sizeof(U64)); }
template <> inline bool equalsArray<F64>(const F64* ptrA, int sizeA, const F64* ptrB, int sizeB)    { return equalsBuffer(ptrA, sizeA * (int)sizeof(F64), ptrB, sizeB * (int)sizeof(F64)); }

template <> inline U32  hashArray<S8>   (const S8* ptr, int size)           { return hashBuffer(ptr, size * (int)sizeof(S8)); }
template <> inline U32  hashArray<U8>   (const U8* ptr, int size)           { return hashBuffer(ptr, size * (int)sizeof(U8)); }
template <> inline U32  hashArray<S16>  (const S16* ptr, int size)          { return hashBuffer(ptr, size * (int)sizeof(S16)); }
template <> inline U32  hashArray<U16>  (const U16* ptr, int size)          { return hashBuffer(ptr, size * (int)sizeof(U16)); }
template <> inline U32  hashArray<S32>  (const S32* ptr, int size)          { return hashBuffer(ptr, size * (int)sizeof(S32)); }
template <> inline U32  hashArray<U32>  (const U32* ptr, int size)          { return hashBuffer(ptr, size * (int)sizeof(U32)); }
template <> inline U32  hashArray<F32>  (const F32* ptr, int size)          { return hashBuffer(ptr, size * (int)sizeof(F32)); }
template <> inline U32  hashArray<S64>  (const S64* ptr, int size)          { return hashBuffer(ptr, size * (int)sizeof(S64)); }
template <> inline U32  hashArray<U64>  (const U64* ptr, int size)          { return hashBuffer(ptr, size * (int)sizeof(U64)); }
template <> inline U32  hashArray<F64>  (const F64* ptr, int size)          { return hashBuffer(ptr, size * (int)sizeof(F64)); }

template <> inline bool equals<S8>      (const S8& a, const S8& b)          { return (a == b); }
template <> inline bool equals<U8>      (const U8& a, const U8& b)          { return (a == b); }
template <> inline bool equals<S16>     (const S16& a, const S16& b)        { return (a == b); }
template <> inline bool equals<U16>     (const U16& a, const U16& b)        { return (a == b); }
template <> inline bool equals<S32>     (const S32& a, const S32& b)        { return (a == b); }
template <> inline bool equals<U32>     (const U32& a, const U32& b)        { return (a == b); }
template <> inline bool equals<F32>     (const F32& a, const F32& b)        { return (floatToBits(a) == floatToBits(b)); }
template <> inline bool equals<S64>     (const S64& a, const S64& b)        { return (a == b); }
template <> inline bool equals<U64>     (const U64& a, const U64& b)        { return (a == b); }
template <> inline bool equals<F64>     (const F64& a, const F64& b)        { return (doubleToBits(a) == doubleToBits(b)); }

template <> inline U32  hash<S8>        (const S8& value)                   { return hashBits(value); }
template <> inline U32  hash<U8>        (const U8& value)                   { return hashBits(value); }
template <> inline U32  hash<S16>       (const S16& value)                  { return hashBits(value); }
template <> inline U32  hash<U16>       (const U16& value)                  { return hashBits(value); }
template <> inline U32  hash<S32>       (const S32& value)                  { return hashBits(value); }
template <> inline U32  hash<U32>       (const U32& value)                  { return hashBits(value); }
template <> inline U32  hash<F32>       (const F32& value)                  { return hashBits(floatToBits(value)); }
template <> inline U32  hash<S64>       (const S64& value)                  { return hashBits((U32)value, (U32)(value >> 32)); }
template <> inline U32  hash<U64>       (const U64& value)                  { return hash<S64>((S64)value); }
template <> inline U32  hash<F64>       (const F64& value)                  { return hash<U64>(doubleToBits(value)); }

//------------------------------------------------------------------------
// Specializations for compound types.
//------------------------------------------------------------------------

template <> inline bool equals<Vec2i>   (const Vec2i& a, const Vec2i& b)    { return (a == b); }
template <> inline bool equals<Vec2f>   (const Vec2f& a, const Vec2f& b)    { return (equals<F32>(a.x, b.x) && equals<F32>(a.y, b.y)); }
template <> inline bool equals<Vec3i>   (const Vec3i& a, const Vec3i& b)    { return (a == b); }
template <> inline bool equals<Vec3f>   (const Vec3f& a, const Vec3f& b)    { return (equals<F32>(a.x, b.x) && equals<F32>(a.y, b.y) && equals<F32>(a.z, b.z)); }
template <> inline bool equals<Vec4i>   (const Vec4i& a, const Vec4i& b)    { return (a == b); }
template <> inline bool equals<Vec4f>   (const Vec4f& a, const Vec4f& b)    { return (equals<F32>(a.x, b.x) && equals<F32>(a.y, b.y) && equals<F32>(a.z, b.z) && equals<F32>(a.w, b.w)); }
template <> inline bool equals<Mat2f>   (const Mat2f& a, const Mat2f& b)    { return equalsBuffer(&a, &b, sizeof(a)); }
template <> inline bool equals<Mat3f>   (const Mat3f& a, const Mat3f& b)    { return equalsBuffer(&a, &b, sizeof(a)); }
template <> inline bool equals<Mat4f>   (const Mat4f& a, const Mat4f& b)    { return equalsBuffer(&a, &b, sizeof(a)); }
template <> inline bool equals<std::string>  (const std::string& a, const std::string& b)  { return equalsBuffer(a.c_str(), a.length(), b.c_str(), b.length()); }

template <> inline U32  hash<Vec2i>     (const Vec2i& value)                { return hashBits(value.x, value.y); }
template <> inline U32  hash<Vec2f>     (const Vec2f& value)                { return hashBits(floatToBits(value.x), floatToBits(value.y)); }
template <> inline U32  hash<Vec3i>     (const Vec3i& value)                { return hashBits(value.x, value.y, value.z); }
template <> inline U32  hash<Vec3f>     (const Vec3f& value)                { return hashBits(floatToBits(value.x), floatToBits(value.y), floatToBits(value.z)); }
template <> inline U32  hash<Vec4i>     (const Vec4i& value)                { return hashBits(value.x, value.y, value.z, value.w); }
template <> inline U32  hash<Vec4f>     (const Vec4f& value)                { return hashBits(floatToBits(value.x), floatToBits(value.y), floatToBits(value.z), floatToBits(value.w)); }
template <> inline U32  hash<Mat2f>     (const Mat2f& value)                { return hashBufferAlign(&value, sizeof(value)); }
template <> inline U32  hash<Mat3f>     (const Mat3f& value)                { return hashBufferAlign(&value, sizeof(value)); }
template <> inline U32  hash<Mat4f>     (const Mat4f& value)                { return hashBufferAlign(&value, sizeof(value)); }
template <> inline U32  hash<std::string>    (const std::string& value)               { return hashBuffer(value.c_str(), value.length()); }

//------------------------------------------------------------------------
// Partial specializations.
//------------------------------------------------------------------------

template <class T, class TT> inline bool equals(TT* const& a, TT* const& b) { return (a == b); }
template <class T, class TT> inline U32 hash(TT* const& value) { return hashBits((U32)(UPTR)value); }

template <class T, class TT> inline bool equals(const std::vector<TT>& a, const std::vector<TT>& b) { return equalsArray<T>( &a[0], a.size(), &b[0], b.size()); }
template <class T, class TT> inline U32  hash(const std::vector<TT>& value) { return hashArray<T>(&value[0], value.size()); }

//------------------------------------------------------------------------

template <class T> bool equalsArray(const T* ptrA, int sizeA, const T* ptrB, int sizeB)
{
  if (sizeA != sizeB) {
    return false;
  }

  for (int i = 0; i < sizeA; i++)
  {
    if (!equals<T>(ptrA[i], ptrB[i])) {
      return false;
    }
  }
  return true;
}

//------------------------------------------------------------------------

template <class T> U32 hashArray(const T* ptr, int size)
{
    FW_ASSERT(size >= 0);
    FW_ASSERT(ptr || !size);

    U32 a = FW_HASH_MAGIC;
    U32 b = FW_HASH_MAGIC;
    U32 c = FW_HASH_MAGIC;

    while (size >= 3)
    {
        a += hash<T>(ptr[0]);
        b += hash<T>(ptr[1]);
        c += hash<T>(ptr[2]);
        FW_JENKINS_MIX(a, b, c);
        ptr += 3;
        size -= 3;
    }

    switch (size)
    {
    case 2: b += hash<T>(ptr[1]);
    case 1: a += hash<T>(ptr[0]);
    }

    c += size;
    FW_JENKINS_MIX(a, b, c);
    return c;
}

//------------------------------------------------------------------------
}
