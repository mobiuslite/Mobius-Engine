#pragma once

class cComponent
{
public:
	virtual ~cComponent() {}
	virtual void Update() = 0;

	const bool IsUpdatable() { return isUpdatable; }


protected:
	//cComponent() {}
	bool isUpdatable = false;
};