#ifndef SCENECR_HPP_
#define SCENECR_HPP_

// CudaRaster
#include <cudaraster/CudaRaster.hpp>
#include <framework/base/Defs.hpp>
#include <framework/gpu/CudaCompiler.hpp>

class Camera;


/// =========================================
/// Scene datas for CudaRaster rendering
/// =========================================
class SceneCR
{
  public:
    /// Some init occured in Constructor, 
    /// => check there if bugs happens
    FW::CudaCompiler m_cudaCompiler;
    FW::CudaRaster m_cudaRaster;
    
    // ++ CudaRaster compatible buffer ++
    FW::Buffer m_inVertices;
    FW::Buffer m_outVertices;
    FW::Buffer m_indices;
    
    int m_numVertices;
    int m_numTriangles;
    
    
    // ++ Pipe ++
    bool m_pipeDirty;
    FW::CudaSurface* m_colorBuffer;
    FW::CudaSurface* m_depthBuffer;
    FW::CudaModule* m_cudaModule;
    CUfunction m_vertexShaderKernel;
    
    
  public:
    SceneCR()
        : m_numVertices(0),
          m_numTriangles(0),
          
          m_pipeDirty(true),          
          m_colorBuffer(NULL),
          m_depthBuffer(NULL),
          m_cudaModule(NULL),
          m_vertexShaderKernel(NULL)
    {}
          
    ~SceneCR();
    
    void init();
    
    void render( const Camera& camera );
    
  
  private:
    // ++ Init the cudaraster pipeline ++
    void initPipe();
    
    // ++ Precompute modules with different options if they don't exist  ++
    void firstTimeInit();
};

#endif //SCENECR_HPP_
