#include "includes.h"

Aimbot g_aimbot{};;

bool AimPlayer::BreakingLagCompensation( Player* pEntity ) {
	if( m_records.size( ) < 2 )
		return false;

	auto prev_org = m_records.at( 0 )->m_origin;
	auto skip_first = true;

	for( auto& record : m_records ) {
		if( skip_first ) {
			skip_first = false;
			continue;
		}

		auto delta = record->m_origin - prev_org;
		if( delta.length_2d_sqr( ) > 4096.f ) {

			return true;
		}

		if( record->m_sim_time <= pEntity->m_flSimulationTime( ) )
			break;

		prev_org = record->m_origin;
	}

	return false;
}

bool AimPlayer::IsFakingYaw( ) {
	const auto bt_records = m_records;
	if( bt_records.size( ) < 2 )
		return true;

	if( fabs( m_records[ 0 ]->m_sim_time - m_records[ 1 ]->m_sim_time ) == g_csgo.m_globals->m_interval )
		return false;

	if( m_records[ 0 ]->m_eye_angles.x != 0 && ( m_records[ 0 ]->m_eye_angles.x > -60.f && m_records[ 0 ]->m_eye_angles.x < 60.f ) )
		return false;

	return true;
}

void FixVelocity( Player* m_player, LagRecord* record, LagRecord* previous, float max_speed ) {
	if( !previous ) {
		if( record->m_layers[ 6 ].m_playback_rate > 0.0f && record->m_layers[ 6 ].m_weight != 0.f && record->m_velocity.length( ) > 0.1f ) {
			auto v30 = max_speed;

			if( record->m_flags & 6 )
				v30 *= 0.34f;
			else if( m_player->m_bIsWalking( ) )
				v30 *= 0.52f;

			auto v35 = record->m_layers[ 6 ].m_weight * v30;
			record->m_velocity *= v35 / record->m_velocity.length( );
		}
		else
			record->m_velocity.clear( );

		if( record->m_flags & 1 )
			record->m_velocity.z = 0.f;

		record->m_anim_velocity = record->m_velocity;
		return;
	}

	if( ( m_player->m_fEffects( ) & 8 ) != 0
		|| m_player->m_ubEFNoInterpParity( ) != m_player->m_ubEFNoInterpParityOld( ) ) {
		record->m_velocity.clear( );
		record->m_anim_velocity.clear( );
		return;
	}

	auto is_jumping = !( record->m_flags & FL_ONGROUND && previous->m_flags & FL_ONGROUND );

	if( record->m_lag > 1 ) {
		record->m_velocity.clear( );
		auto origin_delta = ( record->m_origin - previous->m_origin );

		if( !( previous->m_flags & FL_ONGROUND || record->m_flags & FL_ONGROUND ) )// if not previous on ground or on ground
		{
			auto currently_ducking = record->m_flags & FL_DUCKING;
			if( ( previous->m_flags & FL_DUCKING ) != currently_ducking ) {
				float duck_modifier = 0.f;

				if( currently_ducking )
					duck_modifier = 9.f;
				else
					duck_modifier = -9.f;

				origin_delta.z -= duck_modifier;
			}
		}

		auto sqrt_delta = origin_delta.length_2d_sqr( );

		if( sqrt_delta > 0.f && sqrt_delta < 1000000.f )
			record->m_velocity = origin_delta / game::TICKS_TO_TIME( record->m_lag );

		record->m_velocity.validate_vec( );

		if( is_jumping ) {
			if( record->m_flags & FL_ONGROUND && !g_csgo.sv_enablebunnyhopping->GetInt( ) ) {

				// 260 x 1.1 = 286 units/s.
				float max = m_player->m_flMaxspeed( ) * 1.1f;

				// get current velocity.
				float speed = record->m_velocity.length( );

				// reset velocity to 286 units/s.
				if( max > 0.f && speed > max )
					record->m_velocity *= ( max / speed );
			}

			// assume the player is bunnyhopping here so set the upwards impulse.
			record->m_velocity.z = g_csgo.sv_jump_impulse->GetFloat( );
		}
		// we are not on the ground
		// apply gravity and airaccel.
		else if( !( record->m_flags & FL_ONGROUND ) ) {
			// apply one tick of gravity.
			record->m_velocity.z -= g_csgo.sv_gravity->GetFloat( ) * g_csgo.m_globals->m_interval;
		}
	}

	record->m_anim_velocity = record->m_velocity;

	if( !record->m_fake_walk ) {
		if( record->m_anim_velocity.length_2d( ) > 0 && ( record->m_flags & FL_ONGROUND ) ) {
			float anim_speed = 0.f;

			if( !is_jumping
				&& record->m_layers[ 11 ].m_weight > 0.0f
				&& record->m_layers[ 11 ].m_weight < 1.0f
				&& record->m_layers[ 11 ].m_playback_rate == previous->m_layers[ 11 ].m_playback_rate ) {
				// calculate animation speed yielded by anim overlays
				auto flAnimModifier = 0.35f * ( 1.0f - record->m_layers[ 11 ].m_weight );
				if( flAnimModifier > 0.0f && flAnimModifier < 1.0f )
					anim_speed = max_speed * ( flAnimModifier + 0.55f );
			}

			// this velocity is valid ONLY IN ANIMFIX UPDATE TICK!!!
			// so don't store it to record as m_vecVelocity
			// -L3D451R7
			if( anim_speed > 0.0f ) {
				anim_speed /= record->m_anim_velocity.length_2d( );
				record->m_anim_velocity.x *= anim_speed;
				record->m_anim_velocity.y *= anim_speed;
			}
		}
	}
	else
		record->m_anim_velocity.clear( );

	record->m_anim_velocity.validate_vec( );
}

void SimulateMovement( vec3_t velocity, vec3_t origin, Player* player, int flags, bool in_air ) {
	vec3_t                start, end, normal;
	CGameTrace            trace;
	CTraceFilterWorldOnly filter;

	// define trace start.
	start = origin;

	// move trace end one tick into the future using predicted velocity.
	end = start + ( velocity * g_csgo.m_globals->m_interval );

	// trace.
	g_csgo.m_engine_trace->TraceRay( Ray( start, end, player->m_vecMins( ), player->m_vecMaxs( ) ), CONTENTS_SOLID, &filter, &trace );

	// we hit shit
	// we need to fix hit.
	if( trace.m_fraction != 1.f ) {

		// fix sliding on planes.
		for( int i{}; i < 2; ++i ) {
			velocity -= trace.m_plane.m_normal * velocity.dot( trace.m_plane.m_normal );

			float adjust = velocity.dot( trace.m_plane.m_normal );
			if( adjust < 0.f )
				velocity -= ( trace.m_plane.m_normal * adjust );

			start = trace.m_endpos;
			end = start + ( velocity * ( g_csgo.m_globals->m_interval * ( 1.f - trace.m_fraction ) ) );

			g_csgo.m_engine_trace->TraceRay( Ray( start, end, player->m_vecMins( ), player->m_vecMaxs( ) ), CONTENTS_SOLID, &filter, &trace );
			if( trace.m_fraction == 1.f )
				break;
		}
	}

	// set new final origin.
	start = end = origin = trace.m_endpos;

	// move endpos 2 units down.
	// this way we can check if we are in/on the ground.
	end.z -= 2.f;

	// trace.
	g_csgo.m_engine_trace->TraceRay( Ray( start, end, player->m_vecMins( ), player->m_vecMaxs( ) ), CONTENTS_SOLID, &filter, &trace );

	// strip onground flag.
	flags &= ~FL_ONGROUND;

	// add back onground flag if we are onground.
	if( trace.m_fraction != 1.f && trace.m_plane.m_normal.z > 0.7f )
		flags |= FL_ONGROUND;
}

void AimPlayer::UpdateAnimations( LagRecord* record ) {
	CCSGOPlayerAnimState* state = m_player->m_PlayerAnimState( );

	if( !state )
		return;

	m_player->m_bClientSideAnimation( ) = true;

	// player respawned.
	if( m_player->m_flSpawnTime( ) != m_spawn && ( state->m_last_update_time - g_csgo.m_globals->m_curtime ) > 0.5f ) {
		// reset animation state.
		game::ResetAnimationState( state );

		// note new spawn time.
		m_spawn = m_player->m_flSpawnTime( );
	}

	// backup curtime.
	float curtime = g_csgo.m_globals->m_curtime;
	float frametime = g_csgo.m_globals->m_frametime;

	// set curtime to animtime.
	// set frametime to ipt just like on the server during simulation.
	g_csgo.m_globals->m_curtime = record->m_anim_time;
	if( record->m_lag > 19 )
		g_csgo.m_globals->m_curtime = record->m_sim_time;
	g_csgo.m_globals->m_frametime = g_csgo.m_globals->m_interval;

	// backup stuff that we do not want to fuck with.
	AnimationBackup_t backup;

	backup.m_origin = m_player->m_vecOrigin( );
	backup.m_abs_origin = m_player->GetAbsOrigin( );
	backup.m_velocity = m_player->m_vecVelocity( );
	backup.m_abs_velocity = m_player->m_vecAbsVelocity( );
	backup.m_flags = m_player->m_fFlags( );
	backup.m_eflags = m_player->m_iEFlags( );
	backup.m_duck = m_player->m_flDuckAmount( );
	backup.m_body = m_player->m_flLowerBodyYawTarget( );
	m_player->GetAnimLayers( backup.m_layers );

	LagRecord* previous = m_records.size( ) > 1 ? m_records[ 1 ].get( ) : nullptr;

	if( previous && ( previous->dormant( ) || !previous->m_setup ) )
		previous = nullptr;

	// is player a bot?
	bool bot = game::IsFakePlayer( m_player->index( ) );

	// reset fakewalk state.
	record->m_fake_walk = false;
	record->m_mode = Resolver::Modes::RESOLVE_NONE;

	m_player->SetAbsOrigin( record->m_origin );

	auto m_weapon = m_player->GetActiveWeapon( );

	const auto simulation_ticks = game::TIME_TO_TICKS( record->m_sim_time );
	auto old_simulation_ticks = game::TIME_TO_TICKS( record->m_old_sim_time );

	// fix various issues with the game
	// these issues can only occur when a player is choking data.
	if( !bot ) {
		// detect fakewalk.
		float speed = backup.m_velocity.length( );

		// we need atleast 2 updates/records
		// to fix these issues.
		if( previous ) {
			// get pointer to previous record.
			old_simulation_ticks = game::TIME_TO_TICKS( previous->m_sim_time );
			record->m_lag = simulation_ticks - old_simulation_ticks;

			bool two_tick_ground = (m_player->m_fFlags() & FL_ONGROUND) && (previous->m_flags & FL_ONGROUND);

			// check if player broke lc on this tick
			// https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/game/server/player_lagcompensation.cpp#L462-L467
			record->m_broke_lc = (record->m_origin - previous->m_origin).length_2d_sqr() > 4096.f && !two_tick_ground;

			m_player->SetAnimLayers( previous->m_layers );

			auto v490 = m_player->GetSequenceActivity( record->m_layers[ 5 ].m_sequence );

			if( record->m_layers[ 5 ].m_sequence == previous->m_layers[ 5 ].m_sequence && ( previous->m_layers[ 5 ].m_weight != 0.0f || record->m_layers[ 5 ].m_weight == 0.0f )
				|| !( v490 == ACT_CSGO_LAND_LIGHT || v490 == ACT_CSGO_LAND_HEAVY ) ) {
				if( ( record->m_flags & 1 ) != 0 && ( previous->m_flags & FL_ONGROUND ) == 0 )
					m_player->m_fFlags( ) &= ~FL_ONGROUND;
			}
			else
				m_player->m_fFlags( ) |= FL_ONGROUND;
	

			if( record->m_flags & FL_ONGROUND
				&& record->m_layers[ 4 ].m_weight == 0.0f
				&& record->m_layers[ 5 ].m_weight == 0.0f
				&& record->m_layers[ 6 ].m_playback_rate == 0.0f
				&& speed > 0.0f ) {
				record->m_fake_walk = true;
				m_player->m_iMoveState( ) = 0;
			}

			if( record->m_fake_walk )
				record->m_anim_velocity = record->m_velocity = { 0.f, 0.f, 0.f };

			if( record->m_layers[ 3 ].m_cycle == 0.0f && record->m_layers[ 3 ].m_weight_delta_rate != 0.0f
				|| m_player->GetSequenceActivity( record->m_layers[ 3 ].m_sequence ) == 979
				&& ( previous->m_layers[ 3 ].m_sequence != record->m_layers[ 3 ].m_sequence || record->m_layers[ 3 ].m_cycle <= previous->m_layers[ 3 ].m_cycle ) ) {
				record->m_anim_velocity = record->m_velocity = { 0.f, 0.f, 0.f };
				m_player->m_fFlags( ) |= FL_ONGROUND;
				m_player->m_PlayerAnimState( )->m_on_ground = true;
				m_player->m_PlayerAnimState( )->m_landing = false;
			}

			if( record->m_flags & FL_ONGROUND
				&& ( speed < 0.1f || record->m_fake_walk )
				&& abs( int( previous->m_body - record->m_body ) ) <= 0.00001f ) {
				m_player->m_PlayerAnimState( )->m_abs_yaw = previous->m_body;
			}

			m_player->m_PlayerAnimState( )->m_move_weight = previous->m_layers[ 6 ].m_weight;
			m_player->m_PlayerAnimState( )->m_primary_cycle = previous->m_layers[ 6 ].m_cycle;

			auto playback = record->m_layers[ 11 ].m_playback_rate;
			if( playback != 0.0f ) {
				int ticks = 0;

				auto layer_cycle = record->m_layers[ 11 ].m_cycle;
				auto previous_playback = previous->m_layers[ 11 ].m_playback_rate;

				if( previous_playback != 0.0f ) {
					auto previous_cycle = previous->m_layers[ 11 ].m_cycle;
					ticks = 0;

					if( previous_cycle > layer_cycle )
						layer_cycle = layer_cycle + 1.0f;

					while( layer_cycle > previous_cycle ) {
						const auto ticks_backup = ticks;
						const auto playback_mult_ipt = g_csgo.m_globals->m_interval * previous_playback;

						previous_cycle = previous_cycle + ( g_csgo.m_globals->m_interval * previous_playback );

						if( previous_cycle >= 1.0f )
							previous_playback = playback;

						++ticks;

						if( previous_cycle > layer_cycle && ( previous_cycle - layer_cycle ) > ( playback_mult_ipt * 0.5f ) )
							ticks = ticks_backup;
					}
				}

				record->m_lag = std::clamp( ticks, 0, 20 );
			}

			// fix crouching players.
			// the duck amount we receive when people choke is of the last simulation.
			// if a player chokes packets the issue here is that we will always receive the last duckamount.
			// but we need the one that was animated.
			// therefore we need to compute what the duckamount was at animtime.

			// delta in duckamt and delta in time..
			float duck = record->m_duck - previous->m_duck;
			float time = record->m_sim_time - previous->m_sim_time;

			// get the duckamt change per tick.
			float change = ( duck / time ) * g_csgo.m_globals->m_interval;

			// fix crouching players.
			m_player->m_flDuckAmount( ) = previous->m_duck + change;

			m_player->m_PlayerAnimState( )->m_walk_run_transition = previous->animstate.m_walk_run_transition;
			m_player->m_PlayerAnimState( )->m_velocity_length_xy = previous->animstate.m_velocity_length_xy;
			m_player->m_PlayerAnimState( )->m_speed_as_portion_of_crouch_top_speed = previous->animstate.m_speed_as_portion_of_crouch_top_speed;
			m_player->m_PlayerAnimState( )->m_speed_as_portion_of_walk_top_speed = previous->animstate.m_speed_as_portion_of_walk_top_speed;
			m_player->m_PlayerAnimState( )->m_abs_yaw = previous->animstate.m_abs_yaw;
			m_player->m_PlayerAnimState( )->m_abs_yaw_last = previous->animstate.m_abs_yaw_last;
			m_player->m_PlayerAnimState( )->m_move_yaw = previous->animstate.m_move_yaw;
			m_player->m_PlayerAnimState( )->m_move_yaw_ideal = previous->animstate.m_move_yaw_ideal;
			m_player->m_PlayerAnimState( )->m_move_yaw_current_to_ideal = previous->animstate.m_move_yaw_current_to_ideal;
			m_player->m_PlayerAnimState( )->m_primary_cycle = previous->animstate.m_primary_cycle;
			m_player->m_PlayerAnimState( )->m_move_weight = previous->animstate.m_move_weight;
			m_player->m_PlayerAnimState( )->m_move_weight_smoothed = previous->animstate.m_move_weight_smoothed;
		}
		else {
			if( record->m_flags & FL_ONGROUND ) {
				m_player->m_PlayerAnimState( )->m_on_ground = true;
				m_player->m_PlayerAnimState( )->m_landing = false;
			}
		}
	}

	float max_speed = 260.f;

	if( m_weapon && !m_weapon->IsKnife( ) ) {


		auto data = m_weapon->GetWpnData( );

		if( data )
			max_speed = m_player->m_bIsScoped( ) ? data->m_max_player_speed_alt : data->m_max_player_speed;
	}


	FixVelocity( m_player, record, previous, max_speed );

	record->m_body_flick = false;

	bool fake = !bot && record->m_lag > 0 /*&& IsFakingYaw( ) */ && g_hooks.b[ XOR( "aa_correction" ) ] && !c_playerlist::get( )->should_disable_resolver( m_player ) && m_player->enemy( g_cl.m_local );

	if( fake ) {
		if( record->m_anim_velocity.length_2d( ) > 0.1f && !record->m_fake_walk || ( record->m_flags & FL_ONGROUND ) == 0 ) {
			m_body_update = FLT_MAX;
			m_body_catched = false;
		}
		else {
			// we need atleast 2 updates/records
			// to fix these issues.
			if( m_records.size( ) >= 2 && record->m_lag < 20 ) {
				// get pointer to previous record.
				LagRecord* previous = m_records[ 1 ].get( );

				if( previous && !previous->dormant( ) ) {
					//lby updated
					auto delta = math::NormalizedAngle( record->m_body - previous->m_body );

					if( abs( delta ) >= 35.f && m_body_update == FLT_MAX ) {
						record->m_body_flick = true;
						m_body_catched = true;
						m_body_update = record->m_anim_time + 1.1f;
					}
				}
			}
		}
	}
	
	record->m_feet_yaw = state->m_goal_feet_yaw;
	// if using fake angles, correct angles.
	if( fake /* && record->m_lag < 20 */ )
		g_resolver.ResolveAngles( m_player, record );

	// set stuff before animating.
	m_player->m_vecOrigin( ) = record->m_origin;
	m_player->m_flLowerBodyYawTarget( ) = record->m_body;

	// EFL_DIRTY_ABSVELOCITY
	// skip call to C_BaseEntity::CalcAbsoluteVelocity
	m_player->m_iEFlags( ) &= ~0x1000u;
	m_player->SetAbsVelocity( record->m_anim_velocity );

	// write potentially resolved angles.
	m_player->m_angEyeAngles( ) = record->m_eye_angles;

	//if (m_records.size() < 2)
	//	state->m_goal_feet_yaw = m_player->m_angEyeAngles().y;

	const auto v75 = g_csgo.m_globals->m_frame;
	const auto v136 = g_csgo.m_globals->m_realtime;
	const auto v126 = g_csgo.m_globals->m_curtime;
	const auto v156 = g_csgo.m_globals->m_frametime;
	const auto v139 = g_csgo.m_globals->m_abs_frametime;
	const auto v140 = g_csgo.m_globals->m_interp_amt;
	const auto v155 = g_csgo.m_globals->m_tick_count;

	const auto v76 = record->m_anim_time / g_csgo.m_globals->m_interval;
	const int v77 = v76 + 0.5f;
	g_csgo.m_globals->m_realtime = record->m_anim_time;
	g_csgo.m_globals->m_curtime = record->m_anim_time;
	g_csgo.m_globals->m_frametime = g_csgo.m_globals->m_interval;
	g_csgo.m_globals->m_abs_frametime = g_csgo.m_globals->m_interval;
	g_csgo.m_globals->m_frame = v77;
	g_csgo.m_globals->m_tick_count = v77;
	g_csgo.m_globals->m_interp_amt = 0.0;

	// fix animating in same frame.
	if( state->m_last_update_frame >= g_csgo.m_globals->m_frame )
		state->m_last_update_frame = g_csgo.m_globals->m_frame - 1;

	// 'm_animating' returns true if being called from SetupVelocity, passes raw velocity to animstate.
	g_cl.m_updating_anims = true;
	m_player->UpdateClientSideAnimation( );
	g_cl.m_updating_anims = false;

	state->m_goal_feet_yaw = record->m_feet_yaw;

	m_player->SetAnimLayers( record->m_layers );
	m_player->GetPoseParameters( record->m_poses );

	memcpy( &record->animstate, m_player->m_PlayerAnimState( ), sizeof( CCSGOPlayerAnimState ) );

	record->m_abs_ang = m_player->GetAbsAngles( );
	record->m_abs_ang.y = m_player->m_PlayerAnimState( )->m_abs_yaw;

	m_player->InvalidateBoneCache( );
	record->m_setup = m_player->SetupBones( record->m_bones, 128, 0x100, record->m_anim_time, true );
	record->m_bones_count = m_player->m_BoneCache( ).m_CachedBoneCount;

	g_csgo.m_globals->m_realtime = v136;
	g_csgo.m_globals->m_curtime = v126;
	g_csgo.m_globals->m_frametime = v156;
	g_csgo.m_globals->m_abs_frametime = v139;
	g_csgo.m_globals->m_interp_amt = v140;
	g_csgo.m_globals->m_frame = v75;
	g_csgo.m_globals->m_tick_count = v155;

	// restore backup data.
	m_player->m_vecOrigin( ) = backup.m_origin;
	m_player->m_vecVelocity( ) = backup.m_velocity;
	m_player->m_vecAbsVelocity( ) = backup.m_abs_velocity;
	m_player->m_fFlags( ) = backup.m_flags;
	m_player->m_iEFlags( ) = backup.m_eflags;
	m_player->m_flDuckAmount( ) = backup.m_duck;
	m_player->m_flLowerBodyYawTarget( ) = backup.m_body;
	m_player->SetAbsOrigin( backup.m_abs_origin );

	m_player->InvalidateBoneCache( );

	record->m_render_origin = backup.m_abs_origin;
	m_player->SetupBones( record->m_render_bones, 128, BONE_USED_BY_ANYTHING, record->m_sim_time );

	// IMPORTANT: do not restore poses here, since we want to preserve them for rendering.
	// also dont restore the render angles which indicate the model rotation.

	// restore globals.
	g_csgo.m_globals->m_curtime = curtime;
	g_csgo.m_globals->m_frametime = frametime;
}

float m_flCycle;
float m_flPlaybackRate;

void AimPlayer::OnNetUpdate( Player* player ) {
	bool reset = player->m_lifeState( ) == LIFE_DEAD;

	// if this happens, delete all the lagrecords.
	if( reset ) {
		player->m_bClientSideAnimation( ) = true;
		m_records.clear( );
		last_simulation_time = 0;
		return;
	}

	// update player ptr if required.
	// reset player if changed.
	if( m_player != player ) {
		m_records.clear( );
		last_simulation_time = 0;
	}

	// update player ptr.
	m_player = player;

	// indicate that this player has been out of pvs.
	// insert dummy record to separate records
	// to fix stuff like animation and prediction.
	if( player->dormant( ) ) {
		bool insert = true;

		m_body_update = FLT_MAX;
		m_body_catched = false;
		move_lby_delta = FLT_MAX;
		never_saw_movement = true;
		last_move_lby_valid = false;
		m_moved = false;

		// we have any records already?
		m_records.clear( );
	}

	if( player->m_flSimulationTime( ) == 0.0f || player->dormant( ) ) {
		m_records.clear( );

		m_flCycle = 0;
		m_flPlaybackRate = 0;
		return;
	}


	bool update = true;

	if( m_records.size( ) > 0 ) {
		LagRecord* front = m_records[ 0 ].get( );

		for( auto i = 0; i < 13; i++ ) {
			auto layer = &m_player->m_AnimOverlay( )[ i ];

			update = ( layer->m_cycle != front->m_layers[ i ].m_cycle
					   || layer->m_playback_rate != front->m_layers[ i ].m_playback_rate
					   || layer->m_weight != front->m_layers[ i ].m_weight
					   || layer->m_sequence != front->m_layers[ i ].m_sequence );

			if( update )
				break;
		}

		if( !update ) {
			m_player->m_flOldSimulationTime( ) = front->m_sim_time - game::TICKS_TO_TIME( front->m_lag );
			m_player->m_flSimulationTime( ) = front->m_sim_time;
		}
	}

	// this is the first data update we are receving
	// OR we received data with a newer simulation context.
	if( update ) {
		if( BreakingLagCompensation( player ) )
			for( auto& record : m_records ) {
				record->m_invalid = true;
			}

		// add new record.
		m_records.emplace_front( std::make_shared< LagRecord >( player ) );

		// add new record.
		LagRecord* current = m_records[ 0 ].get( );
		current->store( m_player );

		// mark as non dormant.
		current->m_dormant = false;

		// update animations on current record.
		// call resolver.
		UpdateAnimations( current );

		// create bone matrix for this record.
		//g_bones.setup(m_player, nullptr, current);

		current->m_can_aim = last_simulation_time < player->m_flSimulationTime( );
		last_simulation_time = std::max( last_simulation_time, player->m_flSimulationTime( ) );
	}

	// no need to store insane amt of data.
	while( m_records.size( ) > 64 )
		m_records.pop_back( );
}

void AimPlayer::OnRoundStart( Player* player ) {
	m_player = player;
	memset( &m_walk_record, 0, sizeof( LagRecord ) );
	m_shots = 0;
	m_missed_shots = 0;

	// reset stand index.
	m_last_move = 0;
	m_ff_index = 0;
	m_freestand_index = 0;
	m_body_index = 0;
	m_lby_delta_index = 0;
	cumhook_angle = 0;
	count = 0;

	m_records.clear();
	m_hitboxes.clear();

	memset( &g_inputpred.data, 0, sizeof( CMoveData ) );

	// IMPORTANT: DO NOT CLEAR LAST HIT SHIT.
}

bool CanFireWithExploit( int m_iShiftedTick ) {
	// curtime before shift
	float curtime = game::TICKS_TO_TIME( g_cl.m_local->m_nTickBase( ) - m_iShiftedTick );
	return g_cl.CanFireWeapon( curtime );
}

void Aimbot::handle_doubletap( ) {

	if (g_hooks.b[XOR("double_tap")])
		m_double_tap = true;
	else
		m_double_tap = false;

	if (!m_double_tap && m_charged) {
		m_charge_timer = 0;
		m_tick_to_shift = 13;
	}
	if (!m_double_tap) return;
	if (!m_charged) {
		if (m_charge_timer > game::TIME_TO_TICKS(.5)) { // .5 seconds after shifting, lets recharge
			m_tick_to_recharge = 14;
		}
		else {
			if (!g_aimbot.m_target) {
				m_charge_timer++;
			}
			if (g_cl.m_cmd->m_buttons & IN_ATTACK && g_cl.m_weapon_fire) {
				m_charge_timer = 0;
			}
		}
	}

	if (g_cl.m_cmd->m_buttons & IN_ATTACK && g_cl.m_weapon_fire && m_charged) {
		// shot.. lets shift tickbase back so we can dt.
		m_charge_timer = 0;
		m_tick_to_shift = 14;
		m_shift_cmd = g_cl.m_cmd->m_command_number;
		m_shift_tickbase = g_cl.m_local->m_nTickBase();
		*g_cl.m_packet = false;
	}
	if (!m_charged) {
		m_charged_ticks = 0;
	}
}

	void CL_Move(float accumulated_extra_samples, bool bFinalTick) {
		if (g_tickshift.m_tick_to_recharge > 0) {
			g_tickshift.m_tick_to_recharge--;
			g_tickshift.m_charged_ticks++;
			if (g_tickshift.m_tick_to_recharge == 0) {
				g_tickshift.m_charged = true;
			}
			return; // increment ticksforprocessing by not creating any usercmd's
		}

		o_CLMove(accumulated_extra_samples, bFinalTick);
		g_tickshift.m_shifted = false;
		if (g_tickshift.m_tick_to_shift > 0) {
			g_tickshift.m_shifting = true;
			for (; g_tickshift.m_tick_to_shift > 0; g_tickshift.m_tick_to_shift--) {
				o_CLMove(accumulated_extra_samples, bFinalTick);
				g_tickshift.m_charged_ticks--;
			}
			g_tickshift.m_charged = false; // were just going to assume. not correct.
			g_tickshift.m_shifting = false;
			g_tickshift.m_shifted = true;
		}
	}

float m_anim_frame = 0;
float m_anim_time = 0;
void AimPlayer::SetupHitboxes( LagRecord* record, bool history ) {
	if( !g_cl.m_weapon )
		return;

	// reset hitboxes.
	m_hitboxes.clear( );
	bool ignore_limbs = record->m_velocity.length_2d( ) > 1.f && g_hooks.b[ XOR( "ignor_limbs" ) ];
	bool prefer_head = record->m_velocity.length_2d( ) > 10.f && !( record->m_pred_flags & FL_ONGROUND );
	bool in_move = record->m_velocity.length_2d( ) > .1f;
	std::unordered_map<int, bool> baim1 = g_hooks.m[ XOR( "baim1" ) ];
	std::unordered_map<int, bool> baim2 = g_hooks.m[ XOR( "baim2" ) ];
	std::unordered_map<int, bool> hitbox = g_hooks.m[ XOR( "hitbox" ) ];

	if( g_cl.m_weapon_type == WEAPONTYPE_PISTOL && g_cl.m_weapon_id != REVOLVER ) {
		ignore_limbs = record->m_velocity.length_2d( ) > 71.f && g_hooks.b[ XOR( "ignor_limbs_pistols" ) ];
		baim1 = g_hooks.m[ XOR( "baim1_scout" ) ];
		baim2 = g_hooks.m[ XOR( "baim2_scout" ) ];
		hitbox = g_hooks.m[ XOR( "hitbox_pistols" ) ];
	}

	switch( g_cl.m_weapon_id ) {
		case SSG08:
			ignore_limbs = record->m_velocity.length_2d( ) > 71.f && g_hooks.b[ XOR( "ignor_limbs_scout" ) ];
			baim1 = g_hooks.m[ XOR( "baim1_scout" ) ];
			baim2 = g_hooks.m[ XOR( "baim2_scout" ) ];
			hitbox = g_hooks.m[ XOR( "hitbox_scout" ) ];
			break;

		case AWP:
			ignore_limbs = record->m_velocity.length_2d( ) > 71.f && g_hooks.b[ XOR( "ignor_limbs_awp" ) ];
			baim1 = g_hooks.m[ XOR( "baim1_awp" ) ];
			baim2 = g_hooks.m[ XOR( "baim2_awp" ) ];
			hitbox = g_hooks.m[ XOR( "hitbox_awp" ) ];
			break;

		case DEAGLE:
			ignore_limbs = record->m_velocity.length_2d( ) > 71.f && g_hooks.b[ XOR( "ignor_limbs_deagle" ) ];
			baim1 = g_hooks.m[ XOR( "baim1_deagle" ) ];
			baim2 = g_hooks.m[ XOR( "baim2_deagle" ) ];
			hitbox = g_hooks.m[ XOR( "hitbox_deagle" ) ];
			break;

		case REVOLVER:
			ignore_limbs = record->m_velocity.length_2d( ) > 71.f && g_hooks.b[ XOR( "ignor_limbs_revolver" ) ];
			baim1 = g_hooks.m[ XOR( "baim1_revolver" ) ];
			baim2 = g_hooks.m[ XOR( "baim2_revolver" ) ];
			hitbox = g_hooks.m[ XOR( "hitbox_revolver" ) ];
			break;

		default:
			break;
	}

	if( g_cl.m_weapon_id == ZEUS ) {
		// hitboxes for the zeus.
		m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::PREFER } );
		return;
	}


	if( c_playerlist::get( )->should_prefer_body( m_player ) )
		m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::PREFER } );

	// prefer, always.
	if( baim1[ 0 ] )
		m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::PREFER } );

	// prefer, lethal.
	if( baim1[ 1 ] )
		m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::LETHAL } );

	// prefer, lethal x2.
	if( baim1[ 2 ] )
		m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::LETHAL2 } );

	// prefer, fake.
	if( baim1[ 3 ] && record->m_mode != Resolver::Modes::RESOLVE_NONE && record->m_mode != Resolver::Modes::RESOLVE_WALK )
		m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::PREFER } );

	// prefer, in air.
	if( baim1[ 4 ] && !( record->m_pred_flags & FL_ONGROUND ) )
		m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::PREFER } );

	// prefer, after x misses.
	if( baim1[ 5 ] && ( m_stand_index >= g_hooks.f[ XOR( "misses" ) ] || m_missed_shots >= g_hooks.f[ XOR( "misses" ) ] ) )
		m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::PREFER } );

	bool only{ false };

	// only, always.
	if( baim2[ 0 ] ) {
		only = true;
		m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::PREFER } );
	}

	// only, health.
	if( baim2[ 1 ] && m_player->m_iHealth( ) <= ( int )g_hooks.f[ XOR( "baim_hp" ) ] ) {
		only = true;
		m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::PREFER } );
	}

	// only, fake.
	if( baim2[ 2 ] && record->m_mode != Resolver::Modes::RESOLVE_NONE && record->m_mode != Resolver::Modes::RESOLVE_WALK ) {
		only = true;
		m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::PREFER } );
	}

	// only, in air.
	if( baim2[ 3 ] && !( record->m_pred_flags & FL_ONGROUND ) ) {
		only = true;
		m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::PREFER } );
	}

	// only, after x misses.
	if( baim2[ 4 ] && ( m_stand_index >= g_hooks.f[ XOR( "misses" ) ] || m_missed_shots >= g_hooks.f[ XOR( "misses" ) ] ) ) {
		only = true;
		m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::PREFER } );
	}

	// only, on key.
	if( g_hooks.auto_check( "damage1_key" ) && g_hooks.b[ XOR( "body_aim_enable" ) ] ) {
		only = true;
		m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::PREFER } );
	}

	// only baim conditions have been met.
	// do not insert more hitboxes.



	// prefer
	/*if( g_hooks.m[ XOR( "head1" ) ][ 0 ] )
		m_hitboxes.push_back( { HITBOX_HEAD, HitscanMode::PREFER } );

	if( g_hooks.m[ XOR( "head1" ) ][ 1 ] && prefer_head )
		m_hitboxes.push_back( { HITBOX_HEAD, HitscanMode::PREFER } );

	if( g_hooks.m[ XOR( "head1" ) ][ 2 ] && !( record->m_mode != Resolver::Modes::RESOLVE_NONE && record->m_mode != Resolver::Modes::RESOLVE_WALK && record->m_mode != Resolver::Modes::RESOLVE_BODY ) )
		m_hitboxes.push_back( { HITBOX_HEAD, HitscanMode::PREFER } );

	if( g_hooks.m[ XOR( "head1" ) ][ 3 ] && !( record->m_pred_flags & FL_ONGROUND ) )
		m_hitboxes.push_back( { HITBOX_HEAD, HitscanMode::PREFER } );*/

		// head.
	if( hitbox[ 0 ] && !only) {
		m_hitboxes.push_back( { HITBOX_HEAD, HitscanMode::NORMAL } );
	}

	// chest.
	if( hitbox[ 1 ] ) {
		m_hitboxes.push_back( { HITBOX_UPPER_CHEST, HitscanMode::NORMAL } );
		m_hitboxes.push_back( { HITBOX_THORAX, HitscanMode::NORMAL } );
		m_hitboxes.push_back( { HITBOX_CHEST, HitscanMode::NORMAL } );
	}

	// stomach.
	if( hitbox[ 2 ] ) {
		m_hitboxes.push_back( { HITBOX_BODY, HitscanMode::NORMAL } );
		m_hitboxes.push_back( { HITBOX_PELVIS, HitscanMode::NORMAL } );
	}

	// arms.
	if( hitbox[ 3 ] && !ignore_limbs ) {
		m_hitboxes.push_back( { HITBOX_L_UPPER_ARM, HitscanMode::NORMAL } );
		m_hitboxes.push_back( { HITBOX_R_UPPER_ARM, HitscanMode::NORMAL } );
	}

	// legs.
	if( hitbox[ 4 ] ) {
		m_hitboxes.push_back( { HITBOX_L_THIGH, HitscanMode::NORMAL } );
		m_hitboxes.push_back( { HITBOX_R_THIGH, HitscanMode::NORMAL } );
		m_hitboxes.push_back( { HITBOX_L_CALF, HitscanMode::NORMAL } );
		m_hitboxes.push_back( { HITBOX_R_CALF, HitscanMode::NORMAL } );
	}

	// foot.
	if( hitbox[ 5 ] && !ignore_limbs ) {
		m_hitboxes.push_back( { HITBOX_L_FOOT, HitscanMode::NORMAL } );
		m_hitboxes.push_back( { HITBOX_R_FOOT, HitscanMode::NORMAL } );
	}
}

void Aimbot::init( ) {
	// clear old targets.
	m_targets.clear( );

	m_target = nullptr;
	m_aim = vec3_t{};
	m_angle = ang_t{};
	m_damage = 0.f;
	m_record = nullptr;

	m_best_dist = std::numeric_limits< float >::max( );
	m_best_fov = 180.f + 1.f;
	m_best_damage = 0.f;
	m_best_hp = 100 + 1;
	m_best_lag = std::numeric_limits< float >::max( );
	m_best_height = std::numeric_limits< float >::max( );
}

void Aimbot::StripAttack( ) {
	if( g_cl.m_weapon_id == REVOLVER )
		g_cl.m_cmd->m_buttons &= ~IN_ATTACK2;

	else
		g_cl.m_cmd->m_buttons &= ~IN_ATTACK;
}

void Aimbot::think( ) {
	if( !g_cl.m_processing || !g_hooks.b[ XOR( "rbotenable" ) ] )
		return;

	// do all startup routines.
	init( );

	bool between_shots = g_hooks.b[ XOR( "between_shots" ) ];

	if( g_cl.m_weapon_type == WEAPONTYPE_PISTOL && g_cl.m_weapon_id != REVOLVER ) {
		if( !g_hooks.b[ XOR( "autofire_pistols" ) ] && !( g_cl.m_cmd->m_buttons & IN_ATTACK ) )
			return;

		between_shots = g_hooks.b[ XOR( "between_shots_pistols" ) ];
	}

	switch( g_cl.m_weapon_id ) {
		case SSG08:
			if( !g_hooks.b[ XOR( "autofire_scout" ) ] && !( g_cl.m_cmd->m_buttons & IN_ATTACK ) )
				return;

			between_shots = g_hooks.b[ XOR( "between_shots_scout" ) ];
			break;

		case AWP:
			if( !g_hooks.b[ XOR( "autofire_awp" ) ] && !( g_cl.m_cmd->m_buttons & IN_ATTACK ) )
				return;

			between_shots = g_hooks.b[ XOR( "between_shots_awp" ) ];
			break;

		case DEAGLE:
			if( !g_hooks.b[ XOR( "autofire_deagle" ) ] && !( g_cl.m_cmd->m_buttons & IN_ATTACK ) )
				return;

			between_shots = g_hooks.b[ XOR( "between_shots_deagle" ) ];
			break;

		case REVOLVER:
			if( !g_hooks.b[ XOR( "autofire_revolver" ) ] && !( g_cl.m_cmd->m_buttons & IN_ATTACK ) )
				return;

			between_shots = g_hooks.b[ XOR( "between_shots_revolver" ) ];
			break;

		default:
			if( !g_hooks.b[ XOR( "autofire" ) ] && !( g_cl.m_cmd->m_buttons & IN_ATTACK ) )
				return;

			break;
	}

	// sanity.
	if( !g_cl.m_weapon )
		return;

	// no grenades or bomb.
	if( g_cl.m_weapon_type == WEAPONTYPE_GRENADE || g_cl.m_weapon_type == WEAPONTYPE_C4 )
		return;

	if( !g_cl.m_weapon_fire )
		StripAttack( );

	// we have no aimbot enabled.
	if( !g_hooks.b[ XOR( "rbotenable" ) ] )
		return;

	// animation silent aim, prevent the ticks with the shot in it to become the tick that gets processed.
	// we can do this by always choking the tick before we are able to shoot.
	bool revolver = g_cl.m_weapon_id == REVOLVER && g_cl.m_revolver_cock != 0;

	// one tick before being able to shoot.
	if( revolver && g_cl.m_revolver_cock > 0 && g_cl.m_revolver_cock == g_cl.m_revolver_query ) {
		*g_cl.m_packet = false;
		return;
	}

	// we have a normal weapon or a non cocking revolver
	// choke if its the processing tick.
	/*
	if( g_cl.m_weapon_fire && !g_cl.m_lag && !revolver ) {
		*g_cl.m_packet = false;
		StripAttack( );
		return;
	}*/

	if( !between_shots ) {
		// no point in aimbotting if we cannot fire this tick.
		if( !g_cl.m_weapon_fire )
			return;
	}
	else {
		if( !g_cl.m_local->alive( ) )
			return;

		if( g_cl.m_weapon_type == WEAPONTYPE_GRENADE )
			return;

		// if we have no bullets, we cant shoot.
		if( g_cl.m_weapon_type != WEAPONTYPE_KNIFE && g_cl.m_weapon->m_iClip1( ) < 1 )
			return;

		// get ptr to layer 1.
		C_AnimationLayer* layer1 = &g_cl.m_local->m_AnimOverlay( )[ 1 ];

		// check if reload animation is going on.
		if( layer1->m_weight != 0.f && g_cl.m_local->GetSequenceActivity( layer1->m_sequence ) == ACT_CSGO_RELOAD )
			return;
	}

	// setup bones for all valid targets.
	for( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i ) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );

		if( !IsValidTarget( player ) )
			continue;

		if( c_playerlist::get( )->should_disable_aimbot( player ) )
			continue;

		AimPlayer* data = &m_players[ i - 1 ];
		if( !data )
			continue;

		// store player as potential target this tick.
		m_targets.emplace_back( data );
	}

	if( g_hooks.b[ XOR( "target_limit" ) ] ) {
		if( m_targets.size( ) >= g_hooks.f[ XOR( "target_limit_f" ) ] ) {
			auto first = rand( ) % m_targets.size( );
			auto second = rand( ) % m_targets.size( );
			auto third = rand( ) % m_targets.size( );
			auto fourthly = rand( ) % m_targets.size( );
			auto fifth = rand( ) % m_targets.size( );
			auto sixth = rand( ) % m_targets.size( );
			auto seventh = rand( ) % m_targets.size( );
			auto eighth = rand( ) % m_targets.size( );
			auto ninth = rand( ) % m_targets.size( );
			auto tenth = rand( ) % m_targets.size( );

			switch( int( g_hooks.f[ XOR( "target_limit_f" ) ] ) ) {
				case 1:
					for( auto i = 0; i < m_targets.size( ); ++i ) {
						if( i == first )
							continue;

						m_targets.erase( m_targets.begin( ) + i );

						if( i > 0 )
							--i;
					}
					break;

				case 2:
					for( auto i = 0; i < m_targets.size( ); ++i ) {
						if( i == first || i == second )
							continue;

						m_targets.erase( m_targets.begin( ) + i );

						if( i > 0 )
							--i;
					}
					break;

				case 3:
					for( auto i = 0; i < m_targets.size( ); ++i ) {
						if( i == first || i == second || i == third )
							continue;

						m_targets.erase( m_targets.begin( ) + i );

						if( i > 0 )
							--i;
					}
					break;

				case 4:
					for( auto i = 0; i < m_targets.size( ); ++i ) {
						if( i == first || i == second || i == third || i == fourthly )
							continue;

						m_targets.erase( m_targets.begin( ) + i );

						if( i > 0 )
							--i;
					}
					break;

				case 5:
					for( auto i = 0; i < m_targets.size( ); ++i ) {
						if( i == first || i == second || i == third || i == fourthly || i == fifth )
							continue;

						m_targets.erase( m_targets.begin( ) + i );

						if( i > 0 )
							--i;
					}
					break;

				case 6:
					for( auto i = 0; i < m_targets.size( ); ++i ) {
						if( i == first || i == second || i == third || i == fourthly || i == fifth || i == sixth )
							continue;

						m_targets.erase( m_targets.begin( ) + i );

						if( i > 0 )
							--i;
					}
					break;

				case 7:
					for( auto i = 0; i < m_targets.size( ); ++i ) {
						if( i == first || i == second || i == third || i == fourthly || i == fifth || i == sixth || i == seventh )
							continue;

						m_targets.erase( m_targets.begin( ) + i );

						if( i > 0 )
							--i;
					}
					break;

				case 8:
					for( auto i = 0; i < m_targets.size( ); ++i ) {
						if( i == first || i == second || i == third || i == fourthly || i == fifth || i == sixth || i == seventh || i == eighth )
							continue;

						m_targets.erase( m_targets.begin( ) + i );

						if( i > 0 )
							--i;
					}
					break;

				case 9:
					for( auto i = 0; i < m_targets.size( ); ++i ) {
						if( i == first || i == second || i == third || i == fourthly || i == fifth || i == sixth || i == seventh || i == eighth || i == ninth )
							continue;

						m_targets.erase( m_targets.begin( ) + i );

						if( i > 0 )
							--i;
					}
					break;

				case 10:
					for( auto i = 0; i < m_targets.size( ); ++i ) {
						if( i == first || i == second || i == third || i == fourthly || i == fifth || i == sixth || i == seventh || i == eighth || i == ninth || i == tenth )
							continue;

						m_targets.erase( m_targets.begin( ) + i );

						if( i > 0 )
							--i;
					}
					break;

				default:
					break;
			}
		}
	}

	// run knifebot.
	if( g_cl.m_weapon_type == WEAPONTYPE_KNIFE && g_cl.m_weapon_id != ZEUS ) {
		// no point in aimbotting if we cannot fire this tick.
		if( !g_cl.m_weapon_fire )
			return;

		if( g_hooks.b[ XOR( "knife_bot" ) ] )
			knife( );

		return;
	}

	// scan available targets... if we even have any.
	find( );

	if( between_shots ) {
		// no point in aimbotting if we cannot fire this tick.
		if( !g_cl.m_weapon_fire )
			return;
	}

	// finally set data when shooting.
	apply( );
}

void Aimbot::find( ) {
	struct BestTarget_t { Player* player; vec3_t pos; float damage; LagRecord* record; int index; };

	vec3_t       tmp_pos;
	float        tmp_damage;
	BestTarget_t best;
	best.player = nullptr;
	best.damage = -1.f;
	best.pos = vec3_t{};
	best.index = -1;
	best.record = nullptr;

	if( m_targets.empty( ) )
		return;

	if( g_cl.m_weapon_id == ZEUS && !g_hooks.b[ XOR( "zeus_bot" ) ] )
		return;

	// iterate all targets.
	for( const auto& t : m_targets ) {
		if( t->m_records.size( ) <= 0 )
			continue;

		int hitbox;

		if (g_hooks.i[XOR("lagfix_mide")] == 2 && t->m_records.front().get()->m_broke_lc)
			continue;

		// this player broke lagcomp.
		// his bones have been resetup by our lagcomp.
		// therfore now only the front record is valid.
		if( g_hooks.i[ XOR( "lagfix_mide" ) ] > 0 && ( g_lagcomp.StartPrediction( t ) || t->m_records.front( ).get( )->m_invalid || g_csgo.m_cvar->FindVar( HASH( "cl_lagcompensation" ) )->GetInt( ) == 0 ) ) {
			LagRecord* front = t->m_records.front( ).get( );
	

			t->SetupHitboxes( front, false );
			if( t->m_hitboxes.empty( ) )
				continue;

			// rip something went wrong..
			if( t->GetBestAimPosition( tmp_pos, hitbox, tmp_damage, front ) && SelectTarget( front, tmp_pos, tmp_damage ) ) {

				// if we made it so far, set shit.
				best.player = t->m_player;
				best.pos = tmp_pos;
				best.damage = tmp_damage;
				best.index = hitbox;
				best.record = front;
				break;
			}
		}

		// player did not break lagcomp.
		// history aim is possible at this point.
		else {

			// bro's going out of sync due to latency while breaking lc
			// fuck this dude
			if (t->m_records.front().get()->m_broke_lc)
				continue;

			if( g_hooks.i[ XOR( "accuracy_boost" ) ] == 4 ) {
				if( t->m_records.empty( ) || !t )
					continue;

				AimPlayer::records_t target_records{ t->m_records };

				target_records.clear();
				target_records.push_front(t->m_records.back());

				// fixed that func to return the right type //
				auto ideal{ g_resolver.FindIdealRecord_(t) };


				if (t->m_records.size() > 5)
					target_records.push_front(t->m_records[t->m_records.size()/2]);

				// we dont wanna push the same record
				if (ideal && ideal != t->m_records.front())
					target_records.push_front(ideal);

				target_records.push_front(t->m_records.front());
				
				int sh = 0;

				// iterate records.
				for( const auto& it : target_records) {
					++sh;
					
					LagRecord* cur = it.get();

					// ghetto but works!
					if (cur->dormant() || cur->immune() || !cur->valid()) {

						if (sh == 1)
							cur = g_resolver.FindLastRecord(t);

						if (cur->dormant() || cur->immune() || !cur->valid()) 
							continue;
					}

					t->SetupHitboxes(cur, false );
					if( t->m_hitboxes.empty( ) )
						continue;

					// try to select best record as target.
					if( t->GetBestAimPosition( tmp_pos, hitbox, tmp_damage, cur) && SelectTarget(cur, tmp_pos, tmp_damage ) ) {
						// if we made it so far, set shit.
						best.player = t->m_player;
						best.pos = tmp_pos;
						best.damage = tmp_damage;
						best.record = cur;
						best.index = hitbox;
						break;
					}
				}
			}
			else if( g_hooks.i[ XOR( "accuracy_boost" ) ] == 3 ) {
				LagRecord* ideal = g_resolver.FindIdealRecord( t );
				if( !ideal )
					continue;

				t->SetupHitboxes( ideal, false );
				if( t->m_hitboxes.empty( ) )
					continue;

				// try to select best record as target.
				if( t->GetBestAimPosition( tmp_pos, hitbox, tmp_damage, ideal ) && SelectTarget( ideal, tmp_pos, tmp_damage ) ) {
					// if we made it so far, set shit.
					best.player = t->m_player;
					best.pos = tmp_pos;
					best.damage = tmp_damage;
					best.record = ideal;
					best.index = hitbox;
					break;
				}

				LagRecord* middle = g_resolver.FindIdealRecord( t );
				if( !middle || middle == ideal )
					continue;

				t->SetupHitboxes( middle, true );
				if( t->m_hitboxes.empty( ) )
					continue;

				// rip something went wrong..
				if( t->GetBestAimPosition( tmp_pos, hitbox, tmp_damage, middle ) && SelectTarget( middle, tmp_pos, tmp_damage ) ) {
					// if we made it so far, set shit.
					best.player = t->m_player;
					best.pos = tmp_pos;
					best.damage = tmp_damage;
					best.record = middle;
					best.index = hitbox;
					break;
				}

				LagRecord* last = g_resolver.FindLastRecord( t );
				if( !last || last == ideal || last == middle )
					continue;

				t->SetupHitboxes( last, true );
				if( t->m_hitboxes.empty( ) )
					continue;

				// rip something went wrong..
				if( t->GetBestAimPosition( tmp_pos, hitbox, tmp_damage, last ) && SelectTarget( last, tmp_pos, tmp_damage ) ) {
					// if we made it so far, set shit.
					best.player = t->m_player;
					best.pos = tmp_pos;
					best.damage = tmp_damage;
					best.record = last;
					best.index = hitbox;
					break;
				}
			}
			else { 
				LagRecord* ideal = g_resolver.FindIdealRecord( t );
				if( !ideal )
					continue;

				t->SetupHitboxes( ideal, false );
				if( t->m_hitboxes.empty( ) )
					continue;

				// try to select best record as target.
				if( t->GetBestAimPosition( tmp_pos, hitbox, tmp_damage, ideal ) && SelectTarget( ideal, tmp_pos, tmp_damage ) ) {
					// if we made it so far, set shit.
					best.player = t->m_player;
					best.pos = tmp_pos;
					best.damage = tmp_damage;
					best.record = ideal;
					best.index = hitbox;
					break;
				}

				LagRecord* last = g_resolver.FindLastRecord( t );
				if( !last || last == ideal )
					continue;

				t->SetupHitboxes( last, true );
				if( t->m_hitboxes.empty( ) )
					continue;

				// rip something went wrong..
				if( t->GetBestAimPosition( tmp_pos, hitbox, tmp_damage, last ) && SelectTarget( last, tmp_pos, tmp_damage ) ) {
					// if we made it so far, set shit.
					best.player = t->m_player;
					best.pos = tmp_pos;
					best.damage = tmp_damage;
					best.record = last;
					best.index = hitbox;
					break;
				}
			}
		}
	}

	// verify our target and set needed data.
	if( best.player && best.record ) {
		// calculate aim angle.
		math::VectorAngles( best.pos - g_cl.m_shoot_pos, m_angle );

		// set member vars.
		m_target = best.player;
		m_aim = best.pos;
		m_damage = best.damage;
		m_record = best.record;

		//if( !g_aimbot.AdjustVelocity( ) )
		//	return;

		// write data, needed for traces / etc.
		m_record->cache( );

		// set autostop shit.
		m_stop = !( g_cl.m_buttons & IN_JUMP );

		bool on = g_hooks.b[ XOR( "hitchance" ) ] && g_hooks.b[ XOR( "antiUntrusted" ) ];

		if( g_cl.m_weapon_type == WEAPONTYPE_PISTOL && g_cl.m_weapon_id != REVOLVER )
			on = g_hooks.b[ XOR( "hitchance_pistols" ) ] && g_hooks.b[ XOR( "antiUntrusted" ) ];

		switch( g_cl.m_weapon_id ) {
			case SSG08:
				on = g_hooks.b[ XOR( "hitchance_scout" ) ] && g_hooks.b[ XOR( "antiUntrusted" ) ];
				break;

			case AWP:
				on = g_hooks.b[ XOR( "hitchance_awp" ) ] && g_hooks.b[ XOR( "antiUntrusted" ) ];
				break;

			case DEAGLE:
				on = g_hooks.b[ XOR( "hitchance_deagle" ) ] && g_hooks.b[ XOR( "antiUntrusted" ) ];
				break;

			case REVOLVER:
				on = g_hooks.b[ XOR( "hitchance_revolver" ) ] && g_hooks.b[ XOR( "antiUntrusted" ) ];
				break;

			default:
				break;
		}

		bool hit = ( !g_cl.m_ground && g_cl.m_weapon_id == SSG08 && g_cl.m_weapon && g_cl.m_weapon->GetInaccuracy( ) < 0.009f ) || ( on && CheckHitchance( m_target, best.index, m_angle ) );

		if( g_cl.m_weapon_id == REVOLVER )
			hit = ( !g_cl.m_ground && g_cl.m_weapon_id == SSG08 && g_cl.m_weapon && g_cl.m_weapon->GetInaccuracy( ) < 0.009f ) || ( on && CheckHitchance( m_target, best.index, m_angle ) );

		// if we can scope.
		bool can_scope = !g_cl.m_local->m_bIsScoped( ) && ( g_cl.m_weapon_id == AUG || g_cl.m_weapon_id == SG553 || g_cl.m_weapon_type == WEAPONTYPE_SNIPER_RIFLE );

		if( can_scope ) {

			// always.
			if( g_hooks.b[ XOR( "auto_scope" ) ] ) {
				g_cl.m_cmd->m_buttons |= IN_ATTACK2;
				return;
			}

			// hitchance fail.
			else if( g_menu.main.aimbot.zoom.get( ) == 2 && on && !hit ) {
				g_cl.m_cmd->m_buttons |= IN_ATTACK2;
				return;
			}
		}

		if( hit || !on ) {
			// right click attack.
			if( !g_hooks.b[ XOR( "antiUntrusted" ) ] && g_cl.m_weapon_id == REVOLVER )
				g_cl.m_cmd->m_buttons |= IN_ATTACK2;

			// left click attack.
			else if( g_cl.CanFireWeapon( game::TICKS_TO_TIME( g_cl.m_local->m_nTickBase( ) ) ) )
				g_cl.m_cmd->m_buttons |= IN_ATTACK;
		}
	}
}

static std::vector<std::tuple<float, float, float>> precomputed_seeds = {};
static const int total_seeds = 255;

void build_seed_table( ) {
	if( !precomputed_seeds.empty( ) )
		return;

	for( auto i = 0; i < total_seeds; i++ ) {
		g_csgo.RandomSeed( i + 1 );

		const auto pi_seed = g_csgo.RandomFloat( 0.f, math::pi * 2 );

		precomputed_seeds.emplace_back( g_csgo.RandomFloat( 0.f, 1.f ),
										sin( pi_seed ), cos( pi_seed ) );
	}
}

int Aimbot::CheckHitchance( Player* player, const ang_t& angle ) {
	build_seed_table( );

	const auto weapon = g_cl.m_weapon;
	if( !weapon )
		return false;

	const auto info = weapon->GetWpnData( );
	if( !info )
		return false;

	float chance = g_hooks.f[ XOR( "hitchance_amount" ) ] / 100.f;
	bool accuracy = g_hooks.b[ XOR( "force_accuracy" ) ];

	if( g_cl.m_weapon_type == WEAPONTYPE_PISTOL && g_cl.m_weapon_id != REVOLVER ) {
		chance = g_hooks.f[ XOR( "hitchance_amount_pistols" ) ] / 100.f;
		accuracy = g_hooks.b[ XOR( "force_accuracy_pistols" ) ];
	}

	switch( g_cl.m_weapon_id ) {
		case SSG08:
			chance = g_hooks.f[ XOR( "hitchance_amount_scout" ) ] / 100.f;
			accuracy = g_hooks.b[ XOR( "force_accuracy_scout" ) ];
			break;

		case AWP:
			chance = g_hooks.f[ XOR( "hitchance_amount_awp" ) ] / 100.f;
			accuracy = g_hooks.b[ XOR( "force_accuracy_awp" ) ];
			break;

		case DEAGLE:
			chance = g_hooks.f[ XOR( "hitchance_amount_deagle" ) ] / 100.f;
			accuracy = g_hooks.b[ XOR( "force_accuracy_deagle" ) ];
			break;

		case REVOLVER:
			chance = g_hooks.f[ XOR( "hitchance_amount_revolver" ) ] / 100.f;
			accuracy = g_hooks.b[ XOR( "force_accuracy_revolver" ) ];
			break;

		case ZEUS:
			chance = g_hooks.f[ XOR( "zeusbot_hc" ) ] / 100.f;
			break;

		default:
			break;
	}

	CGameTrace tr;
	// setup calculation parameters.
	const auto round_acc = [ ]( const float accuracy ) { return roundf( accuracy * 1000.f ) / 1000.f; };
	const auto sniper = weapon->m_iItemDefinitionIndex( ) == AWP || weapon->m_iItemDefinitionIndex( ) == G3SG1
		|| weapon->m_iItemDefinitionIndex( ) == SCAR20 || weapon->m_iItemDefinitionIndex( ) == SSG08;
	const auto crouched = g_cl.m_local->m_fFlags( ) & FL_DUCKING;;

	// calculate inaccuracy.
	const auto weapon_inaccuracy = weapon->GetInaccuracy( );

	if( weapon->m_iItemDefinitionIndex( ) == REVOLVER )
		return weapon_inaccuracy < ( crouched ? .0020f : .0055f );

	// no need for hitchance, if we can't increase it anyway.
	if( crouched ) {
		if( round_acc( weapon_inaccuracy ) == round_acc( sniper ? info->m_inaccuracy_crouch_alt : info->m_inaccuracy_crouch ) )
			return true;
	}
	else {
		if( round_acc( weapon_inaccuracy ) == round_acc( sniper ? info->m_inaccuracy_stand_alt : info->m_inaccuracy_stand ) )
			return true;
	}

	// calculate start and angle
	vec3_t forward, right, up;
	math::AngleVectors( angle, &forward, &right, &up );

	// keep track of all traces that hit the enemy.
	auto current = 0;

	// setup calculation parameters.
	vec3_t total_spread, spread_angle, end;
	float inaccuracy, spread_x, spread_y;
	std::tuple<float, float, float>* seed;

	float effective_range = ( weapon_inaccuracy + weapon->GetSpread( ) ) > 0.00001f ? ( 6 / ( weapon_inaccuracy + weapon->GetSpread( ) ) ) : 100000.0f;

	// final accuracy
	float final_accuracy = weapon_inaccuracy + weapon->GetSpread( );

	float dist = ( tr.m_fraction != 1.0f ) ? ( tr.m_startpos - tr.m_endpos ).length( ) : 0;

	// use look-up-table to find average hit probability.
	for( auto i = 0u; i < total_seeds; i++ )  // NOLINT(modernize-loop-convert)
	{
		// get seed.
		seed = &precomputed_seeds[ i ];

		// calculate spread.
		inaccuracy = std::get<0>( *seed ) * weapon_inaccuracy;
		spread_x = std::get<2>( *seed ) * inaccuracy;
		spread_y = std::get<1>( *seed ) * inaccuracy;
		total_spread = ( forward + right * spread_x + up * spread_y ).normalized( );

		// get end of trace.
		end = g_cl.m_shoot_pos + ( total_spread * g_cl.m_weapon_info->m_range );

		// setup ray and trace.
		g_csgo.m_engine_trace->ClipRayToEntity( Ray( g_cl.m_shoot_pos, end ), MASK_SHOT, player, &tr );

		// did we hit the hitbox?
		if( tr.m_entity == player && game::IsValidHitgroup( tr.m_hitgroup ) )
			current++;

		// abort if hitchance is already sufficent.
		if( static_cast< float >( current ) / static_cast< float >( total_seeds ) >= chance )
			return true;

		// force accuracy.
		if( static_cast< float >( current + total_seeds - i ) / static_cast< float >( total_seeds ) < chance && accuracy )
			return false;

		if( final_accuracy < 0.00001f )
			return false;
	}

	return static_cast< float >( current ) / static_cast< float >( total_seeds ) >= chance;
}

/*bool Aimbot::CheckHitchance( Player* player, int hitbox, const ang_t& angle ) {
	constexpr float HITCHANCE_MAX = 100.f;
	constexpr int   SEED_MAX = 255;

	vec3_t     start{ g_cl.m_shoot_pos }, end, fwd, right, up, dir, wep_spread;
	float      weapon_inaccuracy, final_inaccuracy, weapon_spread, final_spread;
	CGameTrace tr;
	size_t     total_hits{ }, needed_hits{ ( size_t )std::ceil( ( ( g_cl.m_weapon_id == ZEUS ? 60.f : g_menu.main.aimbot.hitchance_amount.get( ) ) * SEED_MAX ) / HITCHANCE_MAX ) };

	// store off inaccuracy / spread ( these functions are quite intensive and we only need them once ).
	weapon_inaccuracy = g_cl.m_weapon->GetInaccuracy( );
	weapon_spread = g_cl.m_weapon->GetSpread( );

	// setup calculation parameters.
	const auto round_acc = [ ]( const float accuracy ) {
		return roundf( accuracy * 1000.f ) / 1000.f;
	};

	const auto weapon = g_cl.m_weapon;
	if( !weapon )
		return false;

	const auto sniper = weapon->m_iItemDefinitionIndex( ) == AWP || weapon->m_iItemDefinitionIndex( ) == G3SG1
		|| weapon->m_iItemDefinitionIndex( ) == SCAR20 || weapon->m_iItemDefinitionIndex( ) == SSG08;

	// no need for hitchance, if we can't increase it anyway.
	if( g_cl.m_local->m_fFlags( ) & FL_DUCKING ) {
		if( round_acc( weapon_inaccuracy ) == round_acc( sniper ? g_cl.m_weapon_info->m_inaccuracy_crouch_alt : g_cl.m_weapon_info->m_inaccuracy_crouch ) )
			return true;
	}
	else if( round_acc( weapon_inaccuracy ) == round_acc( sniper ? g_cl.m_weapon_info->m_inaccuracy_stand_alt : g_cl.m_weapon_info->m_inaccuracy_stand ) )
		return true;

	// get needed directional vectors.
	math::AngleVectors( angle, &fwd, &right, &up );

	// iterate all possible seeds.
	for( int i{ }; i <= SEED_MAX; ++i ) {
		g_csgo.RandomSeed( i + 1 );
		float a = g_csgo.RandomFloat( 0.f, 1.f );
		float b = g_csgo.RandomFloat( 0.f, math::pi_2 );
		float c = g_csgo.RandomFloat( 0.f, 1.f );
		float d = g_csgo.RandomFloat( 0.f, math::pi_2 );

		final_inaccuracy = a * weapon_inaccuracy;
		final_spread = c * weapon_spread;

		if( g_cl.m_weapon_id == REVOLVER ) {
			a = 1.f - a * a;
			a = 1.f - c * c;
		}

		// get spread.
		wep_spread = vec3_t( ( cos( b ) * final_inaccuracy ) + ( cos( d ) * final_spread ), ( sin( b ) * final_inaccuracy ) + ( sin( d ) * final_spread ), 0 );

		// get spread direction.
		dir = ( fwd + ( right * wep_spread.x ) + ( up * wep_spread.y ) ).normalized( );

		// get end of trace.
		end = start + ( dir * g_cl.m_weapon_info->m_range );

		// setup ray and trace.
		g_csgo.m_engine_trace->ClipRayToEntity( Ray( start, end ), MASK_SHOT, player, &tr );

		// check if we hit a valid player / hitgroup on the player and increment total hits.
		if( tr.m_entity == player && game::IsValidHitgroup( tr.m_hitgroup ) )
			++total_hits;

		// we made it.
		if( total_hits >= needed_hits )
			return true;

		// we cant make it anymore.
		if( ( SEED_MAX - i + total_hits ) < needed_hits )
			return false;
	}

	return false;
}*/

bool Aimbot::CheckHitchance( Player* player, int hitbox, const ang_t& angle ) {
	float chance = g_hooks.f[ XOR( "hitchance_amount" ) ];

	if( g_cl.m_weapon_type == WEAPONTYPE_PISTOL && g_cl.m_weapon_id != REVOLVER ) {
		chance = g_hooks.f[ XOR( "hitchance_amount_pistols" ) ];
	}

	switch( g_cl.m_weapon_id ) {
		case SSG08:
			chance = g_hooks.f[ XOR( "hitchance_amount_scout" ) ];
			break;

		case AWP:
			chance = g_hooks.f[ XOR( "hitchance_amount_awp" ) ];
			break;

		case DEAGLE:
			chance = g_hooks.f[ XOR( "hitchance_amount_deagle" ) ];
			break;

		case REVOLVER:
			chance = g_hooks.f[ XOR( "hitchance_amount_revolver" ) ];
			break;

		case ZEUS:
			chance = g_hooks.f[ XOR( "zeusbot_hc" ) ];
			break;

		default:
			break;
	}

	constexpr float HITCHANCE_MAX = 100.f;
	constexpr int   SEED_MAX = 128;

	vec3_t     start{ g_cl.m_shoot_pos }, end, fwd, right, up, dir, wep_spread;
	float      inaccuracy, spread;
	CGameTrace tr;
	int     total_hits{}, needed_hits{ int( float( chance / HITCHANCE_MAX ) * SEED_MAX ) };

	// get needed directional vectors.
	math::AngleVectors( angle, &fwd, &right, &up );

	// store off inaccuracy / spread ( these functions are quite intensive and we only need them once ).
	inaccuracy = g_cl.m_weapon->GetInaccuracy( );
	spread = g_cl.m_weapon->GetSpread( );

	if( ( g_inputpred.m_perfect_accuracy + 0.0005f ) >= inaccuracy )
		return true;

	penetration::PenetrationInput_t in;

	in.m_damage = 1.f;
	in.m_damage_pen = 1.f;
	in.m_can_pen = true;
	in.m_target = player;
	in.m_from = g_cl.m_local;

	penetration::PenetrationOutput_t out;

	int autowalls_done = 0;
	int autowalls_hit = 0;

	// iterate all possible seeds.
	for( int i{}; i <= SEED_MAX; ++i ) {
		// get spread.
		wep_spread = g_cl.m_weapon->CalculateSpread( m_static_seeds[ i ], inaccuracy, spread );

		// get spread direction.
		dir = ( fwd + ( right * wep_spread.x ) + ( up * wep_spread.y ) ).normalized( );

		// get end of trace.
		end = start + ( dir * g_cl.m_weapon_info->m_range );

		// setup ray and trace.
		g_csgo.m_engine_trace->ClipRayToEntity( Ray( start, end ), MASK_SHOT_HULL | CONTENTS_HITBOX, player, &tr );

		// check if we hit a valid player / hitgroup on the player and increment total hits.
		if( tr.m_entity == player && game::IsValidHitgroup( tr.m_hitgroup ) ) {
			++total_hits;
		}

		// we cant make it anymore.
		if( ( SEED_MAX - i + total_hits ) < needed_hits )
			break;
	}

	if( total_hits >= needed_hits ) {
		return true;
	}

	return false;
}

bool AimPlayer::SetupHitboxPoints( LagRecord* record, matrix3x4_t* bones, int index, std::vector< vec3_t >& points ) {
	// reset points.
	points.clear( );

	const model_t* model = m_player->GetModel( );
	if( !model )
		return false;

	studiohdr_t* hdr = g_csgo.m_model_info->GetStudioModel( model );
	if( !hdr )
		return false;

	mstudiohitboxset_t* set = hdr->GetHitboxSet( m_player->m_nHitboxSet( ) );
	if( !set )
		return false;

	mstudiobbox_t* bbox = set->GetHitbox( index );
	if( !bbox )
		return false;

	// get hitbox scales.
	float scale = g_hooks.f[ XOR( "scale" ) ] / 100.f;
	float bscale = g_hooks.f[ XOR( "body_scale" ) ] / 100.f;
	auto final_radius = bbox->m_radius * scale;
	std::unordered_map<int, bool> multipoint = g_hooks.m[ XOR( "multipoint" ) ];

	if( g_cl.m_weapon_type == WEAPONTYPE_PISTOL && g_cl.m_weapon_id != REVOLVER ) {
		scale = g_hooks.f[ XOR( "scale_pistols" ) ] / 100.f;
		bscale = g_hooks.f[ XOR( "body_scale_pistols" ) ] / 100.f;
		multipoint = g_hooks.m[ XOR( "multipoint_pistols" ) ];
	}

	switch( g_cl.m_weapon_id ) {
		case SSG08:
			multipoint = g_hooks.m[ XOR( "multipoint_scout" ) ];
			bscale = g_hooks.f[ XOR( "body_scale_scout" ) ] / 100.f;
			scale = g_hooks.f[ XOR( "scale_scout" ) ] / 100.f;
			break;

		case AWP:
			scale = g_hooks.f[ XOR( "scale_awp" ) ] / 100.f;
			bscale = g_hooks.f[ XOR( "body_scale_awp" ) ] / 100.f;
			multipoint = g_hooks.m[ XOR( "multipoint_awp" ) ];
			break;

		case DEAGLE:
			scale = g_hooks.f[ XOR( "scale_deagle" ) ] / 100.f;
			bscale = g_hooks.f[ XOR( "body_scale_deagle" ) ] / 100.f;
			multipoint = g_hooks.m[ XOR( "multipoint_deagle" ) ];
			break;

		case REVOLVER:
			scale = g_hooks.f[ XOR( "scale_revolver" ) ] / 100.f;
			bscale = g_hooks.f[ XOR( "body_scale_revolver" ) ] / 100.f;
			multipoint = g_hooks.m[ XOR( "multipoint_revolver" ) ];
			break;

		default:
			break;
	}

	// these indexes represent boxes.
	if( bbox->m_radius <= 0.f ) {
		// references: 
		//      https://developer.valvesoftware.com/wiki/Rotation_Tutorial
		//      CBaseAnimating::GetHitboxBonePosition
		//      CBaseAnimating::DrawServerHitboxes

		// convert rotation angle to a matrix.
		matrix3x4_t rot_matrix;
		g_csgo.AngleMatrix( bbox->m_angle, rot_matrix );

		// apply the rotation to the entity input space (local).
		matrix3x4_t matrix;
		math::ConcatTransforms( bones[ bbox->m_bone ], rot_matrix, matrix );

		// extract origin from matrix.
		vec3_t origin = matrix.GetOrigin( );

		// compute raw center point.
		vec3_t center = ( bbox->m_mins + bbox->m_maxs ) / 2.f;

		// nothing to do here we are done.
		if (points.empty())
			return false;

		// rotate our bbox points by their correct angle
		// and convert our points to world space.
		for (auto& p : points) {
			// VectorRotate.
			// rotate point by angle stored in matrix.
			p = { p.dot(matrix[0]), p.dot(matrix[1]), p.dot(matrix[2]) };

			// transform point to world space.
			p += origin;
		}
	}

	// these hitboxes are capsules.
	else {
		// factor in the pointscale.
		float r = bbox->m_radius * scale;
		float br = bbox->m_radius * bscale;

		vec3_t center = ( bbox->m_mins + bbox->m_maxs ) / 2.f;

		if( index == HITBOX_HEAD ) {
			points.push_back( center );

			if( multipoint[ 0 ] ) {
				// rotation matrix 45 degrees.
					// https://math.stackexchange.com/questions/383321/rotating-x-y-points-45-degrees
					// std::cos( deg_to_rad( 45.f ) )
				constexpr float rotation = 0.70710678f;

				// top/back 45 deg.
				// this is the best spot to shoot at.
				points.push_back({ bbox->m_maxs.x + (rotation * r), bbox->m_maxs.y + (-rotation * r), bbox->m_maxs.z });

				// right.
				points.push_back({ bbox->m_maxs.x, bbox->m_maxs.y, bbox->m_maxs.z + r });

				// left.
				points.push_back({ bbox->m_maxs.x, bbox->m_maxs.y, bbox->m_maxs.z - r });

				// back.
				points.push_back({ bbox->m_maxs.x, bbox->m_maxs.y - r, bbox->m_maxs.z });

				// get animstate ptr.
				CCSGOPlayerAnimState* state = record->m_player->m_PlayerAnimState();

				// add this point only under really specific circumstances.  THIS IS SILLY LOL -kiera.
				// if we are standing still and have the lowest possible pitch pose.
				if (state && record->m_anim_velocity.length() <= 0.1f && record->m_eye_angles.x <= state->m_min_pitch) {

					// bottom point.
					points.push_back({ bbox->m_maxs.x - r, bbox->m_maxs.y, bbox->m_maxs.z });
				}
			}
		}

		else if( index == HITBOX_BODY ) {
			points.push_back( center );

			if( multipoint[ 2 ] )
				points.push_back( { center.x, bbox->m_maxs.y - br, center.z } );
		}

		else if( index == HITBOX_PELVIS || index == HITBOX_UPPER_CHEST ) {
			points.push_back( { center.x, bbox->m_maxs.y - r, center.z } );
		}

		else if( index == HITBOX_THORAX || index == HITBOX_CHEST ) {
			points.push_back( center );

			if (multipoint[1])
			{
				points.push_back({ bbox->m_maxs.x, bbox->m_maxs.y, bbox->m_maxs.z + final_radius });
				points.push_back({ bbox->m_maxs.x, bbox->m_maxs.y, bbox->m_maxs.z - final_radius });
				points.push_back({ center.x, bbox->m_maxs.y - final_radius, center.z });
			}
		}

		else if( index == HITBOX_R_CALF || index == HITBOX_L_CALF ) {
			points.push_back( center );

			if( multipoint[ 3 ] )
				points.push_back({ bbox->m_maxs.x - (bbox->m_radius / 2.f), bbox->m_maxs.y, bbox->m_maxs.z });
		}

		else if( index == HITBOX_R_THIGH || index == HITBOX_L_THIGH ) {
			points.push_back( center );
		}

		else if( index == HITBOX_R_UPPER_ARM || index == HITBOX_L_UPPER_ARM ) {
			points.push_back( { bbox->m_maxs.x + bbox->m_radius, center.y, center.z } );
		}

		if( points.empty( ) )
			return false;

		for( auto& p : points )
			math::VectorTransform( p, bones[ bbox->m_bone ], p );
	}

	return true;
}

bool AimPlayer::GetBestAimPosition( vec3_t& aim, int& hitbox_id, float& damage, LagRecord* record ) {
	bool                  done, pen, m_scale, m_pen;
	float                 dmg, pendmg, m_dmg;
	HitscanData_t         scan;
	std::vector< vec3_t > points;

	int hp = std::min( 100, m_player->m_iHealth( ) );

	m_dmg = g_hooks.f[ XOR( "minimal_damage" ) ];
	m_pen = g_hooks.b[ XOR( "penetrate" ) ];
	m_scale = g_hooks.b[ XOR( "minimal_damage_on_hp" ) ];

	if( g_cl.m_weapon_type == WEAPONTYPE_PISTOL && g_cl.m_weapon_id != REVOLVER ) {
		m_dmg = g_hooks.f[ XOR( "minimal_damage_pistols" ) ];
		m_pen = g_hooks.b[ XOR( "penetrate_pistols" ) ];
		m_scale = g_hooks.b[ XOR( "minimal_damage_on_hp_pistols" ) ];
	}

	switch( g_cl.m_weapon_id ) {
		case SSG08:
			m_dmg = g_hooks.f[ XOR( "minimal_damage_scout" ) ];
			m_pen = g_hooks.b[ XOR( "penetrate_scout" ) ];
			m_scale = g_hooks.b[ XOR( "minimal_damage_on_hp_scout" ) ];
			break;

		case AWP:
			m_dmg = g_hooks.f[ XOR( "minimal_damage_awp" ) ];
			m_pen = g_hooks.b[ XOR( "penetrate_awp" ) ];
			m_scale = g_hooks.b[ XOR( "minimal_damage_on_hp_awp" ) ];
			break;

		case DEAGLE:
			m_dmg = g_hooks.f[ XOR( "minimal_damage_deagle" ) ];
			m_pen = g_hooks.b[ XOR( "penetrate_deagle" ) ];
			m_scale = g_hooks.b[ XOR( "minimal_damage_on_hp_deagle" ) ];
			break;

		case REVOLVER:
			m_dmg = g_hooks.f[ XOR( "minimal_damage_revolver" ) ];
			m_pen = g_hooks.b[ XOR( "penetrate_revolver" ) ];
			m_scale = g_hooks.b[ XOR( "minimal_damage_on_hp_revolver" ) ];
			break;

		default:
			break;
	}

	if( g_cl.m_weapon_id == ZEUS ) {
		dmg = pendmg = hp;
		pen = false;
	}

	else {
		if( g_hooks.auto_check( "damage_key" ) && g_hooks.b[ XOR( "override_hp" ) ] ) {
			dmg = g_hooks.f[ XOR( "baim_hp_override" ) ];
			pendmg = g_hooks.f[ XOR( "baim_hp_override" ) ];
			pen = m_pen;
		}
		else {
			if( m_scale ) {
				dmg = std::ceil( ( ( m_dmg ) / 100.f ) * hp );
				pendmg = std::ceil( ( ( m_dmg ) / 100.f ) * hp );
			}
			else {
				dmg = m_dmg;
				pendmg = m_dmg;
			}

			if( m_dmg > 100 ) {
				dmg = hp + ( m_dmg - 100 );
				pendmg = hp + ( m_dmg - 100 );
			}

			pen = m_pen;
		}
	}

	// write all data of this record l0l.
	record->cache( );

	// iterate hitboxes.
	for( const auto& it : m_hitboxes ) {
		done = false;
		bool hittable_center_on_this_hb = false;

		// setup points on hitbox.
		if( !SetupHitboxPoints( record, record->m_bones, it.m_index, points ) )
			continue;

		// iterate points on hitbox.
		for( const auto& point : points ) {
			penetration::PenetrationInput_t in;

			in.m_damage = dmg;
			in.m_damage_pen = pendmg;
			in.m_can_pen = pen;
			in.m_target = m_player;
			in.m_from = g_cl.m_local;
			in.m_pos = point;

			// ignore mindmg.
			//if( it.m_mode == HitscanMode::LETHAL || it.m_mode == HitscanMode::LETHAL2 )
			//	in.m_damage = in.m_damage_pen = 1.f;

			penetration::PenetrationOutput_t out;

			// we can hit p!
			if( penetration::run( &in, &out ) ) {

				if (point == points.front())
					hittable_center_on_this_hb = true;

				// nope we did not hit head..
				if( it.m_index == HITBOX_HEAD && out.m_hitgroup != HITGROUP_HEAD
					|| it.m_index != HITBOX_HEAD && out.m_hitgroup == HITGROUP_HEAD )
					continue;

				// i knowi shouldnt be doing this but idc
				if (out.m_damage >= m_player->m_iHealth() && it.m_index > 2)
					done = true;

				// prefered hitbox, just stop now.
				else if( it.m_mode == HitscanMode::PREFER )
					done = true;

				// this hitbox requires lethality to get selected, if that is the case.
				// we are done, stop now.
				else if( it.m_mode == HitscanMode::LETHAL && out.m_damage >= m_player->m_iHealth( ) )
					done = true;

				// 2 shots will be sufficient to kill.
				else if( it.m_mode == HitscanMode::LETHAL2 && ( out.m_damage * 2.f ) >= m_player->m_iHealth( ) )
					done = true;

				// this hitbox has normal selection, it needs to have more damage.
				else if( it.m_mode == HitscanMode::NORMAL ) {
					// we did more damage.
					if( out.m_damage >=
						scan.m_damage ) {
						// save new best data.
						scan.m_damage = out.m_damage;
						scan.m_pos = point;
						scan.m_hitbox = it.m_index;

						// if the first point is lethal
						// screw the other ones.
						if((point == points.front() || !hittable_center_on_this_hb) && out.m_damage >= m_player->m_iHealth( ) )
							break;
					}
				}

				// we found a preferred / lethal hitbox.
				if( done ) {
					// save new best data.
					scan.m_damage = out.m_damage;
					scan.m_pos = point;
					scan.m_hitbox = it.m_index;
			
					if ((point == points.front() || !hittable_center_on_this_hb) && out.m_damage >= m_player->m_iHealth())
						break;
				}
			}
		}

		// ghetto break out of outer loop.
		if( done )
			break;
	}

	// we found something that we can damage.
	// set out vars.
	if( scan.m_damage > 0.f ) {
		aim = scan.m_pos;
		damage = scan.m_damage;
		hitbox_id = scan.m_hitbox;
		return true;
	}

	return false;
}

bool Aimbot::SelectTarget( LagRecord* record, const vec3_t& aim, float damage ) {
	float dist, fov, height;
	int   hp;

	switch( g_hooks.i[ XOR( "selection" ) ] ) {

		// distance.
		case 0:
			dist = ( record->m_pred_origin - g_cl.m_shoot_pos ).length_2d( );

			if( dist < m_best_dist ) {
				m_best_dist = dist;
				return true;
			}

			break;

			// crosshair.
		case 1:
			fov = math::GetFOV( g_cl.m_view_angles, g_cl.m_shoot_pos, aim );

			if( fov < m_best_fov ) {
				m_best_fov = fov;
				return true;
			}

			break;

			// damage.
		case 2:
			if( damage > m_best_damage ) {
				m_best_damage = damage;
				return true;
			}

			break;

			// lowest hp.
		case 3:
			// fix for retarded servers?
			hp = std::min( 100, record->m_player->m_iHealth( ) );

			if( hp < m_best_hp ) {
				m_best_hp = hp;
				return true;
			}

			break;

			// least lag.
		case 4:
			if( record->m_lag < m_best_lag ) {
				m_best_lag = record->m_lag;
				return true;
			}

			break;

			// height.
		case 5:
			height = record->m_pred_origin.z - g_cl.m_local->m_vecOrigin( ).z;

			if( height < m_best_height ) {
				m_best_height = height;
				return true;
			}

			break;

		default:
			return false;
	}

	return false;
}
void Aimbot::apply( ) {
	bool attack, attack2;

	// attack states.
	attack = ( g_cl.m_cmd->m_buttons & IN_ATTACK );
	attack2 = ( g_cl.m_weapon_id == REVOLVER && g_cl.m_cmd->m_buttons & IN_ATTACK2 );

	bool silent = g_hooks.b[ XOR( "silent" ) ];

	if( g_cl.m_weapon_type == WEAPONTYPE_PISTOL && g_cl.m_weapon_id != REVOLVER )
		silent = g_hooks.b[ XOR( "silent_pistols" ) ];

	switch( g_cl.m_weapon_id ) {
		case SSG08:
			silent = g_hooks.b[ XOR( "silent_scout" ) ];
			break;

		case AWP:
			silent = g_hooks.b[ XOR( "silent_awp" ) ];
			break;

		case DEAGLE:
			silent = g_hooks.b[ XOR( "silent_deagle" ) ];
			break;

		case REVOLVER:
			silent = g_hooks.b[ XOR( "silent_revolver" ) ];
			break;

		default:
			break;
	}

	// ensure we're attacking.
	if( attack || attack2 ) {
		// choke every shot.
		if( g_hooks.auto_check( XOR( "rapidfire_key" ) ) && g_hooks.b[ XOR( "rapidfire" ) ] )
			*g_cl.m_packet = true;
		else
			*g_cl.m_packet = false;

		if( m_target ) {


			// Apply correct tick_count
			if( m_record )
				g_cl.m_cmd->m_tick = game::TIME_TO_TICKS( m_record->m_sim_time + g_cl.m_lerp );

			// set angles to target.
			g_cl.m_cmd->m_view_angles = m_angle;

			// if not silent aim, apply the viewangles.
			if( !silent )
				g_csgo.m_engine->SetViewAngles( m_angle );

			if( g_hooks.b[ XOR( "bulletImpacts" ) ] ) {
				CGameTrace trace, trace2;
				CTraceFilterSimple filter, filter2;
				filter.SetPassEntity( g_cl.m_local );
				g_csgo.m_engine_trace->TraceRay( Ray{ g_cl.m_shoot_pos, m_aim }, 0x46004003, &filter, &trace );
				g_csgo.m_debug_overlay->AddBoxOverlay( trace.m_endpos, vec3_t( -2.f, -2.f, -2.f ), vec3_t( 2.f, 2.f, 2.f ), ang_t( 0.f, 0.f, 0.f ), 255, 0, 0, 127, 3.f );
				if( ( trace.m_endpos - m_aim ).length_2d( ) > 10.f )
					g_csgo.m_debug_overlay->AddBoxOverlay( m_aim, vec3_t( -2.f, -2.f, -2.f ), vec3_t( 2.f, 2.f, 2.f ), ang_t( 0.f, 0.f, 0.f ), 255, 0, 0, 127, 3.f );
			}

			if( g_hooks.b[ XOR( "debug_aim" ) ] )
				g_visuals.DrawHitboxMatrix( m_record, Color( g_hooks.c[ XOR( "debug_aim_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "debug_aim_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "debug_aim_color" ) ][ 2 ] * 255.f, g_hooks.c[ XOR( "debug_aim_color" ) ][ 3 ] * 255.f ), g_hooks.f[ XOR( "debug_aim_time" ) ] );
		
			// store fired shot.
			g_shots.StoreLastFireData(m_target ? m_target : nullptr, m_target ? m_damage : -1.f, g_cl.m_weapon_info->m_bullets, m_target ? m_record : nullptr, m_hitbox);

		}

		// nospread.
		if( g_hooks.b[ XOR( "nospread" ) ] && !g_hooks.b[ XOR( "antiUntrusted" ) ] )
			NoSpread( );

		// norecoil.
		if( g_hooks.b[ XOR( "norecoil" ) ] )
			g_cl.m_cmd->m_view_angles -= g_cl.m_local->m_aimPunchAngle( ) * g_csgo.weapon_recoil_scale->GetFloat( );


		// set that we fired.
		g_cl.m_shot = true;
	}
}

void Aimbot::NoSpread( ) {
	bool    attack2;
	vec3_t  spread, forward, right, up, dir;

	// revolver state.
	attack2 = ( g_cl.m_weapon_id == REVOLVER && ( g_cl.m_cmd->m_buttons & IN_ATTACK2 ) );

	// get spread.
	spread = g_cl.m_weapon->CalculateSpread( g_cl.m_cmd->m_random_seed, attack2 );

	// compensate.
	g_cl.m_cmd->m_view_angles -= { -math::rad_to_deg( std::atan( spread.length_2d( ) ) ), 0.f, math::rad_to_deg( std::atan2( spread.x, spread.y ) ) };
}

bool CanHitCustom(vec3_t origin, Player* m_player, int m_index) {
	AimPlayer* AimPlayer = &g_aimbot.m_players[m_player->index() - 1];
	LagRecord* front = AimPlayer->m_records.front().get();
	std::vector< vec3_t > points;
	if (!AimPlayer->SetupHitboxPoints(front, front->m_bones, m_index, points))
		return false;

	// iterate points on hitbox.
	for (const auto& point : points) {
		penetration::PenetrationInput_t in;

		in.m_damage = 1;
		in.m_damage_pen = 1;
		in.m_can_pen = true;
		in.m_target = m_player;
		in.m_from = g_cl.m_local;
		in.m_pos = point;

		penetration::PenetrationOutput_t out;
		// we can hit p!
		if (penetration::runcustom(&in, &out, origin)) {
			return true;
		}
	}
	return false;

}

bool Aimbot::CanHit(vec3_t start, vec3_t end, LagRecord* record, int box, bool in_shot, matrix3x4_t* bones, int ent_idx)
{
	if (!record)
		return false;

	if (!record->m_bones)
		return false;

	if (record->m_pred_origin.IsZero())
		return false;

	Player* m_player = g_csgo.m_entlist->GetClientEntity< Player* >(ent_idx);

	if (!m_player)
		return false;

	if (!m_player->alive())
		return false;

	if (m_player->dormant())
		return false;

	AimPlayer* data = &m_players[ent_idx - 1];

	if (!data)
		return false;

	if (data->m_records.empty())
		return false;

	if (!data->m_records.front().get())
		return false;


	if (!data->m_records.front().get()->valid())
		return false;

	if (!data->m_records.front().get()->m_bones)
		return false;

	const auto backup_cache = m_player->m_BoneCache().m_pCachedBones;

	if (!backup_cache)
		return false;

	// backup player
	const vec3_t backup_origin = m_player->m_vecOrigin();
	const vec3_t backup_abs_origin = m_player->GetAbsOrigin();
	const ang_t backup_abs_angles = m_player->GetAbsAngles();
	const vec3_t backup_obb_mins = m_player->m_vecMins();
	const vec3_t backup_obb_maxs = m_player->m_vecMaxs();

	// always try to use our aimbot matrix first.
	auto matrix = record->m_bones;

	// this is basically for using a custom matrix.
	if (in_shot)
		matrix = bones;

	if (!matrix)
		return false;

	const model_t* model = m_player->GetModel();
	if (!model)
		return false;

	studiohdr_t* hdr = g_csgo.m_model_info->GetStudioModel(model);
	if (!hdr)
		return false;

	mstudiohitboxset_t* set = hdr->GetHitboxSet(m_player->m_nHitboxSet());
	if (!set)
		return false;

	mstudiobbox_t* bbox = set->GetHitbox(box);
	if (!bbox)
		return false;

	return CanHitHitbox(start, end, record, set, box);
}

bool Aimbot::CanHitHitbox(const vec3_t start, const vec3_t end, LagRecord* animation, mstudiohitboxset_t* hdr, int box)
{
	const auto studio_box = hdr->GetHitbox(box);

	if (!studio_box)
		return false;

	vec3_t min, max;

	const auto is_capsule = studio_box->m_radius != -1.f;

	if (is_capsule)
	{
		math::VectorTransform(studio_box->m_mins, animation->m_bones[studio_box->m_bone], min);
		math::VectorTransform(studio_box->m_maxs, animation->m_bones[studio_box->m_bone], max);
		return math::SegmentToSegment(start, end, min, max) < studio_box->m_radius;
	}

	math::VectorTransform(math::VectorRotate(studio_box->m_mins, studio_box->m_angle), animation->m_bones[studio_box->m_bone], min);
	math::VectorTransform(math::VectorRotate(studio_box->m_maxs, studio_box->m_angle), animation->m_bones[studio_box->m_bone], max);

	math::VectorITransform(start, animation->m_bones[studio_box->m_bone], min);
	math::VectorITransform(end, animation->m_bones[studio_box->m_bone], max);

	return math::IntersectLineWithBB(min, max, studio_box->m_mins, studio_box->m_maxs);
}
