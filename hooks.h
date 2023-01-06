#pragma once

enum ECstrike15UserMessages {
	CS_UM_VGUIMenu = 1,
	CS_UM_Geiger = 2,
	CS_UM_Train = 3,
	CS_UM_HudText = 4,
	CS_UM_SayText = 5,
	CS_UM_SayText2 = 6,
	CS_UM_TextMsg = 7,
	CS_UM_HudMsg = 8,
	CS_UM_ResetHud = 9,
	CS_UM_GameTitle = 10,
	CS_UM_Shake = 12,
	CS_UM_Fade = 13,
	CS_UM_Rumble = 14,
	CS_UM_CloseCaption = 15,
	CS_UM_CloseCaptionDirect = 16,
	CS_UM_SendAudio = 17,
	CS_UM_RawAudio = 18,
	CS_UM_VoiceMask = 19,
	CS_UM_RequestState = 20,
	CS_UM_Damage = 21,
	CS_UM_RadioText = 22,
	CS_UM_HintText = 23,
	CS_UM_KeyHintText = 24,
	CS_UM_ProcessSpottedEntityUpdate = 25,
	CS_UM_ReloadEffect = 26,
	CS_UM_AdjustMoney = 27,
	CS_UM_UpdateTeamMoney = 28,
	CS_UM_StopSpectatorMode = 29,
	CS_UM_KillCam = 30,
	CS_UM_DesiredTimescale = 31,
	CS_UM_CurrentTimescale = 32,
	CS_UM_AchievementEvent = 33,
	CS_UM_MatchEndConditions = 34,
	CS_UM_DisconnectToLobby = 35,
	CS_UM_PlayerStatsUpdate = 36,
	CS_UM_DisplayInventory = 37,
	CS_UM_WarmupHasEnded = 38,
	CS_UM_ClientInfo = 39,
	CS_UM_XRankGet = 40,
	CS_UM_XRankUpd = 41,
	CS_UM_CallVoteFailed = 45,
	CS_UM_VoteStart = 46,
	CS_UM_VotePass = 47,
	CS_UM_VoteFailed = 48,
	CS_UM_VoteSetup = 49,
	CS_UM_ServerRankRevealAll = 50,
	CS_UM_SendLastKillerDamageToClient = 51,
	CS_UM_ServerRankUpdate = 52,
	CS_UM_ItemPickup = 53,
	CS_UM_ShowMenu = 54,
	CS_UM_BarTime = 55,
	CS_UM_AmmoDenied = 56,
	CS_UM_MarkAchievement = 57,
	CS_UM_MatchStatsUpdate = 58,
	CS_UM_ItemDrop = 59,
	CS_UM_GlowPropTurnOff = 60,
	CS_UM_SendPlayerItemDrops = 61,
	CS_UM_RoundBackupFilenames = 62,
	CS_UM_SendPlayerItemFound = 63,
	CS_UM_ReportHit = 64,
	CS_UM_XpUpdate = 65,
	CS_UM_QuestProgress = 66,
	CS_UM_ScoreLeaderboardData = 67,
	CS_UM_PlayerDecalDigitalSignature = 68,
	CS_UM_WeaponSound = 69,
	CS_UM_UpdateScreenHealthBar = 70,
	CS_UM_EntityOutlineHighlight = 71,
	CS_UM_SSUI = 72,
	CS_UM_SurvivalStats = 73,
	CS_UM_DisconnectToLobby2 = 74,
	CS_UM_EndOfMatchAllPlayersData = 75,
	CS_UM_RoundImpactScoreData = 79,
	CS_UM_CurrentRoundOdds = 80,
	CS_UM_DeepStats = 81
};

struct msg_t {
	int entity_id;
	std::string msg;
};

struct luaMenuItem
{
	enum luamenuTypes
	{
		LUAMENU_CHECKBOX = 0,
		LUAMENU_SLIDER,
		LUAMENU_COMBO
	};
	enum luamenuTabLoc
	{
		LUAMENU_RAGEBOTITEM = 0,
		LUAMENU_VISUALITEM
	};
	int type;
	void* value;
	int slidermax, slidermin;
	std::string name;
	int tabLocation;
	std::string tag;
	std::vector < std::string > comboitems;


};


class Hooks {
public:
	Color imcolor_to_ccolor(float* col);
	void init( );

public:
	// prototypes.
	using PaintTraverse_t = void( __thiscall* )( void*, VPANEL, bool, bool );
	using DoPostScreenSpaceEffects_t = bool( __thiscall* )( void*, CViewSetup* );
	using CreateMove_t = bool( __thiscall* )( void*, float, CUserCmd* );
	using LevelInitPostEntity_t = void( __thiscall* )( void* );
	using LevelShutdown_t = void( __thiscall* )( void* );
	using LevelInitPreEntity_t = void( __thiscall* )( void*, const char* );
	using IN_KeyEvent_t = int( __thiscall* )( void*, int, int, const char* );
	using FrameStageNotify_t = void( __thiscall* )( void*, Stage_t );
	using UpdateClientSideAnimation_t = void( __thiscall* )( void* );
	//using SetupBones_t				   = bool( __fastcall* )( void* ,matrix3x4_t*, int, int, float);
	using GetActiveWeapon_t = Weapon * ( __thiscall* )( void* );
	using StandardBlendingRules_t = void( __thiscall* )( void*, int, int, int, int, int );
	using DoExtraBoneProcessing_t = void( __thiscall* )( void*, int, int, int, int, int, int );
	using BuildTransformations_t = void( __thiscall* )( void*, int, int, int, int, int, int );
	using CalcViewModelView_t = void( __thiscall* )( void*, vec3_t&, ang_t& );
	using InPrediction_t = bool( __thiscall* )( void* );
	using OverrideView_t = void( __thiscall* )( void*, CViewSetup* );
	using LockCursor_t = void( __thiscall* )( void* );
	using RunCommand_t = void( __thiscall* )( void*, Entity*, CUserCmd*, IMoveHelper* );
	using ProcessPacket_t = void( __thiscall* )( void*, void*, bool );
	using SendDatagram_t = int( __thiscall* )( void*, void* );
	// using CanPacket_t                = bool( __thiscall* )( void* );
	using PlaySound_t = void( __thiscall* )( void*, const char* );
	using GetScreenSize_t = void( __thiscall* )( void*, int&, int& );
	using Push3DView_t = void( __thiscall* )( void*, CViewSetup&, int, void*, void* );
	using SceneEnd_t = void( __thiscall* )( void* );
	using DrawModelExecute_t = void( __thiscall* )( void*, uintptr_t, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4_t* );
	using ComputeShadowDepthTextures_t = void( __thiscall* )( void*, const CViewSetup&, bool );
	using GetInt_t = int( __thiscall* )( void* );
	using GetBool_t = bool( __thiscall* )( void* );
	using IsConnected_t = bool( __thiscall* )( void* );
	using IsHLTV_t = bool( __thiscall* )( void* );
	using IsPaused_t = bool( __thiscall* )( void* );
	using OnEntityCreated_t = void( __thiscall* )( void*, Entity* );
	using OnEntityDeleted_t = void( __thiscall* )( void*, Entity* );
	using RenderSmokeOverlay_t = void( __thiscall* )( void*, bool );
	using ShouldDrawFog_t = bool( __thiscall* )( void* );
	using ShouldDrawParticles_t = bool( __thiscall* )( void* );
	using Render2DEffectsPostHUD_t = void( __thiscall* )( void*, const CViewSetup& );
	using OnRenderStart_t = void( __thiscall* )( void* );
	using RenderView_t = void( __thiscall* )( void*, const CViewSetup&, const CViewSetup&, int, int );
	using GetMatchSession_t = CMatchSessionOnlineHost * ( __thiscall* )( void* );
	using OnScreenSizeChanged_t = void( __thiscall* )( void*, int, int );
	using OverrideConfig_t = bool( __thiscall* )( void*, MaterialSystem_Config_t*, bool );
	using PostDataUpdate_t = void( __thiscall* )( void*, DataUpdateType_t );
	using DispatchUserMessage_t = bool( __thiscall* )( void*, void*, void*, int, unsigned int, unsigned int, const void* );
	using WriteUsercmdDeltaToBuffer_t = bool( __thiscall* )( void*, int, bf_write*, int, int, bool );
	using TempEntities_t = bool( __thiscall* )( void*, void* );
	// using PreDataUpdate_t            = void( __thiscall* )( void*, DataUpdateType_t );

	typedef bool( __thiscall* tDispatchUserMessage )( void*, int type, unsigned int a3, unsigned int length, const void* msg_data );
	typedef bool( __thiscall* SetupBones_t )( void*, matrix3x4_t*, int, int, float );
	typedef int( __thiscall* SendDatagramFn )( INetChannel*, void* );
	typedef int( __thiscall* ProcessPacketFn )( INetChannel*, void*, bool );
	// hkProcessPacket( INetChannel* ecx, void* edx, void* packet, bool header )

public:
	bool                     TempEntities( void* msg );
	void                     PaintTraverse( VPANEL panel, bool repaint, bool force );
	bool                     DoPostScreenSpaceEffects( CViewSetup* setup );
	bool                     CreateMove( float input_sample_time, CUserCmd* cmd );
	void                     LevelInitPostEntity( );
	void                     LevelShutdown( );
	//int                      IN_KeyEvent( int event, int key, const char* bind );
	void                     LevelInitPreEntity( const char* map );
	void                     FrameStageNotify( Stage_t stage );
	void                     UpdateClientSideAnimation( );
	Weapon* GetActiveWeapon( );
	void                     StandardBlendingRules( int a2, int a3, int a4, int a5, int a6 );
	bool                     InPrediction( );
	bool                     ShouldDrawParticles( );
	bool                     ShouldDrawFog( );
	void                     OverrideView( CViewSetup* view );
	void                     LockCursor( );
	void                     PlaySound( const char* name );
	void                     OnScreenSizeChanged( int oldwidth, int oldheight );
	void                     RunCommand( Entity* ent, CUserCmd* cmd, IMoveHelper* movehelper );
	int                      SendDatagram( void* data );
	int						 hkSendDatagram( INetChannel* thisptr, void* edx, void* datagram );
	void                     ProcessPacket( void* packet, bool header );
	void                     hkProcessPacket( INetChannel* ecx, void* edx, void* packet, bool header );
	//void                     GetScreenSize( int& w, int& h );
	void                     SceneEnd( );
	void                     DrawModelExecute( uintptr_t ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bone );
	void                     ComputeShadowDepthTextures( const CViewSetup& view, bool unk );
	int                      DebugSpreadGetInt( );
	bool                     NetShowFragmentsGetBool( );
	void                     DoExtraBoneProcessing( int a2, int a3, int a4, int a5, int a6, int a7 );
	void                     BuildTransformations( int a2, int a3, int a4, int a5, int a6, int a7 );
	bool                     IsConnected( );
	bool                     IsHLTV( );
	bool                     IsPaused( );
	void                     RenderSmokeOverlay( bool unk );
	void                     OnRenderStart( );
	void                     RenderView( const CViewSetup& view, const CViewSetup& hud_view, int clear_flags, int what_to_draw );
	void                     Render2DEffectsPostHUD( const CViewSetup& setup );
	CMatchSessionOnlineHost* GetMatchSession( );
	bool                     OverrideConfig( MaterialSystem_Config_t* config, bool update );
	bool                     DispatchUserMessage( void* _this, void* edx, int nType, unsigned int nPassthroughFlags, unsigned int nSize, const void* pData );
	void                     PostDataUpdate( DataUpdateType_t type );
	bool					 WriteUsercmdDeltaToBuffer( int slot, bf_write* buf, int from, int to, bool isnewcommand );

	//bool __fastcall SetupBones(void* ECX, void* EDX, matrix3x4_t* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime);

	static HRESULT WINAPI Present( IDirect3DDevice9* pDevice, RECT* pRect1, const RECT* pRect2, HWND hWnd, const RGNDATA* pRGNData );
	static HRESULT WINAPI Reset( IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentParameters );
	static LRESULT WINAPI WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

public:
	// vmts.
	VMT m_panel;
	VMT m_client_mode;
	VMT m_client;
	VMT m_client_state;
	VMT m_engine;
	VMT m_prediction;
	VMT m_surface;
	VMT m_render;
	VMT m_net_channel;
	VMT m_render_view;
	VMT m_model_render;
	VMT m_shadow_mgr;
	VMT m_view_render;
	VMT m_match_framework;
	VMT m_material_system;
	VMT m_fire_bullets;
	VMT m_net_show_fragments;
	VMT m_device;

	// player shit.
	std::array< VMT, 65 > m_player;
	std::array< VMT, 65 > m_renderable;

	// cvars
	VMT m_debug_spread;

	// wndproc old ptr.
	WNDPROC m_old_wndproc;

	// old player create fn.
	DoExtraBoneProcessing_t     m_DoExtraBoneProcessing;
	UpdateClientSideAnimation_t m_UpdateClientSideAnimation;
	DWORD						m_SetupBones = 0;
	GetActiveWeapon_t           m_GetActiveWeapon;
	StandardBlendingRules_t		m_StandardBlendingRules;
	SendDatagramFn				original_send_datagram = 0;
	ProcessPacketFn				oProcessPacket = 0;
	BuildTransformations_t      m_BuildTransformations;
	tDispatchUserMessage	    oDispatchUserMessage;

	// netvar proxies.
	RecvVarProxy_t m_Pitch_original;
	RecvVarProxy_t m_Yaw_original;
	RecvVarProxy_t m_Body_original;
	RecvVarProxy_t m_Force_original;
	RecvVarProxy_t m_AbsYaw_original;
	RecvVarProxy_t m_SimTime_original;

public:
	std::unordered_map<std::string, bool> b;
	std::unordered_map<std::string, int> i;
	std::unordered_map<std::string, float> f;
	std::unordered_map<std::string, float[ 4 ]> c;
	std::unordered_map<std::string, std::unordered_map<int, bool>> m;

	std::unordered_map<std::string, bool> pb;
	std::unordered_map<std::string, int> pi;
	std::unordered_map<std::string, float> pf;
	std::unordered_map<std::string, float[ 4 ]> pc;
	std::unordered_map<std::string, std::unordered_map<int, bool>> pm;

	std::unordered_map<int, std::vector<luaMenuItem>> luaMenuItems;

	std::vector<const char*> presets = { "Alpha", "Beta", "Gamma", "Delta", "Epsilon", "Zeta", "Eta", "Theta", "Iota", "Kappa", "Lambda" , "Mu" };
	// все мб дис? или не с ригзом он скажет че нашел там приколы заходи
	float m_opacity;
	int   m_alpha;

	std::string color_to_string( float col[ 4 ] );
	float* string_to_color( std::string s );

	void save( );
	void load( );
	void load_keys( );
	void save_keys( );
	void load_defaults( );

	bool is_key_down( int key );
	bool Is_Open = true;
	bool is_key_up( int key );
	bool is_key_pressed( int key );
	bool auto_check( std::string key );
	int getping();
	void Aimbot( );
	void Visuals( );
	void Misc( );
	void Skins( );
	void Players( );
	void Legit( );
	void Luatap();
	void imgui_menu( );

public:
	float impact_time;
	vec3_t impact_origin;
	std::deque<msg_t> msgs_;

};

// note - dex; these are defined in player.cpp.
class CustomEntityListener : public IEntityListener {
public:
	virtual void OnEntityCreated( Entity* ent );
	virtual void OnEntityDeleted( Entity* ent );

	__forceinline void init( ) {
		g_csgo.AddListenerEntity( this );
	}
};

extern Hooks                g_hooks;
extern CustomEntityListener g_custom_entity_listener;