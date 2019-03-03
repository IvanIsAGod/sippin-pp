#pragma once
#define WHITE(alpha)         D3DCOLOR_ARGB(alpha, 255, 255, 255)
#define BLACK(alpha)         D3DCOLOR_ARGB(alpha, 0, 0, 0)
#define RED(alpha)           D3DCOLOR_ARGB(alpha, 255, 0, 0)
#define GREEN(alpha)         D3DCOLOR_ARGB(alpha, 0, 128, 0)
#define LAWNGREEN(alpha)     D3DCOLOR_ARGB(alpha, 124, 252, 0)
#define BLUE(alpha)          D3DCOLOR_ARGB(alpha, 0, 200, 255)
#define DEEPSKYBLUE(alpha)   D3DCOLOR_ARGB(alpha, 0, 191, 255)
#define SKYBLUE(alpha)       D3DCOLOR_ARGB(alpha, 0, 122, 204)
#define YELLOW(alpha)        D3DCOLOR_ARGB(alpha, 255, 255, 0)
#define ORANGE(alpha)        D3DCOLOR_ARGB(alpha, 255, 165, 0)
#define DARKORANGE(alpha)    D3DCOLOR_ARGB(alpha, 255, 140, 0)
#define PURPLE(alpha)        D3DCOLOR_ARGB(alpha, 125, 0, 255)
#define CYAN(alpha)          D3DCOLOR_ARGB(alpha, 0, 255, 255)
#define PINK(alpha)          D3DCOLOR_ARGB(alpha, 255, 20, 147)
#define GRAY(alpha)          D3DCOLOR_ARGB(alpha, 128, 128, 128)
#define DARKGRAY(alpha)      D3DCOLOR_ARGB(alpha, 73, 73, 73)
#define DARKERGRAY(alpha)    D3DCOLOR_ARGB(alpha, 31, 31, 31)

#define getA(col) (((col)&0xff000000)>>24)
#define getR(col) (((col)&0x00ff0000)>>16)
#define getG(col) (((col)&0x0000ff00)>>8)
#define getB(col) ((col)&0x000000ff)
enum circle_type {
	full = 1, half, quarter
};
enum text_alignment { lefted, centered, righted };

struct CUSTOMVERTEX {
	FLOAT X, Y, Z; DWORD COLOR;
};
#define CUSTOMFVF (D3DFVF_XYZ | D3DFVF_DIFFUSE)

struct vertex
{
	FLOAT x, y, z, rhw;
	DWORD color;
};

#define SAFE_RELEASE(p) { if (p) { (p)->Release(); (p)=NULL; } }

class CRender
{
public:
	CRender();
	~CRender();
	struct sScreen
	{
		float Width;
		float Height;
		float x_center;
		float y_center;
	} Screen;


	LPDIRECT3DDEVICE9 device;
	LPDIRECT3DVERTEXBUFFER9 vb;    // Buffer to hold vertices
	LPDIRECT3DINDEXBUFFER9  ib;    // Buffer to hold indices

	int Menu_control_size = 12;

	LPD3DXLINE line;
	LPD3DXSPRITE sprite;
	LPDIRECT3DTEXTURE9 white_tex;
	struct sFonts
	{
		LPD3DXFONT esp;
		LPD3DXFONT event;
		LPD3DXFONT lby;
		LPD3DXFONT esp2;
	}Fonts;

	void Init(IDirect3DDevice9* pDevice);
	void LostDevice();
	void ResetDevice();
	void Reset();

	int StringWidth(ID3DXFont* font, char* string);
	void CircleFilledDualColor(float x, float y, float rad, float rotate, int type, int resolution, DWORD color, DWORD color2, IDirect3DDevice9 * m_device);
	/*inline*/ RECT GetTextSize2(LPD3DXFONT fnt, char* text);
	int GetTextWitdhW(wchar_t * text, LPD3DXFONT fnt);
	void Line(int x, int y, int x2, int y2, D3DCOLOR color);
	void FilledBox(int x, int y, int width, int height, D3DCOLOR color);
	void FilledBoxOutlined(int x, int y, int width, int height, D3DCOLOR colour, D3DCOLOR outlinecolor, int thickness = 1.0f);
	void BorderedBox(int x, int y, int width, int height, D3DCOLOR color, int thickness = 1.0f);
	void BorderedBoxOutlined(int x, int y, int width, int height, D3DCOLOR colour, D3DCOLOR outlinecolor, int thickness = 1.0f);
	void GradientBox(int x, int y, int width, int height, D3DCOLOR color, D3DCOLOR color2, bool vertical);
	void GradientBoxOutlined(int x, int y, int width, int height, int thickness, D3DCOLOR colour, D3DCOLOR color2, D3DCOLOR outlinecolor, bool vertical);
	void Circle(int x, int y, int radius, int points, D3DCOLOR color);
	void FilledCircle(int x, int y, int radius, int points, D3DCOLOR color);

	void Text(char *text, float x, float y, int orientation, LPD3DXFONT pFont, bool bordered, DWORD color, DWORD bcolor);
	void String(float x, float y, int orientation, LPD3DXFONT pFont, bool bordered, DWORD color, int a, const char *input, ...);
	void Message(char *text, float x, float y, LPD3DXFONT pFont, int orientation);

	void Sprite(LPDIRECT3DTEXTURE9 tex, float x, float y, float resolution_x, float resolution_y, float scale_x, float scale_y, float rotation, DWORD color);

	int FrameRate();
private:
	struct CUSTOMVERTEX
	{
		float x, y, z, rhw;
		DWORD color;
	};
#define D3DX_PI2 3.1415926535897932384626
}; extern CRender* g_pRender;