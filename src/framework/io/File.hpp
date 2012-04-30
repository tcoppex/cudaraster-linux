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
 
#ifndef FRAMEWORK_IO_FILE_HPP
#define FRAMEWORK_IO_FILE_HPP

#include <cstdio>
#include <io/Stream.hpp>

namespace FW {

// Wrapper to read and write file.
// This class is a simple rewrite of the CudaRaster File class.
class File : public InputStream, public OutputStream
{
  public:
    enum Mode
    {
      Read,   // must exist - cannot be written
      Create, // created or truncated - can be read or written
      Modify  // opened or created - can be read or written
    };
    
  private:
    std::string m_name;
    Mode m_mode;
    
    FILE* m_fd;

    S64 m_size;
    S64 m_offset;

  public:
    File(const std::string& name, Mode mode)
        : m_name(name),
          m_mode(mode),
          m_fd(NULL),
          m_size(0),
          m_offset(0)
    {    
      m_fd = fopen( m_name.c_str(), (mode==Read)?"r":(mode==Create)?"w":"w+");
      assert( NULL != m_fd );
      
      fseek( m_fd, 0, SEEK_END);
      m_size = ftell(m_fd);
      fseek( m_fd, 0, SEEK_SET);
    }
    
    virtual ~File(void) 
    { 
      if (m_fd) {
        fclose(m_fd); 
        m_fd = NULL; 
      }
    }

    const std::string& getName(void) const { return m_name; }
    Mode getMode(void) const { return m_mode; }
    bool checkWritable(void) const { return m_mode != Read; }

    S64 getSize(void) const { return m_size; }
    
    S64 getOffset(void) const { return m_offset; }
    
    void seek(S64 ofs)
    {
      if (fseek( m_fd, ofs, SEEK_SET) == 0) {
        m_offset = ofs;
      }
    }

    virtual int read(void* ptr, int size)
    {
      m_offset += fread( ptr, 1, size, m_fd);
    }
    
    virtual void write(const void* ptr, int size) 
    {
      m_offset += fwrite( ptr, 1, size, m_fd);
      m_size = max( m_size, m_offset);
    }
    
    virtual void flush(void) {}
    
  private:
    File(const File&);              // forbidden
    File& operator= (const File&);  // forbidden
};

} // namespace FW

#endif // FRAMEWORK_IO_FILE_HPP
