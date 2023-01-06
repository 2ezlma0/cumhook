#pragma once

class Sequence {
public:
	float m_time;
	int   m_state;
	int   m_seq;

public:
	__forceinline Sequence( ) : m_time{}, m_state{}, m_seq{} {};
	__forceinline Sequence( float time, int state, int seq ) : m_time{ time }, m_state{ state }, m_seq{ seq } {};
};

class NetPos {
public:
	float  m_time;
	vec3_t m_pos;

public:
	__forceinline NetPos( ) : m_time{}, m_pos{} {};
	__forceinline NetPos( float time, vec3_t pos ) : m_time{ time }, m_pos{ pos } {};
};

class C_AnimationLayer {
public:
	float   m_anim_time;			// 0x0000
	float   m_fade_out_time;		// 0x0004
	int     m_flags;				// 0x0008
	int     m_activty;				// 0x000C
	int     m_priority;				// 0x0010
	int     m_order;				// 0x0014
	int     m_sequence;				// 0x0018
	float   m_prev_cycle;			// 0x001C
	float   m_weight;				// 0x0020
	float   m_weight_delta_rate;	// 0x0024
	float   m_playback_rate;		// 0x0028
	float   m_cycle;				// 0x002C
	Entity* m_owner;				// 0x0030
	int     m_bits;					// 0x0034
}; // size: 0x0038

class Client {
public:
	// hack thread.
	static ulong_t __stdcall init( void* arg );

	void StartMove( CUserCmd* cmd );
	void EndMove( CUserCmd* cmd );
	void BackupPlayers( bool restore );
	void DoMove( );
	void UpdateInformation( );
	void DrawHUD( );
	void SetAngles( );
	void UpdateAnimations( );
	static void UnlockHiddenConvars( );
	void KillFeed( );

	void OnPaint( );
	void OnMapload( );
	void OnTick( CUserCmd* cmd );

	void SetupEyePosition();

	// debugprint function.
	void print( const std::string text, ... );

	// check if we are able to fire this tick.
	bool CanFireWeapon( float curtime );
	bool IsFiring( float curtime );
	void UpdateRevolverCock( );
	void UpdateIncomingSequences( );

	void ClanTag( );
	static void Skybox( );

public:
	std::array<float, 24> m_poses;
	std::array<C_AnimationLayer, 13> m_layers;

	// local player variables.
	Player*  m_local;
	bool     m_processing;
	bool	m_updating_anims;
	bool	m_updating_bones;
	int      m_flags;
	vec3_t	 m_shoot_pos;
	bool     m_player_fire;
	bool	 m_shot;
	bool	 m_old_shot;
	float            m_abs_yaw;


	int	 m_tick_to_shift;
	int	 m_tick_to_recharge;
	bool m_charged;

	// active weapon variables.
	Weapon*     m_weapon;
	int         m_weapon_id;
	WeaponInfo* m_weapon_info;
	int         m_weapon_type;
	bool        m_weapon_fire;

	// revolver variables.
	int	 m_revolver_cock;
	int	 m_revolver_query;
	bool m_revolver_fire;

	// general game varaibles.
	bool     m_round_end;
	Stage_t	 m_stage;
	int	     m_max_lag;
	int      m_lag;
	int	     m_old_lag;
	bool*    m_packet;
	bool*    m_final_packet;
	bool	 m_old_packet;
	bool	 m_fake_walk;
	float	 m_lerp;
	float    m_latency;
	int      m_latency_ticks;
	int      m_server_tick;
	int      m_arrival_tick;
	int      m_width, m_height;

	// usercommand variables.
	CUserCmd* m_cmd;
	int	      m_tick;
	int	      m_buttons;
	int       m_old_buttons;
	ang_t     m_view_angles;
	ang_t	  m_strafe_angles;
	ang_t	  m_original_angles;
	vec3_t	  m_forward_dir;
	bool	  m_pressing_move;

    penetration::PenetrationOutput_t m_pen_data;

	std::deque< Sequence > m_sequences;
	std::deque< NetPos >   m_net_pos;

	// animation variables.
	ang_t  m_angle;
	ang_t  m_rotation;
	ang_t  m_radar;
	float  m_body;
	float  m_body_pred;
	float  m_speed;
	float  m_anim_time;
	float  m_anim_frame;
	bool   m_ground;
	bool   m_lagcomp;
	
	// hack username.
	std::string m_user;

	//other 
	float			 m_left_thickness[64], m_right_thickness[64], m_at_target_angle[64];
};

extern Client g_cl;