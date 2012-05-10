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
 
#include "gpu/CudaCompiler.hpp"

#include <cstdio>
#include "base/Hash.hpp"
#include "gpu/CudaModule.hpp"
#include "io/File.hpp"


namespace FW {

#define SHOW_NVCC_OUTPUT    0

//------------------------------------------------------------------------

bool CudaCompiler::s_inited = false;

std::string CudaCompiler::s_staticCudaBinPath;// = "/usr/local/cuda/bin";//
std::string CudaCompiler::s_staticOptions;
std::string CudaCompiler::s_staticPreamble;
std::string CudaCompiler::s_staticBinaryFormat;

U32         CudaCompiler::s_nvccVersionHash = 0;
std::string CudaCompiler::s_nvccCommand;

CudaCompiler::CubinCacheMap_t   CudaCompiler::s_cubinCache;
CudaCompiler::ModuleCacheMap_t  CudaCompiler::s_moduleCache;

//------------------------------------------------------------------------

CudaCompiler::CudaCompiler(void)
    : m_cachePath             ("cudacache"),
      m_sourceFile            ("unspecified.cu"),
      m_overriddenSMArch      (0),

      m_sourceFileHash        (0),
      m_optionHash            (0),
      m_defineHash            (0),
      m_preambleHash          (0),
      m_memHash               (0),
      m_sourceFileHashValid   (false),
      m_optionHashValid       (false),
      m_defineHashValid       (false),
      m_preambleHashValid     (false),
      m_memHashValid          (false)
{
  printf( "\t CudaCompiler::%s\n", __FUNCTION__);
}


CudaCompiler::~CudaCompiler(void)
{
  printf( "\t CudaCompiler::%s\n", __FUNCTION__);
}

//------------------------------------------------------------------------

CudaModule* CudaCompiler::compile(bool enablePrints)
{
  printf( "\t CudaCompiler::%s\n", __FUNCTION__);
  staticInit();

  // Cached in memory => done.
  U64 memHash = getMemHash();
  CudaModule* pModule = s_moduleCache[memHash];
  
  if (pModule) {
    return pModule;
  }

  /// Compile CUBIN file.
  std::string cubinFile = compileCubinFile(enablePrints);
  
  if (!cubinFile.length())
  {
    fprintf( stderr, "%s Error : cubinfile null.\n", __FUNCTION__);
    return NULL;
  }


  // Create module and add to memory cache.
  CudaModule* module = new CudaModule(cubinFile);
  s_moduleCache[memHash] = module;
  return module;
}

//------------------------------------------------------------------------

const std::vector<U8>* CudaCompiler::compileCubin(bool enablePrints)
{
  printf( "\t CudaCompiler::%s\n", __FUNCTION__);
  staticInit();

  // Cached in memory => done.
  U64 memHash = getMemHash();
  std::vector<U8>* pCubin = s_cubinCache[memHash];
  if (pCubin) {
    return pCubin;
  }
  
  // Compile CUBIN file.
  std::string cubinFile = compileCubinFile(enablePrints);
  if (std::string::npos == cubinFile.length()) {
    fprintf( stderr, "%s Error : cubinfile null.\n", __FUNCTION__);
    return NULL;
  }

  // Load CUBIN.  
  File in( cubinFile, File::Read);
  S32 size = (S32)in.getSize();
  
  std::vector<U8>* cubin = new std::vector<U8>(size + 1);  
  in.read( &(*cubin)[0], size);
  (*cubin)[size] = '\0';
  
  // Add to memory cache.
  s_cubinCache[memHash] = cubin;
  
  return cubin;
}

//------------------------------------------------------------------------

std::string CudaCompiler::compileCubinFile(bool enablePrints)
{
  printf( "\t CudaCompiler::%s\n", __FUNCTION__);
  bool bSucceed = true;
  
  staticInit();
    
  /// Check that the source file exists.
  if (!fileExists(m_sourceFile)) {
    fprintf( stderr, "%s : source file does not exist.\n", __FUNCTION__);
    return "";
  }
  
  /// Cache directory does not exist => create it.
  createCacheDir();

  /// Preprocess.
  writeDefineFile();
  std::string cubinFile, finalOpts;
  bSucceed = runPreprocessor(cubinFile, finalOpts);
  
  if (!bSucceed) { 
    fprintf( stderr, "%s : preprocessor failed.\n", __FUNCTION__);
    return ""; 
  }

  /// CUBIN exists => done.
  if (fileExists(cubinFile)) 
  {
#ifndef NDEBUG
    //fprintf( stderr, "CudaCompiler: '%s' already compiled.\n", m_sourceFile.c_str());
#endif
    return cubinFile;
  }
  
  /// Compile.
  if (enablePrints) {
    printf("CudaCompiler: Compiling '%s'...", m_sourceFile.c_str());
  }

  bSucceed = runCompiler( cubinFile, finalOpts);

  if (enablePrints) {
    printf((!bSucceed) ? " Failed.\n" : " Done.\n");
  }
  
  return (bSucceed) ? cubinFile : "";
}

//------------------------------------------------------------------------

void CudaCompiler::staticInit(void)
{
  if (s_inited) {
    return;
  }
  s_inited = true; 
  printf( "\t CudaCompiler::%s\n", __FUNCTION__); 
  
  // Search for CUDA on Linux system

  std::vector<std::string> potentialCudaPaths;  
  potentialCudaPaths.push_back( "/usr/local/cuda" );//
  
  // Query environment variables.
  std::string pathEnv    = queryEnv("PATH");
  std::string includeEnv = queryEnv("INCLUDE");
  std::string cudaBinEnv = queryEnv("CUDA_BIN_PATH");
  std::string cudaIncEnv = queryEnv("CUDA_INC_PATH");
  
  // Find CUDA binary path.
  std::vector<std::string> cudaBinList;
  
  if (s_staticCudaBinPath.length())
  {
    cudaBinList.push_back(s_staticCudaBinPath);
  }
  else
  {
    cudaBinList.push_back(cudaBinEnv);
    splitPathList(cudaBinList, pathEnv);
    for (size_t i = 0u; i < potentialCudaPaths.size(); ++i)
    {
      cudaBinList.push_back(potentialCudaPaths[i] + "/bin");
      cudaBinList.push_back(potentialCudaPaths[i] + "/bin64");
    }
  }
  
  std::string cudaBinPath;
  for (size_t i = 0u; i < cudaBinList.size(); ++i)
  {
    if (!cudaBinList[i].length() || !fileExists(cudaBinList[i] + "/nvcc")) {
      continue;
    }
        
    // Execute "nvcc --version".
    std::string cmd = "\"" + cudaBinList[i] + "/nvcc\" --version 2>/dev/null";    
    FILE* pipe = popen( cmd.c_str(), "r");
    if (!pipe) {
      continue;
    }

    std::vector<char> output;
    while (!feof(pipe)) {
      output.push_back((char)fgetc(pipe));
    }
    pclose(pipe);
    output.push_back('\0');

    // Test wether nvcc --version output is standard or not (kind of a hack)
    // Invalid response => ignore. 
    std::string response(&output[0]);
    if (response.find_first_of("nvcc: NVIDIA") != 0u) {
      continue;
    }

    // A (supposed) valid nvcc compiler has been found
    cudaBinPath = cudaBinList[i];
    s_nvccVersionHash = hash<std::string>(response); // 
    break;
  }

  if (!cudaBinPath.size()) {
    fail( "Unable to detect CUDA Toolkit binary path!\nPlease set CUDA_BIN_PATH"\
          " environment variable." );
  }

  // Find CUDA include path.
  std::vector<std::string> cudaIncList;
  cudaIncList.push_back(cudaBinPath + "/../include");
  cudaIncList.push_back(cudaIncEnv);
  splitPathList(cudaIncList, includeEnv);

  
  std::string cudaIncPath;
  for (size_t i=0u; i<cudaIncList.size(); ++i)
  {
    if (cudaIncList[i].length() && fileExists(cudaIncList[i] + "/cuda.h"))
    {
      cudaIncPath = cudaIncList[i];
      break;
    }
  }
  
  if (!cudaIncPath.length()) {
    fail("Unable to detect CUDA Toolkit include path!\n"
         "Please set CUDA_INC_PATH environment variable.");
  }
  
  system( ("export PATH=$PATH:" + cudaBinPath).c_str() );
  
  s_nvccCommand = "nvcc -I\"" + cudaIncPath + "\" -I. -D_CRT_SECURE_NO_DEPRECATE";
}

//------------------------------------------------------------------------

void CudaCompiler::staticDeinit(void)
{
  s_staticCudaBinPath = "";
  s_staticOptions = "";
  s_staticPreamble = "";
  s_staticBinaryFormat = "";

  if (!s_inited) {
    return;
  }
  s_inited = false;


  printf( "\t CudaCompiler::%s\n", __FUNCTION__);

  flushMemCache();
  s_cubinCache.clear();
  s_moduleCache.clear();
  s_nvccCommand = "";
}

//------------------------------------------------------------------------

void CudaCompiler::flushMemCache(void)
{
  printf( "\t CudaCompiler::%s\n", __FUNCTION__);
  for (CubinCacheMap_t::iterator it=s_cubinCache.begin(); it!=s_cubinCache.end(); ++it) {
    delete it->second;
  }
  s_cubinCache.clear();

  for (ModuleCacheMap_t::iterator it=s_moduleCache.begin(); it!=s_moduleCache.end(); ++it) {
    delete it->second;
  }
  s_moduleCache.clear();
}

//------------------------------------------------------------------------

std::string CudaCompiler::queryEnv(const std::string& name)
{
  printf( "\t CudaCompiler::%s\n", __FUNCTION__);
  
  // Could be a better idea to use getenv() directly..  
  char *env = getenv(name.c_str());
  return (NULL==env)?std::string(""):std::string(env);
}

//------------------------------------------------------------------------

void CudaCompiler::splitPathList( std::vector<std::string>& res, 
                                  const std::string& value)
{
  printf( "\t CudaCompiler::%s\n", __FUNCTION__);
  
  for (size_t startIdx = 0u; startIdx < value.length();)
  {
    size_t endIdx = value.find_first_of(':', startIdx);
    
    if (std::string::npos == endIdx) {
      endIdx = value.length();
    }

    std::string item = value.substr( startIdx, endIdx-startIdx);
    
    if ((item.length() >= 2u) && 
        (item.find_first_of("\"") == 0u) && 
        (item.find_last_of("\"") == (item.length()-1u))) 
    {
      item = item.substr( 1u, item.length() - 2u);
    }
    res.push_back(item);

    startIdx = endIdx + 1u;
  }
}

//------------------------------------------------------------------------

bool CudaCompiler::fileExists(const std::string& name)
{
  printf( "\t CudaCompiler::%s\n", __FUNCTION__);
  
  FILE *fd = fopen( name.c_str(), "r");
  
  if (NULL != fd) {
    fclose(fd);
    return true;
  }
  
  return false;
}

//------------------------------------------------------------------------

std::string CudaCompiler::removeOption(const std::string& opts, 
                                       const std::string& tag, bool hasParam)
{
  printf( "\t CudaCompiler::%s\n", __FUNCTION__);
  std::string res = opts;
    
  for (size_t i=0u; i<res.length(); ++i)
  {
    bool match = true;
    
    for (size_t j=0u; match && (j < tag.length()); ++j) {
      match = (i + j < res.length()) && (res[i + j] == tag[j]);
    }
    
    if (!match) {
      continue;
    }

    size_t idx = res.find_first_of(' ', i);
    if (hasParam && (idx != std::string::npos)) {
      idx = res.find_first_of(' ', idx + 1u);
    }

    res = res.substr( 0u, i) + ((idx == std::string::npos) ? "" : res.substr(idx + 1u));
    if (i>0u) --i;
  }
  
  return res;
}

//------------------------------------------------------------------------

U64 CudaCompiler::getMemHash(void)
{  
  printf( "\t CudaCompiler::%s\n", __FUNCTION__);
  
  if (m_memHashValid) {
    return m_memHash;
  }

  if (!m_sourceFileHashValid)
  {
    m_sourceFileHash = hash<std::string>(m_sourceFile);
    m_sourceFileHashValid = true;
  }

  if (!m_optionHashValid)
  {
    m_optionHash = hash<std::string>(m_options);
    m_optionHashValid = true;
  }

  if (!m_defineHashValid)
  {
    U32 a = FW_HASH_MAGIC, b = FW_HASH_MAGIC, c = FW_HASH_MAGIC;
    DefinesMap_t::iterator it;
    for (it = m_defines.begin(); it != m_defines.end(); ++it)
    {
        a += hash<std::string>(it->first);
        b += hash<std::string>(it->second);
        FW_JENKINS_MIX(a, b, c);
    }
    m_defineHash = ((U64)b << 32) | c;
    m_defineHashValid = true;
  }

  if (!m_preambleHashValid)
  {
    m_preambleHash = hash<std::string>(m_preamble);
    m_preambleHashValid = true;
  }

  U32 a = FW_HASH_MAGIC + m_sourceFileHash;
  U32 b = FW_HASH_MAGIC + m_optionHash;
  U32 c = FW_HASH_MAGIC + m_preambleHash;
  
  FW_JENKINS_MIX(a, b, c);
  a += (U32)(m_defineHash >> 32);
  b += (U32)m_defineHash;
  FW_JENKINS_MIX(a, b, c);
  
  m_memHash = ((U64)b << 32) | c;
  m_memHashValid = true;
  
  return m_memHash;
}

//------------------------------------------------------------------------

void CudaCompiler::createCacheDir(void)
{
  printf( "\t CudaCompiler::%s\n", __FUNCTION__);
  
  std::string cmd = "mkdir --parent " + m_cachePath;
  system( cmd.c_str() );
}

//------------------------------------------------------------------------

void CudaCompiler::writeDefineFile(void)
{  
  printf( "\t CudaCompiler::%s\n", __FUNCTION__);
  
  File file(m_cachePath + "/defines.inl", File::Create);
  BufferedOutputStream out(file);  
  
  DefinesMap_t::iterator it;
  for (it = m_defines.begin(); it != m_defines.end(); ++it) {
    out.writef("#define %s %s\n", it->first.c_str(), it->second.c_str());
  }
  out.writef("%s\n", s_staticPreamble.c_str());
  out.writef("%s\n", m_preamble.c_str());
  out.flush();  
}

//------------------------------------------------------------------------

void CudaCompiler::initLogFile(const std::string& name, const std::string& firstLine)
{  
  printf( "\t CudaCompiler::%s\n", __FUNCTION__);
  File file(name, File::Create);
  BufferedOutputStream out(file);
  out.writef("%s\n", firstLine.c_str());
  out.flush();  
}

//------------------------------------------------------------------------

bool CudaCompiler::runPreprocessor(std::string& cubinFile, std::string& finalOpts)
{
  printf( "\t CudaCompiler::%s\n", __FUNCTION__);
  
  fprintf( stderr, "%s : not COMPLETELY implemented.\n", __FUNCTION__ );
  
  
  // Preprocess.
  finalOpts = "";
  
  if (s_staticOptions.length()) {
    finalOpts += s_staticOptions + " ";
  }
  finalOpts += m_options;

  std::string logFile = m_cachePath + "/preprocess.log";
  
  std::string cmd = s_nvccCommand + " -E -o \"" + m_cachePath + "/preprocessed.cu\" " +
                    "-include \"" + m_cachePath + "/defines.inl\" " + 
                    finalOpts + " \"" + m_sourceFile + 
                    "\" 2>>\"" + logFile + "\"";

  initLogFile(logFile, cmd);
  
  if (0 != system(cmd.c_str()))
  {
    setLoggedError("CudaCompiler: Preprocessing failed!", logFile);
    return false;
  }

  // Specify binary format.
  if (s_staticBinaryFormat.length()) {
    finalOpts += s_staticBinaryFormat;
  } else {
    finalOpts += "-cubin";
  }
  finalOpts += " ";

  
  // Hash and find inline compiler options.
  std::string optionPrefix = "// EMIT_NVCC_OPTIONS ";
  File file(m_cachePath + "/preprocessed.cu", File::Read);
  BufferedInputStream in(file);

  
  U32 hashA = FW_HASH_MAGIC;
  U32 hashB = FW_HASH_MAGIC;
  U32 hashC = FW_HASH_MAGIC;
  
  /*
  for (int lineIdx = 0;; lineIdx++)
  {
    const char* linePtr = in.readLine();
  
    if (!linePtr) {
      break;
    }
    
    // Trim from the left.
    while (*linePtr == ' ' || *linePtr == '\t') {
      linePtr++;
    }
    
    // Directive or empty => ignore.
    if (*linePtr == '#' || *linePtr == '\0') {
      continue;
    }

    // Compiler option directive => record.
    std::string line(linePtr);
    if (line.find_first_of(optionPrefix) == 0u) {
      finalOpts += line.substr(optionPrefix.length()) + " ";
    }
    // Not a comment => hash.
    else if (line.find_first_of("//") != 0)
    {
      hashA += hash<std::string>(line);
      FW_JENKINS_MIX(hashA, hashB, hashC);
    }    
  }
  */
  
  // Override SM architecture.
  S32 smArch = m_overriddenSMArch;
  if (!smArch) {
    smArch = CudaModule::getComputeCapability();
  }

  finalOpts = removeOption(finalOpts, "-arch", true);
  finalOpts = removeOption(finalOpts, "--gpu-architecture", true);
  
  char smArch_str[32];
  sprintf(smArch_str, "-arch sm_%d ", smArch);
  finalOpts += std::string(smArch_str);

  // Override pointer width.
  // CUDA 3.2 => requires -m32 for x86 build and -m64 for x64 build.
  if (CudaModule::getDriverVersion() >= 32)
  {
    finalOpts = removeOption(finalOpts, "-m32", false);
    finalOpts = removeOption(finalOpts, "-m64", false);
    finalOpts = removeOption(finalOpts, "--machine", true);

#if FW_64
      finalOpts += "-m64 ";
#else
      finalOpts += "-m32 ";
#endif
  }
  /**/
  
  // Hash final compiler options and version.
  hashA += hash<std::string>(finalOpts);
  hashB += s_nvccVersionHash;
  FW_JENKINS_MIX(hashA, hashB, hashC);
  
  std::string fileName = hashToString(hashB) + hashToString(hashC);
  cubinFile = m_cachePath + "/" + fileName + ".cubin";
  
  /**/
  return true;
}

//------------------------------------------------------------------------

bool CudaCompiler::runCompiler( const std::string& cubinFile, 
                                const std::string& finalOpts)
{
  printf( "\t CudaCompiler::%s\n", __FUNCTION__);
  std::string logFile = m_cachePath + "/compile.log";
  
  std::string cmd = s_nvccCommand + " -o \"" + cubinFile + 
                    "\" -include \"" + m_cachePath + "/defines.inl\" " + 
                    + " -include cuda.h " +
                    finalOpts + " \"" + 
                    m_sourceFile +  
                    "\" 2>>\"" + logFile + "\"";

  initLogFile( logFile, cmd);
  
  if (system(cmd.c_str()) != 0 || !fileExists(cubinFile)) 
  {
    setLoggedError("CudaCompiler: Compilation failed!", logFile);
    return false;
  }

#if SHOW_NVCC_OUTPUT
  setLoggedError("", logFile);
  printf( "%s\n", getError().c_str());
  clearError();
#endif

  return true;
}

//------------------------------------------------------------------------

void CudaCompiler::setLoggedError(const std::string& description, const std::string& logFile)
{
  printf( "\t CudaCompiler::%s\n", __FUNCTION__);
  fprintf( stderr, "%s : not implemented.\n", __FUNCTION__ );
  
#if 0
  std::string message = description;
  
  File file( logFile, File::Read);
  BufferedInputStream in(file);  
  in.readLine();
  
  while (1)
  {
    const char* linePtr = in.readLine();
    
    if (!linePtr) {
      break;
    }
    
    if (*linePtr) {
      message += '\n';
    }
    
    message += linePtr;
  }
  //setError("%s", message.c_str());
#endif
}

} // namespace FW
