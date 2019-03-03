#include "sdk.h"
#include "global.h"
#include "Draw.h"
#include "GameUtils.h"
#include <chrono>
/*---------------------ENGINE & UTIL CLASSES---------------------*/
CDataMapUtils* g_pData = new CDataMapUtils;
CEncryption* g_pEncrypt = new CEncryption;
CBaseEntity* csgo::LocalPlayer = nullptr;
bool csgo::baimiflethal;
CBaseCombatWeapon* csgo::MainWeapon = nullptr;
int csgo::OldWeapon = 0;
CSWeaponInfo* csgo::WeaponData = nullptr;
CUserCmd*	csgo::UserCmd = nullptr;
CUserCmd* csgo::UserCmdForBacktracking = nullptr;
CBaseEntity* csgo::Target = nullptr;
CPlayerlistInfo* g_pPlayerlistInfo[64] = { nullptr };
std::deque<std::tuple<Vector, float, Color>> csgo::hitscan_points;
/*---------------------QANGLE---------------------*/
QAngle csgo::LastAngle = QAngle();
QAngle csgo::AAAngle = QAngle();
QAngle csgo::StrafeAngle = QAngle();
QAngle csgo::RealAngle = QAngle();
vec_t csgo::PitchAngle = vec_t();
VMatrix csgo::BoneMatrix[128];
QAngle csgo::FakeAngle = QAngle();

/*---------------------BOOLEAN---------------------*/
bool csgo::Aimbotting = false;
bool csgo::IsDef  = strstr(GetCommandLineA(), "-legacyscaleformui");
bool csgo::BreakingLagComp = false;
bool csgo::Return = true;
bool csgo::ForceRealAA = false;
bool csgo::SendPacket = true;
bool csgo::InNotAntiAim = true;
bool csgo::ShowMenu = false;
bool csgo::Opened = false;
bool csgo::savedrecords = true;
bool csgo::NewRound = false;
bool csgo::NormalSpeed = true;
bool csgo::Minwalk = false;
bool csgo::isleft = false;
bool csgo::desyncstill = false;
bool csgo::b_IsThirdPerson = false;
bool csgo::Init = false;
bool csgo::weaponfirecalled = false;
bool csgo::playerhurtcalled = false;
bool csgo::walkbotenabled = false;
bool csgo::bShouldChoke = false;
bool csgo::bFakewalking = false;
bool csgo::lby = false;
bool csgo::shouldbaim = false;

/*---------------------INTEGER---------------------*/
int csgo::wbpoints = -1;
int csgo::localtime;
int csgo::OldTickCount;
int csgo::chokedticks;
int csgo::wbcurpoint = 0;
int csgo::ChokedPackets = 0;
int csgo::DamageDealt;
int csgo::ResolverMode[64];
int csgo::FakeDetection[64];
int csgo::chokedpackets = 0;
int csgo::Shots[64];
int csgo::MenuTab;
int csgo::nChokedTicks = 0;
int csgo::missed_shots[64];
int csgo::TargetIDO = -1;

/*---------------------FLOATING POINT---------------------*/
float csgo::PredictedTime = 0.f;
float csgo::lby_update_end_time;
float csgo::CurrTime;
float csgo::flHurtTime;
float csgo::fake_strength = 0;
float csgo::spread;
float csgo::viewMatrix[4][4] = { 0 };
float csgo::realangles = 0;
/*---------------------D3DX DEFINES---------------------*/
HWND csgo::Window = nullptr;
CScreen csgo::Screen = CScreen();

/*---------------------STANDARD VECTOR MAPS---------------------*/
std::vector<FloatingText> csgo::DamageHit;
std::vector<Vector> csgo::walkpoints;
std::string csgo::resolvermode;

/*---------------------VECTOR---------------------*/
Vector csgo::vecUnpredictedVel = Vector(0, 0, 0);
Vector csgo::fakeOrigin = Vector(0, 0, 0);
Vector csgo::weewee;

/*---------------------CHAR---------------------*/
char* csgo::username = "unknown";

itemTimer::itemTimer() {
	maxTime = 0;
}

itemTimer::itemTimer(float _maxTime) {
	maxTime = _maxTime;
}

float itemTimer::getTimeRemaining() {
	auto time = (timeStarted - (float(clock()) / float(CLOCKS_PER_SEC))) + maxTime;
	return time ? time : 0.00001f; //ensure we don't ever return a 0
}

float itemTimer::getTimeRemainingRatio() {
	return getTimeRemaining() / getMaxTime();
}

float itemTimer::getMaxTime() {
	return maxTime ? maxTime : 1; //don't ever return 0
}

void itemTimer::startTimer() {
	timeStarted = float(float(clock()) / float(CLOCKS_PER_SEC));
}

void itemTimer::setMaxTime(float time) {
	maxTime = time;
}

FloatingText::FloatingText(CBaseEntity* attachEnt, float lifetime, int Damage)
{
	TimeCreated = csgo::CurrTime;
	ExpireTime = TimeCreated + lifetime;
	pEnt = attachEnt;
	DamageAmt = Damage;
}

void FloatingText::Draw()
{
	float Red, Green, Blue;
	Red = Menu.Colors.DamageIndicator[0] * 255;
	Green = Menu.Colors.DamageIndicator[1] * 255;
	Blue = Menu.Colors.DamageIndicator[2] * 255;
	auto head = pEnt->GetBonePos(8);
	Vector screen;

	if (GameUtils::WorldToScreen(head, screen))
	{
		auto lifetime = ExpireTime - TimeCreated;
		auto pct = 1 - ((ExpireTime - csgo::CurrTime) / lifetime);
		int offset = pct * 50;
		int y = screen.y - 15 - offset;

		if (DamageAmt >= 100)
		{
			draw::Textf(screen.x, y, Color(Red, Green, Blue, 255), F::indicators, "-%i", DamageAmt);
		}
		else
		{
			draw::Textf(screen.x, y, Color(Red, Green, Blue, 255), F::indicators, "-%i", DamageAmt);
		}
	}
}

int CDataMapUtils::Find(datamap_t* pMap, const char* szName) {
	while (pMap)
	{
		for (int i = 0; i < pMap->dataNumFields; i++)
		{
			if (pMap->dataDesc[i].fieldName == NULL)
				continue;

			if (strcmp(szName, pMap->dataDesc[i].fieldName) == 0)
				return pMap->dataDesc[i].fieldOffset[TD_OFFSET_NORMAL];

			if (pMap->dataDesc[i].fieldType == FIELD_EMBEDDED)
			{
				if (pMap->dataDesc[i].td)
				{
					unsigned int offset;

					if ((offset = Find(pMap->dataDesc[i].td, szName)) != 0)
						return offset;
				}
			}
		}
		pMap = pMap->baseMap;
	}

	return 0;
}