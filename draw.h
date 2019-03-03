#pragma once
#include "includes.h"

namespace F
{
	extern unsigned int esp;
	extern unsigned int esp2;
	extern unsigned int eventlog;
	extern unsigned int indicators;
}

namespace draw
{
	void InitFonts();
	void PolygonOutline(int count, Vertex_t* Vertexs, Color color, Color colorLine);
	void Polygon(int count, Vertex_t* Vertexs, Color color);
	void PolyLine(int *x, int *y, int count, Color color);
	void Clear(int x, int y, int w, int h, Color color);
	void DrawFilled3DBox(Vector origin, int width, int height, Color outline, Color filling);
	void Text(int x, int y, Color color, DWORD font, const char* text);
	void Text2(int x, int y, const char * _Input, int font, Color color);
	void Textf(int x, int y, Color color, DWORD font, const char* fmt, ...);
	void Line(int x, int y, int x2, int y2, Color color);
	void DrawStringWithFont(LPD3DXFONT fnt, float x, float y, D3DCOLOR color, char *format, ...);
	unsigned int CreateF(std::string font_name, int size, int weight, int blur, int scanlines, int flags);
	int GetTextWitdh(char *text, LPD3DXFONT fnt);
	void DrawF(int X, int Y, unsigned int Font, bool center_width, bool center_height, Color Color, std::string Input, ...);
	void DrawWF(int X, int Y, unsigned int Font, Color Color, const wchar_t* Input);
	Vector2D GetTextSize(unsigned int Font, std::string Input, ...);
	void DrawPixel(int x, int y, Color col);
	void DrawLine(int x1, int y1, int x2, int y2, Color color);
	void DrawGradientRectangle(Vector2D Position, Vector2D Size, Color Top, Color Bottom);
	void DrawEmptyRect(int x1, int y1, int x2, int y2, Color color, unsigned char = 0); // the flags are for which sides to ignore in clockwise, 0b1 is top, 0b10 is right, etc.
	void DrawFilledRect(int x1, int y1, int x2, int y2, Color color);
	void FillRectangle(int x1, int y2, int width, int height, Color color);
	void DrawFilledRectOutline(int x1, int y1, int x2, int y2, Color color);
	void DrawCornerRect(const int32_t x, const int32_t y, const int32_t w, const int32_t h, const bool outlined, const Color& color, const Color& outlined_color);
	void DrawEdges(float topX, float topY, float bottomX, float bottomY, float length, Color color);
	void DrawFilledCirclefloat(int x, int y, int radius, int segments, float color[4]);
	void DrawCircle(int x, int y, int radius, int segments, Color color);
	void DrawFilledCircle(int x, int y, int radius, int segments, Color color);
	//void DrawFilledCircle(Vector2D center, Color color, Color outline, float radius, float points);

	void TexturedPolygon(int n, std::vector<Vertex_t> vertice, Color color);
}