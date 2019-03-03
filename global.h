#pragma once
#include <vector>
#include <deque>
#include "includes.h"
#define SIZEOF(var) ( sizeof( var ) / sizeof( var[0] ))

class itemTimer {
public:
	itemTimer();
	itemTimer(float maxTime);
	float getTimeRemaining();
	float getTimeRemainingRatio();
	float getMaxTime();
	void setMaxTime(float);
	void startTimer();
private:
	float timeStarted;
	float maxTime;
};


class FloatingText
{
public:
	void Draw();
	float TimeCreated;
	float ExpireTime;
	int DamageAmt;
	FloatingText(CBaseEntity* attachEnt, float lifetime, int Damage);

private:
	CBaseEntity* pEnt;


};

class CScreen
{
public:
	int width, height;
};

class CGlobalPointers
{
public:
	CBaseEntity* LocalPlayer;
	CBaseCombatWeapon* MainWeapon;
	CUserCmd*	UserCmd;
}; extern CGlobalPointers* g_GlobalPointers;

class CPlayerlistInfo {
public:
	int iHitbox = 0;

	bool bBaim = false;
	bool bOverrideResolver = false;

	vec_t pitch = 0.f;
	vec_t yaw = 0.f;
	vec_t roll = 0.f;

	QAngle viewangles = QAngle(pitch, yaw, roll);
};

namespace csgo // Global Stuff
{
	extern std::deque<std::tuple<Vector, float, Color>> hitscan_points;

	/*---------------------ENGINE & UTIL CLASSES---------------------*/
	extern QAngle							RealAngle;
	extern QAngle							FakeAngle;
	extern QAngle							LastAngle;
	extern QAngle							StrafeAngle;
	extern QAngle							AAAngle;
	extern vec_t                            PitchAngle;
	extern CBaseEntity*						LocalPlayer;
	extern CBaseCombatWeapon*				MainWeapon;
	extern CUserCmd*						UserCmd;
	extern CUserCmd*						UserCmdForBacktracking;
	extern CSWeaponInfo*					WeaponData;
	extern CBaseEntity*						Target;
	extern Vector							vecUnpredictedVel;
	extern Vector                           fakeOrigin;
	extern Vector							weewee;
	extern VMatrix                          BoneMatrix[128];
	extern HWND								Window;
	extern CScreen							Screen;
	extern std::vector<FloatingText>		DamageHit;
	extern std::vector<Vector>				walkpoints;
	extern std::string                      resolvermode;

	/*---------------------BOOLEAN---------------------*/
	extern bool								Aimbotting;
	extern bool								IsDef;
	extern bool								ForceRealAA;
	extern bool								BreakingLagComp;
	extern bool								Return;
	extern bool								baimiflethal;
	extern bool								InNotAntiAim;
	extern bool								SendPacket;
	extern bool								NormalSpeed;
	extern bool/*/////////////////////////*/Minwalk;
	extern bool								isleft;
	extern bool                             desyncstill;
	extern bool								ShowMenu;
	extern bool                             savedrecords;
	extern bool								Opened;
	extern bool								Init;
	extern bool								NewRound;
	extern bool                             b_IsThirdPerson;
	extern bool								bFakewalking;
	extern bool								weaponfirecalled;
	extern bool								playerhurtcalled;
	extern bool								walkbotenabled;
	extern bool								bShouldChoke;
	extern bool                             lby;
	extern bool                             shouldbaim;
	/*---------------------FLOAT---------------------*/
	extern float							PredictedTime;
	extern float							lby_update_end_time;
	extern float							CurrTime;
	extern float	                        fake_strength;
	extern float	                        flHurtTime;
	extern float	                        spread;
	extern float							realangles;
	extern float							viewMatrix[4][4];


	/*---------------------INTEGER---------------------*/
	extern int                              TargetIDO;
	extern int								ChokedPackets;
	extern int								DamageDealt;
	extern int								OldTickCount;
	extern int								Shots[64];
	extern int								OldWeapon;
	extern int								ResolverMode[64];
	extern int								FakeDetection[64];
	extern int								MenuTab;
	extern int								wbpoints;
	extern int                              chokedpackets;
	extern int								wbcurpoint;
	extern int                              missed_shots[64];
	extern int								nChokedTicks;
	extern int                              localtime;
	extern int                              chokedticks;
	/*---------------------CHAR---------------------*/
	extern char*                            username;

}

class CDataMapUtils {
public:
	int Find(datamap_t *pMap, const char* szName);
};

class CEncryption {
public:
	template<class U>
	void Encrypt(U& szString, int iSize) {
		if (bEncrypted) 
			return;

		for (UINT Iterator = 0; Iterator < iSize; Iterator++)
			szString[Iterator] += (2 * szKey[ (SIZEOF(szKey)) - Iterator ]);

		bEncrypted = true;
	}

	template<class U>
	void Decrypt(U& szString, int iSize) {
		if (!bEncrypted) 
			return;

		for (UINT Iterator = 0; Iterator < iSize; Iterator++)
			szString[Iterator] -= (2 * szKey[ (SIZEOF(szKey)) - Iterator ]);

		bEncrypted = false;
	}

	bool SetState(bool bEncryptState) {
		bEncrypted = bEncryptState;
	}

private:
	bool bEncrypted = false;
	char szKey[29] = { 'A', 'z', 'B', 'y', 'C', 'x', 'D', 'w', 'E', 'v', 'F', 'u', 'G', 't', 'H', 's', 'I', 'r', 'J' };
};
extern CEncryption* g_pEncrypt;

class CEncryptedString {
public:
	CEncryptedString(const char* szInput) {
		std::string szOutput = std::string(szInput);
		g_pEncrypt->Encrypt(szOutput, szOutput.size());

		szEncryptedString = szOutput.c_str();
	};

	const char* c_str() {
		std::string szInput = std::string(szEncryptedString);
		g_pEncrypt->Decrypt(szInput, szInput.size());

		return szInput.c_str();
	};

private:
	const char* szEncryptedString;
};

typedef CEncryptedString EString;

extern CDataMapUtils* g_pData;

extern CPlayerlistInfo* g_pPlayerlistInfo[64];
extern CBaseEntity* csgo::LocalPlayer;
extern CBaseCombatWeapon* csgo::MainWeapon;
extern CUserCmd*	csgo::UserCmd;
