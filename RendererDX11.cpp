#include "Renderer.h"
#include "RendererDX11.h"

namespace tofu
{
	namespace dx11
	{
		class RendererDX11 : public Renderer
		{
			virtual int32_t Init() override
			{

			}

			virtual int32_t Release() override
			{

			}

			virtual int32_t Submit(uint32_t count, RendererCommandBuffer* buffers) override
			{

			}

			virtual int32_t Present() override
			{

			}
		};



		Renderer * CreateRendererDX11()
		{
			return new RendererDX11();
		}

	}
}