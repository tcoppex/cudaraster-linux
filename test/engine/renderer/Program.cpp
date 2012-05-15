#include "Program.hpp"

#include <cstdio>
#include <glsw/glsw.h>


namespace {
  
const unsigned int BUFFER_SIZE = 1024u;
char buffer[BUFFER_SIZE];

void printShaderLog(GLuint shader)
{  
  glGetShaderInfoLog( shader, BUFFER_SIZE, NULL, buffer);
  if (buffer[0] != '\0')  {
    fprintf( stderr, "%s\n", buffer);
  }
}
  
}


bool Program::addShader(const ShaderType shaderType, const char* name)
{
  assert( glswGetError() == 0 );  
  
  const GLchar *source = glswGetShader( name );

  if (NULL == source)
  {
    fprintf(stderr, "Error : shader \"%s\" not found, check your directory.\n", 
            name);
    return false;
  }

  GLuint shader = glCreateShader( shaderType );
  glShaderSource( shader, 1, (const GLchar**)&source, 0);
  glCompileShader( shader );

  GLint status = 0;
  glGetShaderiv( shader, GL_COMPILE_STATUS, &status);

  if (status != GL_TRUE)
  {
    fprintf(stderr, "%s compilation failed.\n", name);
    printShaderLog( shader );//    
    return false;    
  }
  glAttachShader( id(), shader);
  glDeleteShader(shader); //flagged for deletion
  
  
  return true;
}

bool Program::link()
{
  glLinkProgram( id() );

  GLint status = 0;
  glGetProgramiv( id(), GL_LINK_STATUS, &status);
  
  if (status != GL_TRUE)
  {
    fprintf( stderr, "Program linking failed.\n");
    return false;
  }
  
  return true;
}
