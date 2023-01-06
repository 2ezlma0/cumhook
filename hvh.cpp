#include "includes.h"

HVH g_hvh{ };;

void HVH::UpdateHotkeys( Stage_t stage ) {
	if( stage != FRAME_RENDER_START || !g_cl.m_processing )
		return;

	//if( !vars::aa.manual.get<bool>( ) ) {
	//	direction = -1;
	//	return;
	//}

	auto current = -1;

	if( GetAsyncKeyState( g_hooks.i[ XOR( "left" ) ] ) )
		current = 1;
	else if( GetAsyncKeyState( g_hooks.i[ XOR( "right" ) ] ) )
		current = 2;
	else if( GetAsyncKeyState( g_hooks.i[ XOR( "back" ) ] ) )
		current = 0;

	static bool prev_state;
	const auto state = current >= 0;
	if( prev_state != state ) {
		if( state ) {
			if( current == direction )
				direction = -1;
			else
				direction = current;
		}

		prev_state = state;
	}
}

void HVH::IdealPitch( ) {
	CCSGOPlayerAnimState* state = g_cl.m_local->m_PlayerAnimState( );
	if( !state )
		return;

	g_cl.m_cmd->m_view_angles.x = state->m_aim_pitch_max;
}

void HVH::AntiAimPitch( ) {
	bool safe = g_hooks.b[ XOR( "antiUntrusted" ) ];

	switch( m_pitch ) {
		case 1:
			// down.
			g_cl.m_cmd->m_view_angles.x = 89.f;
			break;

		case 2:
			// up.
			g_cl.m_cmd->m_view_angles.x = safe ? -89.f : -540.f;
			break;

		case 3:
			// down.
			g_cl.m_cmd->m_view_angles.x = safe ? 89.f : 540.f;
			break;

		case 4:
			// ideal.
			IdealPitch( );
			break;

		case 5:
			// random.
			g_cl.m_cmd->m_view_angles.x = g_csgo.RandomFloat( safe ? -89.f : -540, safe ? 89.f : 1080.f );
			break;

		default:
			break;
	}
}

void HVH::AutoDirection( ) {
	// constants.
	constexpr float STEP{ 4.f };
	constexpr float RANGE{ 36.f };
	float angel = -g_hooks.f[ XOR( "freestand_add" ) ];

	// best target.
	struct AutoTarget_t { float fov; Player* player; };
	AutoTarget_t target{ 180.f + 1.f, nullptr };

	// iterate players.
	for( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i ) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );

		// validate player.
		if( !g_aimbot.IsValidTarget( player ) )
			continue;

		// skip dormant players.
		if( player->dormant( ) )
			continue;

		// get best target based on fov.
		float fov = math::GetFOV( g_cl.m_view_angles, g_cl.m_shoot_pos, player->WorldSpaceCenter( ) );

		if( fov < target.fov ) {
			target.fov = fov;
			target.player = player;
		}
	}

	if( !target.player ) {
		// we have a timeout.
		if( m_auto_last > 0.f && m_auto_time > 0.f && g_csgo.m_globals->m_curtime < ( m_auto_last + m_auto_time ) )
			return;

		// set angle to backwards.
		m_auto = math::NormalizedAngle( m_view - 180.f );
		m_auto_dist = -1.f;
		return;
	}

	// construct vector of angles to test.
	std::vector< AdaptiveAngle > angles{ };
	angles.emplace_back( m_view - 180.f );
	angles.emplace_back( m_view + angel );
	angles.emplace_back( m_view - angel );

	// start the trace at the enemy shoot pos.
	vec3_t start = target.player->GetShootPorsition( );

	// see if we got any valid result.
	// if this is false the path was not obstructed with anything.
	bool valid{ false };

	// iterate vector of angles.
	for( auto it = angles.begin( ); it != angles.end( ); ++it ) {

		// compute the 'rough' estimation of where our head will be.
		vec3_t end{ g_cl.m_shoot_pos.x + std::cos( math::deg_to_rad( it->m_yaw ) ) * RANGE,
			g_cl.m_shoot_pos.y + std::sin( math::deg_to_rad( it->m_yaw ) ) * RANGE,
			g_cl.m_shoot_pos.z };

		// draw a line for debugging purposes.
		{
			if( g_hooks.b[ XOR( "freestand_debug" ) ] )
				g_csgo.m_debug_overlay->AddLineOverlay( start, end, 255, 0, 0, true, 0.1f );
		}

		// compute the direction.
		vec3_t dir = end - start;
		float len = dir.normalize( );

		// should never happen.
		if( len <= 0.f )
			continue;

		// step thru the total distance, 4 units per step.
		for( float i{ 0.f }; i < len; i += STEP ) {
			// get the current step position.
			vec3_t point = start + ( dir * i );

			// get the contents at this point.
			int contents = g_csgo.m_engine_trace->GetPointContents( point, MASK_SHOT );

			// contains nothing that can stop a bullet.
			if( !( contents & MASK_SHOT ) )
				continue;

			float mult = 1.f;

			// over 50% of the total length, prioritize this shit.
			if( i > ( len * 0.5f ) )
				mult = 1.25f;

			// over 90% of the total length, prioritize this shit.
			if( i > ( len * 0.75f ) )
				mult = 1.25f;

			// over 90% of the total length, prioritize this shit.
			if( i > ( len * 0.9f ) )
				mult = 2.f;

			// append 'penetrated distance'.
			it->m_dist += ( STEP * mult );

			// mark that we found anything.
			valid = true;
		}
	}

	if( !valid ) {
		// set angle to backwards.
		m_auto = math::NormalizedAngle( m_view - 180.f );
		m_auto_dist = -1.f;
		return;
	}

	// put the most distance at the front of the container.
	std::sort( angles.begin( ), angles.end( ),
			   [ ]( const AdaptiveAngle& a, const AdaptiveAngle& b ) {
		return a.m_dist > b.m_dist;
	} );

	// the best angle should be at the front now.
	AdaptiveAngle* best = &angles.front( );

	// check if we are not doing a useless change.
	if( best->m_dist != m_auto_dist ) {
		// set yaw to the best result.
		m_auto = math::NormalizedAngle( best->m_yaw );
		m_auto_dist = best->m_dist;
		m_auto_last = g_csgo.m_globals->m_curtime;
	}
}

void HVH::GetAntiAimDirection( ) {
	// edge aa.
	if( g_hooks.m[ XOR( "freestand" ) ][ 2 ] && g_cl.m_local->m_vecVelocity( ).length_2d( ) < 320.f && direction == -1 ) {

		ang_t ang;
		if( DoEdgeAntiAim( g_cl.m_local, ang ) ) {
			m_direction = ang.y;
			return;
		}
	}

	if (g_hooks.m[XOR("freestand")][3] && g_cl.m_local->m_vecVelocity().length_2d() < 245.f && direction == -1)
	{
		m_direction = g_csgo.RandomFloat(10.f, 30.f);
		float rand69 = rand() % 10 + 16;
		g_cl.m_cmd->m_view_angles.y = sin(g_csgo.m_globals->m_curtime * g_cl.m_cmd->m_view_angles.y) + 2 / g_cl.m_tick;
		if (g_cl.m_tick < 8) {
			m_direction += 62.f;
		}
		g_cl.m_cmd->m_view_angles.y += 80.f;
		if (g_cl.m_tick > 9 && g_cl.m_tick < rand69) {
			m_direction -= 77.f;
		}
		g_cl.m_cmd->m_view_angles.y -= 87.f;
		if (g_cl.m_tick > rand69) {
			m_direction += 55.f;
		}
		if (g_cl.m_tick < 15) {
			m_direction -= 10.f;
		}
		else if (g_cl.m_tick == 22) {
			m_direction += 19.f;
		}
		else if (g_cl.m_tick == 3) {
			m_direction -= 14.f;
		}
		else if (g_cl.m_tick == 69) {
			m_direction -= 77.f;
		}
		else {
			m_direction -= rand() % 100;
		}
	}

	// lock while standing..
	bool lock = g_hooks.b[ XOR( "dir_lock" ) ];

	// save view, depending if locked or not.
	if( ( lock && g_cl.m_speed > 0.1f ) || !lock )
		m_view = g_cl.m_cmd->m_view_angles.y;

	if( true ) {
		// 'static'.
		if( m_base_angle == 0 )
			m_view = 0.f;

		// away options.
		else {
			float  best_fov{ std::numeric_limits< float >::max( ) };
			float  best_dist{ std::numeric_limits< float >::max( ) };
			float  fov, dist;
			Player* target, * best_target{ nullptr };

			for( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i ) {
				target = g_csgo.m_entlist->GetClientEntity< Player* >( i );

				if( !g_aimbot.IsValidTarget( target ) )
					continue;

				if( target->dormant( ) )
					continue;

				// 'away crosshair'.
				if( m_base_angle == 2 ) {

					// check if a player was closer to our crosshair.
					fov = math::GetFOV( g_cl.m_view_angles, g_cl.m_shoot_pos, target->WorldSpaceCenter( ) );
					if( fov < best_fov ) {
						best_fov = fov;
						best_target = target;
					}
				}

				//// 'away distance'.
				//else if( m_base_angle == 3 ) {
				//
				//	// check if a player was closer to us.
				//	dist = ( target->m_vecOrigin( ) - g_cl.m_local->m_vecOrigin( ) ).length_sqr( );
				//	if( dist < best_dist ) {
				//		best_dist = dist;
				//		best_target = target;
				//	}
				//}
			}

			if( best_target ) {
				// todo - dex; calculate only the yaw needed for this (if we're not going to use the x component that is).
				ang_t angle;
				math::VectorAngles( best_target->m_vecOrigin( ) - g_cl.m_local->m_vecOrigin( ), angle );
				m_view = angle.y;
			}
		}
	}

	if( ( g_cl.m_ground && g_cl.m_speed <= 0.1f && g_hooks.m[ XOR( "freestand" ) ][ 0 ] ) || ( g_cl.m_ground && g_cl.m_speed > 0.1f && g_hooks.m[ XOR( "freestand" ) ][ 1 ] ) ) {
		AutoDirection( );
		m_direction = m_auto;

		if( g_hooks.b[ XOR( "manual_aa" ) ] ) {
			switch( direction ) {
				case 0:
					m_direction = m_view + 180.f;
					break;
				case 1:
					m_direction = m_view + 90.f;
					break;
				case 2:
					m_direction = m_view - 90.f;
					break;
			}
		}
	}
	else {
		m_direction = m_view + 180.f;

		if( g_hooks.b[ XOR( "manual_aa" ) ] ) {
			switch( direction ) {
				case 0:
					m_direction = m_view + 180.f;
					break;
				case 1:
					m_direction = m_view + 90.f;
					break;
				case 2:
					m_direction = m_view - 90.f;
					break;
			}
		}
	}

	// normalize the direction.
	math::NormalizeAngle( m_direction );
}

bool HVH::DoEdgeAntiAim( Player* player, ang_t& out ) {
	CGameTrace trace;
	static CTraceFilterSimple_game filter{ };

	if( player->m_MoveType( ) == MOVETYPE_LADDER )
		return false;

	// skip this player in our traces.
	filter.SetPassEntity( player );

	// get player bounds.
	vec3_t mins = player->m_vecMins( );
	vec3_t maxs = player->m_vecMaxs( );

	// make player bounds bigger.
	mins.x -= 20.f;
	mins.y -= 20.f;
	maxs.x += 20.f;
	maxs.y += 20.f;

	// get player origin.
	vec3_t start = player->GetAbsOrigin( );

	// offset the view.
	start.z += 56.f;

	g_csgo.m_engine_trace->TraceRay( Ray( start, start, mins, maxs ), CONTENTS_SOLID, ( ITraceFilter* )&filter, &trace );
	if( !trace.m_startsolid )
		return false;

	float  smallest = 1.f;
	vec3_t plane;

	// trace around us in a circle, in 20 steps (anti-degree conversion).
	// find the closest object.
	for( float step{ }; step <= math::pi_2; step += ( math::pi / 10.f ) ) {
		// extend endpoint x units.
		vec3_t end = start;

		// set end point based on range and step.
		end.x += std::cos( step ) * 32.f;
		end.y += std::sin( step ) * 32.f;

		g_csgo.m_engine_trace->TraceRay( Ray( start, end, { -1.f, -1.f, -8.f }, { 1.f, 1.f, 8.f } ), CONTENTS_SOLID, ( ITraceFilter* )&filter, &trace );

		// we found an object closer, then the previouly found object.
		if( trace.m_fraction < smallest ) {
			// save the normal of the object.
			plane = trace.m_plane.m_normal;
			smallest = trace.m_fraction;
		}
	}

	// no valid object was found.
	if( smallest == 1.f || plane.z >= 0.1f )
		return false;

	// invert the normal of this object
	// this will give us the direction/angle to this object.
	vec3_t inv = -plane;
	vec3_t dir = inv;
	dir.normalize( );

	// extend point into object by 24 units.
	vec3_t point = start;
	point.x += ( dir.x * 24.f );
	point.y += ( dir.y * 24.f );

	// check if we can stick our head into the wall.
	if( g_csgo.m_engine_trace->GetPointContents( point, CONTENTS_SOLID ) & CONTENTS_SOLID ) {
		// trace from 72 units till 56 units to see if we are standing behind something.
		g_csgo.m_engine_trace->TraceRay( Ray( point + vec3_t{ 0.f, 0.f, 16.f }, point ), CONTENTS_SOLID, ( ITraceFilter* )&filter, &trace );

		// we didnt start in a solid, so we started in air.
		// and we are not in the ground.
		if( trace.m_fraction < 1.f && !trace.m_startsolid && trace.m_plane.m_normal.z > 0.7f ) {
			// mean we are standing behind a solid object.
			// set our angle to the inversed normal of this object.
			out.y = math::rad_to_deg( std::atan2( inv.y, inv.x ) );
			return true;
		}
	}

	// if we arrived here that mean we could not stick our head into the wall.
	// we can still see if we can stick our head behind/asides the wall.

	// adjust bounds for traces.
	mins = { ( dir.x * -3.f ) - 1.f, ( dir.y * -3.f ) - 1.f, -1.f };
	maxs = { ( dir.x * 3.f ) + 1.f, ( dir.y * 3.f ) + 1.f, 1.f };

	// move this point 48 units to the left 
	// relative to our wall/base point.
	vec3_t left = start;
	left.x = point.x - ( inv.y * 48.f );
	left.y = point.y - ( inv.x * -48.f );

	g_csgo.m_engine_trace->TraceRay( Ray( left, point, mins, maxs ), CONTENTS_SOLID, ( ITraceFilter* )&filter, &trace );
	float l = trace.m_startsolid ? 0.f : trace.m_fraction;

	// move this point 48 units to the right 
	// relative to our wall/base point.
	vec3_t right = start;
	right.x = point.x + ( inv.y * 48.f );
	right.y = point.y + ( inv.x * -48.f );

	g_csgo.m_engine_trace->TraceRay( Ray( right, point, mins, maxs ), CONTENTS_SOLID, ( ITraceFilter* )&filter, &trace );
	float r = trace.m_startsolid ? 0.f : trace.m_fraction;

	// both are solid, no edge.
	if( l == 0.f && r == 0.f )
		return false;

	// set out to inversed normal.
	out.y = math::rad_to_deg( std::atan2( inv.y, inv.x ) );

	// left started solid.
	// set angle to the left.
	if( l == 0.f ) {
		out.y += 90.f;
		return true;
	}

	// right started solid.
	// set angle to the right.
	if( r == 0.f ) {
		out.y -= 90.f;
		return true;
	}

	return false;
}

void HVH::DoRealAntiAim( ) {
	// if we have a yaw antaim.
	if( m_yaw > 0 ) {

		// if we have a yaw active, which is true if we arrived here.
		// set the yaw to the direction before applying any other operations.
		g_cl.m_cmd->m_view_angles.y = m_direction;

		bool stand = g_hooks.i[ XOR( "yaw_fakebody" ) ] > 0 && m_mode == AntiAimMode::STAND;
		bool air = g_menu.main.antiaim.body_fake_air.get( ) > 0 && m_mode == AntiAimMode::AIR;
		static int negative = false;

		// one tick before the update.
		if( stand && !g_cl.m_lag && g_csgo.m_globals->m_curtime >= ( g_cl.m_body_pred - g_cl.m_anim_frame ) && g_csgo.m_globals->m_curtime < g_cl.m_body_pred ) {
			// z mode.
			if( g_menu.main.antiaim.body_fake_stand.get( ) == 4 )
				g_cl.m_cmd->m_view_angles.y -= 90.f;
		}

		// check if we will have a lby fake this tick.
		if( !g_cl.m_lag && g_csgo.m_globals->m_curtime >= g_cl.m_body_pred && ( stand || air ) ) {
			// there will be an lbyt update on this tick.
			if( stand ) {
				switch( g_hooks.i[ XOR( "yaw_fakebody" ) ] ) {

					// left.
					case 1:
						if( g_hooks.b[ XOR( "body_fake_stand_fakewalk" ) ] && g_hooks.is_key_down( g_hooks.i[ XOR( "slowMotion_key" ) ] ) )
							break;

						g_cl.m_cmd->m_view_angles.y += 110.f;
						break;

						// right.
					case 2:
						if( g_hooks.b[ XOR( "body_fake_stand_fakewalk" ) ] && g_hooks.is_key_down( g_hooks.i[ XOR( "slowMotion_key" ) ] ) )
							break;

						g_cl.m_cmd->m_view_angles.y -= 110.f;
						break;

						// opposite.
					case 3:
						if( g_hooks.b[ XOR( "body_fake_stand_fakewalk" ) ] && g_hooks.is_key_down( g_hooks.i[ XOR( "slowMotion_key" ) ] ) )
							break;

						g_cl.m_cmd->m_view_angles.y += 180.f;
						break;

						// z.
					case 4:
						if( g_hooks.b[ XOR( "body_fake_stand_fakewalk" ) ] && g_hooks.is_key_down( g_hooks.i[ XOR( "slowMotion_key" ) ] ) )
							break;

						negative ? g_cl.m_cmd->m_view_angles.y += 110.f : g_cl.m_cmd->m_view_angles.y -= 110.f;
						negative = ~negative;
						break;

						// optimal.
					case 5:
						if( g_hooks.b[ XOR( "body_fake_stand_fakewalk" ) ] && g_hooks.is_key_down( g_hooks.i[ XOR( "slowMotion_key" ) ] ) )
							break;

						g_cl.m_cmd->m_view_angles.y = m_view + 180.f;
						break;

						// custom.
					case 6:
						if( g_hooks.b[ XOR( "body_fake_stand_fakewalk" ) ] && g_hooks.is_key_down( g_hooks.i[ XOR( "slowMotion_key" ) ] ) )
							break;

						g_cl.m_cmd->m_view_angles.y += g_hooks.f[ XOR( "fakebody_add" ) ];
						break;

						//cumhookbody
					case 7:
						//mmmmm cum
						m_sway++;
						g_cl.m_cmd->m_view_angles.y += cos(m_sway) * 30;
						if ((cos(m_sway) * 30) > 135)
						{
							g_cl.m_cmd->m_view_angles.y += m_sway;
							m_sway = 0;
						}

						if (g_cl.m_tick == 21)
							g_cl.m_cmd->m_view_angles.y -= 69.f;
						if (g_cl.m_tick == 37)
							g_cl.m_cmd->m_view_angles.y += 69.f;
						if (g_cl.m_tick == 63)
							g_cl.m_cmd->m_view_angles.y += m_sway * 2;
						
						break;
				}
			}

			else if( air ) {
				switch( g_menu.main.antiaim.body_fake_air.get( ) ) {

					// left.
					case 1:
						g_cl.m_cmd->m_view_angles.y += 90.f;
						break;

						// right.
					case 2:
						g_cl.m_cmd->m_view_angles.y -= 90.f;
						break;

						// opposite.
					case 3:
						g_cl.m_cmd->m_view_angles.y += 180.f;
						break;
				}
			}
		}

		// run normal aa code.
		else {
			switch( m_yaw ) {

				// direction.
				case 1:
				{
					// do nothing, yaw already is direction.

					// yaw add
					g_cl.m_cmd->m_view_angles.y += g_hooks.f[ XOR( "yaw_add" ) ];
					break;
				}

				// jitter.
				case 2:
				{

					// get the range from the menu.
					float range = m_jitter_range / 2.f;

					// set angle.
					g_cl.m_cmd->m_view_angles.y += g_csgo.RandomFloat( -range, range );
					break;
				}

				// rotate.
				case 3:
				{
					// set base angle.
					g_cl.m_cmd->m_view_angles.y = ( m_direction - m_rot_range / 2.f );

					// apply spin.
					g_cl.m_cmd->m_view_angles.y += std::fmod( g_csgo.m_globals->m_curtime * ( m_rot_speed * 20.f ), m_rot_range );

					break;
				}

				// random.
				case 4:
				{
					// check update time.
					if( g_csgo.m_globals->m_curtime >= m_next_random_update ) {

						// set new random angle.
						m_random_angle = g_csgo.RandomFloat( -180.f, 180.f );

						// set next update time
						m_next_random_update = g_csgo.m_globals->m_curtime + m_rand_update;
					}

					// apply angle.
					g_cl.m_cmd->m_view_angles.y = m_random_angle;
					break;
				}

				//sexy
				case 5:
					//mmm wavy sin sway
					m_sway++;
					g_cl.m_cmd->m_view_angles.y += sin(m_sway) * 20;
					if (m_sway % 10 == 0)
					{
						//random flick stuff for epic dump
						g_cl.m_cmd->m_view_angles.y += ((rand() % 100) - (rand() % 90)) + m_sway;
					}
					if ((sin(m_sway) * 30) > 105)
					{
						g_cl.m_cmd->m_view_angles.y += m_sway;
						m_sway = 0;
					}
					break;

				default:
					break;
			}

			// distortion.
			if( g_hooks.b[ XOR( "distortion" ) ] ) {
				bool active;
				if( g_hooks.m[ XOR( "distortion_triggers" ) ][ 0 ] && g_cl.m_speed < 0.1f && ( !( g_cl.m_buttons & IN_JUMP ) && ( g_cl.m_flags & FL_ONGROUND ) ) )
					active = true;
				else if( g_hooks.m[ XOR( "distortion_triggers" ) ][ 1 ] && g_cl.m_speed > 0.1f )
					active = true;
				else if( g_hooks.m[ XOR( "distortion_triggers" ) ][ 2 ] && ( ( g_cl.m_buttons & IN_JUMP ) || !( g_cl.m_flags & FL_ONGROUND ) ) )
					active = true;

				if( !g_hooks.m[ XOR( "distortion_triggers" ) ][ 3 ] && direction != -1 )
					active = false;

				if( active ) {
					float sine = ( ( sin( g_csgo.m_globals->m_curtime * ( g_hooks.f[ XOR( "distortion_speed" ) ] / 10.f ) ) + 1 ) / 2 ) * g_hooks.f[ XOR( "distortion_amount" ) ];

					g_cl.m_cmd->m_view_angles.y += sine - ( g_hooks.f[ XOR( "distortion_amount" ) ] / 2.f );
				}
			}
		}
	}

	// normalize angle.
	math::NormalizeAngle( g_cl.m_cmd->m_view_angles.y );
}

void HVH::DoFakeAntiAim( ) {
	// do fake yaw operations.

	// enforce this otherwise low fps dies.
	// cuz the engine chokes or w/e
	// the fake became the real, think this fixed it.
	*g_cl.m_packet = true;
	int rand26;

	switch( g_hooks.i[ XOR( "fake_yaw" ) ] ) {

		// default.
		case 1:
			// set base to opposite of direction.
			g_cl.m_cmd->m_view_angles.y = m_direction + 180.f;

			// apply 45 degree jitter.
			g_cl.m_cmd->m_view_angles.y += g_csgo.RandomFloat( -90.f, 90.f );
			break;

			// relative.
		case 2:
			// set base to opposite of direction.
			g_cl.m_cmd->m_view_angles.y = m_direction + 180.f;

			// apply offset correction.
			g_cl.m_cmd->m_view_angles.y += g_hooks.f[ XOR( "fakeyaw_add" ) ];
			break;

			// relative jitter.
		case 3:
		{
			// get fake jitter range from menu.
			float range = g_hooks.f[ XOR( "fakeyaw_add" ) ] / 2.f;

			// set base to opposite of direction.
			g_cl.m_cmd->m_view_angles.y = m_direction + 180.f;

			// apply jitter.
			g_cl.m_cmd->m_view_angles.y += g_csgo.RandomFloat( -range, range );
			break;
		}

		// rotate.
		case 4:
			g_cl.m_cmd->m_view_angles.y = m_direction + 90.f + std::fmod( g_csgo.m_globals->m_curtime * 360.f, 180.f );
			break;

			// random.
		case 5:
			g_cl.m_cmd->m_view_angles.y = g_csgo.RandomFloat( -180.f, 180.f );
			break;

			// local view.
		case 6:
			g_cl.m_cmd->m_view_angles.y = g_cl.m_view_angles.y + g_hooks.f[ XOR( "fakeyaw_add" ) ];
			break;

			// spin bot.
		case 7:
			g_cl.m_cmd->m_view_angles.y = m_direction + 90.f + std::fmod( g_csgo.m_globals->m_curtime * 360.f, 360.f );
			break;

			// reverse lby.
		case 8:
			g_cl.m_cmd->m_view_angles.y = g_cl.m_body + 180.f + g_hooks.f[ XOR( "fakeyaw_add" ) ];
			break;

			// match lby.
		case 9:
			g_cl.m_cmd->m_view_angles.y = g_cl.m_body + g_hooks.f[ XOR( "fakeyaw_add" ) ];
			break;

			//cumhookyaw (featuring sexy yandere dev type code)
		case 10:
			rand26 = rand() % 64;
			if (g_cl.m_tick < 10)
				g_cl.m_cmd->m_view_angles.y += 162.f;
			else if (m_sway == 5)
				g_cl.m_cmd->m_view_angles.y += 80.f;
			else if (m_sway == 20)
				g_cl.m_cmd->m_view_angles.y -= 87.f;
			else if (g_cl.m_tick == 10)
				g_cl.m_cmd->m_view_angles.y += 177.f;
			else if (g_cl.m_tick == rand26)
				g_cl.m_cmd->m_view_angles.y += 179.f;
			else if (g_cl.m_tick == 12)
				g_cl.m_cmd->m_view_angles.y += 170.f;
			else if (g_cl.m_tick == 21)
				g_cl.m_cmd->m_view_angles.y -= 179.f;
			else if (g_cl.m_tick == 37)
				g_cl.m_cmd->m_view_angles.y += 179.f;
			else if (g_cl.m_tick == 63)
				g_cl.m_cmd->m_view_angles.y += 177.f;
			else if (g_csgo.m_globals->m_curtime == 5)
				g_cl.m_cmd->m_view_angles.y += 177.f;
			else if (g_csgo.m_globals->m_curtime == 12)
				g_cl.m_cmd->m_view_angles.y -= 177.f;
			else if (g_csgo.m_globals->m_curtime == 3)
				g_cl.m_cmd->m_view_angles.y += 177.f;
			else if (g_csgo.m_globals->m_curtime == 9)
				g_cl.m_cmd->m_view_angles.y -= 177.f;
			else if (m_sway % 5 == 0)
				g_cl.m_cmd->m_view_angles.y += ((rand() % 180) - (rand() % -180)) + m_sway;
			else
				g_cl.m_cmd->m_view_angles.y = pow(m_sway, asin(m_sway/69));
			break;

		default:
			break;
	}

	// normalize fake angle.
	math::NormalizeAngle( g_cl.m_cmd->m_view_angles.y );
}

void HVH::AntiAim( ) {
	bool attack, attack2;

	//if( !g_hooks.b[XOR("enable_antiaim")] )
	//	return;

	attack = g_cl.m_cmd->m_buttons & IN_ATTACK;
	attack2 = g_cl.m_cmd->m_buttons & IN_ATTACK2;

	if( g_cl.IsFiring( g_csgo.m_globals->m_curtime ) )
		return;

	if( g_cl.m_weapon && g_cl.m_weapon_fire ) {
		bool knife = g_cl.m_weapon_type == WEAPONTYPE_KNIFE && g_cl.m_weapon_id != ZEUS;
		bool revolver = g_cl.m_weapon_id == REVOLVER;

		// if we are in attack and can fire, do not anti-aim.
		if( attack || ( attack2 && ( knife || revolver ) ) )
			return;
	}

	// disable conditions.
	if( g_csgo.m_gamerules->m_bFreezePeriod( ) || ( g_cl.m_flags & FL_FROZEN ) || ( g_cl.m_cmd->m_buttons & IN_USE ) )
		return;

	// grenade throwing
	// CBaseCSGrenade::ItemPostFrame()
	// https://github.com/VSES/SourceEngine2007/blob/master/src_main/game/shared/cstrike/weapon_basecsgrenade.cpp#L209
	if( g_cl.m_weapon_type == WEAPONTYPE_GRENADE
		&& ( !g_cl.m_weapon->m_bPinPulled( ) || attack || attack2 )
		&& g_cl.m_weapon->m_fThrowTime( ) > 0.f && g_cl.m_weapon->m_fThrowTime( ) <= g_csgo.m_globals->m_curtime )
		return;

	m_mode = AntiAimMode::STAND;

	if( ( g_cl.m_buttons & IN_JUMP ) || !( g_cl.m_flags & FL_ONGROUND ) )
		m_mode = AntiAimMode::AIR;

	else if( g_cl.m_speed > 0.1f )
		m_mode = AntiAimMode::WALK;

	// load settings.
	if( m_mode == AntiAimMode::STAND ) {
		m_pitch = g_hooks.i[ XOR( "pitch" ) ];
		m_yaw = g_hooks.i[ XOR( "yaw" ) ];
		m_jitter_range = g_hooks.f[ XOR( "yaw_add" ) ];
		m_rot_range = g_hooks.f[ XOR( "yaw_add" ) ];
		m_rot_speed = 10.f;
		m_rand_update = g_hooks.f[ XOR( "rand_speed" ) ];
		m_dir = g_hooks.m[ XOR( "freestand" ) ][ 0 ];
		m_dir_custom = g_menu.main.antiaim.dir_custom_stand.get( );
		m_base_angle = g_hooks.i[ XOR( "yaw_base" ) ];
		m_auto_time = 5.f;
	}

	else if( m_mode == AntiAimMode::WALK ) {
		m_pitch = g_hooks.i[ XOR( "pitch" ) ];
		m_yaw = g_hooks.i[ XOR( "yaw_move" ) ];
		m_jitter_range = g_hooks.f[ XOR( "yaw_move_add" ) ];
		m_rot_range = g_hooks.f[ XOR( "yaw_move_add" ) ];
		m_rot_speed = 10.f;
		m_rand_update = g_hooks.f[ XOR( "rand_speed" ) ];
		m_dir = g_hooks.m[ XOR( "freestand" ) ][ 1 ];
		m_dir_custom = g_menu.main.antiaim.dir_custom_stand.get( );
		m_base_angle = g_hooks.i[ XOR( "yaw_base" ) ];
		m_auto_time = 5.f;
	}

	else if( m_mode == AntiAimMode::AIR ) {
		m_pitch = g_hooks.i[ XOR( "pitch" ) ];
		m_yaw = g_hooks.i[ XOR( "yaw" ) ];
		m_jitter_range = g_hooks.f[ XOR( "yaw_add" ) ];
		m_rot_range = g_hooks.f[ XOR( "yaw_add" ) ];
		m_rot_speed = 10.f;
		m_rand_update = g_hooks.f[ XOR( "rand_speed" ) ];
		m_dir = g_hooks.m[ XOR( "freestand" ) ][ 0 ];
		m_dir_custom = g_menu.main.antiaim.dir_custom_stand.get( );
		m_base_angle = g_hooks.i[ XOR( "yaw_base" ) ];
		m_auto_time = 5.f;
	}

	// set pitch.
	AntiAimPitch( );

	// if we have any yaw.
	if( m_yaw > 0 ) {
		// set direction.
		GetAntiAimDirection( );
	}

	// we have no real, but we do have a fake.
	else if( g_hooks.i[ XOR( "fake_yaw" ) ] > 0 )
		m_direction = g_cl.m_cmd->m_view_angles.y;

	if( g_hooks.i[ XOR( "fake_yaw" ) ] ) {
		// do not allow 2 consecutive sendpacket true if faking angles.
		if( *g_cl.m_packet && g_cl.m_old_packet )
			*g_cl.m_packet = false;

		// run the real on sendpacket false.
		if( !*g_cl.m_packet || !*g_cl.m_final_packet )
			DoRealAntiAim( );

		// run the fake on sendpacket true.
		else DoFakeAntiAim( );
	}

	// no fake, just run real.
	else DoRealAntiAim( );
}

void HVH::SendPacket( ) {
	// if not the last packet this shit wont get sent anyway.
	// fix rest of hack by forcing to false.
	if( !*g_cl.m_final_packet )
		*g_cl.m_packet = false;

	// fake-lag enabled.
	if( g_hooks.b[ XOR( "enable_fakelag" ) ] && !g_csgo.m_gamerules->m_bFreezePeriod( ) && !( g_cl.m_flags & FL_FROZEN ) ) {
		// limit of lag.
		int variance = std::clamp( ( int )g_hooks.f[ XOR( "fakelag_variance" ) ], 1, 100 );
		int limit = std::min( ( int )g_hooks.f[ XOR( "fakelag_limit" ) ], g_cl.m_max_lag );
		if( g_cl.m_weapon_id == REVOLVER && !g_hooks.is_key_down( g_hooks.i[ XOR( "slowMotion_key" ) ] ) )
			limit = std::min( ( int )6, g_cl.m_max_lag );

		// indicates wether to lag or not.
		bool active{ };

		// get current origin.
		vec3_t cur = g_cl.m_local->m_vecOrigin( );

		// get prevoius origin.
		vec3_t prev = g_cl.m_net_pos.empty( ) ? g_cl.m_local->m_vecOrigin( ) : g_cl.m_net_pos.front( ).m_pos;

		// delta between the current origin and the last sent origin.
		float delta = ( cur - prev ).length_sqr( );

		//auto activation = g_menu.main.antiaim.lag_active.GetActiveIndices( );
		//for( auto it = activation.begin( ); it != activation.end( ); it++ ) {

		// stand.
		if( g_hooks.m[ XOR( "fakelag_triggers" ) ][ 4 ] && delta < 0.1f && g_cl.m_speed < 0.1f && ( !( g_cl.m_buttons & IN_JUMP ) && ( g_cl.m_flags & FL_ONGROUND ) ) ) {
			active = true;
			//break;
		}

		// move.
		else if( g_hooks.m[ XOR( "fakelag_triggers" ) ][ 0 ] && delta > 0.1f && g_cl.m_speed > 0.1f ) {
			active = true;
			//break;
		}

		// air.
		else if( g_hooks.m[ XOR( "fakelag_triggers" ) ][ 1 ] && ( ( g_cl.m_buttons & IN_JUMP ) || !( g_cl.m_flags & FL_ONGROUND ) ) ) {
			active = true;
			//break;
		}

		// crouch.
		else if( g_hooks.m[ XOR( "fakelag_triggers" ) ][ 2 ] && g_cl.m_local->m_bDucking( ) ) {
			active = true;
			//break;
		}

		// peek.
		else if( g_hooks.m[ XOR( "fakelag_triggers" ) ][ 3 ] && g_cl.m_speed > 0.1f ) {
			//if( g_cl.m_valid )
			//	active = true;
		}

		// high speed.
		else if( g_hooks.m[ XOR( "fakelag_triggers" ) ][ 5 ] && g_cl.m_speed > 278.f ) {
			active = true;
		}
		//}

		if( active ) {
			int mode = g_hooks.i[ XOR( "fakelag_mode" ) ];

			// max.
			if( mode == 1 )
				*g_cl.m_packet = false;

			// break.
			else if( mode == 2 && delta <= 4096.f )
				*g_cl.m_packet = false;

			// break step.
			else if( mode == 0 ) {
				// normal break.
				if( m_step_switch ) {
					if( delta <= 4096.f )
						*g_cl.m_packet = false;
				}

				// max.
				else *g_cl.m_packet = false;
			}

			// fluctuate.
			else if( mode == 3 ) {
				if( g_cl.m_cmd->m_command_number % variance >= limit )
					limit = 1;

				*g_cl.m_packet = false;
			}

			if( g_cl.m_lag >= limit )
				*g_cl.m_packet = true;
		}
	}

	if( g_hooks.b[ XOR( "fakelag_on_land_r" ) ] ) {
		if( g_cl.m_local->m_PlayerAnimState( ) )
			if( g_cl.m_local->m_PlayerAnimState( )->m_landing && ( g_cl.m_flags & FL_ONGROUND ) )
				*g_cl.m_packet = true;

	}
	if( g_hooks.b[ XOR( "fakelag_reset" ) ] ) {
		vec3_t                start = g_cl.m_local->m_vecOrigin( ), end = start, vel = g_cl.m_local->m_vecVelocity( );
		CTraceFilterWorldOnly filter;
		CGameTrace            trace;

		// gravity.
		vel.z -= ( g_csgo.sv_gravity->GetFloat( ) * g_csgo.m_globals->m_interval );

		// extrapolate.
		end += ( vel * g_csgo.m_globals->m_interval );

		// move down.
		end.z -= 2.f;

		g_csgo.m_engine_trace->TraceRay( Ray( start, end ), MASK_SOLID, &filter, &trace );

		// check if landed.
		if( trace.m_fraction != 1.f && trace.m_plane.m_normal.z > 0.7f && !( g_cl.m_flags & FL_ONGROUND ) )
			*g_cl.m_packet = true;
	}

	// force fake-lag to 14 when fakelagging.
	if( g_hooks.is_key_down( g_hooks.i[ XOR( "slowMotion_key" ) ] ) && g_hooks.b[ XOR( "slowMotion" ) ] ) {
		*g_cl.m_packet = false;
	}

	// do not lag while shooting.
	if( g_cl.m_old_shot && !g_hooks.b[ XOR( "fakelag_shooting" ) ] )
		*g_cl.m_packet = true;

	// do not lag while dt.
	if( g_hooks.auto_check( XOR( "rapidfire_key" ) ) && g_hooks.b[ XOR( "rapidfire" ) ] )
		*g_cl.m_packet = true;

	// we somehow reached the maximum amount of lag.
	// we cannot lag anymore and we also cannot shoot anymore since we cant silent aim.
	if( g_cl.m_lag >= g_cl.m_max_lag ) {
		// set bSendPacket to true.
		*g_cl.m_packet = true;

		// disable firing, since we cannot choke the last packet.
		g_cl.m_weapon_fire = false;
	}
}