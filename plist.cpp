#include "includes.h"

std::vector<player_info_t> c_playerlist::get_players() {
	std::vector<player_info_t> players;

	if (!g_csgo.m_engine->IsInGame())
		return players;

	for (int i = 0; i < 65; i++) {
		auto e = g_csgo.m_entlist->GetClientEntity(i);

		if (!e)
			continue;

		if (e == g_cl.m_local)
			continue;

		if (e->m_iTeamNum() == g_cl.m_local->m_iTeamNum() && !g_hooks.b[XOR("plist_everyone")])
			continue;

		player_info_t pinfo;
		g_csgo.m_engine->GetPlayerInfo(i, &pinfo);

		if (std::string(pinfo.m_guid) == XOR(""))
			continue;

		players.push_back(pinfo);
	}

	return players;
}

bool c_playerlist::should_disable_resolver(Player* ent) {
	player_info_t pinfo;
	player_info_t Linfo;
	g_csgo.m_engine->GetPlayerInfo(g_cl.m_local->index(), &Linfo);
	g_csgo.m_engine->GetPlayerInfo(ent->index(), &pinfo);
	//if ((std::string(Linfo.m_guid) != XOR("STEAM_1:1:47703009")))
	//{
	//	if (std::string(pinfo.m_guid) == XOR("STEAM_1:1:102395365")) // ilum1nate
	//		return true;

	//	if (std::string(pinfo.m_guid) == XOR("STEAM_1:1:47703009")) // Arrigo
	//		return true;

	//	if (std::string(pinfo.m_guid) == XOR("STEAM_1:0:181949212")) // lexa
	//		return true;
	//	if (std::string(pinfo.m_guid) == XOR("STEAM_1:1:437045499")) // andrey
	//		return true;
	//}
	////else if ((std::string(Linfo.m_guid) != XOR("STEAM_1:1:102395365")))
	////{

	////	if (std::string(pinfo.m_guid) == XOR("STEAM_1:1:102395365")) // ilum1nate
	////		return true;

	////	if (std::string(pinfo.m_guid) == XOR("STEAM_1:1:47703009")) // Arrigo
	////		return true;

	////	if (std::string(pinfo.m_guid) == XOR("STEAM_1:0:181949212")) // lexa
	////		return true;
	////	if (std::string(pinfo.m_guid) == XOR("STEAM_1:1:437045499")) // andrey
	////		return true;

	////}
	////else if ((std::string(Linfo.m_guid) != XOR("STEAM_1:1:47703009")))
	////{

	////	if (std::string(pinfo.m_guid) == XOR("STEAM_1:1:102395365")) // ilum1nate
	////		return true;

	////	if (std::string(pinfo.m_guid) == XOR("STEAM_1:1:47703009")) // Arrigo
	////		return true;

	////	if (std::string(pinfo.m_guid) == XOR("STEAM_1:0:181949212")) // lexa
	////		return true;
	////	if (std::string(pinfo.m_guid) == XOR("STEAM_1:1:437045499")) // andrey
	////		return true;

	////}
	////else if ((std::string(Linfo.m_guid) != XOR("STEAM_1:1:47703009")))
	////{

	////	if (std::string(pinfo.m_guid) == XOR("STEAM_1:1:102395365")) // ilum1nate
	////		return true;

	////	if (std::string(pinfo.m_guid) == XOR("STEAM_1:1:47703009")) // Arrigo
	////		return true;

	////	if (std::string(pinfo.m_guid) == XOR("STEAM_1:0:181949212")) // lexa
	////		return true;
	////	if (std::string(pinfo.m_guid) == XOR("STEAM_1:1:437045499")) // andrey
	////		return true;

	////}
	auto key = XOR("plist_") + std::string(pinfo.m_fake_player ? pinfo.m_name : pinfo.m_guid) + XOR("_");
	auto cfg = g_hooks.pb;

	return cfg[key + XOR("noresolver")];
}

bool c_playerlist::should_disable_aimbot(Player* ent) {
	player_info_t pinfo;
	g_csgo.m_engine->GetPlayerInfo(ent->index(), &pinfo);

	auto key = XOR("plist_") + std::string(pinfo.m_fake_player ? pinfo.m_name : pinfo.m_guid) + XOR("_");
	auto cfg = g_hooks.pb;

	return cfg[key + XOR("friend")];
}

bool c_playerlist::should_disable_visuals(Player* ent) {
	player_info_t pinfo;
	g_csgo.m_engine->GetPlayerInfo(ent->index(), &pinfo);

	auto key = XOR("plist_") + std::string(pinfo.m_fake_player ? pinfo.m_name : pinfo.m_guid) + XOR("_");
	auto cfg = g_hooks.pb;

	return cfg[key + XOR("novis")];
}

bool c_playerlist::should_prefer_body(Player* ent) {
	player_info_t pinfo;
	g_csgo.m_engine->GetPlayerInfo(ent->index(), &pinfo);

	auto key = XOR("plist_") + std::string(pinfo.m_fake_player ? pinfo.m_name : pinfo.m_guid) + XOR("_");
	auto cfg = g_hooks.pb;

	return cfg[key + XOR("baim")];
}

void c_playerlist::apply_for_player(Player* ent, LagRecord* record) {
	player_info_t pinfo;
	g_csgo.m_engine->GetPlayerInfo(ent->index(), &pinfo);

	if (pinfo.m_guid == std::string(""))
		return;

	auto key = XOR("plist_") + std::string(pinfo.m_fake_player ? pinfo.m_name : pinfo.m_guid) + XOR("_");

	if (g_hooks.pb[key + XOR("pitch")]) {
		switch (g_hooks.pi[key + XOR("pitchtype")]) {
		case 0:
			record->m_eye_angles.x = 89.f; // Down
			break;
		case 1:
			record->m_eye_angles.x = -89.f; // Up
			break;
		case 2:
			record->m_eye_angles.x = g_csgo.RandomFloat(-89.f, 89.f); // Random
			break;
		case 3:
			record->m_eye_angles.x = g_hooks.pf[key + XOR("pitchval")]; // Custom
			break;
		}
	}

	AimPlayer* data = &g_aimbot.m_players[ent->index() - 1];
	if (!data || data->m_records.empty())
		return;

	// pointer for easy access.
	LagRecord* move = &data->m_walk_record;

	if (g_hooks.pb[key + XOR("yaw")] && record->m_mode == Resolver::Modes::RESOLVE_STAND && !g_hooks.is_key_down(g_hooks.i[XOR("override_key")])) {
		switch (g_hooks.pi[key + XOR("yawmode")]) {
		case 0:
			record->m_eye_angles.y = data->m_body; // LBY
			break;
		case 1:
			record->m_eye_angles.y = move->m_body; // Last move lby
			break;
		case 2:
			switch (g_hooks.pi[key + XOR("yawtype")]) { // Custom
			case 0:
				record->m_eye_angles.y += g_hooks.pf[key + XOR("yawval")]; // add
				break;
			case 1:
				record->m_eye_angles.y = g_hooks.pf[key + XOR("yawval")]; // set
				break;
			case 2:
				record->m_eye_angles.y -= g_hooks.pf[key + XOR("yawval")]; // subtract
				break;
			}
			break;
		}
	}
}

std::string c_playerlist::get_steam_id(std::vector<player_info_t> players, int userid) {
	for (auto p : players) {
		if (p.m_user_id == userid) {
			if (p.m_fake_player)
				return p.m_name;

			return p.m_guid;
		}
	}

	return XOR("UNKNOWN");
}

std::string c_playerlist::get_skin() {
	if (callbacks::DEAGLE())
		return XOR("DEAGLE");
	else if (callbacks::GLOCK())
		return XOR("GLOCK");
	else if (callbacks::ELITE())
		return XOR("ELITE");
	else if (callbacks::FIVESEVEN())
		return XOR("FIVESEVEN");
	else if (callbacks::AK47())
		return XOR("AK47");
	else if (callbacks::AUG())
		return XOR("AUG");
	else if (callbacks::AWP())
		return XOR("AWP");
	else if (callbacks::FAMAS())
		return XOR("FAMAS");
	else if (callbacks::G3SG1())
		return XOR("G3SG1");
	else if (callbacks::GALIL())
		return XOR("GALIL");
	else if (callbacks::M249())
		return XOR("M249");
	else if (callbacks::M4A4())
		return XOR("M4A4");
	else if (callbacks::MAC10())
		return XOR("MAC10");
	else if (callbacks::P90())
		return XOR("P90");
	else if (callbacks::UMP45())
		return XOR("UMP45");
	else if (callbacks::XM1014())
		return XOR("XM1014");
	else if (callbacks::BIZON())
		return XOR("BIZON");
	else if (callbacks::MAG7())
		return XOR("MAG7");
	else if (callbacks::NEGEV())
		return XOR("NEGEV");
	else if (callbacks::SAWEDOFF())
		return XOR("SAWEDOFF");
	else if (callbacks::TEC9())
		return XOR("TEC9");
	else if (callbacks::P2000())
		return XOR("P2000");
	else if (callbacks::MP7())
		return XOR("MP7");
	else if (callbacks::MP9())
		return XOR("MP9");
	else if (callbacks::NOVA())
		return XOR("NOVA");
	else if (callbacks::P250())
		return XOR("P250");
	else if (callbacks::SCAR20())
		return XOR("SCAR20");
	else if (callbacks::SG553())
		return XOR("SG553");
	else if (callbacks::SSG08())
		return XOR("SSG08");
	else if (callbacks::M4A1S())
		return XOR("M4A1S");
	else if (callbacks::USPS())
		return XOR("USPS");
	else if (callbacks::CZ75A())
		return XOR("CZ75A");
	else if (callbacks::REVOLVER())
		return XOR("REVOLVER");
	else if (callbacks::KNIFE_BAYONET())
		return XOR("KNIFE_BAYONET");
	else if (callbacks::KNIFE_FLIP())
		return XOR("KNIFE_FLIP");
	else if (callbacks::KNIFE_GUT())
		return XOR("KNIFE_GUT");
	else if (callbacks::KNIFE_KARAMBIT())
		return XOR("KNIFE_KARAMBIT");
	else if (callbacks::KNIFE_M9_BAYONET())
		return XOR("KNIFE_M9_BAYONET");
	else if (callbacks::KNIFE_HUNTSMAN())
		return XOR("KNIFE_HUNTSMAN");
	else if (callbacks::KNIFE_FALCHION())
		return XOR("KNIFE_FALCHION");
	else if (callbacks::KNIFE_BOWIE())
		return XOR("KNIFE_BOWIE");
	else if (callbacks::KNIFE_BUTTERFLY())
		return XOR("KNIFE_BUTTERFLY");
	else if (callbacks::KNIFE_SHADOW_DAGGERS())
		return XOR("KNIFE_SHADOW_DAGGERS");

	return XOR("UNKNOWN");
}

int c_playerlist::skin_id() {

	auto skin = XOR("skin_") + c_playerlist::get()->get_skin() + XOR("_");
	auto cfg = g_hooks.pi;

	return cfg[skin + XOR("id")];
}