#pragma once

class CFixMove
{
public:
	void Start(CUserCmd* cmd);
	void End(CUserCmd* cmd);
private:
	float forward, right, up;
	Vector viewforward, viewright, viewup, aimforward, aimright, aimup, vForwardNorm, vRightNorm, vUpNorm;
	QAngle oldAngle;
}; extern CFixMove* g_FixMove;
