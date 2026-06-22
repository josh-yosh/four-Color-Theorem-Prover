### To download packages
vcpkg install --triplet=arm64-osx-dynamic 

### To make cmake file, build on macos-arm
cmake --preset macos-arm        

### to build
cmake --build --preset macos-arm-debug

### to run
./build/macos-arm/Debug/my_executable
./build/macos-arm/Debug/my_executable 2>&1 | tee ~/Desktop/output.txt

### Building and running your application

When you're ready, start your application by running:
`docker compose up --build`.

