#include "GUI.h"

#include "MemoryAllocator.h"

namespace tofu
{
	SINGLETON_IMPL(GUI);

	void GUI::SetScreenSize(float width, float height)
	{
		this->width = width;
		this->height = height;
	}

	void GUI::Reset()
	{
		for (uint32_t i = 0; i < kMaxGUILayers; i++)
		{
			layers[i] = {};
		}
	}

	void GUI::SetupLayer(uint32_t layer, TextureHandle tex, uint32_t maxWidgets, uint32_t maxCharacters)
	{
		if (layer >= kMaxGUILayers) return;

		layers[layer] = {};
		layers[layer].tex = tex;
		
		constexpr uint32_t vertexSize = sizeof(float) * 9;
		if (maxWidgets > 0)
		{
			layers[layer].widgetVerts = reinterpret_cast<float*>(
				MemoryAllocator::FrameAlloc(maxWidgets * 4 * vertexSize));
		}
		if (maxCharacters > 0)
		{
			layers[layer].textVerts = reinterpret_cast<float*>(
				MemoryAllocator::FrameAlloc(maxCharacters * 4 * vertexSize));
		}
	}

	void GUI::Text(uint32_t layer, float x, float y, float size, const char* text, TextAlign align)
	{
		if (layer >= kMaxGUILayers) return;
		if (nullptr == layers[layer].textVerts) return;
		
	}

	void GUI::Image(uint32_t layer, float x, float y, float w, float h, float u0, float v0, float u1, float v1)
	{
		if (layer >= kMaxGUILayers) return;
		if (nullptr == layers[layer].widgetVerts) return;

	}


}
