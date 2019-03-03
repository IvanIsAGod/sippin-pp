#include "hooks.h"

void __fastcall Hooks::TraceRay(void *thisptr, void*, const Ray_t &ray, unsigned int fMask, ITraceFilter *pTraceFilter, trace_t *pTrace)
{

	enginetraceVMT->GetOriginalMethod<TraceRayFn>(5)(thisptr, ray, fMask, pTraceFilter, pTrace);


	pTrace->surface.flags |= SURF_SKY;
}
