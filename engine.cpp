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
// engine.cpp
//
// Does the major work of calling the original Engine Functions

#include "bot_globals.h"
// KWo - 05.09.2009 - (moved) the idea taken from jk-botti (ghost_of_evilspy)
static int WeaponList = 0;
static int CurWeapon = 0;
static int AmmoX = 0;
static int AmmoPickup = 0;
//static int WeapPickup = 0;
//static int ItemPickup = 0;
//static int Health = 0;
//static int Battery = 0;
static int Damage = 0;
static int ScreenFade = 0;
static int DeathMsg = 0;
static int BarTime = 0;
static int BombDrop = 0;
static int BombPickup = 0;
static int Money = 0;
static int ResetHUD = 0;
static int SayText = 0;
static int ShowMenu = 0;
static int StatusIcon = 0;
static int VGUIMenu = 0;
static int TextMsg = 0;
static int HLTV = 0;
static int ScoreInfo = 0;
static int ScoreAttrib = 0;
static int TeamInfo = 0;
static int TeamScore = 0;
static int RoundTime = 0;
static int FlashBat = 0;
static int NVGToggle = 0;

void pfnEmitSound (edict_t *entity, int channel, const char *sample, float volume, float attenuation, int fFlags, int pitch)
{
   SoundAttachToThreat (entity, sample, volume);
//   fp = fopen ("bot.txt", "a"); fprintf (fp, "pfnEmitSound: Entity#%d channel=%d sample=%s volume=%f attenuation=%f fFlags=%d pitch=%d\n", ENTINDEX (entity), channel, sample, volume, attenuation, fFlags, pitch); fclose (fp);
   RETURN_META (MRES_IGNORED);
}


edict_t *pfnFindEntityByString (edict_t *pEdictStartSearchAfter, const char *pszField, const char *pszValue)
{
   // new round in CS 1.5
   if (strcmp ("info_map_parameters", pszValue) == 0)
   {
      UTIL_RoundStart (); // the round has restarted
   }

   RETURN_META_VALUE (MRES_IGNORED, 0);
}


void pfnClientCommand (edict_t *pEdict, char *szFmt, ...)
{
   va_list argptr;
   static char string[1024];

   va_start (argptr, szFmt);
   vsnprintf (string, sizeof (string), szFmt, argptr);
   va_end (argptr);

/*   FILE *fpc = fopen ("ForcedClientCommand.txt", "a"); fprintf (fpc, "Forced Client Command on %s, \"%s\"\n", STRING (pEdict->v.netname), string); fclose (fpc);
   ALERT (at_logged, "Forced Client Command on %s, \"%s\"\n", STRING (pEdict->v.netname), string);

   FakeClientCommand (pEdict, string);*/

   // is the target entity an official bot, a third party bot or a real player ?
   if (pEdict->v.flags & FL_FAKECLIENT)
      RETURN_META (MRES_SUPERCEDE); // prevent bots to be forced to issue client commands

   RETURN_META (MRES_IGNORED);
}


static inline void CallbackLightStyle (const unsigned char style, char *const value)
{
   if (style >= MAX_LIGHTSTYLES)
   {
      g_engfuncs.pfnServerPrint ("SVC_LIGHTSTYLE > MAX_LIGHTSTYLES\n");
      return;
   }

   // OCCURS!
   if (value == NULL)
   {
      cl_lightstyle[style].length = 0u;
      cl_lightstyle[style].map[0u] = '\0';
      return;
   }

   const unsigned short maximumCopyAmount (sizeof (cl_lightstyle[style].map) - sizeof ('\0'));

   strncpy (cl_lightstyle[style].map, value, maximumCopyAmount);

   cl_lightstyle[style].map[maximumCopyAmount] = '\0';
   cl_lightstyle[style].length = strlen (cl_lightstyle[style].map);
   return;
}


static void pfnLightStyle (int style, char *value)
{
   // Update light style for fake clients....
   CallbackLightStyle (static_cast <const unsigned char> (style), value);

   RETURN_META (MRES_IGNORED);
}


// KWo - 20.02.2008 - the idea taken from jk-botti (ghost_of_evilspy)
int FAST_GET_USER_MSG_ID(plid_t plindex, int & value, const char * name, int * size)
{
   return(value ? value : (value = GET_USER_MSG_ID(plindex, name, size)));
}

void pfnMessageBegin (int msg_dest, int msg_type, const float *pOrigin, edict_t *ed)
{
   // Called each Time a Message is about to sent

   static int index;
   static int tab_index;

   botMsgFunction = NULL; // no msg function until known otherwise
   state = 0;

   // Bot involved ?
   if (!FNullEnt (ed))
   {
      index = ENTINDEX (ed) - 1;

      if ((index >= 0) && (index < gpGlobals->maxClients)
          && ((bots[index].pEdict == ed)) || (clients[index].pEdict == ed)) // KWo - 12.12.2006
      {
         if (msg_type == FAST_GET_USER_MSG_ID (PLID, CurWeapon, "CurWeapon", NULL)) // KWo - 20.02.2008
         {
            botMsgFunction = BotClient_CS_CurrentWeapon;
            botMsgIndex = index; // 12.03.2007
         }
         else if (msg_type == FAST_GET_USER_MSG_ID (PLID, BarTime, "BarTime", NULL)) // KWo - 20.02.2008
         {
            botMsgFunction = BotClient_CS_BarTime;
            botMsgIndex = index;
         }
         else if (msg_type == FAST_GET_USER_MSG_ID (PLID, ResetHUD, "ResetHUD", NULL)) // KWo - 04.03.2010
         {
            BotClient_CS_ResetHUD (NULL, index);
            botMsgIndex = index;
         }
      }
      // is this message for a bot?
      if ((index >= 0) && (index < gpGlobals->maxClients) && (bots[index].pEdict == ed))
      {
         botMsgIndex = index; // index of bot receiving message

         // Message handling is done in bot_client.cpp
         if (msg_type == FAST_GET_USER_MSG_ID (PLID, AmmoPickup, "AmmoPickup", NULL)) // KWo - 20.02.2008
            botMsgFunction = BotClient_CS_AmmoPickup;
         else if (msg_type == FAST_GET_USER_MSG_ID (PLID, AmmoX, "AmmoX", NULL)) // KWo - 20.02.2008
            botMsgFunction = BotClient_CS_AmmoX;
         else if (msg_type == FAST_GET_USER_MSG_ID (PLID, BombDrop, "BombDrop", NULL)) // KWo - 20.02.2008
            botMsgFunction = BotClient_CS_BombDrop;
         else if (msg_type == FAST_GET_USER_MSG_ID (PLID, BombPickup, "BombPickup", NULL)) // KWo - 20.02.2008
            botMsgFunction = BotClient_CS_BombPickup;
         else if (msg_type == FAST_GET_USER_MSG_ID (PLID, Damage, "Damage", NULL)) // KWo - 20.02.2008
            botMsgFunction = BotClient_CS_Damage;
         else if (msg_type == FAST_GET_USER_MSG_ID (PLID, Money, "Money", NULL)) // KWo - 20.02.2008
            botMsgFunction = BotClient_CS_Money;
//         else if (msg_type == FAST_GET_USER_MSG_ID (PLID, ResetHUD, "ResetHUD", NULL)) // KWo - 20.02.2008
//            BotClient_CS_ResetHUD (NULL, index);
         else if (msg_type == FAST_GET_USER_MSG_ID (PLID, SayText, "SayText", NULL)) // KWo - 20.02.2008
            botMsgFunction = BotClient_CS_SayText;
         else if (msg_type == FAST_GET_USER_MSG_ID (PLID, ScreenFade, "ScreenFade", NULL)) // KWo - 20.02.2008
            botMsgFunction = BotClient_CS_ScreenFade;
         else if (msg_type == FAST_GET_USER_MSG_ID (PLID, ShowMenu, "ShowMenu", NULL)) // KWo - 20.02.2008
            botMsgFunction = BotClient_CS_ShowMenu;
         else if (msg_type == FAST_GET_USER_MSG_ID (PLID, StatusIcon, "StatusIcon", NULL)) // KWo - 20.02.2008
            botMsgFunction = BotClient_CS_StatusIcon;
         else if (msg_type == FAST_GET_USER_MSG_ID (PLID, VGUIMenu, "VGUIMenu", NULL)) // KWo - 20.02.2008
            botMsgFunction = BotClient_CS_VGUI;
         else if (msg_type == FAST_GET_USER_MSG_ID (PLID, TextMsg, "TextMsg", NULL)) // KWo - 20.02.2008
            botMsgFunction = BotClient_CS_TextMsg1;
         else if (msg_type == FAST_GET_USER_MSG_ID (PLID, FlashBat, "FlashBat", NULL)) // KWo - 25.05.2008
            botMsgFunction = BotClient_CS_FlashBat;
         else if (msg_type == FAST_GET_USER_MSG_ID (PLID, NVGToggle, "NVGToggle", NULL)) // KWo - 25.05.2008
            botMsgFunction = BotClient_CS_NVGToggle;

      }
   }

   // round restart in CS 1.6

   if (!g_bIsOldCS15 && (msg_dest == MSG_SPEC) && (msg_type == FAST_GET_USER_MSG_ID (PLID, HLTV, "HLTV", NULL))) // KWo - 20.02.2008
      botMsgFunction = BotClient_CS_HLTV;

   else if (msg_dest == MSG_ALL)
   {
      botMsgIndex = -1; // index of bot receiving message (none)

      if (msg_type == FAST_GET_USER_MSG_ID (PLID, ScoreInfo, "ScoreInfo", NULL)) // KWo - 20.02.2008
         botMsgFunction = Client_CS_ScoreInfo;
      else if (msg_type == FAST_GET_USER_MSG_ID (PLID, ScoreAttrib, "ScoreAttrib", NULL)) // KWo - 02.03.2010
         botMsgFunction = Client_CS_ScoreAttrib;
      else if (msg_type == FAST_GET_USER_MSG_ID (PLID, TeamInfo, "TeamInfo", NULL)) // KWo - 20.02.2008
         botMsgFunction = Client_CS_TeamInfo;                        // KWo - 12.02.2006 Added by THE STORM
      else if (msg_type == FAST_GET_USER_MSG_ID (PLID, DeathMsg, "DeathMsg", NULL)) // KWo - 20.02.2008
         botMsgFunction = BotClient_CS_DeathMsg;
      else if (msg_type == FAST_GET_USER_MSG_ID (PLID, WeaponList, "WeaponList", NULL)) // KWo - 20.02.2008
         botMsgFunction = BotClient_CS_WeaponList;
      else if (msg_type == FAST_GET_USER_MSG_ID (PLID, TextMsg, "TextMsg", NULL)) // KWo - 20.02.2008
         botMsgFunction = BotClient_CS_TextMsgAll;
      else if (msg_type == SVC_INTERMISSION) // for some reason I (SpLoRyGoN) don't think this ever gets called...
      {
         tab_index = 0;
         for (index = 0; index < gpGlobals->maxClients; index++)
            if (bots[index].is_used)
            {
               BotCreateTab[tab_index].bNeedsCreation = TRUE;
               strncpy (BotCreateTab[tab_index].bot_name, bots[index].name, sizeof (BotCreateTab[tab_index].bot_name));
               BotCreateTab[tab_index].bot_skill = bots[index].bot_skill;
               BotCreateTab[tab_index].bot_personality = bots[index].bot_personality;
               BotCreateTab[tab_index].bot_team = bots[index].bot_team;
               BotCreateTab[tab_index].bot_class = bots[index].bot_class;
               tab_index++;
            }

         // Save collected Experience on Map Change
         SaveExperienceTab ();
         SaveVisTab ();
      }

//      else if (msg_type == GET_USER_MSG_ID (PLID, "TeamScore", NULL))  // KWo - 02.05.2006
      else if (msg_type == FAST_GET_USER_MSG_ID (PLID, TeamScore, "TeamScore", NULL)) // KWo - 20.02.2008
         botMsgFunction = BotClient_CS_TeamScore;

//      else if (msg_type == GET_USER_MSG_ID (PLID, "RoundTime", NULL))
      else if (msg_type == FAST_GET_USER_MSG_ID (PLID, RoundTime, "RoundTime", NULL)) // KWo - 20.02.2008
         botMsgFunction = BotClient_CS_RoundTime;

   }
   RETURN_META (MRES_IGNORED);
}


void pfnMessageEnd (void)
{
   if ((g_i_cv_latencybot == 2)
         && (botMsgFunction == BotClient_CS_DeathMsg)) // KWo - 04.03.2010
      g_bDeathMessageSent = TRUE;

   botMsgFunction = NULL;

   state = 0;

   RETURN_META (MRES_IGNORED);
}


void pfnWriteByte (int iValue)
{
   // if this message is for a bot, call the client message function...
   if (botMsgFunction)
      (*botMsgFunction) ((void *) &iValue, botMsgIndex);

   state++;

   RETURN_META (MRES_IGNORED);
}


void pfnWriteChar (int iValue)
{
   // if this message is for a bot, call the client message function...
   if (botMsgFunction)
      (*botMsgFunction) ((void *) &iValue, botMsgIndex);

   state++;

   RETURN_META (MRES_IGNORED);
}


void pfnWriteShort (int iValue)
{
   // if this message is for a bot, call the client message function...
   if (botMsgFunction)
      (*botMsgFunction) ((void *) &iValue, botMsgIndex);

   state++;

   RETURN_META (MRES_IGNORED);
}


void pfnWriteLong (int iValue)
{
   // if this message is for a bot, call the client message function...
   if (botMsgFunction)
      (*botMsgFunction) ((void *) &iValue, botMsgIndex);

   state++;

   RETURN_META (MRES_IGNORED);
}


void pfnWriteAngle (float flValue)
{
   // if this message is for a bot, call the client message function...
   if (botMsgFunction)
      (*botMsgFunction) ((void *) &flValue, botMsgIndex);

   state++;

   RETURN_META (MRES_IGNORED);
}


void pfnWriteCoord (float flValue)
{
   // if this message is for a bot, call the client message function...
   if (botMsgFunction)
      (*botMsgFunction) ((void *) &flValue, botMsgIndex);

   state++;

   RETURN_META (MRES_IGNORED);
}


void pfnWriteString (const char *sz)
{
   // if this message is for a bot, call the client message function...
   if (botMsgFunction)
      (*botMsgFunction) ((void *) sz, botMsgIndex);

   state++;

   RETURN_META (MRES_IGNORED);
}


void pfnWriteEntity (int iValue)
{
   // if this message is for a bot, call the client message function...
   if (botMsgFunction)
      (*botMsgFunction) ((void *) &iValue, botMsgIndex);

   state++;

   RETURN_META (MRES_IGNORED);
}


void pfnClientPrintf (edict_t *pEdict, PRINT_TYPE ptype, const char *szMsg)
{
   if (pEdict->v.flags & FL_FAKECLIENT)
      RETURN_META (MRES_SUPERCEDE); // disallow client printings for bots

   RETURN_META (MRES_IGNORED);
}


const char *pfnCmd_Args (void)
{
   // this function returns a pointer to the whole current client command string. Since bots
   // have no client DLL and we may want a bot to execute a client command, we had to implement
   // a g_argv string in the bot DLL for holding the bots' commands, and also keep track of the
   // argument count. Hence this hook not to let the engine ask an unexistent client DLL for a
   // command we are holding here. Of course, real clients commands are still retrieved the
   // normal way, by asking the engine.

   if (isFakeClientCommand)
   {
      // is it a "say" or "say_team" client command ?
      if (strncmp ("say ", g_argv, 4) == 0)
         RETURN_META_VALUE (MRES_SUPERCEDE, &g_argv[0] + 4); // skip the "say" bot client command (bug in HL engine)
      else if (strncmp ("say_team ", g_argv, 9) == 0)
         RETURN_META_VALUE (MRES_SUPERCEDE, &g_argv[0] + 9); // skip the "say_team" bot client command (bug in HL engine)

      RETURN_META_VALUE (MRES_SUPERCEDE, &g_argv[0]); // else return the whole bot client command string we know
   }

   RETURN_META_VALUE (MRES_IGNORED, 0);
}


const char *pfnCmd_Argv (int argc)
{
   // this function returns a pointer to a certain argument of the current client command. Since
   // bots have no client DLL and we may want a bot to execute a client command, we had to
   // implement a g_argv string in the bot DLL for holding the bots' commands, and also keep
   // track of the argument count. Hence this hook not to let the engine ask an unexistent client
   // DLL for a command we are holding here. Of course, real clients commands are still retrieved
   // the normal way, by asking the engine.

   if (isFakeClientCommand)
      RETURN_META_VALUE (MRES_SUPERCEDE, GetField (g_argv, argc)); // returns the wanted argument

   RETURN_META_VALUE (MRES_IGNORED, 0);
}


int pfnCmd_Argc (void)
{
   // this function returns the number of arguments the current client command string has. Since
   // bots have no client DLL and we may want a bot to execute a client command, we had to
   // implement a g_argv string in the bot DLL for holding the bots' commands, and also keep
   // track of the argument count. Hence this hook not to let the engine ask an unexistent client
   // DLL for a command we are holding here. Of course, real clients commands are still retrieved
   // the normal way, by asking the engine.

   if (isFakeClientCommand)
      RETURN_META_VALUE (MRES_SUPERCEDE, fake_arg_count); // return the argument count

   RETURN_META_VALUE (MRES_IGNORED, 0);
}


void pfnSetClientMaxspeed (const edict_t *pEdict, float fNewMaxspeed)
{
   bot_t *pBot = UTIL_GetBotPointer ((edict_t*) pEdict);
   if (pBot != NULL)
      pBot->pEdict->v.maxspeed = fNewMaxspeed;

   RETURN_META (MRES_IGNORED);
}


unsigned int pfnGetPlayerWONId (edict_t *e)
{
   if ((e->v.flags & FL_FAKECLIENT) || (bots[ENTINDEX (e) - 1].is_used))
      RETURN_META_VALUE (MRES_SUPERCEDE, 0);

   RETURN_META_VALUE (MRES_IGNORED, 0);
}


const char *pfnGetPlayerAuthId (edict_t *e)
{
   if ((e->v.flags & FL_FAKECLIENT) || (bots[ENTINDEX (e) - 1].is_used))
      RETURN_META_VALUE (MRES_SUPERCEDE, "BOT");

   RETURN_META_VALUE (MRES_IGNORED, NULL);
}

void pfnAlertMessage_Post(ALERT_TYPE atype, char *szFmt, ...)
{
	if (atype != at_logged)
		RETURN_META(MRES_IGNORED);

	/* There are also more messages but we want only logs
	at_notice,
	at_console,		// same	as at_notice, but forces a ConPrintf, not a	message	box
	at_aiconsole,	// same	as at_console, but only	shown if developer level is	2!
	at_warning,
	at_error,
	at_logged		// Server print to console ( only in multiplayer games ).
	*/
/*
	// execute logevents and plugin_log forward
	if (g_logevents.logEventsExist() || FF_PluginLog >= 0)
	{
		va_list	logArgPtr;
		va_start(logArgPtr, szFmt);
		g_logevents.setLogString(szFmt, logArgPtr);
		va_end(logArgPtr);
		g_logevents.parseLogString();

		if (g_logevents.logEventsExist())
			g_logevents.executeLogEvents();

		cell retVal = executeForwards(FF_PluginLog);

		if (retVal)
			RETURN_META(MRES_HANDLED);
	}
*/

	RETURN_META(MRES_IGNORED);
}

void pfnChangeLevel (char* s1, char* s2)
{
   int index;
   int tab_index;

   tab_index = 0;
   for (index = 0; index < gpGlobals->maxClients; index++)
      if (bots[index].is_used)
      {
         BotCreateTab[tab_index].bNeedsCreation = TRUE;
         strncpy (BotCreateTab[tab_index].bot_name, bots[index].name, sizeof (BotCreateTab[tab_index].bot_name));
         BotCreateTab[tab_index].bot_skill = bots[index].bot_skill;
         BotCreateTab[tab_index].bot_personality = bots[index].bot_personality;
         BotCreateTab[tab_index].bot_team = bots[index].bot_team;
         BotCreateTab[tab_index].bot_class = bots[index].bot_class;
         tab_index++;
      }
   RETURN_META (MRES_IGNORED);
}

void C_MessageEnd_Post (void) // KWo - 05.03.2010
{
   static edict_t *pPlayer;
   static int i, player_index, sending;

   if ((g_i_cv_latencybot == 2)
         && g_bDeathMessageSent)
   {
      g_bDeathMessageSent = FALSE;

      for (player_index = 0; player_index < gpGlobals->maxClients; player_index++)
      {
         pPlayer = clients[player_index].pEdict;
         if (FNullEnt (pPlayer))
            continue;
         if (!(pPlayer->v.flags & FL_CLIENT))
            continue;
         if ((pPlayer->v.flags & FL_FAKECLIENT) || (bots[player_index].is_used))
            continue;

         if ((pPlayer->v.button & IN_SCORE) || (pPlayer->v.oldbuttons & IN_SCORE))
         {
            sending = 0;
            for (i = 0; i < gpGlobals->maxClients; i++)
            {
               if (bots[i].is_used && !FNullEnt (bots[i].pEdict))
               {
               // Send message with the weird arguments
                  switch (sending)
                  {
                     case 0:
                     {
                     // Start a new message
                        MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, SVC_PINGS, NULL, pPlayer);
                        WRITE_BYTE((bots[i].iOffsetPing[0] * 64) + (1 + 2 * (i)));
                        WRITE_SHORT(bots[i].iArgPing[0]);
                        sending++;
			            }
                     case 1:
                     {
				         // Append additional data
                        WRITE_BYTE((bots[i].iOffsetPing[1] * 128) + (2 + 4 * (i)));
                        WRITE_SHORT(bots[i].iArgPing[1]);
                        sending++;
                     }
                     case 2:
                     {
                     // Append additional data and end message
                        WRITE_BYTE((4 + 8 * (i)));
                        WRITE_SHORT(bots[i].iArgPing[2]);
                        WRITE_BYTE(0);
                        MESSAGE_END();
                        sending = 0;
                     }
   	            }
               }
            }
            // End message if not yet sent
            if (sending)
            {
               WRITE_BYTE(0);
               MESSAGE_END();
            }
         }
      }
   }
   RETURN_META (MRES_IGNORED);
}

/*
void pfnSetModel_Post (edict_t *pent, const char *pszName)
{
   if (FNullEnt(pent))
      RETURN_META(MRES_IGNORED);

   if (FStrEq (STRING(pent->v.classname),"weapon_hegrenade"))
   {
      ALERT (at_logged, "[DEBUG] SetModel_Post - a HE has been spawned...\n");
      if (!FNullEnt(pent->v.owner))
      {
         if (FStrEq (STRING(pent->v.owner->v.classname),"player"))
            ALERT (at_logged, "[DEBUG] SetModel_Post - %s got a HE...\n", STRING(pent->v.owner->v.netname));
      }
   }
   else if (FStrEq (STRING(pent->v.classname),"weapon_smokegrenade"))
   {
      ALERT (at_logged, "[DEBUG] SetModel_Post - a smoke grenade has been spawned...\n");
      if (!FNullEnt(pent->v.owner))
      {
         if (FStrEq (STRING(pent->v.owner->v.classname),"player"))
            ALERT (at_logged, "[DEBUG] SetModel_Post - %s got a smoke greande...\n", STRING(pent->v.owner->v.netname));
      }
   }
   else if (FStrEq (STRING(pent->v.classname),"weapon_flashbang"))
   {
      ALERT (at_logged, "[DEBUG] SetModel_Post - a flashbang has been spawned...\n");
      if (!FNullEnt(pent->v.owner))
      {
         if (FStrEq (STRING(pent->v.owner->v.classname),"player"))
            ALERT (at_logged, "[DEBUG] SetModel_Post - %s got a flashbang...\n", STRING(pent->v.owner->v.netname));
      }
   }
	RETURN_META(MRES_IGNORED);
}
*/


C_DLLEXPORT int GetEngineFunctions (enginefuncs_t *pengfuncsFromEngine, int *interfaceVersion)
{
   meta_engfuncs.pfnChangeLevel = pfnChangeLevel;
   meta_engfuncs.pfnEmitSound = pfnEmitSound;
   meta_engfuncs.pfnFindEntityByString = pfnFindEntityByString;
   meta_engfuncs.pfnClientCommand = pfnClientCommand;
   meta_engfuncs.pfnLightStyle = pfnLightStyle;   // KWo - 15.03.2012 - thanks to Immortal_BLG
   meta_engfuncs.pfnMessageBegin = pfnMessageBegin;
   meta_engfuncs.pfnMessageEnd = pfnMessageEnd;
   meta_engfuncs.pfnWriteByte = pfnWriteByte;
   meta_engfuncs.pfnWriteChar = pfnWriteChar;
   meta_engfuncs.pfnWriteShort = pfnWriteShort;
   meta_engfuncs.pfnWriteLong = pfnWriteLong;
   meta_engfuncs.pfnWriteAngle = pfnWriteAngle;
   meta_engfuncs.pfnWriteCoord = pfnWriteCoord;
   meta_engfuncs.pfnWriteString = pfnWriteString;
   meta_engfuncs.pfnWriteEntity = pfnWriteEntity;
   meta_engfuncs.pfnClientPrintf = pfnClientPrintf;
   meta_engfuncs.pfnCmd_Args = pfnCmd_Args;
   meta_engfuncs.pfnCmd_Argv = pfnCmd_Argv;
   meta_engfuncs.pfnCmd_Argc = pfnCmd_Argc;
   meta_engfuncs.pfnSetClientMaxspeed = pfnSetClientMaxspeed;
   meta_engfuncs.pfnGetPlayerWONId = pfnGetPlayerWONId;
   meta_engfuncs.pfnGetPlayerAuthId = pfnGetPlayerAuthId;

   memcpy (pengfuncsFromEngine, &meta_engfuncs, sizeof (enginefuncs_t));
   return (TRUE);
}

enginefuncs_t meta_engfuncs_post; // KWo - 19.05.2006
C_DLLEXPORT int GetEngineFunctions_Post (enginefuncs_t *pengfuncsFromEngine, int *interfaceVersion)
{
   memset(&meta_engfuncs_post, 0, sizeof(enginefuncs_t));
   meta_engfuncs_post.pfnAlertMessage = pfnAlertMessage_Post;
   meta_engfuncs_post.pfnMessageEnd = C_MessageEnd_Post;    // KWo - 05.03.2010
//   meta_engfuncs_post.pfnSetModel = pfnSetModel_Post; // KWo - 20.01.2008
//   meta_engfuncs_post.pfnRegUserMsg = pfnRegUserMsg_Post;
   memcpy(pengfuncsFromEngine, &meta_engfuncs_post, sizeof(enginefuncs_t));
   return (TRUE);
}
