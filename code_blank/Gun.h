#pragma once

class Gun
{
public:
	Gun();
	~Gun();

	void SetIsActive(bool);
	bool GetIsActive();

private:
	bool isActive;
};