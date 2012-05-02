CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

PROJECT( CudaRaster )

SET( EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin )

# TODO: create locale CMakeLists.txt
FILE( GLOB_RECURSE Headers src/*.hpp test/*.hpp )
FILE( GLOB_RECURSE Sources src/*.cpp test/*.cpp )

INCLUDE_DIRECTORIES( /usr/local/cuda/include )
# TODO: more specific include directory
INCLUDE_DIRECTORIES( src src/framework )

INCLUDE( FindCUDA )

SET( Libraries GL glut m)
LINK_LIBRARIES( "-L/usr/lib/nvidia-current -lcuda" )

# Static libraries to build
FILE( GLOB GLEW ${CMAKE_SOURCE_DIR}/thirdparty/GL/*.c extern/GL/*.h )
FILE( GLOB GLSW ${CMAKE_SOURCE_DIR}/thirdparty/glsw/*.c extern/glsw/*.h )
ADD_LIBRARY( glew ${GLEW} )
ADD_LIBRARY( glsw ${GLSW} )
INCLUDE_DIRECTORIES( thirdparty )


CUDA_ADD_EXECUTABLE( ${CMAKE_PROJECT_NAME} ${Sources} ${Headers} )

TARGET_LINK_LIBRARIES( ${CMAKE_PROJECT_NAME} ${Libraries} glew glsw)

