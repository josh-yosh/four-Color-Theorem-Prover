#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include "Point.h"


void newPointClick(GLFWwindow* window, int button, int action, std::vector<Point> &clickedPoints) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // Fetch the virtual screen size (e.g., 800x600) which matches xpos/ypos
        int click_width, click_height;
        glfwGetWindowSize(window, &click_width, &click_height);

        float ndcX = (2.0f * (float)xpos) / (float)click_width - 1.0f;
        float ndcY = 1.0f - (2.0f * (float)ypos) / (float)click_height;

        clickedPoints.push_back(Point(ndcX, ndcY));

        std::cout << "Clicked at Screen: (" << xpos << ", " << ypos 
                << ") -> NDC: (" << ndcX << ", " << ndcY << ")\n";
    }
}