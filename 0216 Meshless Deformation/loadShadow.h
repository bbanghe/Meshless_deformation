#pragma once
#include <GL/glew.h> 
#include <GLFW/glfw3.h> 
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <glm/gtx/transform.hpp>
#include <glm/gtx/norm.hpp>
#include <functional>

extern Program shadowProgram;
extern Program program;

struct Shadow
{
	
	glm::vec3 lightPosition; //light의 위치
	glm::vec3 lightColor; //밝기 정도
	glm::vec3 ambientLight;

	glm::mat4 shadowMVP;
	GLuint shadowMVPLocation = 0;

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

	}

	glm::mat4 calculateShadowMVP() {
		glm::mat4 shadowViewMat = glm::lookAt(lightPosition, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

		float near_plane = 1.f;
		float far_plane = 8000.0f;
		glm::mat4 shadowProjMat = glm::ortho(-1000.f, 1000.f, -1000.f, 1000.f, near_plane, far_plane);

		return shadowProjMat * shadowViewMat;

	}

	void makeShadowMap(std::function<void()> renderFunc) {
		if (shadowFBO == 0)
			setupShadow();

		glUseProgram(shadowProgram.programID);

		shadowMVP = calculateShadowMVP();

		glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
		glViewport(0, 0, 1024, 1024);
		glClearColor(1, 1, 1, 1);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		shadowMVPLocation = glGetUniformLocation(shadowProgram.programID, "shadowMVP");
		glUniformMatrix4fv(shadowMVPLocation, 1, 0, glm::value_ptr(shadowMVP));

		renderFunc();
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void render() {
		glUseProgram(program.programID);

		GLuint lightPositionLocation = glGetUniformLocation(program.programID, "lightPosition");
		glUniform3fv(lightPositionLocation, 1, glm::value_ptr(lightPosition));

		GLuint lightColorLocation = glGetUniformLocation(program.programID, "lightColor");
		glUniform3fv(lightColorLocation, 1, glm::value_ptr(lightColor));

		GLuint ambientLightLocation = glGetUniformLocation(program.programID, "ambientLight");
		glUniform3fv(ambientLightLocation, 1, glm::value_ptr(ambientLight));

		shadowMVPLocation = glGetUniformLocation(program.programID, "shadowMVP");
		glUniformMatrix4fv(shadowMVPLocation, 1, 0, glm::value_ptr(shadowMVP));

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, shadowTex);
		GLuint shadowTexLocation = glGetUniformLocation(program.programID, "shadowTex");
		glUniform1i(shadowTexLocation, 4);
	}
};

