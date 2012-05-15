#include "Context.hpp"

#include <cassert>
#include <GL/freeglut.h>
#include "Listener.hpp"


namespace { 

Listener* g_pListener = NULL;

void glut_reshape_callback(int w, int h);
void glut_display_callback();
void glut_keyboard_callback(unsigned char key, int x, int y);
void glut_special_callback(int key, int x, int y);
void glut_specialUp_callback(int key, int x, int y);
void glut_motion_callback(int x, int y);
void glut_mouse_callback(int button, int state, int x, int y);
void glut_idle_callback();

} // namespace 



// -------------------



Context::Context( Listener *pListener, int argc, char *argv[])
    : m_glMajor(2),
      m_glMinor(1),
      m_glFlags(GLUT_FORWARD_COMPATIBLE),
      m_glProfile(GLUT_COMPATIBILITY_PROFILE),
      
      m_handle(0),
      m_width(kDefaultWidth),
      m_height(kDefaultHeight)
{
  glutInit( &argc, argv);

  // Set 'glutMainLoop' to return
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
  
  // ugly.. but safe ! somehow..
  g_pListener = pListener;
  assert( NULL != g_pListener );
}


Context::~Context()
{
}


// -------------------


int Context::createWindow( int width, int height, int flags, const char *name)
{  
  glutInitDisplayMode( flags );
  glutInitWindowSize( width, height);
  
  m_handle = glutCreateWindow( name );

  if (m_handle < 1) {
    // error
    return m_handle;
  }

    
  glutReshapeFunc( glut_reshape_callback );
  glutDisplayFunc( glut_display_callback );
  glutKeyboardFunc( glut_keyboard_callback );
  glutSpecialFunc( glut_special_callback );
  glutSpecialUpFunc( glut_specialUp_callback );
  glutMouseFunc( glut_mouse_callback );
  glutMotionFunc( glut_motion_callback );
  glutIdleFunc( glut_idle_callback );  
  
    
  return m_handle;
}


// -------------------


namespace {
 
void glut_reshape_callback(int w, int h)
{
  g_pListener->reshape(w, h);
  glutPostRedisplay();
}

void glut_display_callback()
{
  g_pListener->display();
  glutPostRedisplay();
}

void glut_keyboard_callback(unsigned char key, int x, int y)
{
  g_pListener->keyboard( key, x, y);
  glutPostRedisplay();
}

void glut_special_callback(int key, int x, int y)
{
  g_pListener->special( key, x, y);
  glutPostRedisplay();
}

void glut_specialUp_callback(int key, int x, int y)
{
  g_pListener->specialUp( key, x, y);
  glutPostRedisplay();
}

void glut_motion_callback(int x, int y)
{
  g_pListener->motion( x, y);
  glutPostRedisplay();
}

void glut_mouse_callback(int button, int state, int x, int y)
{
  g_pListener->mouse( button, state, x, y);
  glutPostRedisplay();
}

void glut_idle_callback()
{
  g_pListener->idle();
  glutPostRedisplay();
}
  
} // namespace 
