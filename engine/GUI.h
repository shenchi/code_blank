#pragma once

#include "Common.h"
#include "TofuMath.h"

struct FONScontext;

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
		uint32_t		maxWidgetVerts;
		uint32_t		maxTextVerts;

		uint32_t		startWidgetVert;
		uint32_t		startTextVert;
	};

	struct GUIStyle
	{
		math::float4	normalColor;
		math::float4	highlightedColor;

		math::float4	normalUVs;
		math::float4	highlightedUVs;
	};

	struct Atlas
	{
		math::float4	rects[64];
		uint32_t		numTextures;

		int32_t LoadFromFile(const char* filename);
	};

	struct RendererCommandBuffer; 
	class RenderingSystem;

	class GUI
	{
		SINGLETON_DECL(GUI);

	private:

		static int TextRenderCreate(void* userPtr, int width, int height);

		static int TextRenderResize(void* userPtr, int width, int height);

		static void TextRenderUpdate(void* userPtr, int* rect, const unsigned char* data);

		static void TextRenderDraw(void* userPtr, const float* verts, const float* tcoords, const unsigned int* colors, int nverts);

		static void TextRenderDelete(void* userPtr);

	private:
		float					width;
		float					height;

		float					fbWidth;
		float					fbHeight;

		GUILayer				layers[kMaxGUILayers];

		FONScontext*			fonsContext;
		int32_t					font;

		uint32_t				fontTexWidth;
		uint32_t				fontTexHeight;
		int32_t					fontTexUpdateLeft;
		int32_t					fontTexUpdateTop;
		int32_t					fontTexUpdateRight;
		int32_t					fontTexUpdateBottom;

	private:
		TextureHandle			fontTex;
		uint32_t				fontTexLastWidth;
		uint32_t				fontTexLastHeight;
		const uint8_t*			fontTexData;

		uint32_t				currentLayer;

	private:
		BufferHandle			vertBuf;
		uint32_t				maxVerts;
		uint32_t				initialMaxVerts;

	private:
		friend class RenderingSystem;
		GUI() = default;

		void SetFrameBufferSize(float width, float height);

	public:

		int32_t Init(BufferHandle vertexBuffer, uint32_t initialMaxVertices, TextureHandle fontTex);

		int32_t Shutdown();

		// set the logic size of the screen
		void SetCanvasSize(float width, float height);

		// clear all data for new frame
		void Reset();

		int32_t Render(RendererCommandBuffer* cmdBuf, BufferHandle frameConstants, PipelineStateHandle overlayPSO, PipelineStateHandle fontPSO, SamplerHandle samp);

	public:

		// initialize for a layer
		void SetupLayer(uint32_t layer, TextureHandle tex = TextureHandle(), uint32_t maxWidgets = 512, uint32_t maxCharacters = 512);

		// draw text
		void Text(uint32_t layer, float x, float y, float size, const char* text, TextAlign align = kTextAlignLeft);

		// draw part of a texture
		void Texture(uint32_t layer, float x, float y, float w, float h, float u0 = 0.0f, float v0 = 0.0f, float u1 = 1.0f, float v1 = 1.0f, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);

	private:

		bool			focused;
		bool			highlighted;
		uint32_t		selectedMenuItem;
		uint32_t		selectedSwitchItem;

		uint32_t		currentMenuIndex;
		uint32_t		currentSwitchIndex;

	public:

		void BeginMenu(uint32_t layer, uint32_t selectedIndex = 0, bool focused = true);

		uint32_t EndMenu();

		void BeginMenuItem(uint32_t layer);

		void EndMenuItem();



		void Label(uint32_t layer, float x, float y, float w, float h, float fontSize, const char * text, const GUIStyle & style);

		void Image(uint32_t layer, float x, float y, float w, float h, const GUIStyle& style);
	};
}