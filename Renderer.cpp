#include "Renderer.h"

#ifdef _MSC_VER
#include "RendererDX11.h"
#endif

namespace tofu
{

	Renderer* Renderer::CreateRenderer()
	{
#ifdef _MSC_VER
		return dx11::CreateRendererDX11();
#else
		return nullptr;
#endif
	}

}