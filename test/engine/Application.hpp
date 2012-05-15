#ifndef ENGINE_APPLICATION_HPP
#define ENGINE_APPLICATION_HPP

#include "Listener.hpp"
#include <cstdlib>
class Context;


/// ==================================
/// Base class to defines applications
/// ==================================
class Application : public Listener
{    
  protected:
    bool m_bInitialized;
    Context *m_Context;
  
  
  public:
    Application() 
        : Listener(), 
          m_bInitialized(false),
          m_Context(NULL) 
    {}
    
    virtual ~Application() {}
    
    // ++ Main initializer ++
    virtual void init( int argc, char *argv[]);
    
    // ++ MainLoop ++
    virtual void run();

    // ++ Events handlers ++
    virtual void reshape(int, int);
    virtual void display();
    virtual void keyboard( unsigned char, int, int);
    virtual void special( int, int, int);
    virtual void specialUp( int, int, int);
    virtual void mouse(int, int, int, int);
    virtual void motion(int, int);
    virtual void idle();    

  protected:
    // ++ Sub initializers ++
    virtual void _initContext( int argc, char *argv[]);
    virtual void _initOpenGL( int argc, char *argv[]);
    virtual void _initCUDA( int argc, char *argv[]);
    virtual void _initObject( int argc, char *argv[]);
     
    //virtual void _update() {}
    //virtual void _render() {}
    
  private:
    // ++ Disallow copies & affectations ++
    Application(const Application&);
    Application& operator= (const Application&);
};

#endif //ENGINE_APPLICATION_HPP
