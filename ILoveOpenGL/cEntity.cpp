#include "cEntity.h"
#include "cTransform.h"

#include "cComponent.h"

#include <typeinfo>

cEntity::cEntity()
{
	AddComponent<cTransform>();
}

cEntity::~cEntity()
{
	for (cComponent* c : this->components)
	{
		RemoveComponent(c);
	}

	for (cEntity* child : this->children)
	{
		delete child;
	}
}

void cEntity::Update(float dt)
{
	for (cComponent* comp : this->components)
	{
		if(comp->IsUpdatable())
			comp->Update(dt);
	}
}

void cEntity::Delete()
{
	this->markedForDeletion = true;
}

void cEntity::RemoveComponent(cComponent* component)
{
	//Finds component to delete
	for (size_t i = 0; i < components.size(); i++)
	{
		if (this->components[i] == component)
		{
			delete this->components[i];

			//If there are components left.
			if (this->components.size() > 0)
			{
				//Sets the current compoenent to the end
				this->components[i] = this->components[components.size()-1];
			}

			//Pop the end
			//NOTE: Doing this stops a bug when you delete something in a list consecutively, and the automatically moving of elements creates a crash.
			this->components.pop_back();
			return;
		}
	}
}