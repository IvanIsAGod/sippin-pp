#pragma once
#include "sdk.h"
#include "global.h"
#include "vmt.h"

using do_post_screen_space_effects_t = bool(__thiscall*)(void*, CViewSetup*);

bool _fastcall hkDoPostScreenSpaceEffects(void* ecx, void* edx, CViewSetup* pSetup)
{
	static auto ofunc = g_pClientModeHook->GetMethod<do_post_screen_space_effects_t>(44);
	if (!g_pEngine->IsConnected() || !g_pEngine->IsInGame() || !csgo::LocalPlayer)
		return ofunc(ecx, pSetup);

	auto m_local = csgo::LocalPlayer;
	for (auto i = 0; i < g_GlowObjManager->size; i++)
	{
		auto glow_object = &g_GlowObjManager->m_GlowObjectDefinitions[i];

		CBaseEntity *m_entity = glow_object->m_pEntity;

		if (!glow_object->m_pEntity || glow_object->IsUnused())
			continue;

		if (m_entity->IsPlayer())
		{
			if (Menu.Visuals.Glow && m_entity->GetTeamNum() != m_local->GetTeamNum())
			{
				float Red = Menu.Colors.Glow[0];
				float Green = Menu.Colors.Glow[1];
				float Blue = Menu.Colors.Glow[2];
				float A = Menu.Colors.Glow[3];

				glow_object->m_vGlowColor = Vector(Red, Green, Blue);
				glow_object->m_flGlowAlpha = A;

				glow_object->m_bRenderWhenOccluded = true;
				glow_object->m_bRenderWhenUnoccluded = false;
			}

			if (m_entity == m_local && Menu.Visuals.LGlow)
			{
				float Red = Menu.Colors.LGlow[0];
				float Green = Menu.Colors.LGlow[1];
				float Blue = Menu.Colors.LGlow[2];
				float A = Menu.Colors.LGlow[3];

				if (Menu.Visuals.PulseLGlow) {
					glow_object->m_bPulsatingChams = true;
				}
				glow_object->m_vGlowColor = Vector(Red, Green, Blue);
				glow_object->m_flGlowAlpha = A;

				glow_object->m_bRenderWhenOccluded = true;
				glow_object->m_bRenderWhenUnoccluded = false;
			}
		}
	}
	return ofunc(ecx, pSetup);
}