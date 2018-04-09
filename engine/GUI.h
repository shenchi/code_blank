#pragma once

#include "Common.h"

namespace tofu
{
	enum TextAlign
	{
		kTextAlignLeft,
		kTextAlignCenter,
		kTextAlignRight,
	};

	struct GUILayer
	{
		TextureHandle	tex;

		float*			widgetVerts;
		float*			textVerts;

		uint32_t		numWidgetVerts;
		uint32_t		numTextVerts;

	};

	class GUI
	{
		SINGLETON_DECL(GUI);

	private:
		float		width;
		float		height;

		GUILayer	layers[kMaxGUILayers];

	public:

		// set the logic size of the screen
		void SetScreenSize(float width, float height);

		// clear all data for new frame
		void Reset();

		void Render();

	public:

		// initialize for a layer
		void SetupLayer(uint32_t layer, TextureHandle tex = TextureHandle(), uint32_t maxWidgets = 512, uint32_t maxCharacters = 512);

		void Text(uint32_t layer, float x, float y, float size, const char* text, TextAlign align = kTextAlignLeft);

		void Image(uint32_t layer, float x, float y, float w, float h, float u0 = 0.0f, float v0 = 0.0f, float u1 = 1.0f, float v1 = 1.0f);
	};
}