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

	void SetActive(bool state);
	bool IsActive();

private:
	bool enabled;

	cInstancedRenderer* renderer;
	glm::vec3 lastPosAdded;
};