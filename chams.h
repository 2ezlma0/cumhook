#pragma once

class Chams {
public:
	enum model_type_t : uint32_t {
		invalid = 0,
		player,
		weapon,
		arms,
		view_weapon
	};

public:
	void SetColor( Color col, IMaterial* mat = nullptr );
	void SetAlpha( float alpha, IMaterial* mat = nullptr );
	void SetupMaterial( IMaterial* mat, Color col, bool z_flag );

	void init( );
	bool GenerateLerpedMatrix( int index, matrix3x4_t* out );
	void RenderHistoryChams( int index );
	void DrawChams( void* ecx, uintptr_t ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bone );
	void RenderFake( );
	void SceneEnd( );
	bool IsInViewPlane( const vec3_t& world );
	bool OverridePlayer( int index );
	bool SortPlayers( );

public:
	std::vector< Player* > m_players;
	matrix3x4_t fakematrix[ 128 ];
	bool m_running;
	IMaterial* debugambientcube;
	IMaterial* debugdrawflat;
	IMaterial* materialMetall;
	IMaterial* materialMetall2;
	IMaterial* materialMetall3;
	IMaterial* materialMetallnZ;
	IMaterial* skeet;
	IMaterial* onetap;
	IMaterial* yeti;
	IMaterial* glow;
};

extern Chams g_chams;