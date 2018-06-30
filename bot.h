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
// bot.h
//
// Contains the Bot Structures and Function Prototypes

#ifndef BOT_H
#define BOT_H


#include <assert.h>

#define PBMM_VERSION_STRING		"V3B23-APG"    // [APG]RoboCop - 27.05.2018


typedef struct bottask_s
{
   bottask_s *pPreviousTask;
   bottask_s *pNextTask;
   int iTask; // Major Task/Action carried out
   float fDesire; // Desire (filled in) for this Task
   int iData; // Additional Data (Waypoint Index)
   float fTime; // Time Task expires
   bool bCanContinue; // If Task can be continued if interrupted
} bottask_t;


// Task Filter functions

inline struct bottask_s *clampdesire (bottask_t *t1, float fMin, float fMax)
{
   if (t1->fDesire < fMin)
      t1->fDesire = fMin;
   else if (t1->fDesire > fMax)
      t1->fDesire = fMax;

   return (t1);
}

inline struct bottask_s *maxdesire (bottask_t *t1,bottask_t *t2)
{
   if (t1->fDesire > t2->fDesire)
      return (t1);

   return (t2);
}

inline bottask_s *subsumedesire (bottask_t *t1,bottask_t *t2)
{
   if (t1->fDesire > 0)
      return (t1);

   return (t2);
}

inline bottask_s *thresholddesire (bottask_t *t1, float t, float d)
{
   if (t1->fDesire >= t)
      return (t1);
   else
   {
      t1->fDesire = d;
      return (t1);
   }
}

inline float hysteresisdesire (float x, float min, float max, float oldval)
{
   if ((x <= min) || (x >= max))
      oldval = x;

   return (oldval);
}


// define Console/CFG Commands
#define CONSOLE_CMD_PAUSE "pause\0"

enum podbotcmdindex
{
   PBCMD = 0,
   PBCMD_ADD,
   PBCMD_DEBUGGOAL,
   PBCMD_EXPERIENCE,
   PBCMD_FILLSERVER,
   PBCMD_HELP,
   PBCMD_KILLBOTS,
   PBCMD_MENU,
   PBCMD_REMOVE,
   PBCMD_REMOVEBOTS,
   PBCMD_SET,
   PBCMD_WEAPONMODE,
   PBCMD_WP,
   PBCMD_WPMENU,
   NUM_PBCOMMANDS
};

enum podbotcvarindex
{
   PBCVAR_AIM_DAMPER_COEFFICIENT_X,
   PBCVAR_AIM_DAMPER_COEFFICIENT_Y,
   PBCVAR_AIM_DEVIATION_X,
   PBCVAR_AIM_DEVIATION_Y,
   PBCVAR_AIM_INFLUENCE_X_ON_Y,
   PBCVAR_AIM_INFLUENCE_Y_ON_X,
   PBCVAR_AIM_NOTARGET_SLOWDOWN_RATIO,
   PBCVAR_AIM_OFFSET_DELAY,
   PBCVAR_AIM_SPRING_STIFFNESS_X,
   PBCVAR_AIM_SPRING_STIFFNESS_Y,
   PBCVAR_AIM_TARGET_ANTICIPATION_RATIO,  // KWo - 04.03.2006
   PBCVAR_AIM_TYPE,
   PBCVAR_AUTOKILL,          // KWo - 02.05.2006
   PBCVAR_AUTOKILLDELAY,     // KWo - 02.05.2006
   PBCVAR_BOTJOINTEAM,       // KWo - 16.09.2006
   PBCVAR_BOTQUOTAMATCH,     // KWo - 16.09.2006
   PBCVAR_CHAT,
   PBCVAR_DANGERFACTOR,
   PBCVAR_DEBUGLEVEL,        // KWo - 20.04.2013
   PBCVAR_DETAILNAMES,
   PBCVAR_FFA,               // KWo - 04.10.2006
   PBCVAR_FIRSTHUMANRESTART, // KWo - 04.10.2010
   PBCVAR_JASONMODE,
   PBCVAR_LATENCYBOT,        // KWo - 16.05.2008
   PBCVAR_MAPSTARTBOTJOINDELAY,
   PBCVAR_MAXBOTS,
   PBCVAR_MAXBOTSKILL,
   PBCVAR_MAXCAMPTIME,       // KWo - 23.03.2008
   PBCVAR_MAXWEAPONPICKUP,
   PBCVAR_MINBOTS,
   PBCVAR_MINBOTSKILL,
   PBCVAR_NUMFOLLOWUSER,
   PBCVAR_PASSWORD,
   PBCVAR_PASSWORDKEY,
   PBCVAR_RADIO,             // KWo - 03.02.2007
   PBCVAR_RESTREQUIPAMMO,    // KWo - 09.03.2006
   PBCVAR_RESTRWEAPONS,      // KWo - 09.03.2006
   PBCVAR_SHOOTTHRUWALLS,
   PBCVAR_SKIN,              // KWo - 18.11.2006
   PBCVAR_SPRAY,
   PBCVAR_TIMER_GRENADE,
   PBCVAR_TIMER_PICKUP,
   PBCVAR_TIMER_SOUND,
   PBCVAR_USESPEECH,
   PBCVAR_VERSION,
   PBCVAR_WELCOMEMSGS,
   PBCVAR_WPTFOLDER,
   NUM_PBCVARS
};

#define NUM_SPRAYPAINTS 16

#define BOT_NAME_LEN 32 // Max Botname len

//#define WANDER_LEFT 1  // What the heck are these for?
//#define WANDER_RIGHT 2

//#define BOT_YAW_SPEED 20.0
//#define BOT_PITCH_SPEED 20.0  // degrees per frame for rotation

// Collision States
enum
{
   COLLISION_NOTDECIDED = 0,
   COLLISION_PROBING,
   COLLISION_NOMOVE,
   COLLISION_JUMP,
   COLLISION_DUCK,
   COLLISION_STRAFELEFT,
   COLLISION_STRAFERIGHT,
   COLLISION_GOBACK
};

// Lift usage States
enum
{
   LIFT_NO_NEARBY = 0,
   LIFT_LOOKING_BUTTON_OUTSIDE,
   LIFT_WAITING_FOR,
   LIFT_GO_IN,
   LIFT_WAITING_FOR_TEAMNATES,
   LIFT_LOOKING_BUTTON_INSIDE,
   LIFT_TRAVELING_BY,
   LIFT_LEAVING
};

enum
{
   PROBE_JUMP = (1 << 0), // Probe Jump when colliding
   PROBE_DUCK = (1 << 1), // Probe Duck when colliding
   PROBE_STRAFE = (1 << 2), // Probe Strafing when colliding
   PROBE_GOBACK = (1 << 3) // Probe Go Back when colliding // KWo - 25.06.2006
};


// game start messages for CS...
enum
{
   MSG_CS_IDLE = 1,
   MSG_CS_TEAM_SELECT,
   MSG_CS_CT_SELECT,
   MSG_CS_T_SELECT
};

// Debug flag levels...
enum
{
   DEBUG_FL_TASKS = (1 << 0),
   DEBUG_FL_NAVIGATION = (1 << 1),
   DEBUG_FL_STUCK = (1 << 2),
   DEBUG_FL_SENSING = (1 << 3),
   DEBUG_FL_COMBAT = (1 << 4),
   DEBUG_FL_ENTITIES = (1 << 5),
   DEBUG_FL_CHAT = (1 << 6),
   DEBUG_FL_WPEDIT  = (1 << 7),
   DEBUG_FL_ALLBOTS = (1 << 8)
};

#define MSG_CS_MOTD	8  // KWo 11.02.2006

// teams for CS...
enum
{
   TEAM_CS_UNASSIGNED = 0,
   TEAM_CS_TERRORIST = 1,
   TEAM_CS_COUNTER = 2,
   TEAM_CS_SPECTATOR = 3
};

// Misc Message Queue Defines
#define MSG_CS_RADIO 200
#define MSG_CS_SAY 10000


// Radio Messages
enum
{
   // Radio Menu 1
   RADIO_COVERME = 1,
   RADIO_YOUTAKEPOINT,
   RADIO_HOLDPOSITION,
   RADIO_REGROUPTEAM,
   RADIO_FOLLOWME,
   RADIO_TAKINGFIRE,

   // Radio Menu 2
   RADIO_GOGOGO = 11,
   RADIO_FALLBACK,
   RADIO_STICKTOGETHER,
   RADIO_GETINPOSITION,
   RADIO_STORMTHEFRONT,
   RADIO_REPORTTEAM,

   // Radio Menu 3
   RADIO_AFFIRMATIVE = 21,
   RADIO_ENEMYSPOTTED,
   RADIO_NEEDBACKUP,
   RADIO_SECTORCLEAR,
   RADIO_IMINPOSITION,
   RADIO_REPORTINGIN,
   RADIO_SHESGONNABLOW,
   RADIO_NEGATIVE,
   RADIO_ENEMYDOWN
};


// Sensing States
enum
{
   STATE_SEEINGENEMY = (1 << 0), // Seeing an Enemy
   STATE_HEARINGENEMY = (1 << 1), // Hearing an Enemy
   STATE_PICKUPITEM = (1 << 2), // Pickup Item Nearby
   STATE_THROWHEGREN = (1 << 3), // Could throw HE Grenade
   STATE_THROWFLASHBANG = (1 << 4), // Could throw Flashbang
   STATE_THROWSMOKEGREN = (1 << 5), // Could throw SmokeGrenade
   STATE_SUSPECTENEMY = (1 << 6) // Suspect Enemy behind Obstacle
};

// Positions to aim at
enum
{
   AIM_DEST = (1 << 0), // Aim at Nav Point
   AIM_CAMP = (1 << 1), // Aim at Camp Vector
   AIM_PREDICTPATH = (1 << 2), // Aim at predicted Path
   AIM_LASTENEMY = (1 << 3), // Aim at Last Enemy
   AIM_ENTITY = (1 << 4), // Aim at Entity like Buttons,Hostages
   AIM_ENEMY = (1 << 5), // Aim at Enemy
   AIM_GRENADE = (1 << 6), // Aim for Grenade Throw
   AIM_OVERRIDE = (1 << 7) // Overrides all others (blinded)
};

// Tasks to do
enum
{
   TASK_NONE = -1,
   TASK_NORMAL = 0,
   TASK_PAUSE = 1,
   TASK_MOVETOPOSITION = 2,
   TASK_FOLLOWUSER = 3,
   TASK_WAITFORGO = 4,
   TASK_PICKUPITEM = 5,
   TASK_CAMP = 6,
   TASK_PLANTBOMB = 7,
   TASK_DEFUSEBOMB = 8,
   TASK_ATTACK = 9,
   TASK_ENEMYHUNT = 10,
   TASK_SEEKCOVER = 11,
   TASK_THROWHEGRENADE = 12,
   TASK_THROWFLASHBANG = 13,
   TASK_THROWSMOKEGRENADE = 14,
   TASK_SHOOTBREAKABLE = 15,
   TASK_HIDE = 16,
   TASK_BLINDED = 17,
   TASK_SPRAYLOGO = 18
};


// Some hardcoded Desire Defines used to override calculated ones
#define TASKPRI_NORMAL 35.0
#define TASKPRI_PAUSE 36.0
#define TASKPRI_CAMP 37.0
#define TASKPRI_SPRAYLOGO 38.0
#define TASKPRI_FOLLOWUSER 39.0
#define TASKPRI_MOVETOPOSITION 50.0
#define TASKPRI_DEFUSEBOMB 88.0  // KWo - 16.12.2007
#define TASKPRI_PLANTBOMB 89.0
#define TASKPRI_ATTACK 90.0
#define TASKPRI_SEEKCOVER 91.0
#define TASKPRI_HIDE 92.0
#define TASKPRI_THROWGRENADE 99.0
#define TASKPRI_BLINDED 100.0
#define TASKPRI_SHOOTBREAKABLE 100.0

#define WPMENU_WELCOMEMSG "\n\n\nWelcome to Austin & SoUlFaThEr's\nWaypoint Editor for POD-Bot mm\0"

// Defines for Pickup Items
enum
{
   PICKUP_NONE = 0,
   PICKUP_WEAPON,
   PICKUP_DROPPED_C4,
   PICKUP_PLANTED_C4,
   PICKUP_HOSTAGE,
   PICKUP_SHIELD,
   PICKUP_DEFUSEKIT
};

// Enemy Body Parts Seen
enum
{
   HEAD_VISIBLE = (1 << 0),
   WAIST_VISIBLE = (1 << 1),
   CUSTOM_VISIBLE = (1 << 2)
};


#define MAX_HOSTAGES 8
#define MAX_DAMAGE_VAL 2040
#define MAX_GOAL_VAL 2040
#define TIME_GRENPRIME 2.2
#define MAX_KILL_HIST 16

#define BOMBMAXHEARDISTANCE 1536.0
#define HAS_NVGOOGLES       (1<<0) // KWo - 26.05.2008

// for private data offsets KWo - 19.05.2006 - taken from AMX MOD X
#if defined __linux__
   #define EXTRAOFFSET 5 // offsets 5 higher in Linux builds
   #define EXTRAOFFSET_WEAPONS 4 // weapon offsets are obviously only 4 steps higher on Linux!
#else
   #define EXTRAOFFSET 0 // no change in Windows builds
   #define EXTRAOFFSET_WEAPONS 0
#endif // defined __linux__

#if !defined __amd64__
   // "hostage_entity" entities
   #define OFFSET_HOSTAGEFOLLOW   86 + EXTRAOFFSET
   #define OFFSET_HOSTAGEID      487 + EXTRAOFFSET
   #define OFFSET_NVGOOGLES      129 + EXTRAOFFSET // KWo - 26.05.2008
#else
   // Amd64 offsets here
   // "hostage_entity" entities
   #define OFFSET_HOSTAGEFOLLOW   51 + EXTRAOFFSET // +21, long=51, int=107! (must use the long* offset because pointers on amd64 are stored the size of longs, 8 bytes, instead of the usual int 4 bytes.)
   #define OFFSET_HOSTAGEID      516 + EXTRAOFFSET // +29
   #define OFFSET_NVGOOGLES      155 + EXTRAOFFSET // +26 // KWo - 26.05.2008
#endif

// This Structure links Waypoints returned from Pathfinder
typedef struct pathnode
{
   int id;
   int iIndex;
   int depth;
   int state; // {OPEN, NEW, CLOSED}
   double g;
   double h;
   double f;
   pathnode *parent;
   pathnode *NextNode;
   pathnode *prev;
} PATHNODE;


// This Structure holds String Messages
typedef struct stringnode
{
   char szString[256];
   struct stringnode *Next;
} STRINGNODE;


// Links Keywords and replies together
typedef struct replynode
{
   char szKeywords[256];
   struct replynode *pNextReplyNode;
   char cNumReplies;
   char cLastReply[4];  // KWo - 27.03.2010
   struct stringnode *pReplies;
} replynode_t;


// Botnames... I'm an idiot -SpLoRyGoN
// TODO: There should be an easier way that can save
//  memory by not having all that left over space
//  from names shorter than 32 characters...
typedef struct
{
   char name[BOT_NAME_LEN];
} botname_t;


typedef struct
{
   int iId; // weapon ID
   int iClip; // amount of ammo in the clip
   int iAmmo1; // amount of ammo in primary reserve
   int iAmmo2; // amount of ammo in secondary reserve
} bot_current_weapon_t;


typedef struct
{
   int iId; // the weapon ID value
   char weapon_name[64]; // name of the weapon when selecting it
   char model_name[64]; // Model Name to separate CS Weapons
   char viewmodel_name[64]; // Client View Model Name to separate CS Weapons
   char buy_shortcut[64]; // Buy Shortcut (CS 1.6 specific)
   char buy_command[64]; // Buy Script to get this Weapon (equivalent of the above but for CS 1.5)
   float primary_min_distance; // 0 = no minimum
   float primary_max_distance; // 9999 = no maximum
   float secondary_min_distance; // 0 = no minimum
   float secondary_max_distance; // 9999 = no maximum
   bool  can_use_underwater; // can use this weapon underwater
   bool  primary_fire_hold; // hold down primary fire button to use?
   float primary_charge_delay; // time to charge weapon
   int iPrice; // Price when buying
   int min_primary_ammo;
   int iTeamStandard; // Used by Team (Number) (standard map)
   int iTeamAS; // Used by Team (AS map)
   bool  bShootsThru; // Can shoot thru Walls
} bot_weapon_select_t;


typedef struct
{
   float fMinSurpriseDelay; // time in secs
   float fMaxSurpriseDelay; // time in secs
   int iPauseProbality; // % Probability to pause after reaching a waypoint
   float fBotCampStartDelay; // time in secs
   float fBotCampEndDelay; // time in secs
} skilldelay_t;


typedef struct
{
   float fAim_X; // X Max Offset
   float fAim_Y; // Y Max Offset
   float fAim_Z; // Z Max Offset
   int iHeadShot_Frequency; // % to aim at Haed
   int iHeardShootThruProb; // % to shoot thru Wall when heard
   int iSeenShootThruProb; // % to shoot thru Wall when seen
} botaim_t;


typedef struct
{
   float fMinTurnSpeed; // minimum turn speed
   float fMaxTurnSpeed; // maximum turn speed
} turnspeed_t;


// Struct for Menus
typedef struct
{
   int ValidSlots; // Ored together Bits for valid Keys
   char *szMenuText; // Ptr to actual String
} menutext_t;

enum clientflags
{
   CLIENT_USED = (1 << 0),
   CLIENT_ALIVE = (1 << 1),
   CLIENT_ADMIN = (1 << 2),
};

// Records some Player Stats each Frame and holds sound events playing
typedef struct
{
   int iFlags;
   edict_t *pEdict; // Ptr to actual Edict
   int iTeam; // What Team
   Vector vOrigin; // Position in the world
   Vector vecSoundPosition; // Position Sound was played
   float fHearingDistance; // Distance this Sound is heared
   float fTimeSoundLasting; // Real Time sound is played/heared (to compare wit the current one)
   float fMaxTimeSoundLasting; // Max time sound is played/heared (to divide the diffrence between that above one and the current one) KWo - 01.08.2006
   float welcome_time;
   float wptmessage_time;
   int iCurrentWeaponId;   // KWo - 12.12.2006
   int iCurrentClip;       // KWo - 15.08.2007
   float fReloadingTime;   // KWo - 15.08.2007
   float fDeathTime;       // KWo - 14.03.2010
   menutext_t *pUserMenu;  // Which menus is open for admin?
} client_t;


// Experience Data hold in memory while playing
typedef struct
{
   unsigned short uTeam0Damage; // Amount of Damage
   unsigned short uTeam1Damage; // "
   signed short iTeam0_danger_index; // Index of Waypoint
   signed short iTeam1_danger_index; // "
   signed short wTeam0Value; // Goal Value
   signed short wTeam1Value; // "
} experience_t;


// Experience Data when saving/loading
typedef struct
{
   unsigned char uTeam0Damage;
   unsigned char uTeam1Damage;
   signed char cTeam0Value;
   signed char cTeam1Value;
} experiencesave_t;


// Array to hold Params for creation of a Bot
typedef struct
{
   bool bNeedsCreation;
   char bot_name[BOT_NAME_LEN + 1];
   int bot_skill;
   int bot_personality;
   int bot_team;
   int bot_class;
} createbot_t;


typedef struct
{
   char cChatProbability;
   float fChatDelay;
   float fTimeNextChat;
   int iEntityIndex;
   char szSayText[512];
} saytext_t;


// Main Bot Structure
typedef struct
{
   bool is_used; // Bot used in the Game
   edict_t *pEdict; // ptr to actual Player edict
   int not_started; // team/class not chosen yet

   int start_action; // Team/Class selection state
   bool bDead; // dead or alive
   char name[BOT_NAME_LEN + 1]; // botname
   int bot_skill; // skill

   char sz_BotModelName[64]; // KWo 01.05.2006 - bot model name (taken by inokeyvalues)

   float fBaseAgressionLevel; // Base Levels used when initializing
   float fBaseFearLevel;
   float fAgressionLevel; // Dynamic Levels used ingame
   float fFearLevel;
   float fNextEmotionUpdate; // Next time to sanitize emotions

   float oldcombatdesire; // holds old desire for filtering

   unsigned char bot_personality; // Personality 0-2
   int iSprayLogo; // Index of Logo to use

   unsigned int iStates; // Sensing BitStates
   bottask_t *pTasks; // Ptr to active Tasks/Schedules

   float fTimePrevThink; // Last time BotThink was called
   float fTimePrevThink2; // Last time BotThink was called
   float fTimeFrameInterval; // Interval Time between BotThink calls

   // things from pev in CBasePlayer...
   int bot_team;
   int bot_class;
   int bot_money; // money in Counter-Strike

   bool bIsVIP; // Bot is VIP
   bool bIsChickenOrZombie; // Bot is a chicken or a zombie;   // KWo - 17.01.2011
   bool bIsLeader; // Bot is leader of his Team;
   float fTimeTeamOrder; // Time of Last Radio command

   float f_move_speed; // Current Speed forward/backward
   float f_sidemove_speed; // Current Speed sideways
   float fMinSpeed; // Minimum Speed in Normal Mode
   bool bCheckTerrain;
   bool bMoveToGoal; // KWo - 13.04.2010

   bool bOnLadder;
   bool bInWater;
   bool bJumpDone;
   float f_timeDoorOpen;
   float f_timeHitDoor;
   bool bCheckMyTeam;

   float prev_time; // Time previously checked movement speed
   float prev_speed; // Speed some frames before
   Vector v_prev_origin; // Origin " " "
   float f_moved_distance; // moved distance during 1 frame - KWo - 19.07.2006

   bool bCanJump; // Bot can jump over obstacle
   bool bCanDuck; // Bot can duck under obstacle

   float f_view_distance; // Current View distance
   float f_maxview_distance; // Maximum View distance

   int iActMessageIndex; // Current processed Message
   int iPushMessageIndex; // Offset for next pushed Message

   int aMessageQueue[32]; // Stack for Messages
   char szMiscStrings[160]; // Space for Strings (SayText...)
   int iRadioSelect; // Radio Entry

   // Holds the Index & the actual Message of the last
   // unprocessed Text Message of a Player
   saytext_t SaytextBuffer;

   float fButtonPushTime; // Next time to allow bot to push a button
   float fButtonNoticedTime; // The time bot noticed a new button - KWo - 14.07.2006
   bool bButtonPushDecided; // if there is a flag USE_BUTTON - bot decided to use it KWo - 09.07.2006
   bool bNewButtonNoticed; // bot noticed the new WP with flag USE_BUTTON KWo - 09.07.2006

   float f_itemcheck_time; // Time next Search for Items needs to be done
   edict_t *pBotPickupItem; // Ptr to Entity of Item to use/pickup
   edict_t *pItemIgnore[3]; // Ptr to Entity to ignore for pickup
   int iPickupType; // Type of Entity which needs to be used/picked up

   edict_t *pShootBreakable; // Ptr to Breakable Entity
   Vector vecBreakable; // Origin of Breakable
   edict_t *pBreakableIgnore; // Ptr to Entity to ignore for break

   int iNumWeaponPickups; // Counter of Pickups done

   float f_spawn_time; // Time this Bot spawned
   float f_kick_time; // "  "  " was kicked

   float f_lastchattime; // Time Bot last chatted

   bool bLogoSprayed; // Logo sprayed this round
   bool bDefendedBomb; // Defend Action issued

   float f_firstcollide_time; // Time of first collision
   float f_probe_time; // Time of probing different moves
   float fNoCollTime; // Time until next collision check
   float f_CollisionSidemove; // Amount to move sideways
   char cCollisionState; // Collision State
   char cCollisionProbeBits; // Bits of possible Collision Moves
   char cCollideMoves[5]; // Sorted Array of Movements  KWo - 25.06.2006
   char cCollStateIndex; // Index into cCollideMoves
   bool bPlayerCollision; // the bot needs back off because of the collision with a player - KWo - 11.07.2006
   bool bBotNeedsObviateObstacle; // the bot needs omit a teamnate or a hostage - KWo - 16.07.2006
   bool bHitDoor; // the bot hits a door - KWo - 24.10.2009

   Vector wpt_origin; // Origin of Current Waypoint
   Vector dest_origin; // Origin of move destination
   Vector str_l_origin; // Origin of strafe left destination // KWo - 14.09.2006
   Vector str_r_origin; // Origin of strafe right destination // KWo - 14.09.2006

   PATHNODE *pWaypointNodes; // Ptr to current Node from Path
   PATHNODE *pWayNodesStart; // Ptr to start of Pathfinding Nodes
   unsigned char byPathType; // Which Pathfinder to use
   int prev_goal_index; // Holds destination Goal wpt
   int chosengoal_index; // Used for experience, same as above
   float f_goal_value; // Ranking Value for this waypoint
   int curr_wpt_index; // Current wpt index
   int prev_wpt_index[5]; // Previous wpt indices from waypointfind
   int iWPTFlags;
   unsigned short curr_travel_flags; // Connection Flags like jumping
   Vector vecDesiredVelocity; // Desired Velocity for jump waypoints
   float f_wpt_timeset; // Time waypoint chosen by Bot
   float f_wpt_tim_str_chg; // Time waypoint strong change (<120 degrees)   // KWo - 02.11.2009

   edict_t *pBotEnemy; // ptr to Enemy Entity
   float fEnemyUpdateTime; // Time to check for new enemies
   float fEnemyReachableTimer; // Time to recheck if Enemy reachable
   bool bEnemyReachable; // Direct Line to Enemy
   float fEnemyOriginUpdateTime; // Time to update enemy's origin - for skills less than 100 in aiming system - KWo - 05.03.2006
   float fLastSeenEnOrgUpdateTime; // Time to update last seen enemy's origin - KWo - 05.05.2007
   float fShootThruSeenCheckTime; // Time to check again the probability of shooting through the wall an seen enemy  - KWo - 23.03.2008
   float fLastHeardEnOrgUpdateTime; // Time to update last heard enemy's origin - KWo - 05.05.2007
   bool bShootThruSeen; // The bot decided to shoot throw the wall an seen enemy - KWo - 05.05.2007
   bool bShootThruHeard;  // The bot decided to shoot throw the wall an heard enemy - KWo - 05.05.2007
   float fShootThruHeardCheckTime; // Time to check again the probability of shooting through the wall an heard enemy  - KWo - 05.05.2007
   float fOffsetUpdateTime; // Offset time to every update enemy's origin - for skills less than 100 - KWo - 05.03.2006

   float f_bot_see_enemy_time; // Time Bot sees Enemy
   float f_bot_see_new_enemy_time; // Time Bot sees the new Enemy  KWo - 29.04.2008
   float f_enemy_surprise_time; // Time of surprise
   float f_ideal_reaction_time; // Time of base reaction
   float f_actual_reaction_time; // Time of current reaction time

   float rgfYawHistory[2];
   float rgfPitchHistory[2];

   edict_t *pLastEnemy; // ptr to last Enemy Entity
   Vector vecLastEnemyOrigin; // Origin of last enemy
   unsigned char ucVisibility; // Which Parts are visible
   Vector vecVisPos; // visible origin KWo - 25.01.2008
   edict_t *pLastVictim; // ptr to killed Entity
   edict_t *pTrackingEdict; // ptr to last tracked Player when camping/hiding
   float fTimeNextTracking; // Time Waypoint Index for tracking Player is recalculated
   edict_t *pHuntEnemy; // ptr to enemy to be hunted  KWo - 11.04.2010
   Vector vecHuntEnemyOrigin; // Rememebered origin of hunt enemy KWo - 11.04.2010

   unsigned int iAimFlags; // Aiming Conditions
   bool bCanChooseAimDirection;
   Vector vecLookAt; // Vector Bot should look at
   Vector vecThrow; // Origin of Waypoint to Throw Grens
   float fChangeAimDirectionTime; // Time to change the aiming direction if AIM_DEST is used - KWo - 09.12.2007
   char cFlashBat; // Flash battery for flashlight
   bool bUsesNVG;  // bot uses nightvision google

   Vector vecEnemy; // Target Origin chosen for shooting
   Vector vecEnemyRandomOffset;  // Target Origin random offset chosen for shooting - 04.10.2009
   Vector vecGrenade; // Calculated Vector for Grenades
   Vector vecEntity; // Origin of Entities like Buttons etc.
   Vector vecCamp; // Aiming Vector when camping.

   int iLiftUsageState;  // KWo - 22.04.2006
   Vector v_LiftTravelPos;   // KWo - 22.04.2006
   edict_t *pLift; // ptr to lift entity KWo - 18.04.2006
   float f_UsageLiftTime; // time using a lift in any state of usage it KWo - 22.04.2006

   float fTimeWaypointMove; // Last Time Bot followed Waypoints

   bool bWantsToFire; // Bot needs consider firing
   edict_t *pAvoidGrenade; // ptr to Grenade Entity to avoid
   char cAvoidGrenade; // which direction to strafe away
   edict_t *pSmokeGrenade; // ptr to Smoke Grenade Entity
   float fTimeAvoidGrenade; // Time to check greanes around again

   Vector vecPosition; // Position to Move To (TASK_MOVETOPOSITION)

   edict_t *pBotUser; // ptr to User Entity (using a Bot)
   float f_bot_use_time; // Time last seen User

   edict_t *pRadioEntity; // ptr to Entity issuing a Radio Command
   int iRadioOrder; // actual Command

   edict_t *pHostages[MAX_HOSTAGES]; // ptr to used Hostage Entities

   float f_hostage_check_time; // Next time to check if Hostage should be used
   float fTimeHostageRescue; // Time the CT bot got Rescue point

   bool bIsReloading; // Bot is reloading a gun
   float f_reloadingtime;
   float f_zoomchecktime; // Time to check Zoom again

   int iBurstShotsFired; // Shots fired in 1 interval
   float fTimeLastFired;
   float fTimeFirePause;
   float f_shoot_time;

   bool  bCheckWeaponSwitch;
   float fTimeWeaponSwitch;
   float fTimeSilencerSwitch; // KWo - 08.01.2007

   int charging_weapon_id;
   float f_primary_charging;
   float f_secondary_charging;

   float f_grenade_check_time; // Time to check Grenade Usage
   bool bUsingGrenade;

   unsigned char byCombatStrafeDir; // Direction to strafe
   unsigned char byFightStyle; // Combat Style to use
   float f_lastfightstylecheck; // Time checked style
   float f_StrafeSetTime; // Time strafe direction was set

   float f_blind_time; // Time Bot is blind
   float f_blindmovespeed_forward; // Mad speeds when Bot is blind
   float f_blindmovespeed_side;
   float f_blindmove_time;    // KWo - 13.08.2008
   bool bShootLastPosition;   // KWo - 13.08.2008
   bool bMadShoot;            // KWo - 13.08.2008

   int iLastDamageType; // Stores Last Damage

   float f_sound_update_time; // Time next sound next soundcheck
   float f_heard_sound_time; // Time enemy has been heard
   float fTimeCamping; // Time to Camp
   int iCampDirection; // Camp Facing direction
   float fNextCampDirectionTime; // Time next Camp Direction change
   int iCampButtons; // Buttons to press while camping

   float f_jumptime; // Time last jump happened
   float f_ducktime; // Time last duck happened  KWo - 20.10.2006
   float f_falldowntime; // Time the bot started to falldown  KWo - 05.03.2010

   bool b_bomb_blinking; // Time to hold Button for planting
   bool b_has_defuse_kit; // Bot has Defuse Kit
   bool b_can_buy; // Buy Zone Icon is lit

   int iBuyCount; // Current Count in Buying
   float f_buy_time; // Time of next Buying

   bot_weapon_select_t *pSelected_weapon; // Weapon chosen to be selected
   bot_current_weapon_t current_weapon; // one current weapon for each bot
   int m_rgAmmo[MAX_AMMO_SLOTS]; // total ammo amounts (1 array for each bot)
   int m_rgAmmoInClip[MAX_WEAPONS]; // ammo in clip for each weapons

   Vector ideal_angles; // angle wanted
   Vector randomized_ideal_angles; // angle wanted with noise
   Vector angular_deviation; // angular deviation from current to ideal angles
   Vector aim_speed; // aim speed calculated
   Vector target_angular_speed;  // target/enemy angular speed (to calculate pos. anticipation) // KWo - 02.03.2006
   Vector randomized_angles; // noising angle
   float randomize_angles_time; // time last randomized location
   float player_target_time; // time last targeting
   float fRecoilTime;
   Vector rgvecRecoil[5];

   int iOffsetPing[2];     // KWo - 02.03.2010
   int iArgPing[3];        // KWo - 02.03.2010

   int i_msecval; // KWo - 17.03.2007
   float f_msecvalrest; // KWo - 17.03.2007

   edict_t *pHit; // KWo - 23.03.2007

   int i_TaskDeep; // KWo - 30.08.2006 added to test if stack for tasks is working correctly (new/delete)
   int i_PathDeep; // KWo - 30.08.2006 added to test if stack for paths is working correctly (new/delete)
   int i_ChatDeep; // KWo - 30.08.2006 added to test if stack for chat is working correctly (new/delete)
} bot_t;


#define MAX_BUTTONS 60  // KWo 07.02.2006
// Saved buttons Structure
typedef struct
{
   short EntIndex;  // Entity index
   char classname[40]; // the classname of entity
   vec3_t origin;   // Vec Model Origin
   char target[64];  // name of the target the button is targetting
} saved_buttons_t;  // KWo 10.02.2006


#define MAX_BREAKABLES 60  // KWo 07.02.2006
// Saved breakables Structure
typedef struct
{
   short EntIndex;  // Entity index
   char     classname[40];
   vec3_t   origin;
   char     target[64];
   bool     ignored; // KWo 04.03.2006
} saved_break_t;  // KWo 10.02.2006

// Saved data about hostages and their positions
typedef struct
{
   short EntIndex;  // Entity index
   vec3_t   OldOrigin;
   bool IsMoving;
   bool Alive;
   short UserEntIndex;
} saved_hostages_t;  // KWo 16.05.2006


// bot.cpp functions...
void EstimateNextFrameDuration (void);
void BotSpawnInit (bot_t *pBot);
void BotCreate (int bot_skill, int bot_personality, int bot_team, int bot_class, const char *bot_name);
bool IsDeadlyDropAtPos (bot_t *pBot, Vector vecTargetPos);     // KWo - 02.04.2010
bool IsDeadlyDrop (bot_t *pBot, Vector vecTargetPos);
void BotPushTask (bot_t *pBot,bottask_t *pTask);
void BotTaskComplete (bot_t *pBot);
void BotResetTasks (bot_t *pBot);
bottask_t *BotGetSafeTask (bot_t *pBot);
void BotRemoveCertainTask (bot_t *pBot, int iTaskNum);
void BotThink (bot_t *pBot);
bool BotEnemyIsThreat (bot_t *pBot);
bool BotReactOnEnemy (bot_t *pBot);
bool BotHasHostage (bot_t *pBot);
int GetBestWeaponCarried (bot_t *pBot);
int GetBestSecondaryWeaponCarried (bot_t *pBot);
int BotInFieldOfView (bot_t *pBot, Vector dest);               // KWo - 29.01.2008
bool BotEntityIsVisible (bot_t *pBot, Vector dest);
int BotGetMessageQueue (bot_t *pBot);
void BotPushMessageQueue (bot_t *pBot, int iMessage);
void BotPlayRadioMessage (bot_t *pBot, int iMessage);
void BotFreeAllMemory (void);
void TestDecal (edict_t *pEdict, char *pszDecalName);

// bot_combat.cpp functions...
int NumTeammatesNearPos (bot_t *pBot, Vector vecPosition, int iRadius);
int NumEnemiesNearPos (bot_t *pBot, Vector vecPosition, int iRadius);
bool BotFindEnemy (bot_t *pBot);
bool BotEnemyIsVisible (bot_t *pBot, edict_t *pEnemy);         // KWo - 27.01.2008
bool IsShootableThruObstacle (edict_t *pEdict, Vector vecDest);
bool BotFireWeapon (Vector v_enemy, bot_t *pBot);
bool BotCheckZoom  (bot_t *pBot);                              // KWo - 07.07.2008
bool BotCheckCorridor (bot_t *pBot);                           // KWo - 21.03.2006
void BotFocusEnemy (bot_t *pBot);
void BotShoot (bot_t *pBot);
void BotDoAttackMovement (bot_t *pBot);
bool WeaponShootsThru (int iId);
bool WeaponIsSniper (int iId);
bool WeaponIsRifle (int iId);
bool WeaponIsAssualtSniper (int iId);
bool WeaponIsSubmachineGun (int iId);
bool WeaponIsPrimaryGun (int iId);
bool WeaponIsPistol (int iId);
bool WeaponIsNade (int iId);                                   // KWo - 15.01.2007
bool BotUsesRifle (bot_t *pBot);
bool BotUsesSniper (bot_t *pBot);
bool BotHasPrimaryWeapon (bot_t *pBot);
bool BotHasSecondaryWeapon (bot_t *pBot);                      // KWo - 18.01.2011
bool BotHasSniperWeapon (bot_t *pBot);                         // KWo - 27.02.2007
bool BotHasRifleWeapon (bot_t *pBot);                          // KWo - 27.02.2007
bool BotHasSubmachineGun (bot_t *pBot);                        // KWo - 19.05.2010
bool BotHasCampWeapon (bot_t *pBot);                           // KWo - 19.05.2010
bool BotHasShield (bot_t *pBot);
bool BotHasShieldDrawn (bot_t *pBot);
bool PlayerHasShieldDrawn (edict_t *pPlayer);                  // KWo - 15.08.2007
int BotCheckGrenades (bot_t *pBot);
int GetWeaponIndexOfBot (bot_t *pBot);                         // KWo - 04.04.2010
int HighestWeaponOfEdict (edict_t *pEdict);
int HighestPistolOfEdict (edict_t *pEdict);
void BotSelectBestWeapon (bot_t *pBot);
void SelectWeaponByName (bot_t *pBot, char *pszName);
void SelectWeaponByNumber (bot_t *pBot, int iNum);
void BotCommandTeam (bot_t *pBot);
bool IsGroupOfEnemies (bot_t *pBot, Vector vLocation);
Vector VecCheckToss (bot_t *pBot, const Vector &vecSpot1, Vector vecSpot2);
Vector VecCheckThrow (bot_t *pBot, const Vector &vecSpot1, Vector vecSpot2, float flSpeed);  // KWo - 22.03.2006

// bot_sound.cpp functions...
void SoundAttachToThreat (edict_t *pEdict, const char *pszSample, float fVolume);
void SoundSimulateUpdate (int iPlayerIndex);

// bot_chat.cpp functions...
void BotPrepareChatMessage (bot_t *pBot, char *pszText);
bool BotRepliesToPlayer (bot_t *pBot);

// bot_client.cpp functions...
void BotClient_CS_VGUI (void *p, int bot_index);
void BotClient_CS_ShowMenu (void *p, int bot_index);
void BotClient_CS_StatusIcon (void *p, int bot_index);
void BotClient_CS_WeaponList (void *p, int bot_index);
void BotClient_CS_CurrentWeapon (void *p, int bot_index);
void BotClient_CS_AmmoX (void *p, int bot_index);
void BotClient_CS_AmmoPickup (void *p, int bot_index);
void BotClient_CS_Damage (void *p, int bot_index);
void BotClient_CS_Money (void *p, int bot_index);
void BotClient_CS_DeathMsg (void *p, int bot_index);
void BotClient_CS_ScreenFade (void *p, int bot_index);
void BotClient_CS_SayText (void *p, int bot_index);
void BotClient_CS_HLTV (void *p, int bot_index);
void BotClient_CS_BombDrop (void *p, int bot_index);
void BotClient_CS_BombPickup (void *p, int bot_index);
void BotClient_CS_TextMsgAll (void *p, int bot_index);
void BotClient_CS_TextMsg1 (void *p, int bot_index);           // KWo - 18.05.2006
void BotClient_CS_TeamScore (void *p, int bot_index);          // KWo - 02.05.2006
void BotClient_CS_ResetHUD (void *p, int bot_index);
void Client_CS_ScoreInfo (void *p, int/* iPlayerIndex*/);
void Client_CS_ScoreAttrib (void *p, int/* iPlayerIndex*/);    // KWo - 02.03.2010
void Client_CS_TeamInfo (void *p, int/* iPlayerIndex*/);       // KWo - 12.02.2006 & THE_STORM
void BotClient_CS_RoundTime (void *p, int bot_index);          // KWo - 01.08.2006
void BotClient_CS_BarTime (void *p, int bot_index);            // KWo - 13.07.2007
void BotClient_CS_FlashBat (void *p, int bot_index);           // KWo - 25.05.2008
void BotClient_CS_NVGToggle (void *p, int bot_index);          // KWo - 28.05.2008

// dll.cpp functions...
void FakeClientCommand (edict_t *pFakeClient, const char *fmt, ...);
const char *GetField (const char *string, int field_number);

// engine.cpp functions...
int FAST_GET_USER_MSG_ID(plid_t plindex, int & value, const char * name, int * size); // KWo - 18.03.2012

// waypoint.cpp functions...
int WaypointLookAt (void); // KWo - 04.10.2006

// util.cpp functions...
unsigned short FixedUnsigned16 (float value, float scale);
short FixedSigned16 (float value, float scale);
int UTIL_GetTeam (edict_t *pEntity);
bot_t *UTIL_GetBotPointer (edict_t *pEdict);
bool IsAlive (edict_t *pEdict);
bool FInViewCone (Vector *pOrigin, edict_t *pEdict);
float GetShootingConeDeviation (edict_t *pEdict, Vector *pvecPosition);
bool IsShootableBreakable (edict_t *pent);
bool FBoxVisible (bot_t *pBot, edict_t *pTargetEdict, Vector *pvHit, unsigned char *ucBodyPart);
bool FVisible (const Vector &vecOrigin, edict_t *pEdict);
Vector Center (edict_t *pEdict);
Vector GetGunPosition (edict_t *pEdict);
Vector VecBModelOrigin (edict_t *pEdict);
void UTIL_ShowMenu (edict_t *pEdict, menutext_t *pMenu);
void UTIL_DisplayWpMenuWelcomeMessage (void);
void UTIL_DecalTrace (TraceResult *pTrace, char *pszDecalName);
int UTIL_GetNearestPlayerIndex (Vector vecOrigin);
void UTIL_HostPrint (const char *fmt, ...);
void UTIL_ServerPrint (const char *fmt, ...);
// void UTIL_ClampAngle (float *fAngle);
// void UTIL_ClampVector (Vector *vecAngles);
void UTIL_RoundStart (void);
void UTIL_GameStarted (void);                                  // KWo 09.02.2006
void UTIL_RoundEnd (void);                                     // KWo 02.05.2006
void UTIL_SaveButtonsData (void);                              // KWo 07.02.2006
void UTIL_SaveBreakableData (void);                            // KWo 10.02.2006
void UTIL_SaveHostagesData (void);                             // KWo 16.05.2006
void UTIL_CheckHostages (void);                                // KWo 17.05.2006
void UTIL_CheckCvars (void);                                   // KWo 06.04.2006
bool UTIL_CanUseWeapon (int iId);                              // KWo 10.03.2006
void UTIL_FindButtonInSphere (void);                           // KWo 09.02.2006
float UTIL_GetVectorsCone (Vector vec1_start, Vector vec1_end, Vector vec2_start, Vector vec2_end);  // KWo - 13.02.2006
void UTIL_DrawBeam (Vector start, Vector end, int life, int width, int noise, int red, int green, int blue, int brightness, int speed);  // KWo - 21.03.2006
void UTIL_CheckSmokeGrenades(void);                            // KWo - 29.01.2008
float UTIL_IlluminationOf (edict_t *pEdict);                   // KWo - 26.03.2008
void SetBotNvg(bot_t *pBot, bool setnv);                       // KWo - 28.05.2008
bool BotHasNvg(bot_t *pBot);                                   // KWo - 26.05.2008
void UTIL_HudMessage(edict_t *pEntity, const hudtextparms_t &textparms, char *pMessage); // KWo - 16.01.2010
char* UTIL_SplitHudMessage(const char *src);                   // KWo - 16.01.2010

#endif // BOT_H
