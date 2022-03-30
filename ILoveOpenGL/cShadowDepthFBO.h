#pragma once
#include "cLightManager.h"

class cShadowDepthFBO
{
public:
	cShadowDepthFBO();
	~cShadowDepthFBO();

	void ClearShadowBuffer();

	unsigned int depthFBO_ID;

	const unsigned int SHADOW_WIDTH = 1028;
	const unsigned int SHADOW_HEIGHT = 1028;

	unsigned int depthMap;
};