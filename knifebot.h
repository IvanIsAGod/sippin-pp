#pragma once
#include "includes.h"
#include "Autowall.h"

class KnifeBot {
public:
	static void Run() {
		if (!csgo::MainWeapon)
			return;
		if (!csgo::MainWeapon->IsKnife())
			return;
		for (int y = 0; y <= 360; y += 360.f / 6.f) {
			for (int x = -89; x <= 89; x += 179.f / 6.f) {
				Vector ang = Vector(x, y, 0);
				Vector dir;
				Math::AngleVectors(ang, dir);
				trace_t trace;

				auto TraceLine = [](Vector& vecAbsStart, Vector& vecAbsEnd, unsigned int mask, CBaseEntity *ignore, int collisionGroup, trace_t *ptr)
				{
					Ray_t ray;

					CTraceFilter filter;
					filter.pSkip1 = ignore;

					ray.Init(vecAbsStart, vecAbsEnd);

					g_pEngineTrace->TraceRay(ray, mask, &filter, ptr);

				};

				TraceLine(csgo::LocalPlayer->GetEyePosition(), csgo::LocalPlayer->GetEyePosition() + (dir * 64), MASK_SHOT, csgo::LocalPlayer, 0, &trace);

				if (trace.m_pEnt == nullptr)
					continue;
				if (trace.m_pEnt == csgo::LocalPlayer)
					continue;
				if (!trace.m_pEnt->isAlive())
					continue;
				if (!(trace.m_pEnt->GetHealth() > 0))
					continue;
				if (trace.m_pEnt->GetTeamNum() == csgo::LocalPlayer->GetTeamNum())
					continue;
				if (trace.m_pEnt->IsDormant())
					continue;
				if (trace.m_pEnt->IsProtected())
					continue;
				player_info_t info;
				if (!(g_pEngine->GetPlayerInfo(trace.m_pEnt->GetIndex(), &info)))
					continue;
				csgo::UserCmd->viewangles = QAngle(x, y, 0);
				csgo::UserCmd->buttons |= IN_ATTACK2;
				return;
			}
		}
	}
};