#pragma once
#include "../Managers/cLightManager.h"

//Holds the depth buffer that will be used for direcetional shadow mapping
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