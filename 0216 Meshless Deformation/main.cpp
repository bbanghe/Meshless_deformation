#include <iostream>
#define GLEW_STATIC 
#include <GL/glew.h> 
#include <GLFW/glfw3.h> 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <glm/gtx/transform.hpp>
#include <glm/gtx/norm.hpp>
#include <vector>
#include "toys.h"

#include "callback.h"
#include "mesh.h"
#include "loadMesh.h"
#include "plane.h"
//#include "loadShadow.h"

#pragma comment (lib, "opengl32")
#pragma comment (lib, "glfw3")
#pragma comment (lib, "glew32s")

void render(GLFWwindow*);
void init();

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 8);
    GLFWwindow* window = glfwCreateWindow(1440, 900, "Test", 0, 0);
    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glewInit();
    init();
    glfwSwapInterval(1);
    while (!glfwWindowShouldClose(window)) {
        render(window);
        glfwPollEvents();
    }
}

using namespace glm;
using namespace std;

Program program;
Program shadowProgram;
vector<Mesh> meshes;

void init() {
    meshes = loadMesh("duck.dae");
}

Plane plane(contact_point, normal_vector);



void render(GLFWwindow* window) {

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    glViewport(0, 0, w, h);
    glClearColor(0.27451f, 0.509804f, 0.705882f, 1); //배경색    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glUseProgram(program.programID);

    GLuint ModelMatLocation = glGetUniformLocation(program.programID, "modelMat");
    glUniformMatrix4fv(ModelMatLocation, 1, 0, value_ptr(mat4(1)));

    GLuint viewMatLocation = glGetUniformLocation(program.programID, "viewMat");
    glUniformMatrix4fv(viewMatLocation, 1, 0, value_ptr(camera.getViewMat()));

    GLuint projMatLocation = glGetUniformLocation(program.programID, "projMat");
    glUniformMatrix4fv(projMatLocation, 1, 0, value_ptr(camera.getProjMat(w, h)));

    plane.render();



    for (Mesh& mesh : meshes) {
        for (int i = 0; i < 10; i++) { //반복 -> 빨리수렴 => 출렁거리는 현상 감소
            mesh.update(0.0166f / 10);
        }
        mesh.render();

    }

    glfwSwapBuffers(window);
}
