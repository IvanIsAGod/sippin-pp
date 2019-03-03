#pragma once
#include "sdk.h"

namespace helping
{
	extern Vector get_hitbox_location(CBaseEntity* pEntity, int Hitbox);
	extern Vector CalcAngle(Vector src, Vector dst);



	extern bool EntityIsValid(int i);
	extern bool IsVisible_enemy(CBaseEntity* pEnemy);



	extern int ClosestBone(CBaseEntity* Entity, CUserCmd* cmd);


	extern float Distance(Vector VecA, Vector VecB);
	extern inline float FASTSQRT(float x);
	extern float getHypotenuseFromCenter(Vector Point);
	extern auto GetFovs(Vector& viewAngle, Vector& aimAngle) -> float;

	extern void VectorAngles(const Vector& forward, Vector &angles);
	extern void CA(Vector src, Vector dst, Vector &angles);
}
