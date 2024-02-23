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
	GLuint PlaneVBO = 0;
	GLuint PlaneVAO = 0;
	GLuint PlaneEBO = 0;

	std::vector<unsigned int> indices;

	glm::vec3 point;
	glm::vec3 normal;
	glm::vec4 color = glm::vec4(0.941176f, 0.972549f, 1.0f, 1.0f);

	int vertexCount = 0;
	
	glm::vec3 plane_vertices[4];
	GLuint plane_indices[6] = {
		0, 2, 1, //012 »ï°¢Çü
		2, 3, 1  //023 »ï°¢Çü
	};

	//constructor
	Plane(glm::vec3 point_, glm::vec3 normal_) : point(point_), normal(normal_) {
		
		//Ax + By + Cz + D = 0 (normal dot point...)
		float A = normal.x;
		float B = normal.y;
		float C = normal.z;
		float D = -glm::dot(normal, point);

		const float sz = 2500.f;
		glm::vec3 p1 = glm::vec3(-sz, ((-D - A * (-sz) - C * (-sz)) / B), -sz);
		glm::vec3 p2 = glm::vec3( sz, ((-D - A * ( sz) - C * (-sz)) / B), -sz);
		glm::vec3 p3 = glm::vec3(-sz, ((-D - A * (-sz) - C * ( sz)) / B),  sz);
		glm::vec3 p4 = glm::vec3( sz, ((-D - A * ( sz) - C * ( sz)) / B),  sz);

		plane_vertices[0] = p1;
		plane_vertices[1] = p2;
		plane_vertices[2] = p3;
		plane_vertices[3] = p4;
	}

	void setupPlane() {

		glGenVertexArrays(1, &PlaneVAO);
		glBindVertexArray(PlaneVAO);

		glGenBuffers(1, &PlaneVBO);
		glBindBuffer(GL_ARRAY_BUFFER, PlaneVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(plane_vertices), plane_vertices, GL_STATIC_DRAW);

		glGenBuffers(1, &PlaneEBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, PlaneEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(plane_indices), plane_indices, GL_STATIC_DRAW);

		// vertex Positions
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindVertexArray(0);
		vertexCount = 6;
	}

	void render() {
		if (PlaneVBO == 0) setupPlane();

		GLuint TexOrColorLocation = glGetUniformLocation(program.programID, "TexOrColor");
		glUniform1i(TexOrColorLocation, 0); // trueÀÌ¸é 1, falseÀÌ¸é 0

		GLuint colorLocation = glGetUniformLocation(program.programID, "color");
		glUniform4fv(colorLocation, 1, glm::value_ptr(color));

		glBindVertexArray(PlaneVAO);
		glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

	}
};