#include "cShadowDepthFBO.h"
#include "GLCommon.h"
#include <glm/gtc/matrix_transform.hpp> 

cShadowDepthFBO::cShadowDepthFBO()
{
	glGenFramebuffers(1, &this->depthFBO_ID);

	glGenTextures(1, &(this->depthMap));
	glBindTexture(GL_TEXTURE_2D, this->depthMap);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, this->SHADOW_WIDTH, this->SHADOW_HEIGHT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, this->depthFBO_ID);
	glFramebufferTexture(GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		this->depthMap, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

cShadowDepthFBO::~cShadowDepthFBO()
{
	glDeleteTextures(1, &(this->depthMap));
	glDeleteFramebuffers(1, &(this->depthFBO_ID));
}

void cShadowDepthFBO::ClearShadowBuffer()
{
	GLfloat rgbBlack[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	glBindFramebuffer(GL_FRAMEBUFFER, this->depthFBO_ID);
	glClearBufferfv(GL_COLOR, 0, rgbBlack);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}