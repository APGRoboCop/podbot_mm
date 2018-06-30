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
// waypoint.h
//
// Contains Defines and Structures for the Waypoint Code

#ifndef WAYPOINT_H
#define WAYPOINT_H

#include "bot_globals_wp.h"

#define MAX_WAYPOINTS 1024
#define MAX_PATH_INDEX 8
#define OLDMAX_PATH_INDEX 4

enum
{
   MAP_AS = (1 << 0),
   MAP_CS = (1 << 1),
   MAP_DE = (1 << 2),
   MAP_AWP = (1 << 3),  // KWo - 18.03.2006
   MAP_AIM = (1 << 4),  // KWo - 18.03.2006
   MAP_FY = (1 << 5),  // KWo - 18.03.2006
   MAP_ES = (1 << 6)  // KWo - 18.03.2006
};


// #define MAX_WAYPOINTS 1024

// defines for waypoint flags field (32 bits are available)
enum
{
   W_FL_USE_BUTTON = (1 << 0), // use a nearby button (lifts, doors, etc.)
   W_FL_LIFT = (1 << 1), // wait for lift to be down before approaching this waypoint
   W_FL_CROUCH = (1 << 2), // must crouch to reach this waypoint
   W_FL_CROSSING = (1 << 3),   // a target waypoint
   W_FL_GOAL = (1 << 4), // mission goal point (bomb, hostage etc.)
   W_FL_LADDER = (1 << 5), // waypoint is on ladder
   W_FL_RESCUE = (1 << 6), // waypoint is a Hostage Rescue Point
   W_FL_CAMP = (1 << 7), // waypoint is a Camping Point
   W_FL_NOHOSTAGE = (1 << 8),   // only use this waypoint if no hostage
   W_FL_TERRORIST = (1 << 29), // It's a specific Terrorist Point
   W_FL_COUNTER = (1 << 30) // It's a specific Counter Terrorist Point
};

enum
{
   WAYPOINT_ADD_NORMAL = 0,
   WAYPOINT_ADD_USE_BUTTON,
   WAYPOINT_ADD_LIFT,
   WAYPOINT_ADD_CROUCH,
   WAYPOINT_ADD_CROSSING,
   WAYPOINT_ADD_GOAL,
   WAYPOINT_ADD_LADDER,
   WAYPOINT_ADD_RESCUE,
   WAYPOINT_ADD_CAMP_START,
   WAYPOINT_ADD_CAMP_END,
   WAYPOINT_ADD_NOHOSTAGE,
   WAYPOINT_ADD_TERRORIST,
   WAYPOINT_ADD_COUNTER,
   WAYPOINT_ADD_JUMP_START,
   WAYPOINT_ADD_JUMP_END
};

enum
{
   PATH_OUTGOING = 0,
   PATH_INCOMING,
   PATH_BOTHWAYS
};

enum
{
   FLAG_CLEAR = 0,
   FLAG_SET,
   FLAG_TOGGLE
};

// defines for waypoint connection flags field (16 bits are available)
#define C_FL_JUMP (1 << 0) // Must Jump for this Connection

#define WAYPOINT_VERSION7 7
#define WAYPOINT_VERSION6 6
#define WAYPOINT_VERSION5 5

#define EXPERIENCE_VERSION 2
#define VISTABLE_VERSION 1


// define the waypoint file header structure...
typedef struct
{
   char filetype[8]; // should be "PODWAY!\0"
   int waypoint_file_version;
   int number_of_waypoints;
   char mapname[32]; // name of map for these waypoints
   char creatorname[32]; // Name of Waypoint File Creator
} WAYPOINT_HDR;


// define the experience file header structure...
typedef struct
{
   char filetype[8]; // should be "PODEXP!\0"
   int experiencedata_file_version;
   int number_of_waypoints;
} EXPERIENCE_HDR;


// define the vistable file header structure...
typedef struct
{
   char filetype[8]; // should be "PODVIS!\0"
   int vistable_file_version;
   int number_of_waypoints;
} VISTABLE_HDR;


// #define MAX_PATH_INDEX 8
// #define OLDMAX_PATH_INDEX 4


// define the structure for waypoint paths (paths are connections between
// two waypoint nodes that indicates the bot can get from point A to point B.
// note that paths DON'T have to be two-way.  sometimes they are just one-way
// connections between two points.  There is an array called "paths" that
// contains head pointers to these structures for each waypoint index.
typedef struct path
{
   int iPathNumber;
   int flags; // button, lift, flag, health, ammo, etc.
   Vector origin; // location

   float Radius; // Maximum Distance WPT Origin can be varied

   float fcampstartx;
   float fcampstarty;
   float fcampendx;
   float fcampendy;
   short int index[MAX_PATH_INDEX]; // indexes of waypoints (index -1 means not used)
   unsigned short connectflag[MAX_PATH_INDEX];
   Vector vecConnectVel[MAX_PATH_INDEX];
   int distance[MAX_PATH_INDEX];
   struct path *next; // link to next structure
} PATH;
// sizeof(PATH) is different on amd64 than on x86 because pointer
// member(next) is 8 bytes on amd64 and 4 bytes on x86.
// That is why we need this macro for getting correct size when loading
// from waypoint file (and when saving).
//  - Jussi "hullu" Kivilinna
#define SIZEOF_PATH (sizeof(PATH) + (sizeof(int) - sizeof(void*)))


// Path Structure used by Version 6
typedef struct path6
{
   int iPathNumber;
   int flags; // button, lift, flag, health, ammo, etc.
   Vector origin; // location

   float Radius; // Maximum Distance WPT Origin can be varied

   float fcampstartx;
   float fcampstarty;
   float fcampendx;
   float fcampendy;
   short int index[MAX_PATH_INDEX]; // indexes of waypoints (index -1 means not used)
   int distance[MAX_PATH_INDEX];
   struct path6 *next; // link to next structure
} PATH6;
#define SIZEOF_PATH6 (sizeof(PATH6) + (sizeof(int) - sizeof(void*)))


// Path Structure used by Version 5
typedef struct path5
{
   int iPathNumber;
   int flags; // button, lift, flag, health, ammo, etc.
   Vector origin;   // location

   float Radius; // Maximum Distance WPT Origin can be varied

   float fcampstartx;
   float fcampstarty;
   float fcampendx;
   float fcampendy;
   short int index[OLDMAX_PATH_INDEX]; // indexes of waypoints (index -1 means not used)
   int distance[OLDMAX_PATH_INDEX];
   struct path5 *next; // link to next structure
} PATH5;
#define SIZEOF_PATH5 (sizeof(PATH5) + (sizeof(int) - sizeof(void*)))


// waypoint function prototypes...
int WaypointFindNearest (void);
void WaypointAdd (int iFlags);
void WaypointDelete (void);
void WaypointCache (void);
void WaypointMoveToPosition (void);          // KWo - 09.11.2007
void WaypointChangeRadius (float radius);
void WaypointChangeFlag (int flag, char status);
void WaypointCreatePath (char direction);
void WaypointDeletePath (void);
void WaypointCleanUnnessPaths (int index);   // KWo - 10.11.2007
void WaypointFixOldCampType (int index);     // KWo - 21.05.2013
void WaypointCalcVisibility (void);

bool ConnectedToWaypoint (int a, int b);
void CalculateWaypointWayzone (void);
void SaveExperienceTab (void);
void InitExperienceTab (void);
void SaveVisTab (void);
void InitVisTab (void);
bool WaypointLoad (void);
void WaypointSave (void);
bool WaypointReachableByEnt (Vector v_srv, Vector v_dest, edict_t *pEntity); // KWo - 30.07.2006
bool WaypointReachable (bot_t *pBot, int WP_Index); // KWo - 30.07.2006
void WaypointThink (void);
bool WaypointIsConnected (int iNum);
void WaypointDrawBeam (Vector start, Vector end, int width, int red, int green, int blue);
bool WaypointNodesValid (void);
bool WaypointNodeReachable (int i_src, int i_dest); // KWo - 06.01.2008


// bot navigate function prototypes
int WaypointFindNearestToMove (edict_t *pEnt, Vector vOrigin); // KWo - 17.04.2008
void WaypointFindInRadius (Vector vecPos, float fRadius, int *pTab, int *iCount);
float GetTravelTime (float fMaxSpeed, Vector vecSource,Vector vecPosition);
bool WaypointIsVisible (int iSourceIndex, int iDestIndex);
void BotChangeWptIndex (bot_t *pBot, int iWptIndex);
int GetAimingWaypoint (bot_t *pBot,Vector vecTargetPos, int iCount);
bool BotFindWaypoint (bot_t *pBot);
void GetValidWaypoint (bot_t *pBot);
void CTBombPointClear (int iIndex);
Vector GetBombPosition (void);
int BotFindDefendWaypoint (bot_t *pBot, Vector vecPosition);
int BotFindCoverWaypoint (bot_t *pBot, float maxdistance);
bool IsConnectedWithWaypoint (int a, int b);
bool BotCantMoveForward (bot_t *pBot, Vector vNormal);
bool BotCanStrafeLeft (bot_t *pBot, TraceResult *tr);
bool BotCanStrafeRight (bot_t *pBot, TraceResult *tr);
bool BotCanJumpUp (bot_t *pBot, Vector vNormal);
bool BotCanDuckUnder (bot_t *pBot, Vector vNormal);
bool BotIsBlockedLeft (bot_t *pBot);
bool BotIsBlockedRight (bot_t *pBot);
bool BotCheckWallOnLeft (bot_t *pBot);
bool BotCheckWallOnRight (bot_t *pBot);
void BotGetCampDirection (bot_t *pBot, Vector *vecDest);
void UpdateGlobalExperienceData (void);
void BotCollectGoalExperience (bot_t *pBot, int iDamage);
void BotCollectExperienceData (edict_t *pVictimEdict,edict_t *pAttackerEdict, int iDamage);
bool BotDoWaypointNav (bot_t *pBot);
bool BotGoalIsValid (bot_t* pBot);
int BotFindGoal (bot_t *pBot);
void BotResetCollideState (bot_t *pBot);
int BotCheckCollisionWithPlayer (bot_t *pBot, edict_t *pPlayer); // KWo - 01.04.2010
void BotCheckTerrain (bot_t *pBot);
STRINGNODE *GetNodeSTRING (STRINGNODE* pNode, int NodeNum);

// Floyd Search Prototypes
void DeleteSearchNodes (bot_t *pBot);
void InitPathMatrix (void);
int GetPathDistance (int iSourceWaypoint, int iDestWaypoint);
PATHNODE *FindShortestPath (int iSourceIndex, int iDestIndex, bool *bValid);


// A* Stuff
enum
{
   OPEN,
   CLOSED,
   NEW
};

int gfunctionKillsT (PATHNODE *p);
int gfunctionKillsCT (PATHNODE *p);
int gfunctionKillsCTWithHostage (PATHNODE *p);
int gfunctionKillsDistT (PATHNODE *p);
int gfunctionKillsDistCT (PATHNODE *p);
int gfunctionKillsDistCTWithHostage (PATHNODE *p);
int hfunctionNone (PATHNODE *p);
int hfunctionSquareDist (PATHNODE *p);
int hfunctionSquareDistWithHostage (PATHNODE *p); // KWo - 22.02.2007
int goal (PATHNODE *p);
PATHNODE *makeChildren (PATHNODE *parent);
int nodeEqual (PATHNODE *a, PATHNODE *b);
void TestAPath (int iTeam, int iWithHostage, int iSourceIndex, int iDestIndex, unsigned char byPathType);
PATHNODE *FindLeastCostPath (bot_t *pBot, int iSourceIndex, int iDestIndex);


// function prototypes
PATHNODE *AStarSearch (PATHNODE *root, int (*gcalc) (PATHNODE *), int (*hcalc) (PATHNODE *),
                       int (*goalNode) (PATHNODE *), PATHNODE * (*children) (PATHNODE *),
                       int (*nodeEqual) (PATHNODE *, PATHNODE *));


int Encode (char *filename, unsigned char* header, int headersize, unsigned char *buffer, int bufsize);
int Decode (char *filename, int headersize, unsigned char *buffer, int bufsize);


#endif // WAYPOINT_H
