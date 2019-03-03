#include "sdk.h"
#include "BacktrackingHelper.h"
#include "Aimbot.h"
#include "global.h"
#include "xor.h"
#include "Math.h"
#include "PredictionSystem.h"



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
#define TICK_INTERVAL			( g_pGlobals->interval_per_tick )
#define TIME_TO_TICKS( dt )		( floorf(( 0.5f + (float)(dt) / TICK_INTERVAL ) ) )
#define TICKS_TO_TIME( t )		( TICK_INTERVAL *( t ) )
CBacktrackHelper* g_BacktrackHelper = new CBacktrackHelper;
CInterpolation* g_Interpolation = new CInterpolation;

bool CBacktrackHelper::IsTickValid(tick_record record)
{
	float correct = 0;

	correct += g_pEngine->GetNetChannelInfo()->GetLatency(FLOW_OUTGOING);
	correct += g_pEngine->GetNetChannelInfo()->GetLatency(FLOW_INCOMING);
	correct += g_BacktrackHelper->GetLerpTime();

	static ConVar* sv_maxunlag = g_pCvar->FindVar("sv_maxunlag");
	correct = clamp<float>(correct, 0.0f, sv_maxunlag->GetFloat());

	float deltaTime = correct - (g_pGlobals->curtime - record.m_flSimulationTime);

	float latecy = Menu.Ragebot.FakeLatencyAmount;
	float ping = Menu.Ragebot.FakeLatency ? latecy : 0.2f;

	return fabsf(deltaTime) < ping;
}

static ConVar* bigUdRate = nullptr;
static ConVar* minUdRate = nullptr;
static ConVar* maxUdRate = nullptr;
static ConVar* interpRatio = nullptr;
static ConVar* clInterp = nullptr;
static ConVar* minInterp = nullptr;
static ConVar* maxInterp = nullptr;

float CBacktrackHelper::GetLerpTime()
{
	if (!bigUdRate)
		bigUdRate = g_pCvar->FindVar("cl_updaterate");
	if (!minUdRate)
		minUdRate = g_pCvar->FindVar("sv_minupdaterate");
	if (!maxUdRate)
		maxUdRate = g_pCvar->FindVar("sv_maxupdaterate");
	if (!interpRatio)
		interpRatio = g_pCvar->FindVar("cl_interp_ratio");
	if (!clInterp)
		clInterp = g_pCvar->FindVar("cl_interp");
	if (!minInterp)
		minInterp = g_pCvar->FindVar("sv_client_min_interp_ratio");
	if (!maxInterp)
		maxInterp = g_pCvar->FindVar("sv_client_max_interp_ratio");

	float updateRate = bigUdRate->GetFloat();

	if (minUdRate && maxUdRate)
		updateRate = clamp(updateRate, (int)minUdRate->GetFloat(), (int)maxUdRate->GetFloat());

	float ratio = interpRatio->GetFloat();

	float lerp = clInterp->GetFloat();

	if (ratio == 0)
		ratio = 1.f;

	if (minInterp && maxInterp && minInterp->GetFloat() != 1)
		ratio = clamp(ratio, minInterp->GetFloat(), maxInterp->GetFloat());

	return max(lerp, ratio / updateRate);
}
float CBacktrackHelper::GetEstimateServerTime()
{
	double v0; // st7@0
	INetChannelInfo* v1; // esi@1
	INetChannelInfo* v2; // eax@1
	float v3; // ST08_4@1
	float v4; // ST0C_4@1

	v1 = (INetChannelInfo*)g_pEngine->GetNetChannelInfo();
	v2 = (INetChannelInfo*)g_pEngine->GetNetChannelInfo();

	v3 = v1->GetAvgLatency(Typetype_t::TYPE_LOCALPLAYER);
	v4 = v2->GetAvgLatency(Typetype_t::TYPE_GENERIC);

	//return floorf(((v3 + v4) / g_pGlobals->interval_per_tick) + 0.5f) + 1 + csgo::UserCmdForBacktracking->tick_count;*/

	return v3 + v4 + TICKS_TO_TIME(1) + TICKS_TO_TIME(csgo::UserCmdForBacktracking->tick_count);

}

float CBacktrackHelper::GetNetworkLatency()
{
	// Get true latency
	INetChannelInfo *nci = g_pEngine->GetNetChannelInfo();
	if (nci)
	{
		//float IncomingLatency = nci->GetAvgLatency(FLOW_INCOMING); //ppl say use only this one, but meh
		float OutgoingLatency = nci->GetLatency(0);
		return OutgoingLatency;
	}
	return 0.0f;
}

void CBacktrackHelper::UpdateBacktrackRecords(CBaseEntity* pPlayer)
{
	int i = pPlayer->Index();
	for (int j = g_BacktrackHelper->PlayerRecord[i].records.size() - 1; j >= 0; j--)
	{
		tick_record rec = g_BacktrackHelper->PlayerRecord[i].records.at(j);
		float amount = Menu.Ragebot.FakeLatencyAmount;
		float ping = Menu.Ragebot.FakeLatency ? amount : 0.2f;
		if (rec.m_flSimulationTime < g_pGlobals->curtime - ping)
			g_BacktrackHelper->PlayerRecord[i].records.erase(g_BacktrackHelper->PlayerRecord[i].records.begin() + j);
	}

	static Vector old_origin[64];
	if (PlayerRecord[i].records.size() > 0 && pPlayer->GetSimulationTime() == PlayerRecord[i].records.back().m_flSimulationTime) //already got such a record
		return;

	for (int j = g_BacktrackHelper->PlayerRecord[i].records.size() - 1; j >= 0; j--)
	{
		tick_record rec = g_BacktrackHelper->PlayerRecord[i].records.at(j);

		switch (Menu.Ragebot.TickType)
		{
		case 0:
			rec = g_BacktrackHelper->PlayerRecord[i].records.at(0);
			break;
		case 1:
			rec = g_BacktrackHelper->PlayerRecord[i].records.at(j);
			break;
		}

		if (PlayerRecord[i].records.size() > 0 && rec.m_flSimulationTime > pPlayer->GetSimulationTime())//Invalid lag record, maybe from diffrent game?
		{
			PlayerRecord[i].records.clear();
			return;
		}
	}

	Vector cur_origin = pPlayer->GetOrigin();
	Vector v = cur_origin - old_origin[i];
	bool breaks_lagcomp = v.LengthSqr() > 4096.f;
	old_origin[i] = cur_origin;
	tick_record new_record;

	CBaseCombatWeapon* pWeapon = pPlayer->GetWeapon();

	new_record.needs_extrapolation = breaks_lagcomp;
	new_record.m_nFlags = *pPlayer->GetFlags();
	new_record.m_absangles = pPlayer->GetAbsAngles();

	new_record.m_flSimulationTime = pPlayer->GetSimulationTime();
	new_record.m_vecAbsOrigin = pPlayer->GetAbsOrigin();
	new_record.m_vecOrigin = pPlayer->GetOrigin();
	new_record.m_vecVelocity = pPlayer->GetVelocity();
	new_record.m_flUpdateTime = g_pGlobals->curtime;
	new_record.backtrack_time = new_record.m_flSimulationTime + GetLerpTime();

	if (pWeapon)
	{
		if (PlayerRecord[i].ShotTime != pWeapon->GetAccuracyPenalty())
		{
			new_record.shot_backtrack = true;
			PlayerRecord[i].ShotTime = pWeapon->GetLastShotTime();
		}
		else
			new_record.shot_backtrack = false;
	}
	else
	{
		new_record.shot_backtrack = false;
		PlayerRecord[i].ShotTime = 0.f;
	}

	for (int i = 0; i < 24; i++)
		new_record.m_flPoseParameter[i] = *(float*)((DWORD)pPlayer + offys.m_flPoseParameter + sizeof(float) * i);

	int sequence = pPlayer->GetSequence();
	if (sequence == Activity::ACT_PLAYER_IDLE_FIRE || sequence == Activity::ACT_PLAYER_RUN_FIRE || sequence == Activity::ACT_PLAYER_WALK_FIRE || sequence == Activity::ACT_PLAYER_CROUCH_FIRE || sequence == Activity::ACT_PLAYER_CROUCH_WALK_FIRE)
	{
		new_record.shot_in_that_record = true;
	}

	pPlayer->SetupBones(new_record.boneMatrix, 128, 256, g_pGlobals->curtime);
	PlayerRecord[i].records.push_back(new_record);
}

static std::deque<CIncomingSequence>sequences;
static int lastincomingsequencenumber;
int Real_m_nInSequencenumber;

void CBacktrackHelper::UpdateIncomingSequences() // fake latency
{
	if (!FakeLatency::ClientState || FakeLatency::ClientState == 0)
		return;
	DWORD ClientState = (DWORD)*(DWORD*)FakeLatency::ClientState;
	if (ClientState)
	{
		INetChannel *netchan = (INetChannel*)*(DWORD*)(ClientState + 0x9C);
		if (netchan)
		{
			if (netchan->m_nInSequenceNr > lastincomingsequencenumber)
			{
				lastincomingsequencenumber = netchan->m_nInSequenceNr;
				sequences.push_front(CIncomingSequence(netchan->m_nInReliableState, netchan->m_nOutReliableState, netchan->m_nInSequenceNr, g_pGlobals->realtime));
			}

			if (sequences.size() > 2048)
				sequences.pop_back();
		}
	}
}

int CBacktrackHelper::GetLatencyTicks()
{
	double v0; // st7@0
	INetChannelInfo* v1; // esi@1
	INetChannelInfo* v2; // eax@1
	float v3; // ST08_4@1
	float v4; // ST0C_4@1

	v1 = (INetChannelInfo*)g_pEngine->GetNetChannelInfo();
	v2 = (INetChannelInfo*)g_pEngine->GetNetChannelInfo();

	v3 = v1->GetAvgLatency(Typetype_t::TYPE_LOCALPLAYER);
	v4 = v2->GetAvgLatency(Typetype_t::TYPE_GENERIC);

	float interval_per_tick = 1.0f / g_pGlobals->interval_per_tick;

	return floorf(((v3 + v4) * interval_per_tick) + 0.5f);
}

void CBacktrackHelper::ClearIncomingSequences()
{
	sequences.clear();
}

void CBacktrackHelper::AddLatencyToNetchan(INetChannel *netchan, float Latency)
{
	for (auto& seq : sequences)
	{
		if (g_pGlobals->realtime - seq.curtime >= Latency || g_pGlobals->realtime - seq.curtime > 1)
		{
			netchan->m_nInReliableState = seq.inreliablestate;
			netchan->m_nInSequenceNr = seq.sequencenr;
			break;
		}
	}
}