#ifndef SCENEGL_HPP_
#define SCENEGL_HPP_

#include <GL/glew.h>
#include "engine/renderer/Program.hpp"

class Camera;

/// =========================================
/// Scene datas for OpenGL rendering
/// =========================================
class SceneGL
{
  public:
    Program m_program;
    
    //MeshGL m_mesh;
    
        
  public:
    SceneGL();
    ~SceneGL();
    
    void init();
    
    void render( const Camera& camera );
    
  private:
    void initShaders();
};

#endif //SCENEGL_HPP_
