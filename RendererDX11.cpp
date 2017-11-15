#include "Renderer.h"

#include "NativeContext.h"

#include "MemoryAllocator.h"

#include <Windows.h>
#include <d3d11_1.h>

#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>

#include <cassert>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#define RELEASE(x) if (nullptr != (x)) { (x)->Release(); (x) = nullptr; }
#define C(x, ret) if ((x) != S_OK) { return (ret); }
#define DXCHECKED(x) C(x, TF_UNKNOWN_ERR)

namespace
{
	DXGI_FORMAT InputSlotTypeTable[][4] =
	{
		// TYPE_FLOAT
		{ DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT },
		// TYPE_INT8
		{ DXGI_FORMAT_R8_SINT, DXGI_FORMAT_R8G8_SINT, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R8G8B8A8_SINT },
		// TYPE_UINT8
		{ DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R8G8_UINT, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R8G8B8A8_UINT },
		// TYPE_INT16
		{ DXGI_FORMAT_R16_SINT, DXGI_FORMAT_R16G16_SINT, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R16G16B16A16_SINT },
		// TYPE_UINT16
		{ DXGI_FORMAT_R16_UINT, DXGI_FORMAT_R16G16_UINT, DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R16G16B16A16_UINT },
		// TYPE_INT32
		{ DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32B32_SINT, DXGI_FORMAT_R32G32B32A32_SINT },
		// TYPE_UINT32
		{ DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32B32_UINT, DXGI_FORMAT_R32G32B32A32_UINT },
	};

	size_t InputSlotSizeTable[] =
	{
		4, // TYPE_FLOAT
		1, // TYPE_INT8
		1, // TYPE_UINT8
		2, // TYPE_INT16
		2, // TYPE_UINT16
		4, // TYPE_INT32
		4, // TYPE_UINT32
	};

	DXGI_FORMAT IndexTypeTable[] =
	{
		DXGI_FORMAT_UNKNOWN, // TYPE_FLOAT
		DXGI_FORMAT_UNKNOWN, // TYPE_INT8
		DXGI_FORMAT_UNKNOWN, // TYPE_UINT8
		DXGI_FORMAT_UNKNOWN, // TYPE_INT16
		DXGI_FORMAT_R16_UINT, // TYPE_UINT16
		DXGI_FORMAT_UNKNOWN, // TYPE_INT32
		DXGI_FORMAT_R32_UINT, // TYPE_UINT32
	};

	LPSTR InputSemanticsTable[] =
	{
		"POSITION",
		"COLOR",
		"NORMAL",
		"TANGENT",
		"BINORMAL",
		"TEXCOORD",
		"TEXCOORD",
		"TEXCOORD",
		"TEXCOORD",
	};

	UINT InputSemanticsIndex[] =
	{
		0,
		0,
		0,
		0,
		0,
		0,
		1,
		2,
		3,
	};

	D3D11_PRIMITIVE_TOPOLOGY PrimitiveTypeTable[] =
	{
		D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,
		D3D11_PRIMITIVE_TOPOLOGY_LINELIST,
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,
	};

	DXGI_FORMAT PixelFormatTable[tofu::NUM_PIXEL_FORMAT] =
	{
		DXGI_FORMAT_UNKNOWN, // AUTO
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_R8G8B8A8_SNORM,
		DXGI_FORMAT_R16G16B16A16_UNORM,
		DXGI_FORMAT_R16G16B16A16_SNORM,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_R16_SINT,
		DXGI_FORMAT_R32_SINT,
		DXGI_FORMAT_R16_UINT,
		DXGI_FORMAT_R32_UINT,
		DXGI_FORMAT_D24_UNORM_S8_UINT,
	};

	D3D11_INPUT_ELEMENT_DESC InputElemDescNormal[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3D11_INPUT_ELEMENT_DESC InputElemDescSkinned[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_SINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3D11_INPUT_ELEMENT_DESC* InputElemDescTable[] = { InputElemDescNormal, InputElemDescSkinned };
	UINT InputElemDescSizeTable[] = { 4u, 6u };

	struct Buffer
	{
		ID3D11Buffer*				buf;
		ID3D11ShaderResourceView*	srv;
		uint32_t					dynamic : 1;
		uint32_t					format : 15;
		uint32_t					bindingFlags : 16;
		uint32_t					size;
		uint32_t					stride;
	};

	struct Texture
	{
		ID3D11Texture2D*			tex;
		ID3D11ShaderResourceView*	srv;
		ID3D11RenderTargetView*		rtv;
		ID3D11DepthStencilView*		dsv;
		uint32_t					dynamic : 1;
		uint32_t					cubeMap : 1;
		uint32_t					_padding : 6;
		uint32_t					format : 8;
		uint32_t					arraySize : 8;
		uint32_t					bindingFlags : 8;
		uint32_t					width;
		uint32_t					height;
		uint32_t					pitch;
	};

	struct Sampler
	{
		ID3D11SamplerState*			samp;
	};

	struct VertexShader
	{
		ID3D11VertexShader*			shader;
		void*						data;
		size_t						size;
	};

	struct PixelShader
	{
		ID3D11PixelShader*			shader;
		void*						data;
		size_t						size;
	};

	struct PipelineState
	{
		ID3D11RasterizerState*		rasterizerState;
		ID3D11DepthStencilState*	depthStencilState;
		ID3D11BlendState*			blendState;
		ID3D11InputLayout*			inputLayout;
		tofu::VertexShaderHandle	vertexShader;
		tofu::PixelShaderHandle		pixelShader;
		D3D11_VIEWPORT				viewport;
	};
}

namespace tofu
{
	namespace dx11
	{

		class RendererDX11 : public Renderer
		{
		public:

			virtual int32_t Init() override
			{
				hWnd = reinterpret_cast<HWND>(NativeContext::instance()->GetContextHandle());

				int32_t result = 0;

				if (TF_OK != (result = CreateDevice()))
				{
					return result;
				}

				if (TF_OK != (result = InitRenderTargets()))
				{
					return result;
				}

				if (TF_OK != (result = InitPipelineStates()))
				{
					return result;
				}

				return TF_OK;
			}

			virtual int32_t Release() override
			{
#define ReleaseResources(RES_ARRAY, MAX_COUNT, MEMBER_TO_CHECK, DESTROY_FUNC) \
				for (uint32_t i = 0; i < MAX_COUNT; i++) \
				{ \
					if (nullptr != RES_ARRAY[i].MEMBER_TO_CHECK) \
					{ \
						DESTROY_FUNC(&i); \
					} \
				} 

				ReleaseResources(buffers, MAX_BUFFERS, buf, DestroyBuffer);
				ReleaseResources(textures, MAX_TEXTURES, tex, DestroyTexture);
				ReleaseResources(samplers, MAX_SAMPLERS, samp, DestroySampler);
				ReleaseResources(vertexShaders, MAX_VERTEX_SHADERS, shader, DestroyVertexShader);
				ReleaseResources(pixelShaders, MAX_PIXEL_SHADERS, shader, DestroyPixelShader);
				ReleaseResources(pipelineStates, MAX_PIPELINE_STATES, depthStencilState, DestroyPipelineState);

#undef ReleaseResources

				{
					Texture& rt = textures[MAX_TEXTURES + 1];
					RELEASE(rt.srv);
					RELEASE(rt.rtv);
					RELEASE(rt.dsv);
					RELEASE(rt.tex);

					Texture& ds = textures[MAX_TEXTURES];
					RELEASE(ds.srv);
					RELEASE(ds.rtv);
					RELEASE(ds.dsv);
					RELEASE(ds.tex);
				}

				swapChain->Release();
				context->Release();
				device->Release();

				return TF_OK;
			}

			virtual int32_t Submit(RendererCommandBuffer* buffer) override
			{
				if (nullptr == buffer)
				{
					return TF_UNKNOWN_ERR;
				}

				for (uint32_t i = 0; i < buffer->size; ++i)
				{
					cmd_callback_t cmd = commands[buffer->cmds[i]];
					CHECKED((this->*cmd)(buffer->params[i]));
				}

				return TF_OK;
			}

			virtual int32_t Present() override
			{
				if (S_OK != swapChain->Present(0, 0))
				{
					return TF_UNKNOWN_ERR;
				}

				return TF_OK;
			}

			virtual int32_t GetFrameBufferSize(int32_t& width, int32_t& height) override
			{
				width = winWidth;
				height = winHeight;
				return TF_OK;
			}

		private:
			int							winWidth;
			int							winHeight;

			HWND						hWnd;

			IDXGISwapChain1*			swapChain;
			ID3D11Device1*				device;
			ID3D11DeviceContext1*		context;

			Buffer						buffers[MAX_BUFFERS];
			Texture						textures[MAX_TEXTURES + 2];
			Sampler						samplers[MAX_SAMPLERS];
			VertexShader				vertexShaders[MAX_VERTEX_SHADERS];
			PixelShader					pixelShaders[MAX_PIXEL_SHADERS];
			PipelineState				pipelineStates[MAX_PIPELINE_STATES];

			PipelineStateHandle			currentPipelineState;

			typedef int32_t(RendererDX11::*cmd_callback_t)(void*);

			cmd_callback_t				commands[RendererCommand::MaxRendererCommands] =
			{
				&RendererDX11::Nop,
				&RendererDX11::CreateBuffer,
				&RendererDX11::UpdateBuffer,
				&RendererDX11::DestroyBuffer,
				&RendererDX11::CreateTexture,
				&RendererDX11::UpdateTexture,
				&RendererDX11::DestroyTexture,
				&RendererDX11::CreateSampler,
				&RendererDX11::DestroySampler,
				&RendererDX11::CreateVertexShader,
				&RendererDX11::DestroyVertexShader,
				&RendererDX11::CreatePixelShader,
				&RendererDX11::DestroyPixelShader,
				&RendererDX11::CreatePipelineState,
				&RendererDX11::DestroyPipelineState,
				&RendererDX11::ClearRenderTargets,
				&RendererDX11::Draw
			};

		private:


			int32_t CreateDevice()
			{
				UINT creationFlags = 0;

#if _DEBUG
				creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

				HRESULT hr = S_OK;

				{
					D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0 };

					ID3D11Device* device = nullptr;
					ID3D11DeviceContext* context = nullptr;

					if (S_OK != D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, featureLevels, 2, D3D11_SDK_VERSION, &device, nullptr, &context))
					{
						return -1;
					}

					if (S_OK != device->QueryInterface(__uuidof(ID3D11Device1), (void**)&(this->device)))
					{
						device->Release();
						return -1;
					}

					if (S_OK != context->QueryInterface(__uuidof(ID3D11DeviceContext1), (void**)&(this->context)))
					{
						context->Release();
						device->Release();
						return -1;
					}

					device->Release();
					context->Release();
				}

				IDXGIFactory2* factory = nullptr;
				{
					IDXGIDevice* dxgiDevice = nullptr;
					if (S_OK == device->QueryInterface<IDXGIDevice>(&dxgiDevice))
					{
						IDXGIAdapter* adapter = nullptr;
						if (S_OK == dxgiDevice->GetAdapter(&adapter))
						{
							hr = adapter->GetParent(__uuidof(IDXGIFactory2), (void**)&factory);
							adapter->Release();
						}
						dxgiDevice->Release();
					}
					if (S_OK != hr)
					{
						context->Release();
						device->Release();
						return -1;
					}
				}


				RECT rect = {};
				GetClientRect(hWnd, &rect);
				winWidth = rect.right - rect.left;
				winHeight = rect.bottom - rect.top;

				DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};

				swapChainDesc.Width = winWidth;
				swapChainDesc.Height = winHeight;
				swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				swapChainDesc.SampleDesc.Count = 1;
				swapChainDesc.SampleDesc.Quality = 0;
				swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				swapChainDesc.BufferCount = 2;
				swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
				swapChainDesc.Flags = 0;// DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

				if (S_OK != (hr = factory->CreateSwapChainForHwnd(device, hWnd, &swapChainDesc, nullptr, nullptr, &(swapChain))))
				{
					context->Release();
					device->Release();
					factory->Release();
					return -1;
				}

				factory->Release();
				return 0;
			}

			int32_t InitRenderTargets()
			{
				HRESULT hr = S_OK;


				ID3D11Texture2D* backbufferTex = nullptr;
				if (S_OK != (hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backbufferTex)))
				{
					return TF_UNKNOWN_ERR;
				}

				D3D11_TEXTURE2D_DESC desc = {};
				backbufferTex->GetDesc(&desc);

				Texture& rt = textures[MAX_TEXTURES + 1];

				//rt.Reset(TextureType::TEXTURE_2D, PixelFormatFromDXGI(desc.Format), BINDING_RENDER_TARGET, width, height);
				rt = {};
				rt.width = winWidth;
				rt.height = winHeight;
				rt.arraySize = 1;

				if (S_OK != (hr = device->CreateRenderTargetView(backbufferTex, nullptr, &(rt.rtv))))
				{
					return -1;
				}
				rt.tex = backbufferTex;



				Texture& ds = textures[MAX_TEXTURES];

				//ds.Reset(TextureType::TEXTURE_2D, PixelFormat::FORMAT_D24_UNORM_S8_UINT, width, height);
				ds = {};
				ds.width = winWidth;
				ds.height = winHeight;
				ds.arraySize = 1;

				ID3D11Texture2D* depthStencilTex = nullptr;
				D3D11_TEXTURE2D_DESC depthDesc{ 0 };
				depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
				depthDesc.Width = winWidth;
				depthDesc.Height = winHeight;
				depthDesc.ArraySize = 1;
				depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
				depthDesc.MipLevels = 1;
				depthDesc.SampleDesc.Count = 1;
				depthDesc.SampleDesc.Quality = 0;
				if (S_OK != (hr = device->CreateTexture2D(&depthDesc, nullptr, &depthStencilTex)))
				{
					return -1;
				}

				if (S_OK != (hr = device->CreateDepthStencilView(depthStencilTex, nullptr, &(ds.dsv))))
				{
					return -1;
				}
				ds.tex = depthStencilTex;


				context->OMSetRenderTargets(1, &(rt.rtv), ds.dsv);

				return 0;
			}

			int32_t InitPipelineStates()
			{
				context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				return TF_OK;
			}

			int32_t Nop(void*) { return TF_OK; }

			int32_t CreateBuffer(void* _params)
			{
				CreateBufferParams* params = reinterpret_cast<CreateBufferParams*>(_params);

				assert(true == params->handle);
				uint32_t id = params->handle.id;

				bool isShaderResource = ((params->bindingFlags & BINDING_SHADER_RESOURCE) != 0u);

				assert(
					!isShaderResource ||
					(params->stride > sizeof(float) * 4)
				);


				assert(nullptr == buffers[id].buf);
				buffers[id] = {};

				if (params->bindingFlags & BINDING_CONSTANT_BUFFER)
				{
					params->size = ((params->size + 0xffu) & (~0xffu));
				}

				CD3D11_BUFFER_DESC bufDesc(
					params->size,
					(params->bindingFlags & 0x7u),
					(params->dynamic == 1 ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT),
					(params->dynamic == 1 ? D3D11_CPU_ACCESS_WRITE : 0U),
					(isShaderResource ? D3D11_RESOURCE_MISC_BUFFER_STRUCTURED : 0U),
					params->stride
				);

				D3D11_SUBRESOURCE_DATA subResData = {};
				subResData.pSysMem = params->data;

				if (S_OK != device->CreateBuffer(
					&bufDesc,
					(nullptr == params->data ? nullptr : &subResData),
					&(buffers[id].buf)))
				{
					return TF_UNKNOWN_ERR;
				}

				if (isShaderResource)
				{
					D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
					srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
					srvDesc.Buffer.NumElements = params->size / params->stride;
					srvDesc.Format = PixelFormatTable[params->format];
					HRESULT ret = device->CreateShaderResourceView(
						buffers[id].buf,
						&srvDesc,
						&(buffers[id].srv)
					);
					assert(S_OK == ret);
				}

				buffers[id].dynamic = params->dynamic;
				buffers[id].format = params->format;
				buffers[id].bindingFlags = params->bindingFlags & 0x7u;
				buffers[id].size = params->size;
				buffers[id].stride = params->stride;

				return TF_OK;
			}

			int32_t UpdateBuffer(void* _params)
			{
				UpdateBufferParams* params = reinterpret_cast<UpdateBufferParams*>(_params);

				assert(true == params->handle);
				uint32_t id = params->handle.id;

				assert(nullptr != buffers[id].buf);
				assert(params->size > 0 && params->offset + params->size <= buffers[id].size);

				bool updateWholeBuffer = (params->offset == 0 && params->size == buffers[id].size);

				if (buffers[id].dynamic)
				{
					if (updateWholeBuffer)
					{
						D3D11_MAPPED_SUBRESOURCE res = {};
						context->Map(buffers[id].buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
						memcpy(res.pData, params->data, params->size);
						context->Unmap(buffers[id].buf, 0);
					}
					else
					{
						D3D11_MAPPED_SUBRESOURCE res = {};
						context->Map(buffers[id].buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
						uint8_t* ptr = reinterpret_cast<uint8_t*>(res.pData);
						memcpy(ptr + params->offset, params->data, params->size);
						context->Unmap(buffers[id].buf, 0);
					}
				}
				else
				{
					assert(
						((buffers[id].bindingFlags & BINDING_CONSTANT_BUFFER) == 0) ||
						(params->offset == 0)
					);
					D3D11_BOX box = {};
					box.left = params->offset;
					box.right = params->offset + params->size;

					context->UpdateSubresource(
						buffers[id].buf, 0,
						(updateWholeBuffer ? nullptr : &box),
						params->data,
						0,
						0);
				}

				return TF_OK;
			}

			int32_t DestroyBuffer(void* params)
			{
				BufferHandle* handle = reinterpret_cast<BufferHandle*>(params);

				assert(true == *handle);
				uint32_t id = handle->id;

				assert(nullptr != buffers[id].buf);

				buffers[id].buf->Release();

				if (nullptr != buffers[id].srv)
				{
					buffers[id].srv->Release();
				}

				buffers[id] = {};

				return TF_OK;
			}

			int32_t CreateTexture(void* _params)
			{
				CreateTextureParams* params = reinterpret_cast<CreateTextureParams*>(_params);

				assert(true == params->handle && params->handle.id < MAX_TEXTURES);

				uint32_t id = params->handle.id;

				assert(nullptr == textures[id].tex);

				uint32_t bindingFlags = params->bindingFlags & (BINDING_SHADER_RESOURCE | BINDING_RENDER_TARGET | BINDING_DEPTH_STENCIL);

				if (params->isFile)
				{
					assert(bindingFlags & BINDING_SHADER_RESOURCE);

					ID3D11Resource* res = nullptr;
					DXCHECKED(DirectX::CreateDDSTextureFromMemoryEx(
						device,
						reinterpret_cast<uint8_t*>(params->data),
						static_cast<size_t>(params->width),
						0u,
						(params->dynamic == 1 ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT),
						bindingFlags,
						(params->dynamic == 1 ? D3D11_CPU_ACCESS_WRITE : 0U),
						0u,
						false,
						&(res),
						&(textures[id].srv)
					));

					DXCHECKED(res->QueryInterface<ID3D11Texture2D>(&(textures[id].tex)));
					res->Release();
				}
				else
				{
					CD3D11_TEXTURE2D_DESC texDesc(
						PixelFormatTable[params->format],
						params->width,
						params->height,
						params->arraySize,
						0U,
						bindingFlags,
						(params->dynamic == 1 ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT),
						(params->dynamic == 1 ? D3D11_CPU_ACCESS_WRITE : 0U),
						1U,
						0U,
						(params->cubeMap == 1 ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0U)

					);

					D3D11_SUBRESOURCE_DATA subResData = {};
					subResData.pSysMem = params->data;
					subResData.SysMemPitch = params->pitch;

					DXCHECKED(device->CreateTexture2D(
						&texDesc,
						(nullptr == params->data ? nullptr : &subResData),
						&(textures[id].tex)));

					if (params->bindingFlags & BINDING_SHADER_RESOURCE)
					{
						DXCHECKED(device->CreateShaderResourceView(textures[id].tex, nullptr, &(textures[id].srv)));
					}
				}

				if (params->bindingFlags & BINDING_RENDER_TARGET)
				{
					DXCHECKED(device->CreateRenderTargetView(textures[id].tex, nullptr, &(textures[id].rtv)));
				}

				if (params->bindingFlags & BINDING_DEPTH_STENCIL)
				{
					DXCHECKED(device->CreateDepthStencilView(textures[id].tex, nullptr, &(textures[id].dsv)));
				}

				D3D11_TEXTURE2D_DESC desc = {};
				textures[id].tex->GetDesc(&desc);

				textures[id].dynamic = params->dynamic;
				textures[id].cubeMap = ((desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) != 0 ? 1 : params->cubeMap);
				textures[id].format = params->isFile ? FORMAT_AUTO : params->format;
				textures[id].arraySize = desc.ArraySize;
				textures[id].bindingFlags = bindingFlags;
				textures[id].width = desc.Width;
				textures[id].height = desc.Height;
				textures[id].pitch = params->pitch;

				return TF_OK;
			}

			int32_t UpdateTexture(void* _params)
			{
				UpdateTextureParams* params = reinterpret_cast<UpdateTextureParams*>(_params);

				assert(true == params->handle && params->handle.id < MAX_TEXTURES);

				uint32_t id = params->handle.id;

				assert(nullptr != textures[id].tex);

				if (textures[id].dynamic)
				{
					// TODO for mipmaps and texture array
					D3D11_MAPPED_SUBRESOURCE res = {};
					context->Map(textures[id].tex, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
					const uint8_t* pSrc = reinterpret_cast<const uint8_t*>(params->data);
					uint8_t* pDst = reinterpret_cast<uint8_t*>(res.pData);

					for (uint32_t i = 0; i < textures[id].height; ++i)
					{
						memcpy(pDst, pSrc, params->pitch);

						pSrc += params->pitch;
						pDst += res.RowPitch;
					}

					context->Unmap(textures[id].tex, 0);
				}
				else
				{
					// TODO for mipmaps and texture array
					context->UpdateSubresource(textures[id].tex, 0, nullptr, params->data, params->pitch, 0);
				}

				return TF_OK;
			}

			int32_t DestroyTexture(void* params)
			{
				TextureHandle* handle = reinterpret_cast<TextureHandle*>(params);

				assert(true == *handle && handle->id < MAX_TEXTURES);

				uint32_t id = handle->id;

				assert(nullptr != textures[id].tex);

				textures[id].tex->Release();
				if (nullptr != textures[id].srv)
				{
					textures[id].srv->Release();
				}
				if (nullptr != textures[id].rtv)
				{
					textures[id].rtv->Release();
				}
				if (nullptr != textures[id].dsv)
				{
					textures[id].dsv->Release();
				}

				textures[id] = {};

				return TF_OK;
			}

			int32_t CreateSampler(void* _params)
			{
				CreateSamplerParams* params = reinterpret_cast<CreateSamplerParams*>(_params);

				assert(true == params->handle);

				uint32_t id = params->handle.id;

				assert(nullptr == samplers[id].samp);

				CD3D11_SAMPLER_DESC samplerDesc(D3D11_DEFAULT);
				samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
				samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
				samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

				DXCHECKED(device->CreateSamplerState(&samplerDesc, &(samplers[id].samp)));

				return TF_OK;
			}

			int32_t DestroySampler(void* params)
			{
				SamplerHandle* handle = reinterpret_cast<SamplerHandle*>(params);

				assert(true == *handle);

				uint32_t id = handle->id;

				assert(nullptr != samplers[id].samp);

				samplers[id].samp->Release();

				samplers[id] = {};

				return TF_OK;
			}

			int32_t CreateVertexShader(void* _params)
			{
				CreateVertexShaderParams* params = reinterpret_cast<CreateVertexShaderParams*>(_params);

				assert(true == params->handle);

				uint32_t id = params->handle.id;

				assert(nullptr == vertexShaders[id].shader);

				void* ptr = MemoryAllocator::Allocators[ALLOC_LEVEL_BASED_MEM].Allocate(params->size, 4);
				assert(nullptr != ptr);
				memcpy(ptr, params->data, params->size);

				DXCHECKED(device->CreateVertexShader(ptr, params->size, nullptr, &(vertexShaders[id].shader)));

				vertexShaders[id].data = ptr;
				vertexShaders[id].size = params->size;

				return TF_OK;
			}

			int32_t DestroyVertexShader(void* params)
			{
				VertexShaderHandle* handle = reinterpret_cast<VertexShaderHandle*>(params);

				assert(true == *handle);

				uint32_t id = handle->id;

				assert(nullptr != vertexShaders[id].shader);

				vertexShaders[id].shader->Release();

				vertexShaders[id] = {};

				return TF_OK;
			}

			int32_t CreatePixelShader(void* _params)
			{
				CreatePixelShaderParams* params = reinterpret_cast<CreatePixelShaderParams*>(_params);

				assert(true == params->handle);

				uint32_t id = params->handle.id;

				assert(nullptr == pixelShaders[id].shader);

				void* ptr = MemoryAllocator::Allocators[ALLOC_LEVEL_BASED_MEM].Allocate(params->size, 4);
				assert(nullptr != ptr);
				memcpy(ptr, params->data, params->size);

				DXCHECKED(device->CreatePixelShader(params->data, params->size, nullptr, &(pixelShaders[id].shader)));

				pixelShaders[id].data = ptr;
				pixelShaders[id].size = params->size;

				return TF_OK;
			}

			int32_t DestroyPixelShader(void* params)
			{
				PixelShaderHandle* handle = reinterpret_cast<PixelShaderHandle*>(params);

				assert(true == *handle);

				uint32_t id = handle->id;

				assert(nullptr != pixelShaders[id].shader);

				pixelShaders[id].shader->Release();

				pixelShaders[id] = {};

				return TF_OK;
			}

			int32_t CreatePipelineState(void* _params)
			{
				CreatePipelineStateParams* params = reinterpret_cast<CreatePipelineStateParams*>(_params);

				assert(true == params->handle);

				uint32_t id = params->handle.id;

				assert(nullptr == pipelineStates[id].depthStencilState);

				CD3D11_DEPTH_STENCIL_DESC dsState(D3D11_DEFAULT);
				dsState.DepthEnable = params->DepthEnable;
				dsState.DepthWriteMask = (D3D11_DEPTH_WRITE_MASK)params->DepthWrite;// ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
				dsState.DepthFunc = (D3D11_COMPARISON_FUNC)(params->DepthFunc + 1);
				DXCHECKED(device->CreateDepthStencilState(&dsState, &(pipelineStates[id].depthStencilState)));

				CD3D11_RASTERIZER_DESC rsState(D3D11_DEFAULT);
				rsState.CullMode = (D3D11_CULL_MODE)(params->CullMode + 1u);
				DXCHECKED(device->CreateRasterizerState(&rsState, &(pipelineStates[id].rasterizerState)));

				CD3D11_BLEND_DESC blendState(D3D11_DEFAULT);
				DXCHECKED(device->CreateBlendState(&blendState, &(pipelineStates[id].blendState)));

				assert(true == params->vertexShader && nullptr != vertexShaders[params->vertexShader.id].shader);
				assert(true == params->pixelShader && nullptr != pixelShaders[params->vertexShader.id].shader);

				DXCHECKED(device->CreateInputLayout(
					InputElemDescTable[params->vertexFormat],
					InputElemDescSizeTable[params->vertexFormat],
					vertexShaders[id].data,
					vertexShaders[id].size,
					&(pipelineStates[id].inputLayout)
				));

				pipelineStates[id].viewport = {
					0.0f, 0.0f, (FLOAT)winWidth, (FLOAT)winHeight, 0.0f, 1.0f
				};

				pipelineStates[id].vertexShader = params->vertexShader;
				pipelineStates[id].pixelShader = params->pixelShader;

				return TF_OK;
			}

			int32_t DestroyPipelineState(void* params)
			{
				PipelineStateHandle* handle = reinterpret_cast<PipelineStateHandle*>(params);

				assert(true == *handle);

				uint32_t id = handle->id;

				assert(nullptr != pipelineStates[id].depthStencilState);

				pipelineStates[id].depthStencilState->Release();
				pipelineStates[id].rasterizerState->Release();
				pipelineStates[id].blendState->Release();
				pipelineStates[id].inputLayout->Release();

				pipelineStates[id] = {};

				return TF_OK;
			}

			int32_t ClearRenderTargets(void* _params)
			{
				ClearParams* params = reinterpret_cast<ClearParams*>(_params);

				for (uint32_t i = 0; i < MAX_RENDER_TARGET_BINDINGS; ++i)
				{
					if (!params->renderTargets[i])
					{
						break;
					}

					uint32_t id = params->renderTargets[i].id;
					assert(nullptr != textures[id].rtv);

					context->ClearRenderTargetView(textures[id].rtv, params->clearColor);
				}

				if (params->depthRenderTarget)
				{
					uint32_t id = params->depthRenderTarget.id;
					assert(nullptr != textures[id].dsv);

					context->ClearDepthStencilView(
						textures[id].dsv,
						D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
						params->clearDepth,
						params->clearStencil);
				}

				return TF_OK;
			}

			int32_t Draw(void* _params)
			{
				DrawParams* params = reinterpret_cast<DrawParams*>(_params);

				assert(true == params->pipelineState);

				if (params->pipelineState.id != currentPipelineState.id)
				{
					PipelineState& pso = pipelineStates[params->pipelineState.id];

					context->IASetInputLayout(pso.inputLayout);
					context->VSSetShader(vertexShaders[pso.vertexShader.id].shader, nullptr, 0);
					context->PSSetShader(pixelShaders[pso.pixelShader.id].shader, nullptr, 0);
					context->RSSetState(pso.rasterizerState);
					context->RSSetViewports(1, &(pso.viewport));
					context->OMSetDepthStencilState(pso.depthStencilState, 0u);
					context->OMSetBlendState(pso.blendState, nullptr, 0xffffffffu);

					currentPipelineState = params->pipelineState;
				}

				{
					ID3D11Buffer* cbs[MAX_CONSTANT_BUFFER_BINDINGS] = {};
					UINT offsets[MAX_CONSTANT_BUFFER_BINDINGS] = {};
					UINT sizes[MAX_CONSTANT_BUFFER_BINDINGS] = {};

					for (uint32_t i = 0; i < MAX_CONSTANT_BUFFER_BINDINGS; i++)
					{
						if (params->vsConstantBuffers[i].bufferHandle)
						{
							Buffer& buf = buffers[params->vsConstantBuffers[i].bufferHandle.id];
							assert(nullptr != buf.buf);
							if (!(buf.bindingFlags & BINDING_CONSTANT_BUFFER))
							{
								return TF_UNKNOWN_ERR;
							}

							cbs[i] = buf.buf;
							offsets[i] = params->vsConstantBuffers[i].offsetInVectors;
							sizes[i] = params->vsConstantBuffers[i].sizeInVectors;
							if (0u == sizes[i])
							{
								sizes[i] = buf.size / 16;
							}
						}
					}
					context->VSSetConstantBuffers1(0, MAX_CONSTANT_BUFFER_BINDINGS, cbs, offsets, sizes);

					ID3D11ShaderResourceView* srvs[MAX_TEXTURE_BINDINGS] = {};
					for (uint32_t i = 0; i < MAX_TEXTURE_BINDINGS; i++)
					{
						if (params->vsTextures[i])
						{
							Texture& tex = textures[params->vsTextures[i].id];
							assert(nullptr != tex.srv);
							if (!(tex.bindingFlags & BINDING_SHADER_RESOURCE))
							{
								return TF_UNKNOWN_ERR;
							}

							srvs[i] = tex.srv;
						}
					}
					context->VSSetShaderResources(0, MAX_TEXTURE_BINDINGS, srvs);

					ID3D11SamplerState* samps[MAX_SAMPLER_BINDINGS] = {};
					for (uint32_t i = 0; i < MAX_SAMPLER_BINDINGS; i++)
					{
						if (params->vsSamplers[i])
						{
							Sampler& samp = samplers[params->vsSamplers[i].id];
							assert(nullptr != samp.samp);

							samps[i] = samp.samp;
						}
					}
					context->VSSetSamplers(0, MAX_SAMPLER_BINDINGS, samps);
				}

				{
					ID3D11Buffer* cbs[MAX_CONSTANT_BUFFER_BINDINGS] = {};
					UINT offsets[MAX_CONSTANT_BUFFER_BINDINGS] = {};
					UINT sizes[MAX_CONSTANT_BUFFER_BINDINGS] = {};

					for (uint32_t i = 0; i < MAX_CONSTANT_BUFFER_BINDINGS; i++)
					{
						if (params->psConstantBuffers[i].bufferHandle)
						{
							Buffer& buf = buffers[params->psConstantBuffers[i].bufferHandle.id];
							assert(nullptr != buf.buf);
							if (!(buf.bindingFlags & BINDING_CONSTANT_BUFFER))
							{
								return TF_UNKNOWN_ERR;
							}

							cbs[i] = buf.buf;
							offsets[i] = params->psConstantBuffers[i].offsetInVectors;
							sizes[i] = params->psConstantBuffers[i].sizeInVectors;
							if (0u == sizes[i])
							{
								sizes[i] = buf.size / 16;
							}
						}
					}
					context->PSSetConstantBuffers1(0, MAX_CONSTANT_BUFFER_BINDINGS, cbs, offsets, sizes);

					ID3D11ShaderResourceView* srvs[MAX_TEXTURE_BINDINGS] = {};
					for (uint32_t i = 0; i < MAX_TEXTURE_BINDINGS; i++)
					{
						if (params->psTextures[i])
						{
							Texture& tex = textures[params->psTextures[i].id];
							assert(nullptr != tex.srv);
							if (!(tex.bindingFlags & BINDING_SHADER_RESOURCE))
							{
								return TF_UNKNOWN_ERR;
							}

							srvs[i] = tex.srv;
						}
					}
					context->PSSetShaderResources(0, MAX_TEXTURE_BINDINGS, srvs);

					ID3D11SamplerState* samps[MAX_SAMPLER_BINDINGS] = {};
					for (uint32_t i = 0; i < MAX_SAMPLER_BINDINGS; i++)
					{
						if (params->psSamplers[i])
						{
							Sampler& samp = samplers[params->psSamplers[i].id];
							assert(nullptr != samp.samp);

							samps[i] = samp.samp;
						}
					}
					context->PSSetSamplers(0, MAX_SAMPLER_BINDINGS, samps);
				}


				{
					assert(true == params->vertexBuffer);
					Buffer& vb = buffers[params->vertexBuffer.id];
					if (!(vb.bindingFlags & BINDING_VERTEX_BUFFER))
					{
						return TF_UNKNOWN_ERR;
					}

					assert(nullptr != vb.buf);
					{
						ID3D11Buffer* buffers[] = { vb.buf };
						UINT strides[] = { vb.stride };
						UINT offsets[] = { 0 };
						context->IASetVertexBuffers(0, 1, buffers, strides, offsets);
					}

					assert(true == params->indexBuffer);
					Buffer& ib = buffers[params->indexBuffer.id];
					if (!(ib.bindingFlags & BINDING_INDEX_BUFFER))
					{
						return TF_UNKNOWN_ERR;
					}
					assert(nullptr != ib.buf);
					context->IASetIndexBuffer(ib.buf, DXGI_FORMAT_R16_UINT, 0);

					context->DrawIndexed(params->indexCount, params->startIndex, params->startVertex);
				}

				return TF_OK;
			}

		};

	}

	Renderer* Renderer::CreateRenderer()
	{
		return new dx11::RendererDX11();
	}
}