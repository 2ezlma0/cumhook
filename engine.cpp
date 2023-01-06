#include "includes.h"

bool Hooks::IsConnected( ) {
	if( !this || !g_csgo.m_engine || !g_csgo.m_engine->IsInGame( ) )
		return g_hooks.m_engine.GetOldMethod< IsConnected_t >( IVEngineClient::ISCONNECTED )( this );

	Stack stack;

	static Address IsLoadoutAllowed{ pattern::find( g_csgo.m_client_dll, XOR( "84 C0 75 04 B0 01 5F" ) ) };

	if( g_hooks.b[ XOR( "unlock_inventory" ) ] && stack.ReturnAddress( ) == IsLoadoutAllowed )
		return false;

	return g_hooks.m_engine.GetOldMethod< IsConnected_t >( IVEngineClient::ISCONNECTED )( this );
}

bool Hooks::IsHLTV( ) {
	if( !this || !g_csgo.m_engine || !g_csgo.m_engine->IsInGame( ) )
		return g_hooks.m_engine.GetOldMethod< IsHLTV_t >( IVEngineClient::ISHLTV )( this );

	Stack stack;

	static Address SetupVelocity{ pattern::find( g_csgo.m_client_dll, XOR( "84 C0 75 38 8B 0D ? ? ? ? 8B 01 8B 80" ) ) };
	static Address AccumulateLayers{ pattern::find( g_csgo.m_client_dll, XOR( "84 C0 75 0D F6 87" ) ) };

	// if wanted, force the game to always call AccumulateLayers.
	if( g_bones.m_running )
		return true;

	// fix for animstate velocity.
	if( stack.ReturnAddress( ) == SetupVelocity )
		return true;

	if( stack.ReturnAddress( ) == AccumulateLayers )
		return true;

	return g_hooks.m_engine.GetOldMethod< IsHLTV_t >( IVEngineClient::ISHLTV )( this );
}

bool Hooks::IsPaused( ) {
	if( !this || !g_csgo.m_engine || !g_csgo.m_engine->IsInGame( ) )
		return g_hooks.m_engine.GetOldMethod< IsPaused_t >( IVEngineClient::ISPAUSED )( this );

	static DWORD* return_to_extrapolation = ( DWORD* )( pattern::find( g_csgo.m_client_dll,
																	   XOR( "FF D0 A1 ?? ?? ?? ?? B9 ?? ?? ?? ?? D9 1D ?? ?? ?? ?? FF 50 34 85 C0 74 22 8B 0D ?? ?? ?? ??" ) ) + 0x29 );

	if( _ReturnAddress( ) == ( void* )return_to_extrapolation )
		return true;

	return g_hooks.m_engine.GetOldMethod< IsPaused_t >( IVEngineClient::ISPAUSED )( this );
}