#include "includes.h"

void ConsolePaint( VPANEL panel ) {
	static VPANEL tools{ }, zoom{ };

	// cache tools panel once.
	if( !tools && panel == g_csgo.m_engine_vgui->GetPanel( PANEL_TOOLS ) )
		tools = panel;

	static bool bShouldRecolorConsole;
	static IMaterial* cMaterial[ 5 ];
	if( !cMaterial[ 0 ] || !cMaterial[ 1 ] || !cMaterial[ 2 ] || !cMaterial[ 3 ] || !cMaterial[ 4 ] ) {
		for( uint16_t i{ g_csgo.m_material_system->FirstMaterial( ) }; i != g_csgo.m_material_system->InvalidMaterial( ); i = g_csgo.m_material_system->NextMaterial( i ) ) {
			auto pMaterial = g_csgo.m_material_system->GetMaterial( i );
			if( !pMaterial ) continue;

			if( strstr( pMaterial->GetName( ), "vgui_white" ) )
				cMaterial[ 0 ] = pMaterial;
			else if( strstr( pMaterial->GetName( ), "800corner1" ) )
				cMaterial[ 1 ] = pMaterial;
			else if( strstr( pMaterial->GetName( ), "800corner2" ) )
				cMaterial[ 2 ] = pMaterial;
			else if( strstr( pMaterial->GetName( ), "800corner3" ) )
				cMaterial[ 3 ] = pMaterial;
			else if( strstr( pMaterial->GetName( ), "800corner4" ) )
				cMaterial[ 4 ] = pMaterial;
		}
	} // Couldn't use find material because for some reason "vgui_white" doesn't exist...
	else {
		// You should check for some other panels name that shouldn't be recolored. Not bother fixing it for all.
		if( panel != tools && g_hooks.b[ XOR( "console" ) ] ) {
			if( g_csgo.m_engine->ConIsVisible( ) ) {
				bShouldRecolorConsole = true;
				if( bShouldRecolorConsole ) {
					for( int num = 0; num < 5; num++ ) {
						cMaterial[ num ]->ColorModulate( g_hooks.c[ XOR( "console_color" ) ][ 0 ], g_hooks.c[ XOR( "console_color" ) ][ 1 ], g_hooks.c[ XOR( "console_color" ) ][ 2 ] );
						cMaterial[ num ]->AlphaModulate( g_hooks.c[ XOR( "console_color" ) ][ 3 ] );
					}
				}
			}
		}
		else if( bShouldRecolorConsole ) {
			for( int num = 0; num < 5; num++ ) {
				cMaterial[ num ]->ColorModulate( 1.f, 1.f, 1.f );
				cMaterial[ num ]->AlphaModulate( 1.0f );
			}
			bShouldRecolorConsole = false;
		}
	}
}

void Hooks::PaintTraverse( VPANEL panel, bool repaint, bool force ) {
	if( !this || !g_csgo.m_panel )
		return g_hooks.m_panel.GetOldMethod< PaintTraverse_t >( IPanel::PAINTTRAVERSE )( this, panel, repaint, force );

	static VPANEL tools{ }, zoom{ };

	// cache CHudZoom panel once.
	if( !zoom && FNV1a::get( g_csgo.m_panel->GetName( panel ) ) == HASH( "HudZoom" ) )
		zoom = panel;

	// cache tools panel once.
	if( !tools && panel == g_csgo.m_engine_vgui->GetPanel( PANEL_TOOLS ) )
		tools = panel;

	ConsolePaint( panel );

	// render hack stuff.
	if (panel == tools)
	{
		g_cl.OnPaint();


	}
	// don't call the original function if we want to remove the scope.
	if( panel == zoom && g_hooks.b[ XOR( "removeScopeOverlay" ) ] )
		return;

	g_hooks.m_panel.GetOldMethod< PaintTraverse_t >( IPanel::PAINTTRAVERSE )( this, panel, repaint, force );
}