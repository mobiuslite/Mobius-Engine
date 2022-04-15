#pragma once

class cEntity;

class cComponent
{
public:
	virtual ~cComponent() {}
	virtual void Update(float dt) = 0;
	
	//Call this if you want to do something that requires "entity" to be not null
	virtual void Awake() {};

	const bool IsUpdatable() { return isUpdatable; }

	cEntity* GetEntity()
	{
		return this->entity;
	}

	friend class cEntity;

private:
	void SetEntity(cEntity* entity)
	{
		this->entity = entity;
	}
	cEntity* entity;
protected:

	//cComponent() {}
	bool isUpdatable = false;	
};