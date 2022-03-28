#pragma once
#include "GLCommon.h"
#include "cShaderManager.h"

class cPingPongFBOs
{
public:

	cPingPongFBOs(float width, float height, cShaderManager::cShaderProgram* program);
	~cPingPongFBOs();

	void ClearBuffers();

	void BlurBuffer(bool firstIteration, bool horizontal, GLuint firstMap = 0);

	unsigned int pingpongFBO[2] = {0, 0};
	unsigned int pingpongBuffer[2] = { 0, 0 };

	cShaderManager::cShaderProgram* program;

	float width;
	float height;
};