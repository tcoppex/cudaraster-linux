
#ifndef MESHGL_HPP
#define MESHGL_HPP

#include <cstdlib>
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>

class MeshGL
{
  private:
    enum Attrib {
      VATTRIB_POSITION = 0,      
      NUM_ATTRIB
    };
  
  
  private:   
    // ++ OpenGL ++
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ibo;

    // ++ CPU buffers ++
    std::vector<glm::vec3> m_positions;
    std::vector<GLuint> m_indices;
    
    size_t m_numTriangles;
    size_t m_numIndices;

  public:
    MeshGL() 
        : m_vao(0u), 
          m_vbo(0u), 
          m_ibo(0u),
          m_numTriangles(0), 
          m_numIndices(0)
    {}
    
    ~MeshGL() { destroy(); }
    
    void generate();
    void destroy();


    // Set the VAO parameters & send data to the GPU
    void setDatas( void* vertices, size_t numVertices,
                   void* indices, size_t numIndices,
                   GLenum usage);
    
    
    GLuint getVBO() const {return m_vbo;}
    GLuint getIBO() const {return m_ibo;}
        
    FW::Buffer& getInVertices() { return m_inVertices; }
    FW::Buffer& getOutVertices() { return m_outVertices; }
    FW::Buffer& getIndices() { return m_indices; }
        
    size_t getNumIndices()   const { return m_numIndices; }//
    size_t getNumTriangles() const { return m_numTriangles; }//
    size_t getNumVertices()  const { return m_numTriangles*3u; }//
    
    void draw() const;
    
    
  private:
    // Enable vertex attribs arrays (for rendering)
    void enable() const;
    
    // Disable vertex attribs arrays 
    void disable() const;
};

namespace meshUtils {
  
void setup_cubeMesh(Mesh& mesh);

}

#endif //MESH_HPP
