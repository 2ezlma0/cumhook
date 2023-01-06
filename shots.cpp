#include "includes.h"
#include <string>

Shots g_shots{};

void Shots::StoreLastFireData(Player* target, float damage, int bullets, LagRecord* record, int hitbox) {
	// setup new shot data.
	ShotRecord shot;

	shot.m_target = target;
	shot.m_record = record;

	// was curtime but in case it somehow isnt correct i put tickbase
	shot.m_time = game::TICKS_TO_TIME(g_cl.m_local->m_nTickBase());

	// why? because 2018 (haha we literally calculating ping + event delay, kms)
	float correct = 0.f;
	correct += g_csgo.m_net->GetLatency(1);
	correct += g_csgo.m_net->GetLatency(0);
	correct += game::TICKS_TO_TIME(g_cl.m_lag);
	shot.m_lat = correct;

	shot.m_damage = damage;
	shot.m_pos = g_cl.m_shoot_pos;
	shot.m_hitbox = hitbox;
	shot.m_impacted = false;
	shot.m_confirmed = false;
	shot.m_hurt = false;
	shot.m_weapon_range = g_cl.m_weapon_info->m_range;

	// increment total shots on this player.
	AimPlayer* data = &g_aimbot.m_players[target->index() - 1];
	if (data) {
		++data->m_shots;

		auto matrix = record->m_bones;

		if (matrix)
			memcpy(&shot.m_matrix, &matrix, sizeof(matrix));

		// add to tracks.
		m_shots.push_front(shot);
	}


	// no need to keep an insane amount of shots.
	while (m_shots.size() > 32)
		m_shots.pop_back();
}
bool hit;

void Shots::OnImpact( IGameEvent* evt ) {
	vec3_t     pos, dir, start, end;
	float      time;
	CGameTrace trace;

	// screw this.
	if( !evt || !g_cl.m_local )
		return;

	// get attacker, if its not us, screw it.
	Player* attacker = g_csgo.m_entlist->GetClientEntity< Player* >( g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "userid" ) )->GetInt( ) ) );
	if( !attacker )
		return;

	// decode impact coordinates and convert to vec3.
	pos = {
		evt->m_keys->FindKey( HASH( "x" ) )->GetFloat( ),
		evt->m_keys->FindKey( HASH( "y" ) )->GetFloat( ),
		evt->m_keys->FindKey( HASH( "z" ) )->GetFloat( )
	};

	// get prediction time at this point.
	time = game::TICKS_TO_TIME( g_cl.m_local->m_nTickBase( ) );

	// add to visual impacts if we have features that rely on it enabled.
	// todo - dex; need to match shots for this to have proper GetShootPosition, don't really care to do it anymore.
	//if( g_hooks.b[ XOR( "bulletTracers" ) ] )
		//m_vis_impacts.push_back( { pos, attacker->GetShootPorsition( ), attacker->m_nTickBase( ) } );

	if( attacker != g_cl.m_local )
		return;

	// we did not take a shot yet.
	if( m_shots.empty( ) )
		return;

	struct ShotMatch_t { float delta; ShotRecord* shot; };
	ShotMatch_t match{};
	match.delta = std::numeric_limits< float >::max( );
	match.shot = nullptr;

	// iterate all shots.
	for( auto& s : m_shots ) {

		// this shot was already matched
		// with a 'bullet_impact' event.
		if( s.m_impacted )
			continue;

		// add the latency to the time when we shot.
		// to predict when we would receive this event.
		float predicted = s.m_time + s.m_lat;

		// get the delta between the current time
		// and the predicted arrival time of the shot.
		float delta = std::abs( time - predicted );

		// store this shot as being the best for now.
		if( delta < match.delta ) {
			match.delta = delta;
			match.shot = &s;
		}
	}

	// no valid shotrecord was found.
	ShotRecord* shot = match.shot;
	if( !shot )
		return;

	// this shot was matched.
	shot->m_impacted = true;
	shot->m_server_pos = pos;

	// create new impact instance that we can match with a player hurt.
	ImpactRecord impact;
	impact.m_shot = shot;
	impact.m_tick = g_cl.m_local->m_nTickBase( );
	impact.m_pos = pos;

	
	//g_cl.print( "imp %x time: %f lat: %f dmg: %f\n", shot->m_record, shot->m_time, shot->m_lat, shot->m_damage );

	// add to track.
	m_impacts.push_front( impact );

	// no need to keep an insane amount of impacts.
	while( m_impacts.size( ) > 16 )
		m_impacts.pop_back( );

	
	
	// nospread mode.
	if( !g_hooks.b[ XOR( "antiUntrusted" ) ] )
		return;

	// not in nospread mode, see if the shot missed due to spread.
	Player* target = shot->m_target;
	if( !target )
		return;

	// not gonna bother anymore.
	if( !target->alive( ) )
		return;

	AimPlayer* data = &g_aimbot.m_players[ target->index( ) - 1 ];
	if( !data || data->m_records.empty( ) )
		return;

	// this record was deleted already.
	if( !shot->m_record->m_bones )
		return;

	// we are going to alter this player.
	// store all his og data.
	BackupRecord backup;
	backup.store( target );

	// write historical matrix of the time that we shot
	// into the games bone cache, so we can trace against it.
	shot->m_record->cache( );

	// start position of trace is where we took the shot.
	start = shot->m_pos;

	// the impact pos contains the spread from the server
	// which is generated with the server seed, so this is where the bullet
	// actually went, compute the direction of this from where the shot landed
	// and from where we actually took the shot.
	dir = ( pos - start ).normalized( );

	// get end pos by extending direction forward.
	// todo; to do this properly should save the weapon range at the moment of the shot, cba..
	end = start + ( dir * 8192.f );

	// intersect our historical matrix with the path the shot took.
	g_csgo.m_engine_trace->ClipRayToEntity( Ray( start, end ), MASK_SHOT, target, &trace );

	// we did not hit jackshit, or someone else.
	/*if (!trace.m_entity || !trace.m_entity->IsPlayer() || trace.m_entity != target) {

		g_notify.add( tfm::format( XOR( "Missed shot due to config issue (spread)\n" ) ) );
	}
	else if (trace.m_entity == target) {

		size_t mode = shot->m_record->m_mode;
		//heheheha perfect logs, dont say we missed so nobody can cry when it prints this when you hit as well >:3
		if (mode == Resolver::Modes::RESOLVE_NONE) {
			g_notify.add(tfm::format(XOR("resolver mode: none??? (if you get this then gg i guess)\n")));
		}
		else if ( mode == Resolver::Modes::RESOLVE_WALK) {
			g_notify.add(tfm::format(XOR("resolver mode: walk\n")));
		}
		else if ( mode == Resolver::Modes::RESOLVE_STAND) {
			g_notify.add(tfm::format(XOR("resolver mode: stand\n")));
		}
		else if ( mode == Resolver::Modes::RESOLVE_AIR) {
			g_notify.add(tfm::format(XOR("resolver mode: air\n")));
		}
		else if ( mode == Resolver::Modes::RESOLVE_BODY) {
			g_notify.add(tfm::format(XOR("resolver mode: body axaxaxaxax nice baim miss nn dog\n")));
		}
		else if ( mode == Resolver::Modes::RESOLVE_FAKEFLICK) {
			g_notify.add(tfm::format(XOR("resolver mode: fakeflick\n")));
		}
		else if ( mode == Resolver::Modes::RESOLVE_LM) {
			g_notify.add(tfm::format(XOR("resolver mode: lastmove\n")));
		}
		else if ( mode == Resolver::Modes::RESOLVE_FREESTAND) {
			g_notify.add(tfm::format(XOR("resolver mode: freestand\n")));
		}
	}

	// PERFECT WAY TO FIGURE OUT RESOLVER MODE
	
	if( trace.m_entity == target && mode == Resolver::Modes::RESOLVE_NONE) {
		g_notify.add( tfm::format( XOR( "Missed shot, resolver mode: none??? (if you get this then gg i guess)") ) );
	}
	else if ( trace.m_entity == target && mode == Resolver::Modes::RESOLVE_WALK) {
		g_notify.add(tfm::format(XOR("Missed shot, resolver mode: walk")));
	}
	else if ( trace.m_entity == target && mode == Resolver::Modes::RESOLVE_STAND) {
		g_notify.add(tfm::format(XOR("Missed shot, resolver mode: stand")));
	}
	else if ( trace.m_entity == target && mode == Resolver::Modes::RESOLVE_AIR) {
		g_notify.add(tfm::format(XOR("Missed shot, resolver mode: air")));
	}
	else if ( trace.m_entity == target && mode == Resolver::Modes::RESOLVE_BODY) {
		g_notify.add(tfm::format(XOR("Missed shot, resolver mode: body axaxaxaxax nice baim miss nn dog")));
	}
	else if ( trace.m_entity == target && mode == Resolver::Modes::RESOLVE_FAKEFLICK) {
		g_notify.add(tfm::format(XOR("Missed shot, resolver mode: fakeflick")));
	}
	else if ( trace.m_entity == target < 0.1 && mode == Resolver::Modes::RESOLVE_LM) {
		g_notify.add(tfm::format(XOR("Missed shot, resolver mode: lastmove")));
	}
	else if ( trace.m_entity == target && mode == Resolver::Modes::RESOLVE_FREESTAND) {
		g_notify.add(tfm::format(XOR("Missed shot, resolver mode: freestand")));
	} */

	/*	RESOLVE_NONE,
		RESOLVE_WALK,
		RESOLVE_STAND,
		RESOLVE_AIR,
		RESOLVE_BODY,
		RESOLVE_FAKEFLICK,
		RESOLVE_LM,
		RESOLVE_FREESTAND
	*/


	// restore player to his original state.
	backup.restore( target );
}

void Shots::OnHurt( IGameEvent* evt ) {
	int         attacker, victim, group, hp;
	float       damage;
	std::string name;

	if( !evt || !g_cl.m_local )
		return;

	hit = false;

	attacker = g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "attacker" ) )->GetInt( ) );
	victim = g_csgo.m_engine->GetPlayerForUserID( evt->m_keys->FindKey( HASH( "userid" ) )->GetInt( ) );

	// skip invalid player indexes.
	// should never happen? world entity could be attacker, or a nade that hits you.
	if( attacker < 1 || attacker > 64 || victim < 1 || victim > 64 )
		return;

	// we were not the attacker or we hurt ourselves.
	else if( attacker != g_csgo.m_engine->GetLocalPlayer( ) || victim == g_csgo.m_engine->GetLocalPlayer( ) )
		return;

	// get hitgroup.
	// players that get naded ( DMG_BLAST ) or stabbed seem to be put as HITGROUP_GENERIC.
	group = evt->m_keys->FindKey( HASH( "hitgroup" ) )->GetInt( );

	// invalid hitgroups ( note - dex; HITGROUP_GEAR isn't really invalid, seems to be set for hands and stuff? ).
	if( group == HITGROUP_GEAR )
		return;

	// get the player that was hurt.
	Player* target = g_csgo.m_entlist->GetClientEntity< Player* >( victim );
	if( !target )
		return;

	// get player info.
	player_info_t info;
	if( !g_csgo.m_engine->GetPlayerInfo( victim, &info ) )
		return;

	// get player name;
	name = std::string( info.m_name ).substr( 0, 24 );

	// get damage reported by the server.
	damage = ( float )evt->m_keys->FindKey( HASH( "dmg_health" ) )->GetInt( );

	// get remaining hp.
	hp = evt->m_keys->FindKey( HASH( "health" ) )->GetInt( );

	// hitmarker.
	if( g_hooks.b[ XOR( "hitmarker" ) ] ) {
		g_visuals.m_hit_duration = 1.f;
		g_visuals.m_hit_start = g_csgo.m_globals->m_curtime;
		g_visuals.m_hit_end = g_visuals.m_hit_start + g_visuals.m_hit_duration;
	}

	if( g_hooks.b[ XOR( "hitmarkerSound" ) ] )
		g_csgo.m_sound->EmitAmbientSound( XOR( "buttons/arena_switch_press_02.wav" ), 1.f );

	// print this shit.
	if( g_hooks.b[ XOR( "logDamageDealt" ) ] ) {
		std::string out = tfm::format( XOR( "Hit %s in the %s for %i damage (%i health remaining)\n" ), name, m_groups[ group ], ( int )damage, hp );
		g_notify.add( out );

		hit = true;
	}

	if (g_hooks.b[XOR("Killsay")])
	{
		if (hp < 1)
		{
			if ((g_hooks.b[XOR("RandomResolver")])) {
				g_csgo.m_engine->ExecuteClientCmd(XOR("say tapped by cumhooks premium random resolver"));
			}
			else {

				switch ((rand() % 20) + 1) {

				case 1:
					g_csgo.m_engine->ExecuteClientCmd(XOR("say tapped by cumhook v420"));
					break;

				case 2:
					g_csgo.m_engine->ExecuteClientCmd(XOR("say i love femboys"));
					break;

				case 3:
					g_csgo.m_engine->ExecuteClientCmd(XOR("say 2ezlma0 is my daddy"));
					break;

				case 4:
					g_csgo.m_engine->ExecuteClientCmd(XOR("say omg cumhook v420 is so good"));
					break;

				case 5:
					g_csgo.m_engine->ExecuteClientCmd(XOR("say my cumshot killed you"));
					break;

				case 6:
					g_csgo.m_engine->ExecuteClientCmd(XOR("say please master rail me harder~"));
					break;

				case 7:
					g_csgo.m_engine->ExecuteClientCmd(XOR("say taste my cock as you jerk yours faggot"));
					break;

				case 8:
					g_csgo.m_engine->ExecuteClientCmd(XOR("say im only gay for you silly~"));
					break;

				case 9:
					g_csgo.m_engine->ExecuteClientCmd(XOR("say cumhook proudly has the best femboy in their discord server"));
					break;

				case 10:
					g_csgo.m_engine->ExecuteClientCmd(XOR("say cumhook is now even more pasted ;)"));
					break;

				case 11:
					g_csgo.m_engine->ExecuteClientCmd(XOR("say *nuzzles your bulge* daddy its all excited to see me~!"));
					break;

				case 12:
					g_csgo.m_engine->ExecuteClientCmd(XOR("say i am sorry for all the homosexuality this killsay contains"));
					break;

				case 13:
					g_csgo.m_engine->ExecuteClientCmd(XOR("say ratio + dont care + didn't ask + cry about it + stay mad + get real + L + mald seethe cope harder + hoes mad + basic + skill issue + ratio + you fell off + 1"));
					break;

				case 14:
					g_csgo.m_engine->ExecuteClientCmd(XOR("say LoL u fanN kuku hdf hurensohn xy0 since 2k7 gang u spic monkey lol who.ru dog fucking nn bitch ure 0 in life im veteran i own u dog."));
					break;

				case 15:
					g_csgo.m_engine->ExecuteClientCmd(XOR("say NIGGER!"));
					break;

				case 16:
					g_csgo.m_engine->ExecuteClientCmd(XOR("say best cumhook update by far"));
					break;

				case 17:
					g_csgo.m_engine->ExecuteClientCmd(XOR("say presh the little butt button that makes me moan in pleasure"));
					break;

				case 18:
					g_csgo.m_engine->ExecuteClientCmd(XOR("say oh- oh god... babe did you cum inside~"));
					break;

				case 19:
					g_csgo.m_engine->ExecuteClientCmd(XOR("say s-stop im gonna... c-cum~ don...don't make me inside your mouth~ mmfh *thrusts into your mouth slightly in pleasure*"));
					break;

				case 20:
					g_csgo.m_engine->ExecuteClientCmd(XOR("say koen is handsome asf"));
					break;
				}
			}
		}
	}

	if( group == HITGROUP_GENERIC )
		return;

	// if we hit a player, mark vis impacts.
	if( !m_vis_impacts.empty( ) ) {
		for( auto& i : m_vis_impacts ) {
			if( i.m_tickbase == g_csgo.m_globals->m_tick_count )
				i.m_hit_player = true;
		}
	}

	// no impacts to match.
	if( m_impacts.empty( ) )
		return;

	ImpactRecord* impact{ nullptr };

	// iterate stored impacts.
	for( auto& i : m_impacts ) {

		// this impact doesnt match with our current hit.
		if( i.m_tick != g_cl.m_local->m_nTickBase( ) )
			continue;

		// wrong player.
		if( i.m_shot->m_target != target )
			continue;

		// shit fond.
		impact = &i;
		break;
	}

	// no impact matched.
	if( !impact )
		return;

	// setup new data for hit track and push to hit track.
	HitRecord hit;
	hit.m_impact = impact;
	hit.m_group = group;
	hit.m_damage = damage;
	impact->m_shot->m_hurt = true;

	//g_cl.print( "hit %x time: %f lat: %f dmg: %f\n", impact->m_shot->m_record, impact->m_shot->m_time, impact->m_shot->m_lat, impact->m_shot->m_damage );

	m_hits.push_front( hit );

	while( m_hits.size( ) > 16 )
		m_hits.pop_back( );
}

void Shots::OnFire(IGameEvent* evt)
{
	int attacker;

	// screw this.
	if (!evt || !g_cl.m_local)
		return;

	// get attacker, if its not us, screw it.
	attacker = g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("userid"))->GetInt());
	if (attacker != g_csgo.m_engine->GetLocalPlayer())
		return;

	struct ShotMatch_t { float delta; ShotRecord* shot; };
	ShotMatch_t match;
	match.delta = std::numeric_limits< float >::max();
	match.shot = nullptr;

	// iterate all shots.
	for (auto& s : m_shots) {

		// this shot was already matched
		// with a 'weapon_fire' event.
		if (s.m_confirmed)
			continue;

		// add the latency to the time when we shot.
		// to predict when we would receive this event.
		float predicted = s.m_time + s.m_lat;

		// get the delta between the current time
		// and the predicted arrival time of the shot.
		float delta = std::abs(game::TICKS_TO_TIME(g_cl.m_local->m_nTickBase()) - predicted);

		// store this shot as being the best for now.
		if (delta < match.delta) {
			match.delta = delta;
			match.shot = &s;
		}
	}


	// no valid shotrecord was found.
	ShotRecord* shot = match.shot;
	if (!shot)
		return;

	// ok, this might seem dumb, BUT i think its smart since thats when sv took the shot
	shot->m_confirmed = true;
	shot->m_pos = g_cl.m_shoot_pos;
	shot->m_weapon_range = g_cl.m_weapon_info->m_range;
}

void Shots::OnFrameStage()
{
	if (!g_cl.m_processing || m_shots.empty()) {
		if (!m_shots.empty())
			m_shots.clear();

		return;
	}

	for (auto it = m_shots.begin(); it != m_shots.end();) {
		if (it->m_time + 1.f + it->m_lat < game::TICKS_TO_TIME(g_cl.m_local->m_nTickBase()))
			it = m_shots.erase(it);
		else
			it = next(it);
	}

	for (auto it = m_shots.begin(); it != m_shots.end();) {
		if (it->m_impacted && !it->m_hurt && it->m_confirmed) {
			// not in nospread mode, see if the shot missed due to spread.
			Player* target = it->m_target;
			if (!target) {
				it = m_shots.erase(it);
				continue;
			}

			// not gonna bother anymore.
			if (!target->alive()) {
				g_notify.add(XOR("missed shot due to target death\n"));
				it = m_shots.erase(it);
				continue;
			}

			AimPlayer* data = &g_aimbot.m_players[target->index() - 1];
			if (!data) {
				g_notify.add(XOR("missed shot due to invalid target (data nullptr)\n"));
				it = m_shots.erase(it);
				continue;
			}

			// this record was deleted already.
			if (!it->m_record->m_bones) {
				g_notify.add(XOR("missed shot due to invalid target (bones nullptr)\n"));
				it = m_shots.erase(it);
				continue;
			}

			if (!it->m_matrix) {
				g_notify.add(XOR("missed shot due to invalid target (matrix nullptr)\n"));
				it = m_shots.erase(it);
				continue;
			}

			// write historical matrix of the time that we shot
			// into the games bone cache, so we can trace against it.
			//it->m_record->cache( );

			// start position of trace is where we took the shot.
			vec3_t start = it->m_pos;

			// the impact pos contains the spread from the server
			// which is generated with the server seed, so this is where the bullet
			// actually went, compute the direction of this from where the shot landed
			// and from where we actually took the shot.
			vec3_t dir = (it->m_server_pos - start).normalized();

			// get end pos by extending direction forward.
			// todo; to do this properly should save the weapon range at the moment of the shot, cba..
			vec3_t end = start + dir * std::clamp((start.dist_to(it->m_record->m_pred_origin) * 6.6f), 0.f, it->m_weapon_range);

			if (!g_aimbot.CanHit(start, end, it->m_record, it->m_hitbox, true, it->m_matrix, data->m_player->index())) {
				g_notify.add(XOR("missed shot due to spread (i came too far)\n"));

				//g_csgo.m_debug_overlay->AddLineOverlay(start, end, 255, 255, 255, false, 3.f);

				it = m_shots.erase(it);

				continue;
			}

			auto mode = it->m_record->m_mode;

			if (mode == g_resolver.RESOLVE_BODY)
				++data->m_body_index;

			// increment
			++data->m_missed_shots;

			// add to modes
			if (mode == Resolver::Modes::RESOLVE_BODY)
				data->m_body_index++;

			if (mode == Resolver::Modes::RESOLVE_STAND)
				data->m_stand_index++;

			if (mode == Resolver::Modes::RESOLVE_FREESTAND)
				data->m_freestand_index++;

			data->m_missed_shots++;

			g_notify.add(XOR("missed shot due to bad resolve (swallowed the cumshot)"));
			g_notify.add(XOR("angle: ") + std::to_string(it->m_record->m_eye_angles.y) + "\n"); //mmm resolver angle

			// we processed this shot, let's delete it.
			it = m_shots.erase(it);
		}
		else {
			it = next(it);
		}
	}
}