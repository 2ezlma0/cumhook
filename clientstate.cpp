#include "includes.h"

bool Hooks::TempEntities( void* msg ) {
	if( !g_cl.m_local || !g_csgo.m_engine->IsInGame( ) )
		return g_hooks.m_client_state.GetOldMethod< TempEntities_t >( CClientState::TEMPENTITIES )( this, msg );

	bool ret = g_hooks.m_client_state.GetOldMethod< TempEntities_t >( CClientState::TEMPENTITIES )( this, msg );
	if( !b[ XOR( "rbotenable" ) ] || !g_cl.m_local->m_iHealth( ) )
		return ret;

	CEventInfo* ei = g_csgo.m_cl->m_events;
	CEventInfo* next = nullptr;

	if( !ei )
		return ret;
	do {
		next = *( CEventInfo** )( ( uintptr_t )ei + 0x38 /* needs update */ );
		uint16_t classid = ei->m_class_id - 1;

		auto create_event = ei->m_client_class->m_pCreateEvent;
		if( !create_event )
			continue;

		void* pCE = create_event( );
		if( !pCE )
			continue;

		if( classid == 170 ) {
			ei->m_fire_delay = 0.0f;
		}
		ei = next;

	} while( next != nullptr );

	return ret;
}