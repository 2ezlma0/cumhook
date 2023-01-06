#include "includes.h"

void IEngineTrace::TraceLine( const vec3_t& src, const vec3_t& dst, int mask, IHandleEntity* entity, int collision_group, CGameTrace* trace ) {
	static auto trace_filter_simple = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 83 E4 F0 83 EC 7C 56 52" ) ) + 0x3D;

	std::uintptr_t filter[ 4 ] = { *reinterpret_cast< std::uintptr_t* >( trace_filter_simple ), reinterpret_cast< std::uintptr_t >( entity ), collision_group, 0 };

	TraceRay( Ray( src, dst ), mask, reinterpret_cast< CTraceFilter* >( &filter ), trace );
}

void IEngineTrace::TraceHull( const vec3_t& src, const vec3_t& dst, const vec3_t& mins, const vec3_t& maxs, int mask, IHandleEntity* entity, int collision_group, CGameTrace* trace ) {
	static auto trace_filter_simple = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 83 E4 F0 83 EC 7C 56 52" ) ) + 0x3D;

	std::uintptr_t filter[ 4 ] = { *reinterpret_cast< std::uintptr_t* >( trace_filter_simple ), reinterpret_cast< std::uintptr_t >( entity ), collision_group, 0 };

	TraceRay( Ray( src, dst, mins, maxs ), mask, reinterpret_cast< CTraceFilter* >( &filter ), trace );
}

void c_grenade_prediction::on_create_move( CUserCmd* cmd ) { }

void DrawBeamPaw( vec3_t src, vec3_t end, Color color ) {
	BeamInfo_t beamInfo;
	beamInfo.m_nType = 0; //TE_BEAMPOINTS
	beamInfo.m_vecStart = src;
	beamInfo.m_vecEnd = end;
	beamInfo.m_pszModelName = "sprites/purplelaser1.vmt";//sprites/purplelaser1.vmt
	beamInfo.m_pszHaloName = "sprites/purplelaser1.vmt";//sprites/purplelaser1.vmt
	beamInfo.m_flHaloScale = 0;//0
	beamInfo.m_flWidth = 2;//11
	beamInfo.m_flEndWidth = 2;//11
	beamInfo.m_flFadeLength = 1.0f;
	beamInfo.m_flAmplitude = 2.3;
	beamInfo.m_flBrightness = color.a( );
	beamInfo.m_flSpeed = 0.2f;
	beamInfo.m_nStartFrame = 0.0;
	beamInfo.m_flFrameRate = 0.0;
	beamInfo.m_flRed = color.r( );
	beamInfo.m_flGreen = color.g( );
	beamInfo.m_flBlue = color.b( );
	beamInfo.m_nSegments = 2;//40
	beamInfo.m_bRenderable = true;
	beamInfo.m_flLife = 0.03f;
	beamInfo.m_nFlags = 256 | 512 | 32768; //FBEAM_ONLYNOISEONCE | FBEAM_NOTILE | FBEAM_HALOBEAM

	Beam_t* myBeam = g_csgo.m_beams->CreateBeamPoints( beamInfo );
	if( myBeam )
		g_csgo.m_beams->DrawBeam( myBeam );
}

void rotate_point( vec2_t& point, vec2_t origin, bool clockwise, float angle ) {
	vec2_t delta = point - origin;
	vec2_t rotated;

	if( clockwise ) {
		rotated = vec2_t( delta.x * cosf( angle ) - delta.y * sinf( angle ), delta.x * sinf( angle ) + delta.y * cosf( angle ) );
	}
	else {
		rotated = vec2_t( delta.x * sinf( angle ) - delta.y * cosf( angle ), delta.x * cosf( angle ) + delta.y * sinf( angle ) );
	}

	point = rotated + origin;
}

/*bool c_grenade_prediction::data_t::draw( ) const {
	if( m_path.size( ) <= 1u || g_csgo.m_globals->m_curtime >= m_expire_time )
		return false;

	vec3_t vecPrevious = vec3_t( 0, 0, 0 );
	vec3_t vecPreviousOrigin = vec3_t( 0, 0, 0 );

	auto prev_screen = vec2_t( );
	auto prev_on_screen = render::WorldToScreen( std::get< vec3_t >( m_path.front( ) ), prev_screen );

	for( auto i = 1u; i < m_path.size( ); ++i ) {
		auto cur_screen = vec2_t( );
		const auto cur_on_screen = render::WorldToScreen( std::get< vec3_t >( m_path.at( i ) ), cur_screen );

		if( prev_on_screen
			&& cur_on_screen ) {
			render::line( prev_screen.x, prev_screen.y, cur_screen.x, cur_screen.y, colors::light_blue );

			if( i == m_path.size( ) - 1 )
				RenderCircle3D( std::get< vec3_t >( m_path.at( i ) ), 32, 171, colors::light_blue );
		}

		prev_screen = cur_screen;
		prev_on_screen = cur_on_screen;
	}

	vec3_t vecTimerPosition = std::get < vec3_t >( m_path[ m_path.size( ) - 1 ] );
	vec2_t vecScreenPosition = vec2_t( 0, 0 );

	std::string szIcon = "";
	switch( m_index ) {
		case HEGRENADE: szIcon = XOR( "j" ); break;
		case SMOKE: szIcon = XOR( "k" ); break;
		case FLASHBANG: szIcon = XOR( "i" ); break;
		case DECOY: szIcon = XOR( "m" ); break;
		case FIREBOMB: szIcon = XOR( "n" ); break;
		case MOLOTOV: szIcon = XOR( "l" ); break;
	}

	auto vecTextSize = render::gw.size( szIcon.c_str( ) );
	render::gw.string( vecScreenPosition.x - vecTextSize.m_width / 2 + 1, vecScreenPosition.y - vecTextSize.m_height / 2, colors::white, szIcon, render::ALIGN_RIGHT );

	//if( g_Settings->m_GrenadeTimers ) {
	float_t flPercentage = ( ( m_expire_time - g_csgo.m_globals->m_curtime ) / game::TICKS_TO_TIME( m_tick ) );
	render::rect_filled( vecScreenPosition.x - 21, vecScreenPosition.y + 14, vecScreenPosition.x + 21, vecScreenPosition.y + 20, Color( 0.0f, 0.0f, 0.0f, 100.0f ) );
	render::rect_filled( vecScreenPosition.x - 20, vecScreenPosition.y + 15, vecScreenPosition.x - 19.0f + 39.0f * flPercentage, vecScreenPosition.y + 19, colors::white );
	//}

	return true;
}*/

bool c_grenade_prediction::data_t::draw( ) const {
	if( m_path.size( ) <= 1u || g_csgo.m_globals->m_curtime >= m_expire_time )
		return false;

	if( !g_hooks.b[ XOR( "grenadeWarning_enemy" ) ] )
		return false;

	if( m_owner != g_cl.m_local && ( m_owner->enemy( g_cl.m_local ) && !g_hooks.b[ XOR( "grenadeWarning_enemy" ) ] ) )
		return false;

	if( m_owner != g_cl.m_local && ( !m_owner->enemy( g_cl.m_local ) && !g_hooks.b[ XOR( "grenadeWarning_teammates" ) ] ) )
		return false;

	if( m_owner == g_cl.m_local && !g_hooks.b[ XOR( "grenadeWarning_local" ) ] )
		return false;

	Color color;

	if( m_owner == g_cl.m_local )
		color = g_hooks.imcolor_to_ccolor( g_hooks.c[ XOR( "grenadeWarning_local_color" ) ] );
	else if( m_owner != g_cl.m_local && !m_owner->enemy( g_cl.m_local ) )
		color = g_hooks.imcolor_to_ccolor( g_hooks.c[ XOR( "grenadeWarning_teammates_color" ) ] );
	else if( m_owner != g_cl.m_local && m_owner->enemy( g_cl.m_local ) )
		color = g_hooks.imcolor_to_ccolor( g_hooks.c[ XOR( "grenadeWarning_enemy_color" ) ] );

	float distance = g_cl.m_local->m_vecOrigin( ).dist_to( m_origin ) / 12.f;

	auto prev_screen = vec2_t( );
	auto prev_on_screen = render::WorldToScreen( std::get< vec3_t >( m_path.front( ) ), prev_screen );

	for( auto i = 1u; i < m_path.size( ); ++i ) {
		auto cur_screen = vec2_t( );
		const auto cur_on_screen = render::WorldToScreen( std::get< vec3_t >( m_path.at( i ) ), cur_screen );

		if( prev_on_screen && cur_on_screen ) {
			DrawBeamPaw( std::get< vec3_t >( m_path.at( i - 1 ) ), std::get< vec3_t >( m_path.at( i ) ), color );
		}

		prev_screen = cur_screen;
		prev_on_screen = cur_on_screen;
	}

	if( g_visuals.DistInFt( g_cl.m_local->m_vecOrigin( ), m_origin ) > 50.f )
		return false;

	static CTraceFilterSimple_game	filter{};
	CGameTrace						trace;
	float							percent = ( ( m_expire_time - g_csgo.m_globals->m_curtime ) / game::TICKS_TO_TIME( m_tick ) );
	int								alpha_damage = 0;

	// get center of mass for player.
	vec3_t center = g_cl.m_local->WorldSpaceCenter( );

	// get delta between center of mass and final nade pos.
	vec3_t delta = center - m_origin;

	// check if our path was obstructed by anything using a trace.
	g_csgo.m_engine_trace->TraceRay( Ray( vec3_t( m_origin.x, m_origin.y, m_origin.z + 5.f ), center ), MASK_SHOT, ( ITraceFilter* )&filter, &trace );
	//g_csgo.m_debug_overlay->AddLineOverlay( vec3_t( m_origin.x, m_origin.y, m_origin.z + 5.f ), center, 255, 0, 0, true, 0.1f );

	if( trace.m_entity == g_cl.m_local ) {
		if( m_index == HEGRENADE && distance <= 20 ) {
			alpha_damage = 255 - 255 * ( distance / 20 );
		}

		if( ( m_index == MOLOTOV || m_index == FIREBOMB ) && distance <= 15 ) {
			alpha_damage = 255 - 255 * ( distance / 15 );
		}
	}
	else
		alpha_damage = 0;

	// { HEGRENADE, 'j' }
	// { MOLOTOV, 'l' }

	if( prev_on_screen ) {
		render::circle( prev_screen.x, prev_screen.y, 24, 30, Color( 0, 0, 0, 180 ) );
		render::circle( prev_screen.x, prev_screen.y, 20, 30, Color( 255, 0, 0, alpha_damage ) );
		render::gw.string( prev_screen.x + 8, prev_screen.y - 13, colors::white, m_index == HEGRENADE ? "j" : "l", render::ALIGN_RIGHT );
	}

	auto is_on_screen = [ ]( vec3_t origin, vec2_t& screen ) -> bool {
		if( !render::WorldToScreen( origin, screen ) )
			return false;

		return ( screen.x > 0 && screen.x < g_cl.m_width ) && ( g_cl.m_height > screen.y && screen.y > 0 );
	};

	vec2_t screenPos;
	vec3_t vEnemyOrigin = m_origin;
	vec3_t vLocalOrigin = g_cl.m_local->GetAbsOrigin( );
	if( !g_cl.m_local->alive( ) )
		vLocalOrigin = g_csgo.m_input->m_camera_offset;

	if( !is_on_screen( vEnemyOrigin, screenPos ) ) //out_of_fov_grenade_warning
	{
		const float wm = g_cl.m_width / 2, hm = g_cl.m_height / 2;
		vec3_t last_pos = std::get< vec3_t >( m_path.at( m_path.size( ) - 1 ) );

		ang_t dir;
		g_csgo.m_engine->GetViewAngles( dir );

		float view_angle = dir.y;

		if( view_angle < 0 )
			view_angle += 360;

		view_angle = math::deg_to_rad( view_angle );

		auto entity_angle = math::calc_angle( vLocalOrigin, vEnemyOrigin );
		entity_angle.normalize( );

		if( entity_angle.y < 0.f )
			entity_angle.y += 360.f;

		entity_angle.y = math::deg_to_rad( entity_angle.y );
		entity_angle.y -= view_angle;

		auto position = vec2_t( wm, hm );
		position.x -= std::clamp( vLocalOrigin.dist_to( vEnemyOrigin ), 100.f, hm - 100 );

		rotate_point( position, vec2_t( wm, hm ), false, entity_angle.y );

		render::circle( position.x, position.y, 24, 30, Color( 0, 0, 0, 180 ) );
		render::circle( position.x, position.y, 20, 30, Color( 255, 0, 0, alpha_damage ) );
		render::gw.string( position.x + 8, position.y - 13, colors::white, m_index == HEGRENADE ? "j" : "l", render::ALIGN_RIGHT );
	}

	return true;
}

float& Player::get_creation_time( ) {
	return *reinterpret_cast< float* >( reinterpret_cast< std::uintptr_t >( this ) + 0x29B0 );
}

void c_grenade_prediction::grenade_warning( Player* entity ) {
	auto& predicted_nades = g_grenades_pred.get_list( );

	static auto last_server_tick = g_csgo.m_cl->m_server_tick;
	if( last_server_tick != g_csgo.m_cl->m_server_tick ) {
		predicted_nades.clear( );

		last_server_tick = g_csgo.m_cl->m_server_tick;
	}

	if( entity->dormant( ) )
		return;

	const auto client_class = entity->GetClientClass( );
	if( !client_class || client_class->m_ClassID != 98 && client_class->m_ClassID != 9 )
		return;

	if( client_class->m_ClassID == 9 ) {
		const auto model = entity->GetModel( );
		if( !model )
			return;

		const auto studio_model = g_csgo.m_model_info->GetStudioModel( model );
		if( !studio_model || std::string_view( studio_model->m_name ).find( "fraggrenade" ) == std::string::npos )
			return;
	}

	const auto handle = entity->GetRefEHandle( );

	if( predicted_nades.find( handle ) == predicted_nades.end( ) ) {
		predicted_nades.emplace( std::piecewise_construct,
								 std::forward_as_tuple( handle ),
								 std::forward_as_tuple( g_csgo.m_entlist->GetClientEntityFromHandle< Player* >( reinterpret_cast< Weapon* >( entity )->m_hThrower( ) ),
														client_class->m_ClassID == 98 ? MOLOTOV : HEGRENADE,
														entity->m_vecOrigin( ),
														reinterpret_cast< Player* >( entity )->m_vecVelocity( ),
														entity->get_creation_time( ),
														game::TIME_TO_TICKS( reinterpret_cast< Player* >( entity )->m_flSimulationTime( ) - entity->get_creation_time( ) ) ) );
	}

	if( predicted_nades.at( handle ).draw( ) )
		return;

	predicted_nades.erase( handle );
}
