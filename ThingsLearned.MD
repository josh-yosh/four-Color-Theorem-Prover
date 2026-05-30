### 5/30/26
1. const on a function makes it so that that function does not mutate anything
2. in order for another function to see a struct funtion, always add to .h file. That's the one that is visible to other files.
3. int& pass: by copy
4. const int&: pass by copy and no mutations 
5. all mousefunctions shouhld be called in the callback, not main

BUG: glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)0);
1. told openGL that I was passing 2

**The Quick Rule of Thumb**
Keep double for any variables interacting with glfwGetCursorPos or local intermediate math to maintain precision.
Keep float inside the Point struct so that the memory layout perfectly aligns with your OpenGL hardware buffers.

### 5/24/26
1. you can't pass a member function of a class directly into a C-style function pointer that expects a regular, global function (or a static method).

### Rendering Process
1. Vertex shader - We inject shaders
2. Shape Assembly
3. Geometry Shader  - We inject shaders
4. Rasterization
5. Fragment Shader  - We inject shaders
6. Tests and blends

### VBO's and VAO's
VBO: Vertex buffer object -> stores all the vertex data
VAO: Vertex array object -> stores configure states for VBO

### How to make resizing render the correct frame
//makes resizing the window possible and sets the viewport to the new dimensions
glfwMakeContextCurrent(window);
glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);