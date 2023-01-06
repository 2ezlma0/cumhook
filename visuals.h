#pragma once

struct OffScreenDamageData_t {
    float m_time, m_color_step;
    Color m_color;

    __forceinline OffScreenDamageData_t( ) : m_time{ 0.f }, m_color{ colors::white } {}
    __forceinline OffScreenDamageData_t( float time, float m_color_step, Color color ) : m_time{ time }, m_color{ color } {}
};

struct DamageIndicator_t {
    int ihitbox;
    int iDamage;
    bool bInitialized;
    float flEraseTime;
    float flLastUpdate;
    Player* Player;
    vec3_t Position;
};

/* sound info */
class sound_info {
public:
    sound_info( vec3_t positions, float times, int userids ) {
        this->position = positions;
        this->time = times;
        this->userid = userids;
    }

    vec3_t position;
    float time;
    int userid;
};

struct impact_info {
    float x, y, z;
    float time;
};

struct hitmarker_info {
    impact_info impact;
    int alpha;
    float time;
};

class Visuals {
public:
    std::array< bool, 64 >                  m_draw;
    std::array< float, 2048 >               m_opacities;
    std::array< OffScreenDamageData_t, 64 > m_offscreen_damage;
    std::vector< DamageIndicator_t >        DamageIndicator;
    std::vector< sound_info >               sound_logs;
    std::vector< impact_info >              impacts;
    std::vector< hitmarker_info >           hitmarkers;
    vec2_t                                  m_crosshair;
    bool                                    m_thirdperson;
    float					                m_hit_start, m_hit_end, m_hit_duration;

    // info about planted c4.
    bool        m_c4_planted;
    Entity* m_planted_c4;
    float       m_planted_c4_explode_time;
    float       m_planted_c4_start;
    float       m_planted_c4_end;
    vec3_t      m_planted_c4_explosion_origin;
    float       m_planted_c4_damage;
    float       m_planted_c4_radius;
    float       m_planted_c4_radius_scaled;
    float       m_planted_c4_defuse;
    bool        m_planted_c4_indefuse;
    float       m_defuse_start;
    float       m_defuse_end;
    float       m_defuse_duration;
    bool        m_planted_c4_planting;
    float       m_plant_start;
    float       m_plant_end;
    float       m_plant_duration;
    int         m_final_damage;
    std::string m_last_bombsite;
    std::string m_who_planting;

    IMaterial* smoke1;
    IMaterial* smoke2;
    IMaterial* smoke3;
    IMaterial* smoke4;

    std::unordered_map< int, char > m_weapon_icons = {
        { DEAGLE, 'A' },
        { ELITE, 'B' },
        { FIVESEVEN, 'C' },
        { GLOCK, 'D' },
        { AK47, 'W' },
        { AUG, 'U' },
        { AWP, 'Z' },
        { FAMAS, 'R' },
        { G3SG1, 'X' },
        { GALIL, 'Q' },
        { M249, 'g' },
        { M4A4, 'S' },
        { MAC10, 'K' },
        { P90, 'P' },
        { UMP45, 'L' },
        { XM1014, 'b' },
        { BIZON, 'M' },
        { MAG7, 'd' },
        { NEGEV, 'f' },
        { SAWEDOFF, 'c' },
        { TEC9, 'H' },
        { ZEUS, 'h' },
        { P2000, 'E' },
        { MP7, 'N' },
        { MP9, 'O' },
        { NOVA, 'e' },
        { P250, 'F' },
        { SCAR20, 'Y' },
        { SG553, 'V' },
        { SSG08, 'a' },
        { KNIFE_CT, ']' },
        { FLASHBANG, 'i' },
        { HEGRENADE, 'j' },
        { SMOKE, 'k' },
        { MOLOTOV, 'l' },
        { DECOY, 'm' },
        { FIREBOMB, 'n' },
        { C4, 'o' },
        { KNIFE_T, '[' },
        { M4A1S, 'T' },
        { USPS, 'G' },
        { CZ75A, 'I' },
        { REVOLVER, 'J' },
        { KNIFE_BAYONET, '1' },
        { KNIFE_FLIP, '2' },
        { KNIFE_GUT, '3' },
        { KNIFE_KARAMBIT, '4' },
        { KNIFE_M9_BAYONET, '5' },
        { KNIFE_HUNTSMAN, '6' },
        { KNIFE_FALCHION, '0' },
        { KNIFE_BOWIE, '7' },
        { KNIFE_BUTTERFLY, '8' },
        { KNIFE_SHADOW_DAGGERS, '9' },
    };

public:
    float DistInFt( vec3_t o, vec3_t dest );

    static void ModulateWorld( );
    void ThirdpersonThink( );
    void Hitmarker( );
    void NoSmoke( );
    void think( );
    void Hitmarker3D( );
    void DrawSpread( );
    void AutomaticPeekIndicator( );
    void DrawDamageIndicator( );
    void ManualAntiAim( );
    void Spectators( );
    void StatusIndicators( );
    void SpreadCrosshair( );
    void PenetrationCrosshair( );
    void DrawPlantedC4( );
    void draw( Entity* ent );
    void DrawProjectile( Weapon* ent );
    void DrawItem( Weapon* item );
    void OffScreen( Player* player, int alpha );
    void DrawPlayer( Player* player );
    bool GetPlayerBoxRect( Player* player, Rect& box );
    void DrawHistorySkeleton( Player* player, int opacity );
    void DrawSkeleton( Player* player, int opacity );
    void RenderGlow( );
    void DrawHitboxMatrix( LagRecord* record, Color col, float time );
    void DrawBeams( );
    void DebugAimbotPoints( Player* player );
    void draw_circle( Color color, vec3_t position );
    void draw_sound( );
};

extern Visuals g_visuals;