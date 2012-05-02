#ifndef ENGINE_OPENGL_HPP_
#define ENGINE_OPENGL_HPP_

#include <GL/glew.h>
#include "Common.hpp"

class OpenGL
{
  private:
    // ++ Constants ++
    static int sMaxDrawBuffer;
    static int sMaxColorAttachments;
    static int sTextureMaxAnisotropy;
    
    // ++ States ++
    static int viewport_[4];    
    
    static bool bDepthTest;
    static bool bBlending;    
    static bool bStencilTest;
    static bool bCullFace;
  
  public:
    static void Init()
    {
    }
  
    // ++ Hardware info ++
    static const unsigned char* GetVendor() {
      return (const unsigned char*)glGetString(GL_VENDOR);
    }

    static const unsigned char* GetRenderer() {
      return (const unsigned char*)glGetString(GL_RENDERER);
    }

    static const unsigned char* GetVersion() {
      return (const unsigned char*)glGetString(GL_VERSION);
    }

    static const unsigned char* GetGLSLVersion() {
      return (const unsigned char*)glGetString(GL_SHADING_LANGUAGE_VERSION);
    }
    
    // ++ Constant getters ++
    static inline int MaxDrawBuffer() { return sMaxDrawBuffer; }
    static inline int MaxColorAttachments() { return sMaxColorAttachments; }
    static inline int TextureMaxAnisotropy() { return sTextureMaxAnisotropy; }
    

    // ++ States ++
    static inline void SetViewport( int x, int y, int width, int height)
    {
      viewport_[0] = x; 
      viewport_[1] = y;
      viewport_[2] = width;
      viewport_[3] = height;
      glViewport( x, y, width, height);
    }

    static inline void GetViewport(int vp[4])
    {
      vp[0] = viewport_[0]; 
      vp[1] = viewport_[1]; 
      vp[2] = viewport_[2]; 
      vp[3] = viewport_[3];
    }

    static inline void EnableDepth() { bDepthTest_ = true; glEnable(GL_DEPTH_TEST); }
    static inline void DisableDepth() { bDepthTest_ = false; glDisable(GL_DEPTH_TEST); }
    static inline bool IsDepthEnabled() { return bDepthTest_; }

    static inline void EnableStencil() { bStencilTest_= true; glEnable(GL_STENCIL_TEST); }
    static inline void DisableStencil() { bStencilTest_= false; glDisable(GL_STENCIL_TEST); }
    static inline bool IsStencilEnabled() { return bStencilTest_; }

    static inline void EnableBlending() { bBlending_ = true; glEnable(GL_BLEND); }
    static inline void DisableBlending() { bBlending_ = false; glDisable(GL_BLEND); }
    static inline bool IsBlendingEnabled() { return bBlending_; }
    
  private:
    static inline void InitConstants()
    {
      glGetIntegerv( GL_MAX_DRAW_BUFFERS, &sMaxDrawBuffer);
      glGetIntegerv( GL_MAX_COLOR_ATTACHMENTS, &sMaxColorAttachments);
      
#ifdef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
      glGetIntegerv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &sTextureMaxAnisotropy);
#else
      sTextureMaxAnisotropy_ = 0;
#endif
    }
};


class State
{
 private:
  static int viewport_[4];
  
  static bool bDepthTest_;
  static bool bStencilTest_;
  static bool bBlending_;
  static bool bCullFace_;


 public:
  static inline void SetViewport( int x, int y, int width, int height)
  {
    viewport_[0] = x; 
    viewport_[1] = y;
    viewport_[2] = width;
    viewport_[3] = height;
    glViewport( x, y, width, height);
  }
  
  //
  static inline void GetViewport(int vp[4])
  {
    vp[0] = viewport_[0]; 
    vp[1] = viewport_[1]; 
    vp[2] = viewport_[2]; 
    vp[3] = viewport_[3];
  }
    
  
  // ++ OpenGL states ++
  static inline void EnableDepth() { bDepthTest_ = true; glEnable(GL_DEPTH_TEST); }
  static inline void DisableDepth() { bDepthTest_ = false; glDisable(GL_DEPTH_TEST); }
  static inline bool IsDepthEnabled() { return bDepthTest_; }
  
  static inline void EnableStencil() { bStencilTest_= true; glEnable(GL_STENCIL_TEST); }
  static inline void DisableStencil() { bStencilTest_= false; glDisable(GL_STENCIL_TEST); }
  static inline bool IsStencilEnabled() { return bStencilTest_; }
  
  static inline void EnableBlending() { bBlending_ = true; glEnable(GL_BLEND); }
  static inline void DisableBlending() { bBlending_ = false; glDisable(GL_BLEND); }
  static inline bool IsBlendingEnabled() { return bBlending_; }

  //static inline void PolygonMode();
  
  //static void Print();
};

/*
class Error
{
  static inline GLenum Get() 
  {
    return glGetError();
  }
  
  // Stringify an OpenGL Error Enum
  static const char* GetString(GLenum err);
  
  static void Check(const char *file, const int line, const char *msg)
  {
    GLenum err = Get();
    
    if (err != GL_NO_ERROR) 
    {
      fprintf( stderr, "OpenGL error @ %s [%d]: %s [%s].\n", 
               file, line, msg, GetString(err) );
    }
  }
};

} // namespace OpenGL

#ifndef CHECKGLERROR
#define CHECKGLERROR(msg) \
        OpenGL::Error::Check( __FILE__, __LINE__, msg)
#endif
*/

#endif // ENGINE_OPENGL_HPP_
