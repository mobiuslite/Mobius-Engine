#pragma once
#include "cLightManager.h"

class cShadowDepthFBO
{
public:
	cShadowDepthFBO();
	~cShadowDepthFBO();

	void ClearShadowBuffer();

	unsigned int depthFBO_ID;

	const unsigned int SHADOW_WIDTH = 2048;
	const unsigned int SHADOW_HEIGHT = 2048;

	unsigned int depthMap;
	unsigned int depthColor;
};