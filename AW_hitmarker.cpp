#pragma once
#include "GameUtils.h"
#include "AW_hitmarker.h"
#include <chrono>
#include "Draw.h"
#include "render.h"
CAW_hitmaker* hitmarker_2 = new CAW_hitmaker();

void CAW_hitmaker::paint()
{
	if (!Menu.Visuals.Hitmarker)
		return;

	float time = g_pGlobals->curtime;

	for (int i = 0; i < hitmarkers.size(); i++)
	{
		bool expired = time >= hitmarkers.at(i).impact.time + 2.f;
		if (expired)
			hitmarkers.at(i).alpha -= 1;
		if (expired && hitmarkers.at(i).alpha <= 0)
		{
			hitmarkers.erase(hitmarkers.begin() + i);
			continue;
		}

		Vector pos3D = Vector(hitmarkers.at(i).impact.x, hitmarkers.at(i).impact.y, hitmarkers.at(i).impact.z), pos2D;
		if (!GameUtils::WorldToScreen(pos3D, pos2D))
			continue;

		int lineSize = Menu.Visuals.hitmarkerSize;
		g_pSurface->SetDrawColor(240, 240, 240, hitmarkers.at(i).alpha);
		switch (Menu.Visuals.hitmarkertype)
		{
		case 0: // aimware
			g_pSurface->DrawLine(pos2D.x - lineSize * 2, pos2D.y - lineSize * 2, pos2D.x - (1), pos2D.y - (1));
			g_pSurface->DrawLine(pos2D.x - lineSize * 2, pos2D.y + lineSize * 2, pos2D.x - (1), pos2D.y + (1));
			g_pSurface->DrawLine(pos2D.x + lineSize * 2, pos2D.y + lineSize * 2, pos2D.x + (2), pos2D.y + (2));
			g_pSurface->DrawLine(pos2D.x + lineSize * 2, pos2D.y - lineSize * 2, pos2D.x + (2), pos2D.y - (2));
			break;
		case 1: // triangle
			g_pSurface->DrawOutlinedCircle(pos2D.x, pos2D.y, lineSize, 3);
			break;
		case 2: //circle
			g_pSurface->DrawOutlinedCircle(pos2D.x, pos2D.y, lineSize, 24);
			break;
		}
	}
}