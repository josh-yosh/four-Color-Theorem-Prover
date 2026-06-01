#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <optional>
#include <set>
#include "Point.h"
#include "buttonControls.h"
#include "lineAndPointLogic.h"
using namespace std;

int windowWidth = 800;
int windowHeight = 600;

// Keep these global so your render loop can see them
vector<Point> clickedPoints;
set<Point> currentConnection; 
set<set<Point>> allConnections;
bool isConnecting = false; // Track if we are in the process of connecting points
unordered_map<set<set<Point>>, optional<Point>, NestedSetPointHash> intersectionPoints; // Store intersection points


void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
}

//for resizing the window
void window_size_callback(GLFWwindow* window, int width, int height) {
    windowWidth = width;
    windowHeight = height;
}

// Mouse button callback function
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if(ConnectingPoints(window, button, action, clickedPoints, currentConnection, allConnections, isConnecting)) {
        cout << isConnecting << endl;
    } else if (!isConnecting){
        // If not connecting points, check for new point creation
        newPointClick(window, button, action, clickedPoints, allConnections, intersectionPoints);
    }

}


struct Engine {
    
    const char *fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
        "}\n\0";

    const char *vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec2 aPos;\n"
        "void main()\n"
        "{\n"
        " gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
        "}\0";

    int failedGlfwWindow(GLFWwindow* window) {
        if (window == NULL){
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return -1;
        }
        return 0;
    }

    int failedGladLoader() {
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
            std::cout << "Failed to initialize GLAD" << std::endl;
            return -1;
        }
        return 0;
    }

    void cleanup(unsigned int VAO, unsigned int VBO) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);

        glfwTerminate();
    }

    void vertexShaderErrorCheck(unsigned int vertexShader) {
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    }

    void vertexShaderSetup(unsigned int &vertexShader) {
        vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        vertexShaderErrorCheck(vertexShader);
    }

    void fragmentShaderErrorCheck(unsigned int fragmentShader) {
        int success;
        char infoLog[512];
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    }

    void fragmentShaderSetup(unsigned int &fragmentShader) {
        fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        fragmentShaderErrorCheck(fragmentShader);
    }

    void shaderProgramErrorCheck(unsigned int shaderProgram) {
        int success;
        char infoLog[512];
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
    }

    void linkingVAOandVBO(unsigned int &VAO, unsigned int &VBO) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
    }

    void glfwHintInit() {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        #ifdef __APPLE__
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        #endif
    }

    unsigned int createShaderProgram(unsigned int vertexShader, unsigned int fragmentShader) {
        unsigned int program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);
        shaderProgramErrorCheck(program);

        // Once linked into the program, we can delete the individual shader objects
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        
        return program;
    }

    void run() {
        glfwInit();
        glfwHintInit();

        GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
        if (failedGlfwWindow(window) == -1) return;
        
        //makes resizing the window possible and sets the viewport to the new dimensions
        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback); // Register the mouse callback
        glfwSetWindowSizeCallback(window, window_size_callback); // Register the window size callback
        glfwGetWindowSize(window, &windowWidth, &windowHeight); // Query the actual physical pixels for the initial OpenGL canvas

        if (failedGladLoader() == -1) return;

        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        glViewport(0, 0, fbWidth, fbHeight);

        //shader setup
        unsigned int vertexShader, fragmentShader;
        vertexShaderSetup(vertexShader); // Compile vertex shader
        fragmentShaderSetup(fragmentShader); // Compile fragment shader
        unsigned int shaderProgram = createShaderProgram(vertexShader, fragmentShader); // Link shaders into a program

        //creating VAO and VBO for rendering points
        unsigned int VAO, VBO;
        linkingVAOandVBO(VAO, VBO);

        // Position attribute
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)0);
        glEnableVertexAttribArray(0);

        // Make points larger so they are easy to see
        glPointSize(10.0f);
        glLineWidth(2.0f); // Set line width for connections

        // render loop
        while(!glfwWindowShouldClose(window)){
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glUseProgram(shaderProgram);

            // If we have points, orphan/sub the buffer with new data
            bool hasPoints = !clickedPoints.empty();
            if (hasPoints) {
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                // Upload the dynamic vector data to the GPU
                glBufferData(GL_ARRAY_BUFFER, clickedPoints.size() * sizeof(Point), &clickedPoints[0], GL_DYNAMIC_DRAW);

                // Bind shaders here if youc are using custom ones
                glBindVertexArray(VAO);
                // Draw all the points we've collected so far
                glDrawArrays(GL_POINTS, 0, clickedPoints.size());
            }

            //if we have an active connection, draw a line between the two points
            if(isConnecting){
                double ndcX, ndcY;
                getCursorPositionInNDC(window, ndcX, ndcY);
                vector<Point> currentLine = {*(currentConnection.begin()), Point(ndcX, ndcY)};

                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                // Upload the dynamic vector data to the GPU
                glBufferData(GL_ARRAY_BUFFER, currentLine.size() * sizeof(Point), &currentLine[0], GL_DYNAMIC_DRAW);

                // Bind shaders here if youc are using custom ones
                glBindVertexArray(VAO);
                // Draw all the points we've collected so far
                glDrawArrays(GL_LINES, 0, currentLine.size());

            }

            bool hasConnections = !allConnections.empty();
            if(hasConnections){
                // 1. Flatten all connections into one continuous vector first
                vector<Point> linesToDraw;
                for (const auto& connection : allConnections) {
                    for (const auto& point : connection) {
                        linesToDraw.push_back(point);
                    }
                }

                // 2. Only perform GPU operations if there is actually data to draw
                if (!linesToDraw.empty()) {
                    // Bind everything ONCE
                    glBindVertexArray(VAO);
                    glBindBuffer(GL_ARRAY_BUFFER, VBO);
                    
                    // Upload ALL line data at once
                    glBufferData(GL_ARRAY_BUFFER, linesToDraw.size() * sizeof(Point), &linesToDraw[0], GL_DYNAMIC_DRAW);

                    // Draw ALL lines with a single GPU command
                    glDrawArrays(GL_LINES, 0, linesToDraw.size());
                }

                for (const auto& connection : allConnections) {
                    for (const auto& connection2 : allConnections) {
                        bool sameConnection = (connection == connection2);
                        bool alreadyChecked = (intersectionPoints.find({connection, connection2}) != intersectionPoints.end());
                        if (!sameConnection && !alreadyChecked) {
                            getIntersectionPoint(connection, connection2, intersectionPoints, clickedPoints, allConnections);
                        }
                    }
                }
            }

            

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        // Cleanup
        glDeleteProgram(shaderProgram);
        cleanup(VAO, VBO);
    }

};


int main() {
    Engine engine;

    engine.run();
    return 0;
}

