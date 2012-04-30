/**
 * 
 *    \file VertexBuffer.cpp  
 * 
 */
 

#include "VertexBuffer.hpp"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

    
void VertexBuffer::generate()
{
  if (!m_vao) glGenVertexArrays( 1, &m_vao);
  if (!m_vbo) glGenBuffers( 1, &m_vbo);
}

void VertexBuffer::destroy()
{
  if (m_vao)
  {
    glDeleteVertexArrays( 1, &m_vao);
    m_vao = 0u;
  }
  
  if (m_vbo)
  {
    glDeleteBuffers( 1, &m_vbo);
    m_vbo = 0u;
  }
  
  if (m_ibo)
  {
    glDeleteBuffers( 1, &m_ibo);
    m_ibo = 0u;
  }
  
  cleanData();
}

void VertexBuffer::cleanData()
{
  m_indices.clear();
  m_position.clear();
  m_normal.clear();
  m_texcoord.clear();
  m_tangent.clear();
  m_bitangent.clear();
}

void VertexBuffer::complete(GLenum usage)
{
  generate();  
  
  m_positionSize = m_normalSize = m_texcoordSize = 0;
  m_tangentSize = m_bitangentSize = 0;
  
  if (!m_position.empty()) m_positionSize = m_position.size() * sizeof(glm::vec3);
  if (!m_normal.empty()) m_normalSize = m_normal.size() * sizeof(glm::vec3);
  if (!m_texcoord.empty()) m_texcoordSize  = m_texcoord.size() * sizeof(glm::vec2);  
  if (!m_tangent.empty()) m_tangentSize = m_tangent.size() * sizeof(glm::vec3);
  if (!m_bitangent.empty()) m_bitangentSize = m_bitangent.size() * sizeof(glm::vec3);
  
  GLsizeiptr bufferSize = m_positionSize + m_normalSize + m_texcoordSize +
                          m_tangentSize + m_bitangentSize;
  
  m_numVertices = m_position.size();//  
  
  
  bind();
  
  glBindBuffer( GL_ARRAY_BUFFER, m_vbo);
  {    
    glBufferData( GL_ARRAY_BUFFER, bufferSize, 0, usage);
      
    m_offset = 0;
    
    if (!m_position.empty())
    {
      glBufferSubData( GL_ARRAY_BUFFER, m_offset, m_positionSize, &m_position[0]);
      glVertexAttribPointer( VATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, (void*)(m_offset));
      m_offset += m_positionSize;
    }
    
    if (!m_normal.empty())
    {
      glBufferSubData( GL_ARRAY_BUFFER, m_offset, m_normalSize, &m_normal[0]);
      glVertexAttribPointer( VATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, (void*)(m_offset));
      m_offset += m_normalSize;
    }
    
    if (!m_texcoord.empty())
    {
      glBufferSubData( GL_ARRAY_BUFFER, m_offset, m_texcoordSize, &m_texcoord[0]);
      glVertexAttribPointer( VATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, (void*)(m_offset));  
      m_offset += m_texcoordSize;
    }
    
    if (!m_tangent.empty())
    {
      glBufferSubData( GL_ARRAY_BUFFER, m_offset, m_tangentSize, &m_tangent[0]);
      glVertexAttribPointer( VATTRIB_TANGENT, 3, GL_FLOAT, GL_FALSE, 0, (void*)(m_offset));
      m_offset += m_tangentSize;
    }
    
    if (!m_bitangent.empty())
    {
      glBufferSubData( GL_ARRAY_BUFFER, m_offset, m_bitangentSize, &m_bitangent[0]);
      glVertexAttribPointer( VATTRIB_BITANGENT, 3, GL_FLOAT, GL_FALSE, 0, (void*)(m_offset));
      m_offset += m_bitangentSize;
    }
  }  
  glBindBuffer( GL_ARRAY_BUFFER, 0u);
  
  
  // do it differently
  if (!m_indices.empty())
  {
    m_numIndices = m_indices.size();
    
    glGenBuffers( 1, &m_ibo);
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*m_numIndices, &m_indices[0], usage); 
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0u);
  }
  
  unbind();
}


void VertexBuffer::bind() const
{
  glBindVertexArray( m_vao );
}

void VertexBuffer::unbind()
{
  glBindVertexArray( 0u );
}

void VertexBuffer::enable() const
{  
  bind();
  
  if (m_positionSize != 0)  glEnableVertexAttribArray( VATTRIB_POSITION );
  if (m_normalSize != 0)    glEnableVertexAttribArray( VATTRIB_NORMAL );
  if (m_texcoordSize != 0)  glEnableVertexAttribArray( VATTRIB_TEXCOORD );
  if (m_tangentSize != 0)  glEnableVertexAttribArray( VATTRIB_TANGENT );
  if (m_bitangentSize != 0)  glEnableVertexAttribArray( VATTRIB_BITANGENT );
  
  if (m_ibo) {
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_ibo);
  }
}

void VertexBuffer::disable() const
{
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0u);
  
 	glDisableVertexAttribArray( VATTRIB_POSITION );
  glDisableVertexAttribArray( VATTRIB_NORMAL );
  glDisableVertexAttribArray( VATTRIB_TEXCOORD );
  glDisableVertexAttribArray( VATTRIB_TANGENT );
  glDisableVertexAttribArray( VATTRIB_BITANGENT );
  
  unbind();
}
