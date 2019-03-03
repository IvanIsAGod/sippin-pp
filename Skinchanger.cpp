#include "Skinchanger.h"
#include <unordered_map>
#define RandomInt(nMin, nMax) (rand() % (nMax - nMin + 1) + nMin);
std::unordered_map<char*, char*> killIcons = {};
#define INVALID_EHANDLE_INDEX 0xFFFFFFFF
HANDLE worldmodel_handle;
CBaseCombatWeapon* worldmodel;
#define MakePtr(cast, ptr, addValue) (cast)( (DWORD)(ptr) + (DWORD)(addValue))

void SetKnifeSkin(CBaseCombatWeapon* pWeapon) {
	int Skin = Menu.Skinchanger.KnifeSkin;
	if (Skin == 0)
	{
		*pWeapon->FallbackPaintKit() = 1; // Default
	}
	else if (Skin == 1)
	{
		*pWeapon->FallbackPaintKit() = 12; // Crimson Web
	}
	else if (Skin == 2)
	{
		*pWeapon->FallbackPaintKit() = 27; // Bone Mask
	}
	else if (Skin == 3)
	{
		*pWeapon->FallbackPaintKit() = 38; // Fade
	}
	else if (Skin == 4)
	{
		*pWeapon->FallbackPaintKit() = 40; // Night
	}
	else if (Skin == 5)
	{
		*pWeapon->FallbackPaintKit() = 42; // Blue Steel
	}
	else if (Skin == 6)
	{
		*pWeapon->FallbackPaintKit() = 43; // Stained
	}
	else if (Skin == 7)
	{
		*pWeapon->FallbackPaintKit() = 44; // Case Hardened
	}
	else if (Skin == 8)
	{
		*pWeapon->FallbackPaintKit() = 59; // Slaughter
	}
	else if (Skin == 9)
	{
		*pWeapon->FallbackPaintKit() = 72; // Safari Mesh
	}
	else if (Skin == 10)
	{
		*pWeapon->FallbackPaintKit() = 77; // Boreal Forest
	}
	else if (Skin == 11)
	{
		*pWeapon->FallbackPaintKit() = 98; // Ultraviolet
	}
	else if (Skin == 12)
	{
		*pWeapon->FallbackPaintKit() = 143; // Urban Masked
	}
	else if (Skin == 13)
	{
		*pWeapon->FallbackPaintKit() = 175; // Scorched
	}
	else if (Skin == 14)
	{
		*pWeapon->FallbackPaintKit() = 323; // Rust Coat
	}
	else if (Skin == 15)
	{
		*pWeapon->FallbackPaintKit() = 409; // Tiger Tooth
	}
	else if (Skin == 16)
	{
		*pWeapon->FallbackPaintKit() = 410; // Damascus Steel
	}
	else if (Skin == 17)
	{
		*pWeapon->FallbackPaintKit() = 411; // Damascus Steel
	}
	else if (Skin == 18)
	{
		*pWeapon->FallbackPaintKit() = 413; // Marble Fade
	}
	else if (Skin == 19)
	{
		*pWeapon->FallbackPaintKit() = 414; // Rust Coat
	}
	else if (Skin == 20)
	{
		*pWeapon->FallbackPaintKit() = 415; // Doppler Ruby
	}
	else if (Skin == 21)
	{
		*pWeapon->FallbackPaintKit() = 416; // Doppler Sapphire
	}
	else if (Skin == 22)
	{
		*pWeapon->FallbackPaintKit() = 417; // Doppler Blackpearl
	}
	else if (Skin == 23)
	{
		*pWeapon->FallbackPaintKit() = 418; // Doppler Phase 1
	}
	else if (Skin == 24)
	{
		*pWeapon->FallbackPaintKit() = 419; // Doppler Phase 2
	}
	else if (Skin == 25)
	{
		*pWeapon->FallbackPaintKit() = 420; // Doppler Phase 3
	}
	else if (Skin == 26)
	{
		*pWeapon->FallbackPaintKit() = 421; // Doppler Phase 4
	}
	else if (Skin == 27)
	{
		*pWeapon->FallbackPaintKit() = 569; // Gamma Doppler Phase1
	}
	else if (Skin == 28)
	{
		*pWeapon->FallbackPaintKit() = 570; // Gamma Doppler Phase2
	}
	else if (Skin == 29)
	{
		*pWeapon->FallbackPaintKit() = 571; // Gamma Doppler Phase3
	}
	else if (Skin == 30)
	{
		*pWeapon->FallbackPaintKit() = 568; // Gamma Doppler Phase4
	}
	else if (Skin == 31)
	{
		*pWeapon->FallbackPaintKit() = 568; // Gamma Doppler Emerald
	}
	else if (Skin == 32)
	{
		*pWeapon->FallbackPaintKit() = 561; // Lore
	}
	else if (Skin == 33)
	{
		*pWeapon->FallbackPaintKit() = 567; // Black Laminate
	}
	else if (Skin == 34)
	{
		*pWeapon->FallbackPaintKit() = 574; // Autotronic
	}
	else if (Skin == 35)
	{
		*pWeapon->FallbackPaintKit() = 580; // Freehand
	}
}

void NewSkinChanger()
{
	if (csgo::LocalPlayer->isAlive())
	{
		int nLocalPlayerID = g_pEngine->GetLocalPlayer();
		CBaseEntity* pLocal = (CBaseEntity*)g_pEntitylist->GetClientEntity(nLocalPlayerID);

		UINT* armas = pLocal->m_hMyWeapons();
		for (size_t i = 0; armas[i]; i++) {

			auto pWeapon = reinterpret_cast<CBaseCombatWeapon*>(g_pEntitylist->GetClientEntityFromHandle(armas[i]));

			CBaseEntity *gunent = g_pEntitylist->GetClientEntityFromHandle(armas[i]);

			if (gunent)
			{
				if (gunent == nullptr)
					return;

				if (pWeapon)
				{
					if (pWeapon == nullptr)
						return;

					CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)gunent;
					ClientClass *pClass = g_pClient->GetAllClasses();
					if (Menu.Skinchanger.skinenabled)
					{
						int iBayonet = g_pModelInfo->GetModelIndex("models/weapons/v_knife_bayonet.mdl");
						int iButterfly = g_pModelInfo->GetModelIndex("models/weapons/v_knife_butterfly.mdl");
						int iFlip = g_pModelInfo->GetModelIndex("models/weapons/v_knife_flip.mdl");
						int iGut = g_pModelInfo->GetModelIndex("models/weapons/v_knife_gut.mdl");
						int iKarambit = g_pModelInfo->GetModelIndex("models/weapons/v_knife_karam.mdl");
						int iM9Bayonet = g_pModelInfo->GetModelIndex("models/weapons/v_knife_m9_bay.mdl");
						int iHuntsman = g_pModelInfo->GetModelIndex("models/weapons/v_knife_tactical.mdl");
						int iFalchion = g_pModelInfo->GetModelIndex("models/weapons/v_knife_falchion_advanced.mdl");
						int iDagger = g_pModelInfo->GetModelIndex("models/weapons/v_knife_push.mdl");
						int iBowie = g_pModelInfo->GetModelIndex("models/weapons/v_knife_survival_bowie.mdl");
						int iGunGame = g_pModelInfo->GetModelIndex("models/weapons/v_knife_gg.mdl");
						int Model = Menu.Skinchanger.Knife;
						if (gunent->GetClientClass()->m_ClassID == (int)CSGOClassID::CKnife)
						{
							if (Model == 0) // Bayonet
							{
								pWeapon->SetModelIndex(iBayonet);
								*pWeapon->ViewModelIndex() = iBayonet;
								*pWeapon->WorldModelIndex() = iBayonet + 1;
								*pWeapon->ItemDefinitionIndex() = 500;
								*pWeapon->GetEntityQuality() = 3;
								SetKnifeSkin(pWeapon);
							}
							else if (Model == 1) // Bowie Knife
							{
								pWeapon->SetModelIndex(iBowie);
								*pWeapon->ViewModelIndex() = iBowie;
								*pWeapon->WorldModelIndex() = iBowie + 1;
								*pWeapon->ItemDefinitionIndex() = 514;
								*pWeapon->GetEntityQuality() = 3;
								SetKnifeSkin(pWeapon);
							}
							else if (Model == 2) // Butterfly Knife
							{
								pWeapon->SetModelIndex(iButterfly);
								*pWeapon->ViewModelIndex() = iButterfly;
								*pWeapon->WorldModelIndex() = iButterfly + 1;
								*pWeapon->ItemDefinitionIndex() = 515;
								*pWeapon->GetEntityQuality() = 3;
								SetKnifeSkin(pWeapon);
							}
							else if (Model == 3) // Falchion Knife
							{
								pWeapon->SetModelIndex(iFalchion);
								*pWeapon->ViewModelIndex() = iFalchion;
								*pWeapon->WorldModelIndex() = iFalchion + 1;
								*pWeapon->ItemDefinitionIndex() = 512;
								*pWeapon->GetEntityQuality() = 3;
								SetKnifeSkin(pWeapon);
							}
							else if (Model == 4) // Flip Knife
							{
								pWeapon->SetModelIndex(iFlip);
								*pWeapon->ViewModelIndex() = iFlip;
								*pWeapon->WorldModelIndex() = iFlip + 1;
								*pWeapon->ItemDefinitionIndex() = 505;
								SetKnifeSkin(pWeapon);
							}
							else if (Model == 5) // Gut Knife
							{
								pWeapon->SetModelIndex(iGut);
								*pWeapon->ViewModelIndex() = iGut;
								*pWeapon->WorldModelIndex() = iGut + 1;
								*pWeapon->ItemDefinitionIndex() = 506;
								*pWeapon->GetEntityQuality() = 3;
								SetKnifeSkin(pWeapon);
							}
							else if (Model == 6) // Huntsman Knife
							{
								pWeapon->SetModelIndex(iHuntsman);
								*pWeapon->ViewModelIndex() = iHuntsman;
								*pWeapon->WorldModelIndex() = iHuntsman + 1;
								*pWeapon->ItemDefinitionIndex() = 509;
								*pWeapon->GetEntityQuality() = 3;
								SetKnifeSkin(pWeapon);
							}
							else if (Model == 7) // Karambit
							{
								pWeapon->SetModelIndex(iKarambit);
								*pWeapon->ViewModelIndex() = iKarambit;
								*pWeapon->WorldModelIndex() = iKarambit + 1;
								*pWeapon->ItemDefinitionIndex() = 507;
								*pWeapon->GetEntityQuality() = 3;
								SetKnifeSkin(pWeapon);
							}
							else if (Model == 8) // M9 Bayonet
							{
								pWeapon->SetModelIndex(iM9Bayonet);
								*pWeapon->ViewModelIndex() = iM9Bayonet;
								*pWeapon->WorldModelIndex() = iM9Bayonet + 1;
								*pWeapon->ItemDefinitionIndex() = 508;
								*pWeapon->GetEntityQuality() = 3;
								SetKnifeSkin(pWeapon);
							}
							else if (Model == 10) // Shadow Daggers
							{
								pWeapon->SetModelIndex(iDagger);
								*pWeapon->ViewModelIndex() = iDagger;
								*pWeapon->WorldModelIndex() = iDagger + 1;
								*pWeapon->ItemDefinitionIndex() = 516;
								*pWeapon->GetEntityQuality() = 3;
								SetKnifeSkin(pWeapon);
							}

							*pWeapon->OwnerXuidLow() = 0;//crash?
							*pWeapon->OwnerXuidHigh() = 0;
							*pWeapon->FallbackWear() = 0.0000001f;
							*pWeapon->ItemIDHigh() = 1;
						}

						int M41S = Menu.Skinchanger.M4A1SSkin;
						int M4A4 = Menu.Skinchanger.M4A4Skin;
						int AK47 = Menu.Skinchanger.AK47Skin;
						int AWP = Menu.Skinchanger.AWPSkin;
						int GLOCK = Menu.Skinchanger.GlockSkin;
						int USPS = Menu.Skinchanger.USPSkin;
						int DEAGLE = Menu.Skinchanger.DeagleSkin;
						int FIVE7 = Menu.Skinchanger.FiveSkin;
						int AUG = Menu.Skinchanger.AUGSkin;
						int FAMAS = Menu.Skinchanger.FAMASSkin;
						int G3SG1 = Menu.Skinchanger.G3sg1Skin;
						int Galil = Menu.Skinchanger.GalilSkin;
						int M249 = Menu.Skinchanger.M249Skin;
						int MAC10 = Menu.Skinchanger.Mac10Skin;
						int P90 = Menu.Skinchanger.P90Skin;
						int UMP45 = Menu.Skinchanger.UMP45Skin;
						int XM1014 = Menu.Skinchanger.XmSkin;
						int BIZON = Menu.Skinchanger.BizonSkin;
						int MAG7 = Menu.Skinchanger.MagSkin;
						int NEGEV = Menu.Skinchanger.NegevSkin;
						int SAWEDOFF = Menu.Skinchanger.SawedSkin;
						int TEC9 = Menu.Skinchanger.tec9Skin;
						int P2000 = Menu.Skinchanger.P2000Skin;
						int MP7 = Menu.Skinchanger.Mp7Skin;
						int MP9 = Menu.Skinchanger.Mp9Skin;
						int NOVA = Menu.Skinchanger.NovaSkin;
						int P250 = Menu.Skinchanger.P250Skin;
						int SCAR20 = Menu.Skinchanger.SCAR20Skin;
						int SG553 = Menu.Skinchanger.Sg553Skin;
						int SSG08 = Menu.Skinchanger.SSG08Skin;
						int Magnum = Menu.Skinchanger.RevolverSkin;
						int Duals = Menu.Skinchanger.DualSkin;

						int weapon = *pWeapon->ItemDefinitionIndex();

						switch (weapon)
						{
						case 7: // AK47 
						{
							switch (AK47)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;//none
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 180;//fire serpent
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 524;//Fuel Injector
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 639;//Bloodsport
								break;
							case 4:
								*pWeapon->FallbackPaintKit() = 302;//vulcan
								break;
							case 5:
								*pWeapon->FallbackPaintKit() = 44;//case hardened
								break;
							case 6:
								*pWeapon->FallbackPaintKit() = 456;//Hydroponic
								break;
							case 7:
								*pWeapon->FallbackPaintKit() = 474;//Aquamarine Revenge
								break;
							case 8:
								*pWeapon->FallbackPaintKit() = 490;//Frontside Misty
								break;
							case 9:
								*pWeapon->FallbackPaintKit() = 506;//Point Disarray
								break;
							case 10:
								*pWeapon->FallbackPaintKit() = 600;//Neon Revolution
								break;
							case 11:
								*pWeapon->FallbackPaintKit() = 14;//red laminate
								break;
							case 12:
								*pWeapon->FallbackPaintKit() = 282;//redline
								break;
							case 13:
								*pWeapon->FallbackPaintKit() = 316;//jaguar
								break;
							case 14:
								*pWeapon->FallbackPaintKit() = 340;//jetset
								break;
							case 15:
								*pWeapon->FallbackPaintKit() = 380;//wasteland rebel
								break;
							case 16:
								*pWeapon->FallbackPaintKit() = 675;//The Empress
								break;
							case 17:
								*pWeapon->FallbackPaintKit() = 422;//Elite Build
								break;
							default:
								break;
							}
						}
						break;
						case 16: // M4A4
						{
							switch (M4A4)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;//none
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 255;//Asiimov
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 309;//Howl
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 400;//Dragon King
								break;
							case 4:
								*pWeapon->FallbackPaintKit() = 449;//Poseidon
								break;
							case 5:
								*pWeapon->FallbackPaintKit() = 471;//Daybreak
								break;
							case 6:
								*pWeapon->FallbackPaintKit() = 512;//Royal Paladin
								break;
							case 7:
								*pWeapon->FallbackPaintKit() = 533;//BattleStar
								break;
							case 8:
								*pWeapon->FallbackPaintKit() = 588;//Desolate Space
								break;
							case 9:
								*pWeapon->FallbackPaintKit() = 632;//Buzz Kill
								break;
							case 10:
								*pWeapon->FallbackPaintKit() = 155;//Bullet Rain
								break;
							case 11:
								*pWeapon->FallbackPaintKit() = 664;//Hell Fire
								break;
							case 12:
								*pWeapon->FallbackPaintKit() = 480;//Evil Daimyo
								break;
							case 13:
								*pWeapon->FallbackPaintKit() = 384;//Griffin
								break;
							case 14:
								*pWeapon->FallbackPaintKit() = 187;//Zirka
								break;
							case 15:
								*pWeapon->FallbackPaintKit() = 167;//Radiation Harzard
								break;
							default:
								break;
							}
						}
						break;
						case 2: // dual
						{
							switch (Duals)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;//none
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 276;
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 491;
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 658;
								break;
							case 4:
								*pWeapon->FallbackPaintKit() = 625;
								break;
							case 5:
								*pWeapon->FallbackPaintKit() = 447;
								break;
							default:
								break;
							}
						}
						break;
						case 60: // M4A1
						{
							switch (M41S)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;//none
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 644;//Decimator
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 326;//Knight
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 548;//Chantico's Fire
								break;
							case 4:
								*pWeapon->FallbackPaintKit() = 497;//Golden Coi
								break;
							case 5:
								*pWeapon->FallbackPaintKit() = 430;//Hyper Beast
								break;
							case 6:
								*pWeapon->FallbackPaintKit() = 321;//Master Piece
								break;
							case 7:
								*pWeapon->FallbackPaintKit() = 445;//Hot Rod
								break;
							case 8:
								*pWeapon->FallbackPaintKit() = 587;//Mecha Industries
								break;
							case 9:
								*pWeapon->FallbackPaintKit() = 360;//Cyrex
								break;
							case 10:
								*pWeapon->FallbackPaintKit() = 440;//Icarus Fell
								break;
							case 11:
								*pWeapon->FallbackPaintKit() = 631;//Flashback
								break;
							case 12:
								*pWeapon->FallbackPaintKit() = 681;//Flashback
								break;
							case 13:
								*pWeapon->FallbackPaintKit() = 430;//Hyper Beast
								break;
							case 14:
								*pWeapon->FallbackPaintKit() = 301;//Atomic Alloy
								break;
							case 15:
								*pWeapon->FallbackPaintKit() = 257;//Guardian
								break;
							case 16:
								*pWeapon->FallbackPaintKit() = 663;//Briefing
								break;
							default:
								break;
							}
						}
						break;
						case 9: // AWP
						{
							switch (AWP)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;//none
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 279;//asiimov
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 344;//dlore
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 640;//Fever Dream
								break;
							case 4:
								*pWeapon->FallbackPaintKit() = 446;//medusa
								break;
							case 5:
								*pWeapon->FallbackPaintKit() = 475;//hyperbeast
								break;
							case 6:
								*pWeapon->FallbackPaintKit() = 174;//boom
								break;
							case 7:
								*pWeapon->FallbackPaintKit() = 51;//lightning strike
								break;
							case 8:
								*pWeapon->FallbackPaintKit() = 84;//pink ddpat
								break;
							case 9:
								*pWeapon->FallbackPaintKit() = 181;//corticera
								break;
							case 10:
								*pWeapon->FallbackPaintKit() = 259;//redline
								break;
							case 11:
								*pWeapon->FallbackPaintKit() = 395;//manowar
								break;
							case 12:
								*pWeapon->FallbackPaintKit() = 212;//graphite
								break;
							case 13:
								*pWeapon->FallbackPaintKit() = 227;//electric hive
								break;
							case 14:
								*pWeapon->FallbackPaintKit() = 451;//Sun in Leo
								break;
							case 15:
								*pWeapon->FallbackPaintKit() = 475;//Hyper Beast
								break;
							case 16:
								*pWeapon->FallbackPaintKit() = 251;//Pit viper
								break;
							case 17:
								*pWeapon->FallbackPaintKit() = 584;//Phobos
								break;
							case 18:
								*pWeapon->FallbackPaintKit() = 525;//Elite Build
								break;
							case 19:
								*pWeapon->FallbackPaintKit() = 424;//Worm God
								break;
							case 20:
								*pWeapon->FallbackPaintKit() = 662;//Oni Taiji
								break;
							case 21:
								*pWeapon->FallbackPaintKit() = 640;//Fever Dream
								break;
							default:
								break;
							}
						}
						break;
						case 61: // USP
						{
							switch (USPS)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;//none
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 653;//Neo-Noir
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 637;//Cyrex
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 313;//Orion
								break;
							case 4:
								*pWeapon->FallbackPaintKit() = 504;//Kill Confirmed
								break;
							case 5:
								*pWeapon->FallbackPaintKit() = 183;//Overgrowth
								break;
							case 6:
								*pWeapon->FallbackPaintKit() = 339;//Caiman
								break;
							case 7:
								*pWeapon->FallbackPaintKit() = 221;//Serum
								break;
							case 8:
								*pWeapon->FallbackPaintKit() = 290;//Guardian
								break;
							case 9:
								*pWeapon->FallbackPaintKit() = 318;//Road Rash
								break;
							default:
								break;
							}
						}
						break;
						case 4: // Glock
						{
							switch (GLOCK)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;//none
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 38;
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 48;
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 437;
								break;
							case 4:
								*pWeapon->FallbackPaintKit() = 586;
								break;
							case 5:
								*pWeapon->FallbackPaintKit() = 353;
								break;
							case 6:
								*pWeapon->FallbackPaintKit() = 680;
								break;
							case 7:
								*pWeapon->FallbackPaintKit() = 607;
								break;
							case 8:
								*pWeapon->FallbackPaintKit() = 532;
								break;
							case 9:
								*pWeapon->FallbackPaintKit() = 381;
								break;
							case 10:
								*pWeapon->FallbackPaintKit() = 230;
								break;
							case 11:
								*pWeapon->FallbackPaintKit() = 159;
								break;
							case 12:
								*pWeapon->FallbackPaintKit() = 623;
								break;
							case 13:
								*pWeapon->FallbackPaintKit() = 479;
								break;
							case 14:
								*pWeapon->FallbackPaintKit() = 680;
								break;
							case 15:
								*pWeapon->FallbackPaintKit() = 367;
								break;
							default:
								break;
							}
						}
						break;
						case 1: // Deagle
						{
							switch (DEAGLE)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;//none
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 37;
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 527;
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 645;
								break;
							case 4:
								*pWeapon->FallbackPaintKit() = 185;
								break;
							case 5:
								*pWeapon->FallbackPaintKit() = 37;
								break;
							case 6:
								*pWeapon->FallbackPaintKit() = 645;
								break;
							case 7:
								*pWeapon->FallbackPaintKit() = 231;
								break;
							case 8:
								*pWeapon->FallbackPaintKit() = 603;
								break;
							default:
								break;
							}
						}
						break;
						case 3: // Five Seven
						{
							switch (FIVE7)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;//none
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 427;
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 660;
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 352;
								break;
							case 4:
								*pWeapon->FallbackPaintKit() = 530;
								break;
							case 5:
								*pWeapon->FallbackPaintKit() = 510;
								break;
							case 6:
								*pWeapon->FallbackPaintKit() = 646;
								break;
							case 7:
								*pWeapon->FallbackPaintKit() = 585;
								break;
							default:
								break;
							}
						}
						break;
						case 8: // AUG
						{
							switch (AUG)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;//none
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 9;
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 33;
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 280;
								break;
							case 4:
								*pWeapon->FallbackPaintKit() = 455;
								break;
							default:
								break;
							}
						}
						break;
						case 10: // Famas
						{
							switch (Menu.Skinchanger.FAMASSkin)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;//none
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 429;
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 371;
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 477;
								break;
							case 4:
								*pWeapon->FallbackPaintKit() = 492;
								break;
							default:
								break;
							}
						}
						break;
						case 11: // G3SG1
						{
							switch (Menu.Skinchanger.G3sg1Skin)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;//none
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 677;
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 511;
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 463;
								break;
							default:
								break;
							}
						}
						break;
						case 13: // Galil
						{
							switch (Menu.Skinchanger.GalilSkin)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;//none
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 398;
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 647;
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 661;
								break;
							case 4:
								*pWeapon->FallbackPaintKit() = 428;
								break;
							case 5:
								*pWeapon->FallbackPaintKit() = 379;
								break;
							default:
								break;
							}
						}
						break;
						case 14: // M249
						{
							switch (Menu.Skinchanger.M249Skin)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 496;
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 401;
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 266;
								break;
							default:
								break;
							}
						}
						break;
						case 17: // Mac 10
						{
							switch (Menu.Skinchanger.Mac10Skin)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;//none
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 433;
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 651;
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 310;
								break;
							case 4:
								*pWeapon->FallbackPaintKit() = 498;
								break;
							default:
								break;
							}
						}
						break;
						case 19: // P90
						{
							*pWeapon->FallbackPaintKit() = 156;
						}
						break;
						case 24: // UMP-45
						{
							switch (Menu.Skinchanger.UMP45Skin)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;//none
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 37;
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 441;
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 448;
								break;
							case 4:
								*pWeapon->FallbackPaintKit() = 556;
								break;
							case 5:
								*pWeapon->FallbackPaintKit() = 688;
								break;
							default:
								break;
							}
						}
						break;
						case 25: // XM1014
						{
							switch (Menu.Skinchanger.XmSkin)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;//none
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 654;
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 363;
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 689;
								break;
							default:
								break;
							}
						}
						break;
						case 63: // CZ75-Auto
						{
							switch (Menu.Skinchanger.Cz75Skin)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;//none
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 543;
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 435;
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 270;
								break;
							case 4:
								*pWeapon->FallbackPaintKit() = 643;
								break;
							default:
								break;
							}
						}
						break;
						case 26: // Bizon
						{
							switch (Menu.Skinchanger.BizonSkin)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 676;
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 542;
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 508;
								break;
							default:
								break;
							}
						}
						break;
						case 27: // Mag 7
						{
							switch (Menu.Skinchanger.MagSkin)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;//none
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 39;
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 431;
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 608;
								break;
							default:
								break;
							}
						}
						break;
						case 28: // Negev
						{
							switch (Menu.Skinchanger.NegevSkin)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;//none
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 514;
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 483;
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 432;
								break;
							default:
								break;
							}
						}
						break;
						case 29: // Sawed Off
						{
							switch (Menu.Skinchanger.SawedSkin)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;//none
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 638;
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 256;
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 517;
								break;
							default:
								break;
							}
						}
						break;
						case 30: // Tec 9
						{
							switch (Menu.Skinchanger.tec9Skin)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;//none
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 179;
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 248;
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 216;
								break;
							case 4:
								*pWeapon->FallbackPaintKit() = 272;
								break;
							case 5:
								*pWeapon->FallbackPaintKit() = 289;
								break;
							case 6:
								*pWeapon->FallbackPaintKit() = 303;
								break;
							case 7:
								*pWeapon->FallbackPaintKit() = 374;
								break;
							case 8:
								*pWeapon->FallbackPaintKit() = 555;
								break;
							case 9:
								*pWeapon->FallbackPaintKit() = 614;
								break;
							default:
								break;
							}
						}
						break;
						case 32: // P2000
						{
							switch (Menu.Skinchanger.P2000Skin)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;//none
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 485;
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 38;
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 184;
								break;
							case 4:
								*pWeapon->FallbackPaintKit() = 211;
								break;
							case 5:
								*pWeapon->FallbackPaintKit() = 389;
								break;
							case 6:
								*pWeapon->FallbackPaintKit() = 442;
								break;
							case 7:
								*pWeapon->FallbackPaintKit() = 443;
								break;
							case 8:
								*pWeapon->FallbackPaintKit() = 515;
								break;
							case 9:
								*pWeapon->FallbackPaintKit() = 550;
								break;
							case 10:
								*pWeapon->FallbackPaintKit() = 591;
								break;
							default:
								break;
							}
						}
						break;
						case 33: // MP7
						{
							switch (Menu.Skinchanger.Mp9Skin)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 481;
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 536;
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 500;
								break;
							default:
								break;
							}
						}
						break;
						case 34: // MP9
						{
							switch (Menu.Skinchanger.Mp9Skin)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 262;
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 482;
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 609;
								break;
							default:
								break;
							}
						}
						break;
						case 35: // Nova
						{
							switch (Menu.Skinchanger.NovaSkin)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 537;
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 356;
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 286;
								break;
							default:
								break;
							}
						}
						break;
						case 36: // P250
						{
							switch (Menu.Skinchanger.P250Skin)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;//none
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 102;
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 466;
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 467;
								break;
							case 4:
								*pWeapon->FallbackPaintKit() = 501;
								break;
							case 5:
								*pWeapon->FallbackPaintKit() = 551;
								break;
							case 6:
								*pWeapon->FallbackPaintKit() = 678;
								break;
							default:
								break;
							}
						}
						break;
						case 38: // Scar 20
						{
							switch (Menu.Skinchanger.SCAR20Skin)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;//none
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 642;
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 298;
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 406;
								break;
							case 4:
								*pWeapon->FallbackPaintKit() = 453;
								break;
							case 5:
								*pWeapon->FallbackPaintKit() = 502;
								break;
							case 6:
								*pWeapon->FallbackPaintKit() = 518;
								break;
							case 7:
								*pWeapon->FallbackPaintKit() = 597;
								break;
							case 8:
								*pWeapon->FallbackPaintKit() = 685;
								break;
							default:
								break;
							}
						}
						break;
						case 39: // SG553
						{
							switch (Menu.Skinchanger.Sg553Skin)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;//none
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 519;
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 487;
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 287;
								break;
							case 4:
								*pWeapon->FallbackPaintKit() = 586;
								break;
							default:
								break;
							}
						}
						break;
						case 40: // SSG08
						{
							switch (Menu.Skinchanger.SSG08Skin)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;//none
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 26;
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 60;
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 96;
								break;
							case 4:
								*pWeapon->FallbackPaintKit() = 99;
								break;
							case 5:
								*pWeapon->FallbackPaintKit() = 157;
								break;
							case 6:
								*pWeapon->FallbackPaintKit() = 200;
								break;
							case 7:
								*pWeapon->FallbackPaintKit() = 222;
								break;
							case 8:
								*pWeapon->FallbackPaintKit() = 233;
								break;
							case 9:
								*pWeapon->FallbackPaintKit() = 253;
								break;
							case 10:
								*pWeapon->FallbackPaintKit() = 304;
								break;
							case 11:
								*pWeapon->FallbackPaintKit() = 319;
								break;
							case 12:
								*pWeapon->FallbackPaintKit() = 361;
								break;
							case 13:
								*pWeapon->FallbackPaintKit() = 503;
								break;
							case 14:
								*pWeapon->FallbackPaintKit() = 538;
								break;
							case 15:
								*pWeapon->FallbackPaintKit() = 554;
								break;
							case 16:
								*pWeapon->FallbackPaintKit() = 624;
								break;
							default:
								break;
							}
						}
						break;
						case 64: // Revolver
						{
							switch (Menu.Skinchanger.RevolverSkin)
							{
							case 0:
								*pWeapon->FallbackPaintKit() = 0;//none
								break;
							case 1:
								*pWeapon->FallbackPaintKit() = 683;
								break;
							case 2:
								*pWeapon->FallbackPaintKit() = 522;
								break;
							case 3:
								*pWeapon->FallbackPaintKit() = 12;
								break;
							default:
								break;
							}
						}
						break;
						default:
							break;
						}

						*pWeapon->OwnerXuidLow() = 0;//crash?
						*pWeapon->OwnerXuidHigh() = 0;
						*pWeapon->FallbackWear() = 0.0000001f;
						*pWeapon->ItemIDHigh() = 1;
					}
				}
			}
		}

	}
}

void InventarNewSkinChanger()
{
	CBaseEntity *pLocal = g_pEntitylist->GetClientEntity(g_pEngine->GetLocalPlayer());
	auto weapons = pLocal->m_hMyWeapons();
	for (size_t i = 0; weapons[i]; i++)
	{
		CBaseEntity *pEntity = g_pEntitylist->GetClientEntityFromHandle(weapons[i]);
		if (pEntity)
		{
			CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)pEntity;
			{
				int Model = Menu.Skinchanger.Knife;
				int weapon = *pWeapon->fixskins();
				switch (weapon)
				{
				case 7:
				{
					*pWeapon->FallbackPaintKit() = 656;
					break;
				}
				break;
				case 16: // M4A4
				{
					*pWeapon->FallbackPaintKit() = 632;
					break;
				}
				break;
				case 2: // dual
				{
					*pWeapon->FallbackPaintKit() = 544;
					break;
				}
				break;
				case 60: // M4A1
				{
					*pWeapon->FallbackPaintKit() = 383;
					break;
				}
				break;
				case 9: // AWP
				{
					*pWeapon->FallbackPaintKit() = 344;
					break;
				}
				break;
				case 61: // USP
				{
					*pWeapon->FallbackPaintKit() = 653;
					break;
				}
				break;
				case 4: // Glock
				{
					*pWeapon->FallbackPaintKit() = 586;
					break;
				}
				break;
				case 1: // Deagle
				{
					*pWeapon->FallbackPaintKit() = 527;
					break;
				}
				break;
				case 3: // Five Seven
				{
					*pWeapon->FallbackPaintKit() = 387;
					break;
				}
				break;
				case 8: // AUG
				{
					*pWeapon->FallbackPaintKit() = 305;
					break;
				}
				break;
				case 10: // Famas
				{
					*pWeapon->FallbackPaintKit() = 260;
					break;
				}
				break;
				case 11: // G3SG1
				{
					*pWeapon->FallbackPaintKit() = 628;
					break;
				}
				break;
				case 13: // Galil
				{
					*pWeapon->FallbackPaintKit() = 629;
					break;
				}
				break;
				case 14: // M249
				{
					*pWeapon->FallbackPaintKit() = 401;
					break;
				}
				break;
				case 17: // Mac 10
				{
					*pWeapon->FallbackPaintKit() = 433;
					break;
				}
				break;
				case 19: // P90
				{
					*pWeapon->FallbackPaintKit() = 283;
					break;
				}
				break;
				case 24: // UMP-45
				{
					*pWeapon->FallbackPaintKit() = 688;
					break;
				}
				break;
				case 25: // XM1014
				{
					*pWeapon->FallbackPaintKit() = 616;
					break;
				}
				break;
				case 63: // CZ75-Auto
				{
					*pWeapon->FallbackPaintKit() = 270;
					break;
				}
				break;
				case 26: // Bizon
				{
					*pWeapon->FallbackPaintKit() = 306;
					break;
				}
				break;
				case 27: // Mag 7
				{
					*pWeapon->FallbackPaintKit() = 198;
					break;
				}
				break;
				case 28: // Negev
				{
					*pWeapon->FallbackPaintKit() = 483;
					break;
				}
				break;
				case 29: // Sawed Off
				{
					*pWeapon->FallbackPaintKit() = 434;
					break;
				}
				break;
				case 30: // Tec 9
				{
					*pWeapon->FallbackPaintKit() = 652;
					break;
				}
				break;
				case 32: // P2000
				{
					*pWeapon->FallbackPaintKit() = 338;
					break;
				}
				break;
				case 33: // MP7
				{
					*pWeapon->FallbackPaintKit() = 481;
					break;
				}
				break;
				case 34: // MP9
				{
					*pWeapon->FallbackPaintKit() = 262;
					break;
				}
				break;
				case 35: // Nova
				{
					*pWeapon->FallbackPaintKit() = 450;
					break;
				}
				break;
				case 36: // P250
				{
					*pWeapon->FallbackPaintKit() = 404;
					break;
				}
				break;
				case 38: // Scar 20
				{
					*pWeapon->FallbackPaintKit() = 232;
					break;
				}
				break;
				case 39: // SG553
				{
					*pWeapon->FallbackPaintKit() = 311;
					break;
				}
				break;
				case 40: // SSG08
				{
					*pWeapon->FallbackPaintKit() = 538;
					break;
				}
				break;
				case 64: // Revolver
				{
					*pWeapon->FallbackPaintKit() = 538;
					break;
				}
				break;
				default:
					break;
				}
				if (csgo::MainWeapon->IsKnife())
				{
					auto pCustomName1 = MakePtr(char*, pWeapon, 0x301C);
					worldmodel_handle = pWeapon->m_hWeaponWorldModel();
					if (worldmodel_handle) worldmodel = (CBaseCombatWeapon*)g_pEntitylist->GetClientEntityFromHandleknife(worldmodel_handle);
					Model == 8;
					if (Model == 8) // M9 Bayonet
					{
						int iM9Bayonet = g_pModelInfo->GetModelIndex("models/weapons/v_knife_m9_bay.mdl");
						*pWeapon->ModelIndex() = iM9Bayonet; // m_nModelIndex
						*pWeapon->ViewModelIndex() = iM9Bayonet;
						if (worldmodel) *worldmodel->ModelIndex() = iM9Bayonet + 1;
						*pWeapon->fixskins() = 508;
						*pWeapon->GetEntityQuality() = 3;
						killIcons.clear();
						killIcons["knife_default_ct"] = "knife_m9_bayonet";
						killIcons["knife_t"] = "knife_m9_bayonet";
						int Skin = Menu.Skinchanger.KnifeSkin;
						*pWeapon->FallbackPaintKit() = 622;

					}
					else if (Model == 9) // Shadow Daggers
					{
						int iDagger = g_pModelInfo->GetModelIndex("models/weapons/v_knife_push.mdl");
						*pWeapon->ModelIndex() = iDagger; // m_nModelIndex
						*pWeapon->ViewModelIndex() = iDagger;
						if (worldmodel) *worldmodel->ModelIndex() = iDagger + 1;
						*pWeapon->fixskins() = 516;
						*pWeapon->GetEntityQuality() = 3;
						killIcons.clear();
						killIcons["knife_default_ct"] = "knife_push";
						killIcons["knife_t"] = "knife_push";
						int Skin = Menu.Skinchanger.KnifeSkin;
						if (Skin == 0)
						{
							*pWeapon->FallbackPaintKit() = 5; // Forest DDPAT
						}
						else if (Skin == 1)
						{
							*pWeapon->FallbackPaintKit() = 12; // Crimson Web
						}
						else if (Skin == 2)
						{
							*pWeapon->FallbackPaintKit() = 27; // Bone Mask
						}
						else if (Skin == 3)
						{
							*pWeapon->FallbackPaintKit() = 38; // Fade
						}
						else if (Skin == 4)
						{
							*pWeapon->FallbackPaintKit() = 40; // Night
						}
						else if (Skin == 5)
						{
							*pWeapon->FallbackPaintKit() = 42; // Blue Steel
						}
						else if (Skin == 6)
						{
							*pWeapon->FallbackPaintKit() = 43; // Stained
						}
						else if (Skin == 7)
						{
							*pWeapon->FallbackPaintKit() = 44; // Case Hardened
						}
						else if (Skin == 8)
						{
							*pWeapon->FallbackPaintKit() = 59; // Slaughter
						}
						else if (Skin == 9)
						{
							*pWeapon->FallbackPaintKit() = 72; // Safari Mesh
						}
						else if (Skin == 10)
						{
							*pWeapon->FallbackPaintKit() = 77; // Boreal Forest
						}
						else if (Skin == 11)
						{
							*pWeapon->FallbackPaintKit() = 98; // Ultraviolet
						}
						else if (Skin == 12)
						{
							*pWeapon->FallbackPaintKit() = 143; // Urban Masked
						}
						else if (Skin == 13)
						{
							*pWeapon->FallbackPaintKit() = 175; // Scorched
						}
						else if (Skin == 14)
						{
							*pWeapon->FallbackPaintKit() = 323; // Rust Coat
						}
						else if (Skin == 15)
						{
							*pWeapon->FallbackPaintKit() = 409; // Tiger Tooth
						}
						else if (Skin == 16)
						{
							*pWeapon->FallbackPaintKit() = 410; // Damascus Steel
						}
						else if (Skin == 17)
						{
							*pWeapon->FallbackPaintKit() = 411; // Damascus Steel
						}
						else if (Skin == 18)
						{
							*pWeapon->FallbackPaintKit() = 413; // Marble Fade
						}
						else if (Skin == 19)
						{
							*pWeapon->FallbackPaintKit() = 414; // Rust Coat
						}
						else if (Skin == 20)
						{
							*pWeapon->FallbackPaintKit() = 415; // Doppler Ruby
						}
						else if (Skin == 21)
						{
							*pWeapon->FallbackPaintKit() = 416; // Doppler Sapphire
						}
						else if (Skin == 22)
						{
							*pWeapon->FallbackPaintKit() = 417; // Doppler Blackpearl
						}
						else if (Skin == 23)
						{
							*pWeapon->FallbackPaintKit() = 418; // Doppler Phase 1
						}
						else if (Skin == 24)
						{
							*pWeapon->FallbackPaintKit() = 419; // Doppler Phase 2
						}
						else if (Skin == 25)
						{
							*pWeapon->FallbackPaintKit() = 420; // Doppler Phase 3
						}
						else if (Skin == 26)
						{
							*pWeapon->FallbackPaintKit() = 421; // Doppler Phase 4
						}
						else if (Skin == 27)
						{
							*pWeapon->FallbackPaintKit() = 569; // Gamma Doppler Phase1
						}
						else if (Skin == 28)
						{
							*pWeapon->FallbackPaintKit() = 570; // Gamma Doppler Phase2
						}
						else if (Skin == 29)
						{
							*pWeapon->FallbackPaintKit() = 571; // Gamma Doppler Phase3
						}
						else if (Skin == 30)
						{
							*pWeapon->FallbackPaintKit() = 568; // Gamma Doppler Phase4
						}
						else if (Skin == 31)
						{
							*pWeapon->FallbackPaintKit() = 568; // Gamma Doppler Emerald
						}
						else if (Skin == 32)
						{
							*pWeapon->FallbackPaintKit() = 561; // Lore
						}
						else if (Skin == 33)
						{
							*pWeapon->FallbackPaintKit() = 567; // Black Laminate
						}
						else if (Skin == 34)
						{
							*pWeapon->FallbackPaintKit() = 574; // Autotronic
						}
						else if (Skin == 35)
						{
							*pWeapon->FallbackPaintKit() = 580; // Freehand
						}
					}
				}
				*pWeapon->OwnerXuidLow() = 0;
				*pWeapon->OwnerXuidHigh() = 0;
				*pWeapon->FallbackWear() = 0.001f;
				*pWeapon->fixItemIDHigh() = 1;
			}
		}
	}

}

