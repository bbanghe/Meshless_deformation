#pragma once

#include <iostream>
#define GLEW_STATIC 
#include <GL/glew.h> 
#include <GLFW/glfw3.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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

#include "math.h"
#include "Callback.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    float weight = 1.0f;
};

static const glm::vec3 gravity = glm::vec3(0.0f, -385.827f, 0.0f);

extern Program program;

extern glm::vec3 contact_point = glm::vec3(0.0f, -600.0f, 0.0f); 
extern glm::vec3 normal_vector = normalize(glm::vec3(0.0f, 1.0f, 0.0f));
extern int height = 600;

extern bool push;
extern bool release;
extern bool moveObj;


extern glm::vec3 pullPoint;
extern glm::vec3 movePoint;
extern glm::vec3 movingPoint;


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

    glm::vec3 init_height = glm::vec3(0.0f, height, 0.0f);

    size_t closestVertexIndex = 0;

    const float springConstant = 0.00005f;
    const float proximityThreshold = 0.0000001f;

    glm::vec3 nearestPoint = glm::vec3(0);


    //constructor
    Mesh(const std::vector<Vertex>& _vertices, const std::vector<unsigned int>& _indices, const std::vector<Texture>& _textures) 
        :vertices(_vertices), indices(_indices), textures(_textures) {
        velocity.resize(vertices.size(), glm::vec3(0));
        origin_point.resize(vertices.size());
        t_0 = optimalTranslation();

        for (int i = 0; i < vertices.size();i++) {
            origin_point[i] = vertices[i].Position;
            vertices[i].Position = rotate(-PI / 6.f, glm::vec3(0, 0, 1)) * glm::vec4(vertices[i].Position, 1);
            vertices[i].Position += init_height;
        }

        q_values.resize(vertices.size(), glm::vec3(0.0f));
        weight_values.resize(vertices.size(), 0.0f);

        for (int i = 0; i < vertices.size();i++) {
            q_values[i] = origin_point[i] - t_0;
            weight_values[i] = vertices[i].weight;
        }
    }

    void updateVertexPosition() {
        float minDistance = std::numeric_limits<float>::max();

        for (size_t i = 0; i < vertices.size(); ++i) {
            float distance = glm::distance(vertices[i].Position, movePoint);
            if (distance < minDistance) {
                minDistance = distance;
                closestVertexIndex = i;
            }
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
        glm::mat3 Rotation;

        //Rotation = Linear_deforamation(p, q , weights);    
        Rotation = Quadratic_deformation(p, q, weights);
        
        return Rotation;
    }

    glm::mat3 glm_rotation_SVD(glm::mat3 A_pq) {
        // SVD 적용

        //glm->Eigen 
        Eigen::Matrix3f A_pq_eigen;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                A_pq_eigen(i, j) = A_pq[i][j];
            }
        }

        Eigen::JacobiSVD<Eigen::Matrix3f> svd(A_pq_eigen, Eigen::ComputeFullU | Eigen::ComputeFullV);
        Eigen::Matrix3f U = svd.matrixU();
        Eigen::Matrix3f V = svd.matrixV();

        // 회전 행렬 R 계산
        Eigen::Matrix3f R_eigen = U * V.transpose();

        //Eigen -> glm
        glm::mat3 Rotation;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                Rotation[i][j] = R_eigen(i, j);
            }
        }
        return Rotation;
    }

    glm::mat3 eigen_rotation_SVD(Eigen::Matrix3f A_pq) {
        // SVD 적용


        Eigen::JacobiSVD<Eigen::Matrix3f> svd(A_pq, Eigen::ComputeFullU | Eigen::ComputeFullV);
        Eigen::Matrix3f U = svd.matrixU();
        Eigen::Matrix3f V = svd.matrixV();

        // 회전 행렬 R 계산
        Eigen::Matrix3f R_eigen = U * V.transpose();

        //Eigen -> glm
        glm::mat3 Rotation;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                Rotation[i][j] = R_eigen(i, j);
            }
        }
        return Rotation;
    }


    glm::mat3 Linear_deforamation(const std::vector<glm::vec3>& p, const std::vector<glm::vec3>& q, const std::vector<float>& weights) {

        // 행렬 A 생성: 
        glm::mat3 A_pq(0.0f);
        glm::mat3 A_qq(0.0f);

        //A_qq 사전 계산 ㄱㄴ... 나중에 수정 
        for (int i = 0; i < q.size(); ++i) {
            A_qq += weights[i] * outerProduct(q[i], q[i]);
        }
        A_qq = glm::inverse(A_qq);

        for (int i = 0; i < q.size(); ++i) {
            A_pq += weights[i] * outerProduct(p[i], q[i]);
        }

        glm::mat3 A = A_pq * A_qq;
        glm::mat3 Rotation = glm_rotation_SVD(A_pq);

        float beta = 0.99f; //더 커지면 오류 발생... 

        float detA = glm::determinant(A);
        A /= pow(detA, 1.0 / 3.0);
        Rotation = beta * A + (1 - beta) * Rotation;

        return Rotation;
    }

    using Matrix9f = Eigen::Matrix<float, 9, 9>;
    using Matrix3x9f = Eigen::Matrix<float, 3, 9>;
    using Vector9f = Eigen::Matrix<float, 9, 1 >; //column vector

    glm::mat3 Quadratic_deformation(const std::vector<glm::vec3>& p, const std::vector<glm::vec3>& q, const std::vector<float>& weights) {
        float beta = 0.99f;

        //q_tilde + A_qq_tilde 한번만 계산해도 ㅇㅋ
        std::vector<Vector9f> q_tilde;
        for (int i = 0; i < q.size(); ++i) {
            Vector9f q_i;
            q_i << q[i].x,          q[i].y,          q[i].z,
                   q[i].x * q[i].x, q[i].y * q[i].y, q[i].z * q[i].z, 
                   q[i].x * q[i].y, q[i].y * q[i].z, q[i].z * q[i].x;
            q_tilde.push_back(q_i);
        }
        
        //A_pq_tilde, A_qq_tilde 계산 후 A_tilde 구하기 
        Matrix3x9f A_pq_tilde = Matrix3x9f::Zero();
        Matrix9f   A_qq_tilde = Matrix9f::Zero();

        for (int i = 0; i < q.size(); ++i) {
            //glm -> eigen
            Eigen::Vector3f p_i;
            p_i << p[i].x, p[i].y, p[i].z;

            A_pq_tilde += weights[i] * p_i * q_tilde[i].transpose(); //  3X1 * 1X9  =  3X9
            A_qq_tilde += weights[i] * q_tilde[i] * q_tilde[i].transpose(); //  9X1  * 1X9  = 9X9
        }
        A_qq_tilde = A_qq_tilde.inverse();

        Matrix3x9f A_tilde = A_pq_tilde * A_qq_tilde; // 3X9 * 9X9  = 3X9
        Eigen::Matrix3f A_pq_svd = A_pq_tilde.block<3, 3>(0, 0); // A_pq의 첫 번째 3x3 블록을 추출

        glm::mat3 Rotation = eigen_rotation_SVD(A_pq_svd);
        glm::mat3 A;

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                A[i][j] = A_tilde(i, j);
            }
        }

        float detA = glm::determinant(A);
        A /= pow(detA, 1.0 / 3.0);
        Rotation = beta * A + (1 - beta) * Rotation;

        return Rotation;
    }




    void update(const float& dt) {

        
        std::vector<glm::vec3> goalPosition(vertices.size());

        float alpha = 0.5f; //탄성 (rigid-body : 1) 클 수록 잘 튄다...~
        glm::vec3 external = glm::vec3(0, -1, 0);
        float repulsive = 0.5f; //반발력 -> 계속 뛰어오르는 현상 방지


        const glm::vec3 fixedPoint = vertices[0].Position;

        //충돌했을 경우 
        for (int i = 0; i < vertices.size();i++) {
            if (fixedPoint != vertices[i].Position || release == true) {
                if (dot(vertices[i].Position - contact_point, normal_vector) < proximityThreshold && dot(velocity[i], normal_vector) < 0.0f) {
                    //collision resolve
                    glm::vec3 resolution = dot((contact_point - vertices[i].Position), normal_vector) * normal_vector;
                    vertices[i].Position += resolution;

                    //response
                    glm::vec3 Vn = dot(normal_vector, velocity[i]) * normal_vector;
                    glm::vec3 Vt = velocity[i] - Vn;
                    velocity[i] = Vt - repulsive * Vn;
                    velocity[i] = glm::vec3(0); //마찰력 0
                }
                velocity[i] = velocity[i] + gravity * dt;

                vertices[i].Position += velocity[i] * dt ;

                if (!release) {
                    glm::vec3 fixForce = springConstant * (fixedPoint - vertices[i].Position);
                    vertices[i].Position += fixForce;
                }
            }
        }

        if (push == true) {
            updateVertexPosition();
            nearestPoint = vertices[closestVertexIndex].Position;
    
            if (moveObj == true) {
                for (int i = 0; i < vertices.size();i++) {
                    vertices[i].Position += (movingPoint - nearestPoint);
                }
            }
            else {
                // pullPoint = 화면 상에서 선택한 위치! 
                for (int i = 0; i < vertices.size();i++) {
                    if (fixedPoint != vertices[i].Position || release == true) {
                        glm::vec3 pullForce = 0.001f * (pullPoint - nearestPoint - vertices[i].Position);
                        vertices[i].Position += pullForce;
                    }
                }

            }    
        }

        //Translation + Rotation 계산
        glm::vec3 t = optimalTranslation();
        std::vector<glm::vec3> p_values;  // p 벡터: 실제 모양 - t
        p_values.resize(vertices.size(), glm::vec3(0.0f));

        for (int i = 0; i < vertices.size(); i++) {
            p_values[i] = vertices[i].Position - t;
        }

        glm::mat3 R = optimalRotation(q_values, p_values, weight_values);
        
        //goalPosition 계산 
        for (int i = 0; i < vertices.size(); ++i) {
            goalPosition[i] = R * (origin_point[i] - t_0) + t;
        }

        for (int i = 0; i < vertices.size();i++) {
            glm::vec3 positionDifference = goalPosition[i] - vertices[i].Position;

            if (fixedPoint != vertices[i].Position || release == true) {
                //fixPoint는 속도와 position이 변하지 않음. 
                velocity[i] = (alpha * positionDifference / dt) + (dt * external / vertices[i].weight) + 0.9999f * velocity[i];
                vertices[i].Position += velocity[i] * dt;
            }
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
