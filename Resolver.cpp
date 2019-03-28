#include "includes.h"
#include "Aimbot.h"
#include "Resolver.h"
#include "BacktrackingHelper.h"

bool Resolver::fakewalk(CBaseEntity* entity)
{
	bool fakewalking = false, stage1 = false, stage2 = false, stage3 = false;

	for (int i = 0; i < 15; i++) {
		AnimationLayer record = entity->GetAnimOverlays()[i];

		if (record.m_nSequence == 26 && record.m_flWeight < 0.4f)
			stage1 = true;
		if (record.m_nSequence == 7 && record.m_flWeight > 0.001f)
			stage2 = true;
		if (record.m_nSequence == 2 && record.m_flWeight == 0)
			stage3 = true;
	}

	if (stage1 && stage2)
		if (stage3 || (entity->GetFlags2() & FL_DUCKING))
			fakewalking = true;
		else
			fakewalking = false;
	else
		fakewalking = false;

	return entity->GetVelocity().Length2D() > 0.1 && entity->GetVelocity().Length2D() < 75;
}

void Resolver::DefaultResolver(CBaseEntity* entity) // made by corgi
{
	float server_time = csgo::LocalPlayer->GetTickBase() * g_pGlobals->interval_per_tick;
	float time = TIME_TO_TICKS(server_time);
	float random = rand() % 100;
	float* stored_pose_parameter[64];
	CBasePlayerAnimState* animation_state = entity->AnimState();
	float* pose_parameters = (float*)((uintptr_t)entity + 0x2774);
	auto& record = pResolverData[entity->GetIndex()];
	g_Aimbot->data[entity->GetIndex()].shotsmissed = g_Aimbot->data[entity->GetIndex()].shoots - g_Aimbot->data[entity->GetIndex()].shoots_hit;

	if (!animation_state || !entity->GetAnimOverlays() || !pose_parameters)
		return;

	while (time >= server_time)
		time = 0.f;

	CBaseCombatWeapon* pWeapon = entity->GetWeapon();
	if (entity->GetSimulationTime() != record.old_simtime) //Basic Desync Checking
	{
		if (entity->GetSimulationTime() - record.old_simtime != g_pGlobals->interval_per_tick) //Desync check
			record.IsDesyncing = true;
		else
			record.IsDesyncing = false;

		record.old_simtime = entity->GetSimulationTime();
	}
	record.oldeyeangles = entity->GetEyeAngles();

	/*Resolver override*/
	if (GetAsyncKeyState(Menu.Ragebot.ResolverOverride)) //Checks if the key is pressed
	{
		Vector pos_enemy; //Vectors
		Vector local_target_angle;//Vectors
		if (GameUtils::WorldToScreen(entity->GetOrigin(), pos_enemy))
		{
			GameUtils::calculate_angle(csgo::LocalPlayer->GetOrigin(), entity->GetOrigin(), local_target_angle); //Calculate position
			ImVec2 ImGui::GetMousePos(); //Get mouse position
			float delta = ImGui::GetMousePos().x - pos_enemy.x; //Get delta
			if (delta < 0) animation_state->m_flGoalFeetYaw = entity->GetEyeAnglesPtr()->y + 60; //Resolve if delta
			else animation_state->m_flGoalFeetYaw = entity->GetEyeAnglesPtr()->y - 60; //Resolve if delta
		}
	}
	else if (record.IsDesyncing)
	{
		memcpy(&stored_pose_parameter[entity->GetIndex()], pose_parameters, sizeof(float) * 24);
		float resimulation = Math::NormalizeYaw((0.24f / (entity->GetSimulationTime() - record.old_simtime)) * (record.oldeyeangles.y - entity->GetEyeAngles().y));
		if (resimulation > 60.f)
			resimulation = 60.f;
		if (resimulation < -60.f)
			resimulation = -60.f;

		if (entity->GetVelocity().Length2D() >= 0.1f && !record.shot && !record.fakewalking)
		{
			float bruteforce = ((g_Aimbot->data[entity->GetIndex()].shotsmissed % 2) == 0) ? 0.f : Math::NormalizeYaw(Math::CalcAngle(Vector(0, 0, 0), entity->GetVelocity()).y);

			float delta = Math::NormalizeYaw(bruteforce - Math::NormalizeYaw(Math::NormalizeYaw(animation_state->m_flGoalFeetYaw + Math::remap_value(pose_parameters[11], 0, 1, -60, 60)) + resimulation));

			if (delta < 0)
				resimulation += (60.f - resimulation);
			else if (delta > 0)
				resimulation += (-60.f - resimulation);
		}
		else if (entity->GetVelocity().Length2D() >= 0.1f && !record.shot && record.fakewalking)
		{
			float bruteforce = ((g_Aimbot->data[entity->GetIndex()].shotsmissed % 2) == 0) ? 0.f : 180.f;

			float delta = Math::NormalizeYaw(Math::NormalizeYaw(entity->LowerBodyYaw() + bruteforce) - Math::NormalizeYaw(Math::NormalizeYaw(animation_state->m_flGoalFeetYaw + Math::remap_value(entity->arr_flPoseParameter(), 0, 1, -60, 60)) + resimulation));

			if (delta > 60.f)
				delta = 60.f;
			if (delta < -60.f)
				delta = -60.f;

			resimulation += delta;

			if (resimulation > 60.f)
				resimulation = 60.f;
			if (resimulation < -60.f)
				resimulation = -60.f;
		}
		else if (!record.shot && entity->GetVelocity().Length2D() <= 0.1f)
		{
			float bruteforce = ((g_Aimbot->data[entity->GetIndex()].shotsmissed % 2) == 0) ? 0.f : 180.f;

			float delta = Math::NormalizeYaw(Math::NormalizeYaw(entity->LowerBodyYaw() + bruteforce) - Math::NormalizeYaw(Math::NormalizeYaw(animation_state->m_flGoalFeetYaw + Math::remap_value(entity->arr_flPoseParameter(), 0, 1, -60, 60)) + resimulation));

			if (delta > 60.f)
				delta = 60.f;
			if (delta < -60.f)
				delta = -60.f;

			resimulation += delta;

			if (resimulation > 60.f)
				resimulation = 60.f;
			if (resimulation < -60.f)
				resimulation = -60.f;
		}
		float Equalized = Math::NormalizeYaw(Math::NormalizeYaw(animation_state->m_flGoalFeetYaw + Math::remap_value(pose_parameters[11], 0, 1, -60, 60)) + resimulation);

		if (entity != csgo::LocalPlayer && entity->GetTeamNum() != csgo::LocalPlayer->GetTeamNum() && (*entity->GetFlags() & FL_ONGROUND))
			animation_state->m_flGoalFeetYaw = Equalized;

		record.oldeyeangles = entity->GetEyeAngles();
		record.old_simtime = record.stored_simtime;
		record.stored_simtime = entity->GetSimulationTime();
		memcpy((void*)pose_parameters, &stored_pose_parameter[entity->GetIndex()], (sizeof(float) * 24));
	}
	else
	{
		entity->GetEyeAnglesPtr()->y = entity->GetEyeAngles().y;
	}
}

void Resolver::ppresolve(CBaseEntity* ent)
{
	static float lastmovelby[65];
	float brute_ang;
	int i = ent->GetIndex();
	bool slowwalk = (ent->GetVelocity().Length2D() > 0.1 && ent->GetVelocity().Length2D() < 75);

	switch (g_Aimbot->data[i].shotsmissed % 3)
	{
	case 0: brute_ang = ent->LowerBodyYaw(); break;
	case 1: brute_ang = ent->LowerBodyYaw() + 60; break;
	case 2: brute_ang = ent->LowerBodyYaw() - 60; break;
	}

	auto& record = pResolverData[ent->GetIndex()];
	if (ent->GetSimulationTime() != record.old_simtime) //Basic Desync Checking
	{
		if (ent->GetSimulationTime() - record.old_simtime != g_pGlobals->interval_per_tick) //Desync check
			record.IsDesyncing = true;
		else
			record.IsDesyncing = false;

		record.old_simtime = ent->GetSimulationTime();
	}
	record.oldeyeangles = ent->GetEyeAngles();

	if (record.IsDesyncing)
	{
		if (ent->GetFlags2() & FL_ONGROUND)
		{
			if (ent->GetVelocity().Length2D() > 75.f && !slowwalk)
			{
				lastmovelby[ent->Index()] = ent->LowerBodyYaw();
				ent->GetEyeAnglesPtr()->y = lastmovelby[i];
				csgo::resolvermode = "moving";
			}
			else
			{
				if (slowwalk)
				{
					ent->GetEyeAnglesPtr()->y = brute_ang;
					csgo::resolvermode = "fakewalk";
				}
				else
				{
					auto activity = ent->GetSequenceActivity(ent->GetAnimOverlay(3)->m_nSequence);
					static float nextlbyupdate[65];
					//lby flick cuz u break lby when desyncing??
					if (ent->GetAnimOverlay(3)->m_flWeight < 0.01f && ent->GetAnimOverlay(3)->m_flCycle < 0.69999f) {
						if (!nextlbyupdate[i])
							nextlbyupdate[i] = ent->GetSimulationTime() + g_pGlobals->interval_per_tick + 1.1f;
						ent->GetEyeAnglesPtr()->y = ent->LowerBodyYaw();
						csgo::resolvermode = "lby";
						return;
					}

					if (activity == 980 && ent->GetVelocity().Length2D() > 0.1f)
					{
						ent->GetEyeAnglesPtr()->y = ent->LowerBodyYaw();
						csgo::resolvermode = "stopping";
					}
					else
					{
						if (activity == 979)
						{
							ent->GetEyeAnglesPtr()->y = ent->GetEyeAngles().y;
							lastmovelby[i] = ent->GetEyeAngles().y;
							csgo::resolvermode = "adjusting balance";
						}
						else
						{
							ent->GetEyeAnglesPtr()->y = lastmovelby[i];
							csgo::resolvermode = "lastmovelby";
						}
					}
				}
			}
		}
		else
		{
			ent->GetEyeAnglesPtr()->y = brute_ang;
			csgo::resolvermode = "jump";
		}
	}
	else
	{
		ent->GetEyeAnglesPtr()->y = ent->GetEyeAngles().y;
		csgo::resolvermode = "no fake";
	}
}

void Resolver::ExperimentalResolver(CBaseEntity * entity)
{
	auto& record = pResolverData[entity->GetIndex()];
	const auto old_record = pResolverData[entity->GetIndex()];
	//udate records
	update(entity);

	if (GetAsyncKeyState(Menu.Ragebot.ResolverOverride)) //Checks if the key is pressed
	{
		auto* animation_state = entity->AnimState();
		Vector pos_enemy; //Vectors
		Vector local_target_angle;//Vectors
		if (GameUtils::WorldToScreen(entity->GetOrigin(), pos_enemy))
		{
			GameUtils::calculate_angle(csgo::LocalPlayer->GetOrigin(), entity->GetOrigin(), local_target_angle); //Calculate position
			ImVec2 ImGui::GetMousePos(); //Get mouse position
			float delta = ImGui::GetMousePos().x - pos_enemy.x; //Get delta
			if (delta < 0) animation_state->m_flGoalFeetYaw = entity->GetEyeAnglesPtr()->y + 60; //Resolve if delta
			else animation_state->m_flGoalFeetYaw = entity->GetEyeAnglesPtr()->y - 60; //Resolve if delta
		}
		csgo::resolvermode = "override";
	}
	else
	{
		if (!(record.isinair) && record.isonground)
		{
			for (int hi = 0; hi < 15; hi++) //idk how many anim overlays 2 look for
			{
				auto activity = entity->GetSequenceActivity(entity->GetAnimOverlay2(hi).m_nSequence);
				if (record.activity >= 961 && record.activity <= 964)  // resolver fucks up when they shoot
				{
					entity->GetEyeAnglesPtr()->x = entity->GetEyeAngles().x;
					entity->GetEyeAnglesPtr()->y = record.animangle;
					csgo::resolvermode = "shooting";
				}
				else
				{
					if (entity->IsMoving())
					{
						if (!fakewalk(entity))
						{
							if (record.movinglbyvalid)
							{
								entity->GetEyeAnglesPtr()->x = entity->GetEyeAngles().x;
								entity->GetEyeAnglesPtr()->y = record.lbyangle;
								csgo::resolvermode = "moving lby";
							}
							else
							{
								entity->GetEyeAnglesPtr()->x = entity->GetEyeAngles().x;
								entity->GetEyeAnglesPtr()->y = record.lbyangle;
								csgo::resolvermode = "moving bruteforce";
							}
						}
						else
						{
							entity->GetEyeAnglesPtr()->x = entity->GetEyeAngles().x;
							entity->GetEyeAnglesPtr()->y = record.lastmovinglby;
							csgo::resolvermode = "fakewalk";
						}
					}
					else
					{
						if (record.activity == 979)
						{
							entity->GetEyeAnglesPtr()->x = entity->GetEyeAngles().x;
							entity->GetEyeAnglesPtr()->y = record.animangle;
							csgo::resolvermode = "979";
						}
						else if (activity == 980)
						{
							entity->GetEyeAnglesPtr()->x = entity->GetEyeAngles().x;
							entity->GetEyeAnglesPtr()->y = record.animangle;
							csgo::resolvermode = "stopped";
						}
						else if (record.islastmovinglbyvalid)
						{
							entity->GetEyeAnglesPtr()->x = entity->GetEyeAngles().x;
							entity->GetEyeAnglesPtr()->y = record.lastmovinglby;
							csgo::resolvermode = "last move lby valid";
						}
						else
						{
							entity->GetEyeAnglesPtr()->x = entity->GetEyeAngles().x;
							entity->GetEyeAnglesPtr()->y = record.lastmovinglby;
							csgo::resolvermode = "last moving lby";
						}
					}
				}
			}
		}
		else
		{
			entity->GetEyeAnglesPtr()->x = entity->GetEyeAngles().x;
			entity->GetEyeAnglesPtr()->y = record.eyeangle;
			csgo::resolvermode = "flying";
		}
	}
}

void Resolver::update(CBaseEntity* entity)
{
	float* pose_parameters = (float*)((uintptr_t)entity + 0x2774);
	const auto old_records = pResolverData[entity->GetIndex()];
	auto& record = pResolverData[entity->GetIndex()];
	CBaseCombatWeapon* pWeapon = entity->GetWeapon();

	//shot
	if (pWeapon)
	{
		if (record.shot_time != pWeapon->GetAccuracyPenalty())
		{
			record.shot = true;
			record.shot_time = pWeapon->GetLastShotTime();
		}
		else
			record.shot = false;
	}
	else
	{
		record.shot = false;
		record.shot_time = 0.f;
	}

	for (int hi = 0; hi < entity->GetNumAnimOverlays(); hi++) //idk how many anim overlays 2 look for
	{
		record.activity = entity->GetSequenceActivity(entity->GetAnimOverlay2(hi).m_nSequence);

		if (record.activity == 980)
		{
			record.animangle = record.lastmovinglby;
		}

		if (record.activity == 979)
		{
			record.animangle = record.eyeangle;
			record.lastmovinglby = record.animangle;
		}

		if (record.activity >= 961 && record.activity <= 964)
		{
			record.animangle = record.lookatlocalp;
		}
	}

	//angle stuff
	record.eyeangle = entity->GetEyeAngles().y;
	record.lbyangle = entity->LowerBodyYaw();
	record.flags = entity->GetFlags2();
	record.simtime = entity->GetSimulationTime();
	record.origin = entity->GetOrigin();
	record.lookatlocalp = GameUtils::CalculateAngle(entity->GetOrigin(), csgo::LocalPlayer->GetOrigin()).y;
	memcpy(&record.stored_poseparameter, pose_parameters, sizeof(float) * 24);

	//moving stuff
	record.fakewalking = fakewalk(entity);
	record.isdormant = entity->IsDormant();
	record.movingspeed = entity->GetVelocity().Length2D();
	record.ismoving = entity->GetVelocity().Length2D() > 1 && !record.fakewalking;
	record.isonground = (record.flags & FL_ONGROUND);
	record.ismovingonground = record.ismoving && record.isonground;
	record.isinair = !(record.flags & FL_ONGROUND);

	///thanks to baimware for valid last moving lby
	//here 
	{
		if (record.ismovingonground && !record.isdormant)
		{
			record.movinglby = record.lbyangle;
			record.islastmovinglbyvalid = true;
			record.islastmovinglbydeltavalid = false;
			record.lastmovinglby = record.lbyangle + 45;
			record.lasttimemoving = Math::GetCurtime();
		}

		if (!record.isdormant && old_records.isdormant)
		{
			if ((record.origin - old_records.origin).Length2D() > 16.f)
				record.islastmovinglbyvalid = false;
		}

		if (!record.islastmovinglbydeltavalid && record.islastmovinglbyvalid && record.movingspeed < 20 && fabs(Math::GetCurtime() - record.lasttimemoving) < 1.0)
		{
			if (record.lbyangle != old_records.lbyangle)
			{
				record.lastmovinglby = Math::NormalizeYaw(record.lastmovinglby - record.lbyangle);
				record.islastmovinglbydeltavalid = false;
			}
		}
	}
	//to here

	record.movinglbyvalid = (record.movinglby != INT_MAX);

	if (record.fakewalking)
		csgo::shouldbaim = true;
	else
		csgo::shouldbaim = false;

	if (record.ismoving && !record.isinair && !record.fakewalking)
	{
		record.lastmovinglby = record.lbyangle;
	}
}

Resolver* g_Resolver = new Resolver;