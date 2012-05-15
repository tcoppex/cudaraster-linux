#ifndef ENGINE_LISTENER_HPP_
#define ENGINE_LISTENER_HPP_

/// ================================================
/// Encapsulate event functions used by the GLUT 
/// Windows Manager.
/// ================================================
class Listener
{
  public:
    // GLUT functions handler
    virtual void reshape(int, int) {}
    virtual void display() {}
    virtual void keyboard( unsigned char, int, int) {}
    virtual void special( int, int, int) {}
    virtual void specialUp( int, int, int) {}
    virtual void mouse(int, int, int, int) {}
    virtual void motion(int, int) {}
    virtual void idle() {}    
};

#endif //ENGINE_LISTENER_HPP_
