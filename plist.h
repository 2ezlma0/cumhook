#pragma once
#include "includes.h"

template <typename T>
class singleton {
public:
	static T* get( ) {
		static T* _inst = nullptr;

		if( !_inst )
			_inst = new T( );

		return _inst;
	}
};

class c_playerlist : public singleton<c_playerlist> {
public:
	bool should_disable_resolver( Player* ent );
	bool should_disable_aimbot( Player* ent );
	bool should_disable_visuals( Player* ent );
	bool should_prefer_body( Player* ent );

	void apply_for_player( Player* ent, LagRecord* record );

	std::vector<player_info_t> get_players( );
	std::string get_steam_id( std::vector<player_info_t> players, int userid );
	std::string get_skin( );
	int skin_id( );
};