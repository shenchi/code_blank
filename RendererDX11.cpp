#include "Renderer.h"
#include "RendererDX11.h"

#include "NativeContext.h"

#include <Windows.h>
#include <d3d11_1.h>

#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>

#include <cassert>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#define RELEASE(x) if (nullptr != (x)) { (x)->Release(); (x) = nullptr; }
#define C(x, ret) if ((x) != S_OK) { return (ret); }
#define CHECKED(x) if ((x) != S_OK) { return false; }

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

	DXGI_FORMAT PixelFormatTable[] =
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


	struct Buffer
	{
		ID3D11Buffer*				buf;
		ID3D11ShaderResourceView*	srv;
		uint32_t					size;
	};

	struct Texture
	{
		ID3D11Texture2D*			tex;
		ID3D11ShaderResourceView*	srv;
		ID3D11RenderTargetView*		rtv;
		ID3D11DepthStencilView*		dsv;
		uint32_t					width;
		uint32_t					height;
		uint32_t					pitch;


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

				return TF_OK;
			}

			virtual int32_t Release() override
			{
				assert(false && "TODO: release resources");


				swapChain->Release();
				context->Release();
				device->Release();

				return TF_OK;
			}

			virtual int32_t Submit(RendererCommandBuffer* buffer) override
			{
				assert(nullptr != buffer);
				for (uint32_t i = 0; i < buffer->size; ++i)
				{
					cmd_callback_t cmd = commands[buffer->cmds[i]];
					int32_t err = (this->*cmd)(buffer->params[i]);
					if (err != TF_OK)
					{
						return err;
					}
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


		private:
			int							width;
			int							height;

			HWND						hWnd;

			IDXGISwapChain1*			swapChain;
			ID3D11Device1*				device;
			ID3D11DeviceContext1*		context;

			Buffer						buffers[MAX_BUFFERS];
			Texture						textures[MAX_TEXTURES + 2];

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
				width = rect.right - rect.left;
				height = rect.bottom - rect.top;

				DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};

				swapChainDesc.Width = width;
				swapChainDesc.Height = height;
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

				{
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
					rt.width = width;
					rt.height = height;

					if (S_OK != (hr = device->CreateRenderTargetView(backbufferTex, nullptr, &(rt.rtv))))
					{
						return -1;
					}
					rt.tex = backbufferTex;
				}

				{
					Texture& ds = textures[MAX_TEXTURES];

					//ds.Reset(TextureType::TEXTURE_2D, PixelFormat::FORMAT_D24_UNORM_S8_UINT, width, height);
					ds = {};
					ds.width = width;
					ds.height = height;

					ID3D11Texture2D* depthStencilTex = nullptr;
					D3D11_TEXTURE2D_DESC depthDesc{ 0 };
					depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
					depthDesc.Width = width;
					depthDesc.Height = height;
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
				}

				return 0;
			}
			
			int32_t Nop(void*) { return TF_OK; }

			int32_t CreateBuffer(void* params)
			{
				return TF_OK;
			}

			int32_t UpdateBuffer(void* params)
			{
				return TF_OK;
			}

			int32_t DestroyBuffer(void* params)
			{
				return TF_OK;
			}

			int32_t CreateTexture(void* params)
			{
				return TF_OK;
			}

			int32_t UpdateTexture(void* params)
			{
				return TF_OK;
			}

			int32_t DestroyTexture(void* params)
			{
				return TF_OK;
			}

			int32_t CreateSampler(void* params)
			{
				return TF_OK;
			}

			int32_t DestroySampler(void* params)
			{
				return TF_OK;
			}

			int32_t CreateVertexShader(void* params)
			{
				return TF_OK;
			}

			int32_t DestroyVertexShader(void* params)
			{
				return TF_OK;
			}

			int32_t CreatePixelShader(void* params)
			{
				return TF_OK;
			}

			int32_t DestroyPixelShader(void* params)
			{
				return TF_OK;
			}

			int32_t CreatePipelineState(void* params)
			{
				return TF_OK;
			}

			int32_t DestroyPipelineState(void* params)
			{
				return TF_OK;
			}

			int32_t Draw(void* params)
			{
				return TF_OK;
			}

		};



		Renderer * CreateRendererDX11()
		{
			return new RendererDX11();
		}

	}
}