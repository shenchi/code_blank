#include "Gun.h"


Gun::Gun()
{
	isActive = false;
}

Gun::~Gun()
{
}

void Gun::SetIsActive(bool _isActive)
{
	isActive = _isActive;
}

bool Gun::GetIsActive()
{
	return isActive;
}