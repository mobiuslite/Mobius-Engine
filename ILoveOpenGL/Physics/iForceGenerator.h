#pragma once
#include "cParticle.h"


class iForceGenerator
{
public:
	virtual ~iForceGenerator() {}
	virtual void Integrate(cParticle* p, float dt) = 0;
};