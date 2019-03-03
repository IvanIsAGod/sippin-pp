#pragma once
class CAimbot
{
public:
	bool HitChance(CBaseEntity * pCSTarget, CBaseCombatWeapon * pCSWeapon, QAngle qAngle, int chance);
	std::vector<Vector> MainHitbox(CBaseEntity * pTarget, VMatrix BoneMatrix[128]);
	void extrapolation();
	Vector RunAimScan(CBaseEntity * pTarget, float & simtime, Vector & origin);
	void DropTarget();
	void AutoZeus();
	void minwalk(CBaseCombatWeapon * weapon);
	void Run();
	void CompensateInaccuracies();
	int target_index = -1;
	float best_distance = 8192.f;
	bool aimbotted_in_current_tick;
	bool fired_in_that_tick;
	bool aimbotted = false;
	float current_aim_simulationtime;
	int current_minusticks;
	Vector current_aim_position;
	Vector current_aim_player_origin;
	CBaseEntity* pTarget;
	struct
	{
		int shoots, shoots_hit;
		int shotsmissed = shoots - shoots_hit;
	}data[65];
}; extern CAimbot* g_Aimbot;
