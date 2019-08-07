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
// bot_client.cpp
//
// Handles Messages sent from the Server to a Client Bot

#include "bot_globals.h"


void BotClient_CS_VGUI (void *p, int bot_index)
{
   // This message is sent when the VGUI menu is displayed.

   if (state == 0)
   {
      if ((*(int *) p) == 2)  // is it a team select menu?
         bots[bot_index].start_action = MSG_CS_TEAM_SELECT;
      else if ((*(int *) p) == 26)  // is it a Terrorist class selection menu?
         bots[bot_index].start_action = MSG_CS_T_SELECT;
      else if ((*(int *) p) == 27)  // is it a CT class selection menu?
         bots[bot_index].start_action = MSG_CS_CT_SELECT;
   }
}


void BotClient_CS_ShowMenu (void *p, int bot_index)
{
   // This message is sent when a menu is being displayed.

   if (state < 3)
      return; // ignore first 3 fields of message

   // Team Selection Messages
   if (strncmp ((char *) p, "#Team_Select", 12) == 0)  // team select menu?
      bots[bot_index].start_action = MSG_CS_TEAM_SELECT;
   else if (strcmp ((char *) p, "#CT_Select") == 0)  // CT model select menu?
      bots[bot_index].start_action = MSG_CS_CT_SELECT;
   else if (strcmp ((char *) p, "#Terrorist_Select") == 0)  // T model select?
      bots[bot_index].start_action = MSG_CS_T_SELECT;
}


void BotClient_CS_StatusIcon (void *p, int bot_index)
{
   // this message tells a game client to display (or stop displaying) a certain status icon on
   // his player's HUD.

   static int icon_state = 0;

   if (state == 0)
      icon_state = *(int *) p;

   else if (state == 1)
   {
      // is it the C4 icon ?
      if (strcmp ("c4", (char *) p) == 0)
      {
         // is icon blinking ?
         if (icon_state == 2)
            bots[bot_index].b_bomb_blinking = TRUE; // bomb icon is blinking
         else
            bots[bot_index].b_bomb_blinking = FALSE;
      }

      // else is it the defuser icon ?
      else if (strcmp ("defuser", (char *) p) == 0)
      {
         // is icon lit ?
         if (icon_state > 0)
            bots[bot_index].b_has_defuse_kit = TRUE; // Bot has Defuse Kit
         else
            bots[bot_index].b_has_defuse_kit = FALSE;
      }

      // else is it the buy zone icon ?
      else if (strcmp ("buyzone", (char *) p) == 0)
      {
         // is icon lit ?
         if (icon_state > 0)
            bots[bot_index].b_can_buy = TRUE; // bot can buy
         else
            bots[bot_index].b_can_buy = FALSE;
      }
   }
}


void BotClient_CS_WeaponList (void *p, int bot_index)
{
   // This message is sent when a client joins the game.  All of the weapons
   // are sent with the weapon ID and information about what ammo is used.

   static bot_weapon_t bot_weapon;

   if (state == 0)
   {
      strncpy_s (bot_weapon.szClassname, sizeof(bot_weapon.szClassname), (char *) p, sizeof (bot_weapon.szClassname) - 1);
      bot_weapon.szClassname[sizeof (bot_weapon.szClassname) - 1] = 0;
   }
   else if (state == 1)
      bot_weapon.iAmmo1 = *(int *) p;  // ammo index 1
   else if (state == 2)
      bot_weapon.iAmmo1Max = *(int *) p;  // max ammo1
   else if (state == 3)
      bot_weapon.iAmmo2 = *(int *) p;  // ammo index 2
   else if (state == 4)
      bot_weapon.iAmmo2Max = *(int *) p;  // max ammo2
   else if (state == 5)
      bot_weapon.iSlot = *(int *) p;  // slot for this weapon
   else if (state == 6)
      bot_weapon.iPosition = *(int *) p;  // position in slot
   else if (state == 7)
      bot_weapon.iId = *(int *) p;  // weapon ID
   else if (state == 8)
   {
      bot_weapon.iFlags = *(int *) p;  // flags for weapon (WTF???)

      // store away this weapon with it's ammo information...
      weapon_defs[bot_weapon.iId] = bot_weapon;
//      ALERT(at_logged,"[DEBUG] WeaponList - weapon %s, ammo_index1=%d, maxAmmo1=%d, ammo_index2=%d, maxAmmo2=%d.\n",
//         bot_weapon.szClassname, bot_weapon.iAmmo1, bot_weapon.iAmmo1Max, bot_weapon.iAmmo2, bot_weapon.iAmmo2Max);
   }
}

void BotClient_CS_CurrentWeapon (void *p, int bot_index)
{
   // This message is sent when a weapon is selected (either by the bot chosing
   // a weapon or by the server auto assigning the bot a weapon).
   // In CS it's also called when Ammo is increased/decreased

   static int iState;
   static int iId;
   static int iClip;

   static bot_t *pBot;

   pBot = &bots[bot_index];
//   edict_t *pPlayer = INDEXENT (bot_index + 1);

   if (state == 0)
      iState = *(int *) p;  // state of the current weapon (WTF???)
   else if (state == 1)
      iId = *(int *) p;  // weapon ID of current weapon
   else if (state == 2)
   {
      iClip = *(int *) p;  // ammo currently in the clip for this weapon
//      const char* bname = STRING(pPlayer->v.netname);
      if (pBot->is_used) // KWo - 12.12.2006
      {
         if ((iId <= 31) && (iState == 1))
         {
            // Ammo amount decreased ? Must have fired a bullet...
            if ((iId == pBot->current_weapon.iId) && (pBot->current_weapon.iClip > iClip))
            {
               // Time fired with in burst firing time ?
               if (pBot->fTimeLastFired + 1.0f > gpGlobals->time)
                  pBot->iBurstShotsFired++;

               pBot->fTimeLastFired = gpGlobals->time; // Remember the last bullet time
            }
            if ((pBot->current_weapon.iId != iId) || (iClip != 0)
               || (pBot->current_weapon.iClip < 5)) // for some reaon sometimes iClip comes here as 0 (from 100)
            {                                       // - don't use that value
               pBot->current_weapon.iClip = iClip;
               pBot->m_rgAmmoInClip[iId] = iClip;
               clients[bot_index].iCurrentClip = iClip; // KWo - 15.08.2007
               if (iClip == 0) // KWo - 15.08.2007
               {
                  if (WeaponIsPistol(iId) || WeaponIsPrimaryGun(iId))
                  {
                     clients[bot_index].fReloadingTime = gpGlobals->time + 1.5f;

                     if (g_b_DebugCombat)
                        ALERT(at_logged, "[DEBUG] BotClient_CS_CurrentWeapon - Player %s is reloading his weapon...\n", STRING(clients[bot_index].pEdict->v.netname));
                  }
               }
            }
            pBot->current_weapon.iId = iId;
            pBot->bIsReloading = FALSE;
            pBot->f_reloadingtime = 0.0f;

            clients[bot_index].iCurrentWeaponId = iId; // KWo - 12.12.2006
            // update the ammo counts for this weapon...
            if (WeaponIsPistol(iId)) // KWo - 25.12.2006
            {
               pBot->current_weapon.iAmmo2 = pBot->m_rgAmmo[weapon_defs[iId].iAmmo1];

               if (g_b_DebugCombat)
                  ALERT(at_logged,"[DEBUG] BotClient_CS_CurrentWeapon - Bot %s current weapon = %s, ammo in clip = %d. \n",
                     pBot->name, weapon_defs[iId].szClassname, iClip);
            }
            else if (WeaponIsPrimaryGun(iId))
            {
               pBot->current_weapon.iAmmo1 = pBot->m_rgAmmo[weapon_defs[iId].iAmmo1];

               if (g_b_DebugCombat)
                  ALERT(at_logged,"[DEBUG] BotClient_CS_CurrentWeapon - Bot %s current weapon = %s, ammo in clip = %d. \n",
                     pBot->name, weapon_defs[iId].szClassname, iClip);
            }
         }
      }
      else // it's not a bot... KWo - 12.12.2006
      {
         if ((iId <= 31) && (iState == 1))
         {
            if ((clients[bot_index].iCurrentWeaponId != iId) || (iClip != 0) // 15.08.2007
               || (clients[bot_index].iCurrentClip < 5)) // for some reaon sometimes iClip comes here as 0 (from 100)
            {                                       // - don't use that value
               clients[bot_index].iCurrentClip = iClip;
               if (iClip == 0) // KWo - 15.08.2007
               {
                  if (WeaponIsPistol(iId) || WeaponIsPrimaryGun(iId))
                  {
                     clients[bot_index].fReloadingTime = gpGlobals->time + 1.5f;

                     if (g_b_DebugCombat)
                     ALERT(at_logged, "[DEBUG] BotClient_CS_CurrentWeapon - Player %s is reloading his weapon...\n",                        
                     STRING(clients[bot_index].pEdict->v.netname));
                  }
               }
            }
            clients[bot_index].iCurrentWeaponId = iId;
         }
      }

      if (g_b_DebugCombat && (iId <= 31) && (iState == 1))
      {
         ALERT(at_logged,"[DEBUG] BotClient_CS_CurrentWeapon - Player %s, weapon = %s, WeaponState = %d, ammo in clip = %d. \n",
            STRING(clients[bot_index].pEdict->v.netname), weapon_defs[iId].szClassname, iState, iClip);
      }
   }
}


void BotClient_CS_AmmoX (void *p, int bot_index)
{
   // This message is sent whenever ammo ammounts are adjusted (up or down).
   // NOTE: Logging reveals that CS uses it very unreliable !

   static int index;
   static int iWeaponSecNum;
   static int iWeaponPrimNum;
   static int iWeaponSecID;
   static int iWeaponPrimID;

   static bot_t *pBot;
   static edict_t *pEdict;

   pBot = &bots[bot_index];
   pEdict = pBot->pEdict;

   if (state == 0)
      index = *(int *) p; // ammo index (for type of ammo)
   else if (state == 1)
   {
      pBot->m_rgAmmo[index] = *(int *) p; // store it away

      if (pBot->current_weapon.iId < CS_WEAPON_INSWITCH) // KWo - 24.12.2006
      {
         iWeaponSecNum = HighestPistolOfEdict (pEdict); // KWo - 08.01.200&
         iWeaponPrimNum = HighestWeaponOfEdict(pEdict); // KWo - 08.01.200&
         iWeaponSecID = cs_weapon_select[iWeaponSecNum].iId; // KWo - 08.01.2007
         iWeaponPrimID = cs_weapon_select[iWeaponPrimNum].iId; // KWo - 08.01.2007

         // update the ammo counts for this weapon...
         if ((iWeaponSecNum > 0) && (iWeaponSecNum < 7)) // KWo - 08.01.2007
         {
            if (index == weapon_defs[iWeaponSecID].iAmmo1)
            {
               pBot->current_weapon.iAmmo2 = pBot->m_rgAmmo[index];

               if (g_b_DebugCombat)
                  ALERT(at_logged,"[DEBUG] BotClient_CS_AmmoX - Bot %s current weapon = %s, ammo = %d, ammoId = %d.\n",
                     pBot->name, weapon_defs[iWeaponSecID].szClassname, pBot->current_weapon.iAmmo2, index);
            }
         }
         if ((iWeaponPrimNum > 6) && (iWeaponPrimNum < MAX_WEAPONS))
         {
            if (index == weapon_defs[iWeaponPrimID].iAmmo1)
            {
               pBot->current_weapon.iAmmo1 = pBot->m_rgAmmo[index];

               if (g_b_DebugCombat)
                  ALERT(at_logged,"[DEBUG] BotClient_CS_AmmoX - Bot %s current weapon = %s, ammo = %d, ammoId = %d.\n",
                     pBot->name, weapon_defs[iWeaponPrimID].szClassname, pBot->current_weapon.iAmmo1, index);
            }
         }
      }
   }
}


void BotClient_CS_AmmoPickup (void *p, int bot_index)
{
   // This message is sent when the bot picks up some ammo (AmmoX messages are
   // also sent so this message is probably not really necessary except it
   // allows the HUD to draw pictures of ammo that have been picked up.  The
   // bots don't really need pictures since they don't have any eyes anyway.

   static int index;
   static int ammo_index;
   static int iWeaponSecNum;
   static int iWeaponPrimNum;
   static int iWeaponSecID;
   static int iWeaponPrimID;

   static edict_t *pEdict;
   pEdict = bots[bot_index].pEdict;

   ammo_index = 1;
   if (state == 0)
      index = *(int *) p;
   else if (state == 1)
   {
      bots[bot_index].m_rgAmmo[index] += *(int *) p;

      iWeaponSecNum = HighestPistolOfEdict (pEdict); // KWo - 08.01.200&
      iWeaponPrimNum = HighestWeaponOfEdict(pEdict); // KWo - 08.01.200&
      iWeaponSecID = cs_weapon_select[iWeaponSecNum].iId; // KWo - 08.01.2007
      iWeaponPrimID = cs_weapon_select[iWeaponPrimNum].iId; // KWo - 08.01.2007

//      ammo_index = bots[bot_index].current_weapon.iId;

      // update the ammo counts for this weapon (ONLY if bot knows its current weapon)...
      if ((ammo_index >= 0) && (ammo_index < MAX_WEAPONS))
      {
         if ((iWeaponSecNum > 0) && (iWeaponSecNum < 7)) // KWo - 08.01.2007
         {
            if (index == weapon_defs[iWeaponSecID].iAmmo1)
            {
               bots[bot_index].current_weapon.iAmmo2 = bots[bot_index].m_rgAmmo[weapon_defs[iWeaponSecID].iAmmo1];

               if (g_b_DebugCombat)
                  ALERT(at_logged, "[DEBUG] BotClient_CS_AmmoPickup - Bot %s weapon %s, current ammo %d, ammoId = %d.\n",
                     bots[bot_index].name, weapon_defs[iWeaponSecID].szClassname, bots[bot_index].current_weapon.iAmmo2, index);
            }
         }
         if ((iWeaponPrimNum > 6) && (iWeaponPrimNum < MAX_WEAPONS))
         {
            if (index == weapon_defs[iWeaponPrimID].iAmmo1)
            {
               bots[bot_index].current_weapon.iAmmo1 = bots[bot_index].m_rgAmmo[weapon_defs[iWeaponPrimID].iAmmo1];

               if (g_b_DebugCombat)
                  ALERT(at_logged, "[DEBUG] BotClient_CS_AmmoPickup - Bot %s weapon %s, current ammo %d, ammoId = %d.\n",
                     bots[bot_index].name, weapon_defs[iWeaponPrimID].szClassname, bots[bot_index].current_weapon.iAmmo1, index);
            }
         }
      }
   }
}


void BotClient_CS_Damage (void *p, int bot_index)
{
   // This message gets sent when the bots are getting damaged.

   static int damage_armor;
   static int damage_taken;
   static int damage_bits;
   static Vector damage_origin;
   static bot_t *pBot;
   static edict_t *pEdict;
   static edict_t *pEnt;
   static bool team_attack;   // KWo - 28.07.2018
   static bool enemy_attack;  // KWo - 28.07.2018

   pBot = &bots[bot_index];
   pEdict = pBot->pEdict;

   if (state == 0)
      damage_armor = *(int *) p;
   else if (state == 1)
      damage_taken = *(int *) p;
   else if (state == 2)
      damage_bits = *(int *) p;
   else if (state == 3)
      damage_origin.x = *(float *) p;
   else if (state == 4)
      damage_origin.y = *(float *) p;
   else if (state == 5)
   {
      damage_origin.z = *(float *) p;

      if ((damage_armor > 0) || (damage_taken > 0))
      {
         pBot->iLastDamageType = damage_bits;
         pEnt = pEdict->v.dmg_inflictor;
         pEdict->v.dmgtime = gpGlobals->time; // KWo - 10.04.2010
         team_attack = false;    // KWo - 28.07.2018
         enemy_attack = true;    // KWo - 28.07.2018
         if (!FNullEnt(pEnt))
         {
            if ((pEnt->v.flags & FL_CLIENT) && (pEnt != pEdict)) // KWo - 28.08.2018
            {   
               if ((UTIL_GetTeam (pEnt) == pBot->bot_team) && (!g_b_cv_ffa)) // KWo - 28.08.2018
                  team_attack = true;
               else
                  enemy_attack = true; // KWo - if ffa is enabled, then threat the teamnate as an enemy, too...
            }

            if ((team_attack) && (g_b_cv_ffrev)) // KWo/THE_STORM - 28.07.2018
            {
               // FIXFIXFIXFIXFIXME: THIS IS BLATANTLY CHEATING!!!!
               // KWo - No - it's when Your teamnate is attacking You, then the bot may consider You as an enemy
               // but BotHurtsFriends function doesn't let him yet to shoot at You...
//               ALERT(at_logged, "Bot %s is checking the TK revenge.\n", pBot->name); // TEST!!!
               if (RANDOM_LONG (1, 100) < 10)
               {
                  if (FNullEnt (pBot->pLastEnemy) && FNullEnt (pBot->pBotEnemy)
                     && (pBot->f_bot_see_enemy_time + 3.0f < gpGlobals->time)
                     && (pBot->f_blind_time < gpGlobals->time)) // KWo - 23.03.2008
                  {
//                     pBot->iAimFlags |= AIM_ENEMY; // KWo - 27.08.2006
//                     pBot->iAimFlags |= AIM_LASTENEMY; // KWo - 27.08.2006
                     pBot->f_heard_sound_time = gpGlobals->time;
//                   pBot->pBotEnemy = pEnt;
                     pBot->pLastEnemy = pEnt;
                     pBot->vecLastEnemyOrigin = pEnt->v.origin;
                     pBot->vecLastEnemyOrigin.x += RANDOM_FLOAT(-200.0f,200.0f);  // KWo - 12.08.2007
                     pBot->vecLastEnemyOrigin.y += RANDOM_FLOAT(-200.0f,200.0f);  // KWo - 12.08.2007
                     pBot->fLastHeardEnOrgUpdateTime = gpGlobals->time + 1.0f;

                     pBot->fAgressionLevel += 0.1f;   // KWo - 14.06.2018
                     if (pBot->fAgressionLevel > 1.0f)
                        pBot->fAgressionLevel = 1.0f;

                  }
               }
            }

            if (enemy_attack)  // KWo- 28.07.2018
            {
               if (pBot->pEdict->v.health > 70)
               {
                  pBot->fAgressionLevel += 0.1f;
                  if (pBot->fAgressionLevel > 1.0f)
                     pBot->fAgressionLevel = 1.0f;
               }
               else
               {
                  pBot->fFearLevel += 0.05f;
                  if (pBot->fFearLevel > 1.0f)
                     pBot->fFearLevel = 1.0f;
               }

               // Stop Bot from Hiding
               BotRemoveCertainTask (pBot, TASK_HIDE);

               // FIXFIXFIXFIXFIXME: THIS IS BLATANTLY CHEATING!!!!
               if ((FNullEnt (pBot->pBotEnemy)) && (pBot->f_bot_see_enemy_time + 1.0f < gpGlobals->time)
                  && (pBot->f_blind_time < gpGlobals->time) /* && (pBot->fLastSeenEnOrgUpdateTime < gpGlobals->time)
                   && (pBot->fLastHeardEnOrgUpdateTime < gpGlobals->time) */) // KWo - 08.04.2010
               {
                  pBot->pLastEnemy = pEnt;
                  pBot->vecLastEnemyOrigin = pEnt->v.origin;
                  pBot->vecLastEnemyOrigin.x += RANDOM_FLOAT(-200.0f,200.0f);  // KWo - 12.08.2007
                  pBot->vecLastEnemyOrigin.y += RANDOM_FLOAT(-200.0f,200.0f);  // KWo - 12.08.2007
//                  pBot->iAimFlags |= AIM_LASTENEMY; // KWo - 12.08.2007
                  pBot->f_heard_sound_time = gpGlobals->time;
                  pBot->fLastHeardEnOrgUpdateTime = gpGlobals->time + 1.0f;
                  pBot->iStates |= STATE_SUSPECTENEMY; // KWo - 08.04.2010
                  pBot->iStates |= STATE_HEARINGENEMY; // KWo - 08.04.2010
               }
            }
         }
         // Check old waypoint
         else
         {
            BotCollectGoalExperience (pBot, damage_taken);
            if (!WaypointReachableByEnt (pEdict->v.origin, pBot->dest_origin, pBot->pEdict)) // KWo - 30.07.2006
            {
               DeleteSearchNodes (pBot);
               BotFindWaypoint (pBot);
            }
         }

         if ((!team_attack) && (pEnt != pEdict))  // KWo - 28.07.2018
         {
            BotCollectGoalExperience (pBot, damage_taken + damage_taken); // KWo - 28.07.2018 - moved now here...
            BotCollectExperienceData (pEdict, pEnt, damage_armor + damage_taken);
         }
      }
   }
}


void BotClient_CS_Money (void *p, int bot_index)
{
   // This message gets sent when the bots money ammount changes

   if (state == 0)
      bots[bot_index].bot_money = *(int *) p;  // amount of money
}

void BotClient_CS_FlashBat (void *p, int bot_index)
{
   // This message gets sent when the battery for flashlight ammount changes

   if (state == 0)
   {
      bots[bot_index].cFlashBat = *(char *) p;  // amount of battery

      if (g_b_DebugCombat)
         ALERT(at_logged,"[DEBUG] Bot %s - battery of the flashlight = %d.\n", bots[bot_index].name, bots[bot_index].cFlashBat);
   }
}

void BotClient_CS_NVGToggle (void *p, int bot_index)
{
   // This message gets sent when the night googles are on/off

   if (state == 0)
      bots[bot_index].bUsesNVG = ((*(char *) p) > 0);  // nvg on/off
}

void BotClient_CS_DeathMsg (void *p, int bot_index)
{
   // This message gets sent when someone got killed

   static int killer_index;
   static int victim_index;
   static edict_t *killer_edict;
   static edict_t *victim_edict;    // KWo - 19.02.2006
   static bot_t *pBot;
   static int i;
   static int index;
   static bool killed_defuser;
//   static float killer_distance;    // KWo - 15.04.2010
//   static float lastenemy_distance; // KWo - 15.04.2010
   static float teamnate_distance;  // KWo - 15.04.2010

   if (state == 0)
      killer_index = *(int *) p; // ENTINDEX() of killer
   else if (state == 1)
      victim_index = *(int *) p; // ENTINDEX() of victim
   else if (state == 2)
   {
      if ((killer_index != 0) && (victim_index != 0) && (killer_index != victim_index))
      {
         killer_edict = INDEXENT (killer_index);
         pBot = UTIL_GetBotPointer (killer_edict);

         // is this message about a bot who killed somebody ?
         if (pBot != NULL)
         {
            pBot->pLastVictim = INDEXENT (victim_index);
         }

         else // Did a human kill a Bot on his team ?
         {
            victim_edict = INDEXENT (victim_index);
            pBot = UTIL_GetBotPointer (victim_edict);  // KWo - 19.02.2006

            if (pBot != NULL)
               pBot->bDead = TRUE;
         }
      }
      if (victim_index != 0)  // KWo 19.02.2006
      {
         victim_edict = INDEXENT (victim_index);
         pBot = UTIL_GetBotPointer (victim_edict);

         if ((victim_index > 0) && (victim_index <= gpGlobals->maxClients)) // KWo - 15.03.2010
         {
            clients[victim_index - 1].fDeathTime = gpGlobals->time + RANDOM_FLOAT (0.3f, 1.0f);
            clients[victim_index - 1].fTimeSoundLasting = gpGlobals->time - 2.0f; // KWo - 16.04.2010
         }

         killed_defuser = FALSE;
         if (pBot != NULL)
         {
            if (victim_index - 1 == g_iDefuser) // KWo - 13.07.2007
            {
               g_bBombDefusing = false;
               g_iDefuser = -1;
               killed_defuser = TRUE;
            }

            pBot->current_weapon.iAmmo1 = 0; // KWo - 25.12.2006
            pBot->current_weapon.iAmmo2 = 0; // KWo - 25.12.2006
/*
            memset (pBot->m_rgAmmoInClip, 0, sizeof (pBot->m_rgAmmoInClip)); // KWo - 25.12.2006
            memset (pBot->m_rgAmmo, 0, sizeof (pBot->m_rgAmmo));  // KWo - 25.12.2006
*/

            for (i = 0; i < MAX_WEAPONS; i++)
            {
               pBot->m_rgAmmoInClip[i] = 0;
            }
            for (i = 0; i < MAX_AMMO_SLOTS; i++)
            {
               pBot->m_rgAmmo[i] = 0;
            }
         }


         // Well - we should check that shootatdead_time, but need more logic (to get it working
         // correctly with CSDM) than it was written until now, so better just clear enemy in tables - KWo
         for (index = 0; index < gpGlobals->maxClients; index++)
         {
            if (bots[index].is_used && !bots[index].bDead)
            {
/*
               if ((bots[index].pLastEnemy == victim_edict) && (bots[index].pBotEnemy == NULL)) // KWo - 14.03.2010
               {
                  bots[index].iAimFlags &= ~AIM_ENEMY;
                  bots[index].iAimFlags &= ~AIM_LASTENEMY;
                  bots[index].iAimFlags &= ~AIM_PREDICTPATH;
                  bots[index].pLastEnemy = NULL;
                  bots[index].pBotEnemy = NULL;
               }
*/

               // that should simulate when the bot can see a dieing teamnate...
               if ((pBot != NULL)
                   && (killer_index > 0) && (killer_index <= gpGlobals->maxClients)
                   && (killer_index != victim_index)) // 20.05.2010
               {
                  teamnate_distance = (bots[index].pEdict->v.origin - victim_edict->v.origin).Length();
                  if (teamnate_distance < 500.0f)
                  {
                     if (bots[index].pLastEnemy == NULL)
                     {
                        bots[index].pLastEnemy = killer_edict;
                        bots[index].vecLastEnemyOrigin = killer_edict->v.origin;
                        bots[index].vecLastEnemyOrigin.x += RANDOM_FLOAT(-200.0f,200.0f);
                        bots[index].vecLastEnemyOrigin.y += RANDOM_FLOAT(-200.0f,200.0f);
                     }
                     bots[index].fFearLevel += 0.2f;
                     if (bots[index].fFearLevel > 1.0f)
                        bots[index].fFearLevel = 1.0f;
/*
                     else if (bots[index].f_bot_see_new_enemy_time + 1.0f < gpGlobals->time)
                     {
                        killer_distance = (killer_edict->v.origin - bots[index].pEdict->v.origin).Length();
                        lastenemy_distance = (bots[index].pEdict->v.origin - bots[index].vecLastEnemyOrigin).Length();
                        if (killer_distance < lastenemy_distance)
                        {
                           bots[index].pLastEnemy = killer_edict;
                           bots[index].vecLastEnemyOrigin = killer_edict->v.origin;
                           bots[index].vecLastEnemyOrigin.x += RANDOM_FLOAT(-200.0f,200.0f);
                           bots[index].vecLastEnemyOrigin.y += RANDOM_FLOAT(-200.0f,200.0f);
                        }
                     }
*/
                  }
               }

               if ((bots[index].pHuntEnemy == victim_edict) && (bots[index].pHuntEnemy == NULL)) // KWo - 13.04.2010
               {
                  bots[index].pHuntEnemy = NULL;
                  bots[index].vecHuntEnemyOrigin = g_vecZero;
               }

               // if the bot victim was a defuser, we need stop camping other CTs and defuse the bomb instead the victim...
               if (killed_defuser) // KWo - 14.08.2008
               {
                  if ((bots[index].bot_team == TEAM_CS_COUNTER) && bots[index].bDefendedBomb)
                  {
                     bots[index].bDefendedBomb = FALSE;
                     BotResetTasks (&bots[index]);
                  }
               }
            }
         }
      }
   }
}


void BotClient_CS_ScreenFade (void *p, int bot_index)
{
   // This message gets sent when the Screen fades (Flashbang)

   static short int f;     // KWo - 10.05.2007
   static bool inf_fade;   // KWo - 10.05.2007
   static unsigned char r;
   static unsigned char g;
   static unsigned char b;
   static bot_t *pBot;
   pBot = &bots[bot_index];

	if (state == 2)         // KWo - 10.05.2007
   {
      f = *(short int *) p;
      if (f & (1 << 2))
         inf_fade = true;
      else
         inf_fade = false ;
   }
   else if (state == 3)
      r = *(unsigned char *) p;
   else if (state == 4)
      g = *(unsigned char *) p;
   else if (state == 5)
   {
      b = *(unsigned char *) p;

      if (g_b_DebugCombat)
         ALERT(at_logged,"[DEBUG] CS_ScreenFade - bot %s, r = %d, g = %d, b = %d.\n",
            pBot->name, r, g, b);
   }
   else if (state == 6)
   {
      unsigned char alpha = *(unsigned char *) p;

      if ((r == 255) && (g == 255) && (b == 255) && (alpha > 180)) // KWo - 23.03.2008
      {
         pBot->pBotEnemy = NULL;
         pBot->f_view_distance = 16; // KWo - 04.10.2009

         // About 3 seconds
         if (inf_fade)
			{
            pBot->f_blind_time = 3.4e+38; // KWo - 10.05.2007 infinite blind...

            if (g_b_DebugCombat)
               ALERT(at_logged,"[DEBUG] CS_ScreenFade - Bot %s got infinite blinded.\n", pBot->name);
			}
         else
         {
            pBot->f_blind_time = gpGlobals->time + ((float) alpha - 180.0f) / 10;  // KWo - 23.03.2008
            pBot->fChangeAimDirectionTime = pBot->f_blind_time;
         }

         if (pBot->bot_skill < 50)
         {
            pBot->f_blindmovespeed_forward = 0.0f;
            pBot->f_blindmovespeed_side = 0.0f;
         }
         else if (pBot->bot_skill < 80)
         {
            pBot->f_blindmovespeed_forward = -pBot->pEdict->v.maxspeed;
            pBot->f_blindmovespeed_side = 0.0f;
         }
         else if (pBot->f_blindmove_time + 2.0f < gpGlobals->time)  // KWo - 13.08.2008
         {
            pBot->f_blindmove_time = gpGlobals->time;
            if (RANDOM_LONG (1, 100) < 50)
            {
               if (RANDOM_LONG (1, 100) < 50)
                  pBot->f_blindmovespeed_side = pBot->pEdict->v.maxspeed;
               else
                  pBot->f_blindmovespeed_side = -pBot->pEdict->v.maxspeed;
            }
            else
            {
               if (pBot->pEdict->v.health > 80)
                  pBot->f_blindmovespeed_forward = pBot->pEdict->v.maxspeed;
               else
                  pBot->f_blindmovespeed_forward = -pBot->pEdict->v.maxspeed;
            }
         }
      }

      if (g_b_DebugCombat)
         ALERT(at_logged,"[DEBUG] CS_ScreenFade - bot %s, r = %d, g = %d, b = %d, alpha = %d.\n",
            pBot->name, r, g, b, alpha);

   }
}


void BotClient_CS_SayText (void *p, int bot_index)
{
   static unsigned char ucEntIndex;
   static bot_t *pBot;

   if (g_bIsOldCS15)
   {
      if (state == 0)
         ucEntIndex = *(unsigned char *) p;
      else if (state == 1)
      {
         pBot = &bots[bot_index];

         if (ENTINDEX (pBot->pEdict) != ucEntIndex)
         {
            pBot->SaytextBuffer.iEntityIndex = (int) ucEntIndex;
            strncpy_s (pBot->SaytextBuffer.szSayText, sizeof(pBot->SaytextBuffer.szSayText), (char *) p, sizeof (pBot->SaytextBuffer.szSayText) - 1);
            pBot->SaytextBuffer.szSayText[sizeof (pBot->SaytextBuffer.szSayText) - 1] = 0;
            pBot->SaytextBuffer.fTimeNextChat = gpGlobals->time + pBot->SaytextBuffer.fChatDelay;
         }
      }
   }
   else
   {
      if (state == 0)
         ucEntIndex = *(unsigned char *) p;
      if (state == 1)
      {
         //ucEntIndex = *(unsigned char *) p;
      }
      else if (state == 3)
      {
         pBot = &bots[bot_index];

         if (ENTINDEX (pBot->pEdict) != ucEntIndex)
         {
            pBot->SaytextBuffer.iEntityIndex = (int) ucEntIndex;
            strncpy_s (pBot->SaytextBuffer.szSayText, sizeof(pBot->SaytextBuffer.szSayText), (char *) p, sizeof (pBot->SaytextBuffer.szSayText) - 1);
            pBot->SaytextBuffer.szSayText[sizeof (pBot->SaytextBuffer.szSayText) - 1] = 0;
            pBot->SaytextBuffer.fTimeNextChat = gpGlobals->time + pBot->SaytextBuffer.fChatDelay;
         }
      }
   }
}


void BotClient_CS_HLTV (void *p, int bot_index)
{
   // This message gets sent when the round restarts in CS 1.6, among other things.
   // Courtesy of stefanhendriks...

   static int players;

   if (state == 0)
      players = *(int *) p;
   else if (state == 1)
   {
      // new round in CS 1.6
      if ((players == 0) && (*(int *) p == 0))
         UTIL_RoundStart ();
   }
}


void BotClient_CS_BombDrop (void *p, int bot_index)
{
   static edict_t *pent;
   static bot_t *pBot;
   static int iIndex; // KWo - 28.08.2006

   pent = NULL;
   pBot = &bots[bot_index];

   // is the bot receiving this message alive and T ?
   if (pBot->is_used && !pBot->bDead && (pBot->bot_team == TEAM_CS_TERRORIST))
   {
      BotRemoveCertainTask (pBot, TASK_CAMP);
      DeleteSearchNodes (pBot); // make all Ts reevaluate their paths immediately
      pBot->iCampButtons = 0; // KWo - 17.02.2008
      pBot->f_ducktime = 0.0f; // KWo - 24.01.2012
      // find the bomb
//      while (!FNullEnt (pent = FIND_ENTITY_BY_CLASSNAME (pent, "weaponbox")))
      while (!FNullEnt (pent = FIND_ENTITY_BY_CLASSNAME (pent, "weapon_c4"))) // KWo - 26.08.2006
      {
//         if (FStrEq (STRING (pent->v.model), "models/w_backpack.mdl"))
         if (pent->v.origin != g_vecZero) // KWo - 26.08.2006
         {
            pBot->vecPosition = pent->v.origin;
            iIndex = WaypointFindNearestToMove (pBot->pEdict, pBot->vecPosition); // KWo - 28.08.2006 - if it will be working correctly , I can randomize the position a bit...
            // Push move task on to stack
            bottask_t TempTask = {NULL, NULL, TASK_MOVETOPOSITION, TASKPRI_MOVETOPOSITION, iIndex, 0.0, TRUE};   // KWo - 28.08.2006
            BotPushTask (pBot, &TempTask);
            break;
         }
      }
   }
   return;
}


void BotClient_CS_BombPickup (void *p, int bot_index)
{
   static bot_t *pBot;
   pBot = &bots[bot_index];

   if (pBot->is_used && !pBot->bDead && (pBot->bot_team == TEAM_CS_TERRORIST))  // KWo - 14.04.2006
   {
      DeleteSearchNodes (pBot); // make all Ts reevaluate their paths immediately
      BotResetTasks (pBot); // barbarian, but fits the job perfectly.
      pBot->iCampButtons = 0; // KWo - 17.02.2008
      pBot->f_ducktime = 0.0f; // KWo - 24.01.2012
   }
   return;
}


void BotClient_CS_TextMsgAll (void *p, int bot_index)
{
   int i;
   static bot_t *pBot;

   // Check if it's the "Bomb Planted" Message
   if ((state == 1) && (strcmp ("#Bomb_Planted", (char *) p) == 0))
   {
      g_bBombPlanted = g_bBombSayString = TRUE;
      g_fTimeBombPlanted = gpGlobals->time;

      for (i = 0; i < gpGlobals->maxClients; i++)
      {
         pBot = &bots[i];

         if (pBot->is_used && !pBot->bDead && (pBot->bot_team == TEAM_CS_COUNTER))
         {
            DeleteSearchNodes (pBot); // make all CTs reevaluate their paths immediately
            BotResetTasks (pBot); // barbarian, but fits the job perfectly.
            pBot->iCampButtons = 0; // KWo - 17.02.2008
            pBot->f_ducktime = 0.0f; // KWo - 24.01.2012
         }
      }
   }
   // Check if it's the "Game Commencing " Message  KWo - 09.02.2006
   if ((state == 1) && (strcmp ("#Game_Commencing", (char *) p) == 0))
   {
      UTIL_GameStarted ();
   }
/*
   if ((state == 2) && ((strcmp ("Terrorists_Win", (char *) p) == 0) || (strcmp ("CTs_Win", (char *) p) == 0)))
   {
      UTIL_RoundEnd ();
   }
*/
}

void BotClient_CS_TextMsg1 (void *p, int bot_index)
{
//   if ((strcmp ("Touched_A_Hostage", (char *) p) == 0) && (bot_index >= 0) && (bot_index < gpGlobals->maxClients))
//      UTIL_ServerPrint("[DEBUG] Bot %s in state %d started to use a hostage catched!!! :).\n", bots[bot_index].name , state);
}


void BotClient_CS_TeamScore (void *p, int bot_index)
{
   if ( /*(state == 1) && */ (strcmp ("TERRORIST", (char *) p) == 0))
      UTIL_RoundEnd ();
}


void BotClient_CS_ResetHUD (void *p, int bot_index)
{
   // bot respawned
   if (bots[bot_index].is_used)
      BotSpawnInit (&bots[bot_index]);
   g_bResetHud = TRUE;
}


void Client_CS_ScoreInfo (void *p, int/* iPlayerIndex*/)
{
// Commented out because we don't need this anymore. - KWo - 12.02.2006 & THE STORM
   // This message is sent whenever information on the ScoreBoard is changed
/*
   static int iPlayerIndex;

   switch (state)
   {
   case 0:
      iPlayerIndex = *(int *)p;
      break;
   case 4:
      if ((iPlayerIndex > 0) && (iPlayerIndex <= gpGlobals->maxClients))
      {
         clients[iPlayerIndex - 1].iTeam = *(int *)p;
      }
   }
*/
}

void Client_CS_ScoreAttrib (void *p, int/* iPlayerIndex*/)
{
/*
*/
}

// This message is sent whenever information for the teams is sent. - KWo - 12.02.2006 & THE STORM
void Client_CS_TeamInfo (void *p, int/* iPlayerIndex*/)
{
   static int iPlayerIndex;

   switch (state)
   {
   case 0:
      iPlayerIndex = *(int *)p - 1;
      break;
   case 1:
      if ((strcmp((char *) p, "UNASSIGNED") == 0) && (iPlayerIndex >= 0) && (iPlayerIndex < gpGlobals->maxClients))
      {
         clients[iPlayerIndex].iTeam = TEAM_CS_UNASSIGNED;
         if (bots[iPlayerIndex].is_used)
            bots[iPlayerIndex].bot_team = TEAM_CS_UNASSIGNED;
		}
      else if ((strcmp((char *) p, "TERRORIST") == 0) && (iPlayerIndex >= 0) && (iPlayerIndex < gpGlobals->maxClients))
      {
         clients[iPlayerIndex].iTeam = TEAM_CS_TERRORIST;
         if (bots[iPlayerIndex].is_used)
            bots[iPlayerIndex].bot_team = TEAM_CS_TERRORIST;
      }
      else if ((strcmp((char *) p, "CT") == 0) && (iPlayerIndex >= 0) && (iPlayerIndex < gpGlobals->maxClients))
      {
         clients[iPlayerIndex].iTeam = TEAM_CS_COUNTER;
         if (bots[iPlayerIndex].is_used)
            bots[iPlayerIndex].bot_team = TEAM_CS_COUNTER;
      }
      else if ((strcmp((char *) p, "SPECTATOR") == 0) && (iPlayerIndex >= 0) && (iPlayerIndex < gpGlobals->maxClients))
      {
         clients[iPlayerIndex].iTeam = TEAM_CS_SPECTATOR;
         if (bots[iPlayerIndex].is_used)
            bots[iPlayerIndex].bot_team = TEAM_CS_SPECTATOR;
      }
/*
      if ((iPlayerIndex >= 0) && (iPlayerIndex < gpGlobals->maxClients))
      {
         if (bots[iPlayerIndex].is_used)
            ALERT(at_logged, "[DEBUG] Bot %s got TeamInfo message - team %d.\n",
               bots[iPlayerIndex].name, bots[iPlayerIndex].bot_team);
      }
*/
      break;
   }
}

void BotClient_CS_RoundTime  (void *p, int bot_index)
{
//   ALERT(at_logged,"[DEBUG] State of round started probably catched ;).\n");
//	UTIL_RoundStart();
}

void BotClient_CS_BarTime  (void *p, int bot_index) // KWo - 13.07.2007
{
   static int iTime;
   iTime = *(int*)p;
   if ( !iTime || !(clients[bot_index].iFlags & CLIENT_ALIVE) ) return;
   if ( iTime != 3 )
   {
      g_iDefuser = bot_index;
      g_bBombDefusing = TRUE;
//      ALERT(at_logged,"[DEBUG] %s is defusing the bomb.\n", STRING(clients[bot_index].pEdict->v.netname));
   }
}
