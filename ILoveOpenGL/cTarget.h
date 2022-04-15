#pragma once
#include "cComponent.h"
#include "cBowComponent.h"

class cTarget : public cComponent
{
public:
	cTarget(cBowComponent* otherBow);

	virtual ~cTarget() {};
	virtual void Update(float dt);
	virtual void Awake();

private:
	cTransform* thisTransform;
	cBowComponent* bow;

	const float TARGET_RADIUS = 1.0f;
};