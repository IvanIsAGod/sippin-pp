#include "ImGUI\imconfig.h"
#include "ImGUI\imgui.h"
#include "ImGUI\imgui_internal.h"
#include "ImGUI\stb_rect_pack.h"
#include "ImGUI\stb_textedit.h"
#include "ImGUI\stb_truetype.h"
#include "ImGUI\dx9\imgui_dx9.h"

struct Vertex_t
{
	Vector2D	m_Position;
	Vector2D	m_TexCoord;

	Vertex_t() {}
	Vertex_t(const Vector2D &pos, const Vector2D &coord = Vector2D(0, 0))
	{
		m_Position = pos;
		m_TexCoord = coord;
	}
	void Init(const Vector2D &pos, const Vector2D &coord = Vector2D(0, 0))
	{
		m_Position = pos;
		m_TexCoord = coord;
	}
};

typedef  Vertex_t FontVertex_t;
class Color
{
public:
	Color()
	{
		*((int *)this) = 0;
	}
	Color(int color32)
	{
		*((int *)this) = color32;
	}
	Color(int _r, int _g, int _b)
	{
		SetColor(_r, _g, _b, 255);
	}
	Color(int _r, int _g, int _b, int _a)
	{
		SetColor(_r, _g, _b, _a);
	}
	Color(float _rgb[3])
	{
		SetColor((int)(_rgb[0] * 255.0f), (int)(_rgb[1] * 255.0f), (int)(_rgb[2] * 255.0f), 255);
	}

	Color(float _rgb[3], float _alpha)
	{
		SetColor((int)(_rgb[0] * 255.0f), (int)(_rgb[1] * 255.0f), (int)(_rgb[2] * 255.0f), _alpha);
	}
	void SetColor(int _r, int _g, int _b, int _a = 255)
	{
		_color[0] = (unsigned char)_r;
		_color[1] = (unsigned char)_g;
		_color[2] = (unsigned char)_b;
		_color[3] = (unsigned char)_a;
	}

	ImU32 GetU32()
	{
		return ((_color[3] & 0xff) << 24) + ((_color[2] & 0xff) << 16) + ((_color[1] & 0xff) << 8)
			+ (_color[0] & 0xff);
		//return (ImU32)(((_color[3] & 0xff) << 24) + ((_color[0] & 0xff) << 16) + ((_color[1] & 0xff) << 8) + (_color[2] & 0xff));
	}

	void GetColor(int &_r, int &_g, int &_b, int &_a) const
	{
		_r = _color[0];
		_g = _color[1];
		_b = _color[2];
		_a = _color[3];
	}
	void SetAlpha(int a)
	{
		_color[3] = (unsigned char)a;
	}
	void SetRawColor(int color32)
	{
		*((int*)this) = color32;
	}

	int GetRawColor() const
	{
		return *((int*)this);
	}

	int GetD3DColor() const
	{
		return ((int)((((_color[3]) & 0xff) << 24) | (((_color[0]) & 0xff) << 16) | (((_color[1]) & 0xff) << 8) | ((_color[2]) & 0xff)));
	}

	inline int r() const	{ return _color[0]; }
	inline int g() const	{ return _color[1]; }
	inline int b() const	{ return _color[2]; }
	inline int a() const	{ return _color[3]; }

	unsigned char &operator[](int index)
	{
		return _color[index];
	}

	const unsigned char &operator[](int index) const
	{
		return _color[index];
	}

	bool operator == (const Color &rhs) const
	{
		return (*((int *)this) == *((int *)&rhs));
	}

	bool operator != (const Color &rhs) const
	{
		return !(operator==(rhs));
	}

	Color &operator=(const Color &rhs)
	{
		SetRawColor(rhs.GetRawColor());
		return *this;
	}

	float* Base()
	{
		float clr[3];

		clr[0] = _color[0] / 255.0f;
		clr[1] = _color[1] / 255.0f;
		clr[2] = _color[2] / 255.0f;

		return &clr[0];
	}

	float* BaseAlpha()
	{
		float clr[4];

		clr[0] = _color[0] / 255.0f;
		clr[1] = _color[1] / 255.0f;
		clr[2] = _color[2] / 255.0f;
		clr[3] = _color[3] / 255.0f;

		return &clr[0];
	}

	float Hue() const
	{
		if (_color[0] == _color[1] && _color[1] == _color[2])
		{
			return 0.0f;
		}

		float r = _color[0] / 255.0f;
		float g = _color[1] / 255.0f;
		float b = _color[2] / 255.0f;

		float max = r > g ? r : g > b ? g : b,
			min = r < g ? r : g < b ? g : b;
		float delta = max - min;
		float hue = 0.0f;

		if (r == max)
		{
			hue = (g - b) / delta;
		}
		else if (g == max)
		{
			hue = 2 + (b - r) / delta;
		}
		else if (b == max)
		{
			hue = 4 + (r - g) / delta;
		}
		hue *= 60;

		if (hue < 0.0f)
		{
			hue += 360.0f;
		}
		return hue;
	}

	float Saturation() const
	{
		float r = _color[0] / 255.0f;
		float g = _color[1] / 255.0f;
		float b = _color[2] / 255.0f;

		float max = r > g ? r : g > b ? g : b,
			min = r < g ? r : g < b ? g : b;
		float l, s = 0;

		if (max != min)
		{
			l = (max + min) / 2;
			if (l <= 0.5f)
				s = (max - min) / (max + min);
			else
				s = (max - min) / (2 - max - min);
		}
		return s;
	}

	float Brightness() const
	{
		float r = _color[0] / 255.0f;
		float g = _color[1] / 255.0f;
		float b = _color[2] / 255.0f;

		float max = r > g ? r : g > b ? g : b,
			min = r < g ? r : g < b ? g : b;
		return (max + min) / 2;
	}

	static Color FromHSB(float hue, float saturation, float brightness)
	{
		float h = hue == 1.0f ? 0 : hue * 6.0f;
		float f = h - (int)h;
		float p = brightness * (1.0f - saturation);
		float q = brightness * (1.0f - saturation * f);
		float t = brightness * (1.0f - (saturation * (1.0f - f)));

		if (h < 1)
		{
			return Color(
				(unsigned char)(brightness * 255),
				(unsigned char)(t * 255),
				(unsigned char)(p * 255)
				);
		}
		else if (h < 2)
		{
			return Color(
				(unsigned char)(q * 255),
				(unsigned char)(brightness * 255),
				(unsigned char)(p * 255)
				);
		}
		else if (h < 3)
		{
			return Color(
				(unsigned char)(p * 255),
				(unsigned char)(brightness * 255),
				(unsigned char)(t * 255)
				);
		}
		else if (h < 4)
		{
			return Color(
				(unsigned char)(p * 255),
				(unsigned char)(q * 255),
				(unsigned char)(brightness * 255)
				);
		}
		else if (h < 5)
		{
			return Color(
				(unsigned char)(t * 255),
				(unsigned char)(p * 255),
				(unsigned char)(brightness * 255)
				);
		}
		else
		{
			return Color(
				(unsigned char)(brightness * 255),
				(unsigned char)(p * 255),
				(unsigned char)(q * 255)
				);
		}
	}

	static Color Red()		{ return Color(255, 0, 0); }
	static Color Green()	{ return Color(0, 255, 0); }
	static Color Blue()		{ return Color(0, 0, 255); }
	static Color LightBlue(){ return Color(100, 100, 255); }
	static Color Grey()		{ return Color(128, 128, 128); }
	static Color DarkGrey()	{ return Color(45, 45, 45); }
	static Color Black()	{ return Color(0, 0, 0); }
	static Color White()	{ return Color(255, 255, 255); }
	static Color Purple()	{ return Color(220, 0, 220); }

private:
	unsigned char _color[4];
};
class CSurface
{
public:

	void SetDrawColor(int r, int g, int b, int a)
	{
		typedef void(__thiscall* Fn)(void*, int, int, int, int);
		return CallVFunction<Fn>(this, 15)(this, r, g, b, a);
	}
	void SetDrawColor(Color color)
	{
		//typedef void(__thiscall* Fn)(PVOID, Color); // int, int , int, int
		//return CallVFunction<Fn>(this, 14)(this, color); // r, g, b, a
		SetDrawColor(color.r(), color.g(), color.b(), color.a());
	}
	void DrawFilledRect( int x, int y, int x2, int y2 )
	{
		typedef void(__thiscall* Fn)(void*, int, int, int, int);
		return CallVFunction<Fn>(this, 16)(this, x, y, x2, y2);
	}

	void DrawOutlinedRect( int x, int y, int x2, int y2 )
	{
		typedef void(__thiscall* Fn)(void*, int, int, int, int);
		return CallVFunction<Fn>(this, 18)(this, x, y, x2, y2);
	}

	void DrawLine( int x, int y, int x2, int y2 )
	{
		typedef void(__thiscall* Fn)(void*, int, int, int, int);
		return CallVFunction<Fn>(this, 19)(this, x, y, x2, y2);
	}

	unsigned int SCreateFont()
	{
		typedef unsigned int(__thiscall* Fn)(void*);
		return CallVFunction<Fn>(this, 71)(this);
	}
	void DrawPolyLine(int *px, int *py, int numPoints)
	{
		typedef void(__thiscall* Fn)(void*, int*, int*, int);
		return CallVFunction<Fn>(this, 20)(this, px, py, numPoints);
	}
	bool SetFontGlyphSet(unsigned int font, const char *windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int nRangeMin = 0, int nRangeMax = 0)
	{
		typedef bool(__thiscall* Fn)(void*, unsigned int, const char*, int, int, int, int, int, int, int);
		return CallVFunction<Fn>(this, 72)(this, font, windowsFontName, tall, weight, blur, scanlines, flags, nRangeMin, nRangeMax);
	}
	void DrawSetTextFont(unsigned int Font)
	{
		typedef void(__thiscall* Fn)(void*, unsigned int);
		return CallVFunction<Fn>(this, 23)(this, Font);
	}

	void DrawSetTextColor(int r, int g, int b, int a)
	{
		typedef void(__thiscall* Fn)(void*, int, int, int, int);
		return CallVFunction<Fn>(this, 25)(this, r, g, b, a);
	}

	void DrawSetTextColor(Color col)
	{
		typedef void(__thiscall* oDrawSetTextColor)(PVOID, Color);
		return CallVFunction< oDrawSetTextColor >(this, 24)(this, col);
	}


	void DrawSetTextPos( int x, int y )
	{
		typedef void(__thiscall* Fn)(void*, int, int);
		return CallVFunction<Fn>(this, 26)(this, x, y);
	}

	void DrawPrintText(const wchar_t* Text, int Len, int DrawType = 0)
	{
		typedef void(__thiscall* Fn)(void*, wchar_t const*, int, int);
		return CallVFunction<Fn>(this, 28)(this, Text, Len, DrawType);
	}
	void       DrawUnicodeChar(wchar_t wch, int DrawType = 0)
	{
		typedef void(__thiscall* Fn)(void*, wchar_t, int);
		return CallVFunction<Fn>(this, 29)(this, wch, DrawType);
	}
	void GetTextSize(unsigned long font, const wchar_t *text, int &wide, int &tall)
	{
		typedef void(__thiscall* Fn)(void*, unsigned long font, const wchar_t *text, int &wide, int &tall);
		return CallVFunction<Fn>(this, 79)(this, font, text, wide, tall);
	}

	void DrawColoredCircle(int centerx, int centery, float radius, int r, int g, int b, int a)
	{
		typedef void(__thiscall* Fn)(void*, int, int, float, int, int, int, int);
		return CallVFunction<Fn>(this, 162)(this, centerx, centery, radius, r, g, b, a);
	}
	void unlockcursor()
	{
		typedef void(__thiscall* OriginalFn)(void*);
		CallVFunction< OriginalFn >(this, 66)(this);
	}
	void lockcursor()
	{
		typedef void(__thiscall* OriginalFn)(void*);
		CallVFunction< OriginalFn >(this, 67)(this);
	}

	void DrawOutlinedCircle(int x, int y, int radius, int segments)
	{
		typedef void(__thiscall* Fn)(void*, int, int, int, int);
		return CallVFunction<Fn>(this, 103)(this, x, y, radius, segments);
	}
	void DrawSetTextureRGBA(int id, const unsigned char *rgba, int wide, int tall)
	{
		typedef void(__thiscall* Fn)(void*, int, const unsigned char*, int, int);
		return  CallVFunction<Fn>(this, 37)(this, id, rgba, wide, tall);
	}
	bool IsTextureIDValid(int TextureID)
	{
		typedef bool(__thiscall* Fn)(void*, int);
		return CallVFunction<Fn>(this, 42)(this, TextureID);
	}
	void DrawTexturedRect(int x0, int y0, int W, int T)
	{
		typedef void(__thiscall* Fn)(void*, int, int, int, int);
		return CallVFunction<Fn>(this, 41)(this, x0, y0, W, T);
	}


	void DrawTexturedPolygon(int vtxCount, FontVertex_t *vtx, bool bClipVertices = true)
	{
		typedef void(__thiscall* oDrawSetTextColor)(PVOID, int, FontVertex_t*, bool);
		CallVFunction<oDrawSetTextColor>(this, 106)(this, vtxCount, vtx, bClipVertices);
	}
	void DrawSetTexture(int id)
	{
		typedef void(__thiscall* Fn)(void*, int);
		return CallVFunction<Fn>(this, 38)(this, id);
	}
	int CreateNewTextureID(bool procedural)
	{
		typedef int(__thiscall* Fn)(void*, bool);
		return CallVFunction<Fn>(this, 43)(this, procedural);
	}
};
enum EFontFlags
{
	FONTFLAG_NONE,
	FONTFLAG_ITALIC = 0x001,
	FONTFLAG_UNDERLINE = 0x002,
	FONTFLAG_STRIKEOUT = 0x004,
	FONTFLAG_SYMBOL = 0x008,
	FONTFLAG_ANTIALIAS = 0x010,
	FONTFLAG_GAUSSIANBLUR = 0x020,
	FONTFLAG_ROTARY = 0x040,
	FONTFLAG_DROPSHADOW = 0x080,
	FONTFLAG_ADDITIVE = 0x100,
	FONTFLAG_OUTLINE = 0x200,
	FONTFLAG_CUSTOM = 0x400,
	FONTFLAG_BITMAP = 0x800,
};