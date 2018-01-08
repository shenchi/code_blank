#pragma once

#include <Tofu.h>
#include <rapidjson/document.h>

class SceneLoadingDemo : public tofu::Module
{
public:
	virtual int32_t Init() override;

	virtual int32_t Shutdown() override;

	virtual int32_t Update() override;

private:

	int32_t LoadSceneEntity(const rapidjson::Value& value, tofu::TransformComponent parent);

	int32_t LoadComponents(const rapidjson::Value& value, tofu::Entity e);

private:
	tofu::ResourceManager		resMgr;
	tofu::CameraComponent		cam;
	tofu::TransformComponent	tCamera;
	float						pitch;
	float						yaw;
};