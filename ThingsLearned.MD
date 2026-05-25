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