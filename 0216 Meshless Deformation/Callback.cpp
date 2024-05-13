//mouse Callback 
#define GLEW_STATIC 
#include <GL/glew.h> 
#include <GLFW/glfw3.h> 

#include "Callback.h"

Camera camera;

bool move = true;
double Xpos, Ypos;

double getXPoint() {
    if (move)
        return Xpos;
    else
        return -1000;
}

double getYPoint() {
    if (move)
        return Ypos;
    else
        return -1000;
}

void cursorPosCallback(GLFWwindow* win, double xpos, double ypos) {
    static double lastX = 0;
    static double lastY = 0;


    if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_1)) {
        if (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT)) {
            move = false;
            double dx = xpos - lastX;
            double dy = ypos - lastY;
            int w, h;
            glfwGetWindowSize(win, &w, &h);
            camera.theta -= dx / w * PI;
            camera.phi -= dy / h * PI;

            if (camera.phi < -PI / 2 + 0.01) camera.phi = -PI / 2 + 0.01; // limit low angle
            else if (camera.phi > PI / 2 - 0.01) camera.phi = PI / 2 - 0.01; // limit high angle
        }
        else {
            move = true;
            Xpos = xpos;
            Ypos = ypos;
        }
        lastX = xpos;
        lastY = ypos;
    }
}

void scrollCallback(GLFWwindow* win, double xoffset, double yoffset) {
    camera.fovy += yoffset / 30;
}
