#pragma once
struct tick_record
{
	Vector m_vecOrigin;//0 //4 // 8
	bool shot_backtrack;
	bool bFakelagging;
	float m_flSimulationTime; //12
	int32_t m_nFlags;
	Vector m_absangles;
	Vector m_vecMins;
	Vector m_vecMax;			  //void SetAbsOrigin;				  //y->28//z->32
	float m_flUpdateTime;//36
	Vector m_vecVelocity;//40 //44 //48
	std::array<float, 24> ragpos;
	float m_flPoseParameter[24];//52
	float backtrack_time;
	Vector m_vecAbsOrigin;
	bool shot_in_that_record = false;
	bool needs_extrapolation = false;
	/*my addition*/
	VMatrix boneMatrix[128];
};
struct player_record
{
	//std::vector<tick_record> records;
	std::vector<tick_record> records;
	float ShotTime;
	int Flags;
	float unknown;
};

struct CIncomingSequence
{
	CIncomingSequence::CIncomingSequence(int instate, int outstate, int seqnr, float time)
	{
		inreliablestate = instate;
		outreliablestate = outstate;
		sequencenr = seqnr;
		curtime = time;
	}
	int inreliablestate;
	int outreliablestate;
	int sequencenr;
	float curtime;
};

namespace FakeLatency
{
	extern DWORD ClientState;
}

class CBacktrackHelper
{
public:
	player_record PlayerRecord[64];

	void AnimationFix(ClientFrameStage_t curstage);
	float GetEstimateServerTime();
	float GetNetworkLatency();
	bool IsTickValid(tick_record record);
	float GetLerpTime();
	void UpdateBacktrackRecords(CBaseEntity* pPlayer);
	void UpdateIncomingSequences();
	int GetLatencyTicks();
	void ClearIncomingSequences();
	void AddLatencyToNetchan(INetChannel *netchan, float Latency);
}; extern CBacktrackHelper* g_BacktrackHelper;

class CInterpolation {
	VarMapping_t* GetVarMap(void* pBaseEntity) {
		return reinterpret_cast<VarMapping_t*>((DWORD)pBaseEntity + 0x24);
	}
public:
	void disable_interpolation(CBaseEntity* ent)
	{
		VarMapping_t* map = GetVarMap(ent);
		if (!map) return;
		for (int i = 0; i < map->m_nInterpolatedEntries; i++) {
			VarMapEntry_t *e = &map->m_Entries[i];
			e->m_bNeedsToInterpolate = false;
		}
	}
}; extern CInterpolation* g_Interpolation;