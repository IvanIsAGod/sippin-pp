#pragma once
#include "FixMove.h"
class CMisc
{
public:
	void RankReveal();
	void Bunnyhop();
	void WalkBotCM(Vector & oldang);
	void AutoStrafe();
	void fakeduck(CUserCmd * cmd, bool & bSendPackets);
	void WpnCfgData();
	void WpnCfgMenu();
	void edgejump(CUserCmd * cmd);
	void FixCmd();
	void FixMovement();
	void HandleClantag();
	void UpdateFlSettings();
	void FakeLag();
	void DoCircle();
	std::vector<Vector> path;
	float m_circle_yaw = 0;
}; extern CMisc* g_Misc;

