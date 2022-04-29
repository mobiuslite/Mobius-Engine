#include "cInstancedBrush.h"

cInstancedBrush::cInstancedBrush()
{
	this->renderer = nullptr;

	this->lastPosAdded = glm::vec3(-100.0f, -100.0f, -100.0f);
	this->enabled = false;
}
cInstancedBrush::~cInstancedBrush()
{
}

void cInstancedBrush::SetActive(bool state)
{
	this->enabled = state;
}
bool cInstancedBrush::IsActive()
{
	return this->enabled;
}

void cInstancedBrush::AddOffset(glm::vec3 pos)
{
	if (this->renderer != nullptr)
	{
		if (glm::distance(pos, this->lastPosAdded) >= this->renderer->GetOffset())
		{
			this->renderer->AddOffset(pos);
			this->lastPosAdded = pos;
		}
	}
}

void cInstancedBrush::ChangeRenderer(cInstancedRenderer* renderer)
{
	this->renderer = renderer;
}