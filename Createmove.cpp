#include "hooks.h"
#include "global.h"
#include "Misc.h"
#include "Menu.h"
#include "BacktrackingHelper.h"
#include "Math.h"
#include "GameUtils.h"
#include "backtrackmanager.h"
#include "Aimbot.h"
#include "PredictionSystem.h"
#include "Antiaim.h"
#include "GrenadePrediction.h"
#include "legitbot.h"
#include <Psapi.h>
#include "knifebot.h"

static CPredictionSystem* Prediction = new CPredictionSystem();

static int Ticks = 0;
static int LastReserve = 0;
namespace FakeLatency
{
	char *clientdllstr = new char[11]{ 25, 22, 19, 31, 20, 14, 84, 30, 22, 22, 0 }; /*client.dll*/
	char *enginedllstr = new char[11]{ 31, 20, 29, 19, 20, 31, 84, 30, 22, 22, 0 }; /*engine.dll*/
	char *cam_tofirstperson_sig = new char[51]{ 59, 75, 90, 90, 69, 69, 90, 90, 69, 69, 90, 90, 69, 69, 90, 90, 69, 69, 90, 90, 56, 67, 90, 90, 69, 69, 90, 90, 69, 69, 90, 90, 69, 69, 90, 90, 69, 69, 90, 90, 60, 60, 90, 90, 67, 74, 90, 90, 67, 74, 0 }; /*A1  ??  ??  ??  ??  B9  ??  ??  ??  ??  FF  90  90*/
	char *cam_tothirdperson_sig = new char[51]{ 59, 75, 90, 90, 69, 69, 90, 90, 69, 69, 90, 90, 69, 69, 90, 90, 69, 69, 90, 90, 56, 67, 90, 90, 69, 69, 90, 90, 69, 69, 90, 90, 69, 69, 90, 90, 69, 69, 90, 90, 60, 60, 90, 90, 67, 74, 90, 90, 66, 57, 0 }; /*A1  ??  ??  ??  ??  B9  ??  ??  ??  ??  FF  90  8C*/
	char *clientstatestr = new char[51]{ 66, 56, 90, 90, 73, 62, 90, 90, 69, 69, 90, 90, 69, 69, 90, 90, 69, 69, 90, 90, 69, 69, 90, 90, 66, 59, 90, 90, 60, 67, 90, 90, 60, 73, 90, 90, 74, 60, 90, 90, 75, 75, 90, 90, 78, 79, 90, 90, 60, 57, 0 }; /*8B  3D  ??  ??  ??  ??  8A  F9  F3  0F  11  45  FC*/
	char *updateclientsideanimfnsigstr = new char[95]{ 79, 79, 90, 90, 66, 56, 90, 90, 63, 57, 90, 90, 79, 75, 90, 90, 79, 76, 90, 90, 66, 56, 90, 90, 60, 75, 90, 90, 66, 74, 90, 90, 56, 63, 90, 90, 69, 69, 90, 90, 69, 69, 90, 90, 74, 74, 90, 90, 74, 74, 90, 90, 74, 74, 90, 90, 77, 78, 90, 90, 73, 76, 90, 90, 66, 56, 90, 90, 74, 76, 90, 90, 60, 60, 90, 90, 67, 74, 90, 90, 69, 69, 90, 90, 69, 69, 90, 90, 74, 74, 90, 90, 74, 74, 0 }; /*55  8B  EC  51  56  8B  F1  80  BE  ??  ??  00  00  00  74  36  8B  06  FF  90  ??  ??  00  00*/
	void DecStr(char *adr, const unsigned len)
	{
		for (unsigned i = 0; i < len; i++)
		{
			adr[i] ^= 50;
			adr[i] ^= 72;
		}
	}

	void EncStr(char *adr, const unsigned len)
	{
		for (unsigned i = 0; i < len; i++)
		{
			adr[i] ^= 72;
			adr[i] ^= 50;
		}
	}

	void DecStr(char *adr)
	{
		int len = strlen(adr);
		for (unsigned i = 0; i < len; i++)
		{
			adr[i] ^= 50;
			adr[i] ^= 72;
		}
	}

	void EncStr(char *adr)
	{
		int len = strlen(adr);
		for (unsigned i = 0; i < len; i++)
		{
			adr[i] ^= 72;
			adr[i] ^= 50;
		}
	}
	HANDLE FindHandle(std::string name)
	{
		return GetModuleHandle(name.c_str());
	}
	uintptr_t FindMemoryPattern(HANDLE ModuleHandle, char* strpattern, int length)
	{
		unsigned char *signature = new unsigned char[length + 1];
		bool *skippable = new bool[length + 1];
		int signaturelength = 0;
		for (int byteoffset = 0; byteoffset < length - 1; byteoffset += 2)
		{
			char charhex[4]; //4 to keep sscanf happy
			*(short*)charhex = *(short*)&strpattern[byteoffset];
			if (charhex[0] != ' ')
			{
				if (charhex[0] == '?')
				{
					signature[signaturelength] = '?';
					skippable[signaturelength] = true;
				}
				else
				{
					charhex[2] = NULL; //add null terminator
					signature[signaturelength] = (unsigned char)std::stoul(charhex, nullptr, 16);
					skippable[signaturelength] = false;
				}
				signaturelength++;
			}
		}
		int searchoffset = 0;
		int maxoffset = signaturelength - 1;


		MODULEINFO dllinfo;
		GetModuleInformation(GetCurrentProcess(), (HMODULE)ModuleHandle, &dllinfo, sizeof(MODULEINFO));
		DWORD endadr = (DWORD)ModuleHandle + dllinfo.SizeOfImage;
		DWORD adrafterfirstmatch = NULL;
		for (DWORD adr = (DWORD)ModuleHandle; adr < endadr; adr++)
		{
			if (skippable[searchoffset] || *(char*)adr == signature[searchoffset] || *(unsigned char*)adr == signature[searchoffset])
			{
				if (searchoffset == 0)
				{
					adrafterfirstmatch = adr + 1;
				}
				searchoffset++;
				if (searchoffset > maxoffset)
				{
					delete[] signature;
					delete[] skippable;
					//timetakentosearch = QPCTime() - startsearch;
					return adr - maxoffset; //FOUND OFFSET!
				}
			}
			else if (adrafterfirstmatch)
			{
				adr = adrafterfirstmatch;
				searchoffset = 0;
				adrafterfirstmatch = NULL;
			}
		}

		delete[] signature;
		delete[] skippable;
		return NULL; //NOT FOUND!
	}
	HANDLE EngineHandle = NULL;
	HANDLE ClientHandle = NULL;
	DWORD ClientState = NULL;
}

int __fastcall Hooked_SendDatagram(void* netchan, void*, void *datagram)
{

	INetChannel* chan = (INetChannel*)netchan;
	bf_write* data = (bf_write*)datagram;


	int instate = chan->m_nInReliableState;
	int insequencenr = chan->m_nInSequenceNr;

	if (Menu.Ragebot.FakeLatency)
	{
		if (!g_pEngine->IsConnected() || !g_pEngine->IsInGame())
			g_BacktrackHelper->ClearIncomingSequences();

		g_BacktrackHelper->AddLatencyToNetchan(chan, Menu.Ragebot.FakeLatencyAmount);
	}

	int ret = HNetchan->GetOriginalMethod<SendDatagramFn>(46)(chan, data);

	chan->m_nInReliableState = instate;
	chan->m_nInSequenceNr = insequencenr;


	return ret;
}

int ground_tick;

void ground_ticks()
{
	auto local_player = csgo::LocalPlayer;

	if (!local_player)
		return;

	if (*local_player->GetFlags() & FL_ONGROUND)
		ground_tick++;
	else
		ground_tick = 0;
}

void setClanTag(const char* tag)//190% paste
{
	static auto pSetClanTag = reinterpret_cast<void(__fastcall*)(const char*, const char*)>(((DWORD)Utilities::Memory::FindPattern("engine.dll", (PBYTE)"\x53\x56\x57\x8B\xDA\x8B\xF9\xFF\x15\x00\x00\x00\x00\x6A\x24\x8B\xC8\x8B\x30", "xxxxxxxxx????xxxxxx")));
	if (pSetClanTag)
	{
		auto tag_ = std::string(tag);
		if (strlen(tag) > 0) {
			auto newline = tag_.find("\\n");
			auto tab = tag_.find("\\t");
			if (newline != std::string::npos) {
				tag_.replace(newline, newline + 2, "\n");
			}
			if (tab != std::string::npos) {
				tag_.replace(tab, tab + 2, "\t");
			}
		}
		static auto dankesttSetClanTag = reinterpret_cast<void(__fastcall*)(const char*, const char*)>(pSetClanTag);
		dankesttSetClanTag(tag_.data(), tag_.data());
	}
}

void rotate(std::string& urtext)
{
	std::string temp_string = urtext;
	urtext.erase(0, 1);
	urtext += temp_string[0];
}

void Clantag()
{
	static int old_time;
	static int i = 0;

	std::string clantagcheck = std::string(Menu.Misc.customclantag);
	static std::string clantag = clantagcheck;
	if (i > 32)
	{
		if (Menu.Misc.Clantag == 1)
		{
			static int counter = 0;
			int server_time = 2.75*((float)g_pGlobals->interval_per_tick * csgo::LocalPlayer->GetTickBase());
			int value = (server_time % 28);
			if (counter % 28 == 0)

			switch (value)
			{
			case 0: setClanTag("                "); break;
			case 1: setClanTag("               s"); break;
			case 2: setClanTag("              si"); break;
			case 3: setClanTag("             sip"); break;
			case 4: setClanTag("            sipp"); break;
			case 5: setClanTag("           sippi"); break;
			case 6: setClanTag("          sippin"); break;
			case 7: setClanTag("         sippin'"); break;
			case 8: setClanTag("        sippin' "); break;
			case 9: setClanTag("       sippin' p"); break;
			case 10:setClanTag("      sippin' pi"); break;
			case 11:setClanTag("     sippin' pis"); break;
			case 12:setClanTag("    sippin' piss"); break;
			case 13:setClanTag("   sippin' piss "); break;
			case 14:setClanTag("  sippin' piss  "); break;
			case 15:setClanTag(" sippin' piss   "); break;
			case 16:setClanTag("sippin' piss    "); break;
			case 17:setClanTag("ippin' piss     "); break;
			case 18:setClanTag("ppin' piss      "); break;
			case 19:setClanTag("pin' piss       "); break;
			case 20:setClanTag("in' piss        "); break;
			case 21:setClanTag("n' piss         "); break;
			case 22:setClanTag("' piss          "); break;
			case 23:setClanTag(" piss           "); break;
			case 24:setClanTag("piss            "); break;
			case 25:setClanTag("iss             "); break;
			case 26:setClanTag("ss              "); break;
			case 27:setClanTag("s               "); break;
			case 28:setClanTag("                "); break;
			}
		}
		else if (Menu.Misc.Clantag == 2)
		{
			rotate(clantag);
			setClanTag(clantag.c_str());
			i = 0;
		}
		else
		{
			char* p = "";
			setClanTag(p);
		}
	}
	else
	{
		i++;
	}
}

void BulletImpacts()
{
	static ConVar* impacts = g_pCvar->FindVar("sv_showimpacts");
	static ConVar* impacttime = g_pCvar->FindVar("sv_showimpacts_time");
	if (Menu.Visuals.drawimpacts) {
		impacts->SetValue(1);
		impacttime->SetValue(1);
	}
	else impacts->SetValue(0);
}

void slowwalkspeed(int speed)
{
	auto sidespeed = g_pCvar->FindVar("cl_sidespeed");
	auto forwardspeed = g_pCvar->FindVar("cl_forwardspeed");
	auto backspeed = g_pCvar->FindVar("cl_backspeed");
	if (sidespeed->GetInt() == 450 && speed == 450) return;
	sidespeed->SetValue(speed);
	forwardspeed->SetValue(speed);
	backspeed->SetValue(speed);
}

void SlowWalk(CUserCmd *userCMD)
{
	if (GetAsyncKeyState(Menu.Misc.Slowwalkkey))
	{
		csgo::NormalSpeed = false;
		int finalspeed = 250 * Menu.Misc.Slowwalkspeed / 100;
		slowwalkspeed(finalspeed);
	}
	if (!GetAsyncKeyState(Menu.Misc.Slowwalkkey) && !csgo::Minwalk)
		csgo::NormalSpeed = true;
	else
		csgo::NormalSpeed = false;

	if (csgo::NormalSpeed) slowwalkspeed(450);
}

TraceRayFn oTraceRay;

bool __fastcall Hooks::CreateMove(void* thisptr, void*, float flInputSampleTime, CUserCmd* cmd)
{
	if (cmd)
	{
		if (!cmd->command_number)
			return true;

		static bool last_sendpacket;
		g_pEngine->SetViewAngles(cmd->viewangles);
		if (g_pEngine->IsInGame() && g_pEngine->IsConnected())
		{
			CBaseEntity* pLocalPlayer = g_pEntitylist->GetClientEntity(g_pEngine->GetLocalPlayer());
			if (pLocalPlayer)
			{
				g_BacktrackHelper->UpdateIncomingSequences();
				csgo::LocalPlayer = pLocalPlayer;
				csgo::UserCmd = cmd;
				csgo::UserCmdForBacktracking = cmd;
				if (pLocalPlayer->GetHealth() > 0)
				{
					CBaseCombatWeapon* pWeapon = pLocalPlayer->GetWeapon();
					if (pWeapon)
					{
						PVOID pebp;
						__asm mov pebp, ebp;
						bool* pbSendPacket = (bool*)(*(PDWORD)pebp - 0x1C);
						bool& bSendPacket = *pbSendPacket;

						csgo::SendPacket = bSendPacket;
						csgo::vecUnpredictedVel = csgo::LocalPlayer->GetVelocity();

						csgo::MainWeapon = pWeapon;
						csgo::WeaponData = pWeapon->GetCSWpnData();

						csgo::StrafeAngle = csgo::UserCmd->viewangles;

						g_Aimbot->DropTarget();
						//Prediction->EnginePrediction(cmd);

						if (Menu.Misc.Clantag > 0)
							Clantag();
						g_Misc->Bunnyhop();
						g_Misc->AutoStrafe();
						g_Misc->DoCircle();
						BulletImpacts();
						g_Misc->edgejump(csgo::UserCmd);
						g_Misc->UpdateFlSettings();

						if (csgo::OldWeapon != csgo::MainWeapon->GetWeaponNum()) {
							g_Misc->WpnCfgMenu();
							g_Misc->WpnCfgData();
						}

						if (Menu.Misc.infiniteduck)
						{
							csgo::UserCmd->buttons |= IN_BULLRUSH;
						}

						Prediction->run_prediction(csgo::UserCmd);
						{ // aimbot stuff in here 4 prediction
							g_Aimbot->extrapolation();
							g_Aimbot->Run();
							g_Aimbot->CompensateInaccuracies();
							if (Menu.Misc.KnifeBot) {
								KnifeBot::Run();
							}
							if ((csgo::UserCmd->buttons & IN_ATTACK && csgo::MainWeapon->WeaponID() != REVOLVER) || (csgo::UserCmd->buttons & IN_ATTACK2 && (csgo::MainWeapon->WeaponID() == REVOLVER)) && GameUtils::IsAbleToShoot())
								g_Aimbot->fired_in_that_tick = true;
							if (*csgo::MainWeapon->ItemDefinitionIndex() == WEAPON_TASER)
							{
								if (Menu.Misc.ZeusBot) {
									g_Aimbot->AutoZeus();
								}
							}
							if (Menu.LegitBot.bEnable && !Menu.Ragebot.EnableAimbot) {
								CBackTrackManager::get().UpdateTicks();
								g_Legitbot->Init();
								g_Legitbot->Move(csgo::UserCmd, csgo::SendPacket);
							}
						}
						Prediction->end_prediction(csgo::UserCmd);

						if (csgo::NormalSpeed)
							g_Antiaim->SetSpeed(450);

						g_Misc->FakeLag();

						if (csgo::ChokedPackets >= Menu.Misc.FakelagAmount)// breaks fakewalk
						{
							csgo::SendPacket = true;
							csgo::ChokedPackets = 0;
						}
						if (Menu.Antiaim.AntiaimEnable)
							g_Antiaim->RunBeta(csgo::UserCmd);
						if (Menu.Antiaim.FakeDuck)
							g_Antiaim->FakeDuck(csgo::UserCmd);
						if (Menu.Misc.Slowwalkenable)
							SlowWalk(csgo::UserCmd);
						if (csgo::SendPacket)
						{
							csgo::FakeAngle = csgo::UserCmd->viewangles;
							csgo::weewee = csgo::LocalPlayer->GetOrigin();
						}
						else
						{
							csgo::ChokedPackets++;
							csgo::RealAngle = csgo::UserCmd->viewangles;
						}
						if ((csgo::UserCmd->buttons & IN_ATTACK || csgo::UserCmd->buttons & IN_ATTACK2 && csgo::MainWeapon->WeaponID() == REVOLVER) && (csgo::MainWeapon->IsPistol() || csgo::MainWeapon->WeaponID() == AWP || csgo::MainWeapon->WeaponID() == SSG08))
						{
							static bool bFlip = false;
							if (bFlip)
							{
								if (csgo::MainWeapon->WeaponID() == REVOLVER)
								{
								}
								else
									csgo::UserCmd->buttons &= ~IN_ATTACK;
							}
							bFlip = !bFlip;
						}
						if (Menu.Ragebot.FakeLatency)
						{
							if (!HNetchan)
							{
								DWORD ClientState = *(DWORD*)FakeLatency::ClientState;
								if (ClientState)
								{
									DWORD NetChannel = *(DWORD*)(*(DWORD*)FakeLatency::ClientState + 0x9C);
									if (NetChannel)
									{
										HNetchan = new VMT((DWORD**)NetChannel);
										HNetchan->HookVM((void*)Hooked_SendDatagram, 46);
										HNetchan->ApplyVMT();
									}
								}
							}
						}

						csgo::spread = csgo::MainWeapon->GetSpread() + (csgo::MainWeapon->GetInaccuracy() * 800);

						g_Misc->FixMovement();
						g_Misc->FixCmd();
						cmd = csgo::UserCmd;
						bSendPacket = csgo::SendPacket;
						grenade_prediction::instance().Tick(csgo::UserCmd->buttons);
						last_sendpacket = csgo::SendPacket;
					}
				}
			}
		}
	}
	return false;
}
