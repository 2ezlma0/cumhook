#include "includes.h"

Chams g_chams{ };;

void Chams::SetColor( Color col, IMaterial* mat ) {
	if( mat )
		mat->ColorModulate( col );

	else
		g_csgo.m_render_view->SetColorModulation( col );
}

void Chams::SetAlpha( float alpha, IMaterial* mat ) {
	if( mat )
		mat->AlphaModulate( alpha );

	else
		g_csgo.m_render_view->SetBlend( alpha );
}

void Chams::SetupMaterial( IMaterial* mat, Color col, bool z_flag ) {
	SetColor( col );

	// mat->SetFlag(  MATERIAL_VAR_HALFLAMBERT, flags  );
	mat->SetFlag( MATERIAL_VAR_ZNEARER, z_flag );
	mat->SetFlag( MATERIAL_VAR_NOFOG, z_flag );
	mat->SetFlag( MATERIAL_VAR_IGNOREZ, z_flag );

	g_csgo.m_studio_render->ForcedMaterialOverride( mat );
}

void Chams::init( ) {
	//	  "$envmaptint" "[ 0.37 0.68 0.89 ]"
	std::ofstream( XOR( "csgo/materials/glowOverlay.vmt" ) ) << XOR( R"#("VertexLitGeneric" {
			"$additive" "1"
			"$envmap" "models/effects/cube_white"
			"$envmaptint" "[1 1 1]"
			"$envmapfresnel" "1"
			"$envmapfresnelminmaxexp" "[0 1 2]"
			"$alpha" "0.8"
		})#" );

	std::ofstream( XOR( "csgo/materials/Wireframe.vmt" ) ) << XOR( R"#( "VertexLitGeneric" {
			"$additive" "1"
	        "$envmap"  "models/effects/cube_white"
			"$envmaptint" "[ 0 0 0 ]"
			"$envmapfresnel" "1"
			"$envmapfresnelminmaxexp" "[ 0 16 12 ]"
			"$alpha" "0.8"
	} )#" );

	std::ofstream( XOR( "csgo/materials/testnigger.vmt" ) ) << XOR( R"#( "VertexLitGeneric" { 

  "$basetexture" "vgui/white_additive"
  "$ignorez"      "1"
  "$envmap"       "env_cubemap"
  "$envmaptint"   "[ 0.6 0.6 0.6 ]"
  "$nofog"        "1"
  "$model"        "1"
  "$nocull"       "0"
  "$selfillum"    "1"
  "$halflambert"  "1"
  "$znearer"      "0"
  "$flat"         "1"
	} )#" );

	std::ofstream( XOR( "csgo\\materials\\regular_ref.vmt" ) ) << XOR( R"#( "VertexLitGeneric" {
			"$basetexture"				    "vgui/white"
			"$envmap"						"env_cubemap"
			"$envmaptint"                   "[ .10 .10 .10 ]"
			"$pearlescent"					"0"
			"$phong"						"1"
			"$phongexponent"				"10"
			"$phongboost"					"1.0"
			"$rimlight"					    "1"
			"$rimlightexponent"		        "1"
			"$rimlightboost"		        "1"
			"$model"						"1"
			"$nocull"						"0"
			"$halflambert"				    "1"
			"$lightwarptexture"             "metalic"
		} )#" );

	std::ofstream( XOR( "csgo/materials/onetap_overlay.vmt" ) ) << XOR( R"#(" VertexLitGeneric "{

			"$additive" "1"
	        "$envmap"  "models/effects/cube_white"
			"$envmaptint" "[0 0 0]"
			"$envmapfresnel" "1"
			"$envmapfresnelminmaxexp" "[0 16 12]"
			"$alpha" "0.8"
	})#" );

	materialMetall = g_csgo.m_material_system->FindMaterial( XOR( "regular_ref" ), XOR( "Model textures" ) );
	materialMetall->IncrementReferenceCount( );

	materialMetall2 = g_csgo.m_material_system->FindMaterial( XOR( "regular_ref" ), XOR( "Model textures" ) );
	materialMetall2->IncrementReferenceCount( );

	materialMetallnZ = g_csgo.m_material_system->FindMaterial( XOR( "skeetchams" ), XOR( "Model textures" ) );
	materialMetallnZ->IncrementReferenceCount( );

	// find stupid materials.
	debugambientcube = g_csgo.m_material_system->FindMaterial( XOR( "debug/debugambientcube" ), XOR( "Model textures" ) );
	debugambientcube->IncrementReferenceCount( );

	debugdrawflat = g_csgo.m_material_system->FindMaterial( XOR( "debug/debugdrawflat" ), XOR( "Model textures" ) );
	debugdrawflat->IncrementReferenceCount( );

	materialMetall3 = g_csgo.m_material_system->FindMaterial( XOR( "testnigger" ), XOR( "Model textures" ) );
	materialMetall3->IncrementReferenceCount( );

	skeet = g_csgo.m_material_system->FindMaterial( XOR( "skeetchams" ), XOR( "Model textures" ) );
	skeet->IncrementReferenceCount( );

	onetap = g_csgo.m_material_system->FindMaterial( XOR( "onetap_overlay" ), XOR( "Model textures" ) );
	onetap->IncrementReferenceCount( );
}

bool Chams::GenerateLerpedMatrix( int index, matrix3x4_t* out ) {
	if( !g_cl.m_processing )
		return false;

	LagRecord* current_record;
	AimPlayer* data;

	Player* ent = g_csgo.m_entlist->GetClientEntity< Player* >( index );
	if( !ent )
		return false;

	if( !g_aimbot.IsValidTarget( ent ) )
		return false;

	data = &g_aimbot.m_players[ index - 1 ];
	if( !data || data->m_records.empty( ) )
		return false;

	if( data->m_records.size( ) < 2 )
		return false;

	auto* channel_info = g_csgo.m_engine->GetNetChannelInfo( );
	if( !channel_info )
		return false;

	static float max_unlag = 0.2f;
	static auto sv_maxunlag = g_csgo.m_cvar->FindVar( HASH( "sv_maxunlag" ) );
	if( sv_maxunlag ) {
		max_unlag = sv_maxunlag->GetFloat( );
	}

	for( auto it = data->m_records.rbegin( ); it != data->m_records.rend( ); it++ ) {
		current_record = it->get( );

		bool end = it + 1 == data->m_records.rend( );

		if( current_record && current_record->valid( ) && ( !end && ( ( it + 1 )->get( ) ) ) ) {
			if( current_record->m_origin.dist_to( ent->GetAbsOrigin( ) ) < 1.f ) {
				return false;
			}

			vec3_t next = end ? ent->GetAbsOrigin( ) : ( it + 1 )->get( )->m_origin;
			float  time_next = end ? ent->m_flSimulationTime( ) : ( it + 1 )->get( )->m_sim_time;

			float total_latency = channel_info->GetAvgLatency( 0 ) + channel_info->GetAvgLatency( 1 );
			std::clamp( total_latency, 0.f, max_unlag );

			float correct = total_latency + g_cl.m_lerp;
			float time_delta = time_next - current_record->m_sim_time;
			float add = end ? 1.f : time_delta;
			float deadtime = current_record->m_sim_time + correct + add;

			float curtime = g_csgo.m_globals->m_curtime;
			float delta = deadtime - curtime;

			float mul = 1.f / add;
			vec3_t lerp = math::Interpolate( next, current_record->m_origin, std::clamp( delta * mul, 0.f, 1.f ) );

			matrix3x4_t ret[ 128 ];

			std::memcpy( ret,
						 current_record->m_bones,
						 sizeof( ret ) );

			for( size_t i{ }; i < 128; ++i ) {
				vec3_t matrix_delta = current_record->m_bones[ i ].GetOrigin( ) - current_record->m_origin;
				ret[ i ].SetOrigin( matrix_delta + lerp );
			}

			std::memcpy( out,
						 ret,
						 sizeof( ret ) );

			return true;
		}
	}

	return false;
}

void Chams::RenderHistoryChams( int index ) {
	AimPlayer* data;

	Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( index );
	if( !player )
		return;

	if( !g_aimbot.IsValidTarget( player ) )
		return;

	bool enemy = g_cl.m_local && player->enemy( g_cl.m_local );
	if( enemy ) {
		data = &g_aimbot.m_players[ index - 1 ];
		if( !data || data->m_records.empty( ) )
			return;

		LagRecord* record = g_resolver.FindLastRecord( data );
		if( !record )
			return;

		if( record->m_broke_lc || record->m_invalid )
			return;

		// override blend.
		SetAlpha( g_hooks.c[ XOR( "shadow_color" ) ][ 3 ] );

		// set material and color.
		SetupMaterial( debugdrawflat, Color( g_hooks.c[ XOR( "shadow_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "shadow_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "shadow_color" ) ][ 2 ] * 255.f ), true );

		// was the matrix properly setup?
		matrix3x4_t arr[ 128 ];
		if( g_chams.GenerateLerpedMatrix( index, arr ) ) {
			// backup the bone cache before we fuck with it.
			auto backup_bones = player->m_BoneCache( ).m_pCachedBones;

			// replace their bone cache with our custom one.
			player->m_BoneCache( ).m_pCachedBones = arr;

			// manually draw the model.
			player->DrawModel( );

			// reset their bone cache to the previous one.
			player->m_BoneCache( ).m_pCachedBones = backup_bones;
		}
	}
}

void Chams::DrawChams( void* ecx, uintptr_t ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bone ) {
	if( strstr( info.m_model->m_name, XOR( "models/player" ) ) != nullptr ) {
		Player* m_entity = g_csgo.m_entlist->GetClientEntity<Player*>( info.m_index );
		if( !m_entity || !m_entity->alive( ) || m_entity->m_iHealth( ) < 0 )
			return;

		if( c_playerlist::get( )->should_disable_visuals( m_entity ) )
			return;

		if( g_hooks.b[ XOR( "localFakeShadow" ) ] && fakematrix != nullptr && m_entity->m_bIsLocalPlayer( ) && g_cl.m_processing && g_csgo.m_input->CAM_IsThirdPerson( ) ) {
			SetAlpha( g_hooks.c[ XOR( "localFakeShadow_color" ) ][ 3 ] - 0.01f );
			SetupMaterial( debugdrawflat, Color( g_hooks.c[ XOR( "localFakeShadow_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "localFakeShadow_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "localFakeShadow_color" ) ][ 2 ] * 255.f ), false );

			if( g_cl.m_local->m_flOldSimulationTime( ) != g_cl.m_local->m_flSimulationTime( ) ) {
				g_cl.m_local->SetAbsAngles( ang_t( 0, g_cl.m_radar.y, 0 ) );
				g_cl.m_local->SetupBones( g_chams.fakematrix, 128, 0x7FF00, g_csgo.m_globals->m_curtime );
				g_cl.m_local->SetAbsAngles( g_cl.m_rotation );
			}

			g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >( IVModelRender::DRAWMODELEXECUTE )( ecx, ctx, state, info, fakematrix );
			g_csgo.m_render_view->SetBlend( 1 );
			g_csgo.m_studio_render->ForcedMaterialOverride( nullptr );
			debugdrawflat->SetFlag( MATERIAL_VAR_ZNEARER, false );
			debugdrawflat->SetFlag( MATERIAL_VAR_NOFOG, false );
			debugdrawflat->SetFlag( MATERIAL_VAR_IGNOREZ, false );
		}

		g_csgo.m_studio_render->ForcedMaterialOverride( nullptr );
		g_csgo.m_render_view->SetColorModulation( colors::white );
		g_csgo.m_render_view->SetBlend( 1.f );

		Color color_glow = Color( g_hooks.c[ XOR( "playerglowColor" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "playerglowColor" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "playerglowColor" ) ][ 2 ] * 255.f );
		Color color_mettal = Color( g_hooks.c[ XOR( "playerReflectivityColor" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "playerReflectivityColor" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "playerReflectivityColor" ) ][ 2 ] * 255.f );
		if( ( g_hooks.i[ XOR( "selc" ) ] == 2 || g_hooks.i[ XOR( "selc" ) ] == 3 ) && m_entity ) {

			if( skeet ) {
				auto tint = skeet->find_var( XOR( "$envmaptint" ) );
				if( tint )
					tint->set_vector( vec3_t( color_glow.r( ) / 255.f, color_glow.g( ) / 255.f, color_glow.b( ) / 255.f ) );
			}

			if( materialMetall ) {
				auto phong = materialMetall->find_var( XOR( "$phongexponent" ) );
				if( phong )
					phong->set_float( g_hooks.f[ XOR( "reflectivity_f" ) ] );

				auto phong2 = materialMetall->find_var( XOR( "$phongboost" ) );
				if( phong2 )
					phong2->set_float( g_hooks.f[ XOR( "shine_f" ) ] / 100.f );

				auto rim2 = materialMetall->find_var( XOR( "$rimlightboost" ) );
				if( rim2 )
					rim2->set_float( g_hooks.f[ XOR( "rim_f" ) ] );

				auto var = materialMetall->find_var( XOR( "$envmaptint" ) );
				if( var )
					var->set_vector( vec3_t( color_mettal.r( ) / 255.f, color_mettal.g( ) / 255.f, color_mettal.b( ) / 255.f ) );
			}

			int dadadaa = 0;
		}

		if( m_entity == g_cl.m_local && g_cl.m_processing && g_cl.m_local ) {
			if( g_hooks.b[ XOR( "localchams" ) ] ) {
				Color color_glow = Color( g_hooks.c[ XOR( "localglowColor" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "localglowColor" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "localglowColor" ) ][ 2 ] * 255.f );
				Color color_mettal = Color( g_hooks.c[ XOR( "localReflectivityColor" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "localReflectivityColor" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "localReflectivityColor" ) ][ 2 ] * 255.f );
				if( materialMetallnZ ) {
					auto tint = materialMetallnZ->find_var( XOR( "$envmaptint" ) );
					if( tint )
						tint->set_vector( vec3_t( color_glow.r( ) / 255.f, color_glow.g( ) / 255.f, color_glow.b( ) / 255.f ) );
				}

				if( materialMetall2 ) {
					auto phong = materialMetall2->find_var( XOR( "$phongexponent" ) );
					if( phong )
						phong->set_float( g_hooks.f[ XOR( "local_reflectivity_f" ) ] );

					auto phong2 = materialMetall2->find_var( XOR( "$phongboost" ) );
					if( phong2 )
						phong2->set_float( g_hooks.f[ XOR( "local_shine_f" ) ] );

					auto rim2 = materialMetall2->find_var( XOR( "$rimlightboost" ) );
					if( rim2 )
						rim2->set_float( g_hooks.f[ XOR( "local_rim_f" ) ] );

					auto var = materialMetall2->find_var( XOR( "$envmaptint" ) );
					if( var )
						var->set_vector( vec3_t( color_mettal.r( ) / 255.f, color_mettal.g( ) / 255.f, color_mettal.b( ) / 255.f ) );
				}

				// override blend.
				Color color = Color( g_hooks.c[ XOR( "localchmas_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "localchmas_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "localchmas_color" ) ][ 2 ] * 255.f );

				SetAlpha( g_hooks.c[ XOR( "localchmas_color" ) ][ 3 ] );

				// set material and color.
				switch( g_hooks.i[ XOR( "local_selc" ) ] ) {
					case 0:
						SetupMaterial( debugambientcube, color, false );
						break;
					case 1:
						SetupMaterial( debugdrawflat, color, false );
						break;
					case 2:
						SetupMaterial( materialMetall, color, false );
						break;
					case 3:
						SetupMaterial( skeet, color, false );
						break;
					case 4:
						SetupMaterial( onetap, color, false );
						break;
					case 5:
						SetupMaterial( materialMetall3, color, false );
						break;
					default:
						break;
				}

				if( g_hooks.b[ XOR( "localblend" ) ] && m_entity->m_bIsScoped( ) )
					SetAlpha( 70.f / 100.f );

				g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >( IVModelRender::DRAWMODELEXECUTE )( ecx, ctx, state, info, bone );
			}

			if( g_hooks.b[ XOR( "localblend" ) ] && m_entity->m_bIsScoped( ) )
				SetAlpha( 70.f / 100.f );
		}

		bool enemy = g_cl.m_local && m_entity->enemy( g_cl.m_local );

		if( enemy && g_hooks.b[ XOR( "shadow" ) ] ) {
			if( g_aimbot.IsValidTarget( m_entity ) ) {
				AimPlayer* data = &g_aimbot.m_players[ m_entity->index( ) - 1 ];
				if( data && !data->m_records.empty( ) ) {
					LagRecord* record = g_resolver.FindLastRecord( data );
					if( record && ( !record->m_broke_lc || !record->m_invalid ) ) {
						// was the matrix properly setup?
						matrix3x4_t arr[ 128 ];
						if( Chams::GenerateLerpedMatrix( m_entity->index( ), arr ) ) {
							// override blend.
							SetAlpha( g_hooks.c[ XOR( "shadow_color" ) ][ 3 ] );

							// set material and color.
							SetupMaterial( debugdrawflat, Color( g_hooks.c[ XOR( "shadow_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "shadow_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "shadow_color" ) ][ 2 ] * 255.f ), true );

							g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >( IVModelRender::DRAWMODELEXECUTE )( ecx, ctx, state, info, arr );
							g_csgo.m_render_view->SetBlend( 1 );
							g_csgo.m_studio_render->ForcedMaterialOverride( nullptr );
							debugdrawflat->SetFlag( MATERIAL_VAR_ZNEARER, false );
							debugdrawflat->SetFlag( MATERIAL_VAR_NOFOG, false );
							debugdrawflat->SetFlag( MATERIAL_VAR_IGNOREZ, false );
						}
					}
				}
			}
		}

		if( ( enemy && g_hooks.b[ XOR( "player" ) ] ) || ( !enemy && g_hooks.b[ XOR( "teammates" ) ] ) ) {
			if( g_hooks.b[ XOR( "playerBehindWall" ) ] ) {

				SetAlpha( g_hooks.c[ XOR( "playerBehindWall_color" ) ][ 3 ] );

				switch( g_hooks.i[ XOR( "selc" ) ] ) {
					case 0:
						SetupMaterial( debugambientcube, Color( g_hooks.c[ XOR( "playerBehindWall_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "playerBehindWall_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "playerBehindWall_color" ) ][ 2 ] * 255.f ), true );
						break;
					case 1:
						SetupMaterial( debugdrawflat, Color( g_hooks.c[ XOR( "playerBehindWall_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "playerBehindWall_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "playerBehindWall_color" ) ][ 2 ] * 255.f ), true );
						break;
					case 2:
						SetupMaterial( materialMetall, Color( g_hooks.c[ XOR( "playerBehindWall_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "playerBehindWall_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "playerBehindWall_color" ) ][ 2 ] * 255.f ), true );
						break;
					case 3:
						SetupMaterial( skeet, Color( g_hooks.c[ XOR( "playerBehindWall_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "playerBehindWall_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "playerBehindWall_color" ) ][ 2 ] * 255.f ), true );
						break;
					case 4:
						SetupMaterial( onetap, Color( g_hooks.c[ XOR( "playerBehindWall_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "playerBehindWall_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "playerBehindWall_color" ) ][ 2 ] * 255.f ), true );
						break;
					case 5:
						SetupMaterial( materialMetall3, Color( g_hooks.c[ XOR( "playerBehindWall_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "playerBehindWall_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "playerBehindWall_color" ) ][ 2 ] * 255.f ), true );
						break;
					default:
						break;
				}

				g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >( IVModelRender::DRAWMODELEXECUTE )( ecx, ctx, state, info, bone );
			}

			SetAlpha( g_hooks.c[ XOR( "player_color" ) ][ 3 ] );

			switch( g_hooks.i[ XOR( "selc" ) ] ) {
				case 0:
					SetupMaterial( debugambientcube, Color( g_hooks.c[ XOR( "player_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "player_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "player_color" ) ][ 2 ] * 255.f ), false );
					break;
				case 1:
					SetupMaterial( debugdrawflat, Color( g_hooks.c[ XOR( "player_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "player_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "player_color" ) ][ 2 ] * 255.f ), false );
					break;
				case 2:
					SetupMaterial( materialMetall, Color( g_hooks.c[ XOR( "player_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "player_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "player_color" ) ][ 2 ] * 255.f ), false );
					break;
				case 3:
					SetupMaterial( skeet, Color( g_hooks.c[ XOR( "player_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "player_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "player_color" ) ][ 2 ] * 255.f ), false );
					break;
				case 4:
					SetupMaterial( onetap, Color( g_hooks.c[ XOR( "player_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "player_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "player_color" ) ][ 2 ] * 255.f ), false );
					break;
				case 5:
					SetupMaterial( materialMetall3, Color( g_hooks.c[ XOR( "player_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "player_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "player_color" ) ][ 2 ] * 255.f ), false );
					break;
				default:
					break;
			}

			g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >( IVModelRender::DRAWMODELEXECUTE )( ecx, ctx, state, info, bone );
		}
	}

	else if( g_cl.m_processing && strstr( info.m_model->m_name, XOR( "arms" ) ) != nullptr ) {
		if( g_cl.m_processing && g_hooks.b[ XOR( "hands" ) ] ) {
			Color color_glow = Color( g_hooks.c[ XOR( "handsglowColor" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "handsglowColor" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "handsglowColor" ) ][ 2 ] * 255.f );
			Color color_mettal = Color( g_hooks.c[ XOR( "handsReflectivityColor" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "handsReflectivityColor" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "handsReflectivityColor" ) ][ 2 ] * 255.f );
			if( materialMetallnZ ) {
				auto tint = materialMetallnZ->find_var( XOR( "$envmaptint" ) );
				if( tint )
					tint->set_vector( vec3_t( color_glow.r( ) / 255.f, color_glow.g( ) / 255.f, color_glow.b( ) / 255.f ) );
			}

			if( materialMetall2 ) {
				auto phong = materialMetall2->find_var( XOR( "$phongexponent" ) );
				if( phong )
					phong->set_float( g_hooks.f[ XOR( "reflectivity_hands" ) ] );

				auto phong2 = materialMetall2->find_var( XOR( "$phongboost" ) );
				if( phong2 )
					phong2->set_float( g_hooks.f[ XOR( "shine_hands" ) ] );

				auto rim2 = materialMetall2->find_var( XOR( "$rimlightboost" ) );
				if( rim2 )
					rim2->set_float( g_hooks.f[ XOR( "rim_hands" ) ] );

				auto var = materialMetall2->find_var( XOR( "$envmaptint" ) );
				if( var )
					var->set_vector( vec3_t( color_mettal.r( ) / 255.f, color_mettal.g( ) / 255.f, color_mettal.b( ) / 255.f ) );
			}

			SetAlpha( g_hooks.c[ XOR( "hands_color" ) ][ 3 ] );

			Color hands_color = Color( g_hooks.c[ XOR( "hands_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "hands_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "hands_color" ) ][ 2 ] * 255.f );

			switch( g_hooks.i[ XOR( "handsMaterial" ) ] ) {
				case 0:
					SetupMaterial( debugambientcube, hands_color, false );
					break;
				case 1:
					SetupMaterial( debugdrawflat, hands_color, false );
					break;
				case 2:
					SetupMaterial( materialMetall2, hands_color, false );
					break;
				case 3:
					SetupMaterial( materialMetallnZ, hands_color, false );
					break;
				case 4:
					SetupMaterial( onetap, hands_color, false );
					break;
				case 5:
					SetupMaterial( materialMetall3, hands_color, false );
					break;
				default:
					break;
			}

			g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >( IVModelRender::DRAWMODELEXECUTE )( ecx, ctx, state, info, bone );
		}
	}

	/*else if ( strstr( info.m_model->m_name, XOR( "v_" ) ) != nullptr ) {
		if ( g_cl.m_local->alive( ) ) {
			SetAlpha( g_menu.main.players.chams_friendly_blend.get( ) / 100.f );
			SetupMaterial( skeet, g_menu.main.players.chams_friendly_vis.get( ), false );
		}
	}*/
}

void Chams::RenderFake( ) {
	if( !g_cl.m_processing )
		return;

	//	// check if active
	//	if ( g_menu.main.antiaim.draw_angles_chams.get( ) ) {
	//		if ( g_cl.m_local ) {
	//			//g_csgo.m_render_view->SetBlend( g_menu.main.antiaim.draw_angles_chams_blend.get( ) / 100.f );
	//			SetAlpha( ( g_menu.main.antiaim.draw_angles_chams_blend.get( ) - 1 ) / 100.f );
	//			SetupMaterial( debugdrawflat, g_menu.main.antiaim.color_draw_angles_chams.get( ), false );
	//			ang_t fake1337( 0, g_cl.m_radar.y, 0 );
	//			ang_t real1337( 0, g_cl.m_abs_yaw, 0 );
	//	
	//			// set the nointerp flag.
	//			g_cl.m_local->SetAbsAngles( fake1337 );
	//			g_cl.m_local->DrawModel( );
	//			g_cl.m_local->SetAbsAngles( real1337 );
	//		}
	//	}
}

void Chams::SceneEnd( ) {
	// store and sort ents by distance.
	if( SortPlayers( ) ) {
	}

	// draw esp on ents.
	for( int i{ 1 }; i <= g_csgo.m_entlist->GetHighestEntityIndex( ); ++i ) {
		Player* ent = g_csgo.m_entlist->GetClientEntity< Player* >( i );
		if( !ent || !ent->enemy( g_cl.m_local ) )
			continue;

		auto client_class = ent->GetClientClass( );
		if( !client_class )
			continue;

		if( client_class->m_ClassID != CCSRagdoll )
			continue;

		if( c_playerlist::get( )->should_disable_visuals( ent ) )
			return;

		bool enemy = g_cl.m_local && ent->enemy( g_cl.m_local );

		if( enemy && g_hooks.b[ XOR( "player_dead" ) ] ) {
			SetAlpha( g_hooks.c[ XOR( "player_color" ) ][ 3 ] );

			switch( g_hooks.i[ XOR( "selc" ) ] ) {
				case 0:
					SetupMaterial( debugambientcube, Color( g_hooks.c[ XOR( "player_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "player_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "player_color" ) ][ 2 ] * 255.f ), false );
					break;
				case 1:
					SetupMaterial( debugdrawflat, Color( g_hooks.c[ XOR( "player_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "player_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "player_color" ) ][ 2 ] * 255.f ), false );
					break;
				case 2:
					SetupMaterial( materialMetall, Color( g_hooks.c[ XOR( "player_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "player_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "player_color" ) ][ 2 ] * 255.f ), false );
					break;
				case 3:
					SetupMaterial( skeet, Color( g_hooks.c[ XOR( "player_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "player_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "player_color" ) ][ 2 ] * 255.f ), false );
					break;
				case 4:
					SetupMaterial( onetap, Color( g_hooks.c[ XOR( "player_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "player_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "player_color" ) ][ 2 ] * 255.f ), false );
					break;
				case 5:
					SetupMaterial( materialMetall3, Color( g_hooks.c[ XOR( "player_color" ) ][ 0 ] * 255.f, g_hooks.c[ XOR( "player_color" ) ][ 1 ] * 255.f, g_hooks.c[ XOR( "player_color" ) ][ 2 ] * 255.f ), false );
					break;
				default:
					break;
			}

			ent->DrawModel( );
		}
	}

	// restore.
	g_csgo.m_studio_render->ForcedMaterialOverride( nullptr );
	g_csgo.m_render_view->SetColorModulation( colors::white );
	g_csgo.m_render_view->SetBlend( 1.f );
}

bool Chams::IsInViewPlane( const vec3_t& world ) {
	float w;

	const VMatrix& matrix = g_csgo.m_engine->WorldToScreenMatrix( );

	w = matrix[ 3 ][ 0 ] * world.x + matrix[ 3 ][ 1 ] * world.y + matrix[ 3 ][ 2 ] * world.z + matrix[ 3 ][ 3 ];

	return w > 0.001f;
}

bool Chams::OverridePlayer( int index ) {
	Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( index );
	if( !player )
		return false;

	// always skip the local player in DrawModelExecute.
	// this is because if we want to make the local player have less alpha
	// the static props are drawn after the players and it looks like aids.
	// therefore always process the local player in scene end.
	if( player->m_bIsLocalPlayer( ) )
		return true;

	// see if this player is an enemy to us.
	bool enemy = g_cl.m_local && player->enemy( g_cl.m_local );

	// we have chams on enemies.
	if( enemy && g_menu.main.players.chams_enemy.get( 0 ) )
		return true;

	// we have chams on friendly.
	else if( !enemy && g_menu.main.players.chams_friendly.get( 0 ) )
		return true;

	return false;
}

bool Chams::SortPlayers( ) {
	// lambda-callback for std::sort.
	// to sort the players based on distance to the local-player.
	static auto distance_predicate = [ ]( Entity* a, Entity* b ) {
		vec3_t local = g_cl.m_local->GetAbsOrigin( );

		// note - dex; using squared length to save out on sqrt calls, we don't care about it anyway.
		float len1 = ( a->GetAbsOrigin( ) - local ).length_sqr( );
		float len2 = ( b->GetAbsOrigin( ) - local ).length_sqr( );

		return len1 < len2;
	};

	// reset player container.
	m_players.clear( );

	// find all players that should be rendered.
	for( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i ) {
		// get player ptr by idx.
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );

		// validate.
		if( !player || !player->IsPlayer( ) || !player->alive( ) || player->m_iHealth( ) < 0 )
			continue;

		// do not draw players occluded by view plane.
		if( !IsInViewPlane( player->WorldSpaceCenter( ) ) )
			continue;

		// this player was not skipped to draw later.
		// so do not add it to our render list.
		if( !OverridePlayer( i ) )
			continue;

		m_players.push_back( player );
	}

	// any players?
	if( m_players.empty( ) )
		return false;

	// sorting fixes the weird weapon on back flickers.
	// and all the other problems regarding Z-layering in this shit game.
	std::sort( m_players.begin( ), m_players.end( ), distance_predicate );

	return true;
}