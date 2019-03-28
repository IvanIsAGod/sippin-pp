#pragma once
#include "includes.h"
#include "Aimbot.h"
#include <deque>

extern int ground_tick;

class Resolver
{
public:
	struct ResolverData
	{
		bool IsDesyncing;
		float old_simtime;
		float stored_simtime;
		float stored_poseparameter;
		float shot_time;

		Vector goodangle;

		float lbydelta;
		float eyeangle;
		float lbyangle;
		float animangle;
		int activity;
		float lookatlocalp;
		float simtime;
		float oldsimtime;
		int flags;
		Vector origin;
		bool fakewalking;
		bool isdormant;
		int movingangle;
		float lastmovinglby;
		float movingspeed;
		float lasttimemoving;
		int movinglby = INT_MAX;
		bool ismoving;
		bool isonground;
		bool ismovingonground;
		bool isinair;
		bool movinglbyvalid = false;
		bool islastmovinglbyvalid;
		bool islastmovinglbydeltavalid;
		bool UseFreestandAngle;
		float FreestandAngle;
		float pitchHit;

		bool shot;
		int desyncrange;
		Vector oldeyeangles;
		float lastlby;
	}pResolverData[64];
	bool fakewalk(CBaseEntity * entity);
	/*Voids*/
	void DefaultResolver(CBaseEntity * entity);
	void ppresolve(CBaseEntity * ent);
	void NewResolver(CBaseEntity * entity);
	void update(CBaseEntity * entity);
	void ExperimentalResolver(CBaseEntity * entity);
	void OnCreateMove();
	void dofsn();
	/*Bools*/
};

extern Resolver* g_Resolver;