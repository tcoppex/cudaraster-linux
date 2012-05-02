
#include "SceneGL.hpp"

#include <glsw/glsw.h>
#include "App.hpp"
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
  
  glDisable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  
  glDisable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  if (App::kState.bDepth)  glEnable(GL_DEPTH_TEST);
  if (App::kState.bBlend)  glEnable(GL_BLEND);

  
  m_program.bind();
    const glm::mat4 &mvpMatrix = camera.getViewProjMatrix();
    m_program.setUniform( "uModelViewProjMatrix", mvpMatrix);
    //m_rMesh.draw();
  m_program.unbind();
}


// -----------------------------------------------
// Initializers
// -----------------------------------------------

void SceneGL::init()
{
  initShaders();
}

void SceneGL::initShaders()
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

