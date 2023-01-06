#pragma once

// pre-declare.
class LagRecord;

class BackupRecord {
public:
	matrix3x4_t m_bones[ 128 ];
	int        m_bones_count;
	vec3_t     m_origin, m_abs_origin;
	vec3_t     m_mins;
	vec3_t     m_maxs;
	ang_t      m_abs_ang;

public:
	__forceinline void store( Player* player ) {
		// get bone cache ptr.
		CBoneCache* cache = &player->m_BoneCache( );

		// store bone data.
		m_bones_count = cache->m_CachedBoneCount;
		memcpy( m_bones, cache->m_pCachedBones, sizeof( matrix3x4_t ) * m_bones_count );

		m_origin = player->m_vecOrigin( );
		m_mins = player->m_vecMins( );
		m_maxs = player->m_vecMaxs( );
		m_abs_origin = player->GetAbsOrigin( );
		m_abs_ang = player->GetAbsAngles( );
	}

	__forceinline void restore( Player* player ) {
		// get bone cache ptr.
		CBoneCache* cache = &player->m_BoneCache( );

		memcpy( cache->m_pCachedBones, m_bones, m_bones_count * sizeof( matrix3x4_t ) );
		cache->m_CachedBoneCount = m_bones_count;

		player->m_vecOrigin( ) = m_origin;
		player->set_collision_bounds( m_mins, m_maxs );
		player->SetAbsAngles( m_abs_ang );
		player->SetAbsOrigin( m_abs_origin );
	}
};

class LagRecord {
public:
	// data.
	matrix3x4_t m_bones[ 128 ];
	matrix3x4_t m_render_bones[ 128 ];
	CCSGOPlayerAnimState animstate;
	C_AnimationLayer m_layers[ 13 ];
	float            m_poses[ 24 ];

	Player* m_player;
	float   m_immune;
	int     m_tick;
	int     m_lag;
	int     m_flChokeTime;
	bool    m_dormant;
	bool    m_invalid;
	bool    m_can_aim;

	// netvars.
	float  m_sim_time;
	float  m_old_sim_time;
	int    m_flags;
	vec3_t m_origin;
	vec3_t m_abs_origin;
	vec3_t m_render_origin;
	vec3_t m_velocity;
	vec3_t m_abs_velocity;
	vec3_t m_mins;
	vec3_t m_maxs;
	ang_t  m_eye_angles;
	ang_t  m_abs_ang;
	float  m_body;
	float  m_feet_yaw;
	float  m_duck;

	// anim stuff.
	vec3_t           m_anim_velocity;

	// bone stuff.
	bool       m_setup;
	int m_bones_count;

	// lagfix stuff.
	bool   m_broke_lc;
	vec3_t m_pred_origin;
	vec3_t m_pred_velocity;
	float  m_pred_time;
	int    m_pred_flags;

	// resolver stuff.
	size_t m_mode;
	bool   m_fake_flick;
	bool   m_fake_walk;
	int    m_shot;
	bool   m_body_flick;
	float  m_away;
	float  m_anim_time;
	float  m_accel_time;
	std::string m_resolver_mode;

public:

	// default ctor.
	__forceinline LagRecord( ) :
		m_setup{ false },
		m_broke_lc{ false },
		m_invalid{ false },
		m_fake_walk{ false },
		m_shot{ false },
		m_lag{},
		m_flChokeTime{},
		m_can_aim{ 0 } {}

	// ctor.
	__forceinline LagRecord( Player* player ) :
		m_setup{ false },
		m_broke_lc{ false },
		m_invalid{ false },
		m_fake_walk{ false },
		m_shot{ false },
		m_lag{},
		m_flChokeTime{},
		m_can_aim{ 0 } {

		store( player );
	}

	// dtor.
	__forceinline ~LagRecord( ) {
		// free heap allocated game mem.
		//g_csgo.m_mem_alloc->Free( m_bones );
	}

	__forceinline void invalidate( ) {
		m_can_aim = false;

		// allocate new memory.
		//m_bones = ( BoneArray* )g_csgo.m_mem_alloc->Alloc( sizeof( BoneArray ) * 128 );
		m_bones_count = 0;
	}

	// function: allocates memory for SetupBones and stores relevant data.
	void store( Player* player ) {
		// allocate game heap.
		//m_bones = ( BoneArray* )g_csgo.m_mem_alloc->Alloc( sizeof( BoneArray ) * 128 );

		// player data.
		m_player = player;
		m_immune = player->m_fImmuneToGunGameDamageTime( );
		m_tick = g_csgo.m_cl->m_server_tick;

		// netvars.
		m_pred_time = m_sim_time = player->m_flSimulationTime( );
		m_old_sim_time = player->m_flOldSimulationTime( );
		m_pred_flags = m_flags = player->m_fFlags( );
		m_pred_origin = m_origin = player->m_vecOrigin( );
		m_abs_origin = player->GetAbsOrigin( );
		m_render_origin = player->m_vecOrigin( );
		m_eye_angles = player->m_angEyeAngles( );
		m_abs_ang = player->GetAbsAngles( );
		m_body = player->m_flLowerBodyYawTarget( );
		m_mins = player->m_vecMins( );
		m_maxs = player->m_vecMaxs( );
		m_duck = player->m_flDuckAmount( );
		m_abs_velocity = player->m_vecAbsVelocity( );
		m_pred_velocity = m_velocity = player->m_vecVelocity( );
		m_accel_time = player->m_flGroundAccelLinearFracLastTime( );

		// save networked animlayers.
		player->GetAnimLayers( m_layers );

		// normalize eye angles.
		m_eye_angles.normalize( );
		math::clamp( m_eye_angles.x, -90.f, 90.f );

		// get lag.
		m_lag = game::TIME_TO_TICKS( m_sim_time - m_old_sim_time );
		m_flChokeTime = game::TICKS_TO_TIME( m_lag );

		// compute animtime.
		m_anim_time = m_old_sim_time + g_csgo.m_globals->m_interval;
		m_bones_count = 0;
	}

	// function: restores 'predicted' variables to their original.
	__forceinline void predict( ) {
		m_invalid = false;
		m_pred_origin = m_origin;
		m_pred_velocity = m_velocity;
		m_pred_time = m_sim_time;
		m_pred_flags = m_flags;
	}

	// function: writes current record to bone cache.
	__forceinline void cache( ) {
		// get bone cache ptr.
		CBoneCache* cache = &m_player->m_BoneCache( );

		memcpy( cache->m_pCachedBones, m_bones, m_bones_count * sizeof( matrix3x4_t ) );
		cache->m_CachedBoneCount = m_bones_count;

		m_player->m_vecOrigin( ) = m_pred_origin;
		//m_player->m_vecMins( )   = m_mins;
		//m_player->m_vecMaxs( )   = m_maxs;
		m_player->set_collision_bounds( m_mins, m_maxs );

		m_player->SetAbsAngles( m_abs_ang );
		m_player->SetAbsOrigin( m_pred_origin );
	}

	__forceinline bool dormant( ) {
		return m_dormant;
	}

	__forceinline bool immune( ) {
		return m_immune > 0.f;
	}

	// function: checks if LagRecord obj is hittable if we were to fire at it now.
	bool valid( ) {
		if( !m_can_aim )
			return false;

		if( m_invalid )
			return false;

		if (dormant())
			return false;


		if (immune())
			return false;

		if (m_broke_lc)
			return false;

		auto net_channel_info = g_csgo.m_engine->GetNetChannelInfo( );
		if( !net_channel_info )
			return false;

		auto outgoing = g_cl.m_latency;
		auto incoming = net_channel_info->GetLatency( INetChannel::FLOW_INCOMING );

		auto correct = std::clamp( outgoing + incoming + g_cl.m_lerp, 0.0f, g_csgo.sv_maxunlag->GetFloat( ) );

		auto curtime = g_cl.m_local->alive( ) ? game::TICKS_TO_TIME( g_cl.m_local->m_nTickBase( ) ) : g_csgo.m_globals->m_curtime; //-V807
		auto delta_time = correct - ( curtime - m_sim_time );

		if( fabs( delta_time ) > 0.2f )
			return false;

		return true;
	}
};