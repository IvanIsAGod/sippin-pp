#include "sdk.h"
#include "FixMove.h"
#include "global.h"
#include "Math.h"
CFixMove* g_FixMove = new CFixMove;

float DotProduct1(Vector v1, Vector v2)
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}

void Normalize(Vector &vecIn, Vector &vecOut)
{
	float flLen = vecIn.Length();

	if (flLen == 0) {
		vecOut.Init(0, 0, 1);
		return;
	}

	flLen = 1 / flLen;
	vecOut.Init(vecIn.x * flLen, vecIn.y * flLen, vecIn.z * flLen);
}

void CFixMove::Start(CUserCmd* cmd)
{
	QAngle angle = cmd->viewangles;
	angle.x = 0;
	forward = cmd->forwardmove;
	right = cmd->sidemove;
	up = cmd->upmove;
	oldAngle = cmd->viewangles;
	Math::AngleVectors(angle, viewforward, viewright, viewup);
}

void CFixMove::End(CUserCmd* cmd)
{
	QAngle angle = cmd->viewangles;
	angle.x = 0;
	Math::AngleVectors(angle, aimforward, aimright, aimup);
	Normalize(viewforward, vForwardNorm);
	Normalize(viewright, vRightNorm);
	Normalize(viewup, vUpNorm);
	cmd->forwardmove = DotProduct1(vForwardNorm * forward, aimforward) + DotProduct1(vRightNorm * right, aimforward) + DotProduct1(vUpNorm * up, aimforward);
	cmd->sidemove = DotProduct1(vForwardNorm * forward, aimright) + DotProduct1(vRightNorm * right, aimright) + DotProduct1(vUpNorm * up, aimright);

	if (cmd->viewangles.x < -90.f && cmd->viewangles.x > -1000 || cmd->viewangles.x > 90.f && cmd->viewangles.x <= 1000.f)
		cmd->forwardmove = -cmd->forwardmove;

	if (csgo::LocalPlayer->GetMoveType() == (int)MoveType_t::MOVETYPE_LADDER)
	{
		if (cmd->viewangles.x > 0)
			cmd->upmove = -cmd->upmove;
	}
}