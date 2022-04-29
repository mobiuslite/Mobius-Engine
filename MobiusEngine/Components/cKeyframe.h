#pragma once


#include "cComponent.h"
#include <glm/vec3.hpp>
#include <glm/gtx/quaternion.hpp>

enum EasingType
{
	EaseIn,
	EaseOut,
	EaseInOut,
	None
};

#define DIRECT_ROTATION 0
#define LERP_ROTATION 1
#define SLERP_ROTATION 2

struct cKeyFramePosition : cComponent
{
	cKeyFramePosition(float time, const glm::vec3& position, EasingType easingType = EasingType::None)
		: time(time), position(position), easingType(easingType)
	{
	}
	float time;
	glm::vec3 position;
	EasingType easingType;
};

struct cKeyFrameScale : cComponent
{
	cKeyFrameScale(float time, const glm::vec3& scale, EasingType easingType = EasingType::None)
		: time(time), scale(scale), easingType(easingType)
	{
	}
	float time;
	glm::vec3 scale;
	EasingType easingType;
};

struct cKeyFrameRotation : cComponent
{
	cKeyFrameRotation(float time, const glm::quat& rotation, int interpolationType = 0, EasingType easingType = EasingType::None)
		: time(time), rotation(rotation), InterpolationType(interpolationType), easingType(easingType)
	{
	}
	float time;
	glm::quat rotation;
	EasingType easingType;
	int InterpolationType;
};