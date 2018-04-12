#include "GUI.h"

#include "MemoryAllocator.h"
#include "TofuMath.h"
#include "Renderer.h"
#include "InputSystem.h"
#include "FileIO.h"

#include <rapidjson/document.h>

extern "C"
{
#include <fontstash.h>
}

namespace
{
	void FillVertex(float* ptr, float x, float y, float z, float r, float g, float b, float a, float u, float v)
	{
		*(ptr + 0) = x;
		*(ptr + 1) = y;
		*(ptr + 2) = z;

		*(ptr + 3) = r;
		*(ptr + 4) = g;
		*(ptr + 5) = b;
		*(ptr + 6) = a;

		*(ptr + 7) = u;
		*(ptr + 8) = v;
	}
}

namespace tofu
{
	SINGLETON_IMPL(GUI);

	int GUI::TextRenderCreate(void* userPtr, int width, int height)
	{
		GUI* gui = reinterpret_cast<GUI*>(userPtr);
		gui->fontTexWidth = uint32_t(width);
		gui->fontTexHeight = uint32_t(height);

		return 1;
	}

	int GUI::TextRenderResize(void* userPtr, int width, int height)
	{
		GUI* gui = reinterpret_cast<GUI*>(userPtr);

		if (!TextRenderCreate(userPtr, width, height))
		{
			return 0;
		}

		gui->fontTexUpdateLeft = 0;
		gui->fontTexUpdateTop = 0;
		gui->fontTexUpdateRight = gui->fontTexWidth;
		gui->fontTexUpdateBottom = gui->fontTexHeight;

		return 1;
	}

	void GUI::TextRenderUpdate(void* userPtr, int* rect, const unsigned char* data)
	{
		GUI* gui = reinterpret_cast<GUI*>(userPtr);
		
		gui->fontTexData = data;

		if (gui->fontTexUpdateLeft == gui->fontTexUpdateRight || 
			gui->fontTexUpdateTop == gui->fontTexUpdateBottom)
		{
			gui->fontTexUpdateLeft = rect[0];
			gui->fontTexUpdateTop = rect[1];
			gui->fontTexUpdateRight = rect[2];
			gui->fontTexUpdateBottom = rect[3];
		}
		else
		{
			gui->fontTexUpdateLeft = math::min(gui->fontTexUpdateLeft, rect[0]);
			gui->fontTexUpdateTop = math::min(gui->fontTexUpdateTop, rect[1]);
			gui->fontTexUpdateRight = math::max(gui->fontTexUpdateRight, rect[2]);
			gui->fontTexUpdateBottom = math::max(gui->fontTexUpdateBottom, rect[3]);
		}
	}

	void GUI::TextRenderDraw(void* userPtr, const float* verts, const float* tcoords, const unsigned int* colors, int nverts)
	{
		GUI* gui = reinterpret_cast<GUI*>(userPtr);

		if (gui->currentLayer > kMaxGUILayers)
			return;

		GUILayer& layer = gui->layers[gui->currentLayer];

		if (layer.maxTextVerts == 0 || nullptr == layer.textVerts)
			return;

		if (layer.numTextVerts + nverts > layer.maxTextVerts) return;

		float scale = gui->fbHeight / gui->height;

		for (int32_t i = 0; i < nverts; i++)
		{
			uint16_t vid = uint16_t(layer.numTextVerts + i);
			float* vert = layer.textVerts + (vid * 9);

			*(vert + 0) = verts[i * 2 + 0] * scale;
			*(vert + 1) = verts[i * 2 + 1] * scale;
			*(vert + 2) = 0.0f;

			*(vert + 3) = 1.0f;
			*(vert + 4) = 1.0f;
			*(vert + 5) = 1.0f;
			*(vert + 6) = 1.0f;

			*(vert + 7) = tcoords[i * 2 + 0];
			*(vert + 8) = tcoords[i * 2 + 1];
		}

		layer.numTextVerts += nverts;
	}

	void GUI::TextRenderDelete(void* userPtr)
	{
	}

	void GUI::SetFrameBufferSize(float width, float height)
	{
		fbWidth = width;
		fbHeight = height;
	}

	int32_t GUI::Init(BufferHandle vertexBuffer, uint32_t initialMaxVertices, TextureHandle fontTex)
	{
		FONSparams params = {};
		params.width = 1024;
		params.height = 1024;
		params.flags = FONS_ZERO_TOPLEFT;
		params.userPtr = this;

		params.renderCreate = TextRenderCreate;
		params.renderResize = TextRenderResize;
		params.renderUpdate = TextRenderUpdate;
		params.renderDraw = TextRenderDraw;
		params.renderDelete = TextRenderDelete;

		fonsContext = fonsCreateInternal(&params);
		if (nullptr == fonsContext)
			return kErrUnknown;

		font = fonsAddFont(fonsContext, "Conthrax Sb", "assets/conthrax-sb.ttf");
		//font = fonsAddFont(fonsContext, "Arial Regular", "C:\\Windows\\Fonts\\arial.ttf");
		//font = fonsAddFont(fonsContext, "sans", "D:\\DroidSerif-Regular.ttf");
		if (font == FONS_INVALID) {
			return kErrUnknown;
		}

		vertBuf = vertexBuffer;
		initialMaxVerts = initialMaxVertices;
		this->fontTex = fontTex;

		_instance = this;

		return kOK;
	}

	int32_t GUI::Shutdown()
	{
		fonsDeleteInternal(fonsContext);

		// let renderer clean up resources itself

		return kOK;
	}

	void GUI::SetCanvasSize(float width, float height)
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

		fontTexData = nullptr;
		fontTexUpdateLeft = 0;
		fontTexUpdateTop = 0;
		fontTexUpdateRight = 0;
		fontTexUpdateBottom = 0;
	}

	int32_t GUI::Render(RendererCommandBuffer* cmdBuf, BufferHandle frameConstants, PipelineStateHandle overlayPSO, PipelineStateHandle fontPSO, SamplerHandle samp)
	{
		if (fontTexWidth != fontTexLastWidth || fontTexHeight != fontTexLastHeight)
		{
			if (fontTexLastWidth > 0 && fontTexLastHeight > 0 && fontTex)
			{
				// delete last one
				cmdBuf->Add(RendererCommand::kCommandDestroyTexture, &fontTex);
			}
			
			// create new font texture
			if (fontTexWidth > 0 && fontTexHeight > 0 && fontTex)
			{
				CreateTextureParams* params = MemoryAllocator::FrameAlloc<CreateTextureParams>();
				params->InitAsTexture2D(fontTex, fontTexWidth, fontTexHeight, kFormatR8Unorm);

				cmdBuf->Add(RendererCommand::kCommandCreateTexture, params);
			}

			fontTexLastWidth = fontTexWidth;
			fontTexLastHeight = fontTexHeight;
		}

		if (nullptr != fontTexData && fontTexUpdateRight > fontTexUpdateLeft && fontTexUpdateBottom > fontTexUpdateTop)
		{
			uint8_t* ptr = const_cast<uint8_t*>(fontTexData);
			ptr += fontTexUpdateLeft + fontTexUpdateTop * fontTexWidth;

			UpdateTextureParams* params = MemoryAllocator::FrameAlloc<UpdateTextureParams>();
			params->handle = fontTex;
			params->data = ptr;
			params->pitch = fontTexWidth;
			params->left = fontTexUpdateLeft;
			params->top = fontTexUpdateTop;
			params->right = fontTexUpdateRight;
			params->bottom = fontTexUpdateBottom;
			params->front = 0;
			params->back = 1;

			cmdBuf->Add(RendererCommand::kCommandUpdateTexture, params);

			fontTexData = nullptr;
			fontTexUpdateLeft = 0;
			fontTexUpdateTop = 0;
			fontTexUpdateRight = 0;
			fontTexUpdateBottom = 0;
		}

		uint32_t bufferSize = 0;
		constexpr uint32_t vertexSize = sizeof(float) * 9;

		for (uint32_t i = 0; i < kMaxGUILayers; i++)
		{
			// for widgets
			if (layers[i].numWidgetVerts > 0 && nullptr != layers[i].widgetVerts)
			{
				layers[i].startWidgetVert = bufferSize;
				bufferSize += layers[i].numWidgetVerts;
			}

			// for texts
			if (layers[i].numTextVerts > 0 && nullptr != layers[i].textVerts)
			{
				layers[i].startTextVert = bufferSize;
				bufferSize += layers[i].numTextVerts;
			}
		}

		if (bufferSize > maxVerts)
		{
			if (maxVerts > 0)
			{
				cmdBuf->Add(RendererCommand::kCommandCreateBuffer, &vertBuf);
				maxVerts = ((bufferSize + 255u) & (~255u));
			}
			else
			{
				maxVerts = initialMaxVerts;
			}

			{
				CreateBufferParams* params = MemoryAllocator::FrameAlloc<CreateBufferParams>();
				params->handle = vertBuf;
				params->bindingFlags = kBindingVertexBuffer;
				params->dynamic = 1;
				params->data = nullptr;
				params->size = vertexSize * maxVerts;
				params->stride = vertexSize;
				params->label = kResourceGlobal;

				cmdBuf->Add(RendererCommand::kCommandCreateBuffer, params);
			}
		}

		if (0 == bufferSize) 
			return kOK;

		bufferSize *= vertexSize;

		uint8_t* buffer = reinterpret_cast<uint8_t*>(MemoryAllocator::FrameAlloc(bufferSize));
		if (nullptr == buffer) 
			return kErrUnknown;

		for (uint32_t i = 0; i < kMaxGUILayers; i++)
		{
			// for widgets
			if (layers[i].numWidgetVerts > 0 && nullptr != layers[i].widgetVerts)
			{
				void* dst = buffer + (layers[i].startWidgetVert * vertexSize);
				memcpy(dst, layers[i].widgetVerts, layers[i].numWidgetVerts * vertexSize);
			}

			// for texts
			if (layers[i].numTextVerts > 0 && nullptr != layers[i].textVerts)
			{
				void* dst = buffer + (layers[i].startTextVert * vertexSize);
				memcpy(dst, layers[i].textVerts, layers[i].numTextVerts * vertexSize);
			}
		}

		{
			UpdateBufferParams* params = MemoryAllocator::FrameAlloc<UpdateBufferParams>();
			params->handle = vertBuf;
			params->data = buffer;
			params->size = bufferSize;
			cmdBuf->Add(RendererCommand::kCommandUpdateBuffer, params);
		}

		for (uint32_t i = 0; i < kMaxGUILayers; i++)
		{
			// for widgets
			if (layers[i].numWidgetVerts > 0 && nullptr != layers[i].widgetVerts)
			{
				DrawParams* params = MemoryAllocator::FrameAlloc<DrawParams>();

				params->pipelineState = overlayPSO;
				params->vertexBuffer = vertBuf;
				params->indexBuffer = BufferHandle();

				params->vsConstantBuffers[0] = { frameConstants, 0, 0 };
				params->psShaderResources[0] = layers[i].tex;
				params->psSamplers[0] = samp;

				params->indexCount = layers[i].numWidgetVerts;
				params->startVertex = layers[i].startWidgetVert;

				cmdBuf->Add(RendererCommand::kCommandDraw, params);
			}

			// for texts
			if (layers[i].numTextVerts > 0 && nullptr != layers[i].textVerts)
			{
				DrawParams* params = MemoryAllocator::FrameAlloc<DrawParams>();

				params->pipelineState = fontPSO;
				params->vertexBuffer = vertBuf;
				params->indexBuffer = BufferHandle();

				params->vsConstantBuffers[0] = { frameConstants, 0, 0 };
				params->psShaderResources[0] = fontTex;
				params->psSamplers[0] = samp;

				params->indexCount = layers[i].numTextVerts;
				params->startVertex = layers[i].startTextVert;

				cmdBuf->Add(RendererCommand::kCommandDraw, params);
			}
		}

		return kOK;
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
				MemoryAllocator::FrameAlloc(maxWidgets * 6 * vertexSize));
			layers[layer].maxWidgetVerts = maxWidgets * 6;
		}
		if (maxCharacters > 0)
		{
			layers[layer].textVerts = reinterpret_cast<float*>(
				MemoryAllocator::FrameAlloc(maxCharacters * 6 * vertexSize));
			layers[layer].maxTextVerts = maxCharacters * 6;
		}
	}

	void GUI::Text(uint32_t layer, float x, float y, float size, const char* text, TextAlign align)
	{
		if (layer >= kMaxGUILayers) return;
		if (nullptr == layers[layer].textVerts) return;
		
		currentLayer = layer;

		float lh = 0;
		fonsClearState(fonsContext);
		fonsSetSize(fonsContext, 18.0f);
		fonsSetFont(fonsContext, font);
		fonsVertMetrics(fonsContext, nullptr, nullptr, &lh);
		fonsSetColor(fonsContext, 0xffffffffu);

		// it will flush the buffer
		x = fonsDrawText(fonsContext, x, y + lh, text, nullptr);
	}

	void GUI::Texture(uint32_t layer, float x, float y, float w, float h, float u0, float v0, float u1, float v1, float r, float g, float b, float a)
	{
		if (layer >= kMaxGUILayers) return;
		if (nullptr == layers[layer].widgetVerts) return;

		if (layers[layer].numWidgetVerts + 6 > layers[layer].maxWidgetVerts)
			return;

		float* ptr = layers[layer].widgetVerts + (layers[layer].numWidgetVerts * 9);

		float scale = fbHeight / height;

		float x0 = x * scale;
		float y0 = y * scale;
		float x1 = (x + w) * scale;
		float y1 = (y + h) * scale;

		FillVertex(ptr, x0, y0, 0, r, g, b, a, u0, v0); ptr += 9;
		FillVertex(ptr, x1, y0, 0, r, g, b, a, u1, v0); ptr += 9;
		FillVertex(ptr, x1, y1, 0, r, g, b, a, u1, v1); ptr += 9;
		FillVertex(ptr, x0, y0, 0, r, g, b, a, u0, v0); ptr += 9;
		FillVertex(ptr, x1, y1, 0, r, g, b, a, u1, v1); ptr += 9;
		FillVertex(ptr, x0, y1, 0, r, g, b, a, u0, v1);

		layers[layer].numWidgetVerts += 6;
	}

	void GUI::BeginMenu(uint32_t layer, uint32_t selectedIndex, bool focused)
	{
		currentMenuIndex = 0;
		selectedMenuItem = selectedIndex;
		this->focused = focused;
	}

	uint32_t GUI::EndMenu()
	{
		if (focused)
		{
			InputSystem& input = *(InputSystem::instance());
			if (input.IsButtonReleased(kKeyDown))
			{
				selectedMenuItem++;
				if (selectedMenuItem >= currentMenuIndex)
				{
					selectedMenuItem = 0;
				}
			}
			else if (input.IsButtonReleased(kKeyUp))
			{
				if (selectedMenuItem == 0)
				{
					selectedMenuItem = currentMenuIndex - 1;
				}
				else
				{
					selectedMenuItem--;
				}
			}
		}
		return selectedMenuItem;
	}

	void GUI::BeginMenuItem(uint32_t layer)
	{
		if (focused)
		{
			highlighted = (currentMenuIndex == selectedMenuItem);
		}
	}

	void GUI::EndMenuItem()
	{
		currentMenuIndex++;
	}

	void GUI::Label(uint32_t layer, float x, float y, float w, float h, float fontSize, const char * text, const GUIStyle & style)
	{
	}

	void GUI::Image(uint32_t layer, float x, float y, float w, float h, const GUIStyle & style)
	{
		math::float4 uvs = style.normalUVs;
		math::float4 color = style.normalColor;

		if (highlighted)
		{
			uvs = style.highlightedUVs;
			color = style.highlightedColor;
		}

		Texture(layer, x, y, w, h, 
			uvs.x, uvs.y, uvs.z, uvs.w,
			color.x, color.y, color.z, color.w);
	}

	int32_t Atlas::LoadFromFile(const char* filename)
	{
		char* json = nullptr;
		CHECKED(FileIO::ReadFile(filename, true, 4, (void**)&json, nullptr));

		rapidjson::Document doc;
		doc.Parse(json);

		if (doc.HasParseError() || !doc.HasMember("atlas"))
			return kErrUnknown;

		const rapidjson::Value& atlas = doc["atlas"];

		if (!atlas.IsArray())
			return kErrUnknown;

		for (rapidjson::SizeType i = 0; i < atlas.Size(); i++)
		{
			const rapidjson::Value& uvs = atlas[i]["uvs"];

			rects[i].x = uvs["x"].GetFloat();
			rects[i].y = uvs["y"].GetFloat();
			rects[i].z = uvs["z"].GetFloat();
			rects[i].w = uvs["w"].GetFloat();
		}

		numTextures = atlas.Size();

		return kOK;
	}

}
