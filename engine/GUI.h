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

	class GUILayer
	{
	private:
		TextureHandle	tex;

		float*			widgetVerts;
		float*			textVerts;

		uint32_t		numWidgetVerts;
		uint32_t		numTextVerts;

	public:
		void Text(float x, float y, float size, const char* text, TextAlign align = kTextAlignLeft);

		void Image(float x, float y, float w, float h, TextureHandle tex);
	};

	class GUI
	{
		SINGLETON_DECL(GUI);

	private:
		float		width;
		float		height;

	public:

		// set the logic size of the screen
		void SetScreenSize(float width, float height);


	};
}