#ifndef RENDERER_VERTEXBUFFER_HPP
#define RENDERER_VERTEXBUFFER_HPP

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


///==================================
/// Structure to create & manage 
/// OpenGL buffers.
///==================================
class VertexBuffer
{
  protected:
    GLuint m_vao;
    GLuint m_vbo;    
    
    GLuint m_ibo;
    std::vector<GLuint> m_indices;
    
    std::vector<glm::vec3> m_positions;
    std::vector<glm::vec3> m_normals;
    std::vector<glm::vec2> m_texcoords;
    std::vector<glm::vec3> m_tangents;
    std::vector<glm::vec3> m_bitangents;
    
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

    /// Destroy the client side memory (CPU)
    virtual void cleanData();

    /// Set the VAO parameters & send data to the GPU
    virtual void complete(GLenum usage);
    
    void bind() const;        
    static void unbind();
    
    /// Enable vertex attribs arrays (for rendering)
    virtual void enable() const;
    
    /// Disable vertex attribs arrays
    virtual void disable() const;
    
    
    GLuint getVAO() const {return m_vao;}//
    GLuint getVBO() const {return m_vbo;}
    GLuint getIBO() const {return m_ibo;}
    
    std::vector<GLuint>& getIndices() {return m_indices;}
    std::vector<glm::vec3>& getPositions() {return m_positions;}
    std::vector<glm::vec3>& getNormals() {return m_normals;}
    std::vector<glm::vec2>& getTexcoords() {return m_texcoords;}
    std::vector<glm::vec3>& getTangents() {return m_tangents;}
    std::vector<glm::vec3>& getBitangents() {return m_bitangents;}
    
    GLintptr getOffset() const { return m_offset; }
    
    size_t getNumIndices() const { return m_numIndices; }//
    size_t getNumVertices() const { return m_numVertices; }//
    
    
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

#endif //RENDERER_VERTEXBUFFER_HPP
