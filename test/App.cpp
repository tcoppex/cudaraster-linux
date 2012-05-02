
#include "App.hpp"

#include <cstdio>
#include <cstdlib>
#include "engine/Context.hpp"



// -----------------------------------------------
// Static Definition
// -----------------------------------------------

const char* App::kShadersPath = "../test/shader/";

App::RenderState App::kState;


// -----------------------------------------------
// Anonymous declaration
// -----------------------------------------------

namespace {

// translate GLUT key as generic Camera key
void moveCamera( Camera& camera, int key, bool isPressed);

}


// -----------------------------------------------
// Constructor / Destructor
// -----------------------------------------------

App::~App()
{
}


// -----------------------------------------------
// Initializers
// -----------------------------------------------

void App::_initContext( int argc, char *argv[])
{
  m_Context = new Context( this, argc, argv);
  
  m_Context->setGLVersion( 3, 3);
  m_Context->setFlags( GLUT_FORWARD_COMPATIBLE );
  m_Context->setProfile( GLUT_CORE_PROFILE );
  
  
  const int flags = GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL;
  const char *name = "CudaRaster test";
  
  int h = m_Context->createWindow( App::kScreenWidth, App::kScreenHeight, flags, name);
  
  if (h < 1)
  {
    fprintf( stderr, "Error: Context creation has failed.\n");
    exit( EXIT_FAILURE );
  }
}

void App::_initObject( int argc, char *argv[])
{     
  /// Create the OpenGL-CudaRaster mesh data
  //meshUtils::setup_cubeMesh( m_mesh );  
  
  //m_sceneCR.setDatas( m_mesh );
  //m_sceneGL.setDatas( m_mesh );
  
  m_sceneCR.init();
  m_sceneGL.init();
  
  /// Set default camera parameters
  m_camera.setViewParams( glm::vec3( 0.0f, 2.0f, 4.0f),       // Eye position
                          glm::vec3( 0.0f, 0.0f, 0.0f) );     // Eye target
}



// -----------------------------------------------
// Event Handlers
// -----------------------------------------------

void App::reshape(int w, int h)
{
  // crappy
  if ((w != kScreenWidth) || (h != kScreenHeight)) {
    glutReshapeWindow( kScreenWidth, kScreenHeight);
    return;
  }
  
  glViewport( 0, 0, w, h);
  
  const float fov = 60.0f;
  float aspectRatio = static_cast<float>(w) / static_cast<float>(h);
  const float zNear = 0.1f;
  const float zFar = 100.0f;
  
  m_camera.setProjectionParams( fov, aspectRatio, zNear, zFar);
}

void App::display()
{
  if (MODE_CUDARASTER==m_mode) {
    m_sceneCR.render( m_camera );
  } else {
    m_sceneGL.render( m_camera );
  }

  m_Context->flush();
}

void App::keyboard( unsigned char key, int x, int y)
{
  switch (key)
  {
    case 27: // ESCAPE KEY
      exit( EXIT_SUCCESS );
    break;
      
    default:
      break;
  }
}

void App::special( int key, int x, int y)
{
  moveCamera( m_camera, key, true);
}

void App::specialUp( int key, int x, int y)
{
  moveCamera( m_camera, key, false);
}

void App::mouse(int button, int state, int x, int y)
{
  if (GLUT_DOWN == state) {
    m_camera.motionHandler( x, y, true);
  }
}

void App::motion(int x, int y)
{
  m_camera.motionHandler( x, y, false);
}

void App::idle()
{
  m_camera.update();
  glutPostRedisplay();
}


// -----------------------------------------------
// Anonymous definition
// -----------------------------------------------

namespace {

void moveCamera( Camera& camera, int key, bool isPressed)
{
  switch (key)
  {
    case GLUT_KEY_UP:
      camera.keyboardHandler( MOVE_FORWARD, isPressed);
    break;
    
    case GLUT_KEY_DOWN:
      camera.keyboardHandler( MOVE_BACKWARD, isPressed);
    break;
    
    case GLUT_KEY_LEFT:
      camera.keyboardHandler( MOVE_LEFT, isPressed);
    break;
    
    case GLUT_KEY_RIGHT:
      camera.keyboardHandler( MOVE_RIGHT, isPressed);
    break;
    
    default:
    break;
  }
}

} // namespace

