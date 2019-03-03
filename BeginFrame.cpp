#include "hooks.h"
#include "sdk.h"
#include "bullet_log.h"
#include "global.h"


void __fastcall Hooks::g_hkBeginFrame(void* thisptr)
{
	beginFrame->GetOriginalMethod<BeginFrameFn>(9)(thisptr);
}