#pragma once

#include "Common.h"

namespace tofu
{
	enum class RendererCommand : uint32_t
	{
		None,
		CreateBuffer,
		UpdateBuffer,
		DestroyBuffer,
		CreateTexture,
		UpdateTexture,
		DestroyTexture,
		CreateSampler,
		DestroySampler,
		CreateVertexShader,
		DestroyVertexShader,
		CreatePixelShader,
		DestroyPixelShader,
		CreatePipelineState,
		DestroyPipelineState,
		Draw,
	};

	struct RendererCommandBuffer
	{
		RendererCommand*	cmds;
		void**				params;



		//void Add(RendererCommand cmd, void* param);
	};


	struct Renderer
	{
		virtual int32_t Init() = 0;

		virtual int32_t Release() = 0;

		virtual int32_t Submit(uint32_t count, RendererCommandBuffer* buffers) = 0;

		virtual int32_t Present() = 0;

		static Renderer* CreateRenderer();
	};

}

