#pragma once

#include "Common.h"

namespace tofu
{
	struct RendererCommand
	{
		enum
		{
			kCommandNone,
			kCommandCreateBuffer,
			kCommandUpdateBuffer,
			kCommandDestroyBuffer,
			kCommandCreateTexture,
			kCommandUpdateTexture,
			kCommandDestroyTexture,
			kCommandCreateSampler,
			kCommandDestroySampler,
			kCommandCreateVertexShader,
			kCommandDestroyVertexShader,
			kCommandCreatePixelShader,
			kCommandDestroyPixelShader,
			kCommandCreatePipelineState,
			kCommandDestroyPipelineState,
			kCommandClearRenderTargets,
			kCommandDraw,
			kMaxRendererCommands
		};
	};

	enum PixelFormat
	{
		kFormatAuto,
		kFormatR8g8b8a8Unorm,
		kFormatR8g8b8a8Snorm,
		kFormatR16g16b16a16Unorm,
		kFormatR16g16b16a16Snorm,
		kFormatR32g32b32a32Float,
		kFormatR16Sint,
		kFormatR32Sint,
		kFormatR16Uint,
		kFormatR32Uint,
		kFormatD24UnormS8Uint,
		kMaxPixelFormats
	};

	enum CullMode
	{
		kCullNone,
		kCullFront,
		kCullBack
	};

	enum ComparisonFunc
	{
		kComparisonNever,
		kComparisonLess,
		kComparisonEqual,
		kComparisonLEqual,
		kComparisonGreater,
		kComparisonNotEqual,
		kComparisonGEqual,
		kComparisonAlways
	};

	enum BindingFlag
	{
		kBindingVertexBuffer = 1 << 0,
		kBindingIndexBuffer = 1 << 1,
		kBindingConstantBuffer = 1 << 2,
		kBindingShaderResource = 1 << 3,
		kBindingRenderTarget = 1 << 5,
		kBindingDepthStencil = 1 << 6,
	};

	enum VertexFormat
	{
		kVertexFormatNormal,
		kVertexFormatSkinned
	};

	enum StencilOp
	{
		kStencilOpKeep = 1,
		kStencilOpZero,
		kStencilOpReplace,
		kStencilOpIncSat,
		kStencilOpDecSat,
		kStencilOpInvert,
		kStencilOpInc,
		kStencilOpDec,
	};

	enum Blend
	{
		kBlendZero = 1,
		kBlendOne,
		kBlendSrcColor,
		kBlendInvSrcColor,
		kBlendSrcAlpha,
		kBlendInvSrcAlpha,
		kBlendDestAlpha,
		kBlendInvDestAlpha,
		kBlendDestColor,
		kBlendInvDestColor,
		kBlendSrcAlphaSature,
		kBlendFactor = 14,
		kBlendInvFactor,
		kBlendSrc1Color,
		kBlendInvSrc1Color,
		kBlendSrc1Alpha,
		kBlendInvSrc1Alpha
	};

	enum BlendOp
	{
		kBlendOpAdd = 1,
		kBlendOpSubtract,
		kBlendOpRevSubtract,
		kBlendOpMin,
		kBlendOpMax
	};

	enum ColorWriteMask
	{
		kColorWriteNone = 0,
		kColorWriteRed = 1,
		kColorWriteGreen = 2,
		kColorWriteBlue = 4,
		kColorWriteAlpha = 8,
		kColorWriteAll = kColorWriteRed | kColorWriteGreen | kColorWriteBlue | kColorWriteAlpha
	};

	enum TextureAdressMode
	{
		kTextureAddressWarp = 1,
		kTextureAddressMirror,
		kTextureAddressClamp,
		kTextureAddressBorder
	};

	struct RendererCommandBuffer
	{
		uint32_t*			cmds;
		void**				params;

		uint32_t			capacity;
		uint32_t			size;

		// create a new command buffer from allocator[allocNo]
		static RendererCommandBuffer* Create(uint32_t capacity, uint32_t allocNo);

		// append a command into the command buffer
		void Add(uint32_t cmd, void* param);
	};

	struct CreateBufferParams
	{
		BufferHandle		handle;
		uint32_t			dynamic : 1;
		uint32_t			format : 15;
		uint32_t			bindingFlags : 16;
		uint32_t			size;
		uint32_t			stride;
		void*				data;
	};

	struct UpdateBufferParams
	{
		BufferHandle		handle;
		uint32_t			offset;
		uint32_t			size;
		void*				data;
	};

	struct CreateTextureParams
	{
		TextureHandle		handle;
		uint32_t			dynamic : 1;
		uint32_t			cubeMap : 1;
		// if set, data: file's content; width: file's size, other fields are ignored
		uint32_t			isFile : 1;
		uint32_t			_reserved : 5;
		uint32_t			format : 8;
		uint32_t			arraySize : 8;
		uint32_t			bindingFlags : 8;
		uint32_t			width;
		uint32_t			height;
		uint32_t			pitch;
		void*				data;
	};

	struct UpdateTextureParams
	{
		TextureHandle		handle;
		uint32_t			pitch;
		void*				data;
	};

	struct CreateSamplerParams
	{
		SamplerHandle		handle;
		
		union
		{
			struct
			{
				uint32_t			textureAddressU : 3;
				uint32_t			textureAddressV : 3;
				uint32_t            textureAddressW : 3;
				uint32_t			_reserved1 : 23;
			};
			uint32_t				textureAddress;
		};
	};

	struct CreateVertexShaderParams
	{
		VertexShaderHandle	handle;
		void*				data;
		size_t				size;
	};

	struct CreatePixelShaderParams
	{
		PixelShaderHandle	handle;
		void*				data;
		size_t				size;
	};

	struct CreatePipelineStateParams
	{
		PipelineStateHandle	handle;
		VertexFormat		vertexFormat;
		VertexShaderHandle	vertexShader;
		PixelShaderHandle	pixelShader;

		union
		{
			struct
			{
				uint64_t			depthEnable : 1;
				uint64_t			depthWrite : 1;
				uint64_t			depthFunc : 3;
				uint64_t			stencilEnable : 1;
				uint64_t			frontFaceFailOp : 4;
				uint64_t			frontFaceDepthFailOp : 4;
				uint64_t			frontFacePassOp : 4;
				uint64_t			frontFaceFunc : 3;
				uint64_t			backFaceFailOp : 4;
				uint64_t			backFaceDepthFailOp : 4;
				uint64_t			backFacePassOp : 4;
				uint64_t			backFaceFunc : 3;
				uint64_t			_reserved1 : 4;
				uint64_t			stencilRef : 8;
				uint64_t			stencilReadMask : 8;
				uint64_t			stencilWriteMask : 8;
			};
			uint64_t				depthStencilState;
		};

		union
		{
			struct
			{
				uint32_t			cullMode : 2;
				uint32_t			_reserved2 : 30;
			};
			uint32_t				rasterizerState;
		};

		union
		{
			struct
			{
				uint32_t			blendEnable : 1;
				uint32_t			srcBlend : 5;
				uint32_t			destBlend : 5;
				uint32_t			blendOp : 3;
				uint32_t			srcBlendAlpha : 5;
				uint32_t			destBlendAlpha : 5;
				uint32_t			blendOpAlpha : 3;
				uint32_t			blendWriteMask : 5;
			};
			uint32_t				blendState;
		};

		CreatePipelineStateParams()
			:
			handle(),
			vertexFormat(kVertexFormatNormal),
			vertexShader(),
			pixelShader(),
			depthEnable(1),
			depthWrite(1),
			depthFunc(kComparisonLess),
			stencilEnable(0),
			frontFaceFailOp(kStencilOpKeep),
			frontFaceDepthFailOp(kStencilOpKeep),
			frontFacePassOp(kStencilOpKeep),
			frontFaceFunc(kComparisonAlways),
			backFaceFailOp(kStencilOpKeep),
			backFaceDepthFailOp(kStencilOpKeep),
			backFacePassOp(kStencilOpKeep),
			backFaceFunc(kComparisonAlways),
			stencilRef(0),
			stencilReadMask(255u),
			stencilWriteMask(255u),
			cullMode(kCullBack),
			blendEnable(0),
			srcBlend(kBlendOne),
			destBlend(kBlendZero),
			blendOp(kBlendOpAdd),
			srcBlendAlpha(kBlendOne),
			destBlendAlpha(kBlendZero),
			blendOpAlpha(kBlendOpAdd),
			blendWriteMask(kColorWriteAll)
		{}
	};

	struct ClearParams
	{
		TextureHandle		renderTargets[kMaxRenderTargetBindings];
		uint32_t			renderTargetSubIds[kMaxRenderTargetBindings];
		float				clearColor[4];
		TextureHandle		depthRenderTarget;
		uint32_t			depthRenderTargetSubId;
		float				clearDepth;
		uint8_t				clearStencil;

		inline ClearParams()
			:
			renderTargets(),
			renderTargetSubIds(),
			clearColor(),
			depthRenderTarget(kMaxTextures),
			depthRenderTargetSubId(0),
			clearDepth(1.0f),
			clearStencil(0u)
		{
			renderTargets[0] = TextureHandle(kMaxTextures + 1);
		}

		inline void SetClearColor(float r, float g, float b, float a)
		{
			clearColor[0] = r;
			clearColor[1] = g;
			clearColor[2] = b;
			clearColor[3] = a;
		}
	};

	struct ConstantBufferBinding
	{
		BufferHandle		bufferHandle;
		uint16_t			offsetInVectors;
		uint16_t			sizeInVectors;
	};

	struct DrawParams
	{
		PipelineStateHandle		pipelineState;
		BufferHandle			vertexBuffer;
		BufferHandle			indexBuffer;
		uint32_t				startIndex;
		uint32_t				startVertex;
		uint32_t				indexCount;
		ConstantBufferBinding	vsConstantBuffers[kMaxConstantBufferBindings];
		ConstantBufferBinding	psConstantBuffers[kMaxConstantBufferBindings];
		TextureHandle			vsTextures[kMaxTextureBindings];
		TextureHandle			psTextures[kMaxTextureBindings];
		SamplerHandle			vsSamplers[kMaxSamplerBindings];
		SamplerHandle			psSamplers[kMaxSamplerBindings];
		TextureHandle			renderTargets[kMaxRenderTargetBindings];
		uint32_t				renderTargetSubIds[kMaxRenderTargetBindings];
		TextureHandle			depthRenderTarget;
		uint32_t				depthRenderTargetSubId;

		static const TextureHandle DefaultRenderTarget;

		DrawParams()
			:
			pipelineState(),
			vertexBuffer(),
			indexBuffer(),
			startIndex(),
			startVertex(),
			indexCount(),
			vsConstantBuffers(),
			psConstantBuffers(),
			vsTextures(),
			psTextures(),
			vsSamplers(),
			psSamplers(),
			renderTargets(),
			renderTargetSubIds(),
			depthRenderTarget(DefaultRenderTarget),
			depthRenderTargetSubId(0)
		{
			renderTargets[0] = DefaultRenderTarget;
		}
	};

	class Renderer
	{
	public:
		virtual ~Renderer() {}

		virtual int32_t Init() = 0;

		virtual int32_t Release() = 0;

		virtual int32_t Submit(RendererCommandBuffer* buffer) = 0;

		virtual int32_t Present() = 0;

		virtual int32_t GetFrameBufferSize(int32_t& width, int32_t& height) = 0;

		static Renderer* CreateRenderer();
	};

}

