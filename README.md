# Collision Detection with CUDA



### Usage

#### Compile

1. Install Visual Studio 2019 or newer version
2. Install CUDA (which will automatically install supporting libraries for VS)
3. Install OpenGL, GLFW，GLAD & GLM
4. A `mylib` directory is placed in the `src` directory，Open `src\ProjectGL\ProjectGL.sln`，and import the relevant header files and libraries in `mylibs`directory to the Visual Studio settings（You can refer to this OpenGL [setting manual](https://learnopengl-cn.github.io/01%20Getting%20started/02%20Creating%20a%20window/#_4)）

5. Choose **realease, x86** mode

#### Versions

1. Windows10 + Visual Studio 2019

2. CUDA 11.1

3. GLM 0.9.9.8

   **CUDA & GLM version is important as of the compatibility**

#### Executable

`bin\CollisionDetection.exe`

#### Use CPU version

1. Delete line 86, `detector.h`
2. Alter the 149 line of `detector.h` to be `updateBallAttrCpu()`

### Modules and Logistics

#### Modules

1. Rendering module
   - `shader.h` is in charge the compilation of shaders，`camera.h` controls the view angle.
   - `main.cpp` includes the basic rendering logic and codes for interaction
   - `src\shader` includes Phong shader
2. 检测模块
   - `octree.h` implements a cubical octree class with creation, insertion, deletion and nearest-neighborhood search functionality. It will give the potential pairs of collision objects.
   - `detector.h` implements the creation of balls, the update of their velocities and positions, and send the potential collision pairs to the CUDA program.
   - `collide.cu`implements the collision detection functionality with CUDA and return the velocities afterwards.
3. 其他模块
   - `global.h`stores the global variables and settings.
   - `sphere.h` is for the creation of sphere points.

#### Logistics

1. Generate the initial information of balls; e.g. position, velocity, mass and collision coefficient and initialize the octree.
2. Update the positions of the balls with their velocities.
3. Render the balls and the walls.
4. Sythesize the data of CPU and GPU
5. According to the search result of the octree, find the potential collision pairs.
6. Use the CUDA program to perform collision detection and return the updated velocities.
7. Return the updated velocities to the CPU

### Video Demo

In `video` directory. Includes a faster one and a slower one.