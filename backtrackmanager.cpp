#include "sdk.h"
#include "backtrackmanager.h"
#include "global.h"
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
bool CBackTrackManager::IsVisibleTick(RecordTick_t tick, bool smokecheck, bool bodycheck)
{
	CBaseEntity* pEntity = g_pEntitylist->GetClientEntity(tick.iIndex);
	if (!pEntity || !pEntity->isAlive() || !pEntity->IsPlayer())
		return false;

	bool bVisible = false;

	Vector vecPoints[] = {
	Vector(tick.vecMin.x, tick.vecMin.y, tick.vecMin.z),
	Vector(tick.vecMin.x, tick.vecMax.y, tick.vecMin.z),
	Vector(tick.vecMax.x, tick.vecMax.y, tick.vecMin.z),
	Vector(tick.vecMax.x, tick.vecMin.y, tick.vecMin.z),
	Vector(tick.vecMax.x, tick.vecMax.y, tick.vecMax.z),
	Vector(tick.vecMin.x, tick.vecMax.y, tick.vecMax.z),
	Vector(tick.vecMin.x, tick.vecMin.y, tick.vecMax.z),
	Vector(tick.vecMax.x, tick.vecMin.y, tick.vecMax.z)
	};
		for (Vector CheckPos : vecPoints)
		{
			Ray_t ray;
			ray.Init(csgo::LocalPlayer->GetEyePosition(), CheckPos);
			CTraceFilter filter;
			filter.pSkip1 = csgo::LocalPlayer;

			trace_t trace;
			g_pEngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &trace);
			if (trace.fraction > .95f)
			{
				bVisible = true;
				break;
			}
		}

	if (bVisible && bodycheck)
	{
		Ray_t ray;
		ray.Init(csgo::LocalPlayer->GetEyePosition(), pEntity->GetBonePos(8, tick.boneMatrix));
		CTraceFilter filter;
		filter.pSkip1 = csgo::LocalPlayer;

		trace_t trace;
		g_pEngineTrace->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &trace);
		bVisible = bVisible && trace.fraction > .95f;
	}

	//if (smokecheck)
	//	bVisible = bVisible && !U::LineGoesThroughSmoke(csgo::LocalPlayer->GetEyePosition(), pEntity->GetBonePosition(8, tick.boneMatrix));

	return bVisible;
}

void CBackTrackManager::SetRecord(CBaseEntity * pEntity, RecordTick_t tick)
{
	if (pEntity->GetIndex() != tick.iIndex) 
		return;	
	pEntity->InvalidateBoneCache();
	pEntity->SetAbsOrigin(tick.vecOrigin);
	pEntity->SetAngle2(tick.angAbsAngle);
}

void CBackTrackManager::UpdateTicks()
{
	for (int i = 0; i < g_pGlobals->maxClients; i++)
	{
		CBaseEntity* pEntity = g_pEntitylist->GetClientEntity(i);
		if (!pEntity->IsValid())
			continue;
		RecordedTicks[pEntity->GetIndex()].push_back(RecordTick_t(pEntity));
		while (!RecordedTicks[pEntity->GetIndex()].empty() && abs(RecordedTicks[pEntity->GetIndex()][0].flSimulationTime - pEntity->GetSimulationTime()) > 1)
			RecordedTicks[pEntity->GetIndex()].erase(RecordedTicks[pEntity->GetIndex()].begin());
	}
}

int CBackTrackManager::GetEstimatedServerTickCount(float latency)
{
	return TIME_TO_TICKS(latency) + 1 + g_pGlobals->tickcount;
}

float CBackTrackManager::GetLerpTime()
{
	static ConVar* minupdate, *maxupdate, *updaterate, *interprate, *cmin, *cmax, *interp;
	if (!minupdate)
		minupdate = g_pCvar->FindVar(XorStr("sv_minupdaterate"));
	if (!maxupdate)
		maxupdate = g_pCvar->FindVar(XorStr("sv_maxupdaterate"));
	if (!updaterate)
		updaterate = g_pCvar->FindVar(XorStr("cl_updaterate"));
	if (!interprate)
		interprate = g_pCvar->FindVar(XorStr("cl_interp_ratio"));
	if (!cmin)
		cmin = g_pCvar->FindVar(XorStr("sv_client_min_interp_ratio"));
	if (!cmax)
		cmax = g_pCvar->FindVar(XorStr("sv_client_max_interp_ratio"));
	if (!interp)
		interp = g_pCvar->FindVar(XorStr("cl_interp"));

	float UpdateRate = updaterate->GetValue();
	float LerpRatio = interprate->GetValue();

	return std::min<float>(LerpRatio / UpdateRate, interp->GetValue());
}

bool CBackTrackManager::IsValidTick(RecordTick_t tick)
{
	float outlatency;
	float inlatency;
	INetChannelInfo *nci = g_pEngine->GetNetChannelInfo();
	if (nci)
	{
		inlatency = nci->GetLatency(FLOW_INCOMING);
		outlatency = nci->GetLatency(FLOW_OUTGOING);
	}
	else
		inlatency = outlatency = 0.0f;

	float totaloutlatency = outlatency;
	if (nci)
		totaloutlatency += nci->GetAvgLatency(FLOW_OUTGOING); //net_graph method

	float servertime = TICKS_TO_TIME(GetEstimatedServerTickCount(outlatency + inlatency));

	float correct = clamp<float>(totaloutlatency + GetLerpTime(), .0f, 1.0f);
	int iTargetTick = TIME_TO_TICKS(tick.flSimulationTime);
	float deltaTime = correct - (servertime - TICKS_TO_TIME(iTargetTick));
	return (fabsf(deltaTime) <= 0.2f);
}

RecordTick_t CBackTrackManager::GetLastValidRecord(CBaseEntity * pEntity)
{
	for (int i = 0; i < RecordedTicks[pEntity->GetIndex()].size(); i++)
	{
		if (IsValidTick(RecordedTicks[pEntity->GetIndex()][i]))
			return RecordedTicks[pEntity->GetIndex()][i];
	}
	return RecordTick_t(pEntity);
}

std::vector<RecordTick_t> CBackTrackManager::GetLastValidRecords()
{
	std::vector<RecordTick_t> records;
	for (int i = 0; i < g_pGlobals->maxClients; i++)
	{
		CBaseEntity* pEntity = g_pEntitylist->GetClientEntity(i);
		if (!pEntity->IsValid())
			continue;
		records.push_back(GetLastValidRecord(pEntity));
	}
	return records;
}

RecordTick_t::RecordTick_t(CBaseEntity * pEntity)
{
	iIndex = pEntity->GetIndex();
	flSimulationTime = pEntity->GetSimulationTime();
	vecOrigin = pEntity->GetAbsOrigin();
	angEyeAngle = pEntity->GetEyeAngles();
	angAbsAngle = pEntity->GetAbsAngles();
	vecVelocity = pEntity->GetVelocity();
	iFlags = *pEntity->GetFlags();
	flLowerBodyYaw = pEntity->LowerBodyYaw();
	iLayerCount = pEntity->GetNumAnimOverlays();

	for (int i = 0; i < 24; i++)
		flPoseParameter[i] = *(float*)((DWORD)pEntity + offys.m_flPoseParameter + sizeof(float) * i);
	pEntity->SetupBones(boneMatrix, MAXSTUDIOBONES, BONE_USED_BY_HITBOX, g_pGlobals->curtime);
	
	for (int i = 0; i < iLayerCount; i++)
	{
		AnimationLayer Layer = pEntity->GetAnimOverlay2(i);
		cAnimationLayer[i].m_flCycle = Layer.m_flCycle;
		cAnimationLayer[i].m_flWeight = Layer.m_flWeight;
		cAnimationLayer[i].m_nOrder = Layer.m_nOrder;
		cAnimationLayer[i].m_nSequence = Layer.m_nSequence;
	}
	pEntity->ComputeHitboxSurroundingBox(&vecMin, &vecMax);
}

