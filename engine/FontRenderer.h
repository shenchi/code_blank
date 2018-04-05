#pragma once

#include "Common.h"

struct FONScontext;

namespace tofu
{
	struct RendererCommandBuffer;

	struct FontRendererContext
	{
		RendererCommandBuffer*	cmdBuf;
		float*					vertices;
		uint16_t*				indices;
		uint32_t				numVerts;
		uint32_t				maxVerts;

		TextureHandle			tex;
		uint32_t				width;
		uint32_t				height;
	};

	class FontRenderer
	{
	public:

		int32_t Setup(PipelineStateHandle pso, TextureHandle tex, SamplerHandle samp, BufferHandle vb, BufferHandle ib, BufferHandle cb, uint32_t maxVertices);

		int32_t Init();

		int32_t Shutdown();

		int32_t Reset(RendererCommandBuffer* cmdBuf);

		int32_t Render(const char* text, float x, float y);

		int32_t Submit();

	private:
		FontRendererContext		context;
		FONScontext*			fonsContext;
		PipelineStateHandle		pso; 
		SamplerHandle			samp;
		BufferHandle			vb;
		BufferHandle			ib;
		BufferHandle			cb;

		int32_t					font;
	};
}