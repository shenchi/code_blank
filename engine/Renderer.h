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
			kCommandCreateComputeShader,
			kCommandDestroyComputeShader,
			kCommandCreatePipelineState,
			kCommandDestroyPipelineState,
			kCommandClearRenderTargets,
			kCommandDraw,
			kCommandCompute,
			kMaxRendererCommands
		};
	};

	enum ResourceLabel
	{
		kResourceGlobal = 1,
		kResourceLevel = 2,

		kResourceMaskAll = kResourceGlobal | kResourceLevel
	};

	enum PixelFormat
	{
		kFormatAuto,
		kFormatR8g8b8a8Unorm,
		kFormatR8g8b8a8Snorm,
		kFormatR16g16b16a16Unorm,
		kFormatR16g16b16a16Snorm,
		kFormatR32g32b32a32Float,
		kFormatR8Sint,
		kFormatR16Sint,
		kFormatR32Sint,
		kFormatR8Uint,
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
		kBindingVertexBuffer = 1u << 0u,
		kBindingIndexBuffer = 1u << 1u,
		kBindingConstantBuffer = 1u << 2u,
		kBindingShaderResource = 1u << 3u,
		kBindingRenderTarget = 1u << 5u,
		kBindingDepthStencil = 1u << 6u,
		kBindingUnorderedAccess = 1u << 7u,
	};

	enum VertexFormat
	{
		kVertexFormatNormal,
		kVertexFormatSkinned,
		kVertexFormatOverlay
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

	enum TextureFilter
	{
		kTextureFilterPoint,
		kTextureFilterLinear,
		kTextureFilterAnisotropic,
		kMaxTextureFilters
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
		static RendererCommandBuffer* Create(uint32_t capacity);

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
		uint32_t			label;
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

		union
		{
			struct
			{
				uint32_t	dynamic : 1;
				uint32_t	cubeMap : 1;
				// if set, data: file's content; width: file's size, other fields are ignored
				uint32_t	isFile : 1;
				uint32_t	isSlice : 1; // it's a slice (resource view) of another resource
				uint32_t	_reserved : 4;
				uint32_t	format : 8;
				uint32_t	arraySize : 8;
				uint32_t	bindingFlags : 8;
			};
			uint32_t		attributes;
		};
		union
		{
			uint32_t		width;
			TextureHandle	sliceSource;
		};
		union
		{
			uint32_t		height;
			uint32_t		sliceStartArrayIndex;
		};
		uint32_t			depth;
		uint32_t			pitch;
		uint32_t			slicePitch;
		uint32_t			label;
		void*				data;

		CreateTextureParams()
			:
			handle(),
			attributes(),
			width(),
			height(),
			depth(),
			pitch(),
			slicePitch(),
			label(),
			data()
		{}

		TF_INLINE void InitAsTextureFromFile(TextureHandle handle, void* data, uint32_t size)
		{
			handle = handle;
			bindingFlags = kBindingShaderResource;
			isFile = 1;
			data = data;
			width = static_cast<uint32_t>(size);
		}

		TF_INLINE void InitAsTexture2D(TextureHandle handle, uint32_t width, uint32_t height, PixelFormat format = kFormatR8g8b8a8Unorm, uint32_t arraySize = 1, uint32_t binding = kBindingShaderResource)
		{
			this->handle = handle;
			this->bindingFlags = binding;
			this->format = format;
			this->arraySize = arraySize;
			this->width = width;
			this->height = height;
		}

		TF_INLINE void InitAsTexture2D(TextureHandle handle, uint32_t width, uint32_t height, PixelFormat format, void* data, uint32_t pitch, uint32_t arraySize = 1, uint32_t binding = kBindingShaderResource)
		{
			this->handle = handle;
			this->bindingFlags = binding;
			this->format = format;
			this->arraySize = arraySize;
			this->width = width;
			this->height = height;
			this->pitch = pitch;
			this->data = data;
		}

		TF_INLINE void InitAsTextureCubeMap(TextureHandle handle, uint32_t width, uint32_t height, PixelFormat format = kFormatR8g8b8a8Unorm, uint32_t binding = kBindingShaderResource)
		{
			this->handle = handle;
			this->bindingFlags = binding;
			this->format = format;
			this->cubeMap = 1;
			this->arraySize = 6;
			this->width = width;
			this->height = height;
		}

		TF_INLINE void InitAsTexture2DSlice(TextureHandle handle, TextureHandle sliceSource, uint32_t startArrayIndex, uint32_t arraySize, PixelFormat format = kFormatAuto, uint32_t binding = kBindingShaderResource)
		{
			this->handle = handle;
			this->bindingFlags = binding;
			this->format = format;
			this->arraySize = arraySize;
			this->isSlice = 1;
			this->sliceSource = sliceSource;
			this->sliceStartArrayIndex = startArrayIndex;
		}

		TF_INLINE void InitAsTexture3D(TextureHandle handle, uint32_t width, uint32_t height, uint32_t depth, PixelFormat format = kFormatR8g8b8a8Unorm, uint32_t binding = kBindingShaderResource)
		{
			this->handle = handle;
			this->bindingFlags = binding;
			this->format = format;
			this->width = width;
			this->height = height;
			this->depth = depth;
		}

		TF_INLINE void InitAsTexture3D(TextureHandle handle, uint32_t width, uint32_t height, uint32_t depth, PixelFormat format, void* data, uint32_t pitch, uint32_t slicePitch, uint32_t binding = kBindingShaderResource)
		{
			this->handle = handle;
			this->bindingFlags = binding;
			this->format = format;
			this->width = width;
			this->height = height;
			this->depth = depth;
			this->pitch = pitch;
			this->slicePitch = slicePitch;
			this->data = data;
		}

	};

	struct UpdateTextureParams
	{
		TextureHandle		handle;
		uint32_t			subRes;
		uint32_t			pitch;
		uint32_t			slicePitch;

		uint32_t			left;
		uint32_t			top;
		uint32_t			front;
		uint32_t			right;
		uint32_t			bottom;
		uint32_t			back;

		void*				data;
	};

	struct CreateSamplerParams
	{
		SamplerHandle		handle;
		
		union
		{
			struct
			{
				uint32_t			filter : 3;
				uint32_t			textureAddressU : 3;
				uint32_t			textureAddressV : 3;
				uint32_t            textureAddressW : 3;
				uint32_t			maxAnisotropy : 5;
				uint32_t			_reserved1 : 15;
			};
			uint32_t				textureAddress;
		};

		uint32_t					label;

		CreateSamplerParams()
			:
			filter(kTextureFilterLinear),
			textureAddressU(kTextureAddressClamp),
			textureAddressV(kTextureAddressClamp),
			textureAddressW(kTextureAddressClamp),
			maxAnisotropy(1),
			label()
		{}
	};

	struct CreateVertexShaderParams
	{
		VertexShaderHandle	handle;
		void*				data;
		size_t				size;
		uint32_t			label;
	};

	struct CreatePixelShaderParams
	{
		PixelShaderHandle	handle;
		void*				data;
		size_t				size;
		uint32_t			label;
	};

	struct CreateComputeShaderParams
	{
		ComputeShaderHandle	handle;
		void*				data;
		size_t				size;
		uint32_t			label;
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

		struct
		{
			float					topLeftX;
			float					topLeftY;
			float					width;
			float					height;
			float					minZ;
			float					maxZ;
		}							viewport;

		uint32_t					label;

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
			blendWriteMask(kColorWriteAll),
			viewport(),
			label()
		{}
	};

	struct ClearParams
	{
		TextureHandle		renderTargets[kMaxRenderTargetBindings];
		float				clearColor[4];
		TextureHandle		depthRenderTarget;
		float				clearDepth;
		uint8_t				clearStencil;

		inline ClearParams()
			:
			renderTargets(),
			clearColor(),
			depthRenderTarget(kMaxTextures),
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
		uint32_t				instanceCount;
		ConstantBufferBinding	vsConstantBuffers[kMaxConstantBufferBindings];
		ConstantBufferBinding	psConstantBuffers[kMaxConstantBufferBindings];
		BaseHandle				vsShaderResources[kMaxTextureBindings];
		BaseHandle				psShaderResources[kMaxTextureBindings];
		SamplerHandle			vsSamplers[kMaxSamplerBindings];
		SamplerHandle			psSamplers[kMaxSamplerBindings];
		TextureHandle			renderTargets[kMaxRenderTargetBindings];
		TextureHandle			depthRenderTarget;

		static const TextureHandle DefaultRenderTarget;

		DrawParams()
			:
			pipelineState(),
			vertexBuffer(),
			indexBuffer(),
			startIndex(),
			startVertex(),
			indexCount(),
			instanceCount(),
			vsConstantBuffers(),
			psConstantBuffers(),
			vsShaderResources(),
			psShaderResources(),
			vsSamplers(),
			psSamplers(),
			renderTargets(),
			depthRenderTarget(DefaultRenderTarget)
		{
			renderTargets[0] = DefaultRenderTarget;
		}
	};

	struct ComputeParams
	{
		ComputeShaderHandle		shader;
		uint32_t				threadGroupCountX;
		uint32_t				threadGroupCountY;
		uint32_t				threadGroupCountZ;
		ConstantBufferBinding	constantBuffers[kMaxConstantBufferBindings];
		BaseHandle				rwShaderResources[kMaxTextureBindings];
		BaseHandle				shaderResources[kMaxTextureBindings];
		SamplerHandle			samplers[kMaxSamplerBindings];
	};

	class Renderer
	{
	public:
		virtual ~Renderer() {}

		virtual int32_t Init() = 0;

		virtual int32_t Release() = 0;

		virtual int32_t Submit(RendererCommandBuffer* buffer) = 0;

		virtual int32_t Present() = 0;

		virtual int32_t CleanupResources(uint32_t labelMask) = 0;

		virtual int32_t GetFrameBufferSize(int32_t& width, int32_t& height) = 0;

		static Renderer* CreateRenderer();
	};

}

