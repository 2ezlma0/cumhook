#pragma once

namespace penetration {
	struct PenetrationInput_t {
		Player* m_from;
		Player* m_target;
		vec3_t  m_pos;
		float	m_damage;
		float   m_damage_pen;
		bool	m_can_pen;
	};

	struct PenetrationOutput_t {
		Player* m_target;
		float   m_damage;
		int     m_hitgroup;
		bool    m_pen;

        __forceinline PenetrationOutput_t() : m_target{ nullptr }, m_damage{ 0.f }, m_hitgroup{ -1 }, m_pen{ false } {}
	};

    float scale( Player* player, float damage, float armor_ratio, int hitgroup );
    bool  TraceToExit( const vec3_t& start, const vec3_t& dir, vec3_t& out, CGameTrace* enter_trace, CGameTrace* exit_trace );
	void  ClipTraceToPlayer( const vec3_t& start, const vec3_t& end, uint32_t mask, CGameTrace* tr, Player* player, float min );
    bool  runcustom( PenetrationInput_t* in, PenetrationOutput_t* out, vec3_t custom_origin);
	bool  run(PenetrationInput_t* in, PenetrationOutput_t* out);
	void  UTIL_TraceLineIgnoreTwoEntities(const vec3_t& vecAbsStart, const vec3_t& vecAbsEnd, uint32_t mask, Entity* ignore, Entity* ignore2, int collisionGroup, CGameTrace* ptr);
}