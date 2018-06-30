// ####################################
// #                                  #
// #       Ping of Death - Bot        #
// #                by                #
// #    Markus Klinge aka Count Floyd #
// #                                  #
// ####################################
//
// Started from the HPB-Bot Alpha Source
// by Botman so Credits for a lot of the basic
// HL Server/Client Stuff goes to him
//
// bot_globals.cpp
//
// Defines and initializes the global Variables

#include "bot_globals.h"

int g_iMapType; // Type of Map - Assault,Defuse etc...
bool g_bIsDedicatedServer = TRUE;
bool g_bWaypointOn = FALSE;
bool g_bWaypointsChanged = TRUE; // Waypoints changed
bool g_bWaypointsSaved = FALSE;
bool g_bAutoWaypoint = FALSE;
float g_fAutoPathMaxDistance = 250;
bool g_bShowWpFlags = TRUE;
bool g_bLearnJumpWaypoint = FALSE;
int g_iNumWaypoints; // number of waypoints currently in use for each team
int g_iCachedWaypoint = -1;
bool g_bLeaderChosenT = FALSE; // Leader for both Teams chosen
bool g_bLeaderChosenCT = FALSE; // Leader for both Teams chosen
int g_rgiTerrorWaypoints[MAX_WAYPOINTS];
int g_iNumTerrorPoints = 0;
int g_rgiCTWaypoints[MAX_WAYPOINTS];
int g_iNumCTPoints = 0;
int g_rgiGoalWaypoints[MAX_WAYPOINTS];
int g_iNumGoalPoints = 0;
int g_rgiCampWaypoints[MAX_WAYPOINTS];
int g_iNumCampPoints = 0;
int g_rgiRescueWaypoints[MAX_WAYPOINTS];        // KWo - 24.12.2006
int g_iNumRescuePoints = 0;                     // KWo - 24.12.2006
float g_fWPDisplayTime[MAX_WAYPOINTS]; // waypoint display time (while editing) KWo - 05.01.2008
float g_fPathDisplayTime[MAX_WAYPOINTS]; // path display time (while editing) KWo - 05.01.2008
int *g_pFloydDistanceMatrix = NULL; // array of head pointers to the path structures (Distance Table)
bool g_bMapInitialised = FALSE;
bool g_bRecalcVis = TRUE;
float g_fTimeDisplayVisTableMsg = 1.0;
int g_iCurrVisIndex = 0;
int g_iRowVisIndex = 0;                         // KWo - 10.09.2006
unsigned char g_rgbyVisLUT[MAX_WAYPOINTS][MAX_WAYPOINTS / 4];
char g_cKillHistory;
int iNumBotNames = 0;
int iNumKillChats = 0;
int iNumBombChats = 0;
int iNumDeadChats = 0;
int iNumNoKwChats = 0;
int g_iPeoBotsKept = 0;
int g_iMin_bots = 0;
int g_iMax_bots = 0;
float botcreation_time = 0.0;
float g_fLastKickedBotTime = 0.0;
bool g_bIgnoreEnemies = FALSE; // Stores if Bots are told to ignore Enemies (debug feature)
float g_fLastChatTime = 0.0; // Stores Last Time chatted - prevents spamming
float g_fTimeRoundStart = 0.0; // stores the Start of the round (in worldtime)
float g_fTimeRoundEnd = 0.0; // Stores the End of the round (in worldtime) gpGlobals->time+roundtime
float g_fTimeRoundMid = 0.0; // Stores the halftime of the round (in worldtime)
bool g_bRoundEnded = FALSE;  // it says the round is finished (to let bots reload i.e.) // KWo - 30.09.2010
float g_fTimeNextBombUpdate = 0.0; // Holds the time to allow the next Search Update
int g_iLastBombPoint; // Stores the last checked Bomb Waypoint
bool g_bBombPlanted = FALSE; // Stores if the Bomb was planted
bool g_bBombDefusing = FALSE; // Stores the bomb is defusing - KWo - 13.07.2007
int g_iDefuser = -1; // Stores the bomb defuser - KWo - 13.07.2007
bool g_bBombSayString;
float g_fTimeBombPlanted = 0.0; // Holds the time when Bomb was planted
int g_rgiBombSpotsVisited[MAXNUMBOMBSPOTS]; // Stores visited Bombspots for Counters when Bomb has been planted
bool g_bBotsCanPause = FALSE; // Stores if Bots should pause
bool g_bHostageRescued = FALSE; // Stores if Counter rescued a Hostage in a Round
int iRadioSelect[32];
int g_rgfLastRadio[2];
float g_rgfLastRadioTime[2] = {0.0, 0.0}; // Stores Time of RadioMessage - prevents too fast responds
char g_szWaypointMessage[512]; // String displayed to Host telling about Waypoints
int g_iNumLogos = 5; // Number of available Spraypaints
int state; // network message state machine state
int g_iDebugGoalIndex = -1; // Assigns a goal in debug mode
int g_iSearchGoalIndex;
int g_iPointedWpIndex = -1;                     // KWo - 04.10.2006
Vector g_vecBomb = g_vecZero;
edict_t *pHostEdict = NULL; // Ptr to Hosting Edict
bool g_bIsOldCS15 = TRUE;
DLL_FUNCTIONS gFunctionTable;
DLL_FUNCTIONS gFunctionTable_Post;
const Vector g_vecZero = Vector (0, 0, 0);
enginefuncs_t g_engfuncs;
globalvars_t  *gpGlobals;
int gmsgFlashlight;                             // KWo - 25.05.2008
int gmsgNVGToggle;                              // KWo - 26.05.2008
char g_argv[1024];
botname_t *g_pszBotNames = NULL;
char szKillChat[100][256]; // ptr to Kill Messages go here
char szBombChat[100][256]; // ptr to BombPlant Messages go here
char szDeadChat[100][256]; // ptr to Deadchats go here
char szNoKwChat[100][256];
const char *szUsedBotNames[32];// ptr to already used Names
int iUsedDeadChatIndex[10]; // index of Keywords & Replies for interactive Chat - KWo - 23.03.2010
int iUsedUnknownChatIndex[5]; // index of universal interactive Chat - KWo - 27.03.2010
replynode_t *pChatReplies = NULL; // ptr to Strings when no keyword was found
createbot_t BotCreateTab[32];
client_t clients[32]; // Array of connected Clients
bool g_bEditNoclip = FALSE; // Flag for noclip wpt editing
int g_pSpriteTexture = 0; // Index of Beam Sprite
bool isFakeClientCommand = FALSE; // Faked Client Command
int fake_arg_count;
int g_iNum_bots = 0;
int g_iNum_players = 0;
int g_iNum_humans = 0;
int g_iNum_hum_tm = 0;                          // KWo - 08.03.2010
int g_iNum_hum_tm_prev = 0;                     // KWo - 04.03.2010
int g_i_MapBuying = 0;                          // KWo - 07.06.2010
bool g_bResetHud = FALSE;                       // KWo - 04.03.2010
char g_cStoreAddbotSkill[4];
char g_cStoreAddbotPersonality[2];	            // KWo - 08.10.2006
char g_cStoreAddbotTeam[2];	                  // KWo - 08.10.2006
char g_cStoreFillServerSkill[4];
char g_cStoreFillServerPersonality[2];	         // KWo - 08.10.2006
char g_cStoreFillServerTeam[2];	               // KWo - 08.10.2006
FILE *fp;
bool file_opened = FALSE;
bot_t bots[32];
int msecnum = 0; // TheFatal's method for calculating the msec value
float msecdel = 0;
float msecval = 0;
void (*botMsgFunction) (void *, int) = NULL;
int botMsgIndex;
PATH *paths[MAX_WAYPOINTS];
experience_t *pBotExperienceData = NULL;
int *g_pFloydPathMatrix = NULL;
int *g_pWithHostageDistMatrix = NULL;
int *g_pWithHostagePathMatrix = NULL;
unsigned short g_iHighestDamageT;               // KWo - 09.04.2006
unsigned short g_iHighestDamageCT;              // KWo - 09.04.2006
int g_iHighestDamageWpT;                        // KWo - 05.01.2008
int g_iHighestDamageWpCT;                       // KWo - 05.01.2008
bool g_bEndJumpPoint = FALSE;
float g_fTimeJumpStarted = 0.0;
Vector g_vecLearnVelocity = g_vecZero;
Vector g_vecLearnPos = g_vecZero;
Vector g_vecLastWaypoint;
Vector g_vecHostOrigin =  g_vecZero;            // KWo - 20.04.2013
int g_iLastJumpWaypoint = -1;
float g_fTimeRestartServer = 0.0;
char g_szGameDirectory[128] = "cstrike";
bool g_bBotSettings = TRUE;
bool g_bWeaponStrip = FALSE;                    // KWo - 10.03.2013
int g_iNumButtons = 0;                          // KWo - 07.02.2006
int g_iNumBreakables = 0;                       // KWo - 10.02.2006
int g_iNumHostages = 0;                         // KWo - 16.05.2006
int g_iWeaponRestricted[NUM_WEAPONS];           // KWo - 10.03.2006
int g_iEquipAmmoRestricted[NUM_EQUIPMENTS];     // KWo - 10.03.2006
int g_iFrameCounter = 0;                        // KWo - 10.03.2006
bool g_bPathWasCalculated = FALSE;              // KWo - 24.03.2007
bool g_bSaveVisTable = FALSE;                   // KWo - 24.03.2007
int g_iAliveCTs;                                // KWo - 19.01.2008
int g_iAliveTs;                                 // KWo - 19.01.2008
saved_buttons_t ButtonsData[MAX_BUTTONS];       // KWo - 07.02.2006
saved_break_t BreakablesData[MAX_BREAKABLES];   // KWo - 10.02.2006
saved_hostages_t HostagesData[MAX_HOSTAGES];    // KWo - 16.05.2006 - to compare if the hostages been moving
hudtextparms_t g_hudset = {0.0, 0.0, 0, 255, 255, 255, 0, 255, 255, 255, 0, 0.0, 0.0, 0.0, 0.0, 0};    // KWo - 16.01.2010

float g_f_cvars_upd_time = 0.0;                 // KWo - 02.05.2006
float g_f_host_upd_time = 0.0;                  // KWo - 18.05.2006
float g_fTimeAvoidGrenade = 0.0;                // KWo - 29.01.2008
int g_i_botthink_index;                         // KWo - 02.05.2006
float g_fAutoKillTime = 0.0;                    // KWo - 02.05.2006
bool g_bAliveHumansPrevious = FALSE;            // KWo - 02.05.2006
int g_iUpdGlExpState = 0;                       // KWo - 02.05.2006
bool g_bRecalcKills = FALSE;                    // KWo - 02.05.2006
bool g_bDeathMessageSent = FALSE;               // KWo - 02.03.2010

bool g_b_DebugTasks = FALSE;                    // KWo - 20.04.2013
bool g_b_DebugNavig = FALSE;                    // KWo - 20.04.2013
bool g_b_DebugStuck = FALSE;                    // KWo - 20.04.2013
bool g_b_DebugSensing = FALSE;                  // KWo - 20.04.2013
bool g_b_DebugCombat = FALSE;                   // KWo - 20.04.2013
bool g_b_DebugEntities = FALSE;                 // KWo - 20.04.2013
bool g_b_DebugChat = FALSE;                     // KWo - 20.04.2013
bool g_b_DebugWpEdit = FALSE;                   // KWo - 04.05.2013

// KWo - 06.04.2006 - these globals below are used to store data about cvars - they will be tested
//  once a second instead calling all frames for every bot separately GET_CVAR_FLOAT
int g_i_cv_FpsMax;                              // KWo - 06.04.2006
float g_f_cv_Gravity;                           // KWo - 16.11.2006
float g_f_cv_FreezeTime;                        // KWo - 17.11.2006
bool g_b_cv_FriendlyFire;                       // KWo - 06.04.2006
bool g_b_cv_FootSteps;                          // KWo - 06.04.2006
float g_f_cv_c4timer;                           // KWo - 17.04.2006
bool g_b_cv_csdm_active;                        // KWo - 15.04.2008
bool g_b_cv_flash_allowed;                      // KWo - 25.05.2008
float g_f_cv_skycolor;                          // KWo - 26.05.2008
bool g_b_cv_Parachute;                          // KWo - 07.03.2010

int g_i_cv_MinBots;                             // KWo - 06.04.2006
int g_i_cv_MaxBots;                             // KWo - 06.04.2006
int g_i_cv_BotsQuotaMatch;                      // KWo - 16.09.2006
int g_i_cv_BotsJoinTeam;                        // KWo - 16.09.2006
float g_f_cv_MapStartBotJoinDelay;              // KWo - 17.05.2008
float g_f_cv_timer_sound;                       // KWo - 06.04.2006
float g_f_cv_timer_pickup;                      // KWo - 06.04.2006
float g_f_cv_timer_grenade;                     // KWo - 06.04.2006
int g_i_cv_skin;                                // KWo - 18.11.2006
bool g_b_cv_spray;                              // KWo - 06.04.2006
bool g_b_cv_shootthruwalls;                     // KWo - 06.04.2006
int g_i_cv_debuglevel;                          // KWo - 20.04.2013
int g_i_cv_detailnames;                         // KWo - 22.03.2008
bool g_b_cv_UseSpeech;                          // KWo - 07.10.2006
int g_i_cv_numfollowuser;                       // KWo - 06.04.2006
int g_i_cv_maxweaponpickup;                     // KWo - 06.04.2006
bool g_b_cv_jasonmode;                          // KWo - 06.04.2006
float g_f_cv_dangerfactor;                      // KWo - 06.04.2006
bool g_b_cv_chat;                               // KWo - 06.04.2006
int g_i_cv_latencybot;                          // KWo - 02.03.2010
bool g_b_cv_autokill;                           // KWo - 02.05.2006
float g_f_cv_autokilldelay;                     // KWo - 02.05.2006
float g_f_cv_maxcamptime;                       // KWo - 23.03.2008
bool g_b_cv_ffa;                                // KWo - 04.10.2006
bool g_b_cv_firsthumanrestart;                  // KWo - 04.10.2010
bool g_b_cv_radio;                              // KWo - 03.02.2007
int g_i_cv_aim_type;                            // KWo - 06.04.2006
float g_f_cv_aim_spring_stiffness_x;            // KWo - 06.04.2006
float g_f_cv_aim_spring_stiffness_y;            // KWo - 06.04.2006
float g_f_cv_aim_damper_coefficient_x;          // KWo - 06.04.2006
float g_f_cv_aim_damper_coefficient_y;          // KWo - 06.04.2006
float g_f_cv_aim_deviation_x;                   // KWo - 06.04.2006
float g_f_cv_aim_deviation_y;                   // KWo - 06.04.2006
float g_f_cv_aim_influence_x_on_y;              // KWo - 06.04.2006
float g_f_cv_aim_influence_y_on_x;              // KWo - 06.04.2006
float g_f_cv_aim_offset_delay;                  // KWo - 06.04.2006
float g_f_cv_aim_notarget_slowdown_ratio;       // KWo - 06.04.2006
float g_f_cv_aim_target_anticipation_ratio;     // KWo - 06.04.2006
const char *g_sz_cv_PasswordField;              // KWo - 06.04.2006
const char *g_sz_cv_Password;                   // KWo - 06.04.2006
const char *g_sz_cv_WPT_Folder;                 // KWo - 17.11.2006
// end of globals for cvars

// beginning of light functions/variables declaration - KWo - 23.03.2012 - thanks to Immortal_BLG
lightstyle_t cl_lightstyle[MAX_LIGHTSTYLES];
int          d_lightstylevalue[MAX_LIGHTSTYLEVALUE];   // 8.8 fraction of base light value
model_t     *sv_worldmodel (NULL);   // Analog of sv.worldmodel.

namespace Light
{
   Color g_pointColor;
   template <typename nodeType, typename surfaceType> const bool RecursiveLightPoint (const nodeType *const node, const Vector &start, const Vector &end);
}
// end of light declaration

const char *g_TaskNames[19] =                   // KWo - 06.01.2008
{
   "TASK_NORMAL",             // 0
   "TASK_PAUSE",              // 1
   "TASK_MOVETOPOSITION",     // 2
   "TASK_FOLLOWUSER",         // 3
   "TASK_WAITFORGO",          // 4
   "TASK_PICKUPITEM",         // 5
   "TASK_CAMP",               // 6
   "TASK_PLANTBOMB",          // 7
   "TASK_DEFUSEBOMB",         // 8
   "TASK_ATTACK",             // 9
   "TASK_ENEMYHUNT",          // 10
   "TASK_SEEKCOVER",          // 11
   "TASK_THROWHEGRENADE",     // 12
   "TASK_THROWFLASHBANG",     // 13
   "TASK_THROWSMOKEGRENADE",  // 14
   "TASK_SHOOTBREAKABLE",     // 15
   "TASK_HIDE",               // 16
   "TASK_BLINDED",            // 17
   "TASK_SPRAYLOGO"           // 18
};

const char *g_ItemTypeNames[7] =
{
   "PICKUP_NONE",             // 0
   "PICKUP_WEAPON",           // 1
   "PICKUP_DROPPED_C4",       // 2
   "PICKUP_PLANTED_C4",       // 3
   "PICKUP_HOSTAGE",          // 4
   "PICKUP_SHIELD",           // 5
   "PICKUP_DEFUSEKIT"         // 6
};

const char *g_CollideMoveNames[8] =             // KWo - 27.03.2010
{
   "COLLISION_NOTDECIDED",    // 0
   "COLLISION_PROBING",       // 1
   "COLLISION_NOMOVE",        // 2
   "COLLISION_JUMP",          // 3
   "COLLISION_DUCK",          // 4
   "COLLISION_STRAFELEFT",    // 5
   "COLLISION_STRAFERIGHT",   // 6
   "COLLISION_GOBACK"         // 7
};

cvar_t *g_p_cv_csdm_active;                     // KWo - 15.04.2008
cvar_t *g_rgcvarPointer[NUM_PBCVARS];           // KWo - 13.10.2006
const char *g_rgpszPbCmds[NUM_PBCOMMANDS + 1] =
{
   "pb",
   "add",
   "debuggoal",
   "experience",
   "fillserver",
   "help",
   "killbots",
   "menu",
   "remove",
   "removebots",
   "set",
   "weaponmode",
   "wp",
   "wpmenu",
   ""
};


const char *g_rgpszPbCvars[NUM_PBCVARS + 1] =
{
   "pb_" "aim_damper_coefficient_x",
   "pb_" "aim_damper_coefficient_y",
   "pb_" "aim_deviation_x",
   "pb_" "aim_deviation_y",
   "pb_" "aim_influence_x_on_y",
   "pb_" "aim_influence_y_on_x",
   "pb_" "aim_notarget_slowdown_ratio",
   "pb_" "aim_offset_delay",
   "pb_" "aim_spring_stiffness_x",
   "pb_" "aim_spring_stiffness_y",
   "pb_" "aim_target_anticipation_ratio",       // KWo - 04.03.2006
   "pb_" "aim_type",
   "pb_" "autokill",                            // KWo - 02.05.2006
   "pb_" "autokilldelay",                       // KWo - 02.05.2006
   "pb_" "bot_join_team",                       // KWo - 16.09.2006
   "pb_" "bot_quota_match",                     // KWo - 16.09.2006
   "pb_" "chat",
   "pb_" "dangerfactor",
   "pb_" "debuglevel",                          // KWo - 20.04.2013
   "pb_" "detailnames",
   "pb_" "ffa",                                 // KWo - 04.10.2006
   "pb_" "firsthumanrestart",                   // KWo - 04.10.2010
   "pb_" "jasonmode",
   "pb_" "latencybot",                          // KWo - 16.05.2008
   "pb_" "mapstartbotdelay",
   "pb_" "maxbots",
   "pb_" "maxbotskill",
   "pb_" "maxcamptime",                         // KWo - 23.03.2008
   "pb_" "maxweaponpickup",
   "pb_" "minbots",
   "pb_" "minbotskill",
   "pb_" "numfollowuser",
   "pb_" "password",
   "pb_" "passwordkey",
   "pb_" "radio",                               // KWo - 03.02.2007
   "pb_" "restrequipammo",                      // KWo - 09.03.2006
   "pb_" "restrweapons",                        // KWo - 09.03.2006
   "pb_" "shootthruwalls",
   "pb_" "skin",                                // KWo - 18.11.2006
   "pb_" "spray",
   "pb_" "timer_grenade",
   "pb_" "timer_pickup",
   "pb_" "timer_sound",
   "pb_" "usespeech",
   "pb_" "version",
   "pb_" "welcomemsgs",
   "pb_" "wptfolder",
   ""
};


// Weapons and their specifications
bot_weapon_select_t cs_weapon_select[NUM_WEAPONS + 1] =
{
   // Knife   iId         weapon_name            model_name               viewmodel_name      buy_shortcut    buy_command                prim._min_dist. prim._max_dist. sec._min_dist. sec._max_dist. can_use_und.wat. prim._fire_hold prim._ch._del. iPrice min_prim._ammo iTeamStand. iTeamAS bShootsThru
   {CS_WEAPON_KNIFE,     "weapon_knife",     "models/w_knife.mdl",     "models/v_knife.mdl",     "",       "",                                   0.0,         64.0,           0.0,           9999.0,          TRUE,           TRUE,           0.0,          0,       0,           -1,        -1,     FALSE}, // ""
   // Pistols
   {CS_WEAPON_USP,       "weapon_usp",       "models/w_usp.mdl",       "models/v_usp.mdl",       "usp",    "buy;menuselect 1;menuselect 1",      0.0,       4096.0,           0.0,           9999.0,          TRUE,          FALSE,           2.8,        500,       1,           -1,        -1,     FALSE}, // "usp"
   {CS_WEAPON_GLOCK18,   "weapon_glock18",   "models/w_glock18.mdl",   "models/v_glock18.mdl",   "glock",  "buy;menuselect 1;menuselect 2",      0.0,       4096.0,           0.0,           9999.0,          TRUE,          FALSE,           2.3,        400,       1,           -1,        -1,     FALSE}, // "glock"
   {CS_WEAPON_DEAGLE,    "weapon_deagle",    "models/w_deagle.mdl",    "models/v_deagle.mdl",    "deagle", "buy;menuselect 1;menuselect 3",      0.0,       4096.0,           0.0,           9999.0,          TRUE,          FALSE,           2.3,        650,       1,            2,         2,      TRUE}, // "deagle"
   {CS_WEAPON_P228,      "weapon_p228",      "models/w_p228.mdl",      "models/v_p228.mdl",      "p228",   "buy;menuselect 1;menuselect 4",      0.0,       4096.0,           0.0,           9999.0,          TRUE,          FALSE,           2.8,        600,       1,            2,         2,     FALSE}, // "p228"
   {CS_WEAPON_ELITE,     "weapon_elite",     "models/w_elite.mdl",     "models/v_elite.mdl",     "elites", "buy;menuselect 1;menuselect 5",      0.0,       4096.0,           0.0,           9999.0,          TRUE,          FALSE,           5.2,       1000,       1,            0,         0,     FALSE}, // "elites"
   {CS_WEAPON_FIVESEVEN, "weapon_fiveseven", "models/w_fiveseven.mdl", "models/v_fiveseven.mdl", "fn57",   "buy;menuselect 1;menuselect 6",      0.0,       4096.0,           0.0,           9999.0,          TRUE,          FALSE,           3.5,        750,       1,            1,         1,     FALSE}, // "fn57"
   // Shotguns
   {CS_WEAPON_M3,        "weapon_m3",        "models/w_m3.mdl",        "models/v_m3.mdl",        "m3",     "buy;menuselect 2;menuselect 1",      0.0,       1024.0,           0.0,           9999.0,          TRUE,           TRUE,           2.4,       1700,       1,            2,         1,     FALSE}, // "m3"
   {CS_WEAPON_XM1014,    "weapon_xm1014",    "models/w_xm1014.mdl",    "models/v_xm1014.mdl",    "xm1014", "buy;menuselect 2;menuselect 2",      0.0,       1024.0,           0.0,           9999.0,          TRUE,           TRUE,           1.6,       3000,       1,            2,         1,     FALSE}, // "xm1014"
   // Sub Machine Guns
   {CS_WEAPON_MP5NAVY,   "weapon_mp5navy",   "models/w_mp5.mdl",       "models/v_mp5.mdl",       "mp5",    "buy;menuselect 3;menuselect 1",      0.0,     /*2048.0*/ 99999.0, 0.0,           9999.0,          TRUE,           TRUE,           2.8,       1500,       1,            2,         1,     FALSE}, // "mp5"
   {CS_WEAPON_TMP,       "weapon_tmp",       "models/w_tmp.mdl",       "models/v_tmp.mdl",       "tmp",    "buy;menuselect 3;menuselect 2",      0.0,     /*2048.0*/ 99999.0, 0.0,           9999.0,          TRUE,           TRUE,           2.2,       1250,       1,            1,         1,     FALSE}, // "tmp"
   {CS_WEAPON_P90,       "weapon_p90",       "models/w_p90.mdl",       "models/v_p90.mdl",       "p90",    "buy;menuselect 3;menuselect 3",      0.0,     /*2048.0*/ 99999.0, 0.0,           9999.0,          TRUE,           TRUE,           3.5,       2350,       1,            2,         1,     FALSE}, // "p90"
   {CS_WEAPON_MAC10,     "weapon_mac10",     "models/w_mac10.mdl",     "models/v_mac10.mdl",     "mac10",  "buy;menuselect 3;menuselect 4",      0.0,     /*2048.0*/ 99999.0, 0.0,           9999.0,          TRUE,           TRUE,           2.7,       1400,       1,            0,         0,     FALSE}, // "mac10"
   {CS_WEAPON_UMP45,     "weapon_ump45",     "models/w_ump45.mdl",     "models/v_ump45.mdl",     "ump45",  "buy;menuselect 3;menuselect 5",      0.0,     /*2048.0*/ 99999.0, 0.0,           9999.0,          TRUE,           TRUE,           4.2,       1700,       1,            2,         2,     FALSE}, // "ump45"
   // Rifles
   {CS_WEAPON_AK47,      "weapon_ak47",      "models/w_ak47.mdl",      "models/v_ak47.mdl",      "ak47",   "buy;menuselect 4;menuselect 1",      0.0,     /*4096.0*/ 99999.0, 0.0,           9999.0,          TRUE,           TRUE,           3.5,       2500,       1,            0,         0,      TRUE}, // "ak47"
   {CS_WEAPON_SG552,     "weapon_sg552",     "models/w_sg552.mdl",     "models/v_sg552.mdl",     "sg552",  "buy;menuselect 4;menuselect 2",      0.0,     /*4096.0*/ 99999.0, 0.0,           9999.0,          TRUE,           TRUE,           5.3,       3500,       1,            0,        -1,      TRUE}, // "sg552"
   {CS_WEAPON_M4A1,      "weapon_m4a1",      "models/w_m4a1.mdl",      "models/v_m4a1.mdl",      "m4a1",   "buy;menuselect 4;menuselect 3",      0.0,     /*4096.0*/ 99999.0, 0.0,           9999.0,          TRUE,           TRUE,           3.7,       3100,       1,            1,         1,      TRUE}, // "m4a1"
   {CS_WEAPON_GALIL,     "weapon_galil",     "models/w_galil.mdl",     "models/v_galil.mdl",     "galil",  "",                                   0.0,     /*4096.0*/ 99999.0, 0.0,           9999.0,          TRUE,           TRUE,           3.8,       2500,       1,            0,         0,      TRUE}, // "galil"
   {CS_WEAPON_FAMAS,     "weapon_famas",     "models/w_famas.mdl",     "models/v_famas.mdl",     "famas",  "",                                   0.0,     /*4096.0*/ 99999.0, 0.0,           9999.0,          TRUE,           TRUE,           3.8,       3100,       1,            1,         1,      TRUE}, // "famas"
   {CS_WEAPON_AUG,       "weapon_aug",       "models/w_aug.mdl",       "models/v_aug.mdl",       "aug",    "buy;menuselect 4;menuselect 4",      0.0,     /*4096.0*/ 99999.0, 0.0,           9999.0,          TRUE,           TRUE,           3.5,       3500,       1,            1,         1,      TRUE}, // "aug"
   // Sniper Rifles
   {CS_WEAPON_SCOUT,     "weapon_scout",     "models/w_scout.mdl",     "models/v_scout.mdl",     "scout",  "buy;menuselect 4;menuselect 5",    768.0,     /*8192.0*/ 99999.0, 0.0,           9999.0,          TRUE,          FALSE,           3.6,       2750,       1,            2,         0,      TRUE}, // "scout"
   {CS_WEAPON_AWP,       "weapon_awp",       "models/w_awp.mdl",       "models/v_awp.mdl",       "awp",    "buy;menuselect 4;menuselect 6",    768.0,     /*8192.0*/ 99999.0, 0.0,           9999.0,          TRUE,          FALSE,           3.8,       4750,       1,            2,         0,      TRUE}, // "awp"
   {CS_WEAPON_G3SG1,     "weapon_g3sg1",     "models/w_g3sg1.mdl",     "models/v_g3sg1.mdl",     "g3sg1",  "buy;menuselect 4;menuselect 7",    512.0,     /*8192.0*/ 99999.0, 0.0,           9999.0,          TRUE,           TRUE,           4.9,       5000,       1,            0,        -1,      TRUE}, // "g3sg1"
   {CS_WEAPON_SG550,     "weapon_sg550",     "models/w_sg550.mdl",     "models/v_sg550.mdl",     "sg550",  "buy;menuselect 4;menuselect 8",    512.0,     /*8192.0*/ 99999.0, 0.0,           9999.0,          TRUE,           TRUE,           5.3,       4200,       1,            1,         1,      TRUE}, // "sg550"
   // Machine Guns
   {CS_WEAPON_M249,      "weapon_m249",      "models/w_m249.mdl",      "models/v_m249.mdl",      "m249",   "buy;menuselect 5;menuselect 1",      0.0,       2048.0,           0.0,           9999.0,          TRUE,           TRUE,           5.3,       5750,       1,            2,         1,      TRUE}, // "m249"
   // Shield
   {CS_WEAPON_SHIELDGUN, "weapon_shield",    "models/w_shield.mdl",    "models/v_shield.mdl",    "shield", "",                                   0.0,       9999.0,           0.0,           9999.0,          TRUE,          FALSE,           0.0,       2200,       0,            1,         1,     FALSE}, // "shield"
    //terminator
   {0,                   "",                 "",                       "",                       "",       "",                                   0.0,          0.0,           0.0,              0.0,         FALSE,          FALSE,           0.0,          0,       0,            0,         0,     FALSE} // ""
};


// weapon firing delay based on skill (min and max delay for each weapon)
// THESE MUST MATCH THE SAME ORDER AS THE WEAPON SELECT ARRAY ABOVE!!!
// Last 2 values are Burstfire Bullet Count & Pause Times
bot_fire_delay_t cs_fire_delay[NUM_WEAPONS + 1] =
{
   // Knife       primary_base_delay      primary_min_delay[6]             primary_max_delay[6]        secondary_base_delay     secondary_min_delay[5]     secondary_max_delay[5]  iMaxFireBullets   fMinBurstPauseFactor
   {CS_WEAPON_KNIFE,     0.10,         {0.0, 0.2, 0.3, 0.4, 0.6, 0.8}, {0.1, 0.3, 0.5, 0.7, 1.0, 1.2},         0.0,           {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0},     255,       MIN_BURST_DISTANCE / 1.0},
   // Pistols
   {CS_WEAPON_USP,       0.06,         {0.0, 0.1, 0.2, 0.3, 0.4, 0.6}, {0.1, 0.2, 0.3, 0.4, 0.5, 0.7},         0.2,           {0.0, 0.0, 0.1, 0.1, 0.2}, {0.1, 0.1, 0.2, 0.2, 0.4},       3,       MIN_BURST_DISTANCE / 0.30},
   {CS_WEAPON_GLOCK18,   0.06,         {0.0, 0.1, 0.2, 0.3, 0.4, 0.6}, {0.1, 0.2, 0.3, 0.4, 0.5, 0.7},         0.2,           {0.0, 0.0, 0.1, 0.1, 0.2}, {0.1, 0.1, 0.2, 0.2, 0.4},       5,       MIN_BURST_DISTANCE / 0.30},
   {CS_WEAPON_DEAGLE,    0.06,         {0.0, 0.1, 0.2, 0.3, 0.4, 0.6}, {0.1, 0.2, 0.3, 0.4, 0.5, 0.7},         0.2,           {0.0, 0.0, 0.1, 0.1, 0.2}, {0.1, 0.1, 0.2, 0.2, 0.4},       2,       MIN_BURST_DISTANCE / 0.40},
   {CS_WEAPON_P228,      0.05,         {0.0, 0.1, 0.2, 0.3, 0.4, 0.6}, {0.1, 0.2, 0.3, 0.4, 0.5, 0.7},         0.2,           {0.0, 0.0, 0.1, 0.1, 0.2}, {0.1, 0.1, 0.2, 0.2, 0.4},       4,       MIN_BURST_DISTANCE / 0.30},
   {CS_WEAPON_ELITE,     0.05,         {0.0, 0.1, 0.2, 0.3, 0.4, 0.6}, {0.1, 0.2, 0.3, 0.4, 0.5, 0.7},         0.2,           {0.0, 0.0, 0.1, 0.1, 0.2}, {0.1, 0.1, 0.2, 0.2, 0.4},       3,       MIN_BURST_DISTANCE / 0.40},
   {CS_WEAPON_FIVESEVEN, 0.05,         {0.0, 0.1, 0.2, 0.3, 0.4, 0.6}, {0.1, 0.2, 0.3, 0.4, 0.5, 0.7},         0.2,           {0.0, 0.0, 0.1, 0.1, 0.2}, {0.1, 0.1, 0.2, 0.2, 0.4},       4,       MIN_BURST_DISTANCE / 0.35},
   // Shotguns
   {CS_WEAPON_M3,        0.86,         {0.0, 0.1, 0.2, 0.3, 0.4, 0.6}, {0.1, 0.2, 0.3, 0.4, 0.5, 0.7},         0.2,           {0.0, 0.0, 0.1, 0.1, 0.2}, {0.1, 0.1, 0.2, 0.2, 0.4},       8,       MIN_BURST_DISTANCE / 0.70},
   {CS_WEAPON_XM1014,    0.15,         {0.0, 0.1, 0.2, 0.3, 0.4, 0.6}, {0.1, 0.2, 0.3, 0.4, 0.5, 0.7},         0.2,           {0.0, 0.0, 0.1, 0.1, 0.2}, {0.1, 0.1, 0.2, 0.2, 0.4},       7,       MIN_BURST_DISTANCE / 0.50},
   // Sub Machine Guns
   {CS_WEAPON_MP5NAVY,   0.10,         {0.0, 0.1, 0.2, 0.3, 0.4, 0.6}, {0.1, 0.2, 0.3, 0.4, 0.5, 0.7},         0.2,           {0.0, 0.0, 0.1, 0.1, 0.2}, {0.1, 0.1, 0.2, 0.2, 0.4},       4,       MIN_BURST_DISTANCE / 0.35},
   {CS_WEAPON_TMP,       0.05,         {0.0, 0.1, 0.2, 0.3, 0.4, 0.6}, {0.1, 0.2, 0.3, 0.4, 0.5, 0.7},         0.2,           {0.0, 0.0, 0.1, 0.1, 0.2}, {0.1, 0.1, 0.2, 0.2, 0.4},       3,       MIN_BURST_DISTANCE / 0.35},
   {CS_WEAPON_P90,       0.10,         {0.0, 0.1, 0.2, 0.3, 0.4, 0.6}, {0.1, 0.2, 0.3, 0.4, 0.5, 0.7},         0.2,           {0.0, 0.0, 0.1, 0.1, 0.2}, {0.1, 0.1, 0.2, 0.2, 0.4},       4,       MIN_BURST_DISTANCE / 0.35},
   {CS_WEAPON_MAC10,     0.06,         {0.0, 0.1, 0.2, 0.3, 0.4, 0.6}, {0.1, 0.2, 0.3, 0.4, 0.5, 0.7},         0.2,           {0.0, 0.0, 0.1, 0.1, 0.2}, {0.1, 0.1, 0.2, 0.2, 0.4},       3,       MIN_BURST_DISTANCE / 0.35},
   {CS_WEAPON_UMP45,     0.15,         {0.0, 0.1, 0.2, 0.3, 0.4, 0.6}, {0.1, 0.2, 0.3, 0.4, 0.5, 0.7},         0.2,           {0.0, 0.0, 0.1, 0.1, 0.2}, {0.1, 0.1, 0.2, 0.2, 0.4},       4,       MIN_BURST_DISTANCE / 0.35},
   // Rifles
   {CS_WEAPON_AK47,/*0.20*/ 0.10,      {0.0, 0.1, 0.2, 0.3, 0.4, 0.6}, {0.1, 0.2, 0.3, 0.4, 0.5, 0.7},         0.2,           {0.0, 0.0, 0.1, 0.1, 0.2}, {0.1, 0.1, 0.2, 0.2, 0.4},       2,       MIN_BURST_DISTANCE / 0.50},
   {CS_WEAPON_SG552,/*0.40*/0.12,      {0.0, 0.1, 0.2, 0.3, 0.4, 0.6}, {0.1, 0.2, 0.3, 0.4, 0.5, 0.7},         0.2,           {0.0, 0.0, 0.1, 0.1, 0.2}, {0.1, 0.1, 0.2, 0.2, 0.4},       3,       MIN_BURST_DISTANCE / 0.50},
   {CS_WEAPON_M4A1, /*0.12*/0.08,      {0.0, 0.1, 0.2, 0.3, 0.4, 0.6}, {0.1, 0.2, 0.3, 0.4, 0.5, 0.7},         0.2,           {0.0, 0.0, 0.1, 0.1, 0.2}, {0.1, 0.1, 0.2, 0.2, 0.4},       3,       MIN_BURST_DISTANCE / 0.50},
   {CS_WEAPON_GALIL,/*0.20*/0.11,      {0.0, 0.1, 0.2, 0.3, 0.4, 0.6}, {0.1, 0.2, 0.3, 0.4, 0.5, 0.7},         0.2,           {0.0, 0.0, 0.1, 0.1, 0.2}, {0.1, 0.1, 0.2, 0.2, 0.4},       4,       MIN_BURST_DISTANCE / 0.50},
   {CS_WEAPON_FAMAS,/*0.20*/0.11,      {0.0, 0.1, 0.2, 0.3, 0.4, 0.6}, {0.1, 0.2, 0.3, 0.4, 0.5, 0.7},         0.2,           {0.0, 0.0, 0.1, 0.1, 0.2}, {0.1, 0.1, 0.2, 0.2, 0.4},       4,       MIN_BURST_DISTANCE / 0.50},
   {CS_WEAPON_AUG,  /*0.20*/0.11,      {0.0, 0.1, 0.2, 0.3, 0.4, 0.6}, {0.1, 0.2, 0.3, 0.4, 0.5, 0.7},         0.2,           {0.0, 0.0, 0.1, 0.1, 0.2}, {0.1, 0.1, 0.2, 0.2, 0.4},       3,       MIN_BURST_DISTANCE / 0.50},
   // Sniper Rifles
   {CS_WEAPON_SCOUT,     0.23,         {0.0, 0.1, 0.2, 0.3, 0.4, 0.6}, {0.1, 0.2, 0.3, 0.4, 0.5, 0.7},         0.2,           {0.0, 0.0, 0.1, 0.1, 0.2}, {0.1, 0.1, 0.2, 0.2, 0.4},      10,       MIN_BURST_DISTANCE / 1.00},
   {CS_WEAPON_AWP,       1.75,         {0.0, 0.1, 0.2, 0.3, 0.4, 0.6}, {0.1, 0.2, 0.3, 0.4, 0.5, 0.7},         0.2,           {0.0, 0.0, 0.1, 0.1, 0.2}, {0.1, 0.1, 0.2, 0.2, 0.4},      10,       MIN_BURST_DISTANCE / 1.50},
   {CS_WEAPON_G3SG1,/*0.30*/0.26,      {0.0, 0.1, 0.2, 0.3, 0.4, 0.6}, {0.1, 0.2, 0.3, 0.4, 0.5, 0.7},         0.2,           {0.0, 0.0, 0.1, 0.1, 0.2}, {0.1, 0.1, 0.2, 0.2, 0.4},       4,       MIN_BURST_DISTANCE / 1.00},
   {CS_WEAPON_SG550,/*0.30*/0.26,      {0.0, 0.1, 0.2, 0.3, 0.4, 0.6}, {0.1, 0.2, 0.3, 0.4, 0.5, 0.7},         0.2,           {0.0, 0.0, 0.1, 0.1, 0.2}, {0.1, 0.1, 0.2, 0.2, 0.4},       4,       MIN_BURST_DISTANCE / 1.00},
   // Machine Guns
   {CS_WEAPON_M249,       0.1,         {0.0, 0.1, 0.2, 0.3, 0.4, 0.6}, {0.1, 0.2, 0.3, 0.4, 0.5, 0.7},         0.2,           {0.0, 0.0, 0.1, 0.1, 0.2}, {0.1, 0.1, 0.2, 0.2, 0.4},       3,       MIN_BURST_DISTANCE / 0.40},
   // Shield
   {CS_WEAPON_SHIELDGUN,  0.0,         {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0, 0.0},         0.0,           {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0},       0,       MIN_BURST_DISTANCE / 1.00},
   // terminator
   {0,                    0.0,         {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0, 0.0},         0.0,           {0.0, 0.0, 0.0, 0.0, 0.0}, {0.0, 0.0, 0.0, 0.0, 0.0},       0,       MIN_BURST_DISTANCE / 1.00}
};


// This Array stores the Aiming Offsets, Headshot Frequency and the ShootThruWalls
// Probability (worst to best skill). Overridden by botskill.cfg
botaim_t BotAimTab[6] =
{
   { 40,  40,  50,   0,   0,   0},
   { 30,  30,  42,  10,   0,   0},
   { 20,  20,  32,  30,   0,  50},
   { 10,  10,  18,  50,  30,  80},
   {  5,   5,  10,  80,  50, 100},
   {  0,   0,   0, 100, 100, 100}
};


int weapon_maxClip[MAX_WEAPONS] =               // KWo - 10.01.2007
{
     0, // ""
    13, // p228
     0, // ""
    10, // scout
     0, // hegrenade
     7, // xm1014
     0, // c4
    30, // mac10
    30, // aug
     0, // smokegrenade
    30, // elite
    20, // fiveseven
    25, // ump45
    30, // sg550
    35, // weapon_galil
    25, // weapon_famas
    12, // usp
    20, // glock18
    10, // awp
    30, // mp5navy
   100, // m249
     8, // m3
    30, // m4a1
    30, // tmp - KWo - 08.10.2010
    20, // g3sg1
     0, // flashbang
     7, // deagle
    30, // sg552
    30, // ak47
     0, // knife
    50, // p90
     0  // ""
};

int weapon_selectIndex[MAX_WEAPONS] =           // KWo - 05.04.2010
{
    26, // ""           - 0
     4, // p228         - 1
    25, // shieldgun    - 2
    20, // scout        - 3
    26, // hegrenade    - 4
     8, // xm1014       - 5
    26, // c4           - 6
    12, // mac10        - 7
    19, // aug          - 8
    26, // smokegrenade - 9
     5, // elite        - 10
     6, // fiveseven    - 11
    13, // ump45        - 12
    23, // sg550        - 13
    17, // weapon_galil - 14
    18, // weapon_famas - 15
     1, // usp          - 16
     2, // glock18      - 17
    21, // awp          - 18
     9, // mp5navy      - 19
    24, // m249         - 20
     7, // m3           - 21
    16, // m4a1         - 22
    10, // tmp          - 23
    22, // g3sg1        - 24
    26, // flashbang    - 25
     3, // deagle       - 26
    15, // sg552        - 27
    14, // ak47         - 28
     0, // knife        - 29
    11, // p90          - 30
    26  // ""           - 31
};


bot_weapon_t weapon_defs[MAX_WEAPONS] =
{
   // szClassname          iAmmo1   iAmmo1Max   iAmmo2   iAmmo2Max   iSlot iPosition   iId   iFlags
   {"",                    0,       0,          0,       0,          0,    0,          0,    0},
   {"weapon_p228",         9,       52,         0,       0,          1,    3,          1,    0},
   {"",                    0,       0,          0,       0,          0,    0,          2,    0},
   {"weapon_scout",        2,       90,         0,       0,          0,    9,          3,    0},
   {"weapon_hegrenade",    12,      1,          0,       0,          3,    1,          4,    0},
   {"weapon_xm1014",       5,       32,         0,       0,          0,    12,         5,    0},
   {"weapon_c4",           14,      1,          0,       0,          4,    3,          6,    0},
   {"weapon_mac10",        6,       100,        0,       0,          0,    13,         7,    0},
   {"weapon_aug",          4,       90,         0,       0,          0,    14,         8,    0},
   {"weapon_smokegrenade", 13,      1,          0,       0,          3,    3,          9,    0},
   {"weapon_elite",        10,      120,        0,       0,          1,    5,          10,   0},
   {"weapon_fiveseven",    7,       100,        0,       0,          1,    6,          11,   0},
   {"weapon_ump45",        6,       100,        0,       0,          0,    15,         12,   0},
   {"weapon_sg550",        4,       90,         0,       0,          0,    16,         13,   0},
   {"weapon_galil",        4,       90,         0,       0,          0,    0,          14,   0},
   {"weapon_famas",        4,       90,         0,       0,          0,    0,          15,   0},
   {"weapon_usp",          6,       100,        0,       0,          1,    4,          16,   0},
   {"weapon_glock18",      10,      120,        0,       0,          1,    2,          17,   0},
   {"weapon_awp",          1,       30,         0,       0,          0,    2,          18,   0},
   {"weapon_mp5navy",      10,      120,        0,       0,          0,    7,          19,   0},
   {"weapon_m249",         3,       200,        0,       0,          0,    4,          20,   0},
   {"weapon_m3",           5,       32,         0,       0,          0,    5,          21,   0},
   {"weapon_m4a1",         4,       90,         0,       0,          0,    6,          22,   0},
   {"weapon_tmp",          10,      120,        0,       0,          0,    11,         23,   0},
   {"weapon_g3sg1",        2,       90,         0,       0,          0,    3,          24,   0},
   {"weapon_flashbang",    11,      2,          0,       0,          3,    2,          25,   0},
   {"weapon_deagle",       8,       35,         0,       0,          1,    1,          26,   0},
   {"weapon_sg552",        4,       90,         0,       0,          0,    10,         27,   0},
   {"weapon_ak47",         2,       90,         0,       0,          0,    1,          28,   0},
   {"weapon_knife",        -1,      -1,         0,       0,          2,    1,          29,   0},
   {"weapon_p90",          7,       100,        0,       0,          0,    8,          30,   0},
   {"",                    0,       0,          0,       0,          0,    0,          31,   0}
};


// These are skill based Delays for an Enemy Surprise Delay
// and the Pause/Camping Delays (weak Bots are longer surprised and
// do Pause/Camp longer as well)
skilldelay_t BotSkillDelays[6] =                // KWo - 23.10.2006
{
   {0.5, 1.0, 5, 40.0, 60.0},
   {0.4, 0.8, 10, 35.0, 55.0},
   {0.3, 0.6, 15, 30.0, 50.0},
   {0.2, 0.4, 20, 25.0, 45.0},
   {0.1, 0.2, 25, 20.0, 40.0},
   {0.0, 0.1, 30, 15.0, 35.0}
};


// Turn speeds for each bot skill level
// Lower skilled bots turn slower
turnspeed_t BotTurnSpeeds[6] =
{
   {2.0,  3.0},
   {3.0,  4.0},
   {4.0,  6.0},
   {6.0,  8.0},
   {8.0, 10.0},
   {9.0, 12.0}
};


// Table with all available Actions for the Bots
// (filtered in & out in BotSetConditions)
// Some of them have subactions included
bottask_t taskFilters[] =
{
   {NULL, NULL, TASK_NORMAL,         0.0, -1, 0.0, TRUE},
   {NULL, NULL, TASK_PAUSE,          0.0, -1, 0.0, FALSE},
   {NULL, NULL, TASK_MOVETOPOSITION, 0.0, -1, 0.0, TRUE},
   {NULL, NULL, TASK_FOLLOWUSER,     0.0, -1, 0.0, TRUE},
   {NULL, NULL, TASK_WAITFORGO,      0.0, -1, 0.0, TRUE},
   {NULL, NULL, TASK_PICKUPITEM,     0.0, -1, 0.0, TRUE},
   {NULL, NULL, TASK_CAMP,           0.0, -1, 0.0, TRUE},
   {NULL, NULL, TASK_PLANTBOMB,      0.0, -1, 0.0, FALSE},
   {NULL, NULL, TASK_DEFUSEBOMB,     0.0, -1, 0.0, FALSE},
   {NULL, NULL, TASK_ATTACK,         0.0, -1, 0.0, FALSE},
   {NULL, NULL, TASK_ENEMYHUNT,      0.0, -1, 0.0, FALSE},
   {NULL, NULL, TASK_SEEKCOVER,      0.0, -1, 0.0, FALSE},
   {NULL, NULL, TASK_THROWHEGRENADE, 0.0, -1, 0.0, FALSE},
   {NULL, NULL, TASK_THROWFLASHBANG, 0.0, -1, 0.0, FALSE},
   {NULL, NULL, TASK_THROWSMOKEGRENADE, 0.0, -1, 0.0, FALSE},
   {NULL, NULL, TASK_SHOOTBREAKABLE, 0.0, -1, 0.0, FALSE},
   {NULL, NULL, TASK_HIDE,           0.0, -1, 0.0, FALSE},
   {NULL, NULL, TASK_BLINDED,        0.0, -1, 0.0, FALSE},
   {NULL, NULL, TASK_SPRAYLOGO,      0.0, -1, 0.0, FALSE}
};


// Default Tables for Personality Weapon Prefs overridden by botweapons.cfg
int NormalWeaponPrefs[NUM_WEAPONS] =
{
   0, 2, 1, 4, 5, 6, 3, 12, 10, 24, 25, 13, 11, 8, 7, 9, 22, 23, 18, 21, 20, 19, 15, 17, 14, 16
};

int AgressiveWeaponPrefs[NUM_WEAPONS] =
{
   0, 2, 1, 4, 5, 6, 3, 24, 25, 23, 20, 21, 22, 10, 12, 13, 11, 9, 7, 8, 18, 17, 19, 15, 16, 14
};

int DefensiveWeaponPrefs[NUM_WEAPONS] =
{
   0, 2, 1, 4, 6, 5, 3, 7, 8, 12, 10, 13, 11, 9, 24, 18, 14, 17, 16, 15, 19, 20, 21, 22, 23, 25
};

int *ptrWeaponPrefs[] =
{
   (int *) &NormalWeaponPrefs,
   (int *) &AgressiveWeaponPrefs,
   (int *) &DefensiveWeaponPrefs
};


// Default Spaynames - overridden by BotLogos.cfg
char szSprayNames[NUM_SPRAYPAINTS][20] =
{
   "{biohaz",
   "{graf004",
   "{graf005",
   "{lambda06",
   "{target",
   "{hand1"
};

// Sentences used with "speak" to welcome a User
const char szSpeechSentences[16][80] =
{
   "speak \"hello user, communication is acquired\"\n",
   "speak \"your presence is acknowledged\"\n",
   "speak \"high man, your in command now\"\n",
   "speak \"blast your hostile for good\"\n",
   "speak \"high man, kill some idiot here\"\n",
   "speak \"check check, test, mike check, talk device is activated\"\n",
   "speak \"good, day mister, your administration is now acknowledged\"\n",
   "speak \"high amigo, shoot some but\"\n",
   "speak \"hello pal, at your service\"\n",
   "speak \"time for some bad ass explosion\"\n",
   "speak \"high man, at your command\"\n",
   "speak \"bad ass son of a breach device activated\"\n",
   "speak \"high, do not question this great service\"\n",
   "speak \"engine is operative, hello and goodbye\"\n",
   "speak \"high amigo, your administration has been great last day\"\n",
   "speak \"all command access granted, over and out\"\n"
};


// Welcome Message
const char *g_szWelcomeMessage = "Welcome to POD-Bot mm created by Count Floyd\n"
                                 "Currently developed by KWo\n"
                                 "Visit http://forums.bots-united.com for Updates"; // KWo - 19.04.2010

// Settings Message
const char *g_szSettingsMessage = "Settings (cvars) for POD-Bot mm loaded from podbot.cfg\n"; // KWo - 17.05.2008

// Commands Message
const char *g_szCommandsMessage = "Commands for POD-Bot mm loaded from podbot.cfg\n"; // KWo - 17.05.2008

// Text and Key Flags for Menues - \d & \r & \w & \y are special CS Colour Tags
menutext_t menuPODBotMain =
{
   0x2ff,
   " \\yPlease choose:\\w\n"
   "\n"
   "\n"
   " 1. Quick add Bot\n"
   " 2. Add specific Bot\n"
   " 3. Kill all Bots\n"
   " 4. New Round\n"
   " 5. Fill Server\n"
   " 6. Kick Bot\n"
   " 7. Kick all Bots\n"
   " 8. Weapon Mode\n"
   "\n"
   " 0. Cancel"
};

menutext_t menuPODBotAddBotSkill =
{
   0x37f,                                       // KWo - 09.01.2006
   " \\yPlease choose a Skill:\\w\n"
   "\n"
   "\n"
   " 1. Stupid (0-19)\n"
   " 2. Newbie (20-39)\n"
   " 3. Average (40-59)\n"
   " 4. Advanced (60-79)\n"
   " 5. Professional (80-99)\n"
   " 6. Godlike (100)\n"
   "\n"
   " 7. Random\n"
   "\n"
   " 9. Main Menu\n"                            // KWo - 09.01.2006
   " 0. Cancel"
};

menutext_t menuPODBotAddBotPersonality =
{
   0x217,
   " \\yPlease choose a Personality:\\w\n"
   "\n"
   "\n"
   " 1. Normal\n"
   " 2. Aggressive\n"
   " 3. Defensive\n"
   "\n"
   " 5. Random\n"
   "\n"
   " 0. Cancel"
};

menutext_t menuPODBotAddBotTeam =
{
   0x213,
   " \\yPlease choose a Team:\\w\n"
   "\n"
   "\n"
   " 1. Terrorist\n"
   " 2. Counter-Terrorist\n"
   "\n"
   " 5. Auto-Assign\n"
   "\n"
   " 0. Cancel"
};

menutext_t menuPODBotAddBotTModel =
{
   0x01f,
   " \\yPlease choose a Terrorist Model:\\w\n"
   "\n"
   "\n"
   " 1. Phoenix Connektion\n"
   " 2. L337 Krew\n"
   " 3. Arctic Avengers\n"
   " 4. Guerilla Warfare\n"
   "\n"
   " 5. Random"
};

menutext_t menuPODBotAddBotCTModel =
{
   0x01f,
   " \\yPlease choose a CT Model:\\w\n"
   "\n"
   "\n"
   " 1. Seal Team 6\n"
   " 2. GSG-9\n"
   " 3. SAS\n"
   " 4. GIGN\n"
   "\n"
   " 5. Random"
};

menutext_t menuPODBotFillServerSkill =
{
   0x27f,
   " \\yPlease choose a Skill:\\w\n"
   "\n"
   "\n"
   " 1. Stupid (0-19)\n"
   " 2. Newbie (20-39)\n"
   " 3. Average (40-59)\n"
   " 4. Advanced (60-79)\n"
   " 5. Professional (80-99)\n"
   " 6. Godlike (100)\n"
   "\n"
   " 7. Random\n"
   "\n"
   " 0. Cancel"
};

menutext_t menuPODBotFillServerPersonality =
{
   0x217,
   " \\yPlease choose a Personality:\\w\n"
   "\n"
   "\n"
   " 1. Normal\n"
   " 2. Aggressive\n"
   " 3. Defensive\n"
   "\n"
   " 5. Random\n"
   "\n"
   " 0. Cancel"
};

menutext_t menuPODBotFillServerTeam =
{
   0x213,
   " \\yPlease choose a Team:\\w\n"
   "\n"
   "\n"
   " 1. Terrorist\n"
   " 2. Counter-Terrorist\n"
   "\n"
   " 5. Auto-Assign\n"
   "\n"
   " 0. Cancel"
};

menutext_t menuPODBotFillServerTModel =
{
   0x01f,
   " \\yPlease choose a Terrorist Model:\\w\n"
   "\n"
   "\n"
   " 1. Phoenix Connektion\n"
   " 2. L337 Krew\n"
   " 3. Arctic Avengers\n"
   " 4. Guerilla Warfare\n"
   "\n"
   " 5. Random"
};

menutext_t menuPODBotFillServerCTModel =
{
   0x01f,
   " \\yPlease choose a CT Model:\\w\n"
   "\n"
   "\n"
   " 1. Seal Team 6\n"
   " 2. GSG-9\n"
   " 3. SAS\n"
   " 4. GIGN\n"
   "\n"
   " 5. Random"
};

menutext_t menuPODBotKickBot1 =
{
   0x0,
   NULL
};

menutext_t menuPODBotKickBot2 =
{
   0x0,
   NULL
};

menutext_t menuPODBotKickBot3 =
{
   0x0,
   NULL
};

menutext_t menuPODBotKickBot4 =
{
   0x0,
   NULL
};

menutext_t menuPODBotWeaponMode =
{
   0x27f,
   " \\yWeapon Mode:\\w\n"
   "\n"
   "\n"
   " 1. Knife only (JasonMode!)\n"
   " 2. Pistols\n"
   " 3. Shotguns\n"
   " 4. Machine Guns\n"
   " 5. Rifles\n"
   " 6. Sniper Weapons\n"
   " 7. All Weapons (Standard)\n"
   "\n"
   " 0. Cancel"
};

// ------------------------
// ABS new WP menus
menutext_t menuWpMain =
{
   0x3ff,
   "\n"
   "\n"
   " Waypoint Menus\n"
   "\n"
   " 1. Add\n"
   " 2. Delete \n"
   " 3. Set Radius\n"
   " 4. Set Flags\n"
   " 5. Add Path\n"
   " 6. Delete Path\n"
   " 7. Check\n"
   " 8. Save\n"
   " 9. Options\n"
   " 0. Cancel"
};

menutext_t menuWpAdd =
{
   0x3ff,
   "\n"
   "\n"
   " Add Waypoint\n"
   "\n"
   " 1. Normal\n"
   " 2. T Important\n"
   " 3. CT Important\n"
   " 4. Ladder\n"
   " 5. Rescue\n"
   " 6. Camp Start\n"
   " 7. Camp End\n"
   " 8. Goal\n"
   " 9. Jump\n"
   " 0. Cancel"
};

menutext_t menuWpDelete =
{
   0x003,
   "\n"
   "\n"
   " Delete Waypoint\n"
   " Are you sure ?\n"
   "\n"
   " 1. Yes\n"
   " 2. No"
};

menutext_t menuWpSetRadius =
{
   0x3ff,
   "\n"
   "\n"
   " Set Waypoint Radius\n"
   "\n"
   " 1. Radius 0\n"
   " 2. Radius 8\n"
   " 3. Radius 16\n"
   " 4. Radius 32\n"
   " 5. Radius 48\n"
   " 6. Radius 64\n"
   " 7. Radius 80\n"
   " 8. Radius 96\n"
   " 9. Radius 112\n"
   " 0. Cancel"
};

menutext_t menuWpSetFlags =
{
   0x3ff,
   "\n"
   "\n"
   " Set Waypoint Flags\n"
   "\n"
   " 1. Use Button\n"
   " 2. Lift\n"
   " 3. Crouch\n"
   " 4. Goal\n"
   " 5. Ladder\n"
   " 6. Rescue\n"
   " 7. Camp\n"
   " 8. No Hostage\n"
   " 9. TEAM Specific\n"
   " 0. Cancel"
};

menutext_t menuWpSetTeam =
{
   0x207,
   "\n"
   "\n"
   " TEAM Specific Waypoint\n"
   " Which Team ?\n"
   "\n"
   " 1. Terrorists\n"
   " 2. Counter-terrorists\n"
   " 3. Both Teams\n"
   "\n"
   " 0. Cancel"
};

menutext_t menuWpAddPath =
{
   0x207,
   "\n"
   "\n"
   " Add Path\n"
   " Which Direction ?\n"
   "\n"
   " 1. Outgoing Path\n"
   " 2. Incoming Path\n"
   " 3. Bidirectional (Both Ways)\n"
   "\n"
   " 0. Cancel"
};

menutext_t menuWpDeletePath =
{
   0x003,
   "\n"
   "\n"
   " Delete Path\n"
   " Are you sure ?\n"
   "\n"
   " 1. Yes\n"
   " 2. No"
};

menutext_t menuWpSave =
{
   0x003,
   "\n"
   "\n"
   " Save Waypoints\n"
   " Errors found!\n"
   "\n"
   "1. Save anyway\n"
   "2. Go back and fix errors"
};

menutext_t menuWpOptions1 =
{
   0x3ff,                                       // KWo - 04.10.2006
   "\n"
   "\n"
   " Waypointing Options (1/2)\n"
   "\n"
   " 1. WP on/off \n"
   " 2. AutoWP on/off\n"
   " 3. NoClip on/off\n"
   " 4. PeaceMode on/off\n"
   " 5. Show/Hide Flags\n"
   " 6. AutoPath Max Distance\n"
   " 7. Cache this Waypoint\n"
   " 8. Move here cached Waypoint\n"            // KWo - 29.03.2008
   " 9. More\n"                                 // KWo - 29.03.2008
   " 0. Cancel"
};

menutext_t menuWpOptions2 =                     // KWo - 29.03.2008
{
   0x283,
   "\n"
   "\n"
   " Waypointing Options (2/2)\n"
   "\n"
   " 1. Debuggoal Pointing WP\n"
   " 2. Debuggoal off\n"
   "\n"
   " 8. Back\n"
   " 0. Cancel"
};

menutext_t menuWpAutoPathMaxDistance =
{
   0x27f,
   "\n"
   "\n"
   " Auto-path Max Distance\n"
   "\n"
   " 1. Dist 0\n"
   " 2. Dist 100\n"
   " 3. Dist 130\n"
   " 4. Dist 160\n"
   " 5. Dist 190\n"
   " 6. Dist 220\n"
   " 7. Dist 250 (Default)\n"
   "\n"
   " 0. Cancel"
};
