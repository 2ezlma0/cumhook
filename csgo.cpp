#include "includes.h"

CSGO	   g_csgo{};;
WinAPI	   g_winapi{};;
Netvars	   g_netvars{};;
EntOffsets g_entoffsets{};;
Menu       g_menu{};;
Notify     g_notify{};


#include <iphlpapi.h>
#include "cpuid.h"
#include "sha256.h"

#pragma comment(lib, "IPHLPAPI.lib")
std::string MACAddressHash( ) {
	VMPBSTART( "mac address" );
	std::string sRet;
	IP_ADAPTER_INFO AdapterInfo[ 16 ];
	DWORD dwBufLen = sizeof( AdapterInfo );
	DWORD dwStatus = LI_FN( GetAdaptersInfo ).get( )( AdapterInfo, &dwBufLen );
	//DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
	if( dwStatus == ERROR_SUCCESS ) {
		PIP_ADAPTER_INFO pAdapterInfo1 = AdapterInfo;
		do {
			sRet += pAdapterInfo1->AdapterName;
			sRet += ( char )pAdapterInfo1->Address[ 1 ];
			sRet += ( char )pAdapterInfo1->Address[ 2 ];
			sRet += ( char )pAdapterInfo1->Address[ 4 ];
			sRet += ( char )pAdapterInfo1->Address[ 5 ];

			//sprintf(szBuf, XOR("%s%02X-%02X-%02X-%02X-%02X"), pAdapterInfo1->AdapterName, pAdapterInfo1->Address[1], pAdapterInfo1->Address[2], pAdapterInfo1->Address[3], pAdapterInfo1->Address[4], pAdapterInfo1->Address[5]);
			//sRet.append(szBuf);
			pAdapterInfo1 = pAdapterInfo1->Next;
		} while( pAdapterInfo1 );
	}
	return sRet;
	VMPEND( );
}

void* cmemset( void* dest, int val, size_t len ) {
	VMProtectBeginUltra( "memset" );
	unsigned char* ptr = ( unsigned char* )dest;
	while( len > 0 )
		*ptr++ = val;
	return dest;
	VMProtectEnd( );
}

void* cmemcpy( void* dest, const void* src, size_t len ) {
	VMProtectBeginUltra( "memcpy" );
	char* d = ( char* )dest;
	const char* s = ( const char* )src;
	while( len-- )
		*d++ = *s++;
	return dest;
	VMProtectEnd( );
}

#include <ctime>


std::string removeChars( std::string str ) {
	VMPBSTART( "remove chars" );
	int occurs = 0;
	std::string newstr;
	for( int i = 0; i < str.length( ); i++ ) {
		if( ( str[ i ] >= '0' ) && ( str[ i ] <= '9' ) ) {
			occurs++;
			newstr += str[ i ];
			if( occurs > 6 )
				break;
		}
	}
	return newstr;
	VMPEND( );
}

int cstrcmp( const char* p1, const char* p2 ) {
	VMPBSTART( "strcmp" );
	const unsigned char* s1 = ( const unsigned char* )p1;
	const unsigned char* s2 = ( const unsigned char* )p2;
	unsigned char c1, c2;

	do {
		c1 = ( unsigned char )*s1++;
		c2 = ( unsigned char )*s2++;
		if( c1 == '\0' )
			return c1 - c2;
	} while( c1 == c2 );

	return c1 - c2;
	VMPEND( );
}
#pragma optimize( "", off ) // типа выдел€ешь код и он не опимизируетс€ по идее

bool CSGO::init( uint32_t* clientHWID ) {
	VMPBSTART( "csgo init" );
	m_done = false;

	if( m_done )
		return false;

	__time64_t t = LI_FN( _time64 ).get( )( 0 );//std::time(0);   // get time now
	std::tm* now = LI_FN( _localtime64 ).get( )( &t );//std::localtime(&t);

	// wait for the game to init.
	// "serverbrowser.dll" is the last module to be loaded.
	// if it gets loaded we can be ensured that the entire game done loading.

//#ifndef KOLO
	while( !m_serverbrowser_dll ) {
		m_serverbrowser_dll = PE::GetModule( HASH( "serverbrowser.dll" ) );
		if( !m_serverbrowser_dll )
			std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
	}
	//#endif

		// grab some modules.
	m_kernel32_dll = PE::GetModule( HASH( "kernel32.dll" ) );
	m_user32_dll = PE::GetModule( HASH( "user32.dll" ) );
	m_shell32_dll = PE::GetModule( HASH( "shell32.dll" ) );
	m_shlwapi_dll = PE::GetModule( HASH( "shlwapi.dll" ) );
	m_client_dll = PE::GetModule( HASH( "client.dll" ) );
	m_engine_dll = PE::GetModule( HASH( "engine.dll" ) );
	m_vstdlib_dll = PE::GetModule( HASH( "vstdlib.dll" ) );
	m_tier0_dll = PE::GetModule( HASH( "tier0.dll" ) );
	m_shaderapidx9_dll = PE::GetModule( HASH( "shaderapidx9.dll" ) );
	m_vguimatsurface_dll = PE::GetModule( HASH( "vguimatsurface.dll" ) );

	// import winapi functions.
	g_winapi.WideCharToMultiByte = PE::GetExport( m_kernel32_dll, HASH( "WideCharToMultiByte" ) ).as< WinAPI::WideCharToMultiByte_t >( );
	g_winapi.MultiByteToWideChar = PE::GetExport( m_kernel32_dll, HASH( "MultiByteToWideChar" ) ).as< WinAPI::MultiByteToWideChar_t >( );
	g_winapi.GetTickCount = PE::GetExport( m_kernel32_dll, HASH( "GetTickCount" ) ).as< WinAPI::GetTickCount_t >( );
	g_winapi.VirtualProtect = PE::GetExport( m_kernel32_dll, HASH( "VirtualProtect" ) ).as< WinAPI::VirtualProtect_t >( );
	g_winapi.VirtualQuery = PE::GetExport( m_kernel32_dll, HASH( "VirtualQuery" ) ).as< WinAPI::VirtualQuery_t >( );
	g_winapi.CreateDirectoryA = PE::GetExport( m_kernel32_dll, HASH( "CreateDirectoryA" ) ).as< WinAPI::CreateDirectoryA_t >( );
	g_winapi.SetWindowLongA = PE::GetExport( m_user32_dll, HASH( "SetWindowLongA" ) ).as< WinAPI::SetWindowLongA_t >( );
	g_winapi.CallWindowProcA = PE::GetExport( m_user32_dll, HASH( "CallWindowProcA" ) ).as< WinAPI::CallWindowProcA_t >( );
	g_winapi.SHGetFolderPathA = PE::GetExport( m_shell32_dll, HASH( "SHGetFolderPathA" ) ).as< WinAPI::SHGetFolderPathA_t >( );
	g_winapi.PathAppendA = PE::GetExport( m_shlwapi_dll, HASH( "PathAppendA" ) ).as< WinAPI::PathAppendA_t >( );

	// run interface collection code.
	Interfaces interfaces{};

	// get interface pointers.
	m_client = interfaces.get< CHLClient* >( HASH( "VClient" ) );
	m_cvar = interfaces.get< ICvar* >( HASH( "VEngineCvar" ) );
	m_engine = interfaces.get< IVEngineClient* >( HASH( "VEngineClient" ) );
	m_entlist = interfaces.get< IClientEntityList* >( HASH( "VClientEntityList" ) );
	m_input_system = interfaces.get< IInputSystem* >( HASH( "InputSystemVersion" ) );
	m_surface = interfaces.get< ISurface* >( HASH( "VGUI_Surface" ) );
	m_panel = interfaces.get< IPanel* >( HASH( "VGUI_Panel" ) );
	m_engine_vgui = interfaces.get< IEngineVGui* >( HASH( "VEngineVGui" ) );
	m_prediction = interfaces.get< CPrediction* >( HASH( "VClientPrediction" ) );
	m_engine_trace = interfaces.get< IEngineTrace* >( HASH( "EngineTraceClient" ) );
	m_game_movement = interfaces.get< CGameMovement* >( HASH( "GameMovement" ) );
	m_render_view = interfaces.get< IVRenderView* >( HASH( "VEngineRenderView" ) );
	m_model_render = interfaces.get< IVModelRender* >( HASH( "VEngineModel" ) );
	m_material_system = interfaces.get< IMaterialSystem* >( HASH( "VMaterialSystem" ) );
	m_studio_render = interfaces.get< CStudioRenderContext* >( HASH( "VStudioRender" ) );
	m_model_info = interfaces.get< IVModelInfo* >( HASH( "VModelInfoClient" ) );
	m_debug_overlay = interfaces.get< IVDebugOverlay* >( HASH( "VDebugOverlay" ) );
	m_phys_props = interfaces.get< IPhysicsSurfaceProps* >( HASH( "VPhysicsSurfaceProps" ) );
	m_game_events = interfaces.get< IGameEventManager2* >( HASH( "GAMEEVENTSMANAGER" ), 1 );
	m_match_framework = interfaces.get< CMatchFramework* >( HASH( "MATCHFRAMEWORK_" ) );
	m_localize = interfaces.get< ILocalize* >( HASH( "Localize_" ) );
	m_networkstringtable = interfaces.get< INetworkStringTableContainer* >( HASH( "VEngineClientStringTable" ) );
	m_sound = interfaces.get< IEngineSound* >( HASH( "IEngineSoundClient" ) );

	// convars.
	clear = m_cvar->FindVar( HASH( "clear" ) );
	toggleconsole = m_cvar->FindVar( HASH( "toggleconsole" ) );
	name = m_cvar->FindVar( HASH( "name" ) );
	sv_maxunlag = m_cvar->FindVar( HASH( "sv_maxunlag" ) );
	sv_gravity = m_cvar->FindVar( HASH( "sv_gravity" ) );
	sv_jump_impulse = m_cvar->FindVar( HASH( "sv_jump_impulse" ) );
	sv_enablebunnyhopping = m_cvar->FindVar( HASH( "sv_enablebunnyhopping" ) );
	sv_airaccelerate = m_cvar->FindVar( HASH( "sv_airaccelerate" ) );
	sv_friction = m_cvar->FindVar( HASH( "sv_friction" ) );
	sv_footsteps = m_cvar->FindVar( HASH( "sv_footsteps" ) );
	sv_min_jump_landing_sound = m_cvar->FindVar( HASH( "sv_min_jump_landing_sound" ) );
	sv_stopspeed = m_cvar->FindVar( HASH( "sv_stopspeed" ) );
	sv_accelerate = m_cvar->FindVar( HASH( "sv_accelerate" ) );
	cl_interp = m_cvar->FindVar( HASH( "cl_interp" ) );
	cl_interp_ratio = m_cvar->FindVar( HASH( "cl_interp_ratio" ) );
	cl_updaterate = m_cvar->FindVar( HASH( "cl_updaterate" ) );
	cl_cmdrate = m_cvar->FindVar( HASH( "cl_cmdrate" ) );
	cl_lagcompensation = m_cvar->FindVar( HASH( "cl_lagcompensation" ) );
	mp_teammates_are_enemies = m_cvar->FindVar( HASH( "mp_teammates_are_enemies" ) );
	weapon_debug_spread_show = m_cvar->FindVar( HASH( "weapon_debug_spread_show" ) );
	r_jiggle_bones = m_cvar->FindVar( HASH( "r_jiggle_bones" ) );
	molotov_throw_detonate_time = m_cvar->FindVar( HASH( "molotov_throw_detonate_time" ) );
	weapon_molotov_maxdetonateslope = m_cvar->FindVar( HASH( "weapon_molotov_maxdetonateslope" ) );
	weapon_recoil_scale = m_cvar->FindVar( HASH( "weapon_recoil_scale" ) );
	view_recoil_tracking = m_cvar->FindVar( HASH( "view_recoil_tracking" ) );
	cl_fullupdate = m_cvar->FindVar( HASH( "cl_fullupdate" ) );
	r_DrawSpecificStaticProp = m_cvar->FindVar( HASH( "r_DrawSpecificStaticProp" ) );
	cl_crosshair_sniper_width = m_cvar->FindVar( HASH( "cl_crosshair_sniper_width" ) );
	hud_scaling = m_cvar->FindVar( HASH( "hud_scaling" ) );
	sv_clip_penetration_traces_to_players = m_cvar->FindVar( HASH( "sv_clip_penetration_traces_to_players" ) );
	weapon_accuracy_shotgun_spread_patterns = m_cvar->FindVar( HASH( "weapon_accuracy_shotgun_spread_patterns" ) );
	net_showfragments = m_cvar->FindVar( HASH( "net_showfragments" ) );
	mat_ambient_light_r = m_cvar->FindVar( HASH( "mat_ambient_light_r" ) );
	mat_ambient_light_g = m_cvar->FindVar( HASH( "mat_ambient_light_g" ) );
	mat_ambient_light_b = m_cvar->FindVar( HASH( "mat_ambient_light_b" ) );

	// hehe xd.
	name->m_callbacks.RemoveAll( );
	//cl_lagcompensation->m_callbacks.RemoveAll( );
	//cl_lagcompensation->m_flags &= ~FCVAR_NOT_CONNECTED;

	// classes by sig.
	m_move_helper = pattern::find( m_client_dll, XOR( "8B 0D ? ? ? ? 8B 46 08 68" ) ).add( 2 ).get< IMoveHelper* >( 2 );
	m_cl = pattern::find( m_engine_dll, XOR( "B9 ? ? ? ? 56 FF 50 14 8B 34 85" ) ).add( 1 ).get< CGlobalState* >( )->m_client_state;
	m_game = pattern::find( m_engine_dll, XOR( "A1 ? ? ? ? B9 ? ? ? ? FF 75 08 FF 50 34" ) ).add( 1 ).get< CGame* >( );
	m_render = pattern::find( m_engine_dll, XOR( "A1 ? ? ? ? B9 ? ? ? ? FF 75 0C FF 75 08 FF 50 0C" ) ).add( 1 ).get< CRender* >( );
	m_shadow_mgr = pattern::find( m_client_dll, XOR( "A1 ? ? ? ? FF 90 ? ? ? ? 6A 00" ) ).add( 1 ).get( ).as< IClientShadowMgr* >( );
	m_view_render = pattern::find( m_client_dll, XOR( "8B 0D ? ? ? ? 8B 01 FF 50 4C 8B 06" ) ).add( 2 ).get< CViewRender* >( 2 );
	// m_entity_listeners   = pattern::find( m_client_dll, XOR( "B9 ? ? ? ? E8 ? ? ? ? 5E 5D C2 04" ) ).add( 0x1 ).get< IClientEntityListener** >( 2 );
	m_hud = pattern::find( m_client_dll, XOR( "B9 ? ? ? ? 0F 94 C0 0F B6 C0 50 68" ) ).add( 0x1 ).get( ).as< CHud* >( );
	m_gamerules = pattern::find( m_client_dll, XOR( "8B 0D ? ? ? ? E8 ? ? ? ? 84 C0 75 6B" ) ).add( 0x2 ).get< C_CSGameRules* >( );
	m_beams = pattern::find( m_client_dll, XOR( "8D 04 24 50 A1 ? ? ? ? B9" ) ).add( 0x5 ).get< IViewRenderBeams* >( );
	m_mem_alloc = PE::GetExport( m_tier0_dll, HASH( "g_pMemAlloc" ) ).get< IMemAlloc* >( );
	GetGlowObjectManager = pattern::find( m_client_dll, XOR( "A1 ? ? ? ? A8 01 75 4B" ) ).as< GetGlowObjectManager_t >( );
	m_glow = GetGlowObjectManager( );
	m_hookable_cl = reinterpret_cast< void* >( *reinterpret_cast< uintptr_t** >( reinterpret_cast< uintptr_t >( m_cl ) + 0x8 ) );

	// classes by offset from virtual.
	m_globals = util::get_method( m_client, CHLClient::INIT ).add( 0x1b ).get< CGlobalVarsBase* >( 2 );
	m_client_mode = util::get_method( m_client, CHLClient::HUDPROCESSINPUT ).add( 0x5 ).get< IClientMode* >( 2 );
	m_input = util::get_method( m_client, CHLClient::INACTIVATEMOUSE ).at< CInput* >( 0x1 );

	// functions.
	MD5_PseudoRandom = pattern::find( m_client_dll, XOR( "55 8B EC 83 E4 F8 83 EC 70 6A 58" ) ).as< MD5_PseudoRandom_t >( );
	SetAbsAngles = pattern::find( m_client_dll, XOR( "55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1 E8" ) );
	InvalidateBoneCache = pattern::find( m_client_dll, XOR( "80 3D ? ? ? ? ? 74 16 A1 ? ? ? ? 48 C7 81" ) );
	LockStudioHdr = pattern::find( m_client_dll, XOR( "55 8B EC 51 53 8B D9 56 57 8D B3" ) );
	SetAbsOrigin = pattern::find( m_client_dll, XOR( "55 8B EC 83 E4 F8 51 53 56 57 8B F1" ) );
	IsBreakableEntity = pattern::find( m_client_dll, XOR( "55 8B EC 51 56 8B F1 85 F6 74 68 83 BE" ) ).as< IsBreakableEntity_t >( );
	SetAbsVelocity = pattern::find( m_client_dll, XOR( "55 8B EC 83 E4 F8 83 EC 0C 53 56 57 8B 7D 08 8B F1" ) );
	AngleMatrix = pattern::find( m_client_dll, XOR( "E8 ? ? ? ? 8B 07 89 46 0C" ) ).rel32( 0x1 ).as< AngleMatrix_t >( );
	ComputeHitboxSurroundingBox = pattern::find( m_client_dll, XOR( "E9 ? ? ? ? 32 C0 5D" ) ).rel32( 0x1 );
	GetSequenceActivity = pattern::find( m_client_dll, XOR( "53 56 8B F1 8B DA 85 F6 74 55" ) );
	LoadFromBuffer = pattern::find( m_client_dll, XOR( "E8 ? ? ? ? 88 44 24 0F 8B 56 FC" ) ).rel32( 0x1 ).as< LoadFromBuffer_t >( );
	ServerRankRevealAll = pattern::find( m_client_dll, XOR( "55 8B EC 8B 0D ? ? ? ? 68" ) ).as< ServerRankRevealAll_t >( );
	HasC4 = pattern::find( m_client_dll, XOR( "E8 ? ? ? ? 38 83" ) ).rel32( 0x1 );
	InvalidatePhysicsRecursive = pattern::find( m_client_dll, XOR( "E8 ? ? ? ? 89 5E 18" ) ).rel32( 0x1 );
	IsReady = pattern::find( m_client_dll, XOR( "E8 ? ? ? ? 59 C2 08 00 51 E8" ) ).rel32( 0x1 ).as< IsReady_t >( );
	ShowAndUpdateSelection = pattern::find( m_client_dll, XOR( "E8 ? ? ? ? A1 ? ? ? ? F3 0F 10 40 ? C6 83" ) ).rel32( 0x1 ).as< ShowAndUpdateSelection_t >( );
	GetEconItemView = pattern::find( m_client_dll, XOR( "8B 81 ? ? ? ? 81 C1 ? ? ? ? FF 50 04 83 C0 40 C3" ) ).as< GetEconItemView_t >( );
	GetStaticData = pattern::find( m_client_dll, XOR( "55 8B EC 51 56 57 8B F1 E8 ? ? ? ? 0F B7 8E" ) ).as< GetStaticData_t >( );
	TEFireBullets = pattern::find( m_client_dll, XOR( "C7 05 ? ? ? ? ? ? ? ? C7 05 ? ? ? ? ? ? ? ? 66 A3" ) ).add( 0x2 ).to( );
	BeamAlloc = pattern::find( m_client_dll, XOR( "E8 ? ? ? ? 8B F0 85 F6 74 7C" ) ).rel32< BeamAlloc_t >( 0x1 );
	SetupBeam = pattern::find( m_client_dll, XOR( "E8 ? ? ? ? 8B 07 33 C9" ) ).rel32< SetupBeam_t >( 0x1 );
	ClearNotices = pattern::find( m_client_dll, XOR( "E8 ? ? ? ? 68 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? 8B F0 85 F6 74 19" ) ).rel32< ClearNotices_t >( 0x1 );
	AddListenerEntity = pattern::find( m_client_dll, XOR( "55 8B EC 8B 0D ? ? ? ? 33 C0 56 85 C9 7E 32 8B 55 08 8B 35" ) ).as< AddListenerEntity_t >( );
	GetShotgunSpread = pattern::find( m_client_dll, XOR( "E8 ? ? ? ? EB 38 83 EC 08" ) ).rel32< GetShotgunSpread_t >( 1 );
	BoneAccessor = pattern::find( m_client_dll, XOR( "8D 81 ? ? ? ? 50 8D 84 24" ) ).add( 2 ).to< size_t >( );
	AnimOverlay = pattern::find( m_client_dll, XOR( "8B 80 ? ? ? ? 8D 34 C8" ) ).add( 2 ).to< size_t >( );
	SpawnTime = pattern::find( m_client_dll, XOR( "F3 0F 5C 88 ? ? ? ? 0F" ) ).add( 4 ).to< size_t >( );
	IsLocalPlayer = pattern::find( m_client_dll, XOR( "74 ? 8A 83 ? ? ? ? 88" ) ).add( 4 ).to< size_t >( );
	PlayerAnimState = pattern::find( m_client_dll, XOR( "8B 8E ? ? ? ? 85 C9 74 3E" ) ).add( 2 ).to< size_t >( );
	studioHdr = pattern::find( m_client_dll, XOR( "8B 86 ? ? ? ? 89 44 24 10 85 C0" ) ).add( 2 ).to< size_t >( );
	UTIL_TraceLine = pattern::find( m_client_dll, XOR( "55 8B EC 83 E4 F0 83 EC 7C 56 52" ) );
	ClipRayToHitboxFn = pattern::find( m_client_dll, XOR( "55 8B EC 83 E4 F8 F3 0F 10 42" ) );
	SetCollisionBounds = pattern::find( m_client_dll, XOR( "53 8B DC 83 EC 08 83 E4 F8 83 C4 04 55 8B 6B 04 89 6C 24 04 8B EC 83 EC 10 56 57 8B 7B" ) );
	AttachementsHelper = pattern::find( m_client_dll, XOR( "55 8B EC 83 EC 48 53 8B 5D 08 89 4D F4" ) );
	CTraceFilterSimple_vmt = UTIL_TraceLine.add( 0x3D ).to( );
	CTraceFilterSkipTwoEntities_vmt = pattern::find( m_client_dll, XOR( "E8 ? ? ? ? F3 0F 10 84 24 ? ? ? ? 8D 84 24 ? ? ? ? F3 0F 58 44 24" ) ).rel32( 1 ).add( 0x59 ).to( );
	LastBoneSetupTime = InvalidateBoneCache.add( 0x11 ).to< size_t >( );
	MostRecentModelBoneCounter = InvalidateBoneCache.add( 0x1B ).to< size_t >( );
	m_device = **pattern::find( m_shaderapidx9_dll, XOR( "B9 ? ? ? ? E8 ? ? ? ? 8B 07 57 FF 50 08 F6 46 14" ) ).add( 0x1 ).as< IDirect3DDevice9*** >( );
	DisablePostProcess = pattern::find( m_client_dll, XOR( "80 3D ? ? ? ? ? 53 56 57 0F 85" ) ).add( 0x2 );

	// exported functions.
	RandomSeed = PE::GetExport( m_vstdlib_dll, HASH( "RandomSeed" ) ).as< RandomSeed_t >( );
	RandomInt = PE::GetExport( m_vstdlib_dll, HASH( "RandomInt" ) ).as< RandomInt_t >( );
	RandomFloat = PE::GetExport( m_vstdlib_dll, HASH( "RandomFloat" ) ).as< RandomFloat_t >( );

	// prediction pointers.
	m_nPredictionRandomSeed = util::get_method( m_prediction, CPrediction::RUNCOMMAND ).add( 0x30 ).get< int* >( );
	m_pPredictionPlayer = util::get_method( m_prediction, CPrediction::RUNCOMMAND ).add( 0x54 ).get< Player* >( );

	// some weird tier0 stuff that prevents me from debugging properly...
#ifdef _DEBUG
	Address debugbreak = pattern::find( g_csgo.m_client_dll, XOR( "CC F3 0F 10 4D ? 0F 57 C0" ) );

	DWORD old;
	g_winapi.VirtualProtect( debugbreak, 1, PAGE_EXECUTE_READWRITE, &old );

	debugbreak.set< uint8_t >( 0x90 );

	g_winapi.VirtualProtect( debugbreak, 1, old, &old );
#endif

#ifdef PRODBUILD

	std::string hashedoutput;
	//CPUID cpuID(0); // Get CPU vendor


	std::array<int, 4> cpui;
	std::vector<std::array<int, 4>> data_;
	std::vector<std::array<int, 4>> extdata_;

	__cpuid( cpui.data( ), 0 );
	int nIds_ = cpui[ 0 ];

	for( int i = 0; i <= nIds_; ++i ) {
		__cpuidex( cpui.data( ), i, 0 );
		data_.push_back( cpui );
	}

	char cpuvendor[ 0x20 ];
	memset( cpuvendor, 0, sizeof( cpuvendor ) );
	*reinterpret_cast< int* >( cpuvendor ) = data_[ 0 ][ 1 ];
	*reinterpret_cast< int* >( cpuvendor + 4 ) = data_[ 0 ][ 3 ];
	*reinterpret_cast< int* >( cpuvendor + 8 ) = data_[ 0 ][ 2 ];

	__cpuid( cpui.data( ), 0x80000000 );
	int nExIds_ = cpui[ 0 ];

	char brand[ 0x40 ];
	memset( brand, 0, sizeof( brand ) );

	for( int i = 0x80000000; i <= nExIds_; ++i ) {
		__cpuidex( cpui.data( ), i, 0 );
		extdata_.push_back( cpui );
	}


	if( nExIds_ >= 0x80000004 ) {
		memcpy( brand, extdata_[ 2 ].data( ), sizeof( cpui ) );
		memcpy( brand + 16, extdata_[ 3 ].data( ), sizeof( cpui ) );
		memcpy( brand + 32, extdata_[ 4 ].data( ), sizeof( cpui ) );
	}


	std::string vendor;
	vendor += cpuvendor;
	vendor += brand;
	SYSTEM_INFO siSysInfo;
	//GetSystemInfo(&siSysInfo);
	LI_FN( GetSystemInfo ).get( )( &siSysInfo );
	std::string commonoutput = vendor;
	hashedoutput = sha256( commonoutput ); //бл€ это самый ебланский сурс который € только видел сху€ли бл€ть оно не компилирует € бл€ть только что это скопировал

	hashedoutput = removeChars( hashedoutput );
	int key1 = ( ( now->tm_year + now->tm_mday + now->tm_mon ) * now->tm_hour );
	std::string strKey = std::to_string( key1 );

	std::string result( hashedoutput.size( ), '\0' );

	for( std::size_t i = 0; i < hashedoutput.size( ); i++ ) {
		char xoredChar = hashedoutput[ i ] ^ strKey[ i % strKey.size( ) ];
		int cNum = ( int )xoredChar;
		if( cNum < 40 || cNum > 127 ) {
			cNum = cNum % 87;
			cNum += 40;
		}
		result[ i ] = ( char )cNum;
	}

	//hashedoutput.erase(std::remove_if(hashedoutput.begin(), hashedoutput.end(),
	//	[](char c) { return (!std::isdigit(c)); }),
	//	hashedoutput.end());
	//hashedoutput = hashedoutput.substr(0, 7);
	//	MessageBoxA(NULL, hashedoutput.c_str(), hashedoutput.c_str(), NULL);
	//targetHWID = atoi(hashedoutput.c_str());

	char buff[ 32 ];


	//targetHWID = targetHWID ^ key1;
	//hwid

	std::string loaderHwid( hashedoutput.size( ), '\0' );

	memcpy( loaderHwid.data( ), clientHWID, hashedoutput.size( ) );

	loaderHwid[ hashedoutput.size( ) ] = '\0';

	// спроси у того чел
	//g_netvars.init();
	//g_entoffsets.init();
	//g_listener.init();

	//render::init();
	//g_menu.init();
	//g_config.LoadHotkeys();
	//g_chams.init();
	//g_hooks.init();

	std::string junkshit = XOR( "1" );

	if( cstrcmp( result.c_str( ), loaderHwid.c_str( ) ) == 0 ) {
		if( !std::filesystem::exists( XOR( "C:/jre.codes/lua" ) ) )
			std::filesystem::create_directories( XOR( "C:/jre.codes/lua" ) );

		//junk//
		if( cstrcmp( result.c_str( ), loaderHwid.c_str( ) ) != 0 ) {
			while( junkshit.size( ) < 5 )
				__cpuid( cpui.data( ), 0x80000000 );
		}
		//junk//
#endif
		g_netvars.init( );
#ifdef PRODBUILD

		//junk//
		if( cstrcmp( result.c_str( ), loaderHwid.c_str( ) ) != ( junkshit.size( ) - 1 ) ) {
			__cpuidex( cpui.data( ), junkshit.size( ), cpui.size( ) );
		}
		//junk//
#endif
		g_entoffsets.init( );
		g_listener.init( );

		render::init( );
		g_menu.init( );
		g_config.LoadHotkeys( );
		g_chams.init( );
		g_hooks.init( );

#ifdef PRODBUILD

	}
	else {
		g_chams.GenerateLerpedMatrix( 0, ( matrix3x4_t* )RandomSeed );
		g_netvars.StoreDataMap( m_nPredictionRandomSeed );
		m_nPredictionRandomSeed = util::get_method( m_prediction, CPrediction::RUNCOMMAND ).add( 0x30 ).get< int* >( );
		m_pPredictionPlayer = util::get_method( m_prediction, CPrediction::RUNCOMMAND ).add( 0x54 ).get< Player* >( );
		IsLocalPlayer = pattern::find( m_client_dll, XOR( "74 ? 8A 83 ? ? ? ? 88" ) ).add( 4 ).to< size_t >( );
	}
#endif

	g_notify.add( XOR( "Welcome to cumhook, all issues are due to your config. " ) );
	//g_notify.add(XOR("Welcome"));

	//if(targetHWID == clientHWID)
	//{
	//	if( !(targetHWID != clientHWID) )
	//	{
	//		g_netvars.init();
	//		g_entoffsets.init();
	//		g_listener.init();
	//	}
	//	else
	//	{
	//		g_netvars.StoreDataMap(m_nPredictionRandomSeed);
	//		g_chams.GenerateLerpedMatrix(0, (matrix3x4_t*)RandomSeed);
	//	}
	//	g_notify.add(tfm::format(XOR("Welcome %s\n"), g_cl.m_user));

	//	render::init();
	//	g_menu.init();
	//	g_config.LoadHotkeys();
	//	g_chams.init();
	//	g_hooks.init();
	//}
	//else
	//{
	//	targetHWID = targetHWID ^ key1;
	//	targetHWID = targetHWID ^ key2;
	//	if( targetHWID == clientHWID )
	//	{
	//		g_netvars.init();
	//		if( !(targetHWID != clientHWID) )
	//		{
	//			g_entoffsets.init();
	//			g_listener.init();
	//			render::init();
	//			g_menu.init();
	//			g_config.LoadHotkeys();
	//			g_chams.init();
	//			g_hooks.init();
	//		}
	//		else
	//		{
	//			g_netvars.StoreDataMap(m_nPredictionRandomSeed);
	//			g_chams.GenerateLerpedMatrix(0, (matrix3x4_t*)RandomSeed);
	//		}
	//	}
	//}



 //   // init everything else.
	//g_config.init( );

 //   // g_netvars stores all netvar offsets into an unordered_map, EntOffsets is for the raw offset values so we don't have to access the unordered_map a bunch.
	//g_netvars.init( );
 //   g_entoffsets.init( );

	//g_listener.init( );
	//render::init( );
	//g_menu.init( );
	//g_config.LoadHotkeys( );
	//g_chams.init( );
	//g_hooks.init( );

	//OBF_BEGIN;
//g_config.init( );
//// заливать? нет € ищу зуйню
//// 
//   // g_netvars stores all netvar offsets into an unordered_map, EntOffsets is for the raw offset values so we don't have to access the unordered_map a bunch.
   //
   //hwidcheck

   //IF(V(targetHWID) == V(clientHWID))
   //{
   //	IF( !( V(targetHWID) != V(clientHWID) ) )
   //	{
   //		g_netvars.init();
   //		g_entoffsets.init();
   //		g_listener.init();
   //	} 
   //	ELSE
   //	{
   //		RETURN(0);
   //	}
   //	ENDIF;

   //	render::init();
   //	g_menu.init();
   //	g_config.LoadHotkeys();
   //	g_chams.init();
   //	g_hooks.init();
   //}
   //ELSE
   //{
   //	targetHWID = targetHWID ^ key1;
   //	targetHWID = targetHWID ^ key2;
   //	IF(V(targetHWID) == V(clientHWID))
   //	{
   //		g_netvars.init();
   //		IF(!(V(targetHWID) != V(clientHWID)))
   //		{
   //			g_entoffsets.init();
   //			g_listener.init();
   //			render::init();
   //			g_menu.init();
   //			g_config.LoadHotkeys();
   //			g_chams.init();
   //			g_hooks.init();
   //		} ENDIF;

   //	}
   //	ENDIF;
   //}
   //ENDIF;

   //OBF_END;


	// if we injected and we're ingame, run map load func.
	if( m_engine->IsInGame( ) ) {
		g_cl.OnMapload( );
		g_csgo.cl_fullupdate->m_callback( );
	}

	m_done = true;
	return true;
	VMPEND( );
}

#pragma optimize( "", on ) // типа выдел€ешь код и он не опимизируетс€ по идее


bool game::IsBreakable( Entity* ent ) {
	bool        ret;
	ClientClass* cc;
	const char* name;
	char* takedmg, old_takedmg;

	static size_t m_takedamage_offset{ *( size_t* )( ( uintptr_t )g_csgo.IsBreakableEntity + 38 ) };

	// skip null ents and the world ent.
	if( !ent || ent->index( ) == 0 )
		return false;

	// get m_takedamage and save old m_takedamage.
	takedmg = ( char* )( ( uintptr_t )ent + m_takedamage_offset );
	old_takedmg = *takedmg;

	// get clientclass.
	cc = ent->GetClientClass( );

	if( cc ) {
		// get clientclass network name.
		name = cc->m_pNetworkName;

		// CBreakableSurface, CBaseDoor, ...
		if( name[ 1 ] != 'F'
			|| name[ 4 ] != 'c'
			|| name[ 5 ] != 'B'
			|| name[ 9 ] != 'h' ) {
			*takedmg = DAMAGE_YES;
		}
	}

	ret = g_csgo.IsBreakableEntity( ent );
	*takedmg = old_takedmg;

	return ret;
}

Beam_t* game::CreateGenericBeam( const BeamInfo_t& beam_info ) {
	Beam_t* out;
	const model_t* sprite;

	out = g_csgo.BeamAlloc( g_csgo.m_beams, beam_info.m_bRenderable );
	if( !out )
		return nullptr;

	out->die = g_csgo.m_globals->m_curtime;

	if( beam_info.m_nModelIndex < 0 )
		return nullptr;

	sprite = g_csgo.m_model_info->GetModel( beam_info.m_nModelIndex );
	if( sprite ) {
		out->type = ( beam_info.m_nType < 0 ) ? 0 : beam_info.m_nType;
		out->modelIndex = beam_info.m_nModelIndex;
		out->haloIndex = beam_info.m_nHaloIndex;
		out->haloScale = beam_info.m_flHaloScale;
		out->frame = 0;
		out->frameRate = 0;
		out->frameCount = g_csgo.m_model_info->GetModelFrameCount( sprite );
		out->freq = g_csgo.m_globals->m_curtime * beam_info.m_flSpeed;
		out->die = g_csgo.m_globals->m_curtime + beam_info.m_flLife;
		out->width = beam_info.m_flWidth;
		out->endWidth = beam_info.m_flEndWidth;
		out->fadeLength = beam_info.m_flFadeLength;
		out->amplitude = beam_info.m_flAmplitude;
		out->brightness = beam_info.m_flBrightness;
		out->speed = beam_info.m_flSpeed;
		out->life = beam_info.m_flLife;
		out->flags = 0;
		out->attachment[ 0 ] = beam_info.m_vecStart;
		out->attachment[ 1 ] = beam_info.m_vecEnd;

		out->delta = ( beam_info.m_vecEnd - beam_info.m_vecStart );

		if( beam_info.m_nSegments == -1 ) {
			if( out->amplitude >= 0.50 )
				out->segments = out->delta.length( ) * 0.25 + 3;  // one per 4 pixels
			else
				out->segments = out->delta.length( ) * 0.075 + 3; // one per 16 pixels
		}

		else
			out->segments = beam_info.m_nSegments;
	}

	// note - dex; this is CViewRenderBeams::SetBeamAttributes, but it got inlined so i have to rebuild it.
	//             not sure if this is needed, it's taken from CViewRenderBeams::CreateBeamPoints.
	out->frame = ( float )beam_info.m_nStartFrame;
	out->frameRate = beam_info.m_flFrameRate;
	out->flags |= beam_info.m_nFlags;
	out->r = beam_info.m_flRed;
	out->g = beam_info.m_flGreen;
	out->b = beam_info.m_flBlue;

	return out;
}