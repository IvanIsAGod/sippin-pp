#include "draw.h"

namespace F
{
	unsigned int esp;
	unsigned int esp2;
	unsigned int eventlog;
	unsigned int indicators;
}

namespace draw
{

	void Line(int x, int y, int x2, int y2, Color color)
	{
		g_pSurface->SetDrawColor(color);
		g_pSurface->DrawLine(x, y, x2, y2);
	}
	void DrawFilled3DBox(Vector origin, int width, int height, Color outline, Color filling)
	{
		float difw = float(width / 2);
		float difh = float(height / 2);
		Vector boxVectors[8] =
		{
			Vector(origin.x - difw, origin.y - difh, origin.z - difw),
			Vector(origin.x - difw, origin.y - difh, origin.z + difw),
			Vector(origin.x + difw, origin.y - difh, origin.z + difw),
			Vector(origin.x + difw, origin.y - difh, origin.z - difw),
			Vector(origin.x - difw, origin.y + difh, origin.z - difw),
			Vector(origin.x - difw, origin.y + difh, origin.z + difw),
			Vector(origin.x + difw, origin.y + difh, origin.z + difw),
			Vector(origin.x + difw, origin.y + difh, origin.z - difw),
		};

		static Vector vec0, vec1, vec2, vec3,
			vec4, vec5, vec6, vec7;

		if (GameUtils::WorldToScreen(boxVectors[0], vec0) &&
			GameUtils::WorldToScreen(boxVectors[1], vec1) &&
			GameUtils::WorldToScreen(boxVectors[2], vec2) &&
			GameUtils::WorldToScreen(boxVectors[3], vec3) &&
			GameUtils::WorldToScreen(boxVectors[4], vec4) &&
			GameUtils::WorldToScreen(boxVectors[5], vec5) &&
			GameUtils::WorldToScreen(boxVectors[6], vec6) &&
			GameUtils::WorldToScreen(boxVectors[7], vec7))
		{
			Vector2D lines[12][2];
			lines[0][0] = vec0;
			lines[0][1] = vec1;
			lines[1][0] = vec1;
			lines[1][1] = vec2;
			lines[2][0] = vec2;
			lines[2][1] = vec3;
			lines[3][0] = vec3;
			lines[3][1] = vec0;

			// top of box
			lines[4][0] = vec4;
			lines[4][1] = vec5;
			lines[5][0] = vec5;
			lines[5][1] = vec6;
			lines[6][0] = vec6;
			lines[6][1] = vec7;
			lines[7][0] = vec7;
			lines[7][1] = vec4;

			lines[8][0] = vec0;
			lines[8][1] = vec4;

			lines[9][0] = vec1;
			lines[9][1] = vec5;

			lines[10][0] = vec2;
			lines[10][1] = vec6;

			lines[11][0] = vec3;
			lines[11][1] = vec7;

			for (int i = 0; i < 12; i++)
				DrawLine(lines[i][0].x, lines[i][0].y, lines[i][1].x, lines[i][1].y, outline);
		}
	}

	void PolyLine(int *x, int *y, int count, Color color)
	{
		g_pSurface->SetDrawColor(color);

		g_pSurface->DrawPolyLine(x, y, count);
	}
	void Polygon(int count, Vertex_t* Vertexs, Color color)
	{
		static int Texture = g_pSurface->CreateNewTextureID(true); //need to make a texture with procedural true
		unsigned char buffer[4] = { 255, 255, 255, 255 };//{ color.r(), color.g(), color.b(), color.a() };

		g_pSurface->DrawSetTextureRGBA(Texture, buffer, 1, 1); //Texture, char array of texture, width, height
		g_pSurface->SetDrawColor(color); // keep this full color and opacity use the RGBA @top to set values.
		g_pSurface->DrawSetTexture(Texture); // bind texture

		g_pSurface->DrawTexturedPolygon(count, Vertexs);
	}
	void PolygonOutline(int count, Vertex_t* Vertexs, Color color, Color colorLine)
	{
		static int x[128];
		static int y[128];
		draw::Polygon(count, Vertexs, color);

		for (int i = 0; i < count; i++)
		{
			x[i] = Vertexs[i].m_Position.x;
			y[i] = Vertexs[i].m_Position.y;
		}

		draw::PolyLine(x, y, count, colorLine);
	}

	void DrawStringWithFont(LPD3DXFONT fnt, float x, float y, D3DCOLOR color, char *format, ...)
	{
		char buffer[256];
		RECT fontRect = { (int)x, (int)y, (int)x, (int)y };

		va_list va_argList;

		va_start(va_argList, format);
		wvsprintf(buffer, format, va_argList);
		va_end(va_argList);

		fnt->DrawText(NULL, buffer, strlen(buffer), &fontRect, DT_NOCLIP, color);
	}

	void Textf(int x, int y, Color color, DWORD font, const char* fmt, ...)
	{

		if (!fmt) return; //if the passed string is null return
		if (strlen(fmt) < 2) return;

		//Set up va_list and buffer to hold the params 
		va_list va_alist;
		char logBuf[256] = { 0 };

		//Do sprintf with the parameters
		va_start(va_alist, fmt);
		_vsnprintf_s(logBuf + strlen(logBuf), 256 - strlen(logBuf), sizeof(logBuf) - strlen(logBuf), fmt, va_alist);
		va_end(va_alist);

		Text(x, y, color, font, logBuf);
	}

	void Text(int x, int y, Color color, DWORD font, const char* text)
	{
		size_t origsize = strlen(text) + 1;
		const size_t newsize = 999;
		size_t convertedChars = 0;
		wchar_t wcstring[newsize];
		mbstowcs_s(&convertedChars, wcstring, origsize, text, _TRUNCATE);

		g_pSurface->DrawSetTextFont(font);

		g_pSurface->DrawSetTextColor(color);
		g_pSurface->DrawSetTextPos(x, y);
		g_pSurface->DrawPrintText(wcstring, wcslen(wcstring));
		return;
	}

	void Text2(int x, int y, const char* _Input, int font, Color color)
	{
		int apple = 0;
		char Buffer[2048] = { '\0' };
		va_list Args;
		va_start(Args, _Input);
		vsprintf_s(Buffer, _Input, Args);
		va_end(Args);
		size_t Size = strlen(Buffer) + 1;
		wchar_t* WideBuffer = new wchar_t[Size];
		mbstowcs_s(0, WideBuffer, Size, Buffer, Size - 1);

		g_pSurface->DrawSetTextColor(color);
		g_pSurface->DrawSetTextFont(font);
		g_pSurface->DrawSetTextPos(x, y);
		g_pSurface->DrawPrintText(WideBuffer, wcslen(WideBuffer));
	}

	unsigned int CreateF(std::string font_name, int size, int weight, int blur, int scanlines, int flags)
	{
		auto font = g_pSurface->SCreateFont();
		g_pSurface->SetFontGlyphSet(font, font_name.c_str(), size, weight, blur, scanlines, flags);

		return font;
	}

	void DrawPixel(int x, int y, Color col)
	{
		g_pSurface->SetDrawColor(col);
		g_pSurface->DrawFilledRect(x, y, x + 1, y + 1);
	}
	void DrawF(int X, int Y, unsigned int Font, bool center_width, bool center_height, Color Color, std::string Input, ...)
	{
		/* char -> wchar */
		size_t size = Input.size() + 1;
		auto wide_buffer = std::make_unique<wchar_t[]>(size);
		mbstowcs_s(0, wide_buffer.get(), size, Input.c_str(), size - 1);

		/* check center */
		int width = 0, height = 0;
		g_pSurface->GetTextSize(Font, wide_buffer.get(), width, height);
		if (!center_width)
			width = 0;
		if (!center_height)
			height = 0;

		/* call and draw*/
		g_pSurface->DrawSetTextColor(Color);
		g_pSurface->DrawSetTextFont(Font);
		g_pSurface->DrawSetTextPos(X - (width * .5), Y - (height * .5));
		g_pSurface->DrawPrintText(wide_buffer.get(), wcslen(wide_buffer.get()), 0);
	}
	void DrawWF(int X, int Y, unsigned int Font, Color Color, const wchar_t* Input) //std::string for the nn's
	{
		/* call and draw*/
		g_pSurface->DrawSetTextColor(Color);
		g_pSurface->DrawSetTextFont(Font);
		g_pSurface->DrawSetTextPos(X, Y);
		g_pSurface->DrawPrintText(Input, wcslen(Input), 0);
	}
	Vector2D GetTextSize(unsigned int Font, std::string Input, ...)
	{
		/* char -> wchar */
		size_t size = Input.size() + 1;
		auto wide_buffer = std::make_unique<wchar_t[]>(size);
		mbstowcs_s(0, wide_buffer.get(), size, Input.c_str(), size - 1);

		int width, height;
		g_pSurface->GetTextSize(Font, wide_buffer.get(), width, height);

		return Vector2D(width, height);
	}
	void FillRectangle(int x1, int y2, int width, int height, Color color) {
		g_pSurface->DrawSetTextColor(color);
		g_pSurface->DrawFilledRect(x1, y2, x1 + width, y2 + height);
	}
	void DrawLine(int x1, int y1, int x2, int y2, Color color)
	{
		g_pSurface->SetDrawColor(color);
		g_pSurface->DrawLine(x1, y1, x2, y2);
	}
	void DrawGradientRectangle(Vector2D Position, Vector2D Size, Color Top, Color Bottom) {
		g_pSurface->SetDrawColor(Top);
		g_pSurface->SetDrawColor(Bottom);
	}

	void DrawEmptyRect(int x1, int y1, int x2, int y2, Color color, unsigned char ignore_flags)
	{
		g_pSurface->SetDrawColor(color);
		if (!(ignore_flags & 0b1))
			g_pSurface->DrawLine(x1, y1, x2, y1);
		if (!(ignore_flags & 0b10))
			g_pSurface->DrawLine(x2, y1, x2, y2);
		if (!(ignore_flags & 0b100))
			g_pSurface->DrawLine(x2, y2, x1, y2);
		if (!(ignore_flags & 0b1000))
			g_pSurface->DrawLine(x1, y2, x1, y1);
	}
	void DrawCornerRect(const int32_t x, const int32_t y, const int32_t w, const int32_t h, const bool outlined, const Color& color, const Color& outlined_color)
	{
		auto corner = [&](const int32_t _x, const int32_t _y, const int32_t width, const int32_t height, const bool right_side, const bool down, const Color& _color, const bool _outlined, const Color& _outlined_color) -> void
		{
			const auto corner_x = right_side ? _x - width : _x;
			const auto corner_y = down ? _y - height : _y;
			const auto corner_w = down && right_side ? width + 1 : width;

			DrawEmptyRect(corner_x, _y, corner_w, 1, _color);
			DrawEmptyRect(_x, corner_y, 1, height, _color);

			if (_outlined) {
				DrawEmptyRect(corner_x, down ? _y + 1 : _y - 1, !down && right_side ? corner_w + 1 : corner_w, 1, _outlined_color);
				DrawEmptyRect(right_side ? _x + 1 : _x - 1, down ? corner_y : corner_y - 1, 1, down ? height + 2 : height + 1, _outlined_color);
			}
		};

		corner(x - w, y, w / 2, w / 2, false, false, color, outlined, outlined_color);
		corner(x - w, y + h, w / 2, w / 2, false, true, color, outlined, outlined_color);
		corner(x + w, y, w / 2, w / 2, true, false, color, outlined, outlined_color);
		corner(x + w, y + h, w / 2, w / 2, true, true, color, outlined, outlined_color);
	}
	void DrawEdges(float topX, float topY, float bottomX, float bottomY, float length, Color color)
	{
		float scale = (bottomY - topY) / 5.0f;
		DrawLine(topX - scale, topX - scale + length, topY, topY, color); //  --- Top left
		DrawLine(topX - scale, topX - scale, topY, topY + length, color); // | Top left
		DrawLine(topX + scale, topX + scale + length, topY, topY, color); // --- Top right
		DrawLine(topX + scale + length, topX + scale + length, topY, topY + length, color); // | Top right
		DrawLine(bottomX - scale, topX - scale + length, bottomY, bottomY, color); // --- Bottom left
		DrawLine(bottomX - scale, topX - scale, bottomY, bottomY - length, color); // | Bottom left
		DrawLine(bottomX + scale, topX + scale + length, bottomY, bottomY, color); // --- Bottom right
		DrawLine(bottomX + scale + length, topX + scale + length, bottomY, bottomY - length, color); // | Bottom right
	}

	void DrawFilledRect(int x1, int y1, int x2, int y2, Color color)
	{
		g_pSurface->SetDrawColor(color);
		g_pSurface->DrawFilledRect(x1, y1, x2, y2);
	}

	void Clear(int x, int y, int w, int h, Color color)
	{
		g_pSurface->SetDrawColor(color);
		g_pSurface->DrawFilledRect(x, y, x + w, y + h);
	}
	void DrawFilledRectOutline(int x1, int y1, int x2, int y2, Color color)
	{
		g_pSurface->SetDrawColor(color);
		g_pSurface->DrawFilledRect(x1, y1, x2, y2);
		DrawEmptyRect(x1 - 1, y1 - 1, x2, y2, Color(0, 0, 0, 100));
	}

	void DrawCircle(int x, int y, int radius, int segments, Color color)
	{
		g_pSurface->SetDrawColor(color);
		g_pSurface->DrawOutlinedCircle(x, y, radius, segments);
	}

	void TexturedPolygon(int n, std::vector<Vertex_t> vertice, Color color)
	{
		static int texture_id = g_pSurface->CreateNewTextureID(true); // 
		static unsigned char buf[4] = { 255, 255, 255, 255 };
		g_pSurface->DrawSetTextureRGBA(texture_id, buf, 1, 1); //
		g_pSurface->SetDrawColor(color); //
		g_pSurface->DrawSetTexture(texture_id); //
		g_pSurface->DrawTexturedPolygon(n, vertice.data()); //
	}

	void DrawFilledCircle(int x, int y, int radius, int segments, Color color)
	{
		std::vector<Vertex_t> vertices;
		float step = M_PI * 2.0f / segments;
		for (float a = 0; a < (M_PI * 2.0f); a += step)
			vertices.push_back(Vertex_t(Vector2D(radius * cosf(a) + x, radius * sinf(a) + y)));

		TexturedPolygon(vertices.size(), vertices, color);
	}

	void DrawFilledCirclefloat(int x, int y, int radius, int segments, float color[4])
	{
		std::vector<Vertex_t> vertices;
		float step = M_PI * 2.0f / segments;
		for (float a = 0; a < (M_PI * 2.0f); a += step)
			vertices.push_back(Vertex_t(Vector2D(radius * cosf(a) + x, radius * sinf(a) + y)));

		Color newcolor = Color(color[0] * 255, color[1] * 255, color[2] * 255, color[3] * 255);
		TexturedPolygon(vertices.size(), vertices, newcolor);
	}


	int GetTextWitdh(char *text, LPD3DXFONT fnt)
	{
		RECT fontRect = { 0,0,0,0 };

		fnt->DrawText(NULL, text, strlen(text), &fontRect, DT_CALCRECT, D3DCOLOR_XRGB(0, 0, 0));

		return fontRect.right - fontRect.left;
	}

	void InitFonts()
	{
		F::esp = draw::CreateF("Verdana", 12, 400, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW);
		F::esp2 = draw::CreateF("Smallest Pixel-7", 10, 100, 0, 0, FONTFLAG_OUTLINE);
		F::eventlog = draw::CreateF("Tahoma", 13, 550, 0, 0, FONTFLAG_ANTIALIAS | FONTFLAG_OUTLINE);
		F::indicators = draw::CreateF("Verdana", 26, 600, 0, 0, FONTFLAG_OUTLINE);
	}
}