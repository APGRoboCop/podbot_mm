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
// bot_globals_wp.h
//
// Only File to include in the Code (does include all the rest)

#ifndef BOT_GLOBALS_WP_H
#define BOT_GLOBALS_WP_H

#ifndef _WIN32
#include <string.h>
#include <ctype.h>
#endif

#include <extdll.h>
#include <dllapi.h>
#include <h_export.h>
#include <meta_api.h>

#include "bot.h"
#include "bot_weapons.h"
#include "waypoint.h"

#include <sys/types.h>
#include <sys/stat.h>

#define MAXNUMBOMBSPOTS 16

extern int g_iMapType;
extern bool g_bIsDedicatedServer;
extern bool g_bWaypointOn;
extern bool g_bWaypointsChanged;
extern bool g_bWaypointsSaved;
extern bool g_bAutoWaypoint;
extern float g_fAutoPathMaxDistance;
extern bool g_bShowWpFlags;
extern bool g_bLearnJumpWaypoint;
extern int g_iNumWaypoints;
extern int g_iCachedWaypoint;

extern char g_cKillHistory;
extern int g_rgiTerrorWaypoints[MAX_WAYPOINTS];
extern int g_iNumTerrorPoints;
extern int g_rgiCTWaypoints[MAX_WAYPOINTS];
extern int g_iNumCTPoints;
extern int g_rgiGoalWaypoints[MAX_WAYPOINTS];
extern int g_iNumGoalPoints;
extern int g_rgiCampWaypoints[MAX_WAYPOINTS];
extern int g_iNumCampPoints;
extern int g_rgiRescueWaypoints[MAX_WAYPOINTS];          // KWo - 24.12.2006
extern int g_iNumRescuePoints;                           // KWo - 24.12.2006
extern float g_fWPDisplayTime[MAX_WAYPOINTS];            // KWo - 05.01.2008
extern float g_fPathDisplayTime[MAX_WAYPOINTS];          // KWo - 05.01.2008
extern int *g_pFloydDistanceMatrix;
extern bool g_bMapInitialised;
extern bool g_bRecalcVis;
extern float g_fTimeDisplayVisTableMsg;
extern int g_iCurrVisIndex;
extern int g_iRowVisIndex;                               // KWo - 10.09.2006
extern unsigned char g_rgbyVisLUT[MAX_WAYPOINTS][MAX_WAYPOINTS / 4];
extern int g_rgiBombSpotsVisited[MAXNUMBOMBSPOTS];

extern int g_iDebugGoalIndex;
extern int g_iSearchGoalIndex;
extern int g_iPointedWpIndex;                            // KWo - 04.10.2006

extern bool g_b_DebugTasks;                     // KWo - 20.04.2013
extern bool g_b_DebugNavig;                     // KWo - 20.04.2013
extern bool g_b_DebugStuck;                     // KWo - 20.04.2013
extern bool g_b_DebugSensing;                   // KWo - 20.04.2013
extern bool g_b_DebugCombat;                    // KWo - 20.04.2013
extern bool g_b_DebugEntities;                  // KWo - 20.04.2013
extern bool g_b_DebugChat;                      // KWo - 20.04.2013
extern bool g_b_DebugWpEdit;                    // KWo - 04.05.2013

extern PATH *paths[MAX_WAYPOINTS];
extern experience_t *pBotExperienceData;
extern int *g_pFloydPathMatrix;
extern int *g_pWithHostageDistMatrix;
extern int *g_pWithHostagePathMatrix;
extern unsigned short g_iHighestDamageT;                 // KWo 09.04.2006
extern unsigned short g_iHighestDamageCT;                // KWo 09.04.2006
extern int g_iHighestDamageWpT;                          // KWo - 05.01.2008
extern int g_iHighestDamageWpCT;                         // KWo - 05.01.2008
extern bool g_bEndJumpPoint;
extern float g_fTimeJumpStarted;
extern Vector g_vecLearnVelocity;
extern Vector g_vecLearnPos;
extern int g_iLastJumpWaypoint;
extern Vector g_vecLastWaypoint;

extern edict_t *pHostEdict;

extern int g_i_botthink_index;                           // KWo - 02.05.2006
extern int g_iFrameCounter;                              // KWo - 10.03.2006


inline bool IsConnectedWithWaypoint (int a, int b)       // KWo - 17.02.2008 (inline again)
{
   // Checks if Waypoint A has a Connection to Waypoint Nr. B

   int ix;

   if ((a < 0) || (a >= g_iNumWaypoints) || (b < 0) || (b >= g_iNumWaypoints))  // KWo - 08.07.2006
      return (FALSE);

   for (ix = 0; ix < MAX_PATH_INDEX; ix++)
      if (paths[a]->index[ix] == b)
         return (TRUE);

   return (FALSE);
}

inline void GetValidWaypoint (bot_t *pBot)               // KWo - 17.02.2008 (inline again)
{
   const char *g_TaskNames1[19] =                        // KWo - 06.01.2008
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

   float fReachWpDelay = 5.0;

   // checks what the time the bot needs to reach the waypoint
   if ((pBot->curr_wpt_index >= 0) && (pBot->curr_wpt_index < g_iNumWaypoints) // KWo - 26.07.2006
        && (pBot->prev_wpt_index[0] >= 0) && (pBot->prev_wpt_index[0] < g_iNumWaypoints))
   {
      float fDistance = (paths[pBot->prev_wpt_index[0]]->origin - paths[pBot->curr_wpt_index]->origin).Length();
      if (fabs(pBot->f_move_speed) <= 0.0) // KWo - 13.04.2016
//      if (pBot->pEdict->v.maxspeed <= 0.0)
         fReachWpDelay = 5.0 * fDistance / 240.0;
      else
         fReachWpDelay = 5.0 * fDistance / fabs(pBot->f_move_speed);
      if ((paths[pBot->curr_wpt_index]->flags & W_FL_CROUCH) || (paths[pBot->curr_wpt_index]->flags & W_FL_LADDER) || (pBot->pEdict->v.button & IN_DUCK))
         fReachWpDelay *= 3.f;
      if (fReachWpDelay < 3.0)
         fReachWpDelay = 3.0;
      if (fReachWpDelay > 10.0)
         fReachWpDelay = 10.0;
   }

   // Checks if the last Waypoint the Bot was heading for is still valid

   // If Bot hasn't got a Waypoint we need a new one anyway
   if (pBot->curr_wpt_index == -1)
   {
      DeleteSearchNodes (pBot);
      BotFindWaypoint (pBot);
      pBot->wpt_origin = paths[pBot->curr_wpt_index]->origin;

      // FIXME
      // Do some error checks if we got a waypoint
   }

   // If time to get there expired get new one as well

   else if ((pBot->f_wpt_timeset + fReachWpDelay < gpGlobals->time) && FNullEnt (pBot->pBotEnemy)) // KWo - 26.07.2006
   {
   	  // KWo - if the bot cannot reach the destination (current waypoint), the calculation of experience
   	  // will store that data to prevent take this route so frequently in the future...

      if (g_b_DebugTasks || g_b_DebugNavig)
         ALERT (at_logged, "[DEBUG] GetValidWaypoint - Bot %s with task = %s didn't reach its waypoint %i during %f time.\n", pBot->name, g_TaskNames1[BotGetSafeTask(pBot)->iTask], pBot->curr_wpt_index, fReachWpDelay);

      if ((g_iNumWaypoints > 0) && (!g_bWaypointsChanged)
       && (pBot->curr_wpt_index >= 0) && (pBot->curr_wpt_index < g_iNumWaypoints)) // KWo - 16.06.2006
      {
         int iValue;
         int i;
         PATH *p;
         short int path_index;
         if (pBot->bot_team == TEAM_CS_TERRORIST)
         {
            iValue = (pBotExperienceData + (pBot->curr_wpt_index * g_iNumWaypoints) + pBot->curr_wpt_index)->uTeam0Damage;
            iValue += 10; // KWo - 06.01.2008
            if (iValue > MAX_DAMAGE_VAL)
               iValue = MAX_DAMAGE_VAL;

            if (iValue > g_iHighestDamageT)
            {
               g_iHighestDamageT = (unsigned short)iValue;
               g_iHighestDamageWpT = pBot->curr_wpt_index; // KWo - 05.01.2008
            }

            (pBotExperienceData + (pBot->curr_wpt_index * g_iNumWaypoints) + pBot->curr_wpt_index)->uTeam0Damage = (unsigned short)iValue;
            p = paths[pBot->curr_wpt_index];
            for (i = 0; i < MAX_PATH_INDEX; i++)
            {
               if ((p->index[i] > -1) && (p->index[i] < MAX_WAYPOINTS) )
               {
                  path_index = p->index[i];
                  iValue = (pBotExperienceData + (path_index * g_iNumWaypoints) + path_index)->uTeam0Damage;
                  iValue += 1; // KWo - 06.01.2008
                  if (iValue > MAX_DAMAGE_VAL)
                     iValue = MAX_DAMAGE_VAL;

                  if (iValue > g_iHighestDamageT)
                  {
                     g_iHighestDamageT = (unsigned short)iValue;
                     g_iHighestDamageWpT = path_index; // KWo - 05.01.2008
                  }

                  (pBotExperienceData + (path_index * g_iNumWaypoints) + path_index)->uTeam0Damage = (unsigned short)iValue;
               }
            }
         }
         else if (pBot->bot_team == TEAM_CS_COUNTER)
         {
            iValue = (pBotExperienceData + (pBot->curr_wpt_index * g_iNumWaypoints) + pBot->curr_wpt_index)->uTeam1Damage;
            iValue += 10; // KWo - 06.01.2008
            if (iValue > MAX_DAMAGE_VAL)
               iValue = MAX_DAMAGE_VAL;

            if (iValue > g_iHighestDamageCT)
            {
               g_iHighestDamageCT = (unsigned short)iValue;
               g_iHighestDamageWpCT = pBot->curr_wpt_index; // KWo - 05.01.2008
            }

            (pBotExperienceData + (pBot->curr_wpt_index * g_iNumWaypoints) + pBot->curr_wpt_index)->uTeam1Damage = (unsigned short)iValue;
            p = paths[pBot->curr_wpt_index];
            for (i = 0; i < MAX_PATH_INDEX; i++)
            {
               if ((p->index[i] > -1) && (p->index[i] < MAX_WAYPOINTS) )
               {
                  path_index = p->index[i];
                  iValue = (pBotExperienceData + (path_index * g_iNumWaypoints) + path_index)->uTeam1Damage;
                  iValue += 1; // KWo - 06.01.2008
                  if (iValue > MAX_DAMAGE_VAL)
                     iValue = MAX_DAMAGE_VAL;

                  if (iValue > g_iHighestDamageCT)
                  {
                     g_iHighestDamageCT = (unsigned short)iValue;
                     g_iHighestDamageWpCT = path_index; // KWo - 05.01.2008
                  }

                  (pBotExperienceData + (path_index * g_iNumWaypoints) + path_index)->uTeam1Damage = (unsigned short)iValue;
               }
            }
         }
      }

      DeleteSearchNodes (pBot);
      BotFindWaypoint (pBot);
      pBot->wpt_origin = paths[pBot->curr_wpt_index]->origin;

      if (g_b_DebugNavig)
         ALERT(at_logged, "[DEBUG] GetValidWaypoint - Bot %s didn't reach the current WP and deletes the path.\n", pBot->name);

   }
   return;
}

inline bool BotGoalIsValid (bot_t* pBot) // KWo - 17.02.2008 (inline again)
{
   int iGoal = BotGetSafeTask (pBot)->iData;

   if (iGoal == -1)
      return (FALSE); // Not decided about a goal

   else if (iGoal == pBot->curr_wpt_index)
      return (TRUE); // No Nodes needed

   else if (pBot->pWaypointNodes == NULL)
      return (FALSE); // No Path calculated

   if ((pBot->f_spawn_time + 10.0 < gpGlobals->time) && (g_i_botthink_index != g_iFrameCounter)) // KWo - 07.01.2008
      return (TRUE);

   // Got Path - check if still valid
   PATHNODE *Node = pBot->pWaypointNodes;

   while (Node->NextNode != NULL)
      Node = Node->NextNode;

   if (Node->iIndex == iGoal)
      return (TRUE);

   return (FALSE);
}

#endif
