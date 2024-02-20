#pragma once
#include <GL/glew.h> 
#include <GLFW/glfw3.h> 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <glm/gtx/transform.hpp>
#include <glm/gtx/norm.hpp>


extern Program shadowProgram;
extern Program program;

struct Shadow
{
	
	glm::vec3 lightPosition = glm::vec3(0); //light의 위치
	glm::vec3 lightColor = glm::vec3(0); //밝기 정도
	glm::vec3 ambientLight = glm::vec3(0.f);



	GLuint shadowFBO = 0;
	GLuint shadowTex = 0;
	GLuint shadowDepth = 0;



	Shadow(glm::vec3 _lightPosition, glm::vec3 _lightColor, glm::vec3 _ambientLight) :lightPosition(_lightPosition), lightColor(_lightColor), ambientLight(_ambientLight) {
		
	}

	void setupShadow() {

		glGenTextures(1, &shadowTex);
		glBindTexture(GL_TEXTURE_2D, shadowTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 1024, 1024, 0, GL_RGB, GL_FLOAT, 0);

		glGenTextures(1, &shadowDepth);
		glBindTexture(GL_TEXTURE_2D, shadowDepth);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

		glGenFramebuffers(1, &shadowFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, shadowTex, 0);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowDepth, 0);
		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, drawBuffers);
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			printf("ERROR");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		GLuint lightPositionLocation = glGetUniformLocation(program.programID, "lightPosition");
		glUniform3fv(lightPositionLocation, 1, glm::value_ptr(lightPosition));

		GLuint lightColorLocation = glGetUniformLocation(program.programID, "lightColor");
		glUniform3fv(lightColorLocation, 1, glm::value_ptr(lightColor));

		GLuint ambientLightLocation = glGetUniformLocation(program.programID, "ambientLight");
		glUniform3fv(ambientLightLocation, 1, glm::value_ptr(ambientLight));



	}

	glm::mat4 calculateShadowMVP() {
		glm::mat4 shadowViewMat = glm::lookAt(lightPosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

		float near_plane = 0.1f;
		float far_plane = 100.0f;
		glm::mat4 shadowProjMat = glm::ortho(0.f, 800.f, 0.f, 800.f, near_plane, far_plane);

		return shadowProjMat * shadowViewMat;

	}

	void render() {
		if(shadowFBO == 0)
			setupShadow();

		//shadow map create
		glm::mat4 shadowMVP = calculateShadowMVP();


		glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
		glViewport(0, 0, 1024, 1024);
		glClearColor(0, 0, 0, 0);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


		glUseProgram(shadowProgram.programID);
		GLuint shadowMVPLocation = glGetUniformLocation(shadowProgram.programID, "shadowMVP");
		glUniformMatrix4fv(shadowMVPLocation, 1, 0, glm::value_ptr(shadowMVP));

		glBindFramebuffer(GL_FRAMEBUFFER, 0);


		//map create done
		glUseProgram(program.programID);

		shadowMVPLocation = glGetUniformLocation(program.programID, "shadowMVP");
		glUniformMatrix4fv(shadowMVPLocation, 1, 0, glm::value_ptr(shadowMVP));

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, shadowTex);
		GLuint shadowTexLocation = glGetUniformLocation(program.programID, "shadowTex");
		glUniform1i(shadowTexLocation, 4);


	}

};

