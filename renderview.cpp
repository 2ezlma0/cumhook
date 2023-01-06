#include "includes.h"

void Hooks::SceneEnd( ) {
	if( !this || !g_csgo.m_engine->IsInGame( ) || !g_csgo.m_render_view || !g_csgo.m_studio_render )
		return g_hooks.m_render_view.GetOldMethod< Hooks::SceneEnd_t >( IVRenderView::SCENEEND )( this );

	g_hooks.m_render_view.GetOldMethod< Hooks::SceneEnd_t >( IVRenderView::SCENEEND )( this );

	g_chams.SceneEnd( );

	if( !g_cl.m_local || !g_hooks.b[ XOR( "disable_occulusion" ) ] )
		return;

	for( int i = 1; i < g_csgo.m_globals->m_max_clients; i++ ) {
		Player* pEntity = g_csgo.m_entlist->GetClientEntity< Player* >( i );
		if( !pEntity || !pEntity->alive( ) || pEntity->dormant( ) ||
			!g_cl.m_local->enemy( pEntity ) )
			continue;

		pEntity->DrawModel( );
	}
}