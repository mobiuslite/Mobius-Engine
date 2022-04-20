#pragma once
#include <string>
#include <vector>

class cComponent;

class cEntity
{
public:

	template<class T> T* AddComponent();
	template<class T> bool AddComponent(T* c);

	template<class T> T* GetComponent();
	template<class T> bool HasComponent();
	void RemoveComponent(cComponent* component);
		
	std::string name;	
	std::string childOf = "";

	std::vector<cComponent*> components;	
	std::vector<cEntity*> children;

	bool isGameplayEntity = false;
	

	friend class cWorld;
	friend class cEmitter;
	friend class cEntityManager;
	friend class cComponent;

	void Update(float dt);
	void Delete();
	const bool IsBeingDeleted() { return markedForDeletion; }
	void StopDeletion() { markedForDeletion = false; }

private:
	bool markedForDeletion = false;

	cEntity();
	virtual ~cEntity();
};

template<class T> bool cEntity::AddComponent(T* c)
{
	//Check if component already exists;
	for (size_t i = 0; i < this->components.size(); i++)
	{
		//If component already exists, return false
		if (dynamic_cast<T*>(components[i]) != nullptr)
		{
			return false;
		}
	}

	//If it is not a component, return false; 
	cComponent* newComp = dynamic_cast<cComponent*>(c);

	if (newComp == nullptr)
	{
		return false;
	}

	newComp->SetEntity(this);
	newComp->Awake();

	this->components.push_back(c);
	return true;
}
template <class T> T* cEntity::AddComponent()
{
	//Check if component already exists;
	for (size_t i = 0; i < this->components.size(); i++)
	{
		//If component already exists, just return that one.
		if (dynamic_cast<T*>(components[i]) != nullptr)
		{
			return (T*)components[i];
		}
	}

	T* newComponent = new T();
	cComponent* newComp = dynamic_cast<cComponent*>(newComponent);

	//Returns nullptr if the templated class is not a component.
	if (newComp == nullptr)
	{
		delete newComponent;
		return nullptr;
	}

	newComp->SetEntity(this);
	newComp->Awake();

	this->components.push_back(newComp);
	return newComponent;
}

template<class T> bool cEntity::HasComponent()
{
	for (size_t i = 0; i < components.size(); i++)
	{
		if (dynamic_cast<T*>(components[i]) != nullptr)
			return true;
	}
	return false;
}

template<class T> T* cEntity::GetComponent()
{
	for (size_t i = 0; i < components.size(); i++)
	{
		if (dynamic_cast<T*>(components[i]) != nullptr)
		{
			return dynamic_cast<T*>(components[i]);
		}
	}
	return nullptr;
}