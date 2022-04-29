#include "cEntityManager.h"

cEntityManager::~cEntityManager()
{
	for (size_t i = 0; i < entities.size(); i++)
	{
		delete entities[i];
	}
	entities.clear();
}

void cEntityManager::Update(float dt)
{
	std::vector<cEntity*> deletedEntities;

	for (size_t i = 0; i < this->entities.size(); i++)
	{
		cEntity* curEntity = this->entities[i];
		curEntity->Update(dt);
		if (curEntity->markedForDeletion)
		{
			deletedEntities.push_back(curEntity);
		}
	}

	for (cEntity* entity : deletedEntities)
	{
		this->DeleteEntity(entity);
	}
}

std::vector<cEntity*> cEntityManager::GetEntities()
{
	return entities;
}
cEntity* cEntityManager::GetEntityByName(std::string name)
{
	for (size_t i = 0; i < entities.size(); i++)
	{
		if (entities[i]->name == name)
			return entities[i];
	}

	return nullptr;
}

cEntity* cEntityManager::CreateEntity(bool addToList)
{
	cEntity* entity = new cEntity();

	if(addToList)
		entities.push_back(entity);

	return entity;
}

void cEntityManager::DeleteEntity(cEntity* e)
{
	std::vector<cEntity*>::iterator it = std::find(this->entities.begin(), this->entities.end(), e);
	if (it != this->entities.end())
	{
		this->entities.erase(it);
	}

	delete e;
	e = nullptr;
}

void cEntityManager::RemoveEntity(cEntity* e)
{
	//this->entities
}

void cEntityManager::RemoveEntity(unsigned int index)
{
	this->entities.erase(this->entities.begin() + index);
}