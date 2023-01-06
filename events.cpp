#include "includes.h"

Listener g_listener{};;
CSoundEsp SoundEsp{};;

void events::round_start( IGameEvent* evt ) {
	// new round has started. no longer round end.
	g_cl.m_round_end = false;

	// fix fix?
	g_cl.m_body_pred = g_csgo.m_globals->m_curtime;

	// remove notices.
	if( g_hooks.b[ XOR( "persistentKillfeed" ) ] ) {
		KillFeed_t* feed = ( KillFeed_t* )g_csgo.m_hud->FindElement( HASH( "SFHudDeathNoticeAndBotStatus" ) );
		if( feed )
			g_csgo.ClearNotices( feed );
	}

	// reset hvh / aa stuff.
	g_hvh.m_next_random_update = 0.f;
	g_hvh.m_auto_last = 0.f;

	// reset bomb stuff.
	g_visuals.m_c4_planted = false;
	g_visuals.m_planted_c4_indefuse = false;
	g_visuals.m_planted_c4_planting = false;
	g_visuals.m_planted_c4 = nullptr;
	g_visuals.m_planted_c4_defuse = 0;

	// reset dormant esp.
	g_visuals.m_draw.fill( false );
	g_visuals.m_opacities.fill( 0.f );
	g_visuals.m_offscreen_damage.fill( OffScreenDamageData_t( ) );

	// buybot.
	{
		switch( g_hooks.i[ XOR( "autobuy1_selc" ) ] ) {
			case 1:
				g_csgo.m_engine->ExecuteClientCmd( XOR( "buy ssg08" ) );
				break;
			case 2:
				g_csgo.m_engine->ExecuteClientCmd( XOR( "buy awp" ) );
				break;
			case 3:
				g_csgo.m_engine->ExecuteClientCmd( XOR( "buy scar20" ) );
				break;
			default:
				break;
		}

		switch( g_hooks.i[ XOR( "autobuy2_selc" ) ] ) {
			case 1:
				g_csgo.m_engine->ExecuteClientCmd( XOR( "buy glock" ) );
				break;
			case 2:
				g_csgo.m_engine->ExecuteClientCmd( XOR( "buy usp_silencer" ) );
				break;
			case 3:
				g_csgo.m_engine->ExecuteClientCmd( XOR( "buy elite" ) );
				break;
			case 4:
				g_csgo.m_engine->ExecuteClientCmd( XOR( "buy p250" ) );
				break;
			case 5:
				g_csgo.m_engine->ExecuteClientCmd( XOR( "buy tec9" ) );
				break;
			case 6:
				g_csgo.m_engine->ExecuteClientCmd( XOR( "buy fn57" ) );
				break;
			case 7:
				g_csgo.m_engine->ExecuteClientCmd( XOR( "buy deagle" ) );
				break;
			default:
				break;
		}

		if( g_hooks.m[ XOR( "autobuy3_selc" ) ][ 0 ] ) {
			g_csgo.m_engine->ExecuteClientCmd( XOR( "buy vest" ) );
		}
		if( g_hooks.m[ XOR( "autobuy3_selc" ) ][ 1 ] ) {
			g_csgo.m_engine->ExecuteClientCmd( XOR( "buy vesthelm" ) );
		}
		if( g_hooks.m[ XOR( "autobuy3_selc" ) ][ 2 ] ) {
			g_csgo.m_engine->ExecuteClientCmd( XOR( "buy taser" ) );
		}
		if( g_hooks.m[ XOR( "autobuy3_selc" ) ][ 3 ] ) {
			g_csgo.m_engine->ExecuteClientCmd( XOR( "buy defuser" ) );
		}
		if( g_hooks.m[ XOR( "autobuy3_selc" ) ][ 4 ] ) {
			g_csgo.m_engine->ExecuteClientCmd( XOR( "buy heavyarmor" ) );
		}
		if( g_hooks.m[ XOR( "autobuy3_selc" ) ][ 5 ] ) {
			g_csgo.m_engine->ExecuteClientCmd( XOR( "buy molotov" ) );
		}
		if( g_hooks.m[ XOR( "autobuy3_selc" ) ][ 6 ] ) {
			g_csgo.m_engine->ExecuteClientCmd( XOR( "buy incgrenade" ) );
		}
		if( g_hooks.m[ XOR( "autobuy3_selc" ) ][ 7 ] ) {
			g_csgo.m_engine->ExecuteClientCmd( XOR( "buy decoy" ) );
		}
		if( g_hooks.m[ XOR( "autobuy3_selc" ) ][ 8 ] ) {
			g_csgo.m_engine->ExecuteClientCmd( XOR( "buy flashbang" ) );
		}
		if( g_hooks.m[ XOR( "autobuy3_selc" ) ][ 9 ] ) {
			g_csgo.m_engine->ExecuteClientCmd( XOR( "buy hegrenade" ) );
		}
		if( g_hooks.m[ XOR( "autobuy3_selc" ) ][ 10 ] ) {
			g_csgo.m_engine->ExecuteClientCmd( XOR( "buy smokegrenade" ) );
		}
	}

	// update all players.
	for( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i ) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );
		if( !player || player->m_bIsLocalPlayer( ) )
			continue;

		AimPlayer* data = &g_aimbot.m_players[ i - 1 ];
		data->OnRoundStart( player );
	}

	// clear origins.
	g_cl.m_net_pos.clear( );
	SoundEsp.ClearSteps( );
}

void events::round_end( IGameEvent* evt ) {
	if( !g_cl.m_local )
		return;

	// get the reason for the round end.
	int reason = evt->m_keys->FindKey( HASH( "reason" ) )->GetInt( );

	// reset.
	g_cl.m_round_end = false;

	if( g_cl.m_local->m_iTeamNum( ) == TEAM_COUNTERTERRORISTS && reason == CSRoundEndReason::CT_WIN )
		g_cl.m_round_end = true;

	else if( g_cl.m_local->m_iTeamNum( ) == TEAM_TERRORISTS && reason == CSRoundEndReason::T_WIN )
		g_cl.m_round_end = true;
}

void events::player_hurt( IGameEvent* evt ) {
	int attacker, victim;

	// forward event to resolver / shots hurt processing.
	// g_resolver.hurt( evt );
	g_shots.OnHurt( evt );

	// offscreen esp damage stuff.
	if( evt ) {
		attacker = g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "attacker" ) )->GetInt( ) );
		victim = g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "userid" ) )->GetInt( ) );

		// a player damaged the local player.
		if( attacker > 0 && attacker < 64 && victim == g_csgo.m_engine->GetLocalPlayer( ) )
			g_visuals.m_offscreen_damage[ attacker ] = { 3.f, 0.f, colors::red };
	}

	// harm logg.
	if( evt ) {
		Player* attacker = g_csgo.m_entlist->GetClientEntity< Player* >( g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "attacker" ) )->GetInt( ) ) );
		Player* victim = g_csgo.m_entlist->GetClientEntity< Player* >( g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "userid" ) )->GetInt( ) ) );
		if( !attacker || !victim || !attacker->IsPlayer( ) )
			return;

		player_info_t info;
		if( !g_csgo.m_engine->GetPlayerInfo( attacker->index( ), &info ) )
			return;

		if( attacker != g_cl.m_local && victim == g_cl.m_local ) {
			std::string name{ std::string( info.m_name ).substr( 0, 24 ) };
			float damage = evt->m_keys->FindKey( HASH( "dmg_health" ) )->GetInt( );
			int hp = evt->m_keys->FindKey( HASH( "health" ) )->GetInt( );

			if( g_hooks.b[ XOR( "logDamageDealt" ) ] )
				g_notify.add( tfm::format( XOR( "Got hit by %s in the %s for %i damage (%i health remaining)\n" ), name, g_shots.m_groups[ evt->m_keys->FindKey( HASH( "hitgroup" ) )->GetInt( ) ], damage, hp ) );
		}
	}

	if( !strcmp( evt->GetName( ), XOR( "player_hurt" ) ) && g_hooks.b[ XOR( "vis_soundesp" ) ] ) {
		static float lasttime[ 64 ] = { 0.f };
		auto ent = g_csgo.m_entlist->GetClientEntity< Player* >( g_csgo.m_engine->GetPlayerForUserID( evt->GetInt( XOR( "userid" ) ) ) );
		if( ent == nullptr )
			return;

		if( !ent && ent == g_cl.m_local || g_cl.m_local->m_iTeamNum( ) == ent->m_iTeamNum( ) || ent->dormant( ) || !ent->alive( ) )
			return;

		if( g_csgo.m_globals->m_curtime - lasttime[ ent->index( ) ] > .5f ) {
			SoundEsp.StepsData.push_back( CSoundEsp::CSoundEsp_info( ent->GetAbsOrigin( ), .5f, g_hooks.imcolor_to_ccolor( g_hooks.c[ XOR( "vis_soundesp_color" ) ] ) ) );

			lasttime[ ent->index( ) ] = g_csgo.m_globals->m_curtime;
		}
	}

	if( evt ) {
		Player* attacker_c = g_csgo.m_entlist->GetClientEntity< Player* >( g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "attacker" ) )->GetInt( ) ) );
		Player* hurt_c = g_csgo.m_entlist->GetClientEntity< Player* >( g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "userid" ) )->GetInt( ) ) );

		if( hurt_c != g_cl.m_local && attacker_c == g_cl.m_local ) {
			DamageIndicator_t DmgIndicator;
			DmgIndicator.iDamage = evt->m_keys->FindKey( HASH( "dmg_health" ) )->GetInt( );
			DmgIndicator.Player = hurt_c;
			DmgIndicator.flEraseTime = g_cl.m_local->m_nTickBase( ) * g_csgo.m_globals->m_interval + 3.f;
			DmgIndicator.bInitialized = false;
			DmgIndicator.ihitbox = evt->m_keys->FindKey( HASH( "hitgroup" ) )->GetInt( );

			g_visuals.DamageIndicator.push_back( DmgIndicator );
		}
	}

	if( evt ) {
		Player* attacker = g_csgo.m_entlist->GetClientEntity<Player*>( g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "attacker" ) )->GetInt( ) ) );
		Player* victim = g_csgo.m_entlist->GetClientEntity<Player*>( g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "userid" ) )->GetInt( ) ) );

		if( !attacker || !victim || attacker != g_cl.m_local )
			return;

		vec3_t enemypos = victim->GetAbsOrigin( );
		impact_info best_impact;
		float best_impact_distance = -1;
		float time = g_csgo.m_globals->m_curtime;


		for( int i = 0; i < g_visuals.impacts.size( ); i++ ) {
			auto iter = g_visuals.impacts[ i ];
			if( time > iter.time + 1.f ) {
				g_visuals.impacts.erase( g_visuals.impacts.begin( ) + i );
				continue;
			}
			vec3_t position = vec3_t( iter.x, iter.y, iter.z );
			float distance = position.dist_to( enemypos );
			if( distance < best_impact_distance || best_impact_distance == -1 ) {
				best_impact_distance = distance;
				best_impact = iter;
			}
		}
		if( best_impact_distance == -1 )
			return;

		hitmarker_info info;
		info.impact = best_impact;
		info.alpha = 255;
		info.time = g_csgo.m_globals->m_curtime;
		g_visuals.hitmarkers.push_back( info );
	}
}
void events::on_fire(IGameEvent* evt) {
	g_shots.OnFire(evt);
}
void events::bullet_impact( IGameEvent* evt ) {
	// forward event to resolver impact processing.
	g_shots.OnImpact( evt );

	// decode impact coordinates and convert to vec3
	vec3_t pos = {
			evt->m_keys->FindKey( HASH( "x" ) )->GetFloat( ),
			evt->m_keys->FindKey( HASH( "y" ) )->GetFloat( ),
			evt->m_keys->FindKey( HASH( "z" ) )->GetFloat( )
	};

	// get attacker, if its not us, screw it.
	int attacker = g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "userid" ) )->GetInt( ) );

	if( g_hooks.b[ XOR( "bulletImpacts" ) ] && attacker == g_csgo.m_engine->GetLocalPlayer( ) )
		g_csgo.m_debug_overlay->AddBoxOverlay( pos, vec3_t( -2.0f, -2.0f, -2.0f ), vec3_t( 2.0f, 2.0f, 2.0f ), ang_t( 0.0f, 0.0f, 0.0f ), 0, 0, 255, 127, 3.0f );

	// get attacker, if its not us, screw it.
	if( g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "userid" ) )->GetInt( ) ) != g_csgo.m_engine->GetLocalPlayer( ) )
		return;

	Player* shooter = g_csgo.m_entlist->GetClientEntity<Player*>( g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "userid" ) )->GetInt( ) ) );
	if( !shooter || shooter != g_cl.m_local )
		return;

	impact_info info;
	info.x = evt->GetFloat( "x" );
	info.y = evt->GetFloat( "y" );
	info.z = evt->GetFloat( "z" );
	info.time = g_csgo.m_globals->m_curtime;
	g_visuals.impacts.push_back( info );
}

void events::item_purchase( IGameEvent* evt ) {
	int           team, purchaser;
	player_info_t info;

	if( !g_cl.m_local || !evt )
		return;

	if( !g_hooks.b[ XOR( "logWeaponPurchases" ) ] )
		return;

	// only log purchases of the enemy team.
	team = evt->m_keys->FindKey( HASH( "team" ) )->GetInt( );
	if( team == g_cl.m_local->m_iTeamNum( ) )
		return;

	// get the player that did the purchase.
	purchaser = g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "userid" ) )->GetInt( ) );

	// get player info of purchaser.
	if( !g_csgo.m_engine->GetPlayerInfo( purchaser, &info ) )
		return;

	std::string weapon = evt->m_keys->FindKey( HASH( "weapon" ) )->m_string;
	if( weapon == XOR( "weapon_unknown" ) )
		return;

	std::string out = tfm::format( XOR( "%s bought %s\n" ), std::string{ info.m_name }.substr( 0, 24 ), weapon );
	g_notify.add( out );
}

void events::player_death( IGameEvent* evt ) {
	if( !evt )
		return;

	// get index of player that died.
	int index = g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "userid" ) )->GetInt( ) );

	// reset opacity scale.
	g_visuals.m_opacities[ index - 1 ] = 0.f;
	g_visuals.m_draw[ index - 1 ] = false;
}

void events::player_given_c4( IGameEvent* evt ) {
	player_info_t info;

	// get the player who received the bomb.
	int index = g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "userid" ) )->GetInt( ) );
	if( index == g_csgo.m_engine->GetLocalPlayer( ) )
		return;

	if( !g_csgo.m_engine->GetPlayerInfo( index, &info ) )
		return;

	std::string out = tfm::format( XOR( "%s received the bomb\n" ), std::string{ info.m_name }.substr( 0, 24 ) );
	g_notify.add( out );
}

void events::bomb_beginplant( IGameEvent* evt ) {
	player_info_t info;

	g_visuals.m_planted_c4_planting = true;

	g_visuals.m_plant_duration = 3.f;
	g_visuals.m_plant_start = g_csgo.m_globals->m_curtime;
	g_visuals.m_plant_end = g_visuals.m_plant_start + g_visuals.m_plant_duration;

	// get the player who played the bomb.
	int index = g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "userid" ) )->GetInt( ) );
	if( index == g_csgo.m_engine->GetLocalPlayer( ) )
		return;

	// get player info of purchaser.
	if( !g_csgo.m_engine->GetPlayerInfo( index, &info ) )
		return;

	std::string out = tfm::format( XOR( "%s started planting the bomb\n" ), std::string{ info.m_name }.substr( 0, 24 ) );
	g_notify.add( out );
}

void events::bomb_abortplant( IGameEvent* evt ) {
	player_info_t info;

	g_visuals.m_planted_c4_planting = false;

	// get the player who stopped planting the bomb.
	int index = g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "userid" ) )->GetInt( ) );
	if( index == g_csgo.m_engine->GetLocalPlayer( ) )
		return;

	// get player info of purchaser.
	if( !g_csgo.m_engine->GetPlayerInfo( index, &info ) )
		return;

	std::string out = tfm::format( XOR( "%s stopped planting the bomb\n" ), std::string{ info.m_name }.substr( 0, 24 ) );
	g_notify.add( out );
}

void events::bomb_planted( IGameEvent* evt ) {
	Entity* bomb_target;
	std::string   site_name;
	int           player_index;
	player_info_t info;
	std::string   out;

	// get the func_bomb_target entity and store info about it.
	bomb_target = g_csgo.m_entlist->GetClientEntity( evt->m_keys->FindKey( HASH( "site" ) )->GetInt( ) );
	if( bomb_target ) {
		site_name = bomb_target->GetBombsiteName( );
		g_visuals.m_last_bombsite = site_name;
	}

	g_visuals.m_planted_c4_planting = false;

	player_index = g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "userid" ) )->GetInt( ) );
	if( player_index == g_csgo.m_engine->GetLocalPlayer( ) )
		out = tfm::format( XOR( "You planted the bomb at %s\n" ), site_name.c_str( ) );

	else {
		g_csgo.m_engine->GetPlayerInfo( player_index, &info );

		out = tfm::format( XOR( "The bomb was planted at %s by %s\n" ), site_name.c_str( ), std::string( info.m_name ).substr( 0, 24 ) );
	}

	g_notify.add( out );
}

void events::bomb_beep( IGameEvent* evt ) {
	Entity* c4;
	vec3_t             explosion_origin, explosion_origin_adjusted;
	CTraceFilterSimple filter;
	CGameTrace         tr;

	// we have a bomb ent already, don't do anything else.
	if( g_visuals.m_c4_planted )
		return;

	// bomb_beep is called once when a player plants the c4 and contains the entindex of the C4 weapon itself, we must skip that here.
	c4 = g_csgo.m_entlist->GetClientEntity( evt->m_keys->FindKey( HASH( "entindex" ) )->GetInt( ) );
	if( !c4 || !c4->is( HASH( "CPlantedC4" ) ) )
		return;

	// planted bomb is currently active, grab some extra info about it and set it for later.
	g_visuals.m_c4_planted = true;
	g_visuals.m_planted_c4 = c4;
	g_visuals.m_planted_c4_explode_time = c4->m_flC4Blow( );
	g_visuals.m_planted_c4_start = g_csgo.m_globals->m_curtime;
	g_visuals.m_planted_c4_end = g_visuals.m_planted_c4_start + g_visuals.m_planted_c4_explode_time;

	// the bomb origin is adjusted slightly inside CPlantedC4::C4Think, right when it's about to explode.
	// we're going to do that here.
	explosion_origin = c4->GetAbsOrigin( );
	explosion_origin_adjusted = explosion_origin;
	explosion_origin_adjusted.z += 8.f;

	// setup filter and do first trace.
	filter.SetPassEntity( c4 );

	g_csgo.m_engine_trace->TraceRay(
		Ray( explosion_origin_adjusted, explosion_origin_adjusted + vec3_t( 0.f, 0.f, -40.f ) ),
		MASK_SOLID,
		&filter,
		&tr
	);

	// pull out of the wall a bit.
	if( tr.m_fraction != 1.f )
		explosion_origin = tr.m_endpos + ( tr.m_plane.m_normal * 0.6f );

	// this happens inside CCSGameRules::RadiusDamage.
	explosion_origin.z += 1.f;

	// set all other vars.
	g_visuals.m_planted_c4_explosion_origin = explosion_origin;

	// todo - dex;  get this radius dynamically... seems to only be available in map bsp file, search string: "info_map_parameters"
	//              info_map_parameters is an entity created on the server, it doesnt seem to have many useful networked vars for clients.
	//
	//              swapping maps between de_dust2 and de_nuke and scanning for 500 and 400 float values will leave you one value.
	//              need to figure out where it's written from.
	//
	// server.dll uses starting 'radius' as damage... the real radius passed to CCSGameRules::RadiusDamage is actually multiplied by 3.5.
	g_visuals.m_planted_c4_damage = 500.f;
	g_visuals.m_planted_c4_radius = g_visuals.m_planted_c4_damage * 3.5f;
	g_visuals.m_planted_c4_radius_scaled = g_visuals.m_planted_c4_radius / 3.f;
}

void events::bomb_begindefuse( IGameEvent* evt ) {
	player_info_t info;

	g_visuals.m_planted_c4_indefuse = true;
	if( evt->m_keys->FindKey( HASH( "haskit" ) )->GetBool( ) ) { g_visuals.m_planted_c4_defuse = 1; g_visuals.m_defuse_duration = 5.f; }
	else { g_visuals.m_planted_c4_defuse = 2; g_visuals.m_defuse_duration = 10.f; }
	g_visuals.m_defuse_start = g_csgo.m_globals->m_curtime;
	g_visuals.m_defuse_end = g_visuals.m_defuse_start + g_visuals.m_defuse_duration;

	// get index of player that started defusing the bomb.
	int index = g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "userid" ) )->GetInt( ) );
	if( index == g_csgo.m_engine->GetLocalPlayer( ) )
		return;

	if( !g_csgo.m_engine->GetPlayerInfo( index, &info ) )
		return;

	bool kit = evt->m_keys->FindKey( HASH( "haskit" ) )->GetBool( );

	if( kit ) {
		std::string out = tfm::format( XOR( "%s started defusing with a kit\n" ), std::string( info.m_name ).substr( 0, 24 ) );
		g_notify.add( out );
	}

	else {
		std::string out = tfm::format( XOR( "%s started defusing without a kit\n" ), std::string( info.m_name ).substr( 0, 24 ) );
		g_notify.add( out );
	}
}

void events::bomb_abortdefuse( IGameEvent* evt ) {
	player_info_t info;

	g_visuals.m_planted_c4_defuse = 0;
	g_visuals.m_planted_c4_indefuse = false;

	// get index of player that stopped defusing the bomb.
	int index = g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "userid" ) )->GetInt( ) );
	if( index == g_csgo.m_engine->GetLocalPlayer( ) )
		return;

	if( !g_csgo.m_engine->GetPlayerInfo( index, &info ) )
		return;

	std::string out = tfm::format( XOR( "%s stopped defusing\n" ), std::string( info.m_name ).substr( 0, 24 ) );
	g_notify.add( out );
}

void events::bomb_defused( IGameEvent* evt ) {
	g_visuals.m_c4_planted = false;
	g_visuals.m_planted_c4_indefuse = false;
	g_visuals.m_planted_c4_planting = false;
	g_visuals.m_planted_c4 = nullptr;
	g_visuals.m_planted_c4_defuse = 0;
}

void events::bomb_exploded( IGameEvent* evt ) {
	g_visuals.m_c4_planted = false;
	g_visuals.m_planted_c4_indefuse = false;
	g_visuals.m_planted_c4_planting = false;
	g_visuals.m_planted_c4 = nullptr;
	g_visuals.m_planted_c4_defuse = 0;
}

void events::player_footstep( IGameEvent* evt ) {
	if( !g_hooks.b[ XOR( "vis_soundesp" ) ] )
		return;

	if( !strcmp( evt->GetName( ), XOR( "player_footstep" ) ) ) {
		static float lasttime[ 64 ] = { 0.f };

		auto ent = g_csgo.m_entlist->GetClientEntity< Player* >( g_csgo.m_engine->GetPlayerForUserID( evt->GetInt( XOR( "userid" ) ) ) );
		if( ent == nullptr )
			return;

		if( !ent && ent == g_cl.m_local || g_cl.m_local->m_iTeamNum( ) == ent->m_iTeamNum( ) || ent->dormant( ) || !ent->alive( ) )
			return;

		if( g_csgo.m_globals->m_curtime - lasttime[ ent->index( ) ] > .5f ) {
			SoundEsp.StepsData.push_back( CSoundEsp::CSoundEsp_info( ent->GetAbsOrigin( ), .5f, g_hooks.imcolor_to_ccolor( g_hooks.c[ XOR( "vis_soundesp_color" ) ] ) ) );

			lasttime[ ent->index( ) ] = g_csgo.m_globals->m_curtime;
		}
	}
}
#pragma optimize( "", off ) // типа выделяешь код и он не опимизируется по идее

void Listener::init( ) {
	VMPBSTART("listener");
	// link events with callbacks.
	add( XOR( "round_start" ), events::round_start );
	add( XOR( "round_end" ), events::round_end );
	add( XOR( "player_hurt" ), events::player_hurt );
	add( XOR( "bullet_impact" ), events::bullet_impact );
	add( XOR( "weapon_fire" ), events::on_fire );
	add( XOR( "item_purchase" ), events::item_purchase );
	add( XOR( "player_death" ), events::player_death );
	add( XOR( "player_given_c4" ), events::player_given_c4 );
	add( XOR( "bomb_beginplant" ), events::bomb_beginplant );
	add( XOR( "bomb_abortplant" ), events::bomb_abortplant );
	add( XOR( "bomb_planted" ), events::bomb_planted );
	add( XOR( "bomb_beep" ), events::bomb_beep );
	add( XOR( "bomb_begindefuse" ), events::bomb_begindefuse );
	add( XOR( "bomb_abortdefuse" ), events::bomb_abortdefuse );
	add( XOR( "bomb_defused" ), events::bomb_defused );
	add( XOR( "bomb_exploded" ), events::bomb_exploded );
	add( XOR( "player_footstep" ), events::player_footstep );

	register_events( );
	VMPEND();
}
#pragma optimize( "", on ) // типа выделяешь код и он не опимизируется по идее

void CSoundEsp::AddStepManually( int ent, vec3_t origin ) {
	static float lasttime[ 4096 ] = { 0.f };

	if( g_csgo.m_globals->m_curtime - lasttime[ ent ] > 1.5f ) { // pFix задержка
		StepsData.push_back( CSoundEsp_info( origin, .5f, g_hooks.imcolor_to_ccolor( g_hooks.c[ XOR( "vis_soundesp_color" ) ] ) ) );

		lasttime[ ent ] = g_csgo.m_globals->m_curtime;
	}
}

void CSoundEsp::ClearSteps( ) {
	StepsData.clear( );
}

void CSoundEsp::DrawSteps( ) {
	if( 1 ) {
		if( g_csgo.m_engine->IsInGame( ) ) {
			BeamInfo_t beamInfo;

			for( size_t i = 0; i < StepsData.size( ); i++ ) {
				auto current = StepsData.at( i );
				beamInfo.m_nType = 0x07;
				beamInfo.m_pszModelName = "sprites/purplelaser1.vmt";
				beamInfo.m_nModelIndex = g_csgo.m_model_info->GetModelIndex( "sprites/purplelaser1.vmt" );
				//beam_info.m_pszHaloName   = "sprites/purplelaser1.vmt";
				beamInfo.m_nHaloIndex = -1;
				beamInfo.m_flHaloScale = 5;
				beamInfo.m_flLife = 1.f; //
				beamInfo.m_flWidth = 10.f;
				beamInfo.m_flFadeLength = 1.0f;
				beamInfo.m_flAmplitude = 0.f;
				beamInfo.m_flRed = current.color.r( );
				beamInfo.m_flGreen = current.color.g( );
				beamInfo.m_flBlue = current.color.b( );
				beamInfo.m_flBrightness = current.color.a( );
				beamInfo.m_flSpeed = 0.f;
				beamInfo.m_nStartFrame = 0.f;
				beamInfo.m_flFrameRate = 60.f;
				beamInfo.m_nSegments = -1;
				//beam_info.m_bRenderable   = true;
				beamInfo.m_nFlags = 0x00000008;
				beamInfo.m_vecCenter = current.origin + vec3_t( 0, 0, 5 );
				beamInfo.m_flStartRadius = 20.f;
				beamInfo.m_flEndRadius = 1000.f;
				auto myBeam = g_csgo.m_beams->CreateBeamRingPoint( beamInfo );

				if( myBeam )
					g_csgo.m_beams->DrawBeam( myBeam );

				StepsData.erase( StepsData.begin( ) + i );
			}
		}
	}
}