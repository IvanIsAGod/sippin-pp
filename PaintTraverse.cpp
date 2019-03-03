#include "hooks.h"
#include "GameUtils.h"
#include "Menu.h"
#include "Misc.h"
#include "global.h"
#include "SpoofedConvar.h"
#include "GrenadePrediction.h"
#include "global.h"
#include "AW_hitmarker.h"
#include "event_log.h"
#include <memory>
#include "Math.h"
#include "Antiaim.h"
#include "esp.h"

bool first_frame_passed = false;
std::string sPanel = XorStr("FocusOverlayPanel");

void forceCross()
{
	if (!Menu.Visuals.ForceCrosshair)
		return;

	if (csgo::LocalPlayer && csgo::LocalPlayer->isAlive())
	{
		if (csgo::LocalPlayer->IsScoped())
			return;

		static char* DrawCrosshairWeaponTypeCheck = reinterpret_cast<char*>(FindPatternIDA(client_dll, "83 F8 05 75 17"));

		DWORD dwOldProtect;
		DWORD dwNewProtect;

		VirtualProtectEx(GetCurrentProcess(), DrawCrosshairWeaponTypeCheck, 3, PAGE_EXECUTE_READWRITE, &dwOldProtect);


		*((DrawCrosshairWeaponTypeCheck + 0x2)) = 0xFF;

		VirtualProtectEx(GetCurrentProcess(), DrawCrosshairWeaponTypeCheck, 3, dwOldProtect, &dwNewProtect);
	}
}

void __fastcall Hooks::PaintTraverse(void* ecx/*thisptr*/, void* edx, unsigned int vgui_panel, bool force_repaint, bool allow_force) // cl
{
	if (Menu.Visuals.EspEnable && Menu.Visuals.Noscope && strcmp("HudZoom", g_pPanel->GetName(vgui_panel)) == 0)
		return;

	panelVMT->GetOriginalMethod<PaintTraverseFn>(41)(ecx, vgui_panel, force_repaint, allow_force);

	static bool bSpoofed = false;

	if (Menu.Misc.TPKey > 0 && !bSpoofed)
	{
		ConVar* svcheats = g_pCvar->FindVar("sv_cheats");
		SpoofedConvar* svcheatsspoof = new SpoofedConvar(svcheats);
		svcheatsspoof->SetInt(1);
		bSpoofed = true;
	}

	const char* pszPanelName = g_pPanel->GetName(vgui_panel);

	if (!strstr(pszPanelName, sPanel.data()))
		return;

	int cur_height, cur_width; g_pEngine->GetScreenSize(cur_width, cur_height);

	if (!first_frame_passed || cur_width != csgo::Screen.width || cur_height != csgo::Screen.height)
	{
		draw::InitFonts();
		g_pEngine->GetScreenSize(cur_width, cur_height);
		csgo::Screen.height = cur_height;
		csgo::Screen.width = cur_width;
		first_frame_passed = true;
	}

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			csgo::viewMatrix[i][j] = g_pEngine->WorldToScreenMatrix()[i][j];
		}
	}

	forceCross();

	if (Menu.Visuals.Hitmarker && csgo::LocalPlayer && csgo::LocalPlayer->isAlive()) {
		hitmarker_2->paint();
	}

	ConVar* PostProcVar = g_pCvar->FindVar("mat_postprocess_enable");
	PostProcVar->SetValue(!Menu.Visuals.RemoveParticles);

	visuals::Do();
	draw::DrawPixel(1, 1, Color(0, 0, 0));
	draw::Textf(csgo::Screen.width - 70, 6, Color::White(), F::eventlog, "Sippin Piss");
	grenade_prediction::instance().Paint();
}