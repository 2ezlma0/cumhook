#pragma once

class CMoveData {
public:
	bool        m_bFirstRunOfFunctions : 1;
	bool        m_bGameCodeMovedPlayer : 1;
	bool        m_bNoAirControl : 1;
	CBaseHandle m_nPlayerHandle;
	int         m_nImpulseCommand;
	ang_t       m_vecViewAngles;
	ang_t       m_vecAbsViewAngles;
	int         m_nButtons;
	int         m_nOldButtons;
	float       m_flForwardMove;
	float       m_flSideMove;
	float       m_flUpMove;
	float       m_flMaxSpeed;
	float       m_flClientMaxSpeed;
	vec3_t      m_vecVelocity;
	vec3_t      m_vecOldVelocity;
	float       m_unknown;
	ang_t       m_vecAngles;
	ang_t       m_vecOldAngles;
	float       m_outStepHeight;
	vec3_t      m_outWishVel;
	vec3_t      m_outJumpVel;
	vec3_t      m_vecConstraintCenter;
	float       m_flConstraintRadius;
	float       m_flConstraintWidth;
	float       m_flConstraintSpeedFactor;
	bool        m_bConstraintPastRadius;
	vec3_t      m_vecAbsOrigin;
};

class IMoveHelper {
public:
	// indexes for virtuals and hooks.
	enum indices : size_t {
		SETHOST = 1,
	};

	__forceinline void SetHost( Entity* host ) {
		return util::get_method< void( __thiscall* )( decltype( this ), Entity* ) >( this, SETHOST )( this, host );
	}
};

class CPrediction {
public:
	// indexes for virtuals and hooks.
	enum indices : size_t {
		UPDATE                  = 3,
        POSTNETWORKDATARECEIVED = 6,
		SETLOCALVIEWANGLES      = 13,
		INPREDICTION            = 14,
		RUNCOMMAND              = 19,
		SETUPMOVE               = 20,
		FINISHMOVE              = 21
	};

public:
	char pad00[8];                            // 0x0
	bool m_in_prediction;                     // 0x8
	bool m_old_cl_predict_value;              // 0x9
	bool m_engine_paused;                     // 0xA
	int m_previous_startframe;               // 0xC
	int m_incoming_packet_number;             // 0x10
	float m_last_server_world_time_stamp;     // 0x14
	bool    m_first_time_predicted;
	bool m_is_first_time_predicted;           // 0x18
	int m_commands_predicted;                 // 0x1C
	int m_server_commands_acknowledged;       // 0x20
	bool m_previous_ack_had_errors;           // 0x24

public:
	// virtual methods
	__forceinline void Update( int startframe, bool validframe, int incoming_acknowledged, int outgoing_command ) {
		return util::get_method< void( __thiscall* )( void*, int, bool, int, int ) >( this, UPDATE )( this, startframe, validframe, incoming_acknowledged, outgoing_command );
	}

	__forceinline void SetLocalViewAngles( const ang_t& ang ) {
		return util::get_method< void( __thiscall* )( decltype( this ), const ang_t& ) >( this, SETLOCALVIEWANGLES )( this, ang );
	}

	__forceinline void SetupMove( Entity* player, CUserCmd* cmd, IMoveHelper* helper, CMoveData* data ) {
		return util::get_method< void( __thiscall* )( decltype( this ), Entity*, CUserCmd*, IMoveHelper*, CMoveData* ) >( this, SETUPMOVE )( this, player, cmd, helper, data );
	}

	__forceinline void FinishMove( Entity* player, CUserCmd* cmd, CMoveData* data ) {
		return util::get_method< void( __thiscall* )( decltype( this ), Entity*, CUserCmd*, CMoveData* ) >( this, FINISHMOVE )( this, player, cmd, data );
	}
};

class CGameMovement {
public:
	// indexes for virtuals and hooks
	enum indices : size_t {
		PROCESSMOVEMENT             = 1,
		STARTTRACKPREDICTIONERRORS  = 3,
		FINISHTRACKPREDICTIONERRORS = 4,
		ONLAND                      = 32
	};

	__forceinline void ProcessMovement( Entity* player, CMoveData* data ) {
		return util::get_method< void( __thiscall* )( decltype( this ), Entity*, CMoveData* ) >( this, PROCESSMOVEMENT )( this, player, data );
	}

	__forceinline void StartTrackPredictionErrors( Entity* player ) {
		return util::get_method< void( __thiscall* )( decltype( this ), Entity* ) >( this, STARTTRACKPREDICTIONERRORS )( this, player );
	}

	__forceinline void FinishTrackPredictionErrors( Entity* player ) {
		return util::get_method< void( __thiscall* )( decltype( this ), Entity* ) >( this, FINISHTRACKPREDICTIONERRORS )( this, player );
	}
};