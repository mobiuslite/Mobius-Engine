#pragma once
#include "cInstancedRenderer.h"


class cInstancedBrush
{
public:
	cInstancedBrush();
	~cInstancedBrush();

	void AddOffset(glm::vec3 pos);
	void ChangeRenderer(cInstancedRenderer* renderer);

	bool HasRenderer() { return this->renderer != nullptr; }

private:
	cInstancedRenderer* renderer;
	glm::vec3 lastPosAdded;

	const float offset = 1.0f;
};