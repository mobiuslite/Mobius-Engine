#pragma once
#include <vector>
#include "cEntity.h"

class cEntityManager
{
public:

	cEntityManager() = default;
	~cEntityManager();

	std::vector<cEntity*> GetEntities();
	cEntity* GetEntityByName(std::string name);

	cEntity* CreateEntity(bool addToList = true);

	void DeleteEntity(cEntity* e);
	void RemoveEntity(cEntity* e);
	void RemoveEntity(unsigned int index);

private:
	std::vector<cEntity*> entities;
};