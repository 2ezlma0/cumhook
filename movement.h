#pragma once

class Movement {
public:
	float  m_speed;
	float  m_ideal;
	float  m_ideal2;
	vec3_t m_mins;
	vec3_t m_maxs;
	vec3_t m_origin;
	float  m_switch_value = 1.f;
	int    m_strafe_index;
	float  m_old_yaw;
	float  m_circle_yaw;
	bool   m_invert;
	vec3_t start_position = vec3_t( 0.0f, 0.0f, 0.0f );
	bool   fired_shot = false;

public:
	void JumpRelated( );
	void StrafeDefault( );
	void Strafe( );
	void DoPrespeed( );
	bool GetClosestPlane( vec3_t& plane );
	bool WillCollide( float time, float step );
	void FixMove( CUserCmd* cmd, const ang_t& old_angles );
	void AutoPeek( CUserCmd* cmd, float wish_yaw );
	void EdgeJump( int pre_flags, int post_flags );
	void FastStop( );
	void FakeWalk( );
	void StopMovement( );
	void QuickStop( );
	void AutoStop( );
};

extern Movement g_movement;