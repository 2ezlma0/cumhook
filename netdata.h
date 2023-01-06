#pragma once

class NetData {
private:
	class StoredData_t {
	public:
		int    m_tickbase;
		ang_t  m_punch;
		ang_t  m_punch_vel;
		vec3_t m_view_offset;
		float m_vel_mod;
		ang_t m_view_punch_angle;
	public:
		__forceinline StoredData_t() : m_tickbase{}, m_punch{}, m_punch_vel{}, m_view_offset{}, m_vel_mod{} {};
	};

	std::array< StoredData_t, MULTIPLAYER_BACKUP > m_data;

public:
	void store(CUserCmd* cmd);
	void apply(int m_tick);
	void reset( );
};

extern NetData g_netdata;