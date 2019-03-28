#include "sdk.h"
#include "Antiaim.h"
#include "global.h"
#include "GameUtils.h"
#include "Math.h"
#include "Aimbot.h"
#include <time.h>
#include "Misc.h"

CAntiaim* g_Antiaim = new CAntiaim;

bool CAntiaim::is_viable_target(CBaseEntity* pEntity)
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

void CAntiaim::SetSpeed(int speed)
{
	auto sidespeed = g_pCvar->FindVar("cl_sidespeed");
	auto forwardspeed = g_pCvar->FindVar("cl_forwardspeed");
	auto backspeed = g_pCvar->FindVar("cl_backspeed");
	if (sidespeed->GetInt() == 450 && speed == 450) return;
	sidespeed->SetValue(speed);
	forwardspeed->SetValue(speed);
	backspeed->SetValue(speed);
}

Vector AtTargets(Vector localview) {
	for (int i = 1; i < 64; ++i)
	{
		if (i == g_pEngine->GetLocalPlayer())
			continue;
		CBaseEntity* target = g_pEntitylist->GetClientEntity(i);
		if (!g_Antiaim->is_viable_target(target))
			continue;
		Vector atenemy = GameUtils::CalculateAngle(csgo::LocalPlayer->GetOrigin(), target->GetOrigin());
		if (atenemy == Vector(0, 0, 0))
			continue;
		return atenemy;
	}
	return localview;
}

float fov_player(Vector view_offset, Vector view, CBaseEntity* m_entity, int hitbox)
{
	CONST FLOAT MaxDegrees = 180.0f;
	Vector Angles = view;
	Vector Origin = view_offset;
	Vector Delta(0, 0, 0);
	Vector Forward(0, 0, 0);
	Math::AngleVectors(Angles, Forward);
	Vector AimPos = GameUtils::get_hitbox_location(m_entity, hitbox);
	VectorSubtract(AimPos, Origin, Delta);
	Math::normalize(Delta, Delta);
	FLOAT DotProduct = Forward.Dot(Delta);
	return (acos(DotProduct) * (MaxDegrees / PI));
}


int closest_to_crosshair()
{
	int index = -1;
	float lowest_fov = INT_MAX;

	CBaseEntity* local_player = csgo::LocalPlayer;

	if (!local_player)
		return -1;

	Vector local_position = local_player->GetOrigin() + local_player->GetViewOffset();

	Vector angles;
	g_pEngine->GetViewAngles(angles);

	for (int i = 1; i <= g_pGlobals->maxClients; i++)
	{
		CBaseEntity *entity = g_pEntitylist->GetClientEntity(i);

		if (!entity || entity->GetHealth() <= 0 || entity->GetTeamNum() == local_player->GetTeamNum() || entity->IsDormant() || entity == local_player)
			continue;

		float fov = fov_player(local_position, angles, entity, 0);

		if (fov < lowest_fov)
		{
			lowest_fov = fov;
			index = i;
		}
	}

	return index;
}

float freeStand()
{
	static float last_autodirect_angle;
	if (csgo::LocalPlayer->GetVelocity().Length() < 300) {
		if (!csgo::SendPacket)
		{
			auto fov_to_player = [](Vector view_offset, Vector view, CBaseEntity* m_entity, int hitbox)
			{
				CONST FLOAT MaxDegrees = 180.0f;
				Vector Angles = view;
				Vector Origin = view_offset;
				Vector Delta(0, 0, 0);
				Vector Forward(0, 0, 0);
				Math::AngleVectors(Angles, Forward);
				Vector AimPos = GameUtils::get_hitbox_location(m_entity, hitbox);
				VectorSubtract(AimPos, Origin, Delta);
				Math::normalize(Delta, Delta);
				FLOAT DotProduct = Forward.Dot(Delta);
				return (acos(DotProduct) * (MaxDegrees / PI));
			};

			int target = -1;
			float mfov = 50;

			Vector viewoffset = csgo::LocalPlayer->GetOrigin() + csgo::LocalPlayer->GetViewOffset();
			Vector view; g_pEngine->GetViewAngles(view);

			for (int i = 0; i < g_pGlobals->maxClients; i++) {
				CBaseEntity* m_entity = g_pEntitylist->GetClientEntity(i);

				if (g_Antiaim->is_viable_target(m_entity)) {

					float fov = fov_to_player(viewoffset, view, m_entity, 0);
					if (fov < mfov) {
						mfov = fov;
						target = i;
					}
				}
			}

			Vector at_target_angle;

			if (target)
			{
				auto m_entity = g_pEntitylist->GetClientEntity(target);

				bool no_active = true;
				float bestrotation = 0.f;
				float highestthickness = 0.f;
				Vector besthead;
				float range = 10.f;

				auto leyepos = csgo::LocalPlayer->GetOrigin() + csgo::LocalPlayer->GetViewOffset();
				auto headpos = GameUtils::get_hitbox_location(csgo::LocalPlayer, 0);
				auto origin = csgo::LocalPlayer->GetAbsOrigin();

				if (!csgo::SendPacket)
				{
					auto checkWallThickness = [&](CBaseEntity* pPlayer, Vector newhead) -> float
					{
						Vector endpos1, endpos2;
						Vector eyepos = pPlayer->GetOrigin() + pPlayer->GetViewOffset();

						Ray_t ray;
						ray.Init(newhead, eyepos);

						CTraceFilterSkipTwoEntities filter(pPlayer, csgo::LocalPlayer);

						trace_t trace1, trace2;
						g_pEngineTrace->TraceRay_NEW(ray, MASK_SHOT_BRUSHONLY, &filter, &trace1);

						if (trace1.DidHit())
							endpos1 = trace1.endpos;
						else
							return 0.f;

						ray.Init(eyepos, newhead);
						g_pEngineTrace->TraceRay_NEW(ray, MASK_SHOT_BRUSHONLY, &filter, &trace2);

						if (trace2.DidHit())
							endpos2 = trace2.endpos;

						float add = newhead.DistTo(eyepos) - leyepos.DistTo(eyepos) + 3.f;
						return endpos1.DistTo(endpos2) + add / 3;
					};

					int index = closest_to_crosshair();
					auto entity = g_pEntitylist->GetClientEntity(index);

					float step = (2 * M_PI) / 18.f;
					float radius = fabs(Vector(headpos - origin).Length2D());



					if (index == -1)
					{
						no_active = true;
					}
					else
					{
						for (float rotation = 0; rotation < (M_PI * 2.0); rotation += step)
						{
							Vector newhead(radius * cos(rotation) + leyepos.x, radius * sin(rotation) + leyepos.y, leyepos.z);

							float totalthickness = 0.f;

							no_active = false;

							totalthickness += checkWallThickness(entity, newhead);

							if (totalthickness > highestthickness)
							{
								highestthickness = totalthickness;
								bestrotation = rotation;
								besthead = newhead;
							}
						}
					}

					if (no_active)
					{
						return 0;
					}
					else
					{
						return (RAD2DEG(bestrotation));
					}
				}
			}
		}
	}
}

float fl_freestanding(Vector at_target_angle) {
	static float last_autodirect_angle;
	if (csgo::LocalPlayer->GetVelocity().Length() < 300) {
		if (!csgo::SendPacket)
		{
			auto fov_to_player = [](Vector view_offset, Vector view, CBaseEntity* m_entity, int hitbox)
			{
				CONST FLOAT MaxDegrees = 180.0f;
				Vector Angles = view;
				Vector Origin = view_offset;
				Vector Delta(0, 0, 0);
				Vector Forward(0, 0, 0);
				Math::AngleVectors(Angles, Forward);
				Vector AimPos = GameUtils::get_hitbox_location(m_entity, hitbox);
				VectorSubtract(AimPos, Origin, Delta);
				Math::normalize(Delta, Delta);
				FLOAT DotProduct = Forward.Dot(Delta);
				return (acos(DotProduct) * (MaxDegrees / PI));
			};

			int target = -1;
			float mfov = 50;

			Vector viewoffset = csgo::LocalPlayer->GetOrigin() + csgo::LocalPlayer->GetViewOffset();
			Vector view; g_pEngine->GetViewAngles(view);

			for (int i = 0; i < g_pGlobals->maxClients; i++) {
				CBaseEntity* m_entity = g_pEntitylist->GetClientEntity(i);

				if (g_Antiaim->is_viable_target(m_entity)) {

					float fov = fov_to_player(viewoffset, view, m_entity, 0);
					if (fov < mfov) {
						mfov = fov;
						target = i;
					}
				}
			}

			if (target)
			{
				auto m_entity = g_pEntitylist->GetClientEntity(target);
				if (g_Antiaim->is_viable_target(m_entity))
				{
					Vector head_pos_screen;
					if (GameUtils::WorldToScreen(m_entity->GetHedPos(), head_pos_screen))
					{
						at_target_angle.x = 0;

						Vector src3D, dst3D, forward, right, up, src, dst;
						float back_two, right_two, left_two;

						trace_t tr;
						Ray_t ray, ray2, ray3, ray4, ray5;
						CTraceFilter filter;

						const Vector to_convert = at_target_angle;
						Math::AngleVectors(to_convert, &forward, &right, &up);

						filter.pSkip1 = csgo::LocalPlayer;
						src3D = csgo::LocalPlayer->GetEyePosition();
						dst3D = src3D + (forward * 384); //Might want to experiment with other numbers, incase you don't know what the number does, its how far the trace will go. Lower = shorter.

						ray.Init(src3D, dst3D);
						g_pEngineTrace->TraceRay_NEW(ray, MASK_SHOT, &filter, &tr);
						back_two = (tr.endpos - tr.startpos).Length();

						ray2.Init(src3D + right * 35, dst3D + right * 35);
						g_pEngineTrace->TraceRay_NEW(ray2, MASK_SHOT, &filter, &tr);
						right_two = (tr.endpos - tr.startpos).Length();

						ray3.Init(src3D - right * 35, dst3D - right * 35);
						g_pEngineTrace->TraceRay_NEW(ray3, MASK_SHOT, &filter, &tr);
						left_two = (tr.endpos - tr.startpos).Length();
						static int timer;
						static int timer1;
						if (left_two > right_two)
						{
							timer1 = 0;

							timer++;
							if (timer >= 15)
							{
								return 90;
								last_autodirect_angle = 90;
							}
						}
						else if (right_two > left_two)
						{
							timer = 0;
							timer1++;
							if (timer1 >= 15)
							{
								return -90;
								last_autodirect_angle = -90;
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

float ManualAA(float manualadd) {
	static bool check = false;

	if (GetAsyncKeyState(Menu.Antiaim.switchkey))
	{
		if (!check)
			csgo::isleft = !csgo::isleft;
		check = true;
	}
	else
		check = false;

	if (csgo::isleft)
		return (manualadd * -1);
	else
		return (manualadd);
}

/*void CAntiaim::lbybreaker(CUserCmd* pCmd, CBaseEntity* entity)
{
	static float lastLowerBodyYawTarget = 0.0f;
	static bool flip = false;
	static float nextLowerBodyYawTargetUpdate = 0;
	static float lastLowerBodyYawTargetUpdate = 0;
	static int lastLowerbodyChangedCount = 0;
	static int lastLowerBodyUpdate = 0;
	auto& globals = global[entity->GetIndex()];
	int changedTimes = globals.lowerbodyChangedCount - lastLowerbodyChangedCount;

	bool lowerbodyChanged = changedTimes > 0;
	bool lowerbodyChangedPredict = g_pGlobals->curtime > nextLowerBodyYawTargetUpdate;

	lastLowerbodyChangedCount = globals.lowerbodyChangedCount;

	bool moving = fabsf(csgo::LocalPlayer->GetVelocity().Length2D()) > 0.1f;
	bool inGround = fabsf(csgo::LocalPlayer->GetVelocity().z) == 0.0f; //(pLocal->GetFlags() & FL_ONGROUND);

	globals.LBYUpdate = (globals.LBYUpdate
		|| (g_pGlobals->curtime - lastLowerBodyUpdate) >= 1.1f + g_pEngine->GetNetChannelInfo()->GetAvgLatency(FLOW_OUTGOING)
		/*|| (moving || inGround)*//*); // looks for next update, cant tell if it's realiable though

	if (globals.LBYUpdate)
	{
		lastLowerBodyUpdate = g_pGlobals->curtime;
		flip = flip + 1 == 2 ? 0 : flip + 1;
	}

	if (!csgo::SendPacket) {

		if (globals.LBYUpdate && csgo::LocalPlayer->LowerBodyYaw() /*this fixes a crash i had*//*) {
			FakeYawOverride(pCmd, csgo::SendPacket); // for this tick set your real angle to your fake angle
		}
		//lowerbodyChanged = false; /* DEBUG *//*
		if (lowerbodyChanged || lowerbodyChangedPredict)
		{
			static float error = 0;

			//error = 0; /* DEBUG 

			auto net = g_pEngine->GetNetChannelInfo();
			float latency = net->GetAvgLatency(FLOW_OUTGOING);

			if (lowerbodyChanged && !lowerbodyChangedPredict)
			{
				error = nextLowerBodyYawTargetUpdate - g_pGlobals->curtime;
			}

			if (lowerbodyChangedPredict)
			{
				nextLowerBodyYawTargetUpdate = g_pGlobals->curtime + 1.1f + latency - error;
			}

			//printf("%d %f %f\n", changedTimes, latency, error);

			lastLowerBodyYawTargetUpdate = g_pGlobals->curtime;
		}


		lastLowerBodyYawTarget = csgo::LocalPlayer->LowerBodyYaw();
	}
	else {
		if (globals.LBYUpdate)
			pCmd->viewangles.y += 180.f; //when your real angle is set to your fake angle, flip the fake angle
	}
}*/
float get_curtime() 
{
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

bool shouldbreaklby()
{
	static float next_lby_update_time = 0;
	const float current_time = get_curtime();
	static bool stop = csgo::LocalPlayer->GetVelocity().Length2D() < 0.1 && *csgo::LocalPlayer->GetFlags() & FL_ONGROUND;
	if (csgo::LocalPlayer->GetVelocity().Length2D() > 0.1 && *csgo::LocalPlayer->GetFlags() & FL_ONGROUND) {
		next_lby_update_time = current_time + 0.22f;
	}
	auto net_channel = g_pEngine->GetNetChannel();
	if (stop) {
		if ((next_lby_update_time < current_time && !net_channel->m_nChokedPackets)) {
			next_lby_update_time = current_time + 1.1f;
			return true;
		}
	}
}

void CAntiaim::FakeDuck(CUserCmd* cmd)
{
	bool do_once = false;
	if (!GetAsyncKeyState(Menu.Antiaim.FakeDuckKey) && do_once) {
		cmd->buttons &= ~IN_DUCK;
		do_once = false;
		return;
	}
	if (GetAsyncKeyState(Menu.Antiaim.FakeDuckKey)) {
		do_once = true;
		if (csgo::SendPacket)
			cmd->buttons |= IN_DUCK;
		else
			cmd->buttons &= ~IN_DUCK;
	}
}

void CAntiaim::DoPitch(int choose)
{
	switch (choose)
	{
	case 1:
		csgo::UserCmd->viewangles.x = 89;
		break;
	case 2:
		csgo::UserCmd->viewangles.x = -89;
		break;
	case 3:
		csgo::UserCmd->viewangles.x = 540;
		break;
	case 4:
		csgo::UserCmd->viewangles.x = -540;
		break;
	case 5:
		csgo::UserCmd->viewangles.x = 0;
		break;
	case 6:
		csgo::UserCmd->viewangles.x = 1080;
		break;
	}
}

void DoYawStill(CUserCmd* cmd, int playerstate) {
	Vector yawbase = Vector(0, 0, 0);
	if (Menu.Antiaim.aatype == 0) yawbase = cmd->viewangles;
	else if (Menu.Antiaim.aatype == 1) yawbase = AtTargets(cmd->viewangles);
	int choked_commands = g_pEngine->GetNetChannel()->m_nChokedPackets;
	float aayaw = 0;

	if (Menu.Antiaim.DirectonType == 0)
		aayaw = Menu.Antiaim.Stand.realyaw;
	else if (Menu.Antiaim.DirectonType == 1)
		aayaw = ManualAA(Menu.Antiaim.Stand.switchangle);
	else if (Menu.Antiaim.DirectonType == 2)
		aayaw = fl_freestanding(yawbase);
	else if (Menu.Antiaim.DirectonType == 3)
		aayaw = freeStand();

	if (choked_commands == 0)
	{
		if (cmd->viewangles.y > 180 && cmd->viewangles.y < 360) // desync yaw backwards
			cmd->viewangles.y = Math::NormalizeYaw(yawbase.y + 140 + aayaw);
		else
			cmd->viewangles.y = Math::NormalizeYaw(yawbase.y + 160 + aayaw);
	}
	else if (choked_commands == 1) {
		if (cmd->viewangles.y > 180 && cmd->viewangles.y < 360) // desync yaw backwards
			cmd->viewangles.y = Math::NormalizeYaw(yawbase.y + 140 + aayaw);
		else
			cmd->viewangles.y = Math::NormalizeYaw(yawbase.y + 160 + aayaw);
		csgo::fake_strength = csgo::LocalPlayer->poseparameterptr()[11] * 58; //for desync indicator
	}
	else
		cmd->viewangles.y = Math::NormalizeYaw(yawbase.y + 180 + aayaw);
}

void DoYawTest(CUserCmd* cmd, int playerstate) {
	Vector yawbase = Vector(0, 0, 0);
	if (Menu.Antiaim.aatype == 0) yawbase = cmd->viewangles;
	else if (Menu.Antiaim.aatype == 1) yawbase = AtTargets(cmd->viewangles);
	float aayaw = 0;

	if (Menu.Antiaim.DirectonType == 0)
		aayaw = Menu.Antiaim.Stand.realyaw;
	else if (Menu.Antiaim.DirectonType == 1)
		aayaw = ManualAA(Menu.Antiaim.Stand.switchangle);
	else if (Menu.Antiaim.DirectonType == 2)
		aayaw = fl_freestanding(yawbase);
	else if (Menu.Antiaim.DirectonType == 3)
		aayaw = freeStand();

	if (csgo::SendPacket) {
		if (cmd->viewangles.y > 180 && cmd->viewangles.y < 360) // desync yaw backwards
			cmd->viewangles.y = Math::NormalizeYaw(yawbase.y + 140 + aayaw);
		else
			cmd->viewangles.y = Math::NormalizeYaw(yawbase.y + 220 + aayaw);

		csgo::fake_strength = csgo::LocalPlayer->poseparameterptr()[11] * 58; //for desync indicator
	}
	else
		cmd->viewangles.y = Math::NormalizeYaw(yawbase.y + 180 + aayaw);

}

void CAntiaim::RunBeta(CUserCmd* cmd) {
	static int iChokedPackets = -1;
	if ((g_Aimbot->fired_in_that_tick && iChokedPackets < 4 && GameUtils::IsAbleToShoot()))
	{
		csgo::SendPacket = false;
		iChokedPackets++;
	}
	else
	{
		iChokedPackets = 0;
		CGrenade* pCSGrenade = (CGrenade*)csgo::LocalPlayer->GetWeapon();
		if (csgo::UserCmd->buttons & IN_USE
			|| csgo::LocalPlayer->GetMoveType() == MOVETYPE_LADDER && csgo::LocalPlayer->GetVelocity().Length() > 0
			|| csgo::LocalPlayer->GetMoveType() == MOVETYPE_NOCLIP
			|| *csgo::LocalPlayer->GetFlags() & FL_FROZEN
			|| (pCSGrenade && pCSGrenade->GetThrowTime() > 0.f)) {
			return;
		}

		choke = !choke;
		if (Menu.Misc.FakelagMode == 0)
			csgo::SendPacket = choke;

		if (!csgo::SendPacket)
			csgo::nChokedTicks++;
		else
			csgo::nChokedTicks = 0;

		if (*csgo::LocalPlayer->GetFlags() & FL_ONGROUND)
		{
			if (csgo::LocalPlayer->GetVelocity().Length2D() < 32 && (!GetAsyncKeyState(Menu.Misc.Slowwalkkey))) {
				DoPitch(Menu.Antiaim.Stand.pitch);
				Menu.Antiaim.DirectonType = Menu.Antiaim.Stand.DirectionType;
				if (Menu.Antiaim.Stand.betamode == 0) {
					csgo::desyncstill = true;
					DoYawStill(cmd, 0);
				}
				else if (Menu.Antiaim.Stand.betamode == 1) {
					csgo::desyncstill = false;
					DoYawTest(cmd, 0);
				}
			}
			else if (csgo::LocalPlayer->GetVelocity().Length2D() > 32 && (!GetAsyncKeyState(Menu.Misc.Slowwalkkey))) {
				DoPitch(Menu.Antiaim.Move.pitch);
				Menu.Antiaim.DirectonType = Menu.Antiaim.Move.DirectionType;
				if (Menu.Antiaim.Move.betamode == 0) {
					csgo::desyncstill = true;
					DoYawStill(cmd, 0);
				}
				else if (Menu.Antiaim.Move.betamode == 1) {
					csgo::desyncstill = false;
					DoYawTest(cmd, 0);
				}
			}
			else if (GetAsyncKeyState(Menu.Misc.Slowwalkkey)) {
				DoPitch(Menu.Antiaim.Slow.pitch);
				Menu.Antiaim.DirectonType = Menu.Antiaim.Slow.DirectionType;
				if (Menu.Antiaim.Slow.betamode == 0) {
					csgo::desyncstill = true;
					DoYawStill(cmd, 0);
				}
				else if (Menu.Antiaim.Slow.betamode == 1) {
					csgo::desyncstill = false;
					DoYawTest(cmd, 0);
				}
			}
		}
		else {
			DoPitch(Menu.Antiaim.Air.pitch);
			Menu.Antiaim.DirectonType = Menu.Antiaim.Air.DirectionType;
			if (Menu.Antiaim.Air.betamode == 0) {
				csgo::desyncstill = true;
				DoYawStill(cmd, 0);
			}
			else if (Menu.Antiaim.Air.betamode == 1) {
				csgo::desyncstill = false;
				DoYawTest(cmd, 0);
			}
		}
	}
}