#include "cShadowDepthFBO.h"
#include "../GLCommon.h"
#include <glm/gtc/matrix_transform.hpp> 

cShadowDepthFBO::cShadowDepthFBO()
{
	glGenFramebuffers(1, &this->depthFBO_ID);
	glBindFramebuffer(GL_FRAMEBUFFER, this->depthFBO_ID);

	glGenTextures(1, &(this->depthColor));
	glBindTexture(GL_TEXTURE_2D, this->depthColor);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, this->SHADOW_WIDTH, this->SHADOW_HEIGHT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	glFramebufferTexture(GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		this->depthColor, 0);

	glGenTextures(1, &(this->depthMap));			
	glBindTexture(GL_TEXTURE_2D, this->depthMap);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH24_STENCIL8, this->SHADOW_WIDTH, this->SHADOW_HEIGHT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);


	glFramebufferTexture(GL_FRAMEBUFFER,
		GL_DEPTH_STENCIL_ATTACHMENT,
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

	glStencilMask(0xFF);
	
	{	// Clear stencil
		//GLint intZero = 0;
		//glClearBufferiv(GL_STENCIL, 0, &intZero );
		glClearBufferfi(GL_DEPTH_STENCIL,
			0,		// Must be zero
			1.0f,	// Clear value for depth
			0);	// Clear value for stencil
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	
	
}