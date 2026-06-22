#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <optional>
#include <set>
#include "Point.h"
#include "Edge.h"
#include "buttonControls.h"
#include "lineAndPointLogic.h"
#include "AtomicEnclosure.h"
#include "colorLogic.h"
#include "mapColoring.h"
#include "triangulation.h"
using namespace std;

int windowWidth = 800;
int windowHeight = 600;

// Keep these global so your render loop can see them
set<Point> clickedPoints;
vector<Point> pointsToRender;
set<Point> currentConnection; 
set<Edge> allEdges;
set<Point> intersectionPoints; // Store intersection points
set<AtomicEnclosure> allAtomicEnclosures;
map<AtomicEnclosure, int> enclosureColors; // Maps each face to its color index (0..3)
unordered_map<Point, set<Edge>> pointToEdgeMap;
bool isConnecting = false; // Track if we are in the process of connecting points


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
    if(ConnectingPoints(window, button, action, clickedPoints, currentConnection, allEdges, intersectionPoints, isConnecting, pointToEdgeMap, allAtomicEnclosures)) {
        // Print the most recently added edge (last in the set)
        if (!allEdges.empty()) {
            const Edge& newest = *allEdges.rbegin();
            cout << "[Edge Added] ("
                 << newest.p1().x << ", " << newest.p1().y << ") -> ("
                 << newest.p2().x << ", " << newest.p2().y << ")\n";
        }

        // Print all current edges
        cout << "[All Edges — " << allEdges.size() << " total]\n";
        int i = 1;
        for (const Edge& e : allEdges) {
            cout << "  " << i++ << ". ("
                 << e.p1().x << ", " << e.p1().y << ") -> ("
                 << e.p2().x << ", " << e.p2().y << ")\n";
        }

        allAtomicEnclosures = findAtomicEnclosures(allEdges);

        // A new face may have been added, so recompute the 4-coloring.
        if (colorMap(allAtomicEnclosures, enclosureColors)) {
            cout << "[Coloring] Found a valid 4-coloring for "
                 << allAtomicEnclosures.size() << " face(s)\n";
        } else {
            cout << "[Coloring] Failed to 4-color the map\n";
        }
    } else if (!isConnecting){
        // If not connecting points, check for new point creation
        newPointClick(window, button, action, clickedPoints, allEdges);
    }
}


struct Engine {
    
    const char *fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "uniform vec4 uColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = uColor;\n"
        "}\n\0";

    const char *vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec2 aPos;\n"
        "void main()\n"
        "{\n"
        " gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
        "}\0";

    const char *geometryShaderSource = "#version 330 core\n"
        "layout (lines) in;\n"
        "layout (triangle_strip, max_vertices = 4) out;\n"
        "uniform vec2 uViewport;\n"
        "uniform float uLineWidth;\n"
        "void main() {\n"
        "    vec2 p0 = gl_in[0].gl_Position.xy;\n"
        "    vec2 p1 = gl_in[1].gl_Position.xy;\n"
        "    vec2 dir = normalize((p1 - p0) * uViewport * 0.5);\n"
        "    vec2 normal = vec2(-dir.y, dir.x);\n"
        "    vec2 offset = normal * uLineWidth / uViewport;\n"
        "    gl_Position = vec4(p0 + offset, 0.0, 1.0); EmitVertex();\n"
        "    gl_Position = vec4(p0 - offset, 0.0, 1.0); EmitVertex();\n"
        "    gl_Position = vec4(p1 + offset, 0.0, 1.0); EmitVertex();\n"
        "    gl_Position = vec4(p1 - offset, 0.0, 1.0); EmitVertex();\n"
        "    EndPrimitive();\n"
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

    void geometryShaderSetup(unsigned int &geometryShader) {
        geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geometryShader, 1, &geometryShaderSource, NULL);
        glCompileShader(geometryShader);
        int success;
        char infoLog[512];
        glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(geometryShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
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

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return program;
    }

    unsigned int createLineShaderProgram(unsigned int vertexShader, unsigned int geometryShader, unsigned int fragmentShader) {
        unsigned int program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, geometryShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);
        shaderProgramErrorCheck(program);

        glDeleteShader(vertexShader);
        glDeleteShader(geometryShader);
        glDeleteShader(fragmentShader);

        return program;
    }

    void run() {
        glfwInit();
        glfwHintInit();

        GLFWwindow* window = glfwCreateWindow(800, 600, "4 color Theorem Map Creator", NULL, NULL);
        if (failedGlfwWindow(window) == -1) return;
        
        //makes resizing the window possible and sets the viewport to the new dimensions
        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback); // Register the mouse callback
        glfwSetWindowSizeCallback(window, window_size_callback); // Register the window size callback
        glfwGetWindowSize(window, &windowWidth, &windowHeight); // Query the actual physical pixels for the initial OpenGL canvas

        if (failedGladLoader() == -1) return;

        // ImGui setup
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");
        ImGui::StyleColorsDark();

        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        glViewport(0, 0, fbWidth, fbHeight);

        //shader setup
        unsigned int vertexShader, fragmentShader;
        vertexShaderSetup(vertexShader);
        fragmentShaderSetup(fragmentShader);
        unsigned int shaderProgram = createShaderProgram(vertexShader, fragmentShader);

        unsigned int lineVertShader, lineGeomShader, lineFragShader;
        vertexShaderSetup(lineVertShader);
        geometryShaderSetup(lineGeomShader);
        fragmentShaderSetup(lineFragShader);
        unsigned int lineShaderProgram = createLineShaderProgram(lineVertShader, lineGeomShader, lineFragShader);

        //creating VAO and VBO for rendering points
        unsigned int VAO, VBO;
        linkingVAOandVBO(VAO, VBO);

        // Position attribute
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)0);
        glEnableVertexAttribArray(0);

        glPointSize(10.0f);

        // render loop
        while(!glfwWindowShouldClose(window)){
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glUseProgram(shaderProgram);

            // ImGui new frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            GLint colorLoc = glGetUniformLocation(shaderProgram, "uColor");

            bool hasAtomicEnclosures = !allAtomicEnclosures.empty();

            if(hasAtomicEnclosures){
                // The 4 colors of the map, indexed by the color assigned in enclosureColors.
                static const float palette[NUM_COLORS][4] = {
                    {0.90f, 0.30f, 0.30f, 1.0f}, // 0: red
                    {0.30f, 0.60f, 0.90f, 1.0f}, // 1: blue
                    {0.40f, 0.80f, 0.40f, 1.0f}, // 2: green
                    {0.95f, 0.85f, 0.35f, 1.0f}, // 3: yellow
                };

                // Bucket every face's triangles by its assigned color so we can
                // upload and draw all faces of one color in a single GPU call.
                vector<Point> trianglesByColor[NUM_COLORS];
                for (const AtomicEnclosure& atomicEnclosure : allAtomicEnclosures) {
                    auto colorIt = enclosureColors.find(atomicEnclosure);
                    if (colorIt == enclosureColors.end()) continue;
                    int color = colorIt->second;

                    vector<Point> boundary = orderEnclosureBoundary(atomicEnclosure);
                    vector<Point> triangles = earClipTriangulate(boundary);
                    if (triangles.empty()) continue;

                    trianglesByColor[color].insert(trianglesByColor[color].end(),
                                                   triangles.begin(), triangles.end());
                }

                // One draw call per color (4 total) instead of one per face.
                glBindVertexArray(VAO);
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                for (int color = 0; color < NUM_COLORS; ++color) {
                    const vector<Point>& triangles = trianglesByColor[color];
                    if (triangles.empty()) continue;

                    glUniform4f(colorLoc, palette[color][0], palette[color][1],
                                          palette[color][2], palette[color][3]);
                    glBufferData(GL_ARRAY_BUFFER, triangles.size() * sizeof(Point), &triangles[0], GL_DYNAMIC_DRAW);
                    glDrawArrays(GL_TRIANGLES, 0, triangles.size());
                }
            }

            glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
            // If we have points, orphan/sub the buffer with new data
            bool hasPoints = !clickedPoints.empty();
            if (hasPoints) {
                pointsToRender.assign(clickedPoints.begin(), clickedPoints.end()); // Convert set to vector for rendering

                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                // Upload the dynamic vector data to the GPU
                glBufferData(GL_ARRAY_BUFFER, pointsToRender.size() * sizeof(Point), &pointsToRender[0], GL_DYNAMIC_DRAW);

                // Bind shaders here if youc are using custom ones
                glBindVertexArray(VAO);
                // Draw all the points we've collected so far
                glDrawArrays(GL_POINTS, 0, pointsToRender.size());
            }

            int fbW, fbH;
            glfwGetFramebufferSize(window, &fbW, &fbH);

            //if we have an active connection, draw a line between the two points
            if(isConnecting){
                double ndcX, ndcY;
                getCursorPositionInNDC(window, ndcX, ndcY);
                vector<Point> currentLine = {*(currentConnection.begin()), Point(ndcX, ndcY)};

                glUseProgram(lineShaderProgram);
                glUniform4f(glGetUniformLocation(lineShaderProgram, "uColor"), 1.0f, 1.0f, 1.0f, 1.0f);
                glUniform2f(glGetUniformLocation(lineShaderProgram, "uViewport"), (float)fbW, (float)fbH);
                glUniform1f(glGetUniformLocation(lineShaderProgram, "uLineWidth"), 3.0f);

                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, currentLine.size() * sizeof(Point), &currentLine[0], GL_DYNAMIC_DRAW);
                glBindVertexArray(VAO);
                glDrawArrays(GL_LINES, 0, currentLine.size());

                glUseProgram(shaderProgram);
            }

            bool hasConnections = !allEdges.empty();
            if(hasConnections){
                vector<Point> linesToDraw;
                for (const auto& edge : allEdges) {
                    linesToDraw.push_back(edge.p1());
                    linesToDraw.push_back(edge.p2());
                }

                if (!linesToDraw.empty()) {
                    glUseProgram(lineShaderProgram);
                    glUniform4f(glGetUniformLocation(lineShaderProgram, "uColor"), 1.0f, 1.0f, 1.0f, 1.0f);
                    glUniform2f(glGetUniformLocation(lineShaderProgram, "uViewport"), (float)fbW, (float)fbH);
                    glUniform1f(glGetUniformLocation(lineShaderProgram, "uLineWidth"), 3.0f);

                    glBindVertexArray(VAO);
                    glBindBuffer(GL_ARRAY_BUFFER, VBO);
                    glBufferData(GL_ARRAY_BUFFER, linesToDraw.size() * sizeof(Point), &linesToDraw[0], GL_DYNAMIC_DRAW);
                    glDrawArrays(GL_LINES, 0, linesToDraw.size());

                    glUseProgram(shaderProgram);
                }
            }

            // Mouse coordinate overlay (top-right)
            double ndcX, ndcY;
            getCursorPositionInNDC(window, ndcX, ndcY);
            ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 10, 10), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
            ImGui::SetNextWindowBgAlpha(0.6f);
            ImGui::Begin("##coords", nullptr,
                ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoFocusOnAppearing |
                ImGuiWindowFlags_NoNav |
                ImGuiWindowFlags_NoMove);
            ImGui::Text("NDC: (%.3f, %.3f)", ndcX, ndcY);
            ImGui::End();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glDeleteProgram(shaderProgram);
        glDeleteProgram(lineShaderProgram);
        cleanup(VAO, VBO);
    }

};


int main() {
    Engine engine;

    engine.run();
    return 0;
}

