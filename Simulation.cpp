#include "sdk.h"
#include "Simulation.h"


RebuildGameMovement::RebuildGameMovement(void)
{

}

void RebuildGameMovement::SetAbsOrigin(CBaseEntity *player, const Vector &vec)
{
	if (player &&
		player->GetMoveType() == MOVETYPE_WALK)
	{
		trace_t pm;
		TracePlayerBBox(vec, vec, MASK_PLAYERSOLID, COLLISION_GROUP_PLAYER_MOVEMENT, pm, player);
		if (pm.startsolid || pm.allsolid || pm.fraction != 1.0f)
		{
			//player will get stuck, lets not?
			return;
		}
	}

	//player->GetAbsOrigin() = vec;
}

int RebuildGameMovement::ClipVelocity(Vector &in, Vector &normal, Vector &out, float overbounce)
{
	float	backoff;
	float	change;
	float angle;
	int		i, blocked;

	angle = normal[2];

	blocked = 0x00;         // Assume unblocked.
	if (angle > 0)			// If the plane that is blocking us has a positive z component, then assume it's a floor.
		blocked |= 0x01;	// 
	if (!angle)				// If the plane has no Z, it is vertical (wall/step)
		blocked |= 0x02;	// 


							// Determine how far along plane to slide based on incoming direction.
	backoff = DotProduct(in, normal) * overbounce;

	for (i = 0; i<3; i++)
	{
		change = normal[i] * backoff;
		out[i] = in[i] - change;
	}

	// iterate once to make sure we aren't still moving through the plane
	float adjust = DotProduct(out, normal);
	if (adjust < 0.0f)
	{
		out -= (normal * adjust);
		//		Msg( "Adjustment = %lf\n", adjust );
	}

	// Return blocking flags.
	return blocked;
}

int RebuildGameMovement::TryPlayerMove(CBaseEntity *player, Vector *pFirstDest, trace_t *pFirstTrace)
{
	Vector  planes[5];
	numbumps[player->GetIndex()] = 4;           // Bump up to four times

	blocked[player->GetIndex()] = 0;           // Assume not blocked
	numplanes[player->GetIndex()] = 0;           //  and not sliding along any planes

	original_velocity[player->GetIndex()] = player->GetVelocity(); // Store original velocity
	primal_velocity[player->GetIndex()] = player->GetVelocity();

	allFraction[player->GetIndex()] = 0;
	time_left[player->GetIndex()] = p_GlobVars->frametime;   // Total time for this movement operation.

	new_velocity[player->GetIndex()].clearVekt();

	for (bumpcount[player->GetIndex()] = 0; bumpcount[player->GetIndex()] < numbumps[player->GetIndex()]; bumpcount[player->GetIndex()]++)
	{
		if (player->GetVelocity().length() == 0.0)
			break;

		// Assume we can move all the way from the current origin to the
		//  end point.
		VectorMA(player->getAbsOriginal(), time_left[player->GetIndex()], player->GetVelocity(), end[player->GetIndex()]);

		// See if we can make it from origin to end point.
		if (true)
		{
			// If their velocity Z is 0, then we can avoid an extra trace here during WalkMove.
			if (pFirstDest && end[player->GetIndex()] == *pFirstDest)
				pm[player->GetIndex()] = *pFirstTrace;
			else
			{
				TracePlayerBBox(player->getAbsOriginal(), end[player->GetIndex()], MASK_PLAYERSOLID, 8, pm[player->GetIndex()], player);
			}
		}
		else
		{
			TracePlayerBBox(player->getAbsOriginal(), end[player->GetIndex()], MASK_PLAYERSOLID, 8, pm[player->GetIndex()], player);
		}

		allFraction[player->GetIndex()] += pm[player->GetIndex()].fraction;

		// If we started in a solid object, or we were in solid space
		//  the whole way, zero out our velocity and return that we
		//  are blocked by floor and wall.
		if (pm[player->GetIndex()].allsolid)
		{
			// entity is trapped in another solid
			player->GetVelocity() = vec3_origin[player->GetIndex()];
			return 4;
		}

		// If we moved some portion of the total distance, then
		//  copy the end position into the pmove.origin and 
		//  zero the plane counter.
		if (pm[player->GetIndex()].fraction > 0)
		{
			if (numbumps[player->GetIndex()] > 0 && pm[player->GetIndex()].fraction == 1)
			{
				// There's a precision issue with terrain tracing that can cause a swept box to successfully trace
				// when the end position is stuck in the triangle.  Re-run the test with an uswept box to catch that
				// case until the bug is fixed.
				// If we detect getting stuck, don't allow the movement
				trace_t stuck;
				TracePlayerBBox(pm[player->GetIndex()].endpos, pm[player->GetIndex()].endpos, MASK_PLAYERSOLID, 8, stuck, player);
				if (stuck.startsolid || stuck.fraction != 1.0f)
				{
					//Msg( "Player will become stuck!!!\n" );
					player->GetVelocity() = vec3_origin[player->GetIndex()];
					break;
				}
			}

			// actually covered some distance
			SetAbsOrigin(player, pm[player->GetIndex()].endpos);
			original_velocity[player->GetIndex()] = player->GetVelocity();
			numplanes[player->GetIndex()] = 0;
		}

		// If we covered the entire distance, we are done
		//  and can return.
		if (pm[player->GetIndex()].fraction == 1)
		{
			break;		// moved the entire distance
		}

		// If the plane we hit has a high z component in the normal, then
		//  it's probably a floor
		if (pm[player->GetIndex()].plane.normal[2] > 0.7)
		{
			blocked[player->GetIndex()] |= 1;		// floor
		}
		// If the plane has a zero z component in the normal, then it's a 
		//  step or wall
		if (!pm[player->GetIndex()].plane.normal[2])
		{
			blocked[player->GetIndex()] |= 2;		// step / wall
		}

		// Reduce amount of m_flFrameTime left by total time left * fraction
		//  that we covered.
		time_left[player->GetIndex()] -= time_left[player->GetIndex()] * pm[player->GetIndex()].fraction;

		// Did we run out of planes to clip against?
		if (numplanes[player->GetIndex()] >= 5)
		{
			// this shouldn't really happen
			//  Stop our movement if so.
			player->GetVelocity() = vec3_origin[player->GetIndex()];
			//Con_DPrintf("Too many planes 4\n");

			break;
		}

		// Set up next clipping plane
		planes[numplanes[player->GetIndex()]] = pm[player->GetIndex()].plane.normal;
		numplanes[player->GetIndex()]++;

		// modify original_velocity so it parallels all of the clip planes
		//

		// reflect player velocity 
		// Only give this a try for first impact plane because you can get yourself stuck in an acute corner by jumping in place
		//  and pressing forward and nobody was really using this bounce/reflection feature anyway...
		if (numplanes[player->GetIndex()] == 1 &&
			player->getFlags() & FL_ONGROUND)
		{
			for (i[player->GetIndex()] = 0; i[player->GetIndex()] < numplanes[player->GetIndex()]; i[player->GetIndex()]++)
			{
				if (planes[i[player->GetIndex()]][2] > 0.7)
				{
					// floor or slope
					ClipVelocity(original_velocity[player->GetIndex()], planes[i[player->GetIndex()]], new_velocity[player->GetIndex()], 1);
					original_velocity[player->GetIndex()] = new_velocity[player->GetIndex()];
				}
				else
				{
					ClipVelocity(original_velocity[player->GetIndex()], planes[i[player->GetIndex()]], new_velocity[player->GetIndex()], 1.0 + p_Console->FindVar("sv_bounce")->fValue * (1 - player->getSurfaceFriction()));
				}
			}

			player->GetVelocity() = new_velocity[player->GetIndex()];
			original_velocity[player->GetIndex()] = new_velocity[player->GetIndex()];
		}
		else
		{
			for (i[player->GetIndex()] = 0; i[player->GetIndex()] < numplanes[player->GetIndex()]; i[player->GetIndex()]++)
			{


				for (j[player->GetIndex()] = 0; j[player->GetIndex()]<numplanes[player->GetIndex()]; j[player->GetIndex()]++)
					if (j[player->GetIndex()] != i[player->GetIndex()])
					{
						// Are we now moving against this plane?
						if (DotProduct(player->GetVelocity(), planes[j[player->GetIndex()]]) < 0)
							break;	// not ok
					}
				if (j[player->GetIndex()] == numplanes[player->GetIndex()])  // Didn't have to clip, so we're ok
					break;
			}

			// Did we go all the way through plane set
			if (i[player->GetIndex()] != numplanes[player->GetIndex()])
			{	// go along this plane
				// pmove.velocity is set in clipping call, no need to set again.
				;
			}
			else
			{	// go along the crease
				if (numplanes[player->GetIndex()] != 2)
				{
					player->GetVelocity() = vec3_origin[player->GetIndex()];
					break;
				}

				CrossProduct(planes[0], planes[1], dir[player->GetIndex()]);
				dir[player->GetIndex()].normalizeInPlace();
				d[player->GetIndex()] = DotProduct(dir[player->GetIndex()], player->GetVelocity());
				VectorMultiply(dir[player->GetIndex()], d[player->GetIndex()], player->GetVelocity());
			}

			//
			// if original velocity is against the original velocity, stop dead
			// to avoid tiny occilations in sloping corners
			//
			d[player->GetIndex()] = DotProduct(player->GetVelocity(), primal_velocity[player->GetIndex()]);
			if (d[player->GetIndex()] <= 0)
			{
				//Con_DPrintf("Back\n");
				player->GetVelocity() = vec3_origin[player->GetIndex()];
				break;
			}
		}
	}

	if (allFraction == 0)
	{
		player->GetVelocity() = vec3_origin[player->GetIndex()];
	}

	// Check if they slammed into a wall
	float fSlamVol = 0.0f;

	float fLateralStoppingAmount = primal_velocity[player->GetIndex()].length2d() - player->GetVelocity().length2d();
	if (fLateralStoppingAmount > 580.f * 2.0f)
	{
		fSlamVol = 1.0f;
	}
	else if (fLateralStoppingAmount > 580.f)
	{
		fSlamVol = 0.85f;
	}

	return blocked[player->GetIndex()];
}

void RebuildGameMovement::Accelerate(CBaseEntity *player, Vector &wishdir, float wishspeed, float accel)
{
	// See if we are changing direction a bit
	currentspeed[player->GetIndex()] = DotProduct(player->GetVelocity(), wishdir);

	// Reduce wishspeed by the amount of veer.
	addspeed[player->GetIndex()] = wishspeed - currentspeed[player->GetIndex()];

	// If not going to add any speed, done.
	if (addspeed[player->GetIndex()] <= 0)
		return;

	// Determine amount of accleration.
	accelspeed[player->GetIndex()] = accel * p_GlobVars->frametime * wishspeed * player->getSurfaceFriction();

	// Cap at addspeed
	if (accelspeed[player->GetIndex()] > addspeed[player->GetIndex()])
		accelspeed[player->GetIndex()] = addspeed[player->GetIndex()];

	// Adjust velocity.
	for (i[player->GetIndex()] = 0; i[player->GetIndex()]<3; i[player->GetIndex()]++)
	{
		player->GetVelocity()[i[player->GetIndex()]] += accelspeed[player->GetIndex()] * wishdir[i[player->GetIndex()]];
	}
}

void RebuildGameMovement::AirAccelerate(CBaseEntity *player, Vector &wishdir, float wishspeed, float accel)
{

	wishspd[player->GetIndex()] = wishspeed;

	// Cap speed
	if (wishspd[player->GetIndex()] > 30.f)
		wishspd[player->GetIndex()] = 30.f;

	// Determine veer amount
	currentspeed[player->GetIndex()] = DotProduct(player->GetVelocity(), wishdir);

	// See how much to add
	addspeed[player->GetIndex()] = wishspd[player->GetIndex()] - currentspeed[player->GetIndex()];

	// If not adding any, done.
	if (addspeed <= 0)
		return;

	// Determine acceleration speed after acceleration
	accelspeed[player->GetIndex()] = accel * wishspeed * p_GlobVars->frametime * player->getSurfaceFriction();

	// Cap it
	if (accelspeed[player->GetIndex()] > addspeed[player->GetIndex()])
		accelspeed[player->GetIndex()] = addspeed[player->GetIndex()];

	// Adjust pmove vel.
	for (i[player->GetIndex()] = 0; i[player->GetIndex()]<3; i[player->GetIndex()]++)
	{
		player->GetVelocity()[i[player->GetIndex()]] += accelspeed[player->GetIndex()] * wishdir[i[player->GetIndex()]];
		p_MoveHelper->SetHost(reinterpret_cast<void*>(player));
		p_MoveHelper->m_outWishVel[i[player->GetIndex()]] += accelspeed[player->GetIndex()] * wishdir[i[player->GetIndex()]];

	}
}

void RebuildGameMovement::AirMove(CBaseEntity *player)
{
	csFuncs->aechseVektor(player->getEyeAechse(), (float*)&forward[player->GetIndex()], (float*)&right[player->GetIndex()], (float*)&up[player->GetIndex()]);  // Determine movement angles

																																						 // Copy movement amounts
	p_MoveHelper->SetHost(player);
	fmove[player->GetIndex()] = p_MoveHelper->m_flForwardMove;
	smove[player->GetIndex()] = p_MoveHelper->m_flSideMove;

	// Zero out z components of movement vectors
	forward[player->GetIndex()][2] = 0;
	right[player->GetIndex()][2] = 0;
	csFuncs->normaleAechse(forward[player->GetIndex()]);  // Normalize remainder of vectors
	csFuncs->normaleAechse(right[player->GetIndex()]);    // 

	for (i[player->GetIndex()] = 0; i[player->GetIndex()]<2; i[player->GetIndex()]++)       // Determine x and y parts of velocity
		wishvel[player->GetIndex()][i[player->GetIndex()]] = forward[player->GetIndex()][i[player->GetIndex()]] * fmove[player->GetIndex()] + right[player->GetIndex()][i[player->GetIndex()]] * smove[player->GetIndex()];

	wishvel[player->GetIndex()][2] = 0;             // Zero out z part of velocity

	wishdir[player->GetIndex()] = wishvel[player->GetIndex()]; // Determine maginitude of speed of move
	wishspeed[player->GetIndex()] = VectorNormalize(wishdir[player->GetIndex()]);

	//
	// clamp to server defined max speed
	//
	if (wishspeed != 0 && (wishspeed[player->GetIndex()] > player->getMaxSpeed()))
	{
		VectorMultiply(wishvel[player->GetIndex()], player->getMaxSpeed() / wishspeed[player->GetIndex()], wishvel[player->GetIndex()]);
		wishspeed[player->GetIndex()] = player->getMaxSpeed();
	}

	AirAccelerate(player, wishdir[player->GetIndex()], wishspeed[player->GetIndex()], p_Console->FindVar("sv_airaccelerate")->fValue);

	// Add in any base velocity to the current velocity.
	VectorAdd(player->GetVelocity(), player->GetVelocity(), player->GetVelocity());
	trace_t trace;
	TryPlayerMove(player, &dest[player->GetIndex()], &trace);

	// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
	VectorSubtract(player->GetVelocity(), player->GetVelocity(), player->GetVelocity());
}

void RebuildGameMovement::StepMove(CBaseEntity *player, Vector &vecDestination, trace_t &trace)
{
	Vector vecEndPos;
	vecEndPos = vecDestination;

	// Try sliding forward both on ground and up 16 pixels
	//  take the move that goes farthest
	Vector vecPos, vecVel;
	vecPos = player->getAbsOriginal();
	vecVel = player->GetVelocity();

	// Slide move down.
	TryPlayerMove(player, &vecEndPos, &trace);

	// Down results.
	Vector vecDownPos, vecDownVel;
	vecDownPos = player->getAbsOriginal();
	vecDownVel = player->GetVelocity();

	// Reset original values.
	SetAbsOrigin(player, vecPos);
	player->GetVelocity() = vecVel;

	// Move up a stair height.
	vecEndPos = player->getAbsOriginal();

	vecEndPos.zAechse += player->getStepSize() + 0.03125;


	TracePlayerBBox(player->getAbsOriginal(), vecEndPos, MASK_PLAYERSOLID, 8, trace, player);
	if (!trace.startsolid && !trace.allsolid)
	{
		SetAbsOrigin(player, trace.endpos);
	}

	TryPlayerMove(player, &dest[player->GetIndex()], &trace);

	// Move down a stair (attempt to).
	vecEndPos = player->getAbsOriginal();

	vecEndPos.zAechse -= player->getStepSize() + 0.03125;


	TracePlayerBBox(player->getAbsOriginal(), vecEndPos, MASK_PLAYERSOLID, 8, trace, player);

	// If we are not on the ground any more then use the original movement attempt.
	if (trace.plane.normal[2] < 0.7)
	{
		SetAbsOrigin(player, vecDownPos);
		player->GetVelocity() = vecDownVel;

		float flStepDist = player->getAbsOriginal().zAechse - vecPos.zAechse;
		if (flStepDist > 0.0f)
		{
			p_MoveHelper->SetHost(player);
			p_MoveHelper->m_outStepHeight += flStepDist;
			p_MoveHelper->SetHost(nullptr);
		}
		return;
	}

	// If the trace ended up in empty space, copy the end over to the origin.
	if (!trace.startsolid && !trace.allsolid)
	{
		player->getAbsOriginal() = trace.endpos;
	}

	// Copy this origin to up.
	Vector vecUpPos;
	vecUpPos = player->getAbsOriginal();

	// decide which one went farther
	float flDownDist = (vecDownPos.xAechse - vecPos.xAechse) * (vecDownPos.xAechse - vecPos.xAechse) + (vecDownPos.yAechse - vecPos.yAechse) * (vecDownPos.yAechse - vecPos.yAechse);
	float flUpDist = (vecUpPos.xAechse - vecPos.xAechse) * (vecUpPos.xAechse - vecPos.xAechse) + (vecUpPos.yAechse - vecPos.yAechse) * (vecUpPos.yAechse - vecPos.yAechse);
	if (flDownDist > flUpDist)
	{
		SetAbsOrigin(player, vecDownPos);
		player->GetVelocity() = vecDownVel;
	}
	else
	{
		// copy z value from slide move
		player->GetVelocity() = vecDownVel;
	}

	float flStepDist = player->getAbsOriginal().zAechse - vecPos.zAechse;
	if (flStepDist > 0)
	{
		p_MoveHelper->SetHost(player);
		p_MoveHelper->m_outStepHeight += flStepDist;
		p_MoveHelper->SetHost(nullptr);
	}
}

void RebuildGameMovement::TracePlayerBBox(const Vector &start, const Vector &end, unsigned int fMask, int collisionGroup, trace_t& pm, CBaseEntity *player)
{
	Ray_t ray;
	TraceFilter filter;
	filter.pSkip = reinterpret_cast<void*>(player);

	ray.Init(start, end, player->getCollideable()->vecMins(), player->getCollideable()->vecMax());
	p_EngineTrace->TraceRay(ray, fMask, &filter, &pm);
}

void RebuildGameMovement::WalkMove(CBaseEntity *player)
{
	csFuncs->aechseVektor(player->getEyeAechse(), (float*)&forward[player->GetIndex()], (float*)&right[player->GetIndex()], (float*)&up[player->GetIndex()]);  // Determine movement angles


																																						 // Copy movement amounts
	p_MoveHelper->SetHost(player);
	fmove[player->GetIndex()] = p_MoveHelper->m_flForwardMove;
	smove[player->GetIndex()] = p_MoveHelper->m_flSideMove;
	p_MoveHelper->SetHost(nullptr);


	if (forward[player->GetIndex()][2] != 0)
	{
		forward[player->GetIndex()][2] = 0;
		VectorNormalize(forward[player->GetIndex()]);
	}

	if (right[player->GetIndex()][2] != 0)
	{
		right[player->GetIndex()][2] = 0;
		VectorNormalize(right[player->GetIndex()]);
	}


	for (i[player->GetIndex()] = 0; i[player->GetIndex()]<2; i[player->GetIndex()]++)       // Determine x and y parts of velocity
		wishvel[player->GetIndex()][i[player->GetIndex()]] = forward[player->GetIndex()][i[player->GetIndex()]] * fmove[player->GetIndex()] + right[player->GetIndex()][i[player->GetIndex()]] * smove[player->GetIndex()];

	wishvel[player->GetIndex()][2] = 0;             // Zero out z part of velocity

	wishdir[player->GetIndex()] = wishvel[player->GetIndex()]; // Determine maginitude of speed of move
	wishspeed[player->GetIndex()] = VectorNormalize(wishdir[player->GetIndex()]);

	//
	// Clamp to server defined max speed
	//
	p_MoveHelper->SetHost(player);
	if ((wishspeed[player->GetIndex()] != 0.0f) && (wishspeed[player->GetIndex()] > p_MoveHelper->m_flMaxSpeed))
	{
		VectorMultiply(wishvel[player->GetIndex()], player->getMaxSpeed() / wishspeed[player->GetIndex()], wishvel[player->GetIndex()]);
		wishspeed[player->GetIndex()] = player->getMaxSpeed();
	}
	p_MoveHelper->SetHost(nullptr);
	// Set pmove velocity
	player->GetVelocity()[2] = 0;
	Accelerate(player, wishdir[player->GetIndex()], wishspeed[player->GetIndex()], p_Console->FindVar("sv_accelerate")->fValue);
	player->GetVelocity()[2] = 0;

	// Add in any base velocity to the current velocity.
	VectorAdd(player->GetVelocity(), player->GetVelocity(), player->GetVelocity());

	spd[player->GetIndex()] = player->GetVelocity().length();

	if (spd[player->GetIndex()] < 1.0f)
	{
		player->GetVelocity().clearVekt();
		// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
		VectorSubtract(player->GetVelocity(), player->GetVelocity(), player->GetVelocity());
		return;
	}

	// first try just moving to the destination	
	dest[player->GetIndex()][0] = player->getAbsOriginal()[0] + player->GetVelocity()[0] * p_GlobVars->frametime;
	dest[player->GetIndex()][1] = player->getAbsOriginal()[1] + player->GetVelocity()[1] * p_GlobVars->frametime;
	dest[player->GetIndex()][2] = player->getAbsOriginal()[2];

	// first try moving directly to the next spot
	TracePlayerBBox(player->getAbsOriginal(), dest[player->GetIndex()], MASK_PLAYERSOLID, 8, pm[player->GetIndex()], player);

	// If we made it all the way, then copy trace end as new player position.
	p_MoveHelper->SetHost(player);
	p_MoveHelper->m_outWishVel += wishdir[player->GetIndex()] * wishspeed[player->GetIndex()];
	p_MoveHelper->SetHost(nullptr);

	if (pm[player->GetIndex()].fraction == 1)
	{
		player->getAbsOriginal() = pm[player->GetIndex()].endpos;
		// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
		VectorSubtract(player->GetVelocity(), player->GetVelocity(), player->GetVelocity());

		//tayOnGround(); //needed?
		return;
	}

	// Don't walk up stairs if not on ground.
	if (!(player->getFlags() & FL_ONGROUND))
	{
		// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
		VectorSubtract(player->GetVelocity(), player->GetVelocity(), player->GetVelocity());
		return;
	}

	StepMove(player, dest[player->GetIndex()], pm[player->GetIndex()]);

	// Now pull the base velocity back out.   Base velocity is set if you are on a moving object, like a conveyor (or maybe another monster?)
	VectorSubtract(player->GetVelocity(), player->GetVelocity(), player->GetVelocity());


}

void RebuildGameMovement::FinishGravity(CBaseEntity *player)
{
	float ent_gravity;

	ent_gravity = p_Console->FindVar("sv_gravity")->fValue;

	// Get the correct velocity for the end of the dt 
	player->GetVelocity()[2] -= (ent_gravity * p_Console->FindVar("sv_gravity")->fValue * p_GlobVars->frametime * 0.5);

	CheckVelocity(player);
}

void RebuildGameMovement::FullWalkMove(CBaseEntity *player)
{

	StartGravity(player);

	// Fricion is handled before we add in any base velocity. That way, if we are on a conveyor, 
	//  we don't slow when standing still, relative to the conveyor.
	if (player->getFlags() & FL_ONGROUND)
	{
		player->GetVelocity()[2] = 0.0;
		Friction(player);
	}

	// Make sure velocity is valid.
	CheckVelocity(player);

	if (player->getFlags() & FL_ONGROUND)
	{
		WalkMove(player);
	}
	else
	{
		AirMove(player);  // Take into account movement when in air.
	}

	// Make sure velocity is valid.
	CheckVelocity(player);

	// Add any remaining gravitational component.
	FinishGravity(player);


	// If we are on ground, no downward velocity.
	if (player->getFlags() & FL_ONGROUND)
	{
		player->GetVelocity()[2] = 0;
	}

	CheckFalling(player);
}

void RebuildGameMovement::Friction(CBaseEntity *player)
{
	// Calculate speed
	speed[player->GetIndex()] = player->GetVelocity().length();

	// If too slow, return
	if (speed[player->GetIndex()] < 0.1f)
	{
		return;
	}

	drop[player->GetIndex()] = 0;

	// apply ground friction
	if (player->getFlags() & FL_ONGROUND)  // On an entity that is the ground
	{
		friction[player->GetIndex()] = p_Console->FindVar("sv_friction")->fValue * player->getSurfaceFriction();

		//  Bleed off some speed, but if we have less than the bleed
		//  threshold, bleed the threshold amount.


		control[player->GetIndex()] = (speed[player->GetIndex()] < p_Console->FindVar("sv_stopspeed")->fValue) ? p_Console->FindVar("sv_stopspeed")->fValue : speed[player->GetIndex()];

		// Add the amount to the drop amount.
		drop[player->GetIndex()] += control[player->GetIndex()] * friction[player->GetIndex()] * p_GlobVars->frametime;
	}

	// scale the velocity
	newspeed[player->GetIndex()] = speed[player->GetIndex()] - drop[player->GetIndex()];
	if (newspeed[player->GetIndex()] < 0)
		newspeed[player->GetIndex()] = 0;

	if (newspeed[player->GetIndex()] != speed[player->GetIndex()])
	{
		// Determine proportion of old speed we are using.
		newspeed[player->GetIndex()] /= speed[player->GetIndex()];
		// Adjust velocity according to proportion.
		VectorMultiply(player->GetVelocity(), newspeed[player->GetIndex()], player->GetVelocity());
	}

	player->GetVelocity() -= (1.f - newspeed[player->GetIndex()]) * player->GetVelocity();
}


void RebuildGameMovement::CheckFalling(CBaseEntity *player)
{
	// this function really deals with landing, not falling, so early out otherwise
	if (player->getFallVel() <= 0)
		return;

	if (!player->GetHealth() && player->getFallVel() >= 303.0f)
	{
		bool bAlive = true;
		float fvol = 0.5;

		//
		// They hit the ground.
		//
		if (player->GetVelocity().z < 0.0f)
		{
			// Player landed on a descending object. Subtract the velocity of the ground entity.
			player->getFallVel() += player->GetVelocity().z;
			player->getFallVel() = max(0.1f, player->getFallVel());
		}

		if (player->getFallVel() > 526.5f)
		{
			fvol = 1.0;
		}
		else if (player->getFallVel() > 526.5f / 2)
		{
			fvol = 0.85;
		}
		else if (player->getFallVel() < 173)
		{
			fvol = 0;
		}

	}

	// let any subclasses know that the player has landed and how hard

	//
	// Clear the fall velocity so the impact doesn't happen again.
	//
	player->getFallVel() = 0;
}
const int nanmask = 255 << 23;
#define	IS_NAN(x) (((*(int *)&x)&nanmask)==nanmask)
void RebuildGameMovement::CheckVelocity(CBaseEntity *player)
{
	Vector org = player->getAbsOriginal();

	for (i[player->GetIndex()] = 0; i[player->GetIndex()] < 3; i[player->GetIndex()]++)
	{
		// See if it's bogus.
		if (IS_NAN(player->GetVelocity()[i[player->GetIndex()]]))
		{
			player->GetVelocity()[i[player->GetIndex()]] = 0;
		}

		if (IS_NAN(org[i[player->GetIndex()]]))
		{
			org[i[player->GetIndex()]] = 0;
			player->getAbsOriginal() = org;
		}

		// Bound it.
		if (player->GetVelocity()[i[player->GetIndex()]] > p_Console->FindVar("sv_maxvelocity")->fValue)
		{
			player->GetVelocity()[i[player->GetIndex()]] = p_Console->FindVar("sv_maxvelocity")->fValue;
		}
		else if (player->GetVelocity()[i[player->GetIndex()]] < -p_Console->FindVar("sv_maxvelocity")->fValue)
		{
			player->GetVelocity()[i[player->GetIndex()]] = -p_Console->FindVar("sv_maxvelocity")->fValue;
		}
	}
}
void RebuildGameMovement::StartGravity(CBaseEntity *player)
{
	if (!player || !player->GetHealth())
		return;

	Vector pVel = player->GetVelocity();

	pVel[2] -= (p_Console->FindVar("sv_gravity")->fValue * 0.5f * g_pGlobals->interval_per_tick);
	pVel[2] += (player->GetVelocity()[2] * p_GlobVars->interval_per_tick);

	player->GetVelocity() = pVel;

	Vector tmp = player->GetVelocity();
	tmp[2] = 0.f;
	player->GetVelocity() = tmp;
}