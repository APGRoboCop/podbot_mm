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
// bot_combat.cpp
//
// Does Enemy Sensing (spotting), combat movement and firing weapons

#include "bot_globals.h"


int NumTeammatesNearPos (bot_t *pBot, Vector vecPosition, int iRadius)
{
   int iCount = 0;
   float fDistance;
   edict_t *pEdict = pBot->pEdict;
   int i;
   Vector vPlayerOrigin; // KWo - 19.10.2008

   if (g_b_cv_ffa) // KWo - 05.10.2006
      return (0);

   for (i = 0; i < gpGlobals->maxClients; i++)
   {
      if (!(clients[i].iFlags & CLIENT_USED)
          || !(clients[i].iFlags & CLIENT_ALIVE)
          || ((clients[i].iTeam != pBot->bot_team) || (g_b_cv_ffa)) // KWo - 05.10.2006
          || (clients[i].pEdict == pEdict))
         continue;

      fDistance = (clients[i].vOrigin - vecPosition).Length ();
      if (fDistance < iRadius)
      {
         vPlayerOrigin = clients[i].pEdict->v.origin;  // KWo - 19.10.2008
         if (FVisible(vPlayerOrigin, pBot->pEdict))    // KWo - 19.10.2008
            iCount++;
      }
   }

   return (iCount);
}


int NumEnemiesNearPos (bot_t *pBot, Vector vecPosition, int iRadius)
{
   int iCount = 0;
   float fDistance;
   int i;
   Vector vPlayerOrigin; // KWo - 19.10.2008

   for (i = 0; i < gpGlobals->maxClients; i++)
   {
      if (!(clients[i].iFlags & CLIENT_USED)
          || !(clients[i].iFlags & CLIENT_ALIVE)
          || (((clients[i].iTeam == pBot->bot_team)) && (!g_b_cv_ffa))) // KWo - 05.10.2006
         continue;

      fDistance = (clients[i].vOrigin - vecPosition).Length ();
      if (fDistance < iRadius)
      {
         vPlayerOrigin = clients[i].pEdict->v.origin;  // KWo - 19.10.2008
         if (FVisible(vPlayerOrigin, pBot->pEdict))    // KWo - 19.10.2008
            iCount++;
      }
   }

   return (iCount);
}

bool BotEnemyIsVisible (bot_t *pBot, edict_t *pEnemy) // KWo - 27.01.2008
{
   TraceResult tr;
   int RenderFx;                 // KWo - 22.03.2008
   int RenderMode;               // KWo - 22.03.2008
   int EntEnemyIndex;            // KWo - 17.01.2011
   int EnemyWeapon;              // KWo - 17.01.2011
   Vector RenderColor;           // KWo - 22.03.2008
   float RenderAmount;           // KWo - 22.03.2008
   float LightLevel;             // KWo - 23.03.2008
   bool SemiTransparent = false; // KWo - 22.03.2008
   bool EnemyWeaponIsGun = false;// KWo - 17.01.2011

   if (FNullEnt(pEnemy))
      return(false);
   if(!FStrEq (STRING (pEnemy->v.classname), "player"))
      return(false);

// Can't see the target entity if blinded or smoked...
   if ((pBot->pEdict->v.origin - pEnemy->v.origin).Length() > pBot->f_view_distance) // KWo - 14.09.2008
      return (FALSE);


// KWo - 22.03.2008 - added invisibility check
   RenderFx = pEnemy->v.renderfx;
   RenderMode = pEnemy->v.rendermode;
   RenderColor = pEnemy->v.rendercolor;
   RenderAmount = pEnemy->v.renderamt;
   EntEnemyIndex = ENTINDEX (pEnemy) - 1;                                                    // 17.01.2011
   if ((EntEnemyIndex >= 0) && (EntEnemyIndex < gpGlobals->maxClients))                      // 17.01.2011
   {
      EnemyWeapon = clients[EntEnemyIndex].iCurrentWeaponId;                                 // 17.01.2011
      EnemyWeaponIsGun = (WeaponIsPistol(EnemyWeapon) || WeaponIsPrimaryGun(EnemyWeapon));   // 17.01.2011
   }

   if (((RenderFx == kRenderFxExplode) || (pEnemy->v.effects & EF_NODRAW))                   // 17.01.2011
      && (!(pEnemy->v.oldbuttons & IN_ATTACK) || !(EnemyWeaponIsGun))) // kRenderFxExplode is always invisible even for mode kRenderNormal
      return(false);
   else if (((RenderFx == kRenderFxExplode) || (pEnemy->v.effects & EF_NODRAW))
      && (pEnemy->v.oldbuttons & IN_ATTACK) && (EnemyWeaponIsGun))
      SemiTransparent = true;
   else if ((RenderFx != kRenderFxHologram) && (RenderFx != kRenderFxExplode)
         && (RenderMode != kRenderNormal)) // kRenderFxHologram is always visible no matter what is the mode
   {
      if (RenderFx == kRenderFxGlowShell)
      {
         if ((RenderAmount <= 20.0f) && (RenderColor.x <= 20)
            && (RenderColor.y <= 20) && (RenderColor.z <= 20))
         {
            if (!(pEnemy->v.oldbuttons & IN_ATTACK) || !(EnemyWeaponIsGun))
            {
               return (FALSE);
            }
            else
            {
               SemiTransparent = true;
            }
         }
         else if ((RenderAmount <= 60.0f) && (RenderColor.x <= 60)
            && (RenderColor.y <= 60) && (RenderColor.z <= 60))
            SemiTransparent = true;
      }
      else
      {
         if (RenderAmount <= 20)
         {
            if (!(pEnemy->v.oldbuttons & IN_ATTACK) || !(EnemyWeaponIsGun))
            {
               return (FALSE);
            }
            else
            {
               SemiTransparent = true;
            }
         }
         else if (RenderAmount <= 60)
            SemiTransparent = true;
      }
   }


// KWo - 23.03.2008 - added darkness check
   LightLevel = UTIL_IlluminationOf(pEnemy);

   if ((!pBot->bUsesNVG) && (((LightLevel < 3.0f) && (g_f_cv_skycolor > 50.0f)) || ((LightLevel < 25.0f) && (g_f_cv_skycolor <= 50.0f)))
      && (!(pEnemy->v.effects & EF_DIMLIGHT)) /* && (!g_bIsOldCS15) */ && (!(pEnemy->v.oldbuttons & IN_ATTACK) || !(EnemyWeaponIsGun))) // 17.01.2011
   {
      return (FALSE);
   }
   else if (((((LightLevel < 10.0f) && (g_f_cv_skycolor > 50.0f)) || ((LightLevel < 30.0f) && (g_f_cv_skycolor <= 50.0f)))
      || ((pEnemy->v.oldbuttons & IN_ATTACK) && (EnemyWeaponIsGun)))
      && (!pBot->bUsesNVG) && (!(pEnemy->v.effects & EF_DIMLIGHT)))
   {
      SemiTransparent = true; // in this case we can notice the enemy, but not so good...
   }

   // trace a line from bot's eyes to the destination...
   TRACE_LINE (GetGunPosition (pBot->pEdict), GetGunPosition(pEnemy), dont_ignore_monsters, pBot->pEdict, &tr);
   if ((tr.flFraction <= 1.0f) && (tr.pHit == pEnemy) && (pBot->bUsesNVG || (!SemiTransparent))) // KWo - 22.03.2008
      return (true);

   TRACE_LINE (GetGunPosition (pBot->pEdict), pEnemy->v.origin, dont_ignore_monsters, pBot->pEdict, &tr);
   if ((tr.flFraction <= 1.0f) && (tr.pHit == pEnemy)) // KWo - 24.02.2008
      return (true);

   return(false);
}

bool BotFindEnemy (bot_t *pBot)
{
   // Returns true if an Enemy can be seen
   // FIXME: Bot should lock onto the best shoot position for
   // a target instead of going through all of them everytime

   static Vector vecEnd;
   static float distance;                 // KWo - 22.10.2006
   static float distance2;                // KWo - 22.10.2006
   static float FrDistLastEn;             // KWo - 08.04.2010
   static float nearestdistance;
   static edict_t *pNewEnemy;
   static edict_t *pNewEnemy2;
   static edict_t *pPlayer;
   static edict_t *pEdict;
   static bot_t *pFriendlyBot;
   static Vector vecVisible;              // KWo - 10.10.2006
   static Vector vecVisible2;             // KWo - 10.10.2006
   static Vector vecVisible3;             // KWo - 08.01.2012
   static unsigned char cHit;             // KWo - 10.10.2006
   static unsigned char cHit2;            // KWo - 10.10.2006
   static unsigned char cHit3;            // KWo - 08.01.2012
   static char szBotEnemyModelName[64];   // KWo - 12.08.2007
   static int i, j;
   static int iEnemyIndex;
   static int iShootThruFreq;
   static Vector vecRandom;               // KWo - 27.01.2008
   static Vector vecOrg;

   // We're blind and can't see anything !!
//   if (pBot->f_blind_time > gpGlobals->time) // Moved to BotSetConditions function - KWo 17.06.2018
//      return (FALSE);

   if (!FNullEnt(pBot->pBotEnemy) && (pBot->iStates & STATE_SEEINGENEMY)
      && ((g_i_botthink_index % 4) != (g_iFrameCounter % 4))) // KWo - 30.05.2010
   {
      pBot->iAimFlags |= AIM_ENEMY;
      pBot->iStates &= ~STATE_SUSPECTENEMY; // KWo - 09.02.2007
      pBot->bShootThruHeard = FALSE;        // KWo - 10.07.2008
      pBot->bShootThruSeen = FALSE;         // KWo - 10.07.2008
      return (true);
   }
   else if (FNullEnt(pBot->pBotEnemy) && (pBot->f_bot_see_enemy_time + 5.0f > gpGlobals->time)) // KWo - 21.01.2008
   {
      pBot->iAimFlags |= AIM_LASTENEMY;
      pBot->iStates |= STATE_SUSPECTENEMY;
   }

   pEdict = pBot->pEdict;
   distance = 0.0f;
   distance2 = 0.0f;
   nearestdistance = pBot->f_view_distance;
   pNewEnemy = NULL;
   pNewEnemy2 = NULL;
   vecVisible = g_vecZero;
   vecVisible2 = g_vecZero;
   vecVisible3 = g_vecZero;   // KWo - 08.01.2012
   cHit = 0;
   cHit2 = 0;
   cHit3 = 0;                 // KWo - 08.01.2012
   iEnemyIndex = -1;
   pBot->ucVisibility = 0;

   // Setup Potentially Visible Set for this Bot
   vecOrg = GetGunPosition (pEdict);

   if (pEdict->v.flags & FL_DUCKING)
      vecOrg = vecOrg + (VEC_HULL_MIN - VEC_DUCK_HULL_MIN);

//   unsigned char *pvs = ENGINE_SET_PVS ((float *) &vecOrg);

   if (!FNullEnt (pBot->pBotEnemy)) // KWo - 10.10.2006
   {
      pPlayer = pBot->pBotEnemy;
      iEnemyIndex = ENTINDEX (pPlayer) - 1; // KWo - 14.03.2010

      if ((iEnemyIndex >= 0) && (iEnemyIndex < gpGlobals->maxClients))
      {
         if (IsAlive (pPlayer) || clients[iEnemyIndex].fDeathTime >= gpGlobals->time) // KWo 15.03.2010
         {
            if ((pBot->bot_team != clients[iEnemyIndex].iTeam) || g_b_cv_ffa
               || (pBot->pBotEnemy == pEdict->v.dmg_inflictor)) // KWo - 19.01.2011
            {
               vecEnd = GetGunPosition (pPlayer);

               if (FInViewCone (&vecEnd, pEdict) && ((pPlayer->v.origin - pEdict->v.origin).Length () <=nearestdistance)) // KWo - 08.09.2009
               {
                  if (FBoxVisible (pBot, pPlayer, &vecVisible, &cHit)) // KWo - 25.03,2007
                  {
                  // the old enemy is still visible
                     pNewEnemy = pPlayer;
                     vecRandom = Vector(RANDOM_FLOAT(-BotAimTab[pBot->bot_skill / 20].fAim_X,BotAimTab[pBot->bot_skill / 20].fAim_X),
                                   RANDOM_FLOAT(-BotAimTab[pBot->bot_skill / 20].fAim_Y,BotAimTab[pBot->bot_skill / 20].fAim_Y),
                                   RANDOM_FLOAT(-BotAimTab[pBot->bot_skill / 20].fAim_Z,BotAimTab[pBot->bot_skill / 20].fAim_Z));  // KWo - 27.01.2008

                     pBot->vecVisPos = vecVisible + vecRandom; // KWo - 25.01.2008
//                   pBot->vecEnemy = vecVisible;
                     distance = (pPlayer->v.origin - pEdict->v.origin).Length (); // KWo - 22.10.2006
                     pBot->ucVisibility = cHit;
                  }
               }
            }
         }
      }
   }

   // the old enemy is no longer visible or it was farer away than 400 units
   if ((FNullEnt (pNewEnemy))
      || ((distance > 400.0f) && ((g_i_botthink_index % 4) == (g_iFrameCounter % 4)))) // KWo - 11.01.2012
   {
      // search the world for enemies...
      for (i = 0; i < gpGlobals->maxClients; i++)
      {
         if (!(clients[i].iFlags & CLIENT_USED)
             || (!(clients[i].iFlags & CLIENT_ALIVE) && (clients[i].fDeathTime <= gpGlobals->time)) // 15.03.2010
             || ((clients[i].iTeam == pBot->bot_team) && (!g_b_cv_ffa)) // KWo - 05.10.2006
             || (clients[i].pEdict == pEdict))
            continue;

         pPlayer = clients[i].pEdict;
         // Let the Engine check if this Player is potentially visible
         vecEnd = GetGunPosition (pPlayer);
         if (!FInViewCone (&vecEnd, pEdict))
            continue;

         distance2 = (pPlayer->v.origin - pEdict->v.origin).Length ();
         memset (szBotEnemyModelName, 0, sizeof(szBotEnemyModelName));
         strncpy_s (szBotEnemyModelName, sizeof(szBotEnemyModelName), (INFOKEY_VALUE (GET_INFOKEYBUFFER (pPlayer), "model")), sizeof (szBotEnemyModelName) - 1);  // KWo - 12.08.2007

         if (((distance2 >= nearestdistance)
            || ((!(FNullEnt (pNewEnemy)) && (distance > 0.0f) && ((distance2 >= 0.9f * distance)
                                                                || (pBot->f_bot_see_new_enemy_time + 0.5f >= gpGlobals->time)))))
            && ((!(g_iMapType & MAP_AS)) || (strcmp ("vip", szBotEnemyModelName) != 0)))     // KWo - 05.09.2009
            continue;

         if (!FBoxVisible (pBot, pPlayer, &vecVisible3, &cHit3))  // KWo - 08.01.2012
            continue;
//         if (!ENGINE_CHECK_VISIBILITY (pPlayer, pvs))
//            continue;


         if (((distance2 < nearestdistance)
            && ((FNullEnt (pNewEnemy) || (distance == 0.0f)
               || ((distance2 < 0.9f * distance) && (pBot->f_bot_see_new_enemy_time + 0.5f < gpGlobals->time)))))
            || ((g_iMapType & MAP_AS) && (strcmp ("vip", szBotEnemyModelName) == 0))) // KWo - 29.04.2008
         {
            nearestdistance = distance2;
            pNewEnemy2 = pPlayer;
            cHit2 = cHit3;                                     // KWo - 08.01.2012
            vecVisible2 = vecVisible3;                         // KWo - 08.01.2012
            pBot->f_bot_see_new_enemy_time = gpGlobals->time;  // KWo - 29.04.2008
            pBot->fChangeAimDirectionTime = gpGlobals->time + 1.0f;
            iEnemyIndex = i;  // KWo - 14.03.2010
            // On Assassination Maps target VIP first !
            if ((g_iMapType & MAP_AS)
                && (strcmp ("vip", szBotEnemyModelName) == 0)) // Is VIP ? -  KWo - 12.08.2007
               break;
         }
      }
      if ((FNullEnt (pNewEnemy) || (distance > 400.0f)) && (!FNullEnt (pNewEnemy2)))
      {
         pNewEnemy = pNewEnemy2;
         pBot->ucVisibility = cHit2;
         vecRandom = Vector(RANDOM_FLOAT(-BotAimTab[pBot->bot_skill / 20].fAim_X,BotAimTab[pBot->bot_skill / 20].fAim_X),
                             RANDOM_FLOAT(-BotAimTab[pBot->bot_skill / 20].fAim_Y,BotAimTab[pBot->bot_skill / 20].fAim_Y),
                             RANDOM_FLOAT(-BotAimTab[pBot->bot_skill / 20].fAim_Z,BotAimTab[pBot->bot_skill / 20].fAim_Z));  // KWo - 27.01.2008
         pBot->vecLastEnemyOrigin = vecVisible2 + vecRandom;   // KWo - 27.01.2008
         pBot->vecEnemy = vecVisible2 + vecRandom;             // KWo - 27.01.2008
         pBot->vecVisPos = vecVisible2 + vecRandom;            // KWo - 27.01.2008
         vecVisible = vecVisible2;
      }
/*
      if (FNullEnt (pNewEnemy))    // KWo - 05.09.2009
         pBot->fEnemyUpdateTime = gpGlobals->time + 0.1f;
*/
/*
      else if (!(FNullEnt (pNewEnemy)) && (!FNullEnt (pNewEnemy2)) && (distance > nearestdistance + 50.0f))
      {
         if (GetShootingConeDeviation (pNewEnemy2, &pEdict->v.origin) >= 0.8f)
         {
            pNewEnemy = pNewEnemy2;
            pBot->ucVisibility = cHit2;
            pBot->vecEnemy = vecVisible2;
            vecVisible = vecVisible2;
         }
      }
*/
   }

   if (pNewEnemy)
   {
      pBot->fEnemyUpdateTime = gpGlobals->time + 0.02f; // KWo - 05.09.2009 (not checked currently in the code)
      pBot->fLastSeenEnOrgUpdateTime = gpGlobals->time + 1.0f; // KWo - 13.07.2008
      g_bBotsCanPause = TRUE;
      pBot->iAimFlags |= AIM_ENEMY;
//      pBot->iAimFlags |= AIM_LASTENEMY;            // KWo - 27.01.2008
      pBot->iStates |= STATE_SEEINGENEMY;          // KWo - 10.07.2008
      pBot->iStates &= ~STATE_SUSPECTENEMY;        // KWo - 09.02.2007
      pBot->bShootThruHeard = FALSE;               // KWo - 10.07.2008
      pBot->bShootThruSeen = FALSE;                // KWo - 10.07.2008

      // Now alarm all Teammates who see this Bot &
      // don't have an actual Enemy of the Bots Enemy
      // Should simulate human players seeing a Teammate firing
      // In this case he should take our enemy as his own suspected enemy...

      if ((pEdict->v.oldbuttons & IN_ATTACK) && (!g_b_cv_ffa)) // KWo - 13.07.2008
      {
         for (j = 0; j < gpGlobals->maxClients; j++)
         {
            if (!(bots[j].is_used)
                || (bots[j].bDead)
                || (bots[j].bot_team != pBot->bot_team)
                || (bots[j].pEdict == pEdict))
               continue;

            pFriendlyBot = &bots[j];
//            pFriendlyBot = UTIL_GetBotPointer (clients[j].pEdict);

            if (pFriendlyBot != NULL)
            {
               FrDistLastEn = 9999.0f;
               if (pFriendlyBot->vecLastEnemyOrigin != g_vecZero)  // KWo - 08.04.2010
                  FrDistLastEn = (pFriendlyBot->pEdict->v.origin - pFriendlyBot->vecLastEnemyOrigin).Length();

               if ((pFriendlyBot->f_bot_see_enemy_time + 4.0f < gpGlobals->time)
                   && (((pFriendlyBot->f_heard_sound_time + 4.0f < gpGlobals->time)
                   && (FNullEnt (pFriendlyBot->pLastEnemy)))
                      || (FrDistLastEn < (pFriendlyBot->pEdict->v.origin - pBot->vecLastEnemyOrigin).Length()))
                   && FNullEnt(pFriendlyBot->pBotEnemy))// KWo - 08.04.2010
               {
                  if (FInViewCone (&pEdict->v.origin, pFriendlyBot->pEdict))
                  {
                     if (FVisible (pEdict->v.origin, pFriendlyBot->pEdict))
                     {
                        pFriendlyBot->pLastEnemy = pNewEnemy;
                        pFriendlyBot->vecLastEnemyOrigin = pBot->vecLastEnemyOrigin;
                        pFriendlyBot->f_heard_sound_time = gpGlobals->time;
                        pFriendlyBot->iStates |= STATE_SUSPECTENEMY;
                        pFriendlyBot->iStates |= STATE_HEARINGENEMY;
                        pFriendlyBot->iAimFlags |= AIM_LASTENEMY;
                        pFriendlyBot->bShootThruHeard = FALSE;
                        pFriendlyBot->bShootThruSeen = FALSE;
                     }
                  }
               }
            }
         }
      }

      // keep track of when we last saw an enemy
      if (pNewEnemy == pBot->pBotEnemy)
      {
         // Zero out reaction time
         pBot->f_actual_reaction_time = 0.0f;
         pBot->pLastEnemy = pNewEnemy;
//         pBot->vecLastEnemyOrigin = vecVisible; // KWo - 10.10.2006
//         pBot->vecVisPos = vecVisible; // KWo - 25.01.2008 // KWo - 08.01.2012
         pBot->pBotUser = NULL;  // don't follow user when enemy found
         pBot->f_bot_see_enemy_time = gpGlobals->time;
         return (TRUE);
      }
      else
      {
         if ((pBot->f_bot_see_enemy_time + 4.0f < gpGlobals->time)
             && ((pEdict->v.weapons & (1 << CS_WEAPON_C4))
                 || BotHasHostage (pBot) || !FNullEnt (pBot->pBotUser))
             && (!g_b_cv_ffa) && (g_b_cv_radio) && (RANDOM_LONG (1, 100) < 20)
                 && (((pBot->bot_team == TEAM_CS_COUNTER) && (g_iAliveCTs > 1))
                  || ((pBot->bot_team == TEAM_CS_TERRORIST) && (g_iAliveTs > 1)))) // KWo - 06.03.2010
            BotPlayRadioMessage (pBot, RADIO_ENEMYSPOTTED);

         pBot->f_enemy_surprise_time = gpGlobals->time + pBot->f_actual_reaction_time;
         pBot->f_bot_see_enemy_time = gpGlobals->time;
//         UTIL_ServerPrint("[DEBUG] Bot %s got surprised by a new enemy.\n", pBot->name);

         // Zero out reaction time
         pBot->f_actual_reaction_time = 0.0f;
         pBot->pBotEnemy = pNewEnemy;
         pBot->pLastEnemy = pNewEnemy;

         vecRandom = Vector(RANDOM_FLOAT(-BotAimTab[pBot->bot_skill / 20].fAim_X,BotAimTab[pBot->bot_skill / 20].fAim_X),
                             RANDOM_FLOAT(-BotAimTab[pBot->bot_skill / 20].fAim_Y,BotAimTab[pBot->bot_skill / 20].fAim_Y),
                             RANDOM_FLOAT(-BotAimTab[pBot->bot_skill / 20].fAim_Z,BotAimTab[pBot->bot_skill / 20].fAim_Z));  // KWo - 27.01.2008
         pBot->vecLastEnemyOrigin = vecVisible + vecRandom; // KWo - 27.01.2008
         pBot->vecEnemy = vecVisible + vecRandom;           // KWo - 27.01.2008
         pBot->vecVisPos = vecVisible + vecRandom;          // KWo - 27.01.2008
         pBot->fEnemyReachableTimer = 0.0f;
         pBot->pBotUser = NULL;  // don't follow user when enemy found

         return (TRUE);
      }
   }
   else if (!FNullEnt (pBot->pBotEnemy))
   {
      pNewEnemy = pBot->pBotEnemy;
      pBot->pLastEnemy = pNewEnemy;
      iEnemyIndex = ENTINDEX (pNewEnemy) - 1; // KWo - 14.03.2010
      if ((iEnemyIndex >= 0) && (iEnemyIndex < gpGlobals->maxClients))
      {
         if ((!IsAlive (pNewEnemy) && (clients[iEnemyIndex].fDeathTime <= gpGlobals->time))
            || (IsAlive (pNewEnemy) && (clients[iEnemyIndex].fDeathTime > gpGlobals->time))
            || ((pEdict->v.dmgtime + 1.5f <= gpGlobals->time) && (pEdict->v.dmg_inflictor == pBot->pBotEnemy)
                && (clients[iEnemyIndex].iTeam == pBot->bot_team) && !(g_b_cv_ffa))) // KWo - 18.01.2011
         {
            pBot->iStates &= ~STATE_SEEINGENEMY;          // KWo - 10.07.2008
            pBot->pBotEnemy = NULL;
            return (FALSE);
         }
      }

      // If no Enemy visible check if last one shootable thru Wall
// KWo - 17.06.2018 - uncommented back the check below...
      if (pBot->fShootThruSeenCheckTime < gpGlobals->time)  // KWo - 23.03.2008
      {
         iShootThruFreq = BotAimTab[pBot->bot_skill / 20].iSeenShootThruProb;
         if ((g_b_cv_shootthruwalls) && (RANDOM_LONG (1, 100) <= iShootThruFreq)
            && WeaponShootsThru (pBot->current_weapon.iId)
            && (pBot->f_bot_see_enemy_time + 0.6f > gpGlobals->time))    // KWo - 08.01.2012
         {
            pBot->fShootThruSeenCheckTime = gpGlobals->time + 0.2f;      // KWo - 23.03.2008
            if (vecVisible == g_vecZero)   // KWo - 17.06.2018
            {
               if (IsShootableThruObstacle (pEdict, pNewEnemy->v.origin))  // KWo - 23.03.2008
                  pBot->bShootThruSeen = TRUE;
               else
                  pBot->bShootThruSeen = FALSE;
            }
            else
               pBot->bShootThruSeen = FALSE;
         }
         else
         {
            pBot->bShootThruSeen = FALSE;
         }
      }

      if (pBot->bShootThruSeen)  // KWo - 05.05.2007
      {
         pBot->iStates |= STATE_SUSPECTENEMY;  // KWo - 02.02.2007
         pBot->iAimFlags |= AIM_LASTENEMY;
         pBot->iAimFlags |= AIM_ENEMY;          // KWo - 10.07.2008 - very important for work this function!!!
         pBot->pLastEnemy = pNewEnemy;
         return (TRUE);
      }
      return (FALSE);
   }
   return (FALSE);
}


Vector BotBodyTarget (edict_t *pBotEnemy, bot_t *pBot)
{
   // Returns the aiming Vector for an Enemy

   static int iWeapId;
   static int iEnemyIndex;                   // KWo - 18.01.2011
   static Vector target;
   static unsigned char ucVis;
   static edict_t *pEdict;
   static float fDistance;
   static Vector vecVel;
   static Vector vecRandom;
   static bool bBotUsesSniper;               // KWo - 14.10.2006
   static bool bBotUsesPistol;               // KWo - 23.10.2006
   static bool bBotUsesAssaultSniper;        // KWo - 09.02.2008
   static bool bBotUsesSubmachine;           // KWo - 09.02.2008
   static bool bBotUsesRifle;                // KWo - 09.02.2008
   static bool bEnemyTeamnate;               // KWo - 18.01.2011
   static float fRandomFactor;

   if (pBot->f_blind_time > gpGlobals->time) // KWo - 04.10.2009
      return (pBot->vecLastEnemyOrigin);

   if (pBot->fEnemyOriginUpdateTime < gpGlobals->time)  // KWo - 04.10.2009
   {
      vecRandom = Vector(RANDOM_FLOAT(-BotAimTab[pBot->bot_skill / 20].fAim_X,BotAimTab[pBot->bot_skill / 20].fAim_X),
                             RANDOM_FLOAT(-BotAimTab[pBot->bot_skill / 20].fAim_Y,BotAimTab[pBot->bot_skill / 20].fAim_Y),
                             RANDOM_FLOAT(-BotAimTab[pBot->bot_skill / 20].fAim_Z,BotAimTab[pBot->bot_skill / 20].fAim_Z));
      pBot->vecEnemyRandomOffset = vecRandom;
      pBot->fEnemyOriginUpdateTime = gpGlobals->time + 0.5f;
   }
   else
      vecRandom = pBot->vecEnemyRandomOffset;

   iWeapId = pBot->current_weapon.iId;
   ucVis = pBot->ucVisibility;
   pEdict = pBot->pEdict;
   fDistance = (pBotEnemy->v.origin - pEdict->v.origin).Length ();

   // Compensate both the enemy's and the bot's own velocity

   vecVel = 1.0f * (pBot->fTimeFrameInterval /* - gpGlobals->frametime */) * pBotEnemy->v.velocity - 1.0f * (pBot->fTimeFrameInterval) * pBot->pEdict->v.velocity;

// bots now don't update target all time, but every 0.5f sec (for skill 100 it's 0.0f)
// so randomize target will not affect bot crosshair all frames - it's more realistic than in PB2.5f,
// but still we didn't find better way to make stupid bots aiming worse... - KWo - 11.03.2006

   iEnemyIndex = ENTINDEX(pBotEnemy) - 1;                      // KWo - 18.01.2011
   bEnemyTeamnate = FALSE;                                     // KWo - 18.01.2011
   if ((iEnemyIndex >=0) && (iEnemyIndex < gpGlobals->time))   // KWo - 18.01.2011
   {
      if ((pBot->bot_team == clients[iEnemyIndex].iTeam) && (!g_b_cv_ffa))
         bEnemyTeamnate = TRUE;
   }

   bBotUsesSniper = BotUsesSniper(pBot);                    // KWo - 14.10.2006
   bBotUsesPistol = WeaponIsPistol(iWeapId);                // KWo - 23.10.2006
   bBotUsesAssaultSniper = WeaponIsAssualtSniper(iWeapId);  // KWo - 09.02.2008
   bBotUsesSubmachine = WeaponIsSubmachineGun(iWeapId);     // KWo - 09.02.2008
   bBotUsesRifle = WeaponIsRifle(iWeapId);                  // KWo - 09.02.2008

   if (!bBotUsesSniper)
   {
      vecVel = vecVel * pBot->fTimeFrameInterval;
   // No Up/Down Compensation
      vecVel.z = 0.0f;
   }
   else
      vecVel = g_vecZero;

   // Waist Visible ?
   if (ucVis & WAIST_VISIBLE)
   {
      // Use Waist as Target for big distances
      if (((fDistance > 1200.0f + 4.0f * pBot->bot_skill) && (!bBotUsesSniper) && (!bBotUsesAssaultSniper))
         || (bEnemyTeamnate))                               // KWo - 18.01.2011
         ucVis &= ~HEAD_VISIBLE;
      if (((bBotUsesRifle && !bBotUsesAssaultSniper) || (iWeapId == CS_WEAPON_M3)
         || (iWeapId == CS_WEAPON_XM1014) || (iWeapId == CS_WEAPON_M249)) && (fDistance > 1200.0f + 4.0f * pBot->bot_skill)) // KWo - 09.02.2008
         ucVis &= ~HEAD_VISIBLE;
   }

   // If we only suspect an Enemy behind a Wall take the worst Skill
   if ((((pBot->iStates & STATE_SUSPECTENEMY) && !(pBot->iStates & STATE_SEEINGENEMY) && (pBot->bShootThruSeen))
      || ((pBot->iStates & STATE_SEEINGENEMY) && (pBot->f_bot_see_enemy_time + 5.0f > gpGlobals->time)
                 && (pBot->f_bot_see_enemy_time < gpGlobals->time)))
      && !FNullEnt(pBotEnemy)) // KWo - 08.01.2012
   {
      if ( ((fDistance < 2000.0f ) || (BotUsesSniper(pBot)))
      /* && ( (pBot->fLastHeardEnOrgUpdateTime < gpGlobals->time) */
         && (pBot->fLastSeenEnOrgUpdateTime < gpGlobals->time)
         && (pBot->f_bot_see_enemy_time + 5.0f > gpGlobals->time)) // KWo - 14.10.2011
      {
         fRandomFactor = 2.0f * (gpGlobals->time - pBot->f_bot_see_enemy_time + 1.0f); // KWo - 11.07.2008
         if (fRandomFactor < 2.0f) // KWo - 11.07.2008
            fRandomFactor = 2.0f;
         else if (fRandomFactor > 8.0f)
            fRandomFactor = 8.0f;

         target = pBot->vecLastEnemyOrigin; // KWo - 13.08.2008
         if ((pBot->fLastHeardEnOrgUpdateTime < gpGlobals->time)
            && (pBot->fLastSeenEnOrgUpdateTime < gpGlobals->time)
            && (pBot->f_bot_see_enemy_time + 1.0f < gpGlobals->time)) // KWo - 08.01.2012
         {
//            target = pBotEnemy->v.origin;
//            target.x = target.x + RANDOM_FLOAT (pBotEnemy->v.mins.x * fRandomFactor, pBotEnemy->v.maxs.x * fRandomFactor);
//            target.y = target.y + RANDOM_FLOAT (pBotEnemy->v.mins.y * fRandomFactor, pBotEnemy->v.maxs.y * fRandomFactor);
            pBot->fLastSeenEnOrgUpdateTime = gpGlobals->time + 1.0f;
         }

         pBot->vecLastEnemyOrigin = target;
         pBot->vecEnemy = target;                                 // KWo - 11.07.2008
         pBot->vecVisPos = pBot->vecLastEnemyOrigin;              // KWo - 10.07.2008
         pBot->ucVisibility = WAIST_VISIBLE;                      // KWo - 10.07.2008
         vecRandom = g_vecZero;                                   // KWo - 12.09.2011
//         if (pHostEdict)
//         {
//            if ((pHostEdict->v.origin - pEdict->v.origin).Length() < 20.0f)
// Vector start, Vector end, int life, int width, int noise, int red, int green, int blue, int brightness, int speed
//               UTIL_DrawBeam(GetGunPosition(pEdict), target, 10, 50, 0, 0, 0, 255, 255, 0);
//         }
      }
      else
      {
         target = pBot->vecLastEnemyOrigin;
         vecRandom = g_vecZero;                                   // KWo - 12.09.2011
      }
   }
   else if ((pBot->iStates & STATE_SEEINGENEMY) && !FNullEnt(pBotEnemy))
   {
      if ((ucVis & HEAD_VISIBLE) && (ucVis & WAIST_VISIBLE))
      {
         if (RANDOM_LONG (1, 100) <= BotAimTab[pBot->bot_skill / 20].iHeadShot_Frequency)
         {
            target = GetGunPosition (pBotEnemy);  // aim for the head

            // the idea taken rom YapB
            if ((fDistance < 3000.0f) && (fDistance > 2 * MIN_BURST_DISTANCE)) // KWo - 09.04.2010
            {
               if (bBotUsesSniper)
                  target.z += 3.5f; // 3.5
               else if (bBotUsesAssaultSniper)
                  target.z += 4.5f; // 2.5
               else if (bBotUsesPistol)
                  target.z += 6.5f; // 4.5
               else if (bBotUsesSubmachine)
                  target.z += 5.5f; // 3.0
               else if (bBotUsesRifle)
                  target.z += 5.5f; // 3.0
               else if (iWeapId == CS_WEAPON_M249)
                  target.z += 2.5f;
               else if ((iWeapId == CS_WEAPON_XM1014) || (iWeapId == CS_WEAPON_M3))
                  target.z += 10.5f; // 1.5
            }
            else if ((fDistance > MIN_BURST_DISTANCE) && (fDistance <= 2 * MIN_BURST_DISTANCE))   // KWo - 09.04.2010
            {
               if (bBotUsesSniper)
                  target.z += 3.5f;
               else if (bBotUsesAssaultSniper)
                  target.z += 3.5f; // - 1.0
               else if (bBotUsesPistol)
                  target.z += 6.5f; // 4.5
               else if (bBotUsesSubmachine)
                  target.z += 3.5f; // 1.0
               else if (bBotUsesRifle)
                  target.z += 1.0f; // -1.0
               else if (iWeapId == CS_WEAPON_M249)
                  target.z -= 2.0f; // -2.0
               else if ((iWeapId == CS_WEAPON_XM1014) || (iWeapId == CS_WEAPON_M3))
                  target.z += 10.0f; // 1.0
            }
            else if (fDistance < MIN_BURST_DISTANCE)   // KWo - 09.04.2010
            {
               if (bBotUsesSniper)
                  target.z += 4.5f;
               else if (bBotUsesAssaultSniper)
                  target.z -= 5.0f;
               else if (bBotUsesPistol)
                  target.z += 4.5f;
               else if (bBotUsesSubmachine)
                  target.z -= 4.5f;
               else if (bBotUsesRifle)
                  target.z -= 4.5f;
               else if (iWeapId == CS_WEAPON_M249)
                  target.z -= 6.0f;
               else if ((iWeapId == CS_WEAPON_XM1014) || (iWeapId == CS_WEAPON_M3))
                  target.z -= 5.0f;
            }

            target.z -= 0.5f * (100 - pBot->bot_skill); // 16.0f * fSkillMin; KWo - 11.03.2006
         }
         else
         {
            target = pBotEnemy->v.origin + Vector(0.0f, 0.0f, 3.0f);  // aim for the chest
         }
      }
      else if (ucVis & HEAD_VISIBLE)
      {
         target = GetGunPosition (pBotEnemy);  // aim for the head
         // the idea taken rom YapB
         if (true) // just to have the same identation like above...
         {
            if ((fDistance < 3000.0f) && (fDistance > 2 * MIN_BURST_DISTANCE)) // KWo - 09.04.2010
            {
               if (bBotUsesSniper)
                  target.z += 5.5f; // 3.5
               else if (bBotUsesAssaultSniper)
                  target.z += 4.5f; // 2.5
               else if (bBotUsesPistol)
                  target.z += 6.5f; // 4.5
               else if (bBotUsesSubmachine)
                  target.z += 5.5f; // 3.0
               else if (bBotUsesRifle)
                  target.z += 5.5f; // 3.0
               else if (iWeapId == CS_WEAPON_M249)
                  target.z += 2.5f;
               else if ((iWeapId == CS_WEAPON_XM1014) || (iWeapId == CS_WEAPON_M3))
                  target.z += 10.5f; // 1.5
            }
            else if ((fDistance > MIN_BURST_DISTANCE) && (fDistance <= 2 * MIN_BURST_DISTANCE))   // KWo - 09.04.2010
            {
               if (bBotUsesSniper)
                  target.z += 3.5f;
               else if (bBotUsesAssaultSniper)
                  target.z += 3.5f; // - 1.0
               else if (bBotUsesPistol)
                  target.z += 6.5f; // 4.5
               else if (bBotUsesSubmachine)
                  target.z += 3.5f; // 1.0
               else if (bBotUsesRifle)
                  target.z += 1.0f; // -1.0
               else if (iWeapId == CS_WEAPON_M249)
                  target.z -= 2.0f; // -2.0
               else if ((iWeapId == CS_WEAPON_XM1014) || (iWeapId == CS_WEAPON_M3))
                  target.z += 10.0f; // 1.0
            }
            else if (fDistance < MIN_BURST_DISTANCE)   // KWo - 09.04.2010
            {
               if (bBotUsesSniper)
                  target.z += 3.5f;
               else if (bBotUsesAssaultSniper)
                  target.z -= 5.0f;
               else if (bBotUsesPistol)
                  target.z += 4.5f;
               else if (bBotUsesSubmachine)
                  target.z -= 4.5f;
               else if (bBotUsesRifle)
                  target.z -= 4.5f;
               else if (iWeapId == CS_WEAPON_M249)
                  target.z -= 6.0f;
               else if ((iWeapId == CS_WEAPON_XM1014) || (iWeapId == CS_WEAPON_M3))
                  target.z -= 5.0f;
            }
            target.z -= 0.5f * (100 - pBot->bot_skill);  // 16.0f * fSkillMin; KWo - 11.03.2006
         }
      }
      else if (ucVis & WAIST_VISIBLE)
      {
         target = pBotEnemy->v.origin + Vector(0.0, 0.0, 3.0f);  // aim for the chest
      }
      else if (ucVis & CUSTOM_VISIBLE)  // aim for custom part
         target = pBot->vecVisPos; // KWo - 25.01.2008
      // Something went wrong - use last enemy origin
      else
      {
//         assert (pBot == NULL);
         target = pBot->vecLastEnemyOrigin;
         vecRandom = g_vecZero;                                   // KWo - 12.09.2011
      }
   }
   else // KWo - 25.01.2008
   {
      target = pBot->vecLastEnemyOrigin;
      vecRandom = g_vecZero;                                   // KWo - 12.09.2011
   }

   if (g_i_cv_aim_type != 5)
      target = target + vecVel + vecRandom; // KWo - 11.03.2006
   else
      target = target + vecRandom; // KWo - 11.03.2006

   pBot->vecEnemy = target;
   pBot->vecLastEnemyOrigin = target; // KWo - 25.01.2008
   return (target);
}


bool WeaponShootsThru (int iId)
{
   // Returns if Weapon can pierce thru a wall

   int i = 0;

   while (cs_weapon_select[i].iId)
   {
      if (cs_weapon_select[i].iId == iId)
         return (cs_weapon_select[i].bShootsThru);

      i++;
   }

   return (FALSE);
}


bool WeaponIsSniper (int iId)
{
   if ((iId == CS_WEAPON_AWP) || (iId == CS_WEAPON_G3SG1)
       || (iId == CS_WEAPON_SCOUT) || (iId == CS_WEAPON_SG550))
      return (TRUE);

   return (FALSE);
}


bool WeaponIsRifle (int iId)
{
   if ((iId == CS_WEAPON_AK47) || (iId == CS_WEAPON_M4A1)
       || (iId == CS_WEAPON_GALIL) || (iId == CS_WEAPON_FAMAS)
       || WeaponIsAssualtSniper (iId))
      return (TRUE);

   return (FALSE);
}


bool WeaponIsAssualtSniper (int iId)
{
   if ((iId == CS_WEAPON_AUG) || (iId == CS_WEAPON_SG552))
      return (TRUE);

   return (FALSE);
}

bool WeaponIsSubmachineGun (int iId)
{
   return ((iId == CS_WEAPON_MP5NAVY) || (iId == CS_WEAPON_TMP)
      || (iId == CS_WEAPON_P90) || (iId == CS_WEAPON_MAC10)
      || (iId == CS_WEAPON_UMP45));
}

bool WeaponIsPrimaryGun (int iId)
{
   return ((WeaponIsSubmachineGun (iId)) || (WeaponIsSniper (iId))
           || (WeaponIsRifle (iId)) || (iId == CS_WEAPON_M3)
           || (iId == CS_WEAPON_XM1014) || (iId == CS_WEAPON_M249));
}

bool WeaponIsPistol (int iId)
{
   if ((iId == CS_WEAPON_USP) || (iId == CS_WEAPON_GLOCK18)
       || (iId == CS_WEAPON_DEAGLE) || (iId == CS_WEAPON_P228)
       || (iId == CS_WEAPON_ELITE) || (iId == CS_WEAPON_FIVESEVEN))
      return (TRUE);

   return (FALSE);
}

bool WeaponIsNade (int iId) // KWo - 15.01.2007
{
   return ((iId == CS_WEAPON_HEGRENADE) || (iId == CS_WEAPON_SMOKEGRENADE)
           || (iId == CS_WEAPON_FLASHBANG));
}


bool BotUsesRifle (bot_t *pBot)
{
   if (WeaponIsRifle (pBot->current_weapon.iId)
       || WeaponIsSniper (pBot->current_weapon.iId))
      return (TRUE);

   return (FALSE);
}


bool BotUsesSniper (bot_t *pBot)
{
   if (WeaponIsSniper (pBot->current_weapon.iId))
      return (TRUE);

   return (FALSE);
}

bool BotUsesSubmachineGun (bot_t *pBot) // KWo - 20.10.2006 (from yapb)
{
   return ((pBot->current_weapon.iId == CS_WEAPON_MP5NAVY) || (pBot->current_weapon.iId == CS_WEAPON_TMP)
      || (pBot->current_weapon.iId == CS_WEAPON_P90) || (pBot->current_weapon.iId == CS_WEAPON_MAC10)
      || (pBot->current_weapon.iId == CS_WEAPON_UMP45));
}


bool BotHasPrimaryWeapon (bot_t *pBot)
{
   bot_weapon_select_t *pSelect = &cs_weapon_select[7];
   int iWeapons = pBot->pEdict->v.weapons;

   // loop through all the weapons until terminator is found...
   while (pSelect->iId)
   {
      // is the bot carrying this weapon?
      if (iWeapons & (1 << pSelect->iId))
         return (TRUE);

      pSelect++;
   }

   return (FALSE);
}

bool BotHasSecondaryWeapon (bot_t *pBot) // KWo - 18.01.2011
{
   bot_weapon_select_t *pSelect = &cs_weapon_select[1];
   int iWeapons = pBot->pEdict->v.weapons;
   int iCount = 1;

   // loop through all the weapons until terminator is found...
   while ((pSelect->iId) && (iCount < 7))
   {
      // is the bot carrying this weapon?
      if (iWeapons & (1 << pSelect->iId))
         return (TRUE);
      iCount++;
      pSelect++;
   }

   return (FALSE);
}

bool BotHasSniperWeapon (bot_t *pBot) // KWo - 27.02.2007
{
   int iWeapons = pBot->pEdict->v.weapons;

   if ((iWeapons & (1 << CS_WEAPON_SG550)) || (iWeapons & (1 << CS_WEAPON_G3SG1))
         || (iWeapons & (1 << CS_WEAPON_AWP)) || (iWeapons & (1 << CS_WEAPON_SCOUT)))
      return (TRUE);

   return (FALSE);
}

bool BotHasRifleWeapon (bot_t *pBot) // KWo - 27.02.2007
{
   int iWeapons = pBot->pEdict->v.weapons;

   if ((iWeapons & (1 << CS_WEAPON_AK47)) || (iWeapons & (1 << CS_WEAPON_SG552))
         || (iWeapons & (1 << CS_WEAPON_M4A1)) || (iWeapons & (1 << CS_WEAPON_GALIL))
         || (iWeapons & (1 << CS_WEAPON_FAMAS)) || (iWeapons & (1 << CS_WEAPON_AUG)))
      return (TRUE);

   return (FALSE);
}

bool BotHasSubmachineGun (bot_t *pBot) // KWo - 19.05.2010
{
   int iWeapons = pBot->pEdict->v.weapons;

   if ((iWeapons & (1 << CS_WEAPON_MP5NAVY)) || (iWeapons & (1 << CS_WEAPON_TMP))
         || (iWeapons & (1 << CS_WEAPON_P90)) || (iWeapons & (1 << CS_WEAPON_MAC10))
         || (iWeapons & (1 << CS_WEAPON_UMP45)))
      return (TRUE);

   return (FALSE);
}

bool BotHasCampWeapon (bot_t *pBot) // KWo - 19.05.2010
{
   if (BotHasSubmachineGun (pBot) || BotHasRifleWeapon (pBot) || BotHasSniperWeapon (pBot))
      return (TRUE);

   return (FALSE);
}

bool BotHasShield (bot_t *pBot)
{
   // code courtesy of Wei Mingzhi. He stuffed all the bot stuff into a CBaseBot class, but
   // I'm a lazy bugger so I won't do the same.

   return (strncmp (STRING (pBot->pEdict->v.viewmodel), "models/shield/v_shield_", 23) == 0);
}


bool BotHasShieldDrawn (bot_t *pBot)
{
   // code courtesy of Wei Mingzhi. BTW check out his YaPB !

   if (!BotHasShield (pBot))
      return (FALSE);
// KWo - 15.08.2007 - rewritten
   if (WeaponIsPistol(pBot->current_weapon.iId))
      return ((pBot->pEdict->v.weaponanim == 6) || (pBot->pEdict->v.weaponanim == 7));
   else if (pBot->current_weapon.iId == CS_WEAPON_KNIFE)
      return (pBot->pEdict->v.weaponanim == 5);
   else if (WeaponIsNade(pBot->current_weapon.iId))
      return (pBot->pEdict->v.weaponanim == 4);
   else
      return (false);
}

bool PlayerHasShieldDrawn (edict_t *pPlayer) // KWo - 15.08.2007
{
   if (FNullEnt(pPlayer))
      return (false);
   if (!(pPlayer->v.flags & FL_CLIENT))
      return (false);

   if (strncmp (STRING (pPlayer->v.viewmodel), "models/shield/v_shield_", 23) != 0)
      return (false);

   int iPlayerIndex = ENTINDEX (pPlayer) - 1;
   int iWeapon = clients[iPlayerIndex].iCurrentWeaponId;

   if (WeaponIsPistol(iWeapon))
      return ((pPlayer->v.weaponanim == 6) || (pPlayer->v.weaponanim == 7));
   else if (iWeapon == CS_WEAPON_KNIFE)
      return (pPlayer->v.weaponanim == 5);
   else if (WeaponIsNade(iWeapon))
      return (pPlayer->v.weaponanim == 4);
   else
      return (false);
}

bool FireHurtsFriend (bot_t *pBot, float fDistance)
{
   edict_t *pEdict = pBot->pEdict;
   edict_t *pPlayer;
   int i;
   TraceResult tr;
   float fTeamnateDist;  // KWo - 28.10.2006
   float fSqDist;   // KWo - 18.11.2006
   float fConeMin;  // KWo - 28.10.2006
   float fConeDev;
   Vector vecDirection = pEdict->v.v_angle;
   MAKE_VECTORS (vecDirection);
   vecDirection = vecDirection.Normalize ();

   // check if friendlyfire is off.. now bots shoot through each other or ffa is on!
   if ((!g_b_cv_FriendlyFire) || (g_b_cv_ffa)) // KWo - 05.10.2006
      return (FALSE);

//   TRACE_HULL (GetGunPosition (pEdict), GetGunPosition (pEdict) + vecDirection * fDistance, dont_ignore_monsters, head_hull, pEdict, &tr);
   TRACE_LINE (GetGunPosition (pEdict), GetGunPosition (pEdict) + vecDirection * fDistance * 1.1f, dont_ignore_monsters, pEdict, &tr); // KWo - 13.10.2006

   if (!FNullEnt (tr.pHit))
   {
      i = ENTINDEX (tr.pHit) - 1;
      if ((i >= 0) && (i < gpGlobals->maxClients))  // KWo - 13.03.2006
      {
         if ((clients[i].iTeam == pBot->bot_team)
            && ((pEdict->v.dmg_inflictor != clients[i].pEdict) || (!g_b_cv_ffrev) || ((pEdict->v.dmgtime + 1.5f * pBot->fAgressionLevel) <= gpGlobals->time))
            && ((clients[i].iFlags & CLIENT_ALIVE) || (clients[i].fDeathTime >= gpGlobals->time)))  // KWo - 28.08.2018
         {
            if (g_b_DebugCombat)
               ALERT (at_logged,"[DEBUG] FireHurtsFriend - Bot %s cannot fire because it may hurt its friend %s (1).\n", pBot->name, STRING (clients[i].pEdict->v.netname));

            return (TRUE);
         }
      }
   }

   // search the world for players...
   for (i = 0; i < gpGlobals->maxClients; i++)
   {
      if (!(clients[i].iFlags & CLIENT_USED)
          || !(clients[i].iFlags & CLIENT_ALIVE)
          || (clients[i].iTeam != pBot->bot_team)
          || ((pEdict->v.dmg_inflictor == clients[i].pEdict) && (g_b_cv_ffrev)) // KWo - 28.08.2018
          || (clients[i].pEdict == pEdict))
         continue;

      pPlayer = clients[i].pEdict;

      fTeamnateDist = (pPlayer->v.origin - pEdict->v.origin).Length ();  // KWo - 28.10.2006
      fSqDist = sqrtf(1089.0f + fTeamnateDist * fTeamnateDist); // KWo - 12.01.2007
      fConeMin = (fTeamnateDist * fTeamnateDist) / (fSqDist * fSqDist); // KWo - 18.11.2006
//      fConeMin = 0.985f;
      fConeDev = GetShootingConeDeviation (pEdict, &pPlayer->v.origin);
      if (fConeDev > fConeMin)  // KWo - 28.10.2006
      {
         if (fTeamnateDist <= fDistance)
         {

            if (g_b_DebugCombat)
               ALERT (at_logged,"[DEBUG] FireHurtsFriend - Bot %s cannot fire because it may hurt its friend %s (2), fConeDev = %f, fConeMin = %f, fTeamnateDist = %f, fDistance = %f.\n",
                  pBot->name, STRING(pPlayer->v.netname), fConeDev, fConeMin, fTeamnateDist, fDistance);

            return (TRUE);
         }
      }
   }
   return (FALSE);
}


bool IsShootableThruObstacle (edict_t *pEdict, Vector vecDest)
{
   // Returns if enemy can be shoot through some obstacle
   // TODO: After seeing the disassembled CS Routine it could be speedup and simplified a lot

   Vector vecSrc = GetGunPosition (pEdict);
   Vector vecDir = (vecDest - vecSrc).Normalize (); // 1 unit long
   Vector vecPoint;
   int iThickness = 0;
   int iHits = 0;
   float fDistance;

   edict_t *pentIgnore = pEdict;
   TraceResult tr;

   UTIL_TraceLine (vecSrc, vecDest, ignore_monsters, ignore_glass, pentIgnore, &tr);

   while ((tr.flFraction != 1.0f) && (iHits < 3))
   {
      iHits++;
      iThickness++;
      vecPoint = tr.vecEndPos + vecDir;

      while ((POINT_CONTENTS (vecPoint) == CONTENTS_SOLID) && (iThickness < 98)) // KWo - 23.10.2006
      {
         vecPoint = vecPoint + vecDir;
         iThickness++;
      }

      UTIL_TraceLine (vecPoint, vecDest, ignore_monsters, ignore_glass, pentIgnore, &tr);
   }

   if ((iHits < 3) && (iThickness < 98)) // KWo - 23.10.2006
   {
      fDistance = (vecDest - vecPoint).Length ();

      if (fDistance < 512.0f /*121.95f */) // KWo - 12.07.2008
         return (TRUE);
   }

   return (FALSE);
}


bool BotDoFirePause (bot_t *pBot, float fDistance, bot_fire_delay_t* pDelay)
{
   // Returns true if Bot needs to pause between firing to compensate for
   // punchangle & weapon spread
   float f_Distance = 0.0f;
   float f_Offset = 20.0f;
   float f_Angle = 0.0f;

   if ((pBot->iAimFlags & AIM_ENEMY) && (pBot->pBotEnemy != NULL))  // KWo - 25.04.2006
   {
      if (PlayerHasShieldDrawn(pBot->pBotEnemy))// KWo - 15.08.2007
      {
         float flEnemyDot = GetShootingConeDeviation (pBot->pBotEnemy, &pBot->pEdict->v.origin);
         if (flEnemyDot > 0.92f) // he is facing us - we cannot hurt him...
            return (TRUE);
      }
      f_Distance = (pBot->pEdict->v.origin - pBot->pBotEnemy->v.origin).Length();
   }

   f_Angle = sqrt((fabsf(pBot->pEdict->v.punchangle.y) * M_PI / 180.0f) * (fabsf(pBot->pEdict->v.punchangle.y) * M_PI / 180.0f)
      + (fabsf(pBot->pEdict->v.punchangle.x) * M_PI / 180.0f) * (fabsf(pBot->pEdict->v.punchangle.x) * M_PI / 180.0f));  // KWo - 06.04.2010

   if ((pBot->fTimeFirePause > gpGlobals->time) /* && (f_Distance * tanf(f_Angle) > 0.1f) */)
      return (TRUE);

   // Thanks Whistler for punchangle stuff
   if (fDistance < MIN_BURST_DISTANCE) // KWo - 09.04.2010
      return (FALSE);
   else if (fDistance < 2.0f * MIN_BURST_DISTANCE)
      f_Offset = 10.0f;
   else
      f_Offset = 5.0f;



   if (!BotUsesSniper (pBot)
      && (f_Distance * tanf(f_Angle) > f_Offset + 30.0f * ((100 - pBot->bot_skill) / 99.0f)))  // KWo - 09.04.2010
//   if (!BotUsesSniper (pBot) && (tan(fAngle) * f_Distance) > 10.0f + 30.0f * ((100 - pBot->bot_skill) / 99.0f))  // KWo - 22.10.2006
   {
      if (pBot->fTimeFirePause < gpGlobals->time - 0.4)
         pBot->fTimeFirePause = gpGlobals->time + RANDOM_FLOAT(0.4f, 0.4f + 0.3f * (100.0f - (float)pBot->bot_skill) / 100.0f);  // KWo - 10.11.2006
//      pBot->f_shoot_time = gpGlobals->time;
      pBot->iBurstShotsFired = 0; // KWo - 14.10.2006

      if (g_b_DebugCombat)
         ALERT(at_logged, "[DEBUG] BotDoFirePause - Bot %s has to wait random time because of punchangle.\n", pBot->name);

      return TRUE;
   }

/*
   if (!BotUsesSniper (pBot) && (pDelay->iMaxFireBullets + RANDOM_LONG (0, 1) <= pBot->iBurstShotsFired))
   {
      float fPauseTime = 0.1 * fDistance / pDelay->fMinBurstPauseFactor;
      if (fPauseTime > (125.0f / (pBot->bot_skill + 1)))
         fPauseTime = 125.0f / (pBot->bot_skill + 1);
      pBot->fTimeFirePause = gpGlobals->time + fPauseTime;
//      pBot->f_shoot_time = gpGlobals->time;
      pBot->iBurstShotsFired = 0;
//      ALERT(at_logged, "[DEBUG] Bot %s has to wait max burst wait time.\n", pBot->name);
      return (TRUE);
   }
*/
   return (FALSE);
}


bool BotFireWeapon (Vector v_enemy, bot_t *pBot)
{
   // BotFireWeapon will return (TRUE) if weapon was fired, FALSE otherwise

   bot_weapon_select_t *pSelect = NULL;
   bot_fire_delay_t *pDelay = NULL;
   int iId;
   int select_index = 0;
   int iChosenWeaponIndex = 0;
   int iNum = 0; // KWo - 04.04.2010
   edict_t *pEdict = pBot->pEdict;
   float fDistance = v_enemy.Length (); // how far away is the enemy?
   float base_delay, min_delay, max_delay;
   int skill = abs ((pBot->bot_skill / 20) - 5);
//   int iTask = BotGetSafeTask(pBot)->iTask;
   bool bSecondaryOk = false; // KWo - 24.06.2008
   bool bUseKnife = false;       // KWo - 04.07.2008
   bool bEnemyIsChicken = false; // KWo - 04.07.2008
   char szEnemyModelName[64];    // KWo - 04.07.2008

   // Currently switching Weapon ?
   if ((pBot->current_weapon.iId == CS_WEAPON_INSWITCH)  || pBot->bUsingGrenade
      || (pBot->fTimeWeaponSwitch + 0.1f > gpGlobals->time) || WeaponIsNade(pBot->current_weapon.iId)) // KWo - 19.01.2008
   {
      pBot->f_shoot_time = gpGlobals->time + 0.1f; // KWo - 17.11.2006

      if (g_b_DebugCombat)
         ALERT(at_logged, "[DEBUG] BotFireWeapon - Bot %s is not fireing because it is switching the weapon; time = %f.\n", pBot->name, gpGlobals->time);

      return (FALSE);
   }

   // Don't shoot through TeamMates !
   if (FireHurtsFriend (pBot, fDistance))
   {
      if (g_b_DebugCombat)
         ALERT(at_logged, "[DEBUG] BotFireWeapon - Bot %s is not fireing because it may hurt its friends; time = %f.\n", pBot->name, gpGlobals->time);

      return (FALSE);
   }
   iId = pBot->current_weapon.iId;

   if ((iId > 0) && (iId < MAX_WEAPONS)) // KWo - 05.04.2010
      iNum = weapon_selectIndex[iId];

   // TODO: Ammo Check doesn't always work in CS !! It seems to be because
   // AmmoX and CurrentWeapon Messages don't work very well together - KWo - done ;)
   if ((pBot->current_weapon.iClip == 0) && (weapon_defs[iId].iAmmo1 != -1)
        && (pBot->m_rgAmmo[weapon_defs[iId].iAmmo1] > 0) && !WeaponIsNade(iId) && !pBot->bUsingGrenade) // KWo - 19.01.2008
   {
      if (pBot->f_reloadingtime == 0.0f)
      {
         pBot->f_reloadingtime = gpGlobals->time + cs_weapon_select[iNum].primary_charge_delay + 0.2; // KWo - 04.04.2010
         pEdict->v.button |= IN_RELOAD;   // reload
         pBot->bIsReloading = TRUE;
         pBot->f_move_speed = -pEdict->v.maxspeed; // KWo - 26.03.2007
         pBot->prev_goal_index = pBot->chosengoal_index; // KWo - 18.03.2010
         DeleteSearchNodes(pBot); // KWo - 18.03.2010
         bottask_t TempTask = {NULL, NULL, TASK_SEEKCOVER, TASKPRI_SEEKCOVER, -1, 0.0, FALSE}; // KWo - 09.04.2010
         BotPushTask (pBot, &TempTask); // KWo - 26.03.2007

         if ((g_b_DebugCombat) && (pBot->bIsReloading) && (iId >= 0) && (iId < MAX_WEAPONS))
         {
            ALERT (at_logged, "[DEBUG] BotFireWeapon - bot %s started to reload the weapon - %s, clip = %d, Ammo1 = %d, Ammo2 = %d, charge time = %.2f, time = %.2f.\n",
               pBot->name, weapon_defs[iId].szClassname, pBot->current_weapon.iClip, pBot->current_weapon.iAmmo1, pBot->current_weapon.iAmmo2, cs_weapon_select[iNum].primary_charge_delay, gpGlobals->time);

         }

         return (FALSE);
      }
      else if ((pBot->bIsReloading) && (pBot->f_reloadingtime != 0.0f)
         && (pBot->f_reloadingtime < gpGlobals->time) && (pBot->fTimeWeaponSwitch + 0.3f < gpGlobals->time)) // KWo - 10.02.2008
      {
         pBot->bIsReloading = FALSE;
         pBot->f_reloadingtime = 0.0f;
         if ((WeaponIsPrimaryGun(iId)) && (pBot->current_weapon.iAmmo1 > 0))
            pBot->current_weapon.iAmmo1 = 0;
         if ((WeaponIsPistol(iId)) && (pBot->current_weapon.iAmmo2 > 0))
            pBot->current_weapon.iAmmo2 = 0;
         pBot->m_rgAmmo[weapon_defs[iId].iAmmo1] = 0;
         if ((pBot->current_weapon.iId != CS_WEAPON_KNIFE) && (pBot->f_spawn_time + 6.0f < gpGlobals->time)) // KWo - 30.03.2008
            SelectWeaponByName (pBot, "weapon_knife");

         if (g_b_DebugCombat)
            ALERT(at_logged, "[DEBUG] BotFireWeapon - Bot %s was reloading the weapon %s too long time; time = %f.\n",
               pBot->name, ((iId >= 0) && (iId < MAX_WEAPONS)) ? weapon_defs[iId].szClassname : "unknown", gpGlobals->time);
      }

      if ((pBot->bIsReloading) && (iId >= 0) && (iId < MAX_WEAPONS))
      {
         if ((BotGetSafeTask(pBot)->iTask != TASK_SEEKCOVER) && (BotGetSafeTask(pBot)->iTask != TASK_HIDE)) // KWo - 18.02.2008
         {
            if (g_b_DebugCombat || g_b_DebugTasks)
               ALERT(at_logged, "[DEBUG] BotFireWeapon - Bot %s is reloading the weapon - preparing TASK_SEEKCOVER.\n", pBot->name);

            bottask_t TempTask = {NULL, NULL, TASK_SEEKCOVER, TASKPRI_SEEKCOVER, -1, 0.0, FALSE}; // KWo - 09.04.2010
            BotPushTask (pBot, &TempTask);
         }
         return (FALSE);
      }
   }

   pBot->bIsReloading = FALSE;
   pBot->f_reloadingtime = 0.0f;

   pSelect = &cs_weapon_select[0];
   pDelay = &cs_fire_delay[0];

   if ((g_b_cv_jasonmode) || (pBot->bIsChickenOrZombie))  // KWo - 14.06.2008
      goto WeaponSelectEnd;

   if (!FNullEnt (pBot->pBotEnemy))
   {
      strncpy_s (szEnemyModelName, sizeof(szEnemyModelName), (INFOKEY_VALUE (GET_INFOKEYBUFFER (pBot->pBotEnemy), "model")), sizeof (szEnemyModelName) - 1); // KWo - 04.07.2008
      bEnemyIsChicken = ((strncmp ("chicken", szEnemyModelName, 7) == 0)
         || (strncmp ("zomb", szEnemyModelName, 4) == 0)); // KWo - 04.07.2008

      if (pBot->bot_skill > 80)
      {
         if ((fDistance < 100.0f) && (!bEnemyIsChicken) && ((pBot->pEdict->v.health > 80)
                              || ((pBot->pBotEnemy->v.weapons == CS_WEAPON_KNIFE)
                                  && !IsGroupOfEnemies (pBot, pEdict->v.origin)))) // KWo - 04.07.2008
            bUseKnife = TRUE;
      }

      if (bUseKnife)
         goto WeaponSelectEnd;
   }

   // loop through all the weapons until terminator is found...
   while (pSelect[select_index].iId)
   {
      // is the bot NOT carrying this weapon?
      if  (!(pEdict->v.weapons & (1 << pSelect[select_index].iId)))
      {
         select_index++;  // skip to next weapon
         continue;
      }

      iId = pSelect[select_index].iId;
/*
      // KWo - 24.06.2008 commenting out - no switching to secondary in short distances
      // Check if it's the currently used weapon because
      // otherwise Ammo in Clip will be ignored
      if ((iId == pBot->current_weapon.iId)
          && (pBot->current_weapon.iClip > 0 || weapon_defs[iId].iAmmo1 == -1))
         iChosenWeaponIndex = select_index;
*/
      // is primary percent less than weapon primary percent AND
      // no ammo required for this weapon OR
      // enough ammo available to fire AND
      // the bot is far enough away to use primary fire AND
      // the bot is close enough to the enemy to use primary fire

      if ((select_index > 0) && (select_index < 7)
         && ((pBot->m_rgAmmo[weapon_defs[iId].iAmmo1] + pBot->m_rgAmmoInClip[iId]) >= pSelect[select_index].min_primary_ammo)) // KWo - 20.04.2010
      {
         bSecondaryOk = true;
         iChosenWeaponIndex = select_index;
      }

      if (((weapon_defs[iId].iAmmo1 == -1)
           || (pBot->m_rgAmmo[weapon_defs[iId].iAmmo1] + pBot->m_rgAmmoInClip[iId] >= pSelect[select_index].min_primary_ammo))
            && (/* ((iTask != TASK_ATTACK) && !bSecondaryOk)
                || */ ((fDistance >= pSelect[select_index].primary_min_distance)
                 && (fDistance <= pSelect[select_index].primary_max_distance))
                 || (!bSecondaryOk && (fDistance > 100.0f)))) // KWo - 25.05.2010
         iChosenWeaponIndex = select_index;

      select_index++;
      if (select_index > 24)
         break;
   }
   select_index = iChosenWeaponIndex;

WeaponSelectEnd:
   iId = pSelect[select_index].iId;

   // select this weapon if it isn't already selected
   if (pBot->current_weapon.iId != iId)
   {

      if (g_b_DebugCombat)
         ALERT(at_logged, "[DEBUG] BotFireWeapon - Bot %s has %s and wants to change to better weapon %s; time = %f.\n", pBot->name,
               weapon_defs[pBot->current_weapon.iId].szClassname, weapon_defs[iId].szClassname, gpGlobals->time);

      if ((pBot->current_weapon.iId != CS_WEAPON_INSWITCH) && (pBot->fTimeWeaponSwitch + 0.7 < gpGlobals->time)) // KWo - 10.02.2008
      {
         SelectWeaponByName (pBot, pSelect[select_index].weapon_name);
         // Reset Burst Fire Variables
         pBot->fTimeLastFired = 0.0f;
         pBot->fTimeFirePause = 0.0f;
         pBot->iBurstShotsFired = 0;

         if (g_b_DebugCombat)
            ALERT(at_logged, "[DEBUG] BotFireWeapon - Bot %s is changing the weapon to a better one - %s; time = %f.\n",
               pBot->name, weapon_defs[iId].szClassname, gpGlobals->time);

         return (FALSE);
      }
   }

   if (BotCheckZoom(pBot))
   {
      if (g_b_DebugCombat)
         ALERT(at_logged, "[DEBUG] BotFireWeapon - Bot %s is changing the zoom of the weapon %s; time = %f.\n",
            pBot->name, weapon_defs[iId].szClassname, gpGlobals->time);

      return (FALSE); // KWo - 07.07.2008
   }

   if (BotHasShieldDrawn (pBot))
      return (FALSE); // KWo - 17.07.2007


   if (/* (pDelay[select_index].iId != iId) && */ (pBot->fTimeWeaponSwitch + 0.7 > gpGlobals->time))
   {
      if (g_b_DebugCombat)
         ALERT(at_logged, "[DEBUG] BotFireWeapon - Bot %s found a better weapon and is switching to it; time = %f.\n", pBot->name, gpGlobals->time);

      return (FALSE);
   }

   if (BotUsesSniper (pBot) && !FNullEnt (pBot->pBotEnemy) && !pBot->bIsReloading)
   {
      if (/* (GetShootingConeDeviation (pBot->pBotEnemy, &pEdict->v.origin) < 0.95f)
         && */ (pEdict->v.velocity.x != 0.0f
            || pEdict->v.velocity.y != 0.0f
            || pEdict->v.velocity.z != 0.0f))
      {
         pBot->f_move_speed = 0.0f;
         pBot->f_sidemove_speed = 0.0f;
         pBot->f_wpt_timeset = gpGlobals->time;

         if (fabs(pEdict->v.velocity.x) > 5.0f
            || fabs(pEdict->v.velocity.y) > 5.0f
            || fabs(pEdict->v.velocity.z) > 5.0f)
         {
            if (g_b_DebugCombat)
               ALERT(at_logged, "[DEBUG] Bot %s cannot shoot, because wants to don't move before shooting; f_move_speed = %f; f_sidemove_speed = %f; time = %f.\n",
                  pBot->name, pBot->f_move_speed, pBot->f_sidemove_speed, gpGlobals->time);

            return (FALSE);
         }
      }
      else if ((GetShootingConeDeviation (pBot->pBotEnemy, &pEdict->v.origin) >= 0.95f)
               && (pBot->f_shoot_time + 1.0f < gpGlobals->time)
               && !(pEdict->v.oldbuttons & IN_DUCK)) // KWo - 24.06.2008
      {
         pBot->f_ducktime = gpGlobals->time + 2.5f;
      }
   }

   base_delay = pDelay[select_index].primary_base_delay;
   if ((g_bIsOldCS15) && (pBot->current_weapon.iId == CS_WEAPON_AWP)) // KWo - 26.08.2006
      base_delay = 1.2f;

   min_delay = pDelay[select_index].primary_min_delay[skill];
   max_delay = pDelay[select_index].primary_max_delay[skill];

   // Need to care for burst fire ?
   if ((fDistance < MIN_BURST_DISTANCE) || (iId == CS_WEAPON_KNIFE) // KWo - 05.06.2006
       || (pBot->f_blind_time > gpGlobals->time))
   {
      if (iId == CS_WEAPON_KNIFE)
      {
         if (fDistance < 64.0f)
         {
            if (RANDOM_LONG (1, 100) > 60)
            {
               pEdict->v.button |= IN_ATTACK;  // use primary attack
            }
            else
            {
               pEdict->v.button |= IN_ATTACK2;  // use secondary attack
            }
         }
      }
      else
      {
         // If Automatic Weapon, just press attack
         if (pSelect[select_index].primary_fire_hold)
         {
            if (pBot->fTimeFirePause <= gpGlobals->time)
               pEdict->v.button |= IN_ATTACK;
//      pBot->f_shoot_time = gpGlobals->time;

//            if (!BotUsesSniper (pBot) && (pDelay[select_index].iMaxFireBullets + RANDOM_LONG (0, 1) <= pBot->iBurstShotsFired))
            if (!BotUsesSniper (pBot) && (10 <= pBot->iBurstShotsFired))
            {
               pBot->iBurstShotsFired = 0;
               pBot->fTimeFirePause = gpGlobals->time + RANDOM_FLOAT(0.2f, 0.4f + 0.3f * (100.0f - (float)pBot->bot_skill) / 100.0f);
            }
         }
         // if not, toggle buttons
         else if ((pEdict->v.oldbuttons & IN_ATTACK) == 0)
         {
            pEdict->v.button |= IN_ATTACK;
         }
      }
      if (pEdict->v.button & IN_ATTACK) // KWo - 23.10.2006
         pBot->f_shoot_time = gpGlobals->time; // KWo - 06.06.2006
//         pBot->f_shoot_time = gpGlobals->time + base_delay + RANDOM_FLOAT (min_delay, max_delay);
   }
   else
   {
      if (BotDoFirePause (pBot, fDistance, &pDelay[select_index]))
      {
         return (FALSE);
      }

      // Don't attack with knife over long distance
      if (iId == CS_WEAPON_KNIFE)
      {
         return (FALSE);
      }

      if (pSelect[select_index].primary_fire_hold)
      {
         pBot->f_shoot_time = gpGlobals->time;
         pEdict->v.button |= IN_ATTACK;  // use primary attack
      }
      else
      {
         pEdict->v.button |= IN_ATTACK;  // use primary attack
         pBot->f_shoot_time = gpGlobals->time + base_delay + RANDOM_FLOAT (min_delay, max_delay);
         pBot->f_zoomchecktime = pBot->f_shoot_time - 0.1f;
      }
   }
   return (TRUE);
}


bool BotCheckZoom (bot_t *pBot)  // KWo - 09.07.2008
{
   int iZoomMagnification;
   float fDistance;
   bool bZoomChange;
   edict_t *pEdict = pBot->pEdict;
   TraceResult tr;

   if ((!BotUsesSniper (pBot) && !WeaponIsAssualtSniper (pBot->current_weapon.iId)))
      return (false); // KWo - 07.07.2008

   iZoomMagnification = 0;

   // check if we are looking at the obstacle in front of us - then unzoom if we are looking
   // at something not far way from us (for example - at the wall :) ).
   TRACE_LINE (GetGunPosition (pEdict), pBot->vecLookAt, ignore_glass, pEdict, &tr);
   fDistance = (tr.vecEndPos - GetGunPosition(pEdict)).Length();

   // Check Distance for correct Sniper Zooming
   if (WeaponIsAssualtSniper (pBot->current_weapon.iId))
   {
      if (fDistance > (MIN_BURST_DISTANCE * 2.0f))
         iZoomMagnification = 1;
   }
   else if (BotUsesSniper (pBot))
   {
      if (!(pBot->iAimFlags & AIM_ENEMY) && !(pBot->iAimFlags & AIM_LASTENEMY)
      && !(pBot->iAimFlags & AIM_PREDICTPATH) && !(pBot->iAimFlags & AIM_CAMP)
      && (pBot->f_bot_see_enemy_time + 5.0f < gpGlobals->time)
      && (pBot->f_heard_sound_time + 5.0f < gpGlobals->time))  // KWo - 08.07.2008
         iZoomMagnification = 0;
      else if ((fDistance < 1500.0f) && (fDistance > MIN_BURST_DISTANCE))
         iZoomMagnification = 1;
      else if (fDistance >= 1500.0f)
         iZoomMagnification = 2;
   }

   if ((fDistance <= MIN_BURST_DISTANCE) || (BotGetSafeTask(pBot)->iTask == TASK_FOLLOWUSER))
      iZoomMagnification = 0;

   bZoomChange = FALSE;

   switch (iZoomMagnification)
   {
   case 0:
      if (pEdict->v.fov < 90)
         bZoomChange = TRUE;
      break;

   case 1:
      if ((pEdict->v.fov != 40) && (pEdict->v.fov != 55))
         bZoomChange = TRUE;
      break;

   case 2:
      if ((pEdict->v.fov != 10) && (pEdict->v.fov != 15))
         bZoomChange = TRUE;
   }

   if ((bZoomChange) && (pBot->f_zoomchecktime < gpGlobals->time)) // KWo - 08.07.2008
   {
      pEdict->v.button |= IN_ATTACK2;
      pBot->f_shoot_time = gpGlobals->time + 0.15f; // KWo - 02.04.2010

      if (g_b_DebugCombat)
         ALERT(at_logged, "[DEBUG] BotCheckZoom - Bot %s is changing zoom; time = %f.\n", pBot->name, gpGlobals->time);

      pBot->f_zoomchecktime = gpGlobals->time + 0.5f;
   }
   return (bZoomChange); // KWo - 07.07.2008
}

bool BotCheckCorridor (bot_t *pBot) // KWo - 21.03.2006
{
   edict_t *pEdict = pBot->pEdict;
   TraceResult tr1;
   TraceResult tr2;
   Vector v_src;
   Vector v_dest;
   Vector vec_mov_dir;
   Vector vec_mov_angle;
   Vector v_left;
   Vector v_right;

   if ((pBot->prev_wpt_index[0]<0) || (pBot->prev_wpt_index[0] >= g_iNumWaypoints))
      return (false);

   if (pBot->f_spawn_time + 30.0f > gpGlobals->time) // KWo - 08.10.2006
      return (false);

   v_src = pBot->wpt_origin + pBot->pEdict->v.view_ofs;
   vec_mov_dir = (pBot->wpt_origin - paths[pBot->prev_wpt_index[0]]->origin).Normalize();
   vec_mov_dir = vec_mov_dir * 150;
   v_dest = pBot->wpt_origin + vec_mov_dir + pBot->pEdict->v.view_ofs;
   // fire a traceline forward to check if we have some space in front of us
   TRACE_LINE (v_src, v_dest, ignore_monsters, pEdict, &tr1);
//   UTIL_DrawBeam (v_src, v_dest, 20, 50, 0, 0, 255, 0, 255, 0);
   if (!FStrEq(STRING(tr1.pHit->v.classname),"worldspawn") || (tr1.flFraction < 1.0f))
      return (false);
   v_src = paths[pBot->prev_wpt_index[0]]->origin + pBot->pEdict->v.view_ofs;
   v_dest = v_src - vec_mov_dir;
   // fire a traceline back to check if we have some space back of us
   TRACE_LINE (v_src, v_dest, ignore_monsters, pEdict, &tr1);
//   UTIL_DrawBeam (v_src, v_dest, 20, 50, 0, 0, 255, 0, 255, 0);
   if (!FStrEq(STRING(tr1.pHit->v.classname),"worldspawn") || (tr1.flFraction < 1.0f))
      return (false);

   vec_mov_angle = UTIL_VecToAngles (vec_mov_dir);
   MAKE_VECTORS (vec_mov_angle);
   v_src = paths[pBot->prev_wpt_index[0]]->origin + pBot->pEdict->v.view_ofs;
   v_left = v_src + gpGlobals->v_right * (-180);
   v_right = v_src + gpGlobals->v_right * 180;
   TRACE_LINE (v_src, v_left, ignore_monsters, pEdict, &tr1);
   TRACE_LINE (v_src, v_right, ignore_monsters, pEdict, &tr2);
//   UTIL_DrawBeam (v_src, v_left, 20, 50, 0, 0, 0, 255, 255, 0);
//   UTIL_DrawBeam (v_src, v_right, 20, 50, 0, 0, 0, 255, 255, 0);

   // if both sides of trace origin are limited by some obstacles - probably we aren't in the free area
   if ((tr1.flFraction < 1.0f) && (tr2.flFraction < 1.0f))
      return (false);

   v_src = pBot->wpt_origin + vec_mov_dir + pBot->pEdict->v.view_ofs;
   v_left = v_src + gpGlobals->v_right * (-250);
   v_right = v_src + gpGlobals->v_right * 250;
   // now we need to fire 2 tracelines from forward position to the left and to the right
   // to check if they are blocked both - it may mean we are in some corridor :)
   TRACE_LINE (v_src, v_left, ignore_monsters, pEdict, &tr1);
   TRACE_LINE (v_src, v_right, ignore_monsters, pEdict, &tr2);
//   UTIL_DrawBeam (v_src, v_left, 20, 50, 0, 255, 0, 0, 255, 0);
//   UTIL_DrawBeam (v_src, v_right, 20, 50, 0, 255, 0, 0, 255, 0);


   if ((FStrEq(STRING(tr1.pHit->v.classname),"worldspawn")) && (tr1.flFraction < 0.5f)
       && (FStrEq(STRING(tr2.pHit->v.classname),"worldspawn")) && (tr2.flFraction < 0.5f))
   {
      v_src = pBot->wpt_origin + pBot->pEdict->v.view_ofs;
      v_dest = pBot->wpt_origin + vec_mov_dir + pBot->pEdict->v.view_ofs;
//      UTIL_DrawBeam (v_src, v_dest, 20, 50, 0, 0, 255, 0, 255, 0);
      return (true);
   }
   return (false);
}

void BotFocusEnemy(bot_t *pBot)
{
   static edict_t *pEdict;
//   static float flDotStart;       // KWo - 18.11.2006
//   static float flDotStop;        // KWo - 18.11.2006
//   static float fSqDist;          // KWo - 18.11.2006
   static Vector vecDist;
   static float f_distance;
   static float fOffset;
   static float flDot;
   static float flEnemyDot;
   static Vector vecEnemy;

   if (FNullEnt(pBot->pBotEnemy))
      return;

   pEdict = pBot->pEdict;

   // aim for the head and/or body
   vecEnemy = BotBodyTarget(pBot->pBotEnemy, pBot);
// + 1.5f * (pBot->fTimeFrameInterval - gpGlobals->frametime) * (1.0f * pBot->pBotEnemy->v.velocity - pBot->pEdict->v.velocity);

   if (pBot->f_enemy_surprise_time > gpGlobals->time)
   {
      if (g_b_DebugCombat)
         ALERT(at_logged, "[DEBUG] BotFocusEnemy - Bot %s is surprised by an enemy %s; time = %f.\n",
            pBot->name, STRING(pBot->pBotEnemy->v.netname), gpGlobals->time);

      return;
   }

   if ((pBot->iAimFlags & AIM_ENEMY) && (pBot->f_bot_see_enemy_time + 0.6 < gpGlobals->time)
      && !(BotGetSafeTask(pBot)->iTask == TASK_SHOOTBREAKABLE)) // KWo - 13.01.2012
   {
      pBot->bWantsToFire = FALSE;
      return;
   }

   vecDist = vecEnemy - GetGunPosition(pEdict);
   vecDist.z = 0;  // ignore z component (up & down)
   f_distance = vecDist.Length ();  // how far away is the enemy scum?

   if ((f_distance < MIN_BURST_DISTANCE) && !BotUsesSniper (pBot))// KWo - 25.08.2006
   {
      flDot = GetShootingConeDeviation (pEdict, &vecEnemy); // KWo - 01.10.2010
      if (pBot->current_weapon.iId == CS_WEAPON_KNIFE)
      {
         if (f_distance < 80.0f)
            pBot->bWantsToFire = TRUE;
         else if (f_distance > 120.0f)       // KWo - 01.10.2010
            pBot->bWantsToFire = FALSE;
      }
      else if (flDot > 0.8f)               // KWo - 01.10.2010
         pBot->bWantsToFire = TRUE;
      else
         pBot->bWantsToFire = FALSE;
   }
   else
   {
//      float fOffset = 1.5f * (100 - pBot->bot_skill) + 15.0f; // KWo - 12.01.2007
      fOffset = 3.0f * BotAimTab[pBot->bot_skill / 20].fAim_Z + 15.0f; // KWo - 18.11.2006
      flDot = GetShootingConeDeviation (pEdict, &vecEnemy); // KWo - 18.11.2006
      if (fOffset < 20.0f) // KWo - 18.11.2006
         fOffset = 20.0f;
//      fSqDist = sqrt(fOffset * fOffset + f_distance * f_distance); // KWo - 18.11.2006
//      flDotStop = (f_distance * f_distance) / (fSqDist * fSqDist); // KWo - 18.11.2006

      if (flDot < 0.9) // KWo - 20.01.2009
//      if (flDot < flDotStop) // KWo - 18.11.2006
         pBot->bWantsToFire = FALSE;
      else
      {
         if (pBot->current_weapon.iId == CS_WEAPON_KNIFE)
            pBot->bWantsToFire = TRUE;
         else
         {
            flEnemyDot = GetShootingConeDeviation (pBot->pBotEnemy, &pEdict->v.origin);

            // Enemy doesn't faces the Bot ?
            if ((flEnemyDot > 0.9f) && (flDot > 0.92f) && !BotUsesSniper (pBot))
            {
               pBot->bWantsToFire = TRUE;
            }
            else
            {
               fOffset = fOffset / 3.0f;
               if (fOffset < 10.0f) // KWo - 12.07.2008
                  fOffset = 10.0f;

//               fSqDist = sqrt(fOffset * fOffset + f_distance * f_distance); // KWo - 18.11.2006
//               flDotStart = (f_distance * f_distance) / (fSqDist * fSqDist); // KWo - 18.11.2006

               if (((pBot->pBotEnemy->v.velocity).Length() > 5.0f)
                  || ((pEdict->v.velocity).Length() > 5.0f)) // KWo - 11.04.2010
//                  flDotStart = 0.99f;
//               if (flDot > flDotStart) // KWo - 18.11.2006
               if (flDot > 0.99f) // KWo - 18.11.2006
               {
                  pBot->bWantsToFire = TRUE;
               }
            }
         }
      }
      if (pBot->bShootThruSeen) // KWo - 12.07.2008
      {
         if (flDot > 0.99f)
         {
            pBot->bWantsToFire = TRUE;
         }
         else
            pBot->bWantsToFire = FALSE;
      }
   }

   if (g_b_DebugCombat)
      ALERT(at_logged, "[DEBUG] BotFocusEnemy - Bot %s %s to fire; time = %f.\n",
         pBot->name, pBot->bWantsToFire ? "wants" : "doesn't want", gpGlobals->time);

   return;
}


// Does the (unwaypointed) attack movement
void BotDoAttackMovement (bot_t *pBot)
{
   // No enemy ? No need to do strafing
   if (FNullEnt (pBot->pBotEnemy))
      return;

   if ((pBot->current_weapon.iId == CS_WEAPON_KNIFE) || pBot->bIsChickenOrZombie) // KWo - 18.01.2011
      pBot->dest_origin = pBot->pBotEnemy->v.origin;

   float f_distance;
   TraceResult tr;
   edict_t *pEdict = pBot->pEdict;
   Vector vecEnemy = pBot->vecEnemy;
   vecEnemy = vecEnemy - GetGunPosition (pEdict);
   vecEnemy.z = 0;  // ignore z component (up & down)
   f_distance = vecEnemy.Length ();  // how far away is the enemy scum?
   Vector vecForward;         // KWo - 25.08.2008
   Vector vecSide;            // KWo - 25.08.2008
   Vector vecTargetPos;       // KWo - 25.08.2008
   Vector2D vec2DirToPoint;   // KWo - 25.08.2008
   Vector2D vec2RightSide;    // KWo - 25.08.2008

   int iEnemyIndex = ENTINDEX(pBot->pBotEnemy) - 1;
   int iWeaponEnemy = clients[iEnemyIndex].iCurrentWeaponId;
   int iId;
   int iApproach;
   int iRand;
   bool bUsesSniper;
//   bool bSeekCover = FALSE; // KWo - 08.04.2010

   if (pBot->fTimeWaypointMove /* - pBot->fTimeFrameInterval */ < gpGlobals->time) // KWo - 17.10.2006 - reverted back... changed again
   {
      iId = pBot->current_weapon.iId;
      bUsesSniper = BotUsesSniper (pBot);

      // If suspecting Enemy stand still
      if ((pBot->iStates & STATE_SUSPECTENEMY) && !(pBot->iStates & STATE_SEEINGENEMY))
         iApproach = 49;

      // If reloading or VIP back off
      else if (pBot->bIsReloading || pBot->bIsVIP)
         iApproach = 29;
      else if ((iId == CS_WEAPON_KNIFE) || pBot->bIsChickenOrZombie) // Knife ?
         iApproach = 100;
      else
      {
         iApproach = (int) (pBot->pEdict->v.health * pBot->fAgressionLevel);	// KWo to remove warning
         if (bUsesSniper && (iApproach > 49))
            iApproach = 49;
      }

      if (((((iApproach < 30) && (!g_bBombPlanted)) || (pBot->bIsVIP) || (pBot->bIsReloading))
         && /* (FInViewCone (&pEdict->v.origin, pBot->pBotEnemy) */ ((GetShootingConeDeviation (pBot->pBotEnemy, &pEdict->v.origin) > 0.8)
         && (pBot->iStates & STATE_SEEINGENEMY)))
          ||  (WeaponIsSniper(iWeaponEnemy)
                  && (GetShootingConeDeviation (pBot->pBotEnemy, &pEdict->v.origin) > 0.95f))) // KWo - 09.04.2010 - stealen from YapB
      {
         pBot->f_move_speed = -pEdict->v.maxspeed;
         bottask_t TempTask = {NULL, NULL, TASK_SEEKCOVER, TASKPRI_SEEKCOVER, -1, 0.0f, FALSE}; // KWo - 09.04.2010
         BotPushTask (pBot, &TempTask);
//         bSeekCover = TRUE;
      }
      else if (iApproach < 50)
         pBot->f_move_speed = 0.0f;
      else
         pBot->f_move_speed = pEdict->v.maxspeed;

      if ((iId != CS_WEAPON_KNIFE) && (f_distance < 96.0f))
         pBot->f_move_speed = -pEdict->v.maxspeed;

      if (/* (!bSeekCover) && */ (bUsesSniper) && !pBot->bIsReloading) // KWo - 20.10.2006  thanks strelomet
      {
         pBot->byFightStyle = 1;
         pBot->f_lastfightstylecheck = gpGlobals->time;
      }
      else if (BotUsesRifle (pBot) || BotUsesSubmachineGun (pBot)) /// KWo - 20.10.2006
      {
         if (pBot->f_lastfightstylecheck + 3.0f < gpGlobals->time)
         {
            iRand = RANDOM_LONG (1, 100);

            if (f_distance < 500.0f)
               pBot->byFightStyle = 0;
            else if (f_distance < 1024.0f)
            {
               if ((IsGroupOfEnemies (pBot, pBot->pBotEnemy->v.origin)) || (WeaponIsSniper(iWeaponEnemy)))     // KWo - 15.04.2010
                  pBot->byFightStyle = 0;
               else if (iRand < (BotUsesSubmachineGun (pBot) ? 50 : 30))   // KWo - 20.10.2006
                  pBot->byFightStyle = 0;
               else
                  pBot->byFightStyle = 1;
            }
            else
            {
               if ((IsGroupOfEnemies (pBot, pBot->pBotEnemy->v.origin)) || (WeaponIsSniper(iWeaponEnemy)))    // KWo - 15.04.2010
                  pBot->byFightStyle = 0;
               else if (iRand < (BotUsesSubmachineGun (pBot) ? 80 : 93))   // KWo - 20.10.2006
                  pBot->byFightStyle = 1;
               else
                  pBot->byFightStyle = 0;
            }
            pBot->f_lastfightstylecheck = gpGlobals->time;
         }
      }
      else
      {
         if (pBot->f_lastfightstylecheck + 3.0f < gpGlobals->time)
         {
            if (!FireHurtsFriend(pBot, f_distance) && (RANDOM_LONG (0, 100) < 50))
            {
               if ((iId == CS_WEAPON_KNIFE) && (pBot->pBotEnemy->v.velocity.Length() > 10.0f)
                  && (iWeaponEnemy == CS_WEAPON_KNIFE)
                  && (2.0f * pBot->pEdict->v.health > pBot->pBotEnemy->v.health)) // KWo - 24.06.2008
                  pBot->byFightStyle = 1;
               else
                  pBot->byFightStyle = 0;
            }
            else
               pBot->byFightStyle = 0;

            pBot->f_lastfightstylecheck = gpGlobals->time;
         }
      }

      if ((WeaponIsPistol(iWeaponEnemy) || WeaponIsPrimaryGun(iWeaponEnemy))
         && FInViewCone (&pEdict->v.origin, pBot->pBotEnemy) && (iId == CS_WEAPON_KNIFE)) // KWo - 23.02.2008
         pBot->byFightStyle = 0;

      if (pBot->bIsChickenOrZombie) // KWo - 24.06.2008
         pBot->byFightStyle = 0;

      if (((pBot->bot_skill > 60) && (pBot->byFightStyle == 0))
         || /* pBot->bIsReloading || */ (WeaponIsPistol(iId) && (f_distance < 500.0f))) // KWo - 23.02.2008
      {
         if (pBot->f_StrafeSetTime < gpGlobals->time)
         {
            // to start strafing, we have to first figure out if the target is on the left side or right side
            MAKE_VECTORS (pBot->pBotEnemy->v.v_angle);

            vec2DirToPoint = (pEdict->v.origin - pBot->pBotEnemy->v.origin).Make2D ().Normalize ();
            vec2RightSide = gpGlobals->v_right.Make2D ().Normalize ();

            if  (DotProduct  (vec2DirToPoint, vec2RightSide) < 0.0f)
               pBot->byCombatStrafeDir = 1;
            else
               pBot->byCombatStrafeDir = 0;

            if (RANDOM_LONG (1, 100) < 30)
               pBot->byCombatStrafeDir ^= 1;

            pBot->f_StrafeSetTime = gpGlobals->time + RANDOM_FLOAT (0.8f, 3.0f);
         }

         if (!(pEdict->v.oldbuttons & IN_DUCK))
         {
            if (pBot->byCombatStrafeDir == 0)
            {
               if (!BotCheckWallOnLeft (pBot))
               {
                  pBot->f_sidemove_speed = -1.0f * pEdict->v.maxspeed; // KWo - 20.09.2008
               }
               else if (!BotCheckWallOnRight (pBot))
               {
                  pBot->byCombatStrafeDir ^= 1;
                  pBot->f_StrafeSetTime = gpGlobals->time + 1.0f;
                  //pBot->f_sidemove_speed = 0.0f;
                  pBot->f_sidemove_speed = 1.0f * pEdict->v.maxspeed; // KWo - 12.04.2010
               }
               else
               {
                  pBot->f_sidemove_speed = 0.0f;
                  pBot->f_StrafeSetTime = gpGlobals->time + 1.0f;
               }
            }
            else
            {
               if (!BotCheckWallOnRight (pBot))
               {
                  pBot->f_sidemove_speed = 1.0f * pEdict->v.maxspeed; // KWo - 20.09.2008
               }
               else if(!BotCheckWallOnLeft (pBot))
               {
                  pBot->byCombatStrafeDir ^= 1;
                  pBot->f_StrafeSetTime = gpGlobals->time + 1.0f;
//                  pBot->f_sidemove_speed = 0.0f;
                  pBot->f_sidemove_speed = -1.0f * pEdict->v.maxspeed; // KWo - 12.04.2010
               }
               else
               {
                  pBot->f_sidemove_speed = 0.0f;
                  pBot->f_StrafeSetTime = gpGlobals->time + 1.0f;
               }
            }
         }

         if (pBot->bot_skill > 80)
         {
            float flEnemyDot = GetShootingConeDeviation (pBot->pBotEnemy, &pEdict->v.origin);
            if ((pBot->f_jumptime + 5.0f < gpGlobals->time) && (pEdict->v.flags & FL_ONGROUND)
               && (f_distance < 2 * MIN_BURST_DISTANCE) && (pBot->f_ducktime < gpGlobals->time))
            {
               if ((g_i_botthink_index == g_iFrameCounter) && (RANDOM_LONG (1, 100) < 30)
                  && (pEdict->v.velocity.Length2D () > 150.0f)
                  && (flEnemyDot > 0.95f) && !bUsesSniper) // KWo - 24.06.2008
               {
                  pEdict->v.button |= IN_JUMP;
                  pBot->f_jumptime = gpGlobals->time + 1.0f;
               }
            }
         }
      }
      else if (pBot->byFightStyle == 1)
      {
 //        int iNearestToEnemyPoint = WaypointFindNearestToMove (pBot->pBotEnemy->v.origin);

         if ((pBot->pTasks->iTask != TASK_SEEKCOVER) && (pBot->pTasks->iTask != TASK_ENEMYHUNT)
            && (pBot->ucVisibility & WAIST_VISIBLE)
            && (!(pBot->ucVisibility & CUSTOM_VISIBLE))
            && (g_i_botthink_index % 4 == g_iFrameCounter % 4)
            && (((bUsesSniper) && (RANDOM_LONG (0, 100) < 85))
               || (BotUsesRifle (pBot) && (RANDOM_LONG (0, 100) < 70))
               || (BotUsesSubmachineGun (pBot) && (RANDOM_LONG (0, 100) < 50)))
            /* && (pBot->f_shoot_time - 0.2f < gpGlobals->time) */
            && (pBot->f_StrafeSetTime < gpGlobals->time) // strafe timer used to check if we can duck our bot...
            && (pBot->f_ducktime <= gpGlobals->time)
            && (!(pEdict->v.button & IN_ATTACK) || !bUsesSniper)
            && !pBot->bIsReloading) // KWo - 24.06.2008
         {
            UTIL_TraceLine(pEdict->v.origin, GetGunPosition(pBot->pBotEnemy), ignore_monsters, ignore_glass, pEdict, &tr);
            if (tr.flFraction >= 1.0f)
            {
               pBot->f_ducktime = gpGlobals->time + 3.0f;
            }
            pBot->f_StrafeSetTime = gpGlobals->time + 1.0f; // KWo - 24.02.2008 - check every 1 second if we are going to duck or not...)
         }
         pBot->f_wpt_timeset = gpGlobals->time;

         pBot->f_move_speed = 0.0f;
         pBot->f_sidemove_speed = 0.0f;
         pBot->prev_time = gpGlobals->time + 1.0f;     // KWo - 27.05.2010
         pBot->f_moved_distance = 15.0f;               // KWo - 27.05.2010

         if (g_b_DebugCombat)
            ALERT (at_logged, "[DEBUG] Bot %s is not moving because of the fightstyle; time = %f.\n", pBot->name, gpGlobals->time);

//         pEdict->v.button |= IN_DUCK;
      }
   }
   else if ((pBot->bot_skill > 80) && (pEdict->v.flags & FL_ONGROUND) && (pBot->f_ducktime < gpGlobals->time) )
   {
      if (f_distance < MIN_BURST_DISTANCE)
      {
/*
         if ((RANDOM_LONG (1, 1000) < 10) && (pEdict->v.velocity.Length2D () > 150)
            && (FInViewCone (&pEdict->v.origin, pBot->pBotEnemy)))  // KWo - 25.02.2008
            pEdict->v.button |= IN_JUMP;
*/
      }
      else
      {
         pEdict->v.button |= IN_DUCK;
         pBot->f_ducktime = gpGlobals->time + 1.5f /* (pBot->fTimeFrameInterval * 3.5f) */;
      }
   }

   if (pBot->f_ducktime > gpGlobals->time) // KWo - 20.10.2006
   {
      pBot->f_move_speed = 0.0f;
      pBot->f_sidemove_speed = 0.0f;
      pBot->prev_time = gpGlobals->time + 1.0f;     // KWo - 27.05.2010
      pBot->f_moved_distance = 15.0f;               // KWo - 27.05.2010
   }

   MAKE_VECTORS (pEdict->v.v_angle);  // KWo - 25.08.2008
   if (pBot->bIsReloading)
   {
      vecForward = (gpGlobals->v_forward * (-pEdict->v.maxspeed)) * 0.2f;
      vecSide = (gpGlobals->v_right * pBot->f_sidemove_speed) * 0.2f;
      vecTargetPos = pEdict->v.origin + vecForward + vecSide;
      if (!IsDeadlyDropAtPos (pBot, vecTargetPos)) // KWo - 25.08.2008
      {
         pBot->f_move_speed = -pEdict->v.maxspeed;
      }
      pBot->f_ducktime = gpGlobals->time - 4.0f; // KWo - 20.10.2006
   }

   if (!pBot->bInWater && !pBot->bOnLadder
      && ((pBot->f_move_speed != 0) || (pBot->f_sidemove_speed != 0)))
   {
      float fTimeRange = pBot->fTimeFrameInterval;  // KWo - 17.10.2006 - reverted back

      vecForward = (gpGlobals->v_forward * pBot->f_move_speed) * 0.2f;
      vecSide = (gpGlobals->v_right * pBot->f_sidemove_speed) * 0.2f;
      vecTargetPos = pEdict->v.origin + vecForward + vecSide + (pEdict->v.velocity * fTimeRange);

      if (IsDeadlyDropAtPos (pBot, vecTargetPos))
      {
         pBot->f_sidemove_speed = -pBot->f_sidemove_speed;
         pBot->f_move_speed = -pBot->f_move_speed;
         pEdict->v.button &= ~IN_JUMP;
         pBot->f_jumptime = 0.0f;

         if (g_b_DebugCombat)
            ALERT(at_logged,"[DEBUG] BotDoAttackMovement - bot %s is changing the strafing direction.\n", pBot->name);
      }
   }
   return;
}


int BotCheckGrenades (bot_t *pBot)
{
   int weapons = pBot->pEdict->v.weapons;

   if (weapons & (1 << CS_WEAPON_HEGRENADE))
      return (CS_WEAPON_HEGRENADE);
   else if (weapons & (1 << CS_WEAPON_FLASHBANG))
      return (CS_WEAPON_FLASHBANG);
   else if (weapons & (1 << CS_WEAPON_SMOKEGRENADE))
      return (CS_WEAPON_SMOKEGRENADE);

   return (-1);
}


void BotSelectBestWeapon (bot_t *pBot)
{
   bot_weapon_select_t *pSelect = &cs_weapon_select[0];
   int select_index = 0;
   int iChosenWeaponIndex = 0;
   int iId;
   int iWeapons = pBot->pEdict->v.weapons;
   int iWeaponEnemy = CS_WEAPON_KNIFE;
   int iEnemyIndex = 0;
   int iDistance = 1000;         // KWo - 24.06.2008
   bool bSecondaryOk = false;    // KWo - 24.06.2008
   bool bUseKnife = FALSE;       // KWo - 17.11.2006
   bool bEnemyIsChicken = false; // KWo - 04.07.2008
   char szEnemyModelName[64];    // KWo - 04.07.2008

   if (!FNullEnt(pBot->pBotEnemy)) // KWo - 09.01.2007
   {
      iEnemyIndex = ENTINDEX(pBot->pBotEnemy)-1;
      iWeaponEnemy = clients[iEnemyIndex].iCurrentWeaponId;
      iDistance = (int)(pBot->pEdict->v.origin - pBot->pBotEnemy->v.origin).Length(); // KWo - 24.06.2008
      strncpy_s (szEnemyModelName, sizeof(szEnemyModelName), (INFOKEY_VALUE (GET_INFOKEYBUFFER (pBot->pBotEnemy), "model")), sizeof (szEnemyModelName) - 1); // KWo - 04.07.2008
      bEnemyIsChicken = ((strncmp ("chicken", szEnemyModelName, 7) == 0)
         || (strncmp ("zomb", szEnemyModelName, 4) == 0)); // KWo - 04.07.2008
   }

   if (pBot->bIsReloading) // KWo - 10.07.2008
      return;

   // loop through all the weapons until terminator is found...
   while (pSelect[select_index].iId)
   {
      // is the bot NOT carrying this weapon?
      if (!(iWeapons & (1 << pSelect[select_index].iId)))
      {
         select_index++;  // skip to next weapon
         continue;
      }

      iId = pSelect[select_index].iId;

      // is primary percent less than weapon primary percent AND
      // no ammo required for this weapon OR
      // enough ammo available to fire AND
      // the bot is far enough away to use primary fire AND
      // the bot is close enough to the enemy to use primary fire

      if ((select_index > 0) && (select_index < 7)
         && ((pBot->m_rgAmmo[weapon_defs[iId].iAmmo1] + pBot->m_rgAmmoInClip[iId]) >= pSelect[select_index].min_primary_ammo)) // KWo - 20.04.2010
      {
         bSecondaryOk = true;
         iChosenWeaponIndex = select_index;
      }

      if (((weapon_defs[iId].iAmmo1 == -1)
               || (pBot->m_rgAmmo[weapon_defs[iId].iAmmo1] + pBot->m_rgAmmoInClip[iId] >= pSelect[select_index].min_primary_ammo))
               && (((!bSecondaryOk) && (iDistance > 100))
                  || ((iDistance >= pSelect[select_index].primary_min_distance)
                   && (iDistance <= pSelect[select_index].primary_max_distance)))) // KWo - 20.04.2010
         iChosenWeaponIndex = select_index;

      select_index++;
      if (select_index > 24) // KWo - 14.07.2007
         break;
   }

   iChosenWeaponIndex %= NUM_WEAPONS + 1;

   if (!FNullEnt (pBot->pBotEnemy)) // KWo - 17.11.2006
   {
      float distance = (GetGunPosition(pBot->pEdict) - pBot->vecEnemy).Length();
      if (pBot->bot_skill > 80)
      {
         if ((distance < 100.0f) && (!bEnemyIsChicken)
            && ((pBot->pEdict->v.health > 80)
               || ((iWeaponEnemy == CS_WEAPON_KNIFE)
                  && (!IsGroupOfEnemies (pBot, pBot->pEdict->v.origin))))) // KWo - 04.07.2008
         {
            bUseKnife = TRUE;
         }
      }
   }

   if ((bUseKnife) || (g_b_cv_jasonmode) || (pBot->bIsChickenOrZombie))  // KWo - 14.06.2008
      select_index = 0;
   else
      select_index = iChosenWeaponIndex;

   iId = pSelect[select_index].iId;

   // select this weapon if it isn't already selected
   if ((pBot->current_weapon.iId != iId) && (pBot->fTimeWeaponSwitch + 1.0f < gpGlobals->time)) // KWo - 08.04.2010
      SelectWeaponByName (pBot, pSelect[select_index].weapon_name);
   return;
}


int GetWeaponIndexOfBot (bot_t *pBot) // KWo - 04.04.2010
{
   bot_weapon_select_t *pSelect = &cs_weapon_select[0];
   int iId = pBot->current_weapon.iId;
   int iNum = 0;
   int i = 0;

   // loop through all the weapons until terminator is found...
   while (pSelect->iId)
   {
      if (pSelect->iId == iId)
      {
         iNum = i;
         break;
      }
      i++;
      pSelect++;
   }
   if (i >= 26)
      iNum = 0;
   return (iNum);
}


int HighestWeaponOfEdict (edict_t *pEdict)
{
   bot_weapon_select_t *pSelect = &cs_weapon_select[0];
   int iWeapons = pEdict->v.weapons;
   int iNum = 0;
   int i = 0;

   // loop through all the weapons until terminator is found...
   while (pSelect->iId)
   {
      // is the bot carrying this weapon?
      if ((iWeapons & (1 << pSelect->iId)))
         iNum = i;
      i++;
      pSelect++;
   }

   return (iNum);
}


int HighestPistolOfEdict (edict_t *pEdict)
{
   bot_weapon_select_t *pSelect = &cs_weapon_select[0];
   int iWeapons = pEdict->v.weapons;
   int iNum = 0;
   int i = 0;

   // loop through all the weapons until terminator is found...
   while (pSelect->iId)
   {
      // is the bot carrying this weapon?
      if ((iWeapons & (1 << pSelect->iId)))
         iNum = i;
      i++;
      pSelect++;
      if (i > 6)
         break;
   }

   return (iNum);
}


void SelectWeaponByName (bot_t *pBot, char *pszName)
{
   if (pBot->fTimeWeaponSwitch + 0.3 > gpGlobals->time)
      return;
//   pBot->current_weapon.iId = CS_WEAPON_INSWITCH;
   pBot->fTimeWeaponSwitch = gpGlobals->time;
   FakeClientCommand (pBot->pEdict, pszName);
   pBot->bIsReloading = FALSE;
   pBot->f_reloadingtime = 0.0f;
   return;
}


void SelectWeaponByNumber (bot_t *pBot, int iNum)
{
   if (pBot->fTimeWeaponSwitch + 0.3 > gpGlobals->time)
      return;
//   pBot->current_weapon.iId = CS_WEAPON_INSWITCH;
   pBot->fTimeWeaponSwitch = gpGlobals->time;
   FakeClientCommand (pBot->pEdict, (char *) &cs_weapon_select[iNum].weapon_name);
   pBot->bIsReloading = FALSE;
   pBot->f_reloadingtime = 0.0f;
   return;
}


void BotCommandTeam (bot_t *pBot)
{
   bool bMemberNear = FALSE;
   bool bSomeTeamnateAlive = FALSE;  // KWo - 25.04.2006
   edict_t *pTeamEdict;
   edict_t *pEdict = pBot->pEdict;
   int ind;

   // Prevent spamming
   if ((pBot->fTimeTeamOrder + 2.0f < gpGlobals->time) && (!g_b_cv_ffa) && (g_b_cv_radio)) // KWo - 03.02.2007
   {
      pBot->fTimeTeamOrder = gpGlobals->time;   // KWo - 06.03.2010

      // Search Teammates seen by this Bot
      for (ind = 0; ind < gpGlobals->maxClients; ind++)
      {
         if (!(clients[ind].iFlags & CLIENT_USED)
             || !(clients[ind].iFlags & CLIENT_ALIVE)
             || (clients[ind].iTeam != pBot->bot_team)
             || (clients[ind].pEdict == pEdict))
            continue;

         bSomeTeamnateAlive = TRUE;  // KWo - 25.04.2006
         pTeamEdict = clients[ind].pEdict;

         if (BotEntityIsVisible (pBot, pTeamEdict->v.origin))
         {
            bMemberNear = TRUE;
            break;
         }
      }

      // Has Teammates ?
      if ((bMemberNear) && (RANDOM_LONG (1, 100) < 30)) // KWo - 06.03.2010
      {
         if ((pBot->bot_personality == 1) && !pBot->bIsVIP)
         {
            BotPlayRadioMessage (pBot, RADIO_STORMTHEFRONT);
            pBot->fTimeTeamOrder = gpGlobals->time;   // KWo - 07.07.2008
         }
         else if (pBot->iStates & STATE_SEEINGENEMY)  // KWo - 07.07.2008
         {
            BotPlayRadioMessage (pBot, RADIO_FALLBACK);
            pBot->fTimeTeamOrder = gpGlobals->time;   // KWo - 07.07.2008
         }
      }
      else if ((bSomeTeamnateAlive) && (pBot->iStates & STATE_SEEINGENEMY) && !FNullEnt(pBot->pBotEnemy)) // KWo - 13.09.2008
      {
         if ((GetShootingConeDeviation (pBot->pBotEnemy, &pEdict->v.origin) >= 0.9)
            && (pBot->pBotEnemy->v.oldbuttons & IN_ATTACK) && (RANDOM_LONG (1, 100) < 30)) // KWo - 06.03.2010
         {
            BotPlayRadioMessage (pBot, RADIO_TAKINGFIRE);
            pBot->fTimeTeamOrder = gpGlobals->time;      // KWo - 07.07.2008
         }
      }
   }
}


bool IsGroupOfEnemies (bot_t *pBot, Vector vLocation)
{
   edict_t *pPlayer;
   edict_t *pEdict = pBot->pEdict;
   int iNumPlayers = 0;
   float fDistance;
   int i;

   // search the world for enemy players...
   for (i = 0; i < gpGlobals->maxClients; i++)
   {
      if (!(clients[i].iFlags & CLIENT_USED)
          || !(clients[i].iFlags & CLIENT_ALIVE)
          || (clients[i].pEdict == pEdict)
          || ((clients[i].iTeam == pBot->bot_team) && (!g_b_cv_ffa)))  // KWo - 25.04.2008
         continue;

      pPlayer = clients[i].pEdict;
      fDistance = (pPlayer->v.origin - vLocation).Length ();

      if (fDistance < 800.0f) // KWo - 24.04.2008
      {
         if (!BotEnemyIsVisible (pBot, pPlayer)) // KWo - 25.04.2008
            continue;

         iNumPlayers++;
      }
   }

   if (iNumPlayers < 2)
      return (FALSE);

   if (g_b_cv_ffa)
   {
      if (g_iAliveCTs + g_iAliveTs > 0)
         return ((float)iNumPlayers/(float)(g_iAliveCTs + g_iAliveTs) >= 0.25f); // 4/16
      else
         return (FALSE);
   }

   if ((pBot->bot_team == TEAM_CS_TERRORIST) && (g_iAliveCTs > 0))
   {
      return ((float)iNumPlayers/(float)(g_iAliveCTs) >= 0.25f);
   }
   else if ((pBot->bot_team == TEAM_CS_COUNTER) && (g_iAliveTs > 0))
   {
      return ((float)iNumPlayers/(float)(g_iAliveTs) >= 0.25f);
   }

   return (FALSE);
}


//
// VecCheckToss - returns the velocity at which an object should be lobbed from vecspot1 to land near vecspot2.
// returns g_vecZero if toss is not feasible.
//
Vector VecCheckToss (bot_t *pBot, const Vector &vecSpot1, Vector vecSpot2)
{
   edict_t *pEdict = pBot->pEdict;  // KWo - 14.12.2006
   edict_t *pEnEdict = pBot->pEdict;  // KWo - 14.12.2006
   float flGravityAdj = 0.5f;
   TraceResult tr;
   Vector vecMidPoint; // halfway point between Spot1 and Spot2
   Vector vecApex; // highest point
   Vector vecScale;
   Vector vecGrenadeVel;
   Vector vecTemp;
   float flGravity = g_f_cv_Gravity * flGravityAdj; // KWo - 16.11.2006

   vecSpot2 = vecSpot2 - pEdict->v.velocity;
   vecSpot2.z -= 15.0f;

   if (vecSpot2.z - vecSpot1.z > 500)
      return (g_vecZero); // to high, fail

   // calculate the midpoint and apex of the 'triangle'
   // UNDONE: normalize any Z position differences between spot1 and spot2 so that triangle is always RIGHT

   vecMidPoint = vecSpot1 + (vecSpot2 - vecSpot1) * 0.5f;

   vecTemp = vecMidPoint + Vector (0.0f, 0.0f, 384.0f);
/*
   if (pHostEdict)
   {
      if ((pHostEdict->v.origin - pEdict->v.origin).Length() < 20.0f)
// Vector start, Vector end, int life, int width, int noise, int red, int green, int blue, int brightness, int speed
         UTIL_DrawBeam (vecMidPoint, vecTemp, 10, 50, 0, 255, 0, 0, 255, 0);
   }
*/
   TRACE_HULL (vecMidPoint, vecTemp, ignore_monsters, head_hull, pEdict, &tr);
   if (tr.flFraction != 1.0f)
   {
      vecMidPoint = tr.vecEndPos;
      vecMidPoint.z = tr.pHit->v.absmin.z;
      vecMidPoint.z--;
   }

   if ((vecMidPoint.z < vecSpot2.z) /* || (vecMidPoint.z < vecSpot1.z) */)
      return (g_vecZero); // to not enough space, fail

   // How high should the grenade travel to reach the apex
   float distance1 = fabs(vecMidPoint.z - vecSpot1.z);
   float distance2 = fabs(vecMidPoint.z - vecSpot2.z);

   // How long will it take for the grenade to travel this distance
   float time1 = sqrtf (distance1 / (0.5f * flGravity));
   float time2 = sqrtf (distance2 / (0.5f * flGravity));

   if (time1 < 0.1f)
      return (g_vecZero); // too close

   if (time1 + time2 > 3.8f) // KWo - 27.07.2007
   {
      if (g_b_DebugCombat)
      {
         float dist3 = (pEdict->v.origin - vecSpot2).Length();
         ALERT(at_logged,"[DEBUG] VecCheckToss - Bot %s - to far away to throw a nade - distance = %.0f.\n", pBot->name, dist3);
      }
      return g_vecZero; // grenade is likely to explode in the sky (from Whistler's YapB)
   }

   // how hard to throw sideways to get there in time.
   vecGrenadeVel = (vecSpot2 - vecSpot1) / (time1 + time2);

   // how hard upwards to reach the apex at the right time.
   vecGrenadeVel.z = flGravity * time1;

   // find the apex
   vecApex = vecSpot1 + vecGrenadeVel * time1;
   vecApex.z = vecMidPoint.z;
/*
   if (pHostEdict)
   {
      if ((pHostEdict->v.origin - pEdict->v.origin).Length() < 20.0f)
// Vector start, Vector end, int life, int width, int noise, int red, int green, int blue, int brightness, int speed
         UTIL_DrawBeam (vecSpot1, vecApex, 10, 50, 0, 0, 255, 0, 255, 0);
   }
*/
   TRACE_HULL (vecSpot1, vecApex, dont_ignore_monsters, head_hull, pEdict, &tr);
   if ((tr.flFraction != 1.0f) || (tr.fAllSolid))
      return (g_vecZero); // fail!

   if ((!FNullEnt (pBot->pLastEnemy)) && (pBot->vecLastEnemyOrigin != g_vecZero)) // KWo - 23.07.2007
      pEnEdict = pBot->pLastEnemy;
   else
      return (g_vecZero);
/*
   if (pHostEdict)
   {
      if ((pHostEdict->v.origin - pEdict->v.origin).Length() < 20.0f)
// Vector start, Vector end, int life, int width, int noise, int red, int green, int blue, int brightness, int speed
         UTIL_DrawBeam (vecApex, vecSpot2, 10, 50, 0, 0, 255, 0, 255, 0);
   }
*/
   TRACE_HULL (vecApex, vecSpot2, ignore_monsters, head_hull, pEnEdict, &tr);
   if (tr.flFraction != 1.0f)
   {
      Vector vecDir = (vecApex - vecSpot2).Normalize ();
      float n = -DotProduct (tr.vecPlaneNormal, vecDir);

      if ((n > 0.7f) || (tr.flFraction < 0.8f)) // 60 degrees
         return (g_vecZero);
   }

   return (vecGrenadeVel);
}

//
// VecCheckThrow - returns the velocity vector at which an object should be thrown from vecspot1 to hit vecspot2.
// returns g_vecZero if throw is not feasible.
//
Vector VecCheckThrow (bot_t *pBot, const Vector &vecSpot1, Vector vecSpot2, float flSpeed)  // KWo - 22.03.2006
{
   edict_t *pEdict = pBot->pEdict;  // KWo - 22.03.2006
   edict_t *pEnEdict = pBot->pEdict;  // KWo - 14.12.2006
//   float flGravityAdj = 0.45; // KWo - 18.06.2006
   float flGravityAdj = 0.40; // KWo - 27.07.2007
   float flGravity = g_f_cv_Gravity * flGravityAdj; // KWo - 16.11.2006

   vecSpot2 = vecSpot2 - pEdict->v.velocity;
   Vector vecGrenadeVel = (vecSpot2 - vecSpot1);

   float time = vecGrenadeVel.Length() / 400.0f;  // KWo - 22.03.2006 - thanks to Whistler

   if ((time < 0.01f) || (time > 4.0f))  // KWo - 19.06.2006
      return g_vecZero; // fail

   vecGrenadeVel = vecGrenadeVel * (1.0f / time);

   // adjust upward toss to compensate for gravity loss
   vecGrenadeVel.z += flGravity * time * 0.5f;

   Vector vecApex = vecSpot1 + (vecSpot2 - vecSpot1) * 0.5f;
//   vecApex.z += 0.5f * flGravity * (time * 0.5f) * (time * 0.5f);
   vecApex.z += 0.5f * flGravity * (time * 0.5f);
/*
   if (pHostEdict)
   {
      if ((pHostEdict->v.origin - pEdict->v.origin).Length() < 20.0f)
// Vector start, Vector end, int life, int width, int noise, int red, int green, int blue, int brightness, int speed
         UTIL_DrawBeam (vecSpot1, vecApex, 10, 50, 0, 0, 0, 255, 255, 0);
   }
*/
   TraceResult tr;
//   TRACE_HULL (vecSpot1, vecApex, dont_ignore_monsters, head_hull, pEdict, &tr);
   TRACE_LINE (vecSpot1, vecApex, dont_ignore_monsters, pEdict, &tr);
   if ((tr.flFraction != 1.0f) || (tr.fAllSolid))  // KWo - 23.07.2007
      return (g_vecZero); // fail!


   if ((!FNullEnt (pBot->pLastEnemy)) && (pBot->vecLastEnemyOrigin != g_vecZero)) // KWo - 23.07.2007
      pEnEdict = pBot->pLastEnemy;
   else
      return (g_vecZero);
/*
   if (pHostEdict)
   {
      if ((pHostEdict->v.origin - pEdict->v.origin).Length() < 20.0f)
// Vector start, Vector end, int life, int width, int noise, int red, int green, int blue, int brightness, int speed
         UTIL_DrawBeam (vecApex, vecSpot2, 10, 50, 0, 0, 0, 255, 255, 0);
   }
*/
//   TRACE_HULL (vecApex, vecSpot2, ignore_monsters, head_hull, pEnEdict, &tr);
   TRACE_LINE (vecApex, vecSpot2, ignore_monsters, pEnEdict, &tr);
   if (tr.flFraction != 1.0f)
   {
      Vector vecDir = (vecApex - vecSpot2).Normalize ();
      float n = -DotProduct (tr.vecPlaneNormal, vecDir);

      // 60 degrees
      if (((n > 0.75f) || (tr.flFraction < 0.8f)))  // KWo - 19.06.2006
         return (g_vecZero);
   }
   return (vecGrenadeVel);
}
