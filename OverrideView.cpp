#include "hooks.h"
#include "global.h"
#include "Menu.h"
#include "sdk.h"
#include "GrenadePrediction.h"
#include "SpoofedConvar.h"

ClientVariables Menu;

void aspectration(float aspect_ratio_multiplier)
{
	int screen_width, screen_height;
	g_pEngine->GetScreenSize(screen_width, screen_height);

	float aspectratio_value = (screen_width * aspect_ratio_multiplier) / screen_height;

	static ConVar* aspectratio = g_pCvar->FindVar("r_aspectratio");

	aspectratio->SetValue(aspectratio_value);
}

void on_aspect_ratio_changed()
{
	float aspect_ratio = (Menu.Misc.aspectratio) * 0.01;
	aspect_ratio = 2 - aspect_ratio;
	if (Menu.Misc.aspectratioenable)
		aspectration(aspect_ratio);
	else
		aspectration(0);
}


void __fastcall Hooks::OverrideView(void* _this, void* _edx, CViewSetup* setup)
{
	if (csgo::LocalPlayer && g_pEngine->IsConnected() && g_pEngine->IsInGame())
	{
		auto zoomsensratio = g_pCvar->FindVar("zoom_sensitivity_ratio_mouse");
		if (csgo::LocalPlayer && csgo::LocalPlayer->GetHealth() > 0)
		{
			if (!csgo::LocalPlayer->IsScoped())
			{
				setup->fov = 90 + Menu.Misc.PlayerFOV;
			}
			else
			{
				if (Menu.Misc.static_scope) {
					setup->fov = 90 + Menu.Misc.PlayerFOV;
					zoomsensratio->SetValue("0");
				}
				else {
					zoomsensratio->SetValue("1");
				}
			}
			if (Menu.Misc.viewmodelchange)
			{
				g_pCvar->FindVar("viewmodel_offset_x")->SetValue(Menu.Misc.viewmodelx);
				g_pCvar->FindVar("viewmodel_offset_y")->SetValue(Menu.Misc.viewmodely);
				g_pCvar->FindVar("viewmodel_offset_z")->SetValue(Menu.Misc.viewmodelz - 10);
			}
		}
		on_aspect_ratio_changed();
	}
	grenade_prediction::instance().View(setup);

	clientmodeVMT->GetOriginalMethod<OverrideViewFn>(18)(_this, setup);
}

float __stdcall GGetViewModelFOV()
{
	float fov = g_pClientModeHook->GetMethod<oGetViewModelFOV>(35)();

	if (g_pEngine->IsConnected() && g_pEngine->IsInGame())
	{
		if (csgo::LocalPlayer && Menu.Misc.viewmodelchange)
		{
				fov += Menu.Misc.PlayerViewmodel;
		}
	}
	return fov;
}