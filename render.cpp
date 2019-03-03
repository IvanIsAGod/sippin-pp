#include "sdk.h"
#include "render.h"
CRender* g_pRender = new CRender;

void CRender::Init(IDirect3DDevice9* pDevice)
{
	auto GenerateTexture = [](IDirect3DDevice9 *pD3Ddev, IDirect3DTexture9 **ppD3Dtex, DWORD colour32)
	{
		if (FAILED(pD3Ddev->CreateTexture(8, 8, 1, 0, D3DFMT_A4R4G4B4, D3DPOOL_MANAGED, ppD3Dtex, NULL)))
			return E_FAIL;

		WORD colour16 = ((WORD)((colour32 >> 28) & 0xF) << 12)
			| (WORD)(((colour32 >> 20) & 0xF) << 8)
			| (WORD)(((colour32 >> 12) & 0xF) << 4)
			| (WORD)(((colour32 >> 4) & 0xF) << 0);

		D3DLOCKED_RECT d3dlr;
		(*ppD3Dtex)->LockRect(0, &d3dlr, 0, 0);
		WORD *pDst16 = (WORD*)d3dlr.pBits;

		for (int xy = 0; xy < 8 * 8; xy++)
			*pDst16++ = colour16;

		(*ppD3Dtex)->UnlockRect(0);

		return S_OK;
	};

	this->device = pDevice;

	D3DXCreateLine(this->device, &this->line);
	D3DXCreateSprite(this->device, &this->sprite);

	GenerateTexture(pDevice, &this->white_tex, D3DCOLOR_ARGB(255, 255, 255, 255));
	D3DXCreateFont(this->device, 12, 0, 700, 1, 0, RUSSIAN_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Arial", &this->Fonts.esp);
	D3DXCreateFont(this->device, 11, 0, 150, 1, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH, "Tahoma", &this->Fonts.esp2);
	D3DXCreateFont(this->device, 13, 0, 600, 1, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Tahoma", &this->Fonts.event);
	D3DXCreateFont(this->device, 28, 0, 650, 1, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH, "Verdana", &this->Fonts.lby);
	return;
}

CRender::CRender()
{
	device = nullptr;
	line = nullptr;

	line = NULL;
	Fonts.esp = NULL;
	Fonts.esp2 = NULL;
	Fonts.event = NULL;
	Fonts.lby = NULL;
}

CRender::~CRender()
{
	SAFE_RELEASE(line);
	SAFE_RELEASE(Fonts.esp);
	SAFE_RELEASE(Fonts.esp2);
	SAFE_RELEASE(Fonts.event);
	SAFE_RELEASE(Fonts.lby);
}

void CRender::LostDevice()
{
	if (Fonts.esp)
		Fonts.esp->OnLostDevice();

	if (Fonts.esp2)
		Fonts.esp2->OnLostDevice();

	if (Fonts.event)
		Fonts.event->OnLostDevice();

	if (Fonts.lby)
		Fonts.lby->OnLostDevice();

}
void CRender::ResetDevice()
{
	if (Fonts.esp)
		Fonts.esp->OnResetDevice();
	if (Fonts.esp2)
		Fonts.esp2->OnResetDevice();
	if (Fonts.event)
		Fonts.event->OnResetDevice();
	if (Fonts.lby)
		Fonts.lby->OnResetDevice();
}
void CRender::Reset()
{
	D3DVIEWPORT9 screen;
	this->device->GetViewport(&screen);

	Screen.Width = screen.Width;
	Screen.Height = screen.Height;
	Screen.x_center = Screen.Width / 2;
	Screen.y_center = Screen.Height / 2;
}


int CRender::StringWidth(ID3DXFont* font, char* string) {
	RECT pRect = RECT();
	font->DrawText(NULL, string, strlen(string), &pRect, DT_CALCRECT, D3DCOLOR_RGBA(0, 0, 0, 0));
	return pRect.right - pRect.left;
}
static LPDIRECT3DVERTEXBUFFER9 g_pVB2;
void CRender::CircleFilledDualColor(float x, float y, float rad, float rotate, int type, int resolution, DWORD color, DWORD color2, IDirect3DDevice9* m_device)
{

	std::vector<CUSTOMVERTEX> circle(resolution + 2);

	float angle = rotate * D3DX_PI2 / 180, pi = D3DX_PI2;

	if (type == full)
		pi = D3DX_PI2; // Full circle 
	if (type == half)
		pi = D3DX_PI2 / 2; // 1/2 circle 
	if (type == quarter)
		pi = D3DX_PI2 / 4; // 1/4 circle 

	pi = D3DX_PI2 / type; // 1/4 circle 

	circle[0].x = x;
	circle[0].y = y;
	circle[0].z = 0;
	circle[0].rhw = 1;
	//circle[0].color = color2; 

	for (int i = 1; i < resolution + 2; i++)
	{
		circle[i].x = (float)(x - rad * cos(pi*((i - 1) / (resolution / 2.0f))));
		circle[i].y = (float)(y - rad * sin(pi*((i - 1) / (resolution / 2.0f))));
		circle[i].z = 0;
		circle[i].rhw = 1;
		circle[i].color = color;
	}

	// Rotate matrix 
	int _res = resolution + 2;
	for (int i = 0; i < _res; i++)
	{
		circle[i].x = x + cos(angle)*(circle[i].x - x) - sin(angle)*(circle[i].y - y);
		circle[i].y = y + sin(angle)*(circle[i].x - x) + cos(angle)*(circle[i].y - y);
	}

	m_device->CreateVertexBuffer((resolution + 2) * sizeof(CUSTOMVERTEX), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB2, NULL);

	VOID* pVertices;
	g_pVB2->Lock(0, (resolution + 2) * sizeof(CUSTOMVERTEX), (void**)&pVertices, 0);
	memcpy(pVertices, &circle[0], (resolution + 2) * sizeof(CUSTOMVERTEX));
	g_pVB2->Unlock();

	m_device->SetTexture(0, NULL);
	m_device->SetPixelShader(NULL);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	m_device->SetStreamSource(0, g_pVB2, 0, sizeof(CUSTOMVERTEX));
	m_device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	m_device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, resolution);
	if (g_pVB2 != NULL)
		g_pVB2->Release();
}


/*inline*/ RECT CRender::GetTextSize2(LPD3DXFONT fnt, char * text)
{
	char Buffer[1024] = { '\0' };

	/* set up varargs*/
	va_list Args;

	va_start(Args, text);
	vsprintf_s(Buffer, text, Args);
	va_end(Args);

	size_t Size = strlen(Buffer) + 1;
	wchar_t* WideBuffer = new wchar_t[Size];

	mbstowcs_s(nullptr, WideBuffer, Size, Buffer, Size - 1);

	RECT rect;
	static int x, y;
	fnt->DrawText(NULL, text, strlen(text), &rect, DT_CALCRECT, D3DCOLOR_XRGB(0, 0, 0));
	rect.left = x;
	rect.bottom = y;
	rect.right = x;
	return rect;

}



int CRender::GetTextWitdhW(wchar_t *text, LPD3DXFONT fnt)
{
	RECT fontRect = { 0,0,0,0 };

	fnt->DrawTextW(NULL, text, -1, &fontRect, DT_CALCRECT, D3DCOLOR_XRGB(0, 0, 0));

	return fontRect.right - fontRect.left;
}

void CRender::Line(int x, int y, int x2, int y2, D3DCOLOR color)
{
	vertex pVertex[2] = { { x, y, 0.0f, 1.0f, color },{ x2, y2, 0.0f, 1.0f, color } };

	this->device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	this->device->SetTexture(0, nullptr);
	this->device->SetRenderState(D3DRS_LIGHTING, FALSE);
	this->device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	this->device->SetRenderState(D3DRS_FOGENABLE, FALSE);
	this->device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	this->device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	this->device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	this->device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	this->device->SetRenderState(D3DRS_LIGHTING, FALSE);
	this->device->SetRenderState(D3DRS_STENCILENABLE, FALSE);

	this->device->DrawPrimitiveUP(D3DPT_LINELIST, 1, &pVertex, sizeof(vertex));
}

void CRender::FilledBox(int x, int y, int width, int height, D3DCOLOR color)
{
	vertex pVertex[4] = { { x, y + height, 0.0f, 1.0f, color },{ x, y, 0.0f, 1.0f, color },{ x + width, y + height, 0.0f, 1.0f, color },{ x + width, y, 0.0f, 1.0f, color } };
	this->device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	this->device->SetTexture(0, nullptr);
	this->device->SetRenderState(D3DRS_LIGHTING, FALSE);
	this->device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	this->device->SetRenderState(D3DRS_FOGENABLE, FALSE);
	this->device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	this->device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	this->device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	this->device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	this->device->SetRenderState(D3DRS_LIGHTING, FALSE);
	this->device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
	this->device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pVertex, sizeof(vertex));
}
void CRender::FilledBoxOutlined(int x, int y, int width, int height, D3DCOLOR color, D3DCOLOR outlinecolor, int thickness)
{

	this->FilledBox(x, y, width, height, color);
	this->BorderedBox(x, y, width, height, outlinecolor);
}
void CRender::BorderedBox(int x, int y, int width, int height, D3DCOLOR color, int thickness)
{
	this->FilledBox(x, y, width, thickness, color);
	this->FilledBox(x, y, thickness, height, color);
	this->FilledBox(x + width - thickness, y, thickness, height, color);
	this->FilledBox(x, y + height - thickness, width, thickness, color);

}
void CRender::BorderedBoxOutlined(int x, int y, int width, int height, D3DCOLOR color, D3DCOLOR outlinecolor, int thickness) {
	this->BorderedBox(x, y, width, height, outlinecolor, thickness);
	this->BorderedBox(x + thickness, y + thickness, width - (thickness * 2), height - (thickness * 2), color, thickness);
	this->BorderedBox(x + (thickness * 2), y + (thickness * 2), width - (thickness * 4), height - (thickness * 4), outlinecolor, thickness);
}
void CRender::GradientBox(int x, int y, int width, int height, D3DCOLOR color, D3DCOLOR color2, bool vertical) {
	vertex pVertex[4] = { { x, y, 0.0f, 1.0f, color },{ x + width, y, 0.0f, 1.0f, vertical ? color : color2 },{ x, y + height, 0.0f, 1.0f, vertical ? color2 : color },{ x + width, y + height, 0.0f, 1.0f, color2 } };
	this->device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	this->device->SetTexture(0, nullptr);
	this->device->SetRenderState(D3DRS_LIGHTING, FALSE);
	this->device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	this->device->SetRenderState(D3DRS_FOGENABLE, FALSE);
	this->device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	this->device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	this->device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	this->device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	this->device->SetRenderState(D3DRS_LIGHTING, FALSE);
	this->device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
	this->device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, pVertex, sizeof(vertex));
}
void CRender::GradientBoxOutlined(int x, int y, int width, int height, int thickness, D3DCOLOR color, D3DCOLOR color2, D3DCOLOR outlinecolor, bool vertical) {

	this->GradientBox(x, y, width, height, color, color2, vertical);
	this->BorderedBox(x, y, width, height, outlinecolor, thickness);
}
void CRender::Circle(int x, int y, int radius, int points, D3DCOLOR color)
{
	vertex* pVertex = new vertex[points + 1];
	for (int i = 0; i <= points; i++) pVertex[i] = { x + radius * cos(D3DX_PI * (i / (points / 2.0f))), y - radius * sin(D3DX_PI * (i / (points / 2.0f))), 0.0f, 1.0f, color };
	this->device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	this->device->DrawPrimitiveUP(D3DPT_LINESTRIP, points, pVertex, sizeof(vertex));
	delete[] pVertex;
}
void CRender::FilledCircle(int x, int y, int radius, int points, D3DCOLOR color) {
	vertex* pVertex = new vertex[points + 1];
	for (int i = 0; i <= points; i++) pVertex[i] = { x + radius * cos(D3DX_PI * (i / (points / 2.0f))), y + radius * sin(D3DX_PI * (i / (points / 2.0f))), 0.0f, 1.0f, color };
	this->device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	this->device->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, points, pVertex, sizeof(vertex));
	delete[] pVertex;
}

void CRender::Text(char *text, float x, float y, int orientation, LPD3DXFONT pFont, bool bordered, DWORD color, DWORD bcolor)
{

	RECT rect;

	switch (orientation)
	{
	case lefted:
		if (bordered)
		{
			SetRect(&rect, x - 1, y, x - 1, y);
			pFont->DrawTextA(NULL, text, -1, &rect, DT_LEFT | DT_NOCLIP, bcolor);
			SetRect(&rect, x + 1, y, x + 1, y);
			pFont->DrawTextA(NULL, text, -1, &rect, DT_LEFT | DT_NOCLIP, bcolor);
			SetRect(&rect, x, y - 1, x, y - 1);
			pFont->DrawTextA(NULL, text, -1, &rect, DT_LEFT | DT_NOCLIP, bcolor);
			SetRect(&rect, x, y + 1, x, y + 1);
			pFont->DrawTextA(NULL, text, -1, &rect, DT_LEFT | DT_NOCLIP, bcolor);
		}
		SetRect(&rect, x, y, x, y);
		pFont->DrawTextA(NULL, text, -1, &rect, DT_LEFT | DT_NOCLIP, color);
		break;
	case centered:
		if (bordered)
		{
			SetRect(&rect, x - 1, y, x - 1, y);
			pFont->DrawTextA(NULL, text, -1, &rect, DT_CENTER | DT_NOCLIP, bcolor);
			SetRect(&rect, x + 1, y, x + 1, y);
			pFont->DrawTextA(NULL, text, -1, &rect, DT_CENTER | DT_NOCLIP, bcolor);
			SetRect(&rect, x, y - 1, x, y - 1);
			pFont->DrawTextA(NULL, text, -1, &rect, DT_CENTER | DT_NOCLIP, bcolor);
			SetRect(&rect, x, y + 1, x, y + 1);
			pFont->DrawTextA(NULL, text, -1, &rect, DT_CENTER | DT_NOCLIP, bcolor);
		}
		SetRect(&rect, x, y, x, y);
		pFont->DrawTextA(NULL, text, -1, &rect, DT_CENTER | DT_NOCLIP, color);
		break;
	case righted:
		if (bordered)
		{
			SetRect(&rect, x - 1, y, x - 1, y);
			pFont->DrawTextA(NULL, text, -1, &rect, DT_RIGHT | DT_NOCLIP, bcolor);
			SetRect(&rect, x + 1, y, x + 1, y);
			pFont->DrawTextA(NULL, text, -1, &rect, DT_RIGHT | DT_NOCLIP, bcolor);
			SetRect(&rect, x, y - 1, x, y - 1);
			pFont->DrawTextA(NULL, text, -1, &rect, DT_RIGHT | DT_NOCLIP, bcolor);
			SetRect(&rect, x, y + 1, x, y + 1);
			pFont->DrawTextA(NULL, text, -1, &rect, DT_RIGHT | DT_NOCLIP, bcolor);
		}
		SetRect(&rect, x, y, x, y);
		pFont->DrawTextA(NULL, text, -1, &rect, DT_RIGHT | DT_NOCLIP, color);
		break;
	}
}
void CRender::String(float x, float y, int orientation, LPD3DXFONT pFont, bool bordered, DWORD color, int a, const char *input, ...)
{
	CHAR szBuffer[MAX_PATH];

	if (!input)
		return;


	DWORD bcolor = D3DCOLOR_RGBA(0, 0, 0, 255);

	vsprintf_s(szBuffer, input, (char*)&input + _INTSIZEOF(input));

	RECT rect;

	switch (orientation)
	{
	case lefted:
		if (bordered)
		{
			SetRect(&rect, x - 1, y, x - 1, y);
			pFont->DrawTextA(NULL, szBuffer, -1, &rect, DT_LEFT | DT_NOCLIP, bcolor);
			SetRect(&rect, x + 1, y, x + 1, y);
			pFont->DrawTextA(NULL, szBuffer, -1, &rect, DT_LEFT | DT_NOCLIP, bcolor);
			SetRect(&rect, x, y - 1, x, y - 1);
			pFont->DrawTextA(NULL, szBuffer, -1, &rect, DT_LEFT | DT_NOCLIP, bcolor);
			SetRect(&rect, x, y + 1, x, y + 1);
			pFont->DrawTextA(NULL, szBuffer, -1, &rect, DT_LEFT | DT_NOCLIP, bcolor);
		}
		SetRect(&rect, x, y, x, y);
		pFont->DrawTextA(NULL, szBuffer, -1, &rect, DT_LEFT | DT_NOCLIP, color);
		break;
	case centered:
		if (bordered)
		{
			SetRect(&rect, x - 1, y, x - 1, y);
			pFont->DrawTextA(NULL, szBuffer, -1, &rect, DT_CENTER | DT_NOCLIP, bcolor);
			SetRect(&rect, x + 1, y, x + 1, y);
			pFont->DrawTextA(NULL, szBuffer, -1, &rect, DT_CENTER | DT_NOCLIP, bcolor);
			SetRect(&rect, x, y - 1, x, y - 1);
			pFont->DrawTextA(NULL, szBuffer, -1, &rect, DT_CENTER | DT_NOCLIP, bcolor);
			SetRect(&rect, x, y + 1, x, y + 1);
			pFont->DrawTextA(NULL, szBuffer, -1, &rect, DT_CENTER | DT_NOCLIP, bcolor);
		}
		SetRect(&rect, x, y, x, y);
		pFont->DrawTextA(NULL, szBuffer, -1, &rect, DT_CENTER | DT_NOCLIP, color);
		break;
	case righted:
		if (bordered)
		{
			SetRect(&rect, x - 1, y, x - 1, y);
			pFont->DrawTextA(NULL, szBuffer, -1, &rect, DT_RIGHT | DT_NOCLIP, bcolor);
			SetRect(&rect, x + 1, y, x + 1, y);
			pFont->DrawTextA(NULL, szBuffer, -1, &rect, DT_RIGHT | DT_NOCLIP, bcolor);
			SetRect(&rect, x, y - 1, x, y - 1);
			pFont->DrawTextA(NULL, szBuffer, -1, &rect, DT_RIGHT | DT_NOCLIP, bcolor);
			SetRect(&rect, x, y + 1, x, y + 1);
			pFont->DrawTextA(NULL, szBuffer, -1, &rect, DT_RIGHT | DT_NOCLIP, bcolor);
		}
		SetRect(&rect, x, y, x, y);
		pFont->DrawTextA(NULL, szBuffer, -1, &rect, DT_RIGHT | DT_NOCLIP, color);
		break;
	}
}



void CRender::Message(char *text, float x, float y, LPD3DXFONT pFont, int orientation)
{
	RECT rect = { x, y, x, y };

	switch (orientation)
	{
	case lefted:
		pFont->DrawTextA(NULL, text, -1, &rect, DT_CALCRECT | DT_LEFT, BLACK(255));

		this->BorderedBox(x - 5, rect.top - 5, rect.right - x + 10, rect.bottom - rect.top + 10, 5, SKYBLUE(255));

		SetRect(&rect, x, y, x, y);
		pFont->DrawTextA(NULL, text, -1, &rect, DT_LEFT | DT_NOCLIP, ORANGE(255));
		break;
	case centered:
		pFont->DrawTextA(NULL, text, -1, &rect, DT_CALCRECT | DT_CENTER, BLACK(255));

		this->BorderedBox(rect.left - 5, rect.top - 5, rect.right - rect.left + 10, rect.bottom - rect.top + 10, 5, SKYBLUE(255));

		SetRect(&rect, x, y, x, y);
		pFont->DrawTextA(NULL, text, -1, &rect, DT_CENTER | DT_NOCLIP, ORANGE(255));
		break;
	case righted:
		pFont->DrawTextA(NULL, text, -1, &rect, DT_CALCRECT | DT_RIGHT, BLACK(255));

		this->BorderedBox(rect.left - 5, rect.top - 5, rect.right - rect.left + 10, rect.bottom - rect.top + 10, 5, SKYBLUE(255));

		SetRect(&rect, x, y, x, y);
		pFont->DrawTextA(NULL, text, -1, &rect, DT_RIGHT | DT_NOCLIP, ORANGE(255));
		break;
	}
}


void CRender::Sprite(LPDIRECT3DTEXTURE9 tex, float x, float y, float resolution_x, float resolution_y, float scale_x, float scale_y, float rotation, DWORD color)
{
	float screen_pos_x = x;
	float screen_pos_y = y;


	// Screen position of the sprite
	D3DXVECTOR2 trans = D3DXVECTOR2(screen_pos_x, screen_pos_y);

	// Build our matrix to rotate, scale and position our sprite
	D3DXMATRIX mat;

	//addition to have same icons
	scale_x = 64 / resolution_x;
	scale_y = 64 / resolution_y;

	// Texture being used is by native resolution
	//D3DXVECTOR2 spriteCentre = D3DXVECTOR2(resolution_x / 2, resolution_y / 2);


	// Texture being used is 64x64:
	D3DXVECTOR2 spriteCentre = D3DXVECTOR2(64 / 2, 64 / 2);


	D3DXVECTOR2 scaling(scale_x, scale_y);

	// out, scaling centre, scaling rotation, scaling, rotation centre, rotation, translation
	D3DXMatrixTransformation2D(&mat, NULL, 0.0, &scaling, &spriteCentre, rotation, &trans);

	//this->device->SetRenderState(D3DRS_ZENABLE, false);
	this->device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	this->device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	this->device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	this->device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	this->device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	this->device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	this->device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1);
	this->device->SetPixelShader(NULL);
	this->sprite->Begin(NULL);
	this->sprite->SetTransform(&mat); // Tell the sprite about the matrix
	this->sprite->Draw(tex, NULL, NULL, NULL, color);
	//this->sprite->Draw(tex, NULL, NULL, &D3DXVECTOR3(x, y, 0), 0xFFFFFFFF);
	this->sprite->End();
}

int CRender::FrameRate()
{
	static int iFps, iLastFps;
	static float flLastTickCount, flTickCount;
	flTickCount = clock() * 0.001f;
	iFps++;
	if ((flTickCount - flLastTickCount) >= 1.0f) {
		flLastTickCount = flTickCount;
		iLastFps = iFps;
		iFps = 0;
	}
	return iLastFps;
}