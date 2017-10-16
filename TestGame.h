#pragma once

#include "Module.h"

class TestGame : public tofu::Module
{
public:
	virtual int32_t Init() override;

	virtual int32_t Shutdown() override;

	virtual int32_t Update() override;
};
