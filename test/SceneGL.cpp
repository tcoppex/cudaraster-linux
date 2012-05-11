
#include "SceneGL.hpp"

#include <glsw/glsw.h>
#include "App.hpp"
#include "Data.hpp"
#include "engine/Camera.hpp"


// -----------------------------------------------
// Constructor / Destructor
// -----------------------------------------------

SceneGL::SceneGL()
{
}

SceneGL::~SceneGL()
{
}


// -----------------------------------------------
// Render
// -----------------------------------------------

void SceneGL::render(const Camera& camera)
{ 
  glClearColor( 0.2f, 0.4f, 0.8f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  
  (App::kState.bDepth) ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  
  (App::kState.bBlend) ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  
  m_program.bind();
  {
    const glm::mat4 &mvpMatrix = camera.getViewProjMatrix();
    m_program.setUniform( "uModelViewProjMatrix", mvpMatrix);
    m_program.setUniform( "uColor", glm::vec3(1.0f, 0.7f, 0.0f));
    m_mesh.draw();
  }
  m_program.unbind();
}


// -----------------------------------------------
// Initializers
// -----------------------------------------------

void SceneGL::init(const Data& data)
{
  initShader();
  initGeometry(data);
}

void SceneGL::initGeometry(const Data& data)
{
  assert( !data.positions.empty() );
 
  // Vertices
  std::vector<glm::vec3> &positions = m_mesh.getPositions();  
  positions.resize( data.numVertices );
  
  for (size_t i=0u; i<positions.size(); ++i)
  {
    positions[i] = glm::vec3( data.positions[3u*i+0u],
                              data.positions[3u*i+1u],
                              data.positions[3u*i+2u] );
  }
  
  // Indices
  if (!data.indices.empty()) {
    std::vector<GLuint> &indices = m_mesh.getIndices();
    indices.assign( data.indices.begin(), data.indices.end());
  }
  
  m_mesh.generate();
  m_mesh.complete( GL_STATIC_DRAW );
  m_mesh.cleanData();
}

void SceneGL::initShader()
{
  /// GLSW, shader file manager
  glswInit();
  glswSetPath( App::kShadersPath, ".glsl");
  glswAddDirectiveToken("*", "#version 330 core");  
  
  m_program.generate();
    m_program.addShader( VERTEX_SHADER, "PassThrough.Vertex");
    m_program.addShader( FRAGMENT_SHADER, "PassThrough.Fragment");
  m_program.link();
  
  glswShutdown();
}

