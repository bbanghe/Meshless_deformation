#pragma once
#include <GL/glew.h> 
#include <GLFW/glfw3.h> 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <glm/gtx/transform.hpp>
#include <glm/gtx/norm.hpp>


/*
    glm::vec3 point = glm::vec3(0.0f, -600.0f, 0.0f); //-0.5 cats
    glm::vec3 normal_vector = normalize(glm::vec3(0.2f, 1.0f, 0.0f));
*/

extern Program program;

struct Plane {
    GLuint vertexBuffer = 0;
    GLuint vertexArray = 0;
    GLuint elementBuffer = 0;

    std::vector<unsigned int> indices;

    glm::vec3 point = glm::vec3(0.0f);
    glm::vec3 normal = glm::vec3(0.0f);
    glm::vec4 color = glm::vec4(0.941176f, 0.972549f, 1.0f, 1.0f);

    int vertexCount = 0;
    
    float plane_vertices[12] = { 0, };
    GLuint plane_indices[6] = {
        0, 1, 2, //012 »ï°¢Çü
        0, 2, 3  //023 »ï°¢Çü
    };

    //constructor
    Plane(glm::vec3 point_, glm::vec3 normal_) : point(point_), normal(normal_) {
        
        //Ax + By + Cz + D = 0 (normal dot point...)
        float A = normal.x;
        float B = normal.y;
        float C = normal.z;
        float D = -glm::dot(normal, point);

        glm::vec3 p1 = glm::vec3(2000.0f, ((-D - A * (2000.0f)) / B) , 0.0f); 
        glm::vec3 p2 = glm::vec3(-2000.0f, ((-D - A * ( - 2000.0f)) / B), 0.0f);
        glm::vec3 p3 = glm::vec3(0.0f, ((-D - C * (2000.0f)) / B), 2000.0f);
        glm::vec3 p4 = glm::vec3(0.0f, ((-D - C * (-2000.0f)) / B), -2000.0f);

        plane_vertices[0] = p1.x;
        plane_vertices[1] = p1.y;
        plane_vertices[2] = p1.z;

        plane_vertices[3] = p2.x;
        plane_vertices[4] = p2.y;
        plane_vertices[5] = p2.z;

        plane_vertices[6] = p3.x;
        plane_vertices[7] = p3.y;
        plane_vertices[8] = p3.z;

        plane_vertices[9] = p4.x;
        plane_vertices[10] = p4.y;
        plane_vertices[11] = p4.z;

    }

    void setupPlane() {
        
        glGenVertexArrays(1, &vertexArray);
        glBindVertexArray(vertexArray);

        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(plane_vertices), plane_vertices, GL_STATIC_DRAW);

        glGenBuffers(1, &elementBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(plane_indices), plane_indices, GL_STATIC_DRAW);

        // vertex Positions
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
        vertexCount = 6;

    }

    void render() {
        if (vertexBuffer == 0) setupPlane();

        GLuint TexOrColorLocation = glGetUniformLocation(program.programID, "TexOrColor");
        glUniform1i(TexOrColorLocation, 0); // trueÀÌ¸é 1, falseÀÌ¸é 0

        GLuint colorLocation = glGetUniformLocation(program.programID, "color");
        glUniform4fv(colorLocation, 1, glm::value_ptr(color));

        glBindVertexArray(vertexArray);
        glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

    }
};