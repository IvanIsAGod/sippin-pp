#include "GloveChanger.h"

typedef unsigned long CBaseHandle;
#define RandomInt(nMin, nMax) (rand() % (nMax - nMin + 1) + nMin);

void GloveChanger()
{
	if (Menu.Skinchanger.glovesenabled && Menu.Skinchanger.gloves > 0)
	{
		player_info_t localPlayerInfo;

		CBaseEntity *pLocal = g_pEntitylist->GetClientEntity(g_pEngine->GetLocalPlayer());
		if (pLocal)
		{
			if (g_pEngine->GetPlayerInfo(g_pEngine->GetLocalPlayer(), &localPlayerInfo))
			{
				DWORD* hMyWearables = (DWORD*)((size_t)pLocal + offys.m_hMyWearables);

				if (hMyWearables)
				{
					if (!g_pEntitylist->GetClientEntityFromHandle((DWORD)hMyWearables[0]))
					{
						static ClientClass* pClass;

						if (!pClass)
							pClass = g_pClient->GetAllClasses();
						while (pClass)
						{
							if (pClass->m_ClassID == 45)
								break;
							pClass = pClass->m_pNext;
						}

						int iEntry = g_pEntitylist->GetHighestEntityIndex() + 1;
						int iSerial = RandomInt(0x0, 0xFFF);

						pClass->m_pCreateFn(iEntry, iSerial);
						hMyWearables[0] = iEntry | (iSerial << 16);

						CBaseEntity* pEnt = g_pEntitylist->GetClientEntityFromHandle((DWORD)hMyWearables[0]);

						if (pEnt)
						{
							int modelindex = 0;

							if (Menu.Skinchanger.gloves == 1)
								modelindex = g_pModelInfo->GetModelIndex(("models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl"));
							else if (Menu.Skinchanger.gloves == 2)
								modelindex = g_pModelInfo->GetModelIndex(("models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl"));
							else if (Menu.Skinchanger.gloves == 3)
								modelindex = g_pModelInfo->GetModelIndex(("models/weapons/v_models/arms/glove_slick/v_glove_slick.mdl"));
							else if (Menu.Skinchanger.gloves == 4)
								modelindex = g_pModelInfo->GetModelIndex(("models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl"));
							else if (Menu.Skinchanger.gloves == 5)
								modelindex = g_pModelInfo->GetModelIndex(("models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl"));
							else if (Menu.Skinchanger.gloves == 6)
								modelindex = g_pModelInfo->GetModelIndex(("models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl"));

							int ItemDefinitionIndex;
							if (Menu.Skinchanger.gloves == 1)
							{
								ItemDefinitionIndex = 5027;
							}
							else if (Menu.Skinchanger.gloves == 2)
							{
								ItemDefinitionIndex = 5030;
							}
							else if (Menu.Skinchanger.gloves == 3)
							{
								ItemDefinitionIndex = 5031;
							}
							else if (Menu.Skinchanger.gloves == 4)
							{
								ItemDefinitionIndex = 5032;
							}
							else if (Menu.Skinchanger.gloves == 5)
							{
								ItemDefinitionIndex = 5033;
							}
							else if (Menu.Skinchanger.gloves == 6)
							{
								ItemDefinitionIndex = 5034;
							}
							else
								ItemDefinitionIndex = 0;

							int paintkit;
							if (Menu.Skinchanger.gloves == 1)
							{
								switch (Menu.Skinchanger.skingloves)
								{
								case 0:
									paintkit = 10006;
									break;
								case 1:
									paintkit = 10007;
									break;
								case 2:
									paintkit = 10008;
									break;
								case 3:
									paintkit = 10039;
									break;
								}
							}
							else if (Menu.Skinchanger.gloves == 2)
							{
								switch (Menu.Skinchanger.skingloves)
								{
								case 0:
									paintkit = 10018;
									break;
								case 1:
									paintkit = 10037;
									break;
								case 2:
									paintkit = 10038;
									break;
								case 3:
									paintkit = 10019;
									break;
								}
							}
							else if (Menu.Skinchanger.gloves == 3)
							{
								switch (Menu.Skinchanger.skingloves)
								{
								case 0:
									paintkit = 10013;
									break;
								case 1:
									paintkit = 10015;
									break;
								case 2:
									paintkit = 10016;
									break;
								case 3:
									paintkit = 10040;
									break;
								}
							}
							else if (Menu.Skinchanger.gloves == 4)
							{
								switch (Menu.Skinchanger.skingloves)
								{
								case 0:
									paintkit = 10009;
									break;
								case 1:
									paintkit = 10010;
									break;
								case 2:
									paintkit = 10021;
									break;
								case 3:
									paintkit = 10036;
									break;
								}
							}
							else if (Menu.Skinchanger.gloves == 5)
							{
								switch (Menu.Skinchanger.skingloves)
								{
								case 0:
									paintkit = 10024;
									break;
								case 1:
									paintkit = 10026;
									break;
								case 2:
									paintkit = 10027;
									break;
								case 3:
									paintkit = 10028;
									break;
								}
							}
							else if (Menu.Skinchanger.gloves == 6)
							{
								switch (Menu.Skinchanger.skingloves)
								{
								case 0:
									paintkit = 10030;
									break;
								case 1:
									paintkit = 10033;
									break;
								case 2:
									paintkit = 10034;
									break;
								case 3:
									paintkit = 10035;
									break;
								}
							}
							*(int*)((DWORD)pEnt + offys.m_iItemDefinitionIndex) = ItemDefinitionIndex;
							pEnt->SetModelIndex(modelindex);
							*(int*)((DWORD)pEnt + offys.m_iItemIDHigh) = -1;
							*(int*)((DWORD)pEnt + offys.m_iAccountID) = localPlayerInfo.m_nXuidLow;
							*(float*)((DWORD)pEnt + offys.m_flFallbackWear) = Menu.Skinchanger.glovewear;
							*(int*)((DWORD)pEnt + offys.m_nFallbackPaintKit) = paintkit;
							pEnt->PreDataUpdate(DATA_UPDATE_CREATED);
						}
					}
				}

			}
		}
	}
}
