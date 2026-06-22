# Four Color Theorem Prover

An interactive GUI application for visualizing and proving the four color theorem using OpenGL 3.3, GLFW, GLAD, and ImGui.

## Why Docker Doesn't Work Here

This app uses **OpenGL 3.3 Core Profile** for rendering. On macOS, Docker Desktop runs containers inside a Linux VM and relies on XQuartz for display forwarding. XQuartz only supports **OpenGL 2.1** over GLX — making it incompatible with this application.

Docker works well for headless/server applications but is not practical for OpenGL 3.3+ GUI apps on macOS without a complex VNC workaround.

## Building and Running Natively (macOS)

### Prerequisites

- [vcpkg](https://github.com/microsoft/vcpkg)
- CMake
- Xcode command line tools: `xcode-select --install`

### Steps

```bash
# Install dependencies
vcpkg install --triplet=arm64-osx-dynamic

# Configure
cmake --preset macos-arm

# Build
cmake --build --preset macos-arm-debug

# Run
./build/macos-arm/Debug/my_executable
```

Example:

https://github.com/user-attachments/assets/c2f027f1-4e11-4d77-881e-da9b72af422f


