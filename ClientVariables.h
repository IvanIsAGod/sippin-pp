#pragma once
#include <string>
#include <vector>


class cPlayersList
{
public:
	bool	Playerlist;
	bool	Resolveall;
	struct
	{
		int YAngle;
		int PAngle;
		int Resolver;
		bool Baim;
	}AAA[64];

};

namespace CPlayerList
{
	extern std::vector<int> Players;
}

class ClientVariables
{
public:
	cPlayersList Players2;
	struct {
		bool bEnable;
		bool autofire;
		bool aimsmokecheck;
		bool friendlyfire;
		int aimkey;
		bool triggerrcs;
		bool triggerboton;
		bool triggerteam;
		bool triggersmokecheck;
		int triggerbotkey;

		bool headhitscan = false;
		bool chesthitscan = false;
		bool stomachhitscan = false;
		bool armshitscan = false;
		bool legshitscan = false;
		//AR
		bool def_pSilent;
		float def_fov;
		float def_rcs;
		float def_aimspeed;
		int def_hitbox;
		//SMG
		bool smg_pSilent;
		float smg_fov;
		float smg_rcs;
		float smg_aimspeed;
		int smg_hitbox;
		//Sniper
		bool sni_pSilent;
		float sni_fov;
		float sni_rcs;
		float sni_aimspeed;
		int sni_hitbox;
		//Pistol
		bool pis_pSilent;
		float pis_fov;
		float pis_rcs;
		float pis_aimspeed;
		int pis_hitbox;
		//Shotgun
		/*int shot_BackTrackType;
		int shot_iFovType;
		int shot_iSmoothType;
		float shot_flSmooth;
		float shot_flFov;
		int shot_iDelay;
		float shot_flSwitchTargetDelay;
		int shot_iAimType;
		bool shot_bUseSpecificAim;
		int shot_iSpecificAimType;*/
		//Heavy
		/*int mg_BackTrackType;
		int mg_iFovType;
		int mg_iSmoothType;
		float mg_flSmooth;
		float mg_flFov;
		int mg_iDelay;
		float mg_flSwitchTargetDelay;
		int mg_iAimType;
		bool mg_bUseSpecificAim;
		int mg_iSpecificAimType;*/
	} LegitBot;
	struct Ragebot
	{

		bool EnableAimbot = false;
		bool AnimFix = false;
		int AimbotSelection = 0;
		int Fov = 0;
		int wpncfg = 0;
		bool AutomaticFire = false;
		bool AutomaticScope = false;
		bool SilentAimbot = false;
		bool NoRecoil = false;
		bool NoSpread = false;
		int Hitbox = 0;
		bool Autowall = false;
		int ForcePelvis = 0;
		bool FriendlyFire = false;
		bool AutomaticResolver;
		int ResolverOverride = 0;
		int Hitscan = 0;
		bool Quickstop = false;
		bool AutomaticRevolver;
		bool NewAutomaticRevolver;
		int NewAutomaticRevolverFactor;
		bool FakeLatency;
		int fakepingkey;
		int FakeLatencyAmount;
		int ResolverType = 0;

		bool headhitscan = false;
		bool chesthitscan = false;
		bool stomachhitscan = false;
		bool armshitscan = false;
		bool legshitscan = false;
		bool feethitscan = false;
		bool PositionAdjustment = false;
		int extrapolation = 0;
		bool delayshot = false;
		int Fakehitbox = 0;
		int AutomaticStop = 0;
		float headscale = 0.f;
		float bodyscale = 0.f;
		float Mindamage = 1.f;	
		float Minhitchance = 0.f;
		int TickType = 0;
		bool baimifair = false;
		bool baimiflethal = false;
		bool baimifslow = false;
		bool baimifmove = false;
		int baimafterx = 0;

		struct Global {
			bool headhitscan = false;
			bool chesthitscan = false;
			bool stomachhitscan = false;
			bool armshitscan = false;
			bool legshitscan = false;
			bool feethitscan = false;
			bool PositionAdjustment = false;
			int extrapolation = 0;
			bool delayshot = false;
			int Fakehitbox = 0;
			int AutomaticStop = 0;
			float headscale = 0.f;
			float bodyscale = 0.f;
			float Mindamage = 1.f;
			float Minhitchance = 0.f;
			int TickType = 0;
			bool baimifair = false;
			bool baimiflethal = false;
			bool baimifslow = false;
			bool baimifmove = false;
			int baimafterx = 0;
		} Global;
		struct Auto {
			bool enable = false;
			bool headhitscan = true;
			bool chesthitscan = true;
			bool stomachhitscan = true;
			bool armshitscan = false;
			bool legshitscan = true;
			bool feethitscan = false;
			bool PositionAdjustment = true;
			int extrapolation = 2;
			bool delayshot = false;
			int Fakehitbox = 1;
			int AutomaticStop = 1;
			float headscale = 43.f;
			float bodyscale = 63.f;
			float Mindamage = 20.f;
			float Minhitchance = 72.f;
			int TickType = 1;
			bool baimifair = false;
			bool baimiflethal = true;
			bool baimifslow = true;
			bool baimifmove = false;
			int baimafterx = 1;
		} Auto;
		struct Scout {
			bool enable = false;
			bool headhitscan = true;
			bool chesthitscan = true;
			bool stomachhitscan = true;
			bool armshitscan = false;
			bool legshitscan = false;
			bool feethitscan = false;
			bool PositionAdjustment = true;
			int extrapolation = 2;
			bool delayshot = true;
			int Fakehitbox = 1;
			int AutomaticStop = 1;
			float headscale = 47.f;
			float bodyscale = 63.f;
			float Mindamage = 55.f;
			float Minhitchance = 78.f;
			int TickType = 1;
			bool baimifair = true;
			bool baimiflethal = true;
			bool baimifmove = false;
			bool baimifslow = true;
			int baimafterx = 1;
		} Scout;
		struct AWP {
			bool enable = false;
			bool headhitscan = true;
			bool chesthitscan = true;
			bool stomachhitscan = true;
			bool armshitscan = false;
			bool legshitscan = false;
			bool feethitscan = false;
			bool PositionAdjustment = true;
			int extrapolation = 2;
			bool delayshot = true;
			int Fakehitbox = 2;
			int AutomaticStop = 1;
			float headscale = 40.f;
			float bodyscale = 63.f;
			float Mindamage = 100.f;
			float Minhitchance = 90.f;
			int TickType = 1;
			bool baimifair = true;
			bool baimiflethal = true;
			bool baimifmove = false;
			bool baimifslow = true;
			int baimafterx = 1;
		} AWP;
		struct Pistols {
			bool enable = false;
			bool headhitscan = true;
			bool chesthitscan = true;
			bool stomachhitscan = true;
			bool armshitscan = false;
			bool legshitscan = false;
			bool feethitscan = false;
			bool PositionAdjustment = true;
			int extrapolation = 2;
			bool delayshot = false;
			int Fakehitbox = 1;
			int AutomaticStop = 1;
			float headscale = 43.f;
			float bodyscale = 63.f;
			float Mindamage = 20.f;
			float Minhitchance = 63.f;
			int TickType = 1;
			bool baimifair = false;
			bool baimiflethal = true;
			bool baimifslow = true;
			bool baimifmove = false;
			int baimafterx = 1;
		} Pistols;
		struct Deagle {
			bool enable = false;
			bool headhitscan = true;
			bool chesthitscan = true;
			bool stomachhitscan = true;
			bool armshitscan = false;
			bool legshitscan = false;
			bool feethitscan = false;
			bool PositionAdjustment = true;
			int extrapolation = 2;
			bool delayshot = true;
			int Fakehitbox = 1;
			int AutomaticStop = 1;
			float headscale = 47.f;
			float bodyscale = 63.f;
			float Mindamage = 30.f;
			float Minhitchance = 80.f;
			int TickType = 1;
			bool baimifair = true;
			bool baimiflethal = true;
			bool baimifslow = true;
			bool baimifmove = false;
			int baimafterx = 1;
		} Deagle;
		struct Revolver {
			bool enable = false;
			bool headhitscan = true;
			bool chesthitscan = true;
			bool stomachhitscan = true;
			bool armshitscan = false;
			bool legshitscan = false;
			bool feethitscan = false;
			bool PositionAdjustment = true;
			int extrapolation = 2;
			bool delayshot = true;
			int Fakehitbox = 1;
			int AutomaticStop = 1;
			float headscale = 47.f;
			float bodyscale = 63.f;
			float Mindamage = 30.f;
			float Minhitchance = 72.f;
			int TickType = 1;
			bool baimifair = true;
			bool baimiflethal = true;
			bool baimifslow = true;
			bool baimifmove = false;
			int baimafterx = 1;
		} Revolver;
		struct Rifles {
			bool enable = false;
			bool headhitscan = true;
			bool chesthitscan = true;
			bool stomachhitscan = true;
			bool armshitscan = false;
			bool legshitscan = false;
			bool feethitscan = false;
			bool PositionAdjustment = true;
			int extrapolation = 2;
			bool delayshot = false;
			int Fakehitbox = 1;
			int AutomaticStop = 1;
			float headscale = 43.f;
			float bodyscale = 63.f;
			float Mindamage = 20.f;
			float Minhitchance = 58.f;
			int TickType = 1;
			bool baimifair = false;
			bool baimiflethal = true;
			bool baimifslow = true;
			bool baimifmove = false;
			int baimafterx = 1;
		} Rifles;
		struct MG {
			bool enable = false;
			bool headhitscan = true;
			bool chesthitscan = true;
			bool stomachhitscan = true;
			bool armshitscan = false;
			bool legshitscan = false;
			bool feethitscan = false;
			bool PositionAdjustment = true;
			int extrapolation = 2;
			bool delayshot = false;
			int Fakehitbox = 1;
			int AutomaticStop = 1;
			float headscale = 43.f;
			float bodyscale = 63.f;
			float Mindamage = 20.f;
			float Minhitchance = 63.f;
			int TickType = 1;
			bool baimifair = false;
			bool baimiflethal = true;
			bool baimifmove = false;
			bool baimifslow = true;
			int baimafterx = 1;
		} MG;
		struct SMG {
			bool enable = false;
			bool headhitscan = true;
			bool chesthitscan = true;
			bool stomachhitscan = true;
			bool armshitscan = false;
			bool legshitscan = false;
			bool feethitscan = false;
			bool PositionAdjustment = true;
			int extrapolation = 2;
			bool delayshot = false;
			int Fakehitbox = 1;
			int AutomaticStop = 1;
			float headscale = 43.f;
			float bodyscale = 63.f;
			float Mindamage = 20.f;
			float Minhitchance = 58.f;
			int TickType = 1;
			bool baimifair = false;
			bool baimiflethal = true;
			bool baimifslow = true;
			bool baimifmove = false;
			int baimafterx = 1;
		} SMG;
		struct Shotgun {
			bool enable = false;
			bool headhitscan = true;
			bool chesthitscan = true;
			bool stomachhitscan = true;
			bool armshitscan = false;
			bool legshitscan = false;
			bool feethitscan = false;
			bool PositionAdjustment = true;
			int extrapolation = 2;
			bool delayshot = false;
			int Fakehitbox = 1;
			int AutomaticStop = 1;
			float headscale = 43.f;
			float bodyscale = 63.f;
			float Mindamage = 50.f;
			float Minhitchance = 58.f;
			int TickType = 1;
			bool baimifair = false;
			bool baimiflethal = true;
			bool baimifslow = true;
			bool baimifmove = false;
			int baimafterx = 1;
		} Shotgun;
	} Ragebot;

	struct Antiaim
	{
		bool AntiaimEnable = false;
		bool DesyncToggle = false;
		bool Lines = false;
		bool Indicator = false;
		int AntiAimType;
		struct
		{
			int pitch;
			int DirectionType;
			float switchangle;
			int betamode;
			float realyaw;
			bool FakelagEnable = false;
			bool FakelagOnground = false;
			int FakelagMode = 0;
			int FakelagAmount = 0;
			int FakelagVariance = 0;
			bool FakelagShoot = false;

		}Stand;

		struct
		{
			int pitch;
			int DirectionType;
			float switchangle;
			int betamode;
			float realyaw;
			bool FakelagEnable = false;
			bool FakelagOnground = false;
			int FakelagMode = 0;
			int FakelagAmount = 0;
			int FakelagVariance = 0;
			bool FakelagShoot = false;
		}Move;

		struct
		{
			int pitch;
			int DirectionType;
			float switchangle;
			int betamode;
			float realyaw;
			bool FakelagEnable = false;
			bool FakelagOnground = false;
			int FakelagMode = 0;
			int FakelagAmount = 0;
			int FakelagVariance = 0;
			bool FakelagShoot = false;
		}Air;

		struct
		{
			int pitch;
			int DirectionType;
			float switchangle;
			int betamode;
			float realyaw;
			bool FakelagEnable = false;
			bool FakelagOnground = false;
			int FakelagMode = 0;
			int FakelagAmount = 0;
			int FakelagVariance = 0;
			bool FakelagShoot = false;
		}Slow;

		int JitterRange;
		int LBYDelta;
		int SpinSpeed;
		int DirectonType;
		int aatype;
		int aabeta;
		int switchkey;
		float betaoffset;
		int betapitch;
		bool FakeDuck;
		int FakeDuckKey;
	} Antiaim;

	struct Visuals
	{
		bool EspEnable = false;
		bool EnemyOnly = false;
		bool BoundingBox = false;
		bool Bones = false;
		bool ForceCrosshair = false;
		bool Health = false;
		bool Armor = false;
		bool FakeDuck = false;
		bool Scoped = false;
		bool Flags = false;
		bool Fake = false;
		bool Dlight = false;
		bool Name = false;
		bool Weapon = false;
		bool Ammo = false;
		bool AllItems = false;
		bool Rank = false;
		bool Radar = false;
		bool NoFlash = false;
		int thirdperson_dist = 100;
		bool Monitor = false;
		bool Wins = false;
		bool Glow = false;
		bool LGlow = false;
		bool PulseLGlow = false;
		bool LineofSight = false;
		bool SnapLines = false;
		bool GrenadePrediction = false;
		int Crosshair = 0;
		bool SpreadCrosshair = false;
		bool RecoilCrosshair = false;
		bool FartherESP = false;
		bool localESP = false;
		int manualaatype = 0;

		//Cbase/filers
		int DroppedWeapons = 0;
		bool Hostage = false;
		bool ThrownNades = false;
		bool LocalPlayer = false;
		bool BulletTracers = false;
		bool footstepesp = false;
		int bulletType = 0;
		bool Bomb = false;
		bool Spectators = false;
		bool OutOfPOVArrows = false;
		bool DamageIndicators = false;
		bool lbytimer = false;
		//Effects/world
		bool nightmode = false;
		bool worldcolor = false;
		bool blendonscope = false;
		int playeralpha = 255;
		int scopeplayeralpha = 255;
		int enemyalpha = 255;
		int Skybox = 0;
		int FlashbangAlpha = 0;
		bool Nosmoke = false;
		bool Noscope = false;
		bool RemoveParticles = false;
		bool Novisrevoil = false;
		bool Hitmarker = false;
		bool drawimpacts = false;
		int hitmarkerSize = 2;
		int hitmarkertype = 0;
		bool ChamsEnable = false;
		bool ShowBacktrack = false;
		int ChamsStyle = 0;
		bool ChamsL = false;
		bool FakeChams = false;
		bool fakelagcham = false;
		bool lagModel = false;
		bool crosssnip = false;
		bool Chamsenemyonly = false;
		bool ChamsPlayer = false;
		bool ChamsPlayerWall = false;
		bool ChamsHands = false;
		bool ChamsHandsWireframe = false;
		bool WeaponWireframe = false;
		bool WeaponChams = false;
		struct
		{
			int resolution = 5;
			int type;
		}Spread;
		int htSound = 0;
	} Visuals;

	struct Misc
	{
		bool AntiUT = true;
		bool WireHand = false;
		bool static_scope = false;
		int PlayerFOV = 0.f;
		int PlayerViewmodel = 0.f;
		int TPangles = 0;
		int TPKey = 0;
		int MenuKey = 0x2d;
		int WalkbotSet = 0x2d;
		int WalkbotDelete = 0x2d;
		int WalkbotStart = 0x2d;
		int Clantag = 0;
		char customclantag[128];
		bool FakeChams;
		bool skins;
		bool AutoJump = false;
		bool AutoStrafe = false;
		bool AutoAccept = false;
		bool Prespeed = false;
		int Retrack = 0.f;
		int PrespeedKey = 0;
		bool FakelagEnable = false;
		bool FakelagOnground = false;
		int FakelagMode = 0;
		int FakelagAmount = 0;
		int FakelagVariance = 0;
		bool FakelagShoot = false;
		int ConfigSelection = 0;
		bool Walkbot = false;
		bool WalkbotBunnyhop = false;
		//int WalkbotSetPoint = 0;
		//int WalkbotDeletePoint = 0;
		//int WalkbotStart = 0;
		bool Slowwalkenable = false;
		int Slowwalkkey = 0;
		bool strafeedgejump;
		int edgejumpkey = 0;
		int fakeduckkey = 0;
		float Slowwalkspeed;
		bool legitbacktrack;
		bool Spectators = true;
		bool KnifeBot = false;
		bool infiniteduck = false;
		bool ZeusBot = false;
		char configname[128];
		int circlestrafekey = 0;

		int aspectratio;
		bool aspectratioenable;
		bool viewmodelchange;
		bool PreserveKillfeed;
		int viewmodelx;
		int viewmodely;
		int viewmodelz;
	} Misc;

	struct Skins
	{
bool fixshit;
		bool glovesenabled;
		int gloves;
		int skingloves;
		float glovewear;
		bool skinenabled;
		int Knife;
		int KnifeSkin;
		int AK47Skin;
		int GalilSkin;
		int M4A1SSkin;
		int M4A4Skin;
		int AUGSkin;
		int FAMASSkin;
		int AWPSkin;
		int SSG08Skin;
		int SCAR20Skin;
		int P90Skin;
		int Mp7Skin;
		int NovaSkin;
		int UMP45Skin;
		int GlockSkin;
		int SawedSkin;
		int USPSkin;
		int MagSkin;
		int XmSkin;
		int DeagleSkin;
		int DualSkin;
		int FiveSkin;
		int RevolverSkin;
		int Mac10Skin;
		int tec9Skin;
		int Cz75Skin;
		int NegevSkin;
		int M249Skin;
		int Mp9Skin;
		int P2000Skin;
		int BizonSkin;
		int Sg553Skin;
		int P250Skin;
		int G3sg1Skin;
	} Skinchanger;

	struct CPlayerlist
	{
		bool bEnabled;
		int iPlayer;
		char* szPlayers[64] = {
			" ", " ", " ", " ", " ", " ", " ", " ", " ",
			" ", " ", " ", " ", " ", " ", " ", " ", " ",
			" ", " ", " ", " ", " ", " ", " ", " ", " ",
			" ", " ", " ", " ", " ", " ", " ", " ", " ",
			" ", " ", " ", " ", " ", " ", " ", " ", " ",
			" ", " ", " ", " ", " ", " ", " ", " ", " ",
			" ", " ", " ", " ", " ", " ", " ", " ", " ",
			" "
		};
	} Playerlist;

	struct NigColors
	{
		float MenuColor[3] = { 1.f, 1.f, 1.f};
		float Name[3] = { 1.f, 1.f, 1.f};
		float wpn[3] = { 1.f, 1.f, 1.f};
		float f[3] = { 1.f, 1.f, 1.f};

		float worldcolor_col[3] = { 1.f, 1.f, 1.f };
		float BoundingBox[3] = { 1.f, 1.f, 1.f};
		float PlayerChams[4] = { 1.f, 1.f, 1.f, 1.f };
		float ChamsHistory[4] = { 1.f, 1.f, 1.f, 1.f };
		float chamsalpha[4] = { 1.f, 1.f, 1.f, 1.f };
		float PlayerChamsl[4] = { 1.f, 1.f, 1.f, 1.f };
		float styleshands[4] = { 1.f, 1.f, 1.f, 0.6f };
		float PlayerChamsWall[4] = { 1.f, 1.f, 1.f, 1.f };
		float Skeletons[3] = { 1.f, 1.f, 1.f};
		float Bulletracer[3] = { 1.f, 1.f, 1.f};
		float BulletracerBox[4] = { 1.f, 1.f, 1.f, 0.7f };
		float FootstepEsp[3] = { 1.f, 1.f, 1.f};
		float WireframeHand[3] = { 1.f, 1.f, 1.f};
		float ChamsHand[4] = { 1.f, 1.f, 1.f, 1.f };
		float ChamsWeapon[4] = { 1.f, 1.f, 1.f, 1.f };
		float WireframeWeapon[4] = { 1.f, 1.f, 1.f, 1.f };
		float manualaacolor[3] = { 1.f, 1.f, 1.f };
		float Glow[4] = { 1.f, 1.f, 1.f, 0.7f };
		float LGlow[4] = { 1.f, 1.f, 1.f, 0.7f };
		float PlayerFakeChams[4] = { 1.f, 1.f, 1.f, 1.f };
		float nocheat[4] = { 1.f, 1.f, 1.f , 1.f };
		float DroppedWeapon[3] = { 1.f, 1.f, 1.f};
		float Bomb[3] = { 1.f, 1.f, 1.f};
		float PlantedBomb[3] = { 1.f, 1.f, 1.f};
		float Hostage[3] = { 1.f, 1.f, 1.f};
		float GrenadePrediction[3] = { 1.f, 1.f, 1.f};
		float FakeAngleGhost[4] = { 1.f, 1.f, 1.f, 1.f };
		float SpreadCrosshair[4] = { 1.f, 1.f, 1.f, 0.7f};
		float Snaplines[4] = { 1.f, 1.f, 1.f, 1.f };
		float DamageIndicator[3] = { 1.f, 1.f, 1.f };
		float lby_timer[4] = { 1.f, 1.f, 1.f, 1.f };
		float ammo[3] = { 1.f, 1.f, 1.f};
		float Radar[4] = { 1.f, 1.f, 1.f, 1.f };
		bool Props;
	}Colors;
	struct {
		int tabs;
		bool Opened;
	}Gui;
	bool Save(std::string file_name);
	bool Load(std::string file_name);
	void CreateConfig(std::string name);
	void Delete(std::string name);
	std::vector<std::string> GetConfigs();
};

extern ClientVariables Menu;
