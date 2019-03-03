#include "sdk.h"
#include "backtrackmanager.h"
#include "legitbot.h"
#include "global.h"
#include "GameUtils.h"
#include "Math.h"
#include <iostream>

#define    HITGROUP_GENERIC    0
#define    HITGROUP_HEAD        1
#define    HITGROUP_CHEST        2
#define    HITGROUP_STOMACH    3
#define HITGROUP_LEFTARM    4    
#define HITGROUP_RIGHTARM    5
#define HITGROUP_LEFTLEG    6
#define HITGROUP_RIGHTLEG    7
#define HITGROUP_GEAR        10

CLegitBot* g_Legitbot = new CLegitBot();

//stiz to kurwa



void CLegitBot::Init()
{
	IsLocked = false;
	TargetID = -1;
	HitBox = -1;
}

void CLegitBot::Draw()
{

}

static int CustomDelay = 0;
static int CustomBreak = 0;

void CLegitBot::Move(CUserCmd *pCmd, bool& bSendPacket)
{

	if (!Menu.LegitBot.bEnable)
		return;
	CBaseEntity* pLocal = csgo::LocalPlayer;
	CBaseCombatWeapon* pWeapon = csgo::MainWeapon;

	static int CustomAimTime = 0;
	static int CustomAimStart = 0;
	if (Menu.LegitBot.bEnable)
	{
		if (StartAim > 0)
		{
			if (CustomAimStart < (StartAim * 333))
			{
				CustomAimStart++;
			}
			else
			{
				if (Aimtime > 0)
				{
					if (CustomAimTime < (Aimtime * 333))
					{

						DoAimbot(pCmd, bSendPacket);
						CustomAimTime++;
					}
					if (!GetAsyncKeyState(Menu.LegitBot.aimkey) || Menu.LegitBot.autofire)
					{
						CustomAimTime = 0;
						CustomAimStart = 0;
					}
				}
				else
				{

					DoAimbot(pCmd, bSendPacket);
					CustomAimTime = 0;
					CustomAimStart = 0;
				}
			}

			if (!GetAsyncKeyState(Menu.LegitBot.aimkey) || Menu.LegitBot.autofire)
			{
				CustomAimStart = 0;
				CustomAimTime = 0;
			}
		}
		else
		{
			if (Aimtime > 0)
			{
				if (CustomAimTime < (Aimtime * 333))
				{
					DoAimbot(pCmd, bSendPacket);
					CustomAimTime++;
				}
				if (!GetAsyncKeyState(Menu.LegitBot.aimkey) || Menu.LegitBot.autofire)
				{
					CustomAimTime = 0;
					CustomAimStart = 0;
				}
			}
			else
			{
				DoAimbot(pCmd, bSendPacket);
				CustomAimTime = 0;
				CustomAimStart = 0;
			}
		}
	}
	if (Menu.LegitBot.triggerboton && (GetAsyncKeyState(Menu.LegitBot.triggerbotkey)))
	{

		DoTrigger(pCmd);
	}

	SyncWeaponSettings();
}

void CLegitBot::SyncWeaponSettings()
{
	CBaseEntity* pLocal = csgo::LocalPlayer;
	CBaseCombatWeapon* pWeapon = csgo::MainWeapon;

	if (!pWeapon)
		return;

	if (pWeapon->IsPistol())
	{

		Speed = Menu.LegitBot.pis_aimspeed / 100;
		FoV = Menu.LegitBot.pis_fov * 2;
		PSilent = Menu.LegitBot.pis_pSilent;
		RecoilControl = Menu.LegitBot.pis_rcs;

		switch (Menu.LegitBot.pis_hitbox)
		{
		case 0:
			HitBox = ((int)CSGOHitboxID::HEAD);
			Multihitbox = false;
			break;
		case 1:
			HitBox = ((int)CSGOHitboxID::NECK);
			Multihitbox = false;

			break;
		case 2:
			HitBox = ((int)CSGOHitboxID::UPPER_CHEST);
			Multihitbox = false;

			break;
		case 3:
			HitBox = ((int)CSGOHitboxID::BELLY);
			Multihitbox = false;

			break;
		case 4:
			Multihitbox = true;
			break;
		}
		Aimtime = 0;
		StartAim = 0;
	}
	else if (pWeapon->IsBoltSniper())
	{
		Speed = Menu.LegitBot.sni_aimspeed / 100;
		FoV = Menu.LegitBot.sni_fov * 2;
		PSilent = Menu.LegitBot.sni_pSilent;
		RecoilControl = Menu.LegitBot.sni_rcs;

		switch (Menu.LegitBot.sni_hitbox)
		{
		case 0:
			HitBox = ((int)CSGOHitboxID::HEAD);
			Multihitbox = false;
			break;
		case 1:
			HitBox = ((int)CSGOHitboxID::NECK);
			Multihitbox = false;

			break;
		case 2:
			HitBox = ((int)CSGOHitboxID::UPPER_CHEST);
			Multihitbox = false;

			break;
		case 3:
			HitBox = ((int)CSGOHitboxID::BELLY);
			Multihitbox = false;

			break;
		case 4:
			Multihitbox = true;
			break;

		}
		Aimtime = 0;
		StartAim = 0;
	}	
	else if (pWeapon->IsSMG())
	{
		Speed = Menu.LegitBot.smg_aimspeed / 100;
		FoV = Menu.LegitBot.smg_fov * 2;
		PSilent = Menu.LegitBot.smg_pSilent;
		RecoilControl = Menu.LegitBot.smg_rcs;

		switch (Menu.LegitBot.smg_hitbox)
		{
		case 0:
			HitBox = ((int)CSGOHitboxID::HEAD);
			Multihitbox = false;
			break;
		case 1:
			HitBox = ((int)CSGOHitboxID::NECK);
			Multihitbox = false;

			break;
		case 2:
			HitBox = ((int)CSGOHitboxID::UPPER_CHEST);
			Multihitbox = false;

			break;
		case 3:
			HitBox = ((int)CSGOHitboxID::BELLY);
			Multihitbox = false;

			break;
		case 4:
			Multihitbox = true;
			break;

		}
		Aimtime = 0;
		StartAim = 0;
	}
	else
	{

		Speed = Menu.LegitBot.def_aimspeed / 100;
		FoV = Menu.LegitBot.def_fov * 2;
		PSilent = Menu.LegitBot.def_pSilent;
		RecoilControl = Menu.LegitBot.def_rcs;

		switch (Menu.LegitBot.def_hitbox)
		{
		case 0:
			HitBox = ((int)CSGOHitboxID::HEAD);
			Multihitbox = false;
			break;
		case 1:
			HitBox = ((int)CSGOHitboxID::NECK);
			Multihitbox = false;

			break;
		case 2:
			HitBox = ((int)CSGOHitboxID::UPPER_CHEST);
			Multihitbox = false;

			break;
		case 3:
			HitBox = ((int)CSGOHitboxID::BELLY);
			Multihitbox = false;

			break;
		case 4:
			Multihitbox = true;
			break;

		}
		Aimtime = 0;
		StartAim = 0;
	}
}

void CLegitBot::DoAimbot(CUserCmd *pCmd, bool &bSendPacket)
{
	CBaseEntity* pTarget = nullptr;
	CBaseEntity* pLocal = csgo::LocalPlayer;
	bool FindNewTarget = true;

	CBaseCombatWeapon* pWeapon = csgo::MainWeapon;
	if (pWeapon)
	{
		if (pWeapon->GetLoadedAmmo() == 0 || pWeapon->IsMiscWeapon())
		{
			return;
		}
		SyncWeaponSettings();
	}
	else
		return;

	if (IsLocked && TargetID >= 0 && HitBox >= 0)
	{

		pTarget = g_pEntitylist->GetClientEntity(TargetID);
		if (pTarget  && TargetMeetsRequirements(pTarget))
		{
			SyncWeaponSettings();
			if (HitBox >= 0)
			{
				Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
				Vector View; g_pEngine->GetViewAngles(View);
				if (pLocal->GetVelocity().Length() > 45.f);
				View += pLocal->GetPunchAngle() * RecoilControl;
				float nFoV = FovToPlayer(ViewOffset, View, pTarget, HitBox);
				if (nFoV < FoV)
					FindNewTarget = false;
			}
		}
	}

	if (FindNewTarget)
	{
		TargetID = 0;
		pTarget = nullptr;
		HitBox = -1;

		TargetID = GetTargetCrosshair();

		if (TargetID >= 0)
		{

			pTarget = g_pEntitylist->GetClientEntity(TargetID);
		}
		else
		{
			pTarget = nullptr;
			HitBox = -1;
		}
	}

	SyncWeaponSettings();

	if (TargetID >= 0 && pTarget)
	{
		SyncWeaponSettings();

		if (!Menu.LegitBot.autofire)
		{

			int Key = Menu.LegitBot.aimkey;
			if (Key >= 0 && !GetAsyncKeyState(Key))
			{
				TargetID = -1;
				pTarget = nullptr;
				HitBox = -1;
				return;
			}
		}

		Vector AimPoint;

		if (Multihitbox)
		{

			AimPoint = GameUtils::get_hitbox_location(pTarget, besthitbox);
		}
		else
		{

			AimPoint = GameUtils::get_hitbox_location(pTarget, HitBox);
		}

		if (AimAtPoint(pLocal, AimPoint, pCmd, bSendPacket))
		{
			if (Menu.LegitBot.autofire && !(pCmd->buttons & IN_ATTACK))
			{

				pCmd->buttons |= IN_ATTACK;
			}
		}
	}

}

bool CLegitBot::TargetMeetsTriggerRequirements(CBaseEntity* pEntity)
{
	if (pEntity && pEntity->IsDormant() == false && pEntity->isAlive() && pEntity->GetIndex() != csgo::LocalPlayer->GetIndex() && pEntity->GetIndex() < 65)
	{
		if (pEntity->GetTeamNum() != csgo::LocalPlayer->GetTeamNum() || Menu.LegitBot.friendlyfire)
		{
			if (!pEntity->IsProtected())
			{

				return true;
			}
		}

	}

	return false;
}

void CLegitBot::DoTrigger(CUserCmd *pCmd)
{
	CBaseEntity* LocalPlayer = csgo::LocalPlayer;

	auto LocalPlayerWeapon = csgo::MainWeapon;

	if (LocalPlayerWeapon) {
		if (LocalPlayerWeapon->GetLoadedAmmo() == 0)
			return;

		if(LocalPlayerWeapon->IsMiscWeapon())
			return;
	}
	else
		return;

	Vector ViewAngles = pCmd->viewangles;
	if (Menu.LegitBot.triggerrcs)
		ViewAngles += LocalPlayer->GetPunchAngle() * 2.0f;

	Vector CrosshairForward;
	Math::AngleVectors(ViewAngles, &CrosshairForward);
	CrosshairForward *= 8000.f;


	Vector TraceSource = LocalPlayer->GetEyePosition();
	Vector TraceDestination = TraceSource + CrosshairForward;

	Ray_t Ray;
	trace_t Trace;
	CTraceFilter Filter;

	Filter.pSkip1 = LocalPlayer;

	Ray.Init(TraceSource, TraceDestination);
	g_pEngineTrace->TraceRay(Ray, MASK_SHOT, &Filter, &Trace);

	if (!Trace.m_pEnt)
		return;
	if (!Trace.m_pEnt->isAlive())
		return;
	if (Trace.m_pEnt->GetHealth() <= 0 || Trace.m_pEnt->GetHealth() > 100)
		return;
	if (Trace.m_pEnt->IsProtected())
		return;

	if (!Menu.LegitBot.triggerteam) {
		if (LocalPlayer->GetTeamNum() == Trace.m_pEnt->GetTeamNum())
			return;
	}

	if (Menu.LegitBot.triggersmokecheck)
	{

		typedef bool(__cdecl* GoesThroughSmoke)(Vector, Vector);

		static uint32_t GoesThroughSmokeOffset = (uint32_t)Utilities::Memory::FindPatternIDA("client_panorama.dll", "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0");
		static GoesThroughSmoke GoesThroughSmokeFunction = (GoesThroughSmoke)GoesThroughSmokeOffset;

		if (GoesThroughSmokeFunction(csgo::LocalPlayer->GetEyePosition(), Trace.m_pEnt->GetBonePos(6)))
			return;
	}

	if ((Menu.LegitBot.headhitscan && Trace.hitgroup == HITGROUP_HEAD) ||
		(Menu.LegitBot.chesthitscan && Trace.hitgroup == HITGROUP_CHEST) ||
		(Menu.LegitBot.stomachhitscan && Trace.hitgroup == HITGROUP_STOMACH) ||
		(Menu.LegitBot.armshitscan && (Trace.hitgroup == HITGROUP_LEFTARM || Trace.hitgroup == HITGROUP_RIGHTARM)) ||
		(Menu.LegitBot.legshitscan && (Trace.hitgroup == HITGROUP_LEFTLEG || Trace.hitgroup == HITGROUP_RIGHTLEG))) {
		pCmd->buttons |= IN_ATTACK;
	}
}

bool CLegitBot::TargetMeetsRequirements(CBaseEntity* pEntity)
{

	if (pEntity && pEntity->IsDormant() == false && pEntity->isAlive() && pEntity->GetIndex() != csgo::LocalPlayer->GetIndex())
	{
		player_info_t pinfo;
		if (pEntity->IsValidTarget() && g_pEngine->GetPlayerInfo(pEntity->GetIndex(), &pinfo))
		{
			if (pEntity->GetTeamNum() != csgo::LocalPlayer->GetTeamNum() || Menu.LegitBot.friendlyfire)
			{

				if (Menu.LegitBot.aimsmokecheck) {
					typedef bool(__cdecl* GoesThroughSmoke)(Vector, Vector);

					static uint32_t GoesThroughSmokeOffset = (uint32_t)Utilities::Memory::FindPatternIDA("client_panorama.dll", "55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0");
					static GoesThroughSmoke GoesThroughSmokeFunction = (GoesThroughSmoke)GoesThroughSmokeOffset;

					if (GoesThroughSmokeFunction(csgo::LocalPlayer->GetEyePosition(), pEntity->GetBonePos(8)))
						return false;
				}

				if (Multihitbox)
				{
					if (!pEntity->IsProtected())
					{
						if (GameUtils::isVisible(pEntity, 0) || GameUtils::isVisible(pEntity, 8) || GameUtils::isVisible(pEntity, 4) || GameUtils::isVisible(pEntity, 1) || GameUtils::isVisible(pEntity, 2) || GameUtils::isVisible(pEntity, 3) || GameUtils::isVisible(pEntity, 5) || GameUtils::isVisible(pEntity, 7) || GameUtils::isVisible(pEntity, 9) || GameUtils::isVisible(pEntity, 10) || GameUtils::isVisible(pEntity, 6))
							return true;
						else
							return false;
					}
				}
				else
				{
					if (!pEntity->IsProtected() && GameUtils::isVisible(pEntity, HitBox))
					{
						return true;
					}
				}

			}

		}
	}
	return false;
}

float Get3dDistance(Vector me, Vector ent)
{
	return sqrt(pow(double(ent.x - me.x), 2.0) + pow(double(ent.y - me.y), 2.0) + pow(double(ent.z - me.z), 2.0));
}

float CLegitBot::FovToPlayer(Vector ViewOffSet, Vector View, CBaseEntity* pEntity, int aHitBox)
{
	CONST FLOAT MaxDegrees = 180.0f;

	Vector Angles = View;

	Vector Origin = ViewOffSet;

	Vector Delta(0, 0, 0);

	Vector Forward(0, 0, 0);

	Math::AngleVectors(Angles, &Forward);
	Vector AimPos = GameUtils::get_hitbox_location(pEntity, aHitBox);

	VectorSubtract(AimPos, Origin, Delta);

	Math::Normalize(Delta, Delta);

	float Distance = Get3dDistance(Origin, AimPos);

	float pitch = sin(Forward.x - Delta.x) * Distance;
	float yaw = sin(Forward.y - Delta.y) * Distance;
	float zaw = sin(Forward.z - Delta.z) * Distance;

	float mag = sqrt((pitch*pitch) + (yaw*yaw) + (zaw*zaw));
	return mag;
}

int CLegitBot::GetTargetCrosshair()
{
	SyncWeaponSettings();
	int target = -1;


	CBaseEntity* pLocal = csgo::LocalPlayer;
	Vector ViewOffset = pLocal->GetOrigin() + pLocal->GetViewOffset();
	Vector View; g_pEngine->GetViewAngles(View);
	View += pLocal->GetPunchAngle() * 2.f;

	for (int i = 0; i < g_pEntitylist->GetHighestEntityIndex(); i++)
	{
		CBaseEntity *pEntity = g_pEntitylist->GetClientEntity(i);

		if (TargetMeetsRequirements(pEntity))
		{
			if (Multihitbox)
			{

				float fov1 = FovToPlayer(ViewOffset, View, pEntity, 0);
				float fov2 = FovToPlayer(ViewOffset, View, pEntity, 4);
				float fov3 = FovToPlayer(ViewOffset, View, pEntity, 6);

				if (fov1 < FoV || fov2 < FoV && fov1 < FoV || fov3 < FoV)
				{
					FoV = fov1;
					target = i;
					besthitbox = 0;
				}

				if (fov2 < FoV || fov1 < FoV && fov2 < FoV || fov3 < FoV)
				{
					FoV = fov2;
					target = i;
					besthitbox = 4;
				}

				if (fov3 < FoV || fov1 < FoV && fov3 < FoV || fov2 < FoV)
				{
					FoV = fov3;
					target = i;
					besthitbox = 6;
				}

			}
			else
			{

				int NewHitBox = HitBox;
				if (NewHitBox >= 0)
				{
					float fov = FovToPlayer(ViewOffset, View, pEntity, 0);
					if (fov < FoV)
					{
						FoV = fov;
						target = i;
					}
				}
			}
		}
	}

	return target;
}

bool ShouldFire()
{
	CBaseEntity* pLocalEntity = csgo::LocalPlayer;
	if (!pLocalEntity)
		return false;

	CBaseCombatWeapon* entwep = csgo::MainWeapon;

	float flServerTime = (float)pLocalEntity->GetTickBase() * g_pGlobals->interval_per_tick;
	float flNextPrimaryAttack = entwep->NextPrimaryAttack();

	std::cout << flServerTime << " " << flNextPrimaryAttack << std::endl;

	return !(flNextPrimaryAttack > flServerTime);
}


bool CLegitBot::AimAtPoint(CBaseEntity* pLocal, Vector point, CUserCmd *pCmd, bool &bSendPacket)
{

	if (point.Length() == 0) return false;

	Vector angles;
	Vector src = pLocal->GetOrigin() + pLocal->GetViewOffset();

	Math::CalcAngle(src, point, angles);
	Math::NormalizeAngle(angles);

	if (angles[0] != angles[0] || angles[1] != angles[1])
	{
		return false;
	}

	if (RecoilControl > 0)
	{

		Vector AimPunch = pLocal->GetPunchAngle();
		if (AimPunch.Length2D() > 0 && AimPunch.Length2D() < 150)
		{
			angles -= AimPunch * RecoilControl;
			Math::NormalizeAngle(angles);
		}
	}

	IsLocked = true;

	Vector shit = angles - pCmd->viewangles;
	bool v = false;
	Math::NormalizeAngle(shit);
	if (shit.Length() > Speed)
	{
		Math::Normalize(shit, shit);
		shit *= Speed;
	}
	else
	{
		v = true;

	}

	Vector ang = angles - pCmd->viewangles;
	bool f = false;

	if (ang.Length() > Speed)
	{
		Math::Normalize(ang, ang);
		ang *= Speed;
	}
	else
	{
		f = true;
	}

	if (PSilent)
	{
		Vector Oldview = pCmd->viewangles;
		Vector qAimAngles = pCmd->viewangles;
		float Oldsidemove = pCmd->sidemove;
		float Oldforwardmove = pCmd->forwardmove;

		static int ChokedPackets = -1;

		if (ShouldFire() && ChokedPackets < 6)
		{
			bSendPacket = false;
			pCmd->viewangles += ang;
			pCmd->viewangles = angles;
			ChokedPackets++;
		}
		else
		{
			bSendPacket = true;
			pCmd->viewangles = Oldview;
			pCmd->sidemove = Oldsidemove;
			pCmd->forwardmove = Oldforwardmove;
			ChokedPackets = -1;
		}

		pCmd->viewangles.z = 0;
	}
	else
	{
		pCmd->viewangles += ang;
		g_pEngine->SetViewAngles(pCmd->viewangles);
	}
	return f;

	pCmd->viewangles += shit;
	g_pEngine->SetViewAngles(pCmd->viewangles);

	return f;
}