#pragma once
#include "cLightManager.h"

class cShadowDepthFBO
{
public:
	cShadowDepthFBO();
	~cShadowDepthFBO();

	void ClearShadowBuffer();

	unsigned int depthFBO_ID;

	const unsigned int SHADOW_WIDTH = 4028;
	const unsigned int SHADOW_HEIGHT = 4028;

	unsigned int depthMap;
	unsigned int depthColor;
};