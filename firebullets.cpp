#include "includes.h"

class C_TEFireBullets {
public:
	PAD( 0xC );
	int		m_index;
	int     m_item_id;
	vec3_t	m_origin;
	ang_t	m_angles;
	int		m_weapon_id;
	int		m_mode;
	int		m_seed;
	float	m_spread;
};

void Hooks::PostDataUpdate( DataUpdateType_t type ) {
	C_TEFireBullets* shot = ( C_TEFireBullets* )this;
	
	/*if (shot->m_index < 65)
	{
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( shot->m_index + 1 );

		auto record = &g_aimbot.m_players[shot->m_index];

		if (record) {
			record->m_last_shot_time = player->m_flSimulationTime();
			record->m_last_shot_angles = shot->m_angles;
			record->m_last_shot_angles.x = math::NormalizedAngle(shot->m_angles.x);
			record->m_last_shot_origin = shot->m_origin;
			record->m_last_shot_tick = g_csgo.m_cl->m_server_tick;
		}
	}*/

	g_hooks.m_fire_bullets.GetOldMethod< PostDataUpdate_t >( 7 )( this, type );
}