#ifndef SCENEGL_HPP_
#define SCENEGL_HPP_

#include <GL/glew.h>
#include "engine/renderer/Program.hpp"
#include "engine/renderer/VertexBuffer.hpp"

class Camera;
class Data;

/// =========================================
/// Scene datas for OpenGL rendering
/// =========================================
class SceneGL
{
  public:
    Program m_program;    
    VertexBuffer m_mesh;
    
    
  public:
    SceneGL();
    ~SceneGL();
    
    void init(const Data& data);
    
    void render( const Camera& camera );
    
  private:
    void initGeometry(const Data& data);
    void initShader();
};

#endif //SCENEGL_HPP_
