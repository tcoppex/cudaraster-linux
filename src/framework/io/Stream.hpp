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
 
#ifndef FRAMEWORK_IO_STREAM_HPP_
#define FRAMEWORK_IO_STREAM_HPP_

#include <cstdarg>
#include <string>
#include <vector>

#include "base/Math.hpp"


namespace FW {

//------------------------------------------------------------------------

class InputStream
{
public:
                            InputStream             (void)          {}
    virtual                 ~InputStream            (void)          {}

    virtual int             read                    (void* ptr, int size) = 0; // out of data => partial result
    void                    readFully               (void* ptr, int size);     // out of data => failure

    U8                      readU8                  (void)          { U8 b;    readFully(&b, sizeof(b)); return b; }
    U16                     readU16BE               (void)          { U8 b[2]; readFully(b, sizeof(b)); return (U16)((b[0] << 8) | b[1]); }
    U16                     readU16LE               (void)          { U8 b[2]; readFully(b, sizeof(b)); return (U16)((b[1] << 8) | b[0]); }
    U32                     readU32BE               (void)          { U8 b[4]; readFully(b, sizeof(b)); return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3]; }
    U32                     readU32LE               (void)          { U8 b[4]; readFully(b, sizeof(b)); return (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0]; }
    U64                     readU64BE               (void)          { U8 b[8]; readFully(b, sizeof(b)); return ((U64)b[0] << 56) | ((U64)b[1] << 48) | ((U64)b[2] << 40) | ((U64)b[3] << 32) | (b[4] << 24) | (b[5] << 16) | (b[6] << 8) | b[7]; }
    U64                     readU64LE               (void)          { U8 b[8]; readFully(b, sizeof(b)); return ((U64)b[7] << 56) | ((U64)b[6] << 48) | ((U64)b[5] << 40) | ((U64)b[4] << 32) | (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0]; }
};

//------------------------------------------------------------------------

class OutputStream
{
public:
                            OutputStream            (void)          {}
    virtual                 ~OutputStream           (void)          {}

    virtual void            write                   (const void* ptr, int size) = 0;
    virtual void            flush                   (void) = 0;

    void                    writeU8                 (U32 v)         { U8 b[1]; b[0] = (U8)v; write(b, sizeof(b)); }
    void                    writeU16BE              (U32 v)         { U8 b[2]; b[0] = (U8)(v >> 8); b[1] = (U8)v; write(b, sizeof(b)); }
    void                    writeU16LE              (U32 v)         { U8 b[2]; b[1] = (U8)(v >> 8); b[0] = (U8)v; write(b, sizeof(b)); }
    void                    writeU32BE              (U32 v)         { U8 b[4]; b[0] = (U8)(v >> 24); b[1] = (U8)(v >> 16); b[2] = (U8)(v >> 8); b[3] = (U8)v; write(b, sizeof(b)); }
    void                    writeU32LE              (U32 v)         { U8 b[4]; b[3] = (U8)(v >> 24); b[2] = (U8)(v >> 16); b[1] = (U8)(v >> 8); b[0] = (U8)v; write(b, sizeof(b)); }
    void                    writeU64BE              (U64 v)         { U8 b[8]; b[0] = (U8)(v >> 56); b[1] = (U8)(v >> 48); b[2] = (U8)(v >> 40); b[3] = (U8)(v >> 32); b[4] = (U8)(v >> 24); b[5] = (U8)(v >> 16); b[6] = (U8)(v >> 8); b[7] = (U8)v; write(b, sizeof(b)); }
    void                    writeU64LE              (U64 v)         { U8 b[8]; b[7] = (U8)(v >> 56); b[6] = (U8)(v >> 48); b[5] = (U8)(v >> 40); b[4] = (U8)(v >> 32); b[3] = (U8)(v >> 24); b[2] = (U8)(v >> 16); b[1] = (U8)(v >> 8); b[0] = (U8)v; write(b, sizeof(b)); }
};

//------------------------------------------------------------------------

class BufferedInputStream : public InputStream
{
  private:
    InputStream&            m_stream;
    std::vector<U8>         m_buffer;
    S32                     m_numRead;
    S32                     m_numConsumed;
    
  public:
                            BufferedInputStream     (InputStream& stream, int bufferSize = 4096);
    virtual                 ~BufferedInputStream    (void);

    virtual int             read                    (void* ptr, int size);
    char*                   readLine                (bool combineWithBackslash = false, bool normalizeWhitespace = false);

    bool                    fillBuffer              (int size);
    int                     getBufferSize           (void)          { return m_numRead - m_numConsumed; }
    U8*                     getBufferPtr            (void)          { return &m_buffer[m_numConsumed]; }
    void                    consumeBuffer           (int num);

  private:
                            BufferedInputStream     (const BufferedInputStream&); // forbidden
    BufferedInputStream&    operator=               (const BufferedInputStream&); // forbidden

};

//------------------------------------------------------------------------

class BufferedOutputStream : public OutputStream
{
  private:
    OutputStream&           m_stream;
    bool                    m_writeOnLF;
    bool                    m_emulateCR;

    std::vector<U8>         m_buffer;
    S32                     m_numValid;
    S32                     m_lineStart;
    S32                     m_currOfs;
    S32                     m_numFlushed;
    
public:
                            BufferedOutputStream    (OutputStream& stream, int bufferSize = 4096, bool writeOnLF = false, bool emulateCR = false);
    virtual                 ~BufferedOutputStream   (void);

    virtual void            write                   (const void* ptr, int size);
    void                    writef                  (const char* fmt, ...);
    void                    writefv                 (const char* fmt, va_list args);
    virtual void            flush                   (void);

    S32                     getNumBytesWritten      (void) const    { return m_numFlushed + m_numValid; }

private:
    void                    addValid                (int size);
    void                    flushInternal           (void);

private:
                            BufferedOutputStream    (const BufferedOutputStream&); // forbidden
    BufferedOutputStream&   operator=               (const BufferedOutputStream&); // forbidden
};

//------------------------------------------------------------------------

class MemoryInputStream : public InputStream
{
  private:
    const U8* m_ptr;
    S32       m_size;
    S32       m_ofs;
    
  public:
                            MemoryInputStream       (void)                      { reset(); }
                            MemoryInputStream       (const void* ptr, int size) { reset(ptr, size); }
    template <class T> explicit MemoryInputStream   (const std::vector<T>& data) { reset(data); }
    virtual                 ~MemoryInputStream      (void);

    virtual int             read                    (void* ptr, int size);

    int                     getOffset               (void) const                { return m_ofs; }
    void                    seek                    (int ofs)                   { FW_ASSERT(ofs >= 0 && ofs <= m_size); m_ofs = ofs; }

    void                    reset                   (void)                      { m_ptr = NULL; m_size = 0; m_ofs = 0; }
    void                    reset                   (const void* ptr, int size) { FW_ASSERT(size >= 0); FW_ASSERT(ptr || !size); m_ptr = (const U8*)ptr; m_size = size; m_ofs = 0; }
    template <class T> void reset                   (const std::vector<T>& data)      { reset(data.getPtr(), data.getNumBytes()); }

private:
                            MemoryInputStream       (const MemoryInputStream&); // forbidden
    MemoryInputStream&      operator=               (const MemoryInputStream&); // forbidden

};

//------------------------------------------------------------------------

class MemoryOutputStream : public OutputStream
{
  private:
      std::vector<U8>       m_data;
  
  public:
                            MemoryOutputStream      (int capacity = 0) { m_data.reserve(capacity); }
    virtual                 ~MemoryOutputStream     (void);

    virtual void            write                   (const void* ptr, int size);
    virtual void            flush                   (void);

    void                    clear                   (void)          { m_data.clear(); }
    std::vector<U8>&        getData                 (void)          { return m_data; }
    const std::vector<U8>&  getData                 (void) const    { return m_data; }

  private:
                            MemoryOutputStream      (const MemoryOutputStream&); // forbidden
    MemoryOutputStream&     operator=               (const MemoryOutputStream&); // forbidden
};

//------------------------------------------------------------------------

class Serializable
{
  public:
                  Serializable            (void)          {}
    virtual       ~Serializable           (void)          {}

    virtual void  readFromStream          (InputStream& s) = 0;
    virtual void  writeToStream           (OutputStream& s) const = 0;
};

//------------------------------------------------------------------------
// Primitive types.
//------------------------------------------------------------------------

inline InputStream&     operator>>  (InputStream& s, U8& v)         { v = s.readU8(); return s; }
inline InputStream&     operator>>  (InputStream& s, U16& v)        { v = s.readU16LE(); return s; }
inline InputStream&     operator>>  (InputStream& s, U32& v)        { v = s.readU32LE(); return s; }
inline InputStream&     operator>>  (InputStream& s, U64& v)        { v = s.readU64LE(); return s; }
inline InputStream&     operator>>  (InputStream& s, S8& v)         { v = s.readU8(); return s; }
inline InputStream&     operator>>  (InputStream& s, S16& v)        { v = s.readU16LE(); return s; }
inline InputStream&     operator>>  (InputStream& s, S32& v)        { v = s.readU32LE(); return s; }
inline InputStream&     operator>>  (InputStream& s, S64& v)        { v = s.readU64LE(); return s; }
inline InputStream&     operator>>  (InputStream& s, F32& v)        { v = bitsToFloat(s.readU32LE()); return s; }
inline InputStream&     operator>>  (InputStream& s, F64& v)        { v = bitsToDouble(s.readU64LE()); return s; }
inline InputStream&     operator>>  (InputStream& s, char& v)       { v = s.readU8(); return s; }
inline InputStream&     operator>>  (InputStream& s, bool& v)       { v = (s.readU8() != 0); return s; }

inline OutputStream&    operator<<  (OutputStream& s, U8 v)         { s.writeU8(v); return s; }
inline OutputStream&    operator<<  (OutputStream& s, U16 v)        { s.writeU16LE(v); return s; }
inline OutputStream&    operator<<  (OutputStream& s, U32 v)        { s.writeU32LE(v); return s; }
//inline OutputStream&    operator<<  (OutputStream& s, size_t v)     { s.writeU32LE(v); return s; } // added
inline OutputStream&    operator<<  (OutputStream& s, U64 v)        { s.writeU64LE(v); return s; }
inline OutputStream&    operator<<  (OutputStream& s, S8 v)         { s.writeU8(v); return s; }
inline OutputStream&    operator<<  (OutputStream& s, S16 v)        { s.writeU16LE(v); return s; }
inline OutputStream&    operator<<  (OutputStream& s, S32 v)        { s.writeU32LE(v); return s; }
inline OutputStream&    operator<<  (OutputStream& s, S64 v)        { s.writeU64LE(v); return s; }
inline OutputStream&    operator<<  (OutputStream& s, F32 v)        { s.writeU32LE(floatToBits(v)); return s; }
inline OutputStream&    operator<<  (OutputStream& s, F64 v)        { s.writeU64LE(doubleToBits(v)); return s; }
inline OutputStream&    operator<<  (OutputStream& s, char v)       { s.writeU8(v); return s; }
inline OutputStream&    operator<<  (OutputStream& s, bool v)       { s.writeU8((v) ? 1 : 0); return s; }

//------------------------------------------------------------------------
// Types defined or included by this header.
//------------------------------------------------------------------------

inline InputStream& operator>>(InputStream& s, Serializable& v)
{
  v.readFromStream(s);
  return s;
}

//------------------------------------------------------------------------

inline OutputStream& operator<<(OutputStream& s, const Serializable& v)
{
  v.writeToStream(s);
  return s;
}

//------------------------------------------------------------------------

template <class T> InputStream& operator>>(InputStream& s, std::vector<T>& v)
{
  S32 len;
  s >> len;
  v.reset(len);
  for (size_t i = 0u; i < len; ++i) {
    s >> v[i];
  }
  return s;
}

//------------------------------------------------------------------------

template <class T> OutputStream& operator<<(OutputStream& s, const std::vector<T>& v)
{
  s << v.getSize();
  for (size_t i = 0u; i < v.getSize(); ++i) {
    s << v[i];
  }
  return s;
}

//------------------------------------------------------------------------

inline InputStream& operator>>(InputStream& s, std::string& v)
{
  S32 len;
  s >> len;
  
  std::vector<char> t(len + 1u);    
  for (size_t i = 0u; i < len; ++i) {
    s >> t[i];
  }
  t[len] = '\0';
  
  v = std::string(&t[0]);
  
  return s;
}

//------------------------------------------------------------------------

inline OutputStream& operator<<(OutputStream& s, const std::string& v)
{
  s << (U32)(v.length());
  for (size_t i = 0u; i < v.length(); ++i) {
    s << v[i];
  }
  return s;
}

//------------------------------------------------------------------------

template <class T, int L, class S> InputStream& operator>>(InputStream& s, VectorBase<T, L, S>& v)
{
  for (size_t i = 0u; i < L; ++i) {
    s >> v[i];
  }
  return s;
}

//------------------------------------------------------------------------

template <class T, int L, class S> OutputStream& operator<<(OutputStream& s, const VectorBase<T, L, S>& v)
{
  for (size_t i = 0u; i < L; ++i) {
    s << v[i];
  }
  return s;
}

//------------------------------------------------------------------------

template <class T, int L, class S> InputStream& operator>>(InputStream& s, MatrixBase<T, L, S>& v)
{
  for (size_t i = 0u; i < L * L; ++i) {
    s >> v.getPtr()[i];
  }
  return s;
}

//------------------------------------------------------------------------

template <class T, int L, class S> OutputStream& operator<<(OutputStream& s, const MatrixBase<T, L, S>& v)
{
  for (size_t i = 0u; i < L * L; ++i) {
    s << v.getPtr()[i];
  }      
  return s;
}

} //namespace FW

#endif //FRAMEWORK_IO_STREAM_HPP_
