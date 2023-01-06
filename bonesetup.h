#pragma once

class Bones {

public:
	bool m_running;

public:
	bool BuildBones( Player* target, int mask, matrix3x4_t* out, LagRecord* record );
	bool setup(Player* player, matrix3x4_t* out, LagRecord* record);
};

extern Bones g_bones;