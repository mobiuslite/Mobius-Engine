#include "cTarget.h"
#include "cMeshRenderer.h"


cTarget::cTarget(cBowComponent* bow)
{
	this->bow = bow;
	this->isUpdatable = true;
	this->thisTransform = nullptr;
}

void cTarget::Awake()
{
	this->thisTransform = this->GetEntity()->GetComponent<cTransform>();
}

void cTarget::Update(float dt)
{
	if (this->thisTransform != nullptr)
	{
		std::vector<cProjectile*> bowProj = bow->GetFiredProjectiles();

		for (cProjectile* proj : bowProj)
		{
			//Projectile is in target
			if (glm::distance(proj->GetProjTransform().position, thisTransform->position) <= this->TARGET_RADIUS)
			{
				cMeshRenderer* targetMesh = this->GetEntity()->GetComponent<cMeshRenderer>();

				targetMesh->bUseWholeObjectDiffuseColour = true;
				targetMesh->wholeObjectDiffuseRGBA = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);

				proj->HitTarget();
			}
		}
	}
}