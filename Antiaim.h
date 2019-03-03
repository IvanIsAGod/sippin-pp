#pragma once

class CAntiaim
{
public:
	bool choke;
	bool is_viable_target(CBaseEntity * pEntity);
	void SetSpeed(int speed);
	void FakeDuck(CUserCmd * cmd);
	void SlowWalk(CUserCmd * cmd);
	void Run(QAngle localview);
	void RunBeta(CUserCmd * cmd);
private:
	int jitter = 3;
	void DoDesync();
	void DoPitch(int choose);
	void DoPitch();
	void DoAntiAims();
}; extern CAntiaim* g_Antiaim;
