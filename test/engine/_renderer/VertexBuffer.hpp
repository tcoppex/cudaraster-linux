/**
 * 
 *    \file VertexBuffer.hpp  
 * 
 *    \todo # handle index buffer
 *          # handle interleaving
 */
 

#pragma once

#ifndef GLTYPE_VERTEXBUFFER_HPP
#define GLTYPE_VERTEXBUFFER_HPP

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>


enum VertexAttribLocation
{
  VATTRIB_POSITION = 0,
  VATTRIB_NORMAL,
  VATTRIB_TEXCOORD,
  VATTRIB_TANGENT,
  VATTRIB_BITANGENT,
  
  NUM_VATTRIB
};

class VertexBuffer
{
  protected:
    GLuint m_vao;
    GLuint m_vbo;    
    
    GLuint m_ibo;
    std::vector<GLuint> m_indices;
    
    std::vector<glm::vec3> m_position;
    std::vector<glm::vec3> m_normal;
    std::vector<glm::vec2> m_texcoord;
    std::vector<glm::vec3> m_tangent;
    std::vector<glm::vec3> m_bitangent;
    
    /// These are not reset after cleanData()
    size_t m_numIndices;
    size_t m_numVertices;
    
    GLsizeiptr m_positionSize;//
    GLsizeiptr m_normalSize;//
    GLsizeiptr m_texcoordSize;//
    GLsizeiptr m_tangentSize;//
    GLsizeiptr m_bitangentSize;//
    
    GLintptr m_offset;
    

  public:
    VertexBuffer() 
      : m_vao(0u), m_vbo(0u), m_ibo(0u),
        m_numIndices(0u), m_numVertices(0u),
        m_positionSize(0), m_normalSize(0), m_texcoordSize(0),
        m_tangentSize(0), m_bitangentSize(0),
        m_offset(0u)
    {}
                     
    virtual ~VertexBuffer() { destroy(); }
    
    virtual void generate();
    virtual void destroy();

    /** Destroy the client side memory (CPU) */
    virtual void cleanData();

    /** Set the VAO parameters & send data to the GPU */
    virtual void complete(GLenum usage);
    
    void bind() const;        
    static void unbind();
    
    /** Enable vertex attribs arrays (for rendering) */
    virtual void enable() const;
    
    /** Disable vertex attribs arrays */
    virtual void disable() const;
    
    
    GLuint getVAO() const {return m_vao;}//
    GLuint getVBO() const {return m_vbo;}
    GLuint getIBO() const {return m_ibo;}
    
    std::vector<GLuint>& getIndices() {return m_indices;} // todo: homogeneize names
    std::vector<glm::vec3>& getPosition() {return m_position;}
    std::vector<glm::vec3>& getNormal() {return m_normal;}
    std::vector<glm::vec2>& getTexcoord() {return m_texcoord;}
    std::vector<glm::vec3>& getTangent() {return m_tangent;}
    std::vector<glm::vec3>& getBitangent() {return m_bitangent;}
    
    GLintptr getOffset() const { return m_offset; }
    
    size_t getNumIndices() const { return m_numIndices; }//
    size_t getNumVertices() const { return m_numVertices; }//
    
    
    //---
    // not sure to keep it, but useful
    /// enable vertex arrays & draw the buffers given, in indexed mode if specified
    /// directly otherwise
    void draw() const
    {
      enable();      
      if (m_ibo) {
        glDrawElements( GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, 0);
      }else{
        glDrawArrays( GL_TRIANGLES, 0, 3u*m_numVertices);
      }
      disable();
    }
    //void draw( GLenum mode, GLint first, GLsizei count);
    //---
};


/*******************************************************************

  // HOW TO USE :

  VertexBuffer vertexBuffer;

  std::vector<glm::vec3> &pos = vertexBuffer.getPosition();
  std::vector<glm::vec3> &nor = vertexBuffer.getNormal();
  std::vector<glm::vec2> &tex = vertexBuffer.getTexcoord();

  // Update pos, nor, tex
  // ..

  // Generate buffer's id
  m_vertexBuffer.generate();  

  // Send data to the GPU
  m_vertexBuffer.complete( GL_STATIC_DRAW );

  // Remove data from the CPU [optional]
  m_vertexBuffer.cleanData();

********************************************************************/

#endif //GLTYPE_VERTEXBUFFER_HPP
