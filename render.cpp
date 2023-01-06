#include "includes.h"

namespace render {
	Font menu;;
	Font menu_shade;;
	Font esp;;
	Font esp_small;;
	Font hud;;
	Font cs;;
	Font indicator;;
	Font menu_shade1;;
	Font logevent;;
	Font icons;;
	Font weapon;;
	Font damage;;
	Font gw;;
}

void render::init( ) {
	menu = Font( XOR( "Verdana" ), 12, FW_NORMAL, FONTFLAG_NONE );
	menu_shade = Font( XOR( "Verdana" ), 12, FW_NORMAL, FONTFLAG_DROPSHADOW );
	esp = Font( XOR( "Comic Sans MS" ), 18, FW_BOLD, FONTFLAG_DROPSHADOW );
	esp_small = Font( XOR( "Comic Sans MS" ), 14, FW_NORMAL, FONTFLAG_OUTLINE );
	hud = Font( XOR( "Verdana" ), 16, FW_NORMAL, FONTFLAG_ANTIALIAS );
	cs = Font( XOR( "Verdana" ), 28, FW_MEDIUM, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW );
	indicator = Font( XOR( "Comic Sans MS" ), 26, FW_BOLD, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW );
	menu_shade1 = Font( XOR( "Verdana" ), 19, FW_BOLD, FONTFLAG_DROPSHADOW | FONTFLAG_ANTIALIAS );
	logevent = Font( XOR( "Comic Sans MS" ), 10, FW_DONTCARE, FONTFLAG_DROPSHADOW );
	icons = Font( XOR( "Verdana" ), 50, FW_DONTCARE, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW );
	weapon = Font( XOR( "undefeated" ), 12, FW_MEDIUM, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW );
	damage = Font( XOR( "Verdana" ), 12, FW_NORMAL, FONTFLAG_ANTIALIAS | FONTFLAG_DROPSHADOW );
	gw = Font( XOR( "undefeated" ), 24, FW_MEDIUM, FONTFLAG_ANTIALIAS );
}

bool render::WorldToScreen( const vec3_t& world, vec2_t& screen ) {
	float w;

	const VMatrix& matrix = g_csgo.m_engine->WorldToScreenMatrix( );

	// check if it's in view first.
	// note - dex; w is below 0 when world position is around -90 / +90 from the player's camera on the y axis.
	w = matrix[ 3 ][ 0 ] * world.x + matrix[ 3 ][ 1 ] * world.y + matrix[ 3 ][ 2 ] * world.z + matrix[ 3 ][ 3 ];
	if( w < 0.001f )
		return false;

	// calculate x and y.
	screen.x = matrix[ 0 ][ 0 ] * world.x + matrix[ 0 ][ 1 ] * world.y + matrix[ 0 ][ 2 ] * world.z + matrix[ 0 ][ 3 ];
	screen.y = matrix[ 1 ][ 0 ] * world.x + matrix[ 1 ][ 1 ] * world.y + matrix[ 1 ][ 2 ] * world.z + matrix[ 1 ][ 3 ];

	screen /= w;

	// calculate screen position.
	screen.x = ( g_cl.m_width / 2 ) + ( screen.x * g_cl.m_width ) / 2;
	screen.y = ( g_cl.m_height / 2 ) - ( screen.y * g_cl.m_height ) / 2;

	return true;
}

void render::line( vec2_t v0, vec2_t v1, Color color ) {
	g_csgo.m_surface->DrawSetColor( color );
	g_csgo.m_surface->DrawLine( v0.x, v0.y, v1.x, v1.y );
}

void render::line( int x0, int y0, int x1, int y1, Color color ) {
	g_csgo.m_surface->DrawSetColor( color );
	g_csgo.m_surface->DrawLine( x0, y0, x1, y1 );
}

void render::rect( int x, int y, int w, int h, Color color ) {
	g_csgo.m_surface->DrawSetColor( color );
	g_csgo.m_surface->DrawOutlinedRect( x, y, x + w, y + h );
}

void render::rect_filled( int x, int y, int w, int h, Color color ) {
	g_csgo.m_surface->DrawSetColor( color );
	g_csgo.m_surface->DrawFilledRect( x, y, x + w, y + h );
}

void render::rect_filled_fade( int x, int y, int w, int h, Color color, int a1, int a2 ) {
	g_csgo.m_surface->DrawSetColor( color );
	g_csgo.m_surface->DrawFilledRectFade( x, y, x + w, y + h, a1, a2, false );
}

void render::rect_outlined( int x, int y, int w, int h, Color color, Color color2 ) {
	rect( x, y, w, h, color );
	rect( x - 1, y - 1, w + 2, h + 2, color2 );
	rect( x + 1, y + 1, w - 2, h - 2, color2 );
}

void render::draw_arc( int x, int y, int radius, int start_angle, int percent, int thickness, Color color ) {
	float precision = ( 2 * math::pi ) / 180;
	float step = math::pi / 180;
	float inner = radius - thickness;
	float end_angle = ( start_angle + percent ) * step;
	float start_angle1337 = ( start_angle * math::pi ) / 180;

	for( ; radius > inner; --radius ) {
		for( float angle = start_angle1337; angle < end_angle; angle += precision ) {
			float cx = round( x + radius * cos( angle ) );
			float cy = round( y + radius * sin( angle ) );

			float cx2 = round( x + radius * cos( angle + precision ) );
			float cy2 = round( y + radius * sin( angle + precision ) );

			g_csgo.m_surface->DrawSetColor( color );
			g_csgo.m_surface->DrawLine( cx, cy, cx2, cy2 );
		}
	}
}

void render::arc( int x, int y, int radius, int radius_inner, int start_angle, int end_angle, int segments, Color color ) {
	g_csgo.m_surface->DrawSetColor( color );

	segments = 360 / segments;

	for( float i = start_angle; i < start_angle + end_angle; i = i + segments ) {

		float rad = i * math::pi / 180;
		float rad2 = ( i + segments ) * math::pi / 180;

		float rad_cos = cos( rad );
		float rad_sin = sin( rad );

		float rad2_cos = cos( rad2 );
		float rad2_sin = sin( rad2 );

		float x1_inner = x + rad_cos * radius_inner;
		float y1_inner = y + rad_sin * radius_inner;

		float x1_outer = x + rad_cos * radius;
		float y1_outer = y + rad_sin * radius;

		float x2_inner = x + rad2_cos * radius_inner;
		float y2_inner = y + rad2_sin * radius_inner;

		float x2_outer = x + rad2_cos * radius;
		float y2_outer = y + rad2_sin * radius;

		Vertex verts[ 3 ] = {
			Vertex( vec2_t{ x1_outer, y1_outer } ),
			Vertex( vec2_t{ x2_outer, y2_outer } ),
			Vertex( vec2_t{ x1_inner, y1_inner } )
		};

		Vertex verts2[ 3 ] = {
			Vertex( vec2_t{ x1_inner, y1_inner } ),
			Vertex( vec2_t{ x2_outer, y2_outer } ),
			Vertex( vec2_t{ x2_inner, y2_inner } )
		};

		if( verts )
			g_csgo.m_surface->DrawTexturedPolygon( 3, verts );

		if( verts2 )
			g_csgo.m_surface->DrawTexturedPolygon( 3, verts2 );
	}
}

void render::Draw3DFilledCircle( const vec3_t& origin, float radius, Color color ) {
	static auto prevScreenPos = vec2_t( 0.f, 0.f ); //-V656
	static auto step = math::pi * 2.0f / 72.0f;

	auto screenPos = vec2_t( 0.f, 0.f );
	auto screen = vec2_t( 0.f, 0.f );

	if( !render::WorldToScreen( origin, screen ) )
		return;

	for( auto rotation = 0.0f; rotation <= math::pi * 2.0f; rotation += step ) {
		vec3_t pos( radius * cos( rotation ) + origin.x, radius * sin( rotation ) + origin.y, origin.z );

		if( render::WorldToScreen( pos, screenPos ) ) {
			if( !prevScreenPos.IsZero( ) && prevScreenPos.valid( ) && screenPos.valid( ) && prevScreenPos != screenPos )
				render::line( prevScreenPos.x, prevScreenPos.y, screenPos.x, screenPos.y, color );

			prevScreenPos = screenPos;
		}
	}
}

void render::circle( int x, int y, int radius, int segments, Color color ) {
	static int texture = g_csgo.m_surface->CreateNewTextureID( true );

	//g_csgo.m_surface->DrawSetTextureRGBA( texture, &colors::white, 1, 1 );
	g_csgo.m_surface->DrawSetColor( color );
	g_csgo.m_surface->DrawSetTexture( texture );

	std::vector< Vertex > vertices{};

	float step = math::pi_2 / segments;
	for( float i{ 0.f }; i < math::pi_2; i += step )
		vertices.emplace_back( vec2_t{ x + ( radius * std::cos( i ) ), y + ( radius * std::sin( i ) ) } );

	g_csgo.m_surface->DrawTexturedPolygon( vertices.size( ), vertices.data( ) );
}

void render::gradient( int x, int y, int w, int h, Color color1, Color color2, bool sideways ) {
	g_csgo.m_surface->DrawSetColor( color1 );
	g_csgo.m_surface->DrawFilledRectFade( x, y, x + w, y + h, color1.a( ), 0, sideways );

	g_csgo.m_surface->DrawSetColor( color2 );
	g_csgo.m_surface->DrawFilledRectFade( x, y, x + w, y + h, 0, color2.a( ), sideways );
}

void render::sphere( vec3_t origin, float radius, float angle, float scale, Color color ) {
	std::vector< Vertex > vertices{};

	// compute angle step for input radius and precision.
	float step = ( 1.f / radius ) + math::deg_to_rad( angle );

	for( float lat{}; lat < ( math::pi * scale ); lat += step ) {
		// reset.
		vertices.clear( );

		for( float lon{}; lon < math::pi_2; lon += step ) {
			vec3_t point{
				origin.x + ( radius * std::sin( lat ) * std::cos( lon ) ),
				origin.y + ( radius * std::sin( lat ) * std::sin( lon ) ),
				origin.z + ( radius * std::cos( lat ) )
			};

			vec2_t screen;
			if( WorldToScreen( point, screen ) )
				vertices.emplace_back( screen );
		}

		if( vertices.empty( ) )
			continue;

		g_csgo.m_surface->DrawSetColor( color );
		g_csgo.m_surface->DrawTexturedPolyLine( vertices.size( ), vertices.data( ) );
	}
}

Vertex render::RotateVertex( const vec2_t& p, const Vertex& v, float angle ) {
	// convert theta angle to sine and cosine representations.
	float c = std::cos( math::deg_to_rad( angle ) );
	float s = std::sin( math::deg_to_rad( angle ) );

	return {
		p.x + ( v.m_pos.x - p.x ) * c - ( v.m_pos.y - p.y ) * s,
		p.y + ( v.m_pos.x - p.x ) * s + ( v.m_pos.y - p.y ) * c
	};
}

void render::Font::string( int x, int y, Color color, const std::string& text, StringFlags_t flags /*= render::ALIGN_LEFT */ ) {
	wstring( x, y, color, util::MultiByteToWide( text ), flags );
}

void render::Font::string( int x, int y, Color color, const std::stringstream& text, StringFlags_t flags /*= render::ALIGN_LEFT */ ) {
	wstring( x, y, color, util::MultiByteToWide( text.str( ) ), flags );
}

void render::Font::wstring( int x, int y, Color color, const std::wstring& text, StringFlags_t flags /*= render::ALIGN_LEFT */ ) {
	int w, h;

	g_csgo.m_surface->GetTextSize( m_handle, text.c_str( ), w, h );
	g_csgo.m_surface->DrawSetTextFont( m_handle );
	g_csgo.m_surface->DrawSetTextColor( color );

	if( flags & ALIGN_RIGHT )
		x -= w;
	if( flags & render::ALIGN_CENTER )
		x -= w / 2;

	g_csgo.m_surface->DrawSetTextPos( x, y );
	g_csgo.m_surface->DrawPrintText( text.c_str( ), ( int )text.size( ) );
}

render::FontSize_t render::Font::size( const std::string& text ) {
	return wsize( util::MultiByteToWide( text ) );
}

render::FontSize_t render::Font::wsize( const std::wstring& text ) {
	FontSize_t res;
	g_csgo.m_surface->GetTextSize( m_handle, text.data( ), res.m_width, res.m_height );
	return res;
}