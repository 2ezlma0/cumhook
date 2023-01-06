#include "includes.h"

#define NET_FRAMES_BACKUP 64 // must be power of 2. 
#define NET_FRAMES_MASK ( NET_FRAMES_BACKUP - 1 )

int SetSuitableInSequence( INetChannel* channel ) {
	float spike = std::max( 1.f, g_hooks.f[ XOR( "fake_latency_amt" ) ] ) / 1000.f;
	float correct = std::max( 0.f, spike - g_cl.m_latency - g_cl.m_lerp );

	channel->m_in_seq -= game::TIME_TO_TICKS( correct );
	return game::TIME_TO_TICKS( correct );
}

static bool m_bIsFlipedState = false;
/*int Hooks::SendDatagram( void* data ) {
	if( !g_csgo.m_engine->IsInGame( ) || data )
		return g_hooks.m_net_channel.GetOldMethod< SendDatagram_t >( INetChannel::SENDDATAGRAM )( this, data );

	if( !g_hooks.b[ XOR( "fake_latency" ) ] && !g_hooks.auto_check( "fake_latency_key" ) )
		return g_hooks.m_net_channel.GetOldMethod< SendDatagram_t >( INetChannel::SENDDATAGRAM )( this, data );

	static int prevrel = 0;
	auto spike = !g_hooks.b[ XOR( "fake_latency" ) ] || m_bIsFlipedState ? 0 : SetSuitableInSequence( g_csgo.m_net );

	int ret = g_hooks.m_net_channel.GetOldMethod< SendDatagram_t >( INetChannel::SENDDATAGRAM )( this, data );

	g_csgo.m_net->m_in_seq += spike;

	if( !g_hooks.b[ XOR( "fake_latency" ) ] || prevrel == g_csgo.m_net->m_out_rel_state )
		m_bIsFlipedState = false;

	prevrel = g_csgo.m_net->m_out_rel_state;
	return ret;
}*/

int Hooks::SendDatagram( void* data ) {
	if( !this || !g_csgo.m_engine->IsInGame( ) || !g_csgo.m_net )
		return g_hooks.m_net_channel.GetOldMethod< SendDatagram_t >( INetChannel::SENDDATAGRAM )( this, data );

	//int backup1 = g_csgo.m_net->m_in_rel_state;
	const auto backup2 = g_csgo.m_net->m_in_seq;

	if( g_hooks.b[ XOR( "fake_latency" ) ] && g_hooks.auto_check( XOR( "fake_latency_key" ) ) && g_cl.m_processing && g_csgo.m_cl->m_delta_tick != -1 ) {
		int ping = g_hooks.f[ XOR( "fake_latency_amt" ) ];

		// the target latency.
		float correct = std::max( 0.f, ( ping / 1000.f ) - g_cl.m_latency - g_cl.m_lerp );

		g_csgo.m_net->m_in_seq += 2 * NET_FRAMES_MASK - static_cast< uint32_t >( NET_FRAMES_MASK * correct );
	}

	int ret = g_hooks.m_net_channel.GetOldMethod< SendDatagram_t >( INetChannel::SENDDATAGRAM )( this, data );

	//g_csgo.m_net->m_in_rel_state = backup1;
	g_csgo.m_net->m_in_seq = backup2;

	return ret;
}

void Hooks::ProcessPacket( void* packet, bool header ) {
	if( !g_csgo.m_net ) {
		g_hooks.m_net_channel.GetOldMethod< ProcessPacket_t >( INetChannel::PROCESSPACKET )( this, packet, header );
		return;
	}	

	const auto orig = g_csgo.m_net->m_in_rel_state;

	g_hooks.m_net_channel.GetOldMethod< ProcessPacket_t >( INetChannel::PROCESSPACKET )( this, packet, header );

	if( g_csgo.m_net->m_in_rel_state != orig )
		m_bIsFlipedState = true;	
}