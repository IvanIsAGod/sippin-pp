#include "sdk.h"
#include <Psapi.h>
#include "xor.h"
#include "global.h"

offsets_t offys;
inline DWORD  Utilities::Memory::FindPatternAimbot(char *pattern, char *mask, DWORD offset, char *moduleName)
{
	HMODULE hModule;
	MODULEINFO moduleInfo;
	DWORD dModule, dModuleSize;

	hModule = GetModuleHandle(moduleName);
	GetModuleInformation(GetCurrentProcess(), hModule, &moduleInfo, sizeof(MODULEINFO));

	dModule = (DWORD)moduleInfo.lpBaseOfDll;
	dModuleSize = moduleInfo.SizeOfImage;

	DWORD start = dModule;
	DWORD end = dModule + dModuleSize;

	int patternLength = strlen(mask);
	bool found = false;

	for (DWORD i = start; i < end - patternLength; i++)
	{
		found = true;

		for (int idx = 0; idx < patternLength; idx++)
		{
			if (*(mask + idx) == 'x' && *(pattern + idx) != *(char*)(i + idx))
			{
				found = false;
				break;
			}
		}

		if (found)
			return i + offset;
	}
	return NULL;
}

uint64_t FindPatternIDA(const char* szModule, const char* szSignature)
{
	//CREDITS: learn_more
#define INRANGE(x,a,b)  (x >= a && x <= b) 
#define getBits( x )    (INRANGE((x&(~0x20)),XorStr('A'),XorStr('F')) ? ((x&(~0x20)) - XorStr('A') + 0xa) : (INRANGE(x,XorStr('0'),XorStr('9')) ? x - XorStr('0') : 0))
#define getByte( x )    (getBits(x[0]) << 4 | getBits(x[1]))

	MODULEINFO modInfo;
	GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(szModule), &modInfo, sizeof(MODULEINFO));
	DWORD startAddress = (DWORD)modInfo.lpBaseOfDll;
	DWORD endAddress = startAddress + modInfo.SizeOfImage;
	const char* pat = szSignature;
	DWORD firstMatch = 0;
	for (DWORD pCur = startAddress; pCur < endAddress; pCur++) {
		if (!*pat) return firstMatch;
		if (*(PBYTE)pat == XorStr('\?') || *(BYTE*)pCur == getByte(pat)) {
			if (!firstMatch) firstMatch = pCur;
			if (!pat[2]) return firstMatch;
			if (*(PWORD)pat == XorStr('\?\?') || *(PBYTE)pat != XorStr('\?')) pat += 3;
			else pat += 2;    //one ?
		}
		else {
			pat = szSignature;
			firstMatch = 0;
		}
	}
	return NULL;
}

static bool bCompare(const BYTE* Data, const BYTE* Mask, const char* szMask)
{
	for (; *szMask; ++szMask, ++Mask, ++Data)
	{
		if (*szMask == 'x' && *Mask != *Data)
		{
			return false;
		}
	}
	return (*szMask) == 0;
}

DWORD WaitOnModuleHandle(std::string moduleName)
{
	DWORD ModuleHandle = NULL;
	while (!ModuleHandle)
	{
		ModuleHandle = (DWORD)GetModuleHandle(moduleName.c_str());
		if (!ModuleHandle)
			Sleep(50);
	}
	return ModuleHandle;
}

DWORD FindPattern(std::string moduleName, BYTE* Mask, char* szMask)
{
	DWORD Address = WaitOnModuleHandle(moduleName.c_str());
	MODULEINFO ModInfo; GetModuleInformation(GetCurrentProcess(), (HMODULE)Address, &ModInfo, sizeof(MODULEINFO));
	DWORD Length = ModInfo.SizeOfImage;
	for (DWORD c = 0; c < Length; c += 1)
	{
		if (bCompare((BYTE*)(Address + c), Mask, szMask))
		{
			return DWORD(Address + c);
		}
	}
	return 0;
}
#define IsInRange(x, a, b) (x >= a && x <= b)
#define GetBits(x) (IsInRange(x, '0', '9') ? (x - '0') : ((x&(~0x20)) - 'A' + 0xA))
#define GetByte(x) (GetBits(x[0]) << 4 | GetBits(x[1]))
DWORD FindSig2(DWORD dwAddress, DWORD dwLength, const char* szPattern)
{
	if (!dwAddress || !dwLength || !szPattern)
		return 0;

	const char* pat = szPattern;
	DWORD firstMatch = NULL;

	for (DWORD pCur = dwAddress; pCur < dwLength; pCur++)
	{
		if (!*pat)
			return firstMatch;

		if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == GetByte(pat))
		{
			if (!firstMatch)
				firstMatch = pCur;

			if (!pat[2])
				return firstMatch;

			if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?')
				pat += 3;

			else pat += 2;
		}
		else
		{
			pat = szPattern;
			firstMatch = 0;
		}
	}

	return 0;
}

DWORD FindSignature(const char* szModuleName, const char* PatternName, char* szPattern)
{
	HMODULE hModule = GetModuleHandleA(szModuleName);
	PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)hModule;
	PIMAGE_NT_HEADERS pNTHeaders = (PIMAGE_NT_HEADERS)(((DWORD)hModule) + pDOSHeader->e_lfanew);

	DWORD ret = FindSig2(((DWORD)hModule) + pNTHeaders->OptionalHeader.BaseOfCode, ((DWORD)hModule) + pNTHeaders->OptionalHeader.SizeOfCode, szPattern);;

	return ret;
}

void SetupOffsets()
{
	//printf(XorStr(":::::Adresses & Offsets:::::\n"));
	g_pNetVars = new CNetVars();


	offys.nWriteableBones = g_pNetVars->GetOffset(XorStr("DT_CSPlayer"), XorStr("m_nForceBone")) + 0x20;
	offys.dwOcclusionArray = *(uintptr_t*)(FindPatternIDA(XorStr(client_dll), XorStr("A1 ? ? ? ? 8B B7 ? ? ? ? 89 75 F8")) + 0x1);
	offys.bDidCheckForOcclusion = *(uintptr_t*)(FindPatternIDA(XorStr(client_dll), XorStr("A1 ? ? ? ? 8B B7 ? ? ? ? 89 75 F8")) + 0x7);

	offys.InvalidateBoneCache = FindPatternIDA(client_dll, "80 3D ? ? ? ? 00 74 16 A1 ? ? ? ? 48 C7 81");

	//printf(XorStr("Array: 0x%X\n"), (DWORD)offys.dwOcclusionArray);
	//printf(XorStr("Bool: 0x%X\n"), (DWORD)offys.bDidCheckForOcclusion);

	offys.dwInitKeyValues = FindPatternIDA(XorStr(client_dll), XorStr("8B 0E 33 4D FC 81 E1 ? ? ? ? 31 0E 88 46 03 C1 F8 08 66 89 46 12 8B C6")) - 0x45;
	//printf(XorStr("InitKeyValues: 0x%X\n"), (DWORD)offys.dwInitKeyValues);

	offys.dwLoadFromBuffer = FindPatternIDA(XorStr(client_dll), XorStr("55 8B EC 83 EC 44 53 56 57 8B F9 89 7D F8 FF 15 ? ? ? ? 8B F0 B9"));
	//printf(XorStr("LoadFromBuffer: 0x%X\n"), (DWORD)offys.dwLoadFromBuffer);

	offys.m_ArmorValue = g_pNetVars->GetOffset("DT_CSPlayer", "m_ArmorValue");
	offys.m_bHasHelmet = g_pNetVars->GetOffset("DT_CSPlayer", "m_bHasHelmet");

	offys.m_angRotation = g_pNetVars->GetOffset("DT_CSPlayer", "m_angRotation");


	offys.m_flPoseParameter = g_pNetVars->GetOffset(XorStr("DT_CSPlayer"), XorStr("m_flPoseParameter"));
	offys.m_flCycle = g_pNetVars->GetOffset(XorStr("DT_CSPlayer"), XorStr("m_flCycle"));
	offys.m_flSimulationTime = g_pNetVars->GetOffset(XorStr("DT_CSPlayer"), XorStr("m_flSimulationTime"));
	offys.m_flAnimTime = g_pNetVars->GetOffset(XorStr("DT_CSPlayer"), XorStr("m_flAnimTime"));
	offys.m_nSequence = g_pNetVars->GetOffset(XorStr("DT_CSPlayer"), XorStr("m_nSequence"));
	offys.m_flLowerBodyYawTarget = g_pNetVars->GetOffset(XorStr("DT_CSPlayer"), XorStr("m_flLowerBodyYawTarget"));
	offys.m_flFlashMaxAlpha = g_pNetVars->GetOffset(XorStr("DT_CSPlayer"), XorStr("m_flFlashMaxAlpha"));
	offys.m_angEyeAngles = g_pNetVars->GetOffset(XorStr("DT_CSPlayer"), XorStr("m_angEyeAngles[0]"));
	offys.m_nTickBase = g_pNetVars->GetOffset(XorStr("DT_BasePlayer"), XorStr("m_nTickBase"));
	offys.m_fFlags = g_pNetVars->GetOffset(XorStr("DT_CSPlayer"), XorStr("m_fFlags"));
	offys.m_flNextPrimaryAttack = g_pNetVars->GetOffset(XorStr("DT_BaseCombatWeapon"), XorStr("m_flNextPrimaryAttack"));
	offys.m_vecOrigin = g_pNetVars->GetOffset(XorStr("DT_BaseEntity"), XorStr("m_vecOrigin"));
	offys.m_vecViewOffset = g_pNetVars->GetOffset(XorStr("DT_BasePlayer"), XorStr("m_vecViewOffset[0]"));
	offys.m_flDuckAmount = g_pNetVars->GetOffset(XorStr("DT_BasePlayer"), XorStr("m_flDuckAmount"));
	offys.m_flDuckSpeed = g_pNetVars->GetOffset(XorStr("DT_BasePlayer"), XorStr("m_flDuckSpeed"));
	offys.m_vecVelocity = g_pNetVars->GetOffset("DT_CSPlayer", "m_vecVelocity[0]");
	offys.m_vecBaseVelocity = g_pNetVars->GetOffset("DT_CSPlayer", "m_vecBaseVelocity");
	offys.m_flFallVelocity = g_pNetVars->GetOffset("DT_CSPlayer", "m_flFallVelocity");
	offys.m_bPinPulled = g_pNetVars->GetOffset(XorStr("DT_BaseCSGrenade"), XorStr("m_bPinPulled"));
//	offys.m_hObserverTarget = g_pNetVars->GetOffset(XorStr("DT_BasePlayer"), XorStr("m_hObserverTarget"));
	offys.m_fThrowTime = g_pNetVars->GetOffset(XorStr("DT_BaseCSGrenade"), XorStr("m_fThrowTime"));
	offys.m_fAccuracyPenalty = g_pNetVars->GetOffset("DT_WeaponCSBase", "m_fAccuracyPenalty");
	offys.m_fLastShotTime = g_pNetVars->GetOffset("DT_WeaponCSBase", "m_fLastShotTime");
	offys.m_iClip1 = g_pNetVars->GetOffset("DT_WeaponCSBase", "m_iClip1");
	offys.m_bReloadVisuallyComplete = g_pNetVars->GetOffset("DT_WeaponFiveSeven", "m_bReloadVisuallyComplete");
	offys.m_iPrimaryReserveAmmoCount = g_pNetVars->GetOffset("DT_BaseCombatWeapon", "m_iPrimaryReserveAmmoCount");
	offys.getSequenceActivity = (DWORD)FindPatternIDA(client_dll, "55 8B EC 83 7D 08 FF 56 8B F1 74 3D");

	offys.m_lifeState = g_pNetVars->GetOffset(XorStr("DT_CSPlayer"), XorStr("m_lifeState"));
	offys.m_flC4Blow = g_pNetVars->GetOffset(XorStr("DT_PlantedC4"), XorStr("m_flC4Blow"));
	offys.m_bBombDefused = g_pNetVars->GetOffset(XorStr("DT_PlantedC4"), XorStr("m_bBombDefused"));
	offys.m_hOwnerEntity = g_pNetVars->GetOffset(XorStr("DT_PlantedC4"), XorStr("m_hOwnerEntity"));

	offys.m_flFriction = g_pNetVars->GetOffset("DT_CSPlayer", "m_flFriction");
	offys.m_CollisionGroup = g_pNetVars->GetOffset("DT_BaseEntity", "m_CollisionGroup");
	offys.m_bIsScoped = g_pNetVars->GetOffset("DT_CSPlayer", "m_bIsScoped");
	offys.m_bClientSideAnimation = g_pNetVars->GetOffset("DT_BaseAnimating", "m_bClientSideAnimation");
	offys.dwComputeHitboxSurroundingBox = FindPatternIDA(client_dll, "E9 ? ? ? ? 32 C0 5D");

	uintptr_t player_resource_pointer = uintptr_t((uintptr_t)GetModuleHandle(client_dll) + 0x2EC5ADC);
	offys.dw_CSPlayerResource = (DWORD)player_resource_pointer;
	offys.m_iCompetetiveRanking = g_pNetVars->GetOffset(XorStr("DT_CSPlayerResource"), XorStr("m_iCompetitiveRanking"));
	offys.m_iCompetetiveWins = g_pNetVars->GetOffset(XorStr("DT_CSPlayerResource"), XorStr("m_iCompetitiveWins"));
	offys.m_iPing = g_pNetVars->GetOffset(XorStr("DT_CSPlayerResource"), XorStr("m_iPing"));

	offys.animstate = 0x39008E;

	offys.m_bIsValveDS = g_pNetVars->GetOffset(XorStr("DT_CSGameRulesProxy"), XorStr("m_bIsValveDS"));

	offys.m_hMyWearables = g_pNetVars->GetOffset("DT_CSPlayer", "m_hMyWearables");

	offys.m_hMyWeapons = g_pNetVars->GetOffset("DT_BasePlayer", "m_hMyWeapons") / 2;
	offys.m_hActiveWeapon = g_pNetVars->GetOffset("DT_BasePlayer","m_hActiveWeapon");

	offys.m_flFriction = g_pNetVars->GetOffset("DT_CSPlayer", "m_flFriction");
	offys.m_flMaxspeed = g_pNetVars->GetOffset("DT_BasePlayer", "m_flMaxspeed");
	offys.m_flStepSize = g_pNetVars->GetOffset("DT_CSPlayer", "m_flStepSize");
	offys.m_bGunGameImmunity = g_pNetVars->GetOffset("DT_CSPlayer", "m_bGunGameImmunity");

	offys.m_flPostponeFireReadyTime = g_pNetVars->GetOffset("DT_WeaponCSBaseGun", "m_flPostponeFireReadyTime");
	offys.m_iShotsFired = g_pNetVars->GetOffset("DT_CSPlayer", "m_iShotsFired");

	/*DT_BaseAttributableItem*/
	offys.m_iItemDefinitionIndex = g_pNetVars->GetOffset("DT_BaseAttributableItem", "m_iItemDefinitionIndex");
	offys.m_iItemIDHigh = g_pNetVars->GetOffset("DT_BaseAttributableItem", "m_iItemIDHigh");
	offys.m_iItemIDLow = g_pNetVars->GetOffset("DT_BaseAttributableItem", "m_iItemIDLow");
	offys.m_iEntityQuality = g_pNetVars->GetOffset("DT_BaseAttributableItem", "m_iEntityQuality");
	offys.m_szCustomName = g_pNetVars->GetOffset("DT_BaseAttributableItem", "m_szCustomName");
	offys.m_OriginalOwnerXuidLow = g_pNetVars->GetOffset("DT_BaseAttributableItem", "m_OriginalOwnerXuidLow");
	offys.m_OriginalOwnerXuidHigh = g_pNetVars->GetOffset("DT_BaseAttributableItem", "m_OriginalOwnerXuidHigh");
	offys.m_nFallbackPaintKit = g_pNetVars->GetOffset("DT_BaseAttributableItem", "m_nFallbackPaintKit");
	offys.m_nFallbackSeed = g_pNetVars->GetOffset("DT_BaseAttributableItem", "m_nFallbackSeed");
	offys.m_flFallbackWear = g_pNetVars->GetOffset("DT_BaseAttributableItem", "m_flFallbackWear");
	offys.m_nFallbackStatTrak = g_pNetVars->GetOffset("DT_BaseAttributableItem", "m_nFallbackStatTrak");
	offys.m_iAccountID = g_pNetVars->GetOffset("DT_BaseAttributableItem", "m_iAccountID");

	offys.m_iViewModelIndex = g_pNetVars->GetOffset("DT_BaseViewModel", "m_nViewModelIndex");
	offys.m_iWorldModelIndex = g_pNetVars->GetOffset("DT_BaseCombatWeapon", "m_iWorldModelIndex");
	offys.m_hWeaponWorldModel = g_pNetVars->GetOffset("DT_BaseCombatWeapon", "m_hWeaponWorldModel");

	offys.m_nModelIndex = g_pNetVars->GetOffset("DT_BaseViewModel", "m_nModelIndex");
	offys.m_hViewModel = g_pNetVars->GetOffset("DT_CSPlayer", "m_hViewModel[0]");
	offys.m_hWeapon = g_pNetVars->GetOffset("DT_BaseViewModel", "m_hWeapon");
}