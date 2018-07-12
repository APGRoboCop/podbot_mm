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
// bot.cpp
//
// Features the core AI of the bot

#include "bot_globals.h"


void EstimateNextFrameDuration (void)
{
   // Rich 'TheFatal' Whitehouse's method for computing the msec value

   if (msecdel <= gpGlobals->time)
   {
      if (msecnum > 0)
         msecval = 475.0 / msecnum;

      msecdel = gpGlobals->time + 0.5; // next check in half a second
      msecnum = 0;
   }
   else
      msecnum++;

   if (msecval < 1)
      msecval = 1; // don't allow the msec delay to be null
   else if (msecval > 100)
      msecval = 100; // don't allow it to last longer than 100 milliseconds either

   return;
}


void BotSpawnInit (bot_t *pBot)
{
   // Initialises a Bot after Creation & at the start of each Round

   int i;

   // set the third party bot flag
   pBot->pEdict->v.flags |= FL_FAKECLIENT; // KWo - 28.04.2010

   // Delete all allocated Path Nodes
   DeleteSearchNodes (pBot);

   pBot->wpt_origin = g_vecZero;
   pBot->dest_origin = g_vecZero;
   pBot->str_l_origin = g_vecZero; // KWo - 14.09.2006
   pBot->str_r_origin = g_vecZero; // KWo - 14.09.2006
   pBot->curr_wpt_index = -1;
   pBot->curr_travel_flags = 0;
   pBot->bJumpDone = FALSE;
   pBot->vecDesiredVelocity = g_vecZero;
   pBot->prev_goal_index = -1;
   pBot->chosengoal_index = -1;
   for (i = 0; i < 5; i++)
      pBot->prev_wpt_index[i] = -1;
   pBot->f_wpt_timeset = gpGlobals->time + 5.0;
   pBot->f_wpt_tim_str_chg = -1.0; // KWo - 02.11.2009

   pBot->fRecoilTime = gpGlobals->time;

   pBot->rgvecRecoil[0] = g_vecZero;
   pBot->rgvecRecoil[1] = g_vecZero;
   pBot->rgvecRecoil[2] = g_vecZero;
   pBot->rgvecRecoil[3] = g_vecZero;
   pBot->rgvecRecoil[4] = g_vecZero;

   switch (pBot->bot_personality)
   {
   case 0: // Normal
      if (RANDOM_LONG (1, 100) > 50)
         pBot->byPathType = 1;
      else
         pBot->byPathType = 2;
      break;
   case 1: // Psycho
      pBot->byPathType = 0;
      break;
   case 2: // Coward
      pBot->byPathType = 2;
      break;
   }

   // Clear all States & Tasks
   pBot->iStates = 0;
   BotResetTasks (pBot);

   pBot->bCheckMyTeam = TRUE;

   pBot->bIsVIP = FALSE;
   pBot->bIsLeader = FALSE;
   pBot->fTimeTeamOrder = 0.0;

   char szBotModelName[64];
   strncpy (szBotModelName, (INFOKEY_VALUE (GET_INFOKEYBUFFER (pBot->pEdict), "model")), sizeof (szBotModelName));
   strncpy (pBot->sz_BotModelName, szBotModelName, sizeof (szBotModelName));

   pBot->fMinSpeed = 260.0;
   pBot->prev_speed = 0.0; // fake "paused" since bot is NOT stuck
//   pBot->v_prev_origin = Vector (9999.0, 9999.0, 9999.0);
   pBot->v_prev_origin = pBot->pEdict->v.origin;
   pBot->prev_time = gpGlobals->time + g_f_cv_FreezeTime + 1.0;  // KWo - 02.01.2010
   pBot->f_moved_distance = 15.0;             // KWo - 24.05.2010

   pBot->bCanDuck = FALSE;
   pBot->f_view_distance = 9999.0;     // KWo - 30.05.2010
   pBot->f_maxview_distance = 9999.0;  // KWo - 30.05.2010

   pBot->fButtonPushTime = 0.0;
   pBot->fButtonNoticedTime = 0.0;     // KWo - 14.07.2006
   pBot->bButtonPushDecided = FALSE;   // KWo - 09.07.2006
   pBot->bNewButtonNoticed = FALSE;    // KWo - 09.07.2006

   pBot->pBotPickupItem = NULL;
   pBot->pItemIgnore[0] = NULL;
   pBot->pItemIgnore[1] = NULL;
   pBot->pItemIgnore[2] = NULL;
   pBot->f_itemcheck_time = 0.0;
   pBot->f_timeDoorOpen = 0.0;
   pBot->f_timeHitDoor =  0.0;            // KWo - 25.04.2006
   pBot->bHitDoor = FALSE;                // KWo - 17.01.2010

   pBot->pShootBreakable = NULL;
   pBot->pBreakableIgnore = NULL;         // KWo - 12.03.2010
   pBot->vecBreakable = g_vecZero;

   BotResetCollideState (pBot);
   pBot->fNoCollTime = gpGlobals->time + g_f_cv_FreezeTime + 1.0;  // KWo - 02.01.2010
   pBot->bPlayerCollision = FALSE;            // KWo - 11.07.2006

   pBot->pBotEnemy = NULL;
   pBot->pLastVictim = NULL;
   pBot->pLastEnemy = NULL;
   pBot->pHuntEnemy = NULL;               // KWo - 11.04.2010
   pBot->vecLastEnemyOrigin = g_vecZero;
   pBot->vecHuntEnemyOrigin = g_vecZero;  // KWo - 11.04.2010
   pBot->vecVisPos = g_vecZero;           // KWo - 25.01.2008
   pBot->pTrackingEdict = NULL;
   pBot->fTimeNextTracking = 0.0;
   pBot->bEnemyReachable = FALSE;         // KWo - 10.09.2008
   pBot->fEnemyReachableTimer = 0.0;      // KWo - 10.09.2008

   pBot->iLiftUsageState = LIFT_NO_NEARBY;// KWo - 22.04.2006
   pBot->v_LiftTravelPos = g_vecZero;     // KWo - 22.04.2006
   pBot->pLift = NULL;                    // KWo - 18.04.2006
   pBot->f_UsageLiftTime = 0.0;           // KWo - 22.04.2006

   pBot->iAimFlags = 0;
   pBot->bCanChooseAimDirection = TRUE;
   pBot->vecLookAt = g_vecZero;
   pBot->vecThrow = g_vecZero;
   pBot->cFlashBat = 100;                 // KWo - 25.05.2008
   pBot->bUsesNVG = false;                // KWo - 26.05.2008

   pBot->vecEnemy = g_vecZero;
   pBot->vecEnemyRandomOffset = g_vecZero;// KWo - 04.10.2009
   pBot->vecGrenade = g_vecZero;
   pBot->vecEntity = g_vecZero;
   pBot->vecCamp = g_vecZero;

   pBot->fEnemyUpdateTime = gpGlobals->time + g_i_botthink_index * 0.002; // KWo - 25.03.2007
   pBot->f_enemy_surprise_time = 0.0;     // KWo - 25.08.2006
   pBot->f_bot_see_enemy_time = 0.0;
   pBot->f_bot_see_new_enemy_time = 0.0;  // KWo - 29.04.2008

   pBot->oldcombatdesire = 0.0;
   pBot->fEnemyOriginUpdateTime = 0.0;    // KWo - 26.03.2007
   pBot->fLastSeenEnOrgUpdateTime = 0.0;  // KWo - 05.05.2007
   pBot->fLastHeardEnOrgUpdateTime = 0.0; // KWo - 05.05.2007
   pBot->fShootThruHeardCheckTime = 0.0;  // KWo - 05.05.2007
   pBot->fShootThruSeenCheckTime = 0.0;   // KWo - 23.03.2008
   pBot->bShootThruSeen = FALSE;          // KWo - 05.05.2007
   pBot->bShootThruHeard = FALSE;         // KWo - 05.05.2007
   pBot->fChangeAimDirectionTime = 0.0;   // KWo - 09.12.2007
   pBot->randomized_angles = g_vecZero;   // KWo - 12.03.2010
   pBot->randomize_angles_time = 0.0;     // KWo - 30.12.2009

   pBot->player_target_time =  0.0;       // KWo - 30.12.2009

   pBot->pAvoidGrenade = NULL;
   pBot->pSmokeGrenade = NULL;            // KWo - 26.03.2007
   pBot->cAvoidGrenade = 0;
   pBot->fTimeAvoidGrenade = gpGlobals->time + g_i_botthink_index * 0.002; // KWo - 25.03.2007

   // Reset Damage
   pBot->iLastDamageType = -1;
   pBot->f_goal_value = 0.0; // KWo - 28.12.2009

   pBot->vecPosition = g_vecZero;

   pBot->f_ideal_reaction_time = BotSkillDelays[pBot->bot_skill / 20].fMinSurpriseDelay;
   pBot->f_actual_reaction_time = BotSkillDelays[pBot->bot_skill / 20].fMinSurpriseDelay;

   pBot->pBotUser = NULL;
   pBot->f_bot_use_time = 0.0;

   for (i = 0; i < MAX_HOSTAGES; i++)
      pBot->pHostages[i] = NULL;

   pBot->f_hostage_check_time = gpGlobals->time;
   pBot->fTimeHostageRescue = 0.0; // KWo - 27.02.2007

   pBot->bIsReloading = FALSE;
   pBot->f_shoot_time = 0.0; // KWo - 23.10.2006
   pBot->fTimeSilencerSwitch = 0.0; // KWo - 08.01.2007
   pBot->fTimeLastFired = 0.0;
   pBot->iBurstShotsFired = 0;
   pBot->fTimeFirePause = 0.0;
   pBot->fTimeWeaponSwitch = 0.0;     // KWo - 23.12.2006
   pBot->f_primary_charging = -1.0;
   pBot->f_secondary_charging = -1.0;

   pBot->f_grenade_check_time = gpGlobals->time + g_i_botthink_index * 0.01; // KWo - 25.03.2007
   pBot->bUsingGrenade = FALSE;

   pBot->charging_weapon_id = 0;

   pBot->f_blind_time = 0.0;
   pBot->f_blindmovespeed_forward = 0.0;  // KWo - 13.08.2008
   pBot->f_blindmovespeed_side = 0.0;     // KWo - 13.08.2008
   pBot->f_blindmove_time = 0.0;          // KWo - 13.08.2008
   pBot->bShootLastPosition = FALSE;      // KWo - 13.08.2008
   pBot->bMadShoot = FALSE;               // KWo - 13.08.2008

   pBot->f_jumptime = 0.0;
   pBot->f_ducktime = 0.0;             // KWo - 20.10.2006
   pBot->f_falldowntime = 0.0;         // KWo - 05.03.2010
   pBot->fNextCampDirectionTime = 0.0; // KWo - 27.02.2007
   pBot->fTimeCamping = 0.0;           // KWo - 27.02.2007
   pBot->iCampButtons = 0;             // KWo - 17.02.2008

   pBot->f_sound_update_time = gpGlobals->time;
   pBot->f_heard_sound_time = 0.0;

   pBot->fTimePrevThink = gpGlobals->time;
   pBot->fTimePrevThink2 = gpGlobals->time;
   pBot->fTimeFrameInterval = 0.0;

   pBot->i_msecval = 0;                // KWo - 17.03.2007
   pBot->f_msecvalrest = 0.0;          // KWo - 17.03.2007

   pBot->b_bomb_blinking = FALSE;

   pBot->SaytextBuffer.fTimeNextChat = gpGlobals->time;
   pBot->SaytextBuffer.iEntityIndex = -1;
   pBot->SaytextBuffer.szSayText[0] = 0x0;

   pBot->iBuyCount = 1;
//   pBot->f_buy_time = gpGlobals->time + RANDOM_FLOAT (0.5f, (g_f_cv_FreezeTime >= 2.f)? g_f_cv_FreezeTime : 2.f);
   pBot->f_buy_time = gpGlobals->time + RANDOM_FLOAT (0.5, 1.0);

   pBot->f_zoomchecktime = 0.0;
   pBot->f_reloadingtime = 0.0;
   pBot->iNumWeaponPickups = 0;

   pBot->f_StrafeSetTime = 0.0;
   pBot->byCombatStrafeDir = 0;
   pBot->byFightStyle = 0;
   pBot->f_lastfightstylecheck = 0.0;

   pBot->bCheckWeaponSwitch = TRUE;

   pBot->pRadioEntity = NULL;
   pBot->iRadioOrder = 0;

   pBot->bLogoSprayed = FALSE;
   pBot->bDefendedBomb = FALSE;

   // KWo - 23.03.2007 - for caching traceline...
   pBot->pHit = NULL;

   pBot->f_spawn_time = gpGlobals->time;
   pBot->f_lastchattime = gpGlobals->time;
   pBot->pEdict->v.v_angle.y = pBot->pEdict->v.ideal_yaw;

   for (i = 0; i < 2; i++)
   {
      pBot->rgfYawHistory[i] = 0;
      pBot->rgfPitchHistory[i] = 0;
   }

   // Clear its Message Queue
   for (i = 0; i < 32; i++)
      pBot->aMessageQueue[i] = 0;

   pBot->iActMessageIndex = 0;
   pBot->iPushMessageIndex = 0;

   bottask_t TempTask = {NULL, NULL, TASK_NORMAL, TASKPRI_NORMAL, -1, 0.0, TRUE};
   BotPushTask (pBot, &TempTask);
   return;
}


void BotCreate (int bot_skill, int bot_personality, int bot_team, int bot_class, const char *bot_name)
{
   // This Function creates the Fakeclient (Bot) - passed Arguments:
   // arg1 - Skill
   // arg2 - Personality
   // arg3 - Team
   // arg4 - Class (Model)
   // arg5 - Botname

   edict_t *pPlayer;
   edict_t *BotEnt;
   bot_t *pBot;
   char c_name[BOT_NAME_LEN + 1];
   int skill;
   int index;
   int name_index;
   char ptr[128]; // allocate space for message from ClientConnect
   char *infobuffer;
   int iUsedCount;
   bool is_used;
   int iMaxBotSkill = (int) g_rgcvarPointer[PBCVAR_MAXBOTSKILL]->value;	// KWo - to remove warning
   int iMinBotSkill = (int) g_rgcvarPointer[PBCVAR_MINBOTSKILL]->value;	// KWo - to remove warning
   iMaxBotSkill = (iMaxBotSkill > 100) ? 100 : (iMaxBotSkill < 1) ? 1 : iMaxBotSkill;
   iMinBotSkill = (iMinBotSkill > 100) ? 100 : (iMinBotSkill < 1) ? 1 : iMinBotSkill;
   if (iMaxBotSkill < iMinBotSkill)
      iMinBotSkill = iMaxBotSkill;

   // Don't allow creating Bots when no waypoints are loaded
   if (g_iNumWaypoints < 1)
   {
      UTIL_ServerPrint ("No Waypoints for this Map, can't create Bot !\n");
      memset (BotCreateTab, 0, sizeof (BotCreateTab));
      botcreation_time = 0.0;
      return;
   }

   // Don't allow creating Bots when max_bots is reached
   else if ((((g_i_cv_BotsQuotaMatch == 0) && (g_iNum_players >= g_iMax_bots))
             || ((g_i_cv_BotsQuotaMatch > 0) && (g_iNum_bots > g_iMax_bots)))
             && (g_iMax_bots > 0) && (g_iNum_bots >= g_iMin_bots))  // KWo - 08.03.2007
   {
      UTIL_ServerPrint ("Max Bots reached, can't create Bot !\n");
      memset (BotCreateTab, 0, sizeof (BotCreateTab));
      botcreation_time = 0.0;
      return;
   }

   // if Waypoints have changed don't allow it because Distance Tables are messed up
   else if (g_bWaypointsChanged)
   {
      UTIL_ServerPrint ("Waypoints changed/not initialised, can't create Bot !\n");
      memset (BotCreateTab, 0, sizeof (BotCreateTab));
      botcreation_time = 0.0;
      return;
   }

   // If Skill is given, assign it
   if ((bot_skill > 0) && (bot_skill <= 100))
      skill = bot_skill;
   else
      skill = RANDOM_LONG (iMinBotSkill, iMaxBotSkill); // else give random skill

   assert ((skill > 0) && (skill <= 100));

   // Create Random Personality
   if ((bot_personality < 0) || (bot_personality > 2))
      bot_personality = RANDOM_LONG (0, 2);

   if ((bot_team != TEAM_CS_TERRORIST) && (bot_team != TEAM_CS_COUNTER))
      bot_team = 5;

   if (g_i_cv_BotsJoinTeam > 0) // KWo - 16.09.2006
      bot_team = g_i_cv_BotsJoinTeam;

   if ((g_i_cv_skin > 0) && (g_i_cv_skin < 5)) // 18.11.2006
      bot_class = g_i_cv_skin;
   if ((bot_class < 1) || (bot_class > 5))
      bot_class = 5;

   // If No Name is given, do our name stuff
   if ((bot_name == NULL) || (*bot_name == 0))
   {
      // If as many Bots as NumBotnames, don't allow Bot Creation
      if (g_iNum_bots >= iNumBotNames)
      {
         UTIL_ServerPrint ("Not enough Bot Names in botnames.txt, can't create Bot !\n");
         memset (BotCreateTab, 0, sizeof (BotCreateTab));
         botcreation_time = 0.0;
         return;
      }

      // Clear Array of used Botnames
      memset (szUsedBotNames, 0, sizeof (szUsedBotNames));

      // Cycle through all Players in Game and pick up Bots Names
      iUsedCount = 0;
      for (index = 1; index <= gpGlobals->maxClients; index++)
      {
         pPlayer = INDEXENT (index);

         if (!FNullEnt (pPlayer) && (pPlayer->v.flags & FL_FAKECLIENT) )
         {
            const char* bname = STRING(pPlayer->v.netname);
            if (!FStrEq(bname, ""))	// KWo - to remove warning cast to a pointer
//             && (STRING (pPlayer->v.netname)[0] != 0))
               szUsedBotNames[iUsedCount++] = STRING (pPlayer->v.netname);
         }
      }

      // Find a Botname from Botnames.txt which isn't used yet
      do
      {
         name_index = RANDOM_LONG (0, iNumBotNames - 1);

         if (iUsedCount == 0)
            is_used = FALSE;
         else
         {
            is_used = FALSE;

            for (index = 0; index < iUsedCount; index++)
               if (strstr (szUsedBotNames[index], (g_pszBotNames + name_index)->name) != NULL)
                  is_used = TRUE;
         }
      }
      while (is_used);

      // If Detailnames are on, attach Clan Tag
      if (g_i_cv_detailnames)
      {
         if (g_i_cv_detailnames == 1) // use prefix and sufix
         {
            if (bot_personality == 0)
               snprintf (c_name, sizeof (c_name), "[POD]%s (%d)", (g_pszBotNames + name_index)->name, skill);
            else if (bot_personality == 1)
               snprintf (c_name, sizeof (c_name), "[P*D]%s (%d)", (g_pszBotNames + name_index)->name, skill);
            else
               snprintf (c_name, sizeof (c_name), "[P0D]%s (%d)", (g_pszBotNames + name_index)->name, skill);
         }
         if (g_i_cv_detailnames == 2) // use prefix only
         {
            if (bot_personality == 0)
               snprintf (c_name, sizeof (c_name), "[POD]%s", (g_pszBotNames + name_index)->name);
            else if (bot_personality == 1)
               snprintf (c_name, sizeof (c_name), "[P*D]%s", (g_pszBotNames + name_index)->name);
            else
               snprintf (c_name, sizeof (c_name), "[P0D]%s", (g_pszBotNames + name_index)->name);
         }
         if (g_i_cv_detailnames == 3) // use sufix only
         {
            snprintf (c_name, sizeof (c_name), "%s (%d)", (g_pszBotNames + name_index)->name, skill);
         }
      }
      else
         strncpy (c_name, (g_pszBotNames + name_index)->name, sizeof (c_name));
   }

   // a name has been given
   else
   {
      // If Detailnames are on, see if we NEED to attach Clan Tag and skill
      if (g_i_cv_detailnames) // KWo - 22.03.2008 - added new meaning of g_i_cv_detailnames
      {
         if ((g_i_cv_detailnames == 1) || (g_i_cv_detailnames == 2))
         {
            if ((strstr (bot_name, "[POD]") != NULL)
                || (strstr (bot_name, "[P*D]") != NULL)
                || (strstr (bot_name, "[P0D]") != NULL)) // the bot already had a prefix
            {
               strncpy (c_name, bot_name, sizeof (c_name));

               if (bot_personality == 0)
                  c_name[2] = 'O';
               else if (bot_personality == 1)
                  c_name[2] = '*';
               else
                  c_name[2] = '0';
            }
            else // KWo - 07.07.2008
            {
               if ((strstr (bot_name, "(") != NULL) && (strstr (bot_name, ")") != NULL)) // the bot already had a sufix
               {
                  if ((g_i_cv_detailnames == 1) || (g_i_cv_detailnames == 2)) // use prefix and eventually existing sufix
                  {
                     if (bot_personality == 0)
                        snprintf (c_name, sizeof (c_name), "[POD]%s", bot_name);
                     else if (bot_personality == 1)
                        snprintf (c_name, sizeof (c_name), "[P*D]%s", bot_name);
                     else
                        snprintf (c_name, sizeof (c_name), "[P0D]%s", bot_name);
                  }
               }
               else
               {
                  if (g_i_cv_detailnames == 1) // use prefix and sufix
                  {
                     if (bot_personality == 0)
                        snprintf (c_name, sizeof (c_name), "[POD]%s (%d)", bot_name, skill);
                     else if (bot_personality == 1)
                        snprintf (c_name, sizeof (c_name), "[P*D]%s (%d)", bot_name, skill);
                     else
                        snprintf (c_name, sizeof (c_name), "[P0D]%s (%d)", bot_name, skill);
                  }
                  else if (g_i_cv_detailnames == 2) // use prefix only
                  {
                     if (bot_personality == 0)
                        snprintf (c_name, sizeof (c_name), "[POD]%s", bot_name);
                     else if (bot_personality == 1)
                        snprintf (c_name, sizeof (c_name), "[P*D]%s", bot_name);
                     else
                        snprintf (c_name, sizeof (c_name), "[P0D]%s", bot_name);
                  }
               }
            }
         }
         else // use sufix only
         {
            if ((strstr (bot_name, "(") == NULL) || (strstr (bot_name, ")") == NULL)) // KWo - 07.07.2008 the bot didn't have yet a sufix
               snprintf (c_name, sizeof (c_name), "%s (%d)", bot_name, skill);
            else
               snprintf (c_name, sizeof (c_name), "%s", bot_name);
         }
      }
      else
         strncpy (c_name, bot_name, sizeof (c_name));
   }
   c_name[sizeof (c_name) - 1] = 0;

   // This call creates the Fakeclient
//   BotEnt = (*g_engfuncs.pfnCreateFakeClient) (c_name);
   BotEnt = g_engfuncs.pfnCreateFakeClient (c_name);  // KWo - 11.02.2006
   // Did the Call succeed ?
   if (FNullEnt (BotEnt))
   {
      UTIL_ServerPrint ("Max Players reached. Can't create bot!\n");
      memset (BotCreateTab, 0, sizeof (BotCreateTab));
      botcreation_time = 0.0;
      return;
   }

   // YEP! Our little Bot has spawned !

   // Notify calling Player of the creation
   UTIL_ServerPrint ("Creating bot...\n");


   if (BotEnt->pvPrivateData != NULL)
      FREE_PRIVATE (BotEnt);
   BotEnt->pvPrivateData = NULL;
   BotEnt->v.frags = 0;

   // create the player entity by calling MOD's player function
   CALL_GAME_ENTITY (PLID, "player", &BotEnt->v);

   // Find a free slot in our Table of Bots
   index = ENTINDEX (BotEnt) - 1;
//   UTIL_ServerPrint ("Bot index = %i created...\n", index); // KWo - test

   // Set all Infobuffer Keys for this Bot
   infobuffer = GET_INFOKEYBUFFER (BotEnt);
   SET_CLIENT_KEYVALUE (index + 1, infobuffer, "model", "gordon");
   SET_CLIENT_KEYVALUE (index + 1, infobuffer, "rate", "3500.000000");
   SET_CLIENT_KEYVALUE (index + 1, infobuffer, "cl_updaterate", "20");
//   SET_CLIENT_KEYVALUE (index + 1, infobuffer, "cl_lw", "1"); // KWo - 14.09.2011 - thanks to Immortal_BLG
//   SET_CLIENT_KEYVALUE (index + 1, infobuffer, "cl_lc", "1"); // KWo - 14.09.2011 - thanks to Immortal_BLG
   SET_CLIENT_KEYVALUE (index + 1, infobuffer, "tracker", "0");
   SET_CLIENT_KEYVALUE (index + 1, infobuffer, "cl_dlmax", "128");
   SET_CLIENT_KEYVALUE (index + 1, infobuffer, "lefthand", "1");
   SET_CLIENT_KEYVALUE (index + 1, infobuffer, "friends", "0");
   SET_CLIENT_KEYVALUE (index + 1, infobuffer, "dm", "0");
   SET_CLIENT_KEYVALUE (index + 1, infobuffer, "_ah", "0");
   SET_CLIENT_KEYVALUE (index + 1, infobuffer, "_vgui_menus", "0");
   if ((!g_bIsOldCS15) && (g_i_cv_latencybot == 1))               // KWo - 02.03.2010
      SET_CLIENT_KEYVALUE (index + 1, infobuffer, "*bot", "1");

   // - End Infobuffer -

   // Connect this client with the local loopback
   MDLL_ClientConnect (BotEnt, c_name, "127.0.0.1", ptr);

   // set the third party bot flag
   BotEnt->v.flags |= FL_FAKECLIENT; // KWo - 08.03.2007

   // Pieter van Dijk - use instead of DispatchSpawn() - Hip Hip Hurray!
   MDLL_ClientPutInServer (BotEnt);

   // PMB - do this because MDLL_ClientPutInServer() does NOT call our own ClientPutInServer()
   clients[index].pEdict = BotEnt;
   clients[index].iFlags |= CLIENT_USED; // KWo - 08.03.2007

   // set the third party bot flag
   BotEnt->v.flags |= FL_FAKECLIENT;

   // initialize all the variables for this bot...
   pBot = &bots[index];
   memset (pBot, 0, sizeof (bot_t));
   pBot->pEdict = BotEnt;

   snprintf (pBot->name, sizeof (pBot->name), STRING (pBot->pEdict->v.netname));
   pBot->is_used = TRUE;
   pBot->not_started = 1; // hasn't joined game yet
   pBot->start_action = MSG_CS_IDLE;
   pBot->bot_money = 0;
   pBot->bDead = TRUE;
   pBot->bot_skill = skill;
   pBot->bot_personality = bot_personality;
   pBot->bUsesNVG = false;                // KWo - 28.05.2008

   // Assign a random Spraypaint
   pBot->iSprayLogo = RANDOM_LONG (0, g_iNumLogos - 1);

   // Assign how talkative this Bot will be
   pBot->SaytextBuffer.fChatDelay = RANDOM_FLOAT (3.0, 10.0);    // KWo - 10.03.2013
   pBot->SaytextBuffer.cChatProbability = RANDOM_LONG (40, 100);  // KWo - 31.03.2006

   BotEnt->v.idealpitch = BotEnt->v.v_angle.x;
   BotEnt->v.ideal_yaw = BotEnt->v.v_angle.y;
   BotEnt->v.yaw_speed = RANDOM_FLOAT (BotTurnSpeeds[skill / 20].fMinTurnSpeed,
      BotTurnSpeeds[skill / 20].fMaxTurnSpeed);
   BotEnt->v.pitch_speed = RANDOM_FLOAT (BotTurnSpeeds[skill / 20].fMinTurnSpeed,
      BotTurnSpeeds[skill / 20].fMaxTurnSpeed);

   // Set the Base Fear/Agression Levels for this Personality
   switch (bot_personality)
   {
      // Normal
   case 0:
      pBot->fBaseAgressionLevel = RANDOM_FLOAT (0.5, 0.6);
      pBot->fBaseFearLevel = RANDOM_FLOAT (0.5, 0.6);
      break;
      // Psycho
   case 1:
      pBot->fBaseAgressionLevel = RANDOM_FLOAT (0.7, 1.0);
      pBot->fBaseFearLevel = RANDOM_FLOAT (0.0, 0.4);
      break;
      // Coward
   case 2:
      pBot->fBaseAgressionLevel = RANDOM_FLOAT (0.0, 0.4);
      pBot->fBaseFearLevel = RANDOM_FLOAT (0.7, 1.0);
      break;
   }

   // Copy them over to the temp Level Variables
   pBot->fAgressionLevel = pBot->fBaseAgressionLevel;
   pBot->fFearLevel = pBot->fBaseFearLevel;
   pBot->fNextEmotionUpdate = gpGlobals->time + 0.5;

   // Just to be sure
   pBot->iActMessageIndex = 0;
   pBot->iPushMessageIndex = 0;

   // Assign Team & Class
   pBot->bot_team = bot_team;
   pBot->bot_class = bot_class;

   if (skill == 100)
     pBot->fOffsetUpdateTime = 0.0;  // enemy origin should be updated in aiming system all the time
   else
   {
      // botskill 99 -> update time = 0.05s, botskill 1 -> update time = 0.25s
      pBot->fOffsetUpdateTime = 0.25 + (skill-1) *(0.05-0.25)/(99.0);  // KWo - 27.04.2006
   }

   BotSpawnInit (pBot);

   return;
}


void BotStartGame (bot_t *pBot)
{
   // Handles the selection of Teams & Class

   // handle Counter-Strike stuff here...

   if (pBot->start_action == MSG_CS_MOTD)  // KWo - 11.02.2006
   {
      pBot->pEdict->v.button = IN_ATTACK;
      return;
   }

   else if (pBot->start_action == MSG_CS_TEAM_SELECT)
   {
      pBot->start_action = MSG_CS_IDLE; // switch back to idle

      if ((pBot->bot_team < 1) || (pBot->bot_team > 2))
         pBot->bot_team = 5;

      FakeClientCommand (pBot->pEdict, "menuselect %d\n", pBot->bot_team);
      return;
   }

   else if ((pBot->start_action == MSG_CS_CT_SELECT)
            || (pBot->start_action == MSG_CS_T_SELECT))
   {
      pBot->start_action = MSG_CS_IDLE; // switch back to idle

      if ((pBot->bot_class < 1) || (pBot->bot_class > 4))
         pBot->bot_class = RANDOM_LONG (1, 4); // use random if invalid

      FakeClientCommand (pBot->pEdict, "menuselect %d\n", pBot->bot_class);

      // bot has now joined the game (doesn't need to be started)
      pBot->not_started = FALSE;
      return;
   }
   return;
}


int BotGetMessageQueue (bot_t *pBot)
{
   // Get the current Message from the Bots Message Queue

   int iMSG;

   iMSG = pBot->aMessageQueue[pBot->iActMessageIndex++];
   pBot->iActMessageIndex &= 0x1f; // Wraparound

   return (iMSG);
}


void BotPushMessageQueue (bot_t *pBot, int iMessage)
{
   // Put a Message into the Message Queue

   pBot->aMessageQueue[pBot->iPushMessageIndex++] = iMessage;
   pBot->iPushMessageIndex &= 0x1f; // Wraparound

   return;
}


int BotInFieldOfView (bot_t *pBot, Vector dest)
{
   int angle;
   float entity_angle;
   float view_angle;

   // find angles from source to destination...
   entity_angle = UTIL_VecToAngles (dest).y;

   // make yaw angle 0 to 360 degrees if negative...
   if (entity_angle < 0)
      entity_angle += 360;

   // get bot's current view angle...
   view_angle = pBot->pEdict->v.v_angle.y;

   // make view angle 0 to 360 degrees if negative...
   if (view_angle < 0)
      view_angle += 360;

   // return the absolute value of angle to destination entity
   // zero degrees means straight ahead,  45 degrees to the left or
   // 45 degrees to the right is the limit of the normal view angle

   // rsm - START angle bug fix
   angle = abs ((int) view_angle - (int) entity_angle);

   if (angle > 180)
      angle = 360 - angle;

   return (angle);
   // rsm - END
}


bool BotItemIsVisible (bot_t *pBot, Vector vecDest, char *pszItemName, bool bBomb)
{
   TraceResult tr;

   // trace a line from bot's eyes to destination...
   TRACE_LINE (GetGunPosition (pBot->pEdict), vecDest, ignore_monsters, pBot->pEdict, &tr);

   // check if line of sight to object is not blocked (i.e. visible)
   if (tr.flFraction < 1.0) // KWo - 28.08.2008
   {
      if ((tr.flFraction > 0.97) && (FStrEq (STRING (tr.pHit->v.classname), pszItemName)))
         return (TRUE);

      if ((strncmp (pszItemName, "weaponbox", 9) == 0) && (tr.flFraction > (bBomb ? 0.80 : 0.95))) // KWo - 28.08.2008
         return (TRUE);

      if ((strncmp (pszItemName, "csdmw_", 6) == 0) && (tr.flFraction > 0.95)) // KWo - 18.11.2006
         return (TRUE);

      return (FALSE);
   }
   return (TRUE);
}


bool BotEntityIsVisible (bot_t *pBot, Vector vecDest)
{
   TraceResult tr;

   // trace a line from bot's eyes to destination...
   TRACE_LINE (GetGunPosition (pBot->pEdict), vecDest, ignore_monsters, pBot->pEdict, &tr);

   // check if line of sight to object is not blocked (i.e. visible)
   if (tr.flFraction < 0.92) // KWo - 14.09.2008
      return (FALSE);

   return (TRUE);
}

bool BotLastEnemyVisible (bot_t *pBot) // not used...
{
   // Checks if Line of Sight established to last Enemy

   edict_t *pEdict = pBot->pEdict;
   TraceResult tr;

   // trace a line from bot's eyes to destination...
   TRACE_LINE (GetGunPosition (pEdict),
               pBot->vecLastEnemyOrigin, ignore_monsters,
               pEdict, &tr);

   // check if line of sight to object is not blocked (i.e. visible)
   if (tr.flFraction >= 1.0)
      return (TRUE);

   return (FALSE);
}


bool BotLastEnemyShootable (bot_t *pBot)
{
   edict_t *pEdict = pBot->pEdict;
   float flDot;

   if (!(pBot->iAimFlags & AIM_LASTENEMY) || FNullEnt (pBot->pLastEnemy)
   /* || (BotGetSafeTask(pBot)->iTask == TASK_PAUSE) */) // KWo - 09.02.2008
      return (FALSE);
   if (pBot->vecLastEnemyOrigin == g_vecZero) // KWo - 14.10.2006
      return (FALSE);

   flDot = GetShootingConeDeviation (pEdict, &pBot->vecLastEnemyOrigin);
   if (flDot >= 0.90)
      return (TRUE);

   return (FALSE);
}

void BotCheckAvoidGrenades (bot_t *pBot)
{
   // Check if Bot 'sees' a SmokeGrenade to simulate the effect of being blinded by smoke
   // + notice Bot of Grenades flying towards him
   // TODO: Split this up and make it more reliable

   static edict_t *pEdict;
   static edict_t *pent;
   static edict_t *pSmoke; // KWo - 26.03.2007
   static Vector vecView;
   static float fDistance;
   static int iInField;

   pEdict = pBot->pEdict;
   pent = pBot->pAvoidGrenade;
   pSmoke = pBot->pSmokeGrenade;
   vecView = GetGunPosition (pEdict);

   // Check if old ptr to Grenade is invalid
   if (FNullEnt (pent))
   {
      pBot->pAvoidGrenade = NULL;
      pBot->cAvoidGrenade = 0;
   }

   else if ((pent->v.flags & FL_ONGROUND) || (pent->v.effects & EF_NODRAW))
   {
      pBot->pAvoidGrenade = NULL;
      pBot->cAvoidGrenade = 0;
   }

   if (FNullEnt (pSmoke)) // KWo - 26.03.2007
   {
      pSmoke = NULL;
      pBot->pSmokeGrenade = NULL;
   }
   else if (!(pSmoke->v.flags & FL_ONGROUND) || (pSmoke->v.effects & EF_NODRAW))
   {
      pSmoke = NULL;
      pBot->pSmokeGrenade = NULL;
   }

   if (!FNullEnt (pBot->pAvoidGrenade)) // KWo - 26.03.2007
   {
      Vector2D vec2DirToPoint;
      Vector2D vec2RightSide;

      // to start strafing, we have to first figure out if the target is on the left side or right side
      MAKE_VECTORS (pEdict->v.angles);

      vec2DirToPoint = (pEdict->v.origin - pent->v.origin).Make2D ().Normalize ();
      vec2RightSide = gpGlobals->v_right.Make2D ().Normalize ();

      if (DotProduct (vec2DirToPoint, vec2RightSide) > 0)
         pBot->cAvoidGrenade = -1;
      else
         pBot->cAvoidGrenade = 1;
//      ALERT(at_logged, "[DEBUG] Bot %s needs to avoid grenade.\n", pBot->name);
   }

   if (!FNullEnt (pSmoke)) // KWo - 13.09.2008
   {
      // Check if in field of view of the Bot
      iInField = BotInFieldOfView (pBot, pSmoke->v.origin - vecView);
      if (iInField < (pEdict->v.fov * 1.0 - 5.0)) // more wide - even +/-85 degress in this case...
      {
//         ALERT(at_logged, "[DEBUG] Bot %s checks a smoke grenade in his field of view.\n", pBot->name);
         fDistance = (pSmoke->v.origin - pEdict->v.origin).Length ();
         // Shrink Bots Viewing Distance to Smoke Grens Distance
         if (pBot->f_view_distance > fDistance)
            pBot->f_view_distance = fDistance;
      }
      else if (BotGetSafeTask(pBot)->iTask != TASK_BLINDED)
         pBot->f_view_distance = pBot->f_maxview_distance;
   }
   else if (BotGetSafeTask(pBot)->iTask != TASK_BLINDED)
      pBot->f_view_distance = pBot->f_maxview_distance;

   return;
}

int GetBestWeaponCarried (bot_t *pBot)
{
   // Returns the best weapon of this Bot (based on Personality Prefs)

   int *ptrWeaponTab = ptrWeaponPrefs[pBot->bot_personality];
   bot_weapon_select_t *pWeaponTab = &cs_weapon_select[0];
   int iWeaponIndex = 0;		// KWo - 23.12.2006
   int i;
   int iWeapId;

   int iWeapons = pBot->pEdict->v.weapons;

   if (BotHasShield (pBot))
      iWeapons |= (1 << CS_WEAPON_SHIELDGUN);

   for (i = 0; i < NUM_WEAPONS; i++)
   {
      iWeapId = pWeaponTab[*ptrWeaponTab].iId;
      if (iWeapons & (1 << iWeapId))
      {
         if ((WeaponIsPistol(iWeapId) || WeaponIsPrimaryGun(iWeapId))
            && ((pBot->m_rgAmmo[weapon_defs[iWeapId].iAmmo1] + pBot->m_rgAmmoInClip[iWeapId])
            >= pWeaponTab[*ptrWeaponTab].min_primary_ammo)) // KWo - 26.12.2006
         {
            iWeaponIndex = i;
         }
      }
      ptrWeaponTab++;
   }
   return (iWeaponIndex);
}


int GetBestSecondaryWeaponCarried (bot_t *pBot)
{
   // Returns the best secondary weapon of this Bot (based on Personality Prefs)

   int *ptrWeaponTab = ptrWeaponPrefs[pBot->bot_personality];
   bot_weapon_select_t *pWeaponTab = &cs_weapon_select[0];
   int iWeaponIndex = 0;		// KWo - 23.12.2006
   int i;
   int iWeapId;

   int iWeapons = pBot->pEdict->v.weapons;

   if (BotHasShield (pBot))
      iWeapons |= (1 << CS_WEAPON_SHIELDGUN);

   for (i = 0; i < NUM_WEAPONS; i++)
   {
      iWeapId = pWeaponTab[*ptrWeaponTab].iId;
      if ((iWeapons & (1 << iWeapId))
         && WeaponIsPistol (iWeapId)
            && ((pBot->m_rgAmmo[weapon_defs[iWeapId].iAmmo1] + pBot->m_rgAmmoInClip[iWeapId])
            >= pWeaponTab[*ptrWeaponTab].min_primary_ammo)) // KWo - 26.12.2006
      {
         iWeaponIndex = i;
      }
      ptrWeaponTab++;
   }

   return (iWeaponIndex);
}

int BotRateGroundWeapon (bot_t *pBot, edict_t *pent)
{
   // Compares Weapons on the Ground to the one the Bot is using

   int i;
   int iHasWeapon;
   int *ptrWeaponTab = ptrWeaponPrefs[pBot->bot_personality];
   bot_weapon_select_t *pWeaponTab = &cs_weapon_select[0];

   int iGroundIndexPref = 0;
   int iGroundWeaponTabIndex = 0;
   char szModelName[40];

   strncpy (szModelName, STRING (pent->v.model), sizeof (szModelName));
   szModelName[sizeof (szModelName) - 1] = 0;

   for (i = 0; i < NUM_WEAPONS; i++)
   {
      if (FStrEq (pWeaponTab[*ptrWeaponTab].model_name, szModelName))
      {
         iGroundIndexPref = i;
         iGroundWeaponTabIndex = *ptrWeaponTab; // KWo - 25.04.2013
         break;
      }

      ptrWeaponTab++;
   }

   // Check if pistol
   if (iGroundWeaponTabIndex < 7) // KWo - 25.04.2013
      iHasWeapon = GetBestSecondaryWeaponCarried (pBot);
   else
      iHasWeapon = GetBestWeaponCarried (pBot);

   if (g_b_DebugEntities)
      ALERT(at_logged, "[DEBUG] RateGroundWeapon - Bot %s has %s and found %s.\n", pBot->name,
         &cs_weapon_select[iGroundWeaponTabIndex].model_name, szModelName);

   return (iGroundIndexPref - iHasWeapon);
}


bool BotFindBreakable (bot_t *pBot)
{
   // Checks if Bot is blocked by a Shootable Breakable in his moving direction

   TraceResult tr;
   edict_t *pEdict = pBot->pEdict;
   Vector v_src;
   Vector v_dest;
   Vector vecDirection;

   for (int i = 0; i < 5; ++i)
   {
      if ( (i==1) && (pBot->pShootBreakable==NULL) ) // KWo - 28.01.2006
         continue;

      v_dest = pBot->wpt_origin; // KWo - 04.10.2007
      if ((pBot->curr_wpt_index >= 0) && (pBot->curr_wpt_index < g_iNumWaypoints)) // KWo - 15.04.2013
      {
         v_dest = paths[pBot->curr_wpt_index]->origin;
      }

      switch (i)
      {
      case 0:
         if ((pBot->prev_wpt_index[0] >= 0) && (pBot->prev_wpt_index[0] < g_iNumWaypoints))  // KWo - 13.02.2006
         {
            v_src = paths[pBot->prev_wpt_index[0]]->origin;
         }
         else
         {
            continue;
         }
         break;
      case 1:
         v_src = GetGunPosition (pEdict);
         v_dest = VecBModelOrigin (pBot->pShootBreakable);
         break;
      case 2:
         v_src = pEdict->v.origin;
         v_dest = pBot->dest_origin;
         break;
      case 3:
         v_src = GetGunPosition (pEdict);
         v_dest = pBot->dest_origin;
         break;
      case 4:
         v_src = GetGunPosition (pEdict);
         break;
      }

      vecDirection = (v_dest - v_src).Normalize ();
      v_dest = v_src + vecDirection * 256;  // KWo - 04.10.2007

      if (g_b_DebugEntities)
      {
         ALERT(at_logged, "[DEBUG] Bot %s - v_src = (%i, %i, %i) ; v_dest = (%i, %i, %i).\n",
            pBot->name, (int)v_src.x, (int)v_src.y, (int)v_src.z, (int)v_dest.x, (int)v_dest.y, (int)v_dest.z);
      }

      TRACE_LINE (v_src, v_dest, dont_ignore_monsters, pEdict, &tr);
      if (FNullEnt (tr.pHit)) // KWo - 28.01.2006
         continue;
      if ((tr.flFraction != 1.0) && ((pEdict->v.origin - tr.vecEndPos).Length() < 120.0)
             && ((IsShootableBreakable (tr.pHit))
             || (((i == 0) || (i == 4)) && (FStrEq(STRING(tr.pHit->v.classname),"func_breakable"))
             && (tr.pHit->v.takedamage > 0) && ((tr.pHit->v.impulse == 0) || (i == 0))))) // KWo - 12.03.2010
      {

         if (g_b_DebugEntities)
         {
            ALERT(at_logged, "[DEBUG] Bot %s found a breakable from position %i ; Current WP = %i; Previous WP = %i, vecBreakable = [%i,%i,%i].\n",
               pBot->name, i, pBot->curr_wpt_index, pBot->prev_wpt_index[0], (int)tr.vecEndPos.x, (int)tr.vecEndPos.y, (int)tr.vecEndPos.z);
            if (i>=0)
               UTIL_DrawBeam (v_src, v_dest, 20, 50, 0, 0, 255, 0, 255, 0);
         }

         pBot->vecBreakable = tr.vecEndPos;
         return (TRUE);
      }
   }
   pBot->pBreakableIgnore = pBot->pShootBreakable; // KWo - 12.03.2010
   pBot->pShootBreakable = NULL;
   pBot->vecBreakable = g_vecZero;
   return (FALSE);
}


bool IsDeadlyDropAtPos (bot_t *pBot, Vector vecTargetPos) // KWo - 02.04.2010
{
   // Returns true if given location would hurt Bot with falling damage
   edict_t *pEdict = pBot->pEdict;
   TraceResult tr;
   float height;
   Vector v_check = vecTargetPos;
   Vector v_down = vecTargetPos;
   v_down.z = v_down.z - 1000.0; // straight down 1000 units

   TRACE_LINE (v_check, v_down, ignore_monsters, pEdict, &tr);
   height = tr.flFraction * 1000.0; // height from ground at goal position

   if (height > 150.0)
      return (TRUE);
   // Wall blocking ?
   if (tr.fStartSolid) // KWo - 07.04.2010
      return (TRUE);
   return (FALSE);
}


bool IsDeadlyDrop (bot_t *pBot, Vector vecTargetPos)
{
   // Returns true if given location and the way to it would hurt Bot with falling damage

   edict_t *pEdict = pBot->pEdict;
   Vector vecBot = pEdict->v.origin;
   TraceResult tr;
   float height, last_height, distance;

   Vector v_direction = (vecTargetPos - vecBot).Normalize (); // 1 unit long
   Vector v_check = vecTargetPos;   // KWo - 02.04.2010
   Vector v_down = vecTargetPos;    // KWo - 02.04.2010

   v_down.z = v_down.z - 1000.0; // straight down 1000 units

   TRACE_LINE (v_check, v_down, ignore_monsters, pEdict, &tr);
/*
   // We're not on ground anymore ?
   if (tr.flFraction > 0.036)
      tr.flFraction = 0.036;
*/

   last_height = tr.flFraction * 1000.0; // height from ground at goal position
   distance = (vecBot - v_check).Length (); // distance from goal

   if ((distance <= 30.0) && (last_height > 150.0)) // KWo - 02.04.2010
      return (TRUE);

   while (distance > 30.0) // KWo - 30.10.2009
   {
      // move 30 units closer to the bot...
      v_check = v_check - (v_direction * 30.0); // KWo - 02.04.2010

      v_down = v_check;
      v_down.z = v_down.z - 1000.0; // straight down 1000 units

      TRACE_LINE (v_check, v_down, ignore_monsters, pEdict, &tr);

      // Wall blocking ?
      if (tr.fStartSolid)
         return (FALSE);

      height = tr.flFraction * 1000.0; // height from ground

      // Drops more than 150 Units ?
      if (last_height < height - 150.0)
      {
         if (g_b_DebugNavig)
            ALERT (at_logged, "[DEBUG] Bot %s can fall down from tested location v_check = [%i, %i, %i].\n", pBot->name, (int) v_check.x, (int) v_check.y, (int) v_check.z);
         return (TRUE);
      }

      last_height = height;

      distance = (vecBot - v_check).Length (); // distance from goal
   }
   return (FALSE);
}


void BotFindItem (bot_t *pBot)
{
   // Finds Items to collect or use in the near of a Bot

   edict_t *pEdict = pBot->pEdict;
   edict_t *pent = NULL;
   edict_t *pPickupEntity = NULL;
   bool bItemExists;
   Vector vecPosition;
   int iPickType = 0;
   Vector vecPickupOrigin;
   Vector vecEntityOrigin;
   bool bCanPickup;
   float fDistance;
   float fDistance2;
   float fDistance3;
   float fMinDistance = 9999;
   float fTraveltime;
   float fTimeMidBlowup;
   float fTimeBlowup;
   bool bPrimaryAmmo = false;
   bool bSecondaryAmmo = false;
   bool bArmor = false;
   bool bGrenade = false;
   bool bMedkit = false;
   bool bBomb;                   // KWo - 28.08.2008
   Vector vecEnd;
   int iIndex;
   int i;
   int h;
   int iWeaponPrimNum;           // KWo - 21.12.2006
   int iWeaponSecNum;            // KWo - 21.12.2006
   int iWeaponSecID;             // KWo - 08.01.2007
   int iWeaponPrimID;            // KWo - 08.01.2007
   char vclassname[64];          // KWo - 13.05.2006
   char vmodelname[64];          // KWo - 18.12.2006

   // Don't try to pickup anything while on ladder...
   if (pBot->bOnLadder)
   {
      pBot->pBotPickupItem = NULL;
      pBot->iPickupType = PICKUP_NONE;
      return;
   }

   iWeaponSecNum = HighestPistolOfEdict (pEdict);        // KWo - 21.12.2006
   iWeaponPrimNum = HighestWeaponOfEdict(pEdict);        // KWo - 21.12.2006
   iWeaponSecID = cs_weapon_select[iWeaponSecNum].iId;   // KWo - 08.01.2007
   iWeaponPrimID = cs_weapon_select[iWeaponPrimNum].iId; // KWo - 08.01.2007

   if (!FNullEnt (pBot->pBotPickupItem))
   {
      bItemExists = FALSE;
      pPickupEntity = pBot->pBotPickupItem;

      while (!FNullEnt (pent = FIND_ENTITY_IN_SPHERE (pent, pEdict->v.origin, 500)))
      {
         if (pent->v.effects & EF_NODRAW)
            continue; // someone owns this weapon or it hasn't respawned yet

         snprintf (vclassname, sizeof (vclassname), STRING (pent->v.classname)); // KWo - 25.12.2006
         if (((strcmp ("weaponbox", vclassname) == 0) || (strncmp (vclassname, "csdmw_", 6) == 0))
            && !FNullEnt(pent->v.owner)) // KWo - 25.12.2006
         {
            if (pent->v.owner == pEdict) // it's our weapon we dropped already
            {
               pBot->pItemIgnore[2] = pBot->pItemIgnore[1];
               pBot->pItemIgnore[1] = pBot->pItemIgnore[0];
               pBot->pItemIgnore[0] = pent;
               continue;
            }
         }

         if (pent == pPickupEntity)
         {
            if (pent->v.absmin != g_vecZero)
               vecPosition = VecBModelOrigin (pent);
            else
               vecPosition = pent->v.origin;
            bBomb = ((pBot->iPickupType == PICKUP_DROPPED_C4) || (pBot->iPickupType == PICKUP_PLANTED_C4)); // KWo - 29.08.2008
            if (BotItemIsVisible(pBot, vecPosition, (char *) STRING (pent->v.classname), bBomb)) // KWo - 29.08.2008
               bItemExists = TRUE;
            break;
         }
      }

      if (bItemExists)
      {
         if (g_b_DebugEntities)
            ALERT(at_logged,"[DEBUG] Bot %s still can take care about found item %s.\n", pBot->name, g_ItemTypeNames[pBot->iPickupType]);
         return;
      }
      else
      {
         pBot->pBotPickupItem = NULL;
         pBot->iPickupType = PICKUP_NONE;
      }
   }

   pent = NULL;
   pPickupEntity = NULL;

   pBot->pBotPickupItem = NULL;
   pBot->iPickupType = PICKUP_NONE;

   while (!FNullEnt (pent = FIND_ENTITY_IN_SPHERE (pent, pEdict->v.origin, 500)))
   {
      bCanPickup = false; // assume can't use it until known otherwise
      bPrimaryAmmo = false;
      bSecondaryAmmo = false;
      bArmor = false;
      bGrenade = false;
      bMedkit = false;

      if ((pent->v.effects & EF_NODRAW) || (pent == pBot->pItemIgnore[0])
          || (pent == pBot->pItemIgnore[1]) || (pent == pBot->pItemIgnore[2])
          || (fabs(pent->v.origin.z - pEdict->v.origin.z) > 100.0))  // KWo - 08.04.2016
         continue; // someone owns this weapon or it hasn't respawned yet or it needs to be ignored

      // see if this is a "func_" type of entity (func_button, etc.)...
      if (pent->v.absmin != g_vecZero)
         vecEntityOrigin = VecBModelOrigin (pent);
      else
         vecEntityOrigin = pent->v.origin;

      vecEnd = vecEntityOrigin;

      bBomb = FALSE;
      if (((strcmp ("weaponbox", STRING (pent->v.classname)) == 0)
                  && (strcmp ("models/w_backpack.mdl", STRING (pent->v.model)) == 0))
         || ((strcmp ("grenade", vclassname) == 0)
                  && ((strcmp ("models/w_c4.mdl", STRING (pent->v.model)) == 0)
                       || ((pent->v.dmg >= 100) && (pent->v.movetype == 6))))) // KWo - 29.08.2008
      {
         bBomb = TRUE;
         if (g_b_DebugEntities)
         {
            fDistance3 = (vecEntityOrigin - pEdict->v.origin).Length ();
            ALERT(at_logged,"[DEBUG] BotFindItem - bot %s is near the dropped C4 - distance = %.0f\n", pBot->name, fDistance3);
         }
      }

      // check if line of sight to object is not blocked (i.e. visible)
      if (BotItemIsVisible (pBot, vecEnd, (char *) STRING (pent->v.classname), bBomb))
      {
         snprintf (vclassname, sizeof (vclassname), STRING (pent->v.classname)); // KWo - 13.05.2006
         snprintf (vmodelname, sizeof (vmodelname), STRING (pent->v.model)); // KWo - 18.12.2006

         if ((strcmp("hostage_entity", vclassname) == 0)&& (pBot->bot_team == TEAM_CS_COUNTER)) // KWo - 03.09.2006
         {
            for (h=0; h < g_iNumHostages; h++) // KWo - 17.05.2006
            {
               if (HostagesData[h].EntIndex == ENTINDEX(pent))
               {
                  if (HostagesData[h].Alive && (HostagesData[h].UserEntIndex == 0)) // KWo - 03.09.2006
                  {
                     if (g_b_DebugEntities)
                        ALERT(at_logged, "[DEBUG] BotFindItem - Bot %s found the hostage %d and can pickup it.\n", pBot->name, h);
                     bCanPickup = TRUE;
                     iPickType = PICKUP_HOSTAGE;
                     break;
                  }
               }
            }
         }

         else if (strcmp ("weapon_shield", vclassname) == 0) // KWo - 13.05.2006
         {
            bCanPickup = TRUE;
            iPickType = PICKUP_SHIELD;
         }

         else if ((strcmp ("models/w_assault.mdl", vmodelname) == 0)
              && !pBot->bUsingGrenade) // KWo - 21.12.2006
         {
            bCanPickup = TRUE;
            iPickType = PICKUP_WEAPON;
            bArmor = true;
         }
         else if (strcmp ("models/w_medkit.mdl", vmodelname) == 0) // KWo - 21.12.2006
         {
            bCanPickup = TRUE;
            iPickType = PICKUP_WEAPON;
            bMedkit = true;
         }
         else if ((strcmp ("models/w_357ammobox.mdl", vmodelname) == 0) && !pBot->bUsingGrenade) // KWo - 21.12.2006
         {
            bCanPickup = TRUE;
            iPickType = PICKUP_WEAPON;
            bSecondaryAmmo = true;
         }
         else if (((strcmp ("models/w_9mmarclip.mdl", vmodelname) == 0)
                || (strcmp ("models/w_shotbox.mdl", vmodelname) == 0) || (strcmp ("models/w_9mmclip.mdl", vmodelname) == 0)
                || (strcmp ("models/w_crossbow_clip.mdl", vmodelname) == 0) || (strcmp ("models/w_chainammo.mdl", vmodelname) == 0))
                && !pBot->bUsingGrenade && !BotHasShield (pBot)) // KWo - 21.12.2006
         {
            bCanPickup = TRUE;
            iPickType = PICKUP_WEAPON;
            bPrimaryAmmo = true;
         }
         else if (((strcmp ("models/w_hegrenade.mdl", vmodelname) == 0)
                || (strcmp ("models/w_smokegrenade.mdl", vmodelname) == 0)
                || (strcmp ("models/w_flashbang.mdl", vmodelname) == 0)) && !pBot->bUsingGrenade
                              && (pent->v.movetype != 10)) // KWo - 12.01.2007
         {
            bCanPickup = TRUE;
            iPickType = PICKUP_WEAPON;
            bGrenade = true;
         }
         else if (((strcmp ("weaponbox", vclassname) == 0) || (strncmp (vclassname, "csdmw_", 6) == 0))
                  && (strcmp ("models/w_backpack.mdl", vmodelname) != 0)
                  && !BotHasShield (pBot) && !pBot->bUsingGrenade ) // KWo - 21.12.2006
         {
            bCanPickup = TRUE;
            iPickType = PICKUP_WEAPON;
         }
         else if ((strcmp ("weaponbox", vclassname) == 0)
                  && (strcmp ("models/w_backpack.mdl", STRING (pent->v.model)) == 0)) // KWo - 13.05.2006
         {
            bCanPickup = TRUE;
            iPickType = PICKUP_DROPPED_C4;
         }

         else if ((strcmp ("grenade", vclassname) == 0)
                  && ((strcmp ("models/w_c4.mdl", STRING (pent->v.model)) == 0)
                       || ((pent->v.dmg >= 100) && (pent->v.movetype == 6)))) // KWo - 17.04.2008
         {
            bCanPickup = TRUE;
            iPickType = PICKUP_PLANTED_C4;
         }
         else if ((strcmp ("item_thighpack", vclassname) == 0)
                  && (pBot->bot_team == TEAM_CS_COUNTER) && !pBot->b_has_defuse_kit) // KWo - 13.05.2006
         {
            bCanPickup = TRUE;
            iPickType = PICKUP_DEFUSEKIT;
         }
      }

      // if the bot found something it can pickup...
      if (bCanPickup)
      {
         fDistance = (vecEntityOrigin - pEdict->v.origin).Length ();

         // see if it's the closest item so far...
         if (fDistance < fMinDistance)
         {
            // Found weapon on ground ?
            if (iPickType == PICKUP_WEAPON)
            {
               if (pBot->bIsChickenOrZombie || (pBot->bIsVIP) && (!bSecondaryAmmo) || ((BotRateGroundWeapon (pBot, pent) <= 0)
                   || ((pBot->iNumWeaponPickups >= g_i_cv_maxweaponpickup) && (g_i_cv_maxweaponpickup > 0)))
                   && (!bPrimaryAmmo) && (!bSecondaryAmmo) && (!bArmor) && (!bGrenade) && (!bMedkit))  // KWo - 11.04.2008
                  bCanPickup = FALSE;
               else if ((pBot->bIsVIP) && (bPrimaryAmmo) || (bArmor) && (pEdict->v.armorvalue > 60.0) || (bMedkit) && (pEdict->v.health > 60.0))
                  bCanPickup = FALSE;
               else if (bPrimaryAmmo) // pickup only ammo for the weapon bot is carrying
               {
                  if ((iWeaponPrimNum < MAX_WEAPONS) && (iWeaponPrimNum > 6))
                  {
                      if (strcmp ("models/w_9mmarclip.mdl", vmodelname) == 0)
                      {
                         if (!WeaponIsRifle(iWeaponPrimID))
                            bCanPickup = FALSE;
                      }
                      else if (strcmp ("models/w_shotbox.mdl", vmodelname) == 0)
                      {
                         if ((iWeaponPrimID != CS_WEAPON_M3) && (iWeaponPrimID != CS_WEAPON_XM1014))
                            bCanPickup = FALSE;
                      }
                      else if (strcmp ("models/w_9mmclip.mdl", vmodelname) == 0)
                      {
                         if (!WeaponIsSubmachineGun(iWeaponPrimID))
                            bCanPickup = FALSE;
                      }
                      else if (strcmp("models/w_crossbow_clip.mdl", vmodelname) == 0)
                      {
                         if (!WeaponIsSniper(iWeaponPrimID))
                            bCanPickup = FALSE;
                      }
                      else if (strcmp ("models/w_chainammo.mdl", vmodelname) == 0)
                      {
                         if (iWeaponPrimID != CS_WEAPON_M249)
                            bCanPickup = FALSE;
                      }
                      else
                      {
                         bCanPickup = FALSE;
                      }

                      if (bCanPickup
                         && ((pBot->m_rgAmmoInClip[iWeaponPrimID] + pBot->m_rgAmmo[weapon_defs[iWeaponPrimID].iAmmo1]) > 0.3 * weapon_defs[iWeaponPrimID].iAmmo1Max))
                        bCanPickup = FALSE;
                  }
                  else
                  {
                     bCanPickup = FALSE;
                  }
               }
               else if (bSecondaryAmmo)
               {
                  if ((iWeaponSecNum < 7) && (iWeaponSecNum > 0))
                  {
                     if (((pBot->m_rgAmmoInClip[iWeaponSecID] + pBot->m_rgAmmo[weapon_defs[iWeaponSecID].iAmmo1]) > 0.3 * weapon_defs[iWeaponSecID].iAmmo1Max)
                        || (pBot->current_weapon.iAmmo2 > 0.3 * weapon_defs[iWeaponSecID].iAmmo1Max))
                        bCanPickup = FALSE;
                  }
                  else
                     bCanPickup = FALSE;
               }
               else if (bGrenade)
               {
                  int weapons = pBot->pEdict->v.weapons;
                  if ((strcmp ("models/w_hegrenade.mdl", vmodelname) == 0) && (weapons & (1 << CS_WEAPON_HEGRENADE)))
                     bCanPickup = FALSE;
                  else if ((strcmp ("models/w_smokegrenade.mdl", vmodelname) == 0) && (weapons & (1 << CS_WEAPON_SMOKEGRENADE)))
                     bCanPickup = FALSE;
                  else if ((strcmp ("models/w_flashbang.mdl", vmodelname) == 0) && (weapons & (1 << CS_WEAPON_FLASHBANG)))
                     bCanPickup = FALSE;
               }

               if (g_b_DebugEntities)
               {
                  if (bCanPickup)
                  {
                     ALERT(at_logged,"[DEBUG] Bot %s found %s on the ground.\n", pBot->name, vmodelname);
                  }
               }
            }

            // Found shield on ground ? (code courtesy of Wei Mingzhi)
            else if (iPickType == PICKUP_SHIELD)
            {
               if ((pEdict->v.weapons & (1 << CS_WEAPON_ELITE))
                   || BotHasShield (pBot) || pBot->bIsVIP
                   || (BotHasPrimaryWeapon (pBot) && (BotRateGroundWeapon (pBot, pent) <= 0))
                   || pBot->bIsChickenOrZombie)
                  bCanPickup = FALSE;
            }

            // Terrorist Team specific
            else if (pBot->bot_team == TEAM_CS_TERRORIST)
            {
               if (iPickType == PICKUP_PLANTED_C4)
               {
                  bCanPickup = FALSE;

                  if ((!pBot->bDefendedBomb) && (!pBot->bIsChickenOrZombie)) // KWo - 18.01.2011
                  {
                     iIndex = BotFindDefendWaypoint (pBot, vecEntityOrigin);
                     fTraveltime = GetTravelTime (pBot->pEdict->v.maxspeed, pEdict->v.origin, paths[iIndex]->origin);
                     if ((g_f_cv_c4timer / 2) > g_f_cv_maxcamptime) // KWo - 16.06.2008
                        fTimeMidBlowup = g_fTimeBombPlanted + g_f_cv_maxcamptime;
                     else
                        fTimeMidBlowup = g_fTimeBombPlanted + (g_f_cv_c4timer / 2);

                     if (fTimeMidBlowup - fTraveltime - g_fTimeBombPlanted > 0.0) // KWo - 16.06.2008
                     {
                        pBot->bDefendedBomb = TRUE; // KWo - 23.03.2008

                        // Push camp task on to stack
                        pBot->fTimeCamping = fTimeMidBlowup; // KWo - 23.03.2008
                        bottask_t TempTask = {NULL, NULL, TASK_CAMP, TASKPRI_CAMP, -1, fTimeMidBlowup, TRUE};
                        BotPushTask (pBot, &TempTask);

                        // Push Move Command
                        TempTask.iTask = TASK_MOVETOPOSITION;
                        TempTask.fDesire = TASKPRI_MOVETOPOSITION;
                        TempTask.iData = iIndex;
                        BotPushTask (pBot, &TempTask);

                        pBot->iCampButtons |= IN_DUCK;
                     }
                     else if ((!g_b_cv_ffa) && (g_b_cv_radio)
                        && ((pBot->bot_team == TEAM_CS_TERRORIST) && (g_iAliveTs > 1))) // KWo - 12.09.2008
							{
                        BotPlayRadioMessage (pBot, RADIO_SHESGONNABLOW); // Issue an additional Radio Message
                        pBot->iCampButtons = 0; // KWo - 17.02.2008
                        pBot->f_ducktime = 0.0; // KWo - 24.01.2012
							}
                  }
               }
            }

            // CT Team specific
            else
            {
               if (iPickType == PICKUP_HOSTAGE)
               {
                  for (i = 0; i < gpGlobals->maxClients; i++)
                  {
                     if (bots[i].is_used && !bots[i].bDead)
                     {
                        for (h = 0; h < g_iNumHostages; h++)  // KWo - 17.05.2006
                        {
                           if ((bots[i].pHostages[h] == pent) && (HostagesData[h].UserEntIndex != 0))  // KWo - 13.09.2006
                           {
                              bCanPickup = FALSE;
                              pBot->iPickupType = PICKUP_NONE;  // KWo - 13.09.2006
                              if (g_b_DebugEntities)
                                 ALERT(at_logged, "[DEBUG] BotFindItem - Bot %s found the hostage %d but cannot pickup it - used by %s.\n", pBot->name, h, bots[i].name);
                              break;
                           }
                        }
                     }
                  }
               }

               else if (iPickType == PICKUP_PLANTED_C4)
               {
                  edict_t *pPlayer;

                  // search the world for teammates...
                  for (i = 0; i < gpGlobals->maxClients; i++)
                  {
                     if (!(clients[i].iFlags & CLIENT_USED)
                         || !(clients[i].iFlags & CLIENT_ALIVE)
                         || (clients[i].iTeam != pBot->bot_team)
                         || (clients[i].pEdict == pEdict))
                        continue;

                     pPlayer = clients[i].pEdict;

                     // find the distance to the target waypoint
                     fDistance2 = (pPlayer->v.origin - vecEntityOrigin).Length();

                     if ((fDistance2 < 60) && (fabs(pPlayer->v.origin.z - vecEntityOrigin.z) < 40.0) && (fDistance > fDistance2)) // KWo - 05.09.2008
                     {
                        bCanPickup = FALSE;

                        if ((!pBot->bDefendedBomb) && (!pBot->bIsChickenOrZombie)) // KWo - 18.01.2011
                        {
                           iIndex = BotFindDefendWaypoint (pBot, vecEntityOrigin);
                           fTraveltime = GetTravelTime (pBot->pEdict->v.maxspeed, pEdict->v.origin, paths[iIndex]->origin);
                           if (g_f_cv_c4timer > g_f_cv_maxcamptime) // KWo - 23.03.2008
                              fTimeBlowup = g_fTimeBombPlanted + g_f_cv_maxcamptime;
                           else
                              fTimeBlowup = g_fTimeBombPlanted + g_f_cv_c4timer;

                           if (fTimeBlowup - g_fTimeBombPlanted - fTraveltime > 0.0) // KWo - 16.06.2008
                           {
                              pBot->bDefendedBomb = TRUE; // KWo - 23.03.2008
                              // Push camp task on to stack
                              pBot->fTimeCamping = fTimeBlowup; // KWo - 23.03.2008
                              bottask_t TempTask = {NULL, NULL, TASK_CAMP, TASKPRI_CAMP, -1, fTimeBlowup, TRUE};

                              BotPushTask (pBot, &TempTask);

                              // Push Move Command
                              TempTask.iTask = TASK_MOVETOPOSITION;
                              TempTask.fDesire = TASKPRI_MOVETOPOSITION;
                              TempTask.iData = iIndex;
                              BotPushTask (pBot, &TempTask);
                              pBot->iCampButtons |= IN_DUCK;

                              return;
                           }
                        }
                     }
                  }
               }

               else if (iPickType == PICKUP_DROPPED_C4)
               {
                  pBot->pItemIgnore[2] = pBot->pItemIgnore[1];
                  pBot->pItemIgnore[1] = pBot->pItemIgnore[0];
                  pBot->pItemIgnore[0] = pent;
                  bCanPickup = FALSE;

                  if (((pBot->pEdict->v.health * RANDOM_LONG (1, 100)) > 3000.0)
                     && (BotHasCampWeapon(pBot)) && (pBot->current_weapon.iAmmo1 > 0)
                     && (pBot->fTimeCamping + 10.0 < gpGlobals->time)
                     && (!pBot->bIsChickenOrZombie)) // KWo - 18.01.2011
                  {
                     // Push camp task on to stack
                     float fCampTime = RANDOM_FLOAT (30.0, 60.0); // KWo - 23.03.2008
                     if (fCampTime > g_f_cv_maxcamptime)          // KWo - 23.03.2008
                        fCampTime = g_f_cv_maxcamptime;           // KWo - 23.03.2008
                     pBot->fTimeCamping = gpGlobals->time + fCampTime; // KWo - 23.03.2008
                     bottask_t TempTask = {NULL, NULL, TASK_CAMP, TASKPRI_CAMP, -1, pBot->fTimeCamping, TRUE}; // KWo - 23.03.2008
                     BotPushTask (pBot, &TempTask);

                     // Push Move Command
                     TempTask.iTask = TASK_MOVETOPOSITION;
                     TempTask.fDesire = TASKPRI_MOVETOPOSITION;
                     TempTask.iData = BotFindDefendWaypoint (pBot, vecEntityOrigin);
                     BotPushTask (pBot, &TempTask);
                     pBot->iCampButtons |= IN_DUCK;

                     return;
                  }
               }
            }

            if (bCanPickup)
            {
               fMinDistance = fDistance; // update the minimum distance
               pPickupEntity = pent; // remember this entity
               vecPickupOrigin = vecEntityOrigin; // remember location of entity
               pBot->iPickupType = iPickType;
            }
            else
            {
               iPickType = PICKUP_NONE;
            }
         }
      }
   } // end while loop

   if (pPickupEntity != NULL)
   {
      for (i = 0; i < gpGlobals->maxClients; i++)
      {
         if (bots[i].is_used && !bots[i].bDead && (bots[i].pEdict != pEdict)) // KWo - 26.10.2006
         {
            // forget it if another bot wants to pickup it...
            if ((bots[i].pBotPickupItem == pPickupEntity) && (BotGetSafeTask(&bots[i])->iTask == TASK_PICKUPITEM))
            {
               pBot->pBotPickupItem = NULL;
               pBot->iPickupType = PICKUP_NONE;
               return;
            }
         }
      }

      // Check if Item is too high to reach
      if (vecPickupOrigin.z > GetGunPosition (pEdict).z + ((pBot->iPickupType == PICKUP_HOSTAGE)? 50.0 : 20.0)) // KWo - 08.03.2010
      {
         pBot->pBotPickupItem = NULL;
         pBot->iPickupType = PICKUP_NONE;

         return;
      }

      // Check if getting the item would hurt Bot
      if (IsDeadlyDrop (pBot, vecPickupOrigin))
      {
         pBot->pBotPickupItem = NULL;
         pBot->iPickupType = PICKUP_NONE;
         return;
      }
      pBot->pBotPickupItem = pPickupEntity; // save the item bot is trying to get
   }
   return;
}


void BotChangePitch (bot_t *pBot, float speed)
{
   edict_t *pEdict = pBot->pEdict;
   float ideal;
   float current;
   float current_180; // current +/- 180 degrees
   float diff;
   float fOld = 0;

   UTIL_ClampAngle (&pEdict->v.idealpitch);

   // turn from the current v_angle pitch to the idealpitch by selecting
   // the quickest way to turn to face that direction
   current = pEdict->v.v_angle.x;
   ideal = pEdict->v.idealpitch;

   // find the difference in the current and ideal angle
   diff = fabs (current - ideal);

   // check if difference is less than the max degrees per turn
   if (diff < speed)
      speed = diff; // just need to turn a little bit (less than max)

   // Sum up old Pitch Changes
   fOld += pBot->rgfPitchHistory[0];
   fOld += pBot->rgfPitchHistory[1];

   fOld += speed;
   fOld *= 1.f / 3.f;

   pBot->rgfPitchHistory[1] = pBot->rgfPitchHistory[0];
   pBot->rgfPitchHistory[0] = speed;

   speed = fOld;

   // here we have four cases, both angle positive, one positive and
   // the other negative, one negative and the other positive, or
   // both negative.  handle each case separately...

   if ((current >= 0) && (ideal >= 0))  // both positive
   {
      if (current > ideal)
         speed = -speed;
   }
   else if ((current >= 0) && (ideal < 0))
   {
      current_180 = current - 180;

      if (current_180 <= ideal)
         speed = -speed;
   }
   else if ((current < 0) && (ideal >= 0))
   {
      current_180 = current + 180;
      if (current_180 <= ideal)
         speed = -speed;
   }
   else // (current < 0) && (ideal < 0)  both negative
   {
      if (current > ideal)
         speed = -speed;
   }

   current += speed;
   pEdict->v.v_angle.x = current;
   return;
}


void BotChangeYaw (bot_t *pBot, float speed)
{
   edict_t *pEdict = pBot->pEdict;
   float ideal;
   float current;
   float current_180; // current +/- 180 degrees
   float diff;
   float fOld = 0;

   UTIL_ClampAngle (&pEdict->v.ideal_yaw);

   // turn from the current v_angle yaw to the ideal_yaw by selecting
   // the quickest way to turn to face that direction
   current = pEdict->v.v_angle.y;
   ideal = pEdict->v.ideal_yaw;

   // find the difference in the current and ideal angle
   diff = fabs (current - ideal);

   // check if difference is less than the max degrees per turn
   if (diff < speed)
      speed = diff; // just need to turn a little bit (less than max)

   // Sum up old Yaw Changes
   fOld += pBot->rgfYawHistory[0];
   fOld += pBot->rgfYawHistory[1];

   fOld += speed;
   fOld *= 1.f / 3.f;

   pBot->rgfYawHistory[1] = pBot->rgfYawHistory[0];
   pBot->rgfYawHistory[0] = speed;

   speed = fOld;

   // here we have four cases, both angle positive, one positive and
   // the other negative, one negative and the other positive, or
   // both negative.  handle each case separately...

   if ((current >= 0) && (ideal >= 0))  // both positive
   {
      if (current > ideal)
         speed = -speed;
   }
   else if ((current >= 0) && (ideal < 0))
   {
      current_180 = current - 180;

      if (current_180 <= ideal)
         speed = -speed;
   }
   else if ((current < 0) && (ideal >= 0))
   {
      current_180 = current + 180;
      if (current_180 <= ideal)
         speed = -speed;
   }
   else  // (current < 0) && (ideal < 0)  both negative
   {
      if (current > ideal)
         speed = -speed;
   }

   current += speed;
   pEdict->v.v_angle.y = current;
   return;
}


void BotPlayRadioMessage (bot_t *pBot, int iMessage)
{
   // Inserts the Radio Message into the Message Queue
   pBot->iRadioSelect = iMessage;
   BotPushMessageQueue (pBot, MSG_CS_RADIO);
   return;  // KWo - 28.01.2006
}


void BotCheckMessageQueue (bot_t *pBot)
{
   // Checks and executes pending Messages

   int iCurrentMSG;
   bool bRadioAnswer = FALSE; // KWo - 20.03.2010

   // No new message ?
   if (pBot->iActMessageIndex == pBot->iPushMessageIndex)
      return;

   // Get Message from Stack
   iCurrentMSG = BotGetMessageQueue (pBot);

   if ((iCurrentMSG == MSG_CS_RADIO) && ((g_b_cv_ffa) || !g_b_cv_radio)) // KWo - 03.02.2007
   {
      pBot->iRadioSelect = 0;
      return;
   }

   switch (iCurrentMSG)
   {
      // General Radio Message issued
      case MSG_CS_RADIO:
      {
         // If last Bot Radio Command (global) happened just 3 second ago, delay response
         if (g_rgfLastRadioTime[pBot->bot_team - 1] + 3.0 < gpGlobals->time)
         {
            char szReport[80];
            int iSize = sizeof (szReport);
/*
            // If same message like previous just do a yes/no
            if ((pBot->iRadioSelect != RADIO_AFFIRMATIVE)
                && (pBot->iRadioSelect != RADIO_NEGATIVE)
                && (pBot->iRadioSelect != RADIO_REPORTINGIN)
                && (pBot->iRadioSelect != RADIO_FOLLOWME))  // KWo - 27.04.2006
            {
               if ((pBot->iRadioSelect == g_rgfLastRadio[pBot->bot_team - 1])
                   && (g_rgfLastRadioTime[pBot->bot_team - 1] + 1.5 > gpGlobals->time))
                  pBot->iRadioSelect = RADIO_AFFIRMATIVE;
               else
                  g_rgfLastRadio[pBot->bot_team - 1] = pBot->iRadioSelect;
            }
*/
            if (pBot->iRadioSelect == RADIO_REPORTINGIN)
            {
               int iTask = BotGetSafeTask(pBot)->iTask;
               int iWPT = pBot->pTasks->iData;

               switch (iTask)
               {
                  case TASK_NORMAL:
                  {
                     if (iWPT != -1)
                     {
                        if (paths[iWPT]->flags & W_FL_GOAL)
                           snprintf (szReport, iSize, "Heading for a Map Goal!");
                        else if (paths[iWPT]->flags & W_FL_RESCUE)
                           snprintf (szReport, iSize, "Heading to Rescue Point");
                        else if (paths[iWPT]->flags & W_FL_CAMP)
                           snprintf (szReport, iSize, "Moving to Camp Spot");
                        else
                           snprintf (szReport, iSize, "Roaming around");
                     }
                     else
                        snprintf (szReport, iSize, "Roaming around");
                     break;
                  }
                  case TASK_MOVETOPOSITION:
                     snprintf (szReport, iSize, "Moving to position");
                     break;
                  case TASK_FOLLOWUSER:
                     if (!FNullEnt (pBot->pBotUser))
                        snprintf (szReport, iSize, "Following %s", STRING (pBot->pBotUser->v.netname));
                     break;
                  case TASK_WAITFORGO:
                     snprintf (szReport, iSize, "Waiting for GO!");
                     break;
                  case TASK_CAMP:
                     snprintf (szReport, iSize, "Camping...");
                     break;
                  case TASK_PLANTBOMB:
                     snprintf (szReport, iSize, "Planting the Bomb!");
                     break;
                  case TASK_DEFUSEBOMB:
                     snprintf (szReport, iSize, "Defusing the Bomb!");
                     break;
                  case TASK_ATTACK:
                     if (!FNullEnt (pBot->pBotEnemy))
                        snprintf (szReport, iSize, "Attacking %s", STRING (pBot->pBotEnemy->v.netname));
                     break;
                  case TASK_ENEMYHUNT:
                     if (!FNullEnt (pBot->pLastEnemy))
                        snprintf (szReport, iSize, "Hunting %s", STRING (pBot->pLastEnemy->v.netname));
                     break;
                  case TASK_SEEKCOVER:
                     snprintf (szReport, iSize, "Fleeing from Battle");
                     break;
                  case TASK_HIDE:
                     snprintf (szReport, iSize, "Hiding from Enemy");
                     break;
                  default:
                     snprintf (szReport, iSize, "Nothing special here...");
                     break;
               }
            }
            if ((pBot->iRadioSelect < 10) && (RANDOM_LONG(1,100) < 10)) // KWo - 06.03.2010
            {
               FakeClientCommand (pBot->pEdict, "radio1;menuselect %d\n", pBot->iRadioSelect);
               bRadioAnswer = TRUE; // KWo - 20.03.2010
            }
            else if ((pBot->iRadioSelect >= 10) && (pBot->iRadioSelect < 20) && (RANDOM_LONG(1,100) < 10)) // KWo - 06.03.2010
            {
               FakeClientCommand (pBot->pEdict, "radio2;menuselect %d\n", pBot->iRadioSelect - 10);
               bRadioAnswer = TRUE; // KWo - 20.03.2010
            }
            else if ((pBot->iRadioSelect >= 20) && (RANDOM_LONG(1,100) < 10)
               || (pBot->iRadioSelect == 21)) // KWo - 20.03.2010
            {
               FakeClientCommand (pBot->pEdict, "radio3;menuselect %d\n", pBot->iRadioSelect - 20);
               bRadioAnswer = TRUE; // KWo - 20.03.2010
            }

            if (pBot->iRadioSelect == RADIO_REPORTINGIN)
               FakeClientCommand (pBot->pEdict, "say_team %s\n", szReport);

            pBot->iRadioSelect = 0;

            // Store last radio usage
            if (bRadioAnswer) // KWo - 20.03.2010
               g_rgfLastRadioTime[pBot->bot_team - 1] = gpGlobals->time;
         }
         else
            BotPushMessageQueue (pBot, MSG_CS_RADIO);
         break;
      }
      // Team independant Saytext
      case MSG_CS_SAY:
      {
         char szMessage[512];
         int iEntIndex;
         int bot_index;

         FakeClientCommand (pBot->pEdict, "say %s\n", pBot->szMiscStrings);
         pBot->f_lastchattime = gpGlobals->time; // KWo - 30.07.2006
         g_fLastChatTime = gpGlobals->time; // KWo - 30.07.2006

         // Notify other Bots of the spoken Text otherwise Bots won't respond to other Bots
         // (Network Messages aren't sent to Bots)
         iEntIndex = ENTINDEX (pBot->pEdict);

         // Add this so the Chat Parser doesn't get confused
         snprintf (szMessage, sizeof (szMessage), "%s:%s", STRING (pBot->pEdict->v.netname), pBot->szMiscStrings);

         for (bot_index = 0; bot_index < gpGlobals->maxClients; bot_index++)
         {
            bot_t *pOtherBot = &bots[bot_index];

            if (pOtherBot->is_used && !FNullEnt (pOtherBot->pEdict) && (pOtherBot != pBot))
            {
               if (!pOtherBot->bDead)
               {
                  pOtherBot->SaytextBuffer.iEntityIndex = iEntIndex;
                  strncpy (pOtherBot->SaytextBuffer.szSayText, szMessage, sizeof (pOtherBot->SaytextBuffer.szSayText));
                  pOtherBot->SaytextBuffer.szSayText[sizeof (pOtherBot->SaytextBuffer.szSayText) - 1] = 0;
               }

               pOtherBot->SaytextBuffer.fTimeNextChat = gpGlobals->time + pOtherBot->SaytextBuffer.fChatDelay;
            }
         }

         break;
      }
      default:
      {
         break;
      }
   }
   return;
}


void BotCheckReload (bot_t *pBot)  // bluebyte
{
   int iId = pBot->current_weapon.iId;
   int iMaxClip = 0;
   int iNum = 0; // KWo - 04.04.2010

   if ((iId > 0) && (iId < MAX_WEAPONS))
   {
      iMaxClip = weapon_maxClip[iId];
      iNum = weapon_selectIndex[iId]; // KWo - 05.04.2010
   }
   else
      return;

   if ((((WeaponIsPrimaryGun(iId)) && (pBot->current_weapon.iAmmo1 == 0))
         || ((WeaponIsPistol(iId)) && (pBot->current_weapon.iAmmo2 == 0)))
         || (pBot->current_weapon.iClip == iMaxClip)
         || (iMaxClip == 0) || WeaponIsNade(iId))
   {
      pBot->bIsReloading = FALSE;
      pBot->f_reloadingtime = 0.0;
      return;
   }

   if (!pBot->bIsReloading && FNullEnt(pBot->pBotEnemy) && (weapon_defs[iId].iAmmo1 != -1)
      && ((pBot->current_weapon.iAmmo1 > 0) && WeaponIsPrimaryGun(iId) || (pBot->current_weapon.iAmmo2 > 0) && WeaponIsPistol(iId))
      && (!(pBot->iStates & STATE_SEEINGENEMY) && !(pBot->iStates & STATE_SUSPECTENEMY)
      && !(pBot->iStates & STATE_HEARINGENEMY) && (pBot->f_bot_see_enemy_time + 4.0 < gpGlobals->time)
      && (pBot->f_heard_sound_time + 4.0 < gpGlobals->time)
      || (!(pBot->iStates & STATE_SEEINGENEMY) && (pBot->f_bot_see_enemy_time + 1.0 < gpGlobals->time) && g_bRoundEnded)
      || ((g_iAliveCTs == 0) && (pBot->bot_team == TEAM_CS_TERRORIST))
      || ((g_iAliveTs == 0) && (pBot->bot_team == TEAM_CS_COUNTER)))) // 30.09.2010
   {

      // bluebyte: we should reload more frequently
      if (((pBot->current_weapon.iClip < (0.8 * iMaxClip)) || (pBot->f_spawn_time + 5.0 > gpGlobals-> time))
         && (pBot->f_reloadingtime == 0.0) && (pBot->f_spawn_time + 2.0 < gpGlobals-> time))  // KWo - 10.07.2008
      {
         pBot->f_reloadingtime = gpGlobals->time + cs_weapon_select[iNum].primary_charge_delay + 0.2; // KWo - 04.04.2010
         pBot->pEdict->v.button |= IN_RELOAD;   // reload
         pBot->bIsReloading = TRUE;

         if (g_b_DebugCombat)
         {
            if ((pBot->bIsReloading) && (iId >= 0) && (iId < MAX_WEAPONS))
            {
               ALERT (at_logged, "[DEBUG] BotCheckReload - bot %s started to reload the weapon - %s, clip = %d, Ammo1 = %d, Ammo2 = %d, charge time = %.2f, time = %.2f.\n",
                  pBot->name, weapon_defs[iId].szClassname, pBot->current_weapon.iClip, pBot->current_weapon.iAmmo1, pBot->current_weapon.iAmmo2, cs_weapon_select[iNum].primary_charge_delay, gpGlobals->time);

            }
         }
      }
   }
   else if (pBot->bIsReloading && (pBot->f_reloadingtime != 0.0)
            && (pBot->f_reloadingtime < gpGlobals->time) && (pBot->fTimeWeaponSwitch + 0.5 < gpGlobals->time)) // KWo - 10.02.2008
   {
      pBot->bIsReloading = FALSE;
      pBot->f_reloadingtime = 0.0;
      if ((WeaponIsPrimaryGun(iId)) && (pBot->current_weapon.iAmmo1 > 0))
         pBot->current_weapon.iAmmo1 = 0;
      if ((WeaponIsPistol(iId)) && (pBot->current_weapon.iAmmo2 > 0))
         pBot->current_weapon.iAmmo2 = 0;
      pBot->m_rgAmmo[weapon_defs[iId].iAmmo1] = 0;

      if (pBot->current_weapon.iId != CS_WEAPON_KNIFE)
         SelectWeaponByName (pBot, "weapon_knife");

      if (g_b_DebugCombat)
      {
         ALERT(at_logged, "[DEBUG] BotCheckReload - Bot %s was reloading the weapon %s too long time; time = %f.\n",
            pBot->name, ((iId >= 0) && (iId < MAX_WEAPONS)) ? weapon_defs[iId].szClassname : "unknown", gpGlobals->time);
      }
   }
   return;
}


void BotBuyStuff (bot_t *pBot)
{
   // Does all the work in selecting correct Buy Menus for most Weapons/Items

   int iCount = 0;
   int iFoundWeapons = 0;
   int iBuyChoices[NUM_WEAPONS];
   int iChosenWeapon;
   bool bWeaponRestricted;   // KWo - 10.03.2006
   float fLightLevel;         // KWo - 13.01.2012
   edict_t *pEdict = pBot->pEdict;

   // If Bot has finished buying
   if (pBot->iBuyCount == 0)
      return;

   // If Fun-Mode no need to buy
   if (g_b_cv_jasonmode) // KWo - 06.04.2006
   {
      pBot->iBuyCount = 0;
      if (pBot->current_weapon.iId != CS_WEAPON_KNIFE)
         SelectWeaponByName (pBot, "weapon_knife");
      return;
   }

   // Prevent VIP from buying
   if ((g_iMapType & MAP_AS)
       && (strcmp ("vip", INFOKEY_VALUE (GET_INFOKEYBUFFER (pEdict), "model")) == 0))
   {
      pBot->iBuyCount = 0;
      pBot->bIsVIP = TRUE;
      pBot->byPathType = 2;
      return;
   }

   // Prevent buying if there is weapon stripping...
   if (g_bWeaponStrip) // KWo - 10.03.2013
   {
      pBot->iBuyCount = 0;
      return;
   }

   // Needs a Weapon ?
   if (pBot->iBuyCount == 1)
   {
      if (((BotHasPrimaryWeapon (pBot) || BotHasShield (pBot)) && (pBot->bot_money < 3700))
         || (g_i_MapBuying == 3) || ((pBot->bot_team == TEAM_CS_TERRORIST) && (g_i_MapBuying == 1))
         || ((pBot->bot_team == TEAM_CS_COUNTER) && (g_i_MapBuying == 2))
         /* ||  g_iMapType & MAP_AWP || g_iMapType & MAP_AIM */ || pBot->bIsChickenOrZombie)  // KWo - 07.06.2010
      {
         pBot->iBuyCount++; // only buy ammo & items

         BotCheckReload(pBot); // KWo - 31.03.2008
         return;
      }
      if ((BotHasPrimaryWeapon (pBot) || ((pBot->bot_personality != 2) && BotHasShield (pBot)))
          && (pBot->bot_money >= 3700) && (!g_b_cv_csdm_active))  // KWo - 15.04.2008
      {
         // Select the Priority Tab for this Personality
         int *ptrWeaponTab = ptrWeaponPrefs[pBot->bot_personality];
         int iCarriedWeapon = GetBestWeaponCarried(pBot);
         int iWeaponPriorityIndex = NUM_WEAPONS;

         // Start from most desired Weapon
         ptrWeaponTab += NUM_WEAPONS;

         do
         {
            ptrWeaponTab--;
            iWeaponPriorityIndex--;
            assert ((*ptrWeaponTab > -1) && (*ptrWeaponTab < NUM_WEAPONS));
            pBot->pSelected_weapon = &cs_weapon_select[*ptrWeaponTab];
            bWeaponRestricted = (g_iWeaponRestricted[*ptrWeaponTab] != 0);   // KWo - 10.03.2006
            iCount++;

            if (bWeaponRestricted)   // KWo - 10.03.2006
               continue;

            // Weapon available for every Team ?
            if (g_iMapType & MAP_AS)
            {
               if ((pBot->pSelected_weapon->iTeamAS != 2)
                   && (pBot->pSelected_weapon->iTeamAS != pBot->bot_team - 1))
                  continue;
            }
            else
            {
               if ((pBot->pSelected_weapon->iTeamStandard != 2)
                   && (pBot->pSelected_weapon->iTeamStandard != pBot->bot_team - 1))
                  continue;
            }

            if ((pBot->pSelected_weapon->iPrice < pBot->bot_money + 500)
               && (iWeaponPriorityIndex > iCarriedWeapon)
               && (cs_weapon_select[ptrWeaponPrefs[pBot->bot_personality][iCarriedWeapon]].iPrice
                  < cs_weapon_select[ptrWeaponPrefs[pBot->bot_personality][iWeaponPriorityIndex]].iPrice)) // KWo - 05.07.2008
            {
               iBuyChoices[iFoundWeapons++] = *ptrWeaponTab;

               if (g_b_DebugCombat)
               {
                  ALERT(at_logged, "[DEBUG] BotBuyStuff - Bot %s has the weapon %s and can buy %s.\n", pBot->name,
                     cs_weapon_select[ptrWeaponPrefs[pBot->bot_personality][iCarriedWeapon]].weapon_name, cs_weapon_select[*ptrWeaponTab].weapon_name);
               }
            }
         }
         while ((iCount < NUM_WEAPONS) && (iFoundWeapons < 4) && (iWeaponPriorityIndex > iCarriedWeapon));
      }

      else if ((pBot->bot_money > 650) && (!BotHasPrimaryWeapon (pBot)) && (!BotHasShield (pBot) && (!g_b_cv_csdm_active))) // KWo - 18.01.2011
      {
         // Select the Priority Tab for this Personality
         int *ptrWeaponTab = ptrWeaponPrefs[pBot->bot_personality];

         // Start from most desired Weapon
         ptrWeaponTab += NUM_WEAPONS;

         do
         {
            ptrWeaponTab--;
            assert ((*ptrWeaponTab > -1) && (*ptrWeaponTab < NUM_WEAPONS));
            pBot->pSelected_weapon = &cs_weapon_select[*ptrWeaponTab];
            bWeaponRestricted = (g_iWeaponRestricted[*ptrWeaponTab] != 0);   // KWo - 10.03.2006
            iCount++;

            if (bWeaponRestricted)   // KWo - 10.03.2006
               continue;

            // Weapon available for every Team ?
            if (g_iMapType & MAP_AS)
            {
               if ((pBot->pSelected_weapon->iTeamAS != 2)
                   && (pBot->pSelected_weapon->iTeamAS != pBot->bot_team - 1))
                  continue;
            }
            else
            {
               if ((pBot->pSelected_weapon->iTeamStandard != 2)
                   && (pBot->pSelected_weapon->iTeamStandard != pBot->bot_team - 1))
                  continue;
            }

            if (pBot->pSelected_weapon->iPrice < pBot->bot_money + 100)
               iBuyChoices[iFoundWeapons++] = *ptrWeaponTab;
         }
         while ((iCount < NUM_WEAPONS) && (iFoundWeapons < 4));

         if ((pBot->bot_money < 900) && (RANDOM_LONG(0, 10) < 3)) // KWo - 31.03.2008
            iFoundWeapons = 0; // bot decided to save money for a better primary weapon in the next round
      }
      // Found a desired weapon ?
      if (iFoundWeapons > 0)
      {
         // Choose randomly from the best ones...
         if (iFoundWeapons > 1)
            iChosenWeapon = iBuyChoices[RANDOM_LONG (0, iFoundWeapons - 1)];
         else
            iChosenWeapon = iBuyChoices[0];

         pBot->pSelected_weapon = &cs_weapon_select[iChosenWeapon];

         if (g_bIsOldCS15)
            FakeClientCommand (pEdict, "%s\n", pBot->pSelected_weapon->buy_command);
         else
            FakeClientCommand (pEdict, "%s\n", pBot->pSelected_weapon->buy_shortcut);
      }

      pBot->iBuyCount++;
      pBot->f_buy_time = gpGlobals->time + RANDOM_FLOAT (0.3, 0.5);
      return;
   }
   else if (pBot->iBuyCount == 2) // KWo - 18.01.2011
   {
      if (((BotHasSecondaryWeapon (pBot)) && (pBot->bot_money < 4000) && (!BotHasSniperWeapon (pBot)) && (!BotHasShield (pBot)))
         || (g_i_MapBuying == 3) || ((pBot->bot_team == TEAM_CS_TERRORIST) && (g_i_MapBuying == 1))
         || ((pBot->bot_team == TEAM_CS_COUNTER) && (g_i_MapBuying == 2))
         /* ||  g_iMapType & MAP_AWP || g_iMapType & MAP_AIM */ || pBot->bIsChickenOrZombie)
      {
         pBot->iBuyCount++;
         return;
      }
      if ((((BotHasSecondaryWeapon (pBot)) && (pBot->bot_money >= 4000))
         || (((BotHasSniperWeapon (pBot)) || (BotHasShield (pBot))) && (pBot->bot_money >= 1000)))
         && (!g_b_cv_csdm_active))
      {
         // Select the Priority Tab for this Personality
         int *ptrWeaponTab = ptrWeaponPrefs[pBot->bot_personality];
         int iCarriedWeapon = GetBestSecondaryWeaponCarried(pBot);
         int iWeaponPriorityIndex = NUM_WEAPONS;

         // Start from most desired Weapon
         ptrWeaponTab += NUM_WEAPONS;

         do
         {
            ptrWeaponTab--;
            iWeaponPriorityIndex--;
            assert ((*ptrWeaponTab > -1) && (*ptrWeaponTab < NUM_WEAPONS));
            pBot->pSelected_weapon = &cs_weapon_select[*ptrWeaponTab];
            bWeaponRestricted = (g_iWeaponRestricted[*ptrWeaponTab] != 0);
            iCount++;

            if (bWeaponRestricted)
               continue;

            // Weapon available for every Team ?
            if (g_iMapType & MAP_AS)
            {
               if ((pBot->pSelected_weapon->iTeamAS != 2)
                   && (pBot->pSelected_weapon->iTeamAS != pBot->bot_team - 1))
                  continue;
            }
            else
            {
               if ((pBot->pSelected_weapon->iTeamStandard != 2)
                   && (pBot->pSelected_weapon->iTeamStandard != pBot->bot_team - 1))
                  continue;
            }

            if (!WeaponIsPistol(pBot->pSelected_weapon->iId))
               continue;

            if ((pBot->pSelected_weapon->iPrice < pBot->bot_money + 500)
               && (iWeaponPriorityIndex > iCarriedWeapon)
               && (cs_weapon_select[ptrWeaponPrefs[pBot->bot_personality][iCarriedWeapon]].iPrice
                  < cs_weapon_select[ptrWeaponPrefs[pBot->bot_personality][iWeaponPriorityIndex]].iPrice))
            {
               iBuyChoices[iFoundWeapons++] = *ptrWeaponTab;

               if (g_b_DebugCombat)
               {
                  ALERT(at_logged, "[DEBUG] BotBuyStuff - Bot %s has the weapon %s and can buy %s.\n", pBot->name,
                     cs_weapon_select[ptrWeaponPrefs[pBot->bot_personality][iCarriedWeapon]].weapon_name, cs_weapon_select[*ptrWeaponTab].weapon_name);
               }
            }
         }
         while ((iCount < NUM_WEAPONS) && (iFoundWeapons < 4) && (iWeaponPriorityIndex > iCarriedWeapon));
      }

      else if ((pBot->bot_money > 650) && (!BotHasSecondaryWeapon (pBot)) && (!g_b_cv_csdm_active))
      {
         // Select the Priority Tab for this Personality
         int *ptrWeaponTab = ptrWeaponPrefs[pBot->bot_personality];

         // Start from the most desired Weapon
         ptrWeaponTab += NUM_WEAPONS;

         do
         {
            ptrWeaponTab--;
            assert ((*ptrWeaponTab > -1) && (*ptrWeaponTab < NUM_WEAPONS));
            pBot->pSelected_weapon = &cs_weapon_select[*ptrWeaponTab];
            bWeaponRestricted = (g_iWeaponRestricted[*ptrWeaponTab] != 0);
            iCount++;

            if (bWeaponRestricted)
               continue;

            // Weapon available for every Team ?
            if (g_iMapType & MAP_AS)
            {
               if ((pBot->pSelected_weapon->iTeamAS != 2)
                   && (pBot->pSelected_weapon->iTeamAS != pBot->bot_team - 1))
                  continue;
            }
            else
            {
               if ((pBot->pSelected_weapon->iTeamStandard != 2)
                   && (pBot->pSelected_weapon->iTeamStandard != pBot->bot_team - 1))
                  continue;
            }

            if (!WeaponIsPistol(pBot->pSelected_weapon->iId))
               continue;

            if (pBot->pSelected_weapon->iPrice < pBot->bot_money + 100)
               iBuyChoices[iFoundWeapons++] = *ptrWeaponTab;
         }
         while ((iCount < NUM_WEAPONS) && (iFoundWeapons < 4));

         if ((pBot->bot_money < 900) && (RANDOM_LONG(0, 10) < 3))
            iFoundWeapons = 0; // bot decided to save money for a better weapon in the next round
      }
      // Found a desired weapon ?
      if (iFoundWeapons > 0)
      {
         // Choose randomly from the best ones...
         if (iFoundWeapons > 1)
            iChosenWeapon = iBuyChoices[RANDOM_LONG (0, iFoundWeapons - 1)];
         else
            iChosenWeapon = iBuyChoices[0];

         pBot->pSelected_weapon = &cs_weapon_select[iChosenWeapon];

         if (g_bIsOldCS15)
            FakeClientCommand (pEdict, "%s\n", pBot->pSelected_weapon->buy_command);
         else
            FakeClientCommand (pEdict, "%s\n", pBot->pSelected_weapon->buy_shortcut);
      }

      pBot->iBuyCount++;
      pBot->f_buy_time = gpGlobals->time + RANDOM_FLOAT (0.3, 0.5);
      return;
   }

   // Needs Ammo ?
   else if (pBot->iBuyCount < 5) // KWo - 18.01.2011
   {
      if (!pBot->bIsChickenOrZombie) // KWo - 24.04.2008
      {
         if (!BotHasPrimaryWeapon (pBot)) // Pistol ?
         {
            if (g_iEquipAmmoRestricted[PB_WEAPON_AMMO2]==0)   // KWo - 10.03.2006
            {
               if (g_bIsOldCS15)
                  FakeClientCommand (pEdict, "buyammo2\n");
               else
                  FakeClientCommand (pEdict, "secammo\n");
            }
         }
         else
         {
            if (g_iEquipAmmoRestricted[PB_WEAPON_AMMO1]==0)   // KWo - 10.03.2006
            {
               if (g_bIsOldCS15)
                  FakeClientCommand (pEdict, "buyammo1\n");
               else
                  FakeClientCommand (pEdict, "primammo\n");
            }
            if ((g_iEquipAmmoRestricted[PB_WEAPON_AMMO2]==0) && (pBot->iBuyCount < 4))  // KWo - 18.01.2011
            {
               if (g_bIsOldCS15)
                  FakeClientCommand (pEdict, "buyammo2\n");
               else
                  FakeClientCommand (pEdict, "secammo\n");
            }
         }
      }
      pBot->iBuyCount++;
      pBot->f_buy_time = gpGlobals->time + RANDOM_FLOAT (0.2, 0.5);
      return;
   }

   // Needs an Item ?
   else if (pBot->iBuyCount < 8)
   {
      // Care first about buying Armor
      if (pBot->iBuyCount == 5)
      {
         if ((pBot->pEdict->v.armorvalue <= 40) && (pBot->bot_money >= 650))
         {
            if ((pBot->bot_money >= 1000) && (g_iEquipAmmoRestricted[PB_WEAPON_VESTHELM]==0))   // KWo - 10.03.2006
            {
               if (g_bIsOldCS15)
                  FakeClientCommand (pEdict, "buy;menuselect 8;menuselect 2\n");
               else
                  FakeClientCommand (pEdict, "vesthelm\n");
            }
            else if (g_iEquipAmmoRestricted[PB_WEAPON_VEST]==0)   // KWo - 10.03.2006
            {
               if (g_bIsOldCS15)
                  FakeClientCommand (pEdict, "buy;menuselect 8;menuselect 1\n");
               else
                  FakeClientCommand (pEdict, "vest\n");
            }
         }
      }

      if ((pBot->iBuyCount == 6) && (pBot->bot_money >= 300))  // KWo - 27.05.2008
      {
         int iGrenadeType = RANDOM_LONG (1, 100);

         // Focus on HE Grenades
         if ((iGrenadeType < 75) && (g_iEquipAmmoRestricted[PB_WEAPON_HEGRENADE]==0))   // KWo - 10.03.2006
         {
            if (g_bIsOldCS15)
               FakeClientCommand (pEdict, "buy;menuselect 8;menuselect 4\n");
            else
               FakeClientCommand (pEdict, "hegren\n");
         }
         else if ((iGrenadeType < 82) && (g_iEquipAmmoRestricted[PB_WEAPON_SMOKEGRENADE]==0))   // KWo - 10.03.2006
         {
            if (g_bIsOldCS15)
               FakeClientCommand (pEdict, "buy;menuselect 8;menuselect 5\n");
            else
               FakeClientCommand (pEdict, "sgren\n");
         }
         else if (g_iEquipAmmoRestricted[PB_WEAPON_FLASHBANG]==0)   // KWo - 10.03.2006
         {
            if (g_bIsOldCS15)
               FakeClientCommand (pEdict, "buy;menuselect 8;menuselect 3\n");
            else
               FakeClientCommand (pEdict, "flash\n");
         }
      }

      if (pBot->iBuyCount == 7)  // KWo - 27.05.2008
      {
         // If Defusion Map & Counter buy Defusion Kit
         if ((g_iMapType & MAP_DE) && (pBot->bot_team == TEAM_CS_COUNTER)
            && !pBot->b_has_defuse_kit && (pBot->bot_money >= 200) && (g_iEquipAmmoRestricted[PB_WEAPON_DEFUSER]==0))   // KWo - 10.03.2006
         {
            if (g_bIsOldCS15)
               FakeClientCommand (pEdict, "buy;menuselect 8;menuselect 6\n");
            else
               FakeClientCommand (pEdict, "defuser\n");
         }
         // If the map is dark, the bot needs a nightgoogles...
         fLightLevel = UTIL_IlluminationOf(pEdict); // KWo - 13.01.2012
         if (((((g_f_cv_skycolor > 50.0) && (fLightLevel < 15.0)) || ((g_f_cv_skycolor <= 50.0) && (fLightLevel < 40.0)))
            && !BotHasNvg(pBot)) && (pBot->bot_money >= 1250) && (g_iEquipAmmoRestricted[PB_WEAPON_NVGS]==0)) // KWo - 13.01.2012
         {
            if (g_bIsOldCS15)
               FakeClientCommand (pEdict, "buy;menuselect 8;menuselect 7\n");
            else
               FakeClientCommand (pEdict, "nvgs\n");
         }
         if (g_b_DebugCombat)
            ALERT(at_logged, "[DEBUG] BotBuyStuff - Bot %s %s the night googles; lightlevel = %d.\n", pBot->name, BotHasNvg(pBot) ? "bought": "didn't buy",(int)fLightLevel);
      }

      pBot->iBuyCount++;
      pBot->f_buy_time = gpGlobals->time + RANDOM_FLOAT (0.2, 0.5);
      return;
   }
   else
   {
      pBot->iBuyCount = 0; // Finished Buying
      if (g_b_DebugCombat)
         ALERT(at_logged,"[DEBUG] Bot %s %s night google.\n", pBot->name, BotHasNvg(pBot) ? "has" : "doesn't have");
   }
   return;
}


void BotSetConditions (bot_t *pBot)
{
   // Carried out each Frame.
   // Does all of the sensing, calculates Emotions and finally
   // sets the desired Action after applying all of the Filters

   static edict_t *pEdict;
   static edict_t *pPlayer;
   static bool bNewEnemy;
//   static bool bCheckNoiseOrigin;    // KWo - What is this for ???
   static float fDistance;
   static float fMinDistance;
   static float fVolume;
   static float fMaxVolume;
   static float fBotOwnSoundLast;
   static float fHearingDistance;
   static int ind;
   static int iEnemyIndex;
   static int iHearEnemyIndex;
   static int iHearEnemyDistance;
   static int iRandomMessage;
   static int iShootThruFreq;
   static Vector vRandom;
   static Vector vVisPos;
   static Vector vPredict;
   static Vector vLastEnLookDir; // KWo - 01.10.2010
   static TraceResult tr;
   static unsigned char cHit;
//   static unsigned char *pas;


   pEdict = pBot->pEdict;
   vLastEnLookDir.x = 90.0;      // KWo - 01.10.2010
   bNewEnemy = FALSE;            // KWo - 13.10.2011

   if ((pBot->iAimFlags & AIM_ENTITY) && (BotGetSafeTask(pBot)->iTask != TASK_PICKUPITEM))  // KWo - 27.10.2006
      pBot->iAimFlags = AIM_ENTITY;
   else if ((pBot->iAimFlags & AIM_LASTENEMY) && (pBot->vecLastEnemyOrigin != g_vecZero)
      && ((pBot->fLastHeardEnOrgUpdateTime >= gpGlobals->time)
          || (pBot->fLastSeenEnOrgUpdateTime >= gpGlobals->time)
          || (pBot->f_sound_update_time >= gpGlobals->time)
          || (pBot->f_bot_see_enemy_time + 2.0 < gpGlobals->time))) // KWo - 29.05.2010
      pBot->iAimFlags = AIM_LASTENEMY;
   else
      pBot->iAimFlags = 0;

   // Slowly increase/decrease dynamic Emotions back to their Base Level
   if (pBot->fNextEmotionUpdate < gpGlobals->time)
   {
      if (pBot->f_bot_see_enemy_time + 1.0 > gpGlobals->time)        // KWo - 10.04.2010
      {
         pBot->fAgressionLevel += 0.05;
         if (pBot->fAgressionLevel > 1.0)
            pBot->fAgressionLevel = 1.0;
      }
      else if (pBot->f_bot_see_enemy_time + 5.0 < gpGlobals->time)   // KWo - 10.04.2010
      {
         if (pBot->fAgressionLevel > pBot->fBaseAgressionLevel)
            pBot->fAgressionLevel -= 0.05;
         else
            pBot->fAgressionLevel += 0.05;

         if (pBot->fFearLevel > pBot->fBaseFearLevel)
            pBot->fFearLevel -= 0.05;
         else
            pBot->fFearLevel += 0.05;

         if (pBot->fAgressionLevel > 1.0)
            pBot->fAgressionLevel = 1.0;
         if (pBot->fFearLevel > 1.0)
            pBot->fFearLevel = 1.0;
         if (pBot->fAgressionLevel < 0.0)
            pBot->fAgressionLevel = 0.0;
         if (pBot->fFearLevel < 0.0)
            pBot->fFearLevel = 0.0;
      }
      pBot->fNextEmotionUpdate = gpGlobals->time + 0.5;
   }

   // Does Bot see an Enemy ?
   if (!g_bIgnoreEnemies) // KWo - 10.07.2008
   {
      BotFindEnemy(pBot);
   }
   else
   {
      pBot->iStates &= ~STATE_SEEINGENEMY;
   }
   if (pBot->f_bot_see_enemy_time + 2.0 < gpGlobals->time)        // KWo - 10.07.2008
   {
      pBot->iStates &= ~STATE_SEEINGENEMY;
      pBot->pBotEnemy = NULL;
      pBot->bShootThruSeen = FALSE;                               // KWo - 10.07.2008
   }
   else if (pBot->f_bot_see_enemy_time + 1.0 < gpGlobals->time)        // KWo - 11.01.2012
   {
      pBot->bShootThruSeen = FALSE;
   }


   // Did Bot just kill an Enemy ?
   if (!FNullEnt (pBot->pLastVictim))
   {
      if ((UTIL_GetTeam (pBot->pLastVictim) != pBot->bot_team) || g_b_cv_ffa)  // KWo - 05.10.2006
      {
         // Add some agression because we just killed somebody MUWHAHA !!
         pBot->fAgressionLevel += 0.1;
         if (pBot->fAgressionLevel > 1.0)
            pBot->fAgressionLevel = 1.0;

         // Taunt Enemy if we feel like it
         iRandomMessage = RANDOM_LONG (0, 100);
         if (g_b_cv_chat) // KWo - 06.04.2006
         {
            if ((iRandomMessage > 20) && (iRandomMessage < 30)) // KWo - 06.03.2010
            {
               BotPrepareChatMessage (pBot, szKillChat[RANDOM_LONG (0, iNumKillChats - 1)]);
               BotPushMessageQueue (pBot, MSG_CS_SAY);
            }
         }

         // Sometimes give some radio message
         if ((iRandomMessage < 20) && (!g_b_cv_ffa) && (g_b_cv_radio)
                        && (((pBot->bot_team == TEAM_CS_COUNTER) && (g_iAliveCTs > 1))
                         || ((pBot->bot_team == TEAM_CS_TERRORIST) && (g_iAliveTs > 1)))) // KWo - 06.03.2010
            BotPlayRadioMessage (pBot, RADIO_ENEMYDOWN);
      }

      pBot->pLastVictim = NULL;
   }

   // Check if our current enemy is still valid
   if (!FNullEnt (pBot->pLastEnemy))
   {
      iEnemyIndex = ENTINDEX (pBot->pLastEnemy) - 1; // KWo - 15.03.2010
      if ((iEnemyIndex >= 0) && (iEnemyIndex < gpGlobals->maxClients))
      {
         if (!IsAlive (pBot->pLastEnemy) && (clients[iEnemyIndex].fDeathTime < gpGlobals->time))
         {
            pBot->pLastEnemy = NULL;
         }
      }
   }
   else
   {
      pBot->pLastEnemy = NULL;
   }

   // Check if our enemy to hunt is still valid
   if (!FNullEnt (pBot->pHuntEnemy)) // KWo - 11.04.2010
   {
      if (!IsAlive (pBot->pHuntEnemy))
      {
         pBot->pHuntEnemy = NULL;
      }
   }
   else
   {
      pBot->pHuntEnemy = NULL;
   }

//   bCheckNoiseOrigin = FALSE; // KWo - what is this for?

   // Check sounds of other players
   // FIXME: Hearing is done by simulating and aproximation
   // Need to check if hooking the Server Playsound Routines
   // wouldn't give better results because the current method
   // is far too sensitive and unreliable

   // Don't listen if seeing enemy, just checked for sounds or being blinded ('cause its inhuman)
   if (!g_bIgnoreEnemies && (pBot->f_sound_update_time < gpGlobals->time)
//        && (pBot->f_bot_see_new_enemy_time + 0.5 < gpGlobals->time) // ??
        && (pBot->f_blind_time < gpGlobals->time)) // KWo - 11.07.2008
   {
      pBot->f_sound_update_time = gpGlobals->time + 0.05 + (100.f - (float)pBot->bot_skill) * 0.01 * g_f_cv_timer_sound;  // KWo - 12.10.2006

      fBotOwnSoundLast = 0.0;       // KWo - 12.10.2006
      iHearEnemyIndex = -1;         // KWo - 12.10.2006
      iHearEnemyDistance = 0;       // KWo - 12.10.2006

      if (clients[g_i_botthink_index].fTimeSoundLasting > gpGlobals->time)  // KWo - 12.10.2006
      {
         if (clients[g_i_botthink_index].fMaxTimeSoundLasting <= 0.0)
            clients[g_i_botthink_index].fMaxTimeSoundLasting = 0.5;
//         fBotOwnSoundLast = (clients[g_i_botthink_index].fHearingDistance * (0.2)) * (clients[g_i_botthink_index].fTimeSoundLasting - gpGlobals->time)/clients[g_i_botthink_index].fMaxTimeSoundLasting;
         if (clients[g_i_botthink_index].fTimeSoundLasting + 1.8 < gpGlobals->time)
            fBotOwnSoundLast = (0.2) * (clients[g_i_botthink_index].fHearingDistance / 1024.0); // KWo - 12.09.2008
      }

/*
      // Setup Engines Potentially Audible Set for this Bot
      vOrg = GetGunPosition (pEdict);
      if (pEdict->v.flags & FL_DUCKING)
         vOrg = vOrg + (VEC_HULL_MIN - VEC_DUCK_HULL_MIN);

//      pas = ENGINE_SET_PAS ((float *) &vOrg);
//fp = fopen ("PAS.txt", "a"); fprintf (fp, "ENGINE_SET_PAS (%f, %f, %f): 0x%X\n", vOrg.x, vOrg.y, vOrg.z, pas); fclose (fp);
*/
      pPlayer = NULL;
      fMinDistance = 9999;
      fMaxVolume = 0.0; // KWo - 01.08.2006
      fVolume = 0.0;

      if (!FNullEnt(pEdict->v.dmg_inflictor)) // KWo - 10.04.2010
      {
         ind = ENTINDEX(pEdict->v.dmg_inflictor) - 1;
         if ((ind >= 0) && (ind < gpGlobals->maxClients) && (ind != g_i_botthink_index)) // KWo - 29.09.2010
         {
            if ((clients[ind].iFlags & CLIENT_ALIVE) && (pEdict->v.dmgtime + 1.5 > gpGlobals->time)) // KWo - 19.01.2011
            {
               if ((FNullEnt(pBot->pBotEnemy)) || (pBot->f_bot_see_enemy_time + 1.0 < gpGlobals->time)
                  && (pBot->pLastEnemy != pPlayer))
               {
                  iHearEnemyIndex = ind;
                  pPlayer = pEdict->v.dmg_inflictor;
                  fMinDistance = 500.0;
                  fVolume = fBotOwnSoundLast * 10 + 500;

                  iHearEnemyDistance = (int)(pPlayer->v.origin - pEdict->v.origin).Length();
                  if (iHearEnemyDistance > (int)fMinDistance)
                     iHearEnemyDistance = (int)fMinDistance;
/*
                  pBot->pLastEnemy = pPlayer;
                  pBot->vecLastEnemyOrigin = pPlayer->v.origin;
                  pBot->vecLastEnemyOrigin.x += RANDOM_FLOAT(-200.0,200.0);
                  pBot->vecLastEnemyOrigin.y += RANDOM_FLOAT(-200.0,200.0);
                  pBot->fLastHeardEnOrgUpdateTime = gpGlobals->time + 1.0;
                  pBot->fChangeAimDirectionTime = gpGlobals->time + 1.0;
                  pBot->iAimFlags |= AIM_LASTENEMY;
*/
               }
            }
            else
            {
               if ((pBot->pBotEnemy == pEdict->v.dmg_inflictor)
                  && (((clients[ind].iTeam == pBot->bot_team) && (!g_b_cv_ffa))
                  || !(clients[ind].iFlags & CLIENT_ALIVE) && (clients[ind].fDeathTime < gpGlobals->time)
                  || (pBot->f_bot_see_enemy_time + 7.0 < gpGlobals->time)))    // 15.10.2011
               {
                  pEdict->v.dmg_inflictor = NULL;
                  pBot->pBotEnemy = NULL;
                  pBot->pLastEnemy = NULL;      // KWo - 13.09.2011
               }
            }
         }
      }

      // Loop through all enemy clients to check for hearable stuff
      for (ind = 0; ind < gpGlobals->maxClients; ind++)
      {
         if (!(clients[ind].iFlags & CLIENT_USED)
             || !(clients[ind].iFlags & CLIENT_ALIVE)
             || (clients[ind].pEdict == pEdict)
             || (clients[ind].fTimeSoundLasting < gpGlobals->time))
            continue;

         fDistance = (clients[ind].vecSoundPosition - pEdict->v.origin).Length ();
         if (fDistance < 1.0)
            fDistance = 1.0;

         fHearingDistance = clients[ind].fHearingDistance;

         if (fDistance > fHearingDistance) // KWo - 09.10.2006
            continue;

         if (clients[ind].fMaxTimeSoundLasting <= 0.0) // KWo - 01.08.2006
            clients[ind].fMaxTimeSoundLasting = 1.0;

         if (clients[ind].fTimeSoundLasting < gpGlobals->time) // KWo - 12.09.2008
            continue;
/*

         if (clients[ind].fTimeSoundLasting - gpGlobals->time > 0.5)
         {
            if (fDistance < 1.0)
               fDistance = 1.0;
            fVolume = fHearingDistance * (1.0 - fDistance / fHearingDistance);
         }
         else if (fDistance <= 0.5 * fHearingDistance) // KWo - 12.10.2006
         {
            if (fDistance < 1.0)
               fDistance = 1.0;

            fVolume = fHearingDistance * (1.0 - fDistance / fHearingDistance) * (clients[ind].fTimeSoundLasting - gpGlobals->time)/clients[ind].fMaxTimeSoundLasting;
         }

         else
            fVolume = 2.0 * fHearingDistance * (1.0 - fDistance / fHearingDistance) * (clients[ind].fTimeSoundLasting - gpGlobals->time)/clients[ind].fMaxTimeSoundLasting; // KWo - 01.08.2006

*/

         fVolume = fHearingDistance/fDistance; // KWo - 12.09.2008

         if ((clients[ind].iTeam == pBot->bot_team) && !(g_b_cv_ffa)) // KWo - 16.10.2006
            fVolume = 0.3 * fVolume;

         if (fVolume < fBotOwnSoundLast) // KWo - 12.10.2006
            continue;
// changed back to hear the closest one... KWo - 31.03.2008
         if (fDistance > fMinDistance)
            continue;

         fMinDistance = fDistance;
         fMaxVolume = fVolume; // KWo - 01.08.2006
         iHearEnemyIndex = ind; // KWo - 12.10.2006
         iHearEnemyDistance = (int)fDistance; // KWo - 12.10.2006
      }

      if (iHearEnemyIndex >= 0)
      {
         if ((clients[iHearEnemyIndex].iTeam != pBot->bot_team) || (g_b_cv_ffa)) // KWo - 12.10.2006
         // Despite its name it also checks for sounds...
         // NOTE: This only checks if sounds could be heard from
         // this position in theory but doesn't care for Volume or
         // real Sound Events. Even if there's no noise it returns true,
         // so we still have the work of simulating sound levels
//         if (ENGINE_CHECK_VISIBILITY (clients[iHearEnemyIndex].pEdict, pas))
            pPlayer = clients[iHearEnemyIndex].pEdict;
      }


      // Did the Bot hear someone ?
      if (!FNullEnt (pPlayer))
      {
         if (((iHearEnemyDistance < 2000) || BotUsesSniper(pBot))) // KWo - 11.07.2008
         {
         // Didn't Bot already have an enemy ? Take this one...
            if ((pBot->vecLastEnemyOrigin == g_vecZero) || (pBot->pLastEnemy == NULL)) // KWo - 15.08.2007
            {
               if (pBot->f_bot_see_new_enemy_time + 1.0 < gpGlobals->time)             // KWo - 11.07.2008
               {
                  pBot->f_heard_sound_time = gpGlobals->time;
                  pBot->iStates |= STATE_HEARINGENEMY;
                  if (iHearEnemyDistance < 600)    // KWo - 08.04.2010
                     pBot->iStates |= STATE_SUSPECTENEMY;
                  pBot->pLastEnemy = pPlayer;
                  pBot->vecLastEnemyOrigin = pPlayer->v.origin;
                  pBot->vecLastEnemyOrigin.x += RANDOM_FLOAT(-200.0,200.0);   // KWo - 05.05.2007
                  pBot->vecLastEnemyOrigin.y += RANDOM_FLOAT(-200.0,200.0);   // KWo - 05.05.2007
                  pBot->fLastHeardEnOrgUpdateTime = gpGlobals->time + 1.0;    // KWo - 05.05.2007
                  pBot->fShootThruHeardCheckTime = 0.0;                       // KWo - 05.05.2007
//                  pBot->fChangeAimDirectionTime = gpGlobals->time + 1.0;      // KWo - 24.02.2008
                  pBot->iAimFlags |= AIM_LASTENEMY;
                  bNewEnemy = TRUE;
               }
               else
               {
                  pPlayer = NULL;
                  goto endhearing;
               }

               if (g_b_DebugSensing)
               {
                  ALERT(at_logged, "[DEBUG] BotSetCond (1) Bot %s heard a new enemy %s - distance = %d, volume = %d, randomizing position....\n", pBot->name, STRING(pPlayer->v.netname), iHearEnemyDistance, (int)fMaxVolume);
               }
            }
            // Bot had an enemy, check if it's the heard one
            else
            {
               if (pPlayer == pBot->pLastEnemy)
               {
                  pBot->f_heard_sound_time = gpGlobals->time;
                  pBot->iStates |= STATE_HEARINGENEMY;

                  // Bot sees enemy ? then bail out !
                  if (pBot->iStates & STATE_SEEINGENEMY)
                     goto endhearing;

                  if (iHearEnemyDistance < 600)    // KWo - 08.04.2010
                     pBot->iStates |= STATE_SUSPECTENEMY;

                  if ((pBot->fLastHeardEnOrgUpdateTime < gpGlobals->time)
                     && (pBot->fLastSeenEnOrgUpdateTime < gpGlobals->time))      // KWo - 14.07.2008
                  {
                     pBot->vecLastEnemyOrigin = pPlayer->v.origin;
                     pBot->vecLastEnemyOrigin.x += RANDOM_FLOAT(-300.0,300.0);   // KWo - 05.05.2007
                     pBot->vecLastEnemyOrigin.y += RANDOM_FLOAT(-300.0,300.0);   // KWo - 05.05.2007
                     pBot->fLastHeardEnOrgUpdateTime = gpGlobals->time + 1.0;    // KWo - 05.05.2007
                     pBot->iAimFlags |= AIM_LASTENEMY;

                     if (g_b_DebugSensing)
                     {
                        ALERT(at_logged, "[DEBUG] BotSetCond (2) Bot %s heard old enemy %s - distance = %d, volume = %d, randomizing position....\n", pBot->name, STRING(pPlayer->v.netname), iHearEnemyDistance, (int)fMaxVolume);
                     }
                  }
               }
               else
               {
                  // If Bot had an enemy but the heard one is nearer, take it instead
                  fDistance = (pBot->vecLastEnemyOrigin - pEdict->v.origin).Length ();
                  if ((0.8 * fDistance > (pPlayer->v.origin - pEdict->v.origin).Length ())  // KWo - 16.12.2007
                   && (pBot->f_bot_see_new_enemy_time + 1.0 < gpGlobals->time))
                  {
                     if ((pBot->iStates & STATE_SEEINGENEMY) && (pBot->f_bot_see_enemy_time + 1.0 > gpGlobals->time)) // KWo - 09.04.2010
                     {
                        pBot->f_heard_sound_time = gpGlobals->time;
                        pBot->iStates |= STATE_HEARINGENEMY;
                        goto endhearing; // that also should stop bots shaking...
                     }
                     else if (FBoxVisible (pBot, pPlayer, &vVisPos, &cHit)) // KWo - 05.07.2008
                     {
                        pBot->f_heard_sound_time = gpGlobals->time;
                        pBot->iStates |= STATE_HEARINGENEMY;
                        pBot->f_bot_see_new_enemy_time = gpGlobals->time;
                        pBot->f_bot_see_enemy_time = gpGlobals->time;
                        pBot->pLastEnemy = pPlayer;
                        pBot->vecLastEnemyOrigin = vVisPos;
                        pBot->pBotEnemy = pPlayer;
                        pBot->vecEnemy = vVisPos;
                        pBot->vecVisPos = vVisPos;
                        pBot->ucVisibility = cHit;
                        pBot->iStates |= STATE_SEEINGENEMY;
                        pBot->iStates &= ~STATE_SUSPECTENEMY;
                        pBot->iAimFlags |= AIM_LASTENEMY;
                        pBot->iAimFlags |= AIM_ENEMY;
                        pBot->fChangeAimDirectionTime = gpGlobals->time + 1.0;
                        pBot->bShootThruHeard = FALSE;

                        if (g_b_DebugSensing)
                           ALERT(at_logged, "[DEBUG] BotSetCond (5) Bot %s heard a new enemy %s and can see him  - distance = %d, volume = %d.\n",
                              pBot->name, STRING(pPlayer->v.netname), iHearEnemyDistance, (int)fMaxVolume);

                        goto endhearing;
                     }
                     else if ((pBot->fLastHeardEnOrgUpdateTime < gpGlobals->time)
                        && (pBot->fLastSeenEnOrgUpdateTime < gpGlobals->time))
                     {
                        pBot->f_heard_sound_time = gpGlobals->time;
                        pBot->iStates |= STATE_HEARINGENEMY;
                        if (iHearEnemyDistance < 600)    // KWo - 08.04.2010
                           pBot->iStates |= STATE_SUSPECTENEMY;
                        pBot->pLastEnemy = pPlayer;
                        pBot->vecLastEnemyOrigin = pPlayer->v.origin;
                        pBot->vecLastEnemyOrigin.x += RANDOM_FLOAT(-300.0,300.0);               // KWo - 05.05.2007
                        pBot->vecLastEnemyOrigin.y += RANDOM_FLOAT(-300.0,300.0);               // KWo - 05.05.2007
                        pBot->fLastHeardEnOrgUpdateTime = gpGlobals->time + 1.0;                // KWo - 05.05.2007
                        pBot->fShootThruHeardCheckTime = 0.0;
//                        pBot->f_bot_see_new_enemy_time = gpGlobals->time;                       // KWo - 15.07.2008
//                        pBot->fChangeAimDirectionTime = gpGlobals->time + 1.0;                  // KWo - 15.07.2008

                        if (g_b_DebugSensing)
                        {
                           ALERT(at_logged, "[DEBUG] BotSetCond (3) Bot %s had an enemy and heard a new one %s - distance = %d, volume = %d, randomizing position....\n",
                              pBot->name, STRING(pPlayer->v.netname), iHearEnemyDistance, (int)fMaxVolume);
                        }

                        goto shootthruwcheck;
                     }
/*
                     else if ((pBot->fLastHeardEnOrgUpdateTime >= gpGlobals->time)
                              || (pBot->fLastSeenEnOrgUpdateTime >= gpGlobals->time))
                     {
                        goto shootthruwcheck;
                     }
*/
                  }
                  else if (pBot->f_bot_see_new_enemy_time + 1.0 >= gpGlobals->time)
                  {
                     pPlayer = NULL;
                     goto endhearing;
                  }
                  else if (0.8 * fDistance <= (pPlayer->v.origin - pEdict->v.origin).Length ())  // KWo - 11.07.2008
                     goto shootthruwcheck;
               }
            }
         }

         // Check if heard enemy can be seen
         if (FBoxVisible (pBot, pPlayer, &vVisPos, &cHit))
         {
            pBot->pBotEnemy = pPlayer;
            pBot->pLastEnemy = pPlayer;
            vRandom = Vector(RANDOM_FLOAT(-BotAimTab[pBot->bot_skill / 20].fAim_X,BotAimTab[pBot->bot_skill / 20].fAim_X),
                             RANDOM_FLOAT(-BotAimTab[pBot->bot_skill / 20].fAim_Y,BotAimTab[pBot->bot_skill / 20].fAim_Y),
                             RANDOM_FLOAT(-BotAimTab[pBot->bot_skill / 20].fAim_Z,BotAimTab[pBot->bot_skill / 20].fAim_Z));  // KWo - 27.01.2008

            pBot->vecLastEnemyOrigin = vVisPos + vRandom;            // KWo - 27.01.2008
            pBot->vecEnemy = vVisPos + vRandom;                      // KWo - 27.01.2008
            pBot->vecVisPos = vVisPos + vRandom;                     // KWo - 27.01.2008
            pBot->ucVisibility = cHit;
            pBot->iStates |= STATE_SEEINGENEMY;
            pBot->iStates &= ~STATE_SUSPECTENEMY;                    // KWo - 09.02.2008
            pBot->f_bot_see_enemy_time = gpGlobals->time;
            if (bNewEnemy)                                           // KWo - 14.07.2008
            {
               pBot->f_bot_see_new_enemy_time = gpGlobals->time;
               pBot->fChangeAimDirectionTime = gpGlobals->time + 1.0;   // KWo - 17.02.2008
            }
            pBot->iAimFlags |= AIM_LASTENEMY;
            pBot->iAimFlags |= AIM_ENEMY;                            // KWo - 27.01.2008
            pBot->bShootThruHeard = FALSE;                           // KWo - 10.07.2008
            goto endhearing;
         }
         // Check if heard enemy can be shoot through some obstacle
         else if (pBot->pLastEnemy == pPlayer)
         {
//            pBot->fChangeAimDirectionTime = gpGlobals->time + 1.0;               // KWo - 17.02.2008
            pBot->iAimFlags |= AIM_LASTENEMY;
         }
      }

shootthruwcheck:

      if (!FNullEnt (pBot->pLastEnemy) && (pBot->f_heard_sound_time + 1.5 >= gpGlobals->time)
            && (pBot->fShootThruHeardCheckTime < gpGlobals->time)
            && !(pBot->iStates & STATE_SEEINGENEMY) && (pBot->vecLastEnemyOrigin != g_vecZero)) // KWo - 13.08.2008
      {
         vLastEnLookDir = UTIL_VecToAngles (pBot->vecLastEnemyOrigin - GetGunPosition (pEdict)); // KWo - 01.10.2010
         vLastEnLookDir.x = -vLastEnLookDir.x;  // KWo - 01.10.2010
         if (fabs(vLastEnLookDir.x) < 45.0)     // KWo - 01.10.2010
         {
            iShootThruFreq = BotAimTab[pBot->bot_skill / 20].iHeardShootThruProb;
			   pBot->fShootThruHeardCheckTime = gpGlobals->time + 1.0;
            if ((g_b_cv_shootthruwalls) && WeaponShootsThru (pBot->current_weapon.iId)
               && (RANDOM_LONG (1, 100) <= iShootThruFreq))
            {
               if ((BotLastEnemyShootable(pBot)) && IsShootableThruObstacle (pEdict, pBot->vecLastEnemyOrigin))
               {
                  pBot->fChangeAimDirectionTime = gpGlobals->time + 1.2;
			         pBot->bShootThruHeard = TRUE;
               }
               else
               {
                  if (pBot->bShootThruHeard) // KWo - 13.08.2008
                     pBot->fShootThruHeardCheckTime = gpGlobals->time + 15.0;
                  pBot->bShootThruHeard = FALSE;
               }
            }
            else
            {
               pBot->bShootThruHeard = FALSE;
            }
         }
         else
         {
            pBot->bShootThruHeard = FALSE;
         }
      }
      if (!FNullEnt (pBot->pLastEnemy) && (pBot->bShootThruHeard)) // KWo - 13.07.2008
      {
         pBot->iAimFlags |= AIM_LASTENEMY;
         pBot->iStates |= STATE_SUSPECTENEMY;
         if ((pBot->f_bot_see_enemy_time + 1.0 < gpGlobals->time)
            && (pBot->fLastHeardEnOrgUpdateTime < gpGlobals->time)
            && (pBot->fLastSeenEnOrgUpdateTime < gpGlobals->time))
         {
            pBot->vecLastEnemyOrigin.x += RANDOM_FLOAT (-300.0, 300.0);
            pBot->vecLastEnemyOrigin.y += RANDOM_FLOAT (-300.0, 300.0);
            pBot->fLastHeardEnOrgUpdateTime = gpGlobals->time + 1.0;
         }

         if (g_b_DebugSensing)
         {
            fDistance = (pEdict->v.origin - pBot->pLastEnemy->v.origin).Length();
               ALERT(at_logged, "[DEBUG] BotSetCond (4) Bot %s heard an enemy (and wants to shoot through the wall) %s - distance = %d, randomizing position....\n",
                  pBot->name, STRING(pBot->pLastEnemy->v.netname), (int)fDistance);
         }
      }
   }
   else if (pBot->f_sound_update_time >= gpGlobals->time)
   {
      if (pBot->f_heard_sound_time + 10.0 < gpGlobals->time)
      {
         pBot->iStates &= ~STATE_HEARINGENEMY;
         pBot->bShootThruHeard = false;
         if ((FNullEnt(pBot->pBotEnemy)) && (pBot->f_bot_see_enemy_time + 10.0 < gpGlobals->time)) // KWo - 29.05.2010
         {
            pBot->vecLastEnemyOrigin = g_vecZero;
            pBot->pLastEnemy = NULL;
            pBot->iAimFlags &= ~AIM_LASTENEMY;
         }
      }
   }

endhearing:

   if ((!FNullEnt(pBot->pLastEnemy)) && (pBot->vecLastEnemyOrigin != g_vecZero))
      pBot->iAimFlags |= AIM_LASTENEMY;

   if ((pBot->iAimFlags & AIM_LASTENEMY) && (!FNullEnt(pBot->pLastEnemy))
       && (pBot->vecLastEnemyOrigin != g_vecZero)
       && !(pBot->iAimFlags & AIM_ENEMY)) // KWo - 11.01.2012
   {
      TRACE_LINE (GetGunPosition (pEdict), pBot->vecLastEnemyOrigin, ignore_monsters, pEdict, &tr);
      fDistance = (pEdict->v.origin - pBot->vecLastEnemyOrigin).Length();

      if (((fDistance >= 300.0) && FNullEnt(pBot->pBotEnemy)
          && (pBot->f_bot_see_enemy_time + 2.0 < gpGlobals->time) && (tr.flFraction <= 0.2)
          && (FStrEq(STRING (tr.pHit->v.classname), "worldspawn"))
          && !(pBot->bShootThruSeen) && !(pBot->bShootThruHeard))
          || (!(pBot->bShootThruSeen) && (fabs(vLastEnLookDir.x) >= 45.0)))     // KWo - 01.10.2010
      {
         pBot->iAimFlags &= ~AIM_LASTENEMY;
      }
      else if (bNewEnemy)   // KWo - 13.10.2011
         pBot->fChangeAimDirectionTime = gpGlobals->time + 1.0;
   }
   else
      pBot->iAimFlags &= ~AIM_LASTENEMY;



   // Clear suspected Flag
   if ((pBot->f_bot_see_enemy_time + 8.0 < gpGlobals->time) && (pBot->f_heard_sound_time + 10.0 < gpGlobals->time)) // KWo - 10.07.2008
      pBot->iStates &= ~STATE_SUSPECTENEMY;
}


void BotCheckGrenadeThrow (bot_t *pBot) // KWo - 11.04.2010
{
   static edict_t *pEdict;
   static bottask_t TempTask;
   static bool bUsingGrenade;
   static bool bThrowGrenade;
   static bool bWantThrowFlashbang;
   static float fDistance;
   static float fEnemyDot;
   static float fRandom;
   static float fSearchRadius;
   static int iCount;
   static int iEnemyIndex;
   static int iEnWepID;
   static int iGrenadeType;
   static int iNumTeamnates;
   static int iTask;
   static int iWeapons;
   static int rgiWptTab[4];
   static Vector vDest;
   static Vector vDistance;
   static Vector vMovDir;
   static Vector vSource;
   static Vector vSourceThrow;
   static Vector vPredict;


   pEdict = pBot->pEdict;

   // Check if throwing a Grenade is a good thing to do...
   bUsingGrenade = pBot->bUsingGrenade;
   bThrowGrenade = FALSE;
   iTask = BotGetSafeTask(pBot)->iTask;
   if ((pBot->f_grenade_check_time < gpGlobals->time)
      && (iTask != TASK_THROWHEGRENADE) && (iTask != TASK_THROWFLASHBANG)
      && (iTask != TASK_THROWSMOKEGRENADE) && (!g_bIgnoreEnemies) && (!pBot->bIsReloading)
       && (pBot->current_weapon.iId != CS_WEAPON_INSWITCH) && (!g_b_cv_jasonmode)
      /* && (strncmp ("zomb", pBot->sz_BotModelName, 4) != 0) */) // KWo - 14.06.2008
   {
      // Check again in some seconds
      pBot->f_grenade_check_time = gpGlobals->time + g_f_cv_timer_grenade; // KWo - 06.04.2006

      iWeapons = pBot->pEdict->v.weapons; // KWo - 21.03.2006
      bWantThrowFlashbang = FALSE; // KWo - 21.03.2006
      if ((pBot->current_weapon.iId != CS_WEAPON_HEGRENADE) && (iWeapons & (1 << CS_WEAPON_FLASHBANG))) // KWo - 20.06.2006
         bWantThrowFlashbang = BotCheckCorridor(pBot); // KWo - 21.03.2006

      if (bWantThrowFlashbang) // KWo - 21.03.2006
      {
         bThrowGrenade = TRUE;

         if (g_b_DebugSensing)
            ALERT (at_logged, "[DEBUG] Bot %s wants to throw a flashbang .\n", pBot->name);

         if ((pBot->prev_wpt_index[0]<0) || (pBot->prev_wpt_index[0] >= g_iNumWaypoints))
            bThrowGrenade = FALSE;
         if (bThrowGrenade)
         {
            vMovDir = (pBot->wpt_origin - paths[pBot->prev_wpt_index[0]]->origin).Normalize();
            vMovDir = vMovDir * 500;
            vDest = pBot->wpt_origin + vMovDir;

            if (NumTeammatesNearPos (pBot, vDest, 256) > 0)
            {
               bThrowGrenade = FALSE;
            }
            else
            {
               pBot->vecThrow = vDest;
            }
         }
         if (bThrowGrenade)
         {
            pBot->iStates |= STATE_THROWFLASHBANG;
            TempTask = taskFilters[TASK_THROWFLASHBANG];
            TempTask.fDesire = TASKPRI_THROWGRENADE;
            TempTask.fTime = gpGlobals->time + 3.0;

            BotPushTask(pBot, &TempTask);
            pBot->f_grenade_check_time = gpGlobals->time + 3.0;
         }
         else if (pBot->current_weapon.iId != CS_WEAPON_FLASHBANG)
            pBot->iStates &= ~STATE_THROWFLASHBANG;
      }
      else if ((!FNullEnt (pBot->pLastEnemy)) && (pBot->vecLastEnemyOrigin != g_vecZero)) // KWo - 23.07.2007
      {
         if (IsAlive (pBot->pLastEnemy)) // KWo - 06.08.2006
         {
            // Check if we have Grenades to throw
            // If we don't have grenades no need to check
            // it this round again -
            // KWo - that above is wrong - how about CSDM? ;)

            iGrenadeType = BotCheckGrenades (pBot);

            if ((iGrenadeType == CS_WEAPON_HEGRENADE) || (iGrenadeType == CS_WEAPON_SMOKEGRENADE)) // KWo - 11.04.2010
            {
               fDistance = (pBot->vecLastEnemyOrigin - pEdict->v.origin).Length ();

               // Too high to throw ?
               if (pBot->vecLastEnemyOrigin.z > pEdict->v.origin.z + 500.0)
                  fDistance = 9999;

               // The enemy is facing what?
               fEnemyDot = GetShootingConeDeviation (pBot->pLastEnemy, &pEdict->v.origin);
               iNumTeamnates = NumTeammatesNearPos(pBot, pBot->pLastEnemy->v.origin, 512);

               // Enemy within a good Throw distance ?
               if (pBot->iStates & STATE_SEEINGENEMY)
               {
                  iEnWepID = CS_WEAPON_KNIFE; // KWo - 14.06.2008
                  iEnemyIndex = ENTINDEX(pBot->pLastEnemy) - 1; // KWo - 17.03.2007
                  if (IsAlive(pBot->pLastEnemy))
                     iEnWepID = clients[iEnemyIndex].iCurrentWeaponId;

                  if ((iNumTeamnates == 0) && (fDistance > 500) && (fDistance < 1600)
                     && ((!WeaponIsPrimaryGun(iEnWepID))
                         || ((fEnemyDot < 0.7) && (pBot->pLastEnemy->v.oldbuttons & IN_ATTACK))
                            /* || (fEnemyDot < 0.4) */ ))
                  {
                     vPredict = pBot->pLastEnemy->v.velocity * fDistance / 400.0;  // KWo - 18.06.2006
                     vPredict.z = 0.0;
                     vPredict = vPredict + pBot->pLastEnemy->v.origin;

                     pBot->vecThrow = vPredict;
                     vSourceThrow = VecCheckThrow (pBot, GetGunPosition (pEdict), pBot->vecThrow, 1.0);

                     if (vSourceThrow == g_vecZero)
                     {
                        vSourceThrow = VecCheckToss (pBot, GetGunPosition (pEdict), pBot->vecThrow);
                        if (vSourceThrow != g_vecZero) // KWo - 24.02.2008
                           pBot->vecThrow = pBot->vecThrow + Vector(0.0, 0.0, 110.0);
                     }

                     if (vSourceThrow != g_vecZero)
                        bThrowGrenade = TRUE;
                  }
               }
               else if ((pBot->iStates & STATE_SUSPECTENEMY)
                  && ((pBot->f_bot_see_enemy_time + 4.0 > gpGlobals->time)
                     || (pBot->f_heard_sound_time + 4.0 > gpGlobals->time)))
               {
                  if ((iNumTeamnates == 0) && (fDistance > 500) && (fDistance < 1600))
                  {
                     vPredict = pBot->pLastEnemy->v.velocity * fDistance / 400.0;  // KWo - 18.06.2006
                     vPredict.z = 0.0;
                     vPredict = vPredict + pBot->pLastEnemy->v.origin;

                     iCount = 4;
                     fSearchRadius = pBot->pLastEnemy->v.velocity.Length2D ();

                     // check the search radius
                     if (fSearchRadius < 128.0)
                        fSearchRadius = 128.0;

                     // search waypoints
                     WaypointFindInRadius(vPredict, fSearchRadius, &rgiWptTab[0], &iCount);

                     while (iCount > 0)
                     {
                        bThrowGrenade = true;

                        // check the throwing
                        pBot->vecThrow = paths[rgiWptTab[iCount--]]->origin;
                        vSourceThrow = VecCheckThrow (pBot, GetGunPosition (pEdict), pBot->vecThrow, 1.0);

                        if ((vSourceThrow).Length() < 10.0)
                           vSourceThrow = VecCheckToss (pBot, GetGunPosition (pEdict), pBot->vecThrow);
                           if (vSourceThrow != g_vecZero)
                              pBot->vecThrow = pBot->vecThrow + Vector(0.0, 0.0, 110.0);

                        if (vSourceThrow == g_vecZero)
                           bThrowGrenade = false;
                        else
                           break;
                     }

                  }
               }
               if (bThrowGrenade)
               {
                  fRandom = RANDOM_FLOAT(0.0,100.0);

                  if ((iWeapons & (1 << CS_WEAPON_HEGRENADE)) && (iWeapons & (1 << CS_WEAPON_SMOKEGRENADE)))
                  {
                     if ((fRandom > 20) || (pBot->current_weapon.iId == CS_WEAPON_HEGRENADE)) // KWo - 20.06.2006
                        iGrenadeType = CS_WEAPON_HEGRENADE;
                     else
                        iGrenadeType = CS_WEAPON_SMOKEGRENADE;
                  }
                  if ((iWeapons & (1 << CS_WEAPON_HEGRENADE)) && !(iWeapons & (1 << CS_WEAPON_SMOKEGRENADE)))
                     iGrenadeType = CS_WEAPON_HEGRENADE;
                  if (!(iWeapons & (1 << CS_WEAPON_HEGRENADE)) && (iWeapons & (1 << CS_WEAPON_SMOKEGRENADE)))
                     iGrenadeType = CS_WEAPON_SMOKEGRENADE;

                  // Care about different Grenades
                  switch (iGrenadeType)
                  {
                     case CS_WEAPON_HEGRENADE:
                     {
                        // Start throwing ?
                        pBot->iStates |= STATE_THROWHEGREN;
                        break;
                     }
                     case CS_WEAPON_SMOKEGRENADE:
                     {
                        // Start throwing ?
                        if (pBot->iStates & STATE_SEEINGENEMY)
                           pBot->iStates |= STATE_THROWSMOKEGREN;
                        else
                        {
                           bThrowGrenade = FALSE;
                           pBot->iStates &= ~STATE_THROWSMOKEGREN;
                        }
                        break;
                     }
                  }

                  TempTask = taskFilters[TASK_THROWHEGRENADE];
                  TempTask.fDesire = TASKPRI_THROWGRENADE;
                  TempTask.fTime = gpGlobals->time + 4.0;

                  if (bThrowGrenade) // KWo - 24.02.2006
                  {
                     pBot->f_grenade_check_time = gpGlobals->time + 6.0;
                  }
                  if (pBot->iStates & STATE_THROWHEGREN)
                     BotPushTask (pBot, &TempTask);
                  else if (pBot->iStates & STATE_THROWSMOKEGREN)
                  {
                     TempTask.iTask = TASK_THROWSMOKEGRENADE;
                     BotPushTask (pBot, &TempTask);
                  }
               }
               else
               {
                  pBot->iStates &= ~STATE_THROWHEGREN;
                  pBot->iStates &= ~STATE_THROWSMOKEGREN;
               }
            }
            else
            {
               pBot->iStates &= ~STATE_THROWHEGREN;
               pBot->iStates &= ~STATE_THROWSMOKEGREN;
            }
         }
      }
   }
   else if (g_bIgnoreEnemies || pBot->bIsReloading || g_b_cv_jasonmode) // KWo - 13.07.2008
   {
      pBot->iStates &= ~STATE_THROWHEGREN;
      pBot->iStates &= ~STATE_THROWFLASHBANG;
      pBot->iStates &= ~STATE_THROWSMOKEGREN;
   }

   iTask = BotGetSafeTask(pBot)->iTask; // KWo - 13.07.2008

   if (iTask != TASK_THROWHEGRENADE)
      pBot->iStates &= ~STATE_THROWHEGREN;
   else
      pBot->bUsingGrenade = TRUE;

   if (iTask != TASK_THROWFLASHBANG)
      pBot->iStates &= ~STATE_THROWFLASHBANG;
   else
      pBot->bUsingGrenade = TRUE;

   if (iTask != TASK_THROWSMOKEGRENADE)
      pBot->iStates &= ~STATE_THROWSMOKEGREN;
   else
      pBot->bUsingGrenade = TRUE;

}


void BotPrepareTask (bot_t *pBot) // KWo - 10.04.2010
{
// part dedicated to selecting the particular task to do...
   static edict_t *pEdict;
   static edict_t *pItem;
   static bottask_t *ptaskOffensive;
   static bottask_t *ptaskPickup;
   static bottask_t *ptaskSurvive;
   static bottask_t *pDefault;
   static bottask_t *pSub1;
   static bottask_t *pFinal;
   static bottask_t TempTask;
   static bot_weapon_select_t *pWeaponTab;
   static float fDistance;
   static float fLevel;
   static float fMinDistance;
   static float fRadius;
   static float fRatio;
   static float fRetreatLevel;
   static float fTempAgression;
   static float fTempFear;
   static float fTimeHeard;
   static float fTimeSeen;
   static int iAliveEnemies;
   static int iAliveTeamnates;
   static int iApproach;
   static int iAwayIndex;
   static int iEnemyNum;
   static int iEnWepID;
   static int iEnemyIndex;
   static int iFriendlyNum;
   static int iGoalIndex;
   static int iGoalChoices[4];
   static int iId;
   static int i;
   static int ind;
   static int iMinIndex;
   static int iSearch;
   static int iTempWp;
   static int iWeap;
   static int *ptrWeaponTab;
   static int rgi_WaypointTab[10];
   static Vector vDest;
   static Vector vecPickme;

   pEdict = pBot->pEdict;
   fTempFear = pBot->fFearLevel;
   fTempAgression = pBot->fAgressionLevel;
   iFriendlyNum = 0;
   iEnemyNum = 0;

   // Decrease Fear if Teammates near
   if (pBot->vecLastEnemyOrigin != g_vecZero)
      iFriendlyNum = NumTeammatesNearPos (pBot, pEdict->v.origin, 300) - NumEnemiesNearPos (pBot, pBot->vecLastEnemyOrigin, 500);
   if (iFriendlyNum > 0)
      fTempFear = fTempFear * 0.5;

   // Increase/Decrease Fear/Agression if Bot uses a sniping weapon
   // to be more careful
   if (BotUsesSniper (pBot))
   {
      fTempFear = fTempFear * 1.5;
      fTempAgression = fTempAgression * 0.5; // KWo - 26.08.2006
   }

   // Initialize & Calculate the Desire for all Actions based on
   // distances, Emotions and other Stuff

   BotGetSafeTask(pBot);
   iAliveTeamnates = 0;
   iAliveEnemies = 0;

   if (g_b_cv_ffa)                                 // KWo - 19.01.2008
   {
      iAliveEnemies = g_iAliveCTs + g_iAliveTs - 1;
   }
   else if (pBot->bot_team == TEAM_CS_TERRORIST)   // KWo - 19.01.2008
   {
      iAliveTeamnates = g_iAliveTs;
      iAliveEnemies = g_iAliveCTs;
   }
   else if (pBot->bot_team == TEAM_CS_COUNTER)     // KWo - 19.01.2008
   {
      iAliveTeamnates = g_iAliveCTs;
      iAliveEnemies = g_iAliveTs;
   }

   if ((g_iMapType & MAP_DE) && (pBot->bot_team == TEAM_CS_TERRORIST)
      && (pEdict->v.weapons & (1 << CS_WEAPON_C4)) && (g_i_botthink_index == g_iFrameCounter)
      && (pBot->chosengoal_index >= 0) && (pBot->chosengoal_index < g_iNumWaypoints)) // KWo - 26.04.2008
   {
      if ((BotGetSafeTask(pBot)->iTask != TASK_MOVETOPOSITION) && (BotGetSafeTask(pBot)->fDesire < TASKPRI_HIDE)
          && (g_iDebugGoalIndex == -1) && (paths[pBot->chosengoal_index]->flags & W_FL_GOAL)
         && ((paths[pBot->chosengoal_index]->origin - pEdict->v.origin).Length() < 800.0))
      {
         if (IsGroupOfEnemies(pBot, paths[pBot->chosengoal_index]->origin))
         {
            iFriendlyNum = NumTeammatesNearPos (pBot, pEdict->v.origin, 500);
            iEnemyNum = NumEnemiesNearPos (pBot, paths[pBot->chosengoal_index]->origin, 800);
            if ((float)iEnemyNum * pBot->fFearLevel * 2.0 > (float)(iFriendlyNum + 1))
            {
               if (g_b_DebugTasks)
                  ALERT(at_logged, "[DEBUG] Bot %s is seeing a group of enemies near the closest bombsite.\n", pBot->name);

               iMinIndex = -1;
               ind = 0;
               fMinDistance = 1200.0;
               for (i = 0; i < g_iNumGoalPoints; i++)
               {
                  iGoalIndex = g_rgiGoalWaypoints[i];
                  fDistance = (paths[iGoalIndex]->origin - paths[pBot->chosengoal_index]->origin).Length();
                  if (fDistance > fMinDistance)
                  {
                     iGoalChoices[ind] = iGoalIndex;
                     iMinIndex = i;
                     ind++;
                     if (ind > 3)
                        break;
                  }
               }
               if (iMinIndex > -1)
               {
                  TempTask = taskFilters[TASK_MOVETOPOSITION];
                  TempTask.fDesire = TASKPRI_HIDE;
                  DeleteSearchNodes (pBot);
                  BotGetSafeTask(pBot)->iData = pBot->chosengoal_index;
                  pBot->chosengoal_index = iGoalChoices[RANDOM_LONG(0, ind - 1)];
                  BotPushTask(pBot, &TempTask);
                  BotGetSafeTask(pBot)->iData = pBot->chosengoal_index;

                  if (g_b_DebugTasks)
                     ALERT(at_logged, "[DEBUG] Bot %s chose another bombsite.\n", pBot->name);
               }
            }
         }
      }
   }

   if ((g_bBombPlanted) && (g_vecBomb == g_vecZero))
       g_vecBomb = GetBombPosition ();

   if ((g_bBombPlanted) && (((pBot->bot_team == TEAM_CS_TERRORIST)
	        && (gpGlobals->time > g_fTimeBombPlanted + g_f_cv_c4timer - 15.0))
              || (((pEdict->v.origin - g_vecBomb).Length() > 80) && (pBot->bot_team == TEAM_CS_COUNTER) && (!g_bBombDefusing)
                 && (((!pBot->b_has_defuse_kit) && (gpGlobals->time > g_fTimeBombPlanted + g_f_cv_c4timer - 15.0))
              || (gpGlobals->time > g_fTimeBombPlanted + g_f_cv_c4timer - 7.0)))))  // KWo - 16.09.2008
   {

      if ((BotGetSafeTask(pBot)->iTask != TASK_MOVETOPOSITION) && (BotGetSafeTask(pBot)->fDesire < TASKPRI_HIDE)
          && (g_iDebugGoalIndex == -1) && ((pEdict->v.origin - g_vecBomb).Length() < BOMBMAXHEARDISTANCE))
      {
		// KWo - it's time to run away... :)
         iApproach = (int) (pBot->pEdict->v.health * pBot->fAgressionLevel);
         if ((iApproach < 50) || (pBot->pBotEnemy == NULL) || (pBot->bot_team == TEAM_CS_COUNTER))
         {
            TempTask = taskFilters[TASK_MOVETOPOSITION];
            TempTask.fDesire = TASKPRI_HIDE;

            iSearch = 0;
            iAwayIndex = RANDOM_LONG (0, g_iNumWaypoints - 1);

            while((iSearch < 20) && ((paths[iAwayIndex]->origin - g_vecBomb).Length () < BOMBMAXHEARDISTANCE))
            {
               iSearch++;
               iAwayIndex = RANDOM_LONG (0, g_iNumWaypoints - 1);
            }
            TempTask.iData = iAwayIndex;
            BotPushTask(pBot, &TempTask);
            pBot->iCampButtons = 0; // KWo - 17.02.2008

            if (g_b_DebugTasks)
				   ALERT(at_logged,"[DEBUG] Bot %s will run away from bomb explosion area...\n", pBot->name);
         }
      }
		else if ((BotGetSafeTask(pBot)->iTask == TASK_MOVETOPOSITION) && (BotGetSafeTask(pBot)->fDesire == TASKPRI_HIDE))
      {
//         taskFilters[TASK_MOVETOPOSITION].fDesire = TASKPRI_HIDE;
         pBot->iCampButtons = 0; // KWo - 17.02.2008
      }
   }

   if ((pBot->current_weapon.iId == CS_WEAPON_KNIFE) && (!FNullEnt(pBot->pBotEnemy)) && (pBot->bIsChickenOrZombie)
      && ((BotGetSafeTask(pBot)->iTask != TASK_MOVETOPOSITION) || (BotGetSafeTask(pBot)->fDesire != TASKPRI_HIDE))) // KWo - 14.06.2008
   {
      if (/* (fabs(pBot->pBotEnemy->v.origin.z - pEdict->v.origin.z) > 45.0)
         && ((pBot->pBotEnemy->v.origin - pBot->pEdict->v.origin).Length2D() < 256.0) */
         ((pBot->pBotEnemy->v.origin - pBot->pEdict->v.origin).Length2D() > 100.0)
         && (pBot->iStates & STATE_SEEINGENEMY))
      {
         iTempWp = WaypointFindNearestToMove (pBot->pBotEnemy, pBot->pBotEnemy->v.origin);
         if ((iTempWp >= 0) && (iTempWp < g_iNumWaypoints) && (iTempWp != pBot->curr_wpt_index))
         {
            if (fabs(paths[iTempWp]->origin.z - pBot->pBotEnemy->v.origin.z) < 15.0)
            {
               TempTask = taskFilters[TASK_MOVETOPOSITION];
               TempTask.fDesire = TASKPRI_HIDE;
               TempTask.iData = iTempWp;
               BotPushTask(pBot, &TempTask);
            }
         }
      }
   }

   // Bot found some Item to use ?
   if ((pBot->pBotPickupItem) && (pBot->iPickupType != PICKUP_NONE)) // 16.09.2006
   {
      pItem = pBot->pBotPickupItem;

      if (strncmp ("func_", STRING (pItem->v.classname), 5) == 0)
         vecPickme = VecBModelOrigin (pItem);
      else
         vecPickme = pItem->v.origin;

      fDistance = (vecPickme - pEdict->v.origin).Length ();
      fDistance = 500.0 - fDistance;
      if (((pBot->iPickupType == PICKUP_PLANTED_C4) && (pBot->bot_team == TEAM_CS_COUNTER))
            || ((pBot->iPickupType == PICKUP_DROPPED_C4) && (pBot->bot_team == TEAM_CS_TERRORIST))) // KWo - 05.09.2008
         fDistance = fDistance * 0.4;
      else
         fDistance = fDistance * 0.2;

      if (fDistance > 35.0) // KWo - 25.06.2008
      {
         if (pBot->iPickupType == PICKUP_WEAPON)
         {
            iWeap = GetBestWeaponCarried (pBot);
            ptrWeaponTab = ptrWeaponPrefs[pBot->bot_personality] + iWeap;
            pWeaponTab = &cs_weapon_select[0];

            iId = pWeaponTab[*ptrWeaponTab].iId;
            if (iId == CS_WEAPON_KNIFE)
               fDistance = TASKPRI_SEEKCOVER;
            else if (((pBot->f_shoot_time + 3.0 > gpGlobals->time) || (pBot->f_bot_see_enemy_time + 3.0 > gpGlobals->time))
                      && (iAliveEnemies > 0) && (!g_bIgnoreEnemies)) // KWo - 13.08.2008
               fDistance = 0.0;
            else if (fDistance > 60.0)
               fDistance = 60.0;
         }
         else if ((pBot->iPickupType == PICKUP_PLANTED_C4) && (pBot->bot_team == TEAM_CS_COUNTER)) // KWo - 13.08.2008
         {
            iFriendlyNum = NumTeammatesNearPos (pBot, pEdict->v.origin, 500);    // KWo - 06.07.2008
            iEnemyNum = NumEnemiesNearPos (pBot, g_vecBomb, 800);                // KWo - 06.07.2008
            if (((float)iEnemyNum > 0.8 * (float)iFriendlyNum) && (!g_bIgnoreEnemies) && (g_iAliveTs != 0)) // KWo - 06.07.2008
            {
               fDistance = 0.0;

               if (g_b_DebugTasks)
                  ALERT(at_logged, "[DEBUG] Too many enemies near the bomb - bot %s is not going to defuse it.\n", pBot->name);
            }
            else if (fDistance > 60.0)
               fDistance = 60.0;
         }
         else if (fDistance > 60.0)
            fDistance = 60.0;
      }
      taskFilters[TASK_PICKUPITEM].fDesire = fDistance;
   }
   else
   {
      pBot->iStates &= ~STATE_PICKUPITEM;
      taskFilters[TASK_PICKUPITEM].fDesire = 0.0;
   }

   // Calculate Desire to Attack
   if ((pBot->iStates & STATE_SEEINGENEMY) && (taskFilters[TASK_PICKUPITEM].fDesire < TASKPRI_ATTACK)
      && (taskFilters[TASK_MOVETOPOSITION].fDesire < TASKPRI_HIDE)) // KWo - 13.07.2007
   {
      if (BotReactOnEnemy(pBot))
         taskFilters[TASK_ATTACK].fDesire = TASKPRI_ATTACK;
      else
         taskFilters[TASK_ATTACK].fDesire = 0;
   }
   else
      taskFilters[TASK_ATTACK].fDesire = 0;

   // Calculate Desires to seek Cover or Hunt
   if ((!FNullEnt (pBot->pLastEnemy)) && (pBot->vecLastEnemyOrigin != g_vecZero))
   {
      fDistance = (pBot->vecLastEnemyOrigin - pEdict->v.origin).Length ();
      if (pBot->pEdict->v.health > 70.0)
         fRetreatLevel = 0.0;
      else
         fRetreatLevel = (70.0 - pBot->pEdict->v.health) * fTempFear;
      if ((pBot->bot_team == TEAM_CS_COUNTER) && (g_bBombPlanted)) // KWo - 07.10.2010
         fRetreatLevel = 0.0;

      fTimeSeen = pBot->f_bot_see_enemy_time - gpGlobals->time;

      if ((pBot->vecLastEnemyOrigin - pEdict->v.origin).Length() < 600.0)
         fTimeHeard = pBot->f_heard_sound_time - gpGlobals->time;
      else
         fTimeHeard = -20.0;

      if (fTimeSeen > fTimeHeard)
      {
         fTimeSeen += 10.0;
         fRatio = fTimeSeen * 0.1;
      }
      else
      {
         fTimeHeard += 10.0;
         fRatio = fTimeHeard * 0.1;
      }

      iEnWepID = CS_WEAPON_KNIFE;
      iEnemyIndex = ENTINDEX(pBot->pLastEnemy) - 1;
      if (IsAlive(pBot->pLastEnemy))
         iEnWepID = clients[iEnemyIndex].iCurrentWeaponId;

      if ((((pBot->current_weapon.iId == CS_WEAPON_KNIFE) && (iEnWepID != CS_WEAPON_KNIFE)
            && (pBot->vecLastEnemyOrigin.z - pBot->pEdict->v.origin.z > 45.0) && (!pBot->bOnLadder)
            && ((pBot->vecLastEnemyOrigin - pBot->pEdict->v.origin).Length2D() < 200.0)
            && (!pBot->bIsChickenOrZombie) && (!g_b_cv_jasonmode))
          || (pBot->bIsReloading)
          || WeaponIsSniper(iEnWepID) && (GetShootingConeDeviation (pBot->pLastEnemy, &pEdict->v.origin) > 0.95))
          && (pBot->f_bot_see_enemy_time + 3.0 > gpGlobals->time)) // KWo - 18.03.2010
      {
         taskFilters[TASK_SEEKCOVER].fDesire = TASKPRI_SEEKCOVER;

         if (g_b_DebugTasks)
         {
            if (pBot->bIsReloading)
               ALERT(at_logged, "[DEBUG] TASK_SEEKCOVER - reloading bot %s pushing the task on the stack.\n", pBot->name);
         }
      }
      else
         taskFilters[TASK_SEEKCOVER].fDesire = fRetreatLevel * fRatio;

      fLevel = 0.0; // KWo - for debug

      // If half of the Round is over, allow hunting
      // FIXME: It probably should be also team/map dependant
      if (((pBot->bot_personality != 2) && (!FNullEnt(pBot->pBotEnemy)
            && (pBot->f_bot_see_enemy_time + 3.0 > gpGlobals->time)
            && (pBot->f_bot_see_enemy_time + 0.3 < gpGlobals->time)
            && (!pBot->bIsVIP))
            || (g_b_cv_jasonmode) || (pBot->bIsChickenOrZombie))
         && (!g_bBombPlanted) && !BotHasHostage(pBot) && !pBot->bUsingGrenade && (fDistance > 112)
         && FNullEnt(pBot->pLift)
         && ((g_fTimeRoundMid < gpGlobals->time)
             || (((pBot->bot_team == TEAM_CS_TERRORIST) && (g_iMapType & MAP_DE))
                || ((pBot->bot_team == TEAM_CS_COUNTER)
                            && ((g_iMapType & MAP_AS) || (g_iMapType & MAP_CS)))
                && (pBot->f_spawn_time + 10.0 < gpGlobals->time))
             || ((iAliveTeamnates >= 2 * iAliveEnemies) || (g_b_cv_jasonmode) || (pBot->bIsChickenOrZombie)))
              && (iAliveEnemies > 0) && (!(pEdict->v.weapons & (1 << CS_WEAPON_C4))))  // KWo - 04.10.2010
      {
         fLevel = 4096.0 - ((1.0 - fTempAgression) * fDistance);
         fLevel = (100 * fLevel) / 4096.0;
         fLevel = fLevel - fRetreatLevel;
         if (fLevel > 89)
            fLevel = 89;
         taskFilters[TASK_ENEMYHUNT].fDesire = fLevel;

         if (g_b_DebugTasks)
            ALERT(at_logged, "[DEBUG] FilteringTasks - TASK_ENEMYHUNT - bot %s, level_priority = %d.\n", pBot->name, (int)fLevel);
      }
      else
      {
         taskFilters[TASK_ENEMYHUNT].fDesire = 0;

         if (g_b_DebugTasks)
         {
            if ((pBot->bot_team == 1) && (g_iFrameCounter == g_i_botthink_index))
               ALERT(at_logged,"[DEBUG] Bot %s has SEEKCOVER's fDesisre = %f; ENEMYHUNT's fDesire = %f.\n",
                  pBot->name, fRetreatLevel * fRatio, fLevel);
         }
      }
   }
   else
   {
      taskFilters[TASK_SEEKCOVER].fDesire = 0;
      taskFilters[TASK_ENEMYHUNT].fDesire = 0;
   }

   // Blinded Behaviour
   if (pBot->f_blind_time > gpGlobals->time)
   {
      taskFilters[TASK_BLINDED].fDesire = TASKPRI_BLINDED;
      if (BotGetSafeTask(pBot)->iTask != TASK_BLINDED) // KWo - 13.08.2008
      {
         pBot->bShootLastPosition = FALSE;
         pBot->bMadShoot = FALSE;
      }
   }
   else
   {
      taskFilters[TASK_BLINDED].fDesire = 0.0;
      pBot->bShootLastPosition = FALSE;
      pBot->bMadShoot = FALSE;
   }



   // Now we've initialised all the desires go through the hard work
   // of filtering all Actions against each other to pick the most
   // rewarding one to the Bot
   // Credits for the basic Idea of filtering comes out of the paper
   // "Game Agent Control Using Parallel Behaviors"
   // by Robert Zubek
   //
   // FIXME: Instead of going through all of the Actions it might be
   // better to use some kind of decision tree to sort out impossible
   // actions
   //
   // Most of the values were found out by Trial-and-Error and a Helper
   // Utility I wrote so there could still be some weird behaviours, it's
   // hard to check them all out


   pBot->oldcombatdesire = hysteresisdesire (taskFilters[TASK_ATTACK].fDesire, 40.0, 90.0, pBot->oldcombatdesire);
   taskFilters[TASK_ATTACK].fDesire = pBot->oldcombatdesire;
   ptaskOffensive = &taskFilters[TASK_ATTACK];
   ptaskPickup = &taskFilters[TASK_PICKUPITEM];

   // Calc Survive (Cover/Hide)
   ptaskSurvive = thresholddesire (&taskFilters[TASK_SEEKCOVER], 40.0, 0.0);
   ptaskSurvive = subsumedesire (&taskFilters[TASK_HIDE], ptaskSurvive);

   // Don't allow hunting if Desire's 60<
   pDefault = thresholddesire (&taskFilters[TASK_ENEMYHUNT], 41.0, 0.0); // KWo - 09.04.2010

   // If offensive Task, don't allow picking up stuff
   ptaskOffensive = subsumedesire (ptaskOffensive, ptaskPickup);

   // Default normal & defensive Tasks against Offensive Actions
   pSub1 = maxdesire (ptaskOffensive, pDefault);

   // Reason about fleeing instead
   pFinal = maxdesire (ptaskSurvive, pSub1);

   pFinal = subsumedesire (&taskFilters[TASK_BLINDED], pFinal);

   if (pBot->pTasks != NULL)
   {
      pFinal = maxdesire (pFinal, pBot->pTasks);
   // Push the final Behaviour in our Tasklist to carry out
      if (pFinal != NULL)
      {
         if ((BotGetSafeTask(pBot)->iTask != TASK_ENEMYHUNT) && (pFinal->iTask == TASK_ENEMYHUNT)
            && (!FNullEnt(pBot->pLastEnemy)) && (pBot->vecLastEnemyOrigin != g_vecZero)) // KWo - 11.06.2008
         {
            if (IsAlive(pBot->pLastEnemy))
            {
               for (i = 0; i < 10; i++)
               {
                  rgi_WaypointTab[i] = 0;
               }
               iSearch = 10;
               vDest = pBot->vecLastEnemyOrigin;
               fRadius = 512;
               WaypointFindInRadius (vDest, fRadius, &rgi_WaypointTab[0], &iSearch);
               if (iSearch > 0)
                  iTempWp = rgi_WaypointTab[(int) RANDOM_LONG(0, iSearch - 1)];
               else
                  iTempWp = WaypointFindNearestToMove (pBot->pLastEnemy, vDest); // KWo - 17.04.2008

               pFinal->iData = iTempWp;
               pBot->pHuntEnemy = pBot->pLastEnemy;                  // KWo - 11.04.2010
               pBot->vecHuntEnemyOrigin = pBot->vecLastEnemyOrigin;  // KWo - 11.04.2010
            }
         }
         BotPushTask (pBot, pFinal);
      }
   }
}


void BotResetTasks (bot_t *pBot)
{
   bottask_t *pPrevTask;
   bottask_t *pNextTask;

   if (g_b_DebugTasks)
      ALERT(at_logged,"[DEBUG] BotResetTasks called for bot %s.\n", pBot->name);

   if (pBot->pTasks == NULL)
      return;

   pNextTask = pBot->pTasks->pNextTask;
   pPrevTask = pBot->pTasks;

   while (pBot->pTasks != NULL)
   {
      pPrevTask = pBot->pTasks->pPreviousTask;
      delete (pBot->pTasks);
      pBot->pTasks = pPrevTask;

      pBot->i_TaskDeep--; // KWo - 30.08.2006 - stack tests
      if (pBot->i_TaskDeep < -999999) // KWo - 30.08.2006 - stack tests
         pBot->i_TaskDeep = -999999;

   }
   pBot->pTasks = pNextTask;

   while (pBot->pTasks != NULL)
   {
      pNextTask = pBot->pTasks->pNextTask;
      delete (pBot->pTasks);
      pBot->pTasks = pNextTask;

      pBot->i_TaskDeep--; // KWo - 30.08.2006 - stack tests
      if (pBot->i_TaskDeep < -999999) // KWo - 30.08.2006 - stack tests
         pBot->i_TaskDeep = -999999;

   }
   pBot->pTasks = NULL;
   return;
}


void BotCheckTaskPriorities (bot_t *pBot) // KWo - 27.08.2006
{
   bottask_t *pPrevTask;
   bottask_t *pNextTask;
   bottask_t *pMaxDesiredTask;
   bottask_t *pFirstTask;
   bottask_t *pOldTask;
   int iTask;
   float fMaxDesire;

   if (pBot->pTasks == NULL)
   {
      BotGetSafeTask(pBot);
      return;
   }

   pOldTask = pBot->pTasks;

   while (pBot->pTasks->pPreviousTask != NULL)
   {
      pPrevTask = pBot->pTasks->pPreviousTask;
      pBot->pTasks = pPrevTask;
   }

   pFirstTask = pBot->pTasks;

   pMaxDesiredTask = pBot->pTasks;
   fMaxDesire = pBot->pTasks->fDesire;

   while (pBot->pTasks->pNextTask != NULL)
   {
      pNextTask = pBot->pTasks->pNextTask;
      pBot->pTasks = pNextTask;
      if (pBot->pTasks->fDesire >= fMaxDesire)
      {
         pMaxDesiredTask = pBot->pTasks;
         fMaxDesire = pBot->pTasks->fDesire;
      }
   }

   pBot->pTasks = pMaxDesiredTask; // now we found the most desired pushed task...

   if (g_b_DebugTasks)
      ALERT(at_logged, "[DEBUG] BotCheckTaskPri - Bot's %s max desired task is %i; task deep = %i.\n",
         pBot->name, pBot->pTasks->iTask, pBot->i_TaskDeep);

   if (pOldTask != pMaxDesiredTask) // something was changed with priorities - check if some task doesn't need to be deleted...
   {
      pBot->pTasks = pFirstTask;
      while (pBot->pTasks != NULL)
      {
         pNextTask = pBot->pTasks->pNextTask;
         if ((pBot->pTasks != pMaxDesiredTask) && (!pBot->pTasks->bCanContinue)) // some task has to be deleted if cannot be continued...
         {
            if (pBot->pTasks->pPreviousTask != NULL)
               pBot->pTasks->pPreviousTask->pNextTask = pBot->pTasks->pNextTask;
            if (pBot->pTasks->pNextTask != NULL)
               pBot->pTasks->pNextTask->pPreviousTask = pBot->pTasks->pPreviousTask;

            iTask = pBot->pTasks->iTask;

            delete (pBot->pTasks);

            pBot->i_TaskDeep--; // KWo - 30.08.2006 - stack tests
            if (pBot->i_TaskDeep < -999999) // KWo - 30.08.2006 - stack tests
               pBot->i_TaskDeep = -999999;

            if (g_b_DebugTasks)
               ALERT(at_logged, "[DEBUG] BotCheckTaskPri - Bot %s deleted the previous task %s (couldn't be continued); task deep = %i.\n",
                  pBot->name, g_TaskNames[iTask], pBot->i_TaskDeep);
         }
         pBot->pTasks = pNextTask;
      }
   }
   pBot->pTasks = pMaxDesiredTask;

   if ((g_iDebugGoalIndex != -1) && (pMaxDesiredTask->iTask == TASK_NORMAL)) // KWo - 07.01.2008
      pBot->chosengoal_index = g_iDebugGoalIndex;
   else if (pBot->pTasks->iData != -1)
      pBot->chosengoal_index = pBot->pTasks->iData;

   return;
}


void BotPushTask (bot_t *pBot, bottask_t *pTask)
{
   bool bNewTaskDifferent = false;
   bool bFoundTaskExisting = false;
   bool bCheckPriorities = false;
   bottask_t *pOldTask = BotGetSafeTask(pBot); // KWo - 27.08.2006 - remember our current task
   bottask_t *pFirstTask = BotGetSafeTask(pBot);  // KWo - 27.08.2006 - first task on the stack

// KWo - at the beginning need to clean up all NULL tasks...

   if (pBot->pTasks == NULL) // KWo - 27.08.2006
   {
      if (pBot->fNoCollTime + 1.0 < gpGlobals->time) // KWo - 02.01.2010
         pBot->fNoCollTime = gpGlobals->time + 1.0;

      bottask_t *pNewTask = new bottask_t;
      pNewTask->iTask = TASK_NORMAL;
      pNewTask->fDesire = TASKPRI_NORMAL;
      pNewTask->iData = -1;
      pNewTask->fTime = 0.0;
      pNewTask->bCanContinue = TRUE;
      pNewTask->pPreviousTask = NULL;
      pNewTask->pNextTask = NULL;
      pBot->pTasks = pNewTask;
      DeleteSearchNodes(pBot);

      pBot->i_TaskDeep++; // KWo - 30.08.2006 - stack tests
      if (pBot->i_TaskDeep > 999999) // KWo - 30.08.2006 - stack tests
         pBot->i_TaskDeep = 999999;

      if (g_b_DebugTasks)
         ALERT(at_logged, "[DEBUG] BotPushTask - Bot %s had got the NULL task on the stack; task deep = %i.\n",
            pBot->name, pBot->i_TaskDeep);

      if (pTask == NULL)
      {
         return;
      }
      else
      {
         if (pTask->iTask == TASK_NORMAL)
         {
            pBot->pTasks->fDesire = TASKPRI_NORMAL;
            pBot->pTasks->iData = pTask->iData;
            pBot->pTasks->fTime = pTask->fTime;
            return;
         }
      }
   }
   else if (pTask == NULL)
   {
      return;
   }

   if (pBot->pTasks != NULL) // it shouldn't happen this condition now as false...
   {
      if (pBot->pTasks->iTask == pTask->iTask)
      {
         if (pBot->pTasks->iData != pTask->iData)
         {
            if (pBot->fNoCollTime + 1.0 < gpGlobals->time) // KWo - 02.01.2010
               pBot->fNoCollTime = gpGlobals->time + 1.0;

            DeleteSearchNodes(pBot);
            pBot->pTasks->iData = pTask->iData;

            if (g_b_DebugTasks)
               ALERT(at_logged, "[DEBUG] BotPushTask - Bot %s got the new-old task %s with different goal; task deep = %i.\n",
                  pBot->name, g_TaskNames[pTask->iTask], pBot->i_TaskDeep);
         }
         if (pBot->pTasks->fDesire != pTask->fDesire)
         {
            pBot->pTasks->fDesire = pTask->fDesire;
            bCheckPriorities = TRUE;

            if (g_b_DebugTasks)
               ALERT(at_logged, "[DEBUG] BotPushTask - Bot %s got the new-old task %s with different desire; task deep = %i.\n",
                  pBot->name, g_TaskNames[pTask->iTask], pBot->i_TaskDeep);
         }
         else if (pBot->pTasks->iData == pTask->iData)
            return;
      }
      else
      {
// find the first task on the stack and don't allow push the new one like the same already existing one
         while (pBot->pTasks->pPreviousTask)  // KWo - 27.08.2006
         {
            pBot->pTasks = pBot->pTasks->pPreviousTask;
            pFirstTask = pBot->pTasks;

            if (pBot->pTasks->iTask == pTask->iTask)
            {
               bFoundTaskExisting = TRUE;
               if (pBot->pTasks->fDesire != pTask->fDesire)
               {
                  bCheckPriorities = TRUE;
               }
               pBot->pTasks->fDesire = pTask->fDesire;
               pBot->pTasks->iData = pTask->iData;
               pBot->pTasks->fTime = pTask->fTime;
               pBot->pTasks->bCanContinue = pTask->bCanContinue;
               pBot->pTasks = pOldTask;

               break; // now we may need to check the current max desire or next tasks...
            }
         }
// now go back to the previous stack position and try to find the same task as one of "the next" ones
// (already pushed before and not finished yet)

         if ((!bFoundTaskExisting) && (!bCheckPriorities))
         {
            pBot->pTasks = pOldTask; // KWo - 27.08.2006
            while (pBot->pTasks->pNextTask) // KWo - 27.08.2006
            {
               pBot->pTasks = pBot->pTasks->pNextTask;
               if (pBot->pTasks->iTask == pTask->iTask)
               {
                  bFoundTaskExisting = TRUE;
                  if (pBot->pTasks->fDesire != pTask->fDesire)
                  {
                     bCheckPriorities = TRUE;
                  }
                  pBot->pTasks->fDesire = pTask->fDesire;
                  pBot->pTasks->iData = pTask->iData;
                  pBot->pTasks->fTime = pTask->fTime;
                  pBot->pTasks->bCanContinue = pTask->bCanContinue;
                  pBot->pTasks = pOldTask;

                  break; // now we may need to check the current max desire...
               }
            }
         }
         if (!bFoundTaskExisting)
            bNewTaskDifferent = true; // we have some new task pushed on the stack...
         else if (g_b_DebugTasks)
            ALERT(at_logged, "[DEBUG] BotPushTask - Bot %s got the new task %s already existing on the stack; task deep = %i.\n",
               pBot->name, g_TaskNames[pTask->iTask], pBot->i_TaskDeep);
      }
   }

   pBot->pTasks = pOldTask;

   if (bNewTaskDifferent)
   {
      pBot->i_TaskDeep++; // KWo - 30.08.2006 - stack tests
      if (pBot->i_TaskDeep > 999999) // KWo - 30.08.2006 - stack tests
         pBot->i_TaskDeep = 999999;

      if (g_b_DebugTasks)
         ALERT(at_logged, "[DEBUG] BotPushTask - Bot %s got the new different task %s; task deep = %i.\n",
            pBot->name, g_TaskNames[pTask->iTask], pBot->i_TaskDeep);

      bottask_t *pNewTask = new bottask_t;
      pNewTask->iTask = pTask->iTask;
      pNewTask->fDesire = pTask->fDesire;
      pNewTask->iData = pTask->iData;
      pNewTask->fTime = pTask->fTime;
      pNewTask->bCanContinue = pTask->bCanContinue;
      pNewTask->pNextTask = NULL;

      while (pBot->pTasks->pNextTask)
         pBot->pTasks = pBot->pTasks->pNextTask;

      pNewTask->pPreviousTask = pBot->pTasks;
      pBot->pTasks->pNextTask = pNewTask;
      bCheckPriorities = TRUE;
   }

   pBot->pTasks = pOldTask;

   if (bCheckPriorities)
   {
      // needs check the priorities and setup the task with the max desire...
      BotCheckTaskPriorities (pBot);

      if (pBot->pTasks != pOldTask) // the max desired task has been changed...
      {
         if (g_b_DebugTasks)
            ALERT(at_logged, "[DEBUG] BotPushTask - Bot %s got other task %s with higher priority (after checking prior.); task deep = %i.\n",
               pBot->name, g_TaskNames[pTask->iTask], pBot->i_TaskDeep);

         DeleteSearchNodes (pBot);

         if (pBot->fNoCollTime + 1.0 < gpGlobals->time) // KWo - 02.01.2010
            pBot->fNoCollTime = gpGlobals->time + 1.0;

         // Leader Bot ?
         if ((pBot->bIsLeader) && (bNewTaskDifferent))
         {
            // Reorganize Team if fleeing
            if (pBot->pTasks->iTask == TASK_SEEKCOVER)
               BotCommandTeam (pBot);
         }
      }
   }
   return;
}


bottask_t *BotGetSafeTask (bot_t *pBot)
{
   if (pBot->pTasks == NULL)
   {
// KWo - 27.08.2006 - new kind of creating safe tasks...
      pBot->i_TaskDeep++; // KWo - 30.08.2006 - stack tests
      if (pBot->i_TaskDeep > 999999) // KWo - 30.08.2006 - stack tests
         pBot->i_TaskDeep = 999999;

      if (g_b_DebugTasks)
         ALERT(at_logged, "[DEBUG] BotGetSafeTask - Bot %s got the NULL task on the stack - creates the new stack !!! task deep = %i.\n",
            pBot->name, pBot->i_TaskDeep);

      bottask_t *pNewTask = new bottask_t;
      pNewTask->iTask = TASK_NORMAL;
      pNewTask->fDesire = TASKPRI_NORMAL;
      pNewTask->iData = -1;
      pNewTask->fTime = 0.0;
      pNewTask->bCanContinue = TRUE;
      pNewTask->pPreviousTask = NULL;
      pNewTask->pNextTask = NULL;
      pBot->pTasks = pNewTask;
      DeleteSearchNodes (pBot);
      if (pBot->fNoCollTime + 1.0 < gpGlobals->time) // KWo - 02.01.2010
         pBot->fNoCollTime = gpGlobals->time + 1.0;
   }
   return (pBot->pTasks);
}


void BotRemoveCertainTask (bot_t *pBot, int iTaskNum)
{
   bottask_t *pTask;
   bottask_t *pNextTask;
   bottask_t *pPrevTask;
   bottask_t *pOldTask;
   bottask_t *pOldNextTask;
   bottask_t *pOldPrevTask;
   bool bCheckPriorities = false;

   pTask = pBot->pTasks;

   if (pTask == NULL)
      return;

   if (pTask->iTask == TASK_NORMAL) //  KWo - 02.09.2006 - since TASK_NORMAL can be only once on the stack, don't remove it...
      return;

   pOldTask = pTask; // KWo - 30.08.2006
   pOldPrevTask = pTask->pPreviousTask; // KWo - 30.08.2006
   pOldNextTask = pTask->pNextTask; // KWo - 30.08.2006

   while (pTask->pPreviousTask != NULL)
      pTask = pTask->pPreviousTask;

   while (pTask != NULL)
   {
      pNextTask = pTask->pNextTask;
      pPrevTask = pTask->pPreviousTask;

      if (pTask->iTask == iTaskNum)
      {
         if (pPrevTask != NULL)
            pPrevTask->pNextTask = pNextTask;
         if (pNextTask != NULL)
            pNextTask->pPreviousTask = pPrevTask;

         if (pTask == pOldTask)
            pOldTask = NULL;
         else if (pTask == pOldPrevTask)
            pOldPrevTask = NULL;
         else if (pTask == pOldNextTask)
            pOldNextTask = NULL;

         delete (pTask);

         pBot->i_TaskDeep--; // KWo - 30.08.2006 - stack tests
         if (pBot->i_TaskDeep < -999999) // KWo - 30.08.2006 - stack tests
            pBot->i_TaskDeep = -999999;

         bCheckPriorities = true;

         if (g_b_DebugTasks)
            ALERT(at_logged, "[DEBUG] BotRemoveCertainTask - Bot %s removes the task %s; task deep %i.\n",
               pBot->name, g_TaskNames[iTaskNum], pBot->i_TaskDeep);

         break;
      }

      pTask = pNextTask;
   }

   if (pOldTask != NULL)
      pBot->pTasks = pOldTask;
   else if (pOldPrevTask != NULL)
      pBot->pTasks = pOldPrevTask;
   else if (pOldNextTask != NULL)
      pBot->pTasks = pOldNextTask;
   else
   {
      BotGetSafeTask(pBot);

      if (g_b_DebugTasks)
         ALERT(at_logged, "[DEBUG] BotRemoveCertainTask - Bot %s calls BotGetSafeTask!; task deep %i.\n",
            pBot->name, pBot->i_TaskDeep);
   }
   if (bCheckPriorities)
      BotCheckTaskPriorities (pBot); // KWo - 02.09.2006

// KWo - 26.08.2006 - removed delete search nodes - done near called function if necessary...
   return;
}


void BotTaskComplete (bot_t *pBot)
{
   // Called whenever a Task is completed

   bottask_t *pPrevTask;
   bottask_t *pNextTask;

   if (g_b_DebugTasks)
      ALERT(at_logged, "[DEBUG] BotTaskComplete called for bot %s.\n", pBot->name);

   if (pBot->pTasks == NULL)
   {
      if (g_b_DebugTasks)
         ALERT(at_logged, "[DEBUG] BotTaskComplete - Bot %s got task NULL to finish; task deep = %i.\n",
            pBot->name, pBot->i_TaskDeep);
      DeleteSearchNodes (pBot); // Delete all Pathfinding Nodes
      return;
   }

   if (pBot->pTasks->iTask == TASK_NORMAL)
   {
      if (g_b_DebugTasks)
         ALERT(at_logged, "[DEBUG] BotTaskComplete - Bot %s finishes the TASK_NORMAL; task deep = %i.\n",
            pBot->name, pBot->i_TaskDeep);
      DeleteSearchNodes (pBot); // Delete all Pathfinding Nodes
      pBot->pTasks->iData = -1;
      pBot->chosengoal_index = -1;
      return;
   }

   pNextTask = pBot->pTasks->pNextTask;
   pPrevTask = pBot->pTasks->pPreviousTask;

   if (pNextTask != NULL)
      pNextTask->pPreviousTask = pPrevTask;
   if (pPrevTask != NULL)
      pPrevTask->pNextTask = pNextTask;

   delete (pBot->pTasks);  // delete the current one
   pBot->pTasks = NULL;

   pBot->i_TaskDeep--; // KWo - 30.08.2006 - stack tests
   if (pBot->i_TaskDeep < -999999) // KWo - 30.08.2006 - stack tests
      pBot->i_TaskDeep = -999999;

   if ((pPrevTask) && (pNextTask))
   {
      if (pPrevTask->fDesire >= pNextTask->fDesire)
      {
         pBot->pTasks = pPrevTask;
         if (g_b_DebugTasks)
            ALERT(at_logged, "[DEBUG] BotTaskComplete - Bot %s got the previous task = %s from the stack; task deep = %i.\n",
               pBot->name, g_TaskNames[pBot->pTasks->iTask], pBot->i_TaskDeep);
      }
      else
      {
         pBot->pTasks = pNextTask;

         if (g_b_DebugTasks)
            ALERT(at_logged, "[DEBUG] BotTaskComplete - Bot %s got the next task  = %s from the stack; task deep = %i.\n",
               pBot->name, g_TaskNames[pBot->pTasks->iTask], pBot->i_TaskDeep);
      }
   }
   else if (pPrevTask)
   {
      pBot->pTasks = pPrevTask;

      if (g_b_DebugTasks)
         ALERT(at_logged, "[DEBUG] BotTaskComplete - Bot %s got the previous task = %s from the stack; task deep = %i.\n",
            pBot->name, g_TaskNames[pBot->pTasks->iTask], pBot->i_TaskDeep);
   }
   else if (pNextTask)
   {
      pBot->pTasks = pNextTask;

      if (g_b_DebugTasks)
         ALERT(at_logged, "[DEBUG] BotTaskComplete - Bot %s got the next task  = %s from the stack; task deep = %i.\n",
            pBot->name, g_TaskNames[pBot->pTasks->iTask], pBot->i_TaskDeep);
   }
   if (pBot->pTasks == NULL)
      BotGetSafeTask(pBot);

   BotCheckTaskPriorities (pBot);

   // Delete all Pathfinding Nodes
   if (g_b_DebugTasks)
      ALERT(at_logged, "[DEBUG] BotTaskComplete - Bot %s does DeleteSearchNodes.\n", pBot->name);
   DeleteSearchNodes(pBot);
   return;
}


inline void BotFacePosition (bot_t *pBot, Vector vecPos) // KWo - 13.04.2010
{
   // Adjust all Bot Body and View Angles to face an absolute Vector

   static edict_t *pEdict;
   static Vector vecDirection;

   static Vector spring_stiffness;
   static Vector damper_coefficient;
   static Vector influence;
   static Vector randomization;
   static float offset_delay;
   static float notarget_slowdown_ratio;
   static float target_anticipation_ratio;
   static float stiffness_multiplier;
   static Vector v_stiffness;
   static Vector vecRandomization;
   static float fDistance;
   static float fDistStiff;

   pEdict = pBot->pEdict;
   vecDirection = UTIL_VecToAngles (vecPos - GetGunPosition (pEdict));

   if (g_bIsOldCS15)  // KWo - 14.10.2006
   {
      vecDirection = vecDirection + pEdict->v.punchangle * (float)(pBot->bot_skill)/100.0;
//      pBot->rgvecRecoil[0] = pEdict->v.punchangle;
//      pBot->rgvecRecoil[0] = g_vecZero; // KWo - test

      pBot->rgvecRecoil[0] = pBot->rgvecRecoil[1];
      pBot->rgvecRecoil[1] = pBot->rgvecRecoil[2];
      pBot->rgvecRecoil[2] = pEdict->v.punchangle;

      pBot->rgvecRecoil[2] = pBot->rgvecRecoil[3];
      pBot->rgvecRecoil[3] = pBot->rgvecRecoil[4];
      pBot->rgvecRecoil[4] = pEdict->v.punchangle;

//      vecDirection = vecDirection + pBot->rgvecRecoil[0];
   }
//   vecDirection = vecDirection  + 1.0 * (pEdict->v.punchangle) * (float)(pBot->bot_skill)/100.0; // KWo - 02.04.2010
   vecDirection.x = -vecDirection.x;
   vecDirection = vecDirection - 0.2 * (pEdict->v.punchangle);

   UTIL_ClampVector(&vecDirection);
   
   if (g_b_DebugEntities)
   {
      ALERT(at_logged, "[DEBUG] BotFacePosition - Bot %s should have view angles = [%i,%i].\n",
               pBot->name, (int)vecDirection.x, (int)vecDirection.y);
      ALERT(at_logged, "[DEBUG] BotFacePosition - Bot %s actual view angles = [%i,%i].\n",
               pBot->name, (int)pBot->pEdict->v.v_angle.x, (int)pBot->pEdict->v.v_angle.y);
   }

   pEdict->v.ideal_yaw = vecDirection.y;
   pEdict->v.idealpitch = vecDirection.x;

   if (pEdict->v.idealpitch > 89)
      pEdict->v.idealpitch = 89;
   else if (pEdict->v.idealpitch < -89)
      pEdict->v.idealpitch = -89;

   BotCheckZoom (pBot); // KWo - 13.04.2010

   switch (g_i_cv_aim_type)  // KWo - 06.04.2006
   {
      case 1:
      {
         pEdict->v.v_angle.x = pEdict->v.idealpitch;
         pEdict->v.v_angle.y = pEdict->v.ideal_yaw;
         break;
      }
      case 2:
      {
      // Update Turnspeeds every frame...
      // Makes them turn less smooth which is more humanlike
//      pEdict->v.yaw_speed = RANDOM_FLOAT (BotTurnSpeeds[pBot->bot_skill / 20].fMinTurnSpeed,
//         BotTurnSpeeds[pBot->bot_skill / 20].fMaxTurnSpeed);
//      pEdict->v.pitch_speed = RANDOM_FLOAT (BotTurnSpeeds[pBot->bot_skill / 20].fMinTurnSpeed,
//         BotTurnSpeeds[pBot->bot_skill / 20].fMaxTurnSpeed);

      // Get Speed Multiply Factor by dividing Target FPS by real FPS
         float fSpeedFactor = g_i_cv_FpsMax * pBot->fTimeFrameInterval;  // KWo - 17.10.2006 - reverted back
         if (fSpeedFactor < 1.0)
            fSpeedFactor = 1.0;

      // Slow turning if ZOOMED!
         float fZoomFactor = pEdict->v.fov * (1.f / 90.f);

         BotChangePitch (pBot, pEdict->v.pitch_speed * fSpeedFactor * fZoomFactor * (((pBot->iAimFlags & AIM_ENEMY) && !(pBot->iAimFlags & AIM_ENTITY)) ? 1 : 0.3)); // KWo - 04.03.2004
         BotChangeYaw (pBot, pEdict->v.yaw_speed * fSpeedFactor * fZoomFactor * (((pBot->iAimFlags & AIM_ENEMY) && !(pBot->iAimFlags & AIM_ENTITY)) ? 1 : 0.3)); // KWo - 04.03.2004
         break;
      }
      case 3:
      {
         Vector vecDeviation = Vector (pEdict->v.idealpitch, pEdict->v.ideal_yaw, 0) - pEdict->v.v_angle;

         UTIL_ClampVector (&vecDeviation);

         float flTurnSkill = (pBot->bot_skill * 0.05) + 0.5;
         float flSpeed = (0.5f - (1.f / 3.f)) + flTurnSkill * (2.f / 33.f);
         float flFrameCompensation = gpGlobals->frametime * 1000 * 0.01f; // KWo - 27.04.2006 - thanks to THE_STORM

         if ((pBot->iAimFlags & AIM_ENEMY) && !(pBot->iAimFlags & AIM_ENTITY)) // KWo - 04.03.2006
            flSpeed *= 1.75;
/*         if (pBot->iAimFlags & AIM_ENEMY)
            flSpeed = 0.7f + (flTurnSkill - 1.f) * (1.f / 15.f); // fast aim
         else
            flSpeed = 0.35f + (flTurnSkill - 1.f) * (1.f / 30.f); // slow aim*/

         float flMomentum = (1 - flSpeed) * 0.5;//exp (log (flSpeed * 0.5) * flFrameCompensation);

         pEdict->v.pitch_speed = ((pEdict->v.pitch_speed * flMomentum) + flSpeed * vecDeviation.x * (1.f - flMomentum)) * flFrameCompensation;
         pEdict->v.yaw_speed = ((pEdict->v.yaw_speed * flMomentum) + flSpeed * vecDeviation.y * (1.f - flMomentum)) * flFrameCompensation;

         if (pBot->bot_skill < 100)
         {
         // influence of y movement on x axis, based on skill (less influence than x on y since it's
         // easier and more natural for the bot to "move its mouse" horizontally than vertically)
            pEdict->v.pitch_speed += pEdict->v.yaw_speed / (10.f * flTurnSkill);
         // influence of x movement on y axis, based on skill
            pEdict->v.yaw_speed += pEdict->v.pitch_speed / (12.f * flTurnSkill);
         }

      // Change Pitch Angles
         pEdict->v.v_angle.x += pEdict->v.pitch_speed;

      // Change Yaw Angles
         pEdict->v.v_angle.y += pEdict->v.yaw_speed;
         break;
      }
      case 4:
      default:
      {
         spring_stiffness.x = g_f_cv_aim_spring_stiffness_x;
         spring_stiffness.y = g_f_cv_aim_spring_stiffness_y;
         spring_stiffness.z = 0.0;
         damper_coefficient.x = g_f_cv_aim_damper_coefficient_x;
         damper_coefficient.y = g_f_cv_aim_damper_coefficient_y;
         damper_coefficient.z = 0.0;
         influence.x = g_f_cv_aim_influence_x_on_y * (100 - pBot->bot_skill)/100.0;
         influence.y = g_f_cv_aim_influence_y_on_x * (100 - pBot->bot_skill)/100.0;
         influence.z = 0.0;
         randomization.x = g_f_cv_aim_deviation_x * (100 - pBot->bot_skill)/100.0;
         randomization.y = g_f_cv_aim_deviation_y * (100 - pBot->bot_skill)/100.0;
         randomization.z = 0.0;
         offset_delay = g_f_cv_aim_offset_delay;
         notarget_slowdown_ratio = 0.8 * g_f_cv_aim_notarget_slowdown_ratio;
         target_anticipation_ratio = g_f_cv_aim_target_anticipation_ratio;
         fDistStiff = 1.0; // KWo - 08.07.2008
         pBot->target_angular_speed = g_vecZero; // KWo - 10.04.2016

         pBot->ideal_angles = Vector (pEdict->v.idealpitch, pEdict->v.ideal_yaw, 0);

//         UTIL_ClampVector (&pBot->target_angular_speed); // KWo - 02.03.2006

         if (pBot->iAimFlags & (AIM_ENEMY | AIM_LASTENEMY)) // KWo - 12.03.2010
         {
            pBot->randomized_ideal_angles = pBot->ideal_angles;
            pBot->randomized_angles = g_vecZero;
            pBot->player_target_time = gpGlobals->time;

            if (pBot->iAimFlags & AIM_ENEMY)
            {
               if (!FNullEnt (pBot->pLastEnemy))
               {
                  fDistance = (pEdict->v.origin - pBot->vecLastEnemyOrigin).Length();
                  if (fDistance < 100.0)
                     fDistance = 100.0;
                  fDistStiff = 500.0/fDistance;
                  if (fDistStiff > 1.5)
                     fDistStiff = 1.5;
                  else if (fDistStiff < 1.0)
                     fDistStiff = 1.0;

//                  pBot->target_angular_speed = (UTIL_VecToAngles (pBot->vecLastEnemyOrigin - pEdict->v.origin
//                     + 1.0 * pBot->fTimeFrameInterval * pBot->pLastEnemy->v.velocity + 1.0 * (pBot->fTimeFrameInterval) * pEdict->v.velocity)
//                     - UTIL_VecToAngles (pBot->vecLastEnemyOrigin - pEdict->v.origin)) * 0.45 * ((float)pBot->bot_skill/100.0) * target_anticipation_ratio;

                  pBot->target_angular_speed = (UTIL_VecToAngles (pBot->vecLastEnemyOrigin - pEdict->v.origin
                     + 1.0 * /* pBot->fTimeFrameInterval * */ pBot->pLastEnemy->v.velocity - 1.0 /* * (pBot->fTimeFrameInterval) */ * pEdict->v.velocity)
                     - UTIL_VecToAngles (pBot->vecLastEnemyOrigin - pEdict->v.origin)) * 0.45 * ((float)pBot->bot_skill/100.0) * target_anticipation_ratio; // KWo - 12.01.2012


                  pBot->target_angular_speed.x = -pBot->target_angular_speed.x;

                  if ((pEdict->v.fov < 90) /* && (pBot->angular_deviation.Length () >= 5.0) */)
                  {
                     spring_stiffness = 2 * spring_stiffness;
                  }
//                  UTIL_ClampVector (&pBot->target_angular_speed);
               }
               else
                  pBot->target_angular_speed = g_vecZero;

               v_stiffness = fDistStiff * spring_stiffness * (0.2 + ((float) pBot->bot_skill) / 125.0);
               if (pEdict->v.punchangle.x > 2.0)
                  v_stiffness.x = v_stiffness.x * 2;
            }
            else
            {
               pBot->target_angular_speed = g_vecZero;
               v_stiffness = 0.4 * spring_stiffness * (0.2 + ((float) pBot->bot_skill) / 125.0);
            }
         }
         else
         {
            // is it time for bot to randomize the aim direction again (more often where moving) ?
            if ((pBot->randomize_angles_time < gpGlobals->time)
                && (((pBot->pEdict->v.velocity.Length () > 1.0) && (pBot->angular_deviation.Length () < 5.0))
                    || (pBot->angular_deviation.Length () < 1.0)))
            {
               // is the bot standing still ?
               if (pBot->pEdict->v.velocity.Length () < 1.0)
                  vecRandomization = randomization * 0.2; // randomize less
               else
                  vecRandomization = randomization;

               // randomize targeted location a bit (slightly towards the ground)
               pBot->randomized_angles = Vector (RANDOM_FLOAT (-vecRandomization.x * 0.5, vecRandomization.x * 1.5),
                                                                          RANDOM_FLOAT (-vecRandomization.y, vecRandomization.y), 0.0);
               // set next time to do this
               pBot->randomize_angles_time = gpGlobals->time + RANDOM_FLOAT (0.4, offset_delay);
            }

            if (((pBot->dest_origin - pBot->pEdict->v.origin).Length() <= 15) && (pBot->bMoveToGoal)
               && ((BotGetSafeTask(pBot)->iTask == TASK_NORMAL) || (BotGetSafeTask(pBot)->iTask == TASK_MOVETOPOSITION)
                  || (BotGetSafeTask(pBot)->iTask == TASK_FOLLOWUSER) || (BotGetSafeTask(pBot)->iTask == TASK_ENEMYHUNT)
                  || (BotGetSafeTask(pBot)->iTask == TASK_SEEKCOVER))
               || (BotGetSafeTask(pBot)->iTask == TASK_PAUSE)) // KWo - 10.04.2016
            {
               pBot->randomized_ideal_angles = pBot->pEdict->v.v_angle;
            }
            else
               pBot->randomized_ideal_angles = pBot->ideal_angles + pBot->randomized_angles;

            pBot->target_angular_speed = g_vecZero;

            stiffness_multiplier = notarget_slowdown_ratio;
            // take in account whether the bot was targeting someone in the last N seconds
            if (gpGlobals->time - (pBot->player_target_time + offset_delay) < notarget_slowdown_ratio * 10.0)
               stiffness_multiplier = 1 - (gpGlobals->time - pBot->fTimeLastFired) * 0.1;

            if (stiffness_multiplier < 0.1) // KWo - 17.02.2008
               stiffness_multiplier = 0.1;
// \\ KWo - TEST against bot "shaking"
            if ((pBot->f_wpt_timeset + 1.0 > gpGlobals->time) && pBot->bMoveToGoal) // KWo - 20.09.2008
               stiffness_multiplier = 1.5 * stiffness_multiplier;

            // also take in account the remaining deviation (slow down the aiming in the last 10°)

            if (pBot->angular_deviation.Length () < 10.0)
            {
               if (pEdict->v.fov < 90) // KWo - 05.03.2006
                  stiffness_multiplier *= pBot->angular_deviation.Length () * 0.2;
               else
                  stiffness_multiplier *= pBot->angular_deviation.Length () * 0.1;
            }

            // slow down even more if we are not moving
            if (pBot->pEdict->v.velocity.Length () < 1)
               stiffness_multiplier *= 0.5;


            // but don't allow getting below a certain value
            if (stiffness_multiplier < 0.2)
               stiffness_multiplier = 0.2;

            v_stiffness = spring_stiffness * stiffness_multiplier; // increasingly slow aim
         }


//         pBot->angular_deviation = pBot->randomized_ideal_angles + pBot->target_angular_speed - pBot->pEdict->v.v_angle;
         pBot->angular_deviation = pBot->randomized_ideal_angles - pBot->pEdict->v.v_angle;
         UTIL_ClampVector (&pBot->angular_deviation);
         UTIL_ClampVector (&pBot->randomized_ideal_angles);
         UTIL_ClampVector (&pBot->target_angular_speed); // KWo - 12.01.2012

         // spring/damper model aiming (thanks Aspirin for the target speed idea)

         pBot->aim_speed.x = (v_stiffness.x * pBot->angular_deviation.x)
             - (damper_coefficient.x * (pBot->aim_speed.x)) + 1.0 * pBot->target_angular_speed.x;  // KWo - 12.01.2012
         pBot->aim_speed.y = (v_stiffness.y * pBot->angular_deviation.y)
             - (damper_coefficient.y * (pBot->aim_speed.y)) + 1.0 * pBot->target_angular_speed.y;  // KWo - 12.01.2012

         // influence of y movement on x axis and vice versa (less influence than x on y since it's
         // easier and more natural for the bot to "move its mouse" horizontally than vertically)
         pBot->aim_speed.x += pBot->aim_speed.y * influence.y;
         pBot->aim_speed.y += pBot->aim_speed.x * influence.x;


         if (!(pBot->iAimFlags & (AIM_ENEMY | AIM_LASTENEMY)))
         {
            if (pBot->aim_speed.x > 50.0)
               pBot->aim_speed.x = 50.0;
            else if (pBot->aim_speed.x < -50.0)
               pBot->aim_speed.x = -50.0;

            if (pBot->aim_speed.y > 200.0)
               pBot->aim_speed.y = 200.0;
            else if (pBot->aim_speed.y < -200.0)
               pBot->aim_speed.y = -200.0;
         }

         // move the aim cursor
         pBot->pEdict->v.v_angle = pBot->pEdict->v.v_angle + pBot->fTimeFrameInterval * Vector (pBot->aim_speed.x, pBot->aim_speed.y, 0.0); // KWo - 17.10.2006
         if (pBot->pEdict->v.v_angle.x < -89.0) // KWo - 10.04.2016
            pBot->pEdict->v.v_angle.x = -89.0;
         else if (pBot->pEdict->v.v_angle.x > 89.0) // KWo - 10.04.2016
            pBot->pEdict->v.v_angle.x = 89.0;

         break;
      }
   }
   if (g_b_DebugEntities) // KWo - 10.04.2016
   {
      ALERT(at_logged, "[DEBUG] BotFacePosition - Bot %s should have view angles  = [%i,%i].\n",
               pBot->name, (int)pBot->pEdict->v.v_angle.x, (int)pBot->pEdict->v.v_angle.y);
   }

}


bool BotEnemyIsThreat (bot_t *pBot)
{
   edict_t *pEdict = pBot->pEdict;
   float fDistance;
   int iEnemyIndex; // KWo - 24.06.2008
   int iWeaponEnemy; // KWo - 24.06.2008

   if (FNullEnt (pBot->pBotEnemy) /* || (pBot->iStates & STATE_SUSPECTENEMY) */
       || (BotGetSafeTask(pBot)->iTask == TASK_SEEKCOVER))
      return (FALSE);

   if (!FNullEnt (pBot->pBotEnemy))
   {
      iEnemyIndex = ENTINDEX(pBot->pBotEnemy) - 1; // KWo - 24.06.2008
      iWeaponEnemy = CS_WEAPON_KNIFE;
      if ((iEnemyIndex > 0) && (iEnemyIndex <= gpGlobals->maxClients))
      {
         iWeaponEnemy = clients[iEnemyIndex].iCurrentWeaponId; // KWo - 24.06.2008
      }
      Vector vDest = pBot->pBotEnemy->v.origin - pBot->pEdict->v.origin;
      fDistance = vDest.Length ();
   }
   else
      return (FALSE);

   // If Bot is camping, he should be firing anyway and NOT leaving his position
   if (BotGetSafeTask(pBot)->iTask == TASK_CAMP)
      return (FALSE);

   // If Enemy is near or facing us directly
   if ((fDistance < 256)
      || (iWeaponEnemy != CS_WEAPON_KNIFE)
          && (GetShootingConeDeviation (pBot->pBotEnemy, &pEdict->v.origin) >= 0.7))
      return (TRUE);

   return (FALSE);
}


bool BotReactOnEnemy (bot_t *pBot)
{
   // Check if Task has to be interrupted because an Enemy is near (run Attack Actions then)

   if (BotEnemyIsThreat (pBot))
   {
      if (pBot->fEnemyReachableTimer < gpGlobals->time)
      {
         int iEnemyIndex = ENTINDEX(pBot->pBotEnemy) - 1; // KWo - 14.06.2008
         int iEnWepID = clients[iEnemyIndex].iCurrentWeaponId; // KWo - 14.06.2008

         int iBotWpIndex = WaypointFindNearestToMove (pBot->pEdict, pBot->pEdict->v.origin); // KWo - 17.04.2008
         int iEnemyWpIndex = WaypointFindNearestToMove (pBot->pBotEnemy, pBot->pBotEnemy->v.origin); // KWo - 17.04.2008
         int iLinDist = (int) (pBot->pBotEnemy->v.origin - pBot->pEdict->v.origin).Length ();	// KWo variable name changed
         int iPathDist = GetPathDistance (iBotWpIndex, iEnemyWpIndex);	// KWo variable name changed

         if  ((iPathDist - iLinDist > 112)
            || ((pBot->current_weapon.iId == CS_WEAPON_KNIFE) && (iEnWepID != CS_WEAPON_KNIFE)
                && (pBot->pBotEnemy->v.origin.z - pBot->pEdict->v.origin.z > 45.0)
                && ((pBot->pBotEnemy->v.origin - pBot->pEdict->v.origin).Length2D() < 100.0)
                && (!pBot->bOnLadder))) // KWo - 14.06.2008
            pBot->bEnemyReachable = FALSE;
         else
            pBot->bEnemyReachable = TRUE;

         pBot->fEnemyReachableTimer = gpGlobals->time + 1.0;
      }

      if (pBot->bEnemyReachable)
      {
         // Override existing movement by attack movement
         pBot->f_wpt_timeset = gpGlobals->time;
         return (TRUE);
      }
   }
   return (FALSE);
}

bool BotFollowUser (bot_t *pBot)
{
   // Check if Bot can still follow a User

   if (FNullEnt (pBot->pBotUser))
      return (FALSE);

   if (!IsAlive (pBot->pBotUser))
   {
      // the bot's user is dead!
      pBot->pBotUser = NULL;
      return (FALSE);
   }

   bool user_visible = false; // KWo - 02.05.2006

   if (g_i_botthink_index == g_iFrameCounter) // KWo - 02.05.2006
      user_visible = FVisible (pBot->pBotUser->v.origin, pBot->pEdict);

   // check if the "user" is still visible or if the user has been visible
   // in the last 30 seconds (or the player just starting "using" the bot)

   if (user_visible || (pBot->f_bot_use_time + 30 > gpGlobals->time))
   {
      if (user_visible)
         pBot->f_bot_use_time = gpGlobals->time; // reset "last visible time"

      return (TRUE);
   }
   else
   {
      // person to follow has gone out of sight...
      pBot->pBotUser = NULL;
   }
   return (FALSE);
}


void BotCheckRadioCommands (bot_t *pBot)
{
   // Radio Handling and Reactings to them

   edict_t *pPlayer = pBot->pRadioEntity; // Entity who used this Radio Command
   edict_t *pEdict = pBot->pEdict; // Bots Entity
   Vector vViewAngles = pBot->pEdict->v.v_angle;
   float f_distance = (pPlayer->v.origin - pEdict->v.origin).Length ();

   if (!FNullEnt(pPlayer))  // KWo - 25.04.2006
   {
      if (pEdict == pPlayer)
      {
         // Radio Command has been handled, reset
         pBot->iRadioOrder = 0;
         return;
      }
   }

   if (g_b_cv_ffa) // KWo - 07.03.2010
   {
   // Radio Command has been handled, reset
      pBot->iRadioOrder = 0;
      return;
   }

   switch (pBot->iRadioOrder)
   {
      case RADIO_FOLLOWME:
      {
         // check if line of sight to object is not blocked (i.e. visible)
         if (BotEntityIsVisible (pBot, pPlayer->v.origin))
         {
            // If Bot isn't already 'used' then follow him about half of the time
            if (FNullEnt (pBot->pBotUser) && (RANDOM_LONG (0, 99) < 50))
            {
               int iNumFollowers = 0;
               int i;

               // Check if no more followers are allowed
               for (i = 0; i < gpGlobals->maxClients; i++)
               {
                  if (bots[i].is_used && !bots[i].bDead)
                  {
                     if (bots[i].pBotUser == pPlayer)
                        iNumFollowers++;
                  }
               }

               if (iNumFollowers < g_i_cv_numfollowuser) // KWo - 06.04.2006
               {
                  // don't pause/camp/follow anymore
                  int iTask = BotGetSafeTask(pBot)->iTask;

                  if ((iTask == TASK_PAUSE) || (iTask == TASK_CAMP))
                     BotGetSafeTask(pBot)->fTime = gpGlobals->time;


                  // KWo - 08.07.2006
                  DeleteSearchNodes (pBot);
                  bool bPathValid;
                  int iTempIndex;
                  int iDestIndex;
                  pBot->pBotUser = pPlayer;
                  iTempIndex = WaypointFindNearestToMove (pBot->pBotUser, pBot->pBotUser->v.origin); // KWo - 17.04.2008
                  int iStepNr;
                  int iTempPathNr;
                  bool bCon = false;
                  iStepNr = 0;

                  do
                  {
                     iTempPathNr = (int) RANDOM_LONG(0, MAX_PATH_INDEX - 1);
                     iDestIndex = paths[iTempIndex]->index[iTempPathNr];
                     if ((iDestIndex > 0) && (iDestIndex <= g_iNumWaypoints))
                     {
                        if (IsConnectedWithWaypoint (iDestIndex, iTempIndex))
                        {
                           bCon = true;
                           break;
                        }
                     }
                     iStepNr++;
                  }
                  while ( (iStepNr < 10) && (!bCon));

                  if (!bCon)
                     iDestIndex = iTempIndex;

                  pBot->prev_goal_index = iDestIndex;
                  pBot->chosengoal_index = iDestIndex; // KWo - 27.06.2006
                  pBot->bCheckTerrain = TRUE;  // 25.06.2006
                  if ((iDestIndex != pBot->curr_wpt_index) && (pBot->curr_wpt_index >= 0))
                  {
                     // Do Pathfinding if it's not the current waypoint
                     pBot->pWayNodesStart = FindShortestPath (pBot->curr_wpt_index, iDestIndex, &bPathValid);  // KWo - 25.03.2006
                     pBot->pWaypointNodes = pBot->pWayNodesStart;
                     if (!bPathValid) // KWo - 25.03.2006
                     {
                        DeleteSearchNodes (pBot);
                        pBot->pBotUser = NULL;
                        if  (g_b_cv_radio) // KWo - 07.03.2010
                           BotPlayRadioMessage (pBot, RADIO_NEGATIVE);     // KWo - 25.08.2008
                     }
                     else
                     {
                        if (g_b_cv_radio) // KWo - 07.03.2010
                           BotPlayRadioMessage (pBot, RADIO_AFFIRMATIVE);  // KWo - 25.08.2008

                        if (g_b_DebugTasks)
                           ALERT(at_logged, "[DEBUG] Start Following - Bot %s is following the player; current wp index = %i, target wp_index = %i.\n",
                              pBot->name, pBot->curr_wpt_index, pBot->chosengoal_index);
                        pBot->f_bot_use_time = gpGlobals->time;
                        bottask_t TempTask = {NULL, NULL, TASK_FOLLOWUSER, TASKPRI_FOLLOWUSER, -1, gpGlobals->time + 60.0f, TRUE};
                        TempTask.iData = iDestIndex;
                        BotPushTask (pBot, &TempTask);
                     }
                  }
                  else
                     pBot->pBotUser = NULL;
               }
               else if (g_b_cv_radio) // KWo - 07.03.2010
                  BotPlayRadioMessage (pBot, RADIO_NEGATIVE);
            }
            else if ((pBot->pBotUser == pPlayer) && (g_b_cv_radio)) // KWo - 07.03.2010
               BotPlayRadioMessage (pBot, RADIO_IMINPOSITION);
            else if ((RANDOM_LONG (0, 100) < 50) && (g_b_cv_radio)) // KWo - 07.03.2010
               BotPlayRadioMessage (pBot, RADIO_NEGATIVE);
         }

         break;
      }
      case RADIO_HOLDPOSITION:
      {
         if (!FNullEnt (pBot->pBotUser) && (pBot->pBotUser == pPlayer))
         {
            pBot->pBotUser = NULL;
            if (g_b_cv_radio) // KWo - 07.03.2010
               BotPlayRadioMessage (pBot, RADIO_AFFIRMATIVE);
            pBot->iCampButtons = 0;
            bottask_t TempTask = {NULL, NULL, TASK_PAUSE, TASKPRI_PAUSE, -1, gpGlobals->time + RANDOM_FLOAT (30.0, 60.0), FALSE};
            BotPushTask (pBot, &TempTask);
         }
         break;
      }
      // Someone called for Assistance
      case RADIO_TAKINGFIRE:
      {
         if (FNullEnt (pBot->pBotUser))
         {
            if (FNullEnt (pBot->pBotEnemy))
            {
               // Decrease Fear Levels to lower probability of Bot seeking Cover again
               pBot->fFearLevel -= 0.2;

               if (pBot->fFearLevel < 0.0)
                  pBot->fFearLevel = 0.0;
               if (g_b_cv_radio) // KWo - 07.03.2010
                  BotPlayRadioMessage (pBot, RADIO_AFFIRMATIVE);

               // don't pause/camp anymore
               int iTask = BotGetSafeTask(pBot)->iTask;

               if ((iTask == TASK_PAUSE) || (iTask == TASK_CAMP))
                  BotGetSafeTask(pBot)->fTime = gpGlobals->time;

               pBot->f_bot_use_time = gpGlobals->time;
               pBot->vecPosition = pPlayer->v.origin;
               DeleteSearchNodes (pBot);
               int iIndex = WaypointFindNearestToMove (pPlayer, pBot->vecPosition); // KWo - 17.04.2008

               bottask_t TempTask = {NULL, NULL, TASK_MOVETOPOSITION, TASKPRI_MOVETOPOSITION, iIndex, 0.0, TRUE}; // KWo - 28.08.2006
               BotPushTask (pBot, &TempTask);
            }
            else if (g_b_cv_radio) // KWo - 07.03.2010
               BotPlayRadioMessage (pBot, RADIO_NEGATIVE);
         }
         break;
      }
      case RADIO_NEEDBACKUP:
      {
         if ((FNullEnt (pBot->pBotEnemy) && (BotEntityIsVisible (pBot, pPlayer->v.origin)))
             || (f_distance < 2048))
         {
            pBot->fFearLevel -= 0.1;
            if (pBot->fFearLevel < 0.0)
               pBot->fFearLevel = 0.0;
            if (g_b_cv_radio) // KWo - 07.03.2010
               BotPlayRadioMessage (pBot, RADIO_AFFIRMATIVE);

            // don't pause/camp anymore
            int iTask = BotGetSafeTask(pBot)->iTask;
            if ((iTask == TASK_PAUSE) || (iTask == TASK_CAMP))
               BotGetSafeTask(pBot)->fTime = gpGlobals->time;

            pBot->f_bot_use_time = gpGlobals->time;
            pBot->vecPosition = pPlayer->v.origin;
            DeleteSearchNodes (pBot);

            int iIndex = WaypointFindNearestToMove (pPlayer, pBot->vecPosition); // KWo - 17.04.2008
            bottask_t TempTask = {NULL, NULL, TASK_MOVETOPOSITION, TASKPRI_MOVETOPOSITION, iIndex, 0.0, TRUE}; // KWo - 28.08.2006
            BotPushTask (pBot, &TempTask);
         }
         else if (g_b_cv_radio) // KWo - 07.03.2010
            BotPlayRadioMessage (pBot, RADIO_NEGATIVE);
      }
      case RADIO_GOGOGO:
      {
         if (pPlayer == pBot->pBotUser)
         {
            if (g_b_cv_radio) // KWo - 07.03.2010
               BotPlayRadioMessage (pBot, RADIO_AFFIRMATIVE);
            pBot->pBotUser = NULL;
            pBot->fFearLevel -= 0.3;

            if (pBot->fFearLevel < 0.0)
               pBot->fFearLevel = 0.0;
         }
         else if ((FNullEnt (pBot->pBotEnemy) && (BotEntityIsVisible (pBot, pPlayer->v.origin)))
                  || (f_distance < 2048))
         {
            int iTask = BotGetSafeTask(pBot)->iTask;

            if ((iTask == TASK_PAUSE) || (iTask == TASK_CAMP))
            {
               pBot->fFearLevel -= 0.3;

               if (pBot->fFearLevel < 0.0)
                  pBot->fFearLevel = 0.0;
               if (g_b_cv_radio) // KWo - 07.03.2010
                  BotPlayRadioMessage (pBot, RADIO_AFFIRMATIVE);

               // don't pause/camp anymore
               BotGetSafeTask(pBot)->fTime = gpGlobals->time;
               pBot->f_bot_use_time = gpGlobals->time;
               pBot->pBotUser = NULL;
               MAKE_VECTORS (vViewAngles);
               pBot->vecPosition = pPlayer->v.origin + gpGlobals->v_forward * RANDOM_LONG (1024, 2048);
               DeleteSearchNodes (pBot);
               int iIndex = WaypointFindNearestToMove (pPlayer, pBot->vecPosition); // KWo - 17.04.2008
               bottask_t TempTask = {NULL, NULL, TASK_MOVETOPOSITION, TASKPRI_MOVETOPOSITION, iIndex, 0.0, TRUE}; // KWo - 28.08.2006

               if (g_b_DebugTasks)
                  ALERT(at_logged, "[DEBUG] >Go go go< the bot %s is moving to the position.\n", pBot->name);
               BotPushTask (pBot, &TempTask);
            }
         }
         else if (g_b_cv_radio) // KWo - 07.03.2010
            BotPlayRadioMessage (pBot, RADIO_NEGATIVE);
         break;
      }
      case RADIO_STORMTHEFRONT:
      {
         if ((FNullEnt (pBot->pBotEnemy) && BotEntityIsVisible (pBot, pPlayer->v.origin))
             || (f_distance < 1024))
         {
            if (g_b_cv_radio) // KWo - 07.03.2010
               BotPlayRadioMessage (pBot, RADIO_AFFIRMATIVE);

            // don't pause/camp anymore
            int iTask = BotGetSafeTask(pBot)->iTask;

            if ((iTask == TASK_PAUSE) || (iTask == TASK_CAMP))
               BotGetSafeTask(pBot)->fTime = gpGlobals->time;

            pBot->f_bot_use_time = gpGlobals->time;
            pBot->pBotUser = NULL;
            MAKE_VECTORS (vViewAngles);

// KWo - maybe instead this we should find the closest door and the bots should storm these doors? only if there is no door - random stuff?
            pBot->vecPosition = pPlayer->v.origin + gpGlobals->v_forward * RANDOM_LONG (1024, 2048);
            DeleteSearchNodes (pBot);
            int iIndex = WaypointFindNearestToMove (pPlayer, pBot->vecPosition); // KWo - 17.04.2008
            bottask_t TempTask = {NULL, NULL, TASK_MOVETOPOSITION, TASKPRI_MOVETOPOSITION, iIndex, 0.0, TRUE}; // KWo - 28.08.2006

            BotPushTask (pBot, &TempTask);
            pBot->fFearLevel -= 0.3;
            if (pBot->fFearLevel < 0.0)
               pBot->fFearLevel = 0.0;

            pBot->fAgressionLevel += 0.3;
            if (pBot->fAgressionLevel > 1.0)
               pBot->fAgressionLevel = 1.0;
         }
         break;
      }
      case RADIO_FALLBACK:
      {
         if ((FNullEnt (pBot->pBotEnemy) && (BotEntityIsVisible (pBot, pPlayer->v.origin)))
             || (f_distance < 1024))
         {
            pBot->fFearLevel += 0.5;
            if (pBot->fFearLevel > 1.0)
               pBot->fFearLevel = 1.0;

            pBot->fAgressionLevel -= 0.5;
            if (pBot->fAgressionLevel < 0.0)
               pBot->fAgressionLevel = 0.0;

            if (g_b_cv_radio) // KWo - 07.03.2010
               BotPlayRadioMessage (pBot, RADIO_AFFIRMATIVE);
            if (BotGetSafeTask(pBot)->iTask == TASK_CAMP)
            {
               BotGetSafeTask(pBot)->fTime += RANDOM_FLOAT (10.0, 15.0);
               pBot->fTimeCamping = BotGetSafeTask(pBot)->fTime; // KWo - 23.03.2008
            }
            else
            {
               // don't pause/camp anymore
               int iTask = BotGetSafeTask(pBot)->iTask;
               if (iTask == TASK_PAUSE)
                  BotGetSafeTask(pBot)->fTime = gpGlobals->time;

               pBot->f_bot_use_time = gpGlobals->time;
               pBot->pBotUser = NULL;

               // FIXME : Bot doesn't see enemy yet!
               pBot->f_bot_see_enemy_time = gpGlobals->time;

               // If Bot has no enemy
               if (pBot->vecLastEnemyOrigin == g_vecZero)
               {
                  float distance;
                  float nearestdistance = 9999.0;
                  int ind;

                  // Take nearest enemy to ordering Player
                  for (ind = 0; ind < gpGlobals->maxClients; ind++)
                  {
                     if (!(clients[ind].iFlags & CLIENT_USED)
                         || !(clients[ind].iFlags & CLIENT_ALIVE)
                         || (clients[ind].iTeam == pBot->bot_team))
                        continue;

                     edict_t *pEnemy = clients[ind].pEdict;
                     distance = (pPlayer->v.origin - pEnemy->v.origin).Length ();

                     if (distance < nearestdistance)
                     {
                        nearestdistance = distance;
                        pBot->pLastEnemy = pEnemy;
                        pBot->vecLastEnemyOrigin = pEnemy->v.origin;
                     }
                  }
               }

               DeleteSearchNodes(pBot);
            }
         }
         break;
      }
      case RADIO_REPORTTEAM:
      {
         if (g_b_cv_radio) // KWo - 07.03.2010
            BotPlayRadioMessage (pBot, RADIO_REPORTINGIN);
         break;
      }
      case RADIO_SECTORCLEAR:
      {
         // Is Bomb planted and it's a Counter
         if (g_bBombPlanted)
         {
            // Check if it's a Counter Command
            if ((UTIL_GetTeam (pPlayer) == TEAM_CS_COUNTER) && (pBot->bot_team == TEAM_CS_COUNTER))
            {
               if (g_fTimeNextBombUpdate < gpGlobals->time)
               {
                  float min_distance = 9999.0;
                  int i;

                  // Find Nearest Bomb Waypoint to Player
                  for (i = 0; i < g_iNumGoalPoints; i++)
                  {
                     f_distance = (paths[g_rgiGoalWaypoints[i]]->origin - pPlayer->v.origin).Length ();

                     if (f_distance < min_distance)
                     {
                        min_distance = f_distance;
                        g_iLastBombPoint = g_rgiGoalWaypoints[i];
                     }
                  }

                  // Enter this WPT Index as taboo wpt
                  CTBombPointClear (g_iLastBombPoint);
                  g_fTimeNextBombUpdate = gpGlobals->time + 0.5;
               }

               // Does this Bot want to defuse ?
               if (BotGetSafeTask(pBot)->iTask == TASK_NORMAL)
               {
                  // Is he approaching this goal ?
                  if (BotGetSafeTask(pBot)->iData == g_iLastBombPoint)
                  {
                     BotGetSafeTask(pBot)->iData = -1;
                     if (g_b_cv_radio)
                        BotPlayRadioMessage (pBot, RADIO_AFFIRMATIVE);
                  }
               }
            }
         }
         break;
      }
      case RADIO_GETINPOSITION:
      {
         if ((FNullEnt (pBot->pBotEnemy) && (BotEntityIsVisible (pBot, pPlayer->v.origin)))
             || (f_distance < 1024))
         {
            if (g_b_cv_radio) // KWo - 07.03.2010
               BotPlayRadioMessage (pBot, RADIO_AFFIRMATIVE);
            float fCampTime = RANDOM_FLOAT (30.0, 60.0); // KWo - 23.03.2008
            if (fCampTime > g_f_cv_maxcamptime) // KWo - 23.03.2008
               fCampTime = g_f_cv_maxcamptime;
            if (BotGetSafeTask(pBot)->iTask == TASK_CAMP)
            {
               BotGetSafeTask(pBot)->fTime = gpGlobals->time + fCampTime; // KWo - 23.03.2008
               pBot->fTimeCamping = BotGetSafeTask(pBot)->fTime;  // KWo - 23.03.2008
            }
            else if (BotHasCampWeapon(pBot) && (pBot->current_weapon.iAmmo1 > 0) && (!pBot->bIsChickenOrZombie)) // KWo - 18.01.2011
            {
               // don't pause anymore
               int iTask = BotGetSafeTask(pBot)->iTask;
               if (iTask == TASK_PAUSE)
                  BotGetSafeTask(pBot)->fTime = gpGlobals->time;

               pBot->f_bot_use_time = gpGlobals->time;
               pBot->pBotUser = NULL;

               // If Bot has no enemy
               if (pBot->vecLastEnemyOrigin == g_vecZero)
               {
                  float distance;
                  float nearestdistance = 9999.0;
                  int ind;

                  // Take nearest enemy to ordering Player
                  for (ind = 0; ind < gpGlobals->maxClients; ind++)
                  {
                     if (!(clients[ind].iFlags & CLIENT_USED)
                         || !(clients[ind].iFlags & CLIENT_ALIVE)
                         || (clients[ind].iTeam == pBot->bot_team))
                        continue;

                     edict_t *pEnemy = clients[ind].pEdict;
                     distance = (pPlayer->v.origin - pEnemy->v.origin).Length ();

                     if (distance < nearestdistance)
                     {
                        nearestdistance = distance;
                        pBot->pLastEnemy = pEnemy;
                        pBot->vecLastEnemyOrigin = pEnemy->v.origin;
                     // FIXME : Bot doesn't see enemy yet!
                        pBot->f_bot_see_enemy_time = gpGlobals->time;
                     }
                  }
               }

               DeleteSearchNodes(pBot);

               // Push camp task on to stack
               pBot->fTimeCamping = gpGlobals->time + fCampTime; // KWo - 23.03.2008
               bottask_t TempTask = {NULL, NULL, TASK_CAMP, TASKPRI_CAMP, -1, pBot->fTimeCamping, TRUE}; // KWo - 23.03.2008
               BotPushTask (pBot, &TempTask);

               // Push Move Command
               TempTask.iTask = TASK_MOVETOPOSITION;
               TempTask.fDesire = TASKPRI_MOVETOPOSITION;
               TempTask.iData = BotFindDefendWaypoint (pBot, pPlayer->v.origin);
               BotPushTask (pBot, &TempTask);
               pBot->iCampButtons |= IN_DUCK;
            }
         }
         break;
      }
   }

   // Radio Command has been handled, reset
   pBot->iRadioOrder = 0;
   return;
}

int GetHighestFragsBot (int iTeam)
{
   bot_t *pFragBot;
   int iBestIndex = 0;
   float fBestFrags = -1;
   int bot_index;

   // Search Bots in this team
   for (bot_index = 0; bot_index < gpGlobals->maxClients; bot_index++)
   {
      pFragBot = &bots[bot_index];

      if (pFragBot->is_used && !FNullEnt (pFragBot->pEdict) && !pFragBot->bDead
          && (pFragBot->bot_team == iTeam) && (pFragBot->pEdict->v.frags > fBestFrags))
      {
         iBestIndex = bot_index;
         fBestFrags = pFragBot->pEdict->v.frags;
      }
   }

   return (iBestIndex);
}


void SelectLeaderEachTeam (bot_t *pBot)
{
   bot_t *pBotLeader;
   edict_t *pEdict = pBot->pEdict;

   if (g_iMapType & MAP_AS)
   {
      if (pBot->bIsVIP && !g_bLeaderChosenCT)
      {
         // VIP Bot is the leader
         pBot->bIsLeader = TRUE;

         if ((RANDOM_LONG (1, 100) < 50) && (!g_b_cv_ffa) && (g_b_cv_radio)
            && (g_iAliveCTs > 1)) // KWo - 13.09.2008
         {
            BotPlayRadioMessage (pBot, RADIO_FOLLOWME);
            pBot->iCampButtons = 0;
         }

         g_bLeaderChosenCT = TRUE;
      }
      else if ((pBot->bot_team == TEAM_CS_TERRORIST) && !g_bLeaderChosenT)
      {
         pBotLeader = &bots[GetHighestFragsBot (pBot->bot_team)];
         pBotLeader->bIsLeader = TRUE;

         if ((RANDOM_LONG (1, 100) < 50) && (!g_b_cv_ffa) && (g_b_cv_radio)
            && (g_iAliveTs > 1)) // KWo - 13.09.2008
         {
            BotPlayRadioMessage (pBotLeader, RADIO_FOLLOWME);
         }

         g_bLeaderChosenT = TRUE;
      }
   }

   if (g_iMapType & MAP_CS)
   {
      if (pBot->bot_team == TEAM_CS_TERRORIST)
      {
         pBotLeader = &bots[GetHighestFragsBot (pBot->bot_team)];
         pBotLeader->bIsLeader = TRUE;

         if ((RANDOM_LONG (1, 100) < 50) && (!g_b_cv_ffa) && (g_b_cv_radio)
            && (g_iAliveTs > 1)) // KWo - 13.09.2008
         {
            BotPlayRadioMessage (pBotLeader, RADIO_FOLLOWME);
         }
      }
      else
      {
         pBotLeader = &bots[GetHighestFragsBot (pBot->bot_team)];
         pBotLeader->bIsLeader = TRUE;

         if ((RANDOM_LONG (1, 100) < 50) && (!g_b_cv_ffa) && (g_b_cv_radio)
            && (g_iAliveCTs > 1)) // KWo - 13.09.2008
         {
            BotPlayRadioMessage (pBotLeader, RADIO_FOLLOWME);
         }
      }
   }

   if (g_iMapType & MAP_DE)
   {
      if ((pBot->bot_team == TEAM_CS_TERRORIST) && !g_bLeaderChosenT)
      {
         if (pEdict->v.weapons & (1 << CS_WEAPON_C4))
         {
            // Bot carrying the Bomb is the leader
            pBot->bIsLeader = TRUE;

            // Terrorist carrying a Bomb needs to have some company so order some Bots sometimes
            if ((RANDOM_LONG (1, 100) < 50) && (!g_b_cv_ffa) && (g_b_cv_radio)
               && (g_iAliveTs > 1)) // KWo - 13.09.2008
            {
               BotPlayRadioMessage (pBot, RADIO_FOLLOWME);
               pBot->iCampButtons = 0;
            }

            g_bLeaderChosenT = TRUE;
         }
      }
      else if (!g_bLeaderChosenCT)
      {
         pBotLeader = &bots[GetHighestFragsBot (pBot->bot_team)];
         pBotLeader->bIsLeader = TRUE;

         if ((RANDOM_LONG (1, 100) < 50) && (!g_b_cv_ffa) && (g_b_cv_radio)
            && (g_iAliveCTs > 1)) // KWo - 13.09.2008
         {
            BotPlayRadioMessage (pBotLeader, RADIO_FOLLOWME);
         }

         g_bLeaderChosenCT = TRUE;
      }
   }

   return;
}

void BotChooseAimDirection (bot_t *pBot)
{
   static edict_t *pEdict;
   static TraceResult tr;
   static unsigned int iFlags;
   static int iIndex;
   static Vector v_src;
   static Vector v_dest;
   static Vector vecDirection;
   static Vector vLastEnLookDir;
   static float fLastEnDistance;
   static bool bPrevWptIndOK;
   static bool bCurWptIndOK;
   static bool bFollowUser;
   static bool bDestAimingSelected;
   static bool bDontShootThroughWall; // KWo - 08.10.2010
   static bool bRecalcPath;
   static float f_z_corr;
   static float f_angle_corr;
   static float f_dist_throw;

   bPrevWptIndOK = ((pBot->prev_wpt_index[0] >= 0) && (pBot->prev_wpt_index[0] < g_iNumWaypoints)); // KWo - 12.08.2006
   bCurWptIndOK = ((pBot->curr_wpt_index >= 0) && (pBot->curr_wpt_index < g_iNumWaypoints)); // KWo - 12.08.2006

   if (!bCurWptIndOK) // KWo - 12.08.2006
   {
      GetValidWaypoint(pBot);
      bCurWptIndOK = TRUE;
   }
   if (pBot->f_blind_time > gpGlobals->time) // KWo - 12.09.2008
      return;

   iFlags = pBot->iAimFlags;
   pEdict = pBot->pEdict;
   fLastEnDistance = 9999.0;
   bFollowUser = (BotGetSafeTask(pBot)->iTask == TASK_FOLLOWUSER);                        // KWo - 12.08.2006
   bDestAimingSelected = false;
   bDontShootThroughWall = false; // KWo - 08.10.2010

   if ((pBot->vecLastEnemyOrigin != g_vecZero) && (!FNullEnt(pBot->pLastEnemy)))          // KWo - 10.07.2008
   {
      TRACE_LINE (GetGunPosition (pEdict),pBot->vecLastEnemyOrigin, ignore_glass, pEdict, &tr);  // KWo - 27.01.2008

      fLastEnDistance = (pEdict->v.origin - pBot->vecLastEnemyOrigin).Length ();          // KWo - 19.01.2008

      vLastEnLookDir = UTIL_VecToAngles (pBot->vecLastEnemyOrigin - GetGunPosition (pEdict)); // KWo - 01.10.2010
      vLastEnLookDir.x = -vLastEnLookDir.x;  // KWo - 01.10.2010

      if (FNullEnt(pBot->pBotEnemy) && (pBot->f_bot_see_enemy_time + 2.0 < gpGlobals->time)
         && (((tr.flFraction <= 0.2) && (FStrEq(STRING (tr.pHit->v.classname), "worldspawn"))
          && (fLastEnDistance >= 300.0)
/*          && !(pBot->bShootThruSeen) && !(pBot->bShootThruHeard) */
          && (pBot->f_bot_see_new_enemy_time - 0.5 < gpGlobals->time))
          || (!(pBot->bShootThruSeen) && (fabs(vLastEnLookDir.x) >= 45.0))))              // KWo - 01.10.2010
      {
         if (((pBot->iAimFlags & AIM_LASTENEMY) || (pBot->iAimFlags & AIM_PREDICTPATH))   // 14.10.2006
            && (pBot->bWantsToFire))
         {
            pBot->bWantsToFire = FALSE;
            bDontShootThroughWall = TRUE; // KWo - 08.10.2010
         }

         if ((pBot->fChangeAimDirectionTime + 0.5 < gpGlobals->time) && !(pBot->bShootThruSeen) && !(pBot->bShootThruHeard))
         {
            pBot->iAimFlags &= ~AIM_LASTENEMY;
            iFlags &= ~AIM_LASTENEMY;
            pBot->iAimFlags &= ~AIM_PREDICTPATH;
            iFlags &= ~AIM_PREDICTPATH;
         }
      }
   }
   else
   {
      pBot->iAimFlags &= ~AIM_LASTENEMY;
      iFlags &= ~AIM_LASTENEMY;
      pBot->iAimFlags &= ~AIM_PREDICTPATH;
      iFlags &= ~AIM_PREDICTPATH;
   }

   if (pBot->vecEnemy == g_vecZero) // KWo - 27.07.2007
   {
      pBot->iAimFlags &= ~AIM_ENEMY;
      iFlags &= ~AIM_ENEMY;
   }

   // Don't allow Bot to look at danger positions under certain circumstances

   if ((BotGetSafeTask(pBot)->iTask != TASK_DEFUSEBOMB) && (pBot->pShootBreakable == NULL) && (pBot->fButtonNoticedTime + 5.0 < gpGlobals->time)
      && (pBot->iLiftUsageState != LIFT_LOOKING_BUTTON_OUTSIDE) && (pBot->iLiftUsageState != LIFT_LOOKING_BUTTON_INSIDE)
      && (BotGetSafeTask(pBot)->iTask != TASK_PICKUPITEM))  // KWo - 27.10.2006
   {
      if (bCurWptIndOK)
      {
         if ((!paths[pBot->curr_wpt_index]->flags & W_FL_USE_BUTTON) && (pBot->fButtonNoticedTime + 3.0 < gpGlobals->time)) // KWo - 29.03.2008
         {
            pBot->bButtonPushDecided = false;
            pBot->iAimFlags &= ~AIM_ENTITY;
            iFlags &= ~AIM_ENTITY;
         }
      }
   }

   if (!(iFlags & AIM_ENEMY) && !FNullEnt(pBot->pHit) && (iFlags & AIM_DEST)) // KWo - 29.03.2008
   {
      if ((strncmp (STRING (pBot->pHit->v.classname), "func_door", 9) == 0)  && (pBot->iLiftUsageState == LIFT_NO_NEARBY))
      {
         pBot->iAimFlags &= ~AIM_LASTENEMY;
         iFlags &= ~AIM_LASTENEMY;
      }
   }


   if (!(iFlags & (AIM_GRENADE | AIM_ENEMY | AIM_ENTITY)))
   {
      if ((pBot->bOnLadder || pBot->bInWater
         || (pBot->curr_travel_flags & C_FL_JUMP) || (pBot->iWPTFlags & W_FL_LADDER)) && (fLastEnDistance > 500.0)) // KWo - 19.01.2008
      {
         iFlags &= ~(AIM_LASTENEMY | AIM_PREDICTPATH);
         pBot->bCanChooseAimDirection = FALSE;
      }
   }

   if (iFlags & AIM_OVERRIDE)
   {
      if (BotGetSafeTask(pBot)->iTask == TASK_SHOOTBREAKABLE)
      {
         pBot->vecLookAt = pBot->vecBreakable;
         if (g_b_DebugEntities)
         {
            ALERT(at_logged, "[DEBUG] BotChooseAimDirection - Bot %s should look at = [%i,%i,%i].\n",
               pBot->name, (int)pBot->vecBreakable.x, (int)pBot->vecBreakable.y, (int)pBot->vecBreakable.z);
         }
      }
      else
         pBot->vecLookAt = pBot->vecCamp;
      pBot->fChangeAimDirectionTime = gpGlobals->time + 1.0;      // KWo - 12.03.2010
   }
   else if (iFlags & AIM_GRENADE)
   {
      pBot->vecLookAt = pBot->vecThrow; // KWo - 19.06.2006
      f_z_corr = 0.0; // KWo - 16.09.2006
      f_angle_corr = 0.0; // KWo - 16.09.2006
      f_dist_throw = (pBot->vecThrow - pEdict->v.origin).Length(); // KWo - 16.09.2006
      if ((f_dist_throw > 100.0) && (f_dist_throw < 800.0)) // KWo - 25.02.2008
      {
         f_angle_corr = 0.0;
         f_z_corr = (0.25) * (pBot->vecThrow.z - pEdict->v.origin.z);  // KWo - 25.02.2008
      }
      else if (f_dist_throw >= 800.0)
      {
         f_angle_corr = 37.0 * (f_dist_throw - 800.0)/800.0;
         if (f_angle_corr > 45.0)
            f_angle_corr = 45.0;
         f_z_corr = f_dist_throw * tan (f_angle_corr * M_PI / 180.0)
           + (0.25) * (pBot->vecThrow.z - pEdict->v.origin.z); // KWo - 25.02.2008
      }

      pBot->vecLookAt.z += f_z_corr; // KWo - 16.09.2006
      pBot->fChangeAimDirectionTime = gpGlobals->time + 1.0;      // KWo - 24.02.2008
   }
   else if (iFlags & AIM_ENTITY)
   {
      if (pBot->vecEntity != g_vecZero) // KWo - 30.12.2009
      {
         pBot->vecLookAt = pBot->vecEntity;
         pBot->fChangeAimDirectionTime = gpGlobals->time + 0.6; // KWo - 06.02.2008
      }
      if ((BotGetSafeTask(pBot)->iTask == TASK_PICKUPITEM) && (pBot->iPickupType == PICKUP_HOSTAGE)) // KWo - 12.10.2006
         pBot->vecLookAt = pBot->vecEntity + pBot->pEdict->v.view_ofs;
      if (((BotGetSafeTask(pBot)->iTask == TASK_PLANTBOMB) || (BotGetSafeTask(pBot)->iTask == TASK_DEFUSEBOMB)
         || (BotGetSafeTask(pBot)->iTask == TASK_ATTACK))
         && (iFlags & AIM_ENEMY))
      {
         BotFocusEnemy (pBot);             // KWo - 10.10.2006
         pBot->vecLookAt = pBot->vecEnemy; // KWo - 10.10.2006
      }
   }
   else if (iFlags & AIM_ENEMY) // KWo - 04.03.2006
   {
      pBot->fLastHeardEnOrgUpdateTime = gpGlobals->time - 0.1;
      if (bDontShootThroughWall) // KWo - 08.10.2010 (they still should track the enemy, but without shot)
      {
         pBot->f_enemy_surprise_time = gpGlobals->time + 0.01;
         pBot->bWantsToFire = FALSE;
      }

      BotFocusEnemy(pBot);             // KWo - 10.10.2006
      pBot->vecLookAt = pBot->vecEnemy; // KWo - 10.10.2006
      pBot->fChangeAimDirectionTime = gpGlobals->time + 1.0; // KWo - 06.02.2008
   }
   else if (iFlags & AIM_LASTENEMY)// KWo - 04.03.2006
   {
      // Did Bot just see Enemy and is quite agressive ?
      if ((pBot->f_heard_sound_time + 4.0 > gpGlobals->time)) // KWo - 12.07.2008
      {
         // Feel free to fire if shootable
         if ((pBot->bShootThruHeard) && (!bDontShootThroughWall)) // KWo - 08.10.2010
         {
            pBot->bWantsToFire = TRUE;
         }
         else
            pBot->bWantsToFire = FALSE;

         pBot->vecLookAt = pBot->vecLastEnemyOrigin;  // KWo - 21.01.2008

         if (pBot->fChangeAimDirectionTime < gpGlobals->time) // KWo - 20.03.2010
            pBot->fChangeAimDirectionTime = gpGlobals->time + 1.0;
      }
      else
      {
         pBot->vecLookAt = pBot->vecLastEnemyOrigin;              // KWo - 21.01.2008
      }
   }
   else if (iFlags & AIM_PREDICTPATH) // KWo - 04.03.2006
   {
      if  ( (((pEdict->v.origin - pBot->vecLastEnemyOrigin).Length () < 1600) || BotUsesSniper(pBot))
          && ( (tr.flFraction >= 0.2) || !(FStrEq(STRING (tr.pHit->v.classname), "worldspawn")))) // KWo - 22.10.2006
      {
         bRecalcPath = TRUE;

         if ((pBot->pTrackingEdict == pBot->pLastEnemy) && (!FNullEnt(pBot->pLastEnemy))
             && (pBot->fTimeNextTracking < gpGlobals->time)) // KWo - 06.08.2006
            bRecalcPath = FALSE;

         if (bRecalcPath)
         {
            pBot->vecLookAt = pBot->vecLastEnemyOrigin; // KWo - 23.10.2006
            pBot->vecCamp = pBot->vecLookAt;
            pBot->fTimeNextTracking = gpGlobals->time + 0.5;
            pBot->pTrackingEdict = pBot->pLastEnemy;

            if (g_b_DebugSensing)
            {
               ALERT (at_logged,"[DEBUG] AIM_PREDICTPATH flag works for bot %s.\n", pBot->name);
            }

            if ((BotLastEnemyShootable (pBot)) && (IsShootableThruObstacle (pEdict, pBot->vecLastEnemyOrigin))) // KWo - 14.10.2006
               pBot->bWantsToFire = TRUE;

         }
         else
            pBot->vecLookAt = pBot->vecCamp;
      }
      else // KWo - 19.02.2006 - forget enemy far away
      {
         pBot->iAimFlags &= ~AIM_PREDICTPATH;
      }
      pBot->fChangeAimDirectionTime = gpGlobals->time + 1.0; // KWo - 17.02.2008
   }
   else if (iFlags & AIM_CAMP)
   {
      pBot->vecLookAt = pBot->vecCamp;
      pBot->fChangeAimDirectionTime = gpGlobals->time + 1.0; // KWo - 17.02.2008
   }
   else if (iFlags & AIM_DEST) // KWo - 04.03.2006
   {

      if (pBot->fChangeAimDirectionTime > gpGlobals->time) // KWo - 16.02.2008
      {
         bDestAimingSelected = TRUE;
      }

//    look at the user after reaching goal WP for following the user
      if (bFollowUser && !FNullEnt(pBot->pBotUser)
         && !(paths[pBot->curr_wpt_index]->flags & W_FL_LADDER) && bPrevWptIndOK
         && (pBot->fChangeAimDirectionTime < gpGlobals->time))  // KWo - 09.12.2007
      {
         if (!(paths[pBot->prev_wpt_index[0]]->flags & W_FL_LADDER) && ((pBot->dest_origin - pBot->pBotUser->v.origin).Length() < 250)
            && (FVisible (pBot->pBotUser->v.origin, pBot->pEdict))
            && ((pBot->curr_wpt_index == pBot->chosengoal_index)) || (pBot->chosengoal_index == -1))
         {
            pBot->vecLookAt = pBot->pBotUser->v.origin + pBot->pEdict->v.view_ofs;
            pBot->fChangeAimDirectionTime = gpGlobals->time + 0.5; // KWo - 09.12.2007
            bDestAimingSelected = TRUE; // KWo - 09.12.2007
         }
      }

      if ((paths[pBot->curr_wpt_index]->flags & W_FL_LADDER)
          || ((paths[pBot->curr_wpt_index]->flags & W_FL_CROUCH) && !(paths[pBot->curr_wpt_index]->flags & W_FL_CAMP))
          || (pBot->curr_travel_flags & C_FL_JUMP)) // KWo - 07.04.2010
      {
         bDestAimingSelected = TRUE;
         pBot->vecLookAt = pBot->dest_origin + pBot->pEdict->v.view_ofs;
         if ((paths[pBot->curr_wpt_index]->flags & W_FL_LADDER) && (pBot->pWaypointNodes != NULL))   // KWo - 22.01.2012
         {
            if (pBot->pWaypointNodes->NextNode != NULL)
            {
               if ((paths[pBot->pWaypointNodes->NextNode->iIndex]->flags & W_FL_LADDER) && ((pEdict->v.origin - pBot->dest_origin).Length () < 100)
                  && (paths[pBot->pWaypointNodes->NextNode->iIndex]->origin.z > paths[pBot->curr_wpt_index]->origin.z + 30.0))
               {
                  pBot->vecLookAt = paths[pBot->pWaypointNodes->NextNode->iIndex]->origin;

                  if (g_b_DebugSensing)
                     ALERT(at_logged, "[DEBUG] Bot %s is looking at the next ladder WP.\n", pBot->name);
               }
            }
         }

         pBot->fChangeAimDirectionTime = gpGlobals->time + 0.3; // KWo - 06.02.2008
      }
// look at the door to let them be opened right...
      if (!FNullEnt(pBot->pHit)) // KWo - 22.03.2008
      {
         if ((strncmp (STRING (pBot->pHit->v.classname), "func_door", 9) == 0)  && (pBot->iLiftUsageState == LIFT_NO_NEARBY))
         {
            pBot->fChangeAimDirectionTime = gpGlobals->time + 1.0;
            bDestAimingSelected = TRUE;
            pBot->vecLookAt = pBot->dest_origin + pBot->pEdict->v.view_ofs;
         }
      }

//    look backward if the bot is looking at the wall...
      if (bPrevWptIndOK && !(paths[pBot->curr_wpt_index]->flags & W_FL_LADDER)
         && !bFollowUser && pBot->bCanChooseAimDirection  && (pBot->fChangeAimDirectionTime < gpGlobals->time)
         && ( (pBot->cCollisionState != COLLISION_PROBING)
          || ((pBot->cCollideMoves[(int) pBot->cCollStateIndex] != COLLISION_STRAFELEFT)
               && (pBot->cCollideMoves[(int) pBot->cCollStateIndex] != COLLISION_STRAFERIGHT))))   // KWo - 16.12.2007
      {
         if (!(paths[pBot->prev_wpt_index[0]]->flags & W_FL_LADDER)
              && !(pBot->curr_travel_flags & C_FL_JUMP)        // KWo - 19.04.2006
              && (!(pBot->iWPTFlags & W_FL_CROUCH) && (fabs(paths[pBot->prev_wpt_index[0]]->origin.z - pBot->dest_origin.z) < 30.0)
                  || (pBot->iWPTFlags & W_FL_CAMP)))// KWo - 23.03.2006
         {
            v_src = paths[pBot->prev_wpt_index[0]]->origin;
            vecDirection = (pBot->dest_origin - v_src).Normalize ();
            v_dest = pBot->dest_origin + vecDirection * 80;

            // trace forward...
            TRACE_LINE (pBot->dest_origin, v_dest, dont_ignore_monsters, pEdict, &tr);
            // check if the trace hit a wall, then look back (I hate them looking at the wall :) )
            if ((tr.flFraction < 1.0) && (FStrEq("worldspawn", STRING (tr.pHit->v.classname))))
            {
               pBot->vecLookAt = paths[pBot->prev_wpt_index[0]]->origin + pBot->pEdict->v.view_ofs;
               pBot->fChangeAimDirectionTime = gpGlobals->time + 0.6; // KWo - 09.12.2007
               bDestAimingSelected = TRUE; // KWo - 09.12.2007
            }
         }
      }

//    look at the danger position if it's seen from bot's eyes...
      if (pBot->bCanChooseAimDirection && (pBot->f_timeHitDoor + 2.0 < gpGlobals->time)
          && !(paths[pBot->curr_wpt_index]->flags & W_FL_LADDER) && !bFollowUser
          && bPrevWptIndOK && (!bDestAimingSelected))  // KWo - 22.03.2008
      {
         iIndex = pBot->curr_wpt_index;
         if (!(paths[pBot->prev_wpt_index[0]]->flags & W_FL_LADDER)) // KWo - 12.08.2006
         {
            if (pBot->bot_team == TEAM_CS_TERRORIST)
            {
               if (((pBotExperienceData + (iIndex * g_iNumWaypoints) + iIndex)->iTeam0_danger_index != -1)
                  && ((float)((pBotExperienceData + (iIndex * g_iNumWaypoints) + iIndex)->uTeam0Damage)/g_iHighestDamageT > 0.6)) // KWo - 07.04.2010
               {
                  v_dest = paths[(pBotExperienceData + (iIndex * g_iNumWaypoints) + iIndex)->iTeam0_danger_index]->origin;   // KWo - 12.03.2006
                  TRACE_LINE (pEdict->v.origin, v_dest, ignore_monsters, pEdict, &tr);  // KWo - 12.03.2006
                  if ((tr.flFraction > 0.8) || (!FStrEq("worldspawn", STRING (tr.pHit->v.classname))))  // KWo - 12.03.2006
                  {
                     pBot->vecLookAt = v_dest + pBot->pEdict->v.view_ofs;
                     pBot->fChangeAimDirectionTime = gpGlobals->time + 0.6; // KWo - 16.02.2008
                     bDestAimingSelected = TRUE; // KWo - 09.12.2007
                  }
               }
            }
            else
            {
               if (((pBotExperienceData + (iIndex * g_iNumWaypoints) + iIndex)->iTeam1_danger_index != -1)
                  && ((float)((pBotExperienceData + (iIndex * g_iNumWaypoints) + iIndex)->uTeam1Damage)/g_iHighestDamageCT > 0.6)) // KWo - 07.04.2010
               {
                  v_dest = paths[(pBotExperienceData + (iIndex * g_iNumWaypoints) + iIndex)->iTeam1_danger_index]->origin;  // KWo - 12.03.2006
                  TRACE_LINE (pEdict->v.origin, v_dest, ignore_monsters, pEdict, &tr);  // KWo - 12.03.2006
                  if ((tr.flFraction > 0.8) || (!FStrEq("worldspawn", STRING (tr.pHit->v.classname))))  // KWo - 12.03.2006
                  {
                     pBot->vecLookAt = v_dest + pBot->pEdict->v.view_ofs;
                     pBot->fChangeAimDirectionTime = gpGlobals->time + 0.6; // KWo - 16.02.2008
                     bDestAimingSelected = TRUE; // KWo - 09.12.2007
                  }
               }
            }
         }
      }

      if ((pBot->dest_origin != g_vecZero)
         && ((pBot->fChangeAimDirectionTime < gpGlobals->time) && (!bDestAimingSelected)
         && ( (pBot->cCollisionState != COLLISION_PROBING)
          || ((pBot->cCollideMoves[(int) pBot->cCollStateIndex] != COLLISION_STRAFELEFT)
           && (pBot->cCollideMoves[(int) pBot->cCollStateIndex] != COLLISION_STRAFERIGHT)
           && (pBot->cCollideMoves[(int) pBot->cCollStateIndex] != COLLISION_GOBACK)))))   // KWo - 29.09.2010
      {
         if ((pBot->dest_origin - pBot->pEdict->v.origin).Length() > 15)
            pBot->vecLookAt = pBot->dest_origin + pBot->pEdict->v.view_ofs;
         else if (pBot->bMoveToGoal) // KWo - 20.04.2013
         {
            if ((pBot->pWaypointNodes != NULL) && !(paths[pBot->curr_wpt_index]->flags & W_FL_LADDER)
               && !bFollowUser )// Look forward at the next waypoint
            {
               if (pBot->pWaypointNodes->NextNode != NULL)
               {
                  if ((pBot->pWaypointNodes->NextNode->iIndex >= 0) && (pBot->pWaypointNodes->NextNode->iIndex < g_iNumWaypoints))
                  {
                     pBot->vecLookAt = paths[pBot->pWaypointNodes->NextNode->iIndex]->origin + pBot->pEdict->v.view_ofs;
                  }
               }
            }
         }
         else
         {
            MAKE_VECTORS (pEdict->v.v_angle);
            pBot->vecLookAt = pBot->pEdict->v.origin + pBot->pEdict->v.view_ofs + gpGlobals->v_forward * 300;
         }
      }
/*
      if (pBot->f_timeHitDoor > gpGlobals->time)
      {
         pBot->vecLookAt = pBot->wpt_origin + pBot->pEdict->v.view_ofs;
      }
*/

   }

   if ((pBot->vecLookAt == g_vecZero) && (pBot->dest_origin != g_vecZero)) // 14.07.2006
   {
      pBot->vecLookAt = pBot->dest_origin + pBot->pEdict->v.view_ofs; // KWo - 27.06.2006
   }
   return;
}


bool BotHasHostage (bot_t *pBot)
{
   int i;

   if ((pBot->bot_team == TEAM_CS_TERRORIST) || (g_iNumHostages == 0)) // KWo - 07.04.2010
      return (FALSE);

   for (i = 0; i < g_iNumHostages; i++)
   {
      if (!FNullEnt (pBot->pHostages[i]))
      {
         if ((pBot->pHostages[i]->v.health > 0) || ((pBot->pHostages[i]->v.origin - pBot->pEdict->v.origin).Length() < 600)) // KWo - 16.07.2006
            return (TRUE);
         else
            pBot->pHostages[i] = NULL;
      }
   }
   return (FALSE);
}


void BotCheckShield (bot_t *pBot) // KWo - 13.04.2010
{
   static int iEnemyIndex;
   static int iWeaponEnemy;
   static float fEnemyDot;

   if (pBot->pBotEnemy)
   {
      iEnemyIndex = ENTINDEX(pBot->pBotEnemy)-1;
      iWeaponEnemy = clients[iEnemyIndex].iCurrentWeaponId;

      if (BotHasShield(pBot)) // KWo - 17.07.2007
      {
         fEnemyDot = GetShootingConeDeviation (pBot->pBotEnemy, &pBot->pEdict->v.origin);

         if (g_b_DebugCombat)
            ALERT(at_logged,"[DEBUG] Bot %s has an enemy %s; he has the weapon %s and his flDot = %0.3f; weapon anim = %d.\n",
               pBot->name, STRING(pBot->pBotEnemy->v.netname), weapon_defs[iWeaponEnemy].szClassname, fEnemyDot, pBot->pEdict->v.weaponanim);

         if (!BotHasShieldDrawn(pBot))
         {
            if ((WeaponIsPistol(iWeaponEnemy) || WeaponIsPrimaryGun(iWeaponEnemy)) && (fEnemyDot > 0.92)
               && (pBot->pBotEnemy->v.button & IN_ATTACK) && (pBot->fTimeWeaponSwitch + 0.5 < gpGlobals->time)
               && (clients[iEnemyIndex].fReloadingTime < gpGlobals->time)) // KWo - 15.08.2007
            {
               pBot->pEdict->v.button |= IN_ATTACK2;
               pBot->fTimeWeaponSwitch = gpGlobals->time;
            }
         }
         else
         {
            if (((!WeaponIsPistol(iWeaponEnemy) && !WeaponIsPrimaryGun(iWeaponEnemy)) || (fEnemyDot < 0.85)
               || (clients[iEnemyIndex].fReloadingTime >= gpGlobals->time)) // KWo - 15.08.2007)
                 && (pBot->fTimeWeaponSwitch + 0.2 < gpGlobals->time))
            {
               pBot->pEdict->v.button |= IN_ATTACK2;
               pBot->fTimeWeaponSwitch = gpGlobals->time;
            }
         }
      }
   }
}


void BotCheckDarkness (bot_t *pBot)  // KWo - 13.04.2010
{
   static bool bNvg;
   static float fLightLevel;
   static edict_t *pEdict;
   pEdict = pBot->pEdict;

   if ((g_i_botthink_index == g_iFrameCounter)
        && (pBot->f_spawn_time + 6.0 < gpGlobals->time)) // KWo - 23.03.2012
   {
      fLightLevel = UTIL_IlluminationOf(pEdict);

      if (g_b_DebugSensing)
         ALERT(at_logged,"[DEBUG] Bot %s has lightlevel = %.1f, skycolor = %d, flashbat = %d.\n",
            pBot->name, fLightLevel, (int)g_f_cv_skycolor, pBot->cFlashBat);
      bNvg = BotHasNvg(pBot);

      if (g_b_cv_flash_allowed && gmsgFlashlight && !bNvg)
      {
         if ((((g_f_cv_skycolor > 50.0) && (fLightLevel < 10.0)) || ((g_f_cv_skycolor <= 50.0) && (fLightLevel < 40.0)))
            && (!(pEdict->v.effects & EF_DIMLIGHT)) && (BotGetSafeTask(pBot)->iTask != TASK_CAMP)
            && (BotGetSafeTask(pBot)->iTask != TASK_ATTACK) && (pBot->f_heard_sound_time + 3.0 < gpGlobals->time)
            && (pBot->cFlashBat > 30))
         {
            pEdict->v.impulse = 100;
         }
         else if ((pEdict->v.effects & EF_DIMLIGHT)
               && ((((fLightLevel > 15.0) && (g_f_cv_skycolor > 50.0)) || ((fLightLevel > 45.0) && (g_f_cv_skycolor <= 50.0)))
               || (BotGetSafeTask(pBot)->iTask == TASK_CAMP) || (BotGetSafeTask(pBot)->iTask == TASK_ATTACK)
               || (pBot->cFlashBat <= 0) || (pBot->f_heard_sound_time + 3.0 >= gpGlobals->time)))
         {
            pEdict->v.impulse = 100;
         }
      }
      else if (bNvg && gmsgNVGToggle)
      {
         if (pEdict->v.effects & EF_DIMLIGHT)
         {
            pEdict->v.impulse = 100;
         }
         else if ((((g_f_cv_skycolor > 50.0) && (fLightLevel < 15.0)) || ((g_f_cv_skycolor <= 50.0) && (fLightLevel < 40.0)))
            && !pBot->bUsesNVG)
         {
            FakeClientCommand (pEdict, "nightvision\n");

            if (g_b_DebugSensing)
               ALERT(at_logged,"[DEBUG] Bot %s uses nvg - lightlevel = %.1f, skycolor = %d, flashbat = %d.\n",
                  pBot->name, fLightLevel, (int)g_f_cv_skycolor, pBot->cFlashBat);
         }
         else if ((((fLightLevel > 20.0) && (g_f_cv_skycolor > 50.0)) || ((fLightLevel > 45.0) && (g_f_cv_skycolor <= 50.0)))
               && pBot->bUsesNVG)
         {
            FakeClientCommand (pEdict, "nightvision\n");

            if (g_b_DebugSensing)
               ALERT(at_logged,"[DEBUG] Bot %s doesn't use nvg - lightlevel = %.1f, skycolor = %d, flashbat = %d.\n",
                  pBot->name, fLightLevel, (int)g_f_cv_skycolor, pBot->cFlashBat);
         }
      }
   }
}


// Executing certain task
void BotExecuteTask (bot_t *pBot)
{
   static edict_t *pEdict;
   static edict_t *pPlayer;
   static bot_t *pOtherBot;
   static TraceResult tr;
   static bool bEndTask;
   static bool bCampingAllowed;
   static bool bHostAlive;
   static bool bNewPosition;
   static bool bPathValid;
   static bool bBotReachedDest;
   static bool bCon;
   static char vmodelname[64];
   static float angle_to_entity;
   static float flDot;
   static float flDot1;
   static float fDesire;
   static float fEnemyDot;
   static float fCampTime;
   static float fDistanceToUser;
   static float fDistanceTraced; // KWo - 20.01.2012
   static int i, c;
   static int index2;
   static int iCount;
   static int iDestIndex;
   static int iGoalIndex;
   static int iGoalChoices[4];
   static int iTempDestIndex;
   static int iTempIndex;
   static int iLoopIndex;
   static int iWpIndex;
   static int iWeaponNum;
   static int iDangerWpIndex;
   static int iDangerMax;
   static int iTempWpIndex1;
   static int iTempWpIndex2;
   static int iTempDanger;
   static int iPathIndex;
   static int min_distance;
   static int distance;
   static int min_index;
   static int iRescueIndex;
   static int iGroundIndex;
   static int iTempPathNr;
   static int iFriendlyNum;
   static int iEnemyNum;
   static int iTask;
   static int *ptrWeaponTab;
   static Vector v_direction;
   static Vector vecDirectionNormal;
   static Vector vecDirection;
   static Vector vecMoveAngles;
   static Vector v_angles;
   static Vector v_dest;
   static Vector v_src;
   static Vector v_diff;
   static Vector vecSprayPos;
   static Vector v_bomb;


   pBot->bMoveToGoal = TRUE;
   bEndTask = false;  // KWo - 05.08.2006
   pEdict = pBot->pEdict;

   iTask = BotGetSafeTask(pBot)->iTask;   // KWo - 12.04.2010
   if (g_iDebugGoalIndex != -1)           // KWo - 12.04.2010
   {
      if (g_b_DebugTasks || g_b_DebugNavig)
         ALERT(at_logged, "[DEBUG] BotExecuteTask - Bot %s - debuggoal activated - with node nr %i.\n", pBot->name, g_iDebugGoalIndex);

      if ((iTask != TASK_NORMAL) || (BotGetSafeTask(pBot)->iData != g_iDebugGoalIndex))
      {
         DeleteSearchNodes (pBot);
         if ((iTask != TASK_NORMAL) && (iTask != TASK_SHOOTBREAKABLE)
            && (iTask != TASK_PICKUPITEM) && (iTask != TASK_SPRAYLOGO))// KWo - 22.10.2011
            BotTaskComplete(pBot);

         if (g_b_DebugTasks || g_b_DebugNavig)
            ALERT(at_logged, "[DEBUG] BotExecuteTask - debuggoal activated - Bot %s - cleaning other tasks.\n", pBot->name);

         BotGetSafeTask(pBot)->iData = g_iDebugGoalIndex;
         pBot->chosengoal_index = g_iDebugGoalIndex;
         pBot->prev_goal_index = -1;
         pBot->pHuntEnemy = NULL;
         pBot->vecHuntEnemyOrigin = g_vecZero;
         pBot->pLastEnemy = NULL;
         pBot->vecLastEnemyOrigin = g_vecZero;
         pBot->pBotEnemy = NULL;
         pBot->vecEnemy = g_vecZero;
      }
   }

   switch (BotGetSafeTask(pBot)->iTask)
   {
   // Normal (roaming) Task
      case TASK_NORMAL:
      {
         pBot->iAimFlags |= AIM_DEST;
         pBot->bUsingGrenade = FALSE;

         if (BotHasShieldDrawn (pBot))
            pEdict->v.button |= IN_ATTACK2;

         BotCheckReload (pBot);  // Check if we should reload, Thanks bluebyte


         // If Bomb planted and it's a Counter
      // calculate new path to Bomb Point if he's not already heading for
         if (g_bBombPlanted && (pBot->bot_team == TEAM_CS_COUNTER) && (g_iDebugGoalIndex == -1) )  // KWo - 03.02.2006
         {
            if (BotGetSafeTask(pBot)->iData != -1)
            {
               if (!(paths[BotGetSafeTask(pBot)->iData]->flags & W_FL_GOAL)
               && ((paths[BotGetSafeTask(pBot)->iData]->origin - g_vecBomb).Length() > 128.0)) // KWo - 16.05.2008
               {
                  DeleteSearchNodes (pBot);
                  BotGetSafeTask(pBot)->iData = -1;
               }
            }
         }

         // if the bot is about to come to the camp spot, but there is already someone else camping ...
         if ((pBot->curr_wpt_index == BotGetSafeTask(pBot)->iData) && (BotGetSafeTask(pBot)->iData != -1)) // KWo - 15.06.2008
         {
            // Heading Waypoint is a Camp Waypoint
            if (paths[pBot->curr_wpt_index]->flags & W_FL_CAMP)
            {
               // Check if Bot has got a primary weapon and hasn't camped before
               if (BotHasCampWeapon (pBot) && (pBot->current_weapon.iAmmo1 > 0) && (pBot->fTimeCamping + 10.0 < gpGlobals->time)
                && !((pBot->bot_team == TEAM_CS_COUNTER) && g_bBombPlanted)) // KWo - 25.05.2010
               {
                  bCampingAllowed = TRUE;

                  // Check if it's not allowed for this team to camp here
                  if (pBot->bot_team == TEAM_CS_TERRORIST)
                  {
                     if (paths[pBot->curr_wpt_index]->flags & W_FL_COUNTER)
                        bCampingAllowed = FALSE;
                  }
                  else
                  {
                     if (paths[pBot->curr_wpt_index]->flags & W_FL_TERRORIST)
                        bCampingAllowed = FALSE;
                  }

                  // Check if another Bot is already camping here
                  for (c = 0; c < gpGlobals->maxClients; c++)
                  {
                     pPlayer = clients[c].pEdict;

                     if (!FNullEnt(pPlayer))
                     {
                        if (!(clients[c].iFlags & CLIENT_USED)
                           || !(clients[c].iFlags & CLIENT_ALIVE)
                           || (pPlayer == pBot->pEdict))
                              continue;

                        if (((pPlayer->v.origin - paths[pBot->curr_wpt_index]->origin).Length() < 50.0)
                           && ((pPlayer->v.velocity).Length2D() < 30.0))
                           bCampingAllowed = FALSE;
                     }
                  }
                  if (!bCampingAllowed)
                  {
                     DeleteSearchNodes (pBot);
                     BotGetSafeTask(pBot)->iData = -1;
                  }
               }
            }
         }

         // Reached the destination (goal) waypoint ?
         if (BotDoWaypointNav (pBot))
         {
            DeleteSearchNodes(pBot);      // KWo - 30.12.2009 moved above checking spraying logo task
            if (!(pBot->b_bomb_blinking)) // KWo - 26.04.2008
            {
               pBot->prev_goal_index = -1;
               pBot->pTasks->iData = -1;
               pBot->chosengoal_index = -1;
            }

            // Spray Logo sometimes if allowed to do so
            if (!pBot->bLogoSprayed && (g_b_cv_spray) && (!(pBot->b_bomb_blinking) || !(pEdict->v.weapons & (1 << CS_WEAPON_C4))))  // KWo - 24.08.2006
            {
               if (RANDOM_LONG (1, 100) < 50)
               {
                  bottask_t TempTask = {NULL, NULL, TASK_SPRAYLOGO, TASKPRI_SPRAYLOGO, -1, gpGlobals->time + 1.0f, FALSE};
                  BotPushTask (pBot, &TempTask);
               }
            }

            if (g_b_DebugTasks || g_b_DebugNavig)
            {
               ALERT(at_logged, "[DEBUG] BotExecuteTask - TASK_NORMAL - Bot %s has reached the goal.\n", pBot->name);
            }

            pBot->prev_time = gpGlobals->time + 1.5f;     // KWo - 25.02.2008
            pBot->fNoCollTime = gpGlobals->time + 2.0f;   // KWo - 02.01.2010

            // Reached Waypoint is a Camp Waypoint
            if (paths[pBot->curr_wpt_index]->flags & W_FL_CAMP)
            {
               // Check if Bot has got a primary weapon and hasn't camped before
               if (BotHasCampWeapon (pBot) && (pBot->current_weapon.iAmmo1 > 0) && (pBot->fTimeCamping + 10.0 < gpGlobals->time)
                && ((pBot->bot_team != TEAM_CS_COUNTER) || (!g_bBombPlanted)) && (!pBot->bIsChickenOrZombie)) // KWo - 18.01.2011
               {
                  bool bCampingAllowed = TRUE;

                  // Check if it's not allowed for this team to camp here
                  if (pBot->bot_team == TEAM_CS_TERRORIST)
                  {
                     if (paths[pBot->curr_wpt_index]->flags & W_FL_COUNTER)
                        bCampingAllowed = FALSE;
                  }
                  else
                  {
                     if (paths[pBot->curr_wpt_index]->flags & W_FL_TERRORIST)
                        bCampingAllowed = FALSE;
                  }

                  // Check if another Bot is already camping here
                  for (c = 0; c < gpGlobals->maxClients; c++)
                  {
                     pOtherBot = &bots[c];

                     if (pOtherBot->is_used)
                     {
                        if (pOtherBot == pBot)
                           continue;

                        if (!pOtherBot->bDead && (pOtherBot->bot_team == pBot->bot_team)
                            && (pOtherBot->curr_wpt_index == pBot->curr_wpt_index))
                           bCampingAllowed = FALSE;
                     }
                  }

                  if (bCampingAllowed)
                  {
                     // Crouched camping here ?
                     if (paths[pBot->curr_wpt_index]->flags & W_FL_CROUCH)
                        pBot->iCampButtons = IN_DUCK;
                     else
                        pBot->iCampButtons = 0;

                     BotCheckReload (pBot);

                     pBot->fTimeCamping = RANDOM_FLOAT (15.0, 30); // KWo - 23.03.2008
                     if (pBot->fTimeCamping > g_f_cv_maxcamptime)  // KWo - 23.03.2008
                        pBot->fTimeCamping = g_f_cv_maxcamptime;
                     pBot->fTimeCamping += gpGlobals->time;
                     bottask_t TempTask = {NULL, NULL, TASK_CAMP, TASKPRI_CAMP, -1, pBot->fTimeCamping, TRUE};
                     BotPushTask (pBot, &TempTask);

                     MAKE_VECTORS (Vector (paths[pBot->curr_wpt_index]->fcampstartx, paths[pBot->curr_wpt_index]->fcampstarty, 0.0));
                     pBot->vecCamp = paths[pBot->curr_wpt_index]->origin + gpGlobals->v_forward * 500;
                     pBot->iAimFlags |= AIM_CAMP;
                     pBot->iCampDirection = 0;

                     // Tell the world we're camping
                     if ((!g_b_cv_ffa) && (g_b_cv_radio) && (RANDOM_LONG (1, 100) < 20)
                        && (((pBot->bot_team == TEAM_CS_COUNTER) && (g_iAliveCTs > 1))
                         || ((pBot->bot_team == TEAM_CS_TERRORIST) && (g_iAliveTs > 1)))) // KWo - 06.03.2010
                        BotPlayRadioMessage (pBot, RADIO_IMINPOSITION);
                     pBot->bMoveToGoal = FALSE;
                     pBot->bCheckTerrain = FALSE;
                     pBot->f_move_speed = 0.0;
                     pBot->f_sidemove_speed = 0.0;
                  }
               }
            }
            else
            {
               // Some Goal Waypoints are map dependant so check it out...
               if (g_iMapType & MAP_CS)
               {
                  // CT Bot has some stupid hossies following ?
                  if (BotHasHostage (pBot) && (pBot->bot_team == TEAM_CS_COUNTER))
                  {
                     // and reached a Rescue Point ?
                     if ((paths[pBot->curr_wpt_index]->flags & W_FL_RESCUE))
                     {
                        if (pBot->fTimeHostageRescue == 0.0) // KWo - 27.02.2007
                           pBot->fTimeHostageRescue = gpGlobals->time;

                        // Clear Array of Hostage ptrs
                        for (i = 0; i < MAX_HOSTAGES; i++)
                        {
                           if (pBot->pHostages[i]!= NULL)  // KWo 16.06.2006
                           {
                              if ((pBot->pHostages[i]->v.takedamage == 0) || (pBot->fTimeHostageRescue + 5.0 < gpGlobals->time)) // KWo - 27.02.2007
                              {
                                 pBot->pHostages[i] = NULL;
                                 if (pBot->fTimeHostageRescue + 5.0 > gpGlobals->time)
                                    g_bHostageRescued = TRUE; // Notify T Bots that there's a rescue going on
                                 pBot->fTimeHostageRescue = 0.0;
                              }
                           }
                        }
                     }
                  }
               }

               if (g_iMapType & MAP_DE)
               {
                  // Reached Goal Waypoint
                  if (paths[pBot->curr_wpt_index]->flags & W_FL_GOAL)
                  {
                     // Is it a Terrorist carrying the bomb ?
                     if ((pBot->b_bomb_blinking) && (pBot->bot_team == TEAM_CS_TERRORIST)
                        && (pBot->f_bot_see_enemy_time + 3.0 < gpGlobals->time)
                        && (pEdict->v.weapons & (1 << CS_WEAPON_C4)))  // KWo - 25.04.2008
                     {
                        if (pBot->current_weapon.iId != CS_WEAPON_C4)
                           SelectWeaponByName (pBot, "weapon_c4");
                        bottask_t TempTask = {NULL, NULL, TASK_PLANTBOMB, TASKPRI_PLANTBOMB, -1, 0.0, TRUE};   // KWo - 21.02.2007
                        BotPushTask (pBot, &TempTask);

                        if (g_b_DebugTasks || g_b_DebugNavig)
                           ALERT (at_logged, "[DEBUG] BotExecuteTask - Bot %s should start the task PLANT_BOMB.\n", pBot->name);

                        // Tell Teammates to move over here...
                        if ((!g_b_cv_ffa) && (g_b_cv_radio)
                              && (g_iAliveCTs > 0) && (g_iAliveTs > 1)
                              && (RANDOM_LONG (1, 100) < 30)) // KWo - 06.03.2010
                           BotPlayRadioMessage (pBot, RADIO_NEEDBACKUP);
                     }

                     // Counter searching the Bomb ?
                     else if ((pBot->bot_team == TEAM_CS_COUNTER) && g_bBombPlanted)
                     {
                        distance = (int)(pEdict->v.origin - g_vecBomb).Length();
                        if (distance < 150)  // KWo - 05.09.2008
                        {
                           iTempWpIndex1 = WaypointFindNearestToMove (pEdict, g_vecBomb);
                           if ((iTempWpIndex1 >= 0) && (iTempWpIndex1 < g_iNumWaypoints) && (iTempWpIndex1 != pBot->curr_wpt_index))
                           {
                              DeleteSearchNodes(pBot);
                              pBot->chosengoal_index = iTempWpIndex1;
                              bottask_t TempTask = {NULL, NULL, TASK_MOVETOPOSITION, TASKPRI_MOVETOPOSITION + 11.0, iTempWpIndex1, 0.0, TRUE}; // KWo - 02.09.2008
                              BotPushTask (pBot, &TempTask);
                           }
                        }
                        else if (distance > 128) // KWo - 25.06.2008
                        {
                           CTBombPointClear (pBot->curr_wpt_index);
                           if ((!g_b_cv_ffa) && (g_b_cv_radio)
                              && (g_iAliveCTs > 1) && (RANDOM_LONG (1, 100) < 20)) // KWo - 06.03.2010
                              BotPlayRadioMessage (pBot, RADIO_SECTORCLEAR);
                        }
                     }
                  }
               }
            }
            if (paths[pBot->curr_wpt_index]->flags & W_FL_CROUCH)
            {
               if (paths[pBot->curr_wpt_index]->flags & W_FL_CROUCH)
                  pBot->iCampButtons = IN_DUCK;
               else
                  pBot->iCampButtons = 0;
            }
         }
         // No more Nodes to follow - search new ones
         else if (!BotGoalIsValid (pBot))
//          && (BotGetSafeTask(pBot)->iTask != TASK_PLANTBOMB)) // KWo - 12.08.2006
         {

            if (g_b_DebugTasks || g_b_DebugNavig)
            {
               ALERT(at_logged, "[DEBUG] BotExecuteTask - TASK_NORMAL - Bot %s has no valid goal.\n", pBot->name);
            }

            pBot->f_move_speed = 0.0;
            pBot->prev_time = gpGlobals->time + 0.5;     // KWo - 25.02.2008
            pBot->fNoCollTime = gpGlobals->time + 1.0;   // KWo - 02.01.2010

            if ((!g_bPathWasCalculated) && ((g_i_botthink_index == g_iFrameCounter) || (pBot->f_spawn_time + 10.0 < gpGlobals->time))) // KWo - 18.02.2008
            {
               DeleteSearchNodes (pBot);

               // Did we already decide about a goal before ?
               if ((BotGetSafeTask(pBot)->iData != -1) && (!(pEdict->v.weapons & (1 << CS_WEAPON_C4))))
                  iDestIndex = BotGetSafeTask(pBot)->iData;
               else
               {
                  iDestIndex = BotFindGoal (pBot);
//                  pBot->chosengoal_index = iDestIndex;  // KWo - 21.04.2006
               }
               pBot->chosengoal_index = iDestIndex;  // KWo - 18.04.2013

               pBot->prev_goal_index = iDestIndex;

               // Remember Index
               BotGetSafeTask(pBot)->iData = iDestIndex;

               if (g_b_DebugTasks || g_b_DebugNavig)
                  ALERT(at_logged, "[DEBUG] BotExecuteTask - TASK_NORMAL - Bot %s had no valid goal so now it has the new one - %i.\n", pBot->name, iDestIndex);

               if (iDestIndex != pBot->curr_wpt_index)
               {
                  if (g_b_DebugTasks || g_b_DebugNavig)
                     ALERT(at_logged, "[DEBUG] BotExecuteTask - TASK_NORMAL - Bot %s does the pathfinding.\n", pBot->name);
                  // Do Pathfinding if it's not the current waypoint
                  pBot->pWayNodesStart = FindLeastCostPath (pBot, pBot->curr_wpt_index, iDestIndex);
                  pBot->pWaypointNodes = pBot->pWayNodesStart;
               }
            }
            else
            {
               pBot->iAimFlags &= ~AIM_DEST;  // KWo - 18.02.2008
               if (!(pEdict->v.flags & FL_DUCKING)
                  && (pBot->fMinSpeed != 60.0))
                  pBot->f_move_speed = 60.0;
            }
         }
         else if ((g_iMapType & MAP_CS) && (pBot->bot_team == TEAM_CS_COUNTER)
               && (pBot->chosengoal_index >= 0) && (pBot->chosengoal_index < g_iNumWaypoints)
               && (g_i_botthink_index == g_iFrameCounter)) // KWo - 18.02.2008
         {
            bHostAlive = false;
            for (i = 0; i < g_iNumHostages; i++)
            {
               if (!FNullEnt(pBot->pHostages[i]))
               {
                  if ((pBot->pHostages[i]->v.health > 0) && ((pBot->pHostages[i]->v.origin - pEdict->v.origin).Length() < 600))
                  {
                     bHostAlive = true;
                     break;
                  }
               }
            }
            if (paths[pBot->chosengoal_index]->flags & W_FL_RESCUE)
            {
               if (!bHostAlive) // no hostage to rescue (got killed or is far away from us)
               {
                  if (g_b_DebugTasks || g_b_DebugNavig)
                     ALERT(at_logged,"[DEBUG] BotExecuteTask - TASK_NORMAL - Bot %s had some hostages and was going to a rescue point, but they got killed or they are too far away.\n",
                        pBot->name);

                  for (i = 0; i < MAX_HOSTAGES; i++)
                     pBot->pHostages[i] = NULL;

                  DeleteSearchNodes (pBot);
                  BotGetSafeTask(pBot)->iData = -1;
               }
            }
            else if (bHostAlive)  // KWo - 24.12.2006 - bot got some hostage and wasn't trying to rescue him
            {                     // even if it's close to some rescue point...
               min_distance = 800;
               min_index = -1;

               if (g_b_DebugTasks || g_b_DebugNavig)
                  ALERT(at_logged, "[DEBUG] Bot %s has some hostages and he wasn't going to a rescue point.\n", pBot->name);

               for (i = 0; i < g_iNumRescuePoints; i++) // KWo - 24.12.2006
               {
                  iRescueIndex = g_rgiRescueWaypoints[i];
                  distance = (int)(paths[iRescueIndex]->origin - pEdict->v.origin).Length();

                  if (distance < min_distance)
                  {
                     min_distance = distance;
                     min_index = i;
                  }
               }
               if (min_index > -1)
               {
                  if (pBot->chosengoal_index != g_rgiRescueWaypoints[min_index]) // KWo - 27.02.2007
                  {
                     DeleteSearchNodes (pBot);
                     BotGetSafeTask(pBot)->iData = g_rgiRescueWaypoints[min_index];
                     pBot->chosengoal_index = g_rgiRescueWaypoints[min_index];
                  }
               }
            }
         }
         else if ((g_iMapType & MAP_DE) && (pBot->bot_team == TEAM_CS_TERRORIST)
               && (pBot->chosengoal_index >= 0) && (pBot->chosengoal_index < g_iNumWaypoints)
               && (g_i_botthink_index == g_iFrameCounter) && (pEdict->v.weapons & (1 << CS_WEAPON_C4))) // KWo - 18.02.2008
         {
      // checks once a while if any bomb site isn't so close so we can just plant the bomb instead go somewhere else...

            min_distance = 1000; // KWo - 14.04.2008
            min_index = -1;

            if (g_b_DebugTasks || g_b_DebugNavig)
               ALERT(at_logged, "[DEBUG] BotExecuteTask - TASK_NORMAL - Bot %s has a C4 and maybe he wasn't going to plant it.\n", pBot->name);

            index2 = 0;
            for (i = 0; i < g_iNumGoalPoints; i++)
            {
               iGoalIndex = g_rgiGoalWaypoints[i];
               distance = (int)(paths[iGoalIndex]->origin - pEdict->v.origin).Length();
               if ((distance < min_distance) && (!IsGroupOfEnemies(pBot, paths[iGoalIndex]->origin))) // KWo - 14.04.2008
               {
                  if (g_b_DebugTasks || g_b_DebugNavig)
                     ALERT(at_logged, "[DEBUG] BotExecuteTask - TASK_NORMAL - Bot %s is not seeing any group of enemies near the goal WP %d.\n",
                        pBot->name, iGoalIndex);

                  iGoalChoices[index2] = iGoalIndex;
                  min_index = i;
                  index2++;
                  if (index2 > 3)
                     break;
               }
            }
            if (min_index > -1)
            {
               iGoalIndex = iGoalChoices[RANDOM_LONG(0, index2 - 1)];
               if (!(paths[pBot->chosengoal_index]->flags & W_FL_GOAL)
                  || ((paths[pBot->chosengoal_index]->origin - paths[iGoalIndex]->origin).Length() > 800.0))
               {
                  if (g_b_DebugTasks || g_b_DebugNavig)
                     ALERT(at_logged, "[DEBUG] BotExecuteTask - TASK_NORMAL - Bot %s chose the closest bombsite - previous goal %d.\n",
                        pBot->name, pBot->chosengoal_index);

                  DeleteSearchNodes (pBot);
                  BotGetSafeTask(pBot)->iData = iGoalIndex;
                  pBot->chosengoal_index = iGoalIndex;

                  if (g_b_DebugTasks || g_b_DebugNavig)
                     ALERT(at_logged, "[DEBUG] BotExecuteTask - TASK_NORMAL - Bot %s chose the closest bombsite - goal %d.\n", pBot->name, iGoalIndex);
               }
            }
            else if ((paths[pBot->chosengoal_index]->origin - pEdict->v.origin).Length() < 1000.0) // KWo - 14.04.2008
            {
               if (IsGroupOfEnemies(pBot, paths[pBot->chosengoal_index]->origin))
               {
                  if (g_b_DebugTasks || g_b_DebugNavig)
                     ALERT(at_logged, "[DEBUG] BotExecuteTask - TASK_NORMAL - Bot %s is seeing a group of enemies near the closest bombsite.\n", pBot->name);

                  min_index = -1;
                  index2 = 0;
                  min_distance = 1200;
                  for (i = 0; i < g_iNumGoalPoints; i++)
                  {
                     iGoalIndex = g_rgiGoalWaypoints[i];
                     distance = (int)(paths[iGoalIndex]->origin - paths[pBot->chosengoal_index]->origin).Length();
                     if (distance > min_distance)
                     {
                        iGoalChoices[index2] = iGoalIndex;
                        min_index = i;
                        index2++;
                        if (index2 > 3)
                           break;
                     }
                  }
                  if (min_index > -1)
                  {
                     DeleteSearchNodes (pBot);
                     pBot->chosengoal_index = iGoalChoices[RANDOM_LONG(0, index2 - 1)];
                     BotGetSafeTask(pBot)->iData = pBot->chosengoal_index;

                     if (g_b_DebugTasks || g_b_DebugNavig)
                        ALERT(at_logged, "[DEBUG] BotExecuteTask - TASK_NORMAL - Bot %s chose another bombsite.\n", pBot->name);
                  }
               }
            }
         }
         else if ((g_iMapType & MAP_DE) && (pBot->bot_team == TEAM_CS_COUNTER)
               && (pBot->chosengoal_index >= 0) && (pBot->chosengoal_index < g_iNumWaypoints)
               && (g_bBombPlanted) && ((pEdict->v.origin - g_vecBomb).Length() < BOMBMAXHEARDISTANCE)
               && (g_i_botthink_index == g_iFrameCounter)) // KWo - 16.05.2008
         {
         // checks once a while if the bomb position isn't so close so we can see it and go there instead to go somewhere else...
            if (((paths[pBot->chosengoal_index]->origin - g_vecBomb).Length() > 128.0)
               || (fabs(paths[pBot->chosengoal_index]->origin.z - g_vecBomb.z) > 40.0)) // KWo - 05.09.2008
            {
               TRACE_LINE (GetGunPosition (pEdict), g_vecBomb, ignore_monsters, pEdict, &tr);
               if (tr.flFraction >= 0.8)
               {
                  if (g_b_DebugTasks || g_b_DebugNavig)
                     ALERT(at_logged,"[DEBUG] BotExecuteTask - TASK_NORMAL - Bot %s can see the bomb.\n", pBot->name);

                  min_index = -1;
                  min_distance = 60; // KWo - 14.08.2008
                  iGoalIndex = -1;
                  for (i = 0; i < g_iNumGoalPoints; i++)
                  {
                     iGoalIndex = g_rgiGoalWaypoints[i];
                     distance = (int)(paths[iGoalIndex]->origin - g_vecBomb).Length();
                     if ((distance < min_distance) && (fabs(paths[iGoalIndex]->origin.z - g_vecBomb.z) < 40.0)) // KWo - 14.08.2008
                     {
                        min_index = i;
                        min_distance = distance;
                     }
                  }
                  if (min_index > -1)
                     iGoalIndex = g_rgiGoalWaypoints[min_index];
                  else
                     iGoalIndex = WaypointFindNearestToMove(pBot->pEdict, g_vecBomb);

                  if (iGoalIndex > -1)
                  {
                     DeleteSearchNodes (pBot);
                     pBot->chosengoal_index = iGoalIndex;
                     bottask_t TempTask = {NULL, NULL, TASK_MOVETOPOSITION, TASKPRI_MOVETOPOSITION + 11.0, iGoalIndex, 0.0, TRUE}; // KWo - 02.09.2008
                     BotPushTask (pBot, &TempTask);

                     if (g_b_DebugTasks || g_b_DebugNavig)
                        ALERT(at_logged, "[DEBUG] BotExecuteTask - TASK_NORMAL - Bot %s chose another goal.\n", pBot->name);
                  }
               }
            }
         }
         else
         {
            if (!(pEdict->v.flags & FL_DUCKING)
                && (pBot->fMinSpeed != pEdict->v.maxspeed))
            {
               pBot->f_move_speed = pBot->fMinSpeed;
            }
         }
         if ((!FNullEnt(pBot->pBotEnemy)) && BotUsesSniper(pBot)
            && (pBot->f_shoot_time - 0.4 <= gpGlobals->time) && (pBot->iStates & STATE_SEEINGENEMY)) // KWo - 22.10.2006
         {
            flDot1 = GetShootingConeDeviation (pEdict, &pBot->vecEnemy);
            if (flDot1 > 0.95)
            {
               pBot->f_move_speed = 0.0;
               pBot->f_sidemove_speed = 0.0;
               pBot->prev_time = gpGlobals->time + 2.0; // KWo - 25.02.2008
               pBot->fNoCollTime = gpGlobals->time + 2.5;   // KWo - 02.01.2010
               pBot->f_wpt_timeset = gpGlobals->time + 3.0;
            }
         }
         break;
      }
      // Bot sprays messy Logos all over the place...
      case TASK_SPRAYLOGO:
      {
         pBot->iAimFlags |= AIM_ENTITY;
         pBot->bUsingGrenade = FALSE;
         pBot->prev_time = gpGlobals->time + 2.0;     // KWo - 25.02.2008
         pBot->f_moved_distance = 15.0;               // KWo - 25.05.2010
         pBot->fNoCollTime = gpGlobals->time + 2.5;   // KWo - 02.01.2010
         pBot->f_wpt_timeset = gpGlobals->time;       // KWo - 28.04.2013

         if (g_b_DebugTasks)
         {
            ALERT(at_logged, "[DEBUG] BotExecuteTask - TASK_SPRAYLOGO - Bot %s - taskend time = %.1f, global time = %.1f.\n",
               pBot->name, BotGetSafeTask(pBot)->fTime, gpGlobals->time);
         }

         // Bot didn't spray this round ?
         if (!pBot->bLogoSprayed && (BotGetSafeTask(pBot)->fTime > gpGlobals->time))
         {
            v_angles = pEdict->v.v_angle;
            MAKE_VECTORS (v_angles);
            vecSprayPos = GetGunPosition (pEdict) + gpGlobals->v_forward * 128;
            TRACE_LINE (GetGunPosition (pEdict), vecSprayPos, ignore_monsters, pEdict, &tr);

            // No Wall in Front ?
            if (tr.flFraction >= 1.0)
               vecSprayPos.z -= 128.0;
            pBot->vecEntity = vecSprayPos;

            if (BotGetSafeTask(pBot)->fTime - 0.5 < gpGlobals->time)
            {
               // Emit Spraycan sound
               EMIT_SOUND_DYN2 (pEdict, CHAN_VOICE, "player/sprayer.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
               TRACE_LINE (GetGunPosition (pEdict), GetGunPosition (pEdict) + gpGlobals->v_forward * 128, ignore_monsters, pEdict, &tr);

               // Paint the actual Logo Decal
               UTIL_DecalTrace (&tr, &szSprayNames[pBot->iSprayLogo][0]);
               pBot->bLogoSprayed = TRUE;
            }
         }
         else if (BotGetSafeTask(pBot)->iTask == TASK_SPRAYLOGO) // KWo - 02.09.2006 - against BotRemoveCertainTask
         {
            BotTaskComplete(pBot);
            pBot->bMoveToGoal = FALSE;
            pBot->bCheckTerrain = FALSE;
            pBot->f_wpt_timeset = gpGlobals->time;
            pBot->f_move_speed = 0.0;
            pBot->f_sidemove_speed = 0.0;
            pBot->iAimFlags &= ~AIM_ENTITY;
            pBot->fChangeAimDirectionTime = gpGlobals->time - 0.6; // KWo - 30.12.2009
         }
         break;
      }
      // Hunt down Enemy
      case TASK_ENEMYHUNT:
      {
         pBot->iAimFlags |= AIM_DEST;
         pBot->bCheckTerrain = TRUE;
         BotCheckReload (pBot);
         pBot->bUsingGrenade = FALSE;

         if (g_b_DebugTasks || g_b_DebugCombat)
            ALERT(at_logged, "[DEBUG] BotExecuteTask - TASK_ENEMYHUNT - Bot %s, time = %.2f.\n", pBot->name, gpGlobals->time);

         // seen the new enemy
         if ((!FNullEnt (pBot->pHuntEnemy) && (pBot->f_bot_see_enemy_time + 0.5 > gpGlobals->time)
            || FNullEnt (pBot->pHuntEnemy)) && (pBot->current_weapon.iId != CS_WEAPON_KNIFE))
         {
            // Forget about it...
            if (BotGetSafeTask(pBot)->iTask == TASK_ENEMYHUNT) // KWo - 02.09.2006 - against BotRemoveCertainTask
            {
               pBot->prev_goal_index = BotGetSafeTask(pBot)->iData; // KWo - 18.02.2008
               BotGetSafeTask(pBot)->iData = -1;
               BotTaskComplete(pBot);
               pBot->chosengoal_index = -1;
            }
            bEndTask = true;

            if (g_b_DebugTasks || g_b_DebugCombat)
               ALERT(at_logged, "[DEBUG] BotExecuteTask - TASK_ENEMYHUNT - Bot %s found a visible enemy or has no last enemy - finishing hunting.\n", pBot->name);
         }
         // Reached Hunt Enemy Pos ?
         else if (BotDoWaypointNav (pBot))
         {
            // Forget about it...
            if (BotGetSafeTask(pBot)->iTask == TASK_ENEMYHUNT) // KWo - 02.09.2006 - against BotRemoveCertainTask
            {
               pBot->prev_goal_index = BotGetSafeTask(pBot)->iData; // KWo - 18.02.2008
               BotGetSafeTask(pBot)->iData = -1;
               BotTaskComplete(pBot);
               pBot->chosengoal_index = -1;
               pBot->prev_time = gpGlobals->time + 1.5; // KWo - 25.02.2008
               pBot->fNoCollTime = gpGlobals->time + 2.0;   // KWo - 02.01.2010

               if (g_b_DebugTasks || g_b_DebugCombat || g_b_DebugNavig)
                  ALERT(at_logged, "[DEBUG] BotExecuteTask - TASK_ENEMYHUNT - Bot %s has reached the destination hunting enemy's position - finishing hunting.\n", pBot->name);
            }
            bEndTask = true;
         }
         else if (!FNullEnt (pBot->pHuntEnemy)) // KWo - 05.08.2006
         {
            bNewPosition = false; // KWo 16.06.2008
            iGoalIndex = BotGetSafeTask(pBot)->iData; // KWo 16.06.2008

            if ((iGoalIndex >= 0) && (iGoalIndex < g_iNumWaypoints) && IsAlive(pBot->pHuntEnemy)) // KWo - 08.04.2010
            {
               if ((paths[iGoalIndex]->origin - pBot->vecHuntEnemyOrigin).Length() > 300.0)
               {
                  if (pBot->iStates & STATE_SEEINGENEMY)
                     bNewPosition = true;
                  else
                  {
                     DeleteSearchNodes(pBot);
                     pBot->chosengoal_index = -1;
                     pBot->prev_goal_index = -1;
                  }
               }
            }

            if ((!IsAlive(pBot->pHuntEnemy)) || ((pEdict->v.origin - pBot->vecHuntEnemyOrigin).Length() < 114.0)
               || (bNewPosition)) // KWo - 16.06.2008
            {
               if (BotGetSafeTask(pBot)->iTask == TASK_ENEMYHUNT) // KWo - 02.09.2006 - against BotRemoveCertainTask
               {
                  pBot->prev_goal_index = BotGetSafeTask(pBot)->iData; // KWo - 18.02.2008
                  BotGetSafeTask(pBot)->iData = -1;
                  pBot->chosengoal_index = -1;
                  BotTaskComplete(pBot);
               }
               bEndTask = true;
            }
         }

         // Do we need to calculate a new Path ?
         if ((pBot->pWaypointNodes == NULL) && (!bEndTask) && (g_iDebugGoalIndex == -1)
            /* && FNullEnt (pBot->pBotEnemy) */ && !FNullEnt (pBot->pHuntEnemy) && (pBot->vecHuntEnemyOrigin != g_vecZero))
         {
            DeleteSearchNodes (pBot);
            iTempDestIndex = BotGetSafeTask(pBot)->iData;
            // Is there a remembered Index ?
            if ((iTempDestIndex >= 0) && (iTempDestIndex < g_iNumWaypoints))
               iDestIndex = iTempDestIndex;
            // No. We need to find a new one
            else
            {
               iDestIndex = WaypointFindNearestToMove(pBot->pHuntEnemy, pBot->vecHuntEnemyOrigin); // KWo - 17.04.2008
            }

            // Remember Index
            pBot->prev_goal_index = iDestIndex;
            BotGetSafeTask(pBot)->iData = iDestIndex;
            pBot->chosengoal_index = iDestIndex;  // KWo - 21.04.2006
            if ((pBot->curr_wpt_index < 0) || (pBot->curr_wpt_index >= g_iNumWaypoints))
            {
               GetValidWaypoint (pBot);

               if (g_b_DebugTasks || g_b_DebugCombat)
                  ALERT(at_logged, "[DEBUG] BotExecuteTask - TASK_ENEMYHUNT - Bot %s was needing to find the current waypoint while hunting - nr %i.\n",
                     pBot->name, pBot->curr_wpt_index);
            }
            pBot->pWayNodesStart = FindLeastCostPath (pBot, pBot->curr_wpt_index, iDestIndex);
            pBot->pWaypointNodes = pBot->pWayNodesStart;
            pBot->prev_time = gpGlobals->time + 1.5; // KWo - 25.02.2008
            pBot->fNoCollTime = gpGlobals->time + 2.0;   // KWo - 02.01.2010

            if (g_b_DebugTasks || g_b_DebugNavig)
            {
               ALERT(at_logged, "[DEBUG] BotExecuteTask - TASK_ENEMYHUNT - Bot %s needed to delete path nodes and now will be heading for WP nr %i.\n",
                  pBot->name, iDestIndex);
            }
         }

         // Bots skill higher than 60 ?
         if ((pBot->bot_skill > 60) && (!FNullEnt (pBot->pHuntEnemy)) && (!bEndTask))
         {
            if ((pBot->bot_skill > 80) && (pBot->bIsChickenOrZombie))
            {
               if ((pBot->f_jumptime + 5.0 < gpGlobals->time) && (pEdict->v.flags & FL_ONGROUND)
                  && ((pEdict->v.origin - pBot->vecHuntEnemyOrigin).Length() < 2 * MIN_BURST_DISTANCE)
                  && (pBot->f_ducktime < gpGlobals->time))
               {
                  fEnemyDot = GetShootingConeDeviation (pBot->pHuntEnemy, &pEdict->v.origin);
                  if ((RANDOM_LONG (1, 1000) < 200) && (pEdict->v.velocity.Length2D () > 100)
                     && (fEnemyDot > 0.95) && (pBot->iStates & STATE_SEEINGENEMY))
                  {
                     pEdict->v.button |= IN_JUMP;
                     pBot->f_jumptime = gpGlobals->time + 1.0;
                  }
               }
            }

            // Then make him move slow if near Enemy
            if (!(pBot->curr_travel_flags & C_FL_JUMP) && (pBot->f_jumptime + 2.0 < gpGlobals->time))
            {
               if ((pBot->curr_wpt_index >= 0) && (pBot->curr_wpt_index < g_iNumWaypoints))
               {
                  if ((paths[pBot->curr_wpt_index]->Radius < 32) && !pBot->bOnLadder
                     && !pBot->bInWater && (pBot->f_bot_see_enemy_time + 4.f > gpGlobals->time)
                     && (pBot->bot_skill < 80))
                  {
                     pEdict->v.button |= IN_DUCK;
                     pBot->f_ducktime = gpGlobals->time + 1.0;
                  }
               }

               v_diff = pBot->vecHuntEnemyOrigin - pEdict->v.origin;

               if ((v_diff.Length () < 512.f) && !(pEdict->v.flags & FL_DUCKING))
                  pBot->f_move_speed = pEdict->v.maxspeed * 0.5;
            }
         }

         if (bEndTask)
         {
            pBot->prev_time = gpGlobals->time + 1.5; // KWo - 25.02.2008
            pBot->fNoCollTime = gpGlobals->time + 2.0;   // KWo - 02.01.2010
            pBot->vecHuntEnemyOrigin = g_vecZero;
            pBot->pHuntEnemy = NULL;
         }
         break;
      }
      // Bot seeks Cover from Enemy
      case TASK_SEEKCOVER:
      {
         pBot->iAimFlags |= AIM_DEST;
         pBot->bUsingGrenade = FALSE;

         if (g_b_DebugCombat)
         {
            if (pBot->bIsReloading)
               ALERT(at_logged, "[DEBUG] BotExecuteTask - TASK_SEEKCOVER - Bot %s is reloading a weapon.\n", pBot->name);
         }

         if ((FNullEnt (pBot->pLastEnemy)) || (pBot->vecLastEnemyOrigin == g_vecZero)) // KWo - 12.04.2010
         {
            if (BotGetSafeTask(pBot)->iTask == TASK_SEEKCOVER) // KWo - 02.09.2006 - against BotRemoveCertainTask
            {
               pBot->prev_goal_index = BotGetSafeTask(pBot)->iData; // KWo - 18.01.2008
               BotTaskComplete(pBot);
               if (paths[pBot->curr_wpt_index]->flags & W_FL_CROUCH)
                  pBot->iCampButtons = IN_DUCK;
               else
                  pBot->iCampButtons = 0;
            }
            bEndTask = true;
         }
         else if (!IsAlive(pBot->pLastEnemy))
         {
            if (BotGetSafeTask(pBot)->iTask == TASK_SEEKCOVER) // KWo - 02.09.2006 - against BotRemoveCertainTask
            {
               pBot->prev_goal_index = BotGetSafeTask(pBot)->iData; // KWo - 18.01.2008
               BotTaskComplete(pBot);
               if (paths[pBot->curr_wpt_index]->flags & W_FL_CROUCH)
                  pBot->iCampButtons = IN_DUCK;
               else
                  pBot->iCampButtons = 0;
            }
            bEndTask = true;
         }

         if (!bEndTask)
         {
         // Reached final Cover waypoint ?
            if (BotDoWaypointNav (pBot))
            {
               pBot->prev_time = gpGlobals->time + 1.5; // KWo - 25.02.2008
               pBot->fNoCollTime = gpGlobals->time + 2.0;   // KWo - 02.01.2010

               // Yep. Activate Hide Behaviour
               if (BotGetSafeTask(pBot)->iTask == TASK_SEEKCOVER) // KWo - 02.09.2006 - against BotRemoveCertainTask
               {
                  pBot->prev_goal_index = BotGetSafeTask(pBot)->iData; // KWo - 18.01.2008
                  BotTaskComplete(pBot);
                  bottask_t TempTask = {NULL, NULL, TASK_HIDE, TASKPRI_HIDE, -1, gpGlobals->time + RANDOM_FLOAT (4.0, 10.0), FALSE};
                  BotPushTask (pBot, &TempTask);

//                if (!BotHasPrimaryWeapon (pBot) || (pBot->current_weapon.iAmmo1 == 0)) // KWo - 10.02.2008
//                   break;

                  v_dest = pBot->vecLastEnemyOrigin;
/*
                  iWpIndex = WaypointFindNearestToMove(pEdict, v_dest); // KWo - 11.04.2010

                  if (iWpIndex == -1) // KWo - 11.04.2010
                  {
                     for (i = 0; i < g_iNumWaypoints; i++)
                     {
                        distance = (int)(paths[i]->origin - pEdict->v.origin).Length ();
                        if ((distance > 200) && (distance < 500))
                        {
                           if (FVisible (paths[i]->origin, pEdict))
                              break;
                        }
                     }

                     if (i < g_iNumWaypoints)
                        v_dest = paths[i]->origin;
                     else if ((pBot->prev_wpt_index[0] >= 0) && (pBot->prev_wpt_index[0] < g_iNumWaypoints))
                        v_dest = paths[pBot->prev_wpt_index[0]]->origin;
                     else
                        v_dest = pBot->vecLookAt; // last chance
                  }
                  else
                     v_dest = paths[iWpIndex]->origin;
*/

                  // Get a valid look direction
                  BotGetCampDirection (pBot, &v_dest);
                  pBot->iAimFlags |= AIM_CAMP;
                  pBot->vecCamp = v_dest + pEdict->v.view_ofs;
                  pBot->iCampDirection = 0;

                  // Chosen Waypoint is a Camp Waypoint ?
                  if (paths[pBot->curr_wpt_index]->flags & W_FL_CAMP)
                  {
                     // Use the existing camp wpt prefs
                     if (paths[pBot->curr_wpt_index]->flags & W_FL_CROUCH)
                        pBot->iCampButtons = IN_DUCK;
                     else
                        pBot->iCampButtons = 0;

                     bottask_t TempTask = {NULL, NULL, TASK_CAMP, TASKPRI_CAMP, -1, gpGlobals->time + RANDOM_FLOAT (15.0, ((20.0 > g_f_cv_maxcamptime) ? g_f_cv_maxcamptime : 20)), FALSE}; // KWo - 18.01.2008
                     BotPushTask (pBot, &TempTask); // KWo - 18.01.2008
                  }
                  else
                  {
                     // Choose a crouch or stand pos
                     if ((RANDOM_LONG (1, 100) < 30) && (paths[pBot->curr_wpt_index]->Radius < 32))
                        pBot->iCampButtons = IN_DUCK;
                     else
                        pBot->iCampButtons = 0;

                  }

                  BotCheckReload (pBot);

                  pBot->f_wpt_timeset = gpGlobals->time;
                  pBot->f_move_speed = 0.0;
                  pBot->f_sidemove_speed = 0.0;
                  pBot->bMoveToGoal = FALSE;
                  pBot->bCheckTerrain = FALSE; // KWo - 18.01.2008
               }
            }
         // We didn't choose a Cover Waypoint yet or lost it due to an attack ?
            else if (!BotGoalIsValid (pBot))
            {
               if (g_b_DebugCombat)
               {
                  if (pBot->bIsReloading)
                     ALERT(at_logged, "[DEBUG] BotExecuteTask - TASK_SEEKCOVER - Bot %s is reloading a weapon and is searching for the goal.\n", pBot->name);
               }

               DeleteSearchNodes (pBot);
               pBot->prev_time = gpGlobals->time + 0.5; // KWo - 25.02.2008
               pBot->fNoCollTime = gpGlobals->time + 1.0;   // KWo - 02.01.2010

               if (BotGetSafeTask(pBot)->iData != -1)
                  iDestIndex = BotGetSafeTask(pBot)->iData;
               else
               {
                  iDestIndex = BotFindCoverWaypoint (pBot, 1024);
                  if (iDestIndex == -1)
                     iDestIndex = RANDOM_LONG (0, g_iNumWaypoints - 1);
               }

               pBot->iCampDirection = 0;
               pBot->iCampButtons = 0;
               pBot->f_ducktime = 0.0; // KWo - 24.01.2012
               pBot->prev_goal_index = iDestIndex;
               BotGetSafeTask(pBot)->iData = iDestIndex;
               pBot->chosengoal_index = iDestIndex;

               if (iDestIndex != pBot->curr_wpt_index)
               {
                  pBot->pWayNodesStart = FindLeastCostPath (pBot, pBot->curr_wpt_index, iDestIndex);
                  pBot->pWaypointNodes = pBot->pWayNodesStart;
               }
            }

            if ((!FNullEnt(pBot->pBotEnemy)) && BotUsesSniper(pBot) && !(pBot->bIsReloading)
               && (pBot->f_shoot_time - 0.4 <= gpGlobals->time) && (pBot->iStates & STATE_SEEINGENEMY)) // KWo - 18.02.2008
            {
               flDot1 = GetShootingConeDeviation (pEdict, &pBot->vecEnemy);
               if (flDot1 > 0.95)
               {
                  pBot->f_move_speed = 0.0;
                  pBot->f_sidemove_speed = 0.0;
                  pBot->prev_time = gpGlobals->time + 1.5; // KWo - 25.02.2008
                  pBot->fNoCollTime = gpGlobals->time + 2.0;   // KWo - 02.01.2010
               }
            }
         }
         else
         {
            pBot->prev_time = gpGlobals->time + 1.5; // KWo - 25.02.2008
            pBot->fNoCollTime = gpGlobals->time + 2.0;   // KWo - 02.01.2010
         }
         break;
      }
      // Plain Attacking
      case TASK_ATTACK:
      {
         pBot->bMoveToGoal = FALSE;
         pBot->bCheckTerrain = TRUE; // KWo - 24.02.2008
         pBot->bUsingGrenade = FALSE;

         if (!FNullEnt (pBot->pBotEnemy) && (pBot->f_bot_see_enemy_time + 1.0 > gpGlobals->time))
         {
            DeleteSearchNodes (pBot);
            if (pBot->cCollisionState == COLLISION_NOTDECIDED) // KWo - 20.05.2008
            {
               BotDoAttackMovement (pBot);
               if ((pBot->current_weapon.iId == CS_WEAPON_KNIFE) && (pBot->vecLastEnemyOrigin != g_vecZero)) // KWo - 20.05.2008
                  pBot->dest_origin = pBot->vecLastEnemyOrigin;
               if (pBot->byFightStyle == 1)              // KWo - 25.02.2008
               {
                  pBot->prev_time = gpGlobals->time + 2.0;
                  pBot->f_moved_distance = 15.0;               // KWo - 25.05.2010
                  pBot->fNoCollTime = gpGlobals->time + 2.5;   // KWo - 02.01.2010
               }
            }
         }
         else
         {
            if (BotGetSafeTask(pBot)->iTask == TASK_ATTACK) // KWo - 02.09.2006 - against BotRemoveCertainTask
            {
               BotTaskComplete(pBot);
               BotFindWaypoint (pBot);
               pBot->prev_time = gpGlobals->time + 2.0;     // KWo - 25.02.2008
               pBot->f_moved_distance = 15.0;               // KWo - 25.05.2010
               pBot->fNoCollTime = gpGlobals->time + 2.5;   // KWo - 02.01.2010
               if (pBot->vecLastEnemyOrigin != g_vecZero)   // KWo - 17.02.2008
                  pBot->chosengoal_index = WaypointFindNearestToMove(pEdict, pBot->vecLastEnemyOrigin); // KWo - 20.05.2008
//                pBot->dest_origin = pBot->vecLastEnemyOrigin;
            }
         }

         pBot->f_wpt_timeset = gpGlobals->time;
         break;
      }
      // Bot is pausing
      case TASK_PAUSE:
      {
         pBot->bMoveToGoal = FALSE;
         pBot->bCheckTerrain = FALSE;
         pBot->f_wpt_timeset = gpGlobals->time;
         pBot->f_move_speed = 0.0;
         pBot->f_sidemove_speed = 0.0;
         pBot->iAimFlags |= AIM_DEST;
         pBot->bUsingGrenade = FALSE;
         pBot->prev_time = gpGlobals->time + 2.0;     // KWo - 25.02.2008
         pBot->f_moved_distance = 15.0;               // KWo - 25.05.2010
         pBot->fNoCollTime = gpGlobals->time + 2.5;   // KWo - 02.01.2010

         // Is Bot blinded and above average skill ?
         if ((pBot->f_view_distance < 500.0) && (pBot->bot_skill > 60))
         {
            // Go mad !
            pBot->f_move_speed = -fabs ((pBot->f_view_distance - 500.0) * 0.5);
            if (pBot->f_move_speed < -pEdict->v.maxspeed)
               pBot->f_move_speed = -pEdict->v.maxspeed;

            v_angles = pEdict->v.v_angle;
            MAKE_VECTORS (v_angles);
            pBot->vecCamp = GetGunPosition (pEdict) + gpGlobals->v_forward * 500;
            pBot->iAimFlags |= AIM_OVERRIDE;
            pBot->bWantsToFire = TRUE;
         }
         else
         {
            pEdict->v.button |= pBot->iCampButtons;
            if (pBot->iCampButtons & IN_DUCK)         // KWo - 24.01.2012
               pBot->f_ducktime = gpGlobals->time + 1.0;
         }

         // Stop camping if Time over or gets Hurt by something else than bullets
         if (((BotGetSafeTask(pBot)->fTime < gpGlobals->time) || (pBot->iLastDamageType > 0)
            || ((g_iDebugGoalIndex != -1) && (BotGetSafeTask(pBot)->fDesire != TASKPRI_DEFUSEBOMB)))
               && (BotGetSafeTask(pBot)->iTask == TASK_PAUSE)) // KWo - 02.09.2006 - against BotRemoveCertainTask
            BotTaskComplete(pBot);
         break;
      }
      // Blinded (flashbanged) Behaviour
      case TASK_BLINDED:
      {
         pBot->bMoveToGoal = FALSE;
         pBot->bCheckTerrain = FALSE;
         pBot->f_wpt_timeset = gpGlobals->time;
         pBot->bUsingGrenade = FALSE;
         pBot->prev_time = gpGlobals->time + 2.0;     // KWo - 25.02.2008
         pBot->f_moved_distance = 15.0;               // KWo - 25.05.2010
         pBot->fNoCollTime = gpGlobals->time + 2.5;   // KWo - 02.01.2010

         switch (pBot->bot_personality)
         {
            // Normal
            case 0:
            {
               if (g_i_botthink_index == g_iFrameCounter)  // KWo - 13.08.2008
               {
                  if ((pBot->bot_skill > 60) && (pBot->vecLastEnemyOrigin != g_vecZero)
                  && (RANDOM_LONG(0,100) > 50)) // KWo - 31.03.2006
                     pBot->bShootLastPosition = TRUE;
                  else
                     pBot->bShootLastPosition = FALSE;
               }
               pBot->bMadShoot = FALSE;
               break;
            }
            // Psycho
            case 1:
            {
               if ((g_i_botthink_index == g_iFrameCounter) && (pBot->vecLastEnemyOrigin != g_vecZero))  // KWo - 13.08.2008
               {
                  if (RANDOM_LONG(0,100) > 30)
                  {
                     pBot->bShootLastPosition = TRUE;
                     pBot->bMadShoot = FALSE;
                  }
                  else
                  {
                     pBot->bMadShoot = TRUE;
                     pBot->bShootLastPosition = FALSE;
                  }
               }
               else if (pBot->vecLastEnemyOrigin == g_vecZero)
               {
                  pBot->bShootLastPosition = FALSE;
                  pBot->bMadShoot = FALSE;
               }
               break;
            }
            // Coward
            case 2:
            {
               pBot->bShootLastPosition = FALSE;
               pBot->bMadShoot = FALSE;
               break;
            }
         }

         v_angles = pEdict->v.v_angle;
         MAKE_VECTORS (v_angles);
         pBot->vecLookAt = GetGunPosition (pEdict) + gpGlobals->v_forward * 500;

         // If Bot remembers last Enemy Position
         if ((pBot->bShootLastPosition) && (pBot->vecLastEnemyOrigin != g_vecZero))
         {
            // Face it and shoot
            pBot->vecLookAt = pBot->vecLastEnemyOrigin;
            pBot->bWantsToFire = TRUE;
         }
         // If Bot is mad
         else if (pBot->bMadShoot)
         {
            // Just shoot in forward direction
            pBot->bWantsToFire = TRUE;
         }

         pBot->f_move_speed = pBot->f_blindmovespeed_forward;
         pBot->f_sidemove_speed = pBot->f_blindmovespeed_side;

         if ((pBot->f_blind_time < gpGlobals->time) && (BotGetSafeTask(pBot)->iTask == TASK_BLINDED)) // KWo - 02.09.2006 - against BotRemoveCertainTask
         {
            pBot->bShootLastPosition = FALSE;
            pBot->bMadShoot = FALSE;
            BotTaskComplete(pBot);
         }
         break;
      }
      // Camping Behaviour
      case TASK_CAMP:
      {
         pBot->iAimFlags |= AIM_CAMP;
         pBot->iAimFlags &= ~AIM_ENTITY; // KWo - 09.02.2008
         pBot->bCheckTerrain = FALSE;
         pBot->bMoveToGoal = FALSE;
         pBot->bUsingGrenade = FALSE;
         pBot->prev_time = gpGlobals->time + 2.0;     // KWo - 25.02.2008
         pBot->f_moved_distance = 15.0;               // KWo - 25.05.2010
         pBot->fNoCollTime = gpGlobals->time + 2.5;   // KWo - 02.01.2010

         // half the reaction time if camping because you're more aware of enemies if camping
         pBot->f_ideal_reaction_time = RANDOM_FLOAT (BotSkillDelays[pBot->bot_skill / 20].fMinSurpriseDelay,
                                                  BotSkillDelays[pBot->bot_skill / 20].fMaxSurpriseDelay) / 2;
         pBot->f_wpt_timeset = gpGlobals->time;
         pBot->f_move_speed = 0.0;
         pBot->f_sidemove_speed = 0.0;
         GetValidWaypoint (pBot);

         if (g_b_DebugTasks)
            ALERT(at_logged,"[DEBUG] ExecuteTask - bot %s is camping; time = %.2f.\n", pBot->name, gpGlobals->time);

         if ((BotHasPrimaryWeapon (pBot)) && (!WeaponIsPrimaryGun(pBot->current_weapon.iId))) // KWo - 27.02.2007
            BotSelectBestWeapon(pBot);
         else if ((!BotHasPrimaryWeapon (pBot) || (pBot->current_weapon.iAmmo1 == 0) || (pBot->bIsChickenOrZombie))
               && (!pBot->bDefendedBomb) && (BotGetSafeTask(pBot)->iTask == TASK_CAMP)) // KWo - 18.01.2011
         {
            BotTaskComplete(pBot);
            break;
         }

         if ((pBot->fNextCampDirectionTime + 15.0 < gpGlobals->time)
            && (pBot->bIsLeader) && (g_b_cv_radio)) // KWo - 13.04.2010
         {
            for (i = 0; i < gpGlobals->maxClients; i++)
            {
               if (!bots[i].is_used || bots[i].bDead || (i == g_i_botthink_index)
                  || (bots[i].bot_team != pBot->bot_team) || (bots[i].pBotUser == NULL))
                  continue;
               if (bots[i].pBotUser == pEdict)
               {
                  BotPlayRadioMessage (pBot, RADIO_HOLDPOSITION);
                  break;
               }
            }
         }

         if (pBot->fNextCampDirectionTime < gpGlobals->time)
         {
            v_dest = g_vecZero;  // KWo - 20.07.2007
            if (paths[pBot->curr_wpt_index]->flags & W_FL_CAMP)
            {
               if (pBot->iCampDirection < 1)
               {
                  v_dest.x = paths[pBot->curr_wpt_index]->fcampstartx;
                  v_dest.y = paths[pBot->curr_wpt_index]->fcampstarty;
                  v_dest.z = 0.0;
                  pBot->iCampDirection = 1; // KWo - 27.02.2007
               }
               else
               {
                  v_dest.x = paths[pBot->curr_wpt_index]->fcampendx;
                  v_dest.y = paths[pBot->curr_wpt_index]->fcampendy;
                  v_dest.z = 0.0;
                  pBot->iCampDirection = 0; // KWo - 27.02.2007
               }
            }
            if (v_dest == g_vecZero)
            {
               iWpIndex = 0;          // KWo - 20.07.2007
               iDangerWpIndex = -1;   // KWo - 17.02.2008
               iDangerMax = 0;        // KWo - 17.02.2008
               iTempWpIndex1 = -1;    // KWo - 17.02.2008
               iTempWpIndex2 = -1;    // KWo - 17.02.2008
               iTempDanger = 0;       // KWo - 17.02.2008

               // KWo - 17.02.2008 first try to find the most danger visible WP for this position
               if (pBot->bot_team == TEAM_CS_TERRORIST)
               {
                  iDangerWpIndex = (pBotExperienceData + (pBot->curr_wpt_index * g_iNumWaypoints) + pBot->curr_wpt_index)->iTeam0_danger_index;
                  if ((iDangerWpIndex >= 0) && (iDangerWpIndex < g_iNumWaypoints))
                     iDangerMax = (pBotExperienceData + (pBot->curr_wpt_index * g_iNumWaypoints) + iDangerWpIndex)->uTeam0Damage;
               }
               else
               {
                  iDangerWpIndex = (pBotExperienceData + (pBot->curr_wpt_index * g_iNumWaypoints) + pBot->curr_wpt_index)->iTeam1_danger_index;
                  if ((iDangerWpIndex >= 0) && (iDangerWpIndex < g_iNumWaypoints))
                     iDangerMax = (pBotExperienceData + (pBot->curr_wpt_index * g_iNumWaypoints) + iDangerWpIndex)->uTeam1Damage;
               }
               if ((iDangerWpIndex >= 0) && (iDangerWpIndex < g_iNumWaypoints))
               {
                  if (!WaypointIsVisible (pBot->curr_wpt_index, iDangerWpIndex))
                  {
                     iDangerWpIndex = -1;
                     iDangerMax = 0;
                  }
               }


               for (iPathIndex = 0; iPathIndex < MAX_PATH_INDEX; iPathIndex++)
               {
                  iTempWpIndex2 = paths[pBot->curr_wpt_index]->index[iPathIndex];
                  if (iTempWpIndex2 == -1)
                     continue;

                  iTempDanger = 0;
                  iTempWpIndex1 = -1;

                  if (pBot->bot_team == TEAM_CS_TERRORIST)
                  {
                     iTempWpIndex1 = (pBotExperienceData + (iTempWpIndex2 * g_iNumWaypoints) + iTempWpIndex2)->iTeam0_danger_index;
                     if ((iTempWpIndex1 >= 0) && (iTempWpIndex1 < g_iNumWaypoints))
                        iTempDanger = (pBotExperienceData + (pBot->curr_wpt_index * g_iNumWaypoints) + iTempWpIndex1)->uTeam0Damage;
                  }
                  else
                  {
                     iTempWpIndex1 = (pBotExperienceData + (iTempWpIndex2 * g_iNumWaypoints) + iTempWpIndex2)->iTeam1_danger_index;
                     if ((iTempWpIndex1 >= 0) && (iTempWpIndex1 < g_iNumWaypoints))
                        iTempDanger = (pBotExperienceData + (pBot->curr_wpt_index * g_iNumWaypoints) + iTempWpIndex1)->uTeam1Damage;
                  }
                  if ((iTempWpIndex1 >= 0) && (iTempWpIndex1 < g_iNumWaypoints))
                  {
                     if (!WaypointIsVisible (pBot->curr_wpt_index, iTempWpIndex1))
                     {
                        iTempDanger = 0;
                        iTempWpIndex1 = -1;
                     }
                  }
                  if (iTempDanger > iDangerMax)
                     iDangerWpIndex = iTempWpIndex1;
               }

               iWpIndex = iDangerWpIndex;
               iLoopIndex = 0;        // KWo - 20.07.2007
               while ((iDangerWpIndex < 0) && (iLoopIndex < g_iNumWaypoints))  // KWo - 17.02.2008
               {
                  iWpIndex = RANDOM_LONG(0, g_iNumWaypoints - 1); // KWo - 20.07.2007
                  distance = (int)(paths[iWpIndex]->origin - pEdict->v.origin).Length ();

                  if ((distance > 200) && (distance < 800) && WaypointIsVisible (pBot->curr_wpt_index, iWpIndex)
                     && (pBot->curr_wpt_index != iWpIndex)) // KWo - 13.10.2011 - should work faster (no need a lot of tracelines)
                     break;
                  iLoopIndex++;
               }

               if ((iWpIndex < g_iNumWaypoints) && (iWpIndex >= 0)) // KWo - 17.02.2008
               {
                  v_dest = UTIL_VecToAngles (paths[iWpIndex]->origin - pEdict->v.origin);
                  v_dest.x = -v_dest.x; // KWo - 17.02.2008
               }
               else // KWo - 20.01.2012
               {
//                  v_dest = Vector (0.0, RANDOM_FLOAT (-180, 180), 0.0); // KWo - 19.01.2012 - last chance
                  index2 = 1;
                  v_dest = GetGunPosition (pEdict) + Vector (1414.0, 1414.0, 0.0); // trace the distance 2000.0 units
                  TRACE_LINE (GetGunPosition (pEdict), v_dest, ignore_monsters, pEdict, &tr);
                  fDistanceTraced = tr.flFraction;
                  v_dest = GetGunPosition (pEdict) + Vector (-1414.0, 1414.0, 0.0); // trace the distance 2000.0 units
                  TRACE_LINE (GetGunPosition (pEdict), v_dest, ignore_monsters, pEdict, &tr);
                  if (tr.flFraction > fDistanceTraced)
                  {
                     index2 = 2;
                     fDistanceTraced = tr.flFraction;
                  }
                  v_dest = GetGunPosition (pEdict) + Vector (-1414.0, -1414.0, 0.0); // trace the distance 2000.0 units
                  TRACE_LINE (GetGunPosition (pEdict), v_dest, ignore_monsters, pEdict, &tr);
                  if (tr.flFraction > fDistanceTraced)
                  {
                     index2 = 3;
                     fDistanceTraced = tr.flFraction;
                  }
                  v_dest = GetGunPosition (pEdict) + Vector (1414.0, -1414.0, 0.0); // trace the distance 2000.0 units
                  TRACE_LINE (GetGunPosition (pEdict), v_dest, ignore_monsters, pEdict, &tr);
                  if (tr.flFraction > fDistanceTraced)
                  {
                     index2 = 4;
                     fDistanceTraced = tr.flFraction;
                  }
                  switch (index2)
                  {
                     case 1:
                        v_dest = UTIL_VecToAngles(Vector (1414.0, 1414.0, 0.0)) + Vector (0.0, RANDOM_FLOAT (-20, 20), 0.0);
                     case 2:
                        v_dest = UTIL_VecToAngles(Vector (-1414.0, 1414.0, 0.0)) + Vector (0.0, RANDOM_FLOAT (-20, 20), 0.0);
                     case 3:
                        v_dest = UTIL_VecToAngles(Vector (-1414.0, -1414.0, 0.0)) + Vector (0.0, RANDOM_FLOAT (-20, 20), 0.0);
                     case 4:
                        v_dest = UTIL_VecToAngles(Vector (1414.0, -1414.0, 0.0)) + Vector (0.0, RANDOM_FLOAT (-20, 20), 0.0);
                  }
               }

               UTIL_ClampVector (&v_dest); // KWo - 17.02.2008
            }

            MAKE_VECTORS (v_dest);
            pBot->vecCamp = paths[pBot->curr_wpt_index]->origin + gpGlobals->v_forward * 500;

            // Switch from 1 direction to the other
            pBot->fNextCampDirectionTime = gpGlobals->time + RANDOM_FLOAT (4.0, 8.0); // KWo - 27.02.2007
         }

         // Press remembered crouch Button
         pEdict->v.button |= pBot->iCampButtons;
         if (pBot->iCampButtons & IN_DUCK)         // KWo - 24.01.2012
            pBot->f_ducktime = gpGlobals->time + 1.0;


         // Stop camping if Time over or gets Hurt by something else than bullets
         if (((BotGetSafeTask(pBot)->fTime < gpGlobals->time) || (pBot->iLastDamageType > 0) || (g_iDebugGoalIndex != -1)) // KWo - 18.07.2006
            && (BotGetSafeTask(pBot)->iTask == TASK_CAMP)) // KWo - 02.09.2006 - against BotRemoveCertainTask
         {
            pBot->iCampButtons = 0; // KWo - 17.02.2008
            BotTaskComplete(pBot);
         }
         break;
      }
      // Hiding Behaviour
      case TASK_HIDE:
      {
         pBot->iAimFlags |= AIM_CAMP;
         pBot->bCheckTerrain = FALSE;
         pBot->bMoveToGoal = FALSE;
         pBot->bUsingGrenade = FALSE;

         // half the reaction time if camping
         pBot->f_ideal_reaction_time = RANDOM_FLOAT (BotSkillDelays[pBot->bot_skill / 20].fMinSurpriseDelay,
                                                  BotSkillDelays[pBot->bot_skill / 20].fMaxSurpriseDelay) / 2;
         pBot->f_wpt_timeset = gpGlobals->time;
         pBot->f_move_speed = 0.0;
         pBot->f_sidemove_speed = 0.0;
         pBot->f_moved_distance = 15.0;               // KWo - 25.05.2010
         GetValidWaypoint (pBot);

         if (BotHasShield (pBot))
         {
            if (!pBot->bIsReloading)
            {
               if (!BotHasShieldDrawn (pBot))
                  pEdict->v.button |= IN_ATTACK2; // draw the shield!
               else
                  pEdict->v.button |= IN_DUCK; // duck under if the shield is already drawn
            }
         }

         // If we see an enemy and aren't at a good camping point leave the spot
         if (pBot->iStates & STATE_SEEINGENEMY)
         {
            if (!(paths[pBot->curr_wpt_index]->flags & W_FL_CAMP))
            {
               if (BotGetSafeTask(pBot)->iTask == TASK_HIDE) // KWo - 02.09.2006 - against BotRemoveCertainTask
               {
                  BotTaskComplete(pBot);
                  pBot->iCampButtons = 0;
                  pBot->prev_goal_index = -1;
                  pBot->prev_time = gpGlobals->time + 2.0; // KWo - 25.02.2008
                  pBot->f_moved_distance = 15.0;               // KWo - 25.05.2010
                  pBot->fNoCollTime = gpGlobals->time + 2.5;   // KWo - 02.01.2010
                  if (!FNullEnt (pBot->pBotEnemy))
                     BotDoAttackMovement(pBot);
                  break;
               }
            }
         }

         pEdict->v.button |= pBot->iCampButtons;
         if (pBot->iCampButtons & IN_DUCK)         // KWo - 24.01.2012
            pBot->f_ducktime = gpGlobals->time + 1.0;

         pBot->f_wpt_timeset = gpGlobals->time;

         // Stop hiding if Time over or gets Hurt by something else than bullets
         if (((BotGetSafeTask(pBot)->fTime < gpGlobals->time) || (pBot->iLastDamageType > 0)
            || ((pBot->current_weapon.iId != CS_WEAPON_KNIFE) && (!pBot->bIsReloading) && (pEdict->v.health > 35))
            || (pBot->vecLastEnemyOrigin == g_vecZero))
               && (BotGetSafeTask(pBot)->iTask == TASK_HIDE)) // KWo - 09.04.2010
         {
            BotTaskComplete(pBot);
            pBot->iCampButtons = 0; // KWo - 17.02.2008
            pBot->prev_goal_index = -1;
            pBot->prev_time = gpGlobals->time + 2.0;     // KWo - 25.02.2008
            pBot->f_moved_distance = 15.0;               // KWo - 25.05.2010
            pBot->fNoCollTime = gpGlobals->time + 2.5;   // KWo - 02.01.2010
         }
         break;
      }
      // Moves to a Position specified in pBot->vecPosition
      // Has a higher Priority than TASK_NORMAL
      case TASK_MOVETOPOSITION:
      {
         pBot->iAimFlags |= AIM_DEST;
         pBot->bUsingGrenade = FALSE;
         fDesire = BotGetSafeTask(pBot)->fDesire; // KWo - 07.01.2008

//       if (BotHasShieldDrawn (pBot))
//          pEdict->v.button |= IN_ATTACK2;

         BotCheckReload (pBot);
/*
         if ((BotUsesSniper (pBot))
            && FNullEnt (pBot->pBotEnemy)
            && (pBot->f_bot_see_enemy_time + 4.0 < gpGlobals->time)
            && (pBot->f_heard_sound_time + 4.0 < gpGlobals->time))
         {
            // Try zooming out to move fast again
            i = HighestPistolOfEdict (pEdict);
            if ((pBot->bot_skill >= 40) && (i != 0)
               && (pBot->current_weapon.iId != cs_weapon_select[i].iId)
               && (pBot->current_weapon.iId != CS_WEAPON_INSWITCH)
               && (pBot->f_bot_see_enemy_time + 4.0 < gpGlobals->time)
               && (pBot->f_heard_sound_time + 4.0 < gpGlobals->time)) // KWo - 19.03.2007
               SelectWeaponByNumber (pBot, i);
         }
*/
         // Reached destination ?
         if (BotDoWaypointNav (pBot) || ((g_iDebugGoalIndex != -1) && (fDesire != TASKPRI_PLANTBOMB)))
         {
            // We're done
            if (BotGetSafeTask(pBot)->iTask == TASK_MOVETOPOSITION) // KWo - 02.09.2006 - against BotRemoveCertainTask
            {
               if (g_b_DebugTasks)
                  ALERT(at_logged, "[DEBUG] BotExecuteTask - TASK_MOVETOPOSITION - Bot %s finished the task - WPTindex = %i; priority = %d.\n",
                     pBot->name, pBot->curr_wpt_index, (int)fDesire);

               pBot->chosengoal_index = -1;
               pBot->prev_goal_index = -1;
               BotGetSafeTask(pBot)->iData = -1;
               BotTaskComplete(pBot);
               pBot->vecPosition = g_vecZero;
               pBot->iAimFlags &= ~AIM_DEST;  // KWo - 18.02.2008
               pBot->prev_time = gpGlobals->time + 1.5; // KWo - 25.02.2008
               pBot->fNoCollTime = gpGlobals->time + 2.0;   // KWo - 02.01.2010
               if (fDesire == TASKPRI_PLANTBOMB) // KWo - 07.01.2008 - high priority to stay away from the ladder
               {
                  if (g_b_DebugNavig || g_b_DebugTasks)
                     ALERT(at_logged, "[DEBUG] BotExecuteTask - TASK_MOVETOPOSITION - Bot %s forced to stay away from the ladder - WPTindex = %i and to pause a bit...\n",
                        pBot->name, pBot->curr_wpt_index);

                  bottask_t TempTask = {NULL, NULL, TASK_PAUSE, TASKPRI_DEFUSEBOMB, -1, gpGlobals->time + 3.0f, TRUE};
                  BotPushTask (pBot, &TempTask);
               }
            }
            break;
         }
         // Didn't choose Goal Waypoint or a path to it yet ?
         else if (!BotGoalIsValid (pBot))
         {
            pBot->f_move_speed = 0.0;
            pBot->prev_time = gpGlobals->time + 0.5; // KWo - 25.02.2008
            pBot->fNoCollTime = gpGlobals->time + 1.0;   // KWo - 02.01.2010

            if ((!g_bPathWasCalculated) && (g_i_botthink_index == g_iFrameCounter) || (pBot->f_spawn_time + 10.0 < gpGlobals->time)) // KWo - 18.02.2008
            {
               DeleteSearchNodes (pBot);
               if ((BotGetSafeTask(pBot)->iData != -1)
                  && (BotGetSafeTask(pBot)->iData < g_iNumWaypoints))
                  iDestIndex = BotGetSafeTask(pBot)->iData;
               else
                  iDestIndex = WaypointFindNearestToMove (pBot->pEdict, pBot->vecPosition); // KWo - 17.04.2008

               pBot->prev_goal_index = iDestIndex;
               pBot->chosengoal_index = iDestIndex; // KWo - 27.06.2006
               BotGetSafeTask(pBot)->iData = iDestIndex;
               pBot->pWayNodesStart = FindLeastCostPath (pBot, pBot->curr_wpt_index, iDestIndex);
               pBot->pWaypointNodes = pBot->pWayNodesStart;

               if (g_b_DebugNavig || g_b_DebugTasks)
                  ALERT(at_logged, "[DEBUG] BotExecuteTask - TASK_MOVETOPOSITION - Bot %s calculated the new path - WPTindex = %i; priority = %d.\n",
                     pBot->name, pBot->curr_wpt_index, (int)fDesire);
            }
         }
         break;
      }
      // Planting the Bomb right now
      case TASK_PLANTBOMB:
      {
         pBot->bUsingGrenade = FALSE;
         if ((pBot->curr_wpt_index >= 0) && (pBot->curr_wpt_index < g_iNumWaypoints))  // KWo - 13.01.2012
         {
            pBot->vecEntity = paths[pBot->curr_wpt_index]->origin - Vector(0.0, 0.0, 36.0);
            pBot->iAimFlags |= AIM_ENTITY;
         }
         else
            pBot->iAimFlags |= AIM_DEST;
         pBot->prev_time = gpGlobals->time + 2.0;     // KWo - 25.02.2008
         pBot->f_moved_distance = 15.0;               // KWo - 25.05.2010
         pBot->fNoCollTime = gpGlobals->time + 2.5;   // KWo - 02.01.2010
         iWeaponNum = GetBestWeaponCarried(pBot);     // KWo - 15.02.2012

         if ((pBot->iStates & STATE_SEEINGENEMY)
             && ((!pBot->bIsChickenOrZombie) && (!g_b_cv_jasonmode)
             && (cs_weapon_select[ptrWeaponPrefs[pBot->bot_personality][iWeaponNum]].iId != CS_WEAPON_KNIFE)
             || ((pEdict->v.origin - pBot->vecLastEnemyOrigin).Length() < 60.0))
             && (BotGetSafeTask(pBot)->iTask == TASK_PLANTBOMB))  // KWo - 22.01.2012
         {
            BotTaskComplete(pBot);
         }
         // We're still in the planting time and got the c4 ?
         else if (pBot->b_bomb_blinking && (pEdict->v.weapons & (1 << CS_WEAPON_C4)))
         {
            if (pBot->current_weapon.iId == CS_WEAPON_C4)
            {
               pBot->bMoveToGoal = FALSE;
               pBot->bCheckTerrain = FALSE;
               pBot->f_wpt_timeset = gpGlobals->time;
               pEdict->v.button |= IN_ATTACK;
               pEdict->v.button |= IN_DUCK;
               pBot->f_ducktime = gpGlobals->time + 1.0;
               pBot->f_move_speed = 0.0;
               pBot->f_sidemove_speed = 0.0;

               if (g_b_DebugTasks || g_b_DebugCombat)
                  ALERT (at_logged, "[DEBUG] BotExecuteTask - TASK_PLANTBOMB - Bot %s is planting C4.\n", pBot->name);
            }
            else if ((pBot->fTimeWeaponSwitch + 0.1 < gpGlobals->time) && (pBot->current_weapon.iId != CS_WEAPON_INSWITCH))
            {
               pBot->bMoveToGoal = FALSE;
               pBot->bCheckTerrain = FALSE;
               pBot->f_wpt_timeset = gpGlobals->time;
               pEdict->v.button |= IN_DUCK;
               pBot->f_ducktime = gpGlobals->time + 1.0;
               pBot->f_move_speed = 0.0;
               pBot->f_sidemove_speed = 0.0;
               SelectWeaponByName (pBot, "weapon_c4");

               if (g_b_DebugTasks || g_b_DebugCombat)
                  ALERT (at_logged, "[DEBUG] BotExecuteTask - TASK_PLANTBOMB - Bot %s is selecting C4.\n", pBot->name);
            }
         }

         // Done with planting
         else
         {
            if (BotGetSafeTask(pBot)->iTask == TASK_PLANTBOMB) // KWo - 02.09.2006 - against BotRemoveCertainTask
            {
               BotTaskComplete(pBot);

               if (!(pEdict->v.weapons & (1 << CS_WEAPON_C4)))
               {
                  // Notify the Team of this heroic action
                  if ((!g_b_cv_ffa) && (g_b_cv_chat) && (g_iAliveTs > 1) && (RANDOM_LONG(1,100) < 10)) // KWo - 08.03.2010
                     FakeClientCommand (pBot->pEdict, "say_team Planted the Bomb!\n");
                  DeleteSearchNodes (pBot);

                  fCampTime = (g_f_cv_c4timer * 0.5); // KWo - 23.03.2008
                  if (fCampTime > g_f_cv_maxcamptime)       // KWo - 23.03.2008
                     fCampTime = g_f_cv_maxcamptime;        // KWo - 23.03.2008

                  // Push camp task on to stack
                  pBot->fTimeCamping = gpGlobals->time + fCampTime; // KWo - 23.03.2008
                  bottask_t TempTask = {NULL, NULL, TASK_CAMP, TASKPRI_CAMP, -1, pBot->fTimeCamping, TRUE}; // KWo - 23.03.2008
                  BotPushTask (pBot, &TempTask);

                  // Push Move Command
                  TempTask.iTask = TASK_MOVETOPOSITION;
                  TempTask.fDesire = TASKPRI_MOVETOPOSITION;
                  TempTask.iData = BotFindDefendWaypoint (pBot, pEdict->v.origin);
                  BotPushTask (pBot, &TempTask);
                  pBot->iCampButtons |= IN_DUCK;
               }
            }
         }
         break;
      }
      // Bomb defusing Behaviour
      case TASK_DEFUSEBOMB:
      {
         pBot->iAimFlags |= AIM_ENTITY;
         pBot->bMoveToGoal = FALSE;
         pBot->bCheckTerrain = FALSE;
         pBot->bUsingGrenade = FALSE;
         pBot->prev_time = gpGlobals->time + 2.0;     // KWo - 25.02.2008
         pBot->f_moved_distance = 15.0;               // KWo - 25.05.2010
         pBot->fNoCollTime = gpGlobals->time + 2.5;   // KWo - 02.01.2010
         v_bomb = g_vecZero; // KWo - 12.08.2006
         pBot->f_wpt_timeset = gpGlobals->time;

         // Bomb still there ?
         if (!FNullEnt (pBot->pBotPickupItem))
         {
            // Get Face Position
            pBot->vecEntity = pBot->pBotPickupItem->v.origin;
            v_bomb = pBot->vecEntity;
            flDot = GetShootingConeDeviation (pEdict, &v_bomb);
            if (flDot > 0.9) // KWo - 12.08.2006
            {
               pEdict->v.button |= IN_USE;
            }
            pEdict->v.button |= IN_DUCK;
            pBot->f_ducktime = gpGlobals->time + 1.0;
         }
         else
         {
            if (BotGetSafeTask(pBot)->iTask == TASK_DEFUSEBOMB) // KWo - 02.09.2006 - against BotRemoveCertainTask
            {
               pBot->iAimFlags &= ~AIM_ENTITY;
               BotTaskComplete(pBot);
            }
         }

         pBot->f_move_speed = 0.0;
         pBot->f_sidemove_speed = 0.0;

         break;
      }
      // Follow User Behaviour
      case TASK_FOLLOWUSER:
      {
         pBot->iAimFlags |= AIM_DEST;
         BotCheckReload (pBot);
         pBot->bUsingGrenade = FALSE;

         if (g_b_cv_ffa)  // KWo - 02.02.2007
         {
            if (BotGetSafeTask(pBot)->iTask == TASK_FOLLOWUSER) // KWo - 02.09.2006 - against BotRemoveCertainTask
               BotTaskComplete(pBot);
            pBot->pBotUser = NULL;
            break;
         }
/*
         if ((BotUsesSniper (pBot))
            && FNullEnt (pBot->pBotEnemy)
            && (pBot->f_bot_see_enemy_time + 1.0 < gpGlobals->time))
         {
            // Try zooming out to move fast again
            i = HighestPistolOfEdict(pEdict);

            if ((pBot->bot_skill >= 40) && (i != 0)
               && (pBot->current_weapon.iId != cs_weapon_select[i].iId)
               && (pBot->current_weapon.iId != CS_WEAPON_INSWITCH)
               && (pBot->fTimeWeaponSwitch + 0.5 < gpGlobals->time)
               && (pBot->f_bot_see_enemy_time + 4.0 < gpGlobals->time)
               && (pBot->f_heard_sound_time + 4.0 < gpGlobals->time)) // KWo - 19.03.2007
                  SelectWeaponByNumber (pBot, i);
         }
*/

         // Follow User ?
         if (BotFollowUser (pBot))
         {
            pBot->bCheckTerrain = TRUE;  // 13.11.2006
            bBotReachedDest = (BotDoWaypointNav (pBot) || (pBot->pWaypointNodes == NULL)
            || (pBot->chosengoal_index == -1) || (BotGetSafeTask(pBot)->iData == -1));

            fDistanceToUser = (pBot->pBotUser->v.origin - pEdict->v.origin).Length(); // KWo - 26.02.2007
            if ((( fDistanceToUser > 256)
               || (!FVisible (pBot->pBotUser->v.origin, pEdict)) )
               && (bBotReachedDest) || (fDistanceToUser < 50.0)) // KWo - 26.02.2007
            {
               if (((g_i_botthink_index != g_iFrameCounter) || (g_bPathWasCalculated))
                  && (pBot->f_spawn_time + 10.0 > gpGlobals->time)) // KWo - 18.02.2008
               {
                  pBot->f_move_speed = 0.0;
                  pBot->f_moved_distance = 15.0;      // KWo - 25.05.2010
                  if (fDistanceToUser > 50.0)         // KWo - 16.01.2010
                  {
                     pBot->prev_time = gpGlobals->time + 1.5; // KWo - 25.02.2008
                     pBot->fNoCollTime = gpGlobals->time + 2.0;   // KWo - 02.01.2010
//                pBot->f_wpt_timeset = gpGlobals->time;  // KWo - 30.07.2006
                  }
                  else
                     pBot->fNoCollTime = gpGlobals->time - 2.0;   // KWo - 27.03.2010
               }
               else
               {
                  iTempIndex = WaypointFindNearestToMove (pBot->pBotUser, pBot->pBotUser->v.origin); // KWo - 17.04.2008
                  iCount = 0;
                  bCon = false;
                  if (fDistanceToUser > 50.0) // KWo - 16.01.2010
                  {
                     pBot->prev_time = gpGlobals->time + 1.5;     // KWo - 25.02.2008
                     pBot->fNoCollTime = gpGlobals->time + 2.0;   // KWo - 02.01.2010
                  }

                  do
                  {
                     iTempPathNr = (int) RANDOM_LONG(0, MAX_PATH_INDEX - 1);
                     iDestIndex = paths[iTempIndex]->index[iTempPathNr];
                     if ((iDestIndex > 0) && (iDestIndex <= g_iNumWaypoints))
                     {
                        if (IsConnectedWithWaypoint (iDestIndex, iTempIndex))
                        {
                           bCon = true;
                           break;
                        }
                     }
                     iCount++;
                  }
                  while ((iCount < 10) && (!bCon));

                  if (!bCon)
                  iDestIndex = iTempIndex;

                  pBot->bCheckTerrain = TRUE;  // 25.06.2006

                  if ((iDestIndex!= pBot->curr_wpt_index))
                  {
                     DeleteSearchNodes (pBot);
                     pBot->prev_goal_index = iDestIndex;
                     pBot->chosengoal_index = iDestIndex; // KWo - 27.06.2006

                     // Do Pathfinding if it's not the current waypoint
                     pBot->pTasks->iData = iDestIndex;
                     pBot->pWayNodesStart = FindShortestPath (pBot->curr_wpt_index, iDestIndex, &bPathValid);  // KWo - 25.03.2006
                     pBot->pWaypointNodes = pBot->pWayNodesStart;

                     if (g_b_DebugTasks || g_b_DebugNavig)
                        ALERT(at_logged, "[DEBUG] BotExecuteTask - TASK_FOLLOWUSER - Bot %s is continuing to follow the player %s; target wp_index = %i.\n",
                           pBot->name, STRING(pBot->pBotUser->v.netname), iDestIndex);

                     if (!bPathValid) // KWo - 25.03.2006
                     {
                        DeleteSearchNodes (pBot);
                        if (BotGetSafeTask(pBot)->iTask == TASK_FOLLOWUSER) // KWo - 02.09.2006 - against BotRemoveCertainTask
                           BotTaskComplete(pBot);
                        pBot->pBotUser = NULL; // KWo - 12.07.2006
                     }
                  }

                  if (g_b_DebugTasks || g_b_DebugNavig)
                     ALERT (at_logged, "[DEBUG] BotExecuteTask - TASK_FOLLOWUSER - Bot %s is following the player %s; target wp_index = %i.\n",
                        pBot->name, STRING(pBot->pBotUser->v.netname), iDestIndex);
               }
            }
            else if (bBotReachedDest)
            {
               pBot->dest_origin = pBot->pEdict->v.origin;
               pBot->prev_time = gpGlobals->time + 0.5;     // KWo - 25.05.2010
               pBot->f_moved_distance = 15.0;               // KWo - 25.05.2010
               // Imitate Users crouching
//             pEdict->v.button |= (pBot->pBotUser->v.button & IN_DUCK);
//            pEdict->v.button &= ~IN_JUMP;
               pBot->bCheckTerrain = TRUE;  // 23.06.2006
            }
         }
         else
         {
            if (BotGetSafeTask(pBot)->iTask == TASK_FOLLOWUSER) // KWo - 02.09.2006 - against BotRemoveCertainTask
               BotTaskComplete(pBot);
            pBot->pBotUser = NULL;
            pBot->f_moved_distance = 15.0;               // KWo - 25.05.2010
            pBot->prev_time = gpGlobals->time + 0.5;     // KWo - 25.05.2010
         }

         if ((pBot->bot_team == TEAM_CS_TERRORIST) && (pBot->b_bomb_blinking)
            && (pBot->f_bot_see_enemy_time + 3.0 < gpGlobals->time)
            && (pEdict->v.weapons & (1 << CS_WEAPON_C4))) // KWo - 15.02.2012
         {
            if (BotGetSafeTask(pBot)->iTask == TASK_FOLLOWUSER) // KWo - 02.09.2006 - against BotRemoveCertainTask
               BotTaskComplete(pBot);
            pBot->pBotUser = NULL;
            pBot->prev_time = gpGlobals->time + 2.0;     // KWo - 25.02.2008
            pBot->f_moved_distance = 15.0;               // KWo - 25.05.2010
            pBot->fNoCollTime = gpGlobals->time + 2.5;   // KWo - 02.01.2010

            if (pBot->current_weapon.iId != CS_WEAPON_C4)
               SelectWeaponByName (pBot, "weapon_c4");
            bottask_t TempTask = {NULL, NULL, TASK_PLANTBOMB, TASKPRI_PLANTBOMB, -1, 0.0, TRUE}; // KWo - 21.02.2007
            BotPushTask (pBot, &TempTask);
         }
         if ((BotGetSafeTask(pBot)->fTime < gpGlobals->time) && (BotGetSafeTask(pBot)->iTask == TASK_FOLLOWUSER))
         {
            BotTaskComplete(pBot);
            pBot->pBotUser = NULL;
            pBot->prev_time = gpGlobals->time + 2.0;     // KWo - 25.02.2008
            pBot->f_moved_distance = 15.0;               // KWo - 25.05.2010
            pBot->fNoCollTime = gpGlobals->time + 2.5;   // KWo - 02.01.2010
         }

         break;
      }
      // HE Grenade Throw Behaviour
      case TASK_THROWHEGRENADE:
      {
         if (g_bIsOldCS15)  //  KWo - 19.06.2006
         {
            pBot->rgvecRecoil[0] = g_vecZero;
            pBot->rgvecRecoil[1] = g_vecZero;
            pBot->rgvecRecoil[2] = g_vecZero;
            pBot->rgvecRecoil[3] = g_vecZero;
            pBot->rgvecRecoil[4] = g_vecZero;
         }
         pBot->iAimFlags |= AIM_GRENADE;
         v_dest = pBot->vecThrow;
         pBot->prev_time = gpGlobals->time + 2.0;     // KWo - 25.02.2008
         pBot->f_moved_distance = 15.0;               // KWo - 25.05.2010
         pBot->fNoCollTime = gpGlobals->time + 2.5;   // KWo - 02.01.2010
         pBot->f_grenade_check_time = gpGlobals->time + RANDOM_FLOAT (4.0, 7.0); // KWo - 24.08.2008

         if (!(pBot->iStates & STATE_SEEINGENEMY))
         {
            pBot->f_sidemove_speed = 0.0;
            pBot->bMoveToGoal = FALSE;
         }

         if (!FNullEnt (pBot->pLastEnemy) && (pBot->vecLastEnemyOrigin != g_vecZero))
         {
            distance = (int)(pBot->vecLastEnemyOrigin - pEdict->v.origin).Length ();
            if (pBot->iStates & STATE_SEEINGENEMY)
            {
               if (IsAlive (pBot->pLastEnemy)) // KWo - 26.07.2007
               {
                  v_dest = pBot->vecLastEnemyOrigin;
                  v_src = pBot->pLastEnemy->v.velocity;
                  v_src.z = 0.0;
                  v_dest = v_dest + (v_src * distance / 400.0);
               }
               else
                  v_dest = g_vecZero;
            }
         }
         else
            v_dest = g_vecZero;

         pBot->bUsingGrenade = TRUE;
         pBot->bCheckTerrain = FALSE;

         pBot->vecThrow = v_dest;  // KWo - 19.06.2006
         if (pBot->vecThrow != g_vecZero) // KWo - 26.07.2007
         {
            pBot->vecGrenade = VecCheckThrow  (pBot, GetGunPosition (pEdict), v_dest, 1.0);
            if (pBot->vecGrenade == g_vecZero /* && !pBot->bMoveToGoal */)
               pBot->vecGrenade = VecCheckToss  (pBot, GetGunPosition (pEdict), v_dest);
         }
         else
            pBot->vecGrenade = g_vecZero;

         if (((pBot->vecGrenade == g_vecZero)  /* && (pBot->current_weapon.iId != CS_WEAPON_HEGRENADE) */ )
            || (BotGetSafeTask(pBot)->fTime < gpGlobals->time)
            || ((pEdict->v.weapons & (1 << CS_WEAPON_HEGRENADE)) == 0)
            || (NumTeammatesNearPos (pBot, v_dest, 512) > 0)) // KWo - 25.08.2008
         {
            pBot->iStates &= ~STATE_THROWHEGREN;   // KWo - 24.02.2006
            pBot->iAimFlags &= ~AIM_GRENADE;       // KWo - 24.02.2006
            pBot->bUsingGrenade = FALSE;           // KWo - 19.03.2007
            if ((pBot->fTimeWeaponSwitch + 1.0 < gpGlobals->time) && (BotGetSafeTask(pBot)->iTask == TASK_THROWHEGRENADE)) // KWo - 22.01.2008
            {
               BotSelectBestWeapon (pBot);
               BotTaskComplete(pBot);
            }
         }
         else if ((pBot->vecGrenade != g_vecZero) /* || (pBot->current_weapon.iId == CS_WEAPON_HEGRENADE) */) // KWo - 22.01.2008 - rewritten again...
         {
            if ((pBot->current_weapon.iId != CS_WEAPON_HEGRENADE) && (pBot->current_weapon.iId != CS_WEAPON_INSWITCH)
               && (pBot->fTimeWeaponSwitch + 0.5 < gpGlobals->time) /* && !FStrEq(STRING(pEdict->v.viewmodel),"models/v_hegrenade.mdl") */)
            {
               if (pEdict->v.weapons & (1 << CS_WEAPON_HEGRENADE))
               {
                  SelectWeaponByName(pBot, "weapon_hegrenade");
                  BotGetSafeTask(pBot)->fTime = gpGlobals->time + TIME_GRENPRIME;
               }
            }
            else if ((pBot->current_weapon.iId == CS_WEAPON_HEGRENADE) && (pBot->fTimeWeaponSwitch + 0.3 < gpGlobals->time)
                        && !(pEdict->v.oldbuttons & IN_ATTACK))
            {
               pEdict->v.button |= IN_ATTACK;
            }
         }
         pEdict->v.button |= pBot->iCampButtons;
         if (pBot->iCampButtons & IN_DUCK)         // KWo - 24.01.2012
            pBot->f_ducktime = gpGlobals->time + 1.0;
         break;
      }
      // Flashbang Throw Behaviour
      // Basically the same code like for HE's
      case TASK_THROWFLASHBANG:
      {
         if (g_bIsOldCS15)  //  KWo - 19.06.2006
         {
            pBot->rgvecRecoil[0] = g_vecZero;
            pBot->rgvecRecoil[1] = g_vecZero;
            pBot->rgvecRecoil[2] = g_vecZero;
            pBot->rgvecRecoil[3] = g_vecZero;
            pBot->rgvecRecoil[4] = g_vecZero;
         }

         pBot->iAimFlags |= AIM_GRENADE;
         v_dest = pBot->vecThrow;
         pBot->prev_time = gpGlobals->time + 2.0;     // KWo - 25.02.2008
         pBot->f_moved_distance = 15.0;               // KWo - 25.05.2010
         pBot->fNoCollTime = gpGlobals->time + 2.5;   // KWo - 02.01.2010
         pBot->f_grenade_check_time = gpGlobals->time + RANDOM_FLOAT (2.0, 5.0); // KWo - 24.08.2008

         pBot->bUsingGrenade = TRUE;
         pBot->bCheckTerrain = FALSE;
         pBot->vecGrenade = VecCheckThrow  (pBot, GetGunPosition (pEdict), v_dest, 1.0);
         if (pBot->vecGrenade == g_vecZero /* && !pBot->bMoveToGoal */)
            pBot->vecGrenade = VecCheckToss  (pBot, GetGunPosition (pEdict), v_dest);

         if ((pBot->vecGrenade == g_vecZero) || (BotGetSafeTask(pBot)->fTime < gpGlobals->time)
          || ((pEdict->v.weapons & (1 << CS_WEAPON_FLASHBANG)) == 0))
         {
            pBot->iStates &= ~STATE_THROWFLASHBANG;   // KWo - 24.02.2006
            pBot->iAimFlags &= ~AIM_GRENADE;          // KWo - 24.02.2006
            pBot->bUsingGrenade = FALSE;              // KWo - 19.03.2007
            if ((pBot->fTimeWeaponSwitch + 1.0 < gpGlobals->time) && (BotGetSafeTask(pBot)->iTask == TASK_THROWFLASHBANG)) // KWo - 22.01.2008
            {
               BotSelectBestWeapon (pBot);
               BotTaskComplete(pBot);
            }
         }
         else if (pBot->vecGrenade != g_vecZero) // KWo - 22.01.2008 - rewritten again...
         {
            if ((pBot->current_weapon.iId != CS_WEAPON_FLASHBANG) && (pBot->current_weapon.iId != CS_WEAPON_INSWITCH)
                 && (pBot->fTimeWeaponSwitch + 0.5 < gpGlobals->time) /* && !FStrEq(STRING(pEdict->v.viewmodel),"models/v_flashbang.mdl") */)
            {
               if (pEdict->v.weapons & (1 << CS_WEAPON_FLASHBANG))
               {
                  SelectWeaponByName(pBot, "weapon_flashbang");
                  BotGetSafeTask(pBot)->fTime = gpGlobals->time + TIME_GRENPRIME;
               }
            }
            else if ((pBot->current_weapon.iId == CS_WEAPON_FLASHBANG) && (pBot->fTimeWeaponSwitch + 0.3 < gpGlobals->time)
                     && !(pEdict->v.oldbuttons & IN_ATTACK) )
            {
               pEdict->v.button |= IN_ATTACK;
            }
         }
         pEdict->v.button |= pBot->iCampButtons;
         if (pBot->iCampButtons & IN_DUCK)         // KWo - 24.01.2012
            pBot->f_ducktime = gpGlobals->time + 1.0;
         break;
      }
      // Smoke Grenade Throw Behaviour
      // A bit different to the others because it mostly
      // tries to throw the Gren on the ground
      case TASK_THROWSMOKEGRENADE:
      {
         if (g_bIsOldCS15)  //  KWo - 19.06.2006
         {
            pBot->rgvecRecoil[0] = g_vecZero;
            pBot->rgvecRecoil[1] = g_vecZero;
            pBot->rgvecRecoil[2] = g_vecZero;
            pBot->rgvecRecoil[3] = g_vecZero;
            pBot->rgvecRecoil[4] = g_vecZero;
         }

         pBot->iAimFlags |= AIM_GRENADE;
         if (!(pBot->iStates & STATE_SEEINGENEMY))
         {
            pBot->f_sidemove_speed = 0.0;
            pBot->bMoveToGoal = FALSE;
         }
         pBot->bCheckTerrain = FALSE;
         pBot->bUsingGrenade = TRUE;

         pBot->prev_time = gpGlobals->time + 2.0;     // KWo - 25.02.2008
         pBot->f_moved_distance = 15.0;               // KWo - 25.05.2010
         pBot->fNoCollTime = gpGlobals->time + 2.5;   // KWo - 02.01.2010
         pBot->f_grenade_check_time = gpGlobals->time + RANDOM_FLOAT (5.0, 7.0); // KWo - 24.08.2008

         if (!FNullEnt (pBot->pLastEnemy) && (pBot->vecLastEnemyOrigin != g_vecZero))
         {
            distance = (int)(pBot->vecLastEnemyOrigin - pEdict->v.origin).Length ();
            if (pBot->iStates & STATE_SEEINGENEMY)
            {
               if (IsAlive (pBot->pLastEnemy)) // KWo - 26.07.2007
               {
                  v_dest = pBot->vecLastEnemyOrigin;
                  v_src = pBot->pLastEnemy->v.velocity;
                  v_src.z = 0.0;
                  v_dest = v_dest + (v_src * distance / 400.0);
                  v_dest = v_dest.Normalize();
                  v_dest = v_dest * 0.6 * distance;
               }
               else
                  v_dest = g_vecZero;
            }
         }
         else
            v_dest = g_vecZero;

         pBot->bUsingGrenade = TRUE;
         pBot->bCheckTerrain = FALSE;

         pBot->vecThrow = v_dest;  // KWo - 19.06.2006
         if (pBot->vecThrow != g_vecZero) // KWo - 26.07.2007
         {
            pBot->vecGrenade = VecCheckThrow  (pBot, GetGunPosition (pEdict), v_dest, 1.0);
            if (pBot->vecGrenade == g_vecZero /* && !pBot->bMoveToGoal */)
               pBot->vecGrenade = VecCheckToss  (pBot, GetGunPosition (pEdict), v_dest);
         }
         else
            pBot->vecGrenade = g_vecZero;

         pBot->vecThrow = v_dest;  // KWo - 19.06.2006

         if (pBot->vecThrow != g_vecZero) // KWo - 26.07.2007
         {
            pBot->vecGrenade = VecCheckThrow  (pBot, GetGunPosition (pEdict), v_src, 1.0);

            if (pBot->vecGrenade == g_vecZero /* && !pBot->bMoveToGoal */)
               pBot->vecGrenade = VecCheckToss  (pBot, GetGunPosition (pEdict), v_src);
         }
         else
            pBot->vecGrenade = g_vecZero;

         if (((pBot->vecGrenade == g_vecZero) && (pBot->current_weapon.iId != CS_WEAPON_SMOKEGRENADE)) || (BotGetSafeTask(pBot)->fTime < gpGlobals->time)
            || ((pEdict->v.weapons & (1 << CS_WEAPON_SMOKEGRENADE)) == 0))
         {
            pBot->iStates &= ~STATE_THROWSMOKEGREN;   // KWo - 24.02.2006
            pBot->iAimFlags &= ~AIM_GRENADE;          // KWo - 24.02.2006
            pBot->bUsingGrenade = FALSE;              // KWo - 19.03.2007
            if ((pBot->fTimeWeaponSwitch + 1.0 < gpGlobals->time) && (BotGetSafeTask(pBot)->iTask == TASK_THROWSMOKEGRENADE)) // KWo - 22.01.2008
            {
               BotSelectBestWeapon (pBot);
               BotTaskComplete(pBot);
            }
         }
         else if ((pBot->vecGrenade != g_vecZero) || (pBot->current_weapon.iId == CS_WEAPON_SMOKEGRENADE))// KWo - 22.01.2008 - rewritten again...
         {
            if ((pBot->current_weapon.iId != CS_WEAPON_SMOKEGRENADE) && (pBot->current_weapon.iId != CS_WEAPON_INSWITCH)
                 && (pBot->fTimeWeaponSwitch + 0.5 < gpGlobals->time))
            {
               if (pEdict->v.weapons & (1 << CS_WEAPON_SMOKEGRENADE))
               {
                  SelectWeaponByName(pBot, "weapon_smokegrenade");
                  BotGetSafeTask(pBot)->fTime = gpGlobals->time + TIME_GRENPRIME;
               }
            }
            else if ((pBot->current_weapon.iId == CS_WEAPON_SMOKEGRENADE) && (pBot->fTimeWeaponSwitch + 0.3 < gpGlobals->time)
                        && !(pEdict->v.oldbuttons & IN_ATTACK))
            {
               pEdict->v.button |= IN_ATTACK;
            }
         }
         pEdict->v.button |= pBot->iCampButtons;
         if (pBot->iCampButtons & IN_DUCK)         // KWo - 24.01.2012
            pBot->f_ducktime = gpGlobals->time + 1.0;
         break;
      }
      // Shooting breakables in the way action
      case TASK_SHOOTBREAKABLE:
      {
         pBot->iAimFlags |= AIM_OVERRIDE;
         pBot->bUsingGrenade = FALSE;
         pBot->prev_time = gpGlobals->time + 2.0;     // KWo - 25.02.2008
         pBot->f_moved_distance = 15.0;               // KWo - 25.05.2010
         pBot->fNoCollTime = gpGlobals->time + 2.5;   // KWo - 02.01.2010

         // Breakable destroyed ?

         if (g_b_DebugTasks || g_b_DebugEntities)
            ALERT (at_logged, "[DEBUG] Bot %s is executing TASK_SHOOTBREAKABLE.\n", pBot->name);

         if ((!BotFindBreakable (pBot)) && (BotGetSafeTask(pBot)->iTask == TASK_SHOOTBREAKABLE)) // KWo - 02.09.2006 - against BotRemoveCertainTask
         {
            pBot->iAimFlags &= ~AIM_ENTITY; // KWo - 09.02.2008
            pBot->iAimFlags &= ~AIM_OVERRIDE; // KWo - 12.03.2010
            if (g_b_DebugTasks || g_b_DebugEntities) // KWo - 10.04.2016
               ALERT (at_logged, "[DEBUG] Bot %s is executing TASK_SHOOTBREAKABLE, but didn't find the breakable.\n", pBot->name);
            BotTaskComplete(pBot);
            break;
         }
         pEdict->v.button |= pBot->iCampButtons;
         if (pBot->iCampButtons & IN_DUCK)         // KWo - 24.01.2012
            pBot->f_ducktime = gpGlobals->time + 1.0;

         pBot->bCheckTerrain = FALSE;
         pBot->bMoveToGoal = FALSE;
         pBot->f_wpt_timeset = gpGlobals->time;
         v_src = pBot->vecBreakable;
         pBot->vecCamp = v_src;

         flDot = GetShootingConeDeviation (pEdict, &v_src);

         // Is Bot facing the Breakable ?
         if (flDot >= 0.95) // KWo - 14.04.2013
         {
            pBot->f_move_speed = 0.0;
            pBot->f_sidemove_speed = 0.0;
            if (!(pEdict->v.oldbuttons & IN_ATTACK))
                pEdict->v.button |= IN_ATTACK;
            pBot->bWantsToFire = TRUE;
         }
         else
         {
            pBot->bCheckTerrain = TRUE;
            pBot->bMoveToGoal = TRUE;
         }

         if ((pBot->current_weapon.iId == CS_WEAPON_KNIFE) && ((pEdict->v.origin - v_src).Length() > 40.0)) // KWo - 20.05.2010
         {
            pBot->bCheckTerrain = TRUE;
            pBot->bMoveToGoal = TRUE;
         }
         break;
      }
      // Picking up Items and stuff behaviour
      case TASK_PICKUPITEM:
      {
         pBot->bUsingGrenade = FALSE;
         if (FNullEnt (pBot->pBotPickupItem))
         {
            pBot->pBotPickupItem = NULL;
            if (BotGetSafeTask(pBot)->iTask == TASK_PICKUPITEM) // KWo - 02.09.2006 - against BotRemoveCertainTask
            {
               pBot->iAimFlags &= ~AIM_ENTITY; // KWo - 16.09.2006
               BotTaskComplete(pBot);
               pBot->prev_time = gpGlobals->time + 2.0; // KWo - 25.02.2008
               pBot->fNoCollTime = gpGlobals->time + 2.5;   // KWo - 02.01.2010
            }
            break;
         }
         if (BotGetSafeTask(pBot)->fTime == 0.0)
         {
            BotGetSafeTask(pBot)->fTime = gpGlobals->time + ((pBot->iPickupType == PICKUP_HOSTAGE)? 8.0 : 4.0); // KWo - 08.03.2010

            if (g_b_DebugTasks || g_b_DebugEntities)
               ALERT(at_logged,"[DEBUG] BotExecuteTask - TASK_PICKUPITEM - Bot %s has started the task with pickup_type = %s.\n",
                  pBot->name, g_ItemTypeNames[pBot->iPickupType]);
         }
         else if ((BotGetSafeTask(pBot)->fTime < gpGlobals->time)
            && ((pBot->iPickupType != PICKUP_PLANTED_C4) || (pBot->bot_team == TEAM_CS_TERRORIST))) // KWo - 02.09.2008
         {
            pBot->pItemIgnore[2] = pBot->pItemIgnore[1];
            pBot->pItemIgnore[1] = pBot->pItemIgnore[0];
            pBot->pItemIgnore[0] = pBot->pBotPickupItem;
            if (BotGetSafeTask(pBot)->iTask == TASK_PICKUPITEM) // KWo - 02.09.2006 - against BotRemoveCertainTask
            {
               pBot->iAimFlags &= ~AIM_ENTITY;

               if (g_b_DebugTasks)
                  ALERT(at_logged,"[DEBUG] BotExecuteTask - TASK_PICKUPITEM - Bot %s finished the task because of time expiring.\n", pBot->name);

               if (g_b_DebugEntities)
               {
                  if (pBot->iPickupType == PICKUP_WEAPON)
                  {
                     int iId = pBot->current_weapon.iId;
                     int iWeaponSecNum = HighestPistolOfEdict (pEdict);
                     int iWeaponPrimNum = HighestWeaponOfEdict(pEdict);
                     int iWeaponSecID = cs_weapon_select[iWeaponSecNum].iId;
                     int iWeaponPrimID = cs_weapon_select[iWeaponPrimNum].iId;
                     char vmodelname[64];
                     snprintf (vmodelname, sizeof (vmodelname), STRING (pBot->pBotPickupItem->v.model));
                     ALERT(at_logged,"[DEBUG] BotExecuteTask - TASK_PICKUPITEM - Bot %s was trying to pickup %s; has %s with %d ammo in clip.\n", pBot->name,
                        vmodelname, weapon_defs[iId].szClassname, pBot->m_rgAmmoInClip[iId]);
                     ALERT(at_logged,"[DEBUG] BotExecuteTask - TASK_PICKUPITEM - Bot %s has primary weapon %s with %d ammo in clip and sec weapon %s with %d ammo in clip.\n",  pBot->name,
                        weapon_defs[iWeaponPrimID].szClassname, pBot->m_rgAmmoInClip[iWeaponPrimID],
                        weapon_defs[iWeaponSecID].szClassname, pBot->m_rgAmmoInClip[iWeaponSecID]);
                  }
               }

               pBot->pBotPickupItem = NULL;
               pBot->iPickupType = PICKUP_NONE;
               BotTaskComplete(pBot);
               pBot->prev_time = gpGlobals->time + 2.0;     // KWo - 25.02.2008
               pBot->f_moved_distance = 15.0;               // KWo - 25.05.2010
               pBot->fNoCollTime = gpGlobals->time + 2.5;   // KWo - 02.01.2010
            }
            break;
         }

         // func Models needs special origin handling
         if ((strncmp ("func_", STRING (pBot->pBotPickupItem->v.classname), 5) == 0)
          || (pBot->pBotPickupItem->v.flags & FL_MONSTER))
            v_dest = VecBModelOrigin (pBot->pBotPickupItem);
         else
            v_dest = pBot->pBotPickupItem->v.origin;

         pBot->dest_origin = v_dest;
         pBot->vecEntity = v_dest;

         // find the distance to the item
         distance = (int)(v_dest - pEdict->v.origin).Length ();

         switch (pBot->iPickupType)
         {
            case PICKUP_WEAPON:
            {
               pBot->iAimFlags |= (pBot->bot_skill < 60)? AIM_DEST : AIM_ENTITY;
               memset (vmodelname, 0, sizeof (vmodelname));
               snprintf (vmodelname, sizeof (vmodelname), STRING (pBot->pBotPickupItem->v.model)); // KWo - 20.12.2006

               if (g_b_DebugEntities)
                  ALERT(at_logged,"[DEBUG] BotExecuteTask - TASK_PICKUPITEM - Bot %s is trying to pickup %s.\n", pBot->name, vmodelname);
               // Near to Weapon ?
               if (distance < 50)
               {
                  ptrWeaponTab = ptrWeaponPrefs[pBot->bot_personality];
                  iGroundIndex = 0;
                  for (i = 0; i < NUM_WEAPONS; i++)
                  {
                     if (FStrEq (cs_weapon_select[*ptrWeaponTab].model_name, vmodelname))
                     {
                        iGroundIndex = i;
                        break;
                     }

                     ptrWeaponTab++;
                  }
                  if (iGroundIndex > 6)
                  {
                     if (g_b_DebugEntities)
                        ALERT(at_logged,"[DEBUG] BotExecuteTask - TASK_PICKUPITEM - Bot %s is trying to pickup prim weapon %s.\n", pBot->name, vmodelname);

                     if (BotHasShield (pBot))
                        FakeClientCommand (pEdict, "drop"); // discard shield

                     // Get current best weapon to check if it's a primary in need to be dropped
                     iWeaponNum = HighestWeaponOfEdict (pEdict);
                     if (iWeaponNum > 6)
                     {
//                        if (pBot->current_weapon.iId != cs_weapon_select[iWeaponNum].iId)
                        if (!WeaponIsPrimaryGun(pBot->current_weapon.iId)) // KWo - 25.12.2006
                        {
                           if (g_b_DebugEntities || g_b_DebugCombat)
                              ALERT(at_logged,"[DEBUG] BotExecuteTask - TASK_PICKUPITEM - Bot %s doesn't have selected a primary gun.\n", pBot->name);

                           if ((pBot->current_weapon.iId != CS_WEAPON_INSWITCH) && (pBot->fTimeWeaponSwitch + 0.5 < gpGlobals->time))
                           {
                              SelectWeaponByNumber (pBot, iWeaponNum);
                           }
                        }
                        else
                        {
                           FakeClientCommand (pEdict, "drop");

                           if (g_b_DebugEntities || g_b_DebugCombat)
                              ALERT(at_logged,"[DEBUG] BotExecuteTask - TASK_PICKUPITEM - Bot %s drops its primary weapon.\n", pBot->name);
                        }
                     }
                     else if (g_i_cv_maxweaponpickup > 0)
                        pBot->iNumWeaponPickups++;
                  }
                  else if (iGroundIndex > 0)
                  {
                     if (BotHasShield (pBot))
                        FakeClientCommand (pEdict, "drop"); // discard shield

                     // Get current best secondary weapon to check if it's a secondary in need to be dropped
                     iWeaponNum = HighestPistolOfEdict (pEdict);

                     if (g_b_DebugEntities || g_b_DebugCombat)
                        ALERT(at_logged,"[DEBUG] BotExecuteTask - TASK_PICKUPITEM - Bot %s is trying to pickup sec weapon %s.; iWeaponNum = %d.\n", pBot->name, vmodelname, iWeaponNum);

                     if (iWeaponNum != 0)
                     {
//                        if (pBot->current_weapon.iId != cs_weapon_select[iWeaponNum].iId)
                        if (!WeaponIsPistol(pBot->current_weapon.iId)) // KWo - 25.12.2006
                        {
                           if ((pBot->current_weapon.iId != CS_WEAPON_INSWITCH) && (pBot->fTimeWeaponSwitch + 0.5 < gpGlobals->time))
                           {
                              SelectWeaponByNumber (pBot, iWeaponNum);
                           }
                        }
                        else
                        {
                           FakeClientCommand (pEdict, "drop");

                           if (g_b_DebugEntities || g_b_DebugCombat)
                              ALERT(at_logged,"[DEBUG] BotExecuteTask - TASK_PICKUPITEM - Bot %s drops its pistol.\n", pBot->name);
                        }
                     }
                     else if (g_i_cv_maxweaponpickup > 0)
                        pBot->iNumWeaponPickups++;
                  }
               }
               break;
            }
            case PICKUP_SHIELD:
            {
               pBot->iAimFlags |= AIM_DEST;

               // shield code courtesy of Wei Mingzhi
               if (BotHasShield (pBot))
               {
                  pBot->pBotPickupItem = NULL;
                  break;
               }

               // Near to Weapon ?
               else if (distance < 50)
               {
                  // Get current best weapon to check if it's a primary in need to be dropped
                  iWeaponNum = HighestWeaponOfEdict (pEdict);
                  if ((iWeaponNum > 6) || BotHasShield (pBot))
                  {
                     if (pBot->current_weapon.iId != cs_weapon_select[iWeaponNum].iId)
                     {
                        if (pBot->current_weapon.iId != CS_WEAPON_INSWITCH)
                           SelectWeaponByNumber (pBot, iWeaponNum);
                     }
                     else
                        FakeClientCommand (pEdict, "drop");
                  }
               }
               break;
            }
            case PICKUP_HOSTAGE:
            {
               pBot->iAimFlags |= AIM_ENTITY;
               v_src = GetGunPosition (pEdict);
               if (distance < ((v_dest.z - pEdict->v.origin.z > 20.0)? 90.0:70.0)) // KWo - 08.03.2010
               {
                  angle_to_entity = BotInFieldOfView (pBot, v_dest - v_src);

                  if (g_b_DebugEntities)
                     ALERT(at_logged, "[DEBUG] BotExecuteTask - TASK_PICKUPITEM - Bot %s found a hostage and is trying to pickup it.\n", pBot->name);

                  // Bot faces the hostage ?
                  if (angle_to_entity <= 30) // KWo - 04.05.2014
                  {
                     for (i = 0; i < g_iNumHostages; i++) // KWo - 17.05.2006
                     {
                        if ((pBot->pBotPickupItem == INDEXENT(HostagesData[i].EntIndex) && HostagesData[i].UserEntIndex == 0))
                        {
                           pEdict->v.button |= IN_USE;
                           HostagesData[i].UserEntIndex = ENTINDEX(pBot->pEdict);

                           if (g_b_DebugEntities) // KWo - 04.05.2014
                              ALERT(at_logged, "[DEBUG] BotExecuteTask - TASK_PICKUPITEM - Bot %s is pickuping the hostage %d.\n", pBot->name, i + 1);

//                           pBot->iAimFlags &= ~AIM_ENTITY; // KWo - 12.10.2006
                           if ((!g_b_cv_ffa) && (g_b_cv_chat)
                              && (g_iAliveCTs > 1) && (RANDOM_LONG(1,100) < 10)) // KWo - 06.03.2010
                              FakeClientCommand (pBot->pEdict, "say_team Trying to rescue a Hostage.\n"); // KWo - 06.03.2010

                           // Store ptr to hostage so other Bots don't steal
                           // from this one or Bot tries to reuse it
                           for (c = 0; c < g_iNumHostages; c++)
                           {
                              if (FNullEnt (pBot->pHostages[c]))
                              {
                                 pBot->pHostages[c] = pBot->pBotPickupItem;
                                 pBot->pBotPickupItem = NULL;
                                 break;
                              }
                           }
                           break;
                        }
      /*
                        else
                        {
                           if ((pBot->pBotPickupItem == INDEXENT(HostagesData[i].EntIndex) && HostagesData[i].UserEntIndex != 0)) // KWo - 04.05.2014
                           {
                              pBot->pItemIgnore[2] = pBot->pItemIgnore[1];
                              pBot->pItemIgnore[1] = pBot->pItemIgnore[0];
                              pBot->pItemIgnore[0] = pBot->pBotPickupItem;
                              if (g_b_DebugEntities)
                                 ALERT(at_logged, "[DEBUG] BotExecuteTask - TASK_PICKUPITEM - Bot %s is ignoring the hostage %d.\n", pBot->name, i + 1);
                           }
                        }
      */
                     }
                  }
               }
               break;
            }
            case PICKUP_PLANTED_C4:
            {
               // Bomb Defuse needs to be checked
               iFriendlyNum = NumTeammatesNearPos (pBot, pEdict->v.origin, 500); // KWo - 06.07.2008
               iEnemyNum = NumEnemiesNearPos (pBot, g_vecBomb, 800);             // KWo - 06.07.2008

               if (pBot->bot_team == TEAM_CS_COUNTER)
               {
                  if (((float)iEnemyNum <  0.8 * (float)iFriendlyNum)
                     || (g_bIgnoreEnemies) || (g_iAliveTs == 0) || (iEnemyNum == 0)
                     || (distance < 80) && (gpGlobals->time > g_fTimeBombPlanted + g_f_cv_c4timer - 12.0))  // KWo - 17.10.2008
                  {
                     iTempWpIndex1 = WaypointFindNearestToMove (pEdict, g_vecBomb);
                     if ((iTempWpIndex1 >= 0) && (iTempWpIndex1 < g_iNumWaypoints) && (iTempWpIndex1 != pBot->curr_wpt_index))
                     {
                        DeleteSearchNodes(pBot);
                        pBot->chosengoal_index = iTempWpIndex1;
                        bottask_t TempTask = {NULL, NULL, TASK_MOVETOPOSITION, TASKPRI_MOVETOPOSITION + 11.0, iTempWpIndex1, 0.0, TRUE}; // KWo - 02.09.2008
                        BotPushTask (pBot, &TempTask);
                     }
                     else if ((distance < 40) && (iTempWpIndex1 == pBot->curr_wpt_index))
                     {
                        pBot->iAimFlags |= AIM_ENTITY;
                        pBot->bMoveToGoal = FALSE;
                        pBot->bCheckTerrain = FALSE;
                        pBot->f_move_speed = 0.0;
                        pBot->f_sidemove_speed = 0.0;
                        pEdict->v.button |= IN_DUCK;

                        // Notify Team of defusing
                        if ((!g_b_cv_ffa) && (g_b_cv_radio)
                           && (g_iAliveTs > 0) && (g_iAliveCTs > 1)
                           && (RANDOM_LONG (1, 100) < 20)) // KWo - 06.03.2010
                           BotPlayRadioMessage (pBot, RADIO_NEEDBACKUP);

                        if ((!g_b_cv_ffa) && (g_b_cv_chat)
                           && (g_iAliveCTs > 1) && (RANDOM_LONG(1,100) < 10))  // KWo - 06.03.2010
                           FakeClientCommand (pBot->pEdict, "say_team Trying to defuse the Bomb!\n");

                        bottask_t TempTask = {NULL, NULL, TASK_DEFUSEBOMB, TASKPRI_DEFUSEBOMB, -1, 0.0, TRUE}; // KWo - 21.02.2007
                        BotPushTask (pBot, &TempTask);
                     }
                     else if ((distance < 80) && (iTempWpIndex1 == pBot->curr_wpt_index))
                     {
                        pBot->iAimFlags |= AIM_ENTITY;
                        pBot->bMoveToGoal = FALSE;
                        pBot->bCheckTerrain = TRUE;
                        if (GetShootingConeDeviation (pEdict, &v_dest) > 0.9)
                           pBot->f_move_speed = pEdict->v.maxspeed;
                        pBot->f_sidemove_speed = 0.0;
                        pEdict->v.button |= IN_DUCK;
                     }
                  }
               }
               break;
            }
            case PICKUP_DROPPED_C4: // KWo - 28.08.2008
            {
               if (pBot->bot_team == TEAM_CS_TERRORIST)
               {
                  pBot->iAimFlags |= AIM_ENTITY;
                  if (!(pBot->iAimFlags & AIM_ENEMY) && (GetShootingConeDeviation (pEdict, &v_dest) > 0.9))
                  {
                     if (distance > 100)
                     {
                        pBot->f_move_speed = pEdict->v.maxspeed;
                     }
                     else if (distance > 20)
                     {
                        pBot->f_move_speed = 0.5 * pEdict->v.maxspeed;
                     }
                     else
                        pBot->f_move_speed = 0.0;
                     pBot->f_sidemove_speed = 0.0;
                  }
               }
            }
            case PICKUP_DEFUSEKIT:
            {
               pBot->iAimFlags |= AIM_DEST; // don't bother looking at it

               if (pBot->b_has_defuse_kit)
               {
                  pBot->pBotPickupItem = NULL;
                  pBot->iPickupType = PICKUP_NONE;
               }
               break;
            }
         }
         break;
      } // end of TASK_PICKUPITEM
   } // end of all tasks execution

  /*****************************************
   * --- End of executing Task Actions --- *
   *****************************************/
}


void BotThink (bot_t *pBot)
{
   // This function gets called each Frame and is the core of
   // all Bot AI. From here all other Subroutines are called

   static edict_t *pEdict;
   static TraceResult tr;
   static bool bBotMovement;
   static bool bEndTask;
   static bool bStringUsed;
   static bool bPrevCrouch;
   static bool bPrevLadder;
   static bool bHostDebugPossible;
   static bool bDebugAllBots;
   static float f_msecval;
   static float fDistance;
   static float fDot;               // KWo - 17.04.2016
   static float fReachWpDelay;
   static int i;
   static int i_msecval;
   static int i_msecvalrest;
   static int iStringIndex;
   static int iCount;
   static int iTask;
   static Vector v_direction;
   static Vector vecDirectionNormal;
   static Vector vecDirection;
   static Vector vecMoveAngles;
   static Vector v_angles;
   static Vector v_dest;
   static Vector v_src;
   static Vector v_diff;
   static Vector vecSprayPos;
   static Vector v_bomb;
   static Vector2D v2_prev1;        // KWo - 17.04.2016
   static Vector2D v2_next1;        // KWo - 17.04.2016

   bBotMovement = FALSE;
   bEndTask = false;  // KWo - 05.08.2006
   pEdict = pBot->pEdict;

   pEdict->v.flags |= FL_FAKECLIENT;
   pBot->fTimeFrameInterval = gpGlobals->time - pBot->fTimePrevThink; // KWo - 17.10.2006 - reverted back
   pBot->fTimePrevThink = gpGlobals->time;

   pEdict->v.button = 0;
   pBot->f_move_speed = 0.0;
   pBot->f_sidemove_speed = 0.0;
   pBot->bDead = (IsAlive (pEdict) ? FALSE : TRUE);
   pBot->bCanChooseAimDirection = TRUE;

   pBot->bIsChickenOrZombie = ((strncmp ("chicken", pBot->sz_BotModelName, 7) == 0)
         || (strncmp ("zomb", pBot->sz_BotModelName, 4) == 0)); // KWo - 17.01.2011

   // Set some debug flags..
   bHostDebugPossible = ((pHostEdict) && ((g_vecHostOrigin - pEdict->v.origin).Length() < 30.0) && !(clients[0].iFlags & CLIENT_ALIVE));
   bDebugAllBots = ((g_i_cv_debuglevel & DEBUG_FL_ALLBOTS) > 0);
   g_b_DebugTasks = ((bHostDebugPossible || bDebugAllBots) && (g_i_cv_debuglevel & DEBUG_FL_TASKS));
   g_b_DebugNavig = ((bHostDebugPossible || bDebugAllBots) && (g_i_cv_debuglevel & DEBUG_FL_NAVIGATION));
   g_b_DebugStuck = ((bHostDebugPossible || bDebugAllBots) && (g_i_cv_debuglevel & DEBUG_FL_STUCK));
   g_b_DebugSensing = ((bHostDebugPossible || bDebugAllBots) && (g_i_cv_debuglevel & DEBUG_FL_SENSING));
   g_b_DebugCombat = ((bHostDebugPossible || bDebugAllBots) && (g_i_cv_debuglevel & DEBUG_FL_COMBAT));
   g_b_DebugEntities = ((bHostDebugPossible || bDebugAllBots) && (g_i_cv_debuglevel & DEBUG_FL_ENTITIES));
   g_b_DebugChat = ((bHostDebugPossible || bDebugAllBots) && (g_i_cv_debuglevel & DEBUG_FL_CHAT));

   // if the bot hasn't selected stuff to start the game yet, go do that...
   if (pBot->not_started)
      BotStartGame (pBot); // Select Team & Class

   // In Cs Health seems to be refilled in Spectator Mode that's why I'm doing this extra
   else if (pBot->bDead)
   {
      // Bot chatting turned on ?
      if ((g_b_cv_chat) && (clients[g_i_botthink_index].fDeathTime + 5.0 < gpGlobals->time)) // KWo - 27.03.2010
      {
         if (g_b_DebugChat)
            ALERT(at_logged, "[DEBUG] BotThink - Bot %s is going to reply to the player; bot's last chat time = %f, global = %f.\n",
               pBot->name, pBot->f_lastchattime, g_fLastChatTime);

         if (!BotRepliesToPlayer (pBot) && (g_fLastChatTime + 1.0 < gpGlobals->time)
            && (pBot->f_lastchattime + 3.0 < gpGlobals->time)) // KWo - 10.03.2013
         {
            pBot->f_lastchattime = gpGlobals->time; // exactly it gives the next time to check if the bot can chat....
            // Say a Text every now and then
            if (RANDOM_LONG (0, 100) < 10) // KWo - 06.03.2010
            {
               g_fLastChatTime = gpGlobals->time;

               // Rotate used Strings Array up - KWo - 23.03.2010 - remember indices only...
               iUsedDeadChatIndex[9] = iUsedDeadChatIndex[8];
               iUsedDeadChatIndex[8] = iUsedDeadChatIndex[7];
               iUsedDeadChatIndex[7] = iUsedDeadChatIndex[6];
               iUsedDeadChatIndex[6] = iUsedDeadChatIndex[5];
               iUsedDeadChatIndex[5] = iUsedDeadChatIndex[4];
               iUsedDeadChatIndex[4] = iUsedDeadChatIndex[3];
               iUsedDeadChatIndex[3] = iUsedDeadChatIndex[2];
               iUsedDeadChatIndex[2] = iUsedDeadChatIndex[1];
               iUsedDeadChatIndex[1] = iUsedDeadChatIndex[0];

               iStringIndex = RANDOM_LONG (0, iNumDeadChats - 1);
               bStringUsed = TRUE;
               iCount = 0;

               while (bStringUsed)
               {
                  iStringIndex = RANDOM_LONG (0, iNumDeadChats - 1);
                  bStringUsed = FALSE;

                  for (i = 0; i < 10; i++)
                  {
                     if (iUsedDeadChatIndex[i] == iStringIndex)
                     {
                        iCount++;
                        if (iCount < 30)
                           bStringUsed = TRUE;
                     }
                  }
               }

               // Save new String
               iUsedDeadChatIndex[0] = iStringIndex;
               BotPrepareChatMessage (pBot, szDeadChat[iStringIndex]);
               BotPushMessageQueue (pBot, MSG_CS_SAY);
            }
         }
      }
   }

   // Bot is still buying - don't move
   else if ((pBot->iBuyCount > 0) && (pBot->iBuyCount < 8) && pBot->b_can_buy)
   {
      if (pBot->f_buy_time < gpGlobals->time)
         BotBuyStuff (pBot);

      // Bot has spawned 3 secs ago ? Something went wrong in buying so cancel it
      if (pBot->f_spawn_time + 3.0 < gpGlobals->time)
         pBot->iBuyCount = 0;
   }
   else
      bBotMovement = TRUE;

   // Check for pending Messages - must be here, because it's executed while bot is dead, too (for radio - only when the bot is alive)
   if (g_iFrameCounter == g_i_botthink_index)
      BotCheckMessageQueue (pBot);

   if (!bBotMovement || (gpGlobals->time <= g_fTimeRoundStart) || g_bWaypointsChanged)
   {
      pBot->f_move_speed = 0.0;
      pBot->f_sidemove_speed = 0.0;
      pEdict->v.button = 0;
      i_msecval = (int) (gpGlobals->frametime * 1000); // KWo - 25.09.2006
      if (i_msecval < 1)
         i_msecval = 1; // don't allow the msec delay to be null
      else if (i_msecval > 100)
         i_msecval = 100; // don't allow it to last longer than 100 milliseconds either
      pBot->i_msecval = i_msecval; // KWo - 17.03.2007
      pBot->f_msecvalrest = 0.0; // KWo - 17.03.2007
      g_engfuncs.pfnRunPlayerMove (pEdict, pEdict->v.v_angle, pBot->f_move_speed, pBot->f_sidemove_speed, 0, pEdict->v.button, 0, (unsigned char) i_msecval);	// KWo - 25.09.2006 - thanks to THE_STORM
      return;
   }

   if (pBot->bCheckMyTeam)
   {
      pBot->bot_team = UTIL_GetTeam (pEdict);
      pBot->bCheckMyTeam = FALSE;
   }

   pBot->bOnLadder = (pEdict->v.movetype == MOVETYPE_FLY);
   pBot->bInWater = ((pEdict->v.waterlevel == 2) || (pEdict->v.waterlevel == 3));

   // Check if we already switched weapon mode
   if (pBot->bCheckWeaponSwitch && (pBot->f_spawn_time + 4.0 < gpGlobals->time))
   {
      if (BotHasShield (pBot))
      {
         if (BotHasShieldDrawn (pBot))
            pEdict->v.button |= IN_ATTACK2;
      }
      else
      {
         // Bot owns a Sniper Weapon ? Try to switch back to no zoom so Bot isn't moving slow
         if (BotUsesSniper (pBot) && (pEdict->v.fov < 90))
            pEdict->v.button |= IN_ATTACK2;

         // If no sniper weapon use a secondary mode at random times
         else
         {
            if (((pBot->current_weapon.iId == CS_WEAPON_M4A1) || (pBot->current_weapon.iId == CS_WEAPON_USP))
                && (pBot->fTimeSilencerSwitch < gpGlobals->time + 30.0)
                && (FNullEnt(pBot->pBotEnemy))) // KWo - 09.04.2010
            {
               // Aggressive bots don't like the silencer (courtesy of Wei Mingzhi - good idea)
               if (RANDOM_LONG (1, 100) <= (pBot->bot_personality == 1 ? 25 : 75))
               {
                  if (pEdict->v.weaponanim > 6) // is the silencer not attached...
                     pEdict->v.button |= IN_ATTACK2; // attach the silencer
               }
               else
               {
                  if (pEdict->v.weaponanim <= 6) // is the silencer attached...
                     pEdict->v.button |= IN_ATTACK2; // detach the silencer
               }
               pBot->fTimeSilencerSwitch = gpGlobals->time; // KWo - 08.01.2007
            }
         }
/*
         // If Psycho Bot switch to Knife
         if ((pBot->bot_skill >= 40) && ((pBot->bot_personality == 1)
            || ((pBot->bot_personality == 0) && (RANDOM_LONG (1, 100) < 50)))
            && (pBot->current_weapon.iId != CS_WEAPON_KNIFE))
            SelectWeaponByName (pBot, "weapon_knife");
*/
      }

      if ((RANDOM_LONG (1, 100) < 20) && (g_b_cv_spray)) // KWo - 06.04.2006
      {
         bottask_t TempTask = {NULL, NULL, TASK_SPRAYLOGO, TASKPRI_SPRAYLOGO, -1, gpGlobals->time + 1.0f, FALSE};
         BotPushTask (pBot, &TempTask);
      }

      // Select a Leader Bot for this team
      if (!g_b_cv_ffa) // KWo - 05.10.2006
         SelectLeaderEachTeam (pBot);

      pBot->bCheckWeaponSwitch = FALSE;
   }

   // FIXME: The following Timers aren't frame independant so it varies on slower/faster computers
   // Increase Reaction Time
   pBot->f_actual_reaction_time += 0.2;
   if (pBot->f_actual_reaction_time > pBot->f_ideal_reaction_time)
      pBot->f_actual_reaction_time = pBot->f_ideal_reaction_time;

   // Bot could be blinded by FlashBang or Smoke, recover from it
   pBot->f_view_distance += 3.0;
   if (pBot->f_view_distance > pBot->f_maxview_distance)
      pBot->f_view_distance = pBot->f_maxview_distance;

   // Maxspeed is set in ClientSetMaxspeed

   if ((pBot->cCollideMoves[(int) pBot->cCollStateIndex] != COLLISION_GOBACK)
      && (pBot->cCollideMoves[(int) pBot->cCollStateIndex] != COLLISION_STRAFELEFT)
      && (pBot->cCollideMoves[(int) pBot->cCollStateIndex] != COLLISION_STRAFERIGHT)
      && !(pBot->bPlayerCollision)) // KWo - 13.09.2008
      pBot->f_move_speed = pEdict->v.maxspeed;

   if (pBot->prev_time <= gpGlobals->time)
   {
      // see how far bot has moved since the previous position...
      v_diff = pBot->v_prev_origin - pEdict->v.origin;
      pBot->f_moved_distance = v_diff.Length ();

      // save current position as previous
      pBot->v_prev_origin = pEdict->v.origin;
//      pBot->prev_time = gpGlobals->time + 0.2; moved below BotCheckTerrain call...
   }
/*
   else
      pBot->f_moved_distance = 2.0;
*/

   // If there's some Radio Message to respond, check it
   if ((pBot->iRadioOrder != 0) && (g_iFrameCounter == g_i_botthink_index))
      BotCheckRadioCommands (pBot);


   // Do all Sensing, calculate/filter all Actions here
   BotSetConditions (pBot);

   // Check if the bot can throw a grenade
   BotCheckGrenadeThrow (pBot); // KWo - 11.04.2010

   // Change to the best weapon if heard something - moved - KWo - 15.01.2012
   if (!(g_b_cv_jasonmode) && !(pBot->bUsingGrenade)
        && ((pBot->iStates & STATE_HEARINGENEMY) && (BotGetSafeTask(pBot)->iTask != TASK_PLANTBOMB)
          && (BotGetSafeTask(pBot)->iTask != TASK_DEFUSEBOMB)
          || (pBot->iStates & STATE_SEEINGENEMY) && (!pBot->bIsChickenOrZombie || !pBot->b_bomb_blinking)
          || (pBot->f_jumptime + 3.0 < gpGlobals->time) && (pBot->f_jumptime + 4.0 > gpGlobals->time))
        /* && (!pBot->bIsReloading) */
        && (pBot->fTimeWeaponSwitch + ((pBot->iStates & STATE_SEEINGENEMY) ? 1.0 : 3.0) < gpGlobals->time)) // KWo - 22.01.2012
            BotSelectBestWeapon (pBot);

   // Check if there are Items needing to be used/collected
   if (((pBot->f_itemcheck_time < gpGlobals->time) || !FNullEnt (pBot->pBotPickupItem))
      && (g_i_botthink_index == g_iFrameCounter)) // KWo - 11.04.2008
   {
      pBot->f_itemcheck_time = gpGlobals->time + g_f_cv_timer_pickup;  // KWo - 06.04.2006
      BotFindItem (pBot);
   }

   // Find the best task to push on tasks stack
   BotPrepareTask (pBot); // KWo - 10.04.2010

   // check if the bot should use the flashlight or nightvision
   BotCheckDarkness (pBot);

   pBot->bCheckTerrain = TRUE;
   pBot->bWantsToFire = FALSE;

   // Get affected by SmokeGrenades (very basic!) and sense Grenades flying towards us
   BotCheckAvoidGrenades (pBot);          // KWo - 13.04.2010

   // check if need to change the shield position
   BotCheckShield (pBot);

//   if (g_b_DebugTasks)
//      ALERT(at_logged, "[DEBUG] BotThink - Bot %s has task deep = %i, path deep = %i.\n", pBot->name, pBot->i_TaskDeep, pBot->i_PathDeep);

   if (g_b_DebugTasks)
   {
      ALERT(at_logged,"[DEBUG] BotThink - Bot %s - before task exec - Current WP index = %i, Goal index = %i , State = %i, Task = %s, AimFlags = %i.\n", pBot->name,
         pBot->curr_wpt_index, pBot->chosengoal_index, pBot->iStates, g_TaskNames[pBot->pTasks->iTask], pBot->iAimFlags);
   }

   // execute the task to do
   BotExecuteTask (pBot);                 // KWo - 12.04.2010

   // choose the aim direction
   BotChooseAimDirection (pBot);

   // look at the certain position
   BotFacePosition (pBot, pBot->vecLookAt); // KWo - 13.04.2010


   // Change Body Angles
   UTIL_ClampVector (&pEdict->v.v_angle);
   pEdict->v.angles.x = -pEdict->v.v_angle.x * (1.f / 3.f);
   pEdict->v.angles.y = pEdict->v.v_angle.y;
   UTIL_ClampVector (&pEdict->v.angles);
   pEdict->v.angles.z = pEdict->v.v_angle.z = 0.0;

   // Enemy behind Obstacle ? Bot wants to fire ?
   if ((pBot->iStates & STATE_SUSPECTENEMY) && pBot->bWantsToFire)
   {
      iTask = BotGetSafeTask(pBot)->iTask;

      // Don't allow shooting through walls when camping
      if ((iTask == TASK_PAUSE) || (iTask == TASK_CAMP))
         pBot->bWantsToFire = FALSE;
   }

   // The Bot wants to fire at something ?
   if (pBot->bWantsToFire && !pBot->bUsingGrenade
      && (pBot->f_shoot_time <= gpGlobals->time) && (pBot->f_spawn_time + 0.5 < gpGlobals->time)) // KWo - 10.07.2008
   {
      // If Bot didn't fire a bullet try again next frame
      BotFireWeapon (pBot->vecLookAt - GetGunPosition (pEdict), pBot);  // KWo - 23.10.2006
   }

   // Set the reaction time (surprise momentum) different each frame
   // according to skill
   pBot->f_ideal_reaction_time = RANDOM_FLOAT (BotSkillDelays[pBot->bot_skill / 20].fMinSurpriseDelay,
      BotSkillDelays[pBot->bot_skill / 20].fMaxSurpriseDelay);



   if (g_b_DebugTasks || g_b_DebugCombat)
   {
      if (g_b_DebugCombat)
      {
         int iId = pBot->current_weapon.iId;
         char szWeapClassname[64];
         if (iId < 32)
         {
            snprintf (szWeapClassname, 63, weapon_defs[iId].szClassname);
         }
         else
         {
            snprintf (szWeapClassname, 15, "unknown weapon");
         }

         int iWeaponSecNum = HighestPistolOfEdict (pEdict);
         int iWeaponPrimNum = HighestWeaponOfEdict(pEdict);
         int iWeaponSecID = cs_weapon_select[iWeaponSecNum].iId;
         int iWeaponPrimID = cs_weapon_select[iWeaponPrimNum].iId;

         if ((pBot->iAimFlags & AIM_ENEMY) && (fabs(pBot->pEdict->v.v_angle.x - pEdict->v.idealpitch) < 0.1)
            && (fabs(pBot->pEdict->v.v_angle.y - pEdict->v.ideal_yaw) < 0.1) && !(pEdict->v.button & IN_ATTACK))
         {
            ALERT(at_logged,"[DEBUG] BotThink - Bot %s has primary weapon %s with %d ammo in clip and sec weapon %s with %d ammo in clip; current weapon - %s.\n",
               pBot->name, weapon_defs[iWeaponPrimID].szClassname, pBot->m_rgAmmoInClip[iWeaponPrimID],
               weapon_defs[iWeaponSecID].szClassname, pBot->m_rgAmmoInClip[iWeaponSecID], szWeapClassname);
            ALERT(at_logged,"[DEBUG] BotThink - Bot %s %s to fire; bot %s reloading; Shoot time = %.2f, Time = %.2f .\n",
               pBot->name, pBot->bWantsToFire ? "wants": "doesn't want", pBot->bIsReloading ? "is" : "isn't",
               pBot->f_shoot_time, gpGlobals->time);

         }
      }

      if (g_b_DebugTasks)
      {
         ALERT(at_logged,"[DEBUG] BotThink - Bot %s - Current WP index = %i, Goal index = %i , State = %i, Task = %s, AimFlags = %i.\n", pBot->name,
               pBot->curr_wpt_index, pBot->chosengoal_index, pBot->iStates, g_TaskNames[pBot->pTasks->iTask], pBot->iAimFlags);
      }
   }

   // Calculate 2 direction Vectors, 1 without the up/down component
   iTask = BotGetSafeTask(pBot)->iTask;
   if ((iTask == TASK_ATTACK) || (iTask == TASK_THROWHEGRENADE)
       || (iTask == TASK_THROWFLASHBANG) || (iTask == TASK_THROWSMOKEGRENADE)) // KWo - 01.10.2008
      v_direction = pBot->vecLookAt - (pEdict->v.origin + (pEdict->v.velocity * pBot->fTimeFrameInterval));
   else
      v_direction = pBot->dest_origin - (pEdict->v.origin + (pEdict->v.velocity * pBot->fTimeFrameInterval));

   vecDirectionNormal = v_direction.Normalize ();
   vecDirection = vecDirectionNormal;
   vecDirectionNormal.z = 0.0;

   vecMoveAngles = UTIL_VecToAngles (v_direction);
   vecMoveAngles.x = -vecMoveAngles.x;
   vecMoveAngles.z = 0;
   UTIL_ClampVector (&vecMoveAngles);


   // Allowed to move to a destination position ?
   if (pBot->bMoveToGoal)
   {
      GetValidWaypoint (pBot);

      bPrevCrouch = FALSE;  // KWo - 17.09.2006
      bPrevLadder = FALSE;  // KWo - 17.09.2006
      fDistance = 0.0;     // KWo - 13.01.2008

      if ((pBot->prev_wpt_index[0] >= 0) && (pBot->prev_wpt_index[0] < g_iNumWaypoints)) // KWo - 17.09.2006
      {
         if (paths[pBot->prev_wpt_index[0]]->flags & W_FL_CROUCH)
            bPrevCrouch = TRUE;
         if (paths[pBot->prev_wpt_index[0]]->flags & W_FL_LADDER)
            bPrevLadder = TRUE;
      }

      // Press duck button if we need to
      if (((paths[pBot->curr_wpt_index]->flags & W_FL_CROUCH))
          /* && (!(paths[pBot->curr_wpt_index]->flags & W_FL_CAMP) || bPrevCrouch) */
          && (((pEdict->v.flags & FL_ONGROUND) && !pBot->bInWater && !pBot->bOnLadder 
          /* && !(paths[pBot->curr_wpt_index]->flags & W_FL_LADDER) && !bPrevLadder */) // KWo - 06.04.2016
             || (paths[pBot->curr_wpt_index]->origin.z < pEdict->v.origin.z + 64))) // KWo - 19.02.2008 - reverted back
      {
         pEdict->v.button |= IN_DUCK;
         pBot->f_ducktime = gpGlobals->time + 1.0; // KWo - 22.01.2012
      }

      if ((paths[pBot->curr_wpt_index]->flags & W_FL_CROUCH) && (paths[pBot->curr_wpt_index]->flags & W_FL_LADDER)
          &&  !bPrevLadder && (pEdict->v.flags & FL_ONGROUND) || pBot->bOnLadder && (pEdict->v.flags & FL_ONGROUND)) // KWo - 08.04.2016
      {
         pEdict->v.button &= ~IN_DUCK;
         pBot->f_ducktime = 0.0;
         pBot->iCampButtons = 0;
      }

      // Press jump button if we need to leave the ladder
      if ((!(paths[pBot->curr_wpt_index]->flags & W_FL_LADDER)) &&  bPrevLadder
          && (pEdict->v.flags & FL_ONGROUND) && pBot->bOnLadder 
          && (pBot->f_move_speed > 50.0) && (pEdict->v.velocity.Length() < 50.0)) // KWo - 09.04.2016
      {
         pEdict->v.button |= IN_JUMP;
         pBot->f_jumptime = gpGlobals->time + 1.0;
      }

      fDistance = (pBot->dest_origin - (pEdict->v.origin + (pEdict->v.velocity * pBot->fTimeFrameInterval))).Length2D (); // KWo - 17.10.2006 - reverted back

// KWo - check if the direction change is really strong and near WP with low radius
      if ((pBot->prev_wpt_index[0] >= 01) && (pBot->prev_wpt_index[0] < g_iNumWaypoints)
            && (pBot->curr_wpt_index > -1) && (pBot->curr_wpt_index < g_iNumWaypoints)
            && (pBot->pEdict->v.movetype != MOVETYPE_FLY) && (pBot->pWaypointNodes != NULL)
            && (!(pBot->curr_travel_flags & C_FL_JUMP)))  // KWo - 18.04.2016
      {
         if ((paths[pBot->curr_wpt_index]->Radius < 17) && (pBot->pWaypointNodes->NextNode != NULL) 
         && (fDistance < paths[pBot->curr_wpt_index]->Radius + 20.0))
         {
            if ((pBot->pWaypointNodes->NextNode->iIndex >= 0) && (pBot->pWaypointNodes->NextNode->iIndex < g_iNumWaypoints))
            {
               v2_next1 = (paths[pBot->curr_wpt_index]->origin - paths[pBot->pWaypointNodes->NextNode->iIndex]->origin).Make2D().Normalize();
               v2_prev1 = (paths[pBot->curr_wpt_index]->origin - paths[pBot->prev_wpt_index[0]]->origin).Make2D().Normalize();
               fDot = DotProduct(v2_prev1, v2_next1);

               if (g_b_DebugNavig)
               {
                  ALERT (at_logged, "[DEBUG] BotThink - Bot %s checks the direction change in movement, fDot = %0.3f.\n",
                     pBot->name, fDot);
               }

               if (fDot > -0.6)
                  pBot->f_wpt_tim_str_chg = gpGlobals->time + 1.0;
            }
         }
      }


      if ((pEdict->v.flags & FL_DUCKING) || (pBot->f_ducktime > gpGlobals->time))  // KWo - 08.04.2016
      {
         pBot->f_move_speed = pEdict->v.maxspeed;
      }
      else if ((paths[pBot->curr_wpt_index]->flags & W_FL_LADDER) && (!pBot->bOnLadder)
            && (pEdict->v.flags & (FL_ONGROUND | FL_PARTIALGROUND)))  // KWo - 15.04.2016
      {

         if (!bPrevLadder)
            pBot->f_move_speed = (pEdict->v.origin - pBot->wpt_origin).Length ();  // Slowly approach the ladder
         else
            pBot->f_move_speed = 150.0;

         if (pBot->f_move_speed < 150.0)
            pBot->f_move_speed = 150.0;
         else if (pBot->f_move_speed > pEdict->v.maxspeed)
            pBot->f_move_speed = pEdict->v.maxspeed;
//         if (bPrevLadder)
//            pBot->f_move_speed = 80.0;    // KWo - 24.01.2012
      }
      else if ((pBot->cCollisionState == COLLISION_NOTDECIDED) && (pBot->f_move_speed != 0.0)
      && (!(paths[pBot->curr_wpt_index]->flags & W_FL_LADDER))
      /*   && !(pEdict->v.flags & FL_DUCKING) && (pBot->f_ducktime < gpGlobals->time) */ ) // KWo - 07.04.2016
      {
         if (pBot->f_wpt_tim_str_chg > gpGlobals->time) // KWo - 17.04.2016
         {
            if (pBot->f_move_speed > 0)
               pBot->f_move_speed = 80.0;
            else
               pBot->f_move_speed = -80.0;
         }
         else if ((pBot->curr_wpt_index == BotGetSafeTask(pBot)->iData) || (BotGetSafeTask(pBot)->iData == -1)) // KWo - 17.04.2016
         {
            if (fDistance < 50.0)
               pBot->f_move_speed = 80;
            else if (fDistance < 10.0)
               pBot->f_move_speed = 0.0;
         }
/*
         else if (pBot->f_move_speed >= 0.0)
         {
            if ((pBot->curr_wpt_index != BotGetSafeTask(pBot)->iData) && (pBot->f_move_speed < 80.0))
               pBot->f_move_speed = 80.0;
            else if (pBot->f_move_speed < 2.0)
               pBot->f_move_speed = 2.0;
         }
*/
      }
      else if ((pBot->cCollisionState == COLLISION_NOTDECIDED) && (fDistance < pBot->f_move_speed * pBot->fTimeFrameInterval)
         && (BotGetSafeTask(pBot)->iData != -1))  // KWo - 07.04.2016
         pBot->f_move_speed = fDistance;

      if (pBot->f_move_speed > pEdict->v.maxspeed)
         pBot->f_move_speed = pEdict->v.maxspeed;


      pBot->fTimeWaypointMove = gpGlobals->time;

      // Special Movement for swimming here
      if (pBot->bInWater) // KWo - 17.10.2006 - reverted back
      {
         // Check if we need to go forward or back
         // Press the correct buttons
         if (BotInFieldOfView (pBot, pBot->dest_origin - GetGunPosition (pEdict)) > 90)
            pEdict->v.button |= IN_BACK;
         else
            pEdict->v.button |= IN_FORWARD;

         if (vecMoveAngles.x > 60.0)
            pEdict->v.button |= IN_DUCK;
         else if (vecMoveAngles.x < -48.0) // KWo - 28.04.2010
            pEdict->v.button |= IN_JUMP;
      }
   }


   if (g_b_DebugStuck)
   {
      ALERT (at_logged,"[DEBUG] BotThink - Bot %s %s able to check the terrain.\n", pBot->name, pBot->bCheckTerrain ? "is" : "is not");
   }

   // Are we allowed to check blocking Terrain (and react to it) ?
   if (((pBot->bCheckTerrain) || (pBot->cCollisionState == COLLISION_PROBING) || (pBot->f_probe_time >= gpGlobals->time))
      && (pBot->f_spawn_time + (((g_fTimeRoundEnd + g_f_cv_FreezeTime > gpGlobals->time) 
      && (g_fTimeRoundEnd < gpGlobals->time)) ? (g_f_cv_FreezeTime + 0.0):(0.2)) < gpGlobals->time)) // KWo - 28.05.2010
   {
      BotCheckTerrain(pBot);
   }

   if (pBot->f_probe_time + 1.0 < gpGlobals->time)
   {
      pBot->bPlayerCollision = FALSE; // KWo - 11.07.2006
   }

   if ((pBot->bHitDoor) && (pBot->f_timeHitDoor + 5.0 < gpGlobals->time)) // KWo - 17.01.2010
   {
      pBot->bHitDoor = FALSE;
   }


// moved here writing the new time to check if the bots gets stuck or not
   if (pBot->prev_time <= gpGlobals->time)
   {
      pBot->prev_time = gpGlobals->time + 0.3; // KWo - 19.03.2010
   }

   // Must avoid a Grenade ?
   if (pBot->cAvoidGrenade != 0)
   {
      // Don't duck to get away faster
      pEdict->v.button &= ~IN_DUCK;
      pBot->iCampButtons = 0; // KWo - 17.02.2008
      pBot->f_ducktime = 0.0; // KWo - 17.02.2008

      v_angles = pEdict->v.v_angle;
      MAKE_VECTORS (v_angles);  // KWo - 28.08.2008
      Vector vecForward = (gpGlobals->v_forward * (-pEdict->v.maxspeed)) * 0.2;
      Vector vecSide = (gpGlobals->v_right * pEdict->v.maxspeed * pBot->cAvoidGrenade) * 0.2;
      Vector vecTargetPos = pEdict->v.origin + vecForward + vecSide + (pEdict->v.velocity * pBot->fTimeFrameInterval);

      if (!IsDeadlyDropAtPos (pBot, vecTargetPos))
      {
         pBot->f_sidemove_speed = pEdict->v.maxspeed * pBot->cAvoidGrenade;
         pBot->f_move_speed = -pEdict->v.maxspeed;

         if (g_b_DebugCombat)
         {
            ALERT(at_logged, "[DEBUG] BotThink - Bot %s must avoid a grenade; cAvoidGrenade = %d ; time = %f.\n", pBot->name, (int)pBot->cAvoidGrenade, gpGlobals->time);
         }
      }
   }



   if (g_b_DebugCombat)
   {
      ALERT(at_logged, "[DEBUG] BotThink - Bot %s - Curr. WP = %i, Goal WP = %i, Dist. to dest = %d, State = %i, Task = %s, AimFlags = %i,\n"
                       "[DEBUG] BotThink - Coll St = %i, Coll moves = %i, Vec look at = {%d, %d, %d}, ChAimDirT = %.2f, Time = %.2f. \n",
            pBot->name, pBot->curr_wpt_index, BotGetSafeTask(pBot)->iData, (int)(pBot->dest_origin - pEdict->v.origin).Length(),
            pBot->iStates, g_TaskNames[pBot->pTasks->iTask], pBot->iAimFlags,
            pBot->cCollisionState, pBot->cCollideMoves[(int) pBot->cCollStateIndex], (int)pBot->vecLookAt.x, (int)pBot->vecLookAt.y, (int)pBot->vecLookAt.z,
            pBot->fChangeAimDirectionTime, gpGlobals->time);

      ALERT(at_logged, "[DEBUG] BotThink - Bot %s - look. pos.: v_angle = [%.1f, %.1f], ideal_ang = [%.1f, %.1f], ang_dev = [%.1f, %.1f], FrTimeInt = %.2f, aim_speed [%.2f, %.2f].\n",
         pBot->name, pBot->pEdict->v.v_angle.x, pBot->pEdict->v.v_angle.y, pBot->randomized_ideal_angles.x, pBot->randomized_ideal_angles.y,
         pBot->angular_deviation.x, pBot->angular_deviation.y, pBot->fTimeFrameInterval, pBot->aim_speed.x, pBot->aim_speed.y);
/*
      if (pHostEdict)
      {
         char szMessage[256];
         g_hudset.x = -1.0;
         g_hudset.y = 0.8;
//       g_hudset.a1 = 1;
         g_hudset.holdTime = 0.1;
         g_hudset.channel = 2;
         float f_Angle = sqrt((fabsf(pBot->pEdict->v.punchangle.y) * M_PI / 180.0) * (fabsf(pBot->pEdict->v.punchangle.y) * M_PI / 180.0)
            + (fabsf(pBot->pEdict->v.punchangle.x) * M_PI / 180.0) * (fabsf(pBot->pEdict->v.punchangle.x) * M_PI / 180.0));  // KWo - 06.04.2010
         float f_Distance = 9999.0;
         if (!FNullEnt(pBot->pBotEnemy))
            f_Distance = (pEdict->v.origin - pBot->pBotEnemy ->v.origin).Length();
         sprintf (szMessage, "State = %i, AimFlags = %i, Task = %s, AggrLevel = %.2f, FearLevel = %.2f \n  EnHunt Des = %.0f, f_Angle = %.1f, Offset = %.1f, WP = %i, Time = %.2f.\n",
            pBot->iStates, pBot->iAimFlags, g_TaskNames[pBot->pTasks->iTask], pBot->fAgressionLevel, pBot->fFearLevel,
            taskFilters[TASK_ENEMYHUNT].fDesire, f_Angle, f_Distance * tanf(f_Angle), pBot->curr_wpt_index, gpGlobals->time);

         UTIL_HudMessage(pHostEdict, g_hudset, szMessage);
      }
*/
   }


   // FIXME: time to reach waypoint should be calculated when getting this waypoint
   // depending on maxspeed and movetype instead of being hardcoded
   // KWo - roger that! done :)
   fReachWpDelay = 5.0;
   // checks what the time the bot needs to reach the waypoint
   if ((pBot->curr_wpt_index >= 0) && (pBot->curr_wpt_index < g_iNumWaypoints) // KWo - 26.07.2006
        && (pBot->prev_wpt_index[0] >= 0) && (pBot->prev_wpt_index[0] < g_iNumWaypoints))
   {
      fDistance = (paths[pBot->prev_wpt_index[0]]->origin - paths[pBot->curr_wpt_index]->origin).Length();
//      if (pBot->pEdict->v.maxspeed <= 0.0) // KWo - 14.04.2016
      if (pBot->f_move_speed <= 0.0) // KWo - 14.04.2016
         fReachWpDelay = 5.0 * fDistance / 240.0;
      else
//         fReachWpDelay = 5.0 * fDistance / pBot->pEdict->v.maxspeed; // KWo - 14.04.2016
         fReachWpDelay = 5.0 * fDistance / fabs(pBot->f_move_speed); // KWo - 14.04.2016
      if ((paths[pBot->curr_wpt_index]->flags & W_FL_CROUCH) || (paths[pBot->curr_wpt_index]->flags & W_FL_LADDER) || (pBot->pEdict->v.button & IN_DUCK))
         fReachWpDelay *= 3.f;
      if (fReachWpDelay < 3.0)
         fReachWpDelay = 3.0;
      if (fReachWpDelay > 10.0)
         fReachWpDelay = 10.0;
   }
   if ((pBot->f_jumptime + 10.0 > gpGlobals->time) || (pBot->f_probe_time + 3.0 > gpGlobals->time))
      fReachWpDelay = 10.0;
   if ((pBot->f_wpt_timeset + fReachWpDelay < gpGlobals->time) && FNullEnt (pBot->pBotEnemy) && pBot->bMoveToGoal) // KWo - 28.08.2008
   {
      if (g_b_DebugNavig)
      {
//         if (g_iFrameCounter == g_i_botthink_index)
         ALERT(at_logged, "[DEBUG] BotThink - Bot %s didn't reach the WP with fReachWpDelay = %f.\n", pBot->name, fReachWpDelay);
      }

      GetValidWaypoint (pBot);

      // Clear these pointers, Bot might be stuck getting to them
      if (!FNullEnt (pBot->pBotPickupItem) && (pBot->iPickupType != PICKUP_PLANTED_C4))
      {
         pBot->pItemIgnore[2] = pBot->pItemIgnore[1];
         pBot->pItemIgnore[1] = pBot->pItemIgnore[0];
         pBot->pItemIgnore[0] = pBot->pBotPickupItem;
      }

      pBot->pBotPickupItem = NULL;
      pBot->iPickupType = PICKUP_NONE;
      pBot->f_itemcheck_time = gpGlobals->time + 5.0;
      pBot->pShootBreakable = NULL;
   }

   if (g_b_DebugNavig || g_b_DebugStuck || g_b_DebugTasks)
   {
      ALERT(at_logged, "[DEBUG] BotThink - Bot %s , Dist. to dest = %d, State = %i, Task = %s, AimFlags = %i, speed = %d,\n"
                       "Coll St = %d, Coll moves = %s, Time = %.2f. \n",
               pBot->name, (int)(pBot->dest_origin - pEdict->v.origin).Length(),
               pBot->iStates, g_TaskNames[pBot->pTasks->iTask], pBot->iAimFlags, (int)pBot->f_move_speed,
               pBot->cCollisionState, g_CollideMoveNames[(int) pBot->cCollideMoves[(int) pBot->cCollStateIndex]],
               gpGlobals->time);
   }


   if (g_b_DebugCombat)
   {
      if (!FNullEnt(pBot->pBotEnemy))
      {
         ALERT(at_logged, "[DEBUG] BotThink - Bot %s - its enemy - %s\n", pBot->name, STRING(pBot->pBotEnemy->v.netname));
      }
   }


   if (pBot->f_ducktime > gpGlobals->time) // KWo - 20.10.2006
   {
      pEdict->v.button |= IN_DUCK;
   }
   else
   {
      pEdict->v.button &= ~IN_DUCK;
   }

/*   if ((pEdict->v.button & IN_JUMP) || (pEdict->v.oldbuttons & IN_JUMP))
      pBot->f_jumptime = gpGlobals->time;
*/
   if (pBot->f_jumptime > gpGlobals->time) // KWo - 09.04.2016
   {
      if (!(pEdict->v.flags & FL_ONGROUND) && !pBot->bInWater && !pBot->bOnLadder)
      {
         pEdict->v.button |= IN_DUCK;
//         pBot->f_ducktime = gpGlobals->time + 1.0; // KWo - cannot be here - otherwise jumping is not working correctly
      }
   }
   if ((pEdict->v.movetype == MOVETYPE_FLY) || (pEdict->v.velocity.z > -50.0)
      || (pEdict->v.flags & (FL_ONGROUND | FL_PARTIALGROUND))) // KWo - 05.03.2010 (parachute support :) )
      pBot->f_falldowntime = 0.0;
   else if (pBot->f_falldowntime == 0.0)
      pBot->f_falldowntime = gpGlobals->time;

   if ((pBot->f_falldowntime + 0.5 < gpGlobals->time)
       && (pBot->f_falldowntime != 0.0) && g_b_cv_Parachute) // KWo - 07.03.2010 (parachute support :) )
      pEdict->v.button |= IN_USE;

   if (pBot->f_move_speed > 0.0)
      pEdict->v.button |= IN_FORWARD;
   if (pBot->f_move_speed < 0.0)
      pEdict->v.button |= IN_BACK;

   if (pBot->f_sidemove_speed > 0.0)
      pEdict->v.button |= IN_MOVERIGHT;
   if (pBot->f_sidemove_speed < 0.0)
      pEdict->v.button |= IN_MOVELEFT;

   // save the previous speed (for checking if stuck)
   pBot->prev_speed = fabs (pBot->f_move_speed);

   // Reset Damage
   pBot->iLastDamageType = -1;

   if (g_b_DebugNavig || g_b_DebugStuck)
   {
      ALERT (at_logged,"[DEBUG] BotThink - Bot %s move speed %.0f, moved distance = %i, dist to dest = %i, wp index = %d, TASK = %s, time = %.2f.\n",
                     pBot->name, pBot->f_move_speed, (int)pBot->f_moved_distance, (int)(pBot->pEdict->v.origin - pBot->dest_origin).Length(),
                     pBot->curr_wpt_index, g_TaskNames[iTask], gpGlobals->time);
   }
   if ((g_b_DebugEntities) && (BotGetSafeTask(pBot)->iTask == TASK_SHOOTBREAKABLE))
   {
      ALERT(at_logged, "[DEBUG] BotThink - Bot %s is trying to look at = [%i,%i,%i].\n",
         pBot->name, (int)pBot->vecLookAt.x, (int)pBot->vecLookAt.y, (int)pBot->vecLookAt.z);
   }

/*
   UTIL_ClampVector (&pEdict->v.angles);
   UTIL_ClampVector (&pEdict->v.v_angle);
   UTIL_ClampAngle (&pEdict->v.idealpitch);
   UTIL_ClampAngle (&pEdict->v.ideal_yaw);
*/
   assert ((pEdict->v.v_angle.x >= -180.0) && (pEdict->v.v_angle.x < 180.0)
           && (pEdict->v.v_angle.y >= -180.0) && (pEdict->v.v_angle.y < 180.0)
           && (pEdict->v.angles.x >= -180.0) && (pEdict->v.angles.x < 180.0)
           && (pEdict->v.angles.y >= -180.0) && (pEdict->v.angles.y < 180.0));

// This is PMB method of computing msec value
//   f_msecval = gpGlobals->frametime * 1000.0; // KWo - 17.03.2007

//   f_msecval = static_cast <byte> ((gpGlobals->time - m_fMsecInterval) * 1000.0);
// this is Leon Hartwig's method for computing the msec value
   f_msecval = (gpGlobals->time - pBot->fTimePrevThink2) * 1000.0;
   pBot->fTimePrevThink2 = gpGlobals->time;

   i_msecvalrest = 0;                         // KWo - 17.03.2007
   i_msecval = (int) (f_msecval);             // KWo - 17.03.2007
   if (i_msecval < 10)                        // KWo - 17.03.2007
   {
      f_msecval = f_msecval - (float)(i_msecval) + pBot->f_msecvalrest;
      i_msecvalrest = (int)(f_msecval);
      pBot->f_msecvalrest = f_msecval - (float)(i_msecvalrest);
   }
   i_msecval = i_msecval + i_msecvalrest;

   if (i_msecval < 1)
      i_msecval = 1; // don't allow the msec delay to be null
   else if (i_msecval > 100)
      i_msecval = 100; // don't allow it to last longer than 100 milliseconds either

   g_engfuncs.pfnRunPlayerMove (pEdict, vecMoveAngles, pBot->f_move_speed, pBot->f_sidemove_speed, 0, pEdict->v.button, 0, (unsigned char) i_msecval);	// KWo - 25.09.2006 - thanks to THE_STORM

   return;
}

void BotFreeAllMemory (void)
{
   STRINGNODE *pNextNode;
   replynode_t *pNextReply;
   PATH *pNextPath;
   int i;

   for (i = 0; i < 32; i++)
   {
      DeleteSearchNodes (&bots[i]); // Delete Nodes from Pathfinding
      BotResetTasks (&bots[i]);
   }

   // Delete all Waypoint Data

   while (paths[0] != NULL)
   {
      pNextPath = paths[0]->next;
      delete (paths[0]);
      paths[0] = pNextPath;
   }
   paths[0] = NULL;

   memset (paths, 0, sizeof (paths));
   g_iNumWaypoints = 0;
   g_vecLastWaypoint = g_vecZero;

   for (i = 0; i < MAX_WAYPOINTS; i++)
   {
      g_fWPDisplayTime[i] = 0.0;
      g_fPathDisplayTime[i] = 0.0; // KWo - 05.01.2008
   }

   if (pBotExperienceData != NULL)
      delete [](pBotExperienceData);
   pBotExperienceData = NULL;

   if (g_pFloydDistanceMatrix != NULL)
      delete [](g_pFloydDistanceMatrix);
   g_pFloydDistanceMatrix = NULL;

   if (g_pFloydPathMatrix != NULL)
      delete [](g_pFloydPathMatrix);
   g_pFloydPathMatrix = NULL;

   if (g_pWithHostageDistMatrix != NULL)
      delete [](g_pWithHostageDistMatrix);
   g_pWithHostageDistMatrix = NULL;

   if (g_pWithHostagePathMatrix != NULL)
      delete [](g_pWithHostagePathMatrix);
   g_pWithHostagePathMatrix = NULL;

   // Delete all Textnodes/strings

   while (pChatReplies != NULL)
   {
      while (pChatReplies->pReplies != NULL)
      {
         pNextNode = pChatReplies->pReplies->Next;
         delete (pChatReplies->pReplies);
         pChatReplies->pReplies = pNextNode;
      }
      pChatReplies->pReplies = NULL;

      pNextReply = pChatReplies->pNextReplyNode;
      delete (pChatReplies);
      pChatReplies = pNextReply;
   }
   pChatReplies = NULL;

   // Delete all Bot Names
   if (g_pszBotNames != NULL)
      delete [] g_pszBotNames;
   g_pszBotNames = NULL;

   return; // KABLAM! everything is nuked, wiped, cleaned, memory stick is shining like a coin =)
}
