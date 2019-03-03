#include "hooks.h"
#include "GameUtils.h"
#include "global.h"
#include "BacktrackingHelper.h"
#include "Menu.h"
#include "Autowall.h"
#include "Math.h"
bool CGameTrace::DidHitWorld() const
{
    return m_pEnt == g_pEntitylist->GetClientEntity( 0 );
}
bool CGameTrace::DidHit() const
{
    return fraction < 1.0f || allsolid || startsolid;
}

// Returns true if we hit something and it wasn't the world.
bool CGameTrace::DidHitNonWorldCBaseEntity() const
{
    return m_pEnt != NULL && !DidHitWorld();
}

bool GameUtils::WorldToScreen( const Vector& in, Vector& position )
{
	static uintptr_t clientDll = 0;
	while (!clientDll)
	{
		clientDll = (uintptr_t)GetModuleHandle((client_dll));
		Sleep(100);
	}

	float w = csgo::viewMatrix[3][0] * in.x + csgo::viewMatrix[3][1] * in.y + csgo::viewMatrix[3][2] * in.z + csgo::viewMatrix[3][3];

	float ScreenWidth = (float)csgo::Screen.width;
	float ScreenHeight = (float)csgo::Screen.height;

	if (w > 0.01)
	{
		float inverseWidth = 1 / w;
		position.x = (float)((ScreenWidth / 2) + (0.5 * ((csgo::viewMatrix[0][0] * in.x + csgo::viewMatrix[0][1] * in.y + csgo::viewMatrix[0][2] * in.z + csgo::viewMatrix[0][3]) * inverseWidth) * ScreenWidth + 0.5));
		position.y = (float)((ScreenHeight / 2) - (0.5 * ((csgo::viewMatrix[1][0] * in.x + csgo::viewMatrix[1][1] * in.y + csgo::viewMatrix[1][2] * in.z + csgo::viewMatrix[1][3]) * inverseWidth) * ScreenHeight + 0.5));
		return true;
	}

	return false;
}

bool IsVisible(const Vector & start, const Vector & end, CBaseEntity * pPlayer)
{
	Ray_t ray;
	trace_t tr;
	ray.Init(start, end);

	CTraceFilter filter;
	filter.pSkip1 = csgo::LocalPlayer;

	g_pEngineTrace->TraceRay(ray, MASK_SHOT, &filter, &tr);

	if (tr.m_pEnt == pPlayer || tr.fraction > 0.97f)
	{
		return true;
	}
	return false;
}

std::vector<Vector> GameUtils::GetMultiplePointsForHitbox(CBaseEntity* pBaseEntity, int iHitbox, VMatrix BoneMatrix[128])
{
	auto VectorTransform_Wrapper = [](const Vector& in1, const VMatrix &in2, Vector &out)
	{
		auto VectorTransform = [](const float *in1, const VMatrix& in2, float *out)
		{
			auto DotProducts = [](const float *v1, const float *v2)
			{
				return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
			};
			out[0] = DotProducts(in1, in2[0]) + in2[0][3];
			out[1] = DotProducts(in1, in2[1]) + in2[1][3];
			out[2] = DotProducts(in1, in2[2]) + in2[2][3];
		};
		VectorTransform(&in1.x, in2, &out.x);
	};

	studiohdr_t* pStudioModel = g_pModelInfo->GetStudioModel(pBaseEntity->GetModel());
	mstudiohitboxset_t* set = pStudioModel->pHitboxSet(0);
	mstudiobbox_t *hitbox = set->pHitbox(iHitbox);

	std::vector<Vector> vecArray;

	Vector max;
	Vector min;
	VectorTransform_Wrapper(hitbox->bbmax, BoneMatrix[hitbox->bone], max);
	VectorTransform_Wrapper(hitbox->bbmin, BoneMatrix[hitbox->bone], min);

	auto center = (min + max) * 0.5f;

	QAngle CurrentAngles = GameUtils::CalculateAngle(center, csgo::LocalPlayer->GetEyePosition());

	Vector Forward;
	Math::AngleVectors(CurrentAngles, &Forward);

	Vector Right = Forward.Cross(Vector(0, 0, 1));
	Vector Left = Vector(-Right.x, -Right.y, Right.z);

	Vector Top = Vector(0, 0, 1);
	Vector Bot = Vector(0, 0, -1);

	const float psh = Menu.Ragebot.headscale / 100;
	const float psb = Menu.Ragebot.bodyscale / 100;
	switch (iHitbox) {
	case (int)CSGOHitboxID::HEAD:
		for (auto i = 0; i < 4; ++i)
			vecArray.emplace_back(center);	
		vecArray.at(1) += Top * (hitbox->radius * psh);
		vecArray.at(2) += Right * (hitbox->radius * psh);
		vecArray.at(3) += Left * (hitbox->radius * psh);
		break;
	case (int)CSGOHitboxID::LOWER_CHEST:
		for (auto i = 0; i < 4; ++i)
			vecArray.emplace_back(center);
		vecArray.at(1) += Top * (hitbox->radius * psh);
		vecArray.at(2) += Right * (hitbox->radius * psh);
		vecArray.at(3) += Left * (hitbox->radius * psh);
		break;
	case (int)CSGOHitboxID::THORAX:
		for (auto i = 0; i < 4; ++i)
			vecArray.emplace_back(center);
		vecArray.at(1) += Top * (hitbox->radius * psh);
		vecArray.at(2) += Right * (hitbox->radius * psh);
		vecArray.at(3) += Left * (hitbox->radius * psh);
		break;
	case (int)CSGOHitboxID::UPPER_CHEST:
		for (auto i = 0; i < 4; ++i)
			vecArray.emplace_back(center);
		vecArray.at(1) += Top * (hitbox->radius * psh);
		vecArray.at(2) += Right * (hitbox->radius * psh);
		vecArray.at(3) += Left * (hitbox->radius * psh);
		break;
	case (int)CSGOHitboxID::RIGHT_THIGH:
		for (auto i = 0; i < 4; ++i)
			vecArray.emplace_back(center);
		vecArray.at(1) += Top * (hitbox->radius * psh);
		vecArray.at(2) += Right * (hitbox->radius * (psh));
		vecArray.at(3) += Left * (hitbox->radius * (psh));
		break;
	case (int)CSGOHitboxID::LEFT_THIGH:
		for (auto i = 0; i < 4; ++i)
			vecArray.emplace_back(center);
		vecArray.at(1) += Top * (hitbox->radius * psh);
		vecArray.at(2) += Right * (hitbox->radius * psh);
		vecArray.at(3) += Left * (hitbox->radius * psh);
		break;
	case (int)CSGOHitboxID::PELVIS:
		for (auto i = 0; i < 4; ++i)
			vecArray.emplace_back(center);
		vecArray.at(1) += Top * (hitbox->radius * 0.01f);
		vecArray.at(2) += Right * (hitbox->radius * psb);
		vecArray.at(3) += Left * (hitbox->radius * psb);
		break;
	case (int)CSGOHitboxID::BELLY:
		for (auto i = 0; i < 4; ++i)
			vecArray.emplace_back(center);
		vecArray.at(1) += Top * (hitbox->radius * psb);
		vecArray.at(2) += Right * (hitbox->radius * psb);
		vecArray.at(3) += Left * (hitbox->radius * psb);
		break;

	default:
		for (auto i = 0; i < 4; ++i)
			vecArray.emplace_back(center);
		vecArray.at(1) += Top * (hitbox->radius * 0.01f);
		vecArray.at(2) += Right * (hitbox->radius * 0.01f);
		vecArray.at(3) += Left * (hitbox->radius * 0.01f);
		break;
	}
	return vecArray;
}

Vector GameUtils::GetBonePosition( CBaseEntity* pPlayer, int Bone, VMatrix MatrixArray[ 128 ] )
{
    Vector pos = Vector();

	VMatrix & HitboxMatrix = MatrixArray[ Bone ];

    pos = Vector( HitboxMatrix[ 0 ][ 3 ], HitboxMatrix[ 1 ][ 3 ], HitboxMatrix[ 2 ][ 3 ] );

    return pos;
}

Vector GameUtils::get_hitbox_location(CBaseEntity* obj, int hitbox_id)
{
	VMatrix bone_matrix[128];
	auto VectorTransform_Wrapper = [](const Vector& in1, const VMatrix &in2, Vector &out)
	{
		auto VectorTransform = [](const float *in1, const VMatrix& in2, float *out)
		{
			auto DotProducts = [](const float *v1, const float *v2)
			{
				return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
			};
			out[0] = DotProducts(in1, in2[0]) + in2[0][3];
			out[1] = DotProducts(in1, in2[1]) + in2[1][3];
			out[2] = DotProducts(in1, in2[2]) + in2[2][3];
		};
		VectorTransform(&in1.x, in2, &out.x);
	};
	if (bone_matrix != nullptr && obj != nullptr)
	{

		if (obj->SetupBones(bone_matrix, 128, 0x00000100, 0.0f)) {
			if (obj->GetModel()) {
				auto studio_model = g_pModelInfo->GetStudioModel(obj->GetModel());
				if (studio_model)
				{
					mstudiohitboxset_t* hitbox0 = studio_model->pHitboxSet(0);
					mstudiobbox_t* hitbox = hitbox0->pHitbox(hitbox_id);
					if (hitbox)
					{
						auto min = Vector{}, max = Vector{};

						VectorTransform_Wrapper(hitbox->bbmin, bone_matrix[hitbox->bone], min);
						VectorTransform_Wrapper(hitbox->bbmax, bone_matrix[hitbox->bone], max);

						return (min + max) / 2.0f;
					}
				}
			}
		}
	}
	return Vector{};
}

void GameUtils::TraceLine( Vector& vecAbsStart, Vector& vecAbsEnd, unsigned int mask, CBaseEntity* ignore, trace_t* ptr )
{
    Ray_t ray;
    ray.Init( vecAbsStart, vecAbsEnd );
    CTraceFilter filter;
    filter.pSkip1 = ignore;

    g_pEngineTrace->TraceRay( ray, mask, &filter, ptr );
}

bool GameUtils::IsVisible_Fix( Vector vecOrigin, Vector vecOther, unsigned int mask, CBaseEntity* pCBaseEntity, CBaseEntity* pIgnore, int& hitgroup )
{
    auto AngleVectors = []( const Vector &angles, Vector *forward )
    {
        Assert( s_bMathlibInitialized );
        Assert( forward );

        float	sp, sy, cp, cy;

        sy = sin( DEG2RAD( angles[ 1 ] ) );
        cy = cos( DEG2RAD( angles[ 1 ] ) );

        sp = sin( DEG2RAD( angles[ 0 ] ) );
        cp = cos( DEG2RAD( angles[ 0 ] ) );

        forward->x = cp*cy;
        forward->y = cp*sy;
        forward->z = -sp;
    };

    Ray_t ray;
    trace_t tr;
    ray.Init( vecOrigin, vecOther );
    CTraceFilter filter;
    filter.pSkip1 = pIgnore;

    g_pEngineTrace->TraceRay( ray, mask, &filter, &tr );

    //not correctly done tho

    //Vector direction;

    //Vector angles = GameUtils::CalculateAngle(vecOrigin, vecOther);
    //AngleVectors(angles, &direction);
    //VectorNormalize(direction);

    //UTIL_ClipTraceToPlayers(vecOrigin, vecOrigin + direction*40.f, 0x4600400B, &filter, &tr);

    hitgroup = tr.hitgroup;

    return ( tr.m_pEnt == pCBaseEntity || tr.fraction >= 1.0f );
}

static float GameUtils::GetFov(const Vector& viewAngle, const Vector& aimAngle)
{
	Vector ang, aim;

	Math::AngleVectors(viewAngle, aim);
	Math::AngleVectors(aimAngle, ang);

	return RAD2DEG(acos(aim.Dot(ang) / aim.LengthSqr()));
}


bool GameUtils::isVisible(CBaseEntity* lul, int bone)
{
	Ray_t ray;
	trace_t tr;

	ray.Init(csgo::LocalPlayer->GetEyePosition(), lul->GetBonePos(bone));

	CTraceFilter filter;
	filter.pSkip1 = csgo::LocalPlayer;

	g_pEngineTrace->TraceRay(ray, (0x1 | 0x4000 | 0x2000000 | 0x2 | 0x4000000 | 0x40000000), &filter, &tr);

	if (tr.m_pEnt == lul)
	{
		return true;
	}

	return false;
}


float GameUtils::GetFoV( QAngle qAngles, Vector vecSource, Vector vecDestination, bool bDistanceBased )
{
    auto MakeVector = []( QAngle qAngles )
    {
        auto ret = Vector();
        auto pitch = float( qAngles[ 0 ] * M_PI / 180.f );
        auto yaw = float( qAngles[ 1 ] * M_PI / 180.f );
        auto tmp = float( cos( pitch ) );
        ret.x = float( -tmp * -cos( yaw ) );
        ret.y = float( sin( yaw )*tmp );
        ret.z = float( -sin( pitch ) );
        return ret;
    };

    Vector ang, aim;
    double fov;

    ang = CalculateAngle( vecSource, vecDestination );
    aim = MakeVector( qAngles );
    ang = MakeVector( ang );

    auto mag_s = sqrt( ( aim[ 0 ] * aim[ 0 ] ) + ( aim[ 1 ] * aim[ 1 ] ) + ( aim[ 2 ] * aim[ 2 ] ) );
    auto mag_d = sqrt( ( aim[ 0 ] * aim[ 0 ] ) + ( aim[ 1 ] * aim[ 1 ] ) + ( aim[ 2 ] * aim[ 2 ] ) );
    auto u_dot_v = aim[ 0 ] * ang[ 0 ] + aim[ 1 ] * ang[ 1 ] + aim[ 2 ] * ang[ 2 ];

    fov = acos( u_dot_v / ( mag_s*mag_d ) ) * ( 180.f / M_PI );

    if( bDistanceBased ) {
        fov *= 1.4;
        float xDist = abs( vecSource[ 0 ] - vecDestination[ 0 ] );
        float yDist = abs( vecSource[ 1 ] - vecDestination[ 1 ] );
        float Distance = sqrt( ( xDist * xDist ) + ( yDist * yDist ) );

        Distance /= 650.f;

        if( Distance < 0.7f )
            Distance = 0.7f;

        if( Distance > 6.5 )
            Distance = 6.5;

        fov *= Distance;
    }

    return (float)fov;
}

Vector GameUtils::CalculateAngle( Vector vecOrigin, Vector vecOther )
{
    auto ret = Vector();
    Vector delta = vecOrigin - vecOther;
    double hyp = delta.Length2D();
    ret.y = ( atan( delta.y / delta.x ) * 57.295779513082f );
    ret.x = ( atan( delta.z / hyp ) * 57.295779513082f );
    ret.z = 0.f;

    if( delta.x >= 0.f )
        ret.y += 180.f;
    return ret;
}
Vector GameUtils::CalcAngle(Vector src, Vector dst)
{
	Vector ret;
	Math::VectorAngles(dst - src, ret);
	return ret;
}
void GameUtils::calculate_angle(Vector src, Vector dst, Vector &angles)
{
	Vector delta = src - dst;
	double hyp = delta.Length2D();
	angles.y = (atan(delta.y / delta.x) * 57.295779513082f);
	angles.x = (atan(delta.z / hyp) * 57.295779513082f);
	angles[2] = 0.0f;
	if (delta.x >= 0.0) angles.y += 180.0f;
}

bool GameUtils::IsAbleToShoot()
{
    if( !g_pGlobals )
        return false;

    if( !csgo::LocalPlayer || !csgo::MainWeapon )
        return false;

    auto flServerTime = (float)csgo::LocalPlayer->GetTickBase() * g_pGlobals->interval_per_tick;
    auto flNextPrimaryAttack = csgo::MainWeapon->NextPrimaryAttack();

    return( !( flNextPrimaryAttack > flServerTime ) );
}

bool GameUtils::IsBreakableEntity( CBaseEntity* pBaseEntity )
{
    if( !pBaseEntity )
        return false;

    if( pBaseEntity->GetCollisionGroup() != COLLISION_GROUP_PUSHAWAY && pBaseEntity->GetCollisionGroup() != COLLISION_GROUP_BREAKABLE_GLASS && pBaseEntity->GetCollisionGroup() != COLLISION_GROUP_NONE )
        return false;

    if( pBaseEntity->GetHealth() > 200 )
        return false;

    IMultiplayerPhysics* pPhysicsInterface = (IMultiplayerPhysics*)pBaseEntity;
    if( pPhysicsInterface ) {
        if( pPhysicsInterface->GetMultiplayerPhysicsMode() != PHYSICS_MULTIPLAYER_SOLID )
            return false;
    }
    else {
        ClientClass * pClass = (ClientClass*)pBaseEntity->GetClientClass();
        if( !string( pClass->m_pNetworkName ).compare( XorStr( "func_breakable" ) ) || !string( pClass->m_pNetworkName ).compare( XorStr( "func_breakable_surf" ) ) ) {
            if( !string( pClass->m_pNetworkName ).compare( XorStr( "func_breakable_surf" ) ) )
                if( pBaseEntity->IsBroken() )
                    return false;
        }
        else if( pBaseEntity->PhysicsSolidMaskForEntity() & CONTENTS_PLAYERCLIP )
            return false;
    }

    IBreakableWithPropData* pBreakableInterface = (IBreakableWithPropData*)pBaseEntity;
    if( pBreakableInterface )
        if( pBreakableInterface->GetDmgModBullet() <= 0.0f )
            return false;

    return true;
}