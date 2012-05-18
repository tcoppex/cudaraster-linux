#ifndef APP_HPP
#define APP_HPP

#include "engine/Application.hpp"

#include "engine/Camera.hpp"
#include "SceneCR.hpp"
#include "SceneGL.hpp"


/// =========================================
/// Sample application to test the cudaraster
/// port.
/// =========================================
class App : public Application
{    
  private:
    /// Defines the rendering mode
    enum Mode
    {
      MODE_CUDARASTER = 0,
      MODE_OPENGL,
            
      NUM_MODE
    };
    
  
  public:
    /// Default window's resolution
    /// (must be a multiple of CR_TILE_SIZE)
    static const int kScreenWidth  = 720;
    static const int kScreenHeight = 480;  
    
    /// Default shaders' path
    static const char* kShadersPath;
    
    /// Holds rendering states
    static struct RenderState
    {
      bool bTexturing;
      bool bDepth;
      bool bBlend;
      bool bLerp;
      bool bQuads;
      int numSamples;
      
      RenderState()
        : bTexturing(false),
          bDepth(true),
          bBlend(false),
          bLerp(false),
          bQuads(false),
          numSamples(1)
      {}
    } kState;
  
  
  private:
    /// Rendering mode
    Mode m_mode;
    
    /// Free-view camera
    Camera m_camera;
    
    /// CudaRaster scene handler
    SceneCR m_sceneCR;
    
    /// OpenGL scene handler
    SceneGL m_sceneGL;
    
  public:
    App() 
      : Application(),        
        m_mode(MODE_OPENGL)
    {}
    
    virtual ~App();

    // ++ Overrided Listener methods ++
    virtual void reshape(int w, int h);
    virtual void display();
    virtual void keyboard( unsigned char key, int x, int y);
    virtual void special( int key, int x, int y);
    virtual void specialUp( int key, int x, int y);
    virtual void mouse(int button, int state, int x, int y);
    virtual void motion(int x, int y);
    virtual void idle();
    
    
  private:
    /// Redefines the context (window + event) creation
    virtual void _initContext(int argc, char *argv[]);

    /// Redefines the generic datas initialization
    virtual void _initObject( int argc, char *argv[]);
};


#endif //APP_HPP
