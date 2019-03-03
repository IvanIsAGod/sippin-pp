#define SECURITY_WIN32 
#pragma warning(disable : 4800)
#pragma warning(disable : 4244)
#pragma once
#include <stdio.h>
#include <vector>
#include <string>
#include <time.h>
#include <iostream>
#include <sstream>
#include <Windows.h>
#include <stdlib.h>
#include <array>
#include <vector>
#include <windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <map>
#include <sstream>
#include <iostream>
#include <cstdio>
#include "ClientVariables.h"

using namespace std;
#include "definitions.h"
#include "common.h"
#include "vmt.h"


#include "structs\vector.h"
#include "structs\Vector2D.h"
#include "structs\vmatrix.h"
#include "structs\materials.h"
#include "structs\baseentity.h"
#include "structs\clientrecvprops.h"
#include "structs\convar.h"
#include "structs\bspflags.h"
#include "structs\cliententlist.h"
#include "structs\enginevgui.h"
#include "structs\GameEvents.h"
#include "structs\usercmd.h"
#include "structs\physics.h"
#include "structs\basecombatweapon.h"
#include "structs\playerinfo.h"
#include "structs\engine.h"
#include "structs\globals.h"
#include "structs\input.h"
#include "structs\panel.h"
#include "structs\surface.h"
#include "structs\client.h"
#include "structs\crc32.h"
#include "structs\quaternion.h"
#include "structs\modelinfo.h"
#include "structs\dt_common2.h"
#include "structs\clientmodeshared.h"
#include "structs\trace.h"
#include "structs\checksum_md5.h"
#include "structs\msg.h"
#include "structs\debugoverlay.h"
#include "structs\firebullet.h"
#include "structs\CGlow.h"
#include "structs\Prediction.h"
#include "netvars.h"
#include "interfaces.h"
#include <d3d9.h>
#include <d3dx9.h>
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

extern void __cdecl Msg(char const* msg, ...);

enum MoveType_t
{
	MOVETYPE_NONE = 0,	// never moves
	MOVETYPE_ISOMETRIC,			// For players -- in TF2 commander view, etc.
	MOVETYPE_WALK,				// Player only - moving on the ground
	MOVETYPE_STEP,				// gravity, special edge handling -- monsters use this
	MOVETYPE_FLY,				// No gravity, but still collides with stuff
	MOVETYPE_FLYGRAVITY,		// flies through the air + is affected by gravity
	MOVETYPE_VPHYSICS,			// uses VPHYSICS for simulation
	MOVETYPE_PUSH,				// no clip to world, push and crush
	MOVETYPE_NOCLIP,			// No gravity, no collisions, still do velocity/avelocity
	MOVETYPE_LADDER,			// Used by players only when going onto a ladder
	MOVETYPE_OBSERVER,			// Observer movement, depends on player's observer mode
	MOVETYPE_CUSTOM,			// Allows the entity to describe its own physics

	// should always be defined as the last item in the list
	MOVETYPE_LAST = MOVETYPE_CUSTOM,

	MOVETYPE_MAX_BITS = 4
};

#define TICK_INTERVAL			( g_pGlobals->interval_per_tick )
#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )
#define TICKS_TO_TIME( t )		( TICK_INTERVAL *( t ) )