#include "sdk.h"
#include "Misc.h"
#include "global.h"
#include <chrono>
#include "GameUtils.h"
#include "Math.h"
#include "xor.h"
#include "Draw.h"
#include "Aimbot.h"
#include <chrono>
CMisc* g_Misc = new CMisc;
//using RevealAllFn = void(*)(int); Double V fix: Can't use this syntax with my VS version!
#define CheckIfNonValidNumber(x) (fpclassify(x) == FP_INFINITE || fpclassify(x) == FP_NAN || fpclassify(x) == FP_SUBNORMAL)
typedef void(*RevealAllFn)(int);
RevealAllFn fnReveal;
void CMisc::RankReveal()
{
	if (!Menu.Visuals.Rank)
		return;

	if (!fnReveal)
		fnReveal = (RevealAllFn)Utilities::Memory::FindPattern(XorStr(client_dll), (PBYTE)XorStr("\x55\x8B\xEC\x8B\x0D\x00\x00\x00\x00\x68\x00\x00\x00\x00"), XorStr("xxxxx????x????"));

	int iBuffer[1];

	if (csgo::UserCmd->buttons & IN_SCORE)
		fnReveal(iBuffer[1]);
}

void CMisc::Bunnyhop()
{
	if ((*csgo::LocalPlayer->GetFlags() & FL_ONGROUND) && csgo::walkbotenabled)
		csgo::UserCmd->buttons = IN_JUMP;

	if (Menu.Misc.AutoJump/* && Cvar->FindVar("sv_enablebunnyhopping")->GetValue() != 1*/)
	{
		static auto bJumped = false;
		static auto bFake = false;
		if (!bJumped && bFake)
		{
			bFake = false;
			csgo::UserCmd->buttons |= IN_JUMP;
		}
		else if (csgo::UserCmd->buttons & IN_JUMP)
		{
			if (*csgo::LocalPlayer->GetFlags() & FL_ONGROUND)
			{
				bJumped = true;
				bFake = true;
			}
			else
			{
				csgo::UserCmd->buttons &= ~IN_JUMP;
				bJumped = false;
			}
		}
		else
		{
			bJumped = false;
			bFake = false;
		}
	}
}

float RightMovement;
bool IsActive;
float StrafeAngle;



void CMisc::WalkBotCM(Vector& oldang)
{
	static bool active = false;
	static bool firstrun = true;
	bool walkbotting = false;
	float wbdistance = 0;

	if (GetAsyncKeyState(Menu.Misc.WalkbotStart) & 0x1)
		active = !active;

	Vector localpos = csgo::LocalPlayer->GetAbsOrigin();

	if (GetAsyncKeyState(Menu.Misc.WalkbotSet) & 0x1)
	{
		csgo::walkpoints.push_back(localpos);
		csgo::wbpoints++;
	}
	else if (GetAsyncKeyState(Menu.Misc.WalkbotDelete) & 0x1)
	{
		if (csgo::walkpoints.size() > 0)
			csgo::walkpoints.pop_back();

		if (csgo::wbpoints > -1)
			csgo::wbpoints--;
	}

	if (csgo::NewRound)
		firstrun = true;

	if (!active)
	{
		csgo::wbcurpoint = 0;
		firstrun = true;
		csgo::walkbotenabled = false;
		return;
	}

	csgo::walkbotenabled = true;
	walkbotting = true;

	if (csgo::wbcurpoint > csgo::wbpoints)
		csgo::wbcurpoint = 0;

	if (csgo::wbpoints == -1)
		return;

	Vector point = csgo::walkpoints[csgo::wbcurpoint];
	wbdistance = fabs(Vector(localpos - point).Length2D());

	if (wbdistance < 25.f)
		csgo::wbcurpoint++;

	if (csgo::wbcurpoint > csgo::wbpoints)
		csgo::wbcurpoint = 0;

	if (csgo::wbpoints == -1)
		return;

	point = csgo::walkpoints[csgo::wbcurpoint];
	wbdistance = fabs(Vector(localpos - point).Length2D());

	if (csgo::wbcurpoint == 0 && firstrun == true)
	{
		float lowdist = wbdistance;

		for (int i = 0; i < csgo::wbpoints; i++)
		{
			Vector pt = csgo::walkpoints[i];
			float dist = fabs(Vector(localpos - pt).Length2D());

			if (dist < lowdist)
			{
				lowdist = dist;
				csgo::wbcurpoint = i;
				point = csgo::walkpoints[csgo::wbcurpoint];
				wbdistance = dist;
			}
		}

		firstrun = false;
	}

	static Vector lastang;

	Vector curang = GameUtils::CalculateAngle(csgo::LocalPlayer->GetEyePosition(), point);
	curang.x = 0.f;

	Math::NormalizeVector(curang);
	Math::ClampAngles(curang);
	lastang = curang;

	csgo::StrafeAngle = curang;
}

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

void CMisc::AutoStrafe()
{
	if (!Menu.Misc.AutoStrafe)
		return;

	if (!GetAsyncKeyState(VK_SPACE))
		return;
	if (csgo::LocalPlayer->GetVelocity().Length2D() < 32)
		return;

	/* AW*/
	static int old_yaw;

	auto get_velocity_degree = [](float length_2d)
	{
		auto tmp = RAD2DEG(atan(30.f / length_2d));

		if (CheckIfNonValidNumber(tmp) || tmp > 90.f)
			return 90.f;

		else if (tmp < 0.f)
			return 0.f;

		else
			return tmp;
	};

	auto velocity = csgo::LocalPlayer->GetVelocity();
	velocity.z = 0;

	static auto flip = false;
	auto turn_direction_modifier = (flip) ? 1.f : -1.f;
	flip = !flip;

	if (*csgo::LocalPlayer->GetFlags() & FL_ONGROUND || csgo::LocalPlayer->GetMoveType() == MOVETYPE_LADDER)
		return;

	if (csgo::UserCmd->forwardmove > 0.f)
		csgo::UserCmd->forwardmove = 0.f;

	auto velocity_length_2d = velocity.Length2D();

	auto strafe_angle = RAD2DEG(atan(15.f / velocity_length_2d));

	if (strafe_angle > 90.f)
		strafe_angle = 90.f;

	else if (strafe_angle < 0.f)
		strafe_angle = 0.f;

	Vector Buffer(0, csgo::StrafeAngle.y - old_yaw, 0);
	Buffer.y = Math::NormalizeYaw(Buffer.y);

	int yaw_delta = Buffer.y;
	old_yaw = csgo::StrafeAngle.y;

	if (yaw_delta > 0.f)
		csgo::UserCmd->sidemove = -450.f;

	else if (yaw_delta < 0.f)
		csgo::UserCmd->sidemove = 450.f;

	auto abs_yaw_delta = abs(yaw_delta);

	if (abs_yaw_delta <= strafe_angle || abs_yaw_delta >= 30.f)
	{
		Vector velocity_angles;
		Math::VectorAngles(velocity, velocity_angles);

		Buffer = Vector(0, csgo::StrafeAngle.y - velocity_angles.y, 0);
		Buffer.y = Math::NormalizeYaw(Buffer.y);
		int velocityangle_yawdelta = Buffer.y;

		auto velocity_degree = get_velocity_degree(velocity_length_2d) * 7; // retrack value, for teleporters

		if (velocityangle_yawdelta <= velocity_degree || velocity_length_2d <= 15.f)
		{
			if (-(velocity_degree) <= velocityangle_yawdelta || velocity_length_2d <= 15.f)
			{
				csgo::StrafeAngle.y += (strafe_angle * turn_direction_modifier);
				csgo::UserCmd->sidemove = 450.f * turn_direction_modifier;
			}
			else
			{
				csgo::StrafeAngle.y = velocity_angles.y - velocity_degree;
				csgo::UserCmd->sidemove = 450.f;
			}
		}
		else
		{
			csgo::StrafeAngle.y = velocity_angles.y + velocity_degree;
			csgo::UserCmd->sidemove = -450.f;
		}
	}
	csgo::UserCmd->buttons &= ~(IN_MOVELEFT | IN_MOVERIGHT | IN_FORWARD | IN_BACK);
	if (csgo::UserCmd->sidemove <= 0.0)
		csgo::UserCmd->buttons |= IN_MOVELEFT;
	else
		csgo::UserCmd->buttons |= IN_MOVERIGHT;

	if (csgo::UserCmd->forwardmove <= 0.0)
		csgo::UserCmd->buttons |= IN_BACK;
	else
		csgo::UserCmd->buttons |= IN_FORWARD;
}

void CMisc::fakeduck(CUserCmd* cmd, bool &bSendPackets)
{
	static bool counter = false;
	static int counters = 0;
	if (counters == 9)
	{
		counters = 0;
		counter = !counter;
	}
	counters++;
	if (counter)
	{
		cmd->buttons |= IN_DUCK;
		bSendPackets = true;
	}
	else
		cmd->buttons &= ~IN_DUCK;
}

void CMisc::WpnCfgData()
{
	auto localp = csgo::LocalPlayer;
	auto weapon = csgo::MainWeapon;
	if (weapon->IsAuto() && Menu.Ragebot.Auto.enable)
	{
		Menu.Ragebot.headhitscan = Menu.Ragebot.Auto.headhitscan;
		Menu.Ragebot.chesthitscan = Menu.Ragebot.Auto.chesthitscan;
		Menu.Ragebot.stomachhitscan = Menu.Ragebot.Auto.stomachhitscan;
		Menu.Ragebot.armshitscan = Menu.Ragebot.Auto.armshitscan;
		Menu.Ragebot.legshitscan = Menu.Ragebot.Auto.legshitscan;
		Menu.Ragebot.feethitscan = Menu.Ragebot.Auto.feethitscan;
		Menu.Ragebot.PositionAdjustment = Menu.Ragebot.Auto.PositionAdjustment;
		Menu.Ragebot.extrapolation = Menu.Ragebot.Auto.extrapolation;
		Menu.Ragebot.delayshot = Menu.Ragebot.Auto.delayshot;
		Menu.Ragebot.Fakehitbox = Menu.Ragebot.Auto.Fakehitbox;
		Menu.Ragebot.AutomaticStop = Menu.Ragebot.Auto.AutomaticStop;
		Menu.Ragebot.headscale = Menu.Ragebot.Auto.headscale;
		Menu.Ragebot.bodyscale = Menu.Ragebot.Auto.bodyscale;
		Menu.Ragebot.Mindamage = Menu.Ragebot.Auto.Mindamage;
		Menu.Ragebot.Minhitchance = Menu.Ragebot.Auto.Minhitchance;
		Menu.Ragebot.TickType = Menu.Ragebot.Auto.TickType;
		Menu.Ragebot.baimifair = Menu.Ragebot.Auto.baimifair;
		Menu.Ragebot.baimiflethal = Menu.Ragebot.Auto.baimiflethal;
		Menu.Ragebot.baimafterx = Menu.Ragebot.Auto.baimafterx;
	}
	else if (weapon->IsScout() && Menu.Ragebot.Scout.enable)
	{
		Menu.Ragebot.headhitscan = Menu.Ragebot.Scout.headhitscan;
		Menu.Ragebot.chesthitscan = Menu.Ragebot.Scout.chesthitscan;
		Menu.Ragebot.stomachhitscan = Menu.Ragebot.Scout.stomachhitscan;
		Menu.Ragebot.armshitscan = Menu.Ragebot.Scout.armshitscan;
		Menu.Ragebot.legshitscan = Menu.Ragebot.Scout.legshitscan;
		Menu.Ragebot.feethitscan = Menu.Ragebot.Scout.feethitscan;
		Menu.Ragebot.PositionAdjustment = Menu.Ragebot.Scout.PositionAdjustment;
		Menu.Ragebot.extrapolation = Menu.Ragebot.Scout.extrapolation;
		Menu.Ragebot.delayshot = Menu.Ragebot.Scout.delayshot;
		Menu.Ragebot.Fakehitbox = Menu.Ragebot.Scout.Fakehitbox;
		Menu.Ragebot.AutomaticStop = Menu.Ragebot.Scout.AutomaticStop;
		Menu.Ragebot.headscale = Menu.Ragebot.Scout.headscale;
		Menu.Ragebot.bodyscale = Menu.Ragebot.Scout.bodyscale;
		Menu.Ragebot.Mindamage = Menu.Ragebot.Scout.Mindamage;
		Menu.Ragebot.Minhitchance = Menu.Ragebot.Scout.Minhitchance;
		Menu.Ragebot.TickType = Menu.Ragebot.Scout.TickType;
		Menu.Ragebot.baimifair = Menu.Ragebot.Scout.baimifair;
		Menu.Ragebot.baimiflethal = Menu.Ragebot.Scout.baimiflethal;
		Menu.Ragebot.baimafterx = Menu.Ragebot.Scout.baimafterx;
	}
	else if (weapon->IsAWP() && Menu.Ragebot.AWP.enable)
	{
		Menu.Ragebot.headhitscan = Menu.Ragebot.AWP.headhitscan;
		Menu.Ragebot.chesthitscan = Menu.Ragebot.AWP.chesthitscan;
		Menu.Ragebot.stomachhitscan = Menu.Ragebot.AWP.stomachhitscan;
		Menu.Ragebot.armshitscan = Menu.Ragebot.AWP.armshitscan;
		Menu.Ragebot.legshitscan = Menu.Ragebot.AWP.legshitscan;
		Menu.Ragebot.feethitscan = Menu.Ragebot.AWP.feethitscan;
		Menu.Ragebot.PositionAdjustment = Menu.Ragebot.AWP.PositionAdjustment;
		Menu.Ragebot.extrapolation = Menu.Ragebot.AWP.extrapolation;
		Menu.Ragebot.delayshot = Menu.Ragebot.AWP.delayshot;
		Menu.Ragebot.Fakehitbox = Menu.Ragebot.AWP.Fakehitbox;
		Menu.Ragebot.AutomaticStop = Menu.Ragebot.AWP.AutomaticStop;
		Menu.Ragebot.headscale = Menu.Ragebot.AWP.headscale;
		Menu.Ragebot.bodyscale = Menu.Ragebot.AWP.bodyscale;
		Menu.Ragebot.Mindamage = Menu.Ragebot.AWP.Mindamage;
		Menu.Ragebot.Minhitchance = Menu.Ragebot.AWP.Minhitchance;
		Menu.Ragebot.TickType = Menu.Ragebot.AWP.TickType;
		Menu.Ragebot.baimifair = Menu.Ragebot.AWP.baimifair;
		Menu.Ragebot.baimiflethal = Menu.Ragebot.AWP.baimiflethal;
		Menu.Ragebot.baimafterx = Menu.Ragebot.AWP.baimafterx;
	}
	else if (weapon->IsLightPistol() && Menu.Ragebot.Pistols.enable)
	{
		Menu.Ragebot.headhitscan = Menu.Ragebot.Pistols.headhitscan;
		Menu.Ragebot.chesthitscan = Menu.Ragebot.Pistols.chesthitscan;
		Menu.Ragebot.stomachhitscan = Menu.Ragebot.Pistols.stomachhitscan;
		Menu.Ragebot.armshitscan = Menu.Ragebot.Pistols.armshitscan;
		Menu.Ragebot.legshitscan = Menu.Ragebot.Pistols.legshitscan;
		Menu.Ragebot.feethitscan = Menu.Ragebot.Pistols.feethitscan;
		Menu.Ragebot.PositionAdjustment = Menu.Ragebot.Pistols.PositionAdjustment;
		Menu.Ragebot.extrapolation = Menu.Ragebot.Pistols.extrapolation;
		Menu.Ragebot.delayshot = Menu.Ragebot.Pistols.delayshot;
		Menu.Ragebot.Fakehitbox = Menu.Ragebot.Pistols.Fakehitbox;
		Menu.Ragebot.AutomaticStop = Menu.Ragebot.Pistols.AutomaticStop;
		Menu.Ragebot.headscale = Menu.Ragebot.Pistols.headscale;
		Menu.Ragebot.bodyscale = Menu.Ragebot.Pistols.bodyscale;
		Menu.Ragebot.Mindamage = Menu.Ragebot.Pistols.Mindamage;
		Menu.Ragebot.Minhitchance = Menu.Ragebot.Pistols.Minhitchance;
		Menu.Ragebot.TickType = Menu.Ragebot.Pistols.TickType;
		Menu.Ragebot.baimifair = Menu.Ragebot.Pistols.baimifair;
		Menu.Ragebot.baimiflethal = Menu.Ragebot.Pistols.baimiflethal;
		Menu.Ragebot.baimafterx = Menu.Ragebot.Pistols.baimafterx;
	}
	else if (weapon->IsDeagle() && Menu.Ragebot.Deagle.enable)
	{
		Menu.Ragebot.headhitscan = Menu.Ragebot.Deagle.headhitscan;
		Menu.Ragebot.chesthitscan = Menu.Ragebot.Deagle.chesthitscan;
		Menu.Ragebot.stomachhitscan = Menu.Ragebot.Deagle.stomachhitscan;
		Menu.Ragebot.armshitscan = Menu.Ragebot.Deagle.armshitscan;
		Menu.Ragebot.legshitscan = Menu.Ragebot.Deagle.legshitscan;
		Menu.Ragebot.feethitscan = Menu.Ragebot.Deagle.feethitscan;
		Menu.Ragebot.PositionAdjustment = Menu.Ragebot.Deagle.PositionAdjustment;
		Menu.Ragebot.extrapolation = Menu.Ragebot.Deagle.extrapolation;
		Menu.Ragebot.delayshot = Menu.Ragebot.Deagle.delayshot;
		Menu.Ragebot.Fakehitbox = Menu.Ragebot.Deagle.Fakehitbox;
		Menu.Ragebot.AutomaticStop = Menu.Ragebot.Deagle.AutomaticStop;
		Menu.Ragebot.headscale = Menu.Ragebot.Deagle.headscale;
		Menu.Ragebot.bodyscale = Menu.Ragebot.Deagle.bodyscale;
		Menu.Ragebot.Mindamage = Menu.Ragebot.Deagle.Mindamage;
		Menu.Ragebot.Minhitchance = Menu.Ragebot.Deagle.Minhitchance;
		Menu.Ragebot.TickType = Menu.Ragebot.Deagle.TickType;
		Menu.Ragebot.baimifair = Menu.Ragebot.Deagle.baimifair;
		Menu.Ragebot.baimiflethal = Menu.Ragebot.Deagle.baimiflethal;
		Menu.Ragebot.baimafterx = Menu.Ragebot.Deagle.baimafterx;
	}
	else if (weapon->IsR8() && Menu.Ragebot.Revolver.enable)
	{
		Menu.Ragebot.headhitscan = Menu.Ragebot.Revolver.headhitscan;
		Menu.Ragebot.chesthitscan = Menu.Ragebot.Revolver.chesthitscan;
		Menu.Ragebot.stomachhitscan = Menu.Ragebot.Revolver.stomachhitscan;
		Menu.Ragebot.armshitscan = Menu.Ragebot.Revolver.armshitscan;
		Menu.Ragebot.legshitscan = Menu.Ragebot.Revolver.legshitscan;
		Menu.Ragebot.feethitscan = Menu.Ragebot.Revolver.feethitscan;
		Menu.Ragebot.PositionAdjustment = Menu.Ragebot.Revolver.PositionAdjustment;
		Menu.Ragebot.extrapolation = Menu.Ragebot.Revolver.extrapolation;
		Menu.Ragebot.delayshot = Menu.Ragebot.Revolver.delayshot;
		Menu.Ragebot.Fakehitbox = Menu.Ragebot.Revolver.Fakehitbox;
		Menu.Ragebot.AutomaticStop = Menu.Ragebot.Revolver.AutomaticStop;
		Menu.Ragebot.headscale = Menu.Ragebot.Revolver.headscale;
		Menu.Ragebot.bodyscale = Menu.Ragebot.Revolver.bodyscale;
		Menu.Ragebot.Mindamage = Menu.Ragebot.Revolver.Mindamage;
		Menu.Ragebot.Minhitchance = Menu.Ragebot.Revolver.Minhitchance;
		Menu.Ragebot.TickType = Menu.Ragebot.Revolver.TickType;
		Menu.Ragebot.baimifair = Menu.Ragebot.Revolver.baimifair;
		Menu.Ragebot.baimiflethal = Menu.Ragebot.Revolver.baimiflethal;
		Menu.Ragebot.baimafterx = Menu.Ragebot.Revolver.baimafterx;
	}
	else if (weapon->IsAR() && Menu.Ragebot.Rifles.enable)
	{
		Menu.Ragebot.headhitscan = Menu.Ragebot.Rifles.headhitscan;
		Menu.Ragebot.chesthitscan = Menu.Ragebot.Rifles.chesthitscan;
		Menu.Ragebot.stomachhitscan = Menu.Ragebot.Rifles.stomachhitscan;
		Menu.Ragebot.armshitscan = Menu.Ragebot.Rifles.armshitscan;
		Menu.Ragebot.legshitscan = Menu.Ragebot.Rifles.legshitscan;
		Menu.Ragebot.feethitscan = Menu.Ragebot.Rifles.feethitscan;
		Menu.Ragebot.PositionAdjustment = Menu.Ragebot.Rifles.PositionAdjustment;
		Menu.Ragebot.extrapolation = Menu.Ragebot.Rifles.extrapolation;
		Menu.Ragebot.delayshot = Menu.Ragebot.Rifles.delayshot;
		Menu.Ragebot.Fakehitbox = Menu.Ragebot.Rifles.Fakehitbox;
		Menu.Ragebot.AutomaticStop = Menu.Ragebot.Rifles.AutomaticStop;
		Menu.Ragebot.headscale = Menu.Ragebot.Rifles.headscale;
		Menu.Ragebot.bodyscale = Menu.Ragebot.Rifles.bodyscale;
		Menu.Ragebot.Mindamage = Menu.Ragebot.Rifles.Mindamage;
		Menu.Ragebot.Minhitchance = Menu.Ragebot.Rifles.Minhitchance;
		Menu.Ragebot.TickType = Menu.Ragebot.Rifles.TickType;
		Menu.Ragebot.baimifair = Menu.Ragebot.Rifles.baimifair;
		Menu.Ragebot.baimiflethal = Menu.Ragebot.Rifles.baimiflethal;
		Menu.Ragebot.baimafterx = Menu.Ragebot.Rifles.baimafterx;
	}
	else if (weapon->IsMachineGun() && Menu.Ragebot.MG.enable)
	{
		Menu.Ragebot.headhitscan = Menu.Ragebot.MG.headhitscan;
		Menu.Ragebot.chesthitscan = Menu.Ragebot.MG.chesthitscan;
		Menu.Ragebot.stomachhitscan = Menu.Ragebot.MG.stomachhitscan;
		Menu.Ragebot.armshitscan = Menu.Ragebot.MG.armshitscan;
		Menu.Ragebot.legshitscan = Menu.Ragebot.MG.legshitscan;
		Menu.Ragebot.feethitscan = Menu.Ragebot.MG.feethitscan;
		Menu.Ragebot.PositionAdjustment = Menu.Ragebot.MG.PositionAdjustment;
		Menu.Ragebot.extrapolation = Menu.Ragebot.MG.extrapolation;
		Menu.Ragebot.delayshot = Menu.Ragebot.MG.delayshot;
		Menu.Ragebot.Fakehitbox = Menu.Ragebot.MG.Fakehitbox;
		Menu.Ragebot.AutomaticStop = Menu.Ragebot.MG.AutomaticStop;
		Menu.Ragebot.headscale = Menu.Ragebot.MG.headscale;
		Menu.Ragebot.bodyscale = Menu.Ragebot.MG.bodyscale;
		Menu.Ragebot.Mindamage = Menu.Ragebot.MG.Mindamage;
		Menu.Ragebot.Minhitchance = Menu.Ragebot.MG.Minhitchance;
		Menu.Ragebot.TickType = Menu.Ragebot.MG.TickType;
		Menu.Ragebot.baimifair = Menu.Ragebot.MG.baimifair;
		Menu.Ragebot.baimiflethal = Menu.Ragebot.MG.baimiflethal;
		Menu.Ragebot.baimafterx = Menu.Ragebot.MG.baimafterx;
	}
	else if (weapon->IsSMG() && Menu.Ragebot.SMG.enable)
	{
		Menu.Ragebot.headhitscan = Menu.Ragebot.SMG.headhitscan;
		Menu.Ragebot.chesthitscan = Menu.Ragebot.SMG.chesthitscan;
		Menu.Ragebot.stomachhitscan = Menu.Ragebot.SMG.stomachhitscan;
		Menu.Ragebot.armshitscan = Menu.Ragebot.SMG.armshitscan;
		Menu.Ragebot.legshitscan = Menu.Ragebot.SMG.legshitscan;
		Menu.Ragebot.feethitscan = Menu.Ragebot.SMG.feethitscan;
		Menu.Ragebot.PositionAdjustment = Menu.Ragebot.SMG.PositionAdjustment;
		Menu.Ragebot.extrapolation = Menu.Ragebot.SMG.extrapolation;
		Menu.Ragebot.delayshot = Menu.Ragebot.SMG.delayshot;
		Menu.Ragebot.Fakehitbox = Menu.Ragebot.SMG.Fakehitbox;
		Menu.Ragebot.AutomaticStop = Menu.Ragebot.SMG.AutomaticStop;
		Menu.Ragebot.headscale = Menu.Ragebot.SMG.headscale;
		Menu.Ragebot.bodyscale = Menu.Ragebot.SMG.bodyscale;
		Menu.Ragebot.Mindamage = Menu.Ragebot.SMG.Mindamage;
		Menu.Ragebot.Minhitchance = Menu.Ragebot.SMG.Minhitchance;
		Menu.Ragebot.TickType = Menu.Ragebot.SMG.TickType;
		Menu.Ragebot.baimifair = Menu.Ragebot.SMG.baimifair;
		Menu.Ragebot.baimiflethal = Menu.Ragebot.SMG.baimiflethal;
		Menu.Ragebot.baimafterx = Menu.Ragebot.SMG.baimafterx;
	}
	else if (weapon->IsShotgun() && Menu.Ragebot.Shotgun.enable)
	{
		Menu.Ragebot.headhitscan = Menu.Ragebot.Shotgun.headhitscan;
		Menu.Ragebot.chesthitscan = Menu.Ragebot.Shotgun.chesthitscan;
		Menu.Ragebot.stomachhitscan = Menu.Ragebot.Shotgun.stomachhitscan;
		Menu.Ragebot.armshitscan = Menu.Ragebot.Shotgun.armshitscan;
		Menu.Ragebot.legshitscan = Menu.Ragebot.Shotgun.legshitscan;
		Menu.Ragebot.feethitscan = Menu.Ragebot.Shotgun.feethitscan;
		Menu.Ragebot.PositionAdjustment = Menu.Ragebot.Shotgun.PositionAdjustment;
		Menu.Ragebot.extrapolation = Menu.Ragebot.Shotgun.extrapolation;
		Menu.Ragebot.delayshot = Menu.Ragebot.Shotgun.delayshot;
		Menu.Ragebot.Fakehitbox = Menu.Ragebot.Shotgun.Fakehitbox;
		Menu.Ragebot.AutomaticStop = Menu.Ragebot.Shotgun.AutomaticStop;
		Menu.Ragebot.headscale = Menu.Ragebot.Shotgun.headscale;
		Menu.Ragebot.bodyscale = Menu.Ragebot.Shotgun.bodyscale;
		Menu.Ragebot.Mindamage = Menu.Ragebot.Shotgun.Mindamage;
		Menu.Ragebot.Minhitchance = Menu.Ragebot.Shotgun.Minhitchance;
		Menu.Ragebot.TickType = Menu.Ragebot.Shotgun.TickType;
		Menu.Ragebot.baimifair = Menu.Ragebot.Shotgun.baimifair;
		Menu.Ragebot.baimiflethal = Menu.Ragebot.Shotgun.baimiflethal;
		Menu.Ragebot.baimafterx = Menu.Ragebot.Shotgun.baimafterx;
	}
	else
	{
		Menu.Ragebot.headhitscan = Menu.Ragebot.Global.headhitscan;
		Menu.Ragebot.chesthitscan = Menu.Ragebot.Global.chesthitscan;
		Menu.Ragebot.stomachhitscan = Menu.Ragebot.Global.stomachhitscan;
		Menu.Ragebot.armshitscan = Menu.Ragebot.Global.armshitscan;
		Menu.Ragebot.legshitscan = Menu.Ragebot.Global.legshitscan;
		Menu.Ragebot.feethitscan = Menu.Ragebot.Global.feethitscan;
		Menu.Ragebot.PositionAdjustment = Menu.Ragebot.Global.PositionAdjustment;
		Menu.Ragebot.extrapolation = Menu.Ragebot.Global.extrapolation;
		Menu.Ragebot.delayshot = Menu.Ragebot.Global.delayshot;
		Menu.Ragebot.Fakehitbox = Menu.Ragebot.Global.Fakehitbox;
		Menu.Ragebot.AutomaticStop = Menu.Ragebot.Global.AutomaticStop;
		Menu.Ragebot.headscale = Menu.Ragebot.Global.headscale;
		Menu.Ragebot.bodyscale = Menu.Ragebot.Global.bodyscale;
		Menu.Ragebot.Mindamage = Menu.Ragebot.Global.Mindamage;
		Menu.Ragebot.Minhitchance = Menu.Ragebot.Global.Minhitchance;
		Menu.Ragebot.TickType = Menu.Ragebot.Global.TickType;
		Menu.Ragebot.baimifair = Menu.Ragebot.Global.baimifair;
		Menu.Ragebot.baimiflethal = Menu.Ragebot.Global.baimiflethal;
		Menu.Ragebot.baimafterx = Menu.Ragebot.Global.baimafterx;
	}
}

void CMisc::WpnCfgMenu() {

	auto weapon = csgo::MainWeapon;
	if (weapon->IsAuto() && Menu.Ragebot.Auto.enable)
	{
		Menu.Ragebot.wpncfg = 1;
		csgo::OldWeapon = csgo::MainWeapon->GetWeaponNum();
	}
	else if (weapon->IsScout() && Menu.Ragebot.Scout.enable)
	{
		Menu.Ragebot.wpncfg = 2;
		csgo::OldWeapon = csgo::MainWeapon->GetWeaponNum();
	}
	else if (weapon->IsAWP() && Menu.Ragebot.AWP.enable)
	{
		Menu.Ragebot.wpncfg = 3;
		csgo::OldWeapon = csgo::MainWeapon->GetWeaponNum();
	}
	else if (weapon->IsLightPistol() && Menu.Ragebot.Pistols.enable)
	{
		Menu.Ragebot.wpncfg = 4;
		csgo::OldWeapon = csgo::MainWeapon->GetWeaponNum();
	}
	else if (weapon->IsDeagle() && Menu.Ragebot.Deagle.enable)
	{
		Menu.Ragebot.wpncfg = 5;
		csgo::OldWeapon = csgo::MainWeapon->GetWeaponNum();
	}
	else if (weapon->IsR8() && Menu.Ragebot.Revolver.enable)
	{
		Menu.Ragebot.wpncfg = 6;
		csgo::OldWeapon = csgo::MainWeapon->GetWeaponNum();
	}
	else if (weapon->IsAR() && Menu.Ragebot.Rifles.enable)
	{
		Menu.Ragebot.wpncfg = 7;
		csgo::OldWeapon = csgo::MainWeapon->GetWeaponNum();
	}
	else if (weapon->IsMachineGun() && Menu.Ragebot.MG.enable)
	{
		Menu.Ragebot.wpncfg = 8;
		csgo::OldWeapon = csgo::MainWeapon->GetWeaponNum();
	}
	else if (weapon->IsSMG() && Menu.Ragebot.SMG.enable)
	{
		Menu.Ragebot.wpncfg = 9;
		csgo::OldWeapon = csgo::MainWeapon->GetWeaponNum();
	}
	else if (weapon->IsShotgun() && Menu.Ragebot.Shotgun.enable)
	{
		Menu.Ragebot.wpncfg = 10;
		csgo::OldWeapon = csgo::MainWeapon->GetWeaponNum();
	}
	else
	{
		Menu.Ragebot.wpncfg = 0;
		csgo::OldWeapon = csgo::MainWeapon->GetWeaponNum();
	}
}
void CMisc::edgejump(CUserCmd* cmd)
{
	auto* LocalP = csgo::LocalPlayer;

	if (LocalP->GetMoveType() == MOVETYPE_LADDER || LocalP->GetMoveType() == MOVETYPE_NOCLIP)
		return;

	Vector Start, End;
	Start = LocalP->GetOrigin();

	VectorCopy(LocalP->GetOrigin(), Start);
	VectorCopy(Start, End);

	End.z -= 32;

	Ray_t ray;
	ray.Init(Start, End);

	trace_t trace;

	CTraceFilter filter;
	filter.pSkip1 = csgo::LocalPlayer;

	g_pEngineTrace->TraceRay(ray, MASK_PLAYERSOLID_BRUSHONLY, &filter, &trace);

	int EdgeJumpKey = Menu.Misc.edgejumpkey;
	if (trace.fraction == 1.0f && EdgeJumpKey > 0 && GetAsyncKeyState(EdgeJumpKey))
	{
		cmd->buttons |= IN_JUMP;
	}

	if (!(*csgo::LocalPlayer->GetFlags() & FL_ONGROUND) && GetAsyncKeyState(EdgeJumpKey) && Menu.Misc.strafeedgejump)
	{
		csgo::UserCmd->forwardmove = (1550.f * 5) / csgo::LocalPlayer->GetVelocity().Length2D();
		csgo::UserCmd->sidemove = (csgo::UserCmd->command_number % 2) == 0 ? -450.f : 450.f;
		if (csgo::UserCmd->forwardmove > 450.f)
			csgo::UserCmd->forwardmove = 450.f;
	}
}

void CMisc::FixCmd()
{
	if (Menu.Misc.AntiUT)
	{
		csgo::UserCmd->viewangles.y = Math::NormalizeYaw(csgo::UserCmd->viewangles.y);
		Math::ClampAngles(csgo::UserCmd->viewangles);
	}
	if (csgo::UserCmd->forwardmove > 450)
		csgo::UserCmd->forwardmove = 450;
	if (csgo::UserCmd->forwardmove < -450)
		csgo::UserCmd->forwardmove = -450;

	if (csgo::UserCmd->sidemove > 450)
		csgo::UserCmd->sidemove = 450;
	if (csgo::UserCmd->sidemove < -450)
		csgo::UserCmd->sidemove = -450;
}

int LagCompBreak() {

	Vector velocity = csgo::LocalPlayer->GetVelocity();
	velocity.z = 0;
	float speed = velocity.Length();
	if (speed > 0.f) {
		auto distance_per_tick = speed *
			g_pGlobals->interval_per_tick;
		int choked_ticks = std::ceilf(65.f / distance_per_tick);
		return std::min<int>(choked_ticks, Menu.Misc.FakelagAmount);
	}
	return 1;
}

void CMisc::UpdateFlSettings() {
	/* Lets put the Fakelag shit here too lol */

	if (*csgo::LocalPlayer->GetFlags() & FL_ONGROUND)
	{
		if (csgo::LocalPlayer->GetVelocity().Length2D() < 32 && (!GetAsyncKeyState(Menu.Misc.Slowwalkkey))) {
			if (Menu.Misc.FakelagAmount != Menu.Antiaim.Stand.FakelagAmount || Menu.Misc.FakelagMode != Menu.Antiaim.Stand.FakelagMode
				|| Menu.Misc.FakelagShoot != Menu.Antiaim.Stand.FakelagShoot || Menu.Misc.FakelagVariance != Menu.Antiaim.Stand.FakelagVariance) {
				Menu.Misc.FakelagAmount = Menu.Antiaim.Stand.FakelagAmount;
				Menu.Misc.FakelagMode = Menu.Antiaim.Stand.FakelagMode;
				Menu.Misc.FakelagShoot = Menu.Antiaim.Stand.FakelagShoot;
				Menu.Misc.FakelagVariance = Menu.Antiaim.Stand.FakelagVariance;
			}
		}
		else if (csgo::LocalPlayer->GetVelocity().Length2D() > 32 && (!GetAsyncKeyState(Menu.Misc.Slowwalkkey))) {
			if (Menu.Misc.FakelagAmount != Menu.Antiaim.Move.FakelagAmount || Menu.Misc.FakelagMode != Menu.Antiaim.Move.FakelagMode
				|| Menu.Misc.FakelagShoot != Menu.Antiaim.Move.FakelagShoot || Menu.Misc.FakelagVariance != Menu.Antiaim.Move.FakelagVariance) {
				Menu.Misc.FakelagAmount = Menu.Antiaim.Move.FakelagAmount;
				Menu.Misc.FakelagMode = Menu.Antiaim.Move.FakelagMode;
				Menu.Misc.FakelagShoot = Menu.Antiaim.Move.FakelagShoot;
				Menu.Misc.FakelagVariance = Menu.Antiaim.Move.FakelagVariance;
			}
		}
		else if (GetAsyncKeyState(Menu.Misc.Slowwalkkey)) {
			if (Menu.Misc.FakelagAmount != Menu.Antiaim.Slow.FakelagAmount || Menu.Misc.FakelagMode != Menu.Antiaim.Slow.FakelagMode
				|| Menu.Misc.FakelagShoot != Menu.Antiaim.Slow.FakelagShoot || Menu.Misc.FakelagVariance != Menu.Antiaim.Slow.FakelagVariance) {
				Menu.Misc.FakelagAmount = Menu.Antiaim.Slow.FakelagAmount;
				Menu.Misc.FakelagMode = Menu.Antiaim.Slow.FakelagMode;
				Menu.Misc.FakelagShoot = Menu.Antiaim.Slow.FakelagShoot;
				Menu.Misc.FakelagVariance = Menu.Antiaim.Slow.FakelagVariance;
			}
		}
	}
	else {
		if (Menu.Misc.FakelagAmount != Menu.Antiaim.Air.FakelagAmount || Menu.Misc.FakelagMode != Menu.Antiaim.Air.FakelagMode
			|| Menu.Misc.FakelagShoot != Menu.Antiaim.Air.FakelagShoot || Menu.Misc.FakelagVariance != Menu.Antiaim.Air.FakelagVariance) {
			Menu.Misc.FakelagAmount = Menu.Antiaim.Air.FakelagAmount;
			Menu.Misc.FakelagMode = Menu.Antiaim.Air.FakelagMode;
			Menu.Misc.FakelagShoot = Menu.Antiaim.Air.FakelagShoot;
			Menu.Misc.FakelagVariance = Menu.Antiaim.Air.FakelagVariance;
		}
	}
}

int UnitBreaker() {
	Vector Velocity = csgo::LocalPlayer->GetVelocity();
	double vel_real = floor(min(10000, sqrt(Velocity.x*Velocity.x + Velocity.y * Velocity.y) + 0.5));
	double distance_per_tick = vel_real * g_pGlobals->interval_per_tick;
	double unitbreaker_ticks = ceil(64 / distance_per_tick);
	return min(unitbreaker_ticks, 14);
}

void CMisc::FakeLag()
{
	bool fakeduck = false;
	if (GetAsyncKeyState(Menu.Antiaim.FakeDuckKey) && Menu.Antiaim.FakeDuck)
		fakeduck = true;
	else fakeduck = false;
	int Type = Menu.Misc.FakelagMode;
	if (csgo::MainWeapon->WeaponID() == ItemDefinitionIndex::REVOLVER)
	{
		csgo::SendPacket = true;
		return;
	}
	if (!fakeduck && g_Aimbot->fired_in_that_tick)
	{
		csgo::SendPacket = true;
		return;
	}
	if (Type == 0)
	{
		csgo::SendPacket = true;
		return;
	}

	static int ticks = 0;
	const int ticksMax = 14;

	static bool new_factor = false;

	if (fakeduck) {
		csgo::chokedpackets = 15;
	}
	else {
		if (csgo::desyncstill) {
			csgo::chokedpackets = 3;
		}
		else {
		int pchoke = 0;
			if (Type == 1)
			{
				pchoke = Menu.Misc.FakelagAmount;
			}
			else if (Type == 2)
			{
				pchoke = UnitBreaker();
			}
			pchoke = Math::RandomInt(pchoke - Menu.Misc.FakelagVariance, pchoke);
			csgo::chokedpackets = pchoke;
		}
	}

	if (csgo::chokedpackets > ticksMax)
		csgo::chokedpackets = ticksMax;

	if (ticks >= csgo::chokedpackets)
	{
		ticks = 0;
		csgo::SendPacket = true;
		new_factor = true;
	}
	else
	{
		csgo::SendPacket = false;
	}
	ticks++;

	return;
}

float VerifyRotation(float ideal_rotation)
{
	static constexpr float ray_length = 1000.f;
	static constexpr float minimum_distance = 50.f;
	static constexpr float rotation_step = 5.f;

	auto local_player = csgo::LocalPlayer;
	if (!local_player)
		return ideal_rotation;

	auto collideable = local_player->GetCollision();
	if (!collideable)
		return ideal_rotation;

	auto TraceRayBoundingBox = [](Vector start, Vector end, Vector min, Vector max, float& fraction) -> void
	{
		Vector starts[8];
		Vector ends[8];

		starts[0] = start + Vector(min.x, min.y, min.z); /// min
		starts[1] = start + Vector(max.x, min.y, min.z);
		starts[2] = start + Vector(max.x, max.y, min.z);
		starts[3] = start + Vector(max.x, max.y, max.z); /// max
		starts[4] = start + Vector(min.x, max.y, max.z);
		starts[5] = start + Vector(min.x, min.y, max.z);
		starts[6] = start + Vector(max.x, min.y, max.z);
		starts[7] = start + Vector(min.x, max.y, min.z);

		ends[0] = end + Vector(min.x, min.y, min.z); /// min
		ends[1] = end + Vector(max.x, min.y, min.z);
		ends[2] = end + Vector(max.x, max.y, min.z);
		ends[3] = end + Vector(max.x, max.y, max.z); /// max
		ends[4] = end + Vector(min.x, max.y, max.z);
		ends[5] = end + Vector(min.x, min.y, max.z);
		ends[6] = end + Vector(max.x, min.y, max.z);
		ends[7] = end + Vector(min.x, max.y, min.z);

		float fractions[8];
		for (int i = 0; i < 8; i++)
		{
			CTraceWorldOnly filter;
			trace_t trace;
			Ray_t ray;
			ray.Init(starts[i], ends[i]);

			g_pEngineTrace->TraceRay_NEW(ray, MASK_ALL, &filter, &trace);
			fractions[i] = trace.fraction;
		}

		fraction = 1.f;
		for (const float& frac : fractions)
		{
			if (frac < fraction)
				fraction = frac;
		}
	};

	Vector bbmin, bbmax;
	bbmin = collideable->VecMins();
	bbmax = collideable->VecMaxs();

	const Vector velocity_angle = GameUtils::CalculateAngle(Vector(0, 0, 0), local_player->GetVelocity());
	float verified_roation = ideal_rotation;

	for (float i = ideal_rotation; ((ideal_rotation > 0) ? (i < 180.f) : (i > -180.f)); ((ideal_rotation > 0) ? (i += rotation_step) : (i -= rotation_step)))
	{
		Vector direction;
		float fraction_1a, fraction_1b;
		float fraction_2a, fraction_2b;

		Math::AngleVectors(Vector(0.f, velocity_angle.y + i, 0), &direction);
		TraceRayBoundingBox(local_player->GetOrigin(), local_player->GetOrigin() + (direction * ray_length), bbmin, bbmax, fraction_1a);
		Math::AngleVectors(Vector(0.f, velocity_angle.y + i + rotation_step, 0), &direction);
		TraceRayBoundingBox(local_player->GetOrigin(), local_player->GetOrigin() + (direction * ray_length), bbmin, bbmax, fraction_1b);

		Math::AngleVectors(Vector(0.f, velocity_angle.y - i, 0), &direction);
		TraceRayBoundingBox(local_player->GetOrigin(), local_player->GetOrigin() + (direction * ray_length), bbmin, bbmax, fraction_2a);
		Math::AngleVectors(Vector(0.f, velocity_angle.y - (i + rotation_step), 0), &direction);
		TraceRayBoundingBox(local_player->GetOrigin(), local_player->GetOrigin() + (direction * ray_length), bbmin, bbmax, fraction_2b);

		if ((fraction_1a * ray_length > minimum_distance) && (fraction_1b * ray_length > minimum_distance))
		{
			verified_roation = i;
			break;
		}
		else if ((fraction_2a * ray_length > minimum_distance) && (fraction_2b * ray_length > minimum_distance))
		{
			verified_roation = -i;
			break;
		}
	}

	return verified_roation;
}

template <class T>
constexpr const T& Clamp(const T& v, const T& lo, const T& hi)
{
	return (v >= lo && v <= hi) ? v : (v < lo ? lo : hi);
}

static inline float GetIdealRotation(float speed)
{
	return Clamp<float>(RAD2DEG(std::atan2(15.f, speed)), 0.f, 45.f) * g_pGlobals->interval_per_tick;
}

void RotateMovement(float yaw)
{
	QAngle viewangles;
	g_pEngine->GetViewAngles(viewangles);

	float rotation = DEG2RAD(viewangles.y - yaw);

	float cos_rot = cos(rotation);
	float sin_rot = sin(rotation);

	float new_forwardmove = (cos_rot * csgo::UserCmd->forwardmove) - (sin_rot * csgo::UserCmd->sidemove);
	float new_sidemove = (sin_rot * csgo::UserCmd->forwardmove) + (cos_rot * csgo::UserCmd->sidemove);

	csgo::UserCmd->forwardmove = new_forwardmove;
	csgo::UserCmd->sidemove = new_sidemove;
}


void CMisc::DoCircle()
{
	if (!GetAsyncKeyState(Menu.Misc.circlestrafekey))
		return;

	auto local_player = csgo::LocalPlayer;
	if (!local_player || local_player->GetHealth() <= 0 || *local_player->GetFlags() & FL_ONGROUND)
		return;

	static bool clock_wise = false;

	const float velocity_yaw = GameUtils::CalculateAngle(Vector(0, 0, 0), local_player->GetVelocity()).y;
	const float ideal_rotation_amount = GetIdealRotation(local_player->GetVelocity().Length2D()) * (clock_wise ? -1.f : 1.f);

	float rotation_amount = VerifyRotation(ideal_rotation_amount);

	csgo::UserCmd->forwardmove = 0.f;
	csgo::UserCmd->sidemove = (rotation_amount > 0) ? -450.f : 450.f;
	RotateMovement(velocity_yaw + rotation_amount);
}


void CMisc::FixMovement()
{
	Vector vMove = Vector(csgo::UserCmd->forwardmove, csgo::UserCmd->sidemove, 0.0f);
	float flSpeed = vMove.Length();
	Vector qMove;
	float normalizedx;
	Math::VectorAngles(vMove, qMove);
	float normalized = fmod(csgo::UserCmd->viewangles.y + 180.f, 360.f) - 180.f;
	if (Menu.Misc.AntiUT)
	normalizedx = fmod(csgo::UserCmd->viewangles.x + 180.f, 360.f) - 180.f;
	else normalizedx = csgo::UserCmd->viewangles.x;
	float flYaw = DEG2RAD((normalized - csgo::StrafeAngle.y) + qMove.y);

	if (normalizedx >= 90.0f || normalizedx <= -90.0f || (csgo::UserCmd->viewangles.x >= 90.f && csgo::UserCmd->viewangles.x <= 200) || csgo::UserCmd->viewangles.x <= -90)
		csgo::UserCmd->forwardmove = -cos(flYaw) * flSpeed;
	else
		csgo::UserCmd->forwardmove = cos(flYaw) * flSpeed;

	csgo::UserCmd->sidemove = sin(flYaw) * flSpeed;
}


void LoadPathing()
{

	char path[MAX_PATH];
	GetModuleFileNameA(GetModuleHandle(NULL), path, 255);
	for (int i = strlen(path); i > 0; i--)
	{
		if (path[i] == '\\')
		{
			path[i + 1] = 0;
			break;
		}
	}

	char size[8];
	char vecstr[64];
	char itostr[8];
	CBaseEntity *pLocal = csgo::LocalPlayer;
	if (!pLocal)
		return;
	if (g_Misc->path.size() > 1)
	{

		g_Misc->path.erase(g_Misc->path.begin(), g_Misc->path.end());
		sprintf_s(path, "\\%s_%s_%d.cfg", "pathes", "test"/*g_pEngine->GetLevelName()*/, pLocal->GetTeamNum());
		GetPrivateProfileStringA("Points", "Size", "0", size, 8, path);
		int numPoints = atoi(size);
		for (int i = 0; i < numPoints - 1; i++)
		{
			char vecstr[64];
			char itostr[8];
			sprintf_s(itostr, "%d", i);
			GetPrivateProfileStringA("Pathing", itostr, "0.0 0.0 0.0", vecstr, 64, path);
			std::string PosStr = vecstr;
			string buffer;
			stringstream ss(PosStr);
			vector<string> floats;
			while (ss >> buffer)
				floats.push_back(buffer);

			g_Misc->path.push_back(Vector(stof(floats[0]), stof(floats[1]), stof(floats[2])));
		}
	}
}

void SavePathing()
{

	char path[MAX_PATH];
	GetModuleFileNameA(GetModuleHandle(NULL), path, 255);
	for (int i = strlen(path); i > 0; i--)
	{
		if (path[i] == '\\')
		{
			path[i + 1] = 0;
			break;
		}
	}
	char size[8];
	char vecstr[64];
	char itostr[8];

	if (g_Misc->path.size() > 1)
	{
		sprintf_s(path, "\\%s_%s_%d.cfg", "pathes", "test"/*g_pEngine->GetLevelName()*/, csgo::LocalPlayer->GetTeamNum());
		printf("Path %s\n", path);
		sprintf_s(size, "%d", g_Misc->path.size() + 1);
		WritePrivateProfileStringA("Points", "Size", size, path);
		for (int i = 0; i < g_Misc->path.size(); i++)
		{
			sprintf_s(itostr, "%d", i);
			sprintf_s(vecstr, "%f %f %f", g_Misc->path.at(i).x, g_Misc->path.at(i).y, g_Misc->path.at(i).z);
			WritePrivateProfileStringA("Pathing", itostr, vecstr, path);
		}
	}
}
bool MarksIsVisible(CBaseEntity* local, Vector& vTo)
{
	Ray_t ray;
	trace_t trace;
	CTraceFilterNoPlayer filter;
	filter.pSkip1 = local;

	ray.Init(local->GetEyePosition(), vTo);
	g_pEngineTrace->TraceRay(ray, 0x4600400B, &filter, &trace);
	return (trace.fraction > 0.99f);
}