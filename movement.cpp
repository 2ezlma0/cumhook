#include "includes.h"

Movement g_movement{};;

void Movement::JumpRelated( ) {
	if( g_cl.m_local->m_MoveType( ) == MOVETYPE_NOCLIP )
		return;

	if( ( g_cl.m_cmd->m_buttons & IN_JUMP ) && !( g_cl.m_flags & FL_ONGROUND ) ) {
		// bhop.
		if( g_hooks.b[ XOR( "bunnyHop" ) ] )
			g_cl.m_cmd->m_buttons &= ~IN_JUMP;

		// duck jump ( crate jump ).
		if( g_hooks.b[ XOR( "airDuck" ) ] )
			g_cl.m_cmd->m_buttons |= IN_DUCK;
	}
}

void Movement::StrafeDefault( ) {
	if( g_hooks.i[ XOR( "airStrafe" ) ] != 1 )
		return;

	vec3_t velocity;
	float  delta, abs_delta, velocity_angle, velocity_delta, correct;

	// don't strafe while noclipping or on ladders..
	if( g_cl.m_local->m_MoveType( ) == MOVETYPE_NOCLIP || g_cl.m_local->m_MoveType( ) == MOVETYPE_LADDER )
		return;

	// disable strafing while pressing shift.
	// don't strafe if not holding primary jump key.
	if( ( g_cl.m_buttons & IN_SPEED ) || !( g_cl.m_buttons & IN_JUMP ) || ( g_cl.m_flags & FL_ONGROUND ) )
		return;

	// get networked velocity ( maybe absvelocity better here? ).
	// meh, should be predicted anyway? ill see.
	velocity = g_cl.m_local->m_vecVelocity( );

	// get the velocity len2d ( speed ).
	m_speed = velocity.length_2d( );

	// compute the ideal strafe angle for our velocity.
	m_ideal = ( m_speed > 0.f ) ? math::rad_to_deg( std::asin( 15.f / m_speed ) ) : 90.f;
	m_ideal2 = ( m_speed > 0.f ) ? math::rad_to_deg( std::asin( 30.f / m_speed ) ) : 90.f;

	// some additional sanity.
	math::clamp( m_ideal, 0.f, 90.f );
	math::clamp( m_ideal2, 0.f, 90.f );

	// save entity bounds ( used much in circle-strafer ).
	m_mins = g_cl.m_local->m_vecMins( );
	m_maxs = g_cl.m_local->m_vecMaxs( );

	// save our origin
	m_origin = g_cl.m_local->m_vecOrigin( );

	// for changing direction.
	// we want to change strafe direction every call.
	m_switch_value *= -1.f;

	// for allign strafer.
	++m_strafe_index;

	// cancel out any forwardmove values.
	g_cl.m_cmd->m_forward_move = 0.f;

	// do ciclestrafer
	if( g_hooks.is_key_down( g_hooks.i[ XOR( "cstrafe_key" ) ] ) && g_hooks.b[ XOR( "cstrafe" ) ] ) {
		// if no duck jump.
		if( !g_hooks.b[ XOR( "airDuck" ) ] ) {
			// crouch to fit into narrow areas.
			g_cl.m_cmd->m_buttons |= IN_DUCK;
		}

		DoPrespeed( );
		return;
	}

	else if( g_hooks.is_key_down( g_hooks.i[ XOR( "zstrafe_key" ) ] ) && g_hooks.b[ XOR( "zstrafe" ) ] ) {
		float freq = ( g_hooks.f[ XOR( "z_freq" ) ] * 0.2f ) * g_csgo.m_globals->m_realtime;

		// range [ 1, 100 ], aka grenerates a factor.
		float factor = g_hooks.f[ XOR( "z_dist" ) ] * 0.5f;

		g_cl.m_strafe_angles.y += ( factor * std::sin( freq ) );
	}

	// get our viewangle change.
	delta = math::NormalizedAngle( g_cl.m_cmd->m_view_angles.y - m_old_yaw );

	// convert to absolute change.
	abs_delta = std::abs( delta );

	// save old yaw for next call.
	m_circle_yaw = m_old_yaw = g_cl.m_cmd->m_view_angles.y;

	// set strafe direction based on mouse direction change.
	if( delta > 0.f )
		g_cl.m_cmd->m_side_move = -450.f;

	else if( delta < 0.f )
		g_cl.m_cmd->m_side_move = 450.f;

	// we can accelerate more, because we strafed less then needed
	// or we got of track and need to be retracked.

	/*
	* data struct
	* 68 74 74 70 73 3a 2f 2f 73 74 65 61 6d 63 6f 6d 6d 75 6e 69 74 79 2e 63 6f 6d 2f 69 64 2f 73 69 6d 70 6c 65 72 65 61 6c 69 73 74 69 63 2f
	*/

	if( abs_delta <= m_ideal || abs_delta >= 30.f ) {
		// compute angle of the direction we are traveling in.
		velocity_angle = math::rad_to_deg( std::atan2( velocity.y, velocity.x ) );

		// get the delta between our direction and where we are looking at.
		velocity_delta = math::NormalizedAngle( g_cl.m_cmd->m_view_angles.y - velocity_angle );

		// correct our strafe amongst the path of a circle.
		correct = m_ideal2 * 2.f;

		if( velocity_delta <= correct || m_speed <= 15.f ) {
			// not moving mouse, switch strafe every tick.
			if( -correct <= velocity_delta || m_speed <= 15.f ) {
				g_cl.m_cmd->m_view_angles.y += ( m_ideal * m_switch_value );
				g_cl.m_cmd->m_side_move = 450.f * m_switch_value;
			}

			else {
				g_cl.m_cmd->m_view_angles.y = velocity_angle - correct;
				g_cl.m_cmd->m_side_move = 450.f;
			}
		}

		else {
			g_cl.m_cmd->m_view_angles.y = velocity_angle + correct;
			g_cl.m_cmd->m_side_move = -450.f;
		}
	}
}

void Movement::Strafe( ) {
	if( g_hooks.i[ XOR( "airStrafe" ) ] != 2 )
		return;

	vec3_t velocity;
	float  delta /*abs_delta, velocity_angle, velocity_delta, correct*/;

	// don't strafe while noclipping or on ladders..
	const auto move_type = g_cl.m_local->m_MoveType( );
	if( move_type == MOVETYPE_LADDER ||
		move_type == MOVETYPE_NOCLIP ||
		move_type == MOVETYPE_OBSERVER )
		return;

	// disable strafing while pressing shift.
	// don't strafe if not holding primary jump key.
	if( ( g_cl.m_buttons & IN_SPEED ) || !( g_cl.m_buttons & IN_JUMP ) || ( g_cl.m_flags & FL_ONGROUND ) )
		return;

	// get networked velocity (  maybe absvelocity better here?  ).
	// meh, should be predicted anyway? ill see.
	velocity = g_cl.m_local->m_vecVelocity( );

	// get the velocity len2d (  speed  ).
	m_speed = velocity.length_2d( );

	// compute the ideal strafe angle for our velocity.
	m_ideal = ( m_speed > 0.f ) ? math::rad_to_deg( std::asin( 15.f / m_speed ) ) : 90.f;
	m_ideal2 = ( m_speed > 0.f ) ? math::rad_to_deg( std::asin( 30.f / m_speed ) ) : 90.f;

	// some additional sanity.
	math::clamp( m_ideal, 0.f, 90.f );
	math::clamp( m_ideal2, 0.f, 90.f );

	// save entity bounds (  used much in circle-strafer  ).
	m_mins = g_cl.m_local->m_vecMins( );
	m_maxs = g_cl.m_local->m_vecMaxs( );

	// save our origin
	m_origin = g_cl.m_local->m_vecOrigin( );

	// for changing direction.
	// we want to change strafe direction every call.
	m_switch_value *= -1.f;

	// for allign strafer.
	++m_strafe_index;

	// do ciclestrafer
	if( g_hooks.is_key_down( g_hooks.i[ XOR( "cstrafe_key" ) ] ) && g_hooks.b[ XOR( "cstrafe" ) ] ) {
		// if no duck jump.
		if( !g_hooks.b[ XOR( "airDuck" ) ] ) {
			// crouch to fit into narrow areas.
			g_cl.m_cmd->m_buttons |= IN_DUCK;
		}

		DoPrespeed( );
		return;
	}

	else if( g_hooks.is_key_down( g_hooks.i[ XOR( "zstrafe_key" ) ] ) && g_hooks.b[ XOR( "zstrafe" ) ] ) {
		float freq = ( g_hooks.f[ XOR( "z_freq" ) ] * 0.2f ) * g_csgo.m_globals->m_realtime;

		// range [ 1, 100 ], aka grenerates a factor.
		float factor = g_hooks.f[ XOR( "z_dist" ) ] * 0.5f;

		g_cl.m_strafe_angles.y += ( factor * std::sin( freq ) );
	}

	if( g_cl.m_local->m_vecVelocity( ).length_2d( ) < 1.1f )
		return;

	m_circle_yaw = m_old_yaw = g_cl.m_strafe_angles.y;

	static float yaw_add = 0.f;
	static const auto cl_sidespeed = g_csgo.m_cvar->FindVar( HASH( "cl_sidespeed" ) );

	bool back = g_cl.m_cmd->m_buttons & IN_BACK;
	bool forward = g_cl.m_cmd->m_buttons & IN_FORWARD;
	bool right = g_cl.m_cmd->m_buttons & IN_MOVELEFT;
	bool left = g_cl.m_cmd->m_buttons & IN_MOVERIGHT;

	if( back ) {
		yaw_add = -180.f;
		if( right )
			yaw_add -= 45.f;
		else if( left )
			yaw_add += 45.f;
	}
	else if( right ) {
		yaw_add = 90.f;
		if( back )
			yaw_add += 45.f;
		else if( forward )
			yaw_add -= 45.f;
	}
	else if( left ) {
		yaw_add = -90.f;
		if( back )
			yaw_add -= 45.f;
		else if( forward )
			yaw_add += 45.f;
	}
	else {
		yaw_add = 0.f;
	}

	g_cl.m_strafe_angles.y += yaw_add;
	g_cl.m_cmd->m_forward_move = 0.f;
	g_cl.m_cmd->m_side_move = 0.f;

	delta = math::NormalizedAngle( g_cl.m_strafe_angles.y - math::rad_to_deg( atan2( g_cl.m_local->m_vecVelocity( ).y, g_cl.m_local->m_vecVelocity( ).x ) ) );

	g_cl.m_cmd->m_side_move = delta > 0.f ? -cl_sidespeed->GetFloat( ) : cl_sidespeed->GetFloat( );

	g_cl.m_strafe_angles.y = math::NormalizedAngle( g_cl.m_strafe_angles.y - delta );
}

void Movement::DoPrespeed( ) {
	float   mod, min, max, step, strafe, time, angle;
	vec3_t  plane;

	// min and max values are based on 128 ticks.
	mod = g_csgo.m_globals->m_interval * 128.f;

	// scale min and max based on tickrate.
	min = 2.25f * mod;
	max = 5.f * mod;

	// compute ideal strafe angle for moving in a circle.
	strafe = m_ideal * 2.f;

	// clamp ideal strafe circle value to min and max step.
	math::clamp( strafe, min, max );

	// calculate time.
	time = 320.f / m_speed;

	// clamp time.
	math::clamp( time, 0.35f, 1.f );

	// init step.
	step = strafe;

	while( true ) {
		// if we will not collide with an object or we wont accelerate from such a big step anymore then stop.
		if( !WillCollide( time, step ) || max <= step )
			break;

		// if we will collide with an object with the current strafe step then increment step to prevent a collision.
		step += 0.2f;
	}

	if( step > max ) {
		// reset step.
		step = strafe;

		while( true ) {
			// if we will not collide with an object or we wont accelerate from such a big step anymore then stop.
			if( !WillCollide( time, step ) || step <= -min )
				break;

			// if we will collide with an object with the current strafe step decrement step to prevent a collision.
			step -= 0.2f;
		}

		if( step < -min ) {
			if( GetClosestPlane( plane ) ) {
				// grab the closest object normal
				// compute the angle of the normal
				// and push us away from the object.
				angle = math::rad_to_deg( std::atan2( plane.y, plane.x ) );
				step = -math::NormalizedAngle( m_circle_yaw - angle ) * 0.1f;
			}
		}

		else
			step -= 0.2f;
	}

	else
		step += 0.2f;

	// add the computed step to the steps of the previous circle iterations.
	m_circle_yaw = math::NormalizedAngle( m_circle_yaw + step );

	// apply data to usercmd.
	g_cl.m_cmd->m_view_angles.y = m_circle_yaw;
	g_cl.m_cmd->m_side_move = ( step >= 0.f ) ? -450.f : 450.f;
}

bool Movement::GetClosestPlane( vec3_t& plane ) {
	CGameTrace            trace;
	CTraceFilterWorldOnly filter;
	vec3_t                start{ m_origin };
	float                 smallest{ 1.f };
	const float		      dist{ 75.f };

	// trace around us in a circle
	for( float step{ }; step <= math::pi_2; step += ( math::pi / 10.f ) ) {
		// extend endpoint x units.
		vec3_t end = start;
		end.x += std::cos( step ) * dist;
		end.y += std::sin( step ) * dist;

		g_csgo.m_engine_trace->TraceRay( Ray( start, end, m_mins, m_maxs ), CONTENTS_SOLID, &filter, &trace );

		// we found an object closer, then the previouly found object.
		if( trace.m_fraction < smallest ) {
			// save the normal of the object.
			plane = trace.m_plane.m_normal;
			smallest = trace.m_fraction;
		}
	}

	// did we find any valid object?
	return smallest != 1.f && plane.z < 0.1f;
}

bool Movement::WillCollide( float time, float change ) {
	struct PredictionData_t {
		vec3_t start;
		vec3_t end;
		vec3_t velocity;
		float  direction;
		bool   ground;
		float  predicted;
	};

	PredictionData_t      data;
	CGameTrace            trace;
	CTraceFilterWorldOnly filter;

	// set base data.
	data.ground = g_cl.m_flags & FL_ONGROUND;
	data.start = m_origin;
	data.end = m_origin;
	data.velocity = g_cl.m_local->m_vecVelocity( );
	data.direction = math::rad_to_deg( std::atan2( data.velocity.y, data.velocity.x ) );

	for( data.predicted = 0.f; data.predicted < time; data.predicted += g_csgo.m_globals->m_interval ) {
		// predict movement direction by adding the direction change.
		// make sure to normalize it, in case we go over the -180/180 turning point.
		data.direction = math::NormalizedAngle( data.direction + change );

		// pythagoras.
		float hyp = data.velocity.length_2d( );

		// adjust velocity for new direction.
		data.velocity.x = std::cos( math::deg_to_rad( data.direction ) ) * hyp;
		data.velocity.y = std::sin( math::deg_to_rad( data.direction ) ) * hyp;

		// assume we bhop, set upwards impulse.
		if( data.ground )
			data.velocity.z = g_csgo.sv_jump_impulse->GetFloat( );

		else
			data.velocity.z -= g_csgo.sv_gravity->GetFloat( ) * g_csgo.m_globals->m_interval;

		// we adjusted the velocity for our new direction.
		// see if we can move in this direction, predict our new origin if we were to travel at this velocity.
		data.end += ( data.velocity * g_csgo.m_globals->m_interval );

		// trace
		g_csgo.m_engine_trace->TraceRay( Ray( data.start, data.end, m_mins, m_maxs ), MASK_PLAYERSOLID, &filter, &trace );

		// check if we hit any objects.
		if( trace.m_fraction != 1.f && trace.m_plane.m_normal.z <= 0.9f )
			return true;
		if( trace.m_startsolid || trace.m_allsolid )
			return true;

		// adjust start and end point.
		data.start = data.end = trace.m_endpos;

		// move endpoint 2 units down, and re-trace.
		// do this to check if we are on th floor.
		g_csgo.m_engine_trace->TraceRay( Ray( data.start, data.end - vec3_t{ 0.f, 0.f, 2.f }, m_mins, m_maxs ), MASK_PLAYERSOLID, &filter, &trace );

		// see if we moved the player into the ground for the next iteration.
		data.ground = trace.hit( ) && trace.m_plane.m_normal.z > 0.7f;
	}

	// the entire loop has ran
	// we did not hit shit.
	return false;
}

void MoonWalk( CUserCmd* cmd ) {
	if( g_cl.m_local->m_MoveType( ) == MOVETYPE_NOCLIP || g_cl.m_local->m_MoveType( ) == MOVETYPE_LADDER )
		return;

	if( cmd->m_side_move < 0.f ) {
		cmd->m_buttons |= IN_MOVERIGHT;
		cmd->m_buttons &= ~IN_MOVELEFT;
	}

	if( cmd->m_side_move > 0.f ) {
		cmd->m_buttons |= IN_MOVELEFT;
		cmd->m_buttons &= ~IN_MOVERIGHT;
	}

	if( cmd->m_forward_move > 0.f ) {
		cmd->m_buttons |= IN_BACK;
		cmd->m_buttons &= ~IN_FORWARD;
	}

	if( cmd->m_forward_move < 0.f ) {
		cmd->m_buttons |= IN_FORWARD;
		cmd->m_buttons &= ~IN_BACK;
	}
}

void Movement::FixMove( CUserCmd* cmd, const ang_t& wish_angles ) {
	vec3_t  move, dir;
	float   delta, len;
	ang_t   move_angle;

	// roll nospread fix.
	if( !( g_cl.m_flags & FL_ONGROUND ) && cmd->m_view_angles.z != 0.f )
		cmd->m_side_move = 0.f;

	// convert movement to vector.
	move = { cmd->m_forward_move, cmd->m_side_move, 0.f };

	// get move length and ensure we're using a unit vector ( vector with length of 1 ).
	len = move.normalize( );
	if( !len )
		return;

	// convert move to an angle.
	math::VectorAngles( move, move_angle );

	// calculate yaw delta.
	delta = ( cmd->m_view_angles.y - wish_angles.y );

	// accumulate yaw delta.
	move_angle.y += delta;

	// calculate our new move direction.
	// dir = move_angle_forward * move_length
	math::AngleVectors( move_angle, &dir );

	// scale to og movement.
	dir *= len;

	// strip old flags.
	g_cl.m_cmd->m_buttons &= ~( IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT );

	// fix ladder and noclip.
	if( g_cl.m_local->m_MoveType( ) == MOVETYPE_LADDER ) {
		// invert directon for up and down.
		if( cmd->m_view_angles.x >= 45.f && wish_angles.x < 45.f && std::abs( delta ) <= 65.f )
			dir.x = -dir.x;

		// write to movement.
		cmd->m_forward_move = dir.x;
		cmd->m_side_move = dir.y;

		// set new button flags.
		if( cmd->m_forward_move > 200.f )
			cmd->m_buttons |= IN_FORWARD;

		else if( cmd->m_forward_move < -200.f )
			cmd->m_buttons |= IN_BACK;

		if( cmd->m_side_move > 200.f )
			cmd->m_buttons |= IN_MOVERIGHT;

		else if( cmd->m_side_move < -200.f )
			cmd->m_buttons |= IN_MOVELEFT;
	}

	// we are moving normally.
	else {
		// we must do this for pitch angles that are out of bounds.
		if( cmd->m_view_angles.x < -90.f || cmd->m_view_angles.x > 90.f )
			dir.x = -dir.x;

		// set move.
		cmd->m_forward_move = dir.x;
		cmd->m_side_move = dir.y;

		// set new button flags.
		if( cmd->m_forward_move > 0.f )
			cmd->m_buttons |= IN_FORWARD;

		else if( cmd->m_forward_move < 0.f )
			cmd->m_buttons |= IN_BACK;

		if( cmd->m_side_move > 0.f )
			cmd->m_buttons |= IN_MOVERIGHT;

		else if( cmd->m_side_move < 0.f )
			cmd->m_buttons |= IN_MOVELEFT;
	}

	if( g_hooks.b[ XOR( "sildewalk" ) ] )
		MoonWalk( cmd );
}

void Movement::AutoPeek( CUserCmd* cmd, float wish_yaw ) {
	if( g_hooks.b[ XOR( "auto_peek" ) ] && g_hooks.auto_check( XOR( "auto_peek_key" ) ) ) {
		if( start_position.IsZero( ) ) {
			start_position = g_cl.m_local->GetAbsOrigin( );

			if( !( g_cl.m_flags & FL_ONGROUND ) ) {
				CTraceFilterWorldOnly filter;
				CGameTrace trace;

				g_csgo.m_engine_trace->TraceRay( Ray( start_position, start_position - vec3_t( 0.0f, 0.0f, 1000.0f ) ), MASK_SOLID, &filter, &trace );

				if( trace.m_fraction < 1.0f )
					start_position = trace.m_endpos + vec3_t( 0.0f, 0.0f, 2.0f );
			}
		}
		else {
			bool revolver_shoot = g_cl.m_weapon_id == REVOLVER && !g_cl.m_revolver_fire && ( cmd->m_buttons & IN_ATTACK || cmd->m_buttons & IN_ATTACK2 );

			if( g_cl.m_old_shot )
				fired_shot = true;

			if( fired_shot ) {
				vec3_t current_position = g_cl.m_local->GetAbsOrigin( );
				vec3_t difference = current_position - start_position;

				if( difference.length_2d( ) > 5.0f ) {
					vec3_t velocity = vec3_t( difference.x * cos( wish_yaw / 180.0f * math::pi ) + difference.y * sin( wish_yaw / 180.0f * math::pi ), difference.y * cos( wish_yaw / 180.0f * math::pi ) - difference.x * sin( wish_yaw / 180.0f * math::pi ), difference.z );

					if( difference.length_2d( ) < 50.0f ) {
						cmd->m_forward_move = -velocity.x * 20.0f;
						cmd->m_side_move = velocity.y * 20.0f;
					}
					else if( difference.length_2d( ) < 100.0f ) {
						cmd->m_forward_move = -velocity.x * 10.0f;
						cmd->m_side_move = velocity.y * 10.0f;
					}
					else if( difference.length_2d( ) < 150.0f ) {
						cmd->m_forward_move = -velocity.x * 5.0f;
						cmd->m_side_move = velocity.y * 5.0f;
					}
					else if( difference.length_2d( ) < 250.0f ) {
						cmd->m_forward_move = -velocity.x * 2.0f;
						cmd->m_side_move = velocity.y * 2.0f;
					}
					else {
						cmd->m_forward_move = -velocity.x * 1.0f;
						cmd->m_side_move = velocity.y * 1.0f;
					}
				}
				else {
					fired_shot = false;
					start_position.clear( );
				}
			}
		}
	}
	else {
		fired_shot = false;
		start_position.clear( );
	}
}

void Movement::EdgeJump( int pre_flags, int post_flags ) {
	if( !g_cl.m_processing )
		return;

	if( !g_hooks.b[ XOR( "edgejump" ) ] )
		return;

	if( !g_hooks.is_key_down( g_hooks.i[ XOR( "edgejump_key" ) ] ) )
		return;

	if( pre_flags & FL_ONGROUND ) {
		if( !( post_flags & FL_ONGROUND ) ) {
			g_cl.m_cmd->m_buttons |= IN_JUMP;
		}
	}
}

void Movement::FastStop( ) {
	if( !g_cl.m_pressing_move && g_hooks.b[ XOR( "faststop" ) ] && g_cl.m_speed > 15.f ) {
		if( !g_cl.m_cmd || !g_cl.m_local || !g_cl.m_local->alive( ) )
			return;

		auto weapon = g_cl.m_local->GetActiveWeapon( );

		// don't fake movement while noclipping or on ladders..
		if( !weapon || !weapon->GetWpnData( ) || g_cl.m_local->m_MoveType( ) == MOVETYPE_NOCLIP || g_cl.m_local->m_MoveType( ) == MOVETYPE_LADDER )
			return;

		if( !( g_cl.m_local->m_fFlags( ) & FL_ONGROUND ) )
			return;

		if( g_cl.m_cmd->m_buttons & IN_JUMP )
			return;

		auto move_speed = sqrtf( ( g_cl.m_cmd->m_forward_move * g_cl.m_cmd->m_forward_move ) + ( g_cl.m_cmd->m_side_move * g_cl.m_cmd->m_side_move ) );
		auto pre_prediction_velocity = g_cl.m_local->m_vecVelocity( ).length_2d( );

		auto v58 = g_csgo.sv_stopspeed->GetFloat( );
		v58 = fmaxf( v58, pre_prediction_velocity );
		v58 = g_csgo.sv_friction->GetFloat( ) * v58;
		auto slow_walked_speed = fmaxf( pre_prediction_velocity - ( v58 * g_csgo.m_globals->m_interval ), 0.0f );

		if( slow_walked_speed <= 0 || pre_prediction_velocity <= slow_walked_speed ) {
			g_cl.m_cmd->m_forward_move = 0;
			g_cl.m_cmd->m_side_move = 0;
			return;
		}

		ang_t angle;
		math::VectorAngles( g_cl.m_local->m_vecVelocity( ), angle );

		// get our current speed of travel.
		float speed = g_cl.m_local->m_vecVelocity( ).length( );

		// fix direction by factoring in where we are looking.
		angle.y = g_cl.m_view_angles.y - angle.y;

		// convert corrected angle back to a direction.
		vec3_t direction;
		math::AngleVectors( angle, &direction );

		vec3_t stop = direction * -speed;

		g_cl.m_cmd->m_forward_move = stop.x;
		g_cl.m_cmd->m_side_move = stop.y;
	}
}

/*void Movement::FakeWalk( ) {
	if( !g_hooks.b[ XOR( "slowMotion" ) ] )
		return;

	g_cl.m_fake_walk = false;

	const auto predict_velocity = [ ]( vec3_t* velocity ) {
		float speed = velocity->length( );
		if( speed >= 0.1f ) {
			float friction = g_csgo.sv_friction->GetFloat( );
			float stop_speed = std::max< float >( speed, g_csgo.sv_stopspeed->GetFloat( ) );
			float time = std::max< float >( g_csgo.m_globals->m_interval, g_csgo.m_globals->m_frametime );
			*velocity *= std::max< float >( 0.f, speed - friction * stop_speed * time / speed );
		}
	};

	vec3_t velocity{ g_cl.m_local->m_vecVelocity( ) };

	static bool was_moving_until_fakewalk = false;

	if( !g_hooks.is_key_down( g_hooks.i[ XOR( "slowMotion_key" ) ] ) ) {
		was_moving_until_fakewalk = velocity.length_2d( ) >= 0.1f;
		return;
	}

	if( !g_cl.m_local->GetGroundEntiy( ) )
		return;

	if( was_moving_until_fakewalk ) {
		if( velocity.length_2d( ) >= 0.1f ) {
			StopMovement( );
		}
		else
			was_moving_until_fakewalk = false;

		return;
	}

	g_cl.m_fake_walk = true;
	g_cl.m_cmd->m_buttons &= ~IN_SPEED;

	const float flServerTime = game::TICKS_TO_TIME( g_cl.m_local->m_nTickBase( ) );
	int nTicksToUpdate = game::TIME_TO_TICKS( g_cl.m_body_pred - flServerTime ) - 1;

	const vec3_t vecMove = { g_cl.m_cmd->m_forward_move, g_cl.m_cmd->m_side_move, g_cl.m_cmd->m_up_move };

	int nTicksToStop;
	for( nTicksToStop = 0; nTicksToStop < 15; ++nTicksToStop ) {
		if( velocity.length_2d( ) < 0.1f )
			break;

		predict_velocity( &velocity );
	}

	const int nMaxTicks = g_hooks.f[ XOR( "fakelag_limit" ) ];
	const int nTicksLeft = nMaxTicks - g_cl.m_lag;

	*g_cl.m_packet = !( g_cl.m_lag < nMaxTicks );

	if( nTicksToStop > nTicksLeft || !g_cl.m_lag || *g_cl.m_packet ) {
		StopMovement( );
	}
}*/

void Movement::FakeWalk( ) {
	if( !g_hooks.b[ XOR( "slowMotion" ) ] )
		return;

	vec3_t velocity{ g_cl.m_local->m_vecVelocity( ) };
	int    ticks{ };
	float  max{ ( g_hooks.f[ XOR( "slowMotion_speed" ) ] / 100.f ) * 16.f };

	if( !g_hooks.is_key_down( g_hooks.i[ XOR( "slowMotion_key" ) ] ) )
		return;

	if( !g_cl.m_local->GetGroundEntiy( ) )
		return;

	// reference:
	// https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/game/shared/gamemovement.cpp#L1612

	// calculate friction.
	float friction = g_csgo.sv_friction->GetFloat( ) * g_cl.m_local->m_surfaceFriction( );

	for( ; ticks < g_cl.m_max_lag; ++ticks ) {
		// calculate speed.
		float speed = velocity.length( );

		// if too slow return.
		if( speed <= 0.1f )
			break;

		// bleed off some speed, but if we have less than the bleed, threshold, bleed the threshold amount.
		float control = std::max( speed, g_csgo.sv_stopspeed->GetFloat( ) );

		// calculate the drop amount.
		float drop = control * friction * g_csgo.m_globals->m_interval;

		// scale the velocity.
		float newspeed = std::max( 0.f, speed - drop );

		if( newspeed != speed ) {
			// determine proportion of old speed we are using.
			newspeed /= speed;

			// adjust velocity according to proportion.
			velocity *= newspeed;
		}
	}

	// zero forwardmove and sidemove.
	if( ticks > ( ( max - 1 ) - g_csgo.m_cl->m_choked_commands ) || !g_csgo.m_cl->m_choked_commands ) {
		StopMovement( );
	}
}

void Movement::StopMovement( ) {
	if( !g_cl.m_cmd || !g_cl.m_local || !g_cl.m_local->alive( ) )
		return;

	auto weapon = g_cl.m_local->GetActiveWeapon( );

	// don't fake movement while noclipping or on ladders..
	if( !weapon || !weapon->GetWpnData( ) || g_cl.m_local->m_MoveType( ) == MOVETYPE_NOCLIP || g_cl.m_local->m_MoveType( ) == MOVETYPE_LADDER )
		return;

	if( !( g_cl.m_local->m_fFlags( ) & FL_ONGROUND ) )
		return;

	if( g_cl.m_cmd->m_buttons & IN_JUMP )
		return;

	auto move_speed = sqrtf( ( g_cl.m_cmd->m_forward_move * g_cl.m_cmd->m_forward_move ) + ( g_cl.m_cmd->m_side_move * g_cl.m_cmd->m_side_move ) );
	auto pre_prediction_velocity = g_cl.m_local->m_vecVelocity( ).length_2d( );

	auto v58 = g_csgo.sv_stopspeed->GetFloat( );
	v58 = fmaxf( v58, pre_prediction_velocity );
	v58 = g_csgo.sv_friction->GetFloat( ) * v58;
	auto slow_walked_speed = fmaxf( pre_prediction_velocity - ( v58 * g_csgo.m_globals->m_interval ), 0.0f );

	if( slow_walked_speed <= 0 || pre_prediction_velocity <= slow_walked_speed ) {
		g_cl.m_cmd->m_forward_move = 0;
		g_cl.m_cmd->m_side_move = 0;
		return;
	}

	ang_t angle;
	math::VectorAngles( g_cl.m_local->m_vecVelocity( ), angle );

	// get our current speed of travel.
	float speed = g_cl.m_local->m_vecVelocity( ).length( );

	// fix direction by factoring in where we are looking.
	angle.y = g_cl.m_view_angles.y - angle.y;

	// convert corrected angle back to a direction.
	vec3_t direction;
	math::AngleVectors( angle, &direction );

	vec3_t stop = direction * -speed;

	g_cl.m_cmd->m_forward_move = stop.x;
	g_cl.m_cmd->m_side_move = stop.y;
}

void Movement::QuickStop( ) {
	if( !g_cl.m_cmd || !g_cl.m_local || !g_cl.m_local->alive( ) )
		return;

	auto weapon = g_cl.m_local->GetActiveWeapon( );

	// don't fake movement while noclipping or on ladders..
	if( !weapon || g_cl.m_local->m_MoveType( ) == MOVETYPE_NOCLIP || g_cl.m_local->m_MoveType( ) == MOVETYPE_LADDER )
		return;

	if( !( g_cl.m_local->m_fFlags( ) & FL_ONGROUND ) )
		return;

	if( g_cl.m_cmd->m_buttons & IN_JUMP || !g_aimbot.m_stop )
		return;

	auto move_speed = sqrtf( ( g_cl.m_cmd->m_forward_move * g_cl.m_cmd->m_forward_move ) + ( g_cl.m_cmd->m_side_move * g_cl.m_cmd->m_side_move ) );
	auto pre_prediction_velocity = g_inputpred.velocity.length_2d( );

	auto v58 = g_csgo.sv_stopspeed->GetFloat( );
	v58 = fmaxf( v58, pre_prediction_velocity );
	v58 = g_csgo.sv_friction->GetFloat( ) * v58;
	auto slow_walked_speed = fmaxf( pre_prediction_velocity - ( v58 * g_csgo.m_globals->m_interval ), 0.0f );

	g_aimbot.m_stop = false;

	if( slow_walked_speed <= 0 || pre_prediction_velocity <= slow_walked_speed ) {
		g_cl.m_cmd->m_forward_move = 0;
		g_cl.m_cmd->m_side_move = 0;
		return;
	}

	ang_t angle;
	math::VectorAngles( g_cl.m_local->m_vecVelocity( ), angle );

	// get our current speed of travel.
	float speed = g_cl.m_local->m_vecVelocity( ).length( );

	// fix direction by factoring in where we are looking.
	angle.y = g_cl.m_view_angles.y - angle.y;

	// convert corrected angle back to a direction.
	vec3_t direction;
	math::AngleVectors( angle, &direction );

	vec3_t stop = direction * -speed;

	g_cl.m_cmd->m_forward_move = stop.x;
	g_cl.m_cmd->m_side_move = stop.y;
}

void ClampMovementSpeed( float speed ) {
	float final_speed = speed;

	if( !g_cl.m_cmd || !g_cl.m_processing )
		return;

	g_cl.m_cmd->m_buttons |= IN_SPEED;

	float squirt = std::sqrtf( ( g_cl.m_cmd->m_forward_move * g_cl.m_cmd->m_forward_move ) + ( g_cl.m_cmd->m_side_move * g_cl.m_cmd->m_side_move ) );

	if( squirt > speed ) {
		float squirt2 = std::sqrtf( ( g_cl.m_cmd->m_forward_move * g_cl.m_cmd->m_forward_move ) + ( g_cl.m_cmd->m_side_move * g_cl.m_cmd->m_side_move ) );

		float cock1 = g_cl.m_cmd->m_forward_move / squirt2;
		float cock2 = g_cl.m_cmd->m_side_move / squirt2;

		auto Velocity = g_cl.m_local->m_vecVelocity( ).length_2d( );

		if( final_speed + 1.0 <= Velocity ) {
			g_cl.m_cmd->m_forward_move = 0;
			g_cl.m_cmd->m_side_move = 0;
		}
		else {
			g_cl.m_cmd->m_forward_move = cock1 * final_speed;
			g_cl.m_cmd->m_side_move = cock2 * final_speed;
		}
	}
}

void Movement::AutoStop( ) {
	if( !g_cl.m_cmd || !g_cl.m_local || !g_cl.m_local->alive( ) )
		return;

	// don't fake movement while noclipping or on ladders..
	if( !g_cl.m_weapon || g_cl.m_local->m_MoveType( ) == MOVETYPE_NOCLIP || g_cl.m_local->m_MoveType( ) == MOVETYPE_LADDER )
		return;

	if( !( g_cl.m_local->m_fFlags( ) & FL_ONGROUND ) )
		return;

	if( g_cl.m_cmd->m_buttons & IN_JUMP || !g_aimbot.m_stop )
		return;

	if( !g_cl.m_weapon_info )
		return;

	auto max_speed = 0.33f * ( g_cl.m_local->m_bIsScoped( ) > 0 ? g_cl.m_weapon_info->m_max_player_speed_alt : g_cl.m_weapon_info->m_max_player_speed );

	g_aimbot.m_stop = false;

	if( g_cl.m_ground && !g_hooks.is_key_down( g_hooks.i[ XOR( "slowMotion_key" ) ] ) ) {
		if( g_cl.m_local->m_vecVelocity( ).length_2d( ) < max_speed ) {
			// get the max possible speed whilest we are still accurate.
			float flMaxSpeed = g_cl.m_local->m_bIsScoped( ) > 0 ? g_cl.m_weapon_info->m_max_player_speed_alt : g_cl.m_weapon_info->m_max_player_speed;
			float flDesiredSpeed = ( flMaxSpeed * 0.33000001 );

			ClampMovementSpeed( flDesiredSpeed );
		}
		else {
			vec3_t Velocity = g_cl.m_local->m_vecVelocity( );

			ang_t direction;
			ang_t real_view = g_cl.m_cmd->m_view_angles;

			math::VectorAngles( Velocity, direction );
			g_csgo.m_engine->GetViewAngles( real_view );

			direction.y = real_view.y - direction.y;

			vec3_t forward;
			math::AngleVectors( direction, &forward );

			static auto cl_forwardspeed = g_csgo.m_cvar->FindVar( HASH( "cl_forwardspeed" ) );
			static auto cl_sidespeed = g_csgo.m_cvar->FindVar( HASH( "cl_sidespeed" ) );

			auto negative_forward_speed = -cl_forwardspeed->GetFloat( );
			auto negative_side_speed = -cl_sidespeed->GetFloat( );

			auto negative_forward_direction = forward * negative_forward_speed;
			auto negative_side_direction = forward * negative_side_speed;

			g_cl.m_cmd->m_forward_move = negative_forward_direction.x;
			g_cl.m_cmd->m_side_move = negative_side_direction.y;
		}
	}
}