#ifndef ENGINE_CONTEXT_HPP_
#define ENGINE_CONTEXT_HPP_

#include <GL/freeglut.h>
class Listener;


/// ================================================
/// Handles window context creation & manages events
/// - Uses the GLUT library
/// ================================================
class Context
{
  public:
    static const int kDefaultWidth = 1024;
    static const int kDefaultHeight = 768;
      
  private:
    // ++ OpenGL Profile infos ++
    int m_glMajor;
    int m_glMinor;
    unsigned int m_glFlags;
    unsigned int m_glProfile;
    
    // ++ Context handle ++
    int m_handle;
    
    // ++ Window resolution ++
    int m_width;
    int m_height;
    
  
  public:
    Context( Listener *pListener, int argc, char *argv[]);
    ~Context();
    

    inline void setGLVersion( int major, int minor)
    {
      m_glMajor = major;
      m_glMinor = minor;  
      glutInitContextVersion( m_glMajor, m_glMinor);
    }

    inline void setFlags( unsigned int flags )
    {
      m_glFlags = flags;
      glutInitContextFlags( m_glFlags );
    }

    inline void setProfile( unsigned int profile )
    {
      m_glProfile = profile;
      glutInitContextProfile( m_glProfile );
    }
    
    
    inline void flush()
    {      
      glutSwapBuffers();
      glutPostRedisplay();//
    }
        
    int createWindow( int width, int height, int flags, const char *name="");
};

#endif //ENGINE_CONTEXT_HPP_
