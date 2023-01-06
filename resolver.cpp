#include "includes.h"

Resolver g_resolver{};;

std::shared_ptr<LagRecord> Resolver::FindIdealRecord_(AimPlayer* data) {

	if (data->m_records.empty())
		return nullptr;

	for (const auto& it : data->m_records) {

		if (!it->valid())
			continue;

		// try to find a record with a lby update, or walking.
		if (it->m_mode == Modes::RESOLVE_BODY || it->m_mode == Modes::RESOLVE_WALK)
			return it;
	}

	return nullptr;
}

LagRecord* Resolver::FindIdealRecord(AimPlayer* data) {
	LagRecord* first_valid, * current;

	if (data->m_records.empty())
		return nullptr;

	first_valid = nullptr;

	// iterate records.
	for (const auto& it : data->m_records) {
		if (it->dormant() || it->immune() || !it->valid())
			continue;

		// get current record.
		current = it.get();

		// first record that was valid, store it for later.
		if (!first_valid)
			first_valid = current;

		// try to find a record with a shot, lby update, walking or no anti-aim.
		if (it->m_shot || it->m_mode == Modes::RESOLVE_BODY || it->m_mode == Modes::RESOLVE_WALK || it->m_mode == Modes::RESOLVE_NONE)
			return current;
	}

	// none found above, return the first valid record if possible.
	return (first_valid) ? first_valid : nullptr;
}

LagRecord* Resolver::FindLastRecord(AimPlayer* data) {
	LagRecord* current;

	if (data->m_records.empty())
		return nullptr;

	// iterate records in reverse.
	for (auto it = data->m_records.crbegin(); it != data->m_records.crend(); ++it) {
		current = it->get();

		// if this record is valid.
		// we are done since we iterated in reverse.
		if (current->valid() && !current->immune() && !current->dormant())
			return current;
	}

	return nullptr;
}



void Resolver::ResolveBodyUpdates(Player* player, LagRecord* record) {
	AimPlayer* data = &g_aimbot.m_players[player->index() - 1];

	// from csgo src sdk.
	const float CSGO_ANIM_LOWER_REALIGN_DELAY = 1.1f;

	// first we are gonna start of with some checks.
	if (!g_cl.m_processing) {
		data->m_moved = false;
		data->m_predict = false;
		return;
	}

	// dont predict on doramnt players.
	if (record->dormant()) {
		data->m_predict = false;
		return;
	}

	// if we have missed 2 shots lets predict until he moves again.
	if (data->m_body_index >= 2) {
		data->m_predict = false;
		return;
	}

	// now we are in the prediction stage.
	data->m_predict = true;

	// on ground and not moving.
	if ((record->m_flags & FL_ONGROUND) && record->m_anim_velocity.length() <= 21.f) {
		// ensure we dont get improper LBY updates.
		if (data->m_old_body != data->m_body) {
			// lets time our updates.
			if (record->m_sim_time >= data->m_body_update) {
				// inform cheat of resolver method.
				//record->m_mode = Modes::RESOLVE_BODY;

				// set angles to current LBY.
				record->m_eye_angles.y = record->m_body;

				// set next predicted time, till update.
				data->m_body_update = record->m_sim_time + CSGO_ANIM_LOWER_REALIGN_DELAY;

				// still in prediction process.
				data->m_predict = true;
			}

			// set next predicted time, till update.
			data->m_body_update = record->m_sim_time + CSGO_ANIM_LOWER_REALIGN_DELAY;

			// still in prediction process.
			data->m_predict = true;
		}
	}
	else {
		// set delayed predicted time, after they stop moving.
		data->m_body_update = record->m_sim_time + (CSGO_ANIM_LOWER_REALIGN_DELAY * 0.2f);

		// this indicates if we need to run the LBY timer or not.
		data->m_predict = false;
	}
}

void Resolver::OnBodyUpdate(Player* player, float value) {
	AimPlayer* data = &g_aimbot.m_players[player->index() - 1];

	// set data.
	data->m_old_body = data->m_body;
	data->m_body = value;
	data->m_update_time = g_csgo.m_globals->m_curtime;
}

float Resolver::GetAwayAngle(LagRecord* record) {
	float  delta{ std::numeric_limits< float >::max() };
	vec3_t pos;
	ang_t  away;

	// other cheats predict you by their own latency.
	// they do this because, then they can put their away angle to exactly
	// where you are on the server at that moment in time.

	// the idea is that you would need to know where they 'saw' you when they created their user-command.
	// lets say you move on your client right now, this would take half of our latency to arrive at the server.
	// the delay between the server and the target client is compensated by themselves already, that is fortunate for us.

	// we have no historical origins.
	// no choice but to use the most recent one.
	//if( g_cl.m_net_pos.empty( ) ) {
	math::VectorAngles(g_cl.m_local->m_vecOrigin() - record->m_pred_origin, away);
	return away.y;
	//}

	// half of our rtt.
	// also known as the one-way delay.
	//float owd = ( g_cl.m_latency / 2.f );

	// since our origins are computed here on the client
	// we have to compensate for the delay between our client and the server
	// therefore the OWD should be subtracted from the target time.
	//float target = record->m_pred_time; //- owd;

	// iterate all.
	//for( const auto &net : g_cl.m_net_pos ) {
		// get the delta between this records time context
		// and the target time.
	//	float dt = std::abs( target - net.m_time );

		// the best origin.
	//	if( dt < delta ) {
	//		delta = dt;
	//		pos   = net.m_pos;
	//	}
	//}

	//math::VectorAngles( pos - record->m_pred_origin, away );
	//return away.y;
}

void Resolver::MatchShot(AimPlayer* data, LagRecord* record) {
	// do not attempt to do this in nospread mode.
	if (g_menu.main.config.mode.get() == 1)
		return;

	float shoot_time = -1.f;

	Weapon* weapon = data->m_player->GetActiveWeapon();
	if (weapon) {
		// with logging this time was always one tick behind.
		// so add one tick to the last shoot time.
		shoot_time = weapon->m_fLastShotTime() + g_csgo.m_globals->m_interval;
	}

	// this record has a shot on it.
	if (game::TIME_TO_TICKS(shoot_time) == game::TIME_TO_TICKS(record->m_sim_time)) {
		if (record->m_lag <= 2)
			record->m_shot = true;

		// more then 1 choke, cant hit pitch, apply prev pitch.
		else if (data->m_records.size() >= 2) {
			LagRecord* previous = data->m_records[1].get();

			if (previous && !previous->dormant())
				record->m_eye_angles.x = previous->m_eye_angles.x;
		}
	}
}

void Resolver::SetMode(LagRecord* record) {
	// the resolver has 3 modes to chose from.
	// these modes will vary more under the hood depending on what data we have about the player
	// and what kind of hack vs. hack we are playing (mm/nospread).

	float speed = record->m_anim_velocity.length();
	AimPlayer* data = &g_aimbot.m_players[record->m_player->index() - 1];
	// if on ground, moving, and not fakewalking.
	if ((record->m_flags & FL_ONGROUND) && !record->m_fake_walk && speed >= 1.f) {
		if ((speed <= 20)) {
			// anti comp technology
			data->comp = true;
			data->anti_comp_time = g_csgo.m_globals->m_curtime;
			record->m_mode = Modes::RESOLVE_STAND;
		}
		else {
			data->comp = false;
			record->m_mode = Modes::RESOLVE_WALK;
		}
	}

	// if on ground, not moving or fakewalking.
	if ((record->m_flags & FL_ONGROUND) && speed < 1.f) {
		record->m_mode = Modes::RESOLVE_STAND;
		data->comp = false;
	}

	// if not on ground.
	else if (!(record->m_flags & FL_ONGROUND))
		record->m_mode = Modes::RESOLVE_AIR;

	data->resolver_angle = record->m_eye_angles.y;
}

void Resolver::ResolveAngles(Player* player, LagRecord* record) {
	AimPlayer* data = &g_aimbot.m_players[player->index() - 1];

	// mark this record if it contains a shot.
	MatchShot(data, record);

	// next up mark this record with a resolver mode that will be used.
	SetMode(record);

	// we arrived here we can do the acutal resolve.

	if (record->m_mode == Modes::RESOLVE_WALK)
		ResolveWalk(data, record);
	else if (record->m_mode == Modes::RESOLVE_STAND)
		ResolveStand(data, record);
	else if (record->m_mode == Modes::RESOLVE_AIR)
		ResolveAir(data, record);


	//premium random resolver
	if ((g_hooks.b[XOR("RandomResolver")]))
		RandomResolver(data, record);

	// normalize the eye angles, doesn't really matter but its clean.
	math::NormalizeAngle(record->m_eye_angles.y);
}

void Resolver::ResolveWalk(AimPlayer* data, LagRecord* record) {
	// apply lby to eyeangles.
	record->m_eye_angles.y = record->m_player->m_flLowerBodyYawTarget();

	// havent checked this yet in ResolveStand( ... )
	data->m_moved = false;

	// reset stand and body index. (might break but not resetting freestand index here anymore lol)		
	data->m_last_move = 0;
	data->m_body_index = 0;
	data->m_ff_index = 0;

	// copy the last record that this player was walking
	// we need it later on because it gives us crucial data.
	std::memcpy(&data->m_walk_record, record, sizeof(LagRecord));
}

void FuckedUpResolverBruteShit(LagRecord* record, AimPlayer* data) {
	{
		// OOOHHH SHIT NIGGA UR FUCKED LEMME TAKE CARE OF THIS
		float away = g_resolver.GetAwayAngle(record);
		float baseangle = away;
		record->m_eye_angles.y = baseangle + (std::sin(g_csgo.m_globals->m_curtime) * 180);
		record->m_feet_yaw = away + (record->m_tick % 2 == 0 ? 60 : -60);
	}
}

bool Resolver::FindBestAngle(LagRecord* record, AimPlayer* data) {
	// constants
	constexpr float STEP{ 4.f };
	constexpr float RANGE{ 32.f };

	// best target.
	vec3_t enemypos = record->m_player->GetShootPorsition();
	float away = GetAwayAngle(record);
	float best_dist = 0.f;

	// construct vector of angles to test.
	std::vector< AdaptiveAngle > angles{ };
	angles.emplace_back(away - 180.f);
	angles.emplace_back(away + 90.f);
	angles.emplace_back(away - 90.f);

	// start the trace at the your shoot pos.
	vec3_t start = g_cl.m_local->GetShootPorsition();

	// see if we got any valid result.
	// if this is false the path was not obstructed with anything.
	bool valid{ false };

	// iterate vector of angles.
	for (auto it = angles.begin(); it != angles.end(); ++it) {

		// compute the 'rough' estimation of where our head will be.
		vec3_t end{ enemypos.x + std::cos(math::deg_to_rad(it->m_yaw)) * RANGE,
			enemypos.y + std::sin(math::deg_to_rad(it->m_yaw)) * RANGE,
			enemypos.z };

		// draw a line for debugging purposes.

		//g_csgo.m_debug_overlay->AddLineOverlay( start, end, 255, 0, 0, true, 0.1f );

		// compute the direction.
		vec3_t dir = end - start;
		float len = dir.normalize();

		// should never happen.
		if (len <= 0.f)
			continue;

		// step thru the total distance, 4 units per step.
		for (float i{ 0.f }; i < len; i += STEP) {
			// get the current step position.
			vec3_t point = start + (dir * i);

			// get the contents at this point.
			int contents = g_csgo.m_engine_trace->GetPointContents(point, MASK_SHOT_HULL);

			// contains nothing that can stop a bullet.
			if (!(contents & MASK_SHOT_HULL))
				continue;

			float mult = 1.f;

			// over 50% of the total length, prioritize this shit.
			if (i > (len * 0.5f))
				mult = 1.25f;

			// over 90% of the total length, prioritize this shit.
			if (i > (len * 0.75f))
				mult = 1.25f;

			// over 90% of the total length, prioritize this shit.
			if (i > (len * 0.9f))
				mult = 2.f;

			// append 'penetrated distance'.
			it->m_dist += (STEP * mult);

			// mark that we found anything.
			valid = true;
		}
	}

	if (!valid) {
		return false;
	}

	// put the most distance at the front of the container.
	std::sort(angles.begin(), angles.end(),
		[](const AdaptiveAngle& a, const AdaptiveAngle& b) {
			return a.m_dist > b.m_dist;
		});

	// the best angle should be at the front now.
	AdaptiveAngle* best = &angles.front();

	// ...
	if (best_dist != best->m_dist) {
		best_dist = best->m_dist;
		record->m_eye_angles.y = best->m_yaw;
		return true;
	}
	return false;
}

float GetAntiFreestandAngle(LagRecord* record, AimPlayer* data) {
	// constants
	constexpr float STEP{ 4.f };
	constexpr float RANGE{ 32.f };

	// best target.
	vec3_t enemypos = record->m_player->GetShootPorsition();
	float away = g_resolver.GetAwayAngle(record);
	float best_dist = 0.f;

	// construct vector of angles to test.
	std::vector< AdaptiveAngle > angles{ };
	angles.emplace_back(away - 180.f);
	angles.emplace_back(away + 90.f);
	angles.emplace_back(away - 90.f);

	// start the trace at the your shoot pos.
	vec3_t start = g_cl.m_local->GetShootPorsition();

	// see if we got any valid result.
	// if this is false the path was not obstructed with anything.
	bool valid{ false };

	// iterate vector of angles.
	for (auto it = angles.begin(); it != angles.end(); ++it) {

		// compute the 'rough' estimation of where our head will be.
		vec3_t end{ enemypos.x + std::cos(math::deg_to_rad(it->m_yaw)) * RANGE,
			enemypos.y + std::sin(math::deg_to_rad(it->m_yaw)) * RANGE,
			enemypos.z };

		// draw a line for debugging purposes.

		//g_csgo.m_debug_overlay->AddLineOverlay( start, end, 255, 0, 0, true, 0.1f );

		// compute the direction.
		vec3_t dir = end - start;
		float len = dir.normalize();

		// should never happen.
		if (len <= 0.f)
			continue;

		// step thru the total distance, 4 units per step.
		for (float i{ 0.f }; i < len; i += STEP) {
			// get the current step position.
			vec3_t point = start + (dir * i);

			// get the contents at this point.
			int contents = g_csgo.m_engine_trace->GetPointContents(point, MASK_SHOT_HULL);

			// contains nothing that can stop a bullet.
			if (!(contents & MASK_SHOT_HULL))
				continue;

			float mult = 1.f;

			// over 50% of the total length, prioritize this shit.
			if (i > (len * 0.5f))
				mult = 1.25f;

			// over 90% of the total length, prioritize this shit.
			if (i > (len * 0.75f))
				mult = 1.25f;

			// over 90% of the total length, prioritize this shit.
			if (i > (len * 0.9f))
				mult = 2.f;

			// append 'penetrated distance'.
			it->m_dist += (STEP * mult);

			// mark that we found anything.
			valid = true;
		}
	}

	if (!valid) {
		return 999;
	}

	// put the most distance at the front of the container.
	std::sort(angles.begin(), angles.end(),
		[](const AdaptiveAngle& a, const AdaptiveAngle& b) {
			return a.m_dist > b.m_dist;
		});

	// the best angle should be at the front now.
	AdaptiveAngle* best = &angles.front();

	// ...
	if (best_dist != best->m_dist) {
		return best->m_yaw;
	}
	return 999;
}

Resolver::Directions Resolver::HandleDirections(AimPlayer* data) {
	CGameTrace tr;
	CTraceFilterSimple filter{ };

	if (!g_cl.m_processing)
		return Directions::YAW_NONE;

	// best target.
	struct AutoTarget_t { float fov; Player* player; };
	AutoTarget_t target{ 180.f + 1.f, nullptr };

	// get best target based on fov.
	auto origin = data->m_player->m_vecOrigin();
	ang_t view;
	float fov = math::GetFOV(g_cl.m_cmd->m_view_angles, g_cl.m_local->GetShootPorsition(), data->m_player->WorldSpaceCenter());

	// set best fov.
	if (fov < target.fov) {
		target.fov = fov;
		target.player = data->m_player;
	}

	// get best player.
	const auto player = target.player;
	if (!player)
		return Directions::YAW_NONE;

	auto& bestOrigin = player->m_vecOrigin();

	// skip this player in our traces.
	filter.SetPassEntity(g_cl.m_local);

	// calculate angle direction from thier best origin to our origin
	ang_t angDirectionAngle;
	math::VectorAngles(g_cl.m_local->m_vecOrigin() - bestOrigin, angDirectionAngle);

	vec3_t forward, right, up;
	math::AngleVectors(angDirectionAngle, &forward, &right, &up);

	auto vecStart = g_cl.m_local->GetShootPorsition();
	auto vecEnd = vecStart + forward * 100.0f;

	Ray rightRay(vecStart + right * 35.0f, vecEnd + right * 35.0f), leftRay(vecStart - right * 35.0f, vecEnd - right * 35.0f);

	g_csgo.m_engine_trace->TraceRay(rightRay, MASK_SOLID, &filter, &tr);
	float rightLength = (tr.m_endpos - tr.m_startpos).length();

	g_csgo.m_engine_trace->TraceRay(leftRay, MASK_SOLID, &filter, &tr);
	float leftLength = (tr.m_endpos - tr.m_startpos).length();

	static auto leftTicks = 0;
	static auto rightTicks = 0;
	static auto backTicks = 0;

	if (rightLength - leftLength > 20.0f)
		leftTicks++;
	else
		leftTicks = 0;

	if (leftLength - rightLength > 20.0f)
		rightTicks++;
	else
		rightTicks = 0;

	if (fabs(rightLength - leftLength) <= 20.0f)
		backTicks++;
	else
		backTicks = 0;

	Directions direction = Directions::YAW_NONE;

	if (rightTicks > 10) {
		direction = Directions::YAW_RIGHT;
	}
	else {
		if (leftTicks > 10) {
			direction = Directions::YAW_LEFT;
		}
		else {
			if (backTicks > 10)
				direction = Directions::YAW_BACK;
		}
	}

	return direction;
}

void Resolver::collect_wall_detect(const Stage_t stage)
{
	if (stage != FRAME_NET_UPDATE_POSTDATAUPDATE_START)
		return;

	if (!g_cl.m_local)
		return;

	auto g_pLocalPlayer = g_cl.m_local;

	last_eye_positions.insert(last_eye_positions.begin(), g_pLocalPlayer->m_vecOrigin() + g_pLocalPlayer->m_vecViewOffset());
	if (last_eye_positions.size() > 128)
		last_eye_positions.pop_back();

	auto nci = g_csgo.m_engine->GetNetChannelInfo();
	if (!nci)
		return;

	const int latency_ticks = game::TIME_TO_TICKS(nci->GetLatency(nci->FLOW_OUTGOING));
	auto latency_based_eye_pos = last_eye_positions.size() <= latency_ticks ? last_eye_positions.back() : last_eye_positions[latency_ticks];

	for (auto i = 1; i < g_csgo.m_globals->m_max_clients; i++)
	{
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);

		if (!player || player == g_pLocalPlayer)
		{
			continue;
		}

		if (!player->enemy(g_pLocalPlayer))
		{
			continue;
		}

		if (!player->alive())
		{
			continue;
		}

		if (player->dormant())
		{
			continue;
		}

		if (player->m_vecVelocity().length_2d() > 21.f)
		{
			continue;
		}

		if (using_anti_freestand)
		{
			const auto at_target_angle = math::CalcAngle(player->m_vecOrigin(), last_eye);

			const auto eye_pos = player->get_eye_pos();

			const float height = 64;

			vec3_t direction_1, direction_2, direction_3;
			math::AngleVectors(ang_t(0.f, math::CalcAngle(g_pLocalPlayer->m_vecOrigin(), player->m_vecOrigin()).y - 90.f, 0.f), &direction_1);
			math::AngleVectors(ang_t(0.f, math::CalcAngle(g_pLocalPlayer->m_vecOrigin(), player->m_vecOrigin()).y + 90.f, 0.f), &direction_2);
			math::AngleVectors(ang_t(0.f, math::CalcAngle(g_pLocalPlayer->m_vecOrigin(), player->m_vecOrigin()).y + 180.f, 0.f), &direction_3);

			const auto left_eye_pos = player->m_vecOrigin() + vec3_t(0, 0, height) + (direction_1 * 16.f);
			const auto right_eye_pos = player->m_vecOrigin() + vec3_t(0, 0, height) + (direction_2 * 16.f);
			const auto back_eye_pos = player->m_vecOrigin() + vec3_t(0, 0, height) + (direction_3 * 16.f);

			anti_freestanding_record.left_damage = penetration::scale(player, left_damage[i], 1.f, HITGROUP_CHEST);
			anti_freestanding_record.right_damage = penetration::scale(player, right_damage[i], 1.f, HITGROUP_CHEST);
			anti_freestanding_record.back_damage = penetration::scale(player, back_damage[i], 1.f, HITGROUP_CHEST);

			Ray ray;
			CGameTrace trace;
			CTraceFilterWorldOnly filter;

			Ray first_ray(left_eye_pos, latency_based_eye_pos);
			g_csgo.m_engine_trace->TraceRay(first_ray, MASK_ALL, &filter, &trace);
			anti_freestanding_record.left_fraction = trace.m_fraction;

			Ray second_ray(right_eye_pos, latency_based_eye_pos);
			g_csgo.m_engine_trace->TraceRay(second_ray, MASK_ALL, &filter, &trace);
			anti_freestanding_record.right_fraction = trace.m_fraction;

			Ray third_ray(back_eye_pos, latency_based_eye_pos);
			g_csgo.m_engine_trace->TraceRay(third_ray, MASK_ALL, &filter, &trace);
			anti_freestanding_record.back_fraction = trace.m_fraction;
		}
	}
}

bool hitPlayer[64];

bool Resolver::AntiFreestanding(Player* entity, AimPlayer* data, float& yaw)
{

	const auto freestanding_record = anti_freestanding_record;

	auto local_player = g_cl.m_local;
	if (!local_player)
		return false;

	const float at_target_yaw = math::CalcAngle(local_player->m_vecOrigin(), entity->m_vecOrigin()).y;

	if (freestanding_record.left_damage >= 20 && freestanding_record.right_damage >= 20)
		yaw = at_target_yaw;

	auto set = false;

	if (freestanding_record.left_damage <= 0 && freestanding_record.right_damage <= 0)
	{
		if (freestanding_record.right_fraction < freestanding_record.left_fraction) {
			set = true;
			yaw = at_target_yaw + 125.f;
		}
		else if (freestanding_record.right_fraction > freestanding_record.left_fraction) {
			set = true;
			yaw = at_target_yaw - 73.f;
		}
		else {
			yaw = at_target_yaw;
		}
	}
	else
	{
		if (freestanding_record.left_damage > freestanding_record.right_damage) {
			yaw = at_target_yaw + 130.f;
			set = true;
		}
		else
			yaw = at_target_yaw;
	}

	return true;
}

void basicbrute(LagRecord* record, AimPlayer* data)
{
	float away = record->m_body;

	switch (data->m_freestand_index % 9) {
	case 0:
		record->m_eye_angles.y = away + 180.f;
		break;

	case 1:
		record->m_eye_angles.y = away - 180.f;
		break;
	case 2:
		record->m_eye_angles.y = away - 150.f;
		break;

	case 3:
		record->m_eye_angles.y = away + 150.f;
		break;
	case 4:
		record->m_eye_angles.y = away - 90.f;
		break;

	case 5:
		record->m_eye_angles.y = away + 90.f;
		break;
	case 6:
		record->m_eye_angles.y = away;
		break;

	case 7:
		record->m_eye_angles.y = away + 169.f;
		break;
	case 8:
		record->m_eye_angles.y = away - 169.f;
		break;

	default:
		break;
	}
}

void handle_sideways(LagRecord* record, AimPlayer* data) {
	data->m_freestand_index = 0; //why the fuck did the og resolver not reset freestanding index when switching resolver that uses the same fucking index its just gonna break everything actual smoothbrain
	switch (data->m_freestand_index % 3) {
	case 0:
	{
		record->m_eye_angles.y = record->m_body - 119;
		// 2nd shot
		break;
	}
	case 1:
	{
		record->m_eye_angles.y = record->m_body + 119;
		// third shot
		break;
	}
	case 2:
		data->m_freestand_index = 0;
		basicbrute(record, data);
		break;

	default:
	{
		// nigga how
		break;
	}
	}
	record->m_feet_yaw = record->m_body + (record->m_tick % 2 == 0 ? 60 : -60);
}


void handle_no_freestand(LagRecord* record, AimPlayer* data) {
	data->m_freestand_index = 0; //why the fuck did the og resolver not reset freestanding index when switching resolver that uses the same fucking index its just gonna break everything actual smoothbrain AGAIN
	record->m_mode = Resolver::Modes::RESOLVE_STAND;
	float away = g_resolver.GetAwayAngle(record);
	int shots = 0;

	//CUMHOOK SELFCODED RESOLVER $$$	
	if (data->cumhook_angle == 0)
	{
		data->cumhook_angle == away;
		data->trolling = false;
		data->count = 0;
		shots = data->m_missed_shots;
	}

	if (data->cumhook_angle < 25.f && data->cumhook_angle > -25.f && data->count < 2) 
	{
		if (shots == data->m_missed_shots)
		{
			data->trolling = true;
			shots++;
		}
		else
		{
			data->trolling = false;
		}

		data->trolling ? record->m_eye_angles.y += 65.f : record->m_eye_angles.y -= 65.f;
		data->count++;
	}
	else 
	{
		if (record->m_eye_angles.y == data->cumhook_angle) 
		{
			data->cumhook_angle = data->cumhook_angle + rand() % 10;
			data->count++;
			if (data->count > 7)
			{
				record->m_eye_angles.y += 179.f;
				data->count = 0;
			}
			else
			{
				record->m_eye_angles.y = data->cumhook_angle;
			}
		}
		else 
		{
			record->m_eye_angles.y = data->cumhook_angle;
		}
	}
}

void Resolver::ResolveStand(AimPlayer* data, LagRecord* record) {
	
	float away = GetAwayAngle(record);
	if (data->comp && data->m_ff_index == 0) {
		record->m_mode = Modes::RESOLVE_FAKEFLICK;
		// this is gonna be so retarded (even more retarded we gonna just random 2 shots less likely to dump because rather than forcing 2 angles and constantly dumping we randomly pray we hit the flick, cumhook knows best after all)
		if (record->m_anim_velocity.length_2d() > 0.1f) {
			// micromoving :scream:
			float delta = math::NormalizedAngle(record->m_body - away);
			float deltaabs = std::abs(delta);
			if (deltaabs > 65 && deltaabs < 110) {
				// we have the side flick?
				if (delta < 0) {
					// possibly right side
					data->real_side = 1;
				}
				else {
					// possibly left side
					data->real_side = 2;
				}
			}
		}
		if (data->real_side != 0) {
			// HURAYY WE HAVE A SIDE LETS GO
			record->m_eye_angles.y = away - (data->real_side == 1 ? rand() % 90 : rand() % -90);
			return; // fuck the og resolver lets headshot this nigga
		}
	}
	else { data->real_side = 0; }
	// for no-spread call a seperate resolver.
	if (g_menu.main.config.mode.get() == 1) {
		StandNS(data, record);
		return;
	}

	// set our resolver mode
	record->m_mode = Modes::RESOLVE_FREESTAND;
	//mmmmmmmmmmmmmmmmm time for sexy cumhook bruteforce the freestand 
	switch (data->m_freestand_index % 6) {
	case 0:
		record->m_eye_angles.y += away + 90.f;
		break;

	case 1:
		record->m_eye_angles.y = away - 90.f;
		break;
		
	case 2:
		record->m_eye_angles.y += away - 65.f;
		break;
	
	case 3:
		record->m_eye_angles.y += away + 65.f;
		break;

	case 4:
		//idk what this is lets try it
		handle_no_freestand(record, data);
		break;

	case 5:
		//obviously if our p100 bruteforce cant hit them that they have to not be freestanding cause their cheat is retarded nice freestand usell??? 
		handle_no_freestand(record, data);
		break;

	default: {
		break;
	}
	}
}

void Resolver::StandNS(AimPlayer* data, LagRecord* record) {
	// get away angles.
	float away = GetAwayAngle(record);

	switch (data->m_shots % 8) {
	case 0:
		record->m_eye_angles.y = away + 180.f;
		break;

	case 1:
		record->m_eye_angles.y = away + 90.f;
		break;
	case 2:
		record->m_eye_angles.y = away - 90.f;
		break;

	case 3:
		record->m_eye_angles.y = away + 45.f;
		break;
	case 4:
		record->m_eye_angles.y = away - 45.f;
		break;

	case 5:
		record->m_eye_angles.y = away + 135.f;
		break;
	case 6:
		record->m_eye_angles.y = away - 135.f;
		break;

	case 7:
		record->m_eye_angles.y = away + 0.f;
		break;

	default:
		break;
	}

	// force LBY to not fuck any pose and do a true bruteforce.
	record->m_body = record->m_eye_angles.y;
}

void Resolver::ResolveAir(AimPlayer* data, LagRecord* record) {
	// for no-spread call a seperate resolver.
	if (g_menu.main.config.mode.get() == 1) {
		AirNS(data, record);
		return;
	}

	// else run our matchmaking air resolver.

	// we have barely any speed. 
	// either we jumped in place or we just left the ground.
	// or someone is trying to fool our resolver.
	if (record->m_velocity.length_2d() < 60.f) {
		// set this for completion.
		// so the shot parsing wont pick the hits / misses up.
		// and process them wrongly.
		record->m_mode = Modes::RESOLVE_STAND;

		// invoke our stand resolver.
		ResolveStand(data, record);

		// we are done.
		return;
	}

	// try to predict the direction of the player based on his velocity direction.
	// this should be a rough estimation of where he is looking.
	float velyaw = math::rad_to_deg(std::atan2(record->m_velocity.y, record->m_velocity.x));

	switch (data->m_shots % 4) {
	case 0:
		FindBestAngle(record, data);
		break;

	case 1:
		record->m_eye_angles.y = velyaw + 180.f;
		break;

	case 2:
		record->m_eye_angles.y = velyaw - 90.f;
		break;

	case 3:
		record->m_eye_angles.y = velyaw + 90.f;
		data->m_shots = 0;
		break;
	}
}

void Resolver::AirNS(AimPlayer* data, LagRecord* record) {
	// get away angles.
	float away = GetAwayAngle(record);

	switch (data->m_shots % 9) {
	case 0:
		record->m_eye_angles.y = away + 180.f;
		break;

	case 1:
		record->m_eye_angles.y = away + 150.f;
		break;
	case 2:
		record->m_eye_angles.y = away - 150.f;
		break;

	case 3:
		record->m_eye_angles.y = away + 165.f;
		break;
	case 4:
		record->m_eye_angles.y = away - 165.f;
		break;

	case 5:
		record->m_eye_angles.y = away + 135.f;
		break;
	case 6:
		record->m_eye_angles.y = away - 135.f;
		break;

	case 7:
		record->m_eye_angles.y = away + 90.f;
		break;
	case 8:
		record->m_eye_angles.y = away - 90.f;
		break;

	default:
		break;
	}
}

void Resolver::ResolvePoses(Player* player, LagRecord* record) {
	AimPlayer* data = &g_aimbot.m_players[player->index() - 1];

	// only do this bs when in air.
	if (record->m_mode == Modes::RESOLVE_AIR) {
		// ang = pose min + pose val x ( pose range )

		// lean_yaw
		player->m_flPoseParameter()[2] = g_csgo.RandomInt(0, 4) * 0.25f;

		// body_yaw
		player->m_flPoseParameter()[11] = g_csgo.RandomInt(1, 3) * 0.25f;
	}
}

void Resolver::RandomResolver(AimPlayer* data, LagRecord* record)
{
	record->m_eye_angles.y = rand() % 360;
}