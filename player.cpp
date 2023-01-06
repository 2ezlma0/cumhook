#include "includes.h"

void Hooks::DoExtraBoneProcessing( int a2, int a3, int a4, int a5, int a6, int a7 ) {
	//// cast thisptr to player ptr.
	Player* player = ( Player* )this;

	///*
	//	zero out animstate player pointer so CCSGOPlayerAnimState::DoProceduralFootPlant will not do anything.

	//	.text:103BB25D 8B 56 60                                mov     edx, [esi+60h]
	//	.text:103BB260 85 D2                                   test    edx, edx
	//	.text:103BB262 0F 84 B4 0E 00 00                       jz      loc_103BC11C
	//*/

	//// get animstate ptr.
	//CCSGOPlayerAnimState* animstate = player->m_PlayerAnimState( );

	//// backup pointer.
	//Player*	backup{ nullptr };
	//
	//if( animstate ) {
	//	// backup player ptr.
	//	backup = animstate->m_player;
	//
	//	// null player ptr, GUWOP gang.
	//	animstate->m_player = nullptr;
	//}

	return;
	// don't call any fucking thing.
}

int previous_tick[ 64 ] = { 0 };
//vec3_t last_origin = {};

void Hooks::UpdateClientSideAnimation( ) {
	Player* player = ( Player* )this;

	if( !player )
		return;

	auto islocal = player->index( ) == g_csgo.m_engine->GetLocalPlayer( );

	if( g_cl.m_updating_anims || ( player->index( ) < 0 || player->index( ) > 64 ) )
		return g_hooks.m_UpdateClientSideAnimation( this );

	if( islocal ) {
		C_AnimationLayer backuplayers[ 14 ];

		const auto old_angles = player->GetAbsAngles( );
		const auto backup_poses = player->m_flPoseParameter( );

		player->GetAnimLayers( backuplayers );

		player->m_flPoseParameter( ) = g_cl.m_poses;
		player->SetAbsAngles( ang_t( 0, g_cl.m_rotation.y, 0 ) );
		player->SetAnimLayers( g_cl.m_layers.data( ) );

		player->SetupBones( 0, -1, BONE_USED_BY_ANYTHING, g_csgo.m_globals->m_curtime );

		player->m_flPoseParameter( ) = backup_poses;
		player->SetAnimLayers( backuplayers );
		player->SetAbsAngles( old_angles );
	}
	else {
		auto data = &g_aimbot.m_players[ player->index( ) - 1 ];
		if( !data || data->m_records.size( ) <= 0 )
			return;

		auto record = data->m_records[ 0 ].get( );
		if( !record
			|| previous_tick[ player->index( ) - 1 ] == g_csgo.m_globals->m_frame
			|| player->m_BoneCache( ).m_CachedBoneCount <= 0 )
			return;

		auto interpolation = player->GetAbsOrigin( );

		if( ( interpolation - record->m_render_origin ).length( ) >= 1.f ) {
			// update matrix origin
			for( auto i = 0; i < record->m_bones_count; i++ ) {
				record->m_render_bones[ i ][ 0 ][ 3 ] -= record->m_render_origin.x;
				record->m_render_bones[ i ][ 1 ][ 3 ] -= record->m_render_origin.y;
				record->m_render_bones[ i ][ 2 ][ 3 ] -= record->m_render_origin.z;

				record->m_render_bones[ i ][ 0 ][ 3 ] += interpolation.x;
				record->m_render_bones[ i ][ 1 ][ 3 ] += interpolation.y;
				record->m_render_bones[ i ][ 2 ][ 3 ] += interpolation.z;

				if( player->m_BoneCache( ).m_CachedBoneCount > i )
					player->m_BoneCache( ).m_pCachedBones[ i ] = record->m_render_bones[ i ];
			}

			record->m_render_origin = interpolation;
			player->attachments_helper( player->GetModelPtr( ) );
		}

		previous_tick[ player->index( ) - 1 ] = g_csgo.m_globals->m_frame;
	}
}

Weapon* Hooks::GetActiveWeapon( ) {
	Stack stack;

	static Address ret_1 = pattern::find( g_csgo.m_client_dll, XOR( "85 C0 74 1D 8B 88 ? ? ? ? 85 C9" ) );

	// note - dex; stop call to CIronSightController::RenderScopeEffect inside CViewRender::RenderView.
	if( g_hooks.b[ XOR( "removeScopeOverlay" ) ] ) {
		if( stack.ReturnAddress( ) == ret_1 )
			return nullptr;
	}

	return g_hooks.m_GetActiveWeapon( this );
}

void Hooks::StandardBlendingRules( int a2, int a3, int a4, int a5, int a6 ) {
	// cast thisptr to player ptr.
	Player* player = ( Player* )this;

	if( !player || ( player->index( ) - 1 ) > 63 || player == g_cl.m_local )
		return g_hooks.m_StandardBlendingRules( this, a2, a3, a4, a5, a6 );

	// disable interpolation.
	if( !( player->m_fEffects( ) & EF_NOINTERP ) )
		player->m_fEffects( ) |= EF_NOINTERP;

	g_hooks.m_StandardBlendingRules( this, a2, a3, a4, a5, a6 );

	// restore interpolation.
	player->m_fEffects( ) &= ~EF_NOINTERP;
}

void Hooks::BuildTransformations( int a2, int a3, int a4, int a5, int a6, int a7 ) {
	// cast thisptr to player ptr.
	Player* player = ( Player* )this;

	// get bone jiggle.
	int bone_jiggle = *reinterpret_cast< int* >( uintptr_t( player ) + 0x291C );

	// null bone jiggle to prevent attachments from jiggling around.
	*reinterpret_cast< int* >( uintptr_t( player ) + 0x291C ) = 0;

	// call og.
	g_hooks.m_BuildTransformations( this, a2, a3, a4, a5, a6, a7 );

	// restore bone jiggle.
	*reinterpret_cast< int* >( uintptr_t( player ) + 0x291C ) = bone_jiggle;
}

void CustomEntityListener::OnEntityCreated( Entity* ent ) {
	if( ent ) {
		// player created.
		if( ent->IsPlayer( ) ) {
			Player* player = ent->as< Player* >( );

			if( player->index( ) < 0 || player->index( ) > 64 )
				return;

			// access out player data stucture and reset player data.
			if( player->index( ) >= 1 ) {
				// access out player data stucture and reset player data.
				AimPlayer* data = &g_aimbot.m_players[ player->index( ) - 1 ];
				if( data )
					data->reset( );
			}

			// get ptr to vmt instance and reset tables.
			VMT* vmt = &g_hooks.m_player[ player->index( ) ];

			if( vmt ) {
				// init vtable with new ptr.
				vmt->reset( );
				vmt->init( player );

				// hook this on every player.
				g_hooks.m_DoExtraBoneProcessing = vmt->add< Hooks::DoExtraBoneProcessing_t >( Player::DOEXTRABONEPROCESSING, util::force_cast( &Hooks::DoExtraBoneProcessing ) );
				g_hooks.m_UpdateClientSideAnimation = vmt->add< Hooks::UpdateClientSideAnimation_t >( Player::UPDATECLIENTSIDEANIMATION, util::force_cast( &Hooks::UpdateClientSideAnimation ) );
				g_hooks.m_StandardBlendingRules = vmt->add< Hooks::StandardBlendingRules_t >( Player::STANDARDBLENDINGRULES, util::force_cast( &Hooks::StandardBlendingRules ) );

				// local gets special treatment.
				if( player->index( ) == g_csgo.m_engine->GetLocalPlayer( ) ) {
					g_hooks.m_UpdateClientSideAnimation = vmt->add< Hooks::UpdateClientSideAnimation_t >( Player::UPDATECLIENTSIDEANIMATION, util::force_cast( &Hooks::UpdateClientSideAnimation ) );
					g_hooks.m_BuildTransformations = vmt->add< Hooks::BuildTransformations_t >( Player::BUILDTRANSFORMATIONS, util::force_cast( &Hooks::BuildTransformations ) );
				}
			}
		}
	}
}

void CustomEntityListener::OnEntityDeleted( Entity* ent ) {
	// note; IsPlayer doesn't work here because the ent class is CBaseEntity.
	if( ent && ent->index( ) >= 0 && ent->index( ) <= 64 ) {
		Player* player = ent->as< Player* >( );

		if( player->index( ) >= 1 ) {
			// access out player data stucture and reset player data.
			AimPlayer* data = &g_aimbot.m_players[ player->index( ) - 1 ];
			if( data )
				data->reset( );
		}

		// get ptr to vmt instance and reset tables.
		VMT* vmt = &g_hooks.m_player[ player->index( ) ];
		//VMT* m_renderablevmt = &g_hooks.m_renderable[ player->index( ) ];

		if( vmt )
			vmt->reset( );

		/*if(m_renderablevmt)
			m_renderablevmt->reset( );*/
	}
}