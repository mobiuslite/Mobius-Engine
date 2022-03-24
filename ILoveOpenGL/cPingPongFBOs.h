#pragma once
#include "GLCommon.h"
#include "cShaderManager.h"

class cPingPongFBOs
{
public:

	cPingPongFBOs(float width, float height);

	void ClearBuffers();

	unsigned int pingpongFBO[2] = {0, 0};
	unsigned int pingpongBuffer[2] = { 0, 0 };

	float width;
	float height;
};