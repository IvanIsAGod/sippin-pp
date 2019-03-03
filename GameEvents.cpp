#include "hooks.h"
#include <time.h>
#include "Mmsystem.h"
#include <thread>
#include "global.h"
#include "Menu.h"
#include "controls.h"
#include "ESP.h"
#include "Math.h"
#include "bullet_log.h"
#include "event_log.h"
#include "AW_hitmarker.h"
#include "sounds.h"
#include "Draw.h"
#include "BacktrackingHelper.h"
#include "Aimbot.h"

#pragma comment(lib, "winmm.lib") 

string prefix = "[Sippin Piss] ";

std::vector<impact_info> impacts;
std::vector<hitmarker_info> hitmarkers;

#define EVENT_HOOK( x )
#define TICK_INTERVAL			(g_pGlobals->interval_per_tick)
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )

cGameEvent g_Event;

std::vector<cbullet_tracer_info> logs;
std::vector<sound_info> sound_logs;
std::vector<trace_info> trace_logs;

std::vector <CMessage> Eventlog::messages;

CBaseEntity* get_player(int userid) {
	int i = g_pEngine->GetPlayerForUserID(userid);
	return g_pEntitylist->GetClientEntity(i);
}

float cur() {
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

int showtime = 3;
void Eventlog::Msg(string str, bool attack)
{
	float blue[3] = { 0.24f, 0.40f, 0.95f };
	Color clr = Color::Color(blue);
	messages.push_back(CMessage(str, cur(), clr, 255));
	g_pEngine->ClientCmd_Unrestricted(("echo " + prefix + str).c_str());
}
void Eventlog::Draw()
{
	for (int i = messages.size() - 1; i >= 0; i--)
	{
		if (messages[i].time + showtime <= g_pGlobals->curtime)
			messages[i].alpha -= 2;

		draw::Textf(70, 20 + (15 * i), Color(255, 255, 255, messages[i].alpha), F::eventlog, (char*)messages[i].str.c_str());
		draw::Textf(10, 20 + (15 * i), Color(messages[i].clr.r(), messages[i].clr.g(), messages[i].clr.b(), messages[i].alpha), F::eventlog, prefix.c_str());

		if (messages[i].alpha <= 0)
			messages.erase(messages.begin() + i);
	}
}


char* HitgroupToName(int hitgroup)
{
	switch (hitgroup)
	{
	case HITGROUP_HEAD:
		return "head";
	case HITGROUP_CHEST:
		return "chest";
	case HITGROUP_STOMACH:
		return "stomach";
	case HITGROUP_LEFTARM:
		return "left arm";
	case HITGROUP_RIGHTARM:
		return "right arm";
	case HITGROUP_LEFTLEG:
		return "left leg";
	case HITGROUP_RIGHTLEG:
		return "right leg";
	default:
		return "body";

	}
}

player_info_t GetInfo(int Index)
{
	player_info_t Info;
	g_pEngine->GetPlayerInfo(Index, &Info);
	return Info;
}

void Hurt(IGameEvent* Event)
{
	if (!csgo::LocalPlayer || !g_pEngine->IsConnected() || !g_pEngine->IsInGame())
		return;
	int attackerid = Event->GetInt("attacker");
	int entityid = g_pEngine->GetPlayerForUserID(attackerid);
	if (entityid == g_pEngine->GetLocalPlayer())
	{
		int nUserID = Event->GetInt("attacker");
		int nDead = Event->GetInt("userid");
		if (nUserID || nDead)
		{
			player_info_t killed_info = GetInfo(g_pEngine->GetPlayerForUserID(nDead));
			player_info_t killer_info = GetInfo(g_pEngine->GetPlayerForUserID(nUserID));
			csgo::DamageDealt = Event->GetInt("dmg_health");

			std::string _1 = "Hit ";
			std::string _2 = killed_info.m_szPlayerName;
			std::string _3 = " at ";
			std::string _4 = HitgroupToName(Event->GetInt("hitgroup"));
			std::string _5 = " for ";
			std::string _6 = Event->GetString("dmg_health");
			std::string _7 = " damage";
			std::string _8 = " (";
			std::string _9 = Event->GetString("health");
			std::string _10 = " health remaining)";
			string out = _1 + _2 + _3 + _4 + _5 + _6 + _7 + _8 + _9 + _10;

			if (Event->GetInt("health") > 0)
				Eventlog::Msg(out, false);
			else
			{
				std::string _1 = "Killed ";
				std::string _2 = killed_info.m_szPlayerName;
				std::string _3 = " at ";
				std::string _4 = HitgroupToName(Event->GetInt("hitgroup"));
				string out = _1 + _2 + _3 + _4;
				Eventlog::Msg(out, true);
			}
		}

	}
}

template<class T>
static T* Find_Hud_Element(const char* name)
{
	static auto pThis = *reinterpret_cast<DWORD**>(FindPatternIDA("client_panorama.dll", "B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08") + 1);
	static auto find_hud_element = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(FindPatternIDA("client_panorama.dll", "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28"));
	return (T*)find_hud_element(pThis, name);
}

void cGameEvent::FireGameEvent(IGameEvent *event)
{
	/*short   m_nUserID		user ID who was hurt
	short	attacker	user ID who attacked
	byte	health		remaining health points
	byte	armor		remaining armor points
	string	weapon		weapon name attacker used, if not the world
	short	dmg_health	damage done to health
	byte	dmg_armor	damage done to armor
	byte	hitgroup	hitgroup that was damaged*/
	if (!csgo::LocalPlayer || !g_pEngine->IsConnected() || !g_pEngine->IsInGame())
		return;
	const char* szEventName = event->GetName();
	if (!szEventName)
		return;

	if (strstr(event->GetName(), "weapon_fire")) //IF the enemy makes a gun fire, make a sound beam around them
	{
		/*
			Votrix says that I should be doing this in Aimbot due to the idea that Eventlistener gets scuffed even though event logs don't get stuffed by a bit
			So I made a check to see if it was the aimbot firing and used the weapon fire event, plus to check if it was the local player

			Easy way to keep track of shots compared to aimbot
		*/

		auto entity = get_player(event->GetInt("userid"));

		if (entity == nullptr) //No reason to do the event if they are nullptr
			return;

		if (!entity->isAlive()) //No reason to do the event if they are dead
			return;

		if (entity == csgo::LocalPlayer)
		{
			if (g_Aimbot->aimbotted) //Checks to see if it was the aimbot who fired
				g_Aimbot->data[entity->GetIndex()].shoots++;
		}

		if (entity != csgo::LocalPlayer && entity->GetTeamNum() != csgo::LocalPlayer->GetTeamNum())
		{
			sound_logs.push_back(sound_info(entity->GetAbsOrigin(), g_pGlobals->curtime, event->GetInt("userid"))); //Send sound logs if weapon fire happens
		}
	}
	if (strstr(event->GetName(), "round_start")) //If the player died, reset the shots
	{
		/*csgo::NewRound = true;
		static DWORD* deathNotice = Find_Hud_Element<DWORD>("CCSGO_HudDeathNotice");

		static DWORD* _death_notice = Find_Hud_Element<DWORD>("CCSGO_HudDeathNotice");
		static void(__thiscall *_clear_notices)(DWORD) = (void(__thiscall*)(DWORD))FindPatternIDA("client_panorama.dll", "55 8B EC 83 EC 0C 53 56 8B 71 58");

		_death_notice = Find_Hud_Element<DWORD>("CCSGO_HudDeathNotice");
		_clear_notices(((DWORD)_death_notice - 20));*/

		g_Aimbot->data->shoots = { 0 };
		g_Aimbot->data->shoots_hit = { 0 };
		g_Aimbot->data->shotsmissed = { 0 };
	}
	else
		csgo::NewRound = false;
	if (strstr(event->GetName(), "player_footstep")) //IF the enemy makes a foot step, they will make a sound
	{
		if (Menu.Visuals.footstepesp)
		{
			auto entity = g_pEntitylist->GetClientEntity(g_pEngine->GetPlayerForUserID(event->GetInt("userid")));

			if (entity == nullptr) //Da faqk 
				return;

			if (!entity) //Da faqk 
				return;

			if (entity->GetTeamNum() == csgo::LocalPlayer->GetTeamNum()) //Fuck teammates
				return;

			if (entity->IsDormant()) //Entity dormant is gay
				return;

			sound_logs.push_back(sound_info(entity->GetAbsOrigin(), g_pGlobals->curtime, event->GetInt("userid")));//Send those sound logs
		}
	}
	if (strstr(event->GetName(), "player_connect_full")) 
	{
		csgo::OldTickCount = g_pGlobals->tickcount;
	}

	if (strcmp(szEventName, "player_hurt") == 0) //If a player is hurt
	{

		Hurt(event); //Event Logs

		auto entity = g_pEntitylist->GetClientEntity(g_pEngine->GetPlayerForUserID(event->GetInt("userid")));
		if (!entity) //IF they not a entity
			return;

		if (entity->GetTeamNum() == csgo::LocalPlayer->GetTeamNum()) //Fuck teammates
			return;

		auto attacker = get_player(event->GetInt("attacker")); //get the attacker
		if (attacker == csgo::LocalPlayer) //IF it's us
		{
			g_Aimbot->data[entity->Index()].shoots_hit++; //Add to our logs

			if (Menu.Visuals.footstepesp) //Footstep esp (they will make a sound of fucking agony from that bullet)
			{
				auto entity = g_pEntitylist->GetClientEntity(g_pEngine->GetPlayerForUserID(event->GetInt("userid")));

				if (entity == nullptr)
					return;

				if (!entity)
					return;

				if (entity->GetTeamNum() == csgo::LocalPlayer->GetTeamNum())
					return;

				if (entity->IsDormant())
					return;

				sound_logs.push_back(sound_info(entity->GetAbsOrigin(), g_pGlobals->curtime, event->GetInt("userid"))); //Send those sound logs
			}
		}
	}

	if (strcmp(szEventName, "player_death") == 0)
	{ 
		auto entity = g_pEntitylist->GetClientEntity(g_pEngine->GetPlayerForUserID(event->GetInt("userid")));
		if (!entity) //IF they not a entity
			return;

		if (entity->GetTeamNum() == csgo::LocalPlayer->GetTeamNum()) //Fuck teammates
			return;

		auto attacker = get_player(event->GetInt("attacker")); //get the attacker
		if (attacker != csgo::LocalPlayer)
			return;

		g_Aimbot->data[entity->Index()].shoots_hit = 0;
		g_Aimbot->data[entity->Index()].shoots = 0;
		g_Aimbot->data[entity->Index()].shotsmissed = 0;

		static DWORD* deathNotice = Find_Hud_Element<DWORD>("CCSGO_HudDeathNotice");

		static DWORD* _death_notice = Find_Hud_Element<DWORD>("CCSGO_HudDeathNotice");
		static void(__thiscall *_clear_notices)(DWORD) = (void(__thiscall*)(DWORD))FindPatternIDA("client_panorama.dll", "55 8B EC 83 EC 0C 53 56 8B 71 58");


		if (_death_notice && Menu.Misc.PreserveKillfeed)
			*(float*)((DWORD)_death_notice + 0x50) = FLT_MAX;
		else
			*(float*)((DWORD)_death_notice + 0x50) = 1.5f;
	}
	//Hitmarker AW
	if (Menu.Visuals.Hitmarker)
	{
		if (strcmp(szEventName, "player_hurt") == 0)
		{
			auto attacker = get_player(event->GetInt("attacker"));
			auto victim = get_player(event->GetInt("userid"));

			if (!attacker || !victim || attacker != csgo::LocalPlayer)
				return;

			Vector enemypos = victim->GetOrigin();
			impact_info best_impact;
			float best_impact_distance = -1;
			float time = g_pGlobals->curtime;

			for (int i = 0; i < impacts.size(); i++)
			{
				auto iter = impacts[i];
				if (time > iter.time + 1.f)
				{
					impacts.erase(impacts.begin() + i);
					continue;
				}
				Vector position = Vector(iter.x, iter.y, iter.z);
				float distance = position.DistTo(enemypos);
				if (distance < best_impact_distance || best_impact_distance == -1)
				{
					best_impact_distance = distance;
					best_impact = iter;
				}
			}
			if (best_impact_distance == -1)
				return;

			hitmarker_info info; //hITMARKER INFO DATA
			info.impact = best_impact; //WRITE DATA
			info.alpha = 255; //WRITE MORE DATA
			hitmarkers.push_back(info); //PUSH THE DATA

			int Attacker = event->GetInt("attacker"); //IDENTIFY THE ATTACKER

			//SOUNDS FOR THOSE HITMARKERS
			if (g_pEngine->GetPlayerForUserID(Attacker) == g_pEngine->GetLocalPlayer())
			{

				switch (Menu.Visuals.htSound)
				{
				case 0:
					PlaySoundA(hitmarker_sound, nullptr, SND_MEMORY | SND_ASYNC | SND_NOSTOP);
					break;
				case 1:
					PlaySoundA(hitmarker_sound2, nullptr, SND_MEMORY | SND_ASYNC | SND_NOSTOP);
					break;
				case 2:
					PlaySoundA(hitmarker_sound3, nullptr, SND_MEMORY | SND_ASYNC | SND_NOSTOP);
					break;
				case 3:
					PlaySoundA(water, nullptr, SND_MEMORY | SND_ASYNC | SND_NOSTOP);
					break;
				case 4:
					g_pEngine->ClientCmd_Unrestricted("play buttons\\arena_switch_press_02.wav");
					break;
				case 5:
					PlaySoundA(stapler, nullptr, SND_MEMORY | SND_ASYNC | SND_NOSTOP);
					break;
				case 6:
					PlaySoundA(vk, nullptr, SND_MEMORY | SND_ASYNC | SND_NOSTOP);
					break;
				}
			}
		}
	}

	/*****************************DON'T TOUCH BECUZ IT'S THE BULLET TRACERS, REMOVE A SINGLE ONE OF THESE AND THE OTHER BREAKS GAY NIG NOG*****************************/
	if (strstr(event->GetName(), "bullet_impact"))
	{
		CBaseEntity* ent = get_player(event->GetInt("userid"));
		if (!ent)
			return;

		impact_info info;
		info.x = event->GetFloat("x");
		info.y = event->GetFloat("y");
		info.z = event->GetFloat("z");

		info.time = g_pGlobals->curtime;

		impacts.push_back(info);
	}
	/*****************************DON'T TOUCH BECUZ IT'S THE BULLET TRACERS, REMOVE A SINGLE ONE OF THESE AND THE OTHER BREAKS GAY NIG NOG*****************************/

	if (Menu.Visuals.BulletTracers)
	{
		if (Menu.Visuals.bulletType == 0)
		{
			if (strcmp(szEventName, "bullet_impact") == 0)
			{
				auto* index = g_pEntitylist->GetClientEntity(g_pEngine->GetPlayerForUserID(event->GetInt("userid")));

				if (csgo::LocalPlayer)
				{
					Vector position(event->GetFloat("x"), event->GetFloat("y"), event->GetFloat("z"));

					if (index)

						trace_logs.push_back(trace_info(index->GetEyePosition(), position, g_pGlobals->curtime, event->GetInt("userid")));
				}
			}
		}
		/*****************************DON'T TOUCH BECUZ IT'S THE BULLET TRACERS, REMOVE A SINGLE ONE OF THESE AND THE OTHER BREAKS GAY NIG NOG*****************************/
		else if (Menu.Visuals.bulletType == 1)
		{
			if (strstr(event->GetName(), "bullet_impact"))
			{
				//get the user who fired the bullet
				auto index = g_pEngine->GetPlayerForUserID(event->GetInt("userid"));

				//return if the userid is not valid or we werent the entity who was fireing
				if (index != g_pEngine->GetLocalPlayer())
					return;

				//get local player
				auto local = static_cast<CBaseEntity*>(g_pEntitylist->GetClientEntity(index));
				if (!local)
					return;

				//get the bullet impact's position
				Vector position(event->GetFloat("x"), event->GetFloat("y"), event->GetFloat("z"));

				Ray_t ray;
				ray.Init(local->GetEyePosition(), position);

				//skip local player
				CTraceFilter filter;
				filter.pSkip1 = local;

				//trace a ray
				trace_t tr;
				g_pEngineTrace->TraceRay(ray, MASK_SHOT, &filter, &tr);

				float Red, Green, Blue;
				Red = Menu.Colors.Bulletracer[0] * 255;
				Green = Menu.Colors.Bulletracer[1] * 255;
				Blue = Menu.Colors.Bulletracer[2] * 255;

				//push info to our vector
				logs.push_back(cbullet_tracer_info(local->GetEyePosition(), position, g_pGlobals->curtime, Color(Red, Green, Blue)));
			}
		}
	}
}
/*****************************DON'T TOUCH BECUZ IT'S THE BULLET TRACERS, REMOVE A SINGLE ONE OF THESE AND THE OTHER BREAKS GAY NIG NOG*****************************/


int cGameEvent::GetEventDebugID()
{
	return 42;
}

void cGameEvent::RegisterSelf()
{
	g_pGameEventManager->AddListener(this, "player_connect_full", false);
	g_pGameEventManager->AddListener(this, "player_connect", false);
	g_pGameEventManager->AddListener(this, "player_hurt", false);
	g_pGameEventManager->AddListener(this, "round_start", false);
	g_pGameEventManager->AddListener(this, "round_end", false);
	g_pGameEventManager->AddListener(this, "player_death", false);
	g_pGameEventManager->AddListener(this, "weapon_fire", false);
	g_pGameEventManager->AddListener(this, "bullet_impact", false);
	g_pGameEventManager->AddListener(this, "player_footstep", false);
}

void cGameEvent::Register()
{
	EVENT_HOOK(FireEvent);
}
