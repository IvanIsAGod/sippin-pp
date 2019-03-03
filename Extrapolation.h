#pragma once
#include "sdk.h"
#include "Aimbot.h"
#include "global.h"
#include "Menu.h"
#include "Math.h"
#include "GameUtils.h"
#include "Autowall.h"
#include "Antiaim.h"
#include "BacktrackingHelper.h"
#include "NoSpread.h"
#include "PredictionSystem.h"

struct simulation_record
{
	Vector origin;
	Vector velocity;
	Vector acceleration;
	float simulation_time;
	float simulation_time_increasment_per_tick;
	float update_time;

};

class CExtrapolation
{
public:
	simulation_record SimRecord[64][7];
	int GetLatencyTicks();
	void AirAccelerate(CBaseEntity * pPlayer, Vector & wishdir, float wishspeed, float accel, Vector & velo);
	float simulationtimeincreasement(simulation_record pre_latest_record, simulation_record latest_record, float difference);
	void UpdateRecords(CBaseEntity * pPlayer);
	void Run(CBaseEntity * pPlayer, Vector & position, float & simtime, Vector velocity);

}; extern CExtrapolation* g_Extrapolation;