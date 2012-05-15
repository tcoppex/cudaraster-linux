#include "Application.hpp"

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <cuda.h>
#include "Context.hpp"


// -----------------------------------------------

void Application::run()
{
  glutMainLoop();
}

void Application::init( int argc, char *argv[])
{
  /// the initialization order matters
  _initContext( argc, argv);
  _initOpenGL( argc, argv);
  _initCUDA( argc, argv);
  _initObject( argc, argv);

  if (NULL == m_Context)
  {
    fprintf( stderr, "Error: Context has not been initialized. Exit.\n");
    exit( EXIT_FAILURE );
  }
  
  m_bInitialized = true;
}


// -----------------------------------------------

void Application::reshape( int w, int h)
{
  glViewport( 0, 0, w, h);
}

void Application::display()
{
  m_Context->flush();
}

void Application::keyboard( unsigned char key, int x, int y)
{
}

void Application::special( int key, int x, int y)
{
}

void Application::specialUp( int key, int x, int y)
{
}

void Application::mouse(int button, int state, int x, int y)
{
}

void Application::motion(int x, int y)
{
}

void Application::idle()
{
}

// -----------------------------------------------


void Application::_initContext( int argc, char *argv[])
{
  // Default context initialization

  m_Context = new Context( this, argc, argv);
  
  m_Context->setGLVersion( 2, 1);
  m_Context->setFlags( GLUT_FORWARD_COMPATIBLE );
  m_Context->setProfile( GLUT_COMPATIBILITY_PROFILE );
  
  
  const int flags = GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH;
      
  int h = m_Context->createWindow( Context::kDefaultWidth, 
                                   Context::kDefaultHeight, 
                                   flags, 
                                   "Base Application");
  
  if (h < 1)
  {
    fprintf( stderr, "Error: Context creation has failed.\n");
    exit( EXIT_FAILURE );
  }
}

void Application::_initOpenGL( int argc, char *argv[])
{
  /// Init OpenGL Extension Wrangler
  glewExperimental = GL_TRUE;
  GLenum result = glewInit();
  
  if (GLEW_OK != result)
  {
    fprintf( stderr, "Error: %s\n", glewGetErrorString(result));
    exit( EXIT_FAILURE );
  }
  fprintf( stderr, "GLEW version: %s\n", glewGetString(GLEW_VERSION));
  
  
  /// Init OpenGL wrapper
  //OpenGL::Init();
  
  // The OpenGL error handler may not be initialized
  glGetError();
}

void Application::_initCUDA( int argc, char *argv[])
{
  if (CUDA_SUCCESS != cuInit(0))
  {
    fprintf( stderr, "Error: CUDA initialization has failed.\n");
    exit( EXIT_FAILURE );
  }
}

void Application::_initObject( int argc, char *argv[])
{
  // initialize application specific datas
}
