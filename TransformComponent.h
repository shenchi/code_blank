#pragma once

#include "Component.h"
#include "TofuMath.h"

namespace tofu
{
	class TransformComponentData;

	typedef Component<TransformComponentData> TransformComponent;

	class TransformComponentData
	{
	public:
		TransformComponentData() : TransformComponentData(Entity()) {}

		TransformComponentData(Entity e)
			: 
			entity(e),
			parent(),
			translation(),
			rotation(),
			scale(),
			dirty(true)
		{}

		inline void					SetParent(TransformComponent parent) { this->parent = parent; dirty = true; }

		inline TransformComponent	GetParent() const { return parent; }

		inline void					SetTranslation(const math::float3& t) { translation = t; dirty = true; }

		inline void					SetTranslation(float x, float y, float z) { translation = { x, y, z }; dirty = true; }

		inline const math::float3&	GetTranslation() const { return translation; }

		inline void					SetRotation(const math::float4& q) { rotation = q; dirty = true; }

		inline const math::float4&	GetRotation() const { return rotation; }

		inline void					SetScale(const math::float3& s) { scale = s; dirty = true; }

		inline void					SetScale(float s) { scale = { s, s, s }; dirty = true; }

		inline void					SetScale(float x, float y, float z) { scale = { x, y, z }; dirty = true; }

		inline const math::float3&	GetScale() const { return scale; }

	public:

		// auxiliary functions

		// TODO

	private:
		Entity				entity;
		TransformComponent	parent;
		math::float3		translation;
		math::float4		rotation;
		math::float3		scale;
		bool				dirty;
	};

}