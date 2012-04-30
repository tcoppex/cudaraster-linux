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

 
#ifndef FRAMEWORK_GPU_CUDACOMPILER_HPP_
#define FRAMEWORK_GPU_CUDACOMPILER_HPP_

#include <cassert>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

#include "base/Defs.hpp"


namespace FW {

class CudaModule;

class CudaCompiler
{
  private:
    typedef std::vector<U8> uint8Array_t;
    typedef std::map<U64, uint8Array_t*> CubinCacheMap_t;
        
    typedef std::map<U64, CudaModule*> ModuleCacheMap_t;
    
    typedef std::map<std::string, std::string> DefinesMap_t;
    
    
  private:
    static bool s_inited;
    
    static std::string s_staticCudaBinPath;
    static std::string s_staticOptions;
    static std::string s_staticPreamble;
    static std::string s_staticBinaryFormat;

    static U32 s_nvccVersionHash;
    static std::string s_nvccCommand;
    static CubinCacheMap_t s_cubinCache;
    static ModuleCacheMap_t s_moduleCache;
    
    
    std::string m_cachePath;
    std::string m_sourceFile;
    S32 m_overriddenSMArch;

    std::string m_options;
    DefinesMap_t m_defines;
    std::string m_preamble;

    U32 m_sourceFileHash;
    U32 m_optionHash;
    U64 m_defineHash;
    U32 m_preambleHash;
    U64 m_memHash;
    bool m_sourceFileHashValid;
    bool m_optionHashValid;
    bool m_defineHashValid;
    bool m_preambleHashValid;
    bool m_memHashValid;
    
    
  public:
    CudaCompiler(void);
    ~CudaCompiler(void);
    
    //++++++++++++
    void setCachePath(const std::string& path) { m_cachePath = path; }
    
    void setSourceFile(const std::string& path) 
    { 
      m_sourceFile = path; 
      m_sourceFileHashValid = false; 
      m_memHashValid = false;
    }
    
    void overrideSMArch(int arch) { m_overriddenSMArch = arch; }
    

    //++++++++++++
    void clearOptions(void)
    { 
      m_options = ""; 
      m_optionHashValid = false; 
      m_memHashValid = false; 
    }
    
    void addOptions(const std::string& options)
    { 
      m_options += options + " "; 
      m_optionHashValid = false; 
      m_memHashValid = false; 
    }
    
    void include(const std::string& path) 
    {
      addOptions( "-I\"" + path + "\"" ); 
    }
    

    //++++++++++++
    void clearDefines(void)
    { 
      m_defines.clear(); 
      m_defineHashValid = false; 
      m_memHashValid = false; 
    }
    
    void undef(const std::string& key) 
    { 
      m_defines.erase(key); 
      m_defineHashValid = false; 
      m_memHashValid = false; 
    }
    
    void define(const std::string& key, const std::string& value = "") 
    { 
      undef(key); 
      m_defines[key] = value; 
      m_defineHashValid = false; 
      m_memHashValid = false; 
    }
    
    void define(const std::string& key, int value)
    { 
      char n[16];
      sprintf( n, "%d", value);
      define(key, n); 
    }

    //++++++++++++
    void clearPreamble(void) 
    { 
      m_preamble = ""; 
      m_preambleHashValid = false; 
      m_memHashValid = false; 
    }
    
    void addPreamble(const std::string& preamble) 
    { 
      m_preamble += preamble + "\n"; 
      m_preambleHashValid = false; 
      m_memHashValid = false; 
    }
    
    //++++++++++++
    CudaModule* compile(bool enablePrints = true);    
    
    // returns data in cubin file, padded with a zero    
    const std::vector<U8>* compileCubin(bool enablePrints = true);
    
    // returns file name, empty std::string on error
    std::string compileCubinFile(bool enablePrints = true);
    

    //++++++++++++
    static void setStaticCudaBinPath(const std::string& path)     
    { 
      assert(!s_inited); 
      s_staticCudaBinPath = path; 
    }
    
    static void setStaticOptions(const std::string& options)      
    { 
      assert(!s_inited); 
      s_staticOptions = options; 
    }
    
    static void setStaticPreamble(const std::string& preamble)    
    { 
      assert(!s_inited); 
      s_staticPreamble = preamble; // e.g. "#include \"myheader.h\""
    }
    
    static void setStaticBinaryFormat(const std::string& format)  
    { 
      assert(!s_inited); 
      s_staticBinaryFormat = format; // e.g. "-ptx"
    }

    //++++++++++++
    static void staticInit(void);
    static void staticDeinit(void);
    static void flushMemCache(void);

  private:
    CudaCompiler(const CudaCompiler&);              // forbidden
    CudaCompiler& operator= (const CudaCompiler&);  // forbidden

    //++++++++++++
    static std::string queryEnv(const std::string& name);
    static void splitPathList( std::vector<std::string>& res, const std::string& value);
    static bool fileExists(const std::string& name);
    static std::string removeOption(const std::string& opts, const std::string& tag, bool hasParam);

    //++++++++++++
    U64  getMemHash(void);
    void createCacheDir(void);
    void writeDefineFile(void);
    void initLogFile(const std::string& name, const std::string& firstLine);

    //++++++++++++
    bool runPreprocessor(std::string& cubinFile, std::string& finalOpts);
    bool runCompiler(const std::string& cubinFile, const std::string& finalOpts);

    //++++++++++++
    void setLoggedError(const std::string& description, const std::string& logFile);
};

} // namespace FW


#endif // FRAMEWORK_GPU_CUDACOMPILER_HPP_
