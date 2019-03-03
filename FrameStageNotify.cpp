#include "hooks.h"
#include "global.h"
#include "Menu.h"
#include "Animfix.h"
#include "GloveChanger.h"
#include "BacktrackingHelper.h"
#include "xor.h"
#include <intrin.h>
#include "SpoofedConvar.h"
#include "Skinchanger.h"
#include "Math.h"
#include "GameUtils.h"
#include "Items.h"
#include "Aimbot.h"
#include "Extrapolation.h"
#include "Misc.h"
#include "Resolver.h"

//SMOKE MATERIALS
std::vector<const char*> smoke_materials = {
	"particle/beam_smoke_01",
	"particle/particle_smokegrenade",
	"particle/particle_smokegrenade1",
	"particle/particle_smokegrenade2",
	"particle/particle_smokegrenade3",
	"particle/particle_smokegrenade_sc",
	"particle/smoke1/smoke1",
	"particle/smoke1/smoke1_ash",
	"particle/smoke1/smoke1_nearcull",
	"particle/smoke1/smoke1_nearcull2",
	"particle/smoke1/smoke1_snow",
	"particle/smokesprites_0001",
	"particle/smokestack",
	"particle/vistasmokev1/vistasmokev1",
	"particle/vistasmokev1/vistasmokev1_emods",
	"particle/vistasmokev1/vistasmokev1_emods_impactdust",
	"particle/vistasmokev1/vistasmokev1_fire",
	"particle/vistasmokev1/vistasmokev1_nearcull",
	"particle/vistasmokev1/vistasmokev1_nearcull_fog",
	"particle/vistasmokev1/vistasmokev1_nearcull_nodepth",
	"particle/vistasmokev1/vistasmokev1_smokegrenade",
	"particle/vistasmokev1/vistasmokev4_emods_nocull",
	"particle/vistasmokev1/vistasmokev4_nearcull",
	"particle/vistasmokev1/vistasmokev4_nocull"
};
//SMOKE MATERIALS

/*DON'T GIVE JACKSHIT, DON'T TOCUH THESE BEAM SPRITES, THEY ARE DONE TO BE PRECISE TO OTHER CHEATS AND ON ACTUAL DETAIL LIKE SOUND RAGES*/
//BULLET BEAM DRAWING
void DrawBeam(Vector src, Vector end, Color color)
{
	BeamInfo_t beamInfo;
	beamInfo.m_nType = TE_BEAMTESLA;
	beamInfo.m_pszModelName = "sprites/purplelaser1.vmt";
	beamInfo.m_nModelIndex = -1;
	beamInfo.m_flHaloScale = 0.0f;
	beamInfo.m_flLife = 3.0f;
	beamInfo.m_flWidth = 2.0f;
	beamInfo.m_flEndWidth = 2.0f;
	beamInfo.m_flFadeLength = 0.0f;
	beamInfo.m_flAmplitude = 2.0f;
	beamInfo.m_flBrightness = 255.f;
	beamInfo.m_flSpeed = 0.2f;
	beamInfo.m_nStartFrame = 0;
	beamInfo.m_flFrameRate = 0.f;
	beamInfo.m_flRed = color.r();
	beamInfo.m_flGreen = color.g();
	beamInfo.m_flBlue = color.b();
	beamInfo.m_nSegments = 2;
	beamInfo.m_bRenderable = true;
	beamInfo.m_vecStart = src;
	beamInfo.m_vecEnd = end;
	Beam_t* myBeam = g_pViewRenderBeams->CreateBeamPoints(beamInfo);
	if (myBeam)
		g_pViewRenderBeams->DrawBeam(myBeam);
}
/*DON'T GIVE JACKSHIT, DON'T TOCUH THESE BEAM SPRITES, THEY ARE DONE TO BE PRECISE TO OTHER CHEATS AND ON ACTUAL DETAIL LIKE SOUND RAGES*/

//This is to draw those sound step esp
void DrawStepBeam(Color color, Vector position)
{
	BeamInfo_t beam_info;
	beam_info.m_nType = TE_BEAMRINGPOINT; //YOU NEED THIS
	beam_info.m_pszModelName = "sprites/purplelaser1.vmt";//Models
	beam_info.m_nModelIndex = g_pModelInfo->GetModelIndex("sprites/purplelaser1.vmt");//Models
	beam_info.m_pszHaloName = "sprites/purplelaser1.vmt"; //Models
	beam_info.m_nHaloIndex = g_pModelInfo->GetModelIndex("sprites/purplelaser1.vmt"); //Better glow then current beams
	beam_info.m_flHaloScale = 5.f;
	beam_info.m_flLife = 0.50f; //DON'T HAVE IT LIVE TO LONG OR YOUR GONNA FUCK FPS AND MAKE HUGE ASS RINGS
	beam_info.m_flWidth = 10.f; //WIDTH OF CIRCLE
	beam_info.m_flFadeLength = 1.0f; //THE FADE AT THE END
	beam_info.m_flAmplitude = 0.f; //DON'T TOUCH
	beam_info.m_flRed = color.r(); //COLOR
	beam_info.m_flGreen = color.g(); //COLOR
	beam_info.m_flBlue = color.b(); //COLOR
	beam_info.m_flBrightness = color.a(); //COLOR
	beam_info.m_flSpeed = 0.f; //SPEED IS NOT IMPORTANT
	beam_info.m_nStartFrame = 0.f; //FRAME THAT WE START FOR THE ANIMATION
	beam_info.m_flFrameRate = 60.f; //KEEP AT 60 SINCE PEOPLE SOMETIMES DON'T HAVE 120FPS
	beam_info.m_nSegments = 0; //GAY SHIT
	beam_info.m_bRenderable = true; //YES
	beam_info.m_nFlags = FBEAM_HALOBEAM; //TYPE OF BEAM
	beam_info.m_vecCenter = position + Vector(0, 0, 0.5); //HOW WE MAKE THE CIRCLE WITH VECTORS AND POSITIONS (POSITIONS WERE DEFINED BELOW IN THE FRAMESTAGENOTIFY IN OUR INFOLOGS)
	beam_info.m_flStartRadius = 20.f; //STARTING CIRCLE (SOUND DISTANCE RADIUS FROM MY TEST) (DON'T TOCUH SINCE IT'S THE RANGE OF SOUND THAT TRAVELS IN GAME)
	beam_info.m_flEndRadius = 500.f; //ENDING CIRCLE (SOUND DISTANCE RADIUS FROM MY TEST) (DON'T TOUCH SINCE IT'S THE RANGE OF SOUND THAT TRAVELS IN GAME)

	Beam_t* beam = g_pViewRenderBeams->CreateBeamRingPoint(beam_info); //THIS IS DIFFERENT TO BULLET BEAMS
	if (beam) //IF THE BEAM IS TRUE
		g_pViewRenderBeams->DrawBeam(beam); //DRAW BEAM
}
/*DON'T GIVE JACKSHIT, DON'T TOCUH THESE BEAM SPRITES, THEY ARE DONE TO BE PRECISE TO OTHER CHEATS AND ON ACTUAL DETAIL LIKE SOUND RAGES*/

bool IsWeaponDefaultKnife(int weaponid)
{
	if (weaponid == WEAPON_KNIFE || weaponid == WEAPON_KNIFE_T)
		return true;

	return false;
}
int GetKnifeDefinitionIndex(int si)
{
	switch (si)
	{
	case 1:
		return WEAPON_KNIFE_BAYONET; break;
	case 2:
		return WEAPON_KNIFE_FLIP; break;
	case 3:
		return WEAPON_KNIFE_GUT; break;
	case 4:
		return WEAPON_KNIFE_KARAMBIT; break;
	case 5:
		return WEAPON_KNIFE_M9_BAYONET; break;
	case 6:
		return WEAPON_KNIFE_TACTICAL; break;
	case 7:
		return WEAPON_KNIFE_BUTTERFLY; break;
	case 8:
		return WEAPON_KNIFE_FALCHION; break;
	case 9:
		return WEAPON_KNIFE_PUSH; break;
	case 10:
		return WEAPON_KNIFE_SURVIVAL_BOWIE; break;
	default:
		return -1;
	}
}


int CurrentKnifeModelIndex(int si)
{
	auto DefaultTT = g_pModelInfo->GetModelIndex(("models/weapons/v_knife_default_t.mdl"));
	auto DefaultCT = g_pModelInfo->GetModelIndex(("models/weapons/v_knife_default_ct.mdl"));
	auto Bayonet = g_pModelInfo->GetModelIndex(("models/weapons/v_knife_bayonet.mdl"));
	auto M9Bayonet = g_pModelInfo->GetModelIndex(("models/weapons/v_knife_m9_bay.mdl"));
	auto Butterfly = g_pModelInfo->GetModelIndex(("models/weapons/v_knife_butterfly.mdl"));
	auto Flip = g_pModelInfo->GetModelIndex(("models/weapons/v_knife_flip.mdl"));
	auto GunGame = g_pModelInfo->GetModelIndex(("models/weapons/v_knife_gg.mdl"));
	auto Gut = g_pModelInfo->GetModelIndex(("models/weapons/v_knife_gut.mdl"));
	auto Karambit = g_pModelInfo->GetModelIndex(("models/weapons/v_knife_karam.mdl"));
	auto Huntsman = g_pModelInfo->GetModelIndex(("models/weapons/v_knife_tactical.mdl"));
	auto Bowie = g_pModelInfo->GetModelIndex(("models/weapons/v_knife_survival_bowie.mdl"));
	auto Falchion = g_pModelInfo->GetModelIndex(("models/weapons/v_knife_falchion_advanced.mdl"));
	auto Dagger = g_pModelInfo->GetModelIndex(("models/weapons/v_knife_push.mdl"));
	CBaseEntity *pLocal = g_pEntitylist->GetClientEntity(g_pEngine->GetLocalPlayer());
	switch (si)
	{
	case 1:
		return Bayonet; break;
	case 2:
		return Flip; break;
	case 3:
		return Gut; break;
	case 4:
		return Karambit; break;
	case 5:
		return M9Bayonet; break;
	case 6:
		return Huntsman; break;
	case 7:
		return Butterfly; break;
	case 8:
		return Falchion; break;
	case 9:
		return Dagger; break;
	case 10:
		return Bowie; break;
	default:
		return pLocal->GetTeamNum() == 1 ? DefaultTT : DefaultCT;
	}
}

void __stdcall Hooks::FrameStageNotify(ClientFrameStage_t curStage)
{
	/*SKY BOX CHANGER*/
	static std::string old_Skyname = "";
	QAngle oldcolor = QAngle(0, 0, 0);
	static int OldSky;
	if (!csgo::LocalPlayer || !g_pEngine->IsConnected() || !g_pEngine->IsInGame())
	{
		clientVMT->GetOriginalMethod<FrameStageNotifyFn>(37)(curStage);
		old_Skyname = "";
		oldcolor = QAngle(0, 0, 0);
		OldSky = 0;
		return;
	}
	if (OldSky != Menu.Visuals.Skybox)
	{
		auto LoadNamedSky = reinterpret_cast<void(__fastcall*)(const char*)>(FindPatternIDA("engine.dll", "55 8B EC 81 EC ? ? ? ? 56 57 8B F9 C7 45"));
		if (old_Skyname == "")
		{
		}
		int type = Menu.Visuals.Skybox;

		if (type == 1)
			LoadNamedSky("cs_baggage_skybox_");
		else if (type == 2)
			LoadNamedSky("cs_tibet");
		else if (type == 3)
			LoadNamedSky("italy");
		else if (type == 4)
			LoadNamedSky("jungle");
		else if (type == 5)
			LoadNamedSky("office");
		else if (type == 6)
			LoadNamedSky("sky_cs15_daylight02_hdr");
		else if (type == 7)
			LoadNamedSky("sky_csgo_night02");
		else if (type == 8)
			LoadNamedSky("vertigo");
		OldSky = Menu.Visuals.Skybox;
	}
	/*SKY BOX CHANGER*/

	static Vector oldViewPunch;
	static Vector oldAimPunch;
	Vector* view_punch = csgo::LocalPlayer->GetViewPunchPtr();
	Vector* aim_punch = csgo::LocalPlayer->GetPunchAnglePtr();

	if (curStage == FRAME_RENDER_START)
	{
		//ThirdPerson Toggle (DON'T PASTE)
		static bool enabledtp = false, check = false;

		if (GetAsyncKeyState(Menu.Misc.TPKey))
		{
			if (!check)
				enabledtp = !enabledtp;
			check = true;
		}
		else
			check = false;
		
		//Third person (Broken)
		if (enabledtp && csgo::LocalPlayer->isAlive())
		{
			*reinterpret_cast<QAngle*>(reinterpret_cast<DWORD>(csgo::LocalPlayer + 0x31D8)) = csgo::RealAngle;
		}

		static bool bSpoofed = false;
		if (enabledtp && csgo::LocalPlayer->isAlive())
		{
			g_pEngine->ClientCmd_Unrestricted("thirdperson");
			csgo::b_IsThirdPerson = true;
		}
		else
		{
			g_pEngine->ClientCmd_Unrestricted("firstperson");
			csgo::b_IsThirdPerson = false;
		}

		//visual recoil
		if (view_punch && aim_punch && Menu.Visuals.Novisrevoil)
		{
			oldViewPunch = *view_punch;
			oldAimPunch = *aim_punch;

			view_punch->Init();
			aim_punch->Init();
		}

	}
	if (curStage == FRAME_NET_UPDATE_START)
	{
		if (Menu.Visuals.BulletTracers && Menu.Visuals.EspEnable)
		{
			float Red, Green, Blue;//Colors
			Red = Menu.Colors.Bulletracer[0] * 255;//Colors
			Green = Menu.Colors.Bulletracer[1] * 255;//Colors
			Blue = Menu.Colors.Bulletracer[2] * 255;//Colors
			for (unsigned int i = 0; i < trace_logs.size(); i++) { //Check if we even have a trace log
				auto *shooter = g_pEntitylist->GetClientEntity(g_pEngine->GetPlayerForUserID(trace_logs[i].userid)); //Identify
				if (!shooter) //If not the shooter, what's the point
					return;
				DrawBeam(trace_logs[i].start, trace_logs[i].position, Color(Red, Green, Blue, 255)); //Get the starting eye position, get the ending position with tracerays, and set a color
				trace_logs.erase(trace_logs.begin() + i);//Erase so we don't fuck up our fps
			}
		}
		for (int i = 0; i < g_pEngine->GetMaxClients(); ++i) {
			CBaseEntity* e = g_pEntitylist->GetClientEntity(i);
			if (!e || !e->isAlive() || e->IsDormant() || e == csgo::LocalPlayer)
				continue;

			g_Interpolation->disable_interpolation(e);

		}
		if (Menu.Visuals.footstepesp && Menu.Visuals.EspEnable)
		{
			float Red, Green, Blue; //Colors
			Red = Menu.Colors.FootstepEsp[0] * 255;//Colors
			Green = Menu.Colors.FootstepEsp[1] * 255;//Colors
			Blue = Menu.Colors.FootstepEsp[2] * 255;//Colors
			for (unsigned int i = 0; i < sound_logs.size(); i++) //Check if we even have a sound log
			{
				DrawStepBeam(Color(Red, Green, Blue, 255), sound_logs[i].position); //We will get the color, and draw at the abs vector (NOT VEC VECTOR SINCE THAT'S GAY) DON'T FUCKING DO IT
				sound_logs.erase(sound_logs.begin() + i); //Erase so we don't fuck up our fps
			}
		}	
	}
	/*Fake Lag PREDICTION*/
	if (curStage == FRAME_RENDER_START)
	{
		if (csgo::LocalPlayer && csgo::LocalPlayer->isAlive())
		{
			for (int i = 1; i < g_pGlobals->maxClients; i++)
			{
				CBaseEntity* pEntity = g_pEntitylist->GetClientEntity(i);
				if (pEntity)
				{
					if (pEntity->GetHealth() > 0)
					{
						if (i != g_pEngine->GetLocalPlayer())
						{
							VarMapping_t* map = pEntity->GetVarMap();
							if (!map) return;
							for (int i = 0; i < map->m_nInterpolatedEntries; i++)
							{
								VarMapEntry_t *e = &map->m_Entries[i];
								e->m_bNeedsToInterpolate = false;
							}

						}
					}
				}
			}
		}
	}
	if (curStage == FRAME_RENDER_START)
	{
		for (int i = 1; i < g_pGlobals->maxClients; i++)
		{
			if (i == g_pEngine->GetLocalPlayer())
				continue;
			CBaseEntity* pEntity = g_pEntitylist->GetClientEntity(i);
			if (pEntity)
			{
				if (pEntity->GetHealth() > 0 && !pEntity->IsDormant())
				{
					*(int*)((uintptr_t)pEntity + 0xA30) = g_pGlobals->framecount; //we'll skip occlusion checks now
					*(int*)((uintptr_t)pEntity + 0xA28) = 0;//clear occlusion flags
				}
			}
		}
	}
	clientVMT->GetOriginalMethod<FrameStageNotifyFn>(37)(curStage);
	/*INSERT RESOLVER FOR THOSE DESYNCERS*/
	if (curStage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
	{
		NewSkinChanger();
		GloveChanger();
	}
	g_BacktrackHelper->AnimationFix(curStage);

	if (curStage == FRAME_RENDER_START)
	{
		for (int i = 1; i < g_pGlobals->maxClients; i++)
		{
			CBaseEntity* pEntity = g_pEntitylist->GetClientEntity(i);
			if (pEntity)
			{
				if (pEntity->GetHealth() > 0 && !pEntity->IsDormant() && (pEntity->GetTeamNum() != csgo::LocalPlayer->GetTeamNum()) && pEntity->isAlive() && (pEntity != csgo::LocalPlayer))
				{
					g_BacktrackHelper->UpdateBacktrackRecords(pEntity);
					g_Extrapolation->UpdateRecords(pEntity);
					g_Resolver->update(pEntity);
					if (Menu.Ragebot.AutomaticResolver && Menu.Ragebot.ResolverType == 0)
						g_Resolver->DefaultResolver(pEntity);
					else if (Menu.Ragebot.AutomaticResolver && Menu.Ragebot.ResolverType == 1)
						g_Resolver->ExperimentalResolver(pEntity);
				}
			}
		}
	}
	if (curStage == FRAME_RENDER_START && csgo::LocalPlayer && csgo::LocalPlayer->GetHealth() > 0)
	{
		for (auto material_name : smoke_materials) {
			IMaterial* mat = g_pMaterialSystem->FindMaterial(material_name, TEXTURE_GROUP_OTHER); //Find the materials
			mat->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, Menu.Visuals.Nosmoke ? true : false);//Set the material to nothing
		}
		if (Menu.Visuals.Nosmoke) { //Pull the amount of smoke from client_panorama.dll
			static int* smokecount = *(int**)(FindPatternIDA(client_dll, "8B 1D ? ? ? ? 56 33 F6 57 85 DB") + 0x2);
			*smokecount = 0;
		}
		if (Menu.Visuals.NoFlash) {
			IMaterial* Flash = g_pMaterialSystem->FindMaterial("effects\\flashbang", "ClientEffect textures"); //Find flashbang effects
			IMaterial* FlashWhite = g_pMaterialSystem->FindMaterial("effects\\flashbang_white", "ClientEffect textures");//Find flashbang effects
			Flash->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true); //Set the material to nothing
			FlashWhite->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, true);//Set the material to nothing
		}
		if (Menu.Visuals.Novisrevoil)
		{
			*aim_punch = oldAimPunch;
			*view_punch = oldViewPunch;
		}
	}
}


