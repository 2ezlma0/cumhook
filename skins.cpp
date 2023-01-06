#include "includes.h"

Skins g_skins{};;

void Skins::load( ) {
	// update model indexes on mapload.
	m_knife_data[ knives_t::BAYONET ].setup( KNIFE_BAYONET,
											 XOR( "models/weapons/v_knife_bayonet.mdl" ),
											 XOR( "models/weapons/w_knife_bayonet.mdl" ),
											 XOR( "bayonet" ) );

	m_knife_data[ knives_t::BOWIE ].setup( KNIFE_BOWIE,
										   XOR( "models/weapons/v_knife_survival_bowie.mdl" ),
										   XOR( "models/weapons/w_knife_survival_bowie.mdl" ),
										   XOR( "knife_survival_bowie" ) );


	m_knife_data[ knives_t::BUTTERFLY ].setup( KNIFE_BUTTERFLY,
											   XOR( "models/weapons/v_knife_butterfly.mdl" ),
											   XOR( "models/weapons/w_knife_butterfly.mdl" ),
											   XOR( "knife_butterfly" ) );

	m_knife_data[ knives_t::FALCHION ].setup( KNIFE_FALCHION,
											  XOR( "models/weapons/v_knife_falchion_advanced.mdl" ),
											  XOR( "models/weapons/w_knife_falchion_advanced.mdl" ),
											  XOR( "knife_falchion" ) );

	m_knife_data[ knives_t::FLIP ].setup( KNIFE_FLIP,
										  XOR( "models/weapons/v_knife_flip.mdl" ),
										  XOR( "models/weapons/w_knife_flip.mdl" ),
										  XOR( "knife_flip" ) );

	m_knife_data[ knives_t::GUT ].setup( KNIFE_GUT,
										 XOR( "models/weapons/v_knife_gut.mdl" ),
										 XOR( "models/weapons/w_knife_gut.mdl" ),
										 XOR( "knife_gut" ) );

	m_knife_data[ knives_t::HUNTSMAN ].setup( KNIFE_HUNTSMAN,
											  XOR( "models/weapons/v_knife_tactical.mdl" ),
											  XOR( "models/weapons/w_knife_tactical.mdl" ),
											  XOR( "knife_tactical" ) );

	m_knife_data[ knives_t::KARAMBIT ].setup( KNIFE_KARAMBIT,
											  XOR( "models/weapons/v_knife_karam.mdl" ),
											  XOR( "models/weapons/w_knife_karam.mdl" ),
											  XOR( "knife_karambit" ) );

	m_knife_data[ knives_t::M9 ].setup( KNIFE_M9_BAYONET,
										XOR( "models/weapons/v_knife_m9_bay.mdl" ),
										XOR( "models/weapons/w_knife_m9_bay.mdl" ),
										XOR( "knife_m9_bayonet" ) );

	m_knife_data[ knives_t::DAGGER ].setup( KNIFE_SHADOW_DAGGERS,
											XOR( "models/weapons/v_knife_push.mdl" ),
											XOR( "models/weapons/w_knife_push.mdl" ),
											XOR( "knife_push" ) );

	// update glove model indexes on mapload.
	m_glove_data[ gloves_t::BLOODHOUND ].setup( 5027,
												XOR( "models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl" ),
												XOR( "models/weapons/w_models/arms/glove_bloodhound/w_glove_bloodhound.mdl" ) );

	m_glove_data[ gloves_t::SPORTY ].setup( 5030,
											XOR( "models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl" ),
											XOR( "models/weapons/w_models/arms/glove_sporty/w_glove_sporty.mdl" ) );

	m_glove_data[ gloves_t::DRIVER ].setup( 5031,
											XOR( "models/weapons/v_models/arms/glove_slick/v_glove_slick.mdl" ),
											XOR( "models/weapons/w_models/arms/glove_slick/w_glove_slick.mdl" ) );

	m_glove_data[ gloves_t::HANDWRAP ].setup( 5032,
											  XOR( "models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl" ),
											  XOR( "models/weapons/w_models/arms/glove_handwrap_leathery/w_glove_handwrap_leathery.mdl" ) );

	m_glove_data[ gloves_t::MOTO ].setup( 5033,
										  XOR( "models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl" ),
										  XOR( "models/weapons/w_models/arms/glove_motorcycle/w_glove_motorcycle.mdl" ) );

	m_glove_data[ gloves_t::SPECIALIST ].setup( 5034,
												XOR( "models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl" ),
												XOR( "models/weapons/w_models/arms/glove_specialist/w_glove_specialist.mdl" ) );

	m_update_time = 0.f;
}

void Skins::think( ) {
	std::vector< Weapon* > weapons{};

	if( !g_hooks.b[ XOR( "skins_enabled" ) ] )
		return;

	if( !g_csgo.m_engine->IsInGame( ) )
		return;

	if( !g_cl.m_local || !g_cl.m_local->alive( ) )
		return;

	player_info_t info;
	if( !g_csgo.m_engine->GetPlayerInfo( g_cl.m_local->index( ), &info ) )
		return;

	// store knife index.
	KnifeData* knife = &m_knife_data[ g_hooks.i[ XOR( "skins_knife_model" ) ] + 1 ];

	for( int i{ 1 }; i <= g_csgo.m_entlist->GetHighestEntityIndex( ); ++i ) {
		Entity* ent = g_csgo.m_entlist->GetClientEntity( i );
		if( !ent )
			continue;

		// run knifechanger.
		if( g_hooks.b[ ( XOR( "knife" ) ) ] && ent->is( HASH( "CPredictedViewModel" ) ) ) {
			// get weapon entity from predicted viewmodel.
			Weapon* weapon = ent->as< Weapon* >( )->GetWeapon( );

			// no need to be here anymore.
			if( !weapon || !weapon->IsKnife( ) )
				continue;

			// this weapon does not belong to us, we are done here.
			// should barely happen unless you pick up someones knife.
			// possible on servers with 'mp_drop_knife_enable 1'.
			if( info.m_xuid_low != weapon->m_OriginalOwnerXuidLow( ) || info.m_xuid_high != weapon->m_OriginalOwnerXuidHigh( ) )
				continue;

			// get and validate world model handle from the weapon
			Weapon* weapon_world_model = weapon->GetWeaponWorldModel( );
			if( !weapon_world_model )
				continue;

			// set model index of the predicted viewmodel.
			ent->m_nModelIndex( ) = knife->m_model_index;

			// set the world model index.
			// do this to have knifechanger in third person. verry p.
			weapon_world_model->m_nModelIndex( ) = knife->m_world_model_index;

			// correct m_nSequence and m_flCycle.
			UpdateAnimations( ent );
		}

		else if( ent->IsBaseCombatWeapon( ) ) {
			// cast to weapon class.
			Weapon* weapon = ent->as< Weapon* >( );

			// this is not our gun.
			if( info.m_xuid_low != weapon->m_OriginalOwnerXuidLow( ) || info.m_xuid_high != weapon->m_OriginalOwnerXuidHigh( ) )
				continue;

			// we found a weapon that we own.
			weapons.push_back( weapon );

			if( weapon->IsKnife( ) ) {
				// if this weapon is a knife, set some additional stuff.
				if( g_hooks.b[ ( XOR( "knife" ) ) ] ) {
					// set the item id, this is for the hud.
					weapon->m_iItemDefinitionIndex( ) = knife->m_id;

					// not needed. but everyone does this, try without.
					weapon->m_nModelIndex( ) = knife->m_model_index;

					// not needed. but everyone does this, try without.
					weapon->m_iViewModelIndex( ) = knife->m_model_index;

					// not needed. but everyone does this, try without.
					weapon->m_iWorldModelIndex( ) = knife->m_world_model_index;

					// set the little star thing.
					weapon->m_iEntityQuality( ) = 3;
				}

				else if( !g_hooks.b[ ( XOR( "knife" ) ) ] ) {
					// fix definition.
					if( g_cl.m_local->m_iTeamNum( ) == TEAM_TERRORISTS )
						weapon->m_iItemDefinitionIndex( ) = KNIFE_T;

					else if( g_cl.m_local->m_iTeamNum( ) == TEAM_COUNTERTERRORISTS )
						weapon->m_iItemDefinitionIndex( ) = KNIFE_CT;

					// reset.
					weapon->m_iEntityQuality( ) = 0;
					weapon->m_nFallbackPaintKit( ) = -1;
					weapon->m_nFallbackStatTrak( ) = -1;
				}
			}

			switch( weapon->m_iItemDefinitionIndex( ) ) {
				case DEAGLE:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_DEAGLE_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_DEAGLE_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_DEAGLE_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_DEAGLE_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case ELITE:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_ELITE_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_ELITE_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_ELITE_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_ELITE_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case FIVESEVEN:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_FIVESEVEN_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_FIVESEVEN_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_FIVESEVEN_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_FIVESEVEN_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case GLOCK:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_GLOCK_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_GLOCK_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_GLOCK_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_GLOCK_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case AK47:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_AK47_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_AK47_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_AK47_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_AK47_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case AUG:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_AUG_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_AUG_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_AUG_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_AUG_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case AWP:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_AWP_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_AWP_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_AWP_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_AWP_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case FAMAS:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_FAMAS_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_FAMAS_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_FAMAS_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_FAMAS_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case G3SG1:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_G3SG1_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_G3SG1_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_G3SG1_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_G3SG1_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case GALIL:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_GALIL_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_GALIL_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_GALIL_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_GALIL_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case M249:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_M249_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_M249_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_M249_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_M249_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case M4A4:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_M4A4_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_M4A4_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_M4A4_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_M4A4_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case MAC10:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_MAC10_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_MAC10_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_MAC10_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_MAC10_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case P90:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_P90_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_P90_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_P90_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_P90_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case UMP45:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_UMP45_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_UMP45_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_UMP45_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_UMP45_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case XM1014:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_XM1014_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_XM1014_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_XM1014_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_XM1014_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case BIZON:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_BIZON_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_BIZON_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_BIZON_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_BIZON_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case MAG7:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_MAG7_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_MAG7_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_MAG7_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_MAG7_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case NEGEV:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_NEGEV_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_NEGEV_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_NEGEV_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_NEGEV_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case SAWEDOFF:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_SAWEDOFF_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_SAWEDOFF_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_SAWEDOFF_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_SAWEDOFF_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case TEC9:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_TEC9_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_TEC9_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_TEC9_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_TEC9_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case P2000:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_P2000_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_P2000_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_P2000_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_P2000_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case MP7:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_MP7_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_MP7_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_MP7_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_MP7_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case MP9:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_MP9_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_MP9_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_MP9_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_MP9_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case NOVA:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_NOVA_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_NOVA_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_NOVA_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_NOVA_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case P250:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_P250_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_P250_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_P250_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_P250_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case SCAR20:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_SCAR20_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_SCAR20_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_SCAR20_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_SCAR20_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case SG553:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_SG553_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_SG553_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_SG553_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_SG553_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case SSG08:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_SSG08_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_SSG08_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_SSG08_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_SSG08_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case M4A1S:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_M4A1S_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_M4A1S_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_M4A1S_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_M4A1S_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case USPS:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_USPS_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_USPS_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_USPS_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_USPS_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case CZ75A:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_CZ75A_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_CZ75A_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_CZ75A_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_CZ75A_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case REVOLVER:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_REVOLVER_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_REVOLVER_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_REVOLVER_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_REVOLVER_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case KNIFE_BAYONET:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_KNIFE_BAYONET_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_KNIFE_BAYONET_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_KNIFE_BAYONET_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_KNIFE_BAYONET_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case KNIFE_FLIP:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_KNIFE_FLIP_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_KNIFE_FLIP_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_KNIFE_FLIP_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_KNIFE_FLIP_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case KNIFE_GUT:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_KNIFE_GUT_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_KNIFE_GUT_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_KNIFE_GUT_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_KNIFE_GUT_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case KNIFE_KARAMBIT:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_KNIFE_KARAMBIT_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_KNIFE_KARAMBIT_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_KNIFE_KARAMBIT_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_KNIFE_KARAMBIT_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case KNIFE_M9_BAYONET:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_KNIFE_M9_BAYONET_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_KNIFE_M9_BAYONET_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_KNIFE_M9_BAYONET_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_KNIFE_M9_BAYONET_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case KNIFE_HUNTSMAN:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_KNIFE_HUNTSMAN_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_KNIFE_HUNTSMAN_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_KNIFE_HUNTSMAN_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_KNIFE_HUNTSMAN_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case KNIFE_FALCHION:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_KNIFE_FALCHION_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_KNIFE_FALCHION_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_KNIFE_FALCHION_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_KNIFE_FALCHION_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case KNIFE_BOWIE:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_KNIFE_BOWIE_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_KNIFE_BOWIE_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_KNIFE_BOWIE_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_KNIFE_BOWIE_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case KNIFE_BUTTERFLY:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_KNIFE_BUTTERFLY_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_KNIFE_BUTTERFLY_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_KNIFE_BUTTERFLY_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_KNIFE_BUTTERFLY_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;
				case KNIFE_SHADOW_DAGGERS:
					weapon->m_nFallbackPaintKit( ) = g_hooks.i[ XOR( "skin_KNIFE_SHADOW_DAGGERS_id" ) ];
					weapon->m_nFallbackStatTrak( ) = g_hooks.b[ XOR( "skin_KNIFE_SHADOW_DAGGERS_stattrak" ) ] ? 1337 : -1;
					weapon->m_nFallbackSeed( ) = g_hooks.f[ XOR( "skin_KNIFE_SHADOW_DAGGERS_seed" ) ];
					weapon->m_flFallbackWear( ) = ( ( 100.f - g_hooks.f[ XOR( "skin_KNIFE_SHADOW_DAGGERS_quality" ) ] ) / 100.f ) + FLT_EPSILON;
					break;

				default:
					break;
			}

			// fix stattrak ownership.
			weapon->m_iAccountID( ) = info.m_xuid_low;

			// fix stattrak in hud.
			if( weapon->m_nFallbackStatTrak( ) >= 0 )
				weapon->m_iEntityQuality( ) = 9;

			// force use fallback values.
			weapon->m_iItemIDHigh( ) = -1;
		}
	}

	if( g_hooks.b[ ( XOR( "glove" ) ) ] ) {
		CBaseHandle* wearables = g_cl.m_local->m_hMyWearables( );

		// get pointer to entity from wearable handle 0.
		// glove is at wearable 0, valve will likely add more wearables like hats and boots. thats why.
		Weapon* glove = g_csgo.m_entlist->GetClientEntityFromHandle< Weapon* >( wearables[ 0 ] );

		// there is no glove entity yet.
		if( !glove ) {
			// attempt to get our old glove.
			Weapon* old = g_csgo.m_entlist->GetClientEntityFromHandle< Weapon* >( m_glove_handle );

			// this glove is still valid.
			if( old ) {

				// write back handle to wearables.
				wearables[ 0 ] = m_glove_handle;

				// set glove pointer.
				glove = old;
			}
		}

		// if we at this point still not have a glove entity.
		// we should create one.
		if( !glove ) {
			ClientClass* list{ g_csgo.m_client->GetAllClasses( ) };

			// iterate list.
			for( ; list != nullptr; list = list->m_pNext ) {

				// break if we found wearable
				if( list->m_ClassID == g_netvars.GetClientID( HASH( "CEconWearable" ) ) )
					break;
			}

			// create an ent index and serial no for the new glove entity.
			int index = g_csgo.m_entlist->GetHighestEntityIndex( ) + 1;
			int serial = g_csgo.RandomInt( 0xA00, 0xFFF );

			// call ctor on CEconWearable entity.
			Address networkable = list->m_pCreate( index, serial );

			// get entity ptr via index.
			glove = g_csgo.m_entlist->GetClientEntity< Weapon* >( index );

			static Address offset = g_netvars.get( HASH( "DT_EconEntity" ), HASH( "m_iItemIDLow" ) );

			// m_bInitialized?
			networkable.add( offset.add( 0x8 ) ).set< bool >( true );

			// no idea what this is.
			networkable.add( offset.sub( 0xC ) ).set< bool >( true );

			// set the wearable handle.
			wearables[ 0 ] = index | ( serial << 16 );

			// use this for later on.
			m_glove_handle = wearables[ 0 ];
		}

		// store glove data.
		GloveData* data = &m_glove_data[ g_hooks.i[ XOR( "skins_glove_model" ) ] + 1 ];
		int id = 0;
		switch( g_hooks.i[ XOR( "glove_selected" ) ] ) {
			case 0:
				id = 10007;
				break;
			case 1:
				id = 10006;
				break;
			case 2:
				id = 10008;
				break;
			case 3:
				id = 10039;
				break;
			case 4:
				id = 10048;
				break;
			case 5:
				id = 10047;
				break;
			case 6:
				id = 10045;
				break;
			case 7:
				id = 10046;
				break;
			case 8:
				id = 10037;
				break;
			case 9:
				id = 10018;
				break;
			case 10:
				id = 10038;
				break;
			case 11:
				id = 10019;
				break;
			case 12:
				id = 10041;
				break;
			case 13:
				id = 10042;
				break;
			case 14:
				id = 10043;
				break;
			case 15:
				id = 10044;
				break;
			case 16:
				id = 10016;
				break;
			case 17:
				id = 10013;
				break;
			case 18:
				id = 10015;
				break;
			case 19:
				id = 10040;
				break;
			case 20:
				id = 10053;
				break;
			case 21:
				id = 10054;
				break;
			case 22:
				id = 10056;
				break;
			case 23:
				id = 10055;
				break;
			case 24:
				id = 10021;
				break;
			case 25:
				id = 10036;
				break;
			case 26:
				id = 10009;
				break;
			case 27:
				id = 10010;
				break;
			case 28:
				id = 10049;
				break;
			case 29:
				id = 10051;
				break;
			case 30:
				id = 10052;
				break;
			case 31:
				id = 10050;
				break;
			case 32:
				id = 10026;
				break;
			case 33:
				id = 10027;
				break;
			case 34:
				id = 10028;
				break;
			case 35:
				id = 10024;
				break;
			case 36:
				id = 10063;
				break;
			case 37:
				id = 10061;
				break;
			case 38:
				id = 10064;
				break;
			case 39:
				id = 10062;
				break;
			case 40:
				id = 10033;
				break;
			case 41:
				id = 10034;
				break;
			case 42:
				id = 10035;
				break;
			case 43:
				id = 10030;
				break;
			default:
				break;
		}

		// set default data,
		glove->m_nFallbackSeed( ) = 0;
		glove->m_nFallbackStatTrak( ) = -1;
		glove->m_iItemIDHigh( ) = -1;
		glove->m_iEntityQuality( ) = 4;
		glove->m_iAccountID( ) = info.m_xuid_low;

		// set custom data.
		glove->m_nFallbackPaintKit( ) = id;
		glove->m_iItemDefinitionIndex( ) = data->m_id;
		glove->SetGloveModelIndex( data->m_model_index );

		// update the glove.
		glove->PreDataUpdate( DATA_UPDATE_CREATED );
	}

	// only force update every 1s.
	if( m_update && g_csgo.m_globals->m_curtime >= m_update_time ) {
		for( auto& w : weapons )
			UpdateItem( w );

		m_update = false;
		m_update_time = g_csgo.m_globals->m_curtime + 1.f;
	}
}

void Skins::UpdateItem( Weapon* item ) {
	if( !item || !item->IsBaseCombatWeapon( ) )
		return;

	if( g_csgo.m_cl->m_delta_tick == -1 )
		return;

	//  if ( sub_106E32D0(v4, "round_start") )
	//		SFWeaponSelection::ShowAndUpdateSelection( v2, 2, 0, v10 );

	// v1 = CHud::FindElement(&g_HUD, "SFWeaponSelection");
	// if( v1 )
	//		SFWeaponSelection::ShowAndUpdateSelection( ( int * )v1, 2, 0, this );

	item->m_bCustomMaterialInitialized( ) = item->m_nFallbackPaintKit( ) <= 0;

	item->m_CustomMaterials( ).RemoveAll( );
	item->m_CustomMaterials2( ).RemoveAll( );

	size_t count = item->m_VisualsDataProcessors( ).Count( );
	for( size_t i{}; i < count; ++i ) {
		auto& elem = item->m_VisualsDataProcessors( )[ i ];
		if( elem ) {
			elem->unreference( );
			elem = nullptr;
		}
	}

	item->m_VisualsDataProcessors( ).RemoveAll( );

	item->PostDataUpdate( DATA_UPDATE_CREATED );
	item->OnDataChanged( DATA_UPDATE_CREATED );

	CHudElement* SFWeaponSelection = g_csgo.m_hud->FindElement( HASH( "SFWeaponSelection" ) );
	g_csgo.ShowAndUpdateSelection( SFWeaponSelection, 0, item, false );
}

void Skins::UpdateAnimations( Entity* ent ) {
	int knife = g_hooks.i[ XOR( "skins_knife_model" ) ] + 1;

	int seq = ent->m_nSequence( );

	// check if this knife needs extra fixing.
	if( knife == knives_t::BUTTERFLY || knife == knives_t::FALCHION || knife == knives_t::DAGGER || knife == knives_t::BOWIE ) {

		// fix the idle sequences.
		if( seq == sequence_default_idle1 || seq == sequence_default_idle2 ) {
			// set the animation to be completed.
			ent->m_flCycle( ) = 0.999f;

			// cycle change, re-render.
			ent->InvalidatePhysicsRecursive( ANIMATION_CHANGED );
		}
	}

	// fix sequences.
	if( m_last_seq != seq ) {
		if( knife == knives_t::BUTTERFLY ) {
			switch( seq ) {
				case sequence_default_draw:
					seq = g_csgo.RandomInt( sequence_butterfly_draw, sequence_butterfly_draw2 );
					break;

				case sequence_default_lookat01:
					seq = g_csgo.RandomInt( sequence_butterfly_lookat01, sequence_butterfly_lookat03 );
					break;

				default:
					seq++;
					break;
			}
		}

		else if( knife == knives_t::FALCHION ) {
			switch( seq ) {
				case sequence_default_draw:
				case sequence_default_idle1:
					break;

				case sequence_default_idle2:
					seq = sequence_falchion_idle1;
					break;

				case sequence_default_heavy_miss1:
					seq = g_csgo.RandomInt( sequence_falchion_heavy_miss1, sequence_falchion_heavy_miss1_noflip );
					break;

				case sequence_default_lookat01:
					seq = g_csgo.RandomInt( sequence_falchion_lookat01, sequence_falchion_lookat02 );
					break;
			}
		}

		else if( knife == knives_t::DAGGER ) {
			switch( seq ) {
				case sequence_default_idle2:
					seq = sequence_push_idle1;
					break;

				case sequence_default_heavy_hit1:
				case sequence_default_heavy_backstab:
				case sequence_default_lookat01:
					seq += 3;
					break;

				case sequence_default_heavy_miss1:
					seq = sequence_push_heavy_miss2;
					break;
			}

		}

		else if( knife == knives_t::BOWIE ) {
			if( seq > sequence_default_idle1 )
				seq--;
		}

		m_last_seq = seq;
	}

	// write back fixed sequence.
	ent->m_nSequence( ) = seq;
}