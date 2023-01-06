#pragma once

class ServerSound {
public:
	// Call before and after ESP.
	void Start( );
	void Finish( );

	void AdjustPlayerBegin( Player* player );
	void AdjustPlayerFinish( );
	void SetupAdjustPlayer( Player* player, SndInfo_t& sound );

	bool ValidSound( SndInfo_t& sound );

	struct SoundPlayer {
		void reset( bool store_data = false, const vec3_t& origin = vec3_t( 0, 0, 0 ), int flags = 0 ) {
			if( store_data ) {
				m_iReceiveTime = g_csgo.m_globals->m_realtime;
				m_vecOrigin = origin;
				m_nFlags = flags;
			}
			else {
				m_iReceiveTime = 0.0f;
				m_vecOrigin.Zero( );
				m_nFlags = 0;
			}
		}

		void Override( SndInfo_t& sound ) {
			m_iIndex = sound.m_nSoundSource;
			m_vecOrigin = *sound.m_pOrigin;
			m_iReceiveTime = g_csgo.m_globals->m_realtime;
		}

		int m_iIndex = 0;
		int m_iReceiveTime = 0;
		vec3_t m_vecOrigin = vec3_t( 0, 0, 0 );
		/* Restore data */
		int m_nFlags = 0;
		Player* player = nullptr;
		vec3_t m_vecAbsOrigin = vec3_t( 0, 0, 0 );
		bool m_bDormant = false;
	} m_cSoundPlayers[ 64 ];
	CUtlVector<SndInfo_t> m_utlvecSoundBuffer;
	CUtlVector<SndInfo_t> m_utlCurSoundList;
	std::vector<SoundPlayer> m_arRestorePlayers;
};

extern ServerSound g_sound;