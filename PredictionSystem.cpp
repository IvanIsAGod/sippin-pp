#include "sdk.h"
#include "PredictionSystem.h"
#include "global.h"
#include "xor.h"

static char* cMoveData = nullptr;

void CPredictionSystem::EnginePrediction(CUserCmd* pCmd)
{
	if (!g_pMoveHelper || !pCmd || !csgo::LocalPlayer)
		return;

	CMoveData C_MoveData;

	//Let's back things up for later use
	iFlagsBackup = *csgo::LocalPlayer->GetFlags();
	iButtonsBackup = pCmd->buttons;

	//Set m_pCurrentCommand to cmd. We'll set this to nullptr later.
	csgo::LocalPlayer->SetCurrentCommand(pCmd);

	//Let's get some signatures now
	if (!m_pPredictionRandomSeed || !m_pSetPredictionPlayer)
	{
		m_pPredictionRandomSeed = *reinterpret_cast<int**>(FindPatternIDA((client_dll), "A3 ? ? ? ? 66 0F 6E 86") + 1);
		m_pSetPredictionPlayer = *reinterpret_cast<int**>(FindPatternIDA((client_dll), "89 35 ? ? ? ? F3 0F 10 48 20") + 2);
	}

	//We don't want to be calling this every tick, bad for framerate.
	//Thanks, zbe and Heep
	if (!cMoveData)
		cMoveData = (char*)(calloc(1, sizeof(CMoveData)));

	//Let's set it up so that it predicts ourselves
	g_pMoveHelper->SetHost(csgo::LocalPlayer);
	*m_pPredictionRandomSeed = MD5_PseudoRandom(pCmd->command_number) & 0x7FFFFFFF;
	*m_pSetPredictionPlayer = uintptr_t(csgo::LocalPlayer);

	//Let's set some global variables
	g_pGlobals->curtime = csgo::LocalPlayer->GetTickBase() * g_pGlobals->interval_per_tick;
	g_pGlobals->frametime = g_pGlobals->interval_per_tick;

	//Don't know is for exactly, but here's the relevant IDA picture of it. https://i.imgur.com/hT6caQV.png
	pCmd->buttons |= *reinterpret_cast< uint8_t* >(uintptr_t(csgo::LocalPlayer) + 0x3310);

	//This is for flashlights in older Source games, Thanks, friendly for the info
	if (pCmd->impulse)
		*reinterpret_cast< uint8_t* >(uintptr_t(csgo::LocalPlayer) + 0x31EC) = pCmd->impulse;

	//Here we're doing CBasePlayer::UpdateButtonState
	C_MoveData.m_nButtons = pCmd->buttons;
	int buttonsChanged = pCmd->buttons ^ *reinterpret_cast<int*>(uintptr_t(csgo::LocalPlayer) + 0x31E8);
	*reinterpret_cast<int*>(uintptr_t(csgo::LocalPlayer) + 0x31DC) = (uintptr_t(csgo::LocalPlayer) + 0x31E8);
	*reinterpret_cast<int*>(uintptr_t(csgo::LocalPlayer) + 0x31E8) = pCmd->buttons;
	*reinterpret_cast<int*>(uintptr_t(csgo::LocalPlayer) + 0x31E0) = pCmd->buttons & buttonsChanged;  //m_afButtonPressed ~ The changed ones still down are "pressed"
	*reinterpret_cast<int*>(uintptr_t(csgo::LocalPlayer) + 0x31E4) = buttonsChanged & ~pCmd->buttons; //m_afButtonReleased ~ The ones not down are "released"

	g_pGameMovement->StartTrackPredictionErrors(csgo::LocalPlayer);

	iTickBaseBackup = csgo::LocalPlayer->GetTickBase();

	g_pPrediction->SetupMove(csgo::LocalPlayer, pCmd, g_pMoveHelper, reinterpret_cast< CMoveData* >(cMoveData));
	g_pGameMovement->ProcessMovement(csgo::LocalPlayer, reinterpret_cast< CMoveData* >(cMoveData));
	g_pPrediction->FinishMove(csgo::LocalPlayer, pCmd, reinterpret_cast< CMoveData* >(cMoveData));

	//Let's override our tickbase with the backed up tickbase
	csgo::LocalPlayer->GetTickBase() = iTickBaseBackup;

	g_pGameMovement->FinishTrackPredictionErrors(csgo::LocalPlayer);

	//Let's nullify these here
	csgo::LocalPlayer->SetCurrentCommand(nullptr);
	*m_pPredictionRandomSeed = -1;
	*m_pSetPredictionPlayer = 0;
	g_pMoveHelper->SetHost(0);

	//Last but not least, set these to their backups 
	*csgo::LocalPlayer->GetFlags() = iFlagsBackup;
	pCmd->buttons = iButtonsBackup;
}



float flOldCurtime;
float flOldFrametime;

void CPredictionSystem::run_prediction(CUserCmd* cmd)
{
	static int flTickBase;
	static CUserCmd* pLastCmd;
	auto local_player = csgo::LocalPlayer;
	if (!local_player) return;

	// fix tickbase if game didnt render previous tick
	if (pLastCmd)
	{
		if (pLastCmd->hasbeenpredicted)
			flTickBase = local_player->GetTickBase();
		else
			++flTickBase;
	}

	// get random_seed as its 0 in clientmode->createmove

	static bool bInit = false;
	if (!bInit) {
		m_pPredictionRandomSeed = *reinterpret_cast<int**>(FindPatternIDA((client_dll), "A3 ? ? ? ? 66 0F 6E 86") + 1);
		bInit = true;
	}

	pLastCmd = cmd;
	flOldCurtime = g_pGlobals->curtime;
	flOldFrametime = g_pGlobals->frametime;
	*m_pPredictionRandomSeed = MD5_PseudoRandom(cmd->command_number) & 0x7FFFFFFF;
	g_pGlobals->curtime = flTickBase * g_pGlobals->interval_per_tick;
	g_pGlobals->frametime = g_pGlobals->interval_per_tick;

	g_pGameMovement->StartTrackPredictionErrors(local_player);

	CMoveData data;
	memset(&data, 0, sizeof(CMoveData));

	g_pMoveHelper->SetHost(local_player);
	g_pPrediction->SetupMove(local_player, cmd, g_pMoveHelper, &data);
	g_pGameMovement->ProcessMovement(local_player, &data);
	g_pPrediction->FinishMove(local_player, cmd, &data);
}

void CPredictionSystem::end_prediction(CUserCmd * cmd)
{
	auto local_player = csgo::LocalPlayer;
	if (!local_player) return

	g_pGameMovement->FinishTrackPredictionErrors(local_player);
	g_pMoveHelper->SetHost(0);

	*m_pPredictionRandomSeed = -1;

	g_pGlobals->curtime = flOldCurtime;
	g_pGlobals->frametime = flOldFrametime;
}