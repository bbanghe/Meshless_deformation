#include <iostream>
#define GLEW_STATIC 
#include <GL/glew.h> 
#include <GLFW/glfw3.h> 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <glm/gtx/transform.hpp>
#include <glm/gtx/norm.hpp>
#include <vector>

#include "callback.h"
#include "mesh.h"
#include "loadMesh.h"
#include "plane.h"
#include "loadShadow.h"

#pragma comment (lib, "opengl32")
#pragma comment (lib, "glfw3")
#pragma comment (lib, "glew32s")

void render(GLFWwindow*);
void init();
void keyFunc(GLFWwindow*, int key, int code, int action, int mods);

double xpos, ypos;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 8);
    GLFWwindow* window = glfwCreateWindow(1440, 900, "Test", 0, 0);
    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);

    glfwSetKeyCallback(window, keyFunc);
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
bool animating = false;
bool push = false;
bool release = false;
bool moveObj = false;


void init() {
    animating = false;
    push = false;
    release = false;
    moveObj = false;

    meshes = loadMesh("duck.dae");
    shadowProgram.loadShaders("shadow.vert", "shadow.frag");
}

void keyFunc(GLFWwindow*, int key, int code, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == ' ') {
            animating = !animating;
        }
        if (key == '1') {
            height = 600;            
            init();
        }
        if (key == '2') {
            height = -600;
            init();
        }
        if (key == 'R' || key == 'r') {
            release = true;
        }
    }
}

Plane plane(contact_point, normal_vector);

glm::vec3 lightPosition = glm::vec3(2000, 3000, 3000);
glm::vec3 lightColor = glm::vec3(10000000);
glm::vec3 ambientLight = glm::vec3(0.0);

Shadow shadow = Shadow(lightPosition, lightColor, ambientLight);

glm::vec3 movePoint;
glm::vec3 movingPoint;
glm::vec3 pullPoint;


void render(GLFWwindow* window) {

    if (animating) {
        for (int i = 0; i < 10; i++) { //�ݺ� -> �������� => �ⷷ�Ÿ��� ���� ����
            for (Mesh& mesh : meshes) {
                mesh.update(0.0166f / 10);
            }
        }
        for (Mesh& mesh : meshes) {
            mesh.updateGL();
        }
    }

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    glViewport(0, 0, w, h);
    glClearColor(0.81, 0.84, 0.96, 1); //����    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    shadow.makeShadowMap([&]() {
        for (Mesh& mesh : meshes) {
            mesh.render();
        }
    });

    glViewport(0, 0, w, h);
    glUseProgram(program.programID);
    shadow.render();

    GLuint ModelMatLocation = glGetUniformLocation(program.programID, "modelMat");
    glUniformMatrix4fv(ModelMatLocation, 1, 0, value_ptr(mat4(1)));

    GLuint viewMatLocation = glGetUniformLocation(program.programID, "viewMat");
    glUniformMatrix4fv(viewMatLocation, 1, 0, value_ptr(camera.getViewMat()));

    GLuint projMatLocation = glGetUniformLocation(program.programID, "projMat");
    glUniformMatrix4fv(projMatLocation, 1, 0, value_ptr(camera.getProjMat(w, h)));

    plane.render();

    for (Mesh& mesh : meshes) { 
        mesh.render();
    }

    glfwSwapBuffers(window);
}
