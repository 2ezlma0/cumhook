#include "includes.h"

Visuals g_visuals{ };;

float Visuals::DistInFt( vec3_t o, vec3_t dest ) {
	vec3_t yo = vec3_t( dest.x - o.x, dest.y - o.y, dest.z - o.z );
	int dist = std::roundf( std::sqrt( yo.x * yo.x + yo.y * yo.y + yo.z * yo.z ) / 12 );

	if( dist > 0 ) {
		if( dist > 5 ) {
			while( !( dist % 5 == 0 ) ) {
				dist--;
			}

			if( dist % 5 == 0 )
				return dist;
		}
		else
			return dist;
	}
	else
		return 0;
}

void Visuals::ModulateWorld( ) {
	std::vector< IMaterial* > world, props;

	// iterate material handles.
	for( uint16_t h{ g_csgo.m_material_system->FirstMaterial( ) }; h != g_csgo.m_material_system->InvalidMaterial( ); h = g_csgo.m_material_system->NextMaterial( h ) ) {
		// get material from handle.
		IMaterial* mat = g_csgo.m_material_system->GetMaterial( h );
		if( !mat )
			continue;

		// store world materials.
		if( FNV1a::get( mat->GetTextureGroupName( ) ) == HASH( "World textures" ) )
			world.push_back( mat );

		// store props.
		else if( FNV1a::get( mat->GetTextureGroupName( ) ) == HASH( "StaticProp textures" ) )
			props.push_back( mat );
	}

	// walls
	for( const auto& w : world )
		w->AlphaModulate( g_hooks.f[ XOR( "transparentWalls" ) ] / 100.f );

	// night
	if( g_hooks.m[ XOR( "brightnessAdjustment_sel" ) ][ 0 ] ) {
		//const float darkness = g_menu.main.visuals.night_darkness.get( ) / 100.f;
			//w->ColorModulate( darkness, darkness, darkness );

		for( const auto& w : world ) {
			w->ColorModulate( g_hooks.c[ XOR( "nightmode_color" ) ][ 0 ], g_hooks.c[ XOR( "nightmode_color" ) ][ 1 ], g_hooks.c[ XOR( "nightmode_color" ) ][ 2 ] );
		}

		// IsUsingStaticPropDebugModes my nigga
		if( g_csgo.r_DrawSpecificStaticProp->GetInt( ) != 0 ) {
			g_csgo.r_DrawSpecificStaticProp->SetValue( 0 );
		}

		for( const auto& p : props )
			p->ColorModulate( 0.5f, 0.5f, 0.5f );
	}

	// disable night.
	else {
		for( const auto& w : world ) {
			w->ColorModulate( 1.f, 1.f, 1.f );
		}

		// restore r_DrawSpecificStaticProp.
		if( g_csgo.r_DrawSpecificStaticProp->GetInt( ) != -1 ) {
			g_csgo.r_DrawSpecificStaticProp->SetValue( -1 );
		}

		for( const auto& p : props )
			p->ColorModulate( 1.f, 1.f, 1.f );
	}

	// transparent props.
	if( g_hooks.f[ XOR( "transparentProps" ) ] > 0 ) {

		// IsUsingStaticPropDebugModes my nigga
		if( g_csgo.r_DrawSpecificStaticProp->GetInt( ) != 0 ) {
			g_csgo.r_DrawSpecificStaticProp->SetValue( 0 );
		}

		float alpha = g_hooks.f[ XOR( "transparentProps" ) ] / 100;
		for( const auto& p : props )
			p->AlphaModulate( alpha );
	}

	// disable transparent props.
	else {

		// restore r_DrawSpecificStaticProp.
		if( g_csgo.r_DrawSpecificStaticProp->GetInt( ) != -1 ) {
			g_csgo.r_DrawSpecificStaticProp->SetValue( -1 );
		}

		for( const auto& p : props )
			p->AlphaModulate( 1.0f );
	}
}

void Visuals::ThirdpersonThink( ) {
	ang_t                          offset;
	vec3_t                         origin, forward;
	static CTraceFilterSimple_game filter{ };
	CGameTrace                     tr;

	// for whatever reason overrideview also gets called from the main menu.
	if( !g_csgo.m_engine->IsInGame( ) )
		return;

	// check if we have a local player and he is alive.
	bool alive = g_cl.m_processing;

	if( g_hooks.b[ XOR( "third_person_on_grenade" ) ] && g_cl.m_weapon_type == WEAPONTYPE_GRENADE ) {
		g_csgo.m_input->CAM_ToFirstPerson( );
		g_csgo.m_input->m_camera_offset.z = 0.f;
		return;
	}

	// camera should be in thirdperson.
	if( g_hooks.auto_check( "thirdperson_key" ) && g_hooks.b[ XOR( "forceThirdPerson" ) ] ) {

		// if alive and not in thirdperson already switch to thirdperson.
		if( alive && !g_csgo.m_input->CAM_IsThirdPerson( ) )
			g_csgo.m_input->CAM_ToThirdPerson( );

		// if dead and spectating in firstperson switch to thirdperson.
		else if( g_cl.m_local->m_iObserverMode( ) == 4 ) {

			// if in thirdperson, switch to firstperson.
			// we need to disable thirdperson to spectate properly.
			if( g_csgo.m_input->CAM_IsThirdPerson( ) ) {
				g_csgo.m_input->CAM_ToFirstPerson( );
				g_csgo.m_input->m_camera_offset.z = 0.f;
			}

			g_cl.m_local->m_iObserverMode( ) = 5;
		}
	}

	// camera should be in firstperson.
	else if( g_csgo.m_input->CAM_IsThirdPerson( ) ) {
		g_csgo.m_input->CAM_ToFirstPerson( );
		g_csgo.m_input->m_camera_offset.z = 0.f;
	}

	// if after all of this we are still in thirdperson.
	if( g_csgo.m_input->CAM_IsThirdPerson( ) ) {
		// get camera angles.
		g_csgo.m_engine->GetViewAngles( offset );

		// get our viewangle's forward directional vector.
		math::AngleVectors( offset, &forward );

		// cam_idealdist convar.
		offset.z = g_hooks.f[ XOR( "thirdperson_distance" ) ];

		// start pos.
		origin = g_cl.m_shoot_pos;

		// setup trace filter and trace.
		filter.SetPassEntity( g_cl.m_local );

		g_csgo.m_engine_trace->TraceRay(
			Ray( origin, origin - ( forward * offset.z ), { -16.f, -16.f, -16.f }, { 16.f, 16.f, 16.f } ),
			MASK_NPCWORLDSTATIC,
			( ITraceFilter* )&filter,
			&tr
		);

		// adapt distance to travel time.
		math::clamp( tr.m_fraction, 0.f, 1.f );
		offset.z *= tr.m_fraction;

		// override camera angles.
		g_csgo.m_input->m_camera_offset = { offset.x, offset.y, offset.z };
	}
}

void Visuals::Hitmarker( ) {
	static auto cross = g_csgo.m_cvar->FindVar( HASH( "weapon_debug_spread_show" ) );
	cross->SetValue( g_hooks.b[ XOR( "crosshair" ) ] && !g_cl.m_local->m_bIsScoped( ) ? 3 : 0 );
	// dont do if dead.
	if( !g_cl.m_processing )
		return;

	if( !g_hooks.b[ XOR( "hitmarker" ) ] )
		return;

	if( g_csgo.m_globals->m_curtime > m_hit_end )
		return;

	if( m_hit_duration <= 0.f )
		return;

	float complete = ( g_csgo.m_globals->m_curtime - m_hit_start ) / m_hit_duration;
	int x = g_cl.m_width,
		y = g_cl.m_height,
		alpha = ( 1.f - complete ) * 240;

	render::rect_filled( x / 2 + 6, y / 2 + 6, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 + 7, y / 2 + 7, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 + 8, y / 2 + 8, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 + 9, y / 2 + 9, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 + 10, y / 2 + 10, 1, 1, { 200, 200, 200, alpha } );

	render::rect_filled( x / 2 - 6, y / 2 - 6, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 - 7, y / 2 - 7, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 - 8, y / 2 - 8, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 - 9, y / 2 - 9, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 - 10, y / 2 - 10, 1, 1, { 200, 200, 200, alpha } );

	render::rect_filled( x / 2 - 6, y / 2 + 6, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 - 7, y / 2 + 7, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 - 8, y / 2 + 8, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 - 9, y / 2 + 9, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 - 10, y / 2 + 10, 1, 1, { 200, 200, 200, alpha } );

	render::rect_filled( x / 2 + 6, y / 2 - 6, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 + 7, y / 2 - 7, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 + 8, y / 2 - 8, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 + 9, y / 2 - 9, 1, 1, { 200, 200, 200, alpha } );
	render::rect_filled( x / 2 + 10, y / 2 - 10, 1, 1, { 200, 200, 200, alpha } );
}

void Visuals::NoSmoke( ) {
	if( !smoke1 )
		smoke1 = g_csgo.m_material_system->FindMaterial( XOR( "particle/vistasmokev1/vistasmokev1_fire" ), XOR( "Other textures" ) );

	if( !smoke2 )
		smoke2 = g_csgo.m_material_system->FindMaterial( XOR( "particle/vistasmokev1/vistasmokev1_smokegrenade" ), XOR( "Other textures" ) );

	if( !smoke3 )
		smoke3 = g_csgo.m_material_system->FindMaterial( XOR( "particle/vistasmokev1/vistasmokev1_emods" ), XOR( "Other textures" ) );

	if( !smoke4 )
		smoke4 = g_csgo.m_material_system->FindMaterial( XOR( "particle/vistasmokev1/vistasmokev1_emods_impactdust" ), XOR( "Other textures" ) );

	if( g_hooks.b[ XOR( "removeSmokeGrenades" ) ] ) {
		if( !smoke1->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke1->SetFlag( MATERIAL_VAR_NO_DRAW, true );

		if( !smoke2->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke2->SetFlag( MATERIAL_VAR_NO_DRAW, true );

		if( !smoke3->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke3->SetFlag( MATERIAL_VAR_NO_DRAW, true );

		if( !smoke4->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke4->SetFlag( MATERIAL_VAR_NO_DRAW, true );
	}

	else {
		if( smoke1->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke1->SetFlag( MATERIAL_VAR_NO_DRAW, false );

		if( smoke2->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke2->SetFlag( MATERIAL_VAR_NO_DRAW, false );

		if( smoke3->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke3->SetFlag( MATERIAL_VAR_NO_DRAW, false );

		if( smoke4->GetFlag( MATERIAL_VAR_NO_DRAW ) )
			smoke4->SetFlag( MATERIAL_VAR_NO_DRAW, false );
	}

	/*
	std::vector<const char*> vistasmoke_mats = {
		XOR( "particle/particle_flares/particle_flare_gray" ),
		XOR( "particle/smoke1/smoke1_nearcull2" ),
		XOR( "particle/vistasmokev1/vistasmokev1_nearcull" ),
		XOR( "particle/smoke1/smoke1_nearcull" ),
		XOR( "particle/vistasmokev1/vistasmokev1_nearcull_nodepth" ),
		XOR( "particle/vistasmokev1/vistasmokev1_nearcull_fog" ),
		XOR( "particle/vistasmokev1/vistasmokev4_nearcull" ),
		XOR( "particle/smoke1/smoke1_nearcull3" ),

		XOR( "particle/fire_burning_character/fire_env_fire_depthblend_oriented" ),
		XOR( "particle/fire_burning_character/fire_burning_character" ),

		// XOR("particle/fire_burning_character/fire_env_fire"),

		XOR( "particle/fire_explosion_1/fire_explosion_1_oriented" ),
		XOR( "particle/fire_explosion_1/fire_explosion_1_bright" ),

		XOR( "particle/fire_burning_character/fire_burning_character_depthblend" ),
		XOR( "particle/fire_burning_character/fire_env_fire_depthblend" ),
	};

	for( auto mat_s : vistasmoke_mats ) {
		IMaterial* mat = g_csgo.m_material_system->FindMaterial( mat_s, XOR( "Other textures" ) );
		if( !mat ) continue;

		if( !mat->GetFlag( MATERIAL_VAR_WIREFRAME ) )
			mat->SetFlag( MATERIAL_VAR_WIREFRAME, true );
	}
	*/
}

void Visuals::think( ) {
	// for whatever reason overrideview also gets called from the main menu.
	if( !g_csgo.m_engine->IsInGame( ) )
		return;

	// don't run anything if our local player isn't valid.
	if( !g_cl.m_local )
		return;

	if( g_hooks.b[ XOR( "removeScopeOverlay" ) ]
		&& g_cl.m_local->alive( )
		&& g_cl.m_local->GetActiveWeapon( )
		/*&& g_cl.m_local->GetActiveWeapon( )->GetWpnData( )->m_weapon_type == CSWeaponType::WEAPONTYPE_SNIPER_RIFLE*/ ) {

		// rebuild the original scope lines.
		int w = g_cl.m_width,
			h = g_cl.m_height,
			x = w / 2,
			y = h / 2,
			size = g_csgo.cl_crosshair_sniper_width->GetInt( ),
			offset = g_hooks.f[ XOR( "scope_line_offset" ) ],
			height = g_hooks.f[ XOR( "scope_line_height" ) ];

		Color color = Color( g_hooks.c[ XOR( "scope_line_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "scope_line_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "scope_line_color" ) ][ 2 ] * 255.f, g_hooks.c[ XOR( "scope_line_color" ) ][ 3 ] * 255.f );

		float frequency = ( g_hooks.f[ XOR( "scope_line_speed" ) ] / 100.f ) * 3.f;
		static float m_opacity;

		// the increment / decrement per frame.
		float step = frequency * g_csgo.m_globals->m_frametime;

		// if open		-> increment
		// if closed	-> decrement
		g_cl.m_local->m_bIsScoped( ) ? m_opacity += step : m_opacity -= step;

		// clamp the opacity.
		math::clamp( m_opacity, 0.f, 1.f );

		// Here We Use The Euclidean distance To Get The Polar-Rectangular Conversion Formula.
		if( size > 1 ) {
			x -= ( size / 2 );
			y -= ( size / 2 );
		}

		// draw our lines.
		if( g_hooks.b[ XOR( "scope_line_enable" ) ] && m_opacity ) {
			render::gradient( g_cl.m_width / 2 + offset + height - ( height * m_opacity ), g_cl.m_height / 2, height * m_opacity, 1, { color.r( ), color.g( ), color.b( ), 0 }, { color.r( ), color.g( ), color.b( ), color.a( ) }, true );
			render::gradient( g_cl.m_width / 2 - height - offset, g_cl.m_height / 2, height * m_opacity, 1, { color.r( ), color.g( ), color.b( ), color.a( ) }, { color.r( ), color.g( ), color.b( ), 0 }, true );
			render::gradient( g_cl.m_width / 2, g_cl.m_height / 2 + offset + height - ( height * m_opacity ), 1, height * m_opacity, { color.r( ), color.g( ), color.b( ), 0 }, { color.r( ), color.g( ), color.b( ), color.a( ) } );
			render::gradient( g_cl.m_width / 2, g_cl.m_height / 2 - height - offset, 1, height * m_opacity, { color.r( ), color.g( ), color.b( ), color.a( ) }, { color.r( ), color.g( ), color.b( ), 0 } );
		}
		else if( g_cl.m_local->m_bIsScoped( ) && g_cl.m_local->GetActiveWeapon( )->GetWpnData( )->m_weapon_type == CSWeaponType::WEAPONTYPE_SNIPER_RIFLE ) {
			render::rect_filled( 0, y, w, size, colors::black );
			render::rect_filled( x, 0, size, h, colors::black );
		}
	}

	// draw esp on ents.
	for( int i{ 1 }; i <= g_csgo.m_entlist->GetHighestEntityIndex( ); ++i ) {
		Entity* ent = g_csgo.m_entlist->GetClientEntity( i );
		if( !ent )
			continue;

		draw( ent );
	}

	// draw everything else.
	AutomaticPeekIndicator( );
	DrawDamageIndicator( );
	ManualAntiAim( );
	StatusIndicators( );
	Spectators( );
	PenetrationCrosshair( );
	Hitmarker( );
	DrawPlantedC4( );
	Hitmarker3D( );
	//DrawSpread( );
	//SoundEsp.DrawSteps( );
}

void Visuals::Hitmarker3D( ) {
	if( !g_hooks.b[ XOR( "hitmarker_3d" ) ] )
		return;

	if( hitmarkers.size( ) == 0 )
		return;

	// draw
	for( int i = 0; i < hitmarkers.size( ); i++ ) {
		vec3_t pos3D = vec3_t( hitmarkers[ i ].impact.x, hitmarkers[ i ].impact.y, hitmarkers[ i ].impact.z );
		vec2_t pos2D;

		if( !render::WorldToScreen( pos3D, pos2D ) )
			continue;

		int r = g_hooks.c[ XOR( "hitmarker_3d_c" ) ][ 0 ] * 255;
		int g = g_hooks.c[ XOR( "hitmarker_3d_c" ) ][ 1 ] * 255;
		int b = g_hooks.c[ XOR( "hitmarker_3d_c" ) ][ 2 ] * 255;

		render::line( pos2D.x + 2, pos2D.y + 2, pos2D.x + 5, pos2D.y + 5, { r, g, b, hitmarkers[ i ].alpha } );
		render::line( pos2D.x - 2, pos2D.y - 2, pos2D.x - 5, pos2D.y - 5, { r, g, b, hitmarkers[ i ].alpha } );
		render::line( pos2D.x + 2, pos2D.y - 2, pos2D.x + 5, pos2D.y - 5, { r, g, b, hitmarkers[ i ].alpha } );
		render::line( pos2D.x - 2, pos2D.y + 2, pos2D.x - 5, pos2D.y + 5, { r, g, b, hitmarkers[ i ].alpha } );
	}

	// proceeed
	for( int i = 0; i < hitmarkers.size( ); i++ ) {
		if( hitmarkers[ i ].time + 1.25f <= g_csgo.m_globals->m_curtime ) {
			hitmarkers[ i ].alpha -= 1;
		}

		if( hitmarkers[ i ].alpha <= 0 )
			hitmarkers.erase( hitmarkers.begin( ) + i );
	}
}

void Visuals::DrawSpread( ) {
	// dont do if dead.
	if( !g_cl.m_processing || g_csgo.m_cl->m_delta_tick == -1 )
		return;

	if( !g_hooks.b[ XOR( "inaccuracyOverlay" ) ] )
		return;

	if( !g_cl.m_weapon || !g_cl.m_weapon_info || !g_cl.m_weapon_type )
		return;

	auto weapon = g_cl.m_weapon;
	if( weapon ) {
		int screen_w, screen_h;
		g_csgo.m_engine->GetScreenSize( screen_w, screen_h );
		int cross_x = screen_w / 2, cross_y = screen_h / 2;

		float recoil_step = screen_h / g_hooks.f[ XOR( "overrideFov" ) ];

		cross_x -= ( int )( g_cl.m_local->m_aimPunchAngle( ).y * recoil_step );
		cross_y += ( int )( g_cl.m_local->m_aimPunchAngle( ).x * recoil_step );

		weapon->UpdateAccuracyPenalty( );
		float inaccuracy = weapon->GetInaccuracy( );
		float spread = weapon->GetSpread( );

		float cone = inaccuracy * spread;
		cone *= screen_h * 0.7f;
		cone *= 90.f / g_hooks.f[ XOR( "overrideFov" ) ];

		for( int seed{ }; seed < 256; ++seed ) {
			g_csgo.RandomSeed( g_csgo.RandomInt( 0, 255 ) + 1 );
			float rand_a = g_csgo.RandomFloat( 0.f, 1.0f );
			float pi_rand_a = g_csgo.RandomFloat( 0.f, 2.0f * math::pi );
			float rand_b = g_csgo.RandomFloat( 0.0f, 1.0f );
			float pi_rand_b = g_csgo.RandomFloat( 0.f, 2.f * math::pi );

			float spread_x = cos( pi_rand_a ) * ( rand_a * inaccuracy ) + cos( pi_rand_b ) * ( rand_b * spread );
			float spread_y = sin( pi_rand_a ) * ( rand_a * inaccuracy ) + sin( pi_rand_b ) * ( rand_b * spread );

			float max_x = cos( pi_rand_a ) * cone + cos( pi_rand_b ) * cone;
			float max_y = sin( pi_rand_a ) * cone + sin( pi_rand_b ) * cone;

			float step = screen_h / g_hooks.f[ XOR( "overrideFov" ) ] * 90.f;
			int screen_spread_x = ( int )( spread_x * step * 0.7f );
			int screen_spread_y = ( int )( spread_y * step * 0.7f );

			float percentage = ( rand_a * inaccuracy + rand_b * spread ) / ( inaccuracy + spread );

			render::rect( cross_x + screen_spread_x, cross_y + screen_spread_y, 5, 5,
						  Color( 255, 255, 255, 255 * ( 0.4f + percentage * 0.6f ) ) * ( 0.1f + percentage * 0.9f ) );
		}
	}
}

void Visuals::AutomaticPeekIndicator( ) {
	// dont do if dead.
	if( !g_cl.m_processing )
		return;

	auto weapon = g_cl.m_local->GetActiveWeapon( );

	if( !weapon )
		return;

	static auto position = vec3_t( 0.f, 0.f, 0.f );

	if( !g_movement.start_position.IsZero( ) )
		position = g_movement.start_position;

	if( position.IsZero( ) )
		return;

	static auto alpha = 0.0f;

	if( g_hooks.b[ XOR( "auto_peek_ind" ) ] ) {
		if( g_hooks.b[ XOR( "auto_peek" ) ] && g_hooks.auto_check( XOR( "auto_peek_key" ) ) || alpha ) {

			if( g_hooks.auto_check( XOR( "auto_peek_key" ) ) )
				alpha += 3.0f * g_csgo.m_globals->m_curtime; //-V807
			else
				alpha -= 3.0f * g_csgo.m_globals->m_curtime;

			Color color = g_hooks.imcolor_to_ccolor( g_hooks.c[ "auto_peek_active" ] );

			if( !g_movement.fired_shot )
				color = g_hooks.imcolor_to_ccolor( g_hooks.c[ "auto_peek_inactive" ] );

			math::clamp( alpha, 0.0f, 1.0f );
			render::Draw3DFilledCircle( position, 25.0f, g_movement.fired_shot ? Color( g_hooks.c[ "auto_peek_active" ][ 0 ] * 255.0f, g_hooks.c[ "auto_peek_active" ][ 1 ] * 255.0f, g_hooks.c[ "auto_peek_active" ][ 2 ] * 255.0f, ( int )( alpha * 255.0f ) ) : Color( g_hooks.c[ "auto_peek_inactive" ][ 0 ] * 255.0f, g_hooks.c[ "auto_peek_inactive" ][ 1 ] * 255.0f, g_hooks.c[ "auto_peek_inactive" ][ 2 ] * 255.0f, ( int )( alpha * 255.0f ) ) );
		}
	}
}

void Visuals::DrawDamageIndicator( ) {
	// dont do if dead.
	if( !g_cl.m_processing || !g_hooks.b[ XOR( "hitmarker_quake" ) ] )
		return;

	float CurrentTime = g_cl.m_local->m_nTickBase( ) * g_csgo.m_globals->m_interval;

	// TODO;- сделать анимацию на альфу.
	Color color = Color( g_hooks.c[ XOR( "hitmarker_quake_c" ) ][ 0 ] * 255, g_hooks.c[ XOR( "hitmarker_quake_c" ) ][ 1 ] * 255, g_hooks.c[ XOR( "hitmarker_quake_c" ) ][ 2 ] * 255 );

	for( int i = 0; i < DamageIndicator.size( ); ++i ) {
		if( DamageIndicator[ i ].flEraseTime < CurrentTime ) {
			DamageIndicator.erase( DamageIndicator.begin( ) + i );
			continue;
		}

		if( !DamageIndicator[ i ].bInitialized ) {
			DamageIndicator[ i ].Position = DamageIndicator[ i ].Player->GetBonePosition( DamageIndicator[ i ].ihitbox );
			DamageIndicator[ i ].bInitialized = true;
		}

		if( CurrentTime - DamageIndicator[ i ].flLastUpdate > 0.0001f ) {
			DamageIndicator[ i ].Position.z -= ( 0.1f * ( CurrentTime - DamageIndicator[ i ].flEraseTime ) );
			DamageIndicator[ i ].flLastUpdate = CurrentTime;
		}

		vec2_t ScreenPosition;
		if( render::WorldToScreen( DamageIndicator[ i ].Position, ScreenPosition ) )
			render::damage.string( ScreenPosition.x, ScreenPosition.y, color, std::to_string( DamageIndicator[ i ].iDamage ).c_str( ) );
	}
}

void Visuals::ManualAntiAim( ) {
	int  x, y;

	// dont do if dead.
	if( !g_cl.m_processing )
		return;

	if( !g_hooks.b[ XOR( "manual_aa" ) ] )
		return;

	x = g_cl.m_width / 2;
	y = g_cl.m_height / 2;

	Color color = Color( g_hooks.c[ XOR( "manual_aa_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "manual_aa_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "manual_aa_color" ) ][ 2 ] * 255.f, g_hooks.c[ XOR( "manual_aa_color" ) ][ 3 ] * 255.f );

	if( g_cl.m_local->m_bIsScoped( ) )
		color.a( ) = 25;

	if( g_hvh.direction == 1 ) {
		render::rect_filled( x - 50, y - 7, 1, 1, color );
		render::rect_filled( x - 50, y - 7 + 14, 1, 1, color );
		render::rect_filled( x - 51, y - 7, 1, 3, color );
		render::rect_filled( x - 51, y - 7 + 12, 1, 3, color );
		render::rect_filled( x - 52, y - 7 + 1, 1, 4, color );
		render::rect_filled( x - 52, y - 7 + 1 + 9, 1, 4, color );
		render::rect_filled( x - 53, y - 7 + 1, 1, 5, color );
		render::rect_filled( x - 53, y - 7 + 1 + 8, 1, 5, color );
		render::rect_filled( x - 54, y - 7 + 2, 1, 5, color );
		render::rect_filled( x - 54, y - 7 + 1 + 7, 1, 5, color );
		render::rect_filled( x - 55, y - 7 + 2, 1, 11, color );
		render::rect_filled( x - 56, y - 7 + 3, 1, 9, color );
		render::rect_filled( x - 57, y - 7 + 3, 1, 9, color );
		render::rect_filled( x - 58, y - 7 + 4, 1, 7, color );
		render::rect_filled( x - 59, y - 7 + 4, 1, 7, color );
		render::rect_filled( x - 60, y - 7 + 5, 1, 5, color );
		render::rect_filled( x - 61, y - 7 + 5, 1, 5, color );
		render::rect_filled( x - 62, y - 7 + 6, 1, 3, color );
		render::rect_filled( x - 63, y - 7 + 6, 1, 3, color );
		render::rect_filled( x - 64, y - 7 + 7, 1, 1, color );
	}

	else if( g_hvh.direction == 2 ) {
		render::rect_filled( x + 50, y - 7, 1, 1, color );
		render::rect_filled( x + 50, y - 7 + 14, 1, 1, color );
		render::rect_filled( x + 51, y - 7, 1, 3, color );
		render::rect_filled( x + 51, y - 7 + 12, 1, 3, color );
		render::rect_filled( x + 52, y - 7 + 1, 1, 4, color );
		render::rect_filled( x + 52, y - 7 + 1 + 9, 1, 4, color );
		render::rect_filled( x + 53, y - 7 + 1, 1, 5, color );
		render::rect_filled( x + 53, y - 7 + 1 + 8, 1, 5, color );
		render::rect_filled( x + 54, y - 7 + 2, 1, 5, color );
		render::rect_filled( x + 54, y - 7 + 1 + 7, 1, 5, color );
		render::rect_filled( x + 55, y - 7 + 2, 1, 11, color );
		render::rect_filled( x + 56, y - 7 + 3, 1, 9, color );
		render::rect_filled( x + 57, y - 7 + 3, 1, 9, color );
		render::rect_filled( x + 58, y - 7 + 4, 1, 7, color );
		render::rect_filled( x + 59, y - 7 + 4, 1, 7, color );
		render::rect_filled( x + 60, y - 7 + 5, 1, 5, color );
		render::rect_filled( x + 61, y - 7 + 5, 1, 5, color );
		render::rect_filled( x + 62, y - 7 + 6, 1, 3, color );
		render::rect_filled( x + 63, y - 7 + 6, 1, 3, color );
		render::rect_filled( x + 64, y - 7 + 7, 1, 1, color );
	}

	else if( g_hvh.direction == 0 ) {
		render::rect_filled( x - 10 + 10, y + 60, 1, 1, color );
		render::rect_filled( x - 10 + 9, y + 60 - 1, 3, 1, color );
		render::rect_filled( x - 10 + 9, y + 60 - 2, 3, 1, color );
		render::rect_filled( x - 10 + 8, y + 60 - 3, 5, 1, color );
		render::rect_filled( x - 10 + 8, y + 60 - 4, 5, 1, color );
		render::rect_filled( x - 10 + 7, y + 60 - 5, 7, 1, color );
		render::rect_filled( x - 10 + 7, y + 60 - 6, 7, 1, color );
		render::rect_filled( x - 10 + 6, y + 60 - 7, 9, 1, color );
		render::rect_filled( x - 10 + 6, y + 60 - 8, 9, 1, color );
		render::rect_filled( x - 10 + 5, y + 60 - 9, 11, 1, color );
		render::rect_filled( x - 10 + 5, y + 60 - 10, 5, 1, color );
		render::rect_filled( x - 10 + 4, y + 60 - 11, 5, 1, color );
		render::rect_filled( x - 10 + 4, y + 60 - 12, 4, 1, color );
		render::rect_filled( x - 10 + 3, y + 60 - 13, 3, 1, color );
		render::rect_filled( x - 10 + 3, y + 60 - 14, 1, 1, color );
		render::rect_filled( x - 10 + 5 + 6, y + 60 - 10, 5, 1, color );
		render::rect_filled( x - 10 + 5 + 7, y + 60 - 11, 5, 1, color );
		render::rect_filled( x - 10 + 5 + 8, y + 60 - 12, 4, 1, color );
		render::rect_filled( x - 10 + 5 + 10, y + 60 - 13, 3, 1, color );
		render::rect_filled( x - 10 + 5 + 12, y + 60 - 14, 1, 1, color );
	}
}

void Visuals::Spectators( ) {
	// dont do if dead.
	if( !g_cl.m_processing )
		return;

	if( g_hooks.i[ XOR( "spectators" ) ] != 1 )
		return;

	std::vector< std::string > spectators{ XOR( "" ) };
	int h = render::esp.m_size.m_height;

	for( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i ) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );
		if( !player )
			continue;

		if( player->m_bIsLocalPlayer( ) )
			continue;

		if( player->dormant( ) )
			continue;

		if( player->m_lifeState( ) == LIFE_ALIVE || player->m_iHealth( ) > 0 )
			continue;

		if( player->GetObserverTarget( ) != g_cl.m_local )
			continue;

		player_info_t info;
		if( !g_csgo.m_engine->GetPlayerInfo( i, &info ) )
			continue;

		spectators.push_back( std::string( info.m_name ).substr( 0, 24 ) );
	}

	size_t total_size = spectators.size( ) * ( h - 1 );

	for( size_t i{ }; i < spectators.size( ); ++i ) {
		const std::string& name = spectators[ i ];

		render::esp.string( g_cl.m_width - 15, ( i * 14 ) - 10, { 220, 220, 220, 179 }, name, render::ALIGN_RIGHT );
	}
}

void Visuals::StatusIndicators( ) {
	//render::esp_small.string(  500, 500, { 255,255,255,255 }, tfm::format(  XOR(  "m_vecViewOffset: x:%f, y:%f, z:%f"  ), g_cl.m_local->m_vecViewOffset( ).x, g_cl.m_local->m_vecViewOffset( ).y, g_cl.m_local->m_vecViewOffset( ).z ), render::ALIGN_RIGHT  );
	//render::esp_small.string(  500, 550, { 255,255,255,255 }, tfm::format(  XOR(  "m_vecOrigin: x:%f, y:%f, z:%f"  ), g_cl.m_local->m_vecOrigin( ).x, g_cl.m_local->m_vecOrigin( ).y, g_cl.m_local->m_vecOrigin( ).z ), render::ALIGN_RIGHT );
	// dont do if dead.
	if( !g_cl.m_processing )
		return;

	struct Indicator_t { Color color; std::string text; };
	std::vector< Indicator_t > indicators{ };
	float ping = 0;
	// latency.
	float latency = 0.f;
	INetChannel* nci = g_csgo.m_engine->GetNetChannelInfo( );
	if( nci ) { latency = nci->GetAvgLatency( INetChannel::FLOW_INCOMING ) + nci->GetAvgLatency( INetChannel::FLOW_OUTGOING ); }
	int ms = std::max( 0, ( int )std::round( ( ( latency * 1000.f ) * 1.1f ) ) );
	int pingammount = g_hooks.f[ XOR( "fake_latency_amt" ) ];
	int color = std::max( 0, ( int )std::clamp<int>( ms - pingammount, 0, 100 ) );
	// fps.
	int m_Framerate = 1 / g_csgo.m_globals->m_frametime;
	int fps = floor( m_Framerate );
	// fatality.win
	auto ping1 = g_hooks.f[ XOR( "fake_latency_amt" ) ] / 1000.f;
	if( !g_hooks.f[ XOR( "fake_latency" ) ] )
		ping1 = 160.f / 1000.f;
	auto multiplier = nci->GetAvgLatency( INetChannel::FLOW_INCOMING ) + nci->GetAvgLatency( INetChannel::FLOW_OUTGOING ) / ping1;
	multiplier = std::clamp( multiplier, 0.f, 1.f );

	const auto red = std::clamp( static_cast< int >( ( 1 - multiplier ) * 120 ) + 124, 0, 255 );
	const auto green = std::clamp( static_cast< int >( multiplier * 255 ) + 195, 0, 255 );

	/*
	* #DEBUG
	* render::esp_small.string(  500, 500, { 192 - (  color * 37 / 106  ), 32 + (  color * 136 / 106  ), 17, 255 }, std::to_string(  ms  ).c_str(    ), render::ALIGN_CENTER  );
	* render::esp_small.string(  500, 550, { 255,255,255,255 }, std::to_string(  g_cl.m_local->m_flDuckAmount(    )  ).c_str(    ), render::ALIGN_CENTER  );
	*/

	// PING
	if( g_hooks.b[ XOR( "fake_latency" ) ] && g_hooks.auto_check( XOR( "fake_latency_key" ) ) ) {
		Indicator_t ind{ };
		ind.color = { 192 - ( color * 37 / 106 ), 32 + ( color * 136 / 106 ), 17, 255 };
		ind.text = XOR( "PING" );
		ping = 30;

		indicators.push_back( ind );
	}

	// LC
	if( g_hooks.b[ XOR( "enable_fakelag" ) ] && ( ( g_cl.m_buttons & IN_JUMP ) || !( g_cl.m_flags & FL_ONGROUND ) ) ) {
		Indicator_t ind{ };
		ind.color = g_cl.m_lagcomp ? 0xff15c27b : 0xff0000ff;
		ind.text = XOR( "LC" );

		indicators.push_back( ind );
	}

	// LBY
	if( g_hooks.i[ XOR( "yaw" ) ] > 0 ) {
		// get the absolute change between current lby and animated angle.
		float change = std::abs( math::NormalizedAngle( g_cl.m_body - g_cl.m_angle.y ) );

		Indicator_t ind{ };
		ind.color = change > 35.f ? 0xff15c27b : 0xff0000ff;
		ind.text = XOR( "LBY" );
		indicators.push_back( ind );
	}

	// BAIM
	if( g_hooks.auto_check( XOR( "damage1_key" ) ) && g_hooks.b[ XOR( "body_aim_enable" ) ] ) {
		Indicator_t ind{ };
		ind.color = 0xff15c27b;
		ind.text = XOR( "BAIM" );

		indicators.push_back( ind );
	}

	// HP
	if( g_hooks.auto_check( XOR( "damage_key" ) ) && g_hooks.b[ XOR( "override_hp" ) ] ) {
		Indicator_t ind{ };
		ind.color = 0xff15c27b;
		ind.text = XOR( "HP" );

		indicators.push_back( ind );
	}

	// DT
	if( g_hooks.auto_check( XOR( "rapidfire_key" ) ) && g_hooks.b[ XOR( "rapidfire" ) ] ) {
		Indicator_t ind{ };
		ind.color = g_cl.m_charged ? 0xff15c27b : 0xff0000ff;
		ind.text = XOR( "DT" );

		indicators.push_back( ind );
	}

	if( indicators.empty( ) )
		return;

	// iterate and draw indicators.
	for( size_t i{ }; i < indicators.size( ); ++i ) {
		auto& indicator = indicators[ i ];

		render::indicator.string( 12, g_cl.m_height - 84 - ( 30 * i ) - g_hooks.f[ XOR( "keybinds_y" ) ], indicator.color, indicator.text );
	}

	float time_remain_to_update = g_cl.m_body_pred - g_csgo.m_globals->m_curtime;

	float fill = 0;
	fill = time_remain_to_update;
	static float add = 0.000f;
	add = 1.1f - fill;
	math::clamp( add, 0.f, 1.f );

	float change1337 = std::abs( math::NormalizedAngle( g_cl.m_body - g_cl.m_angle.y ) );
	Color color1337 = { 255, 0, 0, 255 };
	if( change1337 > 35.f ) {
		color1337 = { 124, 195, 13, 255 };
	}

	Color black = colors::black;
	black.a( ) = 180;

	int weight = render::indicator.size( XOR( "LBY" ) ).m_width;
	if( !( ( g_cl.m_buttons & IN_JUMP ) || !( g_cl.m_flags & FL_ONGROUND ) ) && g_hooks.i[ XOR( "yaw" ) ] > 0 && g_cl.m_speed < 1.f ) {
		//render::rect_filled( 13, g_cl.m_height - 84 - ping + 24, ( weight - 2 ), 4, { 10, 10, 10, 180 } );
		//render::rect_filled( 14, g_cl.m_height - 83 - ping + 24, ( weight - 4 ) * add, 2, color1337 );
		render::arc( 70, g_cl.m_height - 94 - ping + 24 - g_hooks.f[ XOR( "keybinds_y" ) ], 9, 4, 0, 360, 30, black );
		render::arc( 70, g_cl.m_height - 94 - ping + 24 - g_hooks.f[ XOR( "keybinds_y" ) ], 8, 5, 0, 360 * ( 1.f - add ), 30, color1337 );
	}
}

struct CUSTOMVERTEX {
	FLOAT x, y, z;
	FLOAT rhw;
	DWORD color;
	// FLOAT tu, tv; 
};

void CircleFilledDualColor( float x, float y, float rad, float rotate, int type, int resolution, DWORD color, DWORD color2, IDirect3DDevice9* m_device ) {
	LPDIRECT3DVERTEXBUFFER9 g_pVB2;

	std::vector<CUSTOMVERTEX> circle( resolution + 2 );

	float angle = rotate * D3DX_PI / 180, pi = D3DX_PI;

	if( type == 1 )
		pi = D3DX_PI; // Full circle
	if( type == 2 )
		pi = D3DX_PI / 2; // 1/2 circle
	if( type == 3 )
		pi = D3DX_PI / 4; // 1/4 circle

	pi = D3DX_PI / type; // 1/4 circle

	circle[ 0 ].x = x;
	circle[ 0 ].y = y;
	circle[ 0 ].z = 0;
	circle[ 0 ].rhw = 1;
	circle[ 0 ].color = color2;

	for( int i = 1; i < resolution + 2; i++ ) {
		circle[ i ].x = ( float )( x - rad * cos( pi * ( ( i - 1 ) / ( resolution / 2.0f ) ) ) );
		circle[ i ].y = ( float )( y - rad * sin( pi * ( ( i - 1 ) / ( resolution / 2.0f ) ) ) );
		circle[ i ].z = 0;
		circle[ i ].rhw = 1;
		circle[ i ].color = color;
	}

	// Rotate matrix
	int _res = resolution + 2;
	for( int i = 0; i < _res; i++ ) {
		circle[ i ].x = x + cos( angle ) * ( circle[ i ].x - x ) - sin( angle ) * ( circle[ i ].y - y );
		circle[ i ].y = y + sin( angle ) * ( circle[ i ].x - x ) + cos( angle ) * ( circle[ i ].y - y );
	}

	m_device->CreateVertexBuffer( ( resolution + 2 ) * sizeof( CUSTOMVERTEX ), D3DUSAGE_WRITEONLY, D3DFVF_XYZRHW | D3DFVF_DIFFUSE, D3DPOOL_DEFAULT, &g_pVB2, NULL );

	VOID* pVertices;
	g_pVB2->Lock( 0, ( resolution + 2 ) * sizeof( CUSTOMVERTEX ), ( void** )&pVertices, 0 );
	memcpy( pVertices, &circle[ 0 ], ( resolution + 2 ) * sizeof( CUSTOMVERTEX ) );
	g_pVB2->Unlock( );

	m_device->SetTexture( 0, NULL );
	m_device->SetPixelShader( NULL );
	m_device->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	m_device->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	m_device->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	m_device->SetStreamSource( 0, g_pVB2, 0, sizeof( CUSTOMVERTEX ) );
	m_device->SetFVF( D3DFVF_XYZRHW | D3DFVF_DIFFUSE );
	m_device->DrawPrimitive( D3DPT_TRIANGLEFAN, 0, resolution );
	if( g_pVB2 != NULL )
		g_pVB2->Release( );
}

void Visuals::SpreadCrosshair( ) {
	// dont do if dead.
	if( !g_cl.m_processing || g_csgo.m_cl->m_delta_tick == -1 )
		return;

	if( !g_hooks.b[ XOR( "inaccuracyOverlay" ) ] )
		return;

	if( !g_cl.m_weapon || !g_cl.m_weapon_info || !g_cl.m_weapon_type )
		return;

	// get active weapon.
	Weapon* weapon = g_cl.m_local->GetActiveWeapon( );
	if( !weapon )
		return;

	WeaponInfo* data = weapon->GetWpnData( );
	if( !data )
		return;

	// do not do this on: bomb, knife and nades.
	CSWeaponType type = data->m_weapon_type;
	if( !type || type == WEAPONTYPE_KNIFE || type == WEAPONTYPE_C4 || type == WEAPONTYPE_GRENADE )
		return;

	// calc radius.
	float radius = ( ( weapon->GetInaccuracy( ) + weapon->GetSpread( ) ) * 320.f ) / ( std::tan( math::deg_to_rad( g_cl.m_local->GetFOV( ) ) * 0.5f ) + FLT_EPSILON );

	// scale by screen size.
	radius *= g_cl.m_height * ( 1.f / 480.f );

	// get color.
	int r = g_hooks.c[ XOR( "inaccuracyOverlay_color" ) ][ 0 ] * 255.f;
	int g = g_hooks.c[ XOR( "inaccuracyOverlay_color" ) ][ 1 ] * 255.f;
	int b = g_hooks.c[ XOR( "inaccuracyOverlay_color" ) ][ 2 ] * 255.f;
	int a = 200 * ( g_hooks.c[ XOR( "inaccuracyOverlay_color" ) ][ 3 ] );

	int segements = std::max( 16, ( int )std::round( radius * 0.75f ) );
	CircleFilledDualColor( g_cl.m_width / 2, g_cl.m_height / 2, radius, 0, 1, 50, D3DCOLOR_RGBA( r, g, b, a ), D3DCOLOR_RGBA( 0, 0, 0, 0 ), g_csgo.m_device );
}

void Visuals::PenetrationCrosshair( ) {
	int  x, y;
	bool valid_player_hit;
	Color final_color;

	if( !g_hooks.b[ XOR( "penetrationReticle" ) ] || !g_cl.m_processing )
		return;

	x = g_cl.m_width / 2;
	y = g_cl.m_height / 2;

	valid_player_hit = ( g_cl.m_pen_data.m_target && g_cl.m_pen_data.m_target->enemy( g_cl.m_local ) );
	if( valid_player_hit )
		final_color = colors::light_blue;

	else if( g_cl.m_pen_data.m_pen )
		final_color = colors::transparent_green;

	else
		final_color = colors::transparent_red;

	// todo - dex; use fmt library to get damage string here?
	//       draw damage string?

	// draw small square in center of screen.
	int damage1337 = g_cl.m_pen_data.m_damage;

	if( g_cl.m_pen_data.m_damage > 0 ) {
		render::rect_filled( x - 1, y, 1, 1, { final_color } );
		render::rect_filled( x, y, 1, 1, { final_color } );
		render::rect_filled( x + 1, y, 1, 1, { final_color } );
		render::rect_filled( x, y + 1, 1, 1, { final_color } );
		render::rect_filled( x, y - 1, 1, 1, { final_color } );
		//shadow
		render::rect_filled( x - 2, y, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x + 1, y - 1, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x + 2, y, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x, y + 2, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x, y - 2, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x + 1, y - 2, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x + 1, y + 1, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x + 2, y + 1, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x - 1, y + 1, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x - 1, y + 2, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x - 2, y + 1, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x + 1, y + 2, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x - 1, y - 1, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x - 1, y - 2, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x - 2, y - 1, 1, 1, { 0,0, 0, 125 } );
		render::rect_filled( x + 2, y - 1, 1, 1, { 0,0, 0, 125 } );
	}
}

void Visuals::draw( Entity* ent ) {
	if( ent->IsPlayer( ) ) {
		Player* player = ent->as< Player* >( );

		// dont draw dead players.
		if( !player->alive( ) )
			return;

		if( player->m_bIsLocalPlayer( ) )
			return;

		if( c_playerlist::get( )->should_disable_visuals( player ) )
			return;

		// draw player esp.
		g_sound.Start( );
		if( !player->dormant( ) ) g_sound.m_cSoundPlayers[ player->index( ) ].reset( true, player->GetAbsOrigin( ), player->m_fFlags( ) );
		else g_sound.AdjustPlayerBegin( player );
		DrawPlayer( player );
		g_sound.Finish( );
	}

	else if( g_hooks.i[ XOR( "droppedWeapons_sel" ) ] > 0 && ent->IsBaseCombatWeapon( ) && !ent->dormant( ) )
		DrawItem( ent->as< Weapon* >( ) );

	else if( g_hooks.b[ XOR( "grenades" ) ] )
		DrawProjectile( ent->as< Weapon* >( ) );
}

void convex_hull( std::vector<vec2_t>& points ) {
	if( points.size( ) <= 3 )
		return;

	std::sort( points.begin( ), points.end( ), [ ]( vec2_t left, vec2_t right ) {
		return left.x < right.x || left.x == right.x && left.y < right.y;
	}
	);

	auto orientation = [ & ]( vec2_t a, vec2_t b, vec2_t c ) -> float {
		return a.x * ( b.y - c.y ) + b.x * ( c.y - a.y ) + c.x * ( a.y - b.y );
	};

	vec2_t
		front = points.front( ),
		back = points.back( );

	std::vector<vec2_t> up, down;

	up.push_back( front );
	down.push_back( back );

	for( size_t i = 1; i < points.size( ); ++i ) {
		if( i == points.size( ) - 1 || orientation( front, points[ i ], back ) > 0 ) {
			while( up.size( ) >= 2 && orientation( up[ up.size( ) - 2 ], up[ up.size( ) - 1 ], points[ i ] ) < 0 ) {
				up.pop_back( );
			}
			up.push_back( points[ i ] );
		}

		if( i == points.size( ) - 1 || orientation( front, points[ i ], back ) < 0 ) {
			while( down.size( ) >= 2 && orientation( down[ down.size( ) - 2 ], down[ down.size( ) - 1 ], points[ i ] ) > 0 ) {
				down.pop_back( );
			}
			down.push_back( points[ i ] );
		}
	}

	points.clear( );

	for( auto& u : up ) {
		points.emplace_back( u );
	}

	for( size_t i = down.size( ) - 2; i > 0; --i ) {
		points.emplace_back( down[ i ] );
	}
};

void RenderCircle3D( vec3_t vecPosition, int32_t iPointCount, float_t flRadius, Color aColor ) {
	vec2_t screen;
	if( !render::WorldToScreen( vecPosition, screen ) )
		return;

	float_t flStep = ( float_t )( 3.14159265358979323846f ) * 2.0f / ( float_t )( iPointCount );
	for( float a = 0; a < ( 3.14159265358979323846f * 2.0f ); a += flStep ) {
		vec3_t vecStart = vec3_t( flRadius * cosf( a ) + vecPosition.x, flRadius * sinf( a ) + vecPosition.y, vecPosition.z );
		vec3_t vecEnd = vec3_t( flRadius * cosf( a + flStep ) + vecPosition.x, flRadius * sinf( a + flStep ) + vecPosition.y, vecPosition.z );

		vec2_t vecStart2D, vecEnd2D;
		render::WorldToScreen( vecStart, vecStart2D );
		render::WorldToScreen( vecEnd, vecEnd2D );

		if( !vecStart2D.IsZero( ) && !vecEnd2D.IsZero( ) && vecStart2D.valid( ) && vecEnd2D.valid( ) )
			render::line( vecStart2D.x, vecStart2D.y, vecEnd2D.x, vecEnd2D.y, aColor );
	}
}

void Visuals::DrawProjectile( Weapon* ent ) {
	vec2_t screen;
	vec3_t origin = ent->GetAbsOrigin( );
	if( !render::WorldToScreen( origin, screen ) )
		return;

	Color col = Color( g_hooks.c[ XOR( "grenades_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "grenades_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "grenades_color" ) ][ 2 ] * 255.f );
	col.a( ) = 0xb4;

	// draw decoy.
	if( ent->is( HASH( "CDecoyProjectile" ) ) )
		render::esp_small.string( screen.x, screen.y, col, XOR( "DECOY" ), render::ALIGN_CENTER );

	// draw molotov.
	else if( ent->is( HASH( "CMolotovProjectile" ) ) )
		render::esp_small.string( screen.x, screen.y, col, XOR( "MOLLY" ), render::ALIGN_CENTER );

	else if( ent->is( HASH( "CBaseCSGrenadeProjectile" ) ) ) {
		const model_t* model = ent->GetModel( );

		if( model ) {
			// grab modelname.
			std::string name{ ent->GetModel( )->m_name };

			if( name.find( XOR( "flashbang" ) ) != std::string::npos )
				render::esp_small.string( screen.x, screen.y, col, XOR( "FLASH" ), render::ALIGN_CENTER );

			else if( name.find( XOR( "fraggrenade" ) ) != std::string::npos ) {

				// grenade range.
				if( g_menu.main.visuals.proj_range.get( 0 ) )
					render::sphere( origin, 350.f, 5.f, 1.f, g_menu.main.visuals.proj_range_color.get( ) );

				render::esp_small.string( screen.x, screen.y, col, XOR( "FRAG" ), render::ALIGN_CENTER );
			}
		}
	}

	// find classes.
	else if( ent->is( HASH( "CInferno" ) ) ) {
		float spawn_time = ent->m_SpawnTime( );
		float factor = ( spawn_time + 7.03125f - g_csgo.m_globals->m_curtime ) / 7.03125f;

		if( g_hooks.b[ XOR( "molotv_timer" ) ] && factor > 0 ) {
			render::rect_filled( screen.x - 14, screen.y + 9, 30, 4, { 0, 0, 0, 180 } );
			render::rect_filled( screen.x - 14 + 1, screen.y + 9 + 1, 29 * factor, 2, Color( g_hooks.c[ XOR( "molotv_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "molotv_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "molotv_color" ) ][ 2 ] * 255.f ) );
		}

		/*static const auto flame_polygon = [ ] {
			std::array<vec3_t, 3> points;
			for( std::size_t i = 0; i < points.size( ); ++i ) {
				points[ i ] = vec3_t{ 60.0f * std::cos( math::deg_to_rad ( i * ( 360.0f / points.size( ) ) ) ),
									60.0f * std::sin( math::deg_to_rad( i * ( 360.0f / points.size( ) ) ) ),
									0.0f };
			}
			return points;
		}( );

		std::vector<vec3_t> points;

		for( int i = 0; i <= ent->m_fireCount( ); i++ )
			points.push_back( ent->m_vecOrigin( ) + vec3_t( ent->m_fireXDelta( )[ i ], ent->m_fireYDelta( )[ i ], ent->m_fireZDelta( )[ i ] ) );

		std::vector<vec2_t> screen_points;

		for( const auto& pos : points ) {
			for( const auto& point : flame_polygon ) {
				vec2_t screen;

				if( render::WorldToScreen( pos + point, screen ) )
					screen_points.push_back( screen );
			}
		}

		convex_hull( screen_points );
		if( !screen_points.empty( ) )
			render::TexturedPolygon( screen_points, Color( 255, 0, 0, 150 ) );*/

		Color color;
		color = g_hooks.imcolor_to_ccolor( g_hooks.c[ XOR( "grenadeWarning_enemy_color" ) ] );

		if( g_visuals.DistInFt( g_cl.m_local->m_vecOrigin( ), ent->m_vecOrigin( ) ) < 50.f )
			RenderCircle3D( ent->m_vecOrigin( ), 32, 150, color );

		render::esp_small.string( screen.x, screen.y, col, XOR( "FIRE" ), render::ALIGN_CENTER );
	}

	else if( ent->is( HASH( "CSmokeGrenadeProjectile" ) ) ) {
		auto spawn_time = ent->m_SpawnTime( );
		auto factor = ( spawn_time + 19.0f - g_csgo.m_globals->m_curtime ) / 19.0f;

		if( g_hooks.b[ XOR( "smoke_timer" ) ] && factor > 0 && ent->m_vecVelocity( ).length_2d( ) == 0.f ) {
			render::rect_filled( screen.x - 14, screen.y + 9, 30, 4, { 0, 0, 0, 180 } );
			render::rect_filled( screen.x - 14 + 1, screen.y + 9 + 1, 29 * factor, 2, Color( g_hooks.c[ XOR( "smoke_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "smoke_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "smoke_color" ) ][ 2 ] * 255.f ) );
		}

		Color color;
		color = g_hooks.imcolor_to_ccolor( g_hooks.c[ XOR( "grenadeWarning_enemy_color" ) ] );

		if( g_visuals.DistInFt( g_cl.m_local->m_vecOrigin( ), ent->m_vecOrigin( ) ) < 50.f && ent->m_vecVelocity( ).length_2d( ) == 0.f )
			RenderCircle3D( ent->m_vecOrigin( ), 32, 144, color );

		render::esp_small.string( screen.x, screen.y, col, XOR( "SMOKE" ), render::ALIGN_CENTER );
	}
}

void Visuals::DrawItem( Weapon* item ) {
	// we only want to draw shit without owner.
	Entity* owner = g_csgo.m_entlist->GetClientEntityFromHandle( item->m_hOwnerEntity( ) );
	if( owner )
		return;

	// is the fucker even on the screen?
	vec2_t screen;
	vec3_t origin = item->GetAbsOrigin( );
	if( !render::WorldToScreen( origin, screen ) )
		return;

	WeaponInfo* data = item->GetWpnData( );
	if( !data )
		return;

	Color col = Color( g_hooks.c[ XOR( "droppedWeaponsText_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "droppedWeaponsText_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "droppedWeaponsText_color" ) ][ 2 ] * 255.f );
	col.a( ) = 0xb4;

	Color coldist = Color( g_hooks.c[ XOR( "droppedWeaponsDistance_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "droppedWeaponsDistance_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "droppedWeaponsDistance_color" ) ][ 2 ] * 255.f );
	coldist.a( ) = 0xb4;

	Color col_drop = Color( g_hooks.c[ XOR( "droppedWeaponsAmmo_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "droppedWeaponsAmmo_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "droppedWeaponsAmmo_color" ) ][ 2 ] * 255.f );
	col_drop.a( ) = 0xb4;

	Color col_back = { 0,0,0,180 };

	auto dist_world = g_cl.m_local->m_vecOrigin( ).dist_to( origin );
	if( dist_world > 250.f ) {
		col.a( ) *= std::clamp( ( 750.f - ( dist_world - 250.f ) ) / 750.f, 0.f, 1.f );
		coldist.a( ) *= std::clamp( ( 750.f - ( dist_world - 250.f ) ) / 750.f, 0.f, 1.f );
		col_drop.a( ) *= std::clamp( ( 750.f - ( dist_world - 250.f ) ) / 750.f, 0.f, 1.f );
		col_back.a( ) *= std::clamp( ( 750.f - ( dist_world - 250.f ) ) / 750.f, 0.f, 1.f );
	}

	// render bomb in green.
	if( item->is( HASH( "CC4" ) ) )
		render::esp_small.string( screen.x, screen.y, { 150, 200, 60, 0xb4 }, XOR( "BOMB" ), render::ALIGN_CENTER );

	// if not bomb
	// normal item, get its name.
	else {
		std::string name{ item->GetLocalizedName( ) };

		// smallfonts needs uppercase.
		std::transform( name.begin( ), name.end( ), name.begin( ), ::toupper );
		std::string icon = tfm::format( XOR( "%c" ), m_weapon_icons[ item->m_iItemDefinitionIndex( ) ] );
		std::string ammo = tfm::format( XOR( "(%i/%i)" ), item->m_iClip1( ), item->m_iPrimaryReserveAmmoCount( ) );

		int yes = 0;
		int yes1 = 0;

		if( g_hooks.i[ XOR( "droppedWeapons_sel" ) ] == 2 ) {
			yes = 10;
			yes1 = 15;
		}

		if( g_hooks.b[ XOR( "droppedWeaponsDistance" ) ] )
			render::esp_small.string( screen.x, screen.y - 13 - yes, coldist, tfm::format( XOR( "%i FT" ), DistInFt( g_cl.m_local->m_vecOrigin( ), item->m_vecOrigin( ) ) ), render::ALIGN_CENTER );

		if( g_hooks.i[ XOR( "droppedWeapons_sel" ) ] == 1 ) // TEXT
			render::esp_small.string( screen.x, screen.y - 3, col, name, render::ALIGN_CENTER );

		else if( g_hooks.i[ XOR( "droppedWeapons_sel" ) ] == 2 ) // ICON
			render::weapon.string( screen.x, screen.y - 14, col, icon, render::ALIGN_CENTER );
	}

	//if (  g_menu.main.visuals.ammo.get(  0 ) )
	//	return;

	// nades do not have ammo.
	if( data->m_weapon_type == WEAPONTYPE_GRENADE || data->m_weapon_type == WEAPONTYPE_KNIFE )
		return;

	if( item->m_iItemDefinitionIndex( ) == 0 || item->m_iItemDefinitionIndex( ) == C4 )
		return;

	std::string name{ item->GetLocalizedName( ) };
	std::transform( name.begin( ), name.end( ), name.begin( ), ::toupper );
	std::string icon = tfm::format( XOR( "%c" ), m_weapon_icons[ item->m_iItemDefinitionIndex( ) ] );
	int current = item->m_iClip1( );
	int max = data->m_max_clip1;
	float scale = ( float )current / max;
	int bar = ( int )std::round( ( render::esp_small.size( name.c_str( ) ).m_width + 1 - 2 ) * scale );
	int bar_icon = ( int )std::round( ( render::weapon.size( icon.c_str( ) ).m_width + 1 - 2 ) * scale );

	if( g_hooks.b[ XOR( "droppedWeaponsAmmo" ) ] && g_hooks.i[ XOR( "droppedWeapons_sel" ) ] == 1 ) {
		render::rect_filled( screen.x - render::esp_small.size( name.c_str( ) ).m_width / 2, screen.y + 9, render::esp_small.size( name.c_str( ) ).m_width + 1, 4, col_back );
		render::rect_filled( screen.x - render::esp_small.size( name.c_str( ) ).m_width / 2 + 1, screen.y + 1 + 9, bar, 2, col_drop );
	}
	else if( g_hooks.b[ XOR( "droppedWeaponsAmmo" ) ] && g_hooks.i[ XOR( "droppedWeapons_sel" ) ] == 2 ) {
		render::rect_filled( screen.x - render::weapon.size( icon.c_str( ) ).m_width / 2, screen.y + 9 - 10, render::weapon.size( icon.c_str( ) ).m_width + 1, 4, col_back );
		render::rect_filled( screen.x - render::weapon.size( icon.c_str( ) ).m_width / 2 + 1, screen.y + 1 + 9 - 10, bar_icon, 2, col_drop );
	}
}

void Visuals::OffScreen( Player* player, int alpha ) {
	vec3_t view_origin, target_pos, delta;
	vec2_t screen_pos, offscreen_pos;
	float  leeway_x, leeway_y, radius, offscreen_rotation;
	bool   is_on_screen;
	Vertex verts[ 3 ], verts_outline[ 3 ];
	Color  color;

	// todo - dex; move this?
	static auto get_offscreen_data = [ ]( const vec3_t& delta, float radius, vec2_t& out_offscreen_pos, float& out_rotation ) {
		ang_t  view_angles( g_csgo.m_view_render->m_view.m_angles );
		vec3_t fwd, right, up( 0.f, 0.f, 1.f );
		float  front, side, yaw_rad, sa, ca;

		// get viewport angles forward directional vector.
		math::AngleVectors( view_angles, &fwd );

		// convert viewangles forward directional vector to a unit vector.
		fwd.z = 0.f;
		fwd.normalize( );

		// calculate front / side positions.
		right = up.cross( fwd );
		front = delta.dot( fwd );
		side = delta.dot( right );

		// setup offscreen position.
		out_offscreen_pos.x = radius * -side;
		out_offscreen_pos.y = radius * -front;

		// get the rotation ( yaw, 0 - 360 ).
		out_rotation = math::rad_to_deg( std::atan2( out_offscreen_pos.x, out_offscreen_pos.y ) + math::pi );

		// get needed sine / cosine values.
		yaw_rad = math::deg_to_rad( -out_rotation );
		sa = std::sin( yaw_rad );
		ca = std::cos( yaw_rad );

		// rotate offscreen position around.
		out_offscreen_pos.x = ( int )( ( g_cl.m_width / 2.f ) + ( radius * sa ) );
		out_offscreen_pos.y = ( int )( ( g_cl.m_height / 2.f ) - ( radius * ca ) );
	};

	if( !g_hooks.b[ XOR( "outOfFOVArrow" ) ] )
		return;

	if( !g_cl.m_processing || !g_cl.m_local->enemy( player ) )
		return;

	// get the player's center screen position.
	target_pos = player->WorldSpaceCenter( );
	is_on_screen = render::WorldToScreen( target_pos, screen_pos );

	// give some extra room for screen position to be off screen.
	leeway_x = g_cl.m_width / 18.f;
	leeway_y = g_cl.m_height / 18.f;

	// origin is not on the screen at all, get offscreen position data and start rendering.
	if( !is_on_screen
		|| screen_pos.x < -leeway_x
		|| screen_pos.x >( g_cl.m_width + leeway_x )
		|| screen_pos.y < -leeway_y
		|| screen_pos.y >( g_cl.m_height + leeway_y ) ) {

		float size = g_hooks.f[ XOR( "arrowSize" ) ] / 20.f;
		float pos = g_hooks.f[ XOR( "arrowDistance" ) ] * 2.f;

		// get viewport origin.
		view_origin = g_csgo.m_view_render->m_view.m_origin;

		// get direction to target.
		delta = ( target_pos - view_origin ).normalized( );

		// note - dex; this is the 'YRES' macro from the source sdk.
		radius = pos * ( g_cl.m_height / 480.f );

		// get the data we need for rendering.
		get_offscreen_data( delta, radius, offscreen_pos, offscreen_rotation );

		// bring rotation back into range... before rotating verts, sine and cosine needs this value inverted.
		// note - dex; reference: 
		// https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/src_main/game/client/tf/tf_hud_damageindicator.cpp#L182
		offscreen_rotation = -offscreen_rotation;

		// setup vertices for the triangle.
		verts[ 0 ] = { offscreen_pos.x + ( 1 * size ) , offscreen_pos.y + ( 1 * size ) };        // 0,  0
		verts[ 1 ] = { offscreen_pos.x - ( 12.f * size ), offscreen_pos.y + ( 24.f * size ) }; // -1, 1
		verts[ 2 ] = { offscreen_pos.x + ( 12.f * size ), offscreen_pos.y + ( 24.f * size ) }; // 1,  1

		// setup verts for the triangle's outline.
		verts_outline[ 0 ] = { verts[ 0 ].m_pos.x - 1.f, verts[ 0 ].m_pos.y - 1.f };
		verts_outline[ 1 ] = { verts[ 1 ].m_pos.x - 1.f, verts[ 1 ].m_pos.y + 1.f };
		verts_outline[ 2 ] = { verts[ 2 ].m_pos.x + 1.f, verts[ 2 ].m_pos.y + 1.f };

		// rotate all vertices to point towards our target.
		verts[ 0 ] = render::RotateVertex( offscreen_pos, verts[ 0 ], offscreen_rotation );
		verts[ 1 ] = render::RotateVertex( offscreen_pos, verts[ 1 ], offscreen_rotation );
		verts[ 2 ] = render::RotateVertex( offscreen_pos, verts[ 2 ], offscreen_rotation );
		// verts_outline[ 0 ] = render::RotateVertex( offscreen_pos, verts_outline[ 0 ], offscreen_rotation );
		// verts_outline[ 1 ] = render::RotateVertex( offscreen_pos, verts_outline[ 1 ], offscreen_rotation );
		// verts_outline[ 2 ] = render::RotateVertex( offscreen_pos, verts_outline[ 2 ], offscreen_rotation );

		// todo - dex; finish this, i want it.
		// auto &damage_data = m_offscreen_damage[ player->index( ) ];
		// 
		// // the local player was damaged by another player recently.
		// if( damage_data.m_time > 0.f ) {
		//     // // only a small amount of time left, start fading into white again.
		//     // if( damage_data.m_time < 1.f ) {
		//     //     // calculate step needed to reach 255 in 1 second.
		//     //     // float step = UINT8_MAX / ( 1000.f * g_csgo.m_globals->m_frametime );
		//     //     float step = ( 1.f / g_csgo.m_globals->m_frametime ) / UINT8_MAX;
		//     //     
		//     //     // increment the new value for the color.
		//     //     // if( damage_data.m_color_step < 255.f )
		//     //         damage_data.m_color_step += step;
		//     // 
		//     //     math::clamp( damage_data.m_color_step, 0.f, 255.f );
		//     // 
		//     //     damage_data.m_color.g( ) = (uint8_t)damage_data.m_color_step;
		//     //     damage_data.m_color.b( ) = (uint8_t)damage_data.m_color_step;
		//     // }
		//     // 
		//     // g_cl.print( "%f %f %u %u %u\n", damage_data.m_time, damage_data.m_color_step, damage_data.m_color.r( ), damage_data.m_color.g( ), damage_data.m_color.b( ) );
		//     
		//     // decrement timer.
		//     damage_data.m_time -= g_csgo.m_globals->m_frametime;
		// }
		// 
		// else
		//     damage_data.m_color = colors::white;

		// render!
		int alpha1337 = sin( abs( fmod( -math::pi + ( g_csgo.m_globals->m_curtime * ( 2 / .75 ) ), ( math::pi * 2 ) ) ) ) * 255;
		if( alpha1337 < 0 ) alpha1337 = alpha1337 * ( -1 );

		color = Color( g_hooks.c[ XOR( "outOfFOVArrow_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "outOfFOVArrow_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "outOfFOVArrow_color" ) ][ 2 ] * 255.f ); // damage_data.m_color;
		color.a( ) = ( alpha == 255 ) ? alpha1337 : alpha / 2;

		g_csgo.m_surface->DrawSetColor( color );
		g_csgo.m_surface->DrawTexturedPolygon( 3, verts );

		// g_csgo.m_surface->DrawSetColor( colors::black );
		// g_csgo.m_surface->DrawTexturedPolyLine( 3, verts_outline );
	}
}

void Draw3DBox( Player* player, int width, int height, Color outline, Color filling ) {
	float difw = float( width / 2 );
	float difh = float( height / 2 );

	/*vec3_t boxVectors[] = {
		{min[1], min[2], min[3]}, {min[1], max[2], min[3]},
		{max[1], max[2], min[3]}, {max[1], min[2], min[3]},
		{min[1], min[2], max[3]}, {min[1], max[2], max[3]},
		{max[1], max[2], max[3]}, {max[1], min[2], max[3]},
	};*/
	AimPlayer* data = &g_aimbot.m_players[ player->index( ) - 1 ];
	// make sure everything is valid.
	if( !data || data->m_records.empty( ) )
		return;

	// grab lag record.
	LagRecord* record = data->m_records.front( ).get( );
	if( !record )
		return;

	vec3_t boxVectors[ 8 ] =
	{
		vec3_t( record->m_pred_origin.x - difw, record->m_pred_origin.y - difh, record->m_pred_origin.z - difw + 10 ),
		vec3_t( record->m_pred_origin.x - difw, record->m_pred_origin.y - difh, record->m_pred_origin.z + difw + 10 + 48 ),
		vec3_t( record->m_pred_origin.x + difw, record->m_pred_origin.y - difh, record->m_pred_origin.z + difw + 10 + 48 ),
		vec3_t( record->m_pred_origin.x + difw, record->m_pred_origin.y - difh, record->m_pred_origin.z - difw + 10 ),
		vec3_t( record->m_pred_origin.x - difw, record->m_pred_origin.y + difh, record->m_pred_origin.z - difw + 10 ),
		vec3_t( record->m_pred_origin.x - difw, record->m_pred_origin.y + difh, record->m_pred_origin.z + difw + 10 + 48 ),
		vec3_t( record->m_pred_origin.x + difw, record->m_pred_origin.y + difh, record->m_pred_origin.z + difw + 10 + 48 ),
		vec3_t( record->m_pred_origin.x + difw, record->m_pred_origin.y + difh, record->m_pred_origin.z - difw + 10 ),
	};

	static vec2_t vec0, vec1, vec2, vec3,
		vec4, vec5, vec6, vec7;

	if( render::WorldToScreen( boxVectors[ 0 ], vec0 ) &&
		render::WorldToScreen( boxVectors[ 1 ], vec1 ) &&
		render::WorldToScreen( boxVectors[ 2 ], vec2 ) &&
		render::WorldToScreen( boxVectors[ 3 ], vec3 ) &&
		render::WorldToScreen( boxVectors[ 4 ], vec4 ) &&
		render::WorldToScreen( boxVectors[ 5 ], vec5 ) &&
		render::WorldToScreen( boxVectors[ 6 ], vec6 ) &&
		render::WorldToScreen( boxVectors[ 7 ], vec7 ) ) {
		vec2_t lines[ 12 ][ 2 ];
		lines[ 0 ][ 0 ] = vec0;
		lines[ 0 ][ 1 ] = vec1;
		lines[ 1 ][ 0 ] = vec1;
		lines[ 1 ][ 1 ] = vec2;
		lines[ 2 ][ 0 ] = vec2;
		lines[ 2 ][ 1 ] = vec3;
		lines[ 3 ][ 0 ] = vec3;
		lines[ 3 ][ 1 ] = vec0;

		lines[ 4 ][ 0 ] = vec4;
		lines[ 4 ][ 1 ] = vec5;
		lines[ 5 ][ 0 ] = vec5;
		lines[ 5 ][ 1 ] = vec6;
		lines[ 6 ][ 0 ] = vec6;
		lines[ 6 ][ 1 ] = vec7;
		lines[ 7 ][ 0 ] = vec7;
		lines[ 7 ][ 1 ] = vec4;

		lines[ 8 ][ 0 ] = vec0;
		lines[ 8 ][ 1 ] = vec4;

		lines[ 9 ][ 0 ] = vec1;
		lines[ 9 ][ 1 ] = vec5;

		lines[ 10 ][ 0 ] = vec2;
		lines[ 10 ][ 1 ] = vec6;

		lines[ 11 ][ 0 ] = vec3;
		lines[ 11 ][ 1 ] = vec7;

		for( int i = 0; i < 12; i++ )
			render::line( lines[ i ][ 0 ].x, lines[ i ][ 0 ].y, lines[ i ][ 1 ].x, lines[ i ][ 1 ].y, outline );
	}
}

uintptr_t get_player_resource( ) {
	for( int i{ 1 }; i < g_csgo.m_entlist->GetHighestEntityIndex( ); ++i ) {
		auto ent = g_csgo.m_entlist->GetClientEntity< Player* >( i );
		if( !ent ) continue;
		auto class_ = ent->GetClientClass( );
		if( class_ ) {
			if( class_->m_ClassID == CCSPlayerResource ) {
				return uintptr_t( ent );
			}
		}
	}

	return 0;
}

int get_ping( Player* ce ) {
	auto resource = get_player_resource( );
	static auto offset = g_entoffsets.m_iPing;
	int ping = *( int* )( resource + offset + ce->index( ) * 4 );

	return ping;
}

void Visuals::DrawPlayer( Player* player ) {
	constexpr float MAX_DORMANT_TIME = 60.f;
	constexpr float DORMANT_FADE_TIME = MAX_DORMANT_TIME / 2.f;

	Rect		  box;
	player_info_t info;
	Color		  color;

	// get player index.
	int index = player->index( );

	// get reference to array variable.
	float& opacity = m_opacities[ index - 1 ];
	bool& draw = m_draw[ index - 1 ];

	// opacity should reach 1 in 300 milliseconds.
	constexpr int frequency = 1.f / 0.3f;

	// the increment / decrement per frame.
	float step = frequency * g_csgo.m_globals->m_frametime;

	// is player enemy.
	bool enemy = player->enemy( g_cl.m_local );
	bool dormant = player->dormant( );

	if( g_hooks.b[ XOR( "radar" ) ] && enemy && !dormant )
		player->m_bSpotted( ) = true;

	// we can draw this player again.
	if( !dormant )
		draw = true;

	if( !draw )
		return;

	// if non-dormant	-> increment
	// if dormant		-> decrement
	dormant ? opacity -= step : opacity += step;

	// is dormant esp enabled for this player.
	bool dormant_esp = enemy && g_hooks.b[ XOR( "dormant" ) ];

	// clamp the opacity.
	math::clamp( opacity, 0.f, 1.f );
	if( !opacity && !dormant_esp )
		return;

	// stay for x seconds max.
	float dt = g_csgo.m_globals->m_curtime - player->m_flSimulationTime( );
	if( dormant && dt > MAX_DORMANT_TIME )
		return;

	// calculate alpha channels.
	int alpha = ( int )( 255.f * opacity );
	int low_alpha = ( int )( 179.f * opacity );

	// get color based on enemy or not.
	color = enemy ? Color( g_hooks.c[ XOR( "boundingBox_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "boundingBox_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "boundingBox_color" ) ][ 2 ] * 255.f ) : Color( g_hooks.c[ XOR( "boundingBox_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "boundingBox_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "boundingBox_color" ) ][ 2 ] * 255.f );

	if( dormant && dormant_esp ) {
		alpha = 112;
		low_alpha = 80;

		// fade.
		if( dt > DORMANT_FADE_TIME ) {
			// for how long have we been fading?
			float faded = ( dt - DORMANT_FADE_TIME );
			float scale = 1.f - ( faded / DORMANT_FADE_TIME );

			alpha *= scale;
			low_alpha *= scale;
		}

		// override color.
		color = { 112, 112, 112 };
	}

	// override alpha.
	color.a( ) = alpha;

	// get player info.
	if( !g_csgo.m_engine->GetPlayerInfo( index, &info ) )
		return;

	// run offscreen ESP.
	OffScreen( player, alpha );

	// attempt to get player box.
	if( !GetPlayerBoxRect( player, box ) ) {
		// OffScreen( player );
		return;
	}

	if( g_hooks.b[ XOR( "vis_aimbot" ) ] )
		DebugAimbotPoints( player );

	// DebugAimbotPoints( player );

	//{
	//	// iterate records.
	//	AimPlayer* data = &g_aimbot.m_players[ player->index( ) - 1 ];
	//	if( data || data->m_records.size( ) ) {
	//		LagRecord* first = g_resolver.FindFirstRecord( data );
	//		if( first ) {
	//			vec2_t screen;
	//			if( render::WorldToScreen( first->m_origin, screen ) )
	//				render::rect_filled( screen.x, screen.y, 2, 2, { 0, 255, 255, 255 } );
	//		}
	//	}
	//}

	if( !enemy && !g_hooks.b[ XOR( "teammates" ) ] )
		return;

	// draw predict box.
	if( enemy && g_cl.m_processing && g_hooks.b[ XOR( "lagcomp_degug" ) ] ) {
		AimPlayer* data = &g_aimbot.m_players[ player->index( ) - 1 ];
		// make sure everything is valid.
		if( data && data->m_records.size( ) ) {
			// grab lag record.
			LagRecord* record = data->m_records.front( ).get( );

			if( record && record->m_broke_lc ) {
				vec3_t BoxPos = record->m_pred_origin;
				vec3_t BoxRealPos = player->m_vecOrigin( );

				Draw3DBox( player, 30, 35, g_hooks.imcolor_to_ccolor( g_hooks.c[ XOR( "lagcomp_degug_c" ) ] ), 0 );

				vec2_t angle, da;

				if( render::WorldToScreen( player->GetAbsOrigin( ), angle ) && render::WorldToScreen( record->m_pred_origin, da ) ) {

					auto color = g_hooks.imcolor_to_ccolor( g_hooks.c[ XOR( "lagcomp_degug_c" ) ] );

					render::line( da.x, da.y, angle.x, angle.y, colors::light_blue );
				}
			}
		}
	}

	//if ( g_hooks.skeleton )
	//	DrawHistorySkeleton( player, low_alpha );

	bool bone_esp = ( g_hooks.b[ XOR( "skeleton" ) ] );
	if( bone_esp )
		DrawSkeleton( player, low_alpha );

	// is box esp enabled for this player.
	bool box_esp = ( g_hooks.b[ XOR( "boundingBox" ) ] );

	// render box if specified.
	if( box_esp )
		render::rect_outlined( box.x, box.y, box.w, box.h, color, { 10, 10, 10, low_alpha } );

	// is name esp enabled for this player.
	bool name_esp = ( g_hooks.b[ XOR( "name" ) ] );

	// draw name.
	if( name_esp ) {
		// fix retards with their namechange meme 
		// the point of this is overflowing unicode compares with hardcoded buffers, good hvh strat
		std::string name{ std::string( info.m_name ).substr( 0, 24 ) };

		Color clr = Color( g_hooks.c[ XOR( "name_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "name_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "name_color" ) ][ 2 ] * 255.f );
		if( dormant ) clr = { 112, 112, 112 };
		// override alpha.
		clr.a( ) = low_alpha;

		render::esp.string( box.x + box.w / 2, box.y - render::esp.m_size.m_height, clr, name, render::ALIGN_CENTER );
	}

	// is health esp enabled for this player.
	bool health_esp = ( g_hooks.b[ XOR( "healthBar" ) ] );

	if( health_esp ) {
		int y = box.y;
		int h = box.h;

		// retarded servers that go above 100 hp..
		int hp = std::min( 100, player->m_iHealth( ) );

		// calculate hp bar color.
		int r = std::min( ( 510 * ( 100 - hp ) ) / 100, 255 );
		int g = std::min( ( 510 * hp ) / 100, 255 );

		// get hp bar height.
		int fill = ( int )std::round( hp * h / 100.f );

		// render background.
		render::rect_filled( box.x - 6, y - 1, 4, h + 2, { 10, 10, 10, low_alpha } );

		// render actual bar.
		dormant ? render::rect( box.x - 5, y + h - fill, 2, fill, { 112, 112, 112, alpha } ) : render::rect( box.x - 5, y + h - fill, 2, fill, { r, g, 90, alpha } );

		// if hp is below max, draw a string.
		if( hp < 90 )
			dormant ? render::esp_small.string( box.x - 4, y + ( h - fill ) - 5, { 112, 112, 112, low_alpha }, std::to_string( hp ), render::ALIGN_CENTER ) :
			render::esp_small.string( box.x - 4, y + ( h - fill ) - 5, { 220, 220, 220, low_alpha }, std::to_string( hp ), render::ALIGN_CENTER );
	}

	// draw flags.
	{
		std::vector< std::pair< std::string, Color > > flags;

		// NOTE FROM NITRO TO DEX -> stop removing my iterator loops, i do it so i dont have to check the size of the vector
		// with range loops u do that to do that.
		//for( auto it = items.begin( ); it != items.end( ); ++it ) {

		// money.
		if( g_hooks.b[ XOR( "money" ) ] )
			flags.push_back( { tfm::format( XOR( "$%i" ), player->m_iAccount( ) ), { 150, 200, 60, low_alpha } } );

		// armor.
		if( g_hooks.b[ XOR( "flags" ) ] ) {
			// helmet and kevlar.
			if( player->m_bHasHelmet( ) && player->m_ArmorValue( ) > 0 )
				flags.push_back( { XOR( "HK" ), { 220, 220, 220, low_alpha } } );

			// only helmet.
			else if( player->m_bHasHelmet( ) )
				flags.push_back( { XOR( "H" ), { 220, 220, 220, low_alpha } } );

			// only kevlar.
			else if( player->m_ArmorValue( ) > 0 )
				flags.push_back( { XOR( "K" ), { 220, 220, 220, low_alpha } } );
		}

		// scoped.
		if( g_hooks.b[ XOR( "flags" ) ] && player->m_bIsScoped( ) )
			flags.push_back( { XOR( "ZOOM" ), { 60, 180, 225, low_alpha } } );

		// flashed.
		if( g_hooks.b[ XOR( "flags" ) ] && player->m_flFlashBangTime( ) > 0.f )
			flags.push_back( { XOR( "BLIND" ), { 60, 180, 225, low_alpha } } );

		// reload.
		if( g_hooks.b[ XOR( "flags" ) ] ) {
			// get ptr to layer 1.
			C_AnimationLayer* layer1 = &player->m_AnimOverlay( )[ 1 ];

			// check if reload animation is going on.
			if( layer1->m_weight != 0.f && player->GetSequenceActivity( layer1->m_sequence ) == 967 /* ACT_CSGO_RELOAD */ )
				flags.push_back( { XOR( "R" ), { 60, 180, 225, low_alpha } } );
		}

		// bomb.
		if( g_hooks.b[ XOR( "flags" ) ] && player->HasC4( ) )
			flags.push_back( { XOR( "B" ), { 255, 0, 0, low_alpha } } );

		// ping.
		if( g_hooks.b[ XOR( "ping" ) ] && get_ping( player ) > 200 && g_cl.m_processing ) {
			flags.push_back( { tfm::format( XOR( "PING" ) ), get_ping( player ) < 500 ? colors::orange : colors::red } );
		}

		// iterate flags.
		for( size_t i{ }; i < flags.size( ); ++i ) {
			// get flag job (pair).
			const auto& f = flags[ i ];

			int offset = i * ( render::esp_small.m_size.m_height - 1 );

			// draw flag.
			dormant ? render::esp_small.string( box.x + box.w + 2, box.y + offset, Color{ 112, 112, 112, low_alpha }, f.first ) :
				render::esp_small.string( box.x + box.w + 2, box.y + offset, f.second, f.first );
		}
	}

	// draw bottom bars.
	{
		int offset1{ 0 };
		int offset3{ 0 };
		int offset{ 0 };
		int distance1337{ 0 };

		// draw lby update bar.
		if( enemy && g_hooks.b[ XOR( "lby" ) ] && g_cl.m_processing && !dormant ) {
			AimPlayer* data = &g_aimbot.m_players[ player->index( ) - 1 ];
			// make sure everything is valid.
			if( data && data->m_moved && data->m_records.size( ) ) {
				// grab lag record.
				LagRecord* current = data->m_records.front( ).get( );
				if( current ) {
					if( !( current->m_velocity.length_2d( ) > 0.1 && !current->m_fake_walk ) && data->m_body_index <= 3 ) {
						// calculate box width
						float cycle = std::clamp<float>( data->m_body_update - current->m_anim_time, 0.f, 1.0f );
						float width = ( box.w * cycle ) / 1.1f;

						if( width > 0.f ) {
							// draw.
							render::rect_filled( box.x - 1, box.y + box.h + 2, box.w + 2, 4, { 10, 10, 10, low_alpha } );

							Color clr = Color( g_hooks.c[ XOR( "lby_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "lby_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "lby_color" ) ][ 2 ] * 255.f );
							if( dormant ) clr = { 112, 112, 112 };
							clr.a( ) = alpha;
							render::rect( box.x, box.y + box.h + 3, width, 2, clr );

							// move down the offset to make room for the next bar.
							offset += 5;
							offset3 += 1;
						}
					}
				}
			}
		}

		// draw weapon.
		if( ( g_hooks.b[ XOR( "weaponText" ) ] || g_hooks.b[ XOR( "weaponIcon" ) ] || g_hooks.b[ XOR( "ammo" ) ] ) ) {
			Weapon* weapon = player->GetActiveWeapon( );
			if( weapon ) {
				WeaponInfo* data = weapon->GetWpnData( );
				if( data ) {
					int bar;
					float scale;

					// the maxclip1 in the weaponinfo
					int max = data->m_max_clip1;
					int current = weapon->m_iClip1( );

					C_AnimationLayer* layer1 = &player->m_AnimOverlay( )[ 1 ];

					// set reload state.
					bool reload = ( layer1->m_weight != 0.f ) && ( player->GetSequenceActivity( layer1->m_sequence ) == 967 );

					// ammo bar.
					if( max != -1 && g_hooks.b[ XOR( "ammo" ) ] ) {
						// check for reload.
						if( reload )
							scale = layer1->m_cycle;

						// not reloading.
						// make the division of 2 ints produce a float instead of another int.
						else
							scale = ( float )current / max;

						// relative to bar.
						bar = ( int )std::round( box.w * scale );

						// draw.
						render::rect_filled( box.x - 1, box.y + box.h + 2 + offset, box.w + 2, 4, { 10, 10, 10, low_alpha } );

						Color clr = Color( g_hooks.c[ XOR( "ammo_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "ammo_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "ammo_color" ) ][ 2 ] * 255.f );
						if( dormant ) clr = { 112, 112, 112 };
						clr.a( ) = alpha;
						render::rect( box.x, box.y + box.h + 3 + offset, bar, 2, clr );

						// less then a 5th of the bullets left.
						if( current <= ( int )std::round( max / 5 ) && !reload )
							dormant ? render::esp_small.string( box.x + bar, box.y + box.h + offset, { 112, 112, 112, low_alpha }, std::to_string( current ), render::ALIGN_CENTER ) :
							render::esp_small.string( box.x + bar, box.y + box.h + offset, { 220, 220, 220, low_alpha }, std::to_string( current ), render::ALIGN_CENTER );

						offset += 6;
					}
					else
						offset += 1;

					if( g_hooks.b[ XOR( "distance" ) ] ) {
						std::string distance;
						int dist = ( ( ( player->m_vecOrigin( ) - g_cl.m_local->m_vecOrigin( ) ).length_2d_sqr( ) ) * 0.00001 ) * 3;

						if( dist < 0 )
							distance1337 = 0;

						if( DistInFt( g_cl.m_local->m_vecOrigin( ), player->m_vecOrigin( ) ) >= 5 ) {
							distance1337 = 10 + offset3;

							dormant ? render::esp_small.string( box.x + box.w / 2, box.y + box.h + offset + offset3, { 112, 112, 112, low_alpha }, tfm::format( XOR( "%i FT" ), DistInFt( g_cl.m_local->m_vecOrigin( ), player->m_vecOrigin( ) ) ), render::ALIGN_CENTER ) :
								render::esp_small.string( box.x + box.w / 2, box.y + box.h + offset + offset3, { 220, 220, 220, low_alpha }, tfm::format( XOR( "%i FT" ), DistInFt( g_cl.m_local->m_vecOrigin( ), player->m_vecOrigin( ) ) ), render::ALIGN_CENTER );
						}
					}

					// text.
					if( g_hooks.b[ XOR( "weaponText" ) ] ) {
						offset1 -= 8;
						// construct std::string instance of localized weapon name.
						std::string name{ weapon->GetLocalizedName( ) };
						Color clr = Color( g_hooks.c[ XOR( "weaponText_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "weaponText_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "weaponText_color" ) ][ 2 ] * 255.f );
						clr.a( ) = low_alpha;

						// smallfonts needs upper case.
						std::transform( name.begin( ), name.end( ), name.begin( ), ::toupper );

						dormant ? render::esp_small.string( box.x + box.w / 2, box.y + box.h + offset + distance1337, { 112, 112, 112, low_alpha }, name, render::ALIGN_CENTER ) :
							render::esp_small.string( box.x + box.w / 2, box.y + box.h + offset + distance1337, clr, name, render::ALIGN_CENTER );

					}

					// icons.
					if( g_hooks.b[ XOR( "weaponIcon" ) ] ) {
						offset -= 4;
						// icons are super fat..
						// move them back up.
						Color clr = Color( g_hooks.c[ XOR( "weaponIcon_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "weaponIcon_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "weaponIcon_color" ) ][ 2 ] * 255.f );
						clr.a( ) = low_alpha;

						std::string icon = tfm::format( XOR( "%c" ), m_weapon_icons[ weapon->m_iItemDefinitionIndex( ) ] );
						dormant ? render::weapon.string( box.x + box.w / 2, box.y + box.h + offset - offset1 + distance1337 + 5, { 112, 112, 112, low_alpha }, icon, render::ALIGN_CENTER ) :
							render::weapon.string( box.x + box.w / 2, box.y + box.h + offset - offset1 + distance1337 + 5, clr, icon, render::ALIGN_CENTER );
					}
				}
			}
		}
	}
}

void Visuals::DrawPlantedC4( ) {
	bool    mode_2d, mode_3d, is_visible;
	float    explode_time_diff, dist, range_damage;
	vec3_t   dst, to_target;
	std::string time_str, damage_str;
	Color    damage_color;
	vec2_t   screen_pos;

	static auto scale_damage = [ ]( float damage, int armor_value ) {
		float new_damage, armor;

		if( armor_value > 0 ) {
			new_damage = damage * 0.5f;
			armor = ( damage - new_damage ) * 0.5f;

			if( armor > ( float )armor_value ) {
				armor = ( float )armor_value * 2.f;
				new_damage = damage - armor;
			}

			damage = new_damage;
		}

		return std::max( 0, ( int )std::floor( damage ) );
	};

	// store menu vars for later.
	mode_2d = g_hooks.i[ XOR( "planted_c4" ) ] == 1;

	// bomb not currently active, do nothing.
	if( !m_c4_planted )
		return;

	// calculate bomb damage.
	// references:
	//   https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/se2007/game/shared/cstrike/weapon_c4.cpp#L271
	//   https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/se2007/game/shared/cstrike/weapon_c4.cpp#L437
	//   https://github.com/ValveSoftware/source-sdk-2013/blob/master/sp/src/game/shared/sdk/sdk_gamerules.cpp#L173
	{
		// get our distance to the bomb.
		// todo - dex; is dst right? might need to reverse CBasePlayer::BodyTarget...
		dst = g_cl.m_local->WorldSpaceCenter( );
		to_target = m_planted_c4_explosion_origin - dst;
		dist = to_target.length( );

		// calculate the bomb damage based on our distance to the C4's explosion.
		range_damage = m_planted_c4_damage * std::exp( ( dist * dist ) / ( ( m_planted_c4_radius_scaled * -2.f ) * m_planted_c4_radius_scaled ) );

		// now finally, scale the damage based on our armor (  if we have any ).
		m_final_damage = scale_damage( range_damage, g_cl.m_local->m_ArmorValue( ) );
	}

	if( !mode_2d )
		return;

	if( mode_2d ) {
		float complete = ( g_csgo.m_globals->m_curtime - m_plant_start ) / m_plant_duration;
		int alpha = ( 1.f - complete ) * g_cl.m_height;

		if( m_planted_c4_planting && complete > 0 ) {
			std::string b_time_str = tfm::format( XOR( "PLANT - %.1fs" ), 3.f * complete );
			Color color = { 0xff15c27b };
			color.a( ) = 125;

			render::indicator.string( 6, 11, { 0,0, 0, 125 }, b_time_str );
			render::indicator.string( 5, 10, color, b_time_str );

			render::rect_filled( 0, 0, 21, g_cl.m_height, { 0, 0, 0, 80 } );
			render::rect_filled( 0, g_cl.m_height - alpha / 2, 20, alpha, { color } );
		}
	}


	// m_flC4Blow is set to gpGlobals->curtime + m_flTimerLength inside CPlantedC4.
	explode_time_diff = m_planted_c4_explode_time - g_csgo.m_globals->m_curtime;

	// get formatted strings for bomb.
	//time_str = tfm::format( XOR( "%.2f" ), explode_time_diff );
	time_str = tfm::format( XOR( "%s - %.1fs" ), m_last_bombsite.substr( 0, 1 ), explode_time_diff );
	damage_str = tfm::format( XOR( "- %iHP" ), m_final_damage );
	if( g_cl.m_local->m_iHealth( ) <= m_final_damage ) damage_str = tfm::format( XOR( "FATAL" ) );

	Color colortimer = { 0xff15c27b };
	if( explode_time_diff < 10 ) colortimer = { 200, 200, 110, 125 };
	if( explode_time_diff < 5 ) colortimer = { 0xff0000ff };
	colortimer.a( ) = 125;

	Color colortimer1337 = { 0xff15c27b };
	if( explode_time_diff < 10 ) colortimer1337 = { 200, 200, 110, 125 };
	if( explode_time_diff < 5 ) colortimer1337 = { 0xff0000ff };
	colortimer1337.a( ) = 125;

	static Color defusetimer;
	if( explode_time_diff < 5 && !m_planted_c4_indefuse ) defusetimer = { 0xff0000ff };
	else if( explode_time_diff < 10 && !m_planted_c4_indefuse ) defusetimer = { 0xff0000ff };
	else if( m_planted_c4_defuse == 1 && explode_time_diff >= 5 ) defusetimer = { 0xff15c27b };
	else if( m_planted_c4_defuse == 2 && explode_time_diff >= 10 ) defusetimer = { 0xff15c27b };
	defusetimer.a( ) = 125;

	// get damage color.
	damage_color = ( m_final_damage < g_cl.m_local->m_iHealth( ) ) ? colors::white : colors::red;

	// finally do all of our rendering.
	is_visible = render::WorldToScreen( m_planted_c4_explosion_origin, screen_pos );

	float complete = ( g_csgo.m_globals->m_curtime - m_defuse_start ) / m_defuse_duration;
	int alpha = ( 1.f - complete ) * g_cl.m_height;

	// 'on screen (  2D )'.
	if( mode_2d ) {
		if( m_c4_planted && m_planted_c4_indefuse && explode_time_diff > 0 ) {

			if( m_planted_c4_defuse == 1 ) {
				render::rect_filled( 0, 0, 21, g_cl.m_height, { 0, 0, 0, 80 } );
				render::rect_filled( 0, g_cl.m_height - alpha / 2, 20, alpha, { defusetimer } );
			}
			else {
				render::rect_filled( 0, 0, 21, g_cl.m_height, { 0, 0, 0, 80 } );
				render::rect_filled( 0, ( g_cl.m_height - alpha ), 20, alpha, { defusetimer } );
			}
		}

		if( explode_time_diff > 0.f ) {
			render::indicator.string( 6, 11, { 0,0, 0, 125 }, time_str );
			render::indicator.string( 5, 10, colortimer, time_str );
		}

		if( g_cl.m_local->alive( ) && m_final_damage > 0 ) {
			render::indicator.string( 5, 31, { 0,0, 0, 125 }, damage_str );
			render::indicator.string( 6, 30, { 255, 255, 152, 255 }, damage_str );
		}
	}

	// 'on bomb (  3D )'.
	if( mode_2d && is_visible ) {
		if( explode_time_diff > 0.f )
			render::esp_small.string( screen_pos.x, screen_pos.y, colors::white, time_str, render::ALIGN_CENTER );

		// only render damage string if we're alive.
		if( g_cl.m_local->alive( ) )
			render::esp_small.string( screen_pos.x, ( int )screen_pos.y + render::esp_small.m_size.m_height, damage_color, damage_str, render::ALIGN_CENTER );
	}
}

bool Visuals::GetPlayerBoxRect( Player* player, Rect& box ) {
	vec3_t origin, mins, maxs;
	vec2_t bottom, top;

	// get interpolated origin.
	origin = player->GetAbsOrigin( );

	// get hitbox bounds.
	if( !player->ComputeHitboxSurroundingBox( &mins, &maxs ) )
		return false;

	// correct x and y coordinates.
	mins = { origin.x, origin.y, mins.z };
	maxs = { origin.x, origin.y, maxs.z + 8.f };

	if( !render::WorldToScreen( mins, bottom ) || !render::WorldToScreen( maxs, top ) )
		return false;

	box.h = bottom.y - top.y;
	box.w = box.h / 2.f;
	box.x = bottom.x - ( box.w / 2.f );
	box.y = bottom.y - box.h;

	return true;
}

bool get_bone( matrix3x4_t* mx, vec3_t& out, int bone = 0 ) {
	if( bone < 0 || bone >= 128 )
		return false;

	matrix3x4_t* bone_matrix = &mx[ bone ];

	if( !bone_matrix )
		return false;

	out = { bone_matrix->m_flMatVal[ 0 ][ 3 ], bone_matrix->m_flMatVal[ 1 ][ 3 ], bone_matrix->m_flMatVal[ 2 ][ 3 ] };

	return true;
}

void Visuals::DrawHistorySkeleton( Player* player, int opacity ) {
	const model_t* model;
	studiohdr_t* hdr;
	mstudiobone_t* bone;
	AimPlayer* data;
	LagRecord* record;
	int           parent;
	vec3_t        bone_pos, parent_pos;
	vec2_t        bone_pos_screen, parent_pos_screen;

	// get player's model.
	model = player->GetModel( );
	if( !model )
		return;

	// get studio model.
	hdr = g_csgo.m_model_info->GetStudioModel( model );
	if( !hdr )
		return;

	data = &g_aimbot.m_players[ player->index( ) - 1 ];
	if( !data )
		return;

	record = g_resolver.FindLastRecord( data );
	if( !record || !record->m_setup )
		return;

	for( int i{}; i < hdr->m_num_bones; ++i ) {
		// get bone.
		bone = hdr->GetBone( i );
		if( !bone || !( bone->m_flags & BONE_USED_BY_HITBOX ) )
			continue;

		// get parent bone.
		parent = bone->m_parent;
		if( parent == -1 )
			continue;

		// resolve main bone and parent bone positions.
		get_bone( record->m_render_bones, bone_pos, i );
		get_bone( record->m_render_bones, parent_pos, parent );

		Color clr = player->enemy( g_cl.m_local ) ? Color( g_hooks.c[ XOR( "skeleton_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "skeleton_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "skeleton_color" ) ][ 2 ] * 255.f ) : Color( g_hooks.c[ XOR( "skeleton_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "skeleton_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "skeleton_color" ) ][ 2 ] * 255.f );
		clr.a( ) = opacity;

		// world to screen both the bone parent bone then draw.
		if( render::WorldToScreen( bone_pos, bone_pos_screen ) && render::WorldToScreen( parent_pos, parent_pos_screen ) )
			render::line( bone_pos_screen.x, bone_pos_screen.y, parent_pos_screen.x, parent_pos_screen.y, clr );
	}
}

void Visuals::DrawSkeleton( Player* player, int opacity ) {
	const model_t* model;
	studiohdr_t* hdr;
	mstudiobone_t* bone;
	int           parent;
	//BoneArray     matrix[ 128 ];
	vec3_t        bone_pos, parent_pos;
	vec2_t        bone_pos_screen, parent_pos_screen;

	// get player's model.
	model = player->GetModel( );
	if( !model || player->m_BoneCache( ).m_CachedBoneCount <= 0 )
		return;

	// get studio model.
	hdr = g_csgo.m_model_info->GetStudioModel( model );
	if( !hdr )
		return;

	for( int i{}; i < hdr->m_num_bones; ++i ) {
		// get bone.
		bone = hdr->GetBone( i );
		if( !bone || !( bone->m_flags & BONE_USED_BY_HITBOX ) )
			continue;

		// get parent bone.
		parent = bone->m_parent;
		if( parent == -1 )
			continue;

		// resolve main bone and parent bone positions.
		get_bone( player->m_BoneCache( ).m_pCachedBones, bone_pos, i );
		get_bone( player->m_BoneCache( ).m_pCachedBones, parent_pos, parent );

		Color clr = player->enemy( g_cl.m_local ) ? Color( g_hooks.c[ XOR( "skeleton_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "skeleton_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "skeleton_color" ) ][ 2 ] * 255.f ) : Color( g_hooks.c[ XOR( "skeleton_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "skeleton_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "skeleton_color" ) ][ 2 ] * 255.f );
		if( player->dormant( ) ) clr = { 112, 112, 112 };
		clr.a( ) = opacity;

		// world to screen both the bone parent bone then draw.
		if( render::WorldToScreen( bone_pos, bone_pos_screen ) && render::WorldToScreen( parent_pos, parent_pos_screen ) )
			render::line( bone_pos_screen.x, bone_pos_screen.y, parent_pos_screen.x, parent_pos_screen.y, clr );
	}
}

void Visuals::RenderGlow( ) {
	Color   color;
	Player* player;

	if( !g_cl.m_local )
		return;

	if( !g_csgo.m_glow->m_object_definitions.Count( ) )
		return;

	float blend = ( g_hooks.c[ XOR( "glow_color" ) ][ 3 ] );

	for( int i{ }; i < g_csgo.m_glow->m_object_definitions.Count( ); ++i ) {
		GlowObjectDefinition_t* obj = &g_csgo.m_glow->m_object_definitions[ i ];

		// skip non-players.
		if( !obj->m_entity || !obj->m_entity->IsPlayer( ) )
			continue;

		// get player ptr.
		player = obj->m_entity->as< Player* >( );

		if( player->m_bIsLocalPlayer( ) )
			continue;

		// get reference to array variable.
		float& opacity = m_opacities[ player->index( ) - 1 ];

		bool enemy = player->enemy( g_cl.m_local );

		if( !enemy && !g_hooks.b[ XOR( "teammates" ) ] )
			continue;

		if( !g_hooks.b[ XOR( "glow" ) ] )
			continue;

		if( c_playerlist::get( )->should_disable_visuals( player ) )
			return;

		// enemy color
		if( enemy )
			color = Color( g_hooks.c[ XOR( "glow_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "glow_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "glow_color" ) ][ 2 ] * 255.f );

		//// friendly color
		//else
		//	color = g_menu.main.players.glow_friendly.get( );

		obj->m_render_occluded = true;
		obj->m_render_unoccluded = false;
		obj->m_render_full_bloom = false;
		obj->m_color = { ( float )color.r( ) / 255.f, ( float )color.g( ) / 255.f, ( float )color.b( ) / 255.f };
		obj->m_alpha = opacity * blend;
	}
}

void Visuals::DrawHitboxMatrix( LagRecord* record, Color col, float time ) {
	const model_t* model;
	studiohdr_t* hdr;
	mstudiohitboxset_t* set;
	mstudiobbox_t* bbox;
	vec3_t             mins, maxs, origin;
	ang_t			   angle;

	model = record->m_player->GetModel( );
	if( !model )
		return;

	hdr = g_csgo.m_model_info->GetStudioModel( model );
	if( !hdr )
		return;

	set = hdr->GetHitboxSet( record->m_player->m_nHitboxSet( ) );
	if( !set )
		return;

	for( int i{ }; i < set->m_hitboxes; ++i ) {
		bbox = set->GetHitbox( i );
		if( !bbox )
			continue;

		// bbox.
		if( bbox->m_radius <= 0.f ) {
			// https://developer.valvesoftware.com/wiki/Rotation_Tutorial

			// convert rotation angle to a matrix.
			matrix3x4_t rot_matrix;
			g_csgo.AngleMatrix( bbox->m_angle, rot_matrix );

			// apply the rotation to the entity input space (local).
			matrix3x4_t matrix;
			math::ConcatTransforms( record->m_bones[ bbox->m_bone ], rot_matrix, matrix );

			// extract the compound rotation as an angle.
			ang_t bbox_angle;
			math::MatrixAngles( matrix, bbox_angle );

			// extract hitbox origin.
			vec3_t origin = matrix.GetOrigin( );

			// draw box.
			g_csgo.m_debug_overlay->AddBoxOverlay( origin, bbox->m_mins, bbox->m_maxs, bbox_angle, col.r( ), col.g( ), col.b( ), 0, time );
		}

		// capsule.
		else {
			// NOTE; the angle for capsules is always 0.f, 0.f, 0.f.

			// create a rotation matrix.
			matrix3x4_t matrix;
			g_csgo.AngleMatrix( bbox->m_angle, matrix );

			// apply the rotation matrix to the entity output space (world).
			math::ConcatTransforms( record->m_bones[ bbox->m_bone ], matrix, matrix );

			// get world positions from new matrix.
			math::VectorTransform( bbox->m_mins, matrix, mins );
			math::VectorTransform( bbox->m_maxs, matrix, maxs );

			g_csgo.m_debug_overlay->AddCapsuleOverlay( mins, maxs, bbox->m_radius, col.r( ), col.g( ), col.b( ), col.a( ), time, 0, 0 );
		}
	}
}

void Visuals::DrawBeams( ) {
	size_t     impact_count;
	float      curtime, dist;
	bool       is_final_impact;
	vec3_t     va_fwd, start, dir, end;
	BeamInfo_t beam_info;
	Beam_t* beam;

	if( !g_cl.m_local )
		return;

	if( !g_hooks.b[ XOR( "bulletTracers" ) ] )
		return;

	auto vis_impacts = &g_shots.m_vis_impacts;

	// the local player is dead, clear impacts.
	if( !g_cl.m_processing ) {
		if( !vis_impacts->empty( ) )
			vis_impacts->clear( );
	}

	else {
		impact_count = vis_impacts->size( );
		if( !impact_count )
			return;

		curtime = game::TICKS_TO_TIME( g_cl.m_local->m_nTickBase( ) );

		for( size_t i{ impact_count }; i-- > 0; ) {
			auto impact = &vis_impacts->operator[ ]( i );
			if( !impact )
				continue;

			// impact is too old, erase it.
			if( std::abs( curtime - game::TICKS_TO_TIME( impact->m_tickbase ) ) > g_menu.main.visuals.impact_beams_time.get( ) ) {
				vis_impacts->erase( vis_impacts->begin( ) + i );

				continue;
			}

			// already rendering this impact, skip over it.
			if( impact->m_ignore )
				continue;

			// is this the final impact?
			// last impact in the vector, it's the final impact.
			if( i == ( impact_count - 1 ) )
				is_final_impact = true;

			// the current impact's tickbase is different than the next, it's the final impact.
			else if( ( i + 1 ) < impact_count && impact->m_tickbase != vis_impacts->operator[ ]( i + 1 ).m_tickbase )
				is_final_impact = true;

			else
				is_final_impact = false;

			// is this the final impact?
			// is_final_impact = ( ( i == ( impact_count - 1 ) ) || ( impact->m_tickbase != vis_impacts->at( i + 1 ).m_tickbase ) );

			if( is_final_impact ) {
				// calculate start and end position for beam.
				start = impact->m_shoot_pos;

				dir = ( impact->m_impact_pos - start ).normalized( );
				dist = ( impact->m_impact_pos - start ).length( );

				end = start + ( dir * dist );

				// setup beam info.
				// note - dex; possible beam models: sprites/physbeam.vmt | sprites/white.vmt
				beam_info.m_vecStart = start;
				beam_info.m_vecEnd = end;
				beam_info.m_nModelIndex = g_csgo.m_model_info->GetModelIndex( XOR( "sprites/purplelaser1.vmt" ) );
				beam_info.m_pszModelName = XOR( "sprites/purplelaser1.vmt" );
				beam_info.m_flHaloScale = 0.f;
				beam_info.m_flLife = g_menu.main.visuals.impact_beams_time.get( );
				beam_info.m_flWidth = 2.f;
				beam_info.m_flEndWidth = 2.f;
				beam_info.m_flFadeLength = 0.f;
				beam_info.m_flAmplitude = 0.f;   // beam 'jitter'.
				beam_info.m_flBrightness = 255.f;
				beam_info.m_flSpeed = 0.5f;  // seems to control how fast the 'scrolling' of beam is... once fully spawned.
				beam_info.m_nStartFrame = 0;
				beam_info.m_flFrameRate = 0.f;
				beam_info.m_nSegments = 2;     // controls how much of the beam is 'split up', usually makes m_flAmplitude and m_flSpeed much more noticeable.
				beam_info.m_bRenderable = true;  // must be true or you won't see the beam.
				beam_info.m_nFlags = 0;

				if( !impact->m_hit_player ) {
					beam_info.m_flRed = g_menu.main.visuals.impact_beams_color.get( ).r( );
					beam_info.m_flGreen = g_menu.main.visuals.impact_beams_color.get( ).g( );
					beam_info.m_flBlue = g_menu.main.visuals.impact_beams_color.get( ).b( );
				}

				else {
					beam_info.m_flRed = g_menu.main.visuals.impact_beams_hurt_color.get( ).r( );
					beam_info.m_flGreen = g_menu.main.visuals.impact_beams_hurt_color.get( ).g( );
					beam_info.m_flBlue = g_menu.main.visuals.impact_beams_hurt_color.get( ).b( );
				}

				// attempt to render the beam.
				beam = game::CreateGenericBeam( beam_info );
				if( beam ) {
					g_csgo.m_beams->DrawBeam( beam );

					// we only want to render a beam for this impact once.
					impact->m_ignore = true;
				}
			}
		}
	}
}

void Visuals::DebugAimbotPoints( Player* player ) {
	std::vector< vec3_t > p2{};

	if( !player->enemy( g_cl.m_local ) )
		return;

	AimPlayer* data = &g_aimbot.m_players.at( player->index( ) - 1 );
	if( !data || data->m_records.empty( ) )
		return;

	LagRecord* front = data->m_records.front( ).get( );
	if( !front || front->dormant( ) )
		return;

	data->SetupHitboxes( front, false );
	if( data->m_hitboxes.empty( ) )
		return;

	for( const auto& it : data->m_hitboxes ) {
		std::vector< vec3_t > p1{};

		if( !data->SetupHitboxPoints( front, front->m_bones, it.m_index, p1 ) )
			continue;

		for( auto& p : p1 )
			p2.push_back( p );
	}

	if( p2.empty( ) )
		return;

	for( auto& p : p2 ) {
		vec2_t screen;

		if( render::WorldToScreen( p, screen ) )
			render::rect_filled( screen.x, screen.y, 2, 2, g_hooks.imcolor_to_ccolor( g_hooks.c[ XOR( "vis_aimbot_color" ) ] ) );
	}
}