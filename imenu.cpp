#include "includes.h"

#include "ImFont.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include "imgui_internal.h"
#include "imenu.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
LRESULT __stdcall Hooks::WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	if( msg == WM_KEYDOWN && wParam == VK_INSERT )
		g_gui.m_open = !g_gui.m_open;

	if( g_gui.m_open ) {
		ImGui_ImplWin32_WndProcHandler( hWnd, msg, wParam, lParam );
		if( wParam != 'W' && wParam != 'A' && wParam != 'S' && wParam != 'D' && wParam != VK_SHIFT && wParam != VK_CONTROL && wParam != VK_TAB && wParam != VK_SPACE || ImGui::GetIO( ).WantTextInput )
			return true;
	}

	return g_winapi.CallWindowProcA( g_hooks.m_old_wndproc, hWnd, msg, wParam, lParam );
}

void ColorPicker( const char* name, float* color, bool alpha ) {
	ImGuiWindow* window = ImGui::GetCurrentWindow( );
	ImGuiStyle* style = &ImGui::GetStyle( );

	auto alphaSliderFlag = alpha ? ImGuiColorEditFlags_AlphaBar : ImGuiColorEditFlags_NoAlpha;

	ImGui::SameLine( 205.f );
	ImGui::ColorEdit4( std::string{ XOR( "##" ) }.append( name ).append( XOR( "Picker" ) ).c_str( ), color, alphaSliderFlag | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop );
}

void SosiHuy( ) {
	ImGuiStyle* style = &ImGui::GetStyle( );
	// =====================================
	// - Custom controls
	// =====================================

#define InsertSpacer(x1) style->Colors[ImGuiCol_ChildBg] = ImColor(0, 0, 0, 0); ImGui::BeginChild(x1, ImVec2(210.f, 18.f), false); {} ImGui::EndChild(); style->Colors[ImGuiCol_ChildBg] = ImColor(49, 49, 49, 255);
#define InsertGroupboxSpacer(x1) style->Colors[ImGuiCol_ChildBg] = ImColor(0, 0, 0, 0); ImGui::BeginChild(x1, ImVec2(210.f, 9.f), false); {} ImGui::EndChild(); style->Colors[ImGuiCol_ChildBg] = ImColor(49, 49, 49, 255);
#define InsertGroupboxTitle(x1) ImGui::Spacing(); ImGui::NewLine(); ImGui::SameLine(11.f); ImGui::GroupBoxTitle(x1);

#define InsertGroupBoxLeft(x1,x2) ImGui::NewLine(); ImGui::SameLine(19.f); ImGui::BeginGroupBox(x1, ImVec2(238.f, x2), true, ImGuiWindowFlags_NoMove);
#define InsertGroupBoxRight(x1,x2) ImGui::NewLine(); ImGui::SameLine(10.f); ImGui::BeginGroupBox(x1, ImVec2(238.f, x2), true);
#define InsertEndGroupBoxLeft(x1,x2) ImGui::EndGroupBox(); ImGui::SameLine(19.f); ImGui::BeginGroupBoxScroll(x1, x2, ImVec2(238.f, 11.f), true); ImGui::EndGroupBoxScroll();
#define InsertEndGroupBoxRight(x1,x2) ImGui::EndGroupBox(); ImGui::SameLine(10.f); ImGui::BeginGroupBoxScroll(x1, x2, ImVec2(238.f, 11.f), true); ImGui::EndGroupBoxScroll();

#define InsertGroupBoxTop(x1,x2) ImGui::NewLine(); ImGui::SameLine(19.f); ImGui::BeginGroupBox(x1, x2, true);
#define InsertEndGroupBoxTop(x1,x2,x3) ImGui::EndGroupBox(); ImGui::SameLine(19.f); ImGui::BeginGroupBoxScroll(x1, x2, x3, true); ImGui::EndGroupBoxScroll();

// =====================================
// - Default controls
// =====================================
#define InsertCheckbox(x1,x2) if ( itemMap.find(x1) == itemMap.end() ) { \
	itemMap[x1] = menuItem{ &x2 }; } \
	ImGui::Spacing(); ImGui::NewLine(); ImGui::SameLine(19.f); ImGui::Checkbox(x1, &x2);
#define InsertSlider(x1,x2,x3,x4,x5)  if ( itemMap.find(x1) == itemMap.end() ) { \
	itemMap[x1] = menuItem{ &x2 }; } \
	ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(160.f); ImGui::SliderFloat(x1, &x2, x3, x4, x5); ImGui::PopItemWidth();
#define InsertSliderWithoutText(x1,x2,x3,x4,x5)  if ( itemMap.find(x1) == itemMap.end() ) { \
	itemMap[x1] = menuItem{ &x2 }; } \
 ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(160.f); ImGui::SliderFloat(x1, &x2, x3, x4, x5); ImGui::PopItemWidth();
#define InsertCombo(x1,x2,x3)  if ( itemMap.find(x1) == itemMap.end() ) { \
	itemMap[x1] = menuItem{ x2, &x3 }; } \
	ImGui::Spacing(); ImGui::NewLine(); ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f); ImGui::SingleSelect(x1, x2, x3); ImGui::PopItemWidth();
#define InsertComboWithoutText(x1,x2,x3) if ( itemMap.find(x1) == itemMap.end() ) { \
	itemMap[x1] = menuItem{ x2, &x3 }; } \
 ImGui::Spacing(); ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f); ImGui::SingleSelect(x1, x2, x3); ImGui::PopItemWidth();
#define InsertMultiCombo(x1,x2,x3) if ( itemMap.find(x1) == itemMap.end() ) { \
	itemMap[x1] = menuItem{ nullptr, &x3, x2, x3 }; } \
	ImGui::Spacing(); ImGui::NewLine(); ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f); ImGui::MultiSelect(x1, x2, x3); ImGui::PopItemWidth();
#define InsertMultiComboWithoutText(x1,x2,x3) if ( itemMap.find(x1) == itemMap.end() ) { \
	itemMap[x1] = menuItem{ nullptr, &x3, x2, x3 }; } \
	ImGui::Spacing(); ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f); ImGui::MultiSelect(x1, x2, x3); ImGui::PopItemWidth();

#define InsertCombos(x1,x2,x3) if ( itemMap.find(x1) == itemMap.end() ) { \
	itemMap[x1] = menuItem{ x2, &x3 }; } \
ImGui::NewLine(); ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f); ImGui::SingleSelect(x1, x2, x3); ImGui::PopItemWidth();
#define InsertComboWithoutTexts(x1,x2,x3) if ( itemMap.find(x1) == itemMap.end() ) { \
	itemMap[x1] = menuItem{ x2, &x3 }; } \
	ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f); ImGui::SingleSelect(x1, x2, x3); ImGui::PopItemWidth();
#define InsertMultiCombos(x1,x2,x3)if ( itemMap.find(x1) == itemMap.end() ) { \
	itemMap[x1] = menuItem{ nullptr, &x3, x2, x3 }; } \
 ImGui::NewLine(); ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f); ImGui::MultiSelect(x1, x2, x3); ImGui::PopItemWidth();
#define InsertMultiComboWithoutTexts(x1,x2,x3) if ( itemMap.find(x1) == itemMap.end() ) { \
	itemMap[x1] = menuItem{ nullptr, &x3, x2, x3 }; } \
	ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::PushItemWidth(158.f); ImGui::MultiSelect(x1, x2, x3); ImGui::PopItemWidth();

#define InsertColorPicker(x1,x2,x3) ImGui::SameLine(205.f); ColorPicker(x1, x2, x3);

#define InsertKeyBind(x1,x2) ImGui::Keybind(x1, &x2);
#define InsertKeyBindWithText(x1,x2,x3) ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::Text(x1); ImGui::Keybind(x2, &x3);

#define InsertStyleKeyBind(x1,x2,x3) ImGui::Keybind(x1, &x2, x3);
#define InsertStyleKeyBindWithText(x1,x2,x3,x4) ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::Text(x1); ImGui::Keybind(x2, &x3, x4);
}

std::string Hooks::color_to_string( float col[ 4 ] ) {
	return std::to_string( ( int )( col[ 0 ] * 255 ) ) + XOR( "," ) + std::to_string( ( int )( col[ 1 ] * 255 ) ) + XOR( "," ) + std::to_string( ( int )( col[ 2 ] * 255 ) ) + XOR( "," ) + std::to_string( ( int )( col[ 3 ] * 255 ) );
}

float* Hooks::string_to_color( std::string s ) {
	static auto split = [ ]( std::string str, const char* del ) -> std::vector<std::string> {
		char* pTempStr = _strdup( str.c_str( ) );
		char* pWord = strtok( pTempStr, del );
		std::vector<std::string> dest;

		while( pWord != NULL ) {
			dest.push_back( pWord );
			pWord = strtok( NULL, del );
		}

		free( pTempStr );

		return dest;
	};

	std::vector<std::string> col = split( s, XOR( "," ) );
	return new float[ 4 ]{
		( float )std::stoi( col.at( 0 ) ) / 255.f,
		( float )std::stoi( col.at( 1 ) ) / 255.f,
		( float )std::stoi( col.at( 2 ) ) / 255.f,
		( float )std::stoi( col.at( 3 ) ) / 255.f
	};
}
void dmt( std::string key ) {
	if( ImGui::IsItemHovered( ) ) {
		ImGui::BeginTooltip( );
		ImGui::Text( key.c_str( ) );
		ImGui::EndTooltip( );
	}
}
void Hooks::save( ) {
	char file_path[ MAX_PATH ] = { 0 };
	sprintf( file_path, XOR( "C:/cumhook/cfg/%s.idb" ), presets.at( i[ XOR( "_preset" ) ] ) );

	for( auto e : b ) {
		if( !std::string( e.first ).find( XOR( "_" ) ) ) continue;
		char buffer[ 8 ] = { 0 }; _itoa( e.second, buffer, 10 );
		WritePrivateProfileStringA( XOR( "b" ), e.first.c_str( ), std::string( buffer ).c_str( ), file_path );
	}

	for( auto e : i ) {
		if( !std::string( e.first ).find( XOR( "_" ) ) ) continue;
		char buffer[ 32 ] = { 0 }; _itoa( e.second, buffer, 10 );
		WritePrivateProfileStringA( XOR( "i" ), e.first.c_str( ), std::string( buffer ).c_str( ), file_path );
	}

	for( auto e : f ) {
		if( !std::string( e.first ).find( XOR( "_" ) ) ) continue;
		char buffer[ 64 ] = { 0 }; sprintf( buffer, XOR( "%f" ), e.second );
		WritePrivateProfileStringA( XOR( "f" ), e.first.c_str( ), std::string( buffer ).c_str( ), file_path );
	}

	for( auto e : c ) {
		if( !std::string( e.first ).find( XOR( "_" ) ) ) continue;
		WritePrivateProfileStringA( XOR( "c" ), e.first.c_str( ), color_to_string( e.second ).c_str( ), file_path );
	}

	for( auto e : m ) {
		if( !std::string( e.first ).find( XOR( "_" ) ) ) continue;

		std::string vs = "";
		for( auto v : e.second )
			vs += std::to_string( v.first ) + XOR( ":" ) + std::to_string( v.second ) + XOR( "|" );

		WritePrivateProfileStringA( XOR( "m" ), e.first.c_str( ), vs.c_str( ), file_path );
	}

	this->save_keys( );
}
#pragma optimize( "", off ) //‚Ë‰‡Î Í‡Í‡ˇ ıÈÛÌˇ ÂÒÚ¸ı‡ı‡ı‡

void Hooks::load( ) {
	this->load_defaults( );

	char file_path[ MAX_PATH ] = { 0 };
	sprintf( file_path, XOR( "C:/cumhook/cfg/%s.idb" ), presets.at( i[ XOR( "_preset" ) ] ) );

	char b_buffer[ 65536 ], i_buffer[ 65536 ], f_buffer[ 65536 ], c_buffer[ 65536 ], m_buffer[ 65536 ] = { 0 };

	int b_read = GetPrivateProfileSectionA( XOR( "b" ), b_buffer, 65536, file_path );
	int i_read = GetPrivateProfileSectionA( XOR( "i" ), i_buffer, 65536, file_path );
	int f_read = GetPrivateProfileSectionA( XOR( "f" ), f_buffer, 65536, file_path );
	int c_read = GetPrivateProfileSectionA( XOR( "c" ), c_buffer, 65536, file_path );
	int m_read = GetPrivateProfileSectionA( XOR( "m" ), m_buffer, 65536, file_path );

	if( ( 0 < b_read ) && ( ( 65536 - 2 ) > b_read ) ) {
		const char* pSubstr = b_buffer;

		while( '\0' != *pSubstr ) {
			size_t substrLen = strlen( pSubstr );

			const char* pos = strchr( pSubstr, '=' );
			if( NULL != pos ) {
				char name[ 256 ] = "";
				char value[ 256 ] = "";

				strncpy_s( name, _countof( name ), pSubstr, pos - pSubstr );
				strncpy_s( value, _countof( value ), pos + 1, substrLen - ( pos - pSubstr ) );

				//if (this->b.count(name) == 0)
				//{
				//	pSubstr += (substrLen + 1);
				//	continue;
				//}

				this->b[ name ] = atoi( value );
			}

			pSubstr += ( substrLen + 1 );
		}
	}

	if( ( 0 < i_read ) && ( ( 65536 - 2 ) > i_read ) ) {
		const char* pSubstr = i_buffer;

		while( '\0' != *pSubstr ) {
			size_t substrLen = strlen( pSubstr );

			const char* pos = strchr( pSubstr, '=' );
			if( NULL != pos ) {
				char name[ 256 ] = "";
				char value[ 256 ] = "";

				strncpy_s( name, _countof( name ), pSubstr, pos - pSubstr );
				strncpy_s( value, _countof( value ), pos + 1, substrLen - ( pos - pSubstr ) );


				//if (i.count(name) == 0)
				//{
				//	pSubstr += (substrLen + 1);
				//	continue;
				//}


				i[ name ] = atoi( value );
			}

			pSubstr += ( substrLen + 1 );
		}
	}

	if( ( 0 < f_read ) && ( ( 65536 - 2 ) > f_read ) ) {
		const char* pSubstr = f_buffer;

		while( '\0' != *pSubstr ) {
			size_t substrLen = strlen( pSubstr );

			const char* pos = strchr( pSubstr, '=' );
			if( NULL != pos ) {
				char name[ 256 ] = "";
				char value[ 256 ] = "";

				strncpy_s( name, _countof( name ), pSubstr, pos - pSubstr );
				strncpy_s( value, _countof( value ), pos + 1, substrLen - ( pos - pSubstr ) );

				//if (f.count(name) == 0)
				//{
				//	pSubstr += (substrLen + 1);
				//	continue;
				//}

				f[ name ] = atof( value );
			}

			pSubstr += ( substrLen + 1 );
		}
	}

	if( ( 0 < c_read ) && ( ( 65536 - 2 ) > c_read ) ) {
		const char* pSubstr = c_buffer;

		while( '\0' != *pSubstr ) {
			size_t substrLen = strlen( pSubstr );

			const char* pos = strchr( pSubstr, '=' );
			if( NULL != pos ) {
				char name[ 256 ] = "";
				char value[ 256 ] = "";

				strncpy_s( name, _countof( name ), pSubstr, pos - pSubstr );
				strncpy_s( value, _countof( value ), pos + 1, substrLen - ( pos - pSubstr ) );

				//if (c.count(name) == 0)
				//{
				//	pSubstr += (substrLen + 1);
				//	continue;
				//}

				auto col = string_to_color( value );
				c[ name ][ 0 ] = col[ 0 ];
				c[ name ][ 1 ] = col[ 1 ];
				c[ name ][ 2 ] = col[ 2 ];
				c[ name ][ 3 ] = col[ 3 ];
			}

			pSubstr += ( substrLen + 1 );
		}
	}

	static auto split = [ ]( std::string str, const char* del ) -> std::vector<std::string> {
		char* pTempStr = _strdup( str.c_str( ) );
		char* pWord = strtok( pTempStr, del );
		std::vector<std::string> dest;

		while( pWord != NULL ) {
			dest.push_back( pWord );
			pWord = strtok( NULL, del );
		}

		free( pTempStr );

		return dest;
	};

	if( ( 0 < m_read ) && ( ( 65536 - 2 ) > m_read ) ) {
		const char* pSubstr = m_buffer;

		while( '\0' != *pSubstr ) {
			size_t substrLen = strlen( pSubstr );

			const char* pos = strchr( pSubstr, '=' );
			if( NULL != pos ) {
				char name[ 256 ] = "";
				char value[ 256 ] = "";

				strncpy_s( name, _countof( name ), pSubstr, pos - pSubstr );
				strncpy_s( value, _countof( value ), pos + 1, substrLen - ( pos - pSubstr ) );

				std::vector<std::string> kvpa = split( value, "|" );
				std::unordered_map<int, bool> vl = {};
				for( auto kvp : kvpa ) {
					if( kvp == "" )
						continue;

					std::vector<std::string> kv = split( kvp, ":" );
					vl[ std::stoi( kv.at( 0 ) ) ] = std::stoi( kv.at( 1 ) );
				}

				m[ name ] = vl;
			}

			pSubstr += ( substrLen + 1 );
		}
	}

	this->load_keys( );
}

#pragma optimize( "", on ) //‚Ë‰‡Î Í‡Í‡ˇ ıÈÛÌˇ ÂÒÚ¸ı‡ı‡ı‡

void Hooks::load_keys( ) {
	for( int k = 0; k < presets.size( ); k++ ) {
		char buffer[ 32 ] = { 0 }; sprintf( buffer, XOR( "_preset_%i" ), k );
		i[ buffer ] = GetPrivateProfileIntA( XOR( "k" ), buffer, 0, XOR( "C:/cumhook/cfg/eys.idb" ) );
	}
}

void Hooks::save_keys( ) {
	for( int k = 0; k < presets.size( ); k++ ) {
		char buffer[ 32 ] = { 0 }; sprintf( buffer, XOR( "_preset_%i" ), k );
		char value[ 32 ] = { 0 }; sprintf( value, XOR( "%i" ), i[ buffer ] );
		WritePrivateProfileStringA( XOR( "k" ), buffer, value, XOR( "C:/cumhook/cfg/keys.idb" ) );
	}
}

void Hooks::load_defaults( ) {
	int _preset = this->i[ XOR( "_preset" ) ];

	static bool init = false;


	if( !init ) {
		init = true;
		b = std::unordered_map<std::string, bool>( );
		i = std::unordered_map<std::string, int>( );
		f = std::unordered_map<std::string, float>( );
		c = std::unordered_map<std::string, float[ 4 ]>( );
		m = std::unordered_map<std::string, std::unordered_map<int, bool>>( );
	}

	for( auto& it : this->b ) {
		// Do stuff
		it.second = false;
	}

	for( auto& it : this->i ) {
		// Do stuff
		it.second = 0;
	}
	for( auto& it : this->f ) {
		// Do stuff
		it.second = 0.f;
	}
	for( auto& it : this->c ) {
		// Do stuff
		for( int i = 0; i < 4; i++ ) it.second[ i ] = 0.f;
	}
	for( auto& it : this->m ) {
		// Do stuff
		for( auto& it2 : it.second ) {
			it2.second = false;
		}
	}


	i[ XOR( "_preset" ) ] = _preset;

	c[ XOR( "menu_color" ) ][ 0 ] = 147 / 255.f;
	c[ XOR( "menu_color" ) ][ 1 ] = 197 / 255.f;
	c[ XOR( "menu_color" ) ][ 2 ] = 57 / 255.f;
	c[ XOR( "menu_color" ) ][ 3 ] = 1.00f;

	c[ XOR( "boundingBox_color" ) ][ 0 ] = 1.f;
	c[ XOR( "boundingBox_color" ) ][ 1 ] = 1.f;
	c[ XOR( "boundingBox_color" ) ][ 2 ] = 1.f;
	c[ XOR( "boundingBox_color" ) ][ 3 ] = 1.f;

	c[ XOR( "name_color" ) ][ 0 ] = 1.f;
	c[ XOR( "name_color" ) ][ 1 ] = 1.f;
	c[ XOR( "name_color" ) ][ 2 ] = 1.f;
	c[ XOR( "name_color" ) ][ 3 ] = 1.f;

	c[ XOR( "weaponText_color" ) ][ 0 ] = 1.f;
	c[ XOR( "weaponText_color" ) ][ 1 ] = 1.f;
	c[ XOR( "weaponText_color" ) ][ 2 ] = 1.f;
	c[ XOR( "weaponText_color" ) ][ 3 ] = 1.f;

	c[ XOR( "weaponIcon_color" ) ][ 0 ] = 1.f;
	c[ XOR( "weaponIcon_color" ) ][ 1 ] = 1.f;
	c[ XOR( "weaponIcon_color" ) ][ 2 ] = 1.f;
	c[ XOR( "weaponIcon_color" ) ][ 3 ] = 1.f;

	c[ XOR( "ammo_color" ) ][ 0 ] = 80 / 255.f;
	c[ XOR( "ammo_color" ) ][ 1 ] = 140 / 255.f;
	c[ XOR( "ammo_color" ) ][ 2 ] = 200 / 255.f;
	c[ XOR( "ammo_color" ) ][ 3 ] = 1.00f;

	c[ XOR( "lby_color" ) ][ 0 ] = 180 / 255.f;
	c[ XOR( "lby_color" ) ][ 1 ] = 60 / 255.f;
	c[ XOR( "lby_color" ) ][ 2 ] = 120 / 255.f;
	c[ XOR( "lby_color" ) ][ 3 ] = 1.00f;

	c[ XOR( "glow_color" ) ][ 0 ] = 180 / 255.f;
	c[ XOR( "glow_color" ) ][ 1 ] = 60 / 255.f;
	c[ XOR( "glow_color" ) ][ 2 ] = 120 / 255.f;
	c[ XOR( "glow_color" ) ][ 3 ] = 0.7f;

	c[ XOR( "skeleton_color" ) ][ 0 ] = 1.f;
	c[ XOR( "skeleton_color" ) ][ 1 ] = 1.f;
	c[ XOR( "skeleton_color" ) ][ 2 ] = 1.f;
	c[ XOR( "skeleton_color" ) ][ 3 ] = 1.f;

	c[ XOR( "outOfFOVArrow_color" ) ][ 0 ] = 1.f;
	c[ XOR( "outOfFOVArrow_color" ) ][ 1 ] = 1.f;
	c[ XOR( "outOfFOVArrow_color" ) ][ 2 ] = 1.f;
	c[ XOR( "outOfFOVArrow_color" ) ][ 3 ] = 1.f;

	c[ XOR( "hitmarker_quake_c" ) ][ 0 ] = 1.f;
	c[ XOR( "hitmarker_quake_c" ) ][ 1 ] = 1.f;
	c[ XOR( "hitmarker_quake_c" ) ][ 2 ] = 1.f;
	c[ XOR( "hitmarker_quake_c" ) ][ 3 ] = 1.f;

	c[ XOR( "hitmarker_3d_c" ) ][ 0 ] = 1.f;
	c[ XOR( "hitmarker_3d_c" ) ][ 1 ] = 1.f;
	c[ XOR( "hitmarker_3d_c" ) ][ 2 ] = 1.f;
	c[ XOR( "hitmarker_3d_c" ) ][ 3 ] = 1.f;

	c[ XOR( "lagcomp_degug_c" ) ][ 0 ] = 47 / 255.f;
	c[ XOR( "lagcomp_degug_c" ) ][ 1 ] = 117 / 255.f;
	c[ XOR( "lagcomp_degug_c" ) ][ 2 ] = 221 / 255.f;
	c[ XOR( "lagcomp_degug_c" ) ][ 3 ] = 1.f;

	c[ XOR( "player_color" ) ][ 0 ] = 150 / 255.f;
	c[ XOR( "player_color" ) ][ 1 ] = 200 / 255.f;
	c[ XOR( "player_color" ) ][ 2 ] = 60 / 255.f;
	c[ XOR( "player_color" ) ][ 3 ] = 1.00f;

	c[ XOR( "playerBehindWall_color" ) ][ 0 ] = 60 / 255.f;
	c[ XOR( "playerBehindWall_color" ) ][ 1 ] = 120 / 255.f;
	c[ XOR( "playerBehindWall_color" ) ][ 2 ] = 180 / 255.f;
	c[ XOR( "playerBehindWall_color" ) ][ 3 ] = 1.00f;

	c[ XOR( "playerReflectivityColor" ) ][ 0 ] = 255 / 255.f;
	c[ XOR( "playerReflectivityColor" ) ][ 1 ] = 178 / 255.f;
	c[ XOR( "playerReflectivityColor" ) ][ 2 ] = 0 / 255.f;
	c[ XOR( "playerReflectivityColor" ) ][ 3 ] = 1.f;

	c[ XOR( "teammates_chams_color" ) ][ 0 ] = 1.f;
	c[ XOR( "teammates_chams_color" ) ][ 1 ] = 1.f;
	c[ XOR( "teammates_chams_color" ) ][ 2 ] = 1.f;
	c[ XOR( "teammates_chams_color" ) ][ 3 ] = 1.f;

	c[ XOR( "debug_aim_color" ) ][ 0 ] = 1.f;
	c[ XOR( "debug_aim_color" ) ][ 1 ] = 1.f;
	c[ XOR( "debug_aim_color" ) ][ 2 ] = 1.f;
	c[ XOR( "debug_aim_color" ) ][ 3 ] = 1.f;

	c[ XOR( "hands_color" ) ][ 0 ] = 1.f;
	c[ XOR( "hands_color" ) ][ 1 ] = 1.f;
	c[ XOR( "hands_color" ) ][ 2 ] = 1.f;
	c[ XOR( "hands_color" ) ][ 3 ] = 1.f;

	c[ XOR( "shadow_color" ) ][ 0 ] = 1.f;
	c[ XOR( "shadow_color" ) ][ 1 ] = 1.f;
	c[ XOR( "shadow_color" ) ][ 2 ] = 1.f;
	c[ XOR( "shadow_color" ) ][ 3 ] = 1.f;

	c[ XOR( "console_color" ) ][ 0 ] = 50 / 255.f;
	c[ XOR( "console_color" ) ][ 1 ] = 50 / 255.f;
	c[ XOR( "console_color" ) ][ 2 ] = 50 / 255.f;
	c[ XOR( "console_color" ) ][ 3 ] = 0.5f;

	c[ XOR( "localFakeShadow_color" ) ][ 0 ] = 1.f;
	c[ XOR( "localFakeShadow_color" ) ][ 1 ] = 1.f;
	c[ XOR( "localFakeShadow_color" ) ][ 2 ] = 1.f;
	c[ XOR( "localFakeShadow_color" ) ][ 3 ] = 1.f;

	c[ XOR( "localchmas_color" ) ][ 0 ] = 1.f;
	c[ XOR( "localchmas_color" ) ][ 1 ] = 1.f;
	c[ XOR( "localchmas_color" ) ][ 2 ] = 1.f;
	c[ XOR( "localchmas_color" ) ][ 3 ] = 1.f;

	c[ XOR( "glowGrenades_color" ) ][ 0 ] = 1.f;
	c[ XOR( "glowGrenades_color" ) ][ 1 ] = 1.f;
	c[ XOR( "glowGrenades_color" ) ][ 2 ] = 1.f;
	c[ XOR( "glowGrenades_color" ) ][ 3 ] = 1.f;

	c[ XOR( "inaccuracyOverlay_color" ) ][ 0 ] = 1.f;
	c[ XOR( "inaccuracyOverlay_color" ) ][ 1 ] = 1.f;
	c[ XOR( "inaccuracyOverlay_color" ) ][ 2 ] = 1.f;
	c[ XOR( "inaccuracyOverlay_color" ) ][ 3 ] = 1.f;

	c[ XOR( "bomb_color" ) ][ 0 ] = 1.f;
	c[ XOR( "bomb_color" ) ][ 1 ] = 1.f;
	c[ XOR( "bomb_color" ) ][ 2 ] = 1.f;
	c[ XOR( "bomb_color" ) ][ 3 ] = 1.f;

	//XOR render::line( screen0, screen1, { 60, 180, 225 } );
	c[ XOR( "grenadeTrajectory_color" ) ][ 0 ] = 60.f / 255.f;
	c[ XOR( "grenadeTrajectory_color" ) ][ 1 ] = 180.f / 255.f;
	c[ XOR( "grenadeTrajectory_color" ) ][ 2 ] = 225.f / 255.f;
	c[ XOR( "grenadeTrajectory_color" ) ][ 3 ] = 255.f / 255.f;

	c[ XOR( "grenadeWarning_enemy_color" ) ][ 0 ] = 1.f;
	c[ XOR( "grenadeWarning_enemy_color" ) ][ 1 ] = 0.f;
	c[ XOR( "grenadeWarning_enemy_color" ) ][ 2 ] = 0.f;
	c[ XOR( "grenadeWarning_enemy_color" ) ][ 3 ] = 1.f;

	c[ XOR( "grenadeWarning_teammates_color" ) ][ 0 ] = 0.f;
	c[ XOR( "grenadeWarning_teammates_color" ) ][ 1 ] = 0.f;
	c[ XOR( "grenadeWarning_teammates_color" ) ][ 2 ] = 1.f;
	c[ XOR( "grenadeWarning_teammates_color" ) ][ 3 ] = 1.f;

	c[ XOR( "grenadeWarning_local_color" ) ][ 0 ] = 1.f;
	c[ XOR( "grenadeWarning_local_color" ) ][ 1 ] = 1.f;
	c[ XOR( "grenadeWarning_local_color" ) ][ 2 ] = 1.f;
	c[ XOR( "grenadeWarning_local_color" ) ][ 3 ] = 1.f;

	c[ XOR( "grenadeWarning_color" ) ][ 0 ] = 1.f;
	c[ XOR( "grenadeWarning_color" ) ][ 1 ] = 0.f;
	c[ XOR( "grenadeWarning_color" ) ][ 2 ] = 0.f;
	c[ XOR( "grenadeWarning_color" ) ][ 3 ] = 1.f;

	c[ XOR( "droppedWeaponsText_color" ) ][ 0 ] = 1.f;
	c[ XOR( "droppedWeaponsText_color" ) ][ 1 ] = 1.f;
	c[ XOR( "droppedWeaponsText_color" ) ][ 2 ] = 1.f;
	c[ XOR( "droppedWeaponsText_color" ) ][ 3 ] = 1.f;

	c[ XOR( "droppedWeaponsDistance_color" ) ][ 0 ] = 1.f;
	c[ XOR( "droppedWeaponsDistance_color" ) ][ 1 ] = 1.f;
	c[ XOR( "droppedWeaponsDistance_color" ) ][ 2 ] = 1.f;
	c[ XOR( "droppedWeaponsDistance_color" ) ][ 3 ] = 1.f;

	c[ XOR( "ambient_ligtning_color" ) ][ 0 ] = 1.f;
	c[ XOR( "ambient_ligtning_color" ) ][ 1 ] = 1.f;
	c[ XOR( "ambient_ligtning_color" ) ][ 2 ] = 1.f;
	c[ XOR( "ambient_ligtning_color" ) ][ 3 ] = 1.f;

	c[ XOR( "droppedWeaponsAmmo_color" ) ][ 0 ] = 80 / 255.f;
	c[ XOR( "droppedWeaponsAmmo_color" ) ][ 1 ] = 140 / 255.f;
	c[ XOR( "droppedWeaponsAmmo_color" ) ][ 2 ] = 200 / 255.f;
	c[ XOR( "droppedWeaponsAmmo_color" ) ][ 3 ] = 1.00f;

	c[ XOR( "bulletTracers_color" ) ][ 0 ] = 95 / 255.f;
	c[ XOR( "bulletTracers_color" ) ][ 1 ] = 174 / 255.f;
	c[ XOR( "bulletTracers_color" ) ][ 2 ] = 227 / 255.f;
	c[ XOR( "bulletTracers_color" ) ][ 3 ] = 1.00f;

	c[ XOR( "grenades_color" ) ][ 0 ] = 1.f;
	c[ XOR( "grenades_color" ) ][ 1 ] = 1.f;
	c[ XOR( "grenades_color" ) ][ 2 ] = 1.f;
	c[ XOR( "grenades_color" ) ][ 3 ] = 1.f;

	c[ XOR( "handsReflectivityColor" ) ][ 0 ] = 255 / 255.f;
	c[ XOR( "handsReflectivityColor" ) ][ 1 ] = 178 / 255.f;
	c[ XOR( "handsReflectivityColor" ) ][ 2 ] = 0 / 255.f;
	c[ XOR( "handsReflectivityColor" ) ][ 3 ] = 1.00f;

	c[ XOR( "playerglowColor" ) ][ 0 ] = 1.f;
	c[ XOR( "playerglowColor" ) ][ 1 ] = 1.f;
	c[ XOR( "playerglowColor" ) ][ 2 ] = 1.f;
	c[ XOR( "playerglowColor" ) ][ 3 ] = 1.f;

	c[ XOR( "handsglowColor" ) ][ 0 ] = 1.f;
	c[ XOR( "handsglowColor" ) ][ 1 ] = 1.f;
	c[ XOR( "handsglowColor" ) ][ 2 ] = 1.f;
	c[ XOR( "handsglowColor" ) ][ 3 ] = 1.f;

	c[ XOR( "scope_line_color" ) ][ 0 ] = 80 / 255.f;
	c[ XOR( "scope_line_color" ) ][ 1 ] = 140 / 255.f;
	c[ XOR( "scope_line_color" ) ][ 2 ] = 200 / 255.f;
	c[ XOR( "scope_line_color" ) ][ 3 ] = 1.00f;

	c[ XOR( "smoke_color" ) ][ 0 ] = 80 / 255.f;
	c[ XOR( "smoke_color" ) ][ 1 ] = 140 / 255.f;
	c[ XOR( "smoke_color" ) ][ 2 ] = 200 / 255.f;
	c[ XOR( "smoke_color" ) ][ 3 ] = 1.00f;

	c[ XOR( "molotv_color" ) ][ 0 ] = 1.f;
	c[ XOR( "molotv_color" ) ][ 1 ] = 0.f;
	c[ XOR( "molotv_color" ) ][ 2 ] = 0.f;
	c[ XOR( "molotv_color" ) ][ 3 ] = 1.f;

	c[ XOR( "manual_aa_color" ) ][ 0 ] = 1.f;
	c[ XOR( "manual_aa_color" ) ][ 1 ] = 0.f;
	c[ XOR( "manual_aa_color" ) ][ 2 ] = 0.f;
	c[ XOR( "manual_aa_color" ) ][ 3 ] = 1.f;

	c[ XOR( "nightmode_color" ) ][ 0 ] = 0.17f;
	c[ XOR( "nightmode_color" ) ][ 1 ] = 0.16f;
	c[ XOR( "nightmode_color" ) ][ 2 ] = 0.18f;
	c[ XOR( "nightmode_color" ) ][ 3 ] = 1.f;

	c[ XOR( "vis_soundesp_color" ) ][ 0 ] = colors::purple.r( ) / 255.f;
	c[ XOR( "vis_soundesp_color" ) ][ 1 ] = colors::purple.g( ) / 255.f;
	c[ XOR( "vis_soundesp_color" ) ][ 2 ] = colors::purple.b( ) / 255.f;
	c[ XOR( "vis_soundesp_color" ) ][ 3 ] = 1.00f;

	c[ XOR( "vis_aimbot_color" ) ][ 0 ] = 1.f;
	c[ XOR( "vis_aimbot_color" ) ][ 1 ] = 1.f;
	c[ XOR( "vis_aimbot_color" ) ][ 2 ] = 1.f;
	c[ XOR( "vis_aimbot_color" ) ][ 3 ] = 1.f;

	c[ XOR( "auto_peek_active" ) ][ 0 ] = 30 / 255.f;
	c[ XOR( "auto_peek_active" ) ][ 1 ] = 240 / 255.f;
	c[ XOR( "auto_peek_active" ) ][ 2 ] = 30 / 255.f;
	c[ XOR( "auto_peek_active" ) ][ 3 ] = 1.00f;

	c[ XOR( "auto_peek_inactive" ) ][ 0 ] = 240 / 255.f;
	c[ XOR( "auto_peek_inactive" ) ][ 1 ] = 30 / 255.f;
	c[ XOR( "auto_peek_inactive" ) ][ 2 ] = 30 / 255.f;
	c[ XOR( "auto_peek_inactive" ) ][ 3 ] = 1.00f;

	i[ XOR( "selc" ) ] = 0;
	i[ XOR( "handsMaterial" ) ] = 0;
	i[ XOR( "weaponsMaterial" ) ] = 0;
	i[ XOR( "brightnessAdjustment_sel" ) ] = 0;
	i[ XOR( "thirdperson_key" ) ] = 0;
	i[ XOR( "airStrafeType_sel" ) ] = 0;
	i[ XOR( "damage_keystyle" ) ] = 1.f;
	i[ XOR( "damage1_keystyle" ) ] = 1.f;
	i[ XOR( "auto_peek_keystyle" ) ] = 1.f;
	i[ XOR( "damage_keystyle_scout" ) ] = 1.f;
	i[ XOR( "damage1_keystyle_scout" ) ] = 1.f;
	i[ XOR( "damage_keystyle_awp" ) ] = 1.f;
	i[ XOR( "damage1_keystyle_awp" ) ] = 1.f;
	i[ XOR( "damage_keystyle_deagle" ) ] = 1.f;
	i[ XOR( "damage1_keystyle_deagle" ) ] = 1.f;
	i[ XOR( "damage_keystyle_pistols" ) ] = 1.f;
	i[ XOR( "damage1_keystyle_pistols" ) ] = 1.f;
	i[ XOR( "damage_keystyle_revolver" ) ] = 1.f;
	i[ XOR( "damage1_keystyle_revolver" ) ] = 1.f;
	i[ XOR( "quickstop_keystyle" ) ] = 0.f;
	i[ XOR( "thirdperson_keystyle" ) ] = 2.f;
	i[ XOR( "fake_latency_keystyle" ) ] = 2.f;
	f[ XOR( "interp_ammount" ) ] = 1.f;

	f[ XOR( "scope_line_height" ) ] = 300.f;
	f[ XOR( "scope_line_offset" ) ] = 100.f;
	f[ XOR( "scope_line_speed" ) ] = 70.f;
	f[ XOR( "thirdperson_distance" ) ] = 150.f;
	f[ XOR( "transparentWalls" ) ] = 100.f;
	f[ XOR( "transparentProps" ) ] = 100.f;
	f[ XOR( "overrideFov" ) ] = 90.f;
	f[ XOR( "overrideViewmodelFov" ) ] = 60.f;
	f[ XOR( "fov_amount" ) ] = 180.f;
	f[ XOR( "scale" ) ] = 80.f;
	f[ XOR( "scale_scout" ) ] = 70.f;
	f[ XOR( "scale_awp" ) ] = 70.f;
	f[ XOR( "scale_deagle" ) ] = 70.f;
	f[ XOR( "scale_pistols" ) ] = 70.f;
	f[ XOR( "scale_revolver" ) ] = 70.f;
	f[ XOR( "target_limit_f" ) ] = 4.f;
	f[ XOR( "baim_hp_override" ) ] = 1.f;
	f[ XOR( "baim_hp_override_scout" ) ] = 1.f;
	f[ XOR( "baim_hp_override_awp" ) ] = 1.f;
	f[ XOR( "baim_hp_override_deagle" ) ] = 1.f;
	f[ XOR( "baim_hp_override_pistols" ) ] = 1.f;
	f[ XOR( "baim_hp_override_revolver" ) ] = 1.f;
	f[ XOR( "misses" ) ] = 2.f;
	f[ XOR( "misses_scout" ) ] = 2.f;
	f[ XOR( "misses_awp" ) ] = 2.f;
	f[ XOR( "misses_deagle" ) ] = 2.f;
	f[ XOR( "misses_pistols" ) ] = 2.f;
	f[ XOR( "misses_revolver" ) ] = 2.f;
	f[ XOR( "fake_latency_amt" ) ] = 700.f;
	f[ XOR( "z_freq" ) ] = 50.f;
	f[ XOR( "z_dist" ) ] = 20.f;
	f[ XOR( "zeusbot_hc" ) ] = 60.f;
	f[ XOR( "body_scale" ) ] = 70.f;
	f[ XOR( "body_scale_scout" ) ] = 40.f;
	f[ XOR( "body_scale_awp" ) ] = 70.f;
	f[ XOR( "body_scale_deagle" ) ] = 70.f;
	f[ XOR( "body_scale_pistols" ) ] = 70.f;
	f[ XOR( "body_scale_revolver" ) ] = 70.f;
	f[ XOR( "minimal_damage" ) ] = 30.f;
	f[ XOR( "minimal_damage_scout" ) ] = 80.f;
	f[ XOR( "minimal_damage_awp" ) ] = 100.f;
	f[ XOR( "minimal_damage_deagle" ) ] = 50.f;
	f[ XOR( "minimal_damage_pistols" ) ] = 50.f;
	f[ XOR( "minimal_damage_revolver" ) ] = 50.f;
	f[ XOR( "penetrate_minimal_damage" ) ] = 20.f;
	f[ XOR( "penetrate_minimal_damage_scout" ) ] = 80.f;
	f[ XOR( "penetrate_minimal_damage_awp" ) ] = 100.f;
	f[ XOR( "penetrate_minimal_damage_deagle" ) ] = 70.f;
	f[ XOR( "penetrate_minimal_damage_pistols" ) ] = 70.f;
	f[ XOR( "penetrate_minimal_damage_revolver" ) ] = 70.f;
	f[ XOR( "hitchance_amount" ) ] = 50.f;
	f[ XOR( "hitchance_amount_scout" ) ] = 80.f;
	f[ XOR( "hitchance_amount_awp" ) ] = 90.f;
	f[ XOR( "hitchance_amount_deagle" ) ] = 60.f;
	f[ XOR( "hitchance_amount_pistols" ) ] = 60.f;
	f[ XOR( "hitchance_amount_revolver" ) ] = 60.f;
	f[ XOR( "baim_hp" ) ] = 20.f;
	f[ XOR( "baim_hp_scout" ) ] = 20.f;
	f[ XOR( "baim_hp_awp" ) ] = 20.f;
	f[ XOR( "baim_hp_deagle" ) ] = 20.f;
	f[ XOR( "baim_hp_pistols" ) ] = 20.f;
	f[ XOR( "baim_hp_revolver" ) ] = 20.f;
	f[ XOR( "arrowSize" ) ] = 20.f;
	f[ XOR( "arrowDistance" ) ] = 100.f;
	f[ XOR( "ReflectivityColor_da" ) ] = 1.f;
	f[ XOR( "fakelag_variance" ) ] = 0.f;
	f[ XOR( "fakelag_limit" ) ] = 2.f;
	f[ XOR( "yaw_add" ) ] = 0.f;
	f[ XOR( "yaw_move_add" ) ] = 0.f;
	f[ XOR( "fakeyaw_add" ) ] = 0.f;
	f[ XOR( "freestand_add" ) ] = 0.f;
	f[ XOR( "shine_f" ) ] = 1.f;
	f[ XOR( "rim_f" ) ] = 1.f;
	f[ XOR( "reflectivity_f" ) ] = 1.f;
	f[ XOR( "shine_hands" ) ] = 1.f;
	f[ XOR( "rim_hands" ) ] = 1.f;
	f[ XOR( "reflectivity_hands" ) ] = 1.f;
	f[ XOR( "speed_l" ) ] = 0.1f;
	f[ XOR( "speed_l_a" ) ] = 0.1f;
	f[ XOR( "mindmg_tr" ) ] = 1.f;
	f[ XOR( "debug_aim_time" ) ] = 10.f;
	f[ XOR( "distortion_speed" ) ] = 50.f;
	f[ XOR( "slowMotion_speed" ) ] = 100.f;

	// skins
	f[ XOR( "skin_UNKNOWN_id" ) ] = 100.f;
	f[ XOR( "skin_DEAGLE_quality" ) ] = 100.f;
	f[ XOR( "skin_ELITE_quality" ) ] = 100.f;
	f[ XOR( "skin_FIVESEVEN_quality" ) ] = 100.f;
	f[ XOR( "skin_GLOCK_quality" ) ] = 100.f;
	f[ XOR( "skin_AK47_quality" ) ] = 100.f;
	f[ XOR( "skin_AUG_quality" ) ] = 100.f;
	f[ XOR( "skin_AWP_quality" ) ] = 100.f;
	f[ XOR( "skin_FAMAS_quality" ) ] = 100.f;
	f[ XOR( "skin_G3SG1_quality" ) ] = 100.f;
	f[ XOR( "skin_GALIL_quality" ) ] = 100.f;
	f[ XOR( "skin_M249_quality" ) ] = 100.f;
	f[ XOR( "skin_M4A4_quality" ) ] = 100.f;
	f[ XOR( "skin_MAC10_quality" ) ] = 100.f;
	f[ XOR( "skin_P90_quality" ) ] = 100.f;
	f[ XOR( "skin_UMP45_quality" ) ] = 100.f;
	f[ XOR( "skin_XM1014_quality" ) ] = 100.f;
	f[ XOR( "skin_BIZON_quality" ) ] = 100.f;
	f[ XOR( "skin_MAG7_quality" ) ] = 100.f;
	f[ XOR( "skin_NEGEV_quality" ) ] = 100.f;
	f[ XOR( "skin_SAWEDOFF_quality" ) ] = 100.f;
	f[ XOR( "skin_TEC9_quality" ) ] = 100.f;
	f[ XOR( "skin_P2000_quality" ) ] = 100.f;
	f[ XOR( "skin_MP7_quality" ) ] = 100.f;
	f[ XOR( "skin_MP9_quality" ) ] = 100.f;
	f[ XOR( "skin_NOVA_quality" ) ] = 100.f;
	f[ XOR( "skin_P250_quality" ) ] = 100.f;
	f[ XOR( "skin_SCAR20_quality" ) ] = 100.f;
	f[ XOR( "skin_SG553_quality" ) ] = 100.f;
	f[ XOR( "skin_SSG08_quality" ) ] = 100.f;
	f[ XOR( "skin_M4A1S_quality" ) ] = 100.f;
	f[ XOR( "skin_USPS_quality" ) ] = 100.f;
	f[ XOR( "skin_CZ75A_quality" ) ] = 100.f;
	f[ XOR( "skin_REVOLVER_quality" ) ] = 100.f;
	f[ XOR( "skin_KNIFE_BAYONET_quality" ) ] = 100.f;
	f[ XOR( "skin_KNIFE_FLIP_quality" ) ] = 100.f;
	f[ XOR( "skin_KNIFE_GUT_quality" ) ] = 100.f;
	f[ XOR( "skin_KNIFE_KARAMBIT_quality" ) ] = 100.f;
	f[ XOR( "skin_KNIFE_M9_BAYONET_quality" ) ] = 100.f;
	f[ XOR( "skin_KNIFE_HUNTSMAN_quality" ) ] = 100.f;
	f[ XOR( "skin_KNIFE_FALCHION_quality" ) ] = 100.f;
	f[ XOR( "skin_KNIFE_BOWIE_quality" ) ] = 100.f;
	f[ XOR( "skin_KNIFE_BUTTERFLY_quality" ) ] = 100.f;
	f[ XOR( "skin_KNIFE_SHADOW_DAGGERS_quality" ) ] = 100.f;
	//XORb. = false;
	b[ XOR( "antiUntrusted" ) ] = true;
}

bool Hooks::is_key_down( int key ) {
	return HIWORD( GetKeyState( key ) );
}
bool Hooks::is_key_up( int key ) {
	return !HIWORD( GetKeyState( key ) );
}

bool Hooks::is_key_pressed( int key ) {
	return false;
}

int Hooks::getping( ) {
	float latency = 0.f;
	INetChannel* nci = g_csgo.m_engine->GetNetChannelInfo( );
	if( nci ) {
		latency = nci->GetAvgLatency( INetChannel::FLOW_INCOMING ) + nci->GetAvgLatency( INetChannel::FLOW_OUTGOING );

	}
	static auto cl_updaterate = g_csgo.cl_updaterate;
	latency -= 0.5f / cl_updaterate->GetFloat( );

	auto ping = ( std::max( 0.0f, latency ) * 1000.0f );


	return ping;
}
//void dmt(std::string key) {
//	if ( ImGui::IsItemHovered()) {
//		ImGui::BeginTooltip();
//		ImGui::Text(key.c_str());
//		ImGui::EndTooltip();
//	}
//}
bool Hooks::auto_check( std::string key ) {
	switch( i[ key + XOR( "style" ) ] ) {
		case 0:
			return true;
		case 1:
			return is_key_down( i[ key ] );
		case 2:
			return LOWORD( GetKeyState( i[ key ] ) );
		case 3:
			return is_key_up( i[ key ] );
		default:
			return true;
	}
}

void Hooks::Aimbot( ) {
	std::vector<const char*> hitbox_type = {
		XOR( "Head" ),
		XOR( "Chest" ),
		XOR( "Stomach" ),
		XOR( "Arms" ),
		XOR( "Legs" ),
		XOR( "Feet" )
	};
	std::vector<const char*> selection_type = {
		( "Cycle" ),
		( "Near crosshair" ),
		( "Highest damage" ),
		( "Lowest health" ),
		( "Lowest ping" ),
		( "Height" )
	};
	std::vector<const char*> multipoint_type = {
		XOR( "Head" ),
		XOR( "Chest" ),
		XOR( "Body" ),
		XOR( "Legs" )
	};
	std::vector<const char*> zoom_type = {
		XOR( "Off" ),
		XOR( "Always" ),
		XOR( "Hitchance fail" )
	};
	std::vector<const char*> baim1_type = {
		XOR( "Always" ),
		XOR( "Lethal" ),
		XOR( "Lethal x2" ),
		XOR( "Fake" ),
		XOR( "In air" ),
		XOR( "After x misses" )
	};
	std::vector<const char*> baim2_type = {
		XOR( "Always" ),
		XOR( "Health" ),
		XOR( "Fake" ),
		XOR( "In air" ),
		XOR( "After x misses" )
	};
	std::vector<const char*> head1_type = {
		XOR( "always" ),
		XOR( "running" ),
		XOR( "resolved" ),
		XOR( "in air" )
	};
	std::vector<const char*> fakelag_type = {
		XOR( "On move" ),
		XOR( "On jump" ),
		XOR( "On crouch" ),
		XOR( "On peek" ),
		XOR( "On stand" ),
		XOR( "On high speed" )
	};
	std::vector<const char*> fakelag_mode = {
		XOR( "Dynamic" ),
		XOR( "Maximum" ),
		XOR( "Break" ),
		XOR( "Fluctuate" )
	};

	std::vector<const char*> quickstop_mode = {
		XOR( "Off" ),
		XOR( "On" ),
		XOR( "On + slide" )
	};

	std::vector<const char*> accuracy_mode = {
		XOR( "Off" ),
		XOR( "Low" ),
		XOR( "Medium" ),
		XOR( "High" ),
		XOR( "Maximum" )
	};

	std::vector<const char*> lagfix_mode = {
		XOR( "Off" ),
		XOR( "Default" ),
		XOR( "Delay shot" )
	};

	std::vector<const char*> pitch_mode = {
		XOR( "Off" ),
		XOR( "Default" ),
		XOR( "Up" ),
		XOR( "Down" ),
		XOR( "Minimal" ),
		XOR( "Random" )
	};

	std::vector<const char*> yaw_mode = {
		XOR( "Off" ),
		XOR( "180" ),
		XOR( "180 jitter" ),
		XOR( "Rotate" ),
		XOR( "Random" ),
		XOR( "Sexy" )
	};

	std::vector<const char*> fakeyaw_mode = {
		XOR( "Off" ),
		XOR( "Default" ), // no
		XOR( "180" ),
		XOR( "180 jitter" ), // no
		XOR( "Rotate" ), // no
		XOR( "Random" ), // no
		XOR( "Local view" ),
		XOR( "Spinbot" ), // no
		XOR( "Reverse lby" ),
		XOR( "Match lby" ),
		XOR( "CumhookYaw" )
	};

	std::vector<const char*> freestand_mode = {
		XOR( "Default" ),
		XOR( "Running" ),
		XOR( "Edge" ),
		XOR( "Fuck freestanding" )
	};

	std::vector<const char*> fakebody_mode = {
		XOR( "Off" ),
		XOR( "Left" ),
		XOR( "Right" ),
		XOR( "Opposite" ),
		XOR( "z" ),
		XOR( "Optimal" ),
		XOR( "Custom" ),
		XOR( "CumhookBody" )
	};

	std::vector<const char*> yawbase_mode = {
		XOR( "Static" ),
		XOR( "Local view" ),
		XOR( "At targets" )
	};

	std::vector<const char*> weapon_mode = {
		XOR( "General" ),
		XOR( "Scout" ),
		XOR( "AWP" ),
		XOR( "Deagle" ),
		XOR( "Pistols" ),
		XOR( "Revolver" )
	};

	std::vector<const char*> distortion_type = {
		XOR( "On stand" ),
		XOR( "On move" ),
		XOR( "On air" ),
		XOR( "On manual" )
	};

	std::vector<const char*> resolver_type = {
		XOR( "Edge" ),
		XOR( "Last move" )
	};

	ImGuiStyle* style = &ImGui::GetStyle( );
	ImVec4* colors = style->Colors;
	InsertSpacer( XOR( "Top Spacer" ) );

	ImGui::Columns( 2, NULL, false );
	{

		InsertGroupBoxLeft( XOR( "Aimbot" ), 287.f );
		{

			style->ItemSpacing = ImVec2( 4, 2 );
			style->WindowPadding = ImVec2( 4, 4 );
			ImGui::CustomSpacing( 9.f );

			//



			//

			InsertCombo( XOR( "Weapon config" ), &i[ XOR( "weapon_cnofig" ) ], weapon_mode );
			InsertCheckbox( XOR( "Enable" ), b[ XOR( "rbotenable" ) ] );
			InsertCombo( XOR( "Target selection" ), &i[ XOR( "selection" ) ], selection_type );
			if( i[ XOR( "weapon_cnofig" ) ] == 0 ) {
				InsertMultiCombo( XOR( "Target hitbox##0" ), &m[ XOR( "hitbox" ) ], hitbox_type );
				InsertCheckbox( XOR( "Avoid limbs if moving##0" ), b[ XOR( "ignor_limbs" ) ] );
				InsertMultiCombos( XOR( "Multi-point##0" ), &m[ XOR( "multipoint" ) ], multipoint_type );
				if( m[ XOR( "multipoint" ) ][ 0 ] ) {
					InsertSliderWithoutText( XOR( "##Hitbox_scale##0" ), f[ XOR( "scale" ) ], 1.f, 100.f, XOR( "%1.f%%" ) );
				}
				else { ImGui::Spacing( ); }
				if( m[ XOR( "multipoint" ) ][ 2 ] ) {
					InsertSlider( XOR( "Body hitbox scale##0" ), f[ XOR( "body_scale" ) ], 1.f, 100.f, XOR( "%1.f%%" ) );
				}
				InsertCheckbox( XOR( "Automatic fire##0" ), b[ XOR( "autofire" ) ] );
				InsertCheckbox( XOR( "Automatic penetration##0" ), b[ XOR( "penetrate" ) ] );
				InsertCheckbox( XOR( "Silent aim##0" ), b[ XOR( "silent" ) ] );
				if( !b[ XOR( "antiUntrusted" ) ] ) { ImGui::PushDisabled( ); }
				InsertCheckbox( XOR( "Minimum hit chance##0" ), b[ XOR( "hitchance" ) ] );
				if( b[ XOR( "hitchance" ) ] ) {
					InsertSliderWithoutText( XOR( "##Hitchance##0" ), f[ XOR( "hitchance_amount" ) ], 1.f, 100.f, XOR( "%1.f%%" ) );
				}
				if( !b[ XOR( "antiUntrusted" ) ] ) { ImGui::PopDisabled( ); }
				if( f[ XOR( "minimal_damage" ) ] <= 100 ) {
					InsertSlider( XOR( "Minimal damage##0" ), f[ XOR( "minimal_damage" ) ], 1.f, 150.f, XOR( "%1.f" ) );
				}
				else {
					int hp = f[ XOR( "minimal_damage" ) ] - 100;

					std::string da = tfm::format( XOR( "HP + %1.f" ), hp );

					InsertSlider( XOR( "Minimal damage##0" ), f[ XOR( "minimal_damage" ) ], 1.f, 150.f, da.c_str( ) );
				}
				InsertCheckbox( XOR( "Scale damage on hp##0" ), b[ XOR( "minimal_damage_on_hp" ) ] );
			}
			else if( i[ XOR( "weapon_cnofig" ) ] == 1 ) {
				InsertMultiCombo( XOR( "Target hitbox##1" ), &m[ XOR( "hitbox_scout" ) ], hitbox_type );
				InsertCheckbox( XOR( "Avoid limbs if moving##1" ), b[ XOR( "ignor_limbs_scout" ) ] );
				InsertMultiCombos( XOR( "Multi-point" ), &m[ XOR( "multipoint_scout" ) ], multipoint_type );
				if( m[ XOR( "multipoint_scout" ) ][ 0 ] ) {
					InsertSliderWithoutText( XOR( "##Hitbox_scale" ), f[ XOR( "scale_scout" ) ], 1.f, 100.f, XOR( "%1.f%%" ) );
				}
				else { ImGui::Spacing( ); }
				if( m[ XOR( "multipoint_scout" ) ][ 2 ] ) {
					InsertSlider( XOR( "Body hitbox scale" ), f[ XOR( "body_scale_scout" ) ], 1.f, 100.f, XOR( "%1.f%%" ) );
				}
				InsertCheckbox( XOR( "Automatic fire" ), b[ XOR( "autofire_scout" ) ] );
				InsertCheckbox( XOR( "Automatic penetration" ), b[ XOR( "penetrate_scout" ) ] );
				InsertCheckbox( XOR( "Silent aim" ), b[ XOR( "silent_scout" ) ] );
				InsertCheckbox( XOR( "Minimum hit chance" ), b[ XOR( "hitchance_scout" ) ] );
				if( b[ XOR( "hitchance_scout" ) ] ) {
					InsertSliderWithoutText( XOR( "##Hitchance" ), f[ XOR( "hitchance_amount_scout" ) ], 1.f, 100.f, XOR( "%1.f%%" ) );
				}
				if( f[ XOR( "minimal_damage_scout" ) ] <= 100 ) {
					InsertSlider( XOR( "Minimal damage" ), f[ XOR( "minimal_damage_scout" ) ], 1.f, 150.f, XOR( "%1.f" ) );
				}
				else {
					int hp = f[ XOR( "minimal_damage_scout" ) ] - 100;

					std::string da = tfm::format( XOR( "HP + %1.f" ), hp );

					InsertSlider( XOR( "Minimal damage" ), f[ XOR( "minimal_damage_scout" ) ], 1.f, 150.f, da.c_str( ) );
				}
				InsertCheckbox( XOR( "Scale damage on hp" ), b[ XOR( "minimal_damage_on_hp_scout" ) ] );
			}
			else if( i[ XOR( "weapon_cnofig" ) ] == 2 ) {
				InsertMultiCombo( XOR( "Target hitbox" ), &m[ XOR( "hitbox_awp" ) ], hitbox_type );
				InsertCheckbox( XOR( "Avoid limbs if moving" ), b[ XOR( "ignor_limbs_awp" ) ] );
				InsertMultiCombos( XOR( "Multi-point" ), &m[ XOR( "multipoint_awp" ) ], multipoint_type );
				if( m[ XOR( "multipoint_awp" ) ][ 0 ] ) {
					InsertSliderWithoutText( XOR( "##Hitbox_scale" ), f[ XOR( "scale_awp" ) ], 1.f, 100.f, XOR( "%1.f%%" ) );
				}
				else { ImGui::Spacing( ); }
				if( m[ XOR( "multipoint_awp" ) ][ 2 ] ) {
					InsertSlider( XOR( "Body hitbox scale" ), f[ XOR( "body_scale_awp" ) ], 1.f, 100.f, XOR( "%1.f%%" ) );
				}
				InsertCheckbox( XOR( "Automatic fire" ), b[ XOR( "autofire_awp" ) ] );
				InsertCheckbox( XOR( "Automatic penetration" ), b[ XOR( "penetrate_awp" ) ] );
				InsertCheckbox( XOR( "Silent aim" ), b[ XOR( "silent_awp" ) ] );
				InsertCheckbox( XOR( "Minimum hit chance" ), b[ XOR( "hitchance_awp" ) ] );
				if( b[ XOR( "hitchance_awp" ) ] ) {
					InsertSliderWithoutText( XOR( "##Hitchance" ), f[ XOR( "hitchance_amount_awp" ) ], 1.f, 100.f, XOR( "%1.f%%" ) );
				}
				if( f[ XOR( "minimal_damage_awp" ) ] <= 100 ) {
					InsertSlider( XOR( "Minimal damage" ), f[ XOR( "minimal_damage_awp" ) ], 1.f, 150.f, XOR( "%1.f" ) );
				}
				else {
					int hp = f[ XOR( "minimal_damage_awp" ) ] - 100;

					std::string da = tfm::format( XOR( "HP + %1.f" ), hp );

					InsertSlider( XOR( "Minimal damage" ), f[ XOR( "minimal_damage_awp" ) ], 1.f, 150.f, da.c_str( ) );
				}
				InsertCheckbox( XOR( "Scale damage on hp" ), b[ XOR( "minimal_damage_on_hp_awp" ) ] );
			}
			else if( i[ XOR( "weapon_cnofig" ) ] == 3 ) {
				InsertMultiCombo( XOR( "Target hitbox" ), &m[ XOR( "hitbox_deagle" ) ], hitbox_type );
				InsertCheckbox( XOR( "Avoid limbs if moving" ), b[ XOR( "ignor_limbs_deagle" ) ] );
				InsertMultiCombos( XOR( "Multi-point" ), &m[ XOR( "multipoint_deagle" ) ], multipoint_type );
				if( m[ XOR( "multipoint_deagle" ) ][ 0 ] ) {
					InsertSliderWithoutText( XOR( "##Hitbox_scale" ), f[ XOR( "scale_deagle" ) ], 1.f, 100.f, XOR( "%1.f%%" ) );
				}
				else { ImGui::Spacing( ); }
				if( m[ XOR( "multipoint_deagle" ) ][ 2 ] ) {
					InsertSlider( XOR( "Body hitbox scale" ), f[ XOR( "body_scale_deagle" ) ], 1.f, 100.f, XOR( "%1.f%%" ) );
				}
				InsertCheckbox( XOR( "Automatic fire" ), b[ XOR( "autofire_deagle" ) ] );
				InsertCheckbox( XOR( "Automatic penetration" ), b[ XOR( "penetrate_deagle" ) ] );
				InsertCheckbox( XOR( "Silent aim" ), b[ XOR( "silent_deagle" ) ] );
				InsertCheckbox( XOR( "Minimum hit chance" ), b[ XOR( "hitchance_deagle" ) ] );
				if( b[ XOR( "hitchance_deagle" ) ] ) {
					InsertSliderWithoutText( XOR( "##Hitchance" ), f[ XOR( "hitchance_amount_deagle" ) ], 1.f, 100.f, XOR( "%1.f%%" ) );
				}
				if( f[ XOR( "minimal_damage_deagle" ) ] <= 100 ) {
					InsertSlider( XOR( "Minimal damage" ), f[ XOR( "minimal_damage_deagle" ) ], 1.f, 150.f, XOR( "%1.f" ) );
				}
				else {
					int hp = f[ XOR( "minimal_damage_deagle" ) ] - 100;

					std::string da = tfm::format( XOR( "HP + %1.f" ), hp );

					InsertSlider( XOR( "Minimal damage" ), f[ XOR( "minimal_damage_deagle" ) ], 1.f, 150.f, da.c_str( ) );
				}
				InsertCheckbox( XOR( "Scale damage on hp" ), b[ XOR( "minimal_damage_on_hp_deagle" ) ] );
			}
			else if( i[ XOR( "weapon_cnofig" ) ] == 4 ) {
				InsertMultiCombo( XOR( "Target hitbox" ), &m[ XOR( "hitbox_pistols" ) ], hitbox_type );
				InsertCheckbox( XOR( "Avoid limbs if moving" ), b[ XOR( "ignor_limbs_pistols" ) ] );
				InsertMultiCombos( XOR( "Multi-point" ), &m[ XOR( "multipoint_pistols" ) ], multipoint_type );
				if( m[ XOR( "multipoint_pistols" ) ][ 0 ] ) {
					InsertSliderWithoutText( XOR( "##Hitbox_scale" ), f[ XOR( "scale_pistols" ) ], 1.f, 100.f, XOR( "%1.f%%" ) );
				}
				else { ImGui::Spacing( ); }
				if( m[ XOR( "multipoint_pistols" ) ][ 2 ] ) {
					InsertSlider( XOR( "Body hitbox scale" ), f[ XOR( "body_scale_pistols" ) ], 1.f, 100.f, XOR( "%1.f%%" ) );
				}
				InsertCheckbox( XOR( "Automatic fire" ), b[ XOR( "autofire_pistols" ) ] );
				InsertCheckbox( XOR( "Automatic penetration" ), b[ XOR( "penetrate_pistols" ) ] );
				InsertCheckbox( XOR( "Silent aim" ), b[ XOR( "silent_pistols" ) ] );
				InsertCheckbox( XOR( "Minimum hit chance" ), b[ XOR( "hitchance_pistols" ) ] );
				if( b[ XOR( "hitchance_pistols" ) ] ) {
					InsertSliderWithoutText( XOR( "##Hitchance" ), f[ XOR( "hitchance_amount_pistols" ) ], 1.f, 100.f, XOR( "%1.f%%" ) );
				}
				if( f[ XOR( "minimal_damage_pistols" ) ] <= 100 ) {
					InsertSlider( XOR( "Minimal damage" ), f[ XOR( "minimal_damage_pistols" ) ], 1.f, 150.f, XOR( "%1.f" ) );
				}
				else {
					int hp = f[ XOR( "minimal_damage_pistols" ) ] - 100;

					std::string da = tfm::format( XOR( "HP + %1.f" ), hp );

					InsertSlider( XOR( "Minimal damage" ), f[ XOR( "minimal_damage_pistols" ) ], 1.f, 150.f, da.c_str( ) );
				}
				InsertCheckbox( XOR( "Scale damage on hp" ), b[ XOR( "minimal_damage_on_hp_pistols" ) ] );
			}
			else if( i[ XOR( "weapon_cnofig" ) ] == 5 ) {
				InsertMultiCombo( XOR( "Target hitbox" ), &m[ XOR( "hitbox_revolver" ) ], hitbox_type );
				InsertCheckbox( XOR( "Avoid limbs if moving" ), b[ XOR( "ignor_limbs_revolver" ) ] );
				InsertMultiCombos( XOR( "Multi-point" ), &m[ XOR( "multipoint_revolver" ) ], multipoint_type );
				if( m[ XOR( "multipoint_revolver" ) ][ 0 ] ) {
					InsertSliderWithoutText( XOR( "##Hitbox_scale" ), f[ XOR( "scale_revolver" ) ], 1.f, 100.f, XOR( "%1.f%%" ) );
				}
				else { ImGui::Spacing( ); }
				if( m[ XOR( "multipoint_revolver" ) ][ 2 ] ) {
					InsertSlider( XOR( "Body hitbox scale" ), f[ XOR( "body_scale_revolver" ) ], 1.f, 100.f, XOR( "%1.f%%" ) );
				}
				InsertCheckbox( XOR( "Automatic fire" ), b[ XOR( "autofire_revolver" ) ] );
				InsertCheckbox( XOR( "Automatic penetration" ), b[ XOR( "penetrate_revolver" ) ] );
				InsertCheckbox( XOR( "Silent aim" ), b[ XOR( "silent_revolver" ) ] );
				InsertCheckbox( XOR( "Minimum hit chance" ), b[ XOR( "hitchance_revolver" ) ] );
				if( b[ XOR( "hitchance_revolver" ) ] ) {
					InsertSliderWithoutText( XOR( "##Hitchance" ), f[ XOR( "hitchance_amount_revolver" ) ], 1.f, 100.f, XOR( "%1.f%%" ) );
				}
				if( f[ XOR( "minimal_damage_revolver" ) ] <= 100 ) {
					InsertSlider( XOR( "Minimal damage" ), f[ XOR( "minimal_damage_revolver" ) ], 1.f, 150.f, XOR( "%1.f" ) );
				}
				else {
					int hp = f[ XOR( "minimal_damage_revolver" ) ] - 100;

					std::string da = tfm::format( XOR( "HP + %1.f" ), hp );

					InsertSlider( XOR( "Minimal damage" ), f[ XOR( "minimal_damage_revolver" ) ], 1.f, 150.f, da.c_str( ) );
				}
				InsertCheckbox( XOR( "Scale damage on hp" ), b[ XOR( "minimal_damage_on_hp_revolver" ) ] );
			}
			InsertCheckbox( XOR( "Override minimal damage" ), b[ XOR( "override_hp" ) ] );
			InsertStyleKeyBind( XOR( "##Override damage" ), i[ XOR( "damage_key" ) ], &i[ XOR( "damage_keystyle" ) ] );
			if( b[ XOR( "override_hp" ) ] ) {
				InsertSliderWithoutText( XOR( "##Baim_hp_override" ), f[ XOR( "baim_hp_override" ) ], 1.f, 100.f, XOR( "%1.fhp" ) );
			}
			InsertCheckbox( XOR( "Override body aim" ), b[ XOR( "body_aim_enable" ) ] );
			InsertStyleKeyBind( XOR( "##Override damage1" ), i[ XOR( "damage1_key" ) ], &i[ XOR( "damage1_keystyle" ) ] );

			InsertCheckbox( XOR( "Override AWP" ), b[ XOR( "overrideAWP" ) ] );
			InsertCheckbox( XOR( "Automatic scope" ), b[ XOR( "auto_scope" ) ] );

			InsertCheckbox(XOR("Double tap"), b[XOR("double_tap")]);

			InsertCheckbox( XOR( "Quick peek assist" ), b[ XOR( "auto_peek" ) ] );
			InsertStyleKeyBind( XOR( "##Quick peek assist" ), i[ XOR( "auto_peek_key" ) ], &i[ XOR( "auto_peek_keystyle" ) ] );

			if( b[ XOR( "auto_peek" ) ] ) {
				InsertCheckbox( XOR( "Quick peek assist indicator" ), b[ XOR( "auto_peek_ind" ) ] );
				if( b[ XOR( "auto_peek_ind" ) ] ) {
					ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::Text( XOR( "Active color" ) );
					InsertColorPicker( XOR( "##QuickpeekcolorA" ), c[ XOR( "auto_peek_active" ) ], false );
					ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::Text( XOR( "Inactive color" ) );
					InsertColorPicker( XOR( "##QuickpeekcolorinA" ), c[ XOR( "auto_peek_inactive" ) ], false );
				}
			}

			if( !b[ XOR( "antiUntrusted" ) ] ) { ImGui::PushDisabled( ); }
			InsertCheckbox( XOR( "Reduce aim step" ), b[ XOR( "aimstep" ) ] );
			if( !b[ XOR( "antiUntrusted" ) ] ) { ImGui::PopDisabled( ); }
			InsertSlider( XOR( "Maximum FOV" ), f[ XOR( "fov_amount" ) ], 1.f, 180.f, XOR( u8"%1.f∞" ) );

			style->ItemSpacing = ImVec2( 0, 0 );
			style->WindowPadding = ImVec2( 6, 6 );

		} InsertEndGroupBoxLeft( XOR( "Aimbot Cover" ), XOR( "Aimbot" ) );

		InsertSpacer( XOR( "Aimbot - Aimbot Spacer" ) );

		InsertGroupBoxLeft( XOR( "Fake lag" ), 150.f );
		{

			style->ItemSpacing = ImVec2( 4, 2 );
			style->WindowPadding = ImVec2( 4, 4 );
			ImGui::CustomSpacing( 9.f );

			InsertCheckbox( XOR( "Enable" ), b[ XOR( "enable_fakelag" ) ] );
			InsertMultiCombos( XOR( "Triggers" ), &m[ XOR( "fakelag_triggers" ) ], fakelag_type );
			InsertComboWithoutText( XOR( "##Fake lag modes" ), &i[ XOR( "fakelag_mode" ) ], fakelag_mode );
			if( i[ XOR( "fakelag_mode" ) ] == 3 ) { InsertSlider( XOR( "Variance" ), f[ XOR( "fakelag_variance" ) ], 0.f, 100.f, "%1.f%%" ); }
			InsertSlider( XOR( "Limit" ), f[ XOR( "fakelag_limit" ) ], 2.f, 16.f, "%1.f" );
			InsertCheckbox( XOR( "Fake lag while shooting" ), b[ XOR( "fakelag_shooting" ) ] );
			InsertCheckbox( XOR( "Reset on bunny hop" ), b[ XOR( "fakelag_reset" ) ] );
			InsertCheckbox( XOR( "Disable on landing" ), b[ XOR( "fakelag_on_land_r" ) ] );

			style->ItemSpacing = ImVec2( 0, 0 );
			style->WindowPadding = ImVec2( 6, 6 );

		} InsertEndGroupBoxLeft( XOR( "Fake lag Cover" ), XOR( "Fake lag" ) );

	}
	ImGui::NextColumn( );
	{

		InsertGroupBoxRight( XOR( "Other" ), 190.f );
		{

			style->ItemSpacing = ImVec2( 4, 2 );
			style->WindowPadding = ImVec2( 4, 4 );
			ImGui::CustomSpacing( 9.f );

			colors[ ImGuiCol_Text ] = ImColor( 181, 181, 100 );
			InsertCheckbox( XOR( "Remove spread" ), b[ XOR( "nospread" ) ] );
			colors[ ImGuiCol_Text ] = ImColor( 213, 213, 213 );
			InsertCheckbox( XOR( "Remove recoil" ), b[ XOR( "norecoil" ) ] );
			if( i[ XOR( "weapon_cnofig" ) ] == 0 ) {
				ImGui::Spacing( ); ImGui::NewLine( ); InsertStyleKeyBind( XOR( "##Quick stop" ), i[ XOR( "quickstop_key" ) ], &i[ XOR( "quickstop_keystyle" ) ] ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 158.f );
				ImGui::SingleSelect( XOR( "Quick stop" ), &i[ XOR( "quickstop" ) ], quickstop_mode );
				ImGui::PopItemWidth( );

				//InsertCheckbox( XOR( "Quick stop" ), b[ XOR( "quickstop" ) ] );
				//InsertStyleKeyBind( XOR( "##Quick stop" ), i[ XOR( "quickstop_key" ) ], &i[ XOR( "quickstop_keystyle" ) ] );
				InsertCheckbox( XOR( "Quick stop between shots" ), b[ XOR( "between_shots" ) ] );

				//InsertCheckbox( XOR( "Quick stop force accuracy" ), b[ XOR( "force_accuracy" ) ] );
			}
			else if( i[ XOR( "weapon_cnofig" ) ] == 1 ) {
				ImGui::Spacing( ); ImGui::NewLine( ); InsertStyleKeyBind( XOR( "##Quick stop" ), i[ XOR( "quickstop_key_scout" ) ], &i[ XOR( "quickstop_keystyle_scout" ) ] ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 158.f );
				ImGui::SingleSelect( XOR( "Quick stop" ), &i[ XOR( "quickstop_scout" ) ], quickstop_mode );
				ImGui::PopItemWidth( );

				//InsertCheckbox( XOR( "Quick stop" ), b[ XOR( "quickstop_scout" ) ] );
				//InsertStyleKeyBind( XOR( "##Quick stop" ), i[ XOR( "quickstop_key_scout" ) ], &i[ XOR( "quickstop_keystyle_scout" ) ] );
				InsertCheckbox( XOR( "Quick stop between shots" ), b[ XOR( "between_shots_scout" ) ] );

				//InsertCheckbox( XOR( "Quick stop force accuracy" ), b[ XOR( "force_accuracy_scout" ) ] );
			}
			else if( i[ XOR( "weapon_cnofig" ) ] == 2 ) {
				ImGui::Spacing( ); ImGui::NewLine( ); InsertStyleKeyBind( XOR( "##Quick stop" ), i[ XOR( "quickstop_key_awp" ) ], &i[ XOR( "quickstop_keystyle_awp" ) ] ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 158.f );
				ImGui::SingleSelect( XOR( "Quick stop" ), &i[ XOR( "quickstop_awp" ) ], quickstop_mode );
				ImGui::PopItemWidth( );

				//InsertCheckbox( XOR( "Quick stop" ), b[ XOR( "quickstop_awp" ) ] );
				//InsertStyleKeyBind( XOR( "##Quick stop" ), i[ XOR( "quickstop_key_awp" ) ], &i[ XOR( "quickstop_keystyle_awp" ) ] );
				InsertCheckbox( XOR( "Quick stop between shots" ), b[ XOR( "between_shots_awp" ) ] );

				//InsertCheckbox( XOR( "Quick stop force accuracy" ), b[ XOR( "force_accuracy_awp" ) ] );
			}
			else if( i[ XOR( "weapon_cnofig" ) ] == 3 ) {
				ImGui::Spacing( ); ImGui::NewLine( ); InsertStyleKeyBind( XOR( "##Quick stop" ), i[ XOR( "quickstop_key_deagle" ) ], &i[ XOR( "quickstop_keystyle_deagle" ) ] ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 158.f );
				ImGui::SingleSelect( XOR( "Quick stop" ), &i[ XOR( "quickstop_deagle" ) ], quickstop_mode );
				ImGui::PopItemWidth( );

				//InsertCheckbox( XOR( "Quick stop" ), b[ XOR( "quickstop_deagle" ) ] );
				//InsertStyleKeyBind( XOR( "##Quick stop" ), i[ XOR( "quickstop_key_deagle" ) ], &i[ XOR( "quickstop_keystyle_deagle" ) ] );
				InsertCheckbox( XOR( "Quick stop between shots" ), b[ XOR( "between_shots_deagle" ) ] );

				//InsertCheckbox( XOR( "Quick stop force accuracy" ), b[ XOR( "force_accuracy_deagle" ) ] );
			}
			else if( i[ XOR( "weapon_cnofig" ) ] == 4 ) {
				ImGui::Spacing( ); ImGui::NewLine( ); InsertStyleKeyBind( XOR( "##Quick stop" ), i[ XOR( "quickstop_key_pistols" ) ], &i[ XOR( "quickstop_keystyle_pistols" ) ] ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 158.f );
				ImGui::SingleSelect( XOR( "Quick stop" ), &i[ XOR( "quickstop_pistols" ) ], quickstop_mode );
				ImGui::PopItemWidth( );

				//InsertCheckbox( XOR( "Quick stop" ), b[ XOR( "quickstop_pistols" ) ] );
				//InsertStyleKeyBind( XOR( "##Quick stop" ), i[ XOR( "quickstop_key_pistols" ) ], &i[ XOR( "quickstop_keystyle_pistols" ) ] );
				InsertCheckbox( XOR( "Quick stop between shots" ), b[ XOR( "between_shots_pistols" ) ] );

				//InsertCheckbox( XOR( "Quick stop force accuracy" ), b[ XOR( "force_accuracy_pistols" ) ] );
			}
			else if( i[ XOR( "weapon_cnofig" ) ] == 5 ) {
				ImGui::Spacing( ); ImGui::NewLine( ); InsertStyleKeyBind( XOR( "##Quick stop" ), i[ XOR( "quickstop_key_revolver" ) ], &i[ XOR( "quickstop_keystyle_revolver" ) ] ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 158.f );
				ImGui::SingleSelect( XOR( "Quick stop" ), &i[ XOR( "quickstop_revolver" ) ], quickstop_mode );
				ImGui::PopItemWidth( );

				//InsertCheckbox( XOR( "Quick stop" ), b[ XOR( "quickstop_revolver" ) ] );
				//InsertStyleKeyBind( XOR( "##Quick stop" ), i[ XOR( "quickstop_key_revolver" ) ], &i[ XOR( "quickstop_keystyle_revolver" ) ] );
				InsertCheckbox( XOR( "Quick stop between shots" ), b[ XOR( "between_shots_revolver" ) ] );

				//InsertCheckbox( XOR( "Quick stop force accuracy" ), b[ XOR( "force_accuracy_revolver" ) ] );
			}
			InsertCombo( XOR( "Accuracy boost" ), &i[ XOR( "accuracy_boost" ) ], accuracy_mode );
			InsertCheckbox( XOR( "Target limit" ), b[ XOR( "target_limit" ) ] );
			if( b[ XOR( "target_limit" ) ] ) {
				InsertSliderWithoutText( XOR( "##Target limit" ), f[ XOR( "target_limit_f" ) ], 1.f, 10.f, "%1.f" );
			}
			InsertCheckbox( XOR( "Anti-aim correction" ), b[ XOR( "aa_correction" ) ] );
			InsertCheckbox( XOR( "Anti-aim resolver" ), b[ XOR( "resolver" ) ] );
			InsertCheckbox(XOR("RandomResolver"), b[XOR("RandomResolver")]);
			InsertCheckbox( XOR( "Anti-aim edge correction" ), b[ XOR( "aa_edge_correction" ) ] );
			InsertCheckbox( XOR( "Anti-aim resolver override" ), b[ XOR( "override" ) ] );
			InsertKeyBind( XOR( "##Anti-aim resolver override" ), i[ XOR( "override_key" ) ] );
			InsertCombo( XOR( "Fake lag correction" ), &i[ XOR( "lagfix_mide" ) ], lagfix_mode );
			InsertCheckbox( XOR( "Simulate player movement" ), b[ XOR( "lagfix_2" ) ] );

			if( i[ XOR( "weapon_cnofig" ) ] == 0 ) {
				InsertMultiCombo( XOR( "Prefer body aim" ), &m[ XOR( "baim1" ) ], baim1_type );
				InsertMultiCombo( XOR( "Only body aim" ), &m[ XOR( "baim2" ) ], baim2_type );
				if( m[ XOR( "baim2" ) ][ 1 ] ) {
					InsertSliderWithoutText( XOR( "##Hp" ), f[ XOR( "baim_hp" ) ], 1.f, 50.f, "%1.fhp" );
				}
				if( m[ XOR( "baim2" ) ][ 4 ] || m[ XOR( "baim1" ) ][ 5 ] ) {
					InsertSliderWithoutText( XOR( "##Misses" ), f[ XOR( "misses" ) ], 1.f, 10.f, "%1.f" );
				}
			}
			else if( i[ XOR( "weapon_cnofig" ) ] == 1 ) {
				InsertMultiCombo( XOR( "Prefer body aim" ), &m[ XOR( "baim1_scout" ) ], baim1_type );
				InsertMultiCombo( XOR( "Only body aim" ), &m[ XOR( "baim2_scout" ) ], baim2_type );
				if( m[ XOR( "baim2_scout" ) ][ 1 ] ) {
					InsertSliderWithoutText( XOR( "##Hp" ), f[ XOR( "baim_hp_scout" ) ], 1.f, 50.f, "%1.fhp" );
				}
				if( m[ XOR( "baim1_scout" ) ][ 5 ] || m[ XOR( "baim2_scout" ) ][ 4 ] ) {
					InsertSliderWithoutText( XOR( "##Misses" ), f[ XOR( "misses_scout" ) ], 1.f, 10.f, "%1.f" );
				}
			}
			else if( i[ XOR( "weapon_cnofig" ) ] == 2 ) {
				InsertMultiCombo( XOR( "Prefer body aim" ), &m[ XOR( "baim1_awp" ) ], baim1_type );
				InsertMultiCombo( XOR( "Only body aim" ), &m[ XOR( "baim2_awp" ) ], baim2_type );
				if( m[ XOR( "baim2_awp" ) ][ 1 ] ) {
					InsertSliderWithoutText( XOR( "##Hp" ), f[ XOR( "baim_hp_awp" ) ], 1.f, 50.f, "%1.fhp" );
				}
				if( m[ XOR( "baim1_awp" ) ][ 5 ] || m[ XOR( "baim2_awp" ) ][ 4 ] ) {
					InsertSliderWithoutText( XOR( "##Misses" ), f[ XOR( "misses_awp" ) ], 1.f, 10.f, "%1.f" );
				}
			}
			else if( i[ XOR( "weapon_cnofig" ) ] == 3 ) {
				InsertMultiCombo( XOR( "Prefer body aim" ), &m[ XOR( "baim1_deagle" ) ], baim1_type );
				InsertMultiCombo( XOR( "Only body aim" ), &m[ XOR( "baim2_deagle" ) ], baim2_type );
				if( m[ XOR( "baim2_deagle" ) ][ 1 ] ) {
					InsertSliderWithoutText( XOR( "##Hp" ), f[ XOR( "baim_hp_deagle" ) ], 1.f, 50.f, "%1.fhp" );
				}
				if( m[ XOR( "baim1_deagle" ) ][ 5 ] || m[ XOR( "baim2_deagle" ) ][ 4 ] ) {
					InsertSliderWithoutText( XOR( "##Misses" ), f[ XOR( "misses_deagle" ) ], 1.f, 10.f, "%1.f" );
				}
			}
			else if( i[ XOR( "weapon_cnofig" ) ] == 4 ) {
				InsertMultiCombo( XOR( "Prefer body aim" ), &m[ XOR( "baim1_pistols" ) ], baim1_type );
				InsertMultiCombo( XOR( "Only body aim" ), &m[ XOR( "baim2_pistols" ) ], baim2_type );
				if( m[ XOR( "baim2_pistols" ) ][ 1 ] ) {
					InsertSliderWithoutText( XOR( "##Hp" ), f[ XOR( "baim_hp_pistols" ) ], 1.f, 50.f, "%1.fhp" );
				}
				if( m[ XOR( "baim1_pistols" ) ][ 5 ] || m[ XOR( "baim2_pistols" ) ][ 4 ] ) {
					InsertSliderWithoutText( XOR( "##Misses" ), f[ XOR( "misses_pistols" ) ], 1.f, 10.f, "%1.f" );
				}
			}
			else if( i[ XOR( "weapon_cnofig" ) ] == 5 ) {
				InsertMultiCombo( XOR( "Prefer body aim" ), &m[ XOR( "baim1_revolver" ) ], baim1_type );
				InsertMultiCombo( XOR( "Only body aim" ), &m[ XOR( "baim2_revolver" ) ], baim2_type );
				if( m[ XOR( "baim2_revolver" ) ][ 1 ] ) {
					InsertSliderWithoutText( XOR( "##Hp" ), f[ XOR( "baim_hp_revolver" ) ], 1.f, 50.f, "%1.fhp" );
				}
				if( m[ XOR( "baim1_revolver" ) ][ 5 ] || m[ XOR( "baim2_revolver" ) ][ 4 ] ) {
					InsertSliderWithoutText( XOR( "##Misses" ), f[ XOR( "misses_revolver" ) ], 1.f, 10.f, "%1.f" );
				}
			}

			style->ItemSpacing = ImVec2( 0, 0 );
			style->WindowPadding = ImVec2( 6, 6 );

		} InsertEndGroupBoxRight( XOR( "Other Cover" ), XOR( "Other" ) );

		InsertSpacer( XOR( "Other - Other Spacer" ) );

		InsertGroupBoxRight( XOR( "Anti-aimbot" ), 247.f );
		{

			style->ItemSpacing = ImVec2( 4, 2 );
			style->WindowPadding = ImVec2( 4, 4 );
			ImGui::CustomSpacing( 9.f );

			//InsertCheckbox(XOR("Enable"), b[XOR("enable_antiaim")]);
			InsertCombo( XOR( "Pitch" ), &i[ XOR( "pitch" ) ], pitch_mode );
			InsertCombos( XOR( "Yaw" ), &i[ XOR( "yaw" ) ], yaw_mode );
			if( i[ XOR( "yaw" ) ] > 0 && i[ XOR( "yaw" ) ] != 4 ) { InsertSliderWithoutText( XOR( "##Yaw add" ), f[ XOR( "yaw_add" ) ], -180.f, 180.f, XOR( u8"%1.f∞" ) ); dmt( "##Yaw add" ); }
			else if( i[ XOR( "yaw" ) ] == 4 ) { InsertSliderWithoutText( XOR( "##Yaw rand add" ), f[ XOR( "rand_speed" ) ], 0.f, 10.f, XOR( u8"%1.fs" ) ); }
			else { ImGui::Spacing( ); }
			InsertCombos( XOR( "Yaw while running" ), &i[ XOR( "yaw_move" ) ], yaw_mode );
			if( i[ XOR( "yaw_move" ) ] > 0 && i[ XOR( "yaw_move" ) ] != 4 && i[ XOR( "yaw_move" ) ] != 1 ) { InsertSliderWithoutText( XOR( "##Yaw move add" ), f[ XOR( "yaw_move_add" ) ], -180.f, 180.f, XOR( u8"%1.f∞" ) ); }
			else { ImGui::Spacing( ); }
			InsertCombos( XOR( "Fake yaw" ), &i[ XOR( "fake_yaw" ) ], fakeyaw_mode );
			if( i[ XOR( "fake_yaw" ) ] > 1 && i[ XOR( "fake_yaw" ) ] != 5 && i[ XOR( "fake_yaw" ) ] != 7 ) { InsertSliderWithoutText( XOR( "##Fake yaw add" ), f[ XOR( "fakeyaw_add" ) ], -180.f, 180.f, XOR( u8"%1.f∞" ) ); }
			else { ImGui::Spacing( ); }
			InsertMultiCombos( XOR( "Freestanding" ), &m[ XOR( "freestand" ) ], freestand_mode );
			if( m[ XOR( "freestand" ) ][ 0 ] || m[ XOR( "freestand" ) ][ 1 ] ) {
				InsertSliderWithoutText( XOR( "##freestand add" ), f[ XOR( "freestand_add" ) ], 0.f, 180.f, XOR( u8"%1.f∞" ) );
			}
			InsertCombos( XOR( "Fake body" ), &i[ XOR( "yaw_fakebody" ) ], fakebody_mode );
			if( i[ XOR( "yaw_fakebody" ) ] == 6 ) { InsertSliderWithoutText( XOR( "##Fake body add" ), f[ XOR( "fakebody_add" ) ], -180.f, 180.f, XOR( u8"%1.f∞" ) ); }
			InsertCombo( XOR( "Yaw base" ), &i[ XOR( "yaw_base" ) ], yawbase_mode );
			InsertCheckbox( XOR( "Freestand debug" ), b[ XOR( "freestand_debug" ) ] );
			InsertCheckbox( XOR( "Disable fake body while fakewalking" ), b[ XOR( "body_fake_stand_fakewalk" ) ] );
			InsertCheckbox( XOR( "Lock direction" ), b[ XOR( "dir_lock" ) ] );
			InsertCheckbox( XOR( "Distortion" ), b[ XOR( "distortion" ) ] );
			if( b[ XOR( "distortion" ) ] ) {
				InsertMultiCombos( XOR( "Triggers" ), &m[ XOR( "distortion_triggers" ) ], distortion_type );
				InsertSlider( XOR( "Speed" ), f[ XOR( "distortion_speed" ) ], 1.f, 100.f, XOR( u8"%1.f" ) );
				InsertSlider( XOR( "Amount" ), f[ XOR( "distortion_amount" ) ], -360.f, 360.f, XOR( u8"%1.f∞" ) );
			}
			if( i[ XOR( "yaw_base" ) ] > 0 ) {
				InsertCheckbox( XOR( "Manual anti-aim" ), b[ XOR( "manual_aa" ) ] );
				InsertColorPicker( XOR( "##Manual anti-aim color" ), c[ XOR( "manual_aa_color" ) ], true );
				if( b[ XOR( "manual_aa" ) ] ) {
					InsertKeyBindWithText( XOR( "Left" ), XOR( "##Manual aa left" ), i[ XOR( "left" ) ] );
					InsertKeyBindWithText( XOR( "Right" ), XOR( "##Manual aa right" ), i[ XOR( "right" ) ] );
					InsertKeyBindWithText( XOR( "Back" ), XOR( "##Manual aa back" ), i[ XOR( "back" ) ] );
				}
			}

			style->ItemSpacing = ImVec2( 0, 0 );
			style->WindowPadding = ImVec2( 6, 6 );

		} InsertEndGroupBoxRight( XOR( "Anti-aimbot Cover" ), "Anti-aimbot" );
	}
}

void Hooks::Visuals( ) {
	std::vector<const char*> keybinds = {
		XOR( "Off" ),
		XOR( "Old" ),
		XOR( "New" )
	};

	std::vector<const char*> spectators = {
		XOR( "Off" ),
		XOR( "Old" ),
		XOR( "New" )
	};

	std::vector<const char*> planted = {
		XOR( "Off" ),
		XOR( "Old" ),
		XOR( "New" )
	};

	std::vector<const char*> chamsMaterials = {
		XOR( "Default" ),
		XOR( "Solid" ),
		XOR( "Metallic" ),
		XOR( "Glow" ),
		XOR( "Wireframe" ),
		XOR( "Metallic 2" )
	};

	std::vector<const char*> brightnessAdjustment = {
		XOR( "Night mode" ),
		XOR( "Full bright" )
	};

	std::vector<const char*> visualRecoilAdjustment = {
		XOR( "Off" ),
		XOR( "Remove shake" ),
		XOR( "Remove all" )
	};

	std::vector<const char*> droppedWeapons = {
		XOR( "Off" ),
		XOR( "Text" ),
		XOR( "Icon" )
	};

	ImGuiStyle* style = &ImGui::GetStyle( );
	InsertSpacer( XOR( "Top Spacer" ) );

	ImGui::Columns( 2, NULL, false );
	{

		InsertGroupBoxLeft( XOR( "Player ESP" ), 320.f );
		{

			style->ItemSpacing = ImVec2( 4, 2 );
			style->WindowPadding = ImVec2( 4, 4 );
			ImGui::CustomSpacing( 9.f );

			// to do : activation hotkey
			InsertCheckbox( XOR( "Teammates" ), b[ XOR( "teammates" ) ] );
			InsertCheckbox( XOR( "Dormant" ), b[ XOR( "dormant" ) ] );
			InsertCheckbox( XOR( "Bounding box" ), b[ XOR( "boundingBox" ) ] );
			InsertColorPicker( XOR( "##Bounding box color" ), c[ XOR( "boundingBox_color" ) ], false );
			InsertCheckbox( XOR( "Health bar" ), b[ XOR( "healthBar" ) ] );
			InsertCheckbox( XOR( "Name" ), b[ XOR( "name" ) ] );
			InsertColorPicker( XOR( "##Name color" ), c[ XOR( "name_color" ) ], false );
			InsertCheckbox( XOR( "Flags" ), b[ XOR( "flags" ) ] );
			InsertCheckbox( XOR( "Weapon text" ), b[ XOR( "weaponText" ) ] );
			InsertColorPicker( XOR( "##Weapon color" ), c[ XOR( "weaponText_color" ) ], false );
			InsertCheckbox( XOR( "Weapon icon" ), b[ XOR( "weaponIcon" ) ] );
			InsertColorPicker( XOR( "##WeaponIcon color" ), c[ XOR( "weaponIcon_color" ) ], false );
			InsertCheckbox( XOR( "Ammo" ), b[ XOR( "ammo" ) ] );
			InsertColorPicker( XOR( "##Ammo color" ), c[ XOR( "ammo_color" ) ], false );
			InsertCheckbox( XOR( "LBY timer" ), b[ XOR( "lby" ) ] );
			InsertColorPicker( XOR( "##Lby timer" ), c[ XOR( "lby_color" ) ], false );
			InsertCheckbox( XOR( "Distance" ), b[ XOR( "distance" ) ] );
			InsertCheckbox( XOR( "Glow" ), b[ XOR( "glow" ) ] );
			InsertColorPicker( XOR( "##Glow color" ), c[ XOR( "glow_color" ) ], true );
			InsertCheckbox( XOR( "Hit marker" ), b[ XOR( "hitmarker" ) ] );
			InsertCheckbox( XOR( "World hit marker" ), b[ XOR( "hitmarker_3d" ) ] );
			InsertColorPicker( XOR( "##World hit marker color" ), c[ XOR( "hitmarker_3d_c" ) ], false );
			InsertCheckbox( XOR( "Hit marker sound" ), b[ XOR( "hitmarkerSound" ) ] );
			InsertCheckbox( XOR( "Visualize aimbot" ), b[ XOR( "vis_aimbot" ) ] );
			InsertColorPicker( XOR( "##soundaimcol" ), c[ XOR( "vis_aimbot_color" ) ], true );
			InsertCheckbox( XOR( "Visualize sounds" ), b[ XOR( "vis_soundesp" ) ] );
			InsertColorPicker( XOR( "##soundespcol" ), c[ XOR( "vis_soundesp_color" ) ], true );
			InsertCheckbox( XOR( "Damage number" ), b[ XOR( "hitmarker_quake" ) ] );
			InsertColorPicker( XOR( "##Damage number color" ), c[ XOR( "hitmarker_quake_c" ) ], false );
			InsertCheckbox( XOR( "Money" ), b[ XOR( "money" ) ] );
			InsertCheckbox( XOR( "Ping spike warning" ), b[ XOR( "ping" ) ] );
			InsertCheckbox( XOR( "Lag compensation debug" ), b[ XOR( "lagcomp_degug" ) ] );
			InsertColorPicker( XOR( "##Lag compensation debug color" ), c[ XOR( "lagcomp_degug_c" ) ], true );
			InsertCheckbox( XOR( "Skeleton" ), b[ XOR( "skeleton" ) ] );
			InsertColorPicker( XOR( "##Skeleton color" ), c[ XOR( "skeleton_color" ) ], false );
			InsertCheckbox( XOR( "Out of FOV arrow" ), b[ XOR( "outOfFOVArrow" ) ] );
			InsertColorPicker( XOR( "##Out of FOV arrow color" ), c[ XOR( "outOfFOVArrow_color" ) ], false );
			if( b[ XOR( "outOfFOVArrow" ) ] ) {

				InsertSliderWithoutText( XOR( "##arrow size" ), f[ XOR( "arrowSize" ) ], 1.f, 30.f, "%1.fpx" );
				InsertSliderWithoutText( XOR( "##arrow distance" ), f[ XOR( "arrowDistance" ) ], 1.f, 100.f, "%1.f%%" );
			}

			style->ItemSpacing = ImVec2( 0, 0 );
			style->WindowPadding = ImVec2( 6, 6 );

		} InsertEndGroupBoxLeft( XOR( "Player ESP Cover" ), XOR( "Player ESP" ) );

		InsertSpacer( XOR( "Player ESP - Colored models Spacer" ) );

		InsertGroupBoxLeft( XOR( "Colored models" ), 117.f );
		{

			style->ItemSpacing = ImVec2( 4, 2 );
			style->WindowPadding = ImVec2( 4, 4 );
			ImGui::CustomSpacing( 9.f );

			InsertCheckbox( XOR( "Player" ), b[ XOR( "player" ) ] );
			InsertColorPicker( XOR( "##Player color" ), c[ XOR( "player_color" ) ], true );

			if( b[ XOR( "player" ) ] ) {

				InsertCheckbox( XOR( "Player behind wall" ), b[ XOR( "playerBehindWall" ) ] );
				ImGui::SameLine( 219.f );
				InsertColorPicker( XOR( "##reeeeeeeeeee" ), c[ XOR( "playerBehindWall_color" ) ], true );
				InsertComboWithoutText( XOR( "##player material" ), &i[ XOR( "selc" ) ], chamsMaterials );

				if( i[ XOR( "selc" ) ] == 2 ) {

					ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); InsertColorPicker( XOR( "##Reflectivity color" ), c[ XOR( "playerReflectivityColor" ) ], false ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 159.f ); ImGui::SliderFloat( XOR( "Reflectivity" ), &f[ XOR( "reflectivity_f" ) ], 1.f, 100.f, XOR( "" ) ); ImGui::PopItemWidth( );
					InsertSlider( XOR( "Shine" ), f[ XOR( "shine_f" ) ], 1.f, 100.f, XOR( "" ) );
					InsertSlider( XOR( "Rim" ), f[ XOR( "rim_f" ) ], 1.f, 100.f, XOR( "" ) );
				}
				else if( i[ XOR( "selc" ) ] == 3 ) {
					ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::SameLine( 42.f ); ImGui::Text( XOR( "Glow color" ) ); InsertColorPicker( XOR( "##Glow color player" ), c[ XOR( "playerglowColor" ) ], false ); ImGui::Spacing( ); ImGui::Spacing( );
				}

				InsertCheckbox( XOR( "Show teammates" ), b[ XOR( "teammates_chams" ) ] );
				InsertColorPicker( XOR( "##Teammates color" ), c[ XOR( "teammates_chams_color" ) ], true );
			}

			InsertCheckbox( XOR( "Disable model occulusion" ), b[ XOR( "disable_occulusion" ) ] );
			InsertCheckbox( XOR( "Ragdolls" ), b[ XOR( "player_dead" ) ] );
			InsertCheckbox( XOR( "Draw hitbox matrix" ), b[ XOR( "debug_aim" ) ] );
			InsertColorPicker( XOR( "##Draw hitbox matrix color" ), c[ XOR( "debug_aim_color" ) ], true );
			if( b[ XOR( "debug_aim" ) ] ) {
				InsertSliderWithoutText( XOR( "##Time" ), f[ XOR( "debug_aim_time" ) ], 1.f, 10.f, XOR( "%1.fs" ) );
			}
			InsertCheckbox( XOR( "Hands" ), b[ XOR( "hands" ) ] );
			InsertColorPicker( XOR( "##Hands color" ), c[ XOR( "hands_color" ) ], true );
			if( b[ XOR( "hands" ) ] ) {

				InsertComboWithoutText( XOR( "##hands material" ), &i[ XOR( "handsMaterial" ) ], chamsMaterials );

				if( i[ XOR( "handsMaterial" ) ] == 2 ) {

					ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); InsertColorPicker( XOR( "##Reflectivity color1" ), c[ XOR( "handsReflectivityColor" ) ], false ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 159.f ); ImGui::SliderFloat( XOR( "Reflectivity" ), &f[ XOR( "reflectivity_hands" ) ], 1.f, 100.f, "" ); ImGui::PopItemWidth( );
					InsertSlider( XOR( "Shine" ), f[ XOR( "shine_hands" ) ], 1.f, 100.f, XOR( "" ) );
					InsertSlider( XOR( "Rim" ), f[ XOR( "rim_hands" ) ], 1.f, 100.f, XOR( "" ) );
				}
				else if( i[ XOR( "handsMaterial" ) ] == 3 ) {
					ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::SameLine( 42.f ); ImGui::Text( XOR( "Glow color" ) ); InsertColorPicker( XOR( "##Glow color hands" ), c[ XOR( "handsglowColor" ) ], false ); ImGui::Spacing( ); ImGui::Spacing( );
				}
			}

			InsertCheckbox( XOR( "Shadow" ), b[ XOR( "shadow" ) ] );
			InsertColorPicker( XOR( "##Shadow color" ), c[ XOR( "shadow_color" ) ], true );
			InsertCheckbox( XOR( "Local" ), b[ XOR( "localchams" ) ] );
			InsertColorPicker( XOR( "##Local chams color" ), c[ XOR( "localchmas_color" ) ], true );
			if( b[ XOR( "localchams" ) ] ) {
				InsertComboWithoutText( XOR( "##local material" ), &i[ XOR( "local_selc" ) ], chamsMaterials );

				if( i[ XOR( "local_selc" ) ] == 2 ) {

					ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); InsertColorPicker( XOR( "##Reflectivity color" ), c[ XOR( "localReflectivityColor" ) ], false ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 159.f ); ImGui::SliderFloat( XOR( "Reflectivity" ), &f[ XOR( "local_reflectivity_f" ) ], 1.f, 100.f, XOR( "" ) ); ImGui::PopItemWidth( );
					InsertSlider( XOR( "Shine" ), f[ XOR( "local_shine_f" ) ], 1.f, 100.f, XOR( "" ) );
					InsertSlider( XOR( "Rim" ), f[ XOR( "local_rim_f" ) ], 1.f, 100.f, XOR( "" ) );
				}
				else if( i[ XOR( "local_selc" ) ] == 3 ) {
					ImGui::Spacing( ); ImGui::Spacing( ); ImGui::Spacing( ); ImGui::SameLine( 42.f ); ImGui::Text( XOR( "Glow color" ) ); InsertColorPicker( XOR( "##Glow color local glow" ), c[ XOR( "localglowColor" ) ], false ); ImGui::Spacing( ); ImGui::Spacing( );
				}
			}
			InsertCheckbox( XOR( "Remove sleeve rendering" ), b[ XOR( "remove_sleeves" ) ] );
			InsertCheckbox( XOR( "Blend when scoped" ), b[ XOR( "localblend" ) ] );
			InsertCheckbox( XOR( "Local fake shadow" ), b[ XOR( "localFakeShadow" ) ] );
			InsertColorPicker( XOR( "##Local fake shadow color" ), c[ XOR( "localFakeShadow_color" ) ], true );

			style->ItemSpacing = ImVec2( 0, 0 );
			style->WindowPadding = ImVec2( 6, 6 );

		} InsertEndGroupBoxLeft( XOR( "Colored models Cover" ), XOR( "Colored models" ) );
	}
	ImGui::NextColumn( );
	{

		InsertGroupBoxRight( XOR( "Other ESP" ), 181.f );
		{

			style->ItemSpacing = ImVec2( 4, 2 );
			style->WindowPadding = ImVec2( 4, 4 );
			ImGui::CustomSpacing( 9.f );

			InsertCheckbox( XOR( "Radar" ), b[ XOR( "radar" ) ] );
			ImGui::Spacing( ); ImGui::NewLine( );
			InsertColorPicker( XOR( "##Weapons text color" ), c[ XOR( "droppedWeaponsText_color" ) ], false );
			ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 158.f );
			ImGui::SingleSelect( XOR( "Dropped weapons" ), &i[ XOR( "droppedWeapons_sel" ) ], droppedWeapons ); ImGui::PopItemWidth( );
			//InsertCheckbox(XOR("Dropped weapons"),               b[XOR("droppedWeapons")]);
			InsertCheckbox( XOR( "Dropped weapons ammo" ), b[ XOR( "droppedWeaponsAmmo" ) ] );
			InsertColorPicker( XOR( "##Dropped weapons color" ), c[ XOR( "droppedWeaponsAmmo_color" ) ], false );
			InsertCheckbox( XOR( "Dropped weapons distance" ), b[ XOR( "droppedWeaponsDistance" ) ] );
			InsertColorPicker( XOR( "##Weapons distance color" ), c[ XOR( "droppedWeaponsDistance_color" ) ], false );
			InsertCheckbox( XOR( "Grenades" ), b[ XOR( "grenades" ) ] );
			InsertColorPicker( XOR( "##Grenades color" ), c[ XOR( "grenades_color" ) ], false );
			if( b[ XOR( "grenades" ) ] ) {
				InsertCheckbox( XOR( "Molotov timer" ), b[ XOR( "molotv_timer" ) ] );
				InsertColorPicker( XOR( "##Molotov color" ), c[ XOR( "molotv_color" ) ], false );
				InsertCheckbox( XOR( "Smoke timer" ), b[ XOR( "smoke_timer" ) ] );
				InsertColorPicker( XOR( "##Smoke color" ), c[ XOR( "smoke_color" ) ], false );
			}
			InsertCheckbox( XOR( "Inaccuracy overlay" ), b[ XOR( "inaccuracyOverlay" ) ] );
			InsertColorPicker( XOR( "##Inaccuracy overlay color" ), c[ XOR( "inaccuracyOverlay_color" ) ], true );
			InsertCheckbox( XOR( "Crosshair" ), b[ XOR( "crosshair" ) ] );
			InsertCheckbox( XOR( "Grenade trajectory" ), b[ XOR( "grenadeTrajectory" ) ] );
			InsertColorPicker( XOR( "##Grenade trajectory color" ), c[ XOR( "grenadeTrajectory_color" ) ], true );
			InsertCheckbox( XOR( "Grenade proximity warning " ), b[ XOR( "grenadeWarning_enemy" ) ] );
			InsertColorPicker( XOR( "##Grenade proximity color enemy" ), c[ XOR( "grenadeWarning_enemy_color" ) ], true );

			if( b[ XOR( "grenadeWarning_enemy" ) ] ) {
				InsertCheckbox( XOR( "Show teammates" ), b[ XOR( "grenadeWarning_teammates" ) ] );
				InsertColorPicker( XOR( "##Grenade proximity color teammates" ), c[ XOR( "grenadeWarning_teammates_color" ) ], true );

				InsertCheckbox( XOR( "Show local" ), b[ XOR( "grenadeWarning_local" ) ] );
				InsertColorPicker( XOR( "##Grenade proximity color local" ), c[ XOR( "grenadeWarning_local_color" ) ], true );
			}
			InsertCheckbox( XOR( "Keybinds" ), b[ XOR( "keybinds" ) ] );

			InsertSlider( XOR( "Y offset" ), f[ XOR( "keybinds_y" ) ], 0.f, 1080.f, XOR( u8"%1.f" ) );
			InsertCombo( XOR( "Spectators" ), &i[ XOR( "spectators" ) ], spectators );
			InsertCheckbox( XOR( "Penetration reticle" ), b[ XOR( "penetrationReticle" ) ] );
			InsertCombo( XOR( "Planted C4" ), &i[ XOR( "planted_c4" ) ], planted );

			style->ItemSpacing = ImVec2( 0, 0 );
			style->WindowPadding = ImVec2( 6, 6 );

		} InsertEndGroupBoxRight( XOR( "Other ESP Cover" ), XOR( "Other ESP" ) );

		InsertSpacer( XOR( "Other ESP - Effects Spacer" ) );

		InsertGroupBoxRight( XOR( "Effects" ), 256.f );
		{

			style->ItemSpacing = ImVec2( 4, 2 );
			style->WindowPadding = ImVec2( 4, 4 );
			ImGui::CustomSpacing( 9.f );

			InsertCheckbox( XOR( "Remove flashbang effects" ), b[ XOR( "removeFlashbangEffects" ) ] );
			InsertCheckbox( XOR( "Remove smoke grenades" ), b[ XOR( "removeSmokeGrenades" ) ] );
			InsertCheckbox( XOR( "Remove fog" ), b[ XOR( "removeFog" ) ] );
			InsertCombo( XOR( "Remove visual recoil" ), &i[ XOR( "visualRecoilAdjustment" ) ], visualRecoilAdjustment );
			InsertSlider( XOR( "Transparent walls" ), f[ XOR( "transparentWalls" ) ], 1.f, 100.f, XOR( "%1.f%%" ) );
			InsertSlider( XOR( "Transparent props" ), f[ XOR( "transparentProps" ) ], 1.f, 100.f, XOR( "%1.f%%" ) );
			ImGui::Spacing( ); ImGui::NewLine( ); if( g_hooks.m[ XOR( "brightnessAdjustment_sel" ) ][ 0 ] ) { InsertColorPicker( XOR( "##Brightness adjustment color" ), c[ XOR( "nightmode_color" ) ], false ); } ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 158.f );
			ImGui::MultiSelect( XOR( "Brightness adjustment" ), &m[ XOR( "brightnessAdjustment_sel" ) ], brightnessAdjustment );
			ImGui::PopItemWidth( );
			InsertCheckbox( XOR( "Remove scope overlay" ), b[ XOR( "removeScopeOverlay" ) ] );
			InsertCheckbox( XOR( "Disable post processing" ), b[ XOR( "disablePostProcessing" ) ] );
			InsertCheckbox( XOR( "Force third person" ), b[ XOR( "forceThirdPerson" ) ] );
			InsertStyleKeyBind( XOR( "##Force_third_person" ), i[ XOR( "thirdperson_key" ) ], &i[ XOR( "thirdperson_keystyle" ) ] );
			if( b[ XOR( "forceThirdPerson" ) ] ) {
				InsertCheckbox( XOR( "Disable on grenade" ), b[ XOR( "third_person_on_grenade" ) ] );
				InsertSliderWithoutText( XOR( "##Third person dist" ), f[ XOR( "thirdperson_distance" ) ], 50.f, 200.f, XOR( u8"%1.f∞" ) );
			}
			InsertCheckbox( XOR( "Disable rendering of teammates" ), b[ XOR( "disableRenderingOfTeammates" ) ] );
			InsertCheckbox( XOR( "Ambient lightning" ), b[ XOR( "ambient_ligtning" ) ] );
			InsertColorPicker( XOR( "##Ambient lightning color" ), c[ XOR( "ambient_ligtning_color" ) ], false );
			InsertCheckbox( XOR( "Bullet impacts" ), b[ XOR( "bulletImpacts" ) ] );
			InsertCheckbox( XOR( "Bullet tracers" ), b[ XOR( "bulletTracers" ) ] );
			InsertColorPicker( XOR( "##tracer" ), c[ XOR( "bulletTracers_color" ) ], false );

			style->ItemSpacing = ImVec2( 0, 0 );
			style->WindowPadding = ImVec2( 6, 6 );

		} InsertEndGroupBoxRight( XOR( "Effects Cover" ), XOR( "Effects" ) );
	}
}

void Hooks::Misc( ) {
	std::vector<const char*> AirStrafe_Settings = {
		XOR( "None" ),
		XOR( "View angles" ),
		XOR( "Movement keys" )
	};
	std::vector<const char*> ClanTag_Settings = {
		XOR( "None" ),
		XOR( "cumhook" ),
		XOR( "gamesense" ),
		XOR( "skeet.cc" )
	};

	std::vector<const char*> Skybox_Settings = {
	   XOR( "None" ),
	   XOR( "Tibet" ),
	   XOR( "Embassy" ),
	   XOR( "Italy" ),
	   XOR( "Daylight" ),
	   XOR( "Cloudy" ),
	   XOR( "Night 2" ),
	   XOR( "Night 1" ),
	   XOR( "Night flat" ),
	   XOR( "Day hd" ),
	   XOR( "Day" ),
	   XOR( "Rural" ),
	   XOR( "Vertigo hd" ),
	   XOR( "Vertigo blue hd" ),
	   XOR( "Vertigo" ),
	   XOR( "Vietnam" ),
	   XOR( "dusty sky" ),
	   XOR( "Jungle" ),
	   XOR( "Nuke" ),
	   XOR( "Office" )
	};

	std::vector<const char*> AutoBuy1_Settings = {
		XOR( "None" ),
		XOR( "Scout" ),
		XOR( "Awp" ),
		XOR( "Auto" ),
	};

	std::vector<const char*> AutoBuy2_Settings = {
		XOR( "None" ),
		XOR( "Glock" ),
		XOR( "Usp" ),
		XOR( "Dual" ),
		XOR( "P250" ),
		XOR( "Tec9" ),
		XOR( "Fn57" ),
		XOR( "Deagle" )
	};

	std::vector<const char*> AutoBuy3_Settings = {
		XOR( "Vest" ),
		XOR( "Vesthelm" ),
		XOR( "Taser" ),
		XOR( "Defuser" ),
		XOR( "Heavyarmor" ),
		XOR( "Molotov" ),
		XOR( "Incgrenade" ),
		XOR( "Decoy" ),
		XOR( "Flashbang" ),
		XOR( "Hegrenade" ),
		XOR( "Smokegrenade" )
	};

	ImGuiStyle* style = &ImGui::GetStyle( );
	InsertSpacer( XOR( "Top Spacer" ) );
	static bool luaconfigs = false;

	ImGui::Columns( 2, NULL, false );
	{

		InsertGroupBoxLeft( XOR( "Miscellaneous" ), 455.f );
		{

			style->ItemSpacing = ImVec2( 4, 2 );
			style->WindowPadding = ImVec2( 4, 4 );
			ImGui::CustomSpacing( 9.f );
			if( !luaconfigs ) {

				InsertSlider( XOR( "Override FOV" ), f[ XOR( "overrideFov" ) ], 1.f, 135.f, u8"%1.f∞" );
				InsertCheckbox( XOR( "Override viewmodel FOV" ), b[ XOR( "overrideViewmodelFov_b" ) ] );
				if( b[ XOR( "overrideViewmodelFov_b" ) ] ) { InsertSliderWithoutText( XOR( "##Override viewmodel FOV slieder" ), f[ XOR( "overrideViewmodelFov" ) ], 1.f, 135.f, u8"%1.f∞" ); }
				InsertCheckbox( XOR( "Override FOV when scoped" ), b[ XOR( "fov_scoped" ) ] );
				InsertCheckbox( XOR( "Infinite stamina" ), b[ XOR( "fastduck" ) ] );
				InsertCheckbox( XOR( "Bunny hop" ), b[ XOR( "bunnyHop" ) ] );
				InsertCheckbox( XOR( "Edge jump" ), b[ XOR( "edgejump" ) ] );
				InsertKeyBind( XOR( "##EdgeJump" ), i[ XOR( "edgejump_key" ) ] );
				InsertCombo( XOR( "Air strafe" ), &i[ XOR( "airStrafe" ) ], AirStrafe_Settings );
				InsertCheckbox( XOR( "Z-Hop" ), b[ XOR( "zstrafe" ) ] );
				InsertKeyBind( XOR( "##Z Hop" ), i[ XOR( "zstrafe_key" ) ] );
				if( b[ XOR( "zstrafe" ) ] ) {
					InsertSliderWithoutText( XOR( "##z freq" ), f[ XOR( "z_freq" ) ], 1.f, 100.f, "%1.fhz" );
					InsertSliderWithoutText( XOR( "##z dist" ), f[ XOR( "z_dist" ) ], 1.f, 100.f, "%1.f%%" );
				}
				InsertCheckbox( XOR( "Pre-speed" ), b[ XOR( "cstrafe" ) ] );
				InsertKeyBind( XOR( "##Pre speed" ), i[ XOR( "cstrafe_key" ) ] );
				InsertCheckbox( XOR( "Air duck" ), b[ XOR( "airDuck" ) ] );
				InsertCheckbox(XOR("Killsay"), b[XOR("Killsay")]);
				InsertCheckbox( XOR( "Knifebot" ), b[ XOR( "knife_bot" ) ] );
				InsertCheckbox( XOR( "Zeusbot" ), b[ XOR( "zeus_bot" ) ] );
				if( b[ XOR( "zeus_bot" ) ] ) {
					InsertSliderWithoutText( XOR( "##Zeusbot hitchance" ), f[ XOR( "zeusbot_hc" ) ], 1.f, 100.f, "%1.f%%" );
				}
				InsertCheckbox( XOR( "Ragdoll force" ), b[ XOR( "ragdollForce" ) ] );
				InsertCheckbox( XOR( "Slow motion" ), b[ XOR( "slowMotion" ) ] );
				InsertKeyBind( XOR( "##Slow motion" ), i[ XOR( "slowMotion_key" ) ] );
				if( b[ XOR( "slowMotion" ) ] ) {
					InsertSliderWithoutText( XOR( "##Slow motion speed" ), f[ XOR( "slowMotion_speed" ) ], 1.f, 100.f, "%1.f%%" );
				}
				InsertCheckbox( XOR( "Fast stop" ), b[ XOR( "faststop" ) ] );
				InsertCheckbox( XOR( "Reveal competitive ranks" ), b[ XOR( "revealCompetitiveRanks" ) ] );
				InsertCheckbox( XOR( "Auto-accept matchmaking" ), b[ XOR( "autoAcceptMatchmaking" ) ] );
				InsertCheckbox( XOR( "Name spam" ), b[ XOR( "namespam" ) ] );
				InsertCombo( XOR( "Clan tag spammer" ), &i[ XOR( "clantagSpammer_selc" ) ], ClanTag_Settings );
				InsertCheckbox( XOR( "Log weapon purchases" ), b[ XOR( "logWeaponPurchases" ) ] );
				InsertCheckbox( XOR( "Log damage dealt" ), b[ XOR( "logDamageDealt" ) ] );
				InsertCheckbox( XOR( "Persistent Killfeed" ), b[ XOR( "persistentKillfeed" ) ] );
				InsertCheckbox( XOR( "Slide walk" ), b[ XOR( "sildewalk" ) ] );
				InsertCheckbox( XOR( "Watermark" ), b[ XOR( "watermark" ) ] );
				InsertCheckbox( XOR( "Unlock inventory in-game" ), b[ XOR( "unlock_inventory" ) ] );
				InsertCheckbox( XOR( "Ping spike" ), b[ XOR( "fake_latency" ) ] );
				InsertStyleKeyBind( XOR( "##Fake latency" ), i[ XOR( "fake_latency_key" ) ], &i[ XOR( "fake_latency_keystyle" ) ] );
				if( b[ XOR( "fake_latency" ) ] ) {
					InsertSliderWithoutText( XOR( "##Fake latency amount" ), f[ XOR( "fake_latency_amt" ) ], 1.f, 1000.f, "%1.fms" );
				}
				InsertKeyBindWithText( XOR( "Last tick defuse" ), XOR( "##defuse_key" ), i[ XOR( "last_tick_defuse_key" ) ] );
				ImGui::Spacing( ); ImGui::NewLine( ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 158.f );
				if( ImGui::Button( XOR( "Unlock hidden cvars" ) ) )
					g_cl.UnlockHiddenConvars( );
				ImGui::PopItemWidth( );

				InsertCombo( XOR( "Auto buy items" ), &i[ XOR( "autobuy1_selc" ) ], AutoBuy1_Settings );
				InsertComboWithoutText( XOR( "##Secondary auto buy" ), &i[ XOR( "autobuy2_selc" ) ], AutoBuy2_Settings );
				InsertMultiComboWithoutText( XOR( "##Thirdy auto buy" ), &m[ XOR( "autobuy3_selc" ) ], AutoBuy3_Settings );

				ImGui::PopItemWidth( ); ImGui::CustomSpacing( 1.f );
			}

			style->ItemSpacing = ImVec2( 0, 0 );
			style->WindowPadding = ImVec2( 6, 6 );

		}
		InsertEndGroupBoxLeft( XOR( "Miscellaneous Cover" ), "Miscellaneous" );
	}





	ImGui::NextColumn( );
	{

		InsertGroupBoxRight( XOR( "Other" ), 156.f );
		{

			style->ItemSpacing = ImVec2( 4, 2 );
			style->WindowPadding = ImVec2( 4, 4 );
			ImGui::CustomSpacing( 9.f );

			//ImGui::NewLine(); ImGui::SameLine(42.f); ImGui::Text(XOR("Menu key");
			//InsertKeyBind(XOR("menukey"), i[XOR("misc_menukey")]);
			ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::Text( XOR( "Menu color" ) );
			InsertColorPicker( XOR( "##menucolor" ), c[ XOR( "menu_color" ) ], false );
			InsertCheckbox( XOR( "Anti-untrusted" ), b[ XOR( "antiUntrusted" ) ] );
			InsertCheckbox( XOR( "Console color modifier" ), b[ XOR( "console" ) ] );
			InsertColorPicker( XOR( "##Console color" ), c[ XOR( "console_color" ) ], true );
			InsertCheckbox( XOR( "Interpolation" ), b[ XOR( "interp" ) ] );
			if( b[ XOR( "interp" ) ] ) {
				InsertSliderWithoutText( XOR( "##Interpolation ammount" ), f[ XOR( "interp_ammount" ) ], 1.f, 14.f, "%1.ftick" );
			}
			InsertCheckbox( XOR( "Skeet scope line" ), b[ XOR( "scope_line_enable" ) ] );
			InsertColorPicker( XOR( "##Skeet scope line color" ), c[ XOR( "scope_line_color" ) ], true );
			if( b[ XOR( "scope_line_enable" ) ] ) {
				InsertSlider( XOR( "Offset" ), f[ XOR( "scope_line_offset" ) ], 1.f, 500.f, "%1.f" );
				InsertSlider( XOR( "Height" ), f[ XOR( "scope_line_height" ) ], 1.f, 500.f, "%1.f" );
				InsertSlider( XOR( "Speed" ), f[ XOR( "scope_line_speed" ) ], 1.f, 100.f, "%1.f%%" );
			}

			InsertCombo( XOR( "Sky boxes" ), &i[ XOR( "skybox_selc" ) ], Skybox_Settings );
			if( i[ XOR( "skybox_selc" ) ] > 0 ) {
				g_cl.Skybox( );
			}

			style->ItemSpacing = ImVec2( 0, 0 );
			style->WindowPadding = ImVec2( 6, 6 );

		} InsertEndGroupBoxRight( XOR( "Settings Cover" ), XOR( "Settings" ) );

		InsertSpacer( XOR( "Settings - Other Spacer" ) );

		InsertGroupBoxRight( XOR( "Settings" ), 281.f );
		{
			style->ItemSpacing = ImVec2( 4, 2 );
			style->WindowPadding = ImVec2( 4, 4 );
			ImGui::CustomSpacing( 9.f );
			static bool lua = false;
			if( !lua ) {
				ImGui::Spacing( ); ImGui::NewLine( ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 158.f );
				ImGui::SingleSelect( XOR( "Preset" ), &i[ XOR( "_preset" ) ], presets );
				//ImGui::Keybind(XOR("##presetkey"), &i[XOR("_preset_" + std::to_string(i[XOR("_preset")])], 0);
				ImGui::PopItemWidth( ); ImGui::CustomSpacing( 1.f );

				ImGui::Spacing( ); ImGui::NewLine( ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 158.f );
				if( ImGui::Button( XOR( "Load" ) ) ) {
					load( );

					if( !std::filesystem::exists( XOR( "C:/cumhook" ) ) )
						std::filesystem::create_directories( XOR( "C:/cumhook" ) );

					if( !std::filesystem::exists( XOR( "C:/cumhook/cfg" ) ) )
						std::filesystem::create_directories( XOR( "C:/cumhook/cfg" ) );

				}
				ImGui::Spacing( ); ImGui::NewLine( ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 158.f );
				if( ImGui::Button( XOR( "Save" ) ) ) {
					save( );
					if( !std::filesystem::exists( XOR( "C:/cumhook" ) ) )
						std::filesystem::create_directories( XOR( "C:/cumhook" ) );

					if( !std::filesystem::exists( XOR( "C:/cumhook/cfg" ) ) )
						std::filesystem::create_directories( XOR( "C:/cumhook/cfg" ) );
				}
				ImGui::Spacing( ); ImGui::NewLine( ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 158.f );
				if( ImGui::Button( XOR( "Reset" ) ) ) {
					//auto res = MessageBoxA(0, "Are you sure?", "Reset warning", MB_APPLMODAL | MB_ICONWARNING | MB_YESNO);
					//
					//if (res == IDYES)
					load_defaults( );
					if( !std::filesystem::exists( XOR( "C:/cumhook" ) ) )
						std::filesystem::create_directories( XOR( "C:/cumhook" ) );

					if( !std::filesystem::exists( XOR( "C:/cumhook/cfg" ) ) )
						std::filesystem::create_directories( XOR( "C:/cumhook/cfg" ) );
				}
				ImGui::Spacing( ); ImGui::NewLine( ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 158.f );
				if( ImGui::Button( XOR( "Open settings folder" ) ) ) {
					ShellExecuteA( 0, XOR( "open" ), XOR( "C:/cumhook" ), NULL, NULL, SW_NORMAL );
					if( !std::filesystem::exists( XOR( "C:/cumhook" ) ) )
						std::filesystem::create_directories( XOR( "C:/cumhook" ) );

					if( !std::filesystem::exists( XOR( "C:/cumhook/cfg" ) ) )
						std::filesystem::create_directories( XOR( "C:/cumhook/cfg" ) );
				}
				ImGui::Spacing( ); ImGui::NewLine( ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 158.f );
				if( ImGui::Button( XOR( "Lua script manager" ) ) ) {
					lua = true;
					luaconfigs = true;
				}

				ImGui::PopItemWidth( ); ImGui::CustomSpacing( 1.f );
			}
			else {



				ImGui::Spacing( ); ImGui::NewLine( );  ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 158.f );

				ImGui::ListBoxFooter( );

				ImGui::Spacing( ); ImGui::NewLine( ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 158.f );
				if( ImGui::Button( XOR( "Back" ) ) ) {
					lua = false;
					luaconfigs = false;
				}

			}
			style->ItemSpacing = ImVec2( 0, 0 );
			style->WindowPadding = ImVec2( 6, 6 );

		} InsertEndGroupBoxRight( XOR( "Other Cover" ), XOR( "Other" ) );
	}
}

struct skins_info_t {
	const char* m_name;
	int           m_id;
};

void Hooks::Skins( ) {
	std::vector<const char*> bloodhount = {
		// bloodhound
		XOR( "Snakebite" ),        //      10007
		XOR( "Charred" ),          //      10006
		XOR( "Bronzed" ),          //      10008
		XOR( "Guerrilla" ),        //      10039
		// sport
		XOR( "Vice" ),             //      10048
		XOR( "Omega" ),            //      10047
		XOR( "Amphibious" ),       //      10045
		XOR( "Bronze Morph" ),     //      10046
		XOR( "Pandora's Box" ),    //      10037
		XOR( "Superconductor" ),   //      10018
		XOR( "Hedge Maze" ),       //      10038
		XOR( "Arid" ),             //      10019
		// driver
		XOR( "King Snake" ),       //      10041
		XOR( "Imperial Plaid" ),   //      10042
		XOR( "Overtake" ),         //      10043
		XOR( "Racing Green" ),     //      10044
		XOR( "Crimson Weave" ),    //      10016
		XOR( "Lunar Weave" ),      //      10013
		XOR( "Convoy" ),           //      10015
		XOR( "Diamondback" ),      //      10040
		// handwraps
		XOR( "Cobalt Skulls" ),    //      10053
		XOR( "Overprint" ),        //      10054
		XOR( "Arboreal" ),         //      10056
		XOR( "Duct Tape" ),        //      10055
		XOR( "Slaughter" ),        //      10021
		XOR( "Badlands" ),         //      10036
		XOR( "Leather" ),          //      10009
		XOR( "Spruce DDPAT" ),     //      10010
		// moto
		XOR( "POW!" ),             //      10049
		XOR( "Transport" ),        //      10051
		XOR( "Polygon" ),          //      10052
		XOR( "Turtle" ),           //      10050
		XOR( "Spearmint" ),        //      10026
		XOR( "Boom!" ),            //      10027
		XOR( "Cool Mint" ),        //      10028
		XOR( "Eclipse" ),          //      10024
		// specialist
		XOR( "Fade" ),             //      10063
		XOR( "Crimson Web" ),      //      10061
		XOR( "Mogul" ),            //      10064
		XOR( "Buckshot" ),         //      10062
		XOR( "Crimson Kimono" ),   //      10033
		XOR( "Emerald Web" ),      //      10034
		XOR( "Foundation" ),       //      10035
		XOR( "Forest DDPAT" ),     //      10030
	};

	std::vector<const char*> knife_type = {
		XOR( "Bayonet" ),
		XOR( "Bowie" ),
		XOR( "Butterfly" ),
		XOR( "Falchion" ),
		XOR( "Flip" ),
		XOR( "Gut" ),
		XOR( "Huntsman" ),
		XOR( "Karambit" ),
		XOR( "M9 bayonet" ),
		XOR( "Daggers" )
	};

	std::vector<const char*> glow_type = {
		XOR( "Bloodhound" ),
		XOR( "Sport" ),
		XOR( "Driver" ),
		XOR( "Handwraps" ),
		XOR( "Moto" ),
		XOR( "Specialist" )
	};

	ImGuiStyle* style = &ImGui::GetStyle( );
	ImVec4* colors = style->Colors;
	InsertSpacer( XOR( "Top Spacer" ) );

	static int id = 0;
	static int id2 = 0;
	auto skin = XOR( "skin_" ) + c_playerlist::get( )->get_skin( ) + XOR( "_" );

	ImGui::Columns( 2, NULL, false );
	{

		InsertGroupBoxLeft( XOR( "Knife options" ), 112.f );
		{
			style->ItemSpacing = ImVec2( 4, 2 );
			style->WindowPadding = ImVec2( 4, 4 );
			ImGui::CustomSpacing( 9.f );

			InsertCheckbox( ( XOR( "Override knife" ) ), b[ ( XOR( "knife" ) ) ] );
			InsertComboWithoutText( XOR( "##Knife model" ), &i[ XOR( "skins_knife_model" ) ], knife_type );

			style->ItemSpacing = ImVec2( 0, 0 );
			style->WindowPadding = ImVec2( 6, 6 );

		} InsertEndGroupBoxLeft( XOR( "Knife options Cover" ), XOR( "Knife options" ) );

		InsertSpacer( XOR( "Knife options - Other Spacer" ) );

		InsertGroupBoxLeft( XOR( "Glove options" ), 325.f );
		{

			style->ItemSpacing = ImVec2( 4, 2 );
			style->WindowPadding = ImVec2( 4, 4 );
			ImGui::CustomSpacing( 9.f );

			InsertCheckbox( ( XOR( "Override glove" ) ), b[ ( XOR( "glove" ) ) ] );
			InsertComboWithoutText( XOR( "##Glove model" ), &i[ XOR( "skins_glove_model" ) ], glow_type );

			static char uname[ 128 ];
			ImGui::Spacing( ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::InputText( XOR( "" ), uname, 128 );

			ImGui::Spacing( ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::ListBoxHeader( XOR( "##players" ), ImVec2( 170, ImGui::GetWindowSize( ).y - 105 ) );

			std::vector<skins_info_t> skins;
			size_t index{};
			for( auto i = bloodhount.begin( ); i != bloodhount.end( ); i++ ) {
				skins_info_t DmgIndicator;
				DmgIndicator.m_name = bloodhount.at( index );
				DmgIndicator.m_id = index;

				skins.push_back( DmgIndicator );

				index++;
			}

			colors[ ImGuiCol_TextDisabled ] = ImColor( 208, 208, 208 );
			for( auto p : skins ) {
				auto search = std::string( uname );
				auto name = std::string( p.m_name );

				std::transform( search.begin( ), search.end( ), search.begin( ), ::tolower );
				std::transform( name.begin( ), name.end( ), name.begin( ), ::tolower );

				if( search != "" && name.find( search ) == std::string::npos )
					continue;

				if( ImGui::Selectable( p.m_name, i[ XOR( "glove_selected" ) ] == p.m_id ) ) {
					i[ XOR( "glove_selected" ) ] = p.m_id;
					g_csgo.cl_fullupdate->m_callback( );
				}
			}
			colors[ ImGuiCol_TextDisabled ] = ImVec4( 0.60f, 0.60f, 0.60f, 1.00f );

			ImGui::ListBoxFooter( );

			style->ItemSpacing = ImVec2( 0, 0 );
			style->WindowPadding = ImVec2( 6, 6 );
		} InsertEndGroupBoxLeft( XOR( "Glove options Cover" ), XOR( "Glove options" ) );
	}
	ImGui::NextColumn( );
	{

		InsertGroupBoxRight( XOR( "Weapon skin" ), 455.f );
		{
			style->ItemSpacing = ImVec2( 4, 2 );
			style->WindowPadding = ImVec2( 4, 4 );
			ImGui::CustomSpacing( 9.f );

			if( !g_csgo.m_engine->IsInGame( ) ) { ImGui::PushDisabled( ); }
			InsertCheckbox( ( XOR( "Enabled" ) ), b[ XOR( "skins_enabled" ) ] );
			InsertCheckbox( ( XOR( "StatTrak" ) ), b[ skin + ( XOR( "stattrak" ) ) ] );
			InsertSlider( XOR( "Quality" ), f[ skin + XOR( "quality" ) ], 0, 100, XOR( "%1.f%%" ) );
			InsertSlider( XOR( "Seed" ), f[ skin + XOR( "seed" ) ], 0, 255, XOR( "%1.f" ) );

			ImGui::Spacing( ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::InputInt2( XOR( "" ), &i[ skin + XOR( "id" ) ] );

			ImGui::Spacing( ); ImGui::NewLine( ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 158.f );
			if( ImGui::Button( XOR( "Update skins" ) ) ) {
				g_skins.m_update = true;

				g_csgo.m_cl->m_delta_tick = -1;
			}
			if( !g_csgo.m_engine->IsInGame( ) ) { ImGui::PopDisabled( ); }

			style->ItemSpacing = ImVec2( 0, 0 );
			style->WindowPadding = ImVec2( 6, 6 );

		} InsertEndGroupBoxRight( XOR( "Weapon skin Cover" ), XOR( "Weapon skin" ) );
	}
}

void Hooks::Legit( ) {
	std::vector<const char*> accuracy_mode = {
		XOR( "Off" ),
		XOR( "Low" ),
		XOR( "Medium" ),
		XOR( "High" ),
		XOR( "Maximum" )
	};

	ImGuiStyle* style = &ImGui::GetStyle( );
	InsertSpacer( XOR( "Top Spacer" ) );

	static int weapontab_rage = 0;

	InsertGroupBoxTop( XOR( "Weapon Selection" ), ImVec2( 515.f, 61.f ) );
	{
		ImGui::Columns( 6, nullptr, false );
		ImGui::PopFont( );
		ImGui::PushFont( weaponzFont );
		style->ButtonTextAlign = ImVec2( 0.4f, 0.75f );
		ImGui::PushItemWidth( 158.f );

		if( ImGui::TabButton( XOR( "G" ), ImVec2( 65, 45 ) ) ) weapontab_rage = 0;

		ImGui::NextColumn( );
		{
			if( ImGui::TabButton( XOR( "P" ), ImVec2( 70, 45 ) ) ) weapontab_rage = 1;
		}
		ImGui::NextColumn( );
		{
			if( ImGui::TabButton( XOR( "W" ), ImVec2( 75, 45 ) ) ) weapontab_rage = 2;
		}
		ImGui::NextColumn( );
		{
			if( ImGui::TabButton( XOR( "d" ), ImVec2( 65, 45 ) ) ) weapontab_rage = 3;
		}
		ImGui::NextColumn( );
		{
			if( ImGui::TabButton( XOR( "f" ), ImVec2( 65, 45 ) ) ) weapontab_rage = 4;
		}
		ImGui::NextColumn( );
		{
			if( ImGui::TabButton( XOR( "a" ), ImVec2( 80, 45 ) ) ) weapontab_rage = 5;
		}


	} InsertEndGroupBoxTop( XOR( "Weapon Selection Cover" ), XOR( "Weapon Selection" ), ImVec2( 536.f, 11.f ) );

	InsertSpacer( XOR( "Weapon Selection - Main Group boxes Spacer" ) );

	ImGui::Columns( 2, NULL, false );
	{

		InsertGroupBoxLeft( XOR( "Aimbot" ), 376.f );
		{
			style->ItemSpacing = ImVec2( 4, 2 );
			style->WindowPadding = ImVec2( 4, 4 );
			ImGui::CustomSpacing( 9.f );

			InsertCheckbox( ( XOR( "Enabled" ) ), b[ ( XOR( "legitbot_enable" ) ) ] );
			InsertSlider( ( XOR( "Speed" ) ), f[ ( XOR( "speed_l" ) ) ], 0.1f, 7.f, "%.1f" );
			InsertSlider( ( XOR( "Speed (in attack)" ) ), f[ ( XOR( "speed_l_a" ) ) ], 0.1f, 7.f, "%.1f" );
			InsertSlider( ( XOR( "Speed scale - FOV" ) ), f[ ( XOR( "l_fov" ) ) ], 0.f, 100.f, "%1.f%%" );
			InsertSlider( ( XOR( "Maximum lock-on time" ) ), f[ ( XOR( "lock-on" ) ) ], 0.f, 100.f, "%1.f" );
			InsertSlider( ( XOR( "Reaction time" ) ), f[ ( XOR( "react" ) ) ], 0.f, 100.f, "%1.fms" );
			InsertSlider( ( XOR( "Maximum FOV" ) ), f[ ( XOR( "l_fov2" ) ) ], 0.f, 100.f, u8"%1.f∞" );
			InsertSlider( ( XOR( "Recoil compensation (P/Y)" ) ), f[ ( XOR( "recolil" ) ) ], 0.f, 130.f, u8"%1.f%%" );
			InsertSliderWithoutText( ( XOR( "##Y" ) ), f[ ( XOR( "reicol2" ) ) ], 0.f, 100.f, u8"%1.f%%" );
			InsertCheckbox( ( XOR( "Quick stop" ) ), b[ ( XOR( "qucistop" ) ) ] );
			InsertCheckbox( ( XOR( "Aim through smoke" ) ), b[ ( XOR( "aimthrouhsome" ) ) ] );
			InsertCheckbox( ( XOR( "Aim while blind" ) ), b[ ( XOR( "blindwhile" ) ) ] );
			InsertCheckbox( ( XOR( "Head" ) ), b[ ( XOR( "head_l" ) ) ] );
			InsertCheckbox( ( XOR( "Chest" ) ), b[ ( XOR( "Chest_l" ) ) ] );
			InsertCheckbox( ( XOR( "Stomach" ) ), b[ ( XOR( "Stomach_l" ) ) ] );

			style->ItemSpacing = ImVec2( 0, 0 );
			style->WindowPadding = ImVec2( 6, 6 );

		} InsertEndGroupBoxLeft( XOR( "Aimbot Cover" ), XOR( "Aimbot" ) );
	}
	ImGui::NextColumn( );
	{

		InsertGroupBoxRight( XOR( "Triggerbot" ), 257.f );
		{
			style->ItemSpacing = ImVec2( 4, 2 );
			style->WindowPadding = ImVec2( 4, 4 );
			ImGui::CustomSpacing( 9.f );

			InsertCheckbox( ( XOR( "Enabled" ) ), b[ ( XOR( "trigerbot_enable" ) ) ] );
			InsertSlider( ( XOR( "Minimum hit chance" ) ), f[ ( XOR( "trigerbot_hitchance" ) ) ], 0.f, 100.f, "%1.f%%" );
			InsertSlider( ( XOR( "Reaction time" ) ), f[ ( XOR( "tr_react" ) ) ], 0.f, 100.f, "%1.fms" );
			InsertCheckbox( ( XOR( "Burst fire" ) ), b[ ( XOR( "trigerbot_burst" ) ) ] );
			InsertSlider( ( XOR( "Minimum damage" ) ), f[ ( XOR( "mindmg_tr" ) ) ], 1.f, 100, "%1.f" );
			InsertCheckbox( ( XOR( "Automatic penetration" ) ), b[ ( XOR( "autowall_tr" ) ) ] );
			InsertCheckbox( ( XOR( "Shoot through smoke" ) ), b[ ( XOR( "smoke_Tr" ) ) ] );
			InsertCheckbox( ( XOR( "Shoot while blind" ) ), b[ ( XOR( "blind_Tr" ) ) ] );
			InsertCheckbox( ( XOR( "Head" ) ), b[ ( XOR( "head_tr" ) ) ] );
			InsertCheckbox( ( XOR( "Chest" ) ), b[ ( XOR( "Chest_tr" ) ) ] );
			InsertCheckbox( ( XOR( "Stomach" ) ), b[ ( XOR( "Stomach_tr" ) ) ] );

			style->ItemSpacing = ImVec2( 0, 0 );
			style->WindowPadding = ImVec2( 6, 6 );

		} InsertEndGroupBoxRight( XOR( "Triggerbot Cover" ), XOR( "Triggerbot" ) );

		InsertSpacer( XOR( "Triggerbot - Other Spacer" ) );

		InsertGroupBoxRight( XOR( "Other" ), 101.f );
		{
			style->ItemSpacing = ImVec2( 4, 2 );
			style->WindowPadding = ImVec2( 4, 4 );
			ImGui::CustomSpacing( 9.f );

			InsertCombo( ( XOR( "Accuracy boost" ) ), &i[ ( XOR( "accuracy_boost_tr" ) ) ], accuracy_mode );
			InsertCheckbox( ( XOR( "Standalone recoil compensation" ) ), b[ ( XOR( "recoil_ls" ) ) ] );

			style->ItemSpacing = ImVec2( 0, 0 );
			style->WindowPadding = ImVec2( 6, 6 );

		} InsertEndGroupBoxRight( XOR( "Other Cover" ), XOR( "Other" ) );
	}
}

void Hooks::Players( ) {
	std::vector<const char*> yaw_mode = {
		XOR( "LBY" ),
		XOR( "Freestand" ),
		XOR( "Last move lby" ),
		XOR( "Custom" )
	};

	std::vector<const char*> yaw_type = {
		XOR( "Add" ),
		XOR( "Set" ),
		XOR( "Subtract" )
	};

	std::vector<const char*> pitch_type = {
		XOR( "Down" ),
		XOR( "Up" ),
		XOR( "Random" ),
		XOR( "Custom" )
	};

	auto players = c_playerlist::get( )->get_players( );
	auto key = XOR( "plist_" ) + c_playerlist::get( )->get_steam_id( players, pi[ XOR( "plist_selected" ) ] ) + XOR( "_" );

	ImGuiStyle* style = &ImGui::GetStyle( );
	ImVec4* colors = style->Colors;
	InsertSpacer( "Top Spacer" );

	ImGui::Columns( 2, NULL, false );
	{

		InsertGroupBoxLeft( XOR( "Players" ), 455.f );
		{
			style->ItemSpacing = ImVec2( 4, 2 );
			style->WindowPadding = ImVec2( 4, 4 );
			ImGui::CustomSpacing( 9.f );

			InsertCheckbox( ( XOR( "Everyone" ) ), b[ XOR( "plist_everyone" ) ] );

			static char uname[ 128 ];
			ImGui::Spacing( ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::InputText( XOR( "" ), uname, 128 );

			ImGui::Spacing( ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::ListBoxHeader( XOR( "##players" ), ImVec2( 170, ImGui::GetWindowSize( ).y - 105 ) );

			colors[ ImGuiCol_TextDisabled ] = ImColor( 208, 208, 208 );
			for( auto p : players ) {
				auto search = std::string( uname );
				auto name = std::string( p.m_name );

				std::transform( search.begin( ), search.end( ), search.begin( ), ::tolower );
				std::transform( name.begin( ), name.end( ), name.begin( ), ::tolower );

				if( search != "" && name.find( search ) == std::string::npos )
					continue;

				if( ImGui::Selectable( p.m_name, pi[ XOR( "plist_selected" ) ] == p.m_user_id ) )
					pi[ XOR( "plist_selected" ) ] = p.m_user_id;
			}
			colors[ ImGuiCol_TextDisabled ] = ImVec4( 0.60f, 0.60f, 0.60f, 1.00f );

			ImGui::ListBoxFooter( );

			ImGui::Spacing( ); ImGui::NewLine( ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 158.f );
			if( ImGui::Button( XOR( "Reset all" ) ) ) {
				for( auto p : players ) {
					key = XOR( "plist_" ) + c_playerlist::get( )->get_steam_id( players, p.m_user_id ) + XOR( "_" );

					pb[ key + XOR( "friend" ) ] = false;
					pb[ key + XOR( "noresolver" ) ] = false;
					pb[ key + XOR( "pitch" ) ] = false;
					pi[ key + XOR( "pitchtype" ) ] = 0;
					pf[ key + XOR( "pitchval" ) ] = 0;
					pb[ key + XOR( "yaw" ) ] = false;
					pf[ key + XOR( "yawval" ) ] = 0;
					pi[ key + XOR( "yawmode" ) ] = 0;
					pi[ key + XOR( "yawtype" ) ] = 0;
					pb[ key + XOR( "baim" ) ] = false;
					pb[ key + XOR( "novis" ) ] = false;
				}
			}
			ImGui::PopItemWidth( );

			style->ItemSpacing = ImVec2( 0, 0 );
			style->WindowPadding = ImVec2( 6, 6 );

		} InsertEndGroupBoxLeft( "Players Cover", XOR( "Players" ) );
	}
	ImGui::NextColumn( );
	{

		InsertGroupBoxRight( XOR( "Adjustments" ), 455.f );
		{
			style->ItemSpacing = ImVec2( 4, 2 );
			style->WindowPadding = ImVec2( 4, 4 );
			ImGui::CustomSpacing( 9.f );

			InsertCheckbox( ( XOR( "Friend" ) ), pb[ key + XOR( "friend" ) ] );
			InsertCheckbox( ( XOR( "Disable resolver" ) ), pb[ key + XOR( "noresolver" ) ] );
			InsertCheckbox( ( XOR( "Pitch" ) ), pb[ key + XOR( "pitch" ) ] );

			if( pb[ key + "pitch" ] ) {
				InsertComboWithoutText( XOR( "##Type" ), &pi[ key + XOR( "pitchtype" ) ], pitch_type );
				if( pi[ key + XOR( "pitchtype" ) ] == 3 ) {
					InsertSliderWithoutText( XOR( "##custompitch" ), pf[ key + XOR( "pitchval" ) ], -90, 90, XOR( u8"%1.f∞" ) );
				}
			}

			InsertCheckbox( ( XOR( "Yaw" ) ), pb[ key + XOR( "yaw" ) ] );
			if( pb[ key + XOR( "yaw" ) ] ) {
				InsertComboWithoutText( XOR( "" ), &pi[ key + XOR( "yawmode" ) ], yaw_mode );
				if( pi[ key + XOR( "yawmode" ) ] == 3 ) {
					InsertSliderWithoutText( XOR( "##customyaw" ), pf[ key + XOR( "yawval" ) ], -180, 180, XOR( u8"%1.f∞" ) );
					InsertCombo( XOR( "Type" ), &pi[ key + XOR( "yawtype" ) ], yaw_type );
				}
			}

			InsertCheckbox( ( XOR( "Prefer body-aim" ) ), pb[ key + XOR( "baim" ) ] );
			InsertCheckbox( ( XOR( "Disable visuals" ) ), pb[ key + XOR( "novis" ) ] );

			ImGui::Spacing( ); ImGui::NewLine( ); ImGui::NewLine( ); ImGui::SameLine( 42.f ); ImGui::PushItemWidth( 158.f );
			if( ImGui::Button( XOR( "Apply to all" ) ) ) {
				for( auto p : players ) {
					auto key_temp = XOR( "plist_" ) + c_playerlist::get( )->get_steam_id( players, p.m_user_id ) + XOR( "_" );

					pb[ key_temp + XOR( "friend" ) ] = pb[ key + XOR( "friend" ) ];
					pb[ key_temp + XOR( "noresolver" ) ] = pb[ key + XOR( "noresolver" ) ];
					pb[ key_temp + XOR( "pitch" ) ] = pb[ key + XOR( "pitch" ) ];
					pi[ key_temp + XOR( "pitchtype" ) ] = pi[ key + XOR( "pitchtype" ) ];
					pf[ key_temp + XOR( "pitchval" ) ] = pf[ key + XOR( "pitchval" ) ];
					pb[ key_temp + XOR( "yaw" ) ] = pb[ key + XOR( "yaw" ) ];
					pf[ key_temp + XOR( "yawval" ) ] = pf[ key + XOR( "yawval" ) ];
					pi[ key_temp + XOR( "yawmode" ) ] = pi[ key + XOR( "yawmode" ) ];
					pi[ key_temp + XOR( "yawtype" ) ] = pi[ key + XOR( "yawtype" ) ];
					pb[ key_temp + XOR( "baim" ) ] = pb[ key + XOR( "baim" ) ];
					pb[ key_temp + XOR( "novis" ) ] = pb[ key + XOR( "novis" ) ];
				}
			}
			ImGui::PopItemWidth( );

			style->ItemSpacing = ImVec2( 0, 0 );
			style->WindowPadding = ImVec2( 6, 6 );

		} InsertEndGroupBoxRight( XOR( "Adjustments Cover" ), XOR( "Adjustments" ) );
	}
}

void Hooks::Luatap( ) {

}

auto s = ImVec2{}, p = ImVec2{}, gs = ImVec2{ 680, 485 };
static int beep = 0;
static float alpha = 0.f;

void CircularProgressBar( int x, int y, int r1, int r2, int s, int d, ImColor col, bool invert = false ) {
	for( int i = s; i < s + d; i++ ) {
		auto rad = i * 3.1415 / 180;
		if( !invert )
			ImGui::GetWindowDrawList( )->AddLine( ImVec2( x + cos( rad ) * r1, y + sin( rad ) * r1 ), ImVec2( x + cos( rad ) * r2, y + sin( rad ) * r2 ), col );
		else
			ImGui::GetWindowDrawList( )->AddLine( ImVec2( x - cos( rad ) * r1, y - sin( rad ) * r1 ), ImVec2( x - cos( rad ) * r2, y - sin( rad ) * r2 ), col );
	}
}

struct SpectatorList_t {
	std::string m_name;
	std::string ObserverMode;
};

enum ObserverMode_t : int {
	OBS_MODE_NONE = 0,
	OBS_MODE_DEATHCAM = 1,
	OBS_MODE_FREEZECAM = 2,
	OBS_MODE_FIXED = 3,
	OBS_MODE_IN_EYE = 4,
	OBS_MODE_CHASE = 5,
	OBS_MODE_ROAMING = 6
};

void SpectatorList( ) {
	// dont do if dead.
	if( !g_cl.m_processing )
		return;

	if( g_hooks.i[ XOR( "spectators" ) ] != 2 )
		return;

	std::vector< SpectatorList_t > spectators;
	int h = render::esp.m_size.m_height;

	for( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i ) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );
		if( !player )
			continue;

		if( player->m_bIsLocalPlayer( ) )
			continue;

		if( player->dormant( ) )
			continue;

		if( player->m_lifeState( ) == LIFE_ALIVE || player->m_iHealth( ) > 0 )
			continue;

		if( player->GetObserverTarget( ) != g_cl.m_local )
			continue;

		player_info_t info;
		if( !g_csgo.m_engine->GetPlayerInfo( i, &info ) )
			continue;

		std::string mode;
		switch( player->m_iObserverMode( ) ) {
			case OBS_MODE_IN_EYE:
				mode = ( XOR( "Perspective" ) );
				break;
			case OBS_MODE_CHASE:
				mode = ( XOR( "3rd person" ) );
				break;
			case OBS_MODE_ROAMING:
				mode = ( XOR( "Free look" ) );
				break;
			case OBS_MODE_DEATHCAM:
				mode = ( XOR( "Deathcam" ) );
				break;
			case OBS_MODE_FREEZECAM:
				mode = ( XOR( "Freezecam" ) );
				break;
			case OBS_MODE_FIXED:
				mode = ( XOR( "Fixed" ) );
				break;
			default:
				break;
		}

		SpectatorList_t SpectatorList;
		SpectatorList.m_name = tfm::format( XOR( u8"%s" ), std::string( info.m_name ).substr( 0, 12 ) );
		SpectatorList.ObserverMode = mode;

		spectators.push_back( SpectatorList_t( SpectatorList ) );
	}

	ImGui::SetNextWindowSize( ImVec2( 220, ( ( spectators.size( ) + 1 ) * 14 ) + 25 ) );
	ImGui::Begin( XOR( "##SPECTATORS" ), NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground );
	{
		s = ImVec2( ImGui::GetWindowSize( ).x - ImGui::GetStyle( ).WindowPadding.x * 2, ImGui::GetWindowSize( ).y - ImGui::GetStyle( ).WindowPadding.y * 2 ); p = ImVec2( ImGui::GetWindowPos( ).x + ImGui::GetStyle( ).WindowPadding.x, ImGui::GetWindowPos( ).y + ImGui::GetStyle( ).WindowPadding.y ); auto draw = ImGui::GetWindowDrawList( );
		{//main
			ImGui::PushFont( font );//time
			std::vector<std::string> spect = { XOR( "Spect" ), XOR( "ators" ) };
			ImVec2 spect_size[ 2 ] = { ImGui::CalcTextSize( spect.at( 0 ).c_str( ) ),ImGui::CalcTextSize( spect.at( 1 ).c_str( ) ) };
			draw->AddRectFilled( ImVec2( p.x, p.y ), ImVec2( p.x + s.x, p.y + 20 ), ImColor( 230, 231, 232, 200 ) );
			draw->AddRectFilled( ImVec2( p.x, p.y ), ImVec2( p.x + s.x, p.y + 3 ), ImColor( g_hooks.c[ XOR( "menu_color" ) ][ 0 ], g_hooks.c[ XOR( "menu_color" ) ][ 1 ], g_hooks.c[ XOR( "menu_color" ) ][ 2 ] ), 12, ImDrawCornerFlags_Top );
			draw->AddText( ImVec2( p.x + 10, p.y + 11 - spect_size[ 0 ].y / 2 ), ImColor( g_hooks.c[ XOR( "menu_color" ) ][ 0 ], g_hooks.c[ XOR( "menu_color" ) ][ 1 ], g_hooks.c[ XOR( "menu_color" ) ][ 2 ] ), spect.at( 0 ).c_str( ) );
			draw->AddText( ImVec2( p.x + 10 + spect_size[ 0 ].x, p.y + 11 - spect_size[ 0 ].y / 2 ), ImColor( 220, 220, 220 ), spect.at( 1 ).c_str( ) );
			ImGui::PopFont( );
		}
		{//eyes
			ImGui::PushFont( font );//time
			draw->AddRectFilled( ImVec2( p.x, p.y + 20 ), ImVec2( p.x + s.x, p.y + ( ( spectators.size( ) + 1 ) * 14 ) + 10 ), ImColor( 230, 231, 232, 140 ), 4, ImDrawCornerFlags_Bot );
			for( size_t i{ }; i < spectators.size( ); ++i ) {
				const std::string& name = spectators[ i ].m_name;
				const std::string& observe = spectators[ i ].ObserverMode;
				{
					draw->AddCircleFilled( ImVec2( p.x + 7, p.y + 23 + ImGui::CalcTextSize( name.c_str( ) ).y / 2 + ( i * 14 ) ), 3, ImColor( g_hooks.c[ XOR( "menu_color" ) ][ 0 ], g_hooks.c[ XOR( "menu_color" ) ][ 1 ], g_hooks.c[ XOR( "menu_color" ) ][ 2 ] ), 24 );
					draw->AddText( ImVec2( p.x + 15, p.y + 22 + ( i * 14 ) ), ImColor( 120, 220, 20 ), name.c_str( ) );
					draw->AddText( ImVec2( p.x + ( s.x - 5 - ImGui::CalcTextSize( observe.c_str( ) ).x ), p.y + 22 + ( i * 14 ) ), ImColor( 20, 22, 69 ), observe.c_str( ) );
				}
			}
			ImGui::PopFont( );
		}
	}
	ImGui::End( );
}

struct KeyBind_t {
	std::string m_name;
	std::string m_mode;
};

std::string check( std::string key ) {
	switch( g_hooks.i[ key + XOR( "style" ) ] ) {
		case 0:
			return XOR( "Always On" );
		case 1:
			return XOR( "On Hotkey" );
		case 2:
			return XOR( "Toggle" );
		case 3:
			return XOR( "Off Hotkey" );
	}
}

void KeyBind( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_hooks.b[ XOR( "keybinds" ) ] )
		return;

	std::vector< KeyBind_t > keybinds;
	if( g_hooks.b[ XOR( "fake_latency" ) ] && g_hooks.auto_check( XOR( "fake_latency_key" ) ) ) {
		KeyBind_t KeybindList;
		KeybindList.m_name = XOR( "fake latency" );
		KeybindList.m_mode = check( XOR( "fake_latency_key" ) );

		keybinds.push_back( KeyBind_t( KeybindList ) );
	}

	if( g_hooks.auto_check( XOR( "damage1_key" ) ) && g_hooks.b[ XOR( "body_aim_enable" ) ] ) {
		KeyBind_t KeybindList;
		KeybindList.m_name = XOR( "override body" );
		KeybindList.m_mode = check( XOR( "damage1_key" ) );

		keybinds.push_back( KeyBind_t( KeybindList ) );
	}

	if( g_hooks.auto_check( XOR( "damage_key" ) ) && g_hooks.b[ XOR( "override_hp" ) ] ) {
		KeyBind_t KeybindList;
		KeybindList.m_name = XOR( "override damage" );
		KeybindList.m_mode = check( XOR( "damage_key" ) );

		keybinds.push_back( KeyBind_t( KeybindList ) );
	}

	if( g_hooks.b[ XOR( "auto_peek" ) ] && g_hooks.auto_check( XOR( "auto_peek_key" ) ) ) {
		KeyBind_t KeybindList;
		KeybindList.m_name = XOR( "auto peek" );
		KeybindList.m_mode = check( XOR( "auto_peek_key" ) );

		keybinds.push_back( KeyBind_t( KeybindList ) );
	}

	if( g_hooks.is_key_down( g_hooks.i[ XOR( "override_key" ) ] ) && g_hooks.b[ XOR( "override" ) ] ) {
		KeyBind_t KeybindList;
		KeybindList.m_name = XOR( "override resolver" );
		KeybindList.m_mode = XOR( "on Hotkey" );

		keybinds.push_back( KeyBind_t( KeybindList ) );
	}

	if( g_hooks.auto_check( "thirdperson_key" ) && g_hooks.b[ XOR( "forceThirdPerson" ) ] ) {
		KeyBind_t KeybindList;
		KeybindList.m_name = XOR( "third person" );
		KeybindList.m_mode = check( XOR( "thirdperson_key" ) );

		keybinds.push_back( KeyBind_t( KeybindList ) );
	}
	if( g_menu.main.aimbot.double_tap.get() ) {
		KeyBind_t KeybindList;
		KeybindList.m_name = XOR( "double tap" );

		keybinds.push_back( KeyBind_t( KeybindList ) );
	}
	ImGui::SetNextWindowSize( ImVec2( 220, ( ( keybinds.size( ) + 1 ) * 14 ) + 19 ) );
	ImGui::Begin( "##KEY_BINDS", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_::ImGuiWindowFlags_NoBackground );
	{
		s = ImVec2( ImGui::GetWindowSize( ).x - ImGui::GetStyle( ).WindowPadding.x * 2, ImGui::GetWindowSize( ).y - ImGui::GetStyle( ).WindowPadding.y * 2 ); p = ImVec2( ImGui::GetWindowPos( ).x + ImGui::GetStyle( ).WindowPadding.x, ImGui::GetWindowPos( ).y + ImGui::GetStyle( ).WindowPadding.y ); auto draw = ImGui::GetWindowDrawList( );
		{//main
			ImGui::PushFont( font );//time
			std::vector<std::string> keybind = { XOR( "Key" ), XOR( "binds" ) };
			ImVec2 keybind_size[ 2 ] = { ImGui::CalcTextSize( keybind.at( 0 ).c_str( ) ),ImGui::CalcTextSize( keybind.at( 1 ).c_str( ) ) };
			draw->AddRectFilled( ImVec2( p.x, p.y ), ImVec2( p.x + s.x, p.y + 20 ), ImColor( 230, 231, 232, 200 ) );
			draw->AddRectFilled( ImVec2( p.x, p.y ), ImVec2( p.x + s.x, p.y + 3 ), ImColor( g_hooks.c[ XOR( "menu_color" ) ][ 0 ], g_hooks.c[ XOR( "menu_color" ) ][ 1 ], g_hooks.c[ XOR( "menu_color" ) ][ 2 ] ), 12, ImDrawCornerFlags_Top );
			draw->AddText( ImVec2( p.x + 10, p.y + 11 - keybind_size[ 0 ].y / 2 ), ImColor( g_hooks.c[ XOR( "menu_color" ) ][ 0 ], g_hooks.c[ XOR( "menu_color" ) ][ 1 ], g_hooks.c[ XOR( "menu_color" ) ][ 2 ] ), keybind.at( 0 ).c_str( ) );
			draw->AddText( ImVec2( p.x + 10 + keybind_size[ 0 ].x, p.y + 11 - keybind_size[ 0 ].y / 2 ), ImColor( 220, 220, 220 ), keybind.at( 1 ).c_str( ) );
			ImGui::PopFont( );
		}
		{//bind
			ImGui::PushFont( font );//time
			//draw->AddRectFilled( ImVec2( p.x, p.y + 20 ), ImVec2( p.x + s.x, p.y + 80 ), ImColor( 30, 31, 32, 110 ), 4, ImDrawCornerFlags_Bot );

			for( size_t i{ }; i < keybinds.size( ); ++i ) {
				const std::string& name = keybinds[ i ].m_name;
				const std::string& observe = keybinds[ i ].m_mode;
				{
					draw->AddCircleFilled( ImVec2( p.x + 7, p.y + 23 + ImGui::CalcTextSize( name.c_str( ) ).y / 2 + ( i * 14 ) ), 3, ImColor( g_hooks.c[ XOR( "menu_color" ) ][ 0 ], g_hooks.c[ XOR( "menu_color" ) ][ 1 ], g_hooks.c[ XOR( "menu_color" ) ][ 2 ] ), 24 );

					draw->AddText( ImVec2( p.x + 15 + 1, p.y + 22 + ( i * 14 ) + 1 ), ImColor( 220, 220, 220 ), name.c_str( ) );
					draw->AddText( ImVec2( p.x + ( s.x - 5 - ImGui::CalcTextSize( observe.c_str( ) ).x ) + 1, p.y + 22 + ( i * 14 ) + 1 ), ImColor( 190, 190, 190 ), observe.c_str( ) );

					draw->AddText( ImVec2( p.x + 15, p.y + 22 + ( i * 14 ) ), ImColor( 20, 20, 20 ), name.c_str( ) );
					draw->AddText( ImVec2( p.x + ( s.x - 5 - ImGui::CalcTextSize( observe.c_str( ) ).x ), p.y + 22 + ( i * 14 ) ), ImColor( 220, 220, 220 ), observe.c_str( ) );
				}
			}
			ImGui::PopFont( );
		}
	}
	ImGui::End( );
}

void BombTimer( ) {
	ImGuiIO& io = ImGui::GetIO( ); ( void )io;

	ImGui::SetNextWindowSize( ImVec2( 220, 70 ) );
	ImGui::Begin( XOR( "##BOMB_TIMER" ), NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground );
	{
		s = ImVec2( ImGui::GetWindowSize( ).x - ImGui::GetStyle( ).WindowPadding.x * 2, ImGui::GetWindowSize( ).y - ImGui::GetStyle( ).WindowPadding.y * 2 ); p = ImVec2( ImGui::GetWindowPos( ).x + ImGui::GetStyle( ).WindowPadding.x, ImGui::GetWindowPos( ).y + ImGui::GetStyle( ).WindowPadding.y ); auto draw = ImGui::GetWindowDrawList( );

		if( !g_csgo.m_engine->IsInGame( ) )
			return;

		if( g_hooks.i[ XOR( "planted_c4" ) ] != 2 )
			return;

		// bomb not currently active, do nothing.
		if( !g_visuals.m_c4_planted && !g_gui.m_open )
			return;

		float explode_time_diff = g_visuals.m_planted_c4_explode_time - g_csgo.m_globals->m_curtime;
		float complete = ( g_csgo.m_globals->m_curtime - g_visuals.m_planted_c4_start ) / g_csgo.m_cvar->FindVar( HASH( "mp_c4timer" ) )->GetFloat( );
		math::clamp( g_visuals.m_final_damage, 0, 100 );

		if( g_gui.m_open && !g_visuals.m_c4_planted ) {
			explode_time_diff = 0;
			complete = 0;
		}

		if( beep < 80 * ( 1.f - complete ) )
			beep++;
		else
			beep = 0;

		{//main


			draw->AddRectFilled( p, ImVec2( p.x + s.x, p.y + s.y ), ImColor( 30, 31, 32, 140 ), 6 );
			ImGui::PushFont( weapon_icons );
			auto bomb = ImGui::CalcTextSize( "y" );
			draw->AddText( ImVec2( p.x + 10, p.y + s.y / 2 - bomb.y / 2 ), ImColor( 130, 132, 170 ), "y" );
			if( beep <= 2 )
				draw->AddText( ImVec2( p.x + 10, p.y + s.y / 2 - bomb.y / 2 ), ImColor( 220, 0, 0, int( 255 ) ), "y" );
			CircularProgressBar( p.x + 28, p.y + s.y / 2, bomb.y / 2 + 2, bomb.y / 2 + 4, 90, 360 * ( 1.f - complete ), ImColor( 130, 132, 170 ) );
			ImGui::PopFont( );
		}
		{//other
			{//up
				ImGui::PushFont( font );//time
				draw->AddLine( ImVec2( p.x + 60, p.y + 10 ), ImVec2( p.x + 60, p.y + 45 ), ImColor( 130, 132, 170, 170 ) );
				draw->AddText( ImVec2( p.x + 70, p.y + 8 ), ImColor( 130, 132, 170 ), "Time: " );
				draw->AddText( ImVec2( p.x + 70 + ImGui::CalcTextSize( "Time: " ).x, p.y + 8 ), ImColor( 220, 220, 220 ), std::to_string( int( explode_time_diff ) ).c_str( ) );
				ImGui::PopFont( );
				std::string dist = tfm::format( XOR( "%iFT" ), g_visuals.DistInFt( g_cl.m_local->m_vecOrigin( ), g_visuals.m_planted_c4_explosion_origin ) );
				if( g_gui.m_open && !g_visuals.m_c4_planted ) {
					dist = tfm::format( XOR( "0FT" ) );
				}
				ImGui::PushFont( font );//dist
				draw->AddLine( ImVec2( p.x + 65, p.y + 10 ), ImVec2( p.x + 60, p.y + 45 ), ImColor( 130, 132, 170, 170 ) );
				draw->AddText( ImVec2( p.x + 130, p.y + 8 ), ImColor( 130, 132, 170 ), "Dist: " );
				draw->AddText( ImVec2( p.x + 130 + ImGui::CalcTextSize( "Dist: " ).x, p.y + 8 ), ImColor( 220, 220, 220 ), dist.c_str( ) );
				ImGui::PopFont( );
			}
			{//down
				ImGui::PushFont( font );//plant
				draw->AddLine( ImVec2( p.x + 60, p.y + 10 ), ImVec2( p.x + 60, p.y + 45 ), ImColor( 130, 132, 170, 170 ) );
				draw->AddText( ImVec2( p.x + 70, p.y + 28 ), ImColor( 130, 132, 170 ), "Damage: " );
				draw->AddText( ImVec2( p.x + 70 + ImGui::CalcTextSize( "Damage: " ).x, p.y + 28 ), g_visuals.m_final_damage < 90 ? ImColor( 220, 220, 220 ) : ImColor( 230, 10, 10 ), std::to_string( g_visuals.m_final_damage ).c_str( ) );
				ImGui::PopFont( );

				ImGui::PushFont( font );//damage
				std::string plant = tfm::format( XOR( "Plant: %s" ), g_visuals.m_last_bombsite.substr( 0, 1 ) );
				draw->AddLine( ImVec2( p.x + 63, p.y + 10 ), ImVec2( p.x + 60, p.y + 45 ), ImColor( 130, 132, 170, 170 ) );
				draw->AddText( ImVec2( p.x + 150, p.y + 28 ), ImColor( 130, 132, 170 ), plant.c_str( ) );
				ImGui::PopFont( );
			}
		}
	}
	ImGui::End( );
}

void Hooks::imgui_menu( ) {
	// set new alpha
	ImGui::PushStyleVar( ImGuiStyleVar_Alpha, m_opacity );

	ImGuiStyle* style = &ImGui::GetStyle( );
	style->WindowPadding = ImVec2( 6, 6 );
	ImGui::StyleColorsDark( );
	ImVec4* colors = style->Colors;
	colors[ ImGuiCol_MenuTheme ] = ImColor( g_hooks.c[ XOR( "menu_color" ) ][ 0 ], g_hooks.c[ XOR( "menu_color" ) ][ 1 ], g_hooks.c[ XOR( "menu_color" ) ][ 2 ] );

	ImGui::SetNextWindowSizeConstraints( ImVec2( 640.f, 510.f ), ImVec2( 4096, 4096 ) );
	ImGui::SetNextWindowSize( ImVec2( 640.f, 510.f ) );
	ImGui::BeginMenuBackground( XOR( "cumhook" ), &g_hooks.Is_Open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar );
	{
		ImGui::PushAllowKeyboardFocus( false );
		ImGui::BeginChild( XOR( "Complete Border" ), ImVec2( 628.f, 498.f ), false );
		{

			ImGui::Image( m_pTexture, ImVec2( 628.f, 498.f ) );

		} ImGui::EndChild( );

		ImGui::SameLine( 6.f );

		style->Colors[ ImGuiCol_ChildBg ] = ImColor( 250, 250, 250, 250 );

		ImGui::BeginChild( XOR( "Menu Contents" ), ImVec2( 628.f, 548.f ), false );
		{

			ImGui::ColorBar( XOR( "unicorn" ), ImVec2( 628.f, 2.f ) );

			style->ItemSpacing = ImVec2( 0.f, -1.f );

			ImGui::BeginTabs( XOR( "Tabs" ), ImVec2( 75.f, 542.f ), false );
			{

				style->ItemSpacing = ImVec2( 0.f, 0.f );

				style->ButtonTextAlign = ImVec2( 0.5f, 0.47f );

				ImGui::PushFont( tabFont );

				switch( tab ) {

					case 0:
						ImGui::TabSpacer( XOR( "##Top Spacer" ), ImVec2( 75.f, 20.f ) );

						if( ImGui::SelectedTab( XOR( "A" ), ImVec2( 75.f, 75.f ) ) ) tab = 0;
						if( ImGui::Tab( XOR( "B" ), ImVec2( 75.f, 75.f ) ) ) tab = 1;
						if( ImGui::Tab( XOR( "C" ), ImVec2( 75.f, 75.f ) ) ) tab = 2;
						if( ImGui::Tab( XOR( "D" ), ImVec2( 75.f, 75.f ) ) ) tab = 3;
						if( ImGui::Tab( XOR( "E" ), ImVec2( 75.f, 75.f ) ) ) tab = 4;
						if( ImGui::Tab( XOR( "F" ), ImVec2( 75.f, 75.f ) ) ) tab = 5;

						ImGui::TabSpacer2( XOR( "##Bottom Spacer" ), ImVec2( 75.f, 20.f ) );
						break;
					case 1:
						ImGui::TabSpacer( XOR( "##Top Spacer" ), ImVec2( 75.f, 20.f ) );

						if( ImGui::Tab( XOR( "A" ), ImVec2( 75.f, 75.f ) ) ) tab = 0;
						if( ImGui::SelectedTab( XOR( "B" ), ImVec2( 75.f, 75.f ) ) ) tab = 1;
						if( ImGui::Tab( XOR( "C" ), ImVec2( 75.f, 75.f ) ) ) tab = 2;
						if( ImGui::Tab( XOR( "D" ), ImVec2( 75.f, 75.f ) ) ) tab = 3;
						if( ImGui::Tab( XOR( "E" ), ImVec2( 75.f, 75.f ) ) ) tab = 4;
						if( ImGui::Tab( XOR( "F" ), ImVec2( 75.f, 75.f ) ) ) tab = 5;

						ImGui::TabSpacer2( XOR( "##Bottom Spacer" ), ImVec2( 75.f, 20.f ) );
						break;
					case 2:
						ImGui::TabSpacer( XOR( "##Top Spacer" ), ImVec2( 75.f, 20.f ) );

						if( ImGui::Tab( XOR( "A" ), ImVec2( 75.f, 75.f ) ) ) tab = 0;
						if( ImGui::Tab( XOR( "B" ), ImVec2( 75.f, 75.f ) ) ) tab = 1;
						if( ImGui::SelectedTab( XOR( "C" ), ImVec2( 75.f, 75.f ) ) ) tab = 2;
						if( ImGui::Tab( XOR( "D" ), ImVec2( 75.f, 75.f ) ) ) tab = 3;
						if( ImGui::Tab( XOR( "E" ), ImVec2( 75.f, 75.f ) ) ) tab = 4;
						if( ImGui::Tab( XOR( "F" ), ImVec2( 75.f, 75.f ) ) ) tab = 5;

						ImGui::TabSpacer2( XOR( "##Bottom Spacer" ), ImVec2( 75.f, 20.f ) );
						break;
					case 3:
						ImGui::TabSpacer( XOR( "##Top Spacer" ), ImVec2( 75.f, 20.f ) );

						if( ImGui::Tab( XOR( "A" ), ImVec2( 75.f, 75.f ) ) ) tab = 0;
						if( ImGui::Tab( XOR( "B" ), ImVec2( 75.f, 75.f ) ) ) tab = 1;
						if( ImGui::Tab( XOR( "C" ), ImVec2( 75.f, 75.f ) ) ) tab = 2;
						if( ImGui::SelectedTab( XOR( "D" ), ImVec2( 75.f, 75.f ) ) ) tab = 3;
						if( ImGui::Tab( XOR( "E" ), ImVec2( 75.f, 75.f ) ) ) tab = 4;
						if( ImGui::Tab( XOR( "F" ), ImVec2( 75.f, 75.f ) ) ) tab = 5;

						ImGui::TabSpacer2( XOR( "##Bottom Spacer" ), ImVec2( 75.f, 20.f ) );
						break;
					case 4:
						ImGui::TabSpacer( XOR( "##Top Spacer" ), ImVec2( 75.f, 20.f ) );

						if( ImGui::Tab( XOR( "A" ), ImVec2( 75.f, 75.f ) ) ) tab = 0;
						if( ImGui::Tab( XOR( "B" ), ImVec2( 75.f, 75.f ) ) ) tab = 1;
						if( ImGui::Tab( XOR( "C" ), ImVec2( 75.f, 75.f ) ) ) tab = 2;
						if( ImGui::Tab( XOR( "D" ), ImVec2( 75.f, 75.f ) ) ) tab = 3;
						if( ImGui::SelectedTab( XOR( "E" ), ImVec2( 75.f, 75.f ) ) ) tab = 4;
						if( ImGui::Tab( XOR( "F" ), ImVec2( 75.f, 75.f ) ) ) tab = 5;

						ImGui::TabSpacer2( XOR( "##Bottom Spacer" ), ImVec2( 75.f, 20.f ) );
						break;
					case 5:
						ImGui::TabSpacer( XOR( "##Top Spacer" ), ImVec2( 75.f, 20.f ) );

						if( ImGui::Tab( XOR( "A" ), ImVec2( 75.f, 75.f ) ) ) tab = 0;
						if( ImGui::Tab( XOR( "B" ), ImVec2( 75.f, 75.f ) ) ) tab = 1;
						if( ImGui::Tab( XOR( "C" ), ImVec2( 75.f, 75.f ) ) ) tab = 2;
						if( ImGui::Tab( XOR( "D" ), ImVec2( 75.f, 75.f ) ) ) tab = 3;
						if( ImGui::Tab( XOR( "E" ), ImVec2( 75.f, 75.f ) ) ) tab = 4;
						if( ImGui::SelectedTab( XOR( "F" ), ImVec2( 75.f, 75.f ) ) ) tab = 5;

						ImGui::TabSpacer2( XOR( "##Bottom Spacer" ), ImVec2( 75.f, 20.f ) );
						break;
				}

				ImGui::PopFont( );

				style->ButtonTextAlign = ImVec2( 0.5f, 0.5f );

			} ImGui::EndTabs( );

			ImGui::SameLine( 75.f );

			ImGui::BeginChild( XOR( "Tab Contents" ), ImVec2( 572.f, 542.f ), false );
			{

				style->Colors[ ImGuiCol_Border ] = ImColor( 220, 220, 220, 220 );

				ImGui::PushFont( menuFont );

				switch( tab ) {

					case 0:
						g_hooks.Aimbot( );
						break;
					case 1:
						g_hooks.Legit( );
						break;
					case 2:
						g_hooks.Visuals( );
						break;
					case 3:
						g_hooks.Misc( );
						break;
					case 4:
						g_hooks.Skins( );
						break;
					case 5:
						g_hooks.Players( );
						break;
				}

				ImGui::PopFont( );

				style->Colors[ ImGuiCol_Border ] = ImColor( 210, 210, 210, m_alpha );

			} ImGui::EndChild( );

			style->ItemSpacing = ImVec2( 4.f, 4.f );
			style->Colors[ ImGuiCol_ChildBg ] = ImColor( 217, 217, 217, m_alpha );

		} ImGui::EndChild( );

	} ImGui::End( );
}

static bool m_bInitialized = false;
void initialize( IDirect3DDevice9* pDevice ) {
	if( m_bInitialized )
		return;

	ImGui::CreateContext( );
	auto io = ImGui::GetIO( );
	auto style = &ImGui::GetStyle( );

	style->WindowRounding = 0.f;
	style->AntiAliasedLines = true;
	style->AntiAliasedFill = true;
	style->ScrollbarRounding = 0.f;
	style->ScrollbarSize = 6.f;
	style->GrabMinSize = 0.f;
	style->TabRounding = 0.f;
	style->WindowPadding = ImVec2( 0, 0 );
	style->Colors[ ImGuiCol_ScrollbarBg ] = ImVec4( 45 / 255.f, 45 / 255.f, 45 / 255.f, 1.f );
	style->Colors[ ImGuiCol_ScrollbarGrab ] = ImVec4( 65 / 255.f, 65 / 255.f, 65 / 255.f, 1.f );

	IDirect3DSwapChain9* pChain = nullptr;
	D3DPRESENT_PARAMETERS pp = {};
	D3DDEVICE_CREATION_PARAMETERS param = {};
	pDevice->GetCreationParameters( &param );
	pDevice->GetSwapChain( 0, &pChain );

	if( pChain )
		pChain->GetPresentParameters( &pp );

	ImGui_ImplWin32_Init( param.hFocusWindow );
	ImGui_ImplDX9_Init( pDevice );

	D3DXCreateTextureFromFileInMemoryEx(
		pDevice, Texture, sizeof( Texture ), 628, 498, D3DX_DEFAULT, NULL,
		pp.BackBufferFormat, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, NULL, NULL, NULL, &m_pTexture );

	ImFontConfig cfg;
	ImFontConfig font_config;
	font_config.OversampleH = 1; //or 2 is the same
	font_config.OversampleV = 1;
	font_config.PixelSnapH = 1;

	static const ImWchar ranges[ ] =
	{
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x0400, 0x044F, // Cyrillic
		0,
	};

	menuFont = io.Fonts->AddFontFromMemoryCompressedTTF(MyFont_compressed_data, MyFont_compressed_size, 15, &font_config, ranges );
	controlFont = io.Fonts->AddFontFromMemoryCompressedTTF(MyFont_compressed_data, MyFont_compressed_size, 17, &font_config, ranges );
	boldMenuFont = io.Fonts->AddFontFromMemoryCompressedTTF(MyFont_compressed_data, MyFont_compressed_size, 16.5, &font_config, ranges );
	tabFont = io.Fonts->AddFontFromMemoryCompressedTTF(MyFont_compressed_data, MyFont_compressed_size, 50.0f, &font_config, ranges );
	keybinds = io.Fonts->AddFontFromMemoryCompressedTTF(MyFont_compressed_data, MyFont_compressed_size, 15.f, &font_config, ranges );
	weaponzFont = io.Fonts->AddFontFromMemoryCompressedTTF(MyFont_compressed_data, MyFont_compressed_size, 31.0f, &font_config, ranges );
	weapon_icons = io.Fonts->AddFontFromMemoryCompressedTTF(MyFont_compressed_data, MyFont_compressed_size, 42.0f, &font_config, ranges );
	font = io.Fonts->AddFontFromMemoryCompressedTTF(MyFont_compressed_data, MyFont_compressed_size, 20.0f, &font_config, ranges );

	g_hooks.load_defaults( );

	m_bInitialized = true;
}

HRESULT __stdcall Hooks::Present( IDirect3DDevice9* pDevice, RECT* pRect1, const RECT* pRect2, HWND hWnd, const RGNDATA* pRGNData ) {
	IDirect3DVertexDeclaration9* vert_dec;
	IDirect3DVertexShader9* vert_shader;
	IDirect3DStateBlock9* state = NULL;

	pDevice->GetVertexShader( &vert_shader );
	pDevice->GetVertexDeclaration( &vert_dec );
	pDevice->CreateStateBlock( D3DSBT_PIXELSTATE, &state );

	static auto wanted_ret_address = _ReturnAddress( );
	if( _ReturnAddress( ) == wanted_ret_address ) {
		DWORD colorwrite, srgbwrite;
		pDevice->GetRenderState( D3DRS_COLORWRITEENABLE, &colorwrite );
		pDevice->GetRenderState( D3DRS_SRGBWRITEENABLE, &srgbwrite );
		pDevice->SetRenderState( D3DRS_COLORWRITEENABLE, 0xffffffff );
		pDevice->SetRenderState( D3DRS_SRGBWRITEENABLE, false );

		initialize( pDevice );

		// opacity should reach 1 in 500 milliseconds.
		constexpr float frequency = 1.f / 0.1f;

		int x = g_cl.m_width;
		int y = g_cl.m_height;

		// the increment / decrement per frame.
		float step = frequency * g_csgo.m_globals->m_frametime;

		// if open		-> increment
		// if closed	-> decrement
		g_gui.m_open ? g_hooks.m_opacity += step : g_hooks.m_opacity -= step;

		// clamp the opacity.
		math::clamp( g_hooks.m_opacity, 0.f, 1.f );

		g_hooks.m_alpha = 0xff * g_hooks.m_opacity;

		ImGui_ImplDX9_NewFrame( );
		ImGui_ImplWin32_NewFrame( );
		ImGui::NewFrame( );

		KeyBind( );
		SpectatorList( );
		BombTimer( );
		g_visuals.SpreadCrosshair( );
		if( g_hooks.m_alpha ) {
			g_hooks.imgui_menu( );
		}

		ImGui::EndFrame( );
		ImGui::Render( );
		ImGui_ImplDX9_RenderDrawData( ImGui::GetDrawData( ) );

		pDevice->SetRenderState( D3DRS_COLORWRITEENABLE, colorwrite );
		pDevice->SetRenderState( D3DRS_SRGBWRITEENABLE, srgbwrite );
	}

	state->Apply( );
	state->Release( );

	pDevice->SetVertexShader( vert_shader );
	pDevice->SetVertexDeclaration( vert_dec );

	return g_hooks.m_device.GetOldMethod<decltype( &Present )>( 17 )( pDevice, pRect1, pRect2, hWnd, pRGNData );
}

HRESULT __stdcall Hooks::Reset( IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentParameters ) {
	ImGui_ImplDX9_InvalidateDeviceObjects( );
	auto result = g_hooks.m_device.GetOldMethod<decltype( &Reset )>( 16 )( pDevice, pPresentParameters );

	if( result >= 0 )
		ImGui_ImplDX9_CreateDeviceObjects( );

	return result;
}