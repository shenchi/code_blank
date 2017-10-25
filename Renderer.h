#pragma once

#include "Common.h"

namespace tofu
{
	struct RendererCommand
	{
		enum
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
			MaxRendererCommands
		};
	};

	struct RendererCommandBuffer
	{
		uint32_t*			cmds;
		void**				params;

		uint32_t			capacity;
		uint32_t			size;

		//void Add(RendererCommand cmd, void* param);
	};


	class Renderer
	{
	public:
		virtual int32_t Init() = 0;

		virtual int32_t Release() = 0;

		virtual int32_t Submit(RendererCommandBuffer* buffer) = 0;

		virtual int32_t Present() = 0;

		static Renderer* CreateRenderer();
	};

}

