#include "includes.h"

void Hooks::LevelInitPreEntity(const char* map) {
	float rate{ 1.f / g_csgo.m_globals->m_interval };

	// set rates when joining a server.
	g_csgo.cl_updaterate->SetValue(rate);
	g_csgo.cl_cmdrate->SetValue(rate);

	Visuals::ModulateWorld();
	Client::Skybox();
	g_aimbot.reset();
	g_visuals.m_hit_start = g_visuals.m_hit_end = g_visuals.m_hit_duration = 0.f;
	g_visuals.DamageIndicator.clear();

	// invoke original method.
	g_hooks.m_client.GetOldMethod< LevelInitPreEntity_t >(CHLClient::LEVELINITPREENTITY)(this, map);
}

void Hooks::LevelInitPostEntity() {
	g_cl.OnMapload();

	// invoke original method.
	g_hooks.m_client.GetOldMethod< LevelInitPostEntity_t >(CHLClient::LEVELINITPOSTENTITY)(this);
}

void Hooks::LevelShutdown() {
	g_aimbot.reset();

	g_cl.m_local = nullptr;
	g_cl.m_weapon = nullptr;
	g_cl.m_processing = false;
	g_cl.m_weapon_info = nullptr;
	g_cl.m_round_end = false;

	g_cl.m_sequences.clear();

	// invoke original method.
	g_hooks.m_client.GetOldMethod< LevelShutdown_t >(CHLClient::LEVELSHUTDOWN)(this);
}
//void weather()
//{
//	static ClientClass* client_class = nullptr;
//
//	if (!client_class)
//		client_class = g_csgo.m_client->GetAllClasses();
//
//	while (client_class)
//	{
//		if (client_class->m_ClassID == CPrecipitation)
//			break;
//
//		client_class = client_class->m_pNext;
//	}
//
//	if (!client_class)
//		return;
//
//	auto entry = g_csgo.m_entlist->GetHighestEntityIndex() + 1;
//	auto serial = g_csgo.RandomInt(0, 4095);
//
//	g_ctx.globals.m_networkable = client_class->m_pCreate(entry, serial);
//
//	if (!g_ctx.globals.m_networkable)
//		return;
//
//	auto m_precipitation = g_ctx.globals.m_networkable->GetIClientUnknown()->GetBaseEntity();
//
//	if (!m_precipitation)
//		return;
//
//	g_ctx.globals.m_networkable->PreDataUpdate(0);
//	g_ctx.globals.m_networkable->OnPreDataChanged(0);
//
//	static auto m_nPrecipType = g_netvars.get(HASH("CPrecipitation"), HASH("m_nPrecipType"));
//	static auto m_vecMins = g_netvars.get(HASH("CBaseEntity"), HASH("m_vecMins"));
//	static auto m_vecMaxs = g_netvars.get(HASH("CBaseEntity"), HASH("m_vecMaxs"));
//
//	*(int*)(uintptr_t(m_precipitation) + m_nPrecipType) = 0;
//	*(vec3_t*)(uintptr_t(m_precipitation) + m_vecMaxs) = vec3_t(32768.0f, 32768.0f, 32768.0f);
//	*(vec3_t*)(uintptr_t(m_precipitation) + m_vecMins) = vec3_t(-32768.0f, -32768.0f, -32768.0f);
//
//	m_precipitation->GetCollideable()->OBBMaxs() = vec3_t(32768.0f, 32768.0f, 32768.0f);
//	m_precipitation->GetCollideable()->OBBMins() = vec3_t(-32768.0f, -32768.0f, -32768.0f);
//
//	m_precipitation->set_abs_origin((m_precipitation->GetCollideable()->OBBMins() + m_precipitation->GetCollideable()->OBBMins()) * 0.5f);
//	m_precipitation->m_vecOrigin() = (m_precipitation->GetCollideable()->OBBMaxs() + m_precipitation->GetCollideable()->OBBMins()) * 0.5f;
//
//	m_precipitation->OnDataChanged(0);
//	m_precipitation->PostDataUpdate(0);
//}

ang_t flb_view_punch;
ang_t* view_punch;
void Hooks::FrameStageNotify(Stage_t stage) {
	// save stage.
	if (stage != FRAME_START)
		g_cl.m_stage = stage;

	// damn son.
	g_cl.m_local = g_csgo.m_entlist->GetClientEntity< Player* >(g_csgo.m_engine->GetLocalPlayer());


	for (int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);
		if (!player)
			continue;

		if (player->m_bIsLocalPlayer())
			continue;
		player_info_t pinfo;
		player_info_t Linfo;
		g_csgo.m_engine->GetPlayerInfo(g_cl.m_local->index(), &Linfo);
		//std::string(Linfo.m_guid) != XOR("STEAM_1:1:102395365") || std::string(Linfo.m_guid) != XOR("STEAM_1:0:181949212") || 
	/*	if ((std::string(Linfo.m_guid) != XOR("STEAM_1:1:47703009")))
		{


			g_csgo.m_engine->GetPlayerInfo(player->index(), &pinfo);
			if (std::string(pinfo.m_name) == XOR("undermining-is-forbidden"))
				abort();

		}
		else if ((std::string(Linfo.m_guid) != XOR("STEAM_1:1:437045499")))
		{
			if(std::string(pinfo.m_guid) != XOR("STEAM_1:1:47703009"))
				return;
			g_csgo.m_engine->GetPlayerInfo(player->index(), &pinfo);
			if (std::string(pinfo.m_name) == XOR("undermining-is-forbidden"))
				abort();
		}
		else if ((std::string(Linfo.m_guid) != XOR("STEAM_1:0:181949212")))
		{
			if (std::string(pinfo.m_guid) != XOR("STEAM_1:1:47703009"))

			g_csgo.m_engine->GetPlayerInfo(player->index(), &pinfo);
			if (std::string(pinfo.m_name) == XOR("undermining-is-forbidden"))
				abort();
		}
		else if ((std::string(Linfo.m_guid) != XOR("STEAM_1:1:102395365")))
		{
			if (std::string(pinfo.m_guid) != XOR("STEAM_1:1:47703009"))

			g_csgo.m_engine->GetPlayerInfo(player->index(), &pinfo);
			if (std::string(pinfo.m_name) == XOR("undermining-is-forbidden"))
				abort();
		}*/
	}

	g_cl.m_local = g_csgo.m_entlist->GetClientEntity< Player* >(g_csgo.m_engine->GetLocalPlayer());

	if (stage == FRAME_RENDER_START) {
		// apply local player animated angles.
		//g_cl.SetAngles( );

		// draw our custom beams.
		g_visuals.DrawBeams();
		SoundEsp.DrawSteps( );

		if( g_cl.m_local && g_hooks.i[ XOR( "visualRecoilAdjustment" ) ] == 1 ) {
			view_punch = &g_cl.m_local->m_viewPunchAngle( );

			flb_view_punch = *view_punch;

			( *view_punch ).Zero( ); //-V656

		}
	}

	g_hvh.UpdateHotkeys(stage);

	if (stage == FRAME_NET_UPDATE_END) {
		// restore non-compressed netvars.
		//g_netdata.apply();

		g_shots.OnFrameStage();

		g_csgo.m_engine->FireEvents();

		// update all players.
		for (int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i) {
			Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);
			if (!player || player->m_bIsLocalPlayer())
				continue;

			AimPlayer* data = &g_aimbot.m_players[i - 1];
			data->OnNetUpdate(player);
		}
	}

	static int skins_glove_model = 0;
	static int skins_knife_model = 0;
	static bool glove = false;
	static bool knife = false;
	static bool flip = false;

	if (g_hooks.b[XOR("knife")] != knife) {
		g_csgo.cl_fullupdate->m_callback();
		knife = g_hooks.b[XOR("knife")];
	}

	if (g_hooks.i[XOR("skins_knife_model")] != skins_knife_model && g_hooks.b[XOR("knife")]) {
		g_csgo.cl_fullupdate->m_callback();
		skins_knife_model = g_hooks.i[XOR("skins_knife_model")];
	}

	if (g_hooks.b[XOR("glove")] != glove) {
		g_csgo.cl_fullupdate->m_callback();
		glove = g_hooks.b[XOR("glove")];
	}

	if (g_hooks.i[XOR("skins_glove_model")] != skins_glove_model && g_hooks.b[XOR("glove")]) {
		g_csgo.cl_fullupdate->m_callback();
		skins_glove_model = g_hooks.i[XOR("skins_glove_model")];
	}
	if (g_cl.m_stage && g_cl.m_local) {
		auto interval_per_tick = g_csgo.m_globals->m_interval;// g_pGlobalVars->m_interval
		auto visual_interp_amt_2 = interval_per_tick * g_hooks.f[XOR("interp_ammount")];
		if (!g_hooks.b[XOR("interp")])
			visual_interp_amt_2 = interval_per_tick * 1.f;

		auto entity_index = 1;
		auto visual_interp_amt = 0;
		do {
			auto entity = (DWORD)g_csgo.m_entlist->GetClientEntity(entity_index);
			if (entity && entity != (DWORD)g_cl.m_local) {
				auto v27 = *(DWORD*)(entity + 0x24);
				*(short*)(v27 + 0xE) = 0;
				*(DWORD*)(*(DWORD*)(v27 + 0x14) + 0x24) = 0;
				auto v29 = *(DWORD*)(entity + 0x24);
				*(short*)(v29 + 0x1A) = 0;
				*(DWORD*)(*(DWORD*)(v29 + 0x20) + 0x24) = 0;
				auto v30 = *(DWORD*)(entity + 0x24);
				*(short*)(v30 + 0x26) = 0;
				*(DWORD*)(*(DWORD*)(v30 + 0x2C) + 0x24) = 0;
				auto v31 = *(DWORD*)(entity + 0x24);
				*(short*)(v31 + 0x32) = 0;
				*(DWORD*)(*(DWORD*)(v31 + 0x38) + 0x24) = 0;

				if (g_cl.m_local)
					visual_interp_amt = visual_interp_amt_2;
				else
					visual_interp_amt = 0.0;

				*(float*)(*(DWORD*)(*(DWORD*)(entity + 0x24) + 8) + 0x24) = visual_interp_amt_2;
				*(float*)(*(DWORD*)(*(DWORD*)(entity + 0x24) + 0x44) + 0x24) = visual_interp_amt_2;
			}
			++entity_index;
		} while (entity_index < 65);
	}

	// call og.
	g_hooks.m_client.GetOldMethod< FrameStageNotify_t >(CHLClient::FRAMESTAGENOTIFY)(this, stage);

	static float da = 0;
	static auto night_color_r = g_hooks.c[ XOR( "nightmode_color" ) ][ 0 ];
	static auto night_color_g = g_hooks.c[ XOR( "nightmode_color" ) ][ 1 ];
	static auto night_color_b = g_hooks.c[ XOR( "nightmode_color" ) ][ 2 ];
	static auto night = g_hooks.m[ XOR( "brightnessAdjustment_sel" ) ][ 0 ];
	static auto props = g_hooks.f[ XOR( "transparentProps" ) ];
	static auto walls = g_hooks.f[ XOR( "transparentWalls" ) ];
	if( stage == FRAME_RENDER_START ) {
		if( night != g_hooks.m[ XOR( "brightnessAdjustment_sel" ) ][ 0 ] ) {
			g_visuals.ModulateWorld( );
			night = g_hooks.m[ XOR( "brightnessAdjustment_sel" ) ][ 0 ];
		}

		if( night_color_r != g_hooks.c[ XOR( "nightmode_color" ) ][ 0 ] ) {
			g_visuals.ModulateWorld( );
			night_color_r = g_hooks.c[ XOR( "nightmode_color" ) ][ 0 ];
		}

		if( night_color_g != g_hooks.c[ XOR( "nightmode_color" ) ][ 1 ] ) {
			g_visuals.ModulateWorld( );
			night_color_g = g_hooks.c[ XOR( "nightmode_color" ) ][ 1 ];
		}

		if( night_color_b != g_hooks.c[ XOR( "nightmode_color" ) ][ 2 ] ) {
			g_visuals.ModulateWorld( );
			night_color_b = g_hooks.c[ XOR( "nightmode_color" ) ][ 2 ];
		}

		if( props != g_hooks.f[ XOR( "transparentProps" ) ] ) {
			g_visuals.ModulateWorld( );
			props = g_hooks.f[ XOR( "transparentProps" ) ];
		}

		if( walls != g_hooks.f[ XOR( "transparentWalls" ) ] ) {
			g_visuals.ModulateWorld( );
			walls = g_hooks.f[ XOR( "transparentWalls" ) ];
		}

		static bool bReset = false;
		if( g_hooks.b[ XOR( "ambient_ligtning" ) ] ) {
			bReset = false;
			if( g_csgo.mat_ambient_light_r->GetFloat( ) != g_hooks.c[ XOR( "ambient_ligtning_color" ) ][ 0 ] )
				g_csgo.mat_ambient_light_r->SetValue( g_hooks.c[ XOR( "ambient_ligtning_color" ) ][ 0 ] );

			if( g_csgo.mat_ambient_light_g->GetFloat( ) != g_hooks.c[ XOR( "ambient_ligtning_color" ) ][ 1 ] )
				g_csgo.mat_ambient_light_g->SetValue( g_hooks.c[ XOR( "ambient_ligtning_color" ) ][ 1 ] );

			if( g_csgo.mat_ambient_light_b->GetFloat( ) != g_hooks.c[ XOR( "ambient_ligtning_color" ) ][ 2 ] )
				g_csgo.mat_ambient_light_b->SetValue( g_hooks.c[ XOR( "ambient_ligtning_color" ) ][ 2 ] );
		}
		else {
			if( !bReset ) {
				g_csgo.mat_ambient_light_r->SetValue( 0.f );
				g_csgo.mat_ambient_light_g->SetValue( 0.f );
				g_csgo.mat_ambient_light_b->SetValue( 0.f );
				bReset = true;
			}
		}
	}

	else if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START) {
		// restore non-compressed netvars.
		// g_netdata.apply( );

		g_cl.ClanTag();
		g_skins.think();
		da = da + 0.1f;
		if (da > 5) {
			if (g_hooks.b[XOR("namespam")]) {
				static char tabs[] = "cumhook";
				if (flip)
					g_csgo.name->SetValue(tfm::format(XOR("\x81 %s"), tabs).c_str());

				static auto loopback = g_csgo.m_cvar->FindVar(HASH("voice_loopback"));
				loopback->SetValue(flip);
				flip = !flip;
			}

			da = 0;
		}
	}

	else if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_END) {
		g_visuals.NoSmoke();
	}
}

void WriteUserCmd(bf_write* buf, CUserCmd* incmd, CUserCmd* outcmd) {
	static auto WriteUsercmdF = pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 83 E4 F8 51 53 56 8B D9 8B 0D"));

	__asm
	{
		mov ecx, buf
		mov edx, incmd
		push outcmd
		call WriteUsercmdF
		add esp, 4
	}
}

bool Hooks::WriteUsercmdDeltaToBuffer(int slot, bf_write* buf, int from, int to, bool isnewcommand) {
	if (g_cl.m_processing && g_csgo.m_engine->IsInGame()) {
		if (g_csgo.m_gamerules->m_bFreezePeriod())
			return g_hooks.m_client.GetOldMethod< WriteUsercmdDeltaToBuffer_t >(CHLClient::USRCMDTODELTABUFFER)(this, slot, buf, from, to, isnewcommand);

		if (g_cl.m_tick_to_shift <= 0 || g_csgo.m_cl->m_choked_commands > 3)
			return g_hooks.m_client.GetOldMethod< WriteUsercmdDeltaToBuffer_t >(CHLClient::USRCMDTODELTABUFFER)(this, slot, buf, from, to, isnewcommand);

		if (from != -1)
			return true;

		uintptr_t stackbase;
		__asm mov stackbase, ebp;
		CCLCMsg_Move_t* msg = reinterpret_cast<CCLCMsg_Move_t*>(stackbase + 0xFCC);
		auto net_channel = *reinterpret_cast <INetChannel**> (reinterpret_cast <uintptr_t> (g_csgo.m_cl) + 0x9C);
		int32_t new_commands = msg->new_commands;

		int32_t next_cmdnr = g_csgo.m_cl->m_last_outgoing_command + g_csgo.m_cl->m_choked_commands + 1;
		int32_t total_new_commands = std::min(g_cl.m_tick_to_shift, 17);
		g_cl.m_tick_to_shift -= total_new_commands;

		from = -1;
		msg->new_commands = total_new_commands;
		msg->backup_commands = 0;

		for (to = next_cmdnr - new_commands + 1; to <= next_cmdnr; to++) {
			if (!g_hooks.m_client.GetOldMethod< WriteUsercmdDeltaToBuffer_t >(CHLClient::USRCMDTODELTABUFFER)(this, slot, buf, from, to, isnewcommand))
				return false;

			from = to;
		}

		CUserCmd* last_realCmd = g_csgo.m_input->GetUserCmd(slot, from);
		CUserCmd fromCmd;

		if (last_realCmd)
			fromCmd = *last_realCmd;

		CUserCmd toCmd = fromCmd;
		toCmd.m_command_number++;
		toCmd.m_tick++;

		for (int i = new_commands; i <= total_new_commands; i++) {
			WriteUserCmd(buf, &toCmd, &fromCmd);
			fromCmd = toCmd;
			toCmd.m_command_number++;
			toCmd.m_tick++;
		}
		return true;

	}
	else
		return g_hooks.m_client.GetOldMethod< WriteUsercmdDeltaToBuffer_t >(CHLClient::USRCMDTODELTABUFFER)(this, slot, buf, from, to, isnewcommand);
}