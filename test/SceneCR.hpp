#ifndef SCENECR_HPP_
#define SCENECR_HPP_

// CudaRaster
#include <cudaraster/CudaRaster.hpp>
#include <framework/base/Defs.hpp>
#include <framework/gpu/CudaCompiler.hpp>
#include "engine/renderer/Program.hpp"

class Camera;
class Data;


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
    
    // ++ Pipe ++
    bool m_pipeDirty;
    FW::CudaSurface* m_colorBuffer;
    FW::CudaSurface* m_depthBuffer;
    FW::CudaModule* m_cudaModule;
    CUfunction m_vertexShaderKernel;
        
    // ++ Mesh data ++
    FW::Buffer m_inVertices;          // Vertex Shader input vertices
    FW::Buffer m_outVertices;         // Vertex Shader output vertices (transformed)
    FW::Buffer m_indices;
    int m_numVertices;
    int m_numTriangles;
    
    // ++ ScreenMapping Program shader ++
    Program m_screenMappingPS;
    
    
  public:
    SceneCR()
        : m_pipeDirty(true),          
          m_colorBuffer(NULL),
          m_depthBuffer(NULL),
          m_cudaModule(NULL),
          m_vertexShaderKernel(NULL),
          m_numVertices(0),
          m_numTriangles(0)
    {}
          
    ~SceneCR();
        
    void init(const Data& data);    
    void render( const Camera& camera );
    
  
  private:
    /// Setup geometry data as CudaRaster compatible buffers
    void initGeometry(const Data& data);
    
    /// Init the screen mapping shader
    void initShader();
    
    /// Init the cudaraster pipeline
    void initPipe();
    
    /// Precompute modules with different options if they don't exist
    void firstTimeInit();
    
    /// Render the CudaRaster framebuffer to the screen
    void resolveToScreen();
};

#endif //SCENECR_HPP_
