#include "Renderer.h"

#include "NativeContext.h"

#include "MemoryAllocator.h"

#include <Windows.h>
#include <d3d11_1.h>

#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>

#include <cassert>

#ifdef _DEBUG
#include <crtdbg.h>
#define BREAK(x) if (0 != (x)) __debugbreak();
#endif

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#define RELEASE(x) if (nullptr != (x)) { (x)->Release(); (x) = nullptr; }
#define C(x, ret) if ((x) != S_OK) { __debugbreak(); return (ret); }
#define DXCHECKED(x) C(x, kErrUnknown)

namespace
{
	using namespace tofu;

	DXGI_FORMAT PixelFormatTable[tofu::kMaxPixelFormats] =
	{
		DXGI_FORMAT_UNKNOWN, // AUTO
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_FORMAT_R8G8B8A8_SNORM,
		DXGI_FORMAT_R16G16B16A16_UNORM,
		DXGI_FORMAT_R16G16B16A16_SNORM,
		DXGI_FORMAT_R16G16B16A16_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_R8_UNORM,
		DXGI_FORMAT_R8_SINT,
		DXGI_FORMAT_R16_SINT,
		DXGI_FORMAT_R32_SINT,
		DXGI_FORMAT_R8_UINT,
		DXGI_FORMAT_R16_UINT,
		DXGI_FORMAT_R32_UINT,
		DXGI_FORMAT_D24_UNORM_S8_UINT,
	};

	D3D11_FILTER FilterTable[tofu::kMaxTextureFilters] =
	{
		D3D11_FILTER_MIN_MAG_MIP_POINT,
		D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_FILTER_ANISOTROPIC,
	};



	D3D11_INPUT_ELEMENT_DESC InputElemDescNormal[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3D11_INPUT_ELEMENT_DESC InputElemDescSkinned[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3D11_INPUT_ELEMENT_DESC InputElemDescOverlay[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3D11_INPUT_ELEMENT_DESC* InputElemDescTable[] = { InputElemDescNormal, InputElemDescSkinned, InputElemDescOverlay };
	UINT InputElemDescSizeTable[] = { 4u, 6u, 3u };

	struct Buffer
	{
		ID3D11Buffer*				buf;
		ID3D11ShaderResourceView*	srv;
		ID3D11UnorderedAccessView*	uav;
		uint32_t					dynamic : 1;
		uint32_t					format : 15;
		uint32_t					bindingFlags : 16;
		uint32_t					size;
		uint32_t					stride;
		uint32_t					label;
	};

	struct Texture
	{
		ID3D11Resource*				tex;
		ID3D11ShaderResourceView*	srv;
		ID3D11UnorderedAccessView*	uav;
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
		uint32_t					depth;
		uint32_t					label;
	};

	struct Sampler
	{
		ID3D11SamplerState*			samp;
		uint32_t					label;
	};

	struct VertexShader
	{
		ID3D11VertexShader*			shader;
		void*						data; // binary code data (for input layout creation)
		size_t						size;
		uint32_t					label;
	};

	struct PixelShader
	{
		ID3D11PixelShader*			shader;
		void*						data;
		size_t						size;
		uint32_t					label;
	};

	struct ComputeShader
	{
		ID3D11ComputeShader*		shader;
		void*						data;
		size_t						size;
		uint32_t					label;
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
		uint32_t					label;
		uint8_t						stencilRef;
	};

#if TOFU_PERFORMANCE_TIMER_ENABLED == 1
	struct TimeQuery
	{
		ID3D11Query*				disjoint;
		ID3D11Query*				start;
		ID3D11Query*				end;

		// duration in ms
		float						deltaTime;

		int32_t Init(ID3D11Device1* device)
		{
			DXCHECKED(device->CreateQuery(
			&CD3D11_QUERY_DESC(D3D11_QUERY_TIMESTAMP_DISJOINT, 0u),
				&disjoint));

			DXCHECKED(device->CreateQuery(
				&CD3D11_QUERY_DESC(D3D11_QUERY_TIMESTAMP, 0u),
				&start));

			DXCHECKED(device->CreateQuery(
				&CD3D11_QUERY_DESC(D3D11_QUERY_TIMESTAMP, 0u),
				&end));

			return kOK;
		}

		int32_t Release()
		{
			RELEASE(disjoint);
			RELEASE(start);
			RELEASE(end);

			return kOK;
		}

		int32_t Start(ID3D11DeviceContext1* context)
		{
			context->Begin(disjoint);
			context->End(start);

			return kOK;
		}

		int32_t End(ID3D11DeviceContext1* context)
		{
			context->End(end);
			context->End(disjoint);
			return kOK;
		}

		bool RetrieveData(ID3D11DeviceContext1* context)
		{
			D3D11_QUERY_DATA_TIMESTAMP_DISJOINT data = {};
			if (S_OK != context->GetData(disjoint, &data, sizeof(data), 0))
				return false;

			UINT64 startTime, endTime;
			if (S_OK != context->GetData(start, &startTime, sizeof(UINT64), 0))
				return false;

			if (S_OK != context->GetData(end, &endTime, sizeof(UINT64), 0))
				return false;

			deltaTime = (endTime - startTime) / (float)data.Frequency * 1000.0f;

			return true;
		}
	};
#endif
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
				ReleaseResources(computeShaders, kMaxComputeShaders, shader, DestroyComputeShader);
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

#if TOFU_PERFORMANCE_TIMER_ENABLED == 1
				for (uint32_t i = 0; i < kMaxGpuTimeQueries; i++)
				{
					DXCHECKED(queries[i].Release());
				}
#endif

				if (NativeContext::instance()->IsFullScreen())
				{
					swapChain->SetFullscreenState(FALSE, nullptr);
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

				if (standby)
				{
					return kOK;
				}

				// gpu time query begins here
#if TOFU_PERFORMANCE_TIMER_ENABLED == 1
				while (firstQuery < lastQuery)
				{
					uint32_t firstIdx = firstQuery % kMaxGpuTimeQueries;
					if (queries[firstIdx].RetrieveData(context))
					{
						gpuTime = queries[firstIdx].deltaTime;
						firstQuery++;
					}
					else
						break;
				}
				if (lastQuery - firstQuery < kMaxGpuTimeQueries)
				{
					queries[lastQuery % kMaxGpuTimeQueries].Start(context);
				}
#endif

				for (uint32_t i = 0; i < buffer->size; ++i)
				{
					cmd_callback_t cmd = commands[buffer->cmds[i]];
					CHECKED((this->*cmd)(buffer->params[i]));
				}

				// gpu time query ends here
#if TOFU_PERFORMANCE_TIMER_ENABLED == 1
				if (lastQuery - firstQuery < kMaxGpuTimeQueries)
				{
					queries[lastQuery % kMaxGpuTimeQueries].End(context);
					lastQuery++;
				}
#endif

				return kOK;
			}

			virtual int32_t Present() override
			{
				HRESULT ret = S_OK;

				if (standby)
				{
					if (S_OK == swapChain->Present(0, DXGI_PRESENT_TEST))
					{
						standby = false;
					}
					return kOK;
				}

				if (S_OK != (ret = swapChain->Present(TOFU_VSYNC, 0)))
				{
					if (ret == DXGI_STATUS_OCCLUDED)
					{
						standby = true;
						return kOK;
					}
					return kErrUnknown;
				}

				return kOK;
			}

			virtual int32_t CleanupResources(uint32_t labelMask) override
			{
				// unbind resources
				{
					DrawParams params{};
					BindRenderTargets(&params);
					BindShaderResources(&params);
				}
				{
					ComputeParams params{};
					BindShaderResources(&params);
				}

				// release resources

#define ReleaseResources(RES_ARRAY, MAX_COUNT, MEMBER_TO_CHECK, DESTROY_FUNC) \
				for (uint32_t i = 0; i < MAX_COUNT; i++) \
				{ \
					if (nullptr != RES_ARRAY[i].MEMBER_TO_CHECK && (labelMask & RES_ARRAY[i].label) != 0u) \
					{ \
						DESTROY_FUNC(&i); \
					} \
				} 

				ReleaseResources(buffers, kMaxBuffers, buf, DestroyBuffer);
				ReleaseResources(textures, kMaxTextures, tex, DestroyTexture);
				ReleaseResources(samplers, kMaxSamplers, samp, DestroySampler);
				ReleaseResources(vertexShaders, kMaxVertexShaders, shader, DestroyVertexShader);
				ReleaseResources(pixelShaders, kMaxPixelShaders, shader, DestroyPixelShader);
				ReleaseResources(computeShaders, kMaxComputeShaders, shader, DestroyComputeShader);
				ReleaseResources(pipelineStates, kMaxPipelineStates, depthStencilState, DestroyPipelineState);
#undef ReleaseResources

				return kOK;
			}

			virtual int32_t GetFrameBufferSize(int32_t& width, int32_t& height) override
			{
				width = winWidth;
				height = winHeight;
				return kOK;
			}

			virtual float GetGPUTime(uint32_t slot) override
			{
#if TOFU_PERFORMANCE_TIMER_ENABLED == 1
				return gpuTime;
#else
				return 0.0f;
#endif
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
			ComputeShader				computeShaders[kMaxComputeShaders];
			PipelineState				pipelineStates[kMaxPipelineStates];

			PipelineStateHandle			currentPipelineState;
			TextureHandle				renderTargets[kMaxRenderTargetBindings];
			TextureHandle				depthRenderTarget;

			bool						standby;

#if TOFU_PERFORMANCE_TIMER_ENABLED == 1
			TimeQuery					queries[kMaxGpuTimeQueries];
			uint32_t					firstQuery;
			uint32_t					lastQuery;
			float						gpuTime;
#endif

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
				&RendererDX11::CreateComputeShader,
				&RendererDX11::DestroyComputeShader,
				&RendererDX11::CreatePipelineState,
				&RendererDX11::DestroyPipelineState,
				&RendererDX11::ClearRenderTargets,
				&RendererDX11::Draw,
				&RendererDX11::Compute,
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

				bool fullscreen = NativeContext::instance()->IsFullScreen();

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
				swapChainDesc.Flags = fullscreen ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0;

				if (S_OK != (hr = factory->CreateSwapChainForHwnd(device, hWnd, &swapChainDesc, nullptr, nullptr, &(swapChain))))
				{
					context->Release();
					device->Release();
					factory->Release();
					return -1;
				}

				factory->Release();

				if (fullscreen)
				{
					DXGI_MODE_DESC desc = {};
					desc.Width = winWidth;
					desc.Height = winHeight;
					desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
					swapChain->ResizeTarget(&desc);

					swapChain->SetFullscreenState(TRUE, nullptr);
				}

#if TOFU_PERFORMANCE_TIMER_ENABLED == 1
				for (uint32_t i = 0; i < kMaxGpuTimeQueries; i++)
				{
					DXCHECKED(queries[i].Init(device));
				}
				firstQuery = 0;
				lastQuery = 0;
#endif

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
				rt.bindingFlags = kBindingRenderTarget;

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
				ds.bindingFlags = kBindingDepthStencil;

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

				// set render targets
				context->OMSetRenderTargets(1, &(rt.rtv), ds.dsv);

				for (uint32_t i = 0; i < kMaxRenderTargetBindings; i++)
				{
					renderTargets[i] = TextureHandle();
				}

				renderTargets[0] = TextureHandle(kMaxTextures + 1);
				depthRenderTarget = TextureHandle(kMaxTextures);

				return 0;
			}

			int32_t InitPipelineStates()
			{
				context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				return kOK;
			}

			int32_t BindRenderTargets(DrawParams* params)
			{
				if (params->renderTargets[0].id == kDefaultRenderTarget.id)
					params->renderTargets[0] = TextureHandle(kMaxTextures + 1);

				if (params->depthRenderTarget.id == kDefaultRenderTarget.id)
					params->depthRenderTarget = TextureHandle(kMaxTextures);

				bool rebind = false;

				if (params->depthRenderTarget.id != depthRenderTarget.id)
				{
					rebind = true;
				}
				else
				{
					for (uint32_t i = 0; i < kMaxRenderTargetBindings; i++)
					{
						if (params->renderTargets[i].id != renderTargets[i].id)
						{
							rebind = true;
							break;
						}
					}
				}

				if (rebind)
				{
					// unbind textures;
					ID3D11ShaderResourceView* srvs[kMaxTextureBindings] = { nullptr };
					context->VSSetShaderResources(0, kMaxTextureBindings, srvs);
					context->PSSetShaderResources(0, kMaxTextureBindings, srvs);

					ID3D11RenderTargetView* rtvs[kMaxRenderTargetBindings] = {};
					ID3D11DepthStencilView* dsv = nullptr;

					for (uint32_t i = 0; i < kMaxRenderTargetBindings; i++)
					{
						renderTargets[i] = params->renderTargets[i];
						if (renderTargets[i])
						{
							Texture& tex = textures[renderTargets[i].id];
							if (nullptr == tex.rtv ||
								!(tex.bindingFlags & kBindingRenderTarget))
								return kErrUnknown;

							rtvs[i] = tex.rtv;
						}
					}

					depthRenderTarget = params->depthRenderTarget;
					if (depthRenderTarget)
					{
						Texture& tex = textures[depthRenderTarget.id];
						if (nullptr == tex.dsv ||
							!(tex.bindingFlags & kBindingDepthStencil))
							return kErrUnknown;

						dsv = tex.dsv;
					}

					context->OMSetRenderTargets(kMaxRenderTargetBindings, rtvs, dsv);
				}

				return kOK;
			}

			int32_t BindShaderResources(DrawParams* params)
			{
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
						BaseHandle handle = params->vsShaderResources[i];

						if (handle && handle.type == kHandleTypeTexture)
						{
							Texture& tex = textures[handle.id];
							assert(nullptr != tex.srv);
							if (!(tex.bindingFlags & kBindingShaderResource))
							{
								return kErrUnknown;
							}

							srvs[i] = tex.srv;
						}
						else if (handle && handle.type == kHandleTypeBuffer)
						{
							Buffer& buf = buffers[handle.id];
							assert(nullptr != buf.srv);
							if (!(buf.bindingFlags & kBindingShaderResource))
							{
								return kErrUnknown;
							}

							srvs[i] = buf.srv;
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
						BaseHandle handle = params->psShaderResources[i];

						if (handle && handle.type == kHandleTypeTexture)
						{
							Texture& tex = textures[handle.id];
							assert(nullptr != tex.srv);
							if (!(tex.bindingFlags & kBindingShaderResource))
							{
								return kErrUnknown;
							}

							srvs[i] = tex.srv;
						}
						else if (handle && handle.type == kHandleTypeBuffer)
						{
							Buffer& buf = buffers[handle.id];
							assert(nullptr != buf.srv);
							if (!(buf.bindingFlags & kBindingShaderResource))
							{
								return kErrUnknown;
							}

							srvs[i] = buf.srv;
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

				return kOK;
			}

			int32_t BindShaderResources(ComputeParams* params)
			{
				{
					// constant buffer bindings
					ID3D11Buffer* cbs[kMaxConstantBufferBindings] = {};
					UINT offsets[kMaxConstantBufferBindings] = {};
					UINT sizes[kMaxConstantBufferBindings] = {};

					for (uint32_t i = 0; i < kMaxConstantBufferBindings; i++)
					{
						if (params->constantBuffers[i].bufferHandle)
						{
							Buffer& buf = buffers[params->constantBuffers[i].bufferHandle.id];
							assert(nullptr != buf.buf);
							if (!(buf.bindingFlags & kBindingConstantBuffer))
							{
								return kErrUnknown;
							}

							cbs[i] = buf.buf;
							offsets[i] = params->constantBuffers[i].offsetInVectors;
							sizes[i] = params->constantBuffers[i].sizeInVectors;
							if (0u == sizes[i])
							{
								sizes[i] = buf.size / 16;
							}
						}
					}
					context->CSSetConstantBuffers1(0, kMaxConstantBufferBindings, cbs, offsets, sizes);

					// rw texture bindings
					ID3D11UnorderedAccessView* uavs[kMaxTextureBindings] = {};
					for (uint32_t i = 0; i < kMaxTextureBindings; i++)
					{
						BaseHandle handle = params->rwShaderResources[i];

						if (handle && handle.type == kHandleTypeTexture)
						{
							Texture& tex = textures[handle.id];
							assert(nullptr != tex.uav);
							if (!(tex.bindingFlags & kBindingUnorderedAccess))
							{
								return kErrUnknown;
							}

							uavs[i] = tex.uav;
						}
						else if (handle && handle.type == kHandleTypeBuffer)
						{
							Buffer& buf = buffers[handle.id];
							assert(nullptr != buf.uav);
							if (!(buf.bindingFlags & kBindingUnorderedAccess))
							{
								return kErrUnknown;
							}

							uavs[i] = buf.uav;
						}
					}
					context->CSSetUnorderedAccessViews(0, kMaxTextureBindings, uavs, nullptr);

					// texture bindings
					ID3D11ShaderResourceView* srvs[kMaxTextureBindings] = {};
					for (uint32_t i = 0; i < kMaxTextureBindings; i++)
					{
						BaseHandle handle = params->shaderResources[i];

						if (handle && handle.type == kHandleTypeTexture)
						{
							Texture& tex = textures[handle.id];
							assert(nullptr != tex.srv);
							if (!(tex.bindingFlags & kBindingShaderResource))
							{
								return kErrUnknown;
							}

							srvs[i] = tex.srv;
						}
						else if (handle && handle.type == kHandleTypeBuffer)
						{
							Buffer& buf = buffers[handle.id];
							assert(nullptr != buf.srv);
							if (!(buf.bindingFlags & kBindingShaderResource))
							{
								return kErrUnknown;
							}

							srvs[i] = buf.srv;
						}
					}
					context->CSSetShaderResources(0, kMaxTextureBindings, srvs);

					// sampler bindings
					ID3D11SamplerState* samps[kMaxSamplerBindings] = {};
					for (uint32_t i = 0; i < kMaxSamplerBindings; i++)
					{
						if (params->samplers[i])
						{
							Sampler& samp = samplers[params->samplers[i].id];
							assert(nullptr != samp.samp);

							samps[i] = samp.samp;
						}
					}
					context->CSSetSamplers(0, kMaxSamplerBindings, samps);
				}

				return kOK;
			}

			int32_t Nop(void*) { return kOK; }

			int32_t CreateBuffer(void* _params)
			{
				CreateBufferParams* params = reinterpret_cast<CreateBufferParams*>(_params);

				assert(true == params->handle);
				uint32_t id = params->handle.id;

				uint32_t binding = (params->bindingFlags & (
					kBindingVertexBuffer |
					kBindingIndexBuffer |
					kBindingConstantBuffer |
					kBindingShaderResource |
					kBindingUnorderedAccess));

				bool isStructuredBuffer = ((binding & (kBindingShaderResource | kBindingUnorderedAccess)) != 0u);
				bool isPrimitiveBuffer = ((binding & (kBindingVertexBuffer | kBindingIndexBuffer)) != 0u);

				// constant buffer cannot share with other types
				if (isStructuredBuffer || isPrimitiveBuffer)
					binding &= (~kBindingConstantBuffer);

				assert(
					!isStructuredBuffer ||
					(params->stride > sizeof(float) * 4)
				);


				assert(nullptr == buffers[id].buf);
				buffers[id] = {};

				// align size to 256 bytes if it is a constant buffer
				if (binding & kBindingConstantBuffer)
				{
					params->size = ((params->size + 0xffu) & (~0xffu));
				}

				CD3D11_BUFFER_DESC bufDesc(
					params->size,
					binding,
					(params->dynamic == 1 ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT),
					(params->dynamic == 1 ? D3D11_CPU_ACCESS_WRITE : 0U),
					(isStructuredBuffer ? D3D11_RESOURCE_MISC_BUFFER_STRUCTURED : 0U),
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
				if (binding & kBindingShaderResource)
				{
					D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
					srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
					srvDesc.Buffer.NumElements = params->size / params->stride;
					srvDesc.Format = PixelFormatTable[params->format];

					DXCHECKED(device->CreateShaderResourceView(
						buffers[id].buf,
						&srvDesc,
						&(buffers[id].srv)
					));
				}

				if (binding & kBindingUnorderedAccess)
				{
					D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
					uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
					uavDesc.Buffer.NumElements = params->size / params->stride;
					uavDesc.Format = PixelFormatTable[params->format];

					DXCHECKED(device->CreateUnorderedAccessView(
						buffers[id].buf,
						&uavDesc,
						&(buffers[id].uav)
					));
				}

				buffers[id].dynamic = params->dynamic;
				buffers[id].format = params->format;
				buffers[id].bindingFlags = binding;
				buffers[id].size = params->size;
				buffers[id].stride = params->stride;
				buffers[id].label = params->label;

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

				RELEASE(buffers[id].srv);
				RELEASE(buffers[id].uav);

				buffers[id] = {};

				return kOK;
			}

			int32_t CreateTexture(void* _params)
			{
				CreateTextureParams* params = reinterpret_cast<CreateTextureParams*>(_params);

				assert(true == params->handle && params->handle.id < kMaxTextures);

				uint32_t id = params->handle.id;

				assert(nullptr == textures[id].tex);

				uint32_t bindingFlags = params->bindingFlags & (kBindingShaderResource | kBindingRenderTarget | kBindingDepthStencil | kBindingUnorderedAccess);

				ID3D11Texture2D* tex2d = nullptr;
				ID3D11Texture3D* tex3d = nullptr;

				if (params->isFile) // load the texture from file
				{
					assert(bindingFlags & kBindingShaderResource);
					bindingFlags = kBindingShaderResource;

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

					DXCHECKED(res->QueryInterface<ID3D11Texture2D>(&tex2d));
					res->Release();

					textures[id].tex = tex2d;
				}
				else if (params->isSlice)
				{
					if (!params->sliceSource || nullptr == textures[params->sliceSource.id].tex)
						return kErrUnknown;

					Texture& sourceTex = textures[params->sliceSource.id];
					if (sourceTex.arraySize == 1 || sourceTex.arraySize < params->arraySize)
						return kErrUnknown;

					DXCHECKED(sourceTex.tex->QueryInterface<ID3D11Texture2D>(&tex2d));

					if (bindingFlags & kBindingShaderResource)
					{
						CD3D11_SHADER_RESOURCE_VIEW_DESC desc(tex2d,
							D3D11_SRV_DIMENSION_TEXTURE2DARRAY,
							PixelFormatTable[params->format],
							0,
							1,
							params->sliceStartArrayIndex,
							params->arraySize
						);

						DXCHECKED(device->CreateShaderResourceView(tex2d, &desc, &(textures[id].srv)));
					}

					if (bindingFlags & kBindingUnorderedAccess)
					{
						CD3D11_UNORDERED_ACCESS_VIEW_DESC desc(tex2d,
							D3D11_UAV_DIMENSION_TEXTURE2DARRAY,
							PixelFormatTable[params->format],
							0,
							params->sliceStartArrayIndex,
							params->arraySize
						);

						DXCHECKED(device->CreateUnorderedAccessView(tex2d, &desc, &(textures[id].uav)));
					}

					if (bindingFlags & kBindingRenderTarget)
					{
						CD3D11_RENDER_TARGET_VIEW_DESC desc(tex2d,
							D3D11_RTV_DIMENSION_TEXTURE2DARRAY,
							PixelFormatTable[params->format],
							0,
							params->sliceStartArrayIndex,
							params->arraySize
						);

						DXCHECKED(device->CreateRenderTargetView(tex2d, &desc, &(textures[id].rtv)));
					}

					if (bindingFlags & kBindingDepthStencil)
					{
						CD3D11_DEPTH_STENCIL_VIEW_DESC desc(tex2d,
							D3D11_DSV_DIMENSION_TEXTURE2DARRAY,
							PixelFormatTable[params->format],
							0,
							params->sliceStartArrayIndex,
							params->arraySize
						);

						if (desc.Format == DXGI_FORMAT_R24G8_TYPELESS)
							desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

						DXCHECKED(device->CreateDepthStencilView(tex2d, &desc, &(textures[id].dsv)));
					}

					textures[id].tex = tex2d;
				}
				else if (params->depth > 0) // texture 3d
				{
					uint32_t bindingFlags = params->bindingFlags & (kBindingShaderResource | kBindingUnorderedAccess);

					CD3D11_TEXTURE3D_DESC texDesc(
						PixelFormatTable[params->format],
						params->width,
						params->height,
						params->depth,
						1U,
						bindingFlags,
						(params->dynamic == 1 ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT),
						(params->dynamic == 1 ? D3D11_CPU_ACCESS_WRITE : 0U),
						0U
					);

					D3D11_SUBRESOURCE_DATA subResData = {};
					subResData.pSysMem = params->data;
					subResData.SysMemPitch = params->pitch;
					subResData.SysMemSlicePitch = params->slicePitch;

					DXCHECKED(device->CreateTexture3D(
						&texDesc,
						(nullptr == params->data ? nullptr : &subResData),
						&tex3d));

					textures[id].tex = tex3d;

					if (params->bindingFlags & kBindingShaderResource)
					{
						DXCHECKED(device->CreateShaderResourceView(textures[id].tex, nullptr, &(textures[id].srv)));
					}

					if (params->bindingFlags & kBindingUnorderedAccess)
					{
						DXCHECKED(device->CreateUnorderedAccessView(textures[id].tex, nullptr, &(textures[id].uav)));
					}
				}
				else // load the texture from raw data
				{
					CD3D11_TEXTURE2D_DESC texDesc(
						PixelFormatTable[params->format],
						params->width,
						params->height,
						params->arraySize,
						1U,
						bindingFlags,
						(params->dynamic == 1 ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT),
						(params->dynamic == 1 ? D3D11_CPU_ACCESS_WRITE : 0U),
						1U,
						0U,
						(params->cubeMap == 1 ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0U)

					);

					if (texDesc.Format == DXGI_FORMAT_D24_UNORM_S8_UINT && 0u != (params->bindingFlags & kBindingShaderResource))
						texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
					//texDesc.Format = DXGI_FORMAT_R32_TYPELESS;

					D3D11_SUBRESOURCE_DATA subResData = {};
					subResData.pSysMem = params->data;
					subResData.SysMemPitch = params->pitch;

					DXCHECKED(device->CreateTexture2D(
						&texDesc,
						(nullptr == params->data ? nullptr : &subResData),
						&tex2d));

					textures[id].tex = tex2d;

					if (params->bindingFlags & kBindingShaderResource)
					{
						if (params->format == kFormatD24UnormS8Uint)
						{
							//textures[id].tex->GetDesc()
							D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
							desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
							//desc.Format = DXGI_FORMAT_R32_FLOAT;
							if (params->cubeMap == 1)
							{
								desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
								desc.TextureCube.MipLevels = 1;
								desc.TextureCube.MostDetailedMip = 0;
							}
							else if (params->arraySize > 1)
							{
								desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
								desc.Texture2DArray.ArraySize = params->arraySize;
								desc.Texture2DArray.FirstArraySlice = 0;
								desc.Texture2DArray.MipLevels = 1;
								desc.Texture2DArray.MostDetailedMip = 0;
							}
							else
							{
								desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
								desc.Texture2D.MipLevels = 1;
								desc.Texture2D.MostDetailedMip = 0;
							}

							DXCHECKED(device->CreateShaderResourceView(textures[id].tex, &desc, &(textures[id].srv)));
						}
						else
						{
							DXCHECKED(device->CreateShaderResourceView(textures[id].tex, nullptr, &(textures[id].srv)));
						}
					}

					if (params->bindingFlags & kBindingUnorderedAccess)
					{
						DXCHECKED(device->CreateUnorderedAccessView(textures[id].tex, nullptr, &(textures[id].uav)));
					}

					if (params->bindingFlags & kBindingRenderTarget)
					{
						if (params->cubeMap == 1)
						{
							CD3D11_RENDER_TARGET_VIEW_DESC rtvDesc(tex2d,
								D3D11_RTV_DIMENSION_TEXTURE2DARRAY,
								PixelFormatTable[params->format],
								0, 0, 6);

							DXCHECKED(device->CreateRenderTargetView(textures[id].tex, &rtvDesc, &(textures[id].rtv)));
						}
						else
						{
							DXCHECKED(device->CreateRenderTargetView(textures[id].tex, nullptr, &(textures[id].rtv)));
						}
					}

					if (params->bindingFlags & kBindingDepthStencil)
					{
						DXGI_FORMAT dsvForamt = PixelFormatTable[params->format];
						bool isTypeLess = ((params->bindingFlags & kBindingShaderResource) != 0);
						if (isTypeLess)
						{
							dsvForamt = DXGI_FORMAT_D24_UNORM_S8_UINT;
						}

						if (params->cubeMap == 1)
						{
							CD3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc(tex2d,
								D3D11_DSV_DIMENSION_TEXTURE2DARRAY,
								dsvForamt,
								0, 0, 6);

							DXCHECKED(device->CreateDepthStencilView(textures[id].tex, &dsvDesc, &(textures[id].dsv)));
						}
						else
						{
							D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
							dsvDesc.Format = dsvForamt;
							if (params->cubeMap == 1)
							{
								dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
								dsvDesc.Texture2DArray.ArraySize = 6;
								dsvDesc.Texture2DArray.FirstArraySlice = 0;
								dsvDesc.Texture2DArray.MipSlice = 0;
							}
							else if (params->arraySize > 1)
							{
								dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
								dsvDesc.Texture2DArray.ArraySize = params->arraySize;
								dsvDesc.Texture2DArray.FirstArraySlice = 0;
								dsvDesc.Texture2DArray.MipSlice = 0;
							}
							else
							{
								dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
								dsvDesc.Texture2D.MipSlice = 0;
							}

							DXCHECKED(device->CreateDepthStencilView(textures[id].tex, &dsvDesc, &(textures[id].dsv)));
						}
					}
				}

				if (nullptr != tex2d)
				{
					D3D11_TEXTURE2D_DESC desc = {};
					tex2d->GetDesc(&desc);

					textures[id].dynamic = params->dynamic;
					textures[id].cubeMap = ((desc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE) != 0 ? 1 : params->cubeMap);
					textures[id].format = params->isFile ? kFormatAuto : params->format;
					textures[id].arraySize = params->isSlice ? params->arraySize : desc.ArraySize;
					textures[id].bindingFlags = bindingFlags;
					textures[id].width = desc.Width;
					textures[id].height = desc.Height;
					textures[id].depth = 0;
					textures[id].label = params->label;
				}
				else
				{
					D3D11_TEXTURE3D_DESC desc = {};
					tex3d->GetDesc(&desc);

					textures[id].dynamic = params->dynamic;
					textures[id].cubeMap = 0;
					textures[id].format = params->format;
					textures[id].arraySize = 1;
					textures[id].bindingFlags = bindingFlags;
					textures[id].width = desc.Width;
					textures[id].height = desc.Height;
					textures[id].depth = desc.Depth;
					textures[id].label = params->label;
				}

				return kOK;
			}

			int32_t UpdateTexture(void* _params)
			{
				UpdateTextureParams* params = reinterpret_cast<UpdateTextureParams*>(_params);

				assert(true == params->handle && params->handle.id < kMaxTextures);

				uint32_t id = params->handle.id;

				assert(nullptr != textures[id].tex);

				bool updateWhole = (params->left == 0) &&
					(params->top == 0) &&
					(params->front == 0) &&
					(params->right == 0) &&
					(params->bottom == 0) &&
					(params->back == 0);

				if (textures[id].dynamic)
				{
					assert(!updateWhole && "TODO: partially update texture with Map()");

					D3D11_MAPPED_SUBRESOURCE res = {};
					context->Map(textures[id].tex, params->subRes, D3D11_MAP_WRITE_DISCARD, 0, &res);

					const uint8_t* pSrc = reinterpret_cast<const uint8_t*>(params->data);
					uint8_t* pDst = reinterpret_cast<uint8_t*>(res.pData);

					uint32_t depth = textures[id].depth;
					if (depth == 0) depth = 1;

					for (uint32_t iSlice = 0; iSlice < depth; ++iSlice)
					{
						const uint8_t* pSliceSrc = pSrc;
						uint8_t* pSliceDst = pDst;

						for (uint32_t iRow = 0; iRow < textures[id].height; ++iRow)
						{
							memcpy(pDst, pSrc, params->pitch);

							pSrc += params->pitch;
							pDst += res.RowPitch;
						}

						pSliceSrc += params->slicePitch;
						pSliceDst += res.DepthPitch;

						pSrc = pSliceSrc;
						pDst = pSliceDst;
					}

					context->Unmap(textures[id].tex, 0);
				}
				else
				{
					// TODO for mipmaps and texture array
					D3D11_BOX box{
						params->left,
						params->top,
						params->front,
						params->right,
						params->bottom,
						params->back
					};

					context->UpdateSubresource(textures[id].tex, params->subRes,
						updateWhole ? nullptr : &box,
						params->data, params->pitch, params->slicePitch);
				}

				return kOK;
			}

			int32_t DestroyTexture(void* params)
			{
				TextureHandle* handle = reinterpret_cast<TextureHandle*>(params);

				assert(true == *handle && handle->id < kMaxTextures);

				uint32_t id = handle->id;

				RELEASE(textures[id].srv);
				RELEASE(textures[id].uav);
				RELEASE(textures[id].rtv);
				RELEASE(textures[id].dsv);
				RELEASE(textures[id].tex);

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

				samplerDesc.Filter = FilterTable[params->filter];
				samplerDesc.MaxAnisotropy = params->maxAnisotropy;

				samplerDesc.AddressU = (D3D11_TEXTURE_ADDRESS_MODE)params->textureAddressU;
				samplerDesc.AddressV = (D3D11_TEXTURE_ADDRESS_MODE)params->textureAddressV;
				samplerDesc.AddressW = (D3D11_TEXTURE_ADDRESS_MODE)params->textureAddressW;

				DXCHECKED(device->CreateSamplerState(&samplerDesc, &(samplers[id].samp)));
				samplers[id].label = params->label;

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
				void* ptr = nullptr;
				if (params->label == kResourceGlobal)
				{
					ptr = MemoryAllocator::Allocators[kAllocGlobal].Allocate(params->size, 4);
				}
				else
				{
					ptr = MemoryAllocator::Allocators[kAllocLevel].Allocate(params->size, 4);
				}
				assert(nullptr != ptr);
				memcpy(ptr, params->data, params->size);

				DXCHECKED(device->CreateVertexShader(ptr, params->size, nullptr, &(vertexShaders[id].shader)));

				vertexShaders[id].data = ptr;
				vertexShaders[id].size = params->size;
				vertexShaders[id].label = params->label;

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
				void* ptr = nullptr;
				if (params->label == kResourceGlobal)
				{
					ptr = MemoryAllocator::Allocators[kAllocGlobal].Allocate(params->size, 4);
				}
				else
				{
					ptr = MemoryAllocator::Allocators[kAllocLevel].Allocate(params->size, 4);
				}
				assert(nullptr != ptr);
				memcpy(ptr, params->data, params->size);

				DXCHECKED(device->CreatePixelShader(params->data, params->size, nullptr, &(pixelShaders[id].shader)));

				pixelShaders[id].data = ptr;
				pixelShaders[id].size = params->size;
				pixelShaders[id].label = params->label;

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

			int32_t CreateComputeShader(void* _params)
			{
				CreateComputeShaderParams* params = reinterpret_cast<CreateComputeShaderParams*>(_params);

				assert(true == params->handle);

				uint32_t id = params->handle.id;

				assert(nullptr == computeShaders[id].shader);

				// store binary code for further use
				void* ptr = nullptr;
				if (params->label == kResourceGlobal)
				{
					ptr = MemoryAllocator::Allocators[kAllocGlobal].Allocate(params->size, 4);
				}
				else
				{
					ptr = MemoryAllocator::Allocators[kAllocLevel].Allocate(params->size, 4);
				}
				assert(nullptr != ptr);
				memcpy(ptr, params->data, params->size);

				DXCHECKED(device->CreateComputeShader(params->data, params->size, nullptr, &(computeShaders[id].shader)));

				computeShaders[id].data = ptr;
				computeShaders[id].size = params->size;
				computeShaders[id].label = params->label;

				return kOK;
			}

			int32_t DestroyComputeShader(void* params)
			{
				ComputeShaderHandle* handle = reinterpret_cast<ComputeShaderHandle*>(params);

				assert(true == *handle);

				uint32_t id = handle->id;

				assert(nullptr != computeShaders[id].shader);

				computeShaders[id].shader->Release();

				computeShaders[id] = {};

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
				dsState.StencilEnable = params->stencilEnable;
				dsState.FrontFace.StencilFailOp = (D3D11_STENCIL_OP)params->frontFaceFailOp;
				dsState.FrontFace.StencilDepthFailOp = (D3D11_STENCIL_OP)params->frontFaceDepthFailOp;
				dsState.FrontFace.StencilPassOp = (D3D11_STENCIL_OP)params->frontFacePassOp;
				dsState.FrontFace.StencilFunc = (D3D11_COMPARISON_FUNC)(params->frontFaceFunc + 1);
				dsState.BackFace.StencilFailOp = (D3D11_STENCIL_OP)params->backFaceFailOp;
				dsState.BackFace.StencilDepthFailOp = (D3D11_STENCIL_OP)params->backFaceDepthFailOp;
				dsState.BackFace.StencilPassOp = (D3D11_STENCIL_OP)params->backFacePassOp;
				dsState.BackFace.StencilFunc = (D3D11_COMPARISON_FUNC)(params->backFaceFunc + 1);
				dsState.StencilReadMask = params->stencilReadMask;
				dsState.StencilWriteMask = params->stencilWriteMask;
				DXCHECKED(device->CreateDepthStencilState(&dsState, &(pipelineStates[id].depthStencilState)));

				CD3D11_RASTERIZER_DESC rsState(D3D11_DEFAULT);
				rsState.CullMode = (D3D11_CULL_MODE)(params->cullMode + 1u);
				rsState.FrontCounterClockwise = (params->frontFaceCcw == 1 ? TRUE : FALSE);

				DXCHECKED(device->CreateRasterizerState(&rsState, &(pipelineStates[id].rasterizerState)));

				CD3D11_BLEND_DESC blendState(D3D11_DEFAULT);
				blendState.RenderTarget[0].BlendEnable = params->blendEnable;
				blendState.RenderTarget[0].SrcBlend = (D3D11_BLEND)params->srcBlend;
				blendState.RenderTarget[0].DestBlend = (D3D11_BLEND)params->destBlend;
				blendState.RenderTarget[0].BlendOp = (D3D11_BLEND_OP)params->blendOp;
				blendState.RenderTarget[0].SrcBlendAlpha = (D3D11_BLEND)params->srcBlendAlpha;
				blendState.RenderTarget[0].DestBlendAlpha = (D3D11_BLEND)params->destBlendAlpha;
				blendState.RenderTarget[0].BlendOpAlpha = (D3D11_BLEND_OP)params->blendOpAlpha;
				blendState.RenderTarget[0].RenderTargetWriteMask = (D3D11_COLOR_WRITE_ENABLE)params->blendWriteMask;
				DXCHECKED(device->CreateBlendState(&blendState, &(pipelineStates[id].blendState)));

				assert(true == params->vertexShader && nullptr != vertexShaders[params->vertexShader.id].shader);
				//assert(true == params->pixelShader && nullptr != pixelShaders[params->pixelShader.id].shader);

				DXCHECKED(device->CreateInputLayout(
					InputElemDescTable[params->vertexFormat],
					InputElemDescSizeTable[params->vertexFormat],
					vertexShaders[params->vertexShader.id].data,
					vertexShaders[params->vertexShader.id].size,
					&(pipelineStates[id].inputLayout)
				));


				pipelineStates[id].viewport = {
					params->viewport.topLeftX, params->viewport.topLeftY,
					params->viewport.width, params->viewport.height,
					params->viewport.minZ, params->viewport.maxZ
				};

				pipelineStates[id].vertexShader = params->vertexShader;
				pipelineStates[id].pixelShader = params->pixelShader;

				pipelineStates[id].label = params->label;

				pipelineStates[id].stencilRef = params->stencilRef;

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

				if (params->renderTargets[0].id == kDefaultRenderTarget.id)
					params->renderTargets[0] = TextureHandle(kMaxTextures + 1);

				if (params->depthRenderTarget.id == kDefaultRenderTarget.id)
					params->depthRenderTarget = TextureHandle(kMaxTextures);

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
					context->PSSetShader((pso.pixelShader ? pixelShaders[pso.pixelShader.id].shader : nullptr), nullptr, 0);
					context->RSSetState(pso.rasterizerState);
					context->RSSetViewports(1, &(pso.viewport));
					context->OMSetDepthStencilState(pso.depthStencilState, pso.stencilRef);
					context->OMSetBlendState(pso.blendState, nullptr, 0xffffffffu);

					currentPipelineState = params->pipelineState;
				}

				// render targets
				CHECKED(BindRenderTargets(params));

				// bind vs and ps resources
				CHECKED(BindShaderResources(params));

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
					if (true == params->indexBuffer)
					{
						Buffer& ib = buffers[params->indexBuffer.id];
						if (!(ib.bindingFlags & kBindingIndexBuffer))
						{
							return kErrUnknown;
						}
						assert(nullptr != ib.buf);
						context->IASetIndexBuffer(ib.buf, DXGI_FORMAT_R16_UINT, 0);

						// draw it!
						if (params->instanceCount > 0)
						{
							context->DrawIndexedInstanced(params->indexCount, params->instanceCount, params->startIndex, params->startVertex, 0);
						}
						else
						{
							context->DrawIndexed(params->indexCount, params->startIndex, params->startVertex);
						}
					}
					else
					{
						if (params->instanceCount > 0)
						{
							context->DrawInstanced(params->indexCount, params->instanceCount, params->startVertex, 0);
						}
						else
						{
							context->Draw(params->indexCount, params->startVertex);
						}
					}

				}
				//context->RSSetState(0);
				return kOK;
			}

			int32_t Compute(void* _params)
			{
				ComputeParams* params = reinterpret_cast<ComputeParams*>(_params);

				if (!(params->shader))
					return kErrUnknown;

				ComputeShader& cs = computeShaders[params->shader.id];
				if (nullptr == cs.shader)
					return kErrUnknown;

				context->CSSetShader(cs.shader, nullptr, 0);

				CHECKED(BindShaderResources(params));

				context->Dispatch(params->threadGroupCountX, params->threadGroupCountY, params->threadGroupCountZ);

				{
					// texture bindings
					ID3D11ShaderResourceView* srvs[kMaxTextureBindings] = {};
					context->CSSetShaderResources(0, kMaxTextureBindings, srvs);
				}
				{
					// rw texture bindings
					ID3D11UnorderedAccessView* uavs[kMaxTextureBindings] = {};
					context->CSSetUnorderedAccessViews(0, kMaxTextureBindings, uavs, nullptr);
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