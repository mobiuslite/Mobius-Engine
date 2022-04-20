#pragma once
#include "cLightManager.h"

class cShadowDepthFBO
{
public:
	cShadowDepthFBO();
	~cShadowDepthFBO();

	void ClearShadowBuffer();

	unsigned int depthFBO_ID;

	const unsigned int SHADOW_WIDTH = 9028;
	const unsigned int SHADOW_HEIGHT = 9028;

	unsigned int depthMap;
	unsigned int depthColor;
};