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
#define DXCHECKED(x) C(x, kErrUnknown)

namespace
{
	DXGI_FORMAT PixelFormatTable[tofu::kMaxPixelFormats] =
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
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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
		void*						data; // binary code data (for input layout creation)
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

				if (kOK != (result = CreateDevice()))
				{
					return result;
				}

				if (kOK != (result = InitRenderTargets()))
				{
					return result;
				}

				if (kOK != (result = InitPipelineStates()))
				{
					return result;
				}

				return kOK;
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

				ReleaseResources(buffers, kMaxBuffers, buf, DestroyBuffer);
				ReleaseResources(textures, kMaxTextures, tex, DestroyTexture);
				ReleaseResources(samplers, kMaxSamplers, samp, DestroySampler);
				ReleaseResources(vertexShaders, kMaxVertexShaders, shader, DestroyVertexShader);
				ReleaseResources(pixelShaders, kMaxPixelShaders, shader, DestroyPixelShader);
				ReleaseResources(pipelineStates, kMaxPipelineStates, depthStencilState, DestroyPipelineState);

#undef ReleaseResources

				{
					Texture& rt = textures[kMaxTextures + 1];
					RELEASE(rt.srv);
					RELEASE(rt.rtv);
					RELEASE(rt.dsv);
					RELEASE(rt.tex);

					Texture& ds = textures[kMaxTextures];
					RELEASE(ds.srv);
					RELEASE(ds.rtv);
					RELEASE(ds.dsv);
					RELEASE(ds.tex);
				}

				swapChain->Release();
				context->Release();
				device->Release();

				return kOK;
			}

			virtual int32_t Submit(RendererCommandBuffer* buffer) override
			{
				if (nullptr == buffer)
				{
					return kErrUnknown;
				}

				for (uint32_t i = 0; i < buffer->size; ++i)
				{
					cmd_callback_t cmd = commands[buffer->cmds[i]];
					CHECKED((this->*cmd)(buffer->params[i]));
				}

				return kOK;
			}

			virtual int32_t Present() override
			{
				if (S_OK != swapChain->Present(0, 0))
				{
					return kErrUnknown;
				}

				return kOK;
			}

			virtual int32_t GetFrameBufferSize(int32_t& width, int32_t& height) override
			{
				width = winWidth;
				height = winHeight;
				return kOK;
			}

		private:
			int							winWidth;
			int							winHeight;

			HWND						hWnd;

			IDXGISwapChain1*			swapChain;
			ID3D11Device1*				device;
			ID3D11DeviceContext1*		context;

			Buffer						buffers[kMaxBuffers];
			// textures[MAX_TEXTURES] is default depth buffer, and textures[MAX_TEXTURES + 1] is the default back buffer (d3d11 swap them automatically)
			Texture						textures[kMaxTextures + 2];
			Sampler						samplers[kMaxSamplers];
			VertexShader				vertexShaders[kMaxVertexShaders];
			PixelShader					pixelShaders[kMaxPixelShaders];
			PipelineState				pipelineStates[kMaxPipelineStates];

			PipelineStateHandle			currentPipelineState;

			typedef int32_t(RendererDX11::*cmd_callback_t)(void*);

			cmd_callback_t				commands[RendererCommand::kMaxRendererCommands] =
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

					// create a hardware d3d device and context
					if (S_OK != D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, creationFlags, featureLevels, 2, D3D11_SDK_VERSION, &device, nullptr, &context))
					{
						return -1;
					}

					// try obtain the 11.1 feature level interfaces
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

				// try get the dxgi 1.2 interface
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

				// get actual client area size from window size
				RECT rect = {};
				GetClientRect(hWnd, &rect);
				winWidth = rect.right - rect.left;
				winHeight = rect.bottom - rect.top;

				// create swap chain
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

				// obtain back buffer from swap chain, and wrap as a texture
				//
				ID3D11Texture2D* backbufferTex = nullptr;
				if (S_OK != (hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backbufferTex)))
				{
					return kErrUnknown;
				}

				D3D11_TEXTURE2D_DESC desc = {};
				backbufferTex->GetDesc(&desc);

				Texture& rt = textures[kMaxTextures + 1];

				rt = {};
				rt.width = winWidth;
				rt.height = winHeight;
				rt.arraySize = 1;

				if (S_OK != (hr = device->CreateRenderTargetView(backbufferTex, nullptr, &(rt.rtv))))
				{
					return -1;
				}
				rt.tex = backbufferTex;


				// create the default depth buffer
				//
				Texture& ds = textures[kMaxTextures];

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

				// set render targets, they are fixed for now
				context->OMSetRenderTargets(1, &(rt.rtv), ds.dsv);

				return 0;
			}

			int32_t InitPipelineStates()
			{
				context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				return kOK;
			}

			int32_t Nop(void*) { return kOK; }

			int32_t CreateBuffer(void* _params)
			{
				CreateBufferParams* params = reinterpret_cast<CreateBufferParams*>(_params);

				assert(true == params->handle);
				uint32_t id = params->handle.id;

				bool isShaderResource = ((params->bindingFlags & kBindingShaderResource) != 0u);

				assert(
					!isShaderResource ||
					(params->stride > sizeof(float) * 4)
				);


				assert(nullptr == buffers[id].buf);
				buffers[id] = {};

				// align size to 256 bytes if it is a constant buffer
				if (params->bindingFlags & kBindingConstantBuffer)
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
					return kErrUnknown;
				}

				// create a shader resource view if it need to be bound to texture slot (t#)
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

				return kOK;
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
						((buffers[id].bindingFlags & kBindingConstantBuffer) == 0) ||
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

				return kOK;
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

				return kOK;
			}

			int32_t CreateTexture(void* _params)
			{
				CreateTextureParams* params = reinterpret_cast<CreateTextureParams*>(_params);

				assert(true == params->handle && params->handle.id < kMaxTextures);

				uint32_t id = params->handle.id;

				assert(nullptr == textures[id].tex);

				uint32_t bindingFlags = params->bindingFlags & (kBindingShaderResource | kBindingRenderTarget | kBindingDepthStencil);

				if (params->isFile) // load the texture from file
				{
					assert(bindingFlags & kBindingShaderResource);

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
				else // load the texture from raw data
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

					if (params->bindingFlags & kBindingShaderResource)
					{
						DXCHECKED(device->CreateShaderResourceView(textures[id].tex, nullptr, &(textures[id].srv)));
					}
				}

				if (params->bindingFlags & kBindingRenderTarget)
				{
					DXCHECKED(device->CreateRenderTargetView(textures[id].tex, nullptr, &(textures[id].rtv)));
				}

				if (params->bindingFlags & kBindingDepthStencil)
				{
					DXCHECKED(device->CreateDepthStencilView(textures[id].tex, nullptr, &(textures[id].dsv)));
				}

				D3D11_TEXTURE2D_DESC desc = {};
				textures[id].tex->GetDesc(&desc);

				textures[id].dynamic = params->dynamic;
				textures[id].cubeMap = ((desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) != 0 ? 1 : params->cubeMap);
				textures[id].format = params->isFile ? kFormatAuto : params->format;
				textures[id].arraySize = desc.ArraySize;
				textures[id].bindingFlags = bindingFlags;
				textures[id].width = desc.Width;
				textures[id].height = desc.Height;
				textures[id].pitch = params->pitch;

				return kOK;
			}

			int32_t UpdateTexture(void* _params)
			{
				UpdateTextureParams* params = reinterpret_cast<UpdateTextureParams*>(_params);

				assert(true == params->handle && params->handle.id < kMaxTextures);

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

				return kOK;
			}

			int32_t DestroyTexture(void* params)
			{
				TextureHandle* handle = reinterpret_cast<TextureHandle*>(params);

				assert(true == *handle && handle->id < kMaxTextures);

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

				return kOK;
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

				return kOK;
			}

			int32_t DestroySampler(void* params)
			{
				SamplerHandle* handle = reinterpret_cast<SamplerHandle*>(params);

				assert(true == *handle);

				uint32_t id = handle->id;

				assert(nullptr != samplers[id].samp);

				samplers[id].samp->Release();

				samplers[id] = {};

				return kOK;
			}

			int32_t CreateVertexShader(void* _params)
			{
				CreateVertexShaderParams* params = reinterpret_cast<CreateVertexShaderParams*>(_params);

				assert(true == params->handle);

				uint32_t id = params->handle.id;

				assert(nullptr == vertexShaders[id].shader);

				// store binary code for further use (input layout)
				void* ptr = MemoryAllocator::Allocators[kAllocLevelBasedMem].Allocate(params->size, 4);
				assert(nullptr != ptr);
				memcpy(ptr, params->data, params->size);

				DXCHECKED(device->CreateVertexShader(ptr, params->size, nullptr, &(vertexShaders[id].shader)));

				vertexShaders[id].data = ptr;
				vertexShaders[id].size = params->size;

				return kOK;
			}

			int32_t DestroyVertexShader(void* params)
			{
				VertexShaderHandle* handle = reinterpret_cast<VertexShaderHandle*>(params);

				assert(true == *handle);

				uint32_t id = handle->id;

				assert(nullptr != vertexShaders[id].shader);

				vertexShaders[id].shader->Release();

				vertexShaders[id] = {};

				return kOK;
			}

			int32_t CreatePixelShader(void* _params)
			{
				CreatePixelShaderParams* params = reinterpret_cast<CreatePixelShaderParams*>(_params);

				assert(true == params->handle);

				uint32_t id = params->handle.id;

				assert(nullptr == pixelShaders[id].shader);

				// store binary code for further use
				void* ptr = MemoryAllocator::Allocators[kAllocLevelBasedMem].Allocate(params->size, 4);
				assert(nullptr != ptr);
				memcpy(ptr, params->data, params->size);

				DXCHECKED(device->CreatePixelShader(params->data, params->size, nullptr, &(pixelShaders[id].shader)));

				pixelShaders[id].data = ptr;
				pixelShaders[id].size = params->size;

				return kOK;
			}

			int32_t DestroyPixelShader(void* params)
			{
				PixelShaderHandle* handle = reinterpret_cast<PixelShaderHandle*>(params);

				assert(true == *handle);

				uint32_t id = handle->id;

				assert(nullptr != pixelShaders[id].shader);

				pixelShaders[id].shader->Release();

				pixelShaders[id] = {};

				return kOK;
			}

			int32_t CreatePipelineState(void* _params)
			{
				CreatePipelineStateParams* params = reinterpret_cast<CreatePipelineStateParams*>(_params);

				assert(true == params->handle);

				uint32_t id = params->handle.id;

				assert(nullptr == pipelineStates[id].depthStencilState);

				CD3D11_DEPTH_STENCIL_DESC dsState(D3D11_DEFAULT);
				dsState.DepthEnable = params->depthEnable;
				dsState.DepthWriteMask = (D3D11_DEPTH_WRITE_MASK)params->depthWrite;
				dsState.DepthFunc = (D3D11_COMPARISON_FUNC)(params->depthFunc + 1);
				DXCHECKED(device->CreateDepthStencilState(&dsState, &(pipelineStates[id].depthStencilState)));

				CD3D11_RASTERIZER_DESC rsState(D3D11_DEFAULT);
				rsState.CullMode = (D3D11_CULL_MODE)(params->cullMode + 1u);
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

				return kOK;
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

				return kOK;
			}

			int32_t ClearRenderTargets(void* _params)
			{
				ClearParams* params = reinterpret_cast<ClearParams*>(_params);

				for (uint32_t i = 0; i < kMaxRenderTargetBindings; ++i)
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

				return kOK;
			}

			int32_t Draw(void* _params)
			{
				DrawParams* params = reinterpret_cast<DrawParams*>(_params);

				assert(true == params->pipelineState);
				
				// change pipeline states if necessary
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

				// vertex shader resource binding
				{
					// constant buffer bidnings
					ID3D11Buffer* cbs[kMaxConstantBufferBindings] = {};
					UINT offsets[kMaxConstantBufferBindings] = {};
					UINT sizes[kMaxConstantBufferBindings] = {};

					for (uint32_t i = 0; i < kMaxConstantBufferBindings; i++)
					{
						if (params->vsConstantBuffers[i].bufferHandle)
						{
							Buffer& buf = buffers[params->vsConstantBuffers[i].bufferHandle.id];
							assert(nullptr != buf.buf);
							if (!(buf.bindingFlags & kBindingConstantBuffer))
							{
								return kErrUnknown;
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
					context->VSSetConstantBuffers1(0, kMaxConstantBufferBindings, cbs, offsets, sizes);

					// textures bindings
					ID3D11ShaderResourceView* srvs[kMaxTextureBindings] = {};
					for (uint32_t i = 0; i < kMaxTextureBindings; i++)
					{
						if (params->vsTextures[i])
						{
							Texture& tex = textures[params->vsTextures[i].id];
							assert(nullptr != tex.srv);
							if (!(tex.bindingFlags & kBindingShaderResource))
							{
								return kErrUnknown;
							}

							srvs[i] = tex.srv;
						}
					}
					context->VSSetShaderResources(0, kMaxTextureBindings, srvs);

					// samplers bidnings
					ID3D11SamplerState* samps[kMaxSamplerBindings] = {};
					for (uint32_t i = 0; i < kMaxSamplerBindings; i++)
					{
						if (params->vsSamplers[i])
						{
							Sampler& samp = samplers[params->vsSamplers[i].id];
							assert(nullptr != samp.samp);

							samps[i] = samp.samp;
						}
					}
					context->VSSetSamplers(0, kMaxSamplerBindings, samps);
				}

				// pixel shader resource binding
				{
					// constant buffer bindings
					ID3D11Buffer* cbs[kMaxConstantBufferBindings] = {};
					UINT offsets[kMaxConstantBufferBindings] = {};
					UINT sizes[kMaxConstantBufferBindings] = {};

					for (uint32_t i = 0; i < kMaxConstantBufferBindings; i++)
					{
						if (params->psConstantBuffers[i].bufferHandle)
						{
							Buffer& buf = buffers[params->psConstantBuffers[i].bufferHandle.id];
							assert(nullptr != buf.buf);
							if (!(buf.bindingFlags & kBindingConstantBuffer))
							{
								return kErrUnknown;
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
					context->PSSetConstantBuffers1(0, kMaxConstantBufferBindings, cbs, offsets, sizes);

					// texture bindings
					ID3D11ShaderResourceView* srvs[kMaxTextureBindings] = {};
					for (uint32_t i = 0; i < kMaxTextureBindings; i++)
					{
						if (params->psTextures[i])
						{
							Texture& tex = textures[params->psTextures[i].id];
							assert(nullptr != tex.srv);
							if (!(tex.bindingFlags & kBindingShaderResource))
							{
								return kErrUnknown;
							}

							srvs[i] = tex.srv;
						}
					}
					context->PSSetShaderResources(0, kMaxTextureBindings, srvs);

					// sampler bindings
					ID3D11SamplerState* samps[kMaxSamplerBindings] = {};
					for (uint32_t i = 0; i < kMaxSamplerBindings; i++)
					{
						if (params->psSamplers[i])
						{
							Sampler& samp = samplers[params->psSamplers[i].id];
							assert(nullptr != samp.samp);

							samps[i] = samp.samp;
						}
					}
					context->PSSetSamplers(0, kMaxSamplerBindings, samps);
				}

				// draw call
				{
					// set vertex buffer
					assert(true == params->vertexBuffer);
					Buffer& vb = buffers[params->vertexBuffer.id];
					if (!(vb.bindingFlags & kBindingVertexBuffer))
					{
						return kErrUnknown;
					}

					assert(nullptr != vb.buf);
					{
						ID3D11Buffer* buffers[] = { vb.buf };
						UINT strides[] = { vb.stride };
						UINT offsets[] = { 0 };
						context->IASetVertexBuffers(0, 1, buffers, strides, offsets);
					}

					// set index buffer
					assert(true == params->indexBuffer);
					Buffer& ib = buffers[params->indexBuffer.id];
					if (!(ib.bindingFlags & kBindingIndexBuffer))
					{
						return kErrUnknown;
					}
					assert(nullptr != ib.buf);
					context->IASetIndexBuffer(ib.buf, DXGI_FORMAT_R16_UINT, 0);

					// draw it!
					context->DrawIndexed(params->indexCount, params->startIndex, params->startVertex);
				}

				return kOK;
			}

		};

	}

	Renderer* Renderer::CreateRenderer()
	{
		return new dx11::RendererDX11();
	}
}