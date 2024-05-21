//mouse Callback 
#define GLEW_STATIC 
#include <GL/glew.h> 
#include <GLFW/glfw3.h> 

#include "Callback.h"
#include <tuple>


Camera camera;

struct pos_t {
    float x, y;
};

double Xpos, Ypos;

glm::vec3 cursorPt3;
float cursorD;
float depth;
extern bool check;

inline glm::vec2 getFramebufferCursorPos(GLFWwindow* window, float windowH) {
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	pos_t pt = { static_cast<float>(xpos), static_cast<float>(ypos) };
	pt.y = windowH - pt.y;

	int fbWidth, fbHeight;
	glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
	int winWidth, winHeight;
	glfwGetWindowSize(window, &winWidth, &winHeight);
	float ratio = static_cast<float>(fbWidth) / static_cast<float>(winWidth);

	return glm::vec2{ pt.x * ratio, pt.y * ratio };
}

//Normalized Coordinate와 depth 값 
inline std::tuple<glm::vec3, float> getNCCursorPos(GLFWwindow* window, const glm::ivec2& sz, float windowH) {
	glm::vec2 pt = getFramebufferCursorPos(window, windowH);
	float d = 0;
	glReadPixels(static_cast<int>(pt.x), static_cast<int>(pt.y), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &d);
	glm::vec3 pt3 = glm::vec3(pt.x / (static_cast<float>(sz.x)), pt.y / (static_cast<float>(sz.y)), d) * 2.0f - 1.0f;
	return std::make_tuple(pt3, d);
}

inline glm::vec3 getNCCursorPos(GLFWwindow* window, float d, const glm::ivec2& sz, float windowH) {
	glm::vec2 pt = getFramebufferCursorPos(window, windowH);
	glm::vec3 pt3 = glm::vec3(pt.x / (static_cast<float>(sz.x)), pt.y / (static_cast<float>(sz.y)), d) * 2.0f - 1.0f;
	return pt3;
}

//Normalized Coordinate	를 WorldCoord로 
inline std::tuple<glm::vec3, float> get3DCursorPos(GLFWwindow* window, const glm::ivec2& sz, const glm::mat4& vp, float windowH) {
	auto [ptNC, d] = getNCCursorPos(window, sz, windowH);
	glm::vec4 ptWC = glm::inverse(vp) * glm::vec4(ptNC, 1.0f);
	return std::make_tuple(glm::vec3(ptWC) / ptWC.w, d);
}

inline glm::vec3 get3DCursorPos(GLFWwindow* window, float d, const glm::ivec2& sz, const glm::mat4& vp, float windowH) {
	glm::vec3 ptNC = getNCCursorPos(window, d, sz, windowH);
	glm::vec4 ptWC = glm::inverse(vp) * glm::vec4(ptNC, 1.0f);
	return glm::vec3(ptWC) / ptWC.w;
}

extern glm::vec3 movePoint;
extern bool push;
extern glm::vec3 cursorDifference;



void cursorPosCallback(GLFWwindow* win, double xpos, double ypos) {
    static double lastX = 0;
    static double lastY = 0;


    int w, h;
    glfwGetWindowSize(win, &w, &h);

    if (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT)) {
        if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_1)) {
            
            double dx = xpos - lastX;
            double dy = ypos - lastY;
            camera.theta -= dx / w * PI;
            camera.phi -= dy / h * PI;

            if (camera.phi < -PI / 2 + 0.01) camera.phi = -PI / 2 + 0.01; // limit low angle
            else if (camera.phi > PI / 2 - 0.01) camera.phi = PI / 2 - 0.01; // limit high angle
        }
    }
    else {
        glm::mat4 view = camera.getViewMat();
        glm::mat4 projection = camera.getProjMat(w, h);
        glm::mat4 vp = projection * view;

        glm::ivec2 sz = glm::ivec2(w, h);

        std::tie(cursorPt3, cursorD) = get3DCursorPos(win, sz, vp, h);

        if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
            //DRAG + PUSH

            //depth가 유의미한 값일 때 가장 가까운 vertex를 찾아준다. 
            if (cursorD != 1.0f && push == false && cursorPt3.y < 800 && cursorPt3.y > -800) {
                movePoint = cursorPt3;
                depth = cursorD;
                push = true;
            }

        }
        else if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE && push == true) {
            //Release
            //vertex의 위치를 바꾼다. 
            cursorDifference = get3DCursorPos(win, depth, sz, vp, h);
            push = false;
            check = true;
        }
        else {
            //MOVE - 현 위치 get
//            printf("% f % f % f % f\n", cursorPt3.x, cursorPt3.y, cursorPt3.z, cursorD);
            //TODO
        }
    }
    lastX = xpos;
    lastY = ypos;
}



void scrollCallback(GLFWwindow* win, double xoffset, double yoffset) {
    camera.fovy += yoffset / 30;
}


