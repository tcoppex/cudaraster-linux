#ifndef DATA_HPP_
#define DATA_HPP_

#include <vector>

///================================
/// Basic geometry definition use
/// to create OpenGL & CudaRaster
/// specific buffer.
/// see SceneCR, SceneGL
///================================
struct Data
{
  std::vector<float> positions;
  std::vector<unsigned int> indices;
  
  unsigned int numVertices;
  unsigned int numIndices;
  
  void clear()
  {
    positions.clear();
    indices.clear();
  }
};

#endif //DATA_HPP_
