#include "includes.h"

bool Hooks::OverrideConfig( MaterialSystem_Config_t* config, bool update ) {
	if( !this || !g_csgo.m_engine->IsInGame( ) )
		return g_hooks.m_material_system.GetOldMethod< OverrideConfig_t >( IMaterialSystem::OVERRIDECONFIG )( this, config, update );

	if( g_hooks.m[ XOR( "brightnessAdjustment_sel" ) ][ 1 ] )
		config->m_nFullbright = true;

	return g_hooks.m_material_system.GetOldMethod< OverrideConfig_t >( IMaterialSystem::OVERRIDECONFIG )( this, config, update );
}