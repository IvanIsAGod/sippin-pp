#include "sdk.h"
#include "hooks.h"
#include "MaterialHelper.h"
#include "BacktrackingHelper.h"
#include "security.h"
#include "Math.h"
#include "global.h"
#include <dbghelp.h>
#include <tchar.h>
#include "ScreenEffects.h"

HINSTANCE hAppInstance;
CClient* g_pClient;
CClientEntityList* g_pEntitylist;
CEngine* g_pEngine;
CInput* g_pInput;
CModelInfo* g_pModelInfo;
CInputSystem* g_pInputSystem;
CPanel* g_pPanel;
IStudioRender* g_pStudioRender;
CSurface* g_pSurface;
CEngineTrace* g_pEngineTrace;
CDebugOverlay* g_pDebugOverlay;
IPhysicsSurfaceProps* g_pPhysics;
CRenderView* g_pRenderView;
CUtlVectorSimple *g_ClientSideAnimationList;
CClientModeShared* g_pClientMode;
CGlobalVarsBase* g_pGlobals;
CModelRender* g_pModelRender;
CGlowObjectManager* g_GlowObjManager;
CMaterialSystem* g_pMaterialSystem;
IMoveHelper* g_pMoveHelper;

CPrediction* g_pPrediction;
CGameMovement* g_pGameMovement;
IGameEventManager* g_pGameEventManager;
IEngineVGui* g_pEngineVGUI;
ICvar* g_pCvar;
CInterfaces Interfaces;
CCRC gCRC;

C_TEFireBullets* g_pFireBullet;
IViewRenderBeams* g_pViewRenderBeams;

VMT* panelVMT;
VMT* beginFrame;
VMT* clientmodeVMT;
VMT* enginevguiVMT;
VMT* modelrenderVMT;
VMT* clientVMT;
VMT* HNetchan;
VMT* firebulletVMT;
VMT* d3d9VMT;
VMT* netChan;
VMT* enginetraceVMT;
VMT* renderviewVMT;
VMT* surfaceVMT;

namespace FakeLatency
{
	extern char *clientdllstr;
	extern char *enginedllstr;
	extern char *cam_tothirdperson_sig;
	extern char *cam_tofirstperson_sig;
	extern char *clientstatestr;
	extern char *updateclientsideanimfnsigstr;
	extern void DecStr(char *adr, const unsigned len);
	extern void EncStr(char *adr, const unsigned len);
	extern void DecStr(char *adr);
	extern void EncStr(char *adr);
	extern HANDLE FindHandle(std::string name);
	extern uintptr_t FindMemoryPattern(HANDLE ModuleHandle, char* strpattern, int length);
	extern HANDLE EngineHandle;
	extern HANDLE ClientHandle;
	extern DWORD ClientState;
}

CVMTHookManager* g_pD3D = nullptr;
CVMTHookManager* g_pSurfaceHook = nullptr;
CVMTHookManager* g_pClientModeHook = nullptr;
CVMTHookManager* tracehk = nullptr;

HMODULE h_ThisModule;

void Init()
{
	SetupOffsets();
	g_MaterialHelper = new CMaterialHelper();
	g_Event.RegisterSelf();
	srand(time(0));

	panelVMT = new VMT(g_pPanel);
	panelVMT->HookVM((void*)Hooks::PaintTraverse, 41);
	panelVMT->ApplyVMT();

	clientmodeVMT = new VMT(g_pClientMode);
	clientmodeVMT->HookVM((void*)Hooks::CreateMove, 24);
	clientmodeVMT->HookVM((void*)Hooks::OverrideView, 18);
	clientmodeVMT->ApplyVMT();

	g_pClientModeHook = new CVMTHookManager((PDWORD*)g_pClientMode);
	g_pClientModeHook->HookMethod((DWORD)hkDoPostScreenSpaceEffects, 44);
	g_pClientModeHook->HookMethod((DWORD)&GGetViewModelFOV, 35);
	g_pClientModeHook->ReHook();

	renderviewVMT = new VMT(g_pRenderView);
	renderviewVMT->HookVM((void*)Hooks::scene_end, 9);
	renderviewVMT->ApplyVMT();

	modelrenderVMT = new VMT(g_pModelRender);
	modelrenderVMT->HookVM((void*)Hooks::DrawModelExecute, 21);
	modelrenderVMT->ApplyVMT();

	clientVMT = new VMT(g_pClient);
	clientVMT->HookVM((void*)Hooks::FrameStageNotify, 37);
	clientVMT->ApplyVMT();

	beginFrame = new VMT(g_pStudioRender);
	beginFrame->HookVM((void*)Hooks::g_hkBeginFrame, 9);
	beginFrame->ApplyVMT();

	auto dwDevice = **(uint32_t**)(FindPatternIDA(XorStr("shaderapidx9.dll"), XorStr("A1 ? ? ? ? 50 8B 08 FF 51 0C")) + 1);

	d3d9VMT = new VMT((void*)dwDevice);
	d3d9VMT->HookVM((void*)Hooks::D3D9_EndScene, 42);
	d3d9VMT->ApplyVMT();

	g_pD3D = new CVMTHookManager(reinterpret_cast<DWORD**>(dwDevice));
	oResetScene = reinterpret_cast<tReset>(g_pD3D->HookMethod(reinterpret_cast<DWORD>(Hooks::hkdReset), 16));

	g_pSurfaceHook = new CVMTHookManager(reinterpret_cast<DWORD**>(g_pSurface));
	oLockCursor = reinterpret_cast<LockCursor_t>(g_pSurfaceHook->HookMethod(reinterpret_cast<DWORD>(Hooks::hk_lockcursor), 67));

	Hooks::g_pOldWindowProc = (WNDPROC)SetWindowLongPtr(csgo::Window, GWLP_WNDPROC, (LONG_PTR)Hooks::WndProc);

	FakeLatency::ClientState = *(DWORD*)(FindPatternIDA("engine.dll", "8B 3D ? ? ? ? 8A F9") + 2);

	/*tracehk = new CVMTHookManager(reinterpret_cast<DWORD**> (g_pEngineTrace));
	oTraceRay = (TraceRayFn)tracehk->HookMethod(reinterpret_cast< uintptr_t >(g_hkTraceRay), 5);*/
}

void StartCheat()
{
	if (Interfaces.GetInterfaces() && g_pPanel && g_pClientMode)
	{
		Sleep(500);
		Init();
	}	
}

BOOL WINAPI DllMain(HINSTANCE Instance, DWORD dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(Instance);
		csgo::Window = FindWindowA(("Valve001"), 0);
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)StartCheat, 0, 0, 0);
		return TRUE;
	}
	return TRUE;
}

MsgFn oMsg;

void __cdecl Msg(char const* msg, ...)
{
	//DOES NOT CRASH
	if (!oMsg)
		oMsg = (MsgFn)GetProcAddress(GetModuleHandle(XorStr("tier0.dll")), XorStr("Msg"));

	char buffer[989];
	va_list list;
	va_start(list, msg);
	vsprintf_s(buffer, msg, list);
	va_end(list);
	oMsg(buffer, list);
}