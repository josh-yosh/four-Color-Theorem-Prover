#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>


void framebuffer_size_callback(GLFWwindow* window, int width, int height){
glViewport(0, 0, width, height);
}

struct Point {
    float x, y;
};

struct Engine {
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    int failedGlfwWindow(GLFWwindow* window) {
        if (window == NULL){
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return -1;
        }
    }

    void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            std::cout << "Mouse clicked at: (" << xpos << ", " << ypos << ")" << std::endl;
        }
    }

    void run() {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

        GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
        if (failedGlfwWindow(window) == -1) return;

        glfwMakeContextCurrent(window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
            std::cout << "Failed to initialize GLAD" << std::endl;
            return;
        }

        // Register the mouse callback
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        
        // for resizing the window
        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        // render loop
        while(!glfwWindowShouldClose(window)){
            // input
            // processInput(window);
            // rendering commands here
            // ...
            // check and call events and swap the buffers
            glfwPollEvents();
            glfwSwapBuffers(window);
        }

        glfwTerminate();
    }
};


int main() {
    Engine engine;

    engine.run();
    return 0;
}

