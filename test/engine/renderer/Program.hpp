#ifndef RENDERER_PROGRAM_HPP_
#define RENDERER_PROGRAM_HPP_

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


enum ShaderType
{
  VERTEX_SHADER = GL_VERTEX_SHADER,
  GEOMETRY_SHADER = GL_GEOMETRY_SHADER,
  FRAGMENT_SHADER = GL_FRAGMENT_SHADER
};

/// =================================
/// Class wrapper for OpenGL Program
/// =================================
class Program
{
  private:
    GLuint m_id;
 
 public:
    Program() : m_id(0u) {}
    ~Program() 
    {
      if (isGenerated())
      {
        destroy();
        m_id = 0u;
      }
    }
    
    inline void generate() { assert(!isGenerated());  m_id=glCreateProgram(); }  
    virtual void destroy() { glDeleteProgram(m_id); }

    bool addShader( const ShaderType shaderType, const char* name);


    bool link();

    inline void bind() 
    { 
      glUseProgram(id());
    }

    static inline void unbind() 
    { 
      glUseProgram(0u);
    }

    inline GLint getUniformLocation(const char* name) const 
    { 
      return glGetUniformLocation( id(), name);
    }  

    // ++ Set Uniform ++
    inline void setUniform(const char* name, const GLint v) const
    {
      glUniform1i( getUniformLocation(name), v);
    }

    inline void setUniform(const char* name, const GLfloat v) const
    {
      glUniform1f( getUniformLocation(name), v);
    }

    inline void setUniform(const char* name, const float v0, const float v1) const 
    {
      glUniform2f( getUniformLocation(name), v0, v1);
    }

    inline void setUniform(const char* name, const float v0, const float v1, 
                           const float v2) const
    {
      glUniform3f( getUniformLocation(name), v0, v1, v2);
    }

    inline void setUniform(const char* name, const float v0, const float v1, 
                           const float v2, const float v3) const
    {
      glUniform4f( getUniformLocation(name), v0, v1, v2, v3);
    }


    // ++ set Vector uniforms ++
    inline void setUniform(const char* name, const glm::vec2 &v) const 
    {
      glUniform2fv( getUniformLocation(name), 1, glm::value_ptr(v));
    }

    inline void setUniform(const char* name, const glm::vec3 &v) const 
    {
      glUniform3fv( getUniformLocation(name), 1, glm::value_ptr(v));
    }

    inline void setUniform(const char* name, const glm::vec4 &v) const 
    {
      glUniform4fv( getUniformLocation(name), 1, glm::value_ptr(v));
    }


    // ++ set Matrix uniforms ++
    inline void setUniform(const char* name, const glm::mat3 &v) const 
    { 
      glUniformMatrix3fv( getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(v));
    }  

    inline void setUniform(const char* name, const glm::mat4 &v) const 
    { 
      glUniformMatrix4fv( getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(v));
    }


    // ++ set Arrays uniforms ++
    inline void setUniform(const char* name, const GLint *v, int count) const 
    {
      glUniform1iv( getUniformLocation(name), count, v);
    }

    inline void setUniform(const char* name, const GLfloat *v, int count) const 
    {
      glUniform1fv( getUniformLocation(name), count, v);
    }

    inline void setUniform(const char* name, const glm::vec2 *v, int count) const 
    {
      glUniform2fv( getUniformLocation(name), count, glm::value_ptr(*v));
    }

    inline void setUniform(const char* name, const glm::vec3 *v, int count) const 
    {
      glUniform3fv( getUniformLocation(name), count, glm::value_ptr(*v));
    }  

    inline void setUniform(const char* name, const glm::vec4 *v, int count) const 
    {
      glUniform4fv( getUniformLocation(name), count, glm::value_ptr(*v));
    }

    inline void setUniform(const char* name, const glm::mat3 *v, int count) const 
    {
      glUniformMatrix3fv( getUniformLocation(name), count, GL_FALSE, glm::value_ptr(*v));
    }

    inline void setUniform(const char* name, const glm::mat4 *v, int count) const 
    {
      glUniformMatrix4fv( getUniformLocation(name), count, GL_FALSE, glm::value_ptr(*v));
    }


    inline const GLuint id() const {return m_id;}
    inline bool isGenerated() {return m_id!=0u;}

  private:
    // ++ Disallow copies & affectations ++
    Program(const Program&);
    Program& operator= (const Program&);
};



/** 
 ------------------------------------------------------------------------------
 
Program program;

program.generate();
  program.addShader( VERTEX_SHADER, "PassThrough.Vertex");
  program.addShader( FRAGMENT_SHADER, "PassThrough.Fragment");
program.link();

program.bind();
  program.setUniform( "uModelViewProjMatrix", mvpMatrix);
  drawObject();
program.unbind();

program.destroy();

 ------------------------------------------------------------------------------
*/

#endif // RENDERER_PROGRAM_HPP_
