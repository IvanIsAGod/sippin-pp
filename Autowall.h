#pragma once
struct FireBulletData
{
	FireBulletData(const Vector &eye_pos) : src(eye_pos)
	{
	}

	Vector						src;
	trace_t       enter_trace;
	Vector						direction;
	CTraceFilter  filter;
	float						trace_length;
	float						trace_length_remaining;
	float						current_damage;
	int							penetrate_count;
};
class CAutowall
{
public:
	//Old Awall


	bool CanWallBang(float & dmg);






	void TraceLine(Vector & absStart, Vector & absEnd, unsigned int mask, CBaseEntity * ignore, trace_t * ptr);

	void ClipTraceToPlayers(Vector & absStart, Vector absEnd, unsigned int mask, ITraceFilter * filter, trace_t * tr);

	void GetBulletTypeParameters(float & maxRange, float & maxDistance, char * bulletType, bool sv_penetration_type);

	bool IsBreakableEntity(CBaseEntity * entity);

	void ScaleDamage(trace_t & enterTrace, CSWeaponInfo * weaponData, float & currentDamage);

	bool TraceToExit(trace_t & enterTrace, trace_t & exitTrace, Vector startPosition, Vector direction);

	bool HandleBulletPenetration(CSWeaponInfo * weaponData, trace_t & enterTrace, Vector & eyePosition, Vector direction, int & possibleHitsRemaining, float & currentDamage, float penetrationPower, bool sv_penetration_type, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration);

	bool FireBullet(CBaseCombatWeapon * pWeapon, Vector & direction, float & currentDamage);

	float CanHit(Vector & vecEyePos, Vector & point);

	float CanHit(Vector & point);

	float CanHit1(Vector & point, CBaseEntity * pBaseEntity);

	float CanHit1(Vector & point);

	bool PenetrateWall(CBaseEntity * pBaseEntity, Vector & vecPoint);

	bool CanWallbang(float & dmg);

	bool CanHitFloatingPoint(const Vector & point, const Vector & source);

}; extern CAutowall* g_Autowall;
extern void ScaleDamage(int hitgroup, CBaseEntity *enemy, float weapon_armor_ratio, float &current_damage);
//extern void UTIL_ClipTraceToPlayers( const Vector& vecAbsStart, const Vector& vecAbsEnd, unsigned int mask, ITraceFilter* filter, trace_t* tr );

