#include "includes.h"

int Hooks::DebugSpreadGetInt( ) {
	Stack stack;

	static Address DrawCrosshair{ pattern::find( g_csgo.m_client_dll, XOR( "85 C0 0F 84 ? ? ? ? 8B 4C 24 1C" ) ) };

	if( g_menu.main.visuals.force_xhair.get( ) && stack.ReturnAddress( ) == DrawCrosshair )
		return 3;

	return g_hooks.m_debug_spread.GetOldMethod< GetInt_t >( ConVar::GETINT )( this );
}

bool Hooks::NetShowFragmentsGetBool( ) {
	static auto read_sub_channel_data_ret = pattern::find( g_csgo.m_engine_dll, XOR( "85 C0 74 12 53 FF 75 0C 68 ? ? ? ? FF 15 ? ? ? ? 83 C4 0C" ) ).as< uintptr_t*>( );
	static auto check_receiving_list_ret = pattern::find( g_csgo.m_engine_dll, XOR( "8B 1D ? ? ? ? 85 C0 74 16 FF B6" ) ).as< uintptr_t*>( );

	if( !g_csgo.m_engine->IsInGame( ) )
		return g_hooks.m_net_show_fragments.GetOldMethod< GetBool_t >( ConVar::GETBOOL )( this );

	static uint32_t last_fragment = 0;

	float latency = 0.f;
	INetChannel* nci = g_csgo.m_engine->GetNetChannelInfo( );
	if( nci )
	{
		latency = nci->GetAvgLatency( INetChannel::FLOW_INCOMING ) + nci->GetAvgLatency( INetChannel::FLOW_OUTGOING );
	}
	int ms = std::max( 0, ( int )std::round( ( ( latency * 1000.f ) * 1.1f ) ) );

	if( ( g_hooks.b[ XOR( "fake_latency" ) ] && g_hooks.auto_check( "fake_latency_key" ) ) || ms > 200 ) {
		if( _ReturnAddress( ) == reinterpret_cast< void* >( read_sub_channel_data_ret ) && last_fragment > 0 ) {
			const auto data = &reinterpret_cast< uint32_t* >( g_csgo.m_cl->m_net_channel )[ 0x56 ];
			const auto bytes_fragments = reinterpret_cast< uint32_t* >( data )[ 0x43 ];

			if( bytes_fragments == last_fragment ) {
				auto& buffer = reinterpret_cast< uint32_t* >( data )[ 0x42 ];
				buffer = 0;
			}
		}

		if( _ReturnAddress( ) == check_receiving_list_ret ) {
			const auto data = &reinterpret_cast< uint32_t* >( g_csgo.m_cl->m_net_channel )[ 0x56 ];
			const auto bytes_fragments = reinterpret_cast< uint32_t* >( data )[ 0x43 ];

			last_fragment = bytes_fragments;
		}
	}

	return g_hooks.m_net_show_fragments.GetOldMethod< GetBool_t >( ConVar::GETBOOL )( this );
}