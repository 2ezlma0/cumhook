#include "includes.h"
#include "Mmsystem.h"
#include "font.h"
#include "discord.h"

Discord* g_Discord;

Client g_cl{};


// loader will set this fucker.
#pragma optimize( "", off ) //видал какая хйуня естьхахаха
std::string GetVal( HKEY hKey, LPCTSTR lpValue ) {
	OBF_BEGIN;
	std::string data;
	char buffer[ 100 ];
	DWORD size = sizeof( buffer );
	DWORD type = REG_SZ;

	LONG nError = LI_FN( RegQueryValueExA ).get( )( hKey, lpValue, NULL, &type, ( LPBYTE )buffer, &size );

	//LONG nError = RegQueryValueExA( hKey, lpValue, NULL, &type, ( LPBYTE )buffer, &size );

	if( nError ) {
		//cout « "Error: " « nError « " Could not get registry value " « (char*)lpValue « endl;
		data = XOR( "0" ); // The value will be created and set to data next time SetVal() is called.
		return data;

	}

	data = buffer;
	return data;
	OBF_END;
}

// init routine.
ulong_t __stdcall Client::init( void* arg ) {
	VMPBSTART( "client init" );
	OBF_BEGIN;
	// stop here if we failed to getting nickame from loader.
	

	// if not in interwebz mode, the driver will not set the username.
	g_cl.m_user = "admin";

	DWORD menu_icon;
	DWORD weapon_icon;
	AddFontMemResourceEx( NanoX, 5192, nullptr, &menu_icon );
	AddFontMemResourceEx( WeaponFont, 47320, nullptr, &weapon_icon );

	// stop here if we failed to acquire all the data needed from csgo.
	//OBF_BEGIN;
	////bool isDebugged = false;

	////BOOL bIsDbgPresent = FALSE;
	////CheckRemoteDebuggerPresent(GetCurrentProcess(), &bIsDbgPresent);
	////isDebugged = bIsDbgPresent;
	////isDebugged = HardwareBreakpoints();
	//std::string storedHWID = GetVal(HKEY_CURRENT_USER, XOR("IMLASTLOGIN"));
	//uint32_t hwInt = 0;
	////IF(storedHWID != XOR("0"))ой
	////{
	////	hwInt = atoi(storedHWID.c_str());
	////}
	////ENDIF;
	//if (!g_csgo.init(hwInt)) // обфускация ебет игру... из-за оптимизации...
	//	RETURN(0);
	//OBF_END;
	// я от
	// 
	//std::string storedHWID = GetVal(HKEY_CURRENT_USER, XOR("IMLASTLOGIN"));
	//uint32_t hwInt = *(uint32_t*)arg;

	//char buff[32];

	//sprintf(buff, "%d", hwInt);

	//MessageBoxA(NULL, buff, buff, NULL);

	//if (storedHWID != XOR("0"))
	//{
	//	hwInt = atoi(storedHWID.c_str());
	//}
	if( !g_csgo.init( ( uint32_t* )arg ) )
		return 0;

	// Welcome the user.
	//PlaySoundA( ( LPCSTR )hitsound_byte_array, nullptr, SND_MEMORY | SND_ASYNC );
	return 1;
	OBF_END;
	VMPEND( );
}

#pragma optimize( "", on ) // типа выделяешь код и он не опимизируется по идее

void Client::DrawHUD( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_hooks.b[ XOR( "watermark" ) ] )
		return;

	// colors
	const auto col_background = Color( 235, 235, 235, 230 );
	const auto col_accent = Color( 2,2,2, 199 );

	// get time.
	time_t t = std::time( nullptr );
	std::ostringstream time;
	time << std::put_time( std::localtime( &t ), ( "%H:%M:%S" ) );

	// get round trip time in milliseconds.
	int ms = std::max( 0, ( int )std::round( g_cl.m_latency * 1000.f ) );

	// get tickrate.
	int rate = ( int )std::round( 1.f / g_csgo.m_globals->m_interval );

	// get fps.
	int fps = 1.f / g_csgo.m_globals->m_abs_frametime;
	std::string text = tfm::format( XOR( "cumhook v420 | %s | delay: %ims | tick: %i | fps: %i | %s" ), g_cl.m_user, ms, rate, fps, time.str( ).data( ) );
	render::FontSize_t size = render::menu_shade.size( text );

	// background.
	render::rect_filled( m_width - size.m_width - 18, 10, size.m_width + 8, size.m_height + 8, col_background );

	// text.
	render::menu_shade.string( m_width - 14, 14, { 55, 55, 55, 254 }, tfm::format( XOR( " | %s | delay: %ims | tick: %i | fps: %i | %s" ), g_cl.m_user, ms, rate, fps, time.str( ).data( ) ), render::ALIGN_RIGHT );
	render::menu_shade.string( m_width - 14 - render::menu_shade.size( tfm::format( XOR( " | %s | delay: %ims | tick: %i | fps: %i | %s" ), g_cl.m_user, ms, rate, fps, time.str( ).data( ) ) ).m_width, 14, col_accent, tfm::format( XOR( " v420" ) ), render::ALIGN_RIGHT );
	render::menu_shade.string( m_width - 14 - render::menu_shade.size( tfm::format( XOR( " v420 | %s | delay: %ims | tick: %i | fps: %i | %s" ), g_cl.m_user, ms, rate, fps, time.str( ).data( ) ) ).m_width, 14, { 5, 5, 5, 254 }, tfm::format( XOR( "cumhook" ) ), render::ALIGN_RIGHT );
}


void Client::UnlockHiddenConvars( ) {
	if( !g_csgo.m_cvar )
		return;

	auto p = **reinterpret_cast< ConVar*** >( g_csgo.m_cvar + 0x34 );
	for( auto c = p->m_next; c != nullptr; c = c->m_next ) {
		c->m_flags &= ~FCVAR_DEVELOPMENTONLY;
		c->m_flags &= ~FCVAR_HIDDEN;
	}
}

void Client::KillFeed( ) {
	if( !g_hooks.b[ XOR( "persistentKillfeed" ) ] )
		return;

	if( !g_csgo.m_engine->IsInGame( ) )
		return;

	// get the addr of the killfeed.
	KillFeed_t* feed = ( KillFeed_t* )g_csgo.m_hud->FindElement( HASH( "SFHudDeathNoticeAndBotStatus" ) );
	if( !feed )
		return;

	int size = feed->notices.Count( );
	if( !size )
		return;

	for( int i{}; i < size; ++i ) {
		NoticeText_t* notice = &feed->notices[ i ];

		// this is a local player kill, delay it.
		if( notice->fade == 1.5f )
			notice->fade = FLT_MAX;
	}
}

void Client::OnPaint( ) {
	// update screen size.
	g_csgo.m_engine->GetScreenSize( m_width, m_height );

	// render stuff.
	g_visuals.think( );
	g_grenades.paint( );
	g_notify.think( );

	for( auto i = 0; i < g_csgo.m_entlist->GetHighestEntityIndex( ); i++ ) {
		Player* player = g_csgo.m_entlist->GetClientEntity( i )->as<Player*>( );

		if( player == nullptr ||
			!player->GetClientClass( ) ||
			player == g_cl.m_local )
			continue;

		g_grenades_pred.grenade_warning( player );
		g_grenades_pred.get_local_data( ).draw( );
	}

	DrawHUD( );
	KillFeed( );

	//another stab at the dark for discord rpc
	g_Discord->Initialize();
	g_Discord->Update();

	// menu goes last.
	//g_gui.think( );
}

void Client::OnMapload( ) {
	// store class ids.
	g_netvars.SetupClassData( );

	// createmove will not have been invoked yet.
	// but at this stage entites have been created.
	// so now we can retrive the pointer to the local player.
	m_local = g_csgo.m_entlist->GetClientEntity< Player* >( g_csgo.m_engine->GetLocalPlayer( ) );

	// world materials.
	Visuals::ModulateWorld( );
	Skybox( );
	g_aimbot.reset( );
	g_visuals.m_hit_start = g_visuals.m_hit_end = g_visuals.m_hit_duration = 0.f;
	g_visuals.DamageIndicator.clear( );

	// init knife shit.
	g_skins.load( );

	m_sequences.clear( );

	// if the INetChannelInfo pointer has changed, store it for later.
	g_csgo.m_net = g_csgo.m_engine->GetNetChannelInfo( );

	if( g_csgo.m_net ) {
		g_hooks.m_net_channel.reset( );
		g_hooks.m_net_channel.init( g_csgo.m_net );
		g_hooks.m_net_channel.add( INetChannel::PROCESSPACKET, util::force_cast( &Hooks::ProcessPacket ) );
		g_hooks.m_net_channel.add( INetChannel::SENDDATAGRAM, util::force_cast( &Hooks::SendDatagram ) );
	}
}

void Client::StartMove( CUserCmd* cmd ) {
	// save some usercmd stuff.
	m_cmd = cmd;
	m_tick = cmd->m_tick;
	m_view_angles = cmd->m_view_angles;
	m_buttons = cmd->m_buttons;
	m_pressing_move = ( ( m_buttons & IN_LEFT ) || ( m_buttons & IN_FORWARD ) || ( m_buttons & IN_BACK ) ||
						( m_buttons & IN_RIGHT ) || ( m_buttons & IN_MOVELEFT ) || ( m_buttons & IN_MOVERIGHT ) ||
						( m_buttons & IN_JUMP ) );

	if( m_processing && m_tick_to_recharge > 0 && !m_charged ) {
		m_tick_to_recharge--;
		if( m_tick_to_recharge == 0 ) {
			m_charged = true;
		}
		cmd->m_tick = INT_MAX;
		*m_packet = true;
	}

	// store max choke
	// TODO; 11 -> m_bIsValveDS
	m_max_lag = ( m_local->m_fFlags( ) & FL_ONGROUND ) ? 16 : 15;
	m_lag = g_csgo.m_cl->m_choked_commands;
	m_lerp = game::GetClientInterpAmount( );
	m_latency = g_csgo.m_net->GetLatency( INetChannel::FLOW_OUTGOING );
	math::clamp( m_latency, 0.f, 1.f );
	m_latency_ticks = game::TIME_TO_TICKS( m_latency );
	m_server_tick = g_csgo.m_cl->m_server_tick;
	m_arrival_tick = m_server_tick + m_latency_ticks;
	//*g_cl.m_packet = true;

	// get local ptr.
	m_local = g_csgo.m_entlist->GetClientEntity< Player* >( g_csgo.m_engine->GetLocalPlayer( ) );

	// processing indicates that the localplayer is valid and alive.
	m_processing = m_local && m_local->alive( );
	if( !m_processing )
		return;

	// make sure prediction has ran on all usercommands.
	// because prediction runs on frames, when we have low fps it might not predict all usercommands.
	// also fix the tick being inaccurate.
	g_inputpred.update( );

	if( g_hooks.b[ XOR( "fastduck" ) ] )
		m_cmd->m_buttons |= IN_BULLRUSH;

	// store some stuff about the local player.
	m_flags = m_local->m_fFlags( );

	// ...
	m_shot = false;
}

void Client::BackupPlayers( bool restore ) {
	if( restore ) {
		// restore stuff.
		for( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i ) {
			Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );

			if( !g_aimbot.IsValidTarget( player ) )
				continue;

			g_aimbot.m_backup[ i - 1 ].restore( player );
		}
	}

	else {
		// backup stuff.
		for( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i ) {
			Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );

			if( !g_aimbot.IsValidTarget( player ) )
				continue;

			g_aimbot.m_backup[ i - 1 ].store( player );
		}
	}
}

void Client::DoMove( ) {
	penetration::PenetrationOutput_t tmp_pen_data{};

	//discord rpc!!!
	g_Discord->Initialize();
	g_Discord->Update();

	int quickstop_s = g_hooks.i[ XOR( "quickstop" ) ];
	bool can_stop = g_hooks.auto_check( "quickstop_key" );

	if( g_cl.m_weapon_type == WEAPONTYPE_PISTOL ) {
		quickstop_s = g_hooks.i[ XOR( "quickstop_pistols" ) ];
		can_stop = g_hooks.auto_check( "quickstop_key_pistols" );
	}

	switch( g_cl.m_weapon_id ) {
		case SSG08:
			quickstop_s = g_hooks.i[ XOR( "quickstop_scout" ) ];
			can_stop = g_hooks.auto_check( "quickstop_key_scout" );
			break;

		case AWP:
			quickstop_s = g_hooks.i[ XOR( "quickstop_awp" ) ];
			can_stop = g_hooks.auto_check( "quickstop_key_awp" );
			break;

		case DEAGLE:
			quickstop_s = g_hooks.i[ XOR( "quickstop_deagle" ) ];
			can_stop = g_hooks.auto_check( "quickstop_key_deagle" );
			break;

		case REVOLVER:
			quickstop_s = g_hooks.i[ XOR( "quickstop_revolver" ) ];
			can_stop = g_hooks.auto_check( "quickstop_key_revolver" );
			break;

		default:
			break;
	}

	g_inputpred.velocity = m_local->m_vecVelocity( );
	g_inputpred.origin = m_local->m_vecOrigin( );

	// run movement code before input prediction.
	g_movement.JumpRelated( );
	g_movement.FastStop( );
	g_movement.FakeWalk( );
	g_movement.StrafeDefault( );

	if( ( ( g_hooks.b[ XOR( "auto_peek" ) ] && g_hooks.auto_check( XOR( "auto_peek_key" ) ) ) || can_stop ) && g_cl.m_ground ) {
		//if( g_aimbot.m_stop ) {
		switch( quickstop_s ) {
			case 1:
				g_movement.QuickStop( ); // full stop
				break;
			case 2:
				g_movement.AutoStop( ); // slide stop
				break;
			case 3:
				g_movement.FakeWalk( ); // Fake walk
				break;
			default:
				break;
		}
		//}
	}

	// backup strafe angles (we need them for input prediction)
	m_strafe_angles = m_cmd->m_view_angles;

	g_movement.Strafe( );
	g_movement.AutoPeek( g_cl.m_cmd, m_strafe_angles.y );

	const auto pre_flags = g_cl.m_local->m_fFlags( );

	// predict input.
	g_inputpred.run( );

	const auto post_flags = g_cl.m_local->m_fFlags( );
	g_movement.EdgeJump( pre_flags, post_flags );

	// convert viewangles to directional forward vector.
	math::AngleVectors( m_view_angles, &m_forward_dir );

	// restore original angles after input prediction
	m_cmd->m_view_angles = m_view_angles;

	// reset shit.
	m_weapon = nullptr;
	m_weapon_info = nullptr;
	m_weapon_id = -1;
	m_weapon_type = WEAPONTYPE_UNKNOWN;
	m_player_fire = m_weapon_fire = false;

	// store weapon stuff.
	m_weapon = m_local->GetActiveWeapon( );

	if( m_weapon ) {
		m_weapon_info = m_weapon->GetWpnData( );
		m_weapon_id = m_weapon->m_iItemDefinitionIndex( );
		m_weapon_type = m_weapon_info->m_weapon_type;

		m_weapon->UpdateAccuracyPenalty( );

		auto is_special_weapon = m_weapon_id == 9
			|| m_weapon_id == 11
			|| m_weapon_id == 38
			|| m_weapon_id == 40;

		g_inputpred.m_perfect_accuracy = 0.f;

		if( m_weapon_id == SSG08 && ( g_cl.m_local->m_fFlags( ) & FL_ONGROUND ) == 0 )
			g_inputpred.m_perfect_accuracy = 0.00875f;
		else if( g_cl.m_local->m_fFlags( ) & FL_DUCKING ) {
			if( is_special_weapon )
				g_inputpred.m_perfect_accuracy = m_weapon_info->m_inaccuracy_crouch_alt;
			else
				g_inputpred.m_perfect_accuracy = m_weapon_info->m_inaccuracy_crouch;
		}
		else if( is_special_weapon )
			g_inputpred.m_perfect_accuracy = m_weapon_info->m_inaccuracy_stand_alt;
		else
			g_inputpred.m_perfect_accuracy = m_weapon_info->m_inaccuracy_stand;

		g_cl.SetupEyePosition( );

		// run autowall once for penetration crosshair if we have an appropriate weapon.
		if( m_weapon_type != WEAPONTYPE_KNIFE && m_weapon_type != WEAPONTYPE_C4 && m_weapon_type != WEAPONTYPE_GRENADE ) {
			penetration::PenetrationInput_t in;
			in.m_from = m_local;
			in.m_target = nullptr;
			in.m_pos = m_shoot_pos + ( m_forward_dir * m_weapon_info->m_range );
			in.m_damage = 1.f;
			in.m_damage_pen = 1.f;
			in.m_can_pen = true;

			// run autowall.
			penetration::run( &in, &tmp_pen_data);
		}

		// set pen data for penetration crosshair.
		m_pen_data = tmp_pen_data;

		// can the player fire.
		m_player_fire = g_csgo.m_globals->m_curtime >= m_local->m_flNextAttack( ) && !g_csgo.m_gamerules->m_bFreezePeriod( ) && !( g_cl.m_flags & FL_FROZEN );

		if( g_hooks.b[ XOR( "rbotenable" ) ] )
			UpdateRevolverCock( );

		m_weapon_fire = CanFireWeapon( g_csgo.m_globals->m_curtime );
	}

	// last tick defuse.
	// todo - dex;  figure out the range for CPlantedC4::Use?
	//              add indicator if valid (on ground, still have time, not being defused already, etc).
	//              move this? not sure where we should put it.
	if( g_hooks.is_key_down( g_hooks.i[ XOR( "last_tick_defuse_key" ) ] ) && g_visuals.m_c4_planted ) {
		float defuse = ( m_local->m_bHasDefuser( ) ) ? 5.f : 10.f;
		float remaining = g_visuals.m_planted_c4_explode_time - g_csgo.m_globals->m_curtime;
		float dt = remaining - defuse - ( g_cl.m_latency / 2.f );

		m_cmd->m_buttons &= ~IN_USE;
		if( dt <= game::TICKS_TO_TIME( 2 ) )
			m_cmd->m_buttons |= IN_USE;
	}

	// grenade prediction.
	g_grenades.think( );

	// run fakelag.
	g_hvh.SendPacket( );

	// run aimbot.
	g_aimbot.think( );

	// run antiaims.
	g_hvh.AntiAim( );
}

void Client::EndMove( CUserCmd* cmd ) {
	// update client-side animations.
	UpdateAnimations( );

	// if matchmaking mode, anti untrust clamp.
	if( g_hooks.b[ XOR( "antiUntrusted" ) ] )
		m_cmd->m_view_angles.SanitizeAngle( );

	// fix our movement.
	g_movement.FixMove( cmd, m_strafe_angles );

	// this packet will be sent.
	if( *m_packet ) {
		g_hvh.m_step_switch = ( bool )g_csgo.RandomInt( 0, 1 );

		// we are sending a packet, so this will be reset soon.
		// store the old value.
		m_old_lag = m_lag;

		// get radar angles.
		m_radar = cmd->m_view_angles;
		m_radar.normalize( );

		// get current origin.
		vec3_t cur = m_local->m_vecOrigin( );

		// get prevoius origin.
		vec3_t prev = m_net_pos.empty( ) ? cur : m_net_pos.front( ).m_pos;

		// check if we broke lagcomp.
		m_lagcomp = ( cur - prev ).length_sqr( ) > 4096.f;

		// save sent origin and time.
		m_net_pos.emplace_front( g_csgo.m_globals->m_curtime, cur );
	}

	// store some values for next tick.
	m_old_packet = *m_packet;
	m_old_shot = m_shot;
}

void Client::OnTick( CUserCmd* cmd ) {
	// TODO; add this to the menu.
	if( g_hooks.b[ XOR( "revealCompetitiveRanks" ) ] && cmd->m_buttons & IN_SCORE ) {
		static CCSUsrMsg_ServerRankRevealAll msg{ };
		g_csgo.ServerRankRevealAll( &msg );
	}

	// store some data and update prediction.
	StartMove( cmd );

	// not much more to do here.
	if( !m_processing )
		return;

	// save the original state of players.
	BackupPlayers( false );

	// run all movement related code.
	DoMove( );

	// store stome additonal stuff for next tick
	// sanetize our usercommand if needed and fix our movement.
	EndMove( cmd );

	// restore the players.
	BackupPlayers( true );

	// restore curtime/frametime
	// and prediction seed/player.
	g_inputpred.restore( );
	if (!g_tickshift.m_shifting) {
		g_tickshift.handle_doubletap();
	}
}

void Client::SetupEyePosition( ) {

	CCSGOPlayerAnimState* animstate = g_cl.m_local->m_PlayerAnimState( ); if( !animstate ) return;

	float bk[ ] = { FLT_MAX,FLT_MAX };

	static float last_upd;

	const auto absorigin = g_cl.m_local->GetAbsOrigin( );

	g_cl.m_local->InvalidateBoneCache( );
	g_cl.m_local->SetAbsOrigin( g_cl.m_local->m_vecOrigin( ) );
	g_cl.m_local->m_flPoseParameter( ) = g_cl.m_poses;
	g_cl.m_local->m_flPoseParameter( )[ 12 ] = 0.5f;

	g_cl.m_local->SetAbsAngles( g_cl.m_rotation );

	//if( g_cl.m_local->m_PlayerAnimState( ) != nullptr && g_cl.m_local->m_PlayerAnimState( )->m_velocity_length_xy < 0.1f && g_cl.m_local->m_PlayerAnimState( )->m_on_ground && g_cl.m_local->GetSequenceActivity( g_cl.m_local->m_AnimOverlay( )[ 3 ].m_sequence ) == 979 ) {
	//	bk[ 0 ] = g_cl.m_local->m_AnimOverlay( )[ 3 ].m_weight;
	//	bk[ 1 ] = g_cl.m_local->m_AnimOverlay( )[ 3 ].m_cycle;
	//
	//	g_cl.m_local->m_AnimOverlay( )[ 3 ].m_weight = 0.f;
	//	g_cl.m_local->m_AnimOverlay( )[ 3 ].m_cycle = 0.f;
	//}

	if( g_cl.m_local->m_fFlags( ) & FL_ONGROUND ) {
		animstate->m_on_ground = true;
		animstate->m_landing = false;
	}

	animstate->m_time_to_align_lower_body = m_body_pred;
	animstate->m_move_yaw_current_to_ideal = 20.f;

	//if( g_cl.m_local->m_vecVelocity( ).length_2d( ) > 1.0f ) {
	//	g_cl.m_local->m_AnimOverlay( )[ 6 ].m_cycle = ( g_cl.m_local->m_AnimOverlay( )[ 6 ].m_cycle - g_cl.m_local->m_AnimOverlay( )[ 6 ].m_playback_rate + ( g_cl.m_local->m_AnimOverlay( )[ 6 ].m_playback_rate * g_csgo.m_globals->m_interval / g_cl.m_lag ) );
	//	g_cl.m_local->m_AnimOverlay( )[ 5 ].m_cycle = ( g_cl.m_local->m_AnimOverlay( )[ 4 ].m_cycle - g_cl.m_local->m_AnimOverlay( )[ 5 ].m_playback_rate + ( g_cl.m_local->m_AnimOverlay( )[ 4 ].m_playback_rate * g_csgo.m_globals->m_interval / g_cl.m_lag ) );
	//}

	animstate->m_eye_yaw = g_cl.m_body;

	static float last_update;
	static float last_choke;

	if( !( animstate->m_on_ground && animstate->m_landing ) ) {
		for( size_t i{ }; i < 18; ++i ) {
			float lerp = std::min( g_cl.m_lag, ( int )g_csgo.m_globals->m_interval * 2 );
			float update_delta = g_cl.m_lag;
			float update_lerp = std::clamp( update_delta - lerp, 0.f, 1.f );
			if( update_lerp > 0.f )
				lerp = std::clamp( lerp, 0.f, update_delta );

			float lerp_progress = ( last_update + lerp - g_csgo.m_globals->m_curtime ) / lerp;

			if( i == 10 )
				continue;

			auto param = g_cl.m_local->m_flPoseParameter( ).at( i );
			std::array<float, 24> old_param;
			float jmp_param = 0.f;
			if( param != old_param.at( i ) ) {
				jmp_param = math::lerp2( old_param.at( i ), param, update_lerp / update_delta );
				old_param.at( i ) = param;
			}

			float final_param = math::lerp2( param, jmp_param, std::clamp( lerp_progress, 0.f, 1.f ) );

			g_cl.m_local->m_flPoseParameter( )[ i ] = final_param;
		}
	}

	if( g_cl.m_local->m_vecVelocity( ).length( ) <= 0.1f )
		g_cl.m_local->m_AnimOverlay( )[ 6 ].m_weight = 0.f;

	g_cl.m_local->SetupBones( 0, -1, 0x7FF00, g_csgo.m_globals->m_curtime );

	//if( bk[ 0 ] != FLT_MAX ) {
	//	g_cl.m_local->m_AnimOverlay( )[ 3 ].m_weight = bk[ 0 ];
	//	g_cl.m_local->m_AnimOverlay( )[ 3 ].m_cycle = bk[ 1 ];
	//}

	g_cl.m_shoot_pos = g_cl.m_local->GetShootPorsition( );
	g_cl.m_local->SetAbsOrigin( absorigin );
	g_cl.m_local->m_flPoseParameter( ) = g_cl.m_poses;
}

void Client::UpdateAnimations( ) {
	if( g_cl.m_lag > 0 )
		return;

	CCSGOPlayerAnimState* state = g_cl.m_local->m_PlayerAnimState( );
	if( !state )
		return;

	// update time.
	m_anim_frame = g_csgo.m_globals->m_curtime - m_anim_time;
	m_anim_time = g_csgo.m_globals->m_curtime;

	// current angle will be animated.
	m_angle = g_cl.m_cmd->m_view_angles;

	// fix landing anim.
	if( state->m_landing && ( g_cl.m_flags & FL_ONGROUND ) )
		m_angle.x = 12.f;

	math::clamp( m_angle.x, -90.f, 90.f );
	m_angle.normalize( );

	// write angles to model.
	//g_csgo.m_prediction->SetLocalViewAngles( m_angle );
	g_cl.m_local->m_RenderAngles( ) = m_angle;

	if( g_cl.m_local->m_vecAbsVelocity( ) != g_cl.m_local->m_vecVelocity( ) ) {
		g_cl.m_local->SetAbsVelocity( g_cl.m_local->m_vecVelocity( ) );
		g_cl.m_local->m_iEFlags( ) &= ~0x1000u;
		g_cl.m_local->InvalidatePhysicsRecursive( VELOCITY_CHANGED );
	}

	// set lby to predicted value.
	g_cl.m_local->m_flLowerBodyYawTarget( ) = m_body;

	// CCSGOPlayerAnimState::Update, bypass already animated checks.
	if( state->m_last_update_frame >= g_csgo.m_globals->m_frame )
		state->m_last_update_frame = g_csgo.m_globals->m_frame - 1;

	// call original, bypass hook.
	g_hooks.m_UpdateClientSideAnimation( g_cl.m_local );

	m_rotation = g_cl.m_local->GetAbsAngles( );
	m_poses = g_cl.m_local->m_flPoseParameter( );
	g_cl.m_local->GetAnimLayers( m_layers.data( ) );

	// we landed.
	//if( !m_ground && state->m_on_ground ) {
	//	m_body      = m_angle.y;
	//	m_body_pred = m_anim_time;
	//}

	if( state->m_on_ground ) {
		// walking, delay lby update by .22.
		if( state->m_velocity_length_xy > 0.1f ) {
			m_body = m_angle.y;
			m_body_pred = m_anim_time + 0.22f;
		}
		else if( m_anim_time > m_body_pred ) {
			m_body = m_angle.y;
			m_body_pred = m_anim_time + 1.1f;
		}
	}

	// save updated data.
	m_speed = state->m_velocity_length_xy;
	m_ground = state->m_on_ground;
}

void Client::print( const std::string text, ... ) {
	va_list     list;
	int         size;
	std::string buf;

	if( text.empty( ) )
		return;

	va_start( list, text );

	// count needed size.
	size = std::vsnprintf( 0, 0, text.c_str( ), list );

	// allocate.
	buf.resize( size );

	// print to buffer.
	std::vsnprintf( buf.data( ), size + 1, text.c_str( ), list );

	va_end( list );

	// print to console.
	g_csgo.m_cvar->ConsoleColorPrintf( colors::light_blue, XOR( "cumhook -  " ) );
	g_csgo.m_cvar->ConsoleColorPrintf( colors::white, buf.c_str( ) );
}

bool Client::CanFireWeapon( float curtime ) {
	auto server_time = game::TICKS_TO_TIME( g_cl.m_local->m_nTickBase( ) );

	if( server_time < g_cl.m_weapon->m_flNextPrimaryAttack( ) )
		return false;

	if( server_time < g_cl.m_local->m_flNextAttack( ) )
		return false;

	// the player cant fire.
	if( !m_player_fire )
		return false;

	if( m_weapon_type == WEAPONTYPE_GRENADE )
		return false;

	// if we have no bullets, we cant shoot.
	if( m_weapon_type != WEAPONTYPE_KNIFE && m_weapon->m_iClip1( ) < 1 )
		return false;

	// do we have any burst shots to handle?
	if( ( m_weapon_id == GLOCK || m_weapon_id == FAMAS ) && m_weapon->m_iBurstShotsRemaining( ) > 0 ) {
		// new burst shot is coming out.
		if( curtime >= m_weapon->m_fNextBurstShot( ) )
			return true;
	}

	// r8 revolver.
	if( m_weapon_id == REVOLVER ) {
		int act = m_weapon->m_Activity( );

		// mouse1.
		if( !m_revolver_fire ) {
			if( ( act == 185 || act == 193 ) && m_revolver_cock == 0 )
				return curtime >= m_weapon->m_flNextPrimaryAttack( );

			return false;
		}
	}

	// yeez we have a normal gun.
	if( curtime >= m_weapon->m_flNextPrimaryAttack( ) )
		return true;

	return true;
}

bool Client::IsFiring( float curtime ) {
	const auto weapon = m_weapon;
	if( !weapon )
		return false;

	const auto IsZeus = m_weapon_id == ZEUS;
	const auto IsKnife = !IsZeus && m_weapon_type == WEAPONTYPE_KNIFE;

	if( weapon->IsGrenade( ) )
		return !weapon->m_bPinPulled( ) && weapon->m_fThrowTime( ) > 0.f && weapon->m_fThrowTime( ) < curtime;
	else if( IsKnife )
		return ( m_cmd->m_buttons & ( IN_ATTACK ) || m_cmd->m_buttons & ( IN_ATTACK2 ) ) && CanFireWeapon( curtime );
	else
		return m_cmd->m_buttons & ( IN_ATTACK ) && CanFireWeapon( curtime );
}

void Client::UpdateRevolverCock( ) {
	// default to false.
	m_revolver_fire = false;

	// reset properly.
	if( m_revolver_cock == -1 )
		m_revolver_cock = 0;

	// we dont have a revolver.
	// we have no ammo.
	// player cant fire
	// we are waiting for we can shoot again.
	if( m_weapon_id != REVOLVER || m_weapon->m_iClip1( ) < 1 || !m_player_fire || g_csgo.m_globals->m_curtime < m_weapon->m_flNextPrimaryAttack( ) ) {
		// reset.
		m_revolver_cock = 0;
		m_revolver_query = 0;
		return;
	}

	// calculate max number of cocked ticks.
	// round to 6th decimal place for custom tickrates..
	int shoot = ( int )( 0.25f / ( std::round( g_csgo.m_globals->m_interval * 1000000.f ) / 1000000.f ) );

	// amount of ticks that we have to query.
	m_revolver_query = shoot - 1;

	// we held all the ticks we needed to hold.
	if( m_revolver_query == m_revolver_cock ) {
		// reset cocked ticks.
		m_revolver_cock = -1;

		// we are allowed to fire, yay.
		m_revolver_fire = true;
	}

	else {
		// we still have ticks to query.
		// apply inattack.
		if( g_hooks.b[ XOR( "antiUntrusted" ) ] && m_revolver_query > m_revolver_cock )
			m_cmd->m_buttons |= IN_ATTACK;

		// count cock ticks.
		// do this so we can also count 'legit' ticks
		// that didnt originate from the hack.
		if( m_cmd->m_buttons & IN_ATTACK )
			m_revolver_cock++;

		// inattack was not held, reset.
		else m_revolver_cock = 0;
	}

	// remove inattack2 if cocking.
	if( m_revolver_cock > 0 )
		m_cmd->m_buttons &= ~IN_ATTACK2;
}

void Client::UpdateIncomingSequences( ) {
	if( !g_csgo.m_net )
		return;

	if( m_sequences.empty( ) || g_csgo.m_net->m_in_seq > m_sequences.front( ).m_seq ) {
		// store new stuff.
		m_sequences.emplace_front( g_csgo.m_globals->m_realtime, g_csgo.m_net->m_in_rel_state, g_csgo.m_net->m_in_seq );
	}

	// do not save too many of these.
	while( m_sequences.size( ) > 2048 )
		m_sequences.pop_back( );
}

void Client::ClanTag( ) {
	if( !g_csgo.m_engine->IsInGame( ) )
		return;

	// lambda function for setting our clantag.
	auto SetClanTag = [ & ]( std::string tag ) -> void {
		using SetClanTag_t = int( __fastcall* )( const char*, const char* );
		static auto SetClanTagFn = pattern::find( g_csgo.m_engine_dll, XOR( "53 56 57 8B DA 8B F9 FF 15" ) ).as<SetClanTag_t>( );

		SetClanTagFn( tag.c_str( ), XOR( "cumhook" ) );
	};

	std::string szClanTag = XOR( "       cumhook      " );
	static int iPrevFrame = 0;
	static bool bReset = false;
	int iCurFrame = ( ( int )( g_csgo.m_globals->m_curtime * 2.7f ) ) % ( szClanTag.size( ) * 2 );

	if( g_hooks.i[ XOR( "clantagSpammer_selc" ) ] == 1 ) {
		// are we in a new frame?
		if( iPrevFrame != iCurFrame ) {
			// rotate our clantag.
			for( int i = 0; i < iCurFrame; i++ ) {
				std::rotate( szClanTag.begin( ), szClanTag.begin( ) + 1, szClanTag.end( ) );
			}

			// define our clantag
			szClanTag = tfm::format( XOR( "%s" ), szClanTag );

			// set our clantag
			SetClanTag( szClanTag.data( ) );

			// set current/last frame.
			iPrevFrame = iCurFrame;
		}

		// do we want to reset after untoggling the clantag?
		bReset = true;
	}
	// gamesense.
	else if( g_hooks.i[ XOR( "clantagSpammer_selc" ) ] == 2 ) {
		// are we in a new frame?
		if( iPrevFrame != ( int )( g_csgo.m_globals->m_curtime * 2.7f ) % 18 ) {
			switch( int( g_csgo.m_globals->m_curtime * 2.7f ) % 18 ) {
				case 0: SetClanTag( XOR( "% " ) ); break;
				case 1: SetClanTag( XOR( "c " ) ); break;
				case 2: SetClanTag( XOR( "c* " ) ); break;
				case 3: SetClanTag( XOR( "cu " ) ); break;
				case 4: SetClanTag( XOR( "cu# " ) ); break;
				case 5: SetClanTag( XOR( "cum " ) ); break;
				case 6: SetClanTag( XOR( "cum& " ) ); break;
				case 7: SetClanTag( XOR( "cumh " ) ); break;
				case 8: SetClanTag( XOR( "cumh0 " ) ); break;
				case 9: SetClanTag( XOR( "cumho " ) ); break;
				case 10:SetClanTag( XOR( "cumho_ " ) ); break;
				case 11:SetClanTag( XOR( "cumhoo " ) ); break;
				case 12:SetClanTag( XOR( "cumhoo+ " ) ); break;
				case 13:SetClanTag( XOR( "cumhook " ) ); break;
				case 14:SetClanTag( XOR( "cumh0ok " ) ); break;
				case 15:SetClanTag( XOR( "cumh00k " ) ); break;
				case 16:SetClanTag( XOR( "cumho0k " ) ); break;
				case 17:SetClanTag( XOR( "cumhook " ) ); break;
			}

			// set current/last frame.
			iPrevFrame = ( int )( g_csgo.m_globals->m_curtime * 2.7f ) % 21;
		}

		// do we want to reset after untoggling the clantag?
		bReset = true;
	}
	// skeet.cc.
	else if( g_hooks.i[ XOR( "clantagSpammer_selc" ) ] == 3 ) {
		std::string szClanTag2 = XOR( "cumhook" );

		SetClanTag( szClanTag2 );

		// do we want to reset after untoggling the clantag?
		bReset = true;
	}
	else {
		// reset our clantag.
		if( bReset ) {
			SetClanTag( XOR( "" ) );
			bReset = false;
		}
	}
}

void Client::Skybox( ) {
	if( !g_csgo.m_engine->IsInGame( ) )
		return;

	static auto sv_skyname = g_csgo.m_cvar->FindVar( HASH( "sv_skyname" ) );
	switch( g_hooks.i[ XOR( "skybox_selc" ) ] ) {
		case 1: //Tibet
			//sv_skyname->SetValue( "cs_tibet" );
			sv_skyname->SetValue( XOR( "cs_tibet" ) );
			break;
		case 2: //Embassy
			//sv_skyname->SetValue( "embassy" );
			sv_skyname->SetValue( XOR( "embassy" ) );
			break;
		case 3: //Italy
			//sv_skyname->SetValue( "italy" );
			sv_skyname->SetValue( XOR( "italy" ) );
			break;
		case 4: //Daylight 1
			//sv_skyname->SetValue( "sky_cs15_daylight01_hdr" );
			sv_skyname->SetValue( XOR( "sky_cs15_daylight01_hdr" ) );
			break;
		case 5: //Cloudy
			//sv_skyname->SetValue( "sky_csgo_cloudy01" );
			sv_skyname->SetValue( XOR( "sky_csgo_cloudy01" ) );
			break;
		case 6: //Night 1
			sv_skyname->SetValue( XOR( "sky_csgo_night02" ) );
			break;
		case 7: //Night 2
			//sv_skyname->SetValue( "sky_csgo_night02b" );
			sv_skyname->SetValue( XOR( "sky_csgo_night02b" ) );
			break;
		case 8: //Night Flat
			//sv_skyname->SetValue( "sky_csgo_night_flat" );
			sv_skyname->SetValue( XOR( "sky_csgo_night_flat" ) );
			break;
		case 9: //Day HD
			//sv_skyname->SetValue( "sky_day02_05_hdr" );
			sv_skyname->SetValue( XOR( "sky_day02_05_hdr" ) );
			break;
		case 10: //Day
			//sv_skyname->SetValue( "sky_day02_05" );
			sv_skyname->SetValue( XOR( "sky_day02_05" ) );
			break;
		case 11: //Rural
			//sv_skyname->SetValue( "sky_l4d_rural02_ldr" );
			sv_skyname->SetValue( XOR( "sky_l4d_rural02_ldr" ) );
			break;
		case 12: //Vertigo HD
			//sv_skyname->SetValue( "vertigo_hdr" );
			sv_skyname->SetValue( XOR( "vertigo_hdr" ) );
			break;
		case 13: //Vertigo Blue HD
			//sv_skyname->SetValue( "vertigoblue_hdr" );
			sv_skyname->SetValue( XOR( "vertigoblue_hdr" ) );
			break;
		case 14: //Vertigo
			//sv_skyname->SetValue( "vertigo" );
			sv_skyname->SetValue( XOR( "vertigo" ) );
			break;
		case 15: //Vietnam
			//sv_skyname->SetValue( "vietnam" );
			sv_skyname->SetValue( XOR( "vietnam" ) );
			break;
		case 16: //Dusty Sky
			//sv_skyname->SetValue( "sky_dust" );
			sv_skyname->SetValue( XOR( "sky_dust" ) );
			break;
		case 17: //Jungle
			sv_skyname->SetValue( XOR( "jungle" ) );
			break;
		case 18: //Nuke
			sv_skyname->SetValue( XOR( "nukeblank" ) );
			break;
		case 19: //Office
			sv_skyname->SetValue( XOR( "office" ) );
			//game::SetSkybox( XOR( "office" ) );
			break;
		default:
			break;
	}
}