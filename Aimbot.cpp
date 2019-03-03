#include "sdk.h"
#include "Aimbot.h"
#include "global.h"
#include "Menu.h"
#include "Math.h"
#include "GameUtils.h"
#include "Autowall.h"
#include "Antiaim.h"
#include "Extrapolation.h"
#include "BacktrackingHelper.h"
#include "NoSpread.h"

#define TICK_INTERVAL			( g_pGlobals->interval_per_tick )
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )
#define TICKS_TO_TIME( t )		( TICK_INTERVAL *( t ) )

template<class T, class U>
inline T clamp(T in, U low, U high)
{
	if (in <= low)
		return low;
	else if (in >= high)
		return high;
	else
		return in;
}
#define PI_F	((float)(PI)) 
void VectorAngles(const Vector& forward, Vector& up, QAngle& angles)
{
	Vector left = CrossProduct(up, forward);
	left.NormalizeInPlace();

	float forwardDist = forward.Length2D();

	if (forwardDist > 0.001f)
	{
		angles.x = atan2f(-forward.z, forwardDist) * 180 / PI_F;
		angles.y = atan2f(forward.y, forward.x) * 180 / PI_F;

		float upZ = (left.y * forward.x) - (left.x * forward.y);
		angles.z = atan2f(left.z, upZ) * 180 / PI_F;
	}
	else
	{
		angles.x = atan2f(-forward.z, forwardDist) * 180 / PI_F;
		angles.y = atan2f(-left.x, left.y) * 180 / PI_F;
		angles.z = 0;
	}
}

auto hitchance = [](Vector Point, CBaseEntity* ent) -> bool
{
	CBaseCombatWeapon* weapon = csgo::LocalPlayer->GetWeapon();

	if (!weapon)
		return false;
	if (!ent->GetFlags() & FL_ONGROUND && (weapon->WeaponID() != ItemDefinitionIndex::SSG08 || weapon->WeaponID() != ItemDefinitionIndex::AWP))
	{
		float hitchance = 90;

		float inaccuracy = weapon->GetCone();
		if (inaccuracy == 0) inaccuracy = 0.0000001;
		inaccuracy = 1 / inaccuracy;
		hitchance = inaccuracy;
		return hitchance >= Menu.Ragebot.Minhitchance;
	}
	else
	{
		//Count hits
		int hitTraces = 0;

		Vector traceStart = csgo::LocalPlayer->GetEyePosition();

		//Get Angle Vectors
		QAngle angle = Math::CalcAngle(traceStart, Point);
		Vector forward, right, up;
		Math::AngleVectors(angle, forward, right, up);

		//Do this outside of the for statement so we dont loose precious framerate ;)
		float spread = weapon->GetSpread();
		float inaccuracy = weapon->GetInaccuracy();
		float range = weapon->GetCSWpnData()->range;

		weapon->UpdateAccuracyPenalty();

		for (int i = 0; i < 256; i++)
		{
			float random1, random2, random3, random4, sin1, cosine1, sin2, cosine2;

			RandomSeed((i & 0xff) + 1);

			random1 = RandomFloat(-.5f, .5f);
			random2 = RandomFloat(-M_PI, M_PI);
			random3 = RandomFloat(-.5f, .5f);
			random4 = RandomFloat(-M_PI, M_PI);

			sin1 = std::sin(random2);
			cosine1 = std::cos(random2);
			sin2 = std::sin(random4);
			cosine2 = std::cos(random4);

			// calculate spread vector.
			Vector vecSpread = Vector((cosine1 * (random1 * inaccuracy)) + (cosine2 * (random3 * spread)), (sin1 * (random1 * inaccuracy)) + (sin2 * (random3 * spread)), 0.f);

			Vector dir = (forward + (right * vecSpread.x) + (up * vecSpread.y)).Normalized();

			// Start the trace here.
			CGameTrace tr;
			Ray_t ray;

			// get spread direction and normalize.
			Vector traceEnd = traceStart + (dir * range);
			ray.Init(traceStart, traceEnd);

			g_pEngineTrace->ClipRayToCBaseEntity(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, ent, &tr);

			if (tr.DidHit())
				hitTraces++;
		}
		return ((hitTraces / 256.f) * 100 >= Menu.Ragebot.Minhitchance);
	}
};

std::vector<Vector> CAimbot::MainHitbox(CBaseEntity* pTarget, VMatrix BoneMatrix[128])
{
	std::vector<Vector> Points;
	switch (Menu.Ragebot.Hitbox)
	{
	case 0:
		break;
	case 1:
		Points = GameUtils::GetMultiplePointsForHitbox(pTarget, 0, BoneMatrix);
		break;
	case 2:
		Points = GameUtils::GetMultiplePointsForHitbox(pTarget, 4, BoneMatrix);//stomach
		break;
	}
	return Points;
}

std::vector<int> GetHitboxesToScan(CBaseEntity* pTarget)
{
	std::vector<int> HitBoxesToScan;
	int HitScanMode = Menu.Ragebot.Hitscan;
	int Aimspot = Menu.Ragebot.AimbotSelection;

	if (Menu.Ragebot.headhitscan)
		HitBoxesToScan.push_back((int)CSGOHitboxID::HEAD);

	if (Menu.Ragebot.chesthitscan) {
		HitBoxesToScan.push_back((int)CSGOHitboxID::UPPER_CHEST);
		HitBoxesToScan.push_back((int)CSGOHitboxID::LOWER_CHEST);
		HitBoxesToScan.push_back((int)CSGOHitboxID::THORAX);
	}

	if (Menu.Ragebot.stomachhitscan)
	{
		HitBoxesToScan.push_back((int)CSGOHitboxID::PELVIS);
		HitBoxesToScan.push_back((int)CSGOHitboxID::BELLY);
	}
	if (Menu.Ragebot.legshitscan)
	{
		HitBoxesToScan.push_back((int)CSGOHitboxID::LEFT_THIGH);
		HitBoxesToScan.push_back((int)CSGOHitboxID::RIGHT_THIGH);
	}
	if (pTarget->GetVelocity().Length2D() <= 0.15f) {
		if (Menu.Ragebot.armshitscan)
		{
			HitBoxesToScan.push_back((int)CSGOHitboxID::RIGHT_HAND);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LEFT_HAND);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RIGHT_UPPER_ARM);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RIGHT_FOREARM);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LEFT_UPPER_ARM);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LEFT_FOREARM);
		}
		if (Menu.Ragebot.feethitscan)
		{
			HitBoxesToScan.push_back((int)CSGOHitboxID::RIGHT_FOOT);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LEFT_FOOT);
			HitBoxesToScan.push_back((int)CSGOHitboxID::RIGHT_CALF);
			HitBoxesToScan.push_back((int)CSGOHitboxID::LEFT_CALF);
		}
	}
	return HitBoxesToScan;
}

void SetRecords(CBaseEntity* ent, tick_record record)
{
	ent->InvalidateBoneCache();
	ent->SetAbsOrigin(record.m_vecAbsOrigin);
	ent->SetAngle2(record.m_absangles);
	ent->SetVelocity(record.m_vecVelocity);
	ent->SetFlags(record.m_nFlags);
}

void Restore(CBaseEntity* ent)
{
	ent->SetAbsOrigin(ent->GetAbsOrigin());
	ent->SetAngle2(ent->GetAbsAngles());
	ent->SetVelocity(ent->GetVelocity());
	ent->SetFlags(*ent->GetFlags());
}

void HandleMinDamage(int Hitboxin, int Hitboxout) {
	if (Hitboxin == 0) Hitboxout = 1;
	else if (Hitboxin == 6 || Hitboxin == 5) Hitboxout = 2;
	else if (Hitboxin == 2 || Hitboxin == 3) Hitboxout = 3;
	else if (Hitboxin >= 7 && Hitboxin <= 10) Hitboxout = 6;
	else Hitboxout = 0;
}

bool is_viable_target(CBaseEntity* pEntity)
{
	//ClientClass* pClass = (ClientClass*)pEntity->GetClientClass();
	if (!pEntity) return false;
	//if (pClass->m_ClassID != (int)CSGOClassID::CCSPlayer) return false;
	if (pEntity == csgo::LocalPlayer) return false;
	if (pEntity->GetTeamNum() == csgo::LocalPlayer->GetTeamNum()) return false;
	if (pEntity->IsProtected()) return false;
	if (!pEntity->isAlive() || pEntity->IsDormant()) return false;
	return true;
}

void CAimbot::extrapolation() {
	if (Menu.Ragebot.extrapolation == 1) {
		for (int i = 1; i < g_pGlobals->maxClients; i++) {
			CBaseEntity* e = g_pEntitylist->GetClientEntity(i);
			if (!g_Antiaim->is_viable_target(e))
				continue;

			float simtime_delta = e->GetSimulationTime() - e->GetOldSimulationTime();
			int choked_ticks = clamp(TIME_TO_TICKS(simtime_delta), 1, 15);
			Vector lastOrig;

			if (lastOrig.Length() != e->GetOrigin().Length())
				lastOrig = e->GetOrigin();

			bool player_breaks_lagcomp = pTarget->GetOrigin().LengthSqr() > 4096.f;

			if (player_breaks_lagcomp) {
				Vector velocity_per_tick = e->GetVelocity() * g_pGlobals->interval_per_tick;
				auto new_origin = e->GetOrigin() + (velocity_per_tick * choked_ticks);
				e->SetAbsOrigin(new_origin);
			}
		}
	}
	else if (Menu.Ragebot.extrapolation == 2) {
		for (int i = 1; i < g_pGlobals->maxClients; i++) {
			CBaseEntity* e = g_pEntitylist->GetClientEntity(i);
			if (!g_Antiaim->is_viable_target(e))
				continue;

			Vector position = e->GetOrigin();
			float simtime = e->GetSimulationTime();

			bool player_breaks_lagcomp = position.LengthSqr() > 4096.f;
			if (player_breaks_lagcomp)
			g_Extrapolation->Run(e, position, simtime, e->GetVelocity());
		}
	}
}

Vector CAimbot::RunAimScan(CBaseEntity* pTarget, float &simtime, Vector& origin)
{
	Vector vEyePos = csgo::LocalPlayer->GetEyePosition();
	static float minimum_damage = 1.f;
	if (Menu.Ragebot.Mindamage == 0)
		minimum_damage = pTarget->GetHealth();
	else
		minimum_damage = Menu.Ragebot.Mindamage;

	VMatrix BoneMatrix[128];
	auto index = pTarget->GetIndex();
	int shottick[64];
	int mainhitgroup = 0;
	int defhitgroup = 0;
	if (Menu.Ragebot.Hitbox == 1) mainhitgroup = 1;
	else if (Menu.Ragebot.Hitbox == 2) mainhitgroup = 3;
	else mainhitgroup = 0;

	float curdamage = csgo::WeaponData->damage;
	bool player_breaks_lagcomp = pTarget->GetOrigin().LengthSqr() > 4096.f;
	if (!Menu.Ragebot.delayshot || (Menu.Ragebot.delayshot && !player_breaks_lagcomp))
	{
		int* array = reinterpret_cast<int*>(offys.dwOcclusionArray);
		*(int*)((uintptr_t)pTarget + offys.nWriteableBones) = 0;
		*(int*)((uintptr_t)pTarget + offys.bDidCheckForOcclusion) = array[1];

		if (BoneMatrix != nullptr)
			pTarget->SetupBones(BoneMatrix, 128, 0x100, g_pGlobals->curtime);
		simtime = pTarget->GetSimulationTime();

		origin = pTarget->GetOrigin();

		if (Menu.Ragebot.Hitbox > 0) {
			for (auto HitBox : MainHitbox(pTarget, BoneMatrix)) {
				ScaleDamage(mainhitgroup, pTarget, csgo::WeaponData->armor_ratio, curdamage);
				if (g_pEngineTrace->IsVisible(csgo::LocalPlayer, vEyePos, HitBox, pTarget)) {
					if (HitBox != Vector(0, 0, 0))
						return HitBox;
				}
				if (Menu.Ragebot.Autowall && g_Autowall->PenetrateWall(pTarget, HitBox)) {
					if (HitBox != Vector(0, 0, 0) && curdamage >= minimum_damage)
						return HitBox;
				}
			}
		}
		for (auto HitboxID : GetHitboxesToScan(pTarget)) {

			Vector vPoint;
			HandleMinDamage(HitboxID, defhitgroup);
			std::vector<Vector> Points = GameUtils::GetMultiplePointsForHitbox(pTarget, HitboxID, BoneMatrix);
			for (int k = 0; k < Points.size(); k++) {

				vPoint = Points.at(k);
				float damage = 0.f;
				ScaleDamage(defhitgroup, pTarget, csgo::WeaponData->armor_ratio, curdamage);
				if (g_pEngineTrace->IsVisible(csgo::LocalPlayer, vEyePos, vPoint, pTarget, HitboxID)) {
					if (vPoint != Vector(0, 0, 0) && curdamage >= minimum_damage)
						return vPoint;
				}
				if (Menu.Ragebot.Autowall && g_Autowall->PenetrateWall(pTarget, vPoint)) {
					if (vPoint != Vector(0, 0, 0) && curdamage >= minimum_damage)
						return vPoint;
				}
			}
		}
	}
	if (Menu.Ragebot.PositionAdjustment)
	{
		tick_record record;
		for (int j = g_BacktrackHelper->PlayerRecord[pTarget->Index()].records.size() - 1; j >= 0; j--)
		{
			switch (Menu.Ragebot.TickType)
			{
			case 0:
				record = g_BacktrackHelper->PlayerRecord[index].records.at(0);
				break;
			case 1:
				record = g_BacktrackHelper->PlayerRecord[index].records.at(j);
				break;
			}
		}

		if (g_BacktrackHelper->IsTickValid(record))
		{
			simtime = record.m_flSimulationTime;
			if (!Menu.Ragebot.delayshot || (Menu.Ragebot.delayshot && !record.needs_extrapolation))
			{
				for (auto hitbox : GameUtils::GetMultiplePointsForHitbox(pTarget, Menu.Ragebot.Hitbox, record.boneMatrix))
				{
					SetRecords(pTarget, record);

					if (Menu.Ragebot.Autowall && g_Autowall->PenetrateWall(pTarget, hitbox) && hitbox.IsValid())
						return hitbox;

					Restore(pTarget);

					int hitgroup = -1;

					if (g_pEngineTrace->IsVisible(csgo::LocalPlayer, vEyePos, hitbox, pTarget, hitgroup)) {
						float modified_damage = csgo::MainWeapon->GetCSWpnData()->damage * (float)pow(csgo::MainWeapon->GetCSWpnData()->range_modifier, csgo::MainWeapon->GetCSWpnData()->range * 0.002);

						ScaleDamage(hitgroup, pTarget, csgo::MainWeapon->GetCSWpnData()->armor_ratio, modified_damage);

						if (hitbox.IsValid() && modified_damage >= minimum_damage)
							return hitbox;
					}
				}
				for (auto hitbox_id : GetHitboxesToScan(pTarget))
				{
					auto point = GameUtils::GetMultiplePointsForHitbox(pTarget, hitbox_id, record.boneMatrix);

					for (int i = 0; i < point.size(); i++)
					{
						auto hit_vector = point.at(i);

						if (i == 0)
						{
							SetRecords(pTarget, record);

							if (Menu.Ragebot.Autowall && g_Autowall->PenetrateWall(pTarget, hit_vector) && hit_vector.IsValid())
								return hit_vector;

							Restore(pTarget);
						}

						int hitgroup = -1;

						if (g_pEngineTrace->IsVisible(csgo::LocalPlayer, vEyePos, hit_vector, pTarget, hitgroup)) {
							float modified_damage = csgo::MainWeapon->GetCSWpnData()->damage * (float)pow(csgo::MainWeapon->GetCSWpnData()->range_modifier, csgo::MainWeapon->GetCSWpnData()->range * 0.002);

							ScaleDamage(hitgroup, pTarget, csgo::MainWeapon->GetCSWpnData()->armor_ratio, modified_damage);

							if (hit_vector.IsValid() && modified_damage >= minimum_damage)
								return hit_vector;
						}
					}
				}
			}
		}
	}
	return Vector(0, 0, 0);
}
CAimbot* g_Aimbot = new CAimbot;


void CAimbot::DropTarget()
{
	target_index = -1;
	best_distance = 99999.f;
	aimbotted_in_current_tick = false;
	fired_in_that_tick = false;
	current_aim_position = Vector();
	pTarget = nullptr;
}

float get_curtime1() {
	if (!csgo::LocalPlayer)
		return 0;

	int g_tick = 0;
	CUserCmd* g_pLastCmd = nullptr;
	if (!g_pLastCmd || g_pLastCmd->hasbeenpredicted) {
		g_tick = csgo::LocalPlayer->GetTickBase();
	}
	else {
		++g_tick;
	}
	g_pLastCmd = csgo::UserCmd;
	float curtime = g_tick * g_pGlobals->interval_per_tick;
	return curtime;
}

void CAimbot::AutoZeus()
{
	for (int i = 1; i <= g_pGlobals->maxClients; i++)
	{
		CBaseEntity *entity = g_pEntitylist->GetClientEntity(i);
		if (!entity
			|| entity == csgo::LocalPlayer
			|| entity->IsDormant()
			|| !entity->isAlive()
			|| entity->IsProtected()
			|| !entity->IsPlayer()
			|| entity->GetTeamNum() == csgo::LocalPlayer->GetTeamNum())
			continue;

		Vector traceStart, traceEnd;
		QAngle viewAngles;
		g_pEngine->GetViewAngles(viewAngles);
		QAngle viewAnglesRcs = viewAngles + csgo::LocalPlayer->GetPunchAngle() * 2.0f;

		Math::AngleVectors(viewAnglesRcs, &traceEnd);

		traceStart = csgo::LocalPlayer->GetEyePosition();
		traceEnd = traceStart + (traceEnd * 8192.0f);

		Ray_t ray;
		trace_t Trace;
		ray.Init(traceStart, traceEnd);
		CTraceFilter traceFilter;
		traceFilter.pSkip1 = csgo::LocalPlayer;
		g_pEngineTrace->TraceRay(ray, 0x46004003, &traceFilter, &Trace);

		if (!Trace.m_pEnt)
			return;
		if (!Trace.m_pEnt->IsValidTarget())
			return;

		float playerDistance = csgo::LocalPlayer->GetOrigin().DistTo(entity->GetOrigin());
		if (csgo::MainWeapon->NextPrimaryAttack() < get_curtime1()) {
			if (playerDistance <= 180.f)
				csgo::UserCmd->buttons |= IN_ATTACK;
		}
	}
}

void StopMovement(CUserCmd* cmd)
{
	constexpr bool isHexagoneGodlike = true;

	CBaseEntity* pLocalPlayer = csgo::LocalPlayer;

	if (pLocalPlayer->GetMoveType() != MOVETYPE_WALK)
		return;

	Vector hvel = pLocalPlayer->GetVelocity();
	hvel.z = 0;
	float speed = hvel.Length2D();

	if (speed < 1.f) // Will be clipped to zero anyways
	{
		cmd->forwardmove = 0.f;
		cmd->sidemove = 0.f;
		return;
	}

	// Homework: Get these dynamically
	static ConVar* acceleratevar = g_pCvar->FindVar("sv_accelerate");
	static ConVar* speedvar = g_pCvar->FindVar("sv_maxspeed");
	float accel = acceleratevar->GetFloat();
	float maxSpeed = speedvar->GetFloat();
	float playerSurfaceFriction = 1.0f; // I'm a slimy boi
	float max_accelspeed = accel * g_pGlobals->interval_per_tick * maxSpeed * playerSurfaceFriction;

	float wishspeed{};

	// Only do custom deceleration if it won't end at zero when applying max_accel
	// Gamemovement truncates speed < 1 to 0
	if (speed - max_accelspeed <= -1.f)
		wishspeed = max_accelspeed / (speed / (accel * g_pGlobals->interval_per_tick));
	else // Full deceleration, since it won't overshoot
		wishspeed = max_accelspeed;

	// Calculate the negative movement of our velocity, relative to our viewangles
	Vector ndir = (hvel * -1.f).directionvector();
	ndir.y = cmd->viewangles.y - ndir.y; // Relative to local view
	ndir = ndir.Forward(); // Back to vector, y'all

	cmd->forwardmove = ndir.x * wishspeed;
	cmd->sidemove = ndir.y * wishspeed;
}

enum speed
{
	CAK47 = 215,
	CWeaponSSG08 = 230,
	CWeaponAWP_Scoped = 100,
	CWeaponG3SG1_Scoped = 120,
	CWeaponGlock = 240,
	CWeaponElite = 240,
	CDEagle = 230,
	CWeaponSCAR20_Scoped = 120,
	CWeaponHKP2000 = 240
};

bool isstrafing()
{
	if (csgo::UserCmd->buttons & IN_LEFT && csgo::UserCmd->buttons & IN_FORWARD ||
		csgo::UserCmd->buttons & IN_RIGHT && csgo::UserCmd->buttons & IN_FORWARD ||
		csgo::UserCmd->buttons & IN_LEFT && csgo::UserCmd->buttons & IN_BACK ||
		csgo::UserCmd->buttons & IN_RIGHT && csgo::UserCmd->buttons & IN_BACK)
		return true;
	else
		return false;
}

void SetSpeed(int speed)
{
	auto sidespeed = g_pCvar->FindVar("cl_sidespeed");
	auto forwardspeed = g_pCvar->FindVar("cl_forwardspeed");
	auto backspeed = g_pCvar->FindVar("cl_backspeed");
	if (sidespeed->GetInt() == 450 && speed == 450) return;
	sidespeed->SetValue(speed);
	forwardspeed->SetValue(speed);
	backspeed->SetValue(speed);
}

void CAimbot::minwalk(CBaseCombatWeapon* weapon)
{
	csgo::Minwalk = true;
	if (*weapon->ItemDefinitionIndex() == AK47)
	{
		if (isstrafing)
			SetSpeed(51.615);
		else
		{
			float x = speed::CAK47 / 100;
			float y = x * 30;
			SetSpeed(y);
		}
	}
	else if (*weapon->ItemDefinitionIndex() == SSG08)
	{
		if (isstrafing)
			SetSpeed(55.295);
		else
		{
			float x = speed::CWeaponSSG08 / 100;
			float y = x * 30;
			SetSpeed(y);
		}
	}
	else if (*weapon->ItemDefinitionIndex() == AWP && csgo::LocalPlayer->IsScoped())
	{
		if (isstrafing)
			SetSpeed(24.04);
		else
		{
			float x = speed::CWeaponAWP_Scoped / 100;
			float y = x * 30;
			SetSpeed(y);
		}
	}
	else if (*weapon->ItemDefinitionIndex() == G3SG1 && csgo::LocalPlayer->IsScoped())
	{
		if (isstrafing)
			SetSpeed(28.85);
		else
		{
			float x = speed::CWeaponG3SG1_Scoped / 100;
			float y = x * 30;
			SetSpeed(y);
		}
	}
	else if (*weapon->ItemDefinitionIndex() == GLOCK)
	{
		if (isstrafing)
			SetSpeed(57.7);
		else
		{
			float x = speed::CWeaponGlock / 100;
			float y = x * 30;
			SetSpeed(y);
		}
	}
	else if (*weapon->ItemDefinitionIndex() == ELITE)
	{
		if (isstrafing)
			SetSpeed(57.7);
		else
		{
			float x = speed::CWeaponElite / 100;
			float y = x * 30;
			SetSpeed(y);
		}
	}
	else if (*weapon->ItemDefinitionIndex() == DEAGLE)
	{
		if (isstrafing)
			SetSpeed(55.295);
		else
		{
			float x = speed::CDEagle / 100;
			float y = x * 30;
			SetSpeed(y);
		}
	}
	else if (*weapon->ItemDefinitionIndex() == SCAR20 && csgo::LocalPlayer->IsScoped())
	{
		if (isstrafing)
			SetSpeed(28.85);
		else
		{
			float x = speed::CWeaponSCAR20_Scoped / 100;
			float y = x * 30;
			SetSpeed(y);
		}
	}
	else if (*weapon->ItemDefinitionIndex() == HKP2000 || *weapon->ItemDefinitionIndex() == USP_SILENCER)
	{
		if (isstrafing)
			SetSpeed(57.7);
		else
		{
			float x = speed::CWeaponHKP2000 / 100;
			float y = x * 30;
			SetSpeed(y);
		}
	}
	else
	{
		float speed = 250 * 15 / 100;
		SetSpeed(speed);
	}
}

bool ShouldBaim(CBaseEntity* target, CBaseCombatWeapon* Weapon)
{
	if (g_Aimbot->data[target->GetIndex()].shotsmissed >= Menu.Ragebot.baimafterx && Menu.Ragebot.baimafterx > 0)
		return true;
	if (Menu.Ragebot.baimifair && !(*target->GetFlags() & FL_ONGROUND))
		return true;
	if ((Menu.Ragebot.baimiflethal && target->GetHealth() <= Menu.Ragebot.Mindamage))
		return true;
	if (GetAsyncKeyState(Menu.Ragebot.ForcePelvis))
		return true;
	if (target->GetVelocity().Length2D() >= 0.1f && target->GetVelocity().Length2D() <= 125 && Menu.Ragebot.baimifslow)
		return true;
	if (target->GetVelocity().Length2D() >= 0.1f && Menu.Ragebot.baimifmove)
		return true;
	return false;
}

void CAimbot::Run()
{
	if (!Menu.Ragebot.EnableAimbot)
		return;

	CBaseCombatWeapon* pWeapon = csgo::LocalPlayer->GetWeapon();

	if (!pWeapon || pWeapon->Clip1() == 0 || pWeapon->IsMiscWeapon() || !GameUtils::IsAbleToShoot())
		return;

	QAngle view; g_pEngine->GetViewAngles(view);

	if (Menu.Ragebot.AutomaticRevolver)
	{
		if (csgo::MainWeapon->WeaponID() == ItemDefinitionIndex::REVOLVER)
		{
			csgo::UserCmd->buttons |= IN_ATTACK;
			float flPostponeFireReady = csgo::MainWeapon->GetPostponeFireReadyTime();
			if (flPostponeFireReady > 0 && flPostponeFireReady < get_curtime1())
			{
				csgo::UserCmd->buttons &= ~IN_ATTACK;
				if (Menu.Ragebot.NewAutomaticRevolver && flPostponeFireReady + g_pGlobals->interval_per_tick * Menu.Ragebot.NewAutomaticRevolverFactor > get_curtime1())
					csgo::UserCmd->buttons |= IN_ATTACK2;
			}
		}
	}

	for (int i = 1; i < 64; ++i)
	{
		if (i == g_pEngine->GetLocalPlayer())
			continue;
		CBaseEntity* target = g_pEntitylist->GetClientEntity(i);
		if (!target->IsValidTarget())
			continue;

		csgo::Target = target;

		float fov = GameUtils::GetFoV(view, csgo::LocalPlayer->GetEyePosition(), target->GetEyePosition(), false);
		if (fov > Menu.Ragebot.Fov) //FOV
			continue;

		if (Menu.Ragebot.AutomaticStop != 2) {
			float m_flNextPrimaryAttack = csgo::LocalPlayer->NextPrimaryAttack();
			int m_nTickBase = csgo::LocalPlayer->GetTickBase();
			bool can_shoot = (m_flNextPrimaryAttack <= m_nTickBase * g_pGlobals->interval_per_tick);
			if (!can_shoot)
				continue;
		}

		float simtime = 0;
		Vector minus_origin = Vector(0, 0, 0);
		Vector aim_position = RunAimScan(target, simtime, minus_origin);
		if (aim_position == Vector(0, 0, 0))
			continue;

		float selection_value = 0;
		switch (Menu.Ragebot.AimbotSelection) {
		case 0:
			selection_value = fov; //FOV
			break;
		case 2:
			selection_value = Math::RandomFloat(0, 100); //Cycle
			break;
		case 3:
			selection_value = target->GetVelocity().Length(); //Velocity
			break;
		case 4:
			selection_value = target->GetHealth(); //Health
			break;

		default:
			break;
		}

		if (best_distance >= selection_value && aim_position != Vector(0, 0, 0))
		{
			best_distance = selection_value; //Distance
			target_index = i;
			current_aim_position = aim_position;
			pTarget = target;
			current_aim_simulationtime = simtime;
			current_aim_player_origin = minus_origin;
		}
		if (ShouldBaim(target, csgo::MainWeapon)) Menu.Ragebot.Hitbox = 2; else Menu.Ragebot.Hitbox = Menu.Ragebot.Fakehitbox;	
	}
	if (target_index != -1 && current_aim_position != Vector(0, 0, 0) && pTarget)
	{
		aimbotted_in_current_tick = true;

		QAngle aim = GameUtils::CalculateAngle(csgo::LocalPlayer->GetEyePosition(), current_aim_position);

		aim.y = Math::NormalizeYaw(aim.y);

		csgo::UserCmd->viewangles = aim;
		csgo::RealAngle = aim;

		if (!Menu.Ragebot.SilentAimbot)
			g_pEngine->SetViewAngles(csgo::UserCmd->viewangles);

		if (Menu.Ragebot.AutomaticFire)
		{
			if (Menu.Ragebot.AutomaticScope && pWeapon->IsScopeable() && !csgo::LocalPlayer->IsScoped())
				csgo::UserCmd->buttons |= IN_ATTACK2;
			int sequence = csgo::LocalPlayer->GetSequence();
			if (*csgo::LocalPlayer->GetFlags() & FL_ONGROUND && csgo::MainWeapon->WeaponID() != ItemDefinitionIndex::TASER && !csgo::MainWeapon->IsMiscWeapon() && sequence != Activity::ACT_RELOAD && csgo::UserCmd->weaponselect == 0)
			{
				if (csgo::LocalPlayer->GetVelocity().Length() > 0) {
					switch (Menu.Ragebot.AutomaticStop) {
					case 0: break;
					case 1: StopMovement(csgo::UserCmd); break;
					case 2: minwalk(csgo::MainWeapon); break;
					}
				}
			}
			else
				g_Antiaim->SetSpeed(450);
			if (Menu.Ragebot.Minhitchance == 0 || hitchance(current_aim_position, pTarget))
			{
				csgo::Minwalk = false;
				this->fired_in_that_tick = true;
				csgo::UserCmd->buttons |= IN_ATTACK;
			}
		}
		if (csgo::UserCmd->buttons & IN_ATTACK)
		{
			this->aimbotted = true;
		}
		csgo::UserCmd->tick_count = TIME_TO_TICKS(current_aim_simulationtime + g_BacktrackHelper->GetLerpTime());// for backtracking
	}
}

void CAimbot::CompensateInaccuracies()
{
	if (csgo::UserCmd->buttons & IN_ATTACK)
	{
		if (Menu.Ragebot.NoRecoil && Menu.Ragebot.EnableAimbot)
		{
			ConVar* recoilscale = g_pCvar->FindVar("weapon_recoil_scale");

			if (recoilscale) {
				QAngle qPunchAngles = csgo::LocalPlayer->GetPunchAngle();
				QAngle qAimAngles = csgo::UserCmd->viewangles;
				qAimAngles -= qPunchAngles * recoilscale->GetFloat();
				csgo::UserCmd->viewangles = qAimAngles;
			}
		}

		if (Menu.Ragebot.NoSpread)
			csgo::UserCmd->viewangles = g_NoSpread->SpreadFactor(csgo::UserCmd->random_seed);
	}
}
