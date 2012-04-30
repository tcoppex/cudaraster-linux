#ifndef APP_HPP
#define APP_HPP

// Application layout
#include "engine/Application.hpp"

// CudaRaster
#include <cudaraster/CudaRaster.hpp>
#include <framework/base/Defs.hpp>
#include <framework/gpu/CudaCompiler.hpp>


class App : public Application
{
  private:
    enum Mode
    {
      MODE_CUDARASTER = 0,
      MODE_OPENGL,
            
      NUM_MODE
    };

  
  public:
    static const int kScreenWidth  = 960;
    static const int kScreenHeight = 540;
  
  private:    
    /// initialization occured in Constructor, thus some bug can happens !
    FW::CudaCompiler m_cudaCompiler;
    FW::CudaRaster m_cudaRaster;

    // Pipe.
    bool m_pipeDirty;
    FW::CudaSurface* m_colorBuffer;
    FW::CudaSurface* m_depthBuffer;
    FW::CudaModule* m_cudaModule;
    CUfunction m_vertexShaderKernel;
    
    // State.
    Mode m_mode;
    bool m_enableTexPhong;
    bool m_enableDepth;
    bool m_enableBlend;
    bool m_enableLerp;
    bool m_enableQuads;
    FW::S32 m_numSamples;
    
    // Mesh.
#if 0
    //Mesh *m_Mesh;
#else
    //MeshBase* m_mesh;
    FW::S32 m_numVertices;
    FW::S32 m_numMaterials;
    FW::S32 m_numTriangles;
    FW::Buffer m_inputVertices;           // model space vertices
    FW::Buffer m_shadedVertices;          // vertices transform by the 'vertex shader'
    FW::Buffer m_materials;
    FW::Buffer m_vertexIndices;           // model triangulation vertex indices
    FW::Buffer m_vertexMaterialIdx;
    FW::Buffer m_triangleMaterialIdx;
    //TextureAtlas m_textureAtlas;
#endif
    
  public:
    App() 
        : Application(),
          m_pipeDirty(true),
          
          m_colorBuffer(NULL),
          m_depthBuffer(NULL),
          m_cudaModule(NULL),
          m_vertexShaderKernel(NULL),
          
          m_mode(MODE_OPENGL),
          m_enableTexPhong(false),//
          m_enableDepth(true),
          m_enableBlend(false),
          m_enableLerp(false),
          m_enableQuads(false),
          m_numSamples(1)
    {}
    
    virtual ~App();

    /// Listener event handlers override
    virtual void display();
    virtual void idle();  
    
    
  private:
    /// Redefines the context (window + event) creation
    virtual void _initContext(int argc, char *argv[]);

    /// Redefines the generic datas initialization
    virtual void _initObject( int argc, char *argv[]);

    /// -
    void initPipe();
    
    /// Precompute modules with different options if they don't exist 
    void firstTimeInit();
        
    void render_cudaraster();
    void render_opengl();
};


#endif //APP_HPP
