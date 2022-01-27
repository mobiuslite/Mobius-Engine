#pragma once

#include "cKeyframe.h"
#include "cAnimation.h"
#include <vector>
#include "cEntity.h"

class AnimationSystem
{
public:
	void Update(const std::vector<cEntity*>& entities, float dt);
private:
	//const KeyFramePosition& FindCurrentKeyFramePosition(Animation* animation, float time);
	//const KeyFrameScale& FindCurrentKeyFrameScale(Animation* animation, float time);
	int FindKeyFramePositionIndex(cAnimation* animation, float time);
	int FindKeyFrameScaleIndex(cAnimation* animation, float time);
	int FindKeyFrameRotIndex(cAnimation* animation, float time);
};