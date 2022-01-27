#pragma once

#include <vector>
#include "cComponent.h"
#include "cKeyframe.h"

struct cAnimation : cComponent
{
	virtual ~cAnimation() {};

	bool playing;
	float duration;
	float currentTime;
	float speed;
	bool repeat;
	std::vector<cKeyFramePosition> keyFramePositions;
	std::vector<cKeyFrameScale> keyFrameScales;
	std::vector<cKeyFrameRotation> keyFrameRotations;
};