#include "includes.h"

ServerSound g_sound{ };;

bool valid( Player* player, bool check_team, bool check_dormant ) {
	if( !player )
		return false;

	if( !g_cl.m_local )
		return false;

	if( !player->IsPlayer( ) )
		return false;

	if( !player->alive( ) )
		return false;

	if( player->dormant( ) && check_dormant )
		return false;

	if( check_team && g_cl.m_local->m_iTeamNum( ) == player->m_iTeamNum( ) )
		return false;

	return true;
}

void ServerSound::Start( ) {
	m_utlCurSoundList.RemoveAll( );
	g_csgo.m_sound->GetActiveSounds( m_utlCurSoundList );

	// No active sounds.
	if( !m_utlCurSoundList.Count( ) )
		return;

	// Accumulate sounds for esp correction
	for( int iter = 0; iter < m_utlCurSoundList.Count( ); iter++ ) {
		SndInfo_t& sound = m_utlCurSoundList[ iter ];
		if( sound.m_nSoundSource < 1 || sound.m_nSoundSource > 64 )
			continue;

		if( sound.m_pOrigin->IsZero( ) )
			continue;

		if( !ValidSound( sound ) )
			continue;

		Player* player = ( Player* )g_csgo.m_entlist->GetClientEntity( sound.m_nSoundSource );

		if( !valid( player, true, false ) )
			continue;

		SetupAdjustPlayer( player, sound );

		m_cSoundPlayers[ sound.m_nSoundSource ].Override( sound );
	}

	//for ( int iter = 1; iter < g_csgo.m_entlist->GetHighestEntityIndex( ); iter++ ) {
	//	Player* player = ( Player* )g_csgo.m_entlist->GetClientEntity( iter );
	//	if ( !player || !player->dormant( ) || !player->alive( ) )
	//		continue;
	//
	//	AdjustPlayerBegin( player );
	//}

	m_utlvecSoundBuffer = m_utlCurSoundList;
}

void ServerSound::SetupAdjustPlayer( Player* player, SndInfo_t& sound ) {
	vec3_t src3D, dst3D;
	CGameTrace tr;
	CTraceFilterWorldOnly filter;

	//filter.SetPassEntity(  player  );
	src3D = ( *sound.m_pOrigin ) + vec3_t( 0, 0, 1 ); // So they dont dig into ground incase shit happens /shrug
	dst3D = src3D - vec3_t( 0, 0, 100 );

	g_csgo.m_engine_trace->TraceRay( Ray( src3D, dst3D ), MASK_PLAYERSOLID, &filter, &tr );

	// step = ( tr.fraction < 0.20 )
	// shot = ( tr.fraction > 0.20 )
	// stand = ( tr.fraction > 0.50 )
	// crouch = ( tr.fraction < 0.50 )

	/* Corrects origin and important flags. */

	// Player stuck, idk how this happened
	if( tr.m_allsolid )
		m_cSoundPlayers[ sound.m_nSoundSource ].m_iReceiveTime = -1;

	*sound.m_pOrigin = ( ( tr.m_fraction < 0.97 ) ? tr.m_endpos : *sound.m_pOrigin );
	m_cSoundPlayers[ sound.m_nSoundSource ].m_nFlags = player->m_fFlags( );
	m_cSoundPlayers[ sound.m_nSoundSource ].m_nFlags |= ( tr.m_fraction < 0.50f ? FL_DUCKING : 0 ) | ( tr.m_fraction != 1 ? FL_ONGROUND : 0 );   // Turn flags on
	m_cSoundPlayers[ sound.m_nSoundSource ].m_nFlags &= ( tr.m_fraction > 0.50f ? ~FL_DUCKING : 0 ) | ( tr.m_fraction == 1 ? ~FL_ONGROUND : 0 ); // Turn flags off
}

void ServerSound::Finish( ) {
	// Do any finishing code here. If we add smtn like sonar radar this will be useful.
	AdjustPlayerFinish( );
}

void ServerSound::AdjustPlayerFinish( ) {
	// Restore and clear saved players for next loop.
	for( auto& RestorePlayer : m_arRestorePlayers ) {
		auto player = RestorePlayer.player;
		if( !valid( player, true, false ) )
			continue;

		player->m_fFlags( ) = RestorePlayer.m_nFlags;
		player->m_vecOrigin( ) = RestorePlayer.m_vecOrigin;
		player->SetAbsOrigin( RestorePlayer.m_vecAbsOrigin );
		//*( bool* )( ( DWORD )player + 233 ) = RestorePlayer.m_bDormant; // dormant check
	}
	m_arRestorePlayers.clear( );
}

void ServerSound::AdjustPlayerBegin( Player* player ) {
	// Adjusts player's origin and other vars so we can show full-ish esp.
	constexpr int EXPIRE_DURATION = 450; // miliseconds-ish?
	auto& sound_player = m_cSoundPlayers[ player->index( ) ];
	bool sound_expired = g_csgo.m_globals->m_realtime - sound_player.m_iReceiveTime > EXPIRE_DURATION;
	if( sound_expired )
		return;

	SoundPlayer current_player;
	current_player.player = player;
	current_player.m_bDormant = true;
	current_player.m_nFlags = player->m_fFlags( );
	current_player.m_vecOrigin = player->m_vecOrigin( );
	current_player.m_vecAbsOrigin = player->GetAbsOrigin( );
	m_arRestorePlayers.emplace_back( current_player );

	//if ( !sound_expired )
	//	*( bool* )( ( DWORD )player + 233 ) = false; // dormant check
	player->m_fFlags( ) = sound_player.m_nFlags;
	player->m_vecOrigin( ) = sound_player.m_vecOrigin;
	player->SetAbsOrigin( sound_player.m_vecOrigin );

	if( g_hooks.b[ XOR( "vis_soundesp" ) ] )
		SoundEsp.AddStepManually( player->index( ), sound_player.m_vecOrigin );

	player->InvalidateBoneCache( );
	player->SetupBones( nullptr, 128, 0x100, player->m_flSimulationTime( ) );
}

bool ServerSound::ValidSound( SndInfo_t& sound ) {
	// Use only server dispatched sounds.
	if( !sound.m_bFromServer )
		return false;

	// We don't want the sound to keep following client's predicted origin.
	for( int iter = 0; iter < m_utlvecSoundBuffer.Count( ); iter++ ) {
		SndInfo_t& cached_sound = m_utlvecSoundBuffer[ iter ];
		if( cached_sound.m_nGuid == sound.m_nGuid )
			return false;
	}

	return true;
}