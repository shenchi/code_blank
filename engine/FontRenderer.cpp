#include "FontRenderer.h"
#include "Renderer.h"
#include "MemoryAllocator.h"
#include <cstring>

extern "C" {
#include <fontstash.h>
}


namespace
{
	using namespace tofu;

	int TofuRenderCreate(void* userPtr, int width, int height)
	{
		FontRendererContext* ctx = reinterpret_cast<FontRendererContext*>(userPtr);

		CreateTextureParams* params = MemoryAllocator::FrameAlloc<CreateTextureParams>();
		params->InitAsTexture2D(ctx->tex, width, height, kFormatR8Unorm);

		ctx->cmdBuf->Add(RendererCommand::kCommandCreateTexture, params);

		ctx->width = width;
		ctx->height = height;

		return 1;
	}

	int TofuRenderResize(void* userPtr, int width, int height)
	{
		FontRendererContext* ctx = reinterpret_cast<FontRendererContext*>(userPtr);

		ctx->cmdBuf->Add(RendererCommand::kCommandDestroyTexture, &(ctx->tex));

		return TofuRenderCreate(userPtr, width, height);
	}

	void TofuRenderUpdate(void* userPtr, int* rect, const unsigned char* data)
	{
		FontRendererContext* ctx = reinterpret_cast<FontRendererContext*>(userPtr);
		int w = rect[2] - rect[0];
		int h = rect[3] - rect[1];

		if (!ctx->tex)
			return;

		uint8_t* ptr = const_cast<unsigned char*>(data);
		ptr += rect[0] + rect[1] * ctx->width;

		UpdateTextureParams* params = MemoryAllocator::FrameAlloc<UpdateTextureParams>();
		params->handle = ctx->tex;
		params->data = ptr;
		params->pitch = ctx->width;
		params->left = rect[0];
		params->top = rect[1];
		params->right = rect[2];
		params->bottom = rect[3];
		params->front = 0;
		params->back = 1;
		
		ctx->cmdBuf->Add(RendererCommand::kCommandUpdateTexture, params);
	}

	void TofuRenderDraw(void* userPtr, const float* verts, const float* tcoords, const unsigned int* colors, int nverts)
	{
		FontRendererContext* ctx = reinterpret_cast<FontRendererContext*>(userPtr);

		if (ctx->numVerts + nverts > ctx->maxVerts) return;

		for (uint32_t i = 0; i < nverts; i++)
		{
			uint16_t vid = uint16_t(ctx->numVerts + i);
			float* vert = ctx->vertices + (vid * 9);
		
			*(vert + 0) = verts[i * 2 + 0];
			*(vert + 1) = verts[i * 2 + 1];
			*(vert + 2) = 0.0f;
		
			*(vert + 3) = 1.0f;
			*(vert + 4) = 1.0f;
			*(vert + 5) = 1.0f;
			*(vert + 6) = 1.0f;
		
			*(vert + 7) = tcoords[i * 2 + 0];
			*(vert + 8) = tcoords[i * 2 + 1];
		
			*(ctx->indices + vid) = vid;
		}

		ctx->numVerts += nverts;
	}

	void TofuRenderDelete(void* userPtr)
	{
		FontRendererContext* ctx = reinterpret_cast<FontRendererContext*>(userPtr);

		ctx->cmdBuf->Add(RendererCommand::kCommandDestroyTexture, &(ctx->tex));
	}
}

namespace tofu
{

	int32_t FontRenderer::Setup(PipelineStateHandle pso, TextureHandle tex, SamplerHandle samp, BufferHandle vb, BufferHandle ib, BufferHandle cb, uint32_t maxVertices)
	{
		this->pso = pso;
		context.tex = tex;
		this->samp = samp;
		this->vb = vb;
		this->ib = ib;
		this->cb = cb;
		context.maxVerts = maxVertices;

		return kOK;
	}

	int32_t FontRenderer::Init()
	{
		FONSparams params = {};
		params.width = 1024;
		params.height = 1024;
		params.flags = FONS_ZERO_TOPLEFT;
		params.userPtr = &context;

		params.renderCreate = TofuRenderCreate;
		params.renderResize = TofuRenderResize;
		params.renderUpdate = TofuRenderUpdate;
		params.renderDraw = TofuRenderDraw;
		params.renderDelete = TofuRenderDelete;

		fonsContext = fonsCreateInternal(&params);
		if (nullptr == fonsContext)
			return kErrUnknown;
		
		//font = fonsAddFont(fonsContext, "Arial Regular", "C:\\Windows\\Fonts\\arial.ttf");
		font = fonsAddFont(fonsContext, "sans", "D:\\DroidSerif-Regular.ttf");
		if (font == FONS_INVALID) {
			return kErrUnknown;
		}

		return kOK;
	}

	int32_t FontRenderer::Shutdown()
	{
		fonsDeleteInternal(fonsContext);
		return kOK;
	}

	int32_t FontRenderer::Reset(RendererCommandBuffer* cmdBuf)
	{
		//if (nullptr == fonsContext) return kOK;

		context.cmdBuf = cmdBuf;

		context.vertices = reinterpret_cast<float*>(
			MemoryAllocator::FrameAlloc(context.maxVerts * sizeof(float) * 9u)
			);

		context.indices = reinterpret_cast<uint16_t*>(
			MemoryAllocator::FrameAlloc(context.maxVerts * sizeof(uint16_t))
			);

		context.numVerts = 0;

		return kOK;
	}

	int32_t FontRenderer::Render(const char * text, float x, float y)
	{
		float lh = 0;
		fonsClearState(fonsContext);
		fonsSetSize(fonsContext, 18.0f);
		fonsSetFont(fonsContext, font);
		fonsVertMetrics(fonsContext, nullptr, nullptr, &lh);
		fonsSetColor(fonsContext, 0xffffffffu);

		x = fonsDrawText(fonsContext, x, y + lh, text, nullptr);

		return kOK;
	}

	int32_t FontRenderer::Submit()
	{
		if (context.numVerts == 0) return kOK;

		{
			UpdateBufferParams* params = MemoryAllocator::FrameAlloc<UpdateBufferParams>();
			params->handle = vb;
			params->data = context.vertices;
			params->size = context.numVerts * sizeof(float) * 9u;
			context.cmdBuf->Add(RendererCommand::kCommandUpdateBuffer, params);
		}

		{
			UpdateBufferParams* params = MemoryAllocator::FrameAlloc<UpdateBufferParams>();
			params->handle = ib;
			params->data = context.indices;
			params->size = context.numVerts * sizeof(uint16_t);
			context.cmdBuf->Add(RendererCommand::kCommandUpdateBuffer, params);
		}

		{
			DrawParams* params = MemoryAllocator::FrameAlloc<DrawParams>();

			params->pipelineState = pso;
			params->vertexBuffer = vb;
			params->indexBuffer = ib;

			params->vsConstantBuffers[0] = { cb, 0, 0 };
			params->psShaderResources[0] = context.tex;
			params->psSamplers[0] = samp;

			params->indexCount = context.numVerts;
			
			context.cmdBuf->Add(RendererCommand::kCommandDraw, params);
		}

		return kOK;
	}
}