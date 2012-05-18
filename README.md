cudaraster-linux
================

About
---------------------------------

Simple Linux port of cudaraster, the Nvidia GPU rasterizer.
see http://code.google.com/p/cudaraster/

It provides a simple test application which display a cube with a simple
passthrough shader.


Despite the works remaining (mainly to improve documentation) there is no main 
updates planned in the distant future.


Note: the original code is under the Apache license.


Structure
---------------------------------

* src/

Cudaraster, with part of the original framework.

* test/

Sample application displaying a simple cube with depth-buffer enabled.
OpenGL GLSL & CudaRaster 'PassThrough' shaders are provided.
Pressing F1 switches between the OpenGL and the CudaRaster render mode,
pressing F2 display statistics for the CudaRaster mode.
A free camera is also provided.
Note: the Shaders.* files are from the original implementation and are not used yet;
they described a more complexe shaders using texture mapping and Phong lighting
computation.

* thirdparty/

Libraries used by the sample application.


System Requirements
---------------------------------

* Linux (32 or 64 bits).

* At least 1GB of system memory.

* NVIDIA CUDA-compatible GPU with compute capability 2.0 and at least 512
  megabytes of RAM. GeForce GTX 480 is recommended.
  
* NVIDIA CUDA 4.0 or later with API driver.



Building
---------------------------------
```bash
cd cudaraster_linux
mkdir build
cd build
cmake ..
make
```

Running
---------------------------------

```bash
../bin/CudaRaster
```


Future works
---------------------------------

* port to CUDA 4.2.

* write documentation.

* add more samples.

* minimize dependencies over cudaraster framework.

* port to OpenCL ? (someday.. maybe)

