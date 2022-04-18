#pragma once
#include "cLightManager.h"

class cShadowDepthFBO
{
public:
	cShadowDepthFBO();
	~cShadowDepthFBO();

	void ClearShadowBuffer();

	unsigned int depthFBO_ID;

	const unsigned int SHADOW_WIDTH = 8028;
	const unsigned int SHADOW_HEIGHT = 8028;

	unsigned int depthMap;
	unsigned int depthColor;
};