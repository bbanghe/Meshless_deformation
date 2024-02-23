#pragma once

#include <iostream>
#define GLEW_STATIC 
#include <GL/glew.h> 
#include <GLFW/glfw3.h> 
#include <glm/glm.hpp>
#include <vector>
#include "toys.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <Eigen/Dense>
#include <Eigen/SVD>

#include "texture.h"
#include "plane.h"
#include "camera.h"
#include "loadShadow.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    float weight = 1.0f;
};

static const glm::vec3 gravity = glm::vec3(0.0f, -385.827f, 0.0f);

extern Program program;

extern glm::vec3 contact_point = glm::vec3(0.0f, -600.0f, 0.0f); 
extern glm::vec3 normal_vector = normalize(glm::vec3(0.2f, 1.0f, 0.1f));

struct Mesh {
    GLuint vertexBuffer = 0;
    GLuint vertexArray = 0;
    GLuint elementBuffer = 0;

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    std::vector<glm::vec3> velocity;


    glm::vec3 t_0 = glm::vec3(0.0f);
    std::vector<glm::vec3> q_values;  // q 벡터:  초기모양 - t_0 
    std::vector<float> weight_values;  // weights 벡터의 값들... vertices.weight
    std::vector<glm::vec3> origin_point = std::vector<glm::vec3>(0);

    glm::vec3 init_height = glm::vec3(0.0f, 600.0f, 0.0f);


    //constructor
    Mesh(const std::vector<Vertex>& _vertices, const std::vector<unsigned int>& _indices, const std::vector<Texture>& _textures) :vertices(_vertices), indices(_indices), textures(_textures) {
        velocity.resize(vertices.size(), glm::vec3(0));
        origin_point.resize(vertices.size());
        t_0 = optimalTranslation();

        for (int i = 0; i < vertices.size();i++) {
            origin_point[i] = vertices[i].Position;
            vertices[i].Position = rotate(-PI / 6.f, glm::vec3(0, 0, 1)) * glm::vec4(vertices[i].Position, 1);
            vertices[i].Position += init_height; //duck : 600
        }

        q_values.resize(vertices.size(), glm::vec3(0.0f));
        weight_values.resize(vertices.size(), 0.0f);

        for (int i = 0; i < vertices.size();i++) {
            q_values[i] = origin_point[i] - t_0;
            weight_values[i] = vertices[i].weight;
        }
    }


    glm::vec3 optimalTranslation() {
        glm::vec3 Translation = glm::vec3(0.0f);
        glm::vec3 sumPosition = glm::vec3(0.0f);
        float sumWeight = 0;

        for (int i = 0; i < vertices.size();i++) {
            sumPosition += vertices[i].weight * vertices[i].Position;
            sumWeight += vertices[i].weight;
        }

        Translation = sumPosition / sumWeight;

        return Translation;
    }

    glm::mat3 optimalRotation(const std::vector<glm::vec3>& q, const std::vector<glm::vec3>& p, const std::vector<float>& weights) {

        //행렬 A 생성: 
        glm::mat3 A_pq(0.0f);
        glm::mat3 A_qq(0.0f);

        for (int i = 0; i < q.size(); ++i) {
            A_pq += weights[i] * outerProduct(p[i], q[i]);
            A_qq += weights[i] * outerProduct(q[i], q[i]);
        }

        // SVD
        Eigen::Matrix3f A_pq_eigen, A_qq_eigen;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                A_pq_eigen(i, j) = A_pq[i][j];
                A_qq_eigen(i, j) = A_qq[i][j];
            }
        }

        Eigen::JacobiSVD<Eigen::Matrix3f> svd(A_pq_eigen, Eigen::ComputeFullU | Eigen::ComputeFullV);
        Eigen::Matrix3f U = svd.matrixU();
        Eigen::Matrix3f V = svd.matrixV();

        // 회전 행렬 R 계산
        Eigen::Matrix3f R_eigen = U * V.transpose();

        glm::mat3 Rotation;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                Rotation[i][j] = R_eigen(i, j);
            }
        }
        return Rotation;
    }

    void update(const float& dt) {

        std::vector<glm::vec3> goalPosition(vertices.size());


        float proximityThreshold = 0.000000001f;

        float alpha = 0.5f; //탄성 (rigid-body : 1)
        glm::vec3 external = glm::vec3(0, -1, 0);

        for (int i = 0; i < vertices.size();i++) {
            if (dot(vertices[i].Position - contact_point, normal_vector) < proximityThreshold && dot(velocity[i], normal_vector) < 0.0f) {
                //collision resolve
                glm::vec3 resolution = dot((contact_point - vertices[i].Position), normal_vector) * normal_vector;
                vertices[i].Position += resolution;

                //response
                glm::vec3 Vn = dot(normal_vector, velocity[i]) * normal_vector;
                glm::vec3 Vt = velocity[i] - Vn;
                velocity[i] = Vt - 0.9f * Vn;
            }
            velocity[i] += gravity * dt;
            vertices[i].Position = vertices[i].Position + velocity[i] * dt;
        }
        glm::vec3 t = optimalTranslation();
        std::vector<glm::vec3> p_values;  // p 벡터: 실제 모양 - t
        p_values.resize(vertices.size(), glm::vec3(0.0f));


        for (int i = 0; i < vertices.size(); i++) {
            p_values[i] = vertices[i].Position - t;
        }

        glm::mat3 R = optimalRotation(q_values, p_values, weight_values);

        for (int i = 0; i < vertices.size(); ++i) {
            goalPosition[i] = R * (origin_point[i] - t_0) + t;
        }
        for (int i = 0; i < vertices.size();i++) {
            glm::vec3 positionDifference = goalPosition[i] - vertices[i].Position;
            velocity[i] = (alpha * positionDifference / dt) + (dt * external / vertices[i].weight) + 0.9999f * velocity[i];
            vertices[i].Position += velocity[i] * dt;
        }
    }
    void updateGL() {
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());
        glFlush();
    }



    void setupMesh() {
        //vertexBuffer
        glGenVertexArrays(1, &vertexArray);
        glGenBuffers(1, &vertexBuffer);
        glGenBuffers(1, &elementBuffer);


        glBindVertexArray(vertexArray);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // vertex Positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        glBindVertexArray(0);

    }
    void render() {

        if (vertexBuffer == 0) setupMesh();


        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;

        GLuint TexOrColorLocation = glGetUniformLocation(program.programID, "TexOrColor");
        glUniform1i(TexOrColorLocation, 1); // true이면 1, false이면 0
        

        for (unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            std::string number;
            std::string name = textures[i].type;
            if (name == "diffuse")
                number = std::to_string(diffuseNr++);
            else if (name == "specular")
                number = std::to_string(specularNr++);
            else if (name == "normal")
                number = std::to_string(normalNr++);
            else if (name == "height")
                number = std::to_string(heightNr++);

            glBindTexture(GL_TEXTURE_2D, textures[i].id);
            glUniform1i(glGetUniformLocation(program.programID, (name + number).c_str()), i);

        }

        glBindVertexArray(vertexArray);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBuffer);

        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);

        glActiveTexture(GL_TEXTURE0);
    }
};
