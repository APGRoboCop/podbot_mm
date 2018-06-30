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
// dll.cpp
//
// Links Functions, handles Client Commands, initializes DLL and misc Stuff

#include "bot_globals.h"

// server command handler
void PODBot_ServerCommand (void);
void PbCmdParser (edict_t *pEdict, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5);
void UserNewroundAll (void);
void GetGameDir (void);
void ShowPBKickBotMenu (edict_t *pEntity, int iMenuNum);
bool IsPBAdmin (edict_t *pEdict);

// cvars
cvar_t g_rgcvarTemp[NUM_PBCVARS] =
{
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_AIM_DAMPER_COEFFICIENT_X]),      "0.22", FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_AIM_DAMPER_COEFFICIENT_Y]),      "0.22", FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_AIM_DEVIATION_X]),               "2.0",  FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_AIM_DEVIATION_Y]),               "1.0",  FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_AIM_INFLUENCE_X_ON_Y]),          "0.25", FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_AIM_INFLUENCE_Y_ON_X]),          "0.17", FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_AIM_NOTARGET_SLOWDOWN_RATIO]),   "0.5",  FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_AIM_OFFSET_DELAY]),              "1.2",  FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_AIM_SPRING_STIFFNESS_X]),        "13.0", FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_AIM_SPRING_STIFFNESS_Y]),        "13.0", FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_AIM_TARGET_ANTICIPATION_RATIO]), "2.2",  FCVAR_SERVER | FCVAR_EXTDLL},      // KWo - 04.03.2006
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_AIM_TYPE]),                      "4",    FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_AUTOKILL]),                      "0.0",  FCVAR_SERVER | FCVAR_EXTDLL},      // KWo - 02.05.2006
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_AUTOKILLDELAY]),                 "45.0", FCVAR_SERVER | FCVAR_EXTDLL},      // KWo - 02.05.2006
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_BOTJOINTEAM]),                   "ANY",  FCVAR_SERVER | FCVAR_EXTDLL},      // KWo - 16.09.2006
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_BOTQUOTAMATCH]),                 "0.0",  FCVAR_SERVER | FCVAR_EXTDLL},      // KWo - 16.09.2006
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_CHAT]),                          "1",    FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_DANGERFACTOR]),                  "800",  FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_DEBUGLEVEL]),                    "0",    FCVAR_SERVER | FCVAR_EXTDLL},      // KWo - 20.04.2013
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_DETAILNAMES]),                   "0",    FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_FFA]),                           "0",    FCVAR_SERVER | FCVAR_EXTDLL},      // KWo - 04.10.2006
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_FIRSTHUMANRESTART]),             "0",    FCVAR_SERVER | FCVAR_EXTDLL},      // KWo - 04.10.2010
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_JASONMODE]),                     "0",    FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_LATENCYBOT]),                    "0",    FCVAR_SERVER | FCVAR_EXTDLL},      // KWo - 16.05.2008
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_MAPSTARTBOTJOINDELAY]),          "5",    FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_MAXBOTS]),                       "0",    FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_MAXBOTSKILL]),                   "100",  FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_MAXCAMPTIME]),                   "30",   FCVAR_SERVER | FCVAR_EXTDLL},      // KWo - 30.23.2008
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_MAXWEAPONPICKUP]),               "3",    FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_MINBOTS]),                       "0",    FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_MINBOTSKILL]),                   "60",   FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_NUMFOLLOWUSER]),                 "3",    FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_PASSWORD]),                      "",     FCVAR_EXTDLL | FCVAR_PROTECTED},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_PASSWORDKEY]),                   "_pbadminpw", FCVAR_EXTDLL | FCVAR_PROTECTED},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_RADIO]),                         "1",    FCVAR_SERVER | FCVAR_EXTDLL},      // KWo - 03.02.2007
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_RESTREQUIPAMMO]),                "000000000",    FCVAR_SERVER | FCVAR_EXTDLL},  // KWo - 09.03.2006
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_RESTRWEAPONS]),                  "00000000000000000000000000",    FCVAR_SERVER | FCVAR_EXTDLL},  // KWo - 09.03.2006
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_SHOOTTHRUWALLS]),                "1",    FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_SKIN]),                          "5",    FCVAR_SERVER | FCVAR_EXTDLL},      // KWo - 18.11.2006
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_SPRAY]),                         "1",    FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_TIMER_GRENADE]),                 "0.5",  FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_TIMER_PICKUP]),                  "0.3",  FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_TIMER_SOUND]),                   "1.0",  FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_USESPEECH]),                     "1",    FCVAR_SERVER | FCVAR_EXTDLL},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_VERSION]),                       "",     FCVAR_SERVER | FCVAR_SPONLY},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_WELCOMEMSGS]),                   "1",    FCVAR_SERVER | FCVAR_EXTDLL | FCVAR_SPONLY},
   {const_cast<char *>(g_rgpszPbCvars[PBCVAR_WPTFOLDER]),                     "wptdefault", FCVAR_SERVER | FCVAR_EXTDLL},
};

// START of Metamod stuff

enginefuncs_t meta_engfuncs;
gamedll_funcs_t *gpGamedllFuncs;
mutil_funcs_t *gpMetaUtilFuncs;
meta_globals_t *gpMetaGlobals;

META_FUNCTIONS gMetaFunctionTable =
{
   NULL, // pfnGetEntityAPI()
   NULL, // pfnGetEntityAPI_Post()
   GetEntityAPI2, // pfnGetEntityAPI2()
   GetEntityAPI2_Post, // pfnGetEntityAPI2_Post()
   NULL, // pfnGetNewDLLFunctions()
   NULL, // pfnGetNewDLLFunctions_Post()
   GetEngineFunctions, // pfnGetEngineFunctions()
   GetEngineFunctions_Post, // pfnGetEngineFunctions_Post() KWo - 19.05.2006
//   NULL, // pfnGetEngineFunctions_Post()
};

plugin_info_t Plugin_info = {
   META_INTERFACE_VERSION, // interface version
   "POD-Bot mm", // plugin name
   PBMM_VERSION_STRING, // plugin version
   __DATE__, // date of creation
   "Count Floyd & Bots United", // plugin author
   "http://www.bots-united.com", // plugin URL
   "PODBOTMM", // plugin logtag
   PT_CHANGELEVEL, // when loadable
   PT_ANYTIME, // when unloadable
};


C_DLLEXPORT int Meta_Query (char *ifvers, plugin_info_t **pPlugInfo, mutil_funcs_t *pMetaUtilFuncs)
{
   // this function is the first function ever called by metamod in the plugin DLL. Its purpose
   // is for metamod to retrieve basic information about the plugin, such as its meta-interface
   // version, for ensuring compatibility with the current version of the running metamod.

   // keep track of the pointers to metamod function tables metamod gives us
   gpMetaUtilFuncs = pMetaUtilFuncs;
   *pPlugInfo = &Plugin_info;

   // check for interface version compatibility
   if (strcmp (ifvers, Plugin_info.ifvers) != 0)
   {
      int mmajor = 0, mminor = 0, pmajor = 0, pminor = 0;

      LOG_CONSOLE (PLID, "%s: meta-interface version mismatch (metamod: %s, %s: %s)", Plugin_info.name, ifvers, Plugin_info.name, Plugin_info.ifvers);
      LOG_MESSAGE (PLID, "%s: meta-interface version mismatch (metamod: %s, %s: %s)", Plugin_info.name, ifvers, Plugin_info.name, Plugin_info.ifvers);

      // if plugin has later interface version, it's incompatible (update metamod)
      sscanf (ifvers, "%d:%d", &mmajor, &mminor);
      sscanf (META_INTERFACE_VERSION, "%d:%d", &pmajor, &pminor);

      if ((pmajor > mmajor) || ((pmajor == mmajor) && (pminor > mminor)))
      {
         LOG_CONSOLE (PLID, "metamod version is too old for this plugin; update metamod");
         LOG_ERROR (PLID, "metamod version is too old for this plugin; update metamod");
         return (FALSE);
      }

      // if plugin has older major interface version, it's incompatible (update plugin)
      else if (pmajor < mmajor)
      {
         LOG_CONSOLE (PLID, "metamod version is incompatible with this plugin; please find a newer version of this plugin");
         LOG_ERROR (PLID, "metamod version is incompatible with this plugin; please find a newer version of this plugin");
         return (FALSE);
      }
   }

   return (TRUE); // tell metamod this plugin looks safe
}


C_DLLEXPORT int Meta_Attach (PLUG_LOADTIME now, META_FUNCTIONS *pFunctionTable, meta_globals_t *pMGlobals, gamedll_funcs_t *pGamedllFuncs)
{
   // this function is called when metamod attempts to load the plugin. Since it's the place
   // where we can tell if the plugin will be allowed to run or not, we wait until here to make
   // our initialization stuff, like registering CVARs and dedicated server commands.

   // are we allowed to load this plugin now ?
   if (now > Plugin_info.loadable)
   {
      LOG_CONSOLE (PLID, "%s: plugin NOT attaching (can't load plugin right now)", Plugin_info.name);
      LOG_ERROR (PLID, "%s: plugin NOT attaching (can't load plugin right now)", Plugin_info.name);
      return (FALSE); // returning FALSE prevents metamod from attaching this plugin
   }

   // keep track of the pointers to engine function tables metamod gives us
   gpMetaGlobals = pMGlobals;
   memcpy (pFunctionTable, &gMetaFunctionTable, sizeof (META_FUNCTIONS));
   gpGamedllFuncs = pGamedllFuncs;

   // print a message to notify about plugin attaching
   LOG_CONSOLE (PLID, "%s: plugin attaching", Plugin_info.name);
   LOG_MESSAGE (PLID, "%s: plugin attaching", Plugin_info.name);

   // ask the engine to register the server commands this plugin uses
   REG_SVR_COMMAND ((char *)g_rgpszPbCmds[PBCMD], PODBot_ServerCommand);

   // Register CVARS
   for (int i = 0; i < NUM_PBCVARS; ++i)
   {
      CVAR_REGISTER (&g_rgcvarTemp[i]);
      g_rgcvarPointer[i] = CVAR_GET_POINTER(g_rgcvarTemp[i].name); // KWo - 07.10.2006 - thanks BAILOPAN
   }

   CVAR_SET_STRING (g_rgpszPbCvars[PBCVAR_VERSION], Plugin_info.version);

   return (TRUE); // returning TRUE enables metamod to attach this plugin
}


C_DLLEXPORT int Meta_Detach (PLUG_LOADTIME now, PL_UNLOAD_REASON reason)
{
   // this function is called when metamod unloads the plugin. A basic check is made in order
   // to prevent unloading the plugin if its processing should not be interrupted.

   // is metamod allowed to unload the plugin ?
   if ((now > Plugin_info.unloadable) && (reason != PNL_CMD_FORCED))
   {
      LOG_CONSOLE (PLID, "%s: plugin NOT detaching (can't unload plugin right now)", Plugin_info.name);
      LOG_ERROR (PLID, "%s: plugin NOT detaching (can't unload plugin right now)", Plugin_info.name);
      return (FALSE); // returning FALSE prevents metamod from unloading this plugin
   }

   PbCmdParser ((g_bIsDedicatedServer) ? NULL : pHostEdict, g_rgpszPbCmds[PBCMD_REMOVEBOTS], NULL, NULL, NULL, NULL, NULL); // KWo - 12.03.2012 - thanks to Immortal_BLG
   // Delete all allocated Memory
   BotFreeAllMemory();

   return (TRUE); // returning TRUE enables metamod to unload this plugin
}

// END of Metamod stuff

// If we're using MS compiler, we need to specify the export parameter...
// by Jozef Wagner - START
#if _MSC_VER > 1000
#pragma comment(linker, "/EXPORT:GiveFnptrsToDll=_GiveFnptrsToDll@8,@1")
#pragma comment(linker, "/SECTION:.data,RW")
#endif
// Jozef Wagner - END

#ifndef __linux__
BOOL WINAPI DllMain (HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
   // Required DLL entry point

   return (TRUE);
}
#endif

static inline void SetupLightStyles (void)
{
   // Setup lighting information....

   // reset all light styles
   for (unsigned char index (0u); index < MAX_LIGHTSTYLES; ++index)
   {
      cl_lightstyle[index].length = 0u;
      cl_lightstyle[index].map[0u] = '\0';
   }

   for (unsigned short index (0u); index < MAX_LIGHTSTYLEVALUE; ++index)
      d_lightstylevalue[index] = 264;   // normal light value
}

static inline void R_AnimateLight (void)
{
   // light animations
   // 'm' is normal light, 'a' is no light, 'z' is double bright
   const int i (static_cast <int> (gpGlobals->time * 10.0f));
   int k;

   for (unsigned char j (0u); j < MAX_LIGHTSTYLES; ++j)
   {
      if (cl_lightstyle[j].length == 0u)
      {
         d_lightstylevalue[j] = 256;

         continue;
      }

      k = cl_lightstyle[j].map[i % cl_lightstyle[j].length] - 'a';
      k *= 22;

      d_lightstylevalue[j] = k;
   }
}

inline void ShowMagic (void)
{
   /// @todo REMOVE THIS SHIT

   edict_t *const hostPlayerEdict (pHostEdict);

   if (hostPlayerEdict == NULL || hostPlayerEdict->free || hostPlayerEdict->pvPrivateData == NULL || (hostPlayerEdict->v.flags & FL_FAKECLIENT))
      return;

   char message[192];
   snprintf (message, sizeof (message), "ShowMagic(): \"%s\"->v.light_level=%i, R_LightPoint(hostOrg)=%i\n", STRING (hostPlayerEdict->v.netname), hostPlayerEdict->v.light_level, Light::R_LightPoint (hostPlayerEdict->v.origin));

//   CLIENT_PRINTF (hostPlayerEdict, print_chat, message);

   if (GET_USER_MSG_ID (PLID, "TextMsg", NULL) == 0)
      REG_USER_MSG ("TextMsg", -1);

   MESSAGE_BEGIN (MSG_ONE_UNRELIABLE, GET_USER_MSG_ID (PLID, "TextMsg", NULL), NULL, hostPlayerEdict);
      WRITE_BYTE (HUD_PRINTTALK);
      WRITE_STRING (message);
   MESSAGE_END ();
}

static inline void CallbackStartFrame (void)
{
   R_AnimateLight ();

   /// @todo REMOVE THIS SHIT
//   #pragma message ("\tWARNING: REMOVE THIS SHIT!!!")
//   ShowMagic ();
}
static inline void CallbackPM_Move (playermove_t *const playerMove, const bool server)
{
   // Reliability checks.
   assert (playerMove != NULL);
   assert (server == true);   // ALWAYS SHOULD BE TRUE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

   // Honestly this is need only once per changelevel, but....
   sv_worldmodel = playerMove->physents[0u].model;   // Always point at sv.worldmodel!

   // Reliability check.
   assert (sv_worldmodel != NULL);
}

void WINAPI GiveFnptrsToDll (enginefuncs_t* pengfuncsFromEngine, globalvars_t *pGlobals)
{
   // get the engine functions from the engine...

   memcpy (&g_engfuncs, pengfuncsFromEngine, sizeof (enginefuncs_t));
   gpGlobals = pGlobals;
   SetupLightStyles (); // KWo - 15.03.2012 - thanks to Immortal_BLG
}


void GameDLLInit (void)
{
   // First function called from HL in our replacement Server DLL

   unsigned char *tempbuf;
   int tempsize;
   g_bIsDedicatedServer = (IS_DEDICATED_SERVER () > 0);

   // If this is a listen server account for the Host
   if (!g_bIsDedicatedServer)
      g_iPeoBotsKept = 1;

   // Update g_szGameDirectory
   GetGameDir ();

   // Counter-Strike 1.6 detection
   tempbuf = LOAD_FILE_FOR_ME ("models/w_famas.mdl", &tempsize);
   if (tempbuf != NULL)
   {
      g_bIsOldCS15 = FALSE;
      FREE_FILE (tempbuf);
   }
   else
      g_bIsOldCS15 = TRUE;

   // Reset the bot creation tab
   memset (BotCreateTab, 0, sizeof (BotCreateTab));

   // Get all of the user's settings from podbot.cfg
   g_bBotSettings = TRUE;
   g_bWeaponStrip = FALSE; // KWo - 10.03.2013

   RETURN_META (MRES_IGNORED);
}


int Spawn (edict_t *pent)
{
   // Something gets spawned in the game

   if (strcmp (STRING (pent->v.classname), "worldspawn") == 0)
   {
      g_iMapType = 0; // reset g_iMapType as worldspawn is the first entity spawned
      PRECACHE_SOUND ((char *)STRING (ALLOC_STRING ("weapons/xbow_hit1.wav"))); // waypoint add - KWo - 11.03.2012 - thanks to Immortal_BLG
      PRECACHE_SOUND ((char *)STRING (ALLOC_STRING ("weapons/mine_activate.wav"))); // waypoint delete - KWo - 11.03.2012 - thanks to Immortal_BLG
      PRECACHE_SOUND ((char *)STRING (ALLOC_STRING ("common/wpn_hudon.wav"))); // path add/delete done - KWo - 11.03.2012 - thanks to Immortal_BLG
      PRECACHE_SOUND ((char *)STRING (ALLOC_STRING ("debris/bustglass1.wav"))); // waypoint error found - KWo - 11.03.2012 - thanks to Immortal_BLG
      g_pSpriteTexture = PRECACHE_MODEL ((char *)STRING (ALLOC_STRING ("sprites/lgtning.spr"))); // KWo - 11.03.2012 - thanks to Immortal_BLG
   }

   else if (strcmp (STRING (pent->v.classname), "info_player_start") == 0)
   {
      SET_MODEL (pent, (char *)STRING (ALLOC_STRING ("models/player/urban/urban.mdl"))); // KWo - 11.03.2012 - thanks to Immortal_BLG
      pent->v.rendermode = kRenderTransAlpha; // set its render mode to transparency
      pent->v.renderamt = 127; // set its transparency amount
      pent->v.effects |= EF_NODRAW;
   }

   else if (strcmp (STRING (pent->v.classname), "info_player_deathmatch") == 0)
   {
      SET_MODEL (pent, (char *)STRING (ALLOC_STRING ("models/player/terror/terror.mdl"))); // KWo - 11.03.2012 - thanks to Immortal_BLG
      pent->v.rendermode = kRenderTransAlpha; // set its render mode to transparency
      pent->v.renderamt = 127; // set its transparency amount
      pent->v.effects |= EF_NODRAW;
   }

   else if (strcmp (STRING (pent->v.classname), "info_vip_start") == 0)
   {
      SET_MODEL (pent, (char *)STRING (ALLOC_STRING ("models/player/vip/vip.mdl"))); // KWo - 11.03.2012 - thanks to Immortal_BLG
      pent->v.rendermode = kRenderTransAlpha; // set its render mode to transparency
      pent->v.renderamt = 127; // set its transparency amount
      pent->v.effects |= EF_NODRAW;
   }
   // KWo - 20.06.2006 - thanks to strelomet for fixing crash on scoutzknives maps
   else if (strcmp (STRING (pent->v.classname), "player_weaponstrip") == 0)
   {
      char szTemp[64]; // KWo - 23.12.2006
      snprintf (szTemp, sizeof (szTemp), STRING (pent->v.target));
      g_bWeaponStrip = TRUE; // KWo - 10.03.2013
      if (g_bIsOldCS15) // KWo - 01.04.2013
      {
         if ((szTemp[0] == '\0') /* && (g_iNumWaypoints) */)
         {
            pent->v.target = ALLOC_STRING ("fake"); // KWo - 11.03.2012 - thanks to Immortal_BLG
            pent->v.targetname = ALLOC_STRING ("fake"); // KWo - 11.03.2012 - thanks to Immortal_BLG
         }
      }
      else
      {
         REMOVE_ENTITY(pent); // KWo - 10.03.2013
         RETURN_META_VALUE (MRES_SUPERCEDE, 0);  // KWo - 25.03.2013
      }
   }
   else if ((strcmp (STRING (pent->v.classname), "func_vip_safetyzone") == 0)
            || (strcmp (STRING (pent->v.classname), "info_vip_safetyzone") == 0))
      g_iMapType |= MAP_AS; // assassination map

   else if (strcmp (STRING (pent->v.classname), "hostage_entity") == 0)
      g_iMapType |= MAP_CS; // rescue map

   else if ((strcmp (STRING (pent->v.classname), "func_bomb_target") == 0)
            || (strcmp (STRING (pent->v.classname), "info_bomb_target") == 0))
      g_iMapType |= MAP_DE; // defusion map


   if ((pent->v.rendermode == kRenderTransTexture) && (pent->v.flags & FL_WORLDBRUSH))
      pent->v.flags &= ~FL_WORLDBRUSH; // clear the FL_WORLDBRUSH flag out of transparent ents


   RETURN_META_VALUE (MRES_IGNORED, 0);
}


int Spawn_Post (edict_t *pent)
{
   // KWo - 04.03.2006
   int i;

   if ( (FStrEq (STRING(pent->v.classname),"func_breakable") || FStrEq (STRING(pent->v.classname),"func_pushable"))
         && IsShootableBreakable (pent) && (g_iNumBreakables < MAX_BREAKABLES) )
   {
      if (g_iNumBreakables == 0)
      {
         for (i=0; i < MAX_BREAKABLES; i++) // KWo - 14.04.2013 (fixed a small mistake...)
         {
            BreakablesData[i].EntIndex = -1;
            BreakablesData[i].classname[0] = '\0';
            BreakablesData[i].origin = g_vecZero;
            BreakablesData[i].target[0] = '\0';
            BreakablesData[i].ignored = false;
         }
      }

      BreakablesData[g_iNumBreakables].EntIndex = ENTINDEX(pent);
      snprintf (BreakablesData[g_iNumBreakables].classname, sizeof (BreakablesData[g_iNumBreakables].classname), STRING (pent->v.classname));
      BreakablesData[g_iNumBreakables].origin = VecBModelOrigin (pent);
      snprintf (BreakablesData[g_iNumBreakables].target, sizeof (BreakablesData[g_iNumBreakables].target), STRING (pent->v.target));
      if (pent->v.impulse > 0) // KWo - 18.05.2006
         BreakablesData[g_iNumBreakables].ignored = true;
      else
         BreakablesData[g_iNumBreakables].ignored = false;
      g_iNumBreakables++;
   }
   // solves the bots unable to see through certain types of glass bug.
   // MAPPERS: NEVER EVER ALLOW A TRANSPARENT ENTITY TO WEAR THE FL_WORLDBRUSH FLAG !!!
   // KWo - 04.03.2006 - FL_WORDBRUSH defines unbreakable glasses... - it doesn't work at all (tested on de_frosty)

   if ((pent->v.rendermode == kRenderTransTexture) && (pent->v.flags & FL_WORLDBRUSH))
      pent->v.flags &= ~FL_WORLDBRUSH; // clear the FL_WORLDBRUSH flag out of transparent ents

   RETURN_META_VALUE (MRES_IGNORED, 0);
}

BOOL ClientConnect (edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ])
{
   // Client connects to this Server

   // check if this client is the listen server client
   if (FStrEq (pszAddress, "loopback"))
      pHostEdict = pEntity; // save the edict of the listen server client...

   RETURN_META_VALUE (MRES_IGNORED, 0);
}


void ClientDisconnect (edict_t *pEntity)
{
   // Client disconnects from this Server

   int i;
   i = ENTINDEX (pEntity) - 1;
   if ((i >= 0) && (i < gpGlobals->maxClients))
   {
      pEntity->v.light_level = 0;
      // Find & remove this Client from our list of Clients connected
      clients[i].welcome_time = 0.0;
      clients[i].wptmessage_time = 0.0;
      clients[i].fDeathTime = 0.0;  // KWo - 14.03.2010
      memset (&clients[i], 0, sizeof (client_t));
   }

   // Check if its a Bot
   if (bots[i].pEdict == pEntity)
   {
      // Delete Nodes from Pathfinding
      DeleteSearchNodes (&bots[i]);
      BotResetTasks (&bots[i]);
      memset (&bots[i], 0, sizeof (bot_t));
      bots[i].is_used = FALSE; // this slot is now free to use
      bots[i].pEdict = NULL;
      bots[i].f_kick_time = gpGlobals->time; // save the kicked time
   }

   // Check if its the Host disconnecting
   if (pEntity == pHostEdict)
      pHostEdict = NULL;

   RETURN_META (MRES_IGNORED);
}


void ClientPutInServer (edict_t *pEntity)
{
   // Client is finally put into the Server

   int i;
   bool bWelcome = false;
   i = ENTINDEX (pEntity) - 1;

   if (g_rgcvarPointer[PBCVAR_WELCOMEMSGS])
      if (g_rgcvarPointer[PBCVAR_WELCOMEMSGS]->value > 0.f)
         bWelcome = true;
   else
      if (CVAR_GET_FLOAT (g_rgpszPbCvars[PBCVAR_WELCOMEMSGS]) > 0.f)
         bWelcome = true;

   if ((i >= 0) && (i < 32))
   {
      clients[i].pEdict = pEntity;
      clients[i].iFlags |= CLIENT_USED;

      if (bWelcome)
      {
         clients[i].welcome_time = -1.0;     // KWo - 17.04.2010
         clients[i].wptmessage_time = -1.0;  // KWo - 17.04.2010
         clients[i].fTimeSoundLasting = 0.0;    // KWo - 15.08.2007
         clients[i].fMaxTimeSoundLasting = 0.5; // KWo - 15.08.2007
         clients[i].fReloadingTime = 0.0;       // KWo - 15.08.2007
         clients[i].fDeathTime = 0.0;           // KWo - 14.03.2010
      }
   }
   RETURN_META (MRES_IGNORED);
}


void ClientCommand (edict_t *pEntity)
{
   // Executed if a client typed some sort of command into the console
   const char * pcmd = CMD_ARGV(0);	// KWo - 17.01.2006
   const char * arg1 = CMD_ARGV(1);
   const char * arg2 = CMD_ARGV(2);
   const char * arg3 = CMD_ARGV(3);
   const char * arg4 = CMD_ARGV(4);
   const char * arg5 = CMD_ARGV(5);
   const char * arg6 = CMD_ARGV(6);
//   const char * arg7 = CMD_ARGV(7);
//   const char * arg8 = CMD_ARGV(8);
//   const char * arg9 = CMD_ARGV(9);

   edict_t *pSpawnPoint = NULL;
   int iClientIndex = ENTINDEX (pEntity) - 1;
   int iRadioCommand;
   int i;
   char szTemp[64];
   client_t *pClient = &clients[iClientIndex];
   // don't search ClientCommands of Bots or other Edicts than Admins!
   if (!isFakeClientCommand && IsPBAdmin (pEntity))
   {
      // "pb" Check if any were the Podbot commands..
      if (FStrEq (pcmd, g_rgpszPbCmds[PBCMD]))
      {
         PbCmdParser (pEntity, arg1, arg2, arg3, arg4, arg5, arg6);
         RETURN_META (MRES_SUPERCEDE);
      }

      // Care for Menus instead...
      else if (pClient->pUserMenu != NULL)
      {
         if (FStrEq (pcmd, "menuselect"))
         {
            // Waypoint Main Menu
            if (pClient->pUserMenu == &menuWpMain)
            {
               UTIL_ShowMenu (pEntity, NULL); // reset menu display

               if (FStrEq (arg1, "1"))
               {
                  g_bWaypointOn = TRUE; // turn waypoints on if off
                  UTIL_ShowMenu (pEntity, &menuWpAdd);
               }
               else if (FStrEq (arg1, "2"))
               {
                  g_bWaypointOn = TRUE; // turn waypoints on if off
                  UTIL_ShowMenu (pEntity, &menuWpDelete);
               }
               else if (FStrEq (arg1, "3"))
               {
                  g_bWaypointOn = TRUE; // turn waypoints on if off
                  UTIL_ShowMenu (pEntity, &menuWpSetRadius);
               }
               else if (FStrEq (arg1, "4"))
               {
                  g_bWaypointOn = TRUE; // turn waypoints on if off
                  UTIL_ShowMenu (pEntity, &menuWpSetFlags);
               }
               else if (FStrEq (arg1, "5"))
               {
                  g_bWaypointOn = TRUE; // turn waypoints on if off
                  UTIL_ShowMenu (pEntity, &menuWpAddPath);
               }
               else if (FStrEq (arg1, "6"))
               {
                  g_bWaypointOn = TRUE; // turn waypoints on if off
                  UTIL_ShowMenu (pEntity, &menuWpDeletePath);
               }
               else if (FStrEq (arg1, "7"))
               {
                  if (WaypointNodesValid ())
                     UTIL_HostPrint ("All Nodes work fine !\n");
               }
               else if (FStrEq (arg1, "8"))
               {
                  if (WaypointNodesValid ())
                  {
                     WaypointSave ();
                     UTIL_HostPrint ("Waypoints saved!\n");
                  }
                  else
                     UTIL_ShowMenu (pEntity, &menuWpSave);
               }
               else if (FStrEq (arg1, "9"))
                  UTIL_ShowMenu (pEntity, &menuWpOptions1);

               RETURN_META (MRES_SUPERCEDE);
            }

            // Waypoint Add Menu
            else if (pClient->pUserMenu == &menuWpAdd)
            {
               UTIL_ShowMenu (pEntity, NULL); // reset menu display

               if (FStrEq (arg1, "1"))
                  WaypointAdd (WAYPOINT_ADD_NORMAL); // normal
               else if (FStrEq (arg1, "2"))
                  WaypointAdd (WAYPOINT_ADD_TERRORIST); // t important
               else if (FStrEq (arg1, "3"))
                  WaypointAdd (WAYPOINT_ADD_COUNTER); // ct important
               else if (FStrEq (arg1, "4"))
                  WaypointAdd (WAYPOINT_ADD_LADDER); // ladder
               else if (FStrEq (arg1, "5"))
                  WaypointAdd (WAYPOINT_ADD_RESCUE); // rescue
               else if (FStrEq (arg1, "6"))
                  WaypointAdd (WAYPOINT_ADD_CAMP_START); // camp start
               else if (FStrEq (arg1, "7"))
                  WaypointAdd (WAYPOINT_ADD_CAMP_END); // camp end
               else if (FStrEq (arg1, "8"))
                  WaypointAdd (WAYPOINT_ADD_GOAL); // goal
               else if (FStrEq (arg1, "9"))
               {
                  g_bLearnJumpWaypoint = TRUE;
                  UTIL_HostPrint ("Observation on !\n");
                  if (g_b_cv_UseSpeech)
                     SERVER_COMMAND ("speak \"movement check ok\"\n");
               }

               RETURN_META (MRES_SUPERCEDE);
            }

            // Waypoint Delete Menu
            else if (pClient->pUserMenu == &menuWpDelete)
            {
               UTIL_ShowMenu (pEntity, NULL); // reset menu display

               if (FStrEq (arg1, "1"))
                  WaypointDelete ();

               RETURN_META (MRES_SUPERCEDE);
            }

            // Waypoint SetRadius Menu
            else if (pClient->pUserMenu == &menuWpSetRadius)
            {
               UTIL_ShowMenu (pEntity, NULL); // reset menu display

               if (FStrEq (arg1, "1"))
                  WaypointChangeRadius (0);
               else if (FStrEq (arg1, "2"))
                  WaypointChangeRadius (8);
               else if (FStrEq (arg1, "3"))
                  WaypointChangeRadius (16);
               else if (FStrEq (arg1, "4"))
                  WaypointChangeRadius (32);
               else if (FStrEq (arg1, "5"))
                  WaypointChangeRadius (48);
               else if (FStrEq (arg1, "6"))
                  WaypointChangeRadius (64);
               else if (FStrEq (arg1, "7"))
                  WaypointChangeRadius (80);
               else if (FStrEq (arg1, "8"))
                  WaypointChangeRadius (96);
               else if (FStrEq (arg1, "9"))
                  WaypointChangeRadius (112);

               RETURN_META (MRES_SUPERCEDE);
            }

            // Waypoint SetFlags Menu
            else if (pClient->pUserMenu == &menuWpSetFlags)
            {
               UTIL_ShowMenu (pEntity, NULL); // reset menu display
               if (FStrEq (arg1, "1"))
                  WaypointChangeFlag (W_FL_USE_BUTTON, FLAG_TOGGLE);
               else if (FStrEq (arg1, "2"))
                  WaypointChangeFlag (W_FL_LIFT, FLAG_TOGGLE);
               else if (FStrEq (arg1, "3"))
                  WaypointChangeFlag (W_FL_CROUCH, FLAG_TOGGLE);
               else if (FStrEq (arg1, "4"))
                  WaypointChangeFlag (W_FL_GOAL, FLAG_TOGGLE);
               else if (FStrEq (arg1, "5"))
                  WaypointChangeFlag (W_FL_LADDER, FLAG_TOGGLE);
               else if (FStrEq (arg1, "6"))
                  WaypointChangeFlag (W_FL_RESCUE, FLAG_TOGGLE);
               else if (FStrEq (arg1, "7"))
                  WaypointChangeFlag (W_FL_CAMP, FLAG_TOGGLE);
               else if (FStrEq (arg1, "8"))
                  WaypointChangeFlag (W_FL_NOHOSTAGE, FLAG_TOGGLE);
               else if (FStrEq (arg1, "9"))
                  UTIL_ShowMenu (pEntity, &menuWpSetTeam);

               RETURN_META (MRES_SUPERCEDE);
            }

            // Waypoint Set Team Menu
            else if (pClient->pUserMenu == &menuWpSetTeam)
            {
               UTIL_ShowMenu (pEntity, NULL); // reset menu display

               if (FStrEq (arg1, "1"))
               {
                  WaypointChangeFlag (W_FL_TERRORIST, FLAG_SET);
                  WaypointChangeFlag (W_FL_COUNTER, FLAG_CLEAR);
               }
               else if (FStrEq (arg1, "2"))
               {
                  WaypointChangeFlag (W_FL_TERRORIST, FLAG_CLEAR);
                  WaypointChangeFlag (W_FL_COUNTER, FLAG_SET);
               }
               else if (FStrEq (arg1, "3"))
               {
                  WaypointChangeFlag (W_FL_TERRORIST, FLAG_CLEAR);
                  WaypointChangeFlag (W_FL_COUNTER, FLAG_CLEAR);
               }

               RETURN_META (MRES_SUPERCEDE);
            }

            // Waypoint Add Path Menu
            else if (pClient->pUserMenu == &menuWpAddPath)
            {
               UTIL_ShowMenu (pEntity, NULL); // reset menu display

               if (FStrEq (arg1, "1"))
                  WaypointCreatePath (PATH_OUTGOING);
               else if (FStrEq (arg1, "2"))
                  WaypointCreatePath (PATH_INCOMING);
               else if (FStrEq (arg1, "3"))
                  WaypointCreatePath (PATH_BOTHWAYS);

               RETURN_META (MRES_SUPERCEDE);
            }

            // Waypoint Delete Path Menu
            else if (pClient->pUserMenu == &menuWpDeletePath)
            {
               UTIL_ShowMenu (pEntity, NULL); // reset menu display

               if (FStrEq (arg1, "1"))
                  WaypointDeletePath ();

               RETURN_META (MRES_SUPERCEDE);
            }

            // Waypoint Save Menu
            else if (pClient->pUserMenu == &menuWpSave)
            {
               UTIL_ShowMenu (pEntity, NULL); // reset menu display

               if (FStrEq (arg1, "1"))
               {
                  WaypointSave ();
                  UTIL_HostPrint ("WARNING: Waypoints saved with errors!\n");
               }

               RETURN_META (MRES_SUPERCEDE);
            }

            // Waypoint Options Menu 1
            else if (pClient->pUserMenu == &menuWpOptions1)
            {
               UTIL_ShowMenu (pEntity, NULL); // reset menu display

               if (FStrEq (arg1, "1")) // wp on/off
               {
                  g_bWaypointOn ^= TRUE; // switch variable on/off (XOR it)

                  if (g_bWaypointOn)
                  {
                     UTIL_HostPrint ("Waypoints Editing is ON\n");

                     while (!FNullEnt (pSpawnPoint = FIND_ENTITY_BY_STRING (pSpawnPoint, "classname", "info_player_start")))
                        pSpawnPoint->v.effects &= ~EF_NODRAW;
                     while (!FNullEnt (pSpawnPoint = FIND_ENTITY_BY_STRING (pSpawnPoint, "classname", "info_player_deathmatch")))
                        pSpawnPoint->v.effects &= ~EF_NODRAW;
                     while (!FNullEnt (pSpawnPoint = FIND_ENTITY_BY_STRING (pSpawnPoint, "classname", "info_vip_start")))
                        pSpawnPoint->v.effects &= ~EF_NODRAW;
                  }
                  else
                  {
                     UTIL_HostPrint ("Waypoint Editing turned OFF\n");

                     while (!FNullEnt (pSpawnPoint = FIND_ENTITY_BY_STRING (pSpawnPoint, "classname", "info_player_start")))
                        pSpawnPoint->v.effects |= EF_NODRAW;
                     while (!FNullEnt (pSpawnPoint = FIND_ENTITY_BY_STRING (pSpawnPoint, "classname", "info_player_deathmatch")))
                        pSpawnPoint->v.effects |= EF_NODRAW;
                     while (!FNullEnt (pSpawnPoint = FIND_ENTITY_BY_STRING (pSpawnPoint, "classname", "info_vip_start")))
                        pSpawnPoint->v.effects |= EF_NODRAW;

                     if (g_bWaypointsChanged && g_bWaypointsSaved)
                     {
                        UTIL_HostPrint ("The map will restart in 5 seconds!\n");
                        g_fTimeRestartServer = gpGlobals->time + 4.0;
                     }
                     else if (g_bWaypointsChanged)
                        UTIL_HostPrint ("Don't forget to SAVE your waypoints...\n");
                  }
               }
               else if (FStrEq (arg1, "2"))
               {
                  g_bAutoWaypoint ^= TRUE; // Switch Variable on/off (XOR it)
                  UTIL_HostPrint ("Auto-Waypointing is %s\n", (g_bAutoWaypoint ? "ENABLED" : "DISABLED"));
               }
               else if (FStrEq (arg1, "3"))
               {
                  g_bEditNoclip ^= TRUE; // Switch Variable on/off (XOR it)
                  if (g_bEditNoclip)
                     pHostEdict->v.movetype = MOVETYPE_NOCLIP;
                  else
                     pHostEdict->v.movetype = MOVETYPE_WALK;
                  UTIL_HostPrint ("No Clipping Cheat is %s\n", (g_bEditNoclip ? "ENABLED" : "DISABLED"));
               }
               else if (FStrEq (arg1, "4"))
               {
                  g_bIgnoreEnemies ^= TRUE; // Switch Variable on/off (XOR it)
                  UTIL_HostPrint ("Peace Mode is %s (Bots %signore Enemies)\n", (g_bIgnoreEnemies ? "ENABLED" : "DISABLED"), (g_bIgnoreEnemies ? "" : "DON'T "));
               }
               else if (FStrEq (arg1, "5"))
               {
                  g_bShowWpFlags ^= TRUE; // Switch Variable on/off (XOR it)
                  UTIL_HostPrint ("Waypoint Flag display is %s\n", (g_bShowWpFlags ? "ENABLED" : "DISABLED"));
               }
               else if (FStrEq (arg1, "6"))
                  UTIL_ShowMenu (pEntity, &menuWpAutoPathMaxDistance);
               else if (FStrEq (arg1, "7"))
                  WaypointCache ();
               else if (FStrEq (arg1, "8")) // KWo - 29.03.2008
               {
                  if (g_iCachedWaypoint == -1)
                     UTIL_HostPrint ("No cached waypoint to move.\n");
                  else if ((g_iCachedWaypoint >= 0) && (g_iCachedWaypoint < g_iNumWaypoints))
                  {
                     WaypointMoveToPosition();
                  }
               }
               else if (FStrEq (arg1, "9")) // KWo - 29.03.2008
               {
                  UTIL_ShowMenu (pEntity, &menuWpOptions2);
               }
               RETURN_META (MRES_SUPERCEDE);
            }

            // Waypoint Options Menu 2
            else if (pClient->pUserMenu == &menuWpOptions2)
            {
               UTIL_ShowMenu (pEntity, NULL); // reset menu display

               if (FStrEq (arg1, "1")) // wp on/off
               {
                  i = WaypointLookAt();
                  if (i >= 0)
                  {
                     snprintf (szTemp, sizeof (szTemp), "%d", i);
                     PbCmdParser (pEntity, g_rgpszPbCmds[PBCMD_DEBUGGOAL], szTemp, NULL, NULL, NULL, NULL);
                  }
               }
               else if (FStrEq (arg1, "2"))
               {
                  if (g_iDebugGoalIndex != -1)
                  {
                     snprintf (szTemp, sizeof (szTemp), "%d", -1);
                     PbCmdParser (pEntity, g_rgpszPbCmds[PBCMD_DEBUGGOAL], szTemp, NULL, NULL, NULL, NULL);
                  }
               }
               else if (FStrEq (arg1, "8")) // KWo - 04.10.2006
               {
                  UTIL_ShowMenu (pEntity, &menuWpOptions1);
               }
               RETURN_META (MRES_SUPERCEDE);
            }

            // Waypoint AutoPathMaxDistance Menu
            else if (pClient->pUserMenu == &menuWpAutoPathMaxDistance)
            {
               UTIL_ShowMenu (pEntity, NULL); // reset menu display

               if (FStrEq (arg1, "1"))
                  g_fAutoPathMaxDistance = 0;
               else if (FStrEq (arg1, "2"))
                  g_fAutoPathMaxDistance = 100;
               else if (FStrEq (arg1, "3"))
                  g_fAutoPathMaxDistance = 130;
               else if (FStrEq (arg1, "4"))
                  g_fAutoPathMaxDistance = 160;
               else if (FStrEq (arg1, "5"))
                  g_fAutoPathMaxDistance = 190;
               else if (FStrEq (arg1, "6"))
                  g_fAutoPathMaxDistance = 220;
               else if (FStrEq (arg1, "7"))
                  g_fAutoPathMaxDistance = 250;

               if (g_fAutoPathMaxDistance == 0)
                  UTIL_HostPrint ("Auto-path disabled\n");
               else
                  UTIL_HostPrint ("Auto-path Max Distance set to %f\n", g_fAutoPathMaxDistance);

               RETURN_META (MRES_SUPERCEDE);
            }

            // Main PODBot Menu ?
            else if (pClient->pUserMenu == &menuPODBotMain)
            {
               UTIL_ShowMenu (pEntity, NULL); // reset menu display

               if (FStrEq (arg1, "1"))
               {
                  PbCmdParser (pEntity, g_rgpszPbCmds[PBCMD_ADD], NULL, NULL, NULL, NULL, NULL);
               }
               else if (FStrEq (arg1, "2"))
                  UTIL_ShowMenu (pEntity, &menuPODBotAddBotSkill);
               else if (FStrEq (arg1, "3"))
               {
                  PbCmdParser (pEntity, g_rgpszPbCmds[PBCMD_KILLBOTS], NULL, NULL, NULL, NULL, NULL);
               }
               else if (FStrEq (arg1, "4"))
                  UserNewroundAll ();
               else if (FStrEq (arg1, "5"))
                  UTIL_ShowMenu (pEntity, &menuPODBotFillServerSkill);
               else if (FStrEq (arg1, "6"))
                  ShowPBKickBotMenu (pEntity, 1);
               else if (FStrEq (arg1, "7"))
               {
                  PbCmdParser (pEntity, g_rgpszPbCmds[PBCMD_REMOVEBOTS], NULL, NULL, NULL, NULL, NULL);
               }
               else if (FStrEq (arg1, "8"))
                  UTIL_ShowMenu (pEntity, &menuPODBotWeaponMode);

               RETURN_META (MRES_SUPERCEDE);
            }

            // Add Bot Skill menu ?
            else if (pClient->pUserMenu == &menuPODBotAddBotSkill)
            {
               UTIL_ShowMenu (pEntity, NULL); // reset menu display

               if (FStrEq (arg1, "1"))
               {
                  snprintf (g_cStoreAddbotSkill, sizeof (g_cStoreAddbotSkill), "%d", RANDOM_LONG (1, 19));
                  UTIL_ShowMenu (pEntity, &menuPODBotAddBotPersonality);
               }
               else if (FStrEq (arg1, "2"))
               {
                  snprintf (g_cStoreAddbotSkill, sizeof (g_cStoreAddbotSkill), "%d", RANDOM_LONG (20, 39));
                  UTIL_ShowMenu (pEntity, &menuPODBotAddBotPersonality);
               }
               else if (FStrEq (arg1, "3"))
               {
                  snprintf (g_cStoreAddbotSkill, sizeof (g_cStoreAddbotSkill), "%d", RANDOM_LONG (40, 59));
                  UTIL_ShowMenu (pEntity, &menuPODBotAddBotPersonality);
               }
               else if (FStrEq (arg1, "4"))
               {
                  snprintf (g_cStoreAddbotSkill, sizeof (g_cStoreAddbotSkill), "%d", RANDOM_LONG (60, 79));
                  UTIL_ShowMenu (pEntity, &menuPODBotAddBotPersonality);
               }
               else if (FStrEq (arg1, "5"))
               {
                  snprintf (g_cStoreAddbotSkill, sizeof (g_cStoreAddbotSkill), "%d", RANDOM_LONG (80, 99));
                  UTIL_ShowMenu (pEntity, &menuPODBotAddBotPersonality);
               }
               else if (FStrEq (arg1, "6"))
               {
                  g_cStoreAddbotSkill[0] = '1';
                  g_cStoreAddbotSkill[1] = '0';
                  g_cStoreAddbotSkill[2] = '0';
                  g_cStoreAddbotSkill[3] = 0;
                  UTIL_ShowMenu (pEntity, &menuPODBotAddBotPersonality);
               }
               else if (FStrEq (arg1, "7"))
               {
                  g_cStoreAddbotSkill[0] = '0';
                  g_cStoreAddbotSkill[1] = 0;
                  UTIL_ShowMenu (pEntity, &menuPODBotAddBotPersonality);
               }
               else if (FStrEq (arg1, "9"))	// KWo 09.01.2006
                  UTIL_ShowMenu (pEntity, &menuPODBotMain);
               RETURN_META (MRES_SUPERCEDE);
            }

            // Add Bot Personality menu ?
            else if (pClient->pUserMenu == &menuPODBotAddBotPersonality)
            {
               UTIL_ShowMenu (pEntity, NULL); // reset menu display

               if ((atoi (arg1) >= 1) && (atoi (arg1) <= 3))
               {
                  g_cStoreAddbotPersonality[0] = arg1[0];	// KWo 08.10.2006
                  g_cStoreAddbotPersonality[1] = 0;	// KWo 08.10.2006
                  UTIL_ShowMenu (pEntity, &menuPODBotAddBotTeam);
               }
               else if (FStrEq (arg1, "5"))
               {
                  g_cStoreAddbotPersonality[0] = arg1[0];	// KWo 08.10.2006
                  g_cStoreAddbotPersonality[1] = 0;	// KWo 08.10.2006
                  UTIL_ShowMenu (pEntity, &menuPODBotAddBotTeam);
               }


               RETURN_META (MRES_SUPERCEDE);
            }

            // Add Bot Team Select Menu ?
            else if (pClient->pUserMenu == &menuPODBotAddBotTeam)
            {
               UTIL_ShowMenu (pEntity, NULL); // reset menu display

               if (atoi (arg1) >= 1 && (atoi (arg1) <= 2))
               {
                  g_cStoreAddbotTeam[0] = arg1[0];	// KWo 08.01.2006
                  g_cStoreAddbotTeam[1] = 0;	// KWo 08.01.2006
                  if (atoi (arg1) == TEAM_CS_TERRORIST)
                     UTIL_ShowMenu (pEntity, &menuPODBotAddBotTModel);
                  else // TEAM_CS_COUNTER
                     UTIL_ShowMenu (pEntity, &menuPODBotAddBotCTModel);
               }
               else if (FStrEq (arg1, "5"))
               {
                  PbCmdParser (pEntity, g_rgpszPbCmds[PBCMD_ADD], g_cStoreAddbotSkill, g_cStoreAddbotPersonality, arg1, NULL, NULL);  // KWo 08.01.2006
                  UTIL_ShowMenu (pEntity, &menuPODBotAddBotSkill);  // KWo 09.10.2006
               }

               RETURN_META (MRES_SUPERCEDE);
            }

            // Add Bot Model Selection Menu ?
            else if ((pClient->pUserMenu == &menuPODBotAddBotTModel)
                     || (pClient->pUserMenu == &menuPODBotAddBotCTModel))
            {
               UTIL_ShowMenu (pEntity, NULL); // reset menu display

               if ((atoi (arg1) >= 1) && (atoi (arg1) <= 5))
               {
                  PbCmdParser (pEntity, g_rgpszPbCmds[PBCMD_ADD], g_cStoreAddbotSkill, g_cStoreAddbotPersonality, g_cStoreAddbotTeam, arg1, NULL);  // KWo 08.10.2006
                  UTIL_ShowMenu (pEntity, &menuPODBotAddBotSkill);  // KWo 09.10.2006
               }

               RETURN_META (MRES_SUPERCEDE);
            }

            // Fill Server Bot Skill menu ?
            else if (pClient->pUserMenu == &menuPODBotFillServerSkill)
            {
               UTIL_ShowMenu (pEntity, NULL); // reset menu display

               if (FStrEq (arg1, "1"))
               {
                  snprintf (g_cStoreFillServerSkill, sizeof (g_cStoreFillServerSkill), "%d", RANDOM_LONG (1, 19));
                  UTIL_ShowMenu (pEntity, &menuPODBotFillServerPersonality);
               }
               else if (FStrEq (arg1, "2"))
               {
                  snprintf (g_cStoreFillServerSkill, sizeof (g_cStoreFillServerSkill), "%d", RANDOM_LONG (20, 39));
                  UTIL_ShowMenu (pEntity, &menuPODBotFillServerPersonality);
               }
               else if (FStrEq (arg1, "3"))
               {
                  snprintf (g_cStoreFillServerSkill, sizeof (g_cStoreFillServerSkill), "%d", RANDOM_LONG (40, 59));
                  UTIL_ShowMenu (pEntity, &menuPODBotFillServerPersonality);
               }
               else if (FStrEq (arg1, "4"))
               {
                  snprintf (g_cStoreFillServerSkill, sizeof (g_cStoreFillServerSkill), "%d", RANDOM_LONG (60, 79));
                  UTIL_ShowMenu (pEntity, &menuPODBotFillServerPersonality);
               }
               else if (FStrEq (arg1, "5"))
               {
                  snprintf (g_cStoreFillServerSkill, sizeof (g_cStoreFillServerSkill), "%d", RANDOM_LONG (80, 99));
                  UTIL_ShowMenu (pEntity, &menuPODBotFillServerPersonality);
               }
               else if (FStrEq (arg1, "6"))
               {
                  g_cStoreFillServerSkill[0] = '1';
                  g_cStoreFillServerSkill[1] = '0';
                  g_cStoreFillServerSkill[2] = '0';
                  g_cStoreFillServerSkill[3] = 0;
                  UTIL_ShowMenu (pEntity, &menuPODBotFillServerPersonality);
               }
               else if (FStrEq (arg1, "7"))
               {
                  g_cStoreFillServerSkill[0] = '0';
                  g_cStoreFillServerSkill[1] = 0;
                  UTIL_ShowMenu (pEntity, &menuPODBotFillServerPersonality);
               }

               RETURN_META (MRES_SUPERCEDE);
            }

            // Fill Server Bot Personality menu ?
            else if (pClient->pUserMenu == &menuPODBotFillServerPersonality)
            {
               UTIL_ShowMenu (pEntity, NULL); // reset menu display

               if ((atoi (arg1) >= 1) && (atoi (arg1) <= 3))
               {
                  g_cStoreFillServerPersonality[0] = arg1[0];	// KWo 08.01.2006
                  g_cStoreFillServerPersonality[1] = 0;	// KWo 08.01.2006
                  UTIL_ShowMenu (pEntity, &menuPODBotFillServerTeam);
               }
               else if (FStrEq (arg1, "5"))
               {
                  g_cStoreFillServerPersonality[0] = arg1[0];	// KWo 08.0.2006
                  g_cStoreFillServerPersonality[1] = 0;	// KWo 08.0.2006
                  UTIL_ShowMenu (pEntity, &menuPODBotFillServerTeam);
               }

               RETURN_META (MRES_SUPERCEDE);
            }

            // Fill Server Team Menu Select ?
            else if (pClient->pUserMenu == &menuPODBotFillServerTeam)
            {
               UTIL_ShowMenu (pEntity, NULL); // reset menu display

               if (atoi (arg1) >= 1 && (atoi (arg1) <= 2))
               {
                  g_cStoreFillServerTeam[0] = arg1[0];	// KWo 08.01.2006
                  g_cStoreFillServerTeam[1] = 0;	// KWo 08.01.2006
                  if (atoi (arg1) == TEAM_CS_TERRORIST)
                     UTIL_ShowMenu (pEntity, &menuPODBotFillServerTModel);
                  else // TEAM_CS_COUNTER
                     UTIL_ShowMenu (pEntity, &menuPODBotFillServerCTModel);
               }
               else if (FStrEq (arg1, "5"))
               {
                  PbCmdParser (pEntity, g_rgpszPbCmds[PBCMD_FILLSERVER], g_cStoreFillServerSkill, g_cStoreFillServerPersonality, arg1, NULL, NULL);	// KWo 08.01.2006
               }

               RETURN_META (MRES_SUPERCEDE);
            }

            // Fill Server Model Menu Select ?
            else if ((pClient->pUserMenu == &menuPODBotFillServerTModel)
                     || (pClient->pUserMenu == &menuPODBotFillServerCTModel))
            {
               UTIL_ShowMenu (pEntity, NULL); // reset menu display

               if ((atoi (arg1) >= 1) && (atoi (arg1) <= 5))
               {
                  PbCmdParser (pEntity, g_rgpszPbCmds[PBCMD_FILLSERVER], g_cStoreFillServerSkill, g_cStoreFillServerPersonality, g_cStoreFillServerTeam, arg1, NULL);  // KWo 08.01.2006
               }

               RETURN_META (MRES_SUPERCEDE);
            }

            // Kick Bot1 Select Menu ?
            else if (pClient->pUserMenu == &menuPODBotKickBot1)
            {
               UTIL_ShowMenu (pEntity, NULL); // reset menu display

               if (FStrEq (arg1, "1") || FStrEq (arg1, "2")
                  || FStrEq (arg1, "3") || FStrEq (arg1, "4")
                  || FStrEq (arg1, "5") || FStrEq (arg1, "6")
                  || FStrEq (arg1, "7") || FStrEq (arg1, "8"))
               {
                  memset (szTemp, 0, sizeof (szTemp));
                  i = atoi (arg1) - 1;
                  snprintf (szTemp, sizeof (szTemp), "%s", STRING (bots[i].pEdict->v.netname));
                  PbCmdParser (pEntity, g_rgpszPbCmds[PBCMD_REMOVE], szTemp, NULL, NULL, NULL, NULL);
                  ShowPBKickBotMenu (pEntity, 1);  // KWo 11.02.2006 (here is a need to add some delay and a task to show it...)
               }
               else if (FStrEq (arg1, "9"))
                  ShowPBKickBotMenu (pEntity, 2);
               else if (FStrEq (arg1, "10"))
                  UTIL_ShowMenu (pEntity, &menuPODBotMain);

               RETURN_META (MRES_SUPERCEDE);
            }

            // Kick Bot2 Select Menu ?
            else if (pClient->pUserMenu == &menuPODBotKickBot2)
            {
               UTIL_ShowMenu (pEntity, NULL); // reset menu display

               if (FStrEq (arg1, "1") || FStrEq (arg1, "2")
                  || FStrEq (arg1, "3") || FStrEq (arg1, "4")
                  || FStrEq (arg1, "5") || FStrEq (arg1, "6")
                  || FStrEq (arg1, "7") || FStrEq (arg1, "8"))
               {
                  memset (szTemp, 0, sizeof (szTemp));
                  i = atoi (arg1) + 8 - 1;
                  snprintf (szTemp, sizeof (szTemp), "%s", STRING (bots[i].pEdict->v.netname));
                  PbCmdParser (pEntity, g_rgpszPbCmds[PBCMD_REMOVE], szTemp, NULL, NULL, NULL, NULL);
                  ShowPBKickBotMenu (pEntity, 2);  // KWo 11.02.2006 (here is a need to add some delay and a task to show it...)
               }
               else if (FStrEq (arg1, "9"))
                  ShowPBKickBotMenu (pEntity, 3);
               else if (FStrEq (arg1, "10"))
                  ShowPBKickBotMenu (pEntity, 1);

               RETURN_META (MRES_SUPERCEDE);
            }

            // Kick Bot3 Select Menu ?
            else if (pClient->pUserMenu == &menuPODBotKickBot3)
            {
               UTIL_ShowMenu (pEntity, NULL); // reset menu display

               if (FStrEq (arg1, "1") || FStrEq (arg1, "2")
                  || FStrEq (arg1, "3") || FStrEq (arg1, "4")
                  || FStrEq (arg1, "5") || FStrEq (arg1, "6")
                  || FStrEq (arg1, "7") || FStrEq (arg1, "8"))
               {
                  memset (szTemp, 0, sizeof (szTemp));
                  i = atoi (arg1) + 16 - 1;
                  snprintf (szTemp, sizeof (szTemp), "%s", STRING (bots[i].pEdict->v.netname));
                  PbCmdParser (pEntity, g_rgpszPbCmds[PBCMD_REMOVE], szTemp, NULL, NULL, NULL, NULL);
                  ShowPBKickBotMenu (pEntity, 3);  // KWo 11.02.2006 (here is a need to add some delay and a task to show it...)
               }
               else if (FStrEq (arg1, "9"))
                  ShowPBKickBotMenu (pEntity, 4);
               else if (FStrEq (arg1, "10"))
                  ShowPBKickBotMenu (pEntity, 2);

               RETURN_META (MRES_SUPERCEDE);
            }

            // Kick Bot4 Select Menu ?
            else if (pClient->pUserMenu == &menuPODBotKickBot4)
            {
               UTIL_ShowMenu (pEntity, NULL); // reset menu display

               if (FStrEq (arg1, "1") || FStrEq (arg1, "2")
                  || FStrEq (arg1, "3") || FStrEq (arg1, "4")
                  || FStrEq (arg1, "5") || FStrEq (arg1, "6")
                  || FStrEq (arg1, "7") || FStrEq (arg1, "8"))
               {
                  memset (szTemp, 0, sizeof (szTemp));
                  i = atoi (arg1) + 24 - 1;
                  snprintf (szTemp, sizeof (szTemp), "%s", STRING (bots[i].pEdict->v.netname));
                  PbCmdParser (pEntity, g_rgpszPbCmds[PBCMD_REMOVE], szTemp, NULL, NULL, NULL, NULL);
                  ShowPBKickBotMenu (pEntity, 4);  // KWo 11.02.2006 (here is a need to add some delay and a task to show it...)
               }
               else if (FStrEq (arg1, "10"))
                  ShowPBKickBotMenu (pEntity, 3);

               RETURN_META (MRES_SUPERCEDE);
            }

            // Weapon Mode Selection Menu ?
            else if (pClient->pUserMenu == &menuPODBotWeaponMode)
            {
               UTIL_ShowMenu (pEntity, NULL); // reset menu display

               if ((atoi (arg1) >= 1) && (atoi (arg1) <= 7))
               {
                  PbCmdParser (pEntity, g_rgpszPbCmds[PBCMD_WEAPONMODE], arg1, NULL, NULL, NULL, NULL);
               }

               RETURN_META (MRES_SUPERCEDE);
            }
         }

         RETURN_META (MRES_SUPERCEDE);
      }
   }

   // Check Radio Commands
   if (clients[iClientIndex].iFlags & CLIENT_ALIVE)
   {
      if ((iRadioSelect[iClientIndex] != 0) && FStrEq (pcmd, "menuselect"))
      {
         iRadioCommand = atoi (arg1);

         if (iRadioCommand != 0)
         {
            iRadioCommand += 10 * (iRadioSelect[iClientIndex] - 1);

            if ((iRadioCommand != RADIO_AFFIRMATIVE)
                && (iRadioCommand != RADIO_NEGATIVE)
                && (iRadioCommand != RADIO_REPORTINGIN))
            {
               for (i = 0; i < gpGlobals->maxClients; i++)
               {
                  if (bots[i].is_used && (bots[i].bot_team == clients[iClientIndex].iTeam)
                      && (pEntity != bots[i].pEdict))
                  {
                     if (bots[i].iRadioOrder == 0)
                     {
                        bots[i].iRadioOrder = iRadioCommand;
                        bots[i].pRadioEntity = pEntity;
                     }
                  }
               }
            }

            g_rgfLastRadioTime[clients[iClientIndex].iTeam - 1] = gpGlobals->time;
         }

         iRadioSelect[iClientIndex] = 0;
      }

      else if (strncmp (pcmd, "radio", 5) == 0)
         iRadioSelect[iClientIndex] = atoi (pcmd + 5);
   }
   // End Radio Commands

   RETURN_META (MRES_IGNORED);
}


void ClientUserInfoChanged_Post (edict_t *pEntity, char *infobuffer)
{
   char szBotModelName[64]; // KWo - 20.10.2006
   int EntInd = ENTINDEX(pEntity) - 1; // KWo - 20.10.2006
   if ((bots[EntInd].is_used) && (bots[EntInd].pEdict == pEntity)) // KWo - 20.10.2006
   {
      strncpy (szBotModelName, (INFOKEY_VALUE (infobuffer, "model")), sizeof (szBotModelName));
      strncpy (bots[EntInd].sz_BotModelName, szBotModelName, sizeof (szBotModelName));
//      UTIL_ServerPrint("[DEBUG] Bot %s changed the model to %s.\n", STRING(pEntity->v.netname), szBotModelName);
      RETURN_META (MRES_IGNORED);
   }

   char szPasswordField[64];
   if (g_rgcvarPointer[PBCVAR_PASSWORDKEY])  // KWo - 20.10.2006
   {
      strncpy (szPasswordField, g_rgcvarPointer[PBCVAR_PASSWORDKEY]->string, 63);
   }
   else
   {
      strncpy (szPasswordField, CVAR_GET_STRING (g_rgpszPbCvars[PBCVAR_PASSWORDKEY]), 63);
   }

   char szPassword[64];
   if (g_rgcvarPointer[PBCVAR_PASSWORD])  // KWo - 20.10.2006
   {
      strncpy (szPassword, g_rgcvarPointer[PBCVAR_PASSWORD]->string, 63);
   }
   else
   {
      strncpy (szPassword, CVAR_GET_STRING (g_rgpszPbCvars[PBCVAR_PASSWORD]), 63);
   }

   if ((*szPasswordField == 0) && (*szPassword == 0))
      RETURN_META (MRES_IGNORED);
   int iClientIndex = ENTINDEX (pEntity) - 1;
   const char *pszPBAdminLoginAttempt = INFOKEY_VALUE (infobuffer, const_cast<char *> (szPasswordField));
   if (FStrEq (szPassword, pszPBAdminLoginAttempt))
      clients[iClientIndex].iFlags |= CLIENT_ADMIN;
   else
      clients[iClientIndex].iFlags &= ~CLIENT_ADMIN;
   RETURN_META (MRES_IGNORED);
}


void ServerActivate (edict_t *pEdictList, int edictCount, int clientMax)
{
   char line_buffer[256];
   int iBufferSize = sizeof (line_buffer);
   int i, c;
   int iChatType;
   replynode_t *pTempReply = NULL;
   replynode_t **pReply = NULL;
   char arg0[80];
   char arg1[80];
   int *ptrWeaponPrefs;
   int iParseWeapons;
   int iWeaponPrefsType = MAP_DE; // KWo - to remove warning uninitialised
   char *pszStart;
   char *pszEnd;
   char filename[256];  // KWo - 18.03.2006
   STRINGNODE **pNode = NULL; // KWo - to remove warning uninitialised
   STRINGNODE *pTempNode;
   char szDirectory[256];

//   static char cmd_line[80];
   FILE *bot_cfg_fp = NULL;
   char cmd[128];

   BotFreeAllMemory ();

   // Load & Initialise Botnames from 'Botnames.txt'
   snprintf (szDirectory, sizeof (szDirectory), "%s/addons/podbot/botnames.txt", g_szGameDirectory);
   fp = fopen (szDirectory, "r");
   if (fp == NULL)
      UTIL_ServerPrint ("POD-Bot couldn't find botnames.txt!\n");
   else
   {
//      memset (szBotNames, 0, sizeof (szBotNames));
      iNumBotNames = 0;

      while ((fgets (line_buffer, iBufferSize, fp) != NULL))
      {
         if ((line_buffer[0] == '#') || (line_buffer[0] == 0) || (line_buffer[0] == '\r') || (line_buffer[0] == '\n'))
            continue; // ignore comments or blank lines

         ++iNumBotNames;
      }

      g_pszBotNames = new botname_t[iNumBotNames];
      for (i = 0; i < iNumBotNames; ++i)
         memset (g_pszBotNames + i, 0, sizeof (g_pszBotNames));
      fseek (fp, 0, SEEK_SET);
      c = 0;

      while ((fgets (line_buffer, iBufferSize, fp) != NULL))
      {
         if ((line_buffer[0] == '#') || (line_buffer[0] == 0) || (line_buffer[0] == '\r') || (line_buffer[0] == '\n'))
            continue; // ignore comments or blank lines

         i = (int)strlen (line_buffer);

         while ((line_buffer[i - 1] == '\n' || line_buffer[i - 1] == ' '
            || line_buffer[i - 1] == '\r' || line_buffer[i - 1] == '\t') && (i > 0))
            line_buffer[--i] = 0;

         strncpy ((g_pszBotNames + c)->name, line_buffer, sizeof (g_pszBotNames->name));
         (g_pszBotNames + c++)->name[sizeof (g_pszBotNames->name) - 1] = 0;
      }

      fclose (fp);
   }
   // End Botnames

   // Load & Initialise Botchats from 'Botchat.txt'
   snprintf (szDirectory, sizeof (szDirectory), "%s/addons/podbot/botchat.txt", g_szGameDirectory);
   fp = fopen (szDirectory, "r");
   if (fp == NULL)
      UTIL_ServerPrint ("POD-Bot couldn't find botchat.txt!\n");
   else
   {
      memset (szKillChat, 0, sizeof (szKillChat));
      memset (szBombChat, 0, sizeof (szBombChat));
      memset (szDeadChat, 0, sizeof (szDeadChat));
      memset (szNoKwChat, 0, sizeof (szNoKwChat));
      iNumKillChats = 0;
      iNumBombChats = 0;
      iNumDeadChats = 0;
      iNumNoKwChats = 0;

      iChatType = -1;

      while (fgets (line_buffer, iBufferSize, fp) != NULL)
      {
         if ((line_buffer[0] == '#') || (line_buffer[0] == 0) || (line_buffer[0] == '\r') || (line_buffer[0] == '\n'))
            continue; // ignore comments or blank lines

         strncpy (arg0, GetField (line_buffer, 0), sizeof (arg0));
         arg0[sizeof (arg0) - 1] = 0;

         // Killed Chat Section ?
         if (FStrEq (arg0, "[KILLED]"))
         {
            iChatType = 0;
            continue;
         }

         // Bomb Chat Section ?
         else if (FStrEq (arg0, "[BOMBPLANT]"))
         {
            iChatType = 1;
            continue;
         }

         // Dead Chat Section ?
         else if (FStrEq (arg0, "[DEADCHAT]"))
         {
            iChatType = 2;
            continue;
         }

         // Keyword Chat Section ?
         else if (FStrEq (arg0, "[REPLIES]"))
         {
            iChatType = 3;
            pReply = &pChatReplies;
            continue;
         }

         // Unknown Keyword Section ?
         else if (FStrEq (arg0, "[UNKNOWN]"))
         {
            iChatType = 4;
            continue;
         }

         if (iChatType == 0)
         {
            strncat (line_buffer, "\n", iBufferSize);
            line_buffer[79] = 0;

            strncpy (szKillChat[iNumKillChats], line_buffer, sizeof (szKillChat[iNumKillChats]));
            iNumKillChats++;
         }

         else if (iChatType == 1)
         {
            strncat (line_buffer, "\n", iBufferSize);
            line_buffer[79] = 0;

            strncpy (szBombChat[iNumBombChats], line_buffer, sizeof (szBombChat[iNumBombChats]));
            iNumBombChats++;
         }

         else if (iChatType == 2)
         {
            strncat (line_buffer, "\n", iBufferSize);
            line_buffer[79] = 0;

            strncpy (szDeadChat[iNumDeadChats], line_buffer, sizeof (szDeadChat[iNumDeadChats]));
            iNumDeadChats++;
         }

         else if (iChatType == 3)
         {
            if (strstr (line_buffer, "@KEY") != NULL)
            {
               pTempReply = new (replynode_t);
               *pReply = pTempReply;
               pTempReply->pNextReplyNode = NULL;
               pTempReply->pReplies = NULL;
               pTempReply->cNumReplies = 0;
               pTempReply->cLastReply[0] = 0;   // KWo - 27.03.2010
               pTempReply->cLastReply[1] = 0;   // KWo - 27.03.2010
               pTempReply->cLastReply[2] = 0;   // KWo - 27.03.2010
               pTempReply->cLastReply[3] = 0;   // KWo - 27.03.2010
               pNode = &pTempReply->pReplies;
               pTempNode = NULL;
               memset (pTempReply->szKeywords, 0, sizeof (pTempReply->szKeywords));

               c = 0;

               for (i = 0; i < iBufferSize; i++)
               {
                  if (line_buffer[i] == '\"')
                  {
                     i++;
                     while (line_buffer[i] != '\"')
                        pTempReply->szKeywords[c++] = line_buffer[i++];
                     pTempReply->szKeywords[c++] = '@';
                  }
                  else if (line_buffer[i] == 0)
                     break;
               }
               pReply = &pTempReply->pNextReplyNode;
            }
            else if (pTempReply)
            {
               strncat (line_buffer, "\n", iBufferSize);
               line_buffer[255] = 0;

               pTempNode = new (STRINGNODE);
               if (pTempNode == NULL)
                  UTIL_ServerPrint ("POD-Bot out of Memory!\n");
               else
               {
                  *pNode = pTempNode;
                  pTempNode->Next = NULL;
                  strncpy (pTempNode->szString, line_buffer, sizeof (pTempNode->szString));

                  pTempReply->cNumReplies++;
                  pNode = &pTempNode->Next;
               }
            }
         }

         else if (iChatType == 4)
         {
            strncat (line_buffer, "\n", iBufferSize);
            line_buffer[79] = 0;

            strncpy (szNoKwChat[iNumNoKwChats], line_buffer, sizeof (szNoKwChat[iNumNoKwChats]));
            iNumNoKwChats++;
         }
      }

      fclose (fp);
   }

   for (i = 0; i < 10; i++) // KWo - 23.03.2010
      iUsedDeadChatIndex[i] = -1;

   for (i = 0; i < 5; i++) // KWo - 27.03.2010
      iUsedUnknownChatIndex[i] = -1;

   // End Botchats

   // Load & Initialise Botskill.cfg
   snprintf (szDirectory, sizeof (szDirectory), "%s/addons/podbot/botskill.cfg", g_szGameDirectory);
   fp = fopen (szDirectory, "r");
   if (fp == NULL)
      UTIL_ServerPrint ("No Botskill.cfg ! Using defaults...\n");
   else
   {
      i = 0;

      while (fgets (line_buffer, iBufferSize, fp) != NULL)
      {
         if ((line_buffer[0] == '#') || (line_buffer[0] == 0) || (line_buffer[0] == '\r') || (line_buffer[0] == '\n'))
            continue; // ignore comments or blank lines

         strncpy (arg0, GetField (line_buffer, 0), sizeof (arg0));
         strncpy (arg1, GetField (line_buffer, 1), sizeof (arg1));

         if (FStrEq (arg0, "MIN_DELAY"))
            BotSkillDelays[i].fMinSurpriseDelay = (float) atof (arg1);
         else if (FStrEq (arg0, "MAX_DELAY"))
            BotSkillDelays[i].fMaxSurpriseDelay = (float) atof (arg1);
         else if (FStrEq (arg0, "MIN_TURNSPEED"))
            BotTurnSpeeds[i].fMinTurnSpeed = (float) atof (arg1);
         else if (FStrEq (arg0, "MAX_TURNSPEED"))
            BotTurnSpeeds[i].fMaxTurnSpeed = (float) atof (arg1);
         else if (FStrEq (arg0, "AIM_OFFS_X"))
            BotAimTab[i].fAim_X = (float) atof (arg1);
         else if (FStrEq (arg0, "AIM_OFFS_Y"))
            BotAimTab[i].fAim_Y = (float) atof (arg1);
         else if (FStrEq (arg0, "AIM_OFFS_Z"))
            BotAimTab[i].fAim_Z = (float) atof (arg1);
         else if (FStrEq (arg0, "HEADSHOT_ALLOW"))
            BotAimTab[i].iHeadShot_Frequency = atoi (arg1);
         else if (FStrEq (arg0, "HEAR_SHOOTTHRU"))
            BotAimTab[i].iHeardShootThruProb = atoi (arg1);
         else if (FStrEq (arg0, "SEEN_SHOOTTHRU"))
         {
            BotAimTab[i].iSeenShootThruProb = atoi (arg1);

            if (i < 5)
               i++; // Prevent Overflow if Errors in cfg
         }
      }

      fclose (fp);
   }
   // End Botskill.cfg

   // Load & Initialise BotLogos from BotLogos.cfg
   snprintf (szDirectory, sizeof (szDirectory), "%s/addons/podbot/botlogos.cfg", g_szGameDirectory);
   fp = fopen (szDirectory, "r");
   if (fp == NULL)
      UTIL_ServerPrint ("No BotLogos.cfg ! Using Defaults...\n");
   else
   {
      g_iNumLogos = 0;

      while (fgets (line_buffer, iBufferSize, fp) != NULL)
      {
         if ((line_buffer[0] == '#') || (line_buffer[0] == 0) || (line_buffer[0] == '\r') || (line_buffer[0] == '\n'))
            continue; // ignore comments or blank lines

         strncpy (szSprayNames[g_iNumLogos], GetField (line_buffer, 0), sizeof (szSprayNames[g_iNumLogos]));
         g_iNumLogos++;
      }

      fclose (fp);
   }
   // End BotLogos

   // Load & initialise Weapon Stuff from 'BotWeapons.cfg'
   snprintf (szDirectory, sizeof (szDirectory), "%s/addons/podbot/botweapons.cfg", g_szGameDirectory);
   fp = fopen (szDirectory, "r");
   if (fp == NULL)
      UTIL_ServerPrint ("No BotWeapons.cfg ! Using Defaults...\n");
   else
   {
      iParseWeapons = 0;
      ptrWeaponPrefs = NULL;

      while (fgets (line_buffer, iBufferSize, fp) != NULL)
      {
         if ((line_buffer[0] == '#') || (line_buffer[0] == 0) || (line_buffer[0] == '\r') || (line_buffer[0] == '\n'))
            continue; // ignore comments or blank lines

         strncpy (arg0, GetField (line_buffer, 0), sizeof (arg0));

         if (iParseWeapons < 2)
         {
            if (FStrEq (arg0, "[STANDARD]"))
               iWeaponPrefsType = MAP_DE;
            else if (FStrEq (arg0, "[AS]"))
               iWeaponPrefsType = MAP_AS;
            else
            {
               pszStart = &line_buffer[0];
               pszEnd = NULL;

               if (iWeaponPrefsType == MAP_DE)
               {
                  for (i = 0; i < NUM_WEAPONS; i++)
                  {
                     pszEnd = strchr (pszStart, ',');
                     cs_weapon_select[i].iTeamStandard = atoi (pszStart);
                     pszStart = pszEnd + 1;
                  }
               }
               else
               {
                  for (i = 0; i < NUM_WEAPONS; i++)
                  {
                     pszEnd = strchr (pszStart, ',');
                     cs_weapon_select[i].iTeamAS = atoi (pszStart);
                     pszStart = pszEnd + 1;
                  }
               }

               iParseWeapons++;
            }
         }
         else
         {
            if (FStrEq (arg0, "[NORMAL]"))
               ptrWeaponPrefs = &NormalWeaponPrefs[0];
            else if (FStrEq (arg0, "[AGRESSIVE]"))
               ptrWeaponPrefs = &AgressiveWeaponPrefs[0];
            else if (FStrEq (arg0, "[DEFENSIVE]"))
               ptrWeaponPrefs = &DefensiveWeaponPrefs[0];
            else
            {
               pszStart = &line_buffer[0];

               for (i = 0; i < NUM_WEAPONS; i++)
               {
                  pszEnd = strchr (pszStart, ',');
                  *ptrWeaponPrefs++ = atoi (pszStart);
                  pszStart = pszEnd + 1;
               }
            }
         }
      }

      fclose (fp);
   }

// we should load first the settings from podbot.cfg, then in StartFrame execute commands
   if (g_bBotSettings) // KWo - 17.05.2008
   {
      UTIL_ServerPrint ("Taking settings from podbot.cfg\n");

      snprintf (szDirectory, sizeof (szDirectory), "%s/addons/podbot/podbot.cfg", g_szGameDirectory);
      bot_cfg_fp = fopen (szDirectory, "r");
      if (bot_cfg_fp == NULL)
         UTIL_ServerPrint ("podbot.cfg File not found\n");

      // if the bot.cfg file is still open and time to execute command...
      while ((bot_cfg_fp != NULL) && !feof (bot_cfg_fp))
      {
         if (fgets (line_buffer, iBufferSize, bot_cfg_fp) != NULL)
         {
            if ((line_buffer[0] == '#') || (line_buffer[0] == '\r') || (line_buffer[0] == '\n') || (line_buffer[0] == 0))
               continue; // ignore comments or blank lines

            snprintf (cmd, sizeof (cmd), GetField (line_buffer, 0));
            snprintf (arg1, sizeof (arg1), GetField (line_buffer, 1));

            if ((FStrEq (cmd, "bind"))
               || (FStrEq (cmd, g_rgpszPbCmds[PBCMD])
                  && (FStrEq (arg1, g_rgpszPbCmds[PBCMD_ADD])
                  || FStrEq (arg1, g_rgpszPbCmds[PBCMD_DEBUGGOAL])
                  || FStrEq (arg1, g_rgpszPbCmds[PBCMD_EXPERIENCE])
                  || FStrEq (arg1, g_rgpszPbCmds[PBCMD_FILLSERVER])
                  || FStrEq (arg1, g_rgpszPbCmds[PBCMD_HELP])
                  || FStrEq (arg1, g_rgpszPbCmds[PBCMD_KILLBOTS])
                  || FStrEq (arg1, g_rgpszPbCmds[PBCMD_MENU])
                  || FStrEq (arg1, g_rgpszPbCmds[PBCMD_REMOVE])
                  || FStrEq (arg1, g_rgpszPbCmds[PBCMD_REMOVEBOTS])
                  || FStrEq (arg1, g_rgpszPbCmds[PBCMD_SET])
                  || FStrEq (arg1, g_rgpszPbCmds[PBCMD_WEAPONMODE])
                  || FStrEq (arg1, g_rgpszPbCmds[PBCMD_WP])
                  || FStrEq (arg1, g_rgpszPbCmds[PBCMD_WPMENU]))))
            {
               continue; // ignore commands here...
            }
            else
            {
               UTIL_ServerPrint ("Executing server command: %s", line_buffer);
               SERVER_COMMAND (line_buffer);
               if (FStrEq (cmd, g_rgpszPbCvars[PBCVAR_MAPSTARTBOTJOINDELAY]) && (arg1 != NULL) && (*arg1 != 0))
               {
                  g_f_cv_MapStartBotJoinDelay = atof(arg1);
                  if (g_f_cv_MapStartBotJoinDelay < 0.0)
                     g_f_cv_MapStartBotJoinDelay = 0.0;
                  else if (g_f_cv_MapStartBotJoinDelay > 3600.0)
                     g_f_cv_MapStartBotJoinDelay = 3600.0;
                  CVAR_SET_FLOAT(g_rgpszPbCvars[PBCVAR_MAPSTARTBOTJOINDELAY], g_f_cv_MapStartBotJoinDelay);
               }
               else if (FStrEq (cmd, g_rgpszPbCvars[PBCVAR_WPTFOLDER]) && (arg1 != NULL) && (*arg1 != 0))
               {
                  CVAR_SET_STRING(g_rgpszPbCvars[PBCVAR_WPTFOLDER], arg1);
               }
            }
         }
      }

      // if bot.cfg file is open and reached end of file, then close and free it
      if ((bot_cfg_fp != NULL) && feof (bot_cfg_fp))
      {
         fclose (bot_cfg_fp);
         // print the settings message to console
         UTIL_ServerPrint (g_szSettingsMessage);
      }
   }

   // Load the Waypoints for this Map
   g_sz_cv_WPT_Folder = CVAR_GET_STRING (g_rgpszPbCvars[PBCVAR_WPTFOLDER]);

   WaypointLoad ();
   InitVisTab ();
   InitExperienceTab ();

   // Initialise the Client Struct for welcoming and keeping track who's connected
   memset (clients, 0, sizeof (clients));

   // Initialize the bots array of structures
   memset (bots, 0, sizeof (bots));

   // Initialize TheFatal's method for calculating the msec value
   msecnum = 0;
   msecdel = 0;
   msecval = 0;

   // a new map has started...
   g_f_cv_MapStartBotJoinDelay = CVAR_GET_FLOAT(g_rgpszPbCvars[PBCVAR_MAPSTARTBOTJOINDELAY]);
   if (g_f_cv_MapStartBotJoinDelay < 0.0)
      g_f_cv_MapStartBotJoinDelay = 0.0;
   else if (g_f_cv_MapStartBotJoinDelay > 3600.0)
      g_f_cv_MapStartBotJoinDelay = 3600.0;

   botcreation_time = gpGlobals->time + g_f_cv_MapStartBotJoinDelay; // KWo - 17.05.2008

   // set the respawn time
   g_fTimeRoundStart = gpGlobals->time + CVAR_GET_FLOAT ("mp_freezetime");

   // clear the frame counter
   g_iFrameCounter = 0;
   g_fLastChatTime = 0.0;                    // KWo - 30.07.2006
   g_f_cvars_upd_time = 0.0;                 // KWo - 02.05.2006
   g_f_host_upd_time = 0.0;                  // KWo - 16.06.2006
   g_fTimeAvoidGrenade = 0.0;                // KWo - 29.01.2008
   g_fAutoKillTime = 0.0;                    // KWo - 02.05.2006
   g_bAliveHumansPrevious = false;           // KWo - 02.05.2006
   g_iUpdGlExpState = 0;                     // KWo - 02.05.2006
   g_bBombPlanted = false;                   // KWo - 13.07.2007
   g_bBombDefusing = false;                  // KWo - 13.07.2007
   g_iDefuser = -1;                          // KWo - 13.07.2007
   g_fTimeBombPlanted = 0.0;                 // KWo - 12.07.2007
   g_iCachedWaypoint = -1;                   // KWo - 11.02.2008
   g_bRoundEnded = FALSE;                    // KWo - 30.09.2010

   // Assume Map type for aim or awp type
   strcpy(filename, STRING(gpGlobals->mapname)); // KWo - 18.03.2006 - Thanks to THE_STORM
   if (filename[0] == 'a')
   {
      if (filename[1] == 'w')
         g_iMapType |= MAP_AWP;
      else if (filename[1] == 'i')
         g_iMapType |= MAP_AIM;
   }
   else if (filename[0] == 'e')
      g_iMapType |= MAP_ES;
   else if (filename[0] == 'f')
      g_iMapType |= MAP_FY;
   else if (filename[0] == 't')
      g_iMapType |= MAP_DE;      //Training-Maps
   else if ((filename[0] == 's') && (filename[1] == 'c')) // scouts
      g_iMapType |= MAP_AWP;

   for (i=0; i<32; i++) // KWo - 30.08.2006 - added to check some stacks (new/delete)
   {
      bots[i].i_TaskDeep = 0;
      bots[i].i_PathDeep = 0;
      bots[i].i_ChatDeep = 0;
      bots[i].i_msecval = 0;  // KWo - 17.03.2007
      bots[i].f_msecvalrest = 0.0;  // KWo - 17.03.2007
   }

   RETURN_META (MRES_IGNORED);
}


void ServerDeactivate (void)
{
   int index;
   int tab_index;

   tab_index = 0;
   for (index = 0; index < gpGlobals->maxClients; index++)
   {
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
   }

   // Save collected Experience on Shutdown
   SaveExperienceTab ();
   SaveVisTab ();
   g_iUpdGlExpState = 0; // KWo - 02.05.2006

   // Free everything that's freeable
   BotFreeAllMemory ();

   // clear breakables table
   g_iNumBreakables = 0;  // KWo - 04.03.2006
   for (index=0; index < MAX_BUTTONS; index++)
   {
      BreakablesData[index].EntIndex = -1;
      BreakablesData[index].classname[0] = '\0';
      BreakablesData[index].origin = g_vecZero;
      BreakablesData[index].target[0 ]= '\0';
      BreakablesData[index].ignored = false;
   }

   g_iNumHostages = 0;  // KWo - 16.05.2006
   for (index=0; index < MAX_HOSTAGES; index++) // KWo - 16.05.2006
   {
      HostagesData[index].EntIndex = 0;
      HostagesData[index].OldOrigin = g_vecZero;
      HostagesData[index].Alive = false;
      HostagesData[index].IsMoving = false;
      HostagesData[index].UserEntIndex = 0;
   }
   g_i_MapBuying = 0;   // KWo - 07.06.2010
   g_bWeaponStrip = FALSE; // KWo - 10.03.2013

   RETURN_META (MRES_IGNORED);
}


// Called each Server frame at the very beginning
void StartFrame (void)
{
   static edict_t *pPlayer;
   static int i, player_index, bot_index;
   static char cmd_line[80];
   static char szDirectory[256];
   FILE *bot_cfg_fp = NULL;
   static char cmd1[128];
   static char cmd2[128];
   static char arg1[128];
   static char arg2[128];
   static char arg3[128];
   static char arg4[128];
   static char arg5[128];
   static bool bAliveHumans; // KWo - 02.05.2006

   CallbackStartFrame (); // KWo - 15.03.2012 - thanks to Immortal_BLG

   g_bPathWasCalculated = FALSE; // KWo 24.03.2007

   g_iMax_bots = g_i_cv_MaxBots;	// KWo - 06.04.2006
   g_iMin_bots = g_i_cv_MinBots;	// KWo - 06.04.2006
   g_iMax_bots = (g_iMax_bots > gpGlobals->maxClients) ? gpGlobals->maxClients : (g_iMax_bots < 0) ? 0 : g_iMax_bots;
   g_iMin_bots = (g_iMin_bots > gpGlobals->maxClients) ? gpGlobals->maxClients : (g_iMin_bots < 0) ? 0 : g_iMin_bots;
   if (g_iMax_bots < g_iMin_bots)
      g_iMin_bots = g_iMax_bots;

   // Should the Map restart now ?
   if ((g_fTimeRestartServer > 0) && (g_fTimeRestartServer < gpGlobals->time))
   {
      g_fTimeRestartServer = 0; // don't keep restarting over and over again
      SERVER_COMMAND ("restart\n"); // restart the map
   }

   if (pHostEdict)      // KWo - 20.04.2013
   {
      g_vecHostOrigin = pHostEdict->v.origin;
   }
   else
      g_vecHostOrigin = g_vecZero;

   g_b_DebugWpEdit = ((pHostEdict) && (g_i_cv_debuglevel & DEBUG_FL_WPEDIT)); // KWo - 06.05.2013

   // Record some Stats of all Players on the Server
   g_iNum_players = 0;
   g_iNum_humans = 0;
   g_iNum_hum_tm = 0;   // KWo - 08.03.2010
   bAliveHumans = false;
   g_iAliveCTs = 0;  // KWo - 19.01.2008
   g_iAliveTs = 0;   // KWo - 19.01.2008


   for (player_index = 0; player_index < gpGlobals->maxClients; player_index++)
   {
      pPlayer = INDEXENT (player_index + 1);

      if (!FNullEnt (pPlayer) && (pPlayer->v.flags & FL_CLIENT))
      {
         g_iNum_players++;
         clients[player_index].pEdict = pPlayer;
         clients[player_index].iFlags |= CLIENT_USED;
         IsAlive (pPlayer) ? clients[player_index].iFlags |= CLIENT_ALIVE : clients[player_index].iFlags &= ~CLIENT_ALIVE;

         if ((pPlayer->v.flags & FL_FAKECLIENT) && (clients[player_index].iFlags & CLIENT_ALIVE)) // KWo - 23.03.2012 - thanks to Immortal_BLG
            pPlayer->v.light_level = Light::R_LightPoint (pPlayer->v.origin);

         if ((clients[player_index].iTeam == TEAM_CS_TERRORIST) && (clients[player_index].iFlags & CLIENT_ALIVE)) // KWo - 19.01.2008
            g_iAliveTs++;
         if ((clients[player_index].iTeam == TEAM_CS_COUNTER) && (clients[player_index].iFlags & CLIENT_ALIVE))    // KWo - 19.01.2008
            g_iAliveCTs++;

         if (!(pPlayer->v.flags & FL_FAKECLIENT) && !(bots[player_index].is_used)
            && ((!g_i_cv_BotsQuotaMatch) || (clients[player_index].iTeam == TEAM_CS_TERRORIST)
                 || (clients[player_index].iTeam == TEAM_CS_COUNTER))) // KWo - 16.10.2006
            g_iNum_humans++;

         if (!(pPlayer->v.flags & FL_FAKECLIENT) && !(bots[player_index].is_used)
            && ((clients[player_index].iTeam == TEAM_CS_TERRORIST)
                 || (clients[player_index].iTeam == TEAM_CS_COUNTER))) // KWo - 08.03.2010
            g_iNum_hum_tm++;

         if (!(pPlayer->v.flags & FL_FAKECLIENT) && !(bots[player_index].is_used) && (clients[player_index].iFlags & CLIENT_ALIVE) && g_b_cv_autokill) // KWo - 02.05.2006
         {
            bAliveHumans = true;
//            if (g_iFrameCounter == 10)
//               UTIL_ServerPrint("[DEBUG] Player %s is alive.\n", STRING(pPlayer->v.netname));
         }

         if (clients[player_index].iFlags & CLIENT_ALIVE)
         {
            // Keep noclip on or else it might turn off after new round
            if ((pHostEdict == pPlayer) && g_bEditNoclip)
               pHostEdict->v.movetype = MOVETYPE_NOCLIP;

            // Replaced with Client_CS_ScoreInfo
//            clients[player_index].iTeam = UTIL_GetTeam (pPlayer);
            clients[player_index].vOrigin = pPlayer->v.origin;
            SoundSimulateUpdate (player_index);
            if ((pPlayer->v.button & IN_RELOAD) && (clients[player_index].fReloadingTime < gpGlobals->time)
               && (clients[player_index].iCurrentClip < weapon_maxClip[clients[player_index].iCurrentWeaponId])) // KWo - 15.08.2007
            {
               clients[player_index].fReloadingTime = gpGlobals->time + 1.5;
//               ALERT(at_logged, "[DEBUG] SF - Player %s is reloading his weapon...\n", STRING(pPlayer->v.netname));
            }
         }

         // Does Client need to be shocked by the ugly red welcome message ?
         if (clients[player_index].welcome_time == -2.0) // KWo - 19.04.2010
            clients[player_index].welcome_time = gpGlobals->time + 15.0;
         else if ((clients[player_index].welcome_time > 0)
            && (clients[player_index].welcome_time < gpGlobals->time))
         {
            // Real Clients only
            if (!(pPlayer->v.flags & FL_FAKECLIENT))
            {
               // Hacked together Version of HUD_DrawString
               MESSAGE_BEGIN (MSG_ONE_UNRELIABLE, SVC_TEMPENTITY, NULL, pPlayer);
               WRITE_BYTE (TE_TEXTMESSAGE);
               WRITE_BYTE (1); // channel
               WRITE_SHORT (FixedSigned16 (-1, 1 << 13)); // x coordinates * 8192
               WRITE_SHORT (FixedSigned16 (0, 1 << 13)); // y coordinates * 8192
               WRITE_BYTE (2); // effect (fade in/out)
               WRITE_BYTE (255); // initial RED
               WRITE_BYTE (0); // initial GREEN
               WRITE_BYTE (0); // initial BLUE
               WRITE_BYTE (0); // initial effect ALPHA
               WRITE_BYTE (255); // effect RED
               WRITE_BYTE (255); // effect GREEN
               WRITE_BYTE (255); // effect BLUE
               WRITE_BYTE (200); // effect ALPHA
               WRITE_SHORT (FixedUnsigned16 (0.0078125, 1 << 8)); // fade-in time in seconds * 256
               WRITE_SHORT (FixedUnsigned16 (2, 1 << 8)); // fade-out time in seconds * 256
               WRITE_SHORT (FixedUnsigned16 (6, 1 << 8)); // hold time in seconds * 256
               WRITE_SHORT (FixedUnsigned16 (0.1, 1 << 8)); // effect time * 256
               WRITE_STRING (g_szWelcomeMessage); // write message
               MESSAGE_END ();

               // If this is the Host, scare him even more with a spoken Message
               if ((g_b_cv_UseSpeech) && (pPlayer == pHostEdict))
                  SERVER_COMMAND ((char *) &szSpeechSentences[RANDOM_LONG (0, 15)]);
            }

            clients[player_index].welcome_time = 0.0;
         }

         // Does Client need to be shocked by the ugly yellow waypoint message ?
         if (clients[player_index].wptmessage_time == -2.0) // KWo - 19.04.2010
            clients[player_index].wptmessage_time = gpGlobals->time + 12.0;
         else if ((clients[player_index].wptmessage_time > 0)
            && (clients[player_index].wptmessage_time < gpGlobals->time)
            && !(pPlayer->v.flags & FL_SPECTATOR))
         {
            // Real Clients only
            if (!(pPlayer->v.flags & FL_FAKECLIENT))
            {
               if (GET_USER_MSG_ID (PLID, "TextMsg", NULL) == 0)
                  REG_USER_MSG ("TextMsg", -1);

               MESSAGE_BEGIN (MSG_ONE_UNRELIABLE, GET_USER_MSG_ID (PLID, "TextMsg", NULL), NULL, pPlayer);
               WRITE_BYTE (HUD_PRINTCENTER);
               WRITE_STRING (g_szWaypointMessage);
               MESSAGE_END ();
            }

            clients[player_index].wptmessage_time = 0.0;
         }
      }
      else
      {
         clients[player_index].pEdict = NULL;
         clients[player_index].iFlags &= ~(CLIENT_USED | CLIENT_ALIVE);
      }
   }
   g_bDeathMessageSent = FALSE; // KWo - 02.03.2010
/*
   KWo - 27.04.2006 - Removed due to the new msec code taken from THE_STORM
   // Estimate next Frame's duration
   EstimateNextFrameDuration ();
*/

   // Executing the .cfg file - most work already done by Botman
   // Command Names are the same as in ClientCommand
   if (g_bBotSettings) // KWo - 17.05.2008 - moved here and moved settings loading to ServerActivate...
   {
      UTIL_ServerPrint ("Executing commands from podbot.cfg\n");
      memset(szDirectory, 0, sizeof(szDirectory));
      memset(cmd1, 0, sizeof (cmd1));
      memset(cmd2, 0, sizeof (cmd2));
      memset(arg1, 0, sizeof (arg1));
      memset(arg2, 0, sizeof (arg2));
      memset(arg3, 0, sizeof (arg3));
      memset(arg4, 0, sizeof (arg4));
      memset(arg5, 0, sizeof (arg5));
      snprintf (szDirectory, sizeof (szDirectory), "%s/addons/podbot/podbot.cfg", g_szGameDirectory);
      bot_cfg_fp = fopen (szDirectory, "r");
      if (bot_cfg_fp == NULL)
         UTIL_ServerPrint ("podbot.cfg File not found\n");

      // if the bot.cfg file is still open and time to execute command...
      while ((bot_cfg_fp != NULL) && !feof (bot_cfg_fp))
      {
         if (fgets (cmd_line, sizeof (cmd_line), bot_cfg_fp) != NULL)
         {
            if ((cmd_line[0] == '#') || (cmd_line[0] == '\r') || (cmd_line[0] == '\n') || (cmd_line[0] == 0))
               continue; // ignore comments or blank lines

            snprintf (cmd1, sizeof (cmd1), GetField (cmd_line, 0));
            snprintf (cmd2, sizeof (cmd2), GetField (cmd_line, 1));
            snprintf (arg1, sizeof (arg1), GetField (cmd_line, 2));
            snprintf (arg2, sizeof (arg2), GetField (cmd_line, 3));
            snprintf (arg3, sizeof (arg3), GetField (cmd_line, 4));
            snprintf (arg4, sizeof (arg4), GetField (cmd_line, 5));
            snprintf (arg5, sizeof (arg5), GetField (cmd_line, 6));

            if ((FStrEq (cmd1, g_rgpszPbCmds[PBCMD])
                     && (FStrEq (cmd2, g_rgpszPbCmds[PBCMD_ADD])
                     || FStrEq (cmd2, g_rgpszPbCmds[PBCMD_DEBUGGOAL])
                     || FStrEq (cmd2, g_rgpszPbCmds[PBCMD_EXPERIENCE])
                     || FStrEq (cmd2, g_rgpszPbCmds[PBCMD_FILLSERVER])
                     || FStrEq (cmd2, g_rgpszPbCmds[PBCMD_HELP])
                     || FStrEq (cmd2, g_rgpszPbCmds[PBCMD_KILLBOTS])
                     || FStrEq (cmd2, g_rgpszPbCmds[PBCMD_MENU])
                     || FStrEq (cmd2, g_rgpszPbCmds[PBCMD_REMOVE])
                     || FStrEq (cmd2, g_rgpszPbCmds[PBCMD_REMOVEBOTS])
                     || FStrEq (cmd2, g_rgpszPbCmds[PBCMD_SET])
                     || FStrEq (cmd2, g_rgpszPbCmds[PBCMD_WEAPONMODE])
                     || FStrEq (cmd2, g_rgpszPbCmds[PBCMD_WP])
                     || FStrEq (cmd2, g_rgpszPbCmds[PBCMD_WPMENU])))) // KWo - 17.05.2008
            {
               UTIL_ServerPrint ("Executing server command: %s", cmd_line);
               PbCmdParser (NULL, cmd2, arg1, arg2, arg3, arg4, arg5);
            }
            else if (FStrEq (cmd1, "bind"))
            {
               UTIL_ServerPrint ("Executing server command: %s", cmd_line);
               SERVER_COMMAND (cmd_line);
            }
         }
      }

      // if bot.cfg file is open and reached end of file, then close and free it
      if ((bot_cfg_fp != NULL) && feof (bot_cfg_fp))
      {
         g_bBotSettings = FALSE;
         fclose (bot_cfg_fp);
         bot_cfg_fp = NULL;

         // print the commands message to console
         UTIL_ServerPrint (g_szCommandsMessage); // KWo - 17.05.2008
         // print the waypoint message to console
         UTIL_ServerPrint (g_szWaypointMessage);
      }
   }

   g_iFrameCounter++;   // KWo - 10.03.2006
   if (g_iFrameCounter >= gpGlobals->maxClients) // KWo - 23.03.2007
   {
      g_iFrameCounter = 0;
   }

   if (g_f_cvars_upd_time <= gpGlobals->time) // KWo - 02.05.2006
   {
      UTIL_CheckCvars();  // check restrictions and other cvars every 1 second - don't need more often
      g_f_cvars_upd_time = gpGlobals->time + 1.0;
   }

   if ((g_f_host_upd_time <= gpGlobals->time) &&  (g_iNumHostages > 0)) // KWo - 18.05.2006
   {
      UTIL_CheckHostages();  // check hostages - if they moved or got killed etc
      g_f_host_upd_time = gpGlobals->time + 1.0;
   }


   // Updating global experience executed in few steps (initiated at Round Start - KWo - 02.05.2006
   if (g_iUpdGlExpState > 0)
      UpdateGlobalExperienceData ();

   // Check HE and smoke grenades
   if (g_fTimeAvoidGrenade < gpGlobals->time) // KWo - 29.01.2008
   {
      g_fTimeAvoidGrenade = gpGlobals->time + 0.1;
      UTIL_CheckSmokeGrenades();
   }

   if ((g_iMapType & MAP_DE) && g_bBombPlanted
      && (g_fTimeBombPlanted + 1.0 < gpGlobals->time) && (g_fTimeBombPlanted + 1.2 > gpGlobals->time)) // KWo - 05.09.2008
      g_vecBomb = GetBombPosition();

   // Go through all active Bots, calling their Think function
   g_iNum_bots = 0;

   for (bot_index = 0; bot_index < gpGlobals->maxClients; bot_index++)
   {
      if (bots[bot_index].is_used
          && !FNullEnt (bots[bot_index].pEdict))
      {
         g_i_botthink_index = bot_index; // KWo - 02.05.2006
         BotThink (&bots[bot_index]);
         g_iNum_bots++;
      }
   }

   // Show Waypoints to host if turned on and no dedicated Server
   if (!g_bIsDedicatedServer && g_bWaypointOn && !FNullEnt (pHostEdict))
      WaypointThink ();

   if (g_bMapInitialised && g_bRecalcVis)
      WaypointCalcVisibility ();

   // Keep it within the limits
   for (i = 0; i < gpGlobals->maxClients; i++)
      if (BotCreateTab[i].bNeedsCreation)
         break;
   if ((i >= gpGlobals->maxClients) && (botcreation_time == 0.0))
   {
      if (g_i_cv_BotsQuotaMatch == 0) // 16.09.2006
      {
         if ((g_iPeoBotsKept > g_iMax_bots) && (g_iMax_bots > 0))
            g_iPeoBotsKept = g_iMax_bots;
         if (g_iPeoBotsKept < g_iMin_bots)
            g_iPeoBotsKept = g_iMin_bots;
         if (g_iMax_bots == 0)
            g_iPeoBotsKept = g_iNum_players;
      }
      else
      {
         g_iPeoBotsKept = g_i_cv_BotsQuotaMatch * g_iNum_humans;
         if (g_iPeoBotsKept > g_iMax_bots)
            g_iPeoBotsKept = g_iMax_bots;
         if (g_iPeoBotsKept < g_iMin_bots)
            g_iPeoBotsKept = g_iMin_bots;
         if (g_iPeoBotsKept + g_iNum_humans > gpGlobals->maxClients)
            g_iPeoBotsKept = gpGlobals->maxClients - g_iNum_humans;
      }
   }

   // Kick a bot if there is more than the maximum allowed
   if ((((g_i_cv_BotsQuotaMatch == 0) && (g_iNum_players > g_iPeoBotsKept))  // 16.09.2006
         || ((g_i_cv_BotsQuotaMatch > 0) && (g_iNum_bots > g_iPeoBotsKept)))
      && (g_iMax_bots > 0) && (g_iNum_bots > g_iMin_bots)
      && (g_fLastKickedBotTime + 0.5 < gpGlobals->time))
      for (i = 0; i < gpGlobals->maxClients; i++)
         if (bots[i].is_used && !FNullEnt (bots[i].pEdict))
         {
            snprintf (cmd_line, sizeof (cmd_line), "kick \"%s\"\n", STRING (bots[i].pEdict->v.netname));
            SERVER_COMMAND (cmd_line);
            break;
         }

   // make sure it is allowed to create bots
   if (botcreation_time > 0.0)
   {
      // Don't allow creating Bots when max_bots is reached
      if ((((g_i_cv_BotsQuotaMatch == 0) && (g_iNum_players >= g_iMax_bots)) // KWo - 08.03.2007
           || ((g_i_cv_BotsQuotaMatch > 0) && ((g_iNum_bots >= g_iMax_bots) || (g_iNum_players == gpGlobals->maxClients))))
          && (g_iMax_bots > 0) && (g_iNum_bots >= g_iMin_bots))
      {
         UTIL_ServerPrint ("Max Bots reached, can't create Bot !\n");
         memset (BotCreateTab, 0, sizeof (BotCreateTab));
         botcreation_time = 0.0;
      }

      // Don't allow creating Bots when no waypoints are loaded
      else if (g_iNumWaypoints < 1)
      {
         UTIL_ServerPrint ("No Waypoints for this Map, can't create Bot !\n");
         memset (BotCreateTab, 0, sizeof (BotCreateTab));
         botcreation_time = 0.0;
      }

      // if Waypoints have changed don't allow it because Distance Tables are messed up
      else if (g_bWaypointsChanged)
      {
         UTIL_ServerPrint ("Waypoints changed/not initialised, can't create Bot !\n");
         memset (BotCreateTab, 0, sizeof (BotCreateTab));
         botcreation_time = 0.0;
      }
   }
   else if ((gpGlobals->maxClients > g_iNum_players) && ((g_iNum_bots < g_iMin_bots)
           || ((g_iNum_players < g_iPeoBotsKept) && (g_i_cv_BotsQuotaMatch == 0))
              || ((g_iNum_bots < g_iPeoBotsKept) && (g_i_cv_BotsQuotaMatch > 0)))
              && (g_iNumWaypoints > 0) && !(g_bWaypointsChanged) && (iNumBotNames > g_iNum_bots)
              && (g_iMax_bots > 0)) // KWo - 16.09.2006
   {
      for (i = 0; i < gpGlobals->maxClients; i++)
         if (!BotCreateTab[i].bNeedsCreation)
            break;
      if (i < gpGlobals->maxClients)
      {
         memset (&BotCreateTab[i], 0, sizeof (createbot_t));
         BotCreateTab[i].bot_skill = 101;
         BotCreateTab[i].bot_personality = 5;
         BotCreateTab[i].bot_team = 5;
         BotCreateTab[i].bot_class = 5;
         BotCreateTab[i].bNeedsCreation = TRUE;
         if (botcreation_time == 0.0)
            botcreation_time = gpGlobals->time;
      }
   }

   // are we currently spawning bots and is it time to spawn one yet?
   if ((botcreation_time > 0.0) && (botcreation_time < gpGlobals->time))
   {
      // find bot needing to be spawned...
      for (bot_index = 0; bot_index < gpGlobals->maxClients; bot_index++)
         if (BotCreateTab[bot_index].bNeedsCreation)
            break;

      if (bot_index < gpGlobals->maxClients)
      {
         botcreation_time = gpGlobals->time + 0.5; // set next spawn time
         BotCreate (BotCreateTab[bot_index].bot_skill, BotCreateTab[bot_index].bot_personality,
            BotCreateTab[bot_index].bot_team, BotCreateTab[bot_index].bot_class, BotCreateTab[bot_index].bot_name);
         memset (&BotCreateTab[bot_index], 0, sizeof (createbot_t));
      }
      else
         botcreation_time = 0.0;
   }

// autokill function here - KWo 02.05.2006

   if (!bAliveHumans && g_bAliveHumansPrevious && g_b_cv_autokill && (g_fAutoKillTime == 0.0))   //
   {
      g_fAutoKillTime = gpGlobals->time + g_f_cv_autokilldelay;
//      UTIL_ServerPrint("[DEBUG] Autokill time started .\n");
   }
   g_bAliveHumansPrevious = bAliveHumans;

   if (g_b_cv_autokill && (g_fAutoKillTime < gpGlobals->time) && (g_fAutoKillTime > 0.0) && !g_bBombPlanted) // KWo - 25.07.2006
   {
      for (i = 0; i < gpGlobals->maxClients; i++)
      {
         if (bots[i].is_used && !bots[i].bDead)
         {
            // If a Bot gets killed it decreases his frags, so add 1 here to not give human
            // players an advantage using this command
            bots[i].pEdict->v.frags++;
            MDLL_ClientKill (bots[i].pEdict);
         }
      }

      g_fAutoKillTime = 0.0;
      UTIL_HostPrint ("All Bots autokilled!\n");
   }

   if ((!g_bResetHud) && (g_iNum_bots > 0) && (g_iNum_hum_tm_prev == 0)
         && (g_iNum_hum_tm == 1) && (g_iAliveCTs > 0) && (g_iAliveTs > 0)
         && g_bIsDedicatedServer && g_b_cv_firsthumanrestart) // KWo - 04.10.2010
      SERVER_COMMAND ("sv_restartround 1\n");

   g_iNum_hum_tm_prev = g_iNum_hum_tm; // KWo - 08.03.2010
   g_bResetHud = FALSE;                // KWo - 04.03.2010

   RETURN_META (MRES_IGNORED);
}

void Pfn_KeyValue (edict_t *pentKeyvalue, KeyValueData *pkvd )  // KWo - 06.03.2006
{
   int Ent_index = ENTINDEX(pentKeyvalue);
   int i;
   if (FStrEq(STRING(pentKeyvalue->v.classname),"func_breakable"))
   {
      if (FStrEq(pkvd->szKeyName, "material") )
      {
         if(atoi( pkvd->szValue ) == 7) // it's unbreakble glass
         {
            for (i=0; i < g_iNumBreakables; i++)
            {
               if (BreakablesData[i].EntIndex == Ent_index)
               {
                  BreakablesData[i].ignored = true;
                  break;
               }
            }
         }
      }
   }
   else if (FStrEq(STRING(pentKeyvalue->v.classname),"info_map_parameters")) // KWo - 07.06.2010
   {
      if (FStrEq(pkvd->szKeyName, "buying") )
      {
         g_i_MapBuying = atoi( pkvd->szValue );
//         ALERT(at_logged, "[DEBUG] Pfn_KeyValue - Info map parameters - buying = %d.\n", g_i_MapBuying);
      }
   }
   RETURN_META (MRES_IGNORED);
}

static void PM_Move (playermove_t *playerMove, qboolean server)
{
   // this is the player movement code clients run to predict things when the server can't update
   // them often enough (or doesn't want to). The server runs exactly the same function for
   // moving players. There is normally no distinction between them, else client-side prediction
   // wouldn't work properly (and it doesn't work that well, already...)

   CallbackPM_Move (playerMove, server == TRUE);

   RETURN_META (MRES_IGNORED);
}

void Pfn_UpdateClientData ( const struct edict_s *ent, int sendweapons, struct clientdata_s *cd ) // KWo 02.03.2010 - thanks to MeRcyLeZZ
{
   static int sending;
   static int bot_index;
   static edict_t *pPlayer;

   if (g_i_cv_latencybot != 2)
      RETURN_META (MRES_IGNORED);

   sending = 0;

	// Scoreboard key being pressed?
   if (!FNullEnt (ent) && (ent->v.flags & FL_CLIENT)
      && ((ent->v.button & IN_SCORE) || (ent->v.oldbuttons & IN_SCORE)))
   {
      pPlayer = INDEXENT(ENTINDEX(ent));

      for (bot_index = 0; bot_index < gpGlobals->maxClients; bot_index++)
      {
         if (bots[bot_index].is_used && !FNullEnt (bots[bot_index].pEdict))
         {
            // Send message with the weird arguments
            switch (sending)
            {
               case 0:
               {
				      // Start a new message
                  MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, SVC_PINGS, NULL, pPlayer);
                  WRITE_BYTE((bots[bot_index].iOffsetPing[0] * 64) + (1 + 2 * (bot_index)));
                  WRITE_SHORT(bots[bot_index].iArgPing[0]);
                  sending++;
			      }
               case 1:
               {
				      // Append additional data
                  WRITE_BYTE((bots[bot_index].iOffsetPing[1] * 128) + (2 + 4 * (bot_index)));
                  WRITE_SHORT(bots[bot_index].iArgPing[1]);
                  sending++;
               }
               case 2:
               {
                  // Append additional data and end message
                  WRITE_BYTE((4 + 8 * (bot_index)));
                  WRITE_SHORT(bots[bot_index].iArgPing[2]);
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
   RETURN_META (MRES_IGNORED);
}

void FakeClientCommand (edict_t *pFakeClient, const char *fmt, ...)
{
   // the purpose of this function is to provide fakeclients (bots) with the same client
   // command-scripting advantages (putting multiple commands in one line between semicolons)
   // as real players. It is an improved version of botman's FakeClientCommand, in which you
   // supply directly the whole string as if you were typing it in the bot's "console". It
   // is supposed to work exactly like the pfnClientCommand (server-sided client command).

   va_list argptr;
   static char command[256];
   int length, fieldstart, fieldstop, i, index, stringindex = 0;
   int iClientIndex;
   int iRadioCommand;

   if (FNullEnt (pFakeClient))
      return; // reliability check

   // concatenate all the arguments in one string
   va_start (argptr, fmt);
   vsnprintf (command, sizeof (command), fmt, argptr);
   va_end (argptr);

   if ((command == NULL) || (*command == 0))
      return; // if nothing in the command buffer, return

   isFakeClientCommand = TRUE; // set the "fakeclient command" flag
   length = (int)strlen (command); // get the total length of the command string

   // process all individual commands (separated by a semicolon) one each a time
   while (stringindex < length)
   {
      fieldstart = stringindex; // save field start position (first character)
      while ((stringindex < length) && (command[stringindex] != ';'))
         stringindex++; // reach end of field
      if (command[stringindex - 1] == '\n')
         fieldstop = stringindex - 2; // discard any trailing '\n' if needed
      else
         fieldstop = stringindex - 1; // save field stop position (last character before semicolon or end)
      for (i = fieldstart; i <= fieldstop; i++)
         g_argv[i - fieldstart] = command[i]; // store the field value in the g_argv global string
      g_argv[i - fieldstart] = 0; // terminate the string
      stringindex++; // move the overall string index one step further to bypass the semicolon

      index = 0;
      fake_arg_count = 0; // let's now parse that command and count the different arguments

      // count the number of arguments
      while (index < i - fieldstart)
      {
         while ((index < i - fieldstart) && (g_argv[index] == ' '))
            index++; // ignore spaces

         // is this field a group of words between quotes or a single word ?
         if (g_argv[index] == '"')
         {
            index++; // move one step further to bypass the quote
            while ((index < i - fieldstart) && (g_argv[index] != '"'))
               index++; // reach end of field
            index++; // move one step further to bypass the quote
         }
         else
            while ((index < i - fieldstart) && (g_argv[index] != ' '))
               index++; // this is a single word, so reach the end of field

         fake_arg_count++; // we have processed one argument more
      }

      // Check Radio Commands (fix): do it here since metamod won't call our own ClientCommand()
      iClientIndex = ENTINDEX (pFakeClient) - 1;

      if ((clients[iClientIndex].iFlags & CLIENT_ALIVE) && (iRadioSelect[iClientIndex] != 0) && (strncmp (g_argv, "menuselect", 10) == 0))
      {
         iRadioCommand = atoi (g_argv + 11);

         if (iRadioCommand != 0)
         {
            iRadioCommand += 10 * (iRadioSelect[iClientIndex] - 1);

            if ((iRadioCommand != RADIO_AFFIRMATIVE)
                && (iRadioCommand != RADIO_NEGATIVE)
                && (iRadioCommand != RADIO_REPORTINGIN))
            {
               for (i = 0; i < gpGlobals->maxClients; i++)
               {
                  if (bots[i].is_used && (bots[i].bot_team == clients[iClientIndex].iTeam)
                      && (pFakeClient != bots[i].pEdict))
                  {
                     if (bots[i].iRadioOrder == 0)
                     {
                        bots[i].iRadioOrder = iRadioCommand;
                        bots[i].pRadioEntity = pFakeClient;
                     }
                  }
               }
            }

            g_rgfLastRadioTime[clients[iClientIndex].iTeam - 1] = gpGlobals->time;
         }

         iRadioSelect[iClientIndex] = 0;
      }
      else if (strncmp (g_argv, "radio", 5) == 0)
         iRadioSelect[iClientIndex] = atoi (g_argv + 5);

      // End Radio Commands

      MDLL_ClientCommand (pFakeClient); // tell now the MOD DLL to execute this ClientCommand...
   }

   g_argv[0] = 0; // when it's done, reset the g_argv field
   isFakeClientCommand = FALSE; // reset the "fakeclient command" flag
   fake_arg_count = 0; // and the argument count
}


const char *GetField (const char *string, int field_number)
{
   // This function gets and returns a particuliar field in a string where several fields are
   // concatenated. Fields can be words, or groups of words between quotes ; separators may be
   // white space or tabs. A purpose of this function is to provide bots with the same Cmd_Argv
   // convenience the engine provides to real clients. This way the handling of real client
   // commands and bot client commands is exactly the same, just have a look in engine.cpp
   // for the hooking of pfnCmd_Argc, pfnCmd_Args and pfnCmd_Argv, which redirects the call
   // either to the actual engine functions (when the caller is a real client), either on
   // our function here, which does the same thing, when the caller is a bot.

   static char field[256];
   int length, i, index = 0, field_count = 0, fieldstart, fieldstop;

   field[0] = 0; // reset field
   length = (int)strlen (string); // get length of string

   while ((length > 0) && ((string[length - 1] == '\n') || (string[length - 1] == '\r')))
      length--; // discard trailing newlines

   // while we have not reached end of line
   while ((index < length) && (field_count <= field_number))
   {
      while ((index < length) && ((string[index] == ' ') || (string[index] == '\t')))
         index++; // ignore spaces or tabs

      // is this field multi-word between quotes or single word ?
      if (string[index] == '"')
      {
         index++; // move one step further to bypass the quote
         fieldstart = index; // save field start position
         while ((index < length) && (string[index] != '"'))
            index++; // reach end of field
         fieldstop = index - 1; // save field stop position
         index++; // move one step further to bypass the quote
      }
      else
      {
         fieldstart = index; // save field start position
         while ((index < length) && ((string[index] != ' ') && (string[index] != '\t')))
            index++; // reach end of field
         fieldstop = index - 1; // save field stop position
      }

      // is this field we just processed the wanted one ?
      if (field_count == field_number)
      {
         for (i = fieldstart; i <= fieldstop; i++)
            field[i - fieldstart] = string[i]; // store the field value in a string
         field[i - fieldstart] = 0; // terminate the string
         break; // and stop parsing
      }

      field_count++; // we have parsed one field more
   }

   return (&field[0]); // returns the wanted field
}


void PODBot_ServerCommand (void)
{
   const char* pcmd = CMD_ARGV (1);	// KWo - 18.01.2006
   const char* arg1 = CMD_ARGV (2);
   const char* arg2 = CMD_ARGV (3);
   const char* arg3 = CMD_ARGV (4);
   const char* arg4 = CMD_ARGV (5);
   const char* arg5 = CMD_ARGV (6);
   PbCmdParser ((g_bIsDedicatedServer) ? NULL : pHostEdict, pcmd, arg1, arg2, arg3, arg4, arg5);
}


void PbCmdParser (edict_t *pEdict, const char *pcmd, const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5)
{
   // This is cleaner than polling a CVAR, eh ? :) -- PM
   char kickcmd[80];
   int index;
   int iSkill, iPersonality, iTeam, iClass, iSelection;
   const char *const szIsNotHostOfListen = "You must be the host of a listen server to use this command\n";
   const char *const szSetPWUseRCON = "You MUST use RCON to change password or passwordkey\n";

   // "add" adds a bot to creation queue with specified args
   if (FStrEq (pcmd, g_rgpszPbCmds[PBCMD_ADD]))
   {
      if ((arg1 != NULL) && (*arg1 != 0))
         iSkill = atoi (arg1);
      else
         iSkill = 101;

      if ((arg2 != NULL) && (*arg2 != 0))
         iPersonality = atoi (arg2) - 1;
      else
         iPersonality = 5;

      if (g_i_cv_BotsJoinTeam == 0)  // 16.09.2006
      {
         if ((arg3 != NULL) && (*arg3 != 0))
            iTeam = atoi (arg3);
         else
            iTeam = 5;
      }
      else
         iTeam = g_i_cv_BotsJoinTeam;

      if (g_i_cv_skin == 5) // 18.11.2006
      {
         if ((arg4 != NULL) && (*arg4 != 0))
            iClass = atoi (arg4);
         else
            iClass = 5;
      }
      else
         iClass = g_i_cv_skin;

      for (index = 0; index < gpGlobals->maxClients; index++)
         if (!BotCreateTab[index].bNeedsCreation)
            break;
      if (index < gpGlobals->maxClients)
      {
         memset (&BotCreateTab[index], 0, sizeof (createbot_t));
         if ((arg5 != NULL) && (*arg4 != 0))
            strncpy (BotCreateTab[index].bot_name, arg5, sizeof (BotCreateTab[index].bot_name));
         BotCreateTab[index].bot_skill = iSkill;
         BotCreateTab[index].bot_personality = iPersonality;
         BotCreateTab[index].bot_team = iTeam;
         BotCreateTab[index].bot_class = iClass;
         BotCreateTab[index].bNeedsCreation = TRUE;
         if (botcreation_time == 0.0)
            botcreation_time = gpGlobals->time;
         if (g_i_cv_BotsQuotaMatch == 0)
         {
            if ( ( ((g_bIsDedicatedServer) && (g_iPeoBotsKept==0)) || ((!g_bIsDedicatedServer) && (g_iPeoBotsKept==1) && (g_iNum_players>0)) )
               && (g_iNum_bots==0) ) // KWo 08.01.2006
               g_iPeoBotsKept = g_iNum_players;
            ++g_iPeoBotsKept;
         }
      }
//      ALERT(at_logged, "[DEBUG] Add command is executed with g_iPeoBotsKept = %d.\n", g_iPeoBotsKept);
   }

   // "debuggoal" Forces Bots to use a specified Waypoint as a Goal
   else if (FStrEq (pcmd, g_rgpszPbCmds[PBCMD_DEBUGGOAL]))
   {
      if ((arg1 != NULL) && (*arg1 != 0))
         g_iDebugGoalIndex = atoi (arg1);

      // Bots ignore Enemies in Goal Debug Mode
      if ((g_iDebugGoalIndex >= 0) && (g_iDebugGoalIndex < g_iNumWaypoints))
      {
         g_bIgnoreEnemies = TRUE;
         bottask_t TempTask = {NULL, NULL, TASK_NORMAL, TASKPRI_NORMAL, g_iDebugGoalIndex, 0.0, TRUE};

         for (index = 0; index < gpGlobals->maxClients; index++)
         {
            if (!bots[index].is_used
                || bots[index].bDead
                || FNullEnt(bots[index].pEdict))
               continue;

            DeleteSearchNodes (&bots[index]);
            BotResetTasks(&bots[index]);
            BotPushTask (&bots[index], &TempTask);
         }
      }
      else
         g_bIgnoreEnemies = FALSE;

      UTIL_HostPrint ("%s is set to %i\n", g_rgpszPbCmds[PBCMD_DEBUGGOAL], g_iDebugGoalIndex); // KWo - 04.10.2006
   }

   // "experience"
   else if (FStrEq (pcmd, g_rgpszPbCmds[PBCMD_EXPERIENCE]))
   {
      if (FStrEq (arg1, "save"))
      {
         SaveExperienceTab ();
         SaveVisTab ();
      }
      if (FStrEq (arg1, "show"))
      {
         int j;
         unsigned short  iExp0,iExp1;
         signed short iInd0, iInd1; // KWo - 06.01.2008
         for (j = 0; j < g_iNumWaypoints; j++)
         {
            iExp0 = (pBotExperienceData + (j * g_iNumWaypoints) + j)->uTeam0Damage;
            iExp1 = (pBotExperienceData + (j * g_iNumWaypoints) + j)->uTeam1Damage;
            iInd0 = (pBotExperienceData + (j * g_iNumWaypoints) + j)->iTeam0_danger_index;  // KWo - 06.01.2008
            iInd1 = (pBotExperienceData + (j * g_iNumWaypoints) + j)->iTeam1_danger_index;  // KWo - 06.01.2008
            UTIL_ServerPrint("Stored experience for T team - WP index %d ; experience = %d ; dangerest WP index = %d\n",j, iExp0, iInd0);   // KWo - 06.01.2008
            UTIL_ServerPrint("Stored experience for CT team - WP index %d ; experience = %d ; dangerest WP index = %d\n",j, iExp1, iInd1);  // KWo - 06.01.2008
         }
         UTIL_ServerPrint("The most danger T value = %d, the most danger CT value = %d\n", g_iHighestDamageT, g_iHighestDamageCT);
      }
      if (FStrEq (arg1, "test_a_T"))
      {
         int iTeam = 1;
         int iWithHostage = 0;
         int iSourceIndex = atoi (arg2);
         int iDestIndex = atoi (arg3);
         unsigned char byPathType = (unsigned char) atoi (arg4);
         int ibyPathType = atoi (arg4);
         if ((iSourceIndex >= 0) && (iSourceIndex < g_iNumWaypoints)
             && (iDestIndex >= 0) && (iDestIndex < g_iNumWaypoints)
             && (ibyPathType >= 0) &&(ibyPathType < 3))
            TestAPath (iTeam, iWithHostage, iSourceIndex, iDestIndex, byPathType);
      }
      if (FStrEq (arg1, "test_a_CT"))
      {
         int iTeam = 2;
         int iWithHostage = 0;
         int iSourceIndex = atoi (arg2);
         int iDestIndex = atoi (arg3);
         unsigned char byPathType = (unsigned char) atoi (arg4);
         int ibyPathType = atoi (arg4);
         if ((iSourceIndex >= 0) && (iSourceIndex < g_iNumWaypoints)
             && (iDestIndex >= 0 ) && (iDestIndex < g_iNumWaypoints)
             && (ibyPathType >= 0) &&(ibyPathType < 3))
            TestAPath (iTeam, iWithHostage, iSourceIndex, iDestIndex, byPathType);
      }
      if (FStrEq (arg1, "test_a_CTH"))
      {
         int iTeam = 2;
         int iWithHostage = 1;
         int iSourceIndex = atoi (arg2);
         int iDestIndex = atoi (arg3);
         unsigned char byPathType = (unsigned char) atoi (arg4);
         int ibyPathType = atoi (arg4);
         if ((iSourceIndex>=0) && (iSourceIndex < g_iNumWaypoints)
             && (iDestIndex >= 0) && (iDestIndex < g_iNumWaypoints)
             && (ibyPathType >= 0) &&(ibyPathType < 3))
            TestAPath (iTeam, iWithHostage, iSourceIndex, iDestIndex, byPathType);
      }

   }

   // "fillserver" Fills the server with bots
   else if (FStrEq (pcmd, g_rgpszPbCmds[PBCMD_FILLSERVER]))
   {
      // fillserver is against BotsQuotaMatch - needs make it zero;
      if (g_i_cv_BotsQuotaMatch > 0) // 16.09.2006
      {
         CVAR_SET_STRING (g_rgpszPbCvars[PBCVAR_BOTQUOTAMATCH], "0");
         g_i_cv_BotsQuotaMatch = 0;
      }
      if ((arg1 != NULL) && (*arg1 != 0))
         iSkill = atoi (arg1);
      else
         iSkill = 101;

      if ((arg2 != NULL) && (*arg2 != 0))
         iPersonality = atoi (arg2) - 1;
      else
         iPersonality = 5;

      if ((arg3 != NULL) && (*arg3 != 0))
      {
         iTeam = atoi (arg3);
         CVAR_SET_STRING ("mp_limitteams", "0");
         CVAR_SET_STRING ("mp_autoteambalance", "0");
      }
      else
         iTeam = 5;

      if ((arg4 != NULL) && (*arg4 != 0))
         iClass = atoi (arg4);
      else
         iClass = 5;

      for (index = 0; index < gpGlobals->maxClients; index++)
      {
         if (!BotCreateTab[index].bNeedsCreation)
         {
            memset (&BotCreateTab[index], 0, sizeof (createbot_t));
            BotCreateTab[index].bot_skill = iSkill;
            BotCreateTab[index].bot_personality = iPersonality;
            BotCreateTab[index].bot_team = iTeam;
            BotCreateTab[index].bot_class = iClass;
            BotCreateTab[index].bNeedsCreation = TRUE;
         }
      }
      g_iPeoBotsKept = (g_iMax_bots == 0) ? gpGlobals->maxClients : g_iMax_bots;

      if (botcreation_time == 0.0)
         botcreation_time = gpGlobals->time;
   }

   // "killbots" kills all bots
   else if (FStrEq (pcmd, g_rgpszPbCmds[PBCMD_KILLBOTS]))
   {
      for (index = 0; index < gpGlobals->maxClients; index++)
         if (bots[index].is_used && !bots[index].bDead)
         {
            // If a Bot gets killed it decreases his frags, so add 1 here to not give human
            // players an advantage using this command
            bots[index].pEdict->v.frags++;
            MDLL_ClientKill (bots[index].pEdict);
         }

      UTIL_HostPrint ("All Bots killed!\n");
   }

   // "menu"
   else if (FStrEq (pcmd, g_rgpszPbCmds[PBCMD_MENU])
            && !FNullEnt (pEdict))
   {
      UTIL_ShowMenu (pEdict, &menuPODBotMain);
   }

   // "remove" removes a bot
   else if (FStrEq (pcmd, g_rgpszPbCmds[PBCMD_REMOVE]))
   {
      if (arg1[0] == '#')
      {
         for (index = 0; index < gpGlobals->maxClients; ++index)
            if (bots[index].is_used && !FNullEnt (bots[index].pEdict)
               && (GETPLAYERUSERID (bots[index].pEdict) == atoi (&arg1[1])))
            {
               snprintf (kickcmd, sizeof (kickcmd), "kick \"%s\"\n", STRING (bots[index].pEdict->v.netname));
               SERVER_COMMAND (kickcmd); // kick the bot using (kick "name")
               --g_iPeoBotsKept;
               g_fLastKickedBotTime = gpGlobals->time;
            }
      }
      else
      {
         for (index = 0; index < gpGlobals->maxClients; ++index)
            if (bots[index].is_used && !FNullEnt (bots[index].pEdict)
               && !strcmp (arg1, STRING (bots[index].pEdict->v.netname)))
            {
               snprintf (kickcmd, sizeof (kickcmd), "kick \"%s\"\n", STRING (bots[index].pEdict->v.netname));
               SERVER_COMMAND (kickcmd); // kick the bot using (kick "name")
               --g_iPeoBotsKept;
               bots[index].is_used = FALSE; // KWo - 11.02.2006
               g_fLastKickedBotTime = gpGlobals->time;
            }
      }
   }

   // "removebots" removes all bots
   else if (FStrEq (pcmd, g_rgpszPbCmds[PBCMD_REMOVEBOTS]))
   {
      // removebots is against BotsQuotaMatch - needs make it zero;
      if (g_i_cv_BotsQuotaMatch > 0) // 16.09.2006
      {
         CVAR_SET_STRING (g_rgpszPbCvars[PBCVAR_BOTQUOTAMATCH], "0");
         g_i_cv_BotsQuotaMatch = 0;
      }

      g_iPeoBotsKept = ((g_i_cv_MinBots == 0) && !g_bIsDedicatedServer)? 1 : g_i_cv_MinBots;

      for (index = 0; index < gpGlobals->maxClients; index++)
      {
         // Reset our Creation Tab if there are still Bots waiting to be spawned
         memset (&BotCreateTab[index], 0, sizeof (createbot_t));
         botcreation_time = 0.0;

         // is this slot used?
         if (bots[index].is_used && !FNullEnt (bots[index].pEdict))
         {
            snprintf (kickcmd, sizeof (kickcmd), "kick \"%s\"\n", STRING (bots[index].pEdict->v.netname));
            SERVER_COMMAND (kickcmd); // kick the bot using (kick "name")
            bots[index].is_used = FALSE; // KWo - 11.02.2006
         }
      }

      UTIL_HostPrint ("All Bots removed!\n");
   }

   // "set"
   else if (FStrEq (pcmd, g_rgpszPbCmds[PBCMD_SET]))
   {
      for (index = 0; index < NUM_PBCVARS; index++)
      {
         if (FStrEq (arg1, g_rgpszPbCvars[index] + static_cast<char>(3)))
         {
            if ((index == PBCVAR_PASSWORD) || (index == PBCVAR_PASSWORDKEY))
            {
               if (!FNullEnt (pEdict))
                  CLIENT_PRINTF (pEdict, print_console, szSetPWUseRCON);
               else
                  UTIL_ServerPrint (szSetPWUseRCON);
               break;
            }

            CVAR_SET_STRING (g_rgpszPbCvars[index], arg2);
            break;
         }
      }
   }

   // "weaponmode"
   else if (FStrEq (pcmd, g_rgpszPbCmds[PBCMD_WEAPONMODE]))
   {
      if ((atoi (arg1) >= 1) && (atoi (arg1) <= 7))
      {
         iSelection = atoi (arg1);

         int rgiWeaponTabStandard[7][NUM_WEAPONS] =
         {
            {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // Knife only
            {-1,-1,-1, 2, 2, 0, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // Pistols only
            {-1,-1,-1,-1,-1,-1,-1, 2, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // Shotgun only
            {-1,-1,-1,-1,-1,-1,-1,-1,-1, 2, 1, 2, 0, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 2,-1}, // Machine Guns only
            {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 0, 0, 1, 0, 1, 1,-1,-1,-1,-1,-1,-1}, // Rifles only
            {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 2, 2, 0, 1,-1,-1}, // Snipers only
            {-1,-1,-1, 2, 2, 0, 1, 2, 2, 2, 1, 2, 0, 2, 0, 0, 1, 0, 1, 1, 2, 2, 0, 1, 2, 1}  // Standard
         };
         int rgiWeaponTabAS[7][NUM_WEAPONS] =
         {
            {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // Knife only
            {-1,-1,-1, 2, 2, 0, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // Pistols only
            {-1,-1,-1,-1,-1,-1,-1, 1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // Shotgun only
            {-1,-1,-1,-1,-1,-1,-1,-1,-1, 1, 1, 1, 0, 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 1,-1}, // Machine Guns only
            {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 0,-1, 1, 0, 1, 1,-1,-1,-1,-1,-1,-1}, // Rifles only
            {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1, 0, 0,-1, 1,-1,-1}, // Snipers only
            {-1,-1,-1, 2, 2, 0, 1, 1, 1, 1, 1, 1, 0, 2, 0,-1, 1, 0, 1, 1, 0, 0,-1, 1, 1, 1}  // Standard
         };

         char szMode[7][12] =
         {
            {"Knife"},
            {"Pistol"},
            {"Shotgun"},
            {"Machine Gun"},
            {"Rifle"},
            {"Sniper"},
            {"Standard"}
         };

         iSelection--;
         for (index = 0; index < NUM_WEAPONS; index++)
         {
            cs_weapon_select[index].iTeamStandard = rgiWeaponTabStandard[iSelection][index];
            cs_weapon_select[index].iTeamAS = rgiWeaponTabAS[iSelection][index];
         }

         if (iSelection == 0)
         {
            CVAR_SET_FLOAT (g_rgpszPbCvars[PBCVAR_JASONMODE], 1.f);
            g_b_cv_jasonmode = true;
         }
         else
         {
            CVAR_SET_FLOAT (g_rgpszPbCvars[PBCVAR_JASONMODE], 0.f);
            g_b_cv_jasonmode = false;
         }
         UTIL_HostPrint ("%s Weapon Mode selected!\n", &szMode[iSelection][0]);
      }
      else
         UTIL_ServerPrint ("weaponmode must be between 1 and 7\n");
   }

   // "wp"
   else if (FStrEq (pcmd, g_rgpszPbCmds[PBCMD_WP]))
   {
      if (FStrEq (arg1, "stats"))
      {
         int iButtonPoints = 0;
         int iLiftPoints = 0;
         int iCrouchPoints = 0;
         int iCrossingPoints = 0;
         int iGoalPoints = 0;
         int iLadderPoints = 0;
         int iRescuePoints = 0;
         int iCampPoints = 0;
         int iNoHostagePoints = 0;
         int iTerroristPoints = 0;
         int iCounterPoints = 0;
         int iNormalPoints = 0;

         for (int i = 0; i < g_iNumWaypoints; i++)
         {
            if (paths[i]->flags & W_FL_USE_BUTTON)
               iButtonPoints++;
            if (paths[i]->flags & W_FL_LIFT)
               iLiftPoints++;
            if (paths[i]->flags & W_FL_CROUCH)
               iCrouchPoints++;
            if (paths[i]->flags & W_FL_CROSSING)
               iCrossingPoints++;
            if (paths[i]->flags & W_FL_GOAL)
               iGoalPoints++;
            if (paths[i]->flags & W_FL_LADDER)
               iLadderPoints++;
            if (paths[i]->flags & W_FL_RESCUE)
               iRescuePoints++;
            if (paths[i]->flags & W_FL_CAMP)
               iCampPoints++;
            if (paths[i]->flags & W_FL_NOHOSTAGE)
               iNoHostagePoints++;
            if (paths[i]->flags & W_FL_TERRORIST)
               iTerroristPoints++;
            if (paths[i]->flags & W_FL_COUNTER)
               iCounterPoints++;
            if (paths[i]->flags == 0)
               iNormalPoints++;
         }

         UTIL_ServerPrint ("Waypoint Statistics:\n"
                           "--------------------\n"
                           "Waypoints classification per flag:\n");
         UTIL_ServerPrint ("W_FL_USE_BUTTON: %d\n", iButtonPoints);
         UTIL_ServerPrint ("W_FL_LIFT: %d\n", iLiftPoints);
         UTIL_ServerPrint ("W_FL_CROUCH: %d\n", iCrouchPoints);
         UTIL_ServerPrint ("W_FL_CROSSING: %d\n", iCrossingPoints);
         UTIL_ServerPrint ("W_FL_GOAL: %d\n", iGoalPoints);
         UTIL_ServerPrint ("W_FL_LADDER: %d\n", iLadderPoints);
         UTIL_ServerPrint ("W_FL_RESCUE: %d\n", iRescuePoints);
         UTIL_ServerPrint ("W_FL_CAMP: %d\n", iCampPoints);
         UTIL_ServerPrint ("W_FL_NOHOSTAGE: %d\n", iNoHostagePoints);
         UTIL_ServerPrint ("W_FL_TERRORIST: %d\n", iTerroristPoints);
         UTIL_ServerPrint ("W_FL_COUNTER: %d\n", iCounterPoints);
         UTIL_ServerPrint ("Not flagged: %d\n", iNormalPoints);
         UTIL_ServerPrint ("--------------------\n"
                           "Total number of waypoints: %d\n", g_iNumWaypoints);
      }
      else if (FStrEq (arg1, "teleport"))
      {
         if ((pEdict == pHostEdict) && !g_bIsDedicatedServer)
         {
            if (FStrEq (arg2, "use"))  // KWo - 30.07.2006
            {
               TraceResult tr;
               int iTemp_Ind = WaypointFindNearest();
               int i;
               bool b_flag_found = false;
               if ((iTemp_Ind >= 0) && (iTemp_Ind < g_iNumWaypoints))
               {
                  for (i = iTemp_Ind + 1; i < g_iNumWaypoints; i++)
                  {
                     if (paths[i]->flags & W_FL_USE_BUTTON)
                     {
                        TRACE_HULL (paths[i]->origin + Vector (0, 0, 32),
                           paths[i]->origin + Vector (0, 0, -32),
                           ignore_monsters, human_hull, pHostEdict, &tr);
                        SET_ORIGIN (pHostEdict, tr.vecEndPos);
                        b_flag_found = true;
                        break;
                     }
                  }
                  if (!b_flag_found) // reached end and found nothing - search from the first WP (index = 0)
                  {
                     for (i = 0; i < g_iNumWaypoints; i++)
                     {
                        if (paths[i]->flags & W_FL_USE_BUTTON)
                        {
                           TRACE_HULL (paths[i]->origin + Vector (0, 0, 32),
                              paths[i]->origin + Vector (0, 0, -32),
                              ignore_monsters, human_hull, pHostEdict, &tr);
                           SET_ORIGIN (pHostEdict, tr.vecEndPos);
                           b_flag_found = true;
                           break;
                        }
                     }
                     if (!b_flag_found)
                        UTIL_ServerPrint ("There is no Waypoint with USE_BUTTON flag.\n");
                  }
               }
            }
            else if (FStrEq (arg2, "camp"))  // KWo - 30.07.2006
            {
               TraceResult tr;
               int iTemp_Ind = WaypointFindNearest();
               int i;
               bool b_flag_found = false;
               if ((iTemp_Ind >= 0) && (iTemp_Ind < g_iNumWaypoints))
               {
                  for (i = iTemp_Ind + 1; i < g_iNumWaypoints; i++)
                  {
                     if (paths[i]->flags & W_FL_CAMP)
                     {
                        TRACE_HULL (paths[i]->origin + Vector (0, 0, 32),
                           paths[i]->origin + Vector (0, 0, -32),
                           ignore_monsters, human_hull, pHostEdict, &tr);
                        SET_ORIGIN (pHostEdict, tr.vecEndPos);
                        b_flag_found = true;
                        break;
                     }
                  }
                  if (!b_flag_found) // reached end and found nothing - search from the first WP (index = 0)
                  {
                     for (i = 0; i < g_iNumWaypoints; i++)
                     {
                        if (paths[i]->flags & W_FL_CAMP)
                        {
                           TRACE_HULL (paths[i]->origin + Vector (0, 0, 32),
                              paths[i]->origin + Vector (0, 0, -32),
                              ignore_monsters, human_hull, pHostEdict, &tr);
                           SET_ORIGIN (pHostEdict, tr.vecEndPos);
                           b_flag_found = true;
                           break;
                        }
                     }
                     if (!b_flag_found)
                        UTIL_ServerPrint ("There is no Waypoint with CAMP flag.\n");
                  }
               }
            }
            else if (FStrEq (arg2, "goal"))  // KWo - 08.06.2010
            {
               TraceResult tr;
               int iTemp_Ind = WaypointFindNearest();
               int i;
               bool b_flag_found = false;
               if ((iTemp_Ind >= 0) && (iTemp_Ind < g_iNumWaypoints))
               {
                  for (i = iTemp_Ind + 1; i < g_iNumWaypoints; i++)
                  {
                     if (paths[i]->flags & W_FL_GOAL)
                     {
                        TRACE_HULL (paths[i]->origin + Vector (0, 0, 32),
                           paths[i]->origin + Vector (0, 0, -32),
                           ignore_monsters, human_hull, pHostEdict, &tr);
                        SET_ORIGIN (pHostEdict, tr.vecEndPos);
                        b_flag_found = true;
                        break;
                     }
                  }
                  if (!b_flag_found) // reached end and found nothing - search from the first WP (index = 0)
                  {
                     for (i = 0; i < g_iNumWaypoints; i++)
                     {
                        if (paths[i]->flags & W_FL_GOAL)
                        {
                           TRACE_HULL (paths[i]->origin + Vector (0, 0, 32),
                              paths[i]->origin + Vector (0, 0, -32),
                              ignore_monsters, human_hull, pHostEdict, &tr);
                           SET_ORIGIN (pHostEdict, tr.vecEndPos);
                           b_flag_found = true;
                           break;
                        }
                     }
                     if (!b_flag_found)
                        UTIL_ServerPrint ("There is no Waypoint with GOAL flag.\n");
                  }
               }
            }
            else if ((atoi (arg2) >= 0) && (atoi (arg2) < g_iNumWaypoints))
            {
               TraceResult tr;
               TRACE_HULL (paths[atoi (arg2)]->origin + Vector (0, 0, 32),
                           paths[atoi (arg2)]->origin + Vector (0, 0, -32),
                           ignore_monsters, human_hull, pHostEdict, &tr);
               SET_ORIGIN (pHostEdict, tr.vecEndPos);
            }
         }
         else
         {
            UTIL_ServerPrint (szIsNotHostOfListen);
         }
      }
      else if (FStrEq (arg1, "clean"))
      {
         if ((pEdict == pHostEdict) && !g_bIsDedicatedServer)
         {
            if (FStrEq (arg2, "all"))
            {
               int i;
               for (i = 0; i < g_iNumWaypoints; i++)
               {
//                  if (!(paths[i]->flags & W_FL_JUMP))
//                  {
                  WaypointCleanUnnessPaths(i);
//                  }
               }
               UTIL_ServerPrint("Command >wp clean all< has been executed.\n");
            }
            else if ((arg2 == NULL) || (*arg2 == 0))
            {
               int iTemp_Ind = WaypointFindNearest();
               if ((iTemp_Ind >= 0) && (iTemp_Ind < g_iNumWaypoints))
                  WaypointCleanUnnessPaths(iTemp_Ind);
            }
            else if ((arg2 != NULL) && (*arg2 != 0))
            {
               if ((atoi (arg2) >= 0) && (atoi (arg2) < g_iNumWaypoints))
               {
                  int iTemp_Ind = atoi (arg2);
                  if ((iTemp_Ind >= 0) && (iTemp_Ind < g_iNumWaypoints))
                     WaypointCleanUnnessPaths(iTemp_Ind);
               }
            }
         }
         else
         {
            UTIL_ServerPrint (szIsNotHostOfListen);
         }
      }
      else if (FStrEq (arg1, "fixcamp"))
      {
         if ((pEdict == pHostEdict) && !g_bIsDedicatedServer)
         {
            if (FStrEq (arg2, "all"))
            {
               int i;
               for (i = 0; i < g_iNumWaypoints; i++)
               {
                  WaypointFixOldCampType(i);
               }
               UTIL_ServerPrint("Command >wp fix all< has been executed.\n");
            }
            else if ((arg2 == NULL) || (*arg2 == 0))
            {
               int iTemp_Ind = WaypointFindNearest();
               if ((iTemp_Ind >= 0) && (iTemp_Ind < g_iNumWaypoints))
                  WaypointFixOldCampType(iTemp_Ind);
            }
            else if ((arg2 != NULL) && (*arg2 != 0))
            {
               if ((atoi (arg2) >= 0) && (atoi (arg2) < g_iNumWaypoints))
               {
                  int iTemp_Ind = atoi (arg2);
                  if ((iTemp_Ind >= 0) && (iTemp_Ind < g_iNumWaypoints))
                     WaypointFixOldCampType(iTemp_Ind);
               }
            }
         }
      }
      else
      {
         UTIL_ServerPrint ("Unknown waypoint command.\n"
                           "waypoint commands are: stats teleport clean fixcamp\n");  // KWo - 21.05.2013
      }
   }

   // "wpmenu"
   else if (FStrEq (pcmd, g_rgpszPbCmds[PBCMD_WPMENU]))
   {
      if ((pEdict == pHostEdict) && !g_bIsDedicatedServer)
      {
         UTIL_ShowMenu (pEdict, &menuWpMain);
         if (!g_bWaypointOn) // KWo - 13.08.2006
            UTIL_DisplayWpMenuWelcomeMessage ();
      }
      else
      {
         UTIL_ServerPrint (szIsNotHostOfListen);
      }
   }

   // "help"
   else if (FStrEq (pcmd, g_rgpszPbCmds[PBCMD_HELP])) // KWo - 31.01.2010
   {
      if (FStrEq (arg1, g_rgpszPbCmds[PBCMD_ADD])) // pb add coommand
      {
         UTIL_ServerPrint ("pb add [skill [personality [team [model [name]]]]]\n");
         UTIL_ServerPrint ("This command adds a Bot with parameters:\n");
         UTIL_ServerPrint ("a) Skill - You can write it from 1 to 100. If You will write 101, it will mean\n");
         UTIL_ServerPrint ("   a random skill from pb_minbotskill to pb_maxbotskill cvars.\n");
         UTIL_ServerPrint ("b) Personality - 1 to 3 or 5 and these value mean:\n");
         UTIL_ServerPrint ("   1 - Normal [POD], 2 - Aggresive [P*D], 3 - Defensive [P0D],\n");
         UTIL_ServerPrint ("   5 - Random personality from these 3 above.\n");
         UTIL_ServerPrint ("c) Team - 1 is for Terrorists, 2 for CTs and 5 to use auto-assign.\n");
         UTIL_ServerPrint ("   Note - this setting might be overwritten by forcing bots to join always\n");
         UTIL_ServerPrint ("   one team if pb_bot_join_team cvar is equal to \"T\" or \"CT\".\n");
         UTIL_ServerPrint ("d) Model (just a class) - is 1-4 or 5 (5 means random CS models from 1 to 4).\n");
         UTIL_ServerPrint ("e) Name is of course the name for the bot.\n");
         UTIL_ServerPrint ("You can skip all parameters from the right of the last specified, but\n");
         UTIL_ServerPrint ("You cannot skip any parameter in the middle of the command nor between, i.e\n");
         UTIL_ServerPrint ("pb add 100 3 - it's the right command, but pb add 100 \"Roger\" is not (You\n");
         UTIL_ServerPrint ("skipped the personality and the model parameters - between skill and name.\n");
      }
      else if (FStrEq (arg1, g_rgpszPbCmds[PBCMD_FILLSERVER])) // pb fillserver coommand
      {
         UTIL_ServerPrint ("pb fillserver [skill [personality [team [model]]]]\n");
         UTIL_ServerPrint ("This command fills the server with bots with parameters:\n");
         UTIL_ServerPrint ("a) Skill - You can write it from 1 to 100. If You will write 101 it will mean\n");
         UTIL_ServerPrint ("   a random skill from pb_minbotskili to pb_maxbotskill cvars.\n");
         UTIL_ServerPrint ("b) Personality - 1 to 3 or 5 and these value mean:\n");
         UTIL_ServerPrint ("   1 - Normal [POD], 2 - Aggresive [P*D], 3 - Defensive [P0D],\n");
         UTIL_ServerPrint ("   5 - Random personality from these 3 above.\n");
         UTIL_ServerPrint ("c) Team - 1 is for Terrorists, 2 for CTs and 5 to use auto-assign.\n");
         UTIL_ServerPrint ("   Note - this setting might be overwritten by forcing bots to join always\n");
         UTIL_ServerPrint ("   one team if pb_bot_join_team cvar is equal to \"T\" or \"CT\".\n");
         UTIL_ServerPrint ("d) Model (just a class) - is 1-4 or 5 (5 means random CS models from 1 to 4).\n");
         UTIL_ServerPrint ("\n");
         UTIL_ServerPrint ("The server will be filled by bots until podbots + human_players\n");
         UTIL_ServerPrint ("(or other bots) amount on the server will reach:\n");
         UTIL_ServerPrint ("a) the pb_maxbots value if it is specified (different than 0),\n");
         UTIL_ServerPrint ("b) the mp_maxplayers value if pb_maxbots is not specified (or is equal to 0)\n");
         UTIL_ServerPrint ("Note:\n");
         UTIL_ServerPrint ("The command pb fillserver forces the cvar pb_bot_quota_match to 0.\n");
      }
      else if (FStrEq (arg1, g_rgpszPbCmds[PBCMD_KILLBOTS])) // pb killbots
      {
         UTIL_ServerPrint ("pb killbots\n");
         UTIL_ServerPrint ("It just kills all podbot mm bots on the server.\n");
      }
      else if (FStrEq (arg1, g_rgpszPbCmds[PBCMD_REMOVEBOTS]))  // pb removebots
      {
         UTIL_ServerPrint ("pb removebots\n");
         UTIL_ServerPrint ("It kicks all bots from the server (until reaching pb_minbots amount of them\n");
         UTIL_ServerPrint ("on the server).\n");
         UTIL_ServerPrint ("Note:\n");
         UTIL_ServerPrint ("This command overwrites pb_bot_quota_match value to 0 - similary like\n");
         UTIL_ServerPrint ("pb fillserver command.\n");
      }
      else if (FStrEq (arg1, g_rgpszPbCmds[PBCMD_REMOVE])) // pb remove
      {
         UTIL_ServerPrint ("pb remove #userid\n");
         UTIL_ServerPrint ("It kicks from the server the bot with specified userid (preceded with #).\n");
         UTIL_ServerPrint ("You can see the userid of any player/bot on the server after typing\n");
         UTIL_ServerPrint ("command \"status\" in the CS console.\n");
         UTIL_ServerPrint ("\n");
         UTIL_ServerPrint ("pb remove botname\n");
         UTIL_ServerPrint ("It kicks from the server the bot with specified name (preceded with [POD],\n");
         UTIL_ServerPrint ("[P*D], [P0D] prefix - if they are used) and skill nr (also if used).\n");
         UTIL_ServerPrint ("Note:\n");
         UTIL_ServerPrint ("If the name of the bot contains the space (\" \"), to successful use that\n");
         UTIL_ServerPrint ("command, it has to be used with quotes - i.e.\n");
         UTIL_ServerPrint ("pb remove \"[POD]James Bond(100)\"\n");
      }
      else if (FStrEq (arg1, g_rgpszPbCmds[PBCMD_WEAPONMODE])) // pb weaponmode
      {
         UTIL_ServerPrint ("pb weaponmode nr\n");
         UTIL_ServerPrint ("It selects the weapon mode for bots with weapon type nr 1-7.\n");
         UTIL_ServerPrint ("The available weaponmode nr parameter meaning:\n");
         UTIL_ServerPrint ("1 - Knife, 2 - Pistol, 3 - Shotgun, 4 - Machine Gun, 5 - Rifle, 6 - Sniper,\n");
         UTIL_ServerPrint ("7 - Standard (last means back to normal weapon mode - all weapons available).\n");
      }
      else if (FStrEq (arg1, g_rgpszPbCmds[PBCMD_SET])) // pb set
      {
         UTIL_ServerPrint ("pb set cvar value\n");
         UTIL_ServerPrint ("It sets the certain cvar to the value specified (if the user has rights\n");
         UTIL_ServerPrint ("to change it).\n");
      }
      else if (FStrEq (arg1, g_rgpszPbCmds[PBCMD_DEBUGGOAL]))  // pb debuggoal
      {
         UTIL_ServerPrint ("pb debuggoal x\n");
         UTIL_ServerPrint ("Forces all bots to use the specified waypoint (x argument) as a Goal.\n");
         UTIL_ServerPrint ("If the argument is \"-1\", bots are no longer forced to use the waypoint x\n");
         UTIL_ServerPrint ("(specified earlier with that command) as their goal (they are released\n");
         UTIL_ServerPrint ("to normal choosing their goals).\n");
      }
      else if (FStrEq (arg1, g_rgpszPbCmds[PBCMD_EXPERIENCE])) // pb experience
      {
         UTIL_ServerPrint ("pb experience string\n");
         UTIL_ServerPrint ("This command cannot be used separately (like just pb experience).\n");
         UTIL_ServerPrint ("It needs other string arguments like:\n");
         UTIL_ServerPrint ("save, show, test_a_T, test_a_CT, test_a_CTH.\n");
         UTIL_ServerPrint ("\n");
         UTIL_ServerPrint ("pb experience save\n");
         UTIL_ServerPrint ("It saves on the HDD the experience table (*.pxp) and the visibility\n");
         UTIL_ServerPrint ("table (*.pvi).\n");
         UTIL_ServerPrint ("\n");
         UTIL_ServerPrint ("pb experience show\n");
         UTIL_ServerPrint ("It lists on the screen (in the console) the experience table (with\n");
         UTIL_ServerPrint ("\"danger\" values for each waypoint). It also shows the most danger WP\n");
         UTIL_ServerPrint ("index for each WP for each team.\n");
         UTIL_ServerPrint ("\n");
         UTIL_ServerPrint ("pb experience test_a_T sourceWP destWP byPathType\n");
         UTIL_ServerPrint ("It shows on the screen the shortest path the Terrorist bot would choose\n");
         UTIL_ServerPrint ("if it would start from the sourceWP index (int) to destWP index (int)\n");
         UTIL_ServerPrint ("depanding on byPathType argument:\n");
         UTIL_ServerPrint ("0 - only 3D distance is taken into account to calculate the path;\n");
         UTIL_ServerPrint ("1 - both (3D dist. and exp. data) are taken into account to calculate the path;\n");
         UTIL_ServerPrint ("2 - only experience data is taken into account to calculate the path.\n");
         UTIL_ServerPrint ("\n");
         UTIL_ServerPrint ("pb experience test_a_CT sourceWP destWP byPathType\n");
         UTIL_ServerPrint ("It shows on the screen the shortest path the CT bot would choose\n");
         UTIL_ServerPrint ("if it would start from the sourceWP index (int) to destWP index (int)\n");
         UTIL_ServerPrint ("depanding on byPathType argument:\n");
         UTIL_ServerPrint ("0 - only 3D distance is taken into account to calculate the path;\n");
         UTIL_ServerPrint ("1 - both (3D dist. and exp. data) are taken into account to calculate the path;\n");
         UTIL_ServerPrint ("2 - only experience data is taken into account to calculate the path.\n");
         UTIL_ServerPrint ("\n");
         UTIL_ServerPrint ("pb experience test_a_CTH sourceWP destWP byPathType\n");
         UTIL_ServerPrint ("It shows on the screen the shortest path the CT bot with a hostage would choose\n");
         UTIL_ServerPrint ("if it would start from the sourceWP index (int) to destWP index (int)\n");
         UTIL_ServerPrint ("depanding on byPathType argument:\n");
         UTIL_ServerPrint ("0 - only 3D distance is taken into account to calculate the path;\n");
         UTIL_ServerPrint ("1 - both (3D dist. and exp. data) are taken into account to calculate the path;\n");
         UTIL_ServerPrint ("2 - only experience data is taken into account to calculate the path.\n");
      }
      else if (FStrEq (arg1, g_rgpszPbCmds[PBCMD_WP]))
      {
         UTIL_ServerPrint ("pb wp arg1 [arg2]\n");
         UTIL_ServerPrint ("It neads other string as an argument - stats, teleport, clean, fixcamp.\n");    // KWo - 21.05.2013
         UTIL_ServerPrint ("\n");
         UTIL_ServerPrint ("pb wp stats\n");
         UTIL_ServerPrint ("shows all the statistics about used waypoints on the map (with the number\n");
         UTIL_ServerPrint ("of used particular types flags for them).\n");
         UTIL_ServerPrint ("\n");
         UTIL_ServerPrint ("pb wp teleport nr\n");
         UTIL_ServerPrint ("It teleports the host of the listenserver to the ceratin WP number.\n");
         UTIL_ServerPrint ("\n");
         UTIL_ServerPrint ("pb wp teleport camp\n");
         UTIL_ServerPrint ("It teleports the host of the listenserver to the next camp WP (useful to fix\n");
         UTIL_ServerPrint ("all camp waypoints on the map, if the waypoint file has been created by any\n");
         UTIL_ServerPrint ("earlier than 2006 version of podbot mm).\n");
         UTIL_ServerPrint ("\n");
         UTIL_ServerPrint ("pb wp teleport use\n");
         UTIL_ServerPrint ("It teleports the host of the listenserver to the next WP with \"USE\" flag.\n");
         UTIL_ServerPrint ("That flag was added by default (by earlier versions of podbot) in waypoints\n");
         UTIL_ServerPrint ("file to any WP close to any button, camera etc.\n");
         UTIL_ServerPrint ("\n");
         UTIL_ServerPrint ("pb wp clean nr\n");
         UTIL_ServerPrint ("It cleans all connections from/to certain WP considered by the code\n");
         UTIL_ServerPrint ("as unnecessary.\n");
         UTIL_ServerPrint ("\n");
         UTIL_ServerPrint ("pb wp clean all\n");
         UTIL_ServerPrint ("It needs to be used very carefully - it cleans all connections considered\n");
         UTIL_ServerPrint ("by the code as unnecessary from all waypoints. It may cause troubles sometimes.\n");
         UTIL_ServerPrint ("pb wp fixcamp nr\n");                                                           // KWo - 21.05.2013
         UTIL_ServerPrint ("It converts certain WP from vector (old type) to angles (pb mm type).\n");      // KWo - 21.05.2013
         UTIL_ServerPrint ("\n");                                                                           // KWo - 21.05.2013
         UTIL_ServerPrint ("pb wp fixcamp all\n");                                                          // KWo - 21.05.2013
         UTIL_ServerPrint ("It converts all camp WPs from vectors (old type) to angles (pb mm type).\n");   // KWo - 21.05.2013
      }
      else if (FStrEq (arg1, g_rgpszPbCmds[PBCMD_WPMENU])) // pb wpmenu
      {
         UTIL_ServerPrint ("pb wpmenu\n");
         UTIL_ServerPrint ("It invokes the waypoint menu for the host on the listenserver and it starts\n");
         UTIL_ServerPrint ("the Waypoint Editor (by showing waypoints around if they are already there).\n");
      }
      else if (FStrEq (arg1, g_rgpszPbCmds[PBCMD_MENU])) // pb menu
      {
         UTIL_ServerPrint ("pb menu\n");
         UTIL_ServerPrint ("It invokes for the user the main podbot control menu (if the user has rights\n");
         UTIL_ServerPrint ("to use it)\n");
         UTIL_ServerPrint ("To get the access to the main podbot menu on the dedicated server, You need\n");
         UTIL_ServerPrint ("to take care about 2 cvars strings in podbot.cfg:\n");
         UTIL_ServerPrint ("pb_passwordkey string - passwordkey - needs to be set as setinfo to get\n");
         UTIL_ServerPrint ("the access to the main podbot menu. Default string value is \"_pbadminpw\"\n");
         UTIL_ServerPrint ("pb_password string - sets the password to let the admin control bots on\n");
         UTIL_ServerPrint ("the server through podbotmenu.\n");
         UTIL_ServerPrint ("So - if there are lines in Your podbot.cfg (on the server) like these:\n");
         UTIL_ServerPrint ("pb_password \"your_password\"\n");
         UTIL_ServerPrint ("and\n");
         UTIL_ServerPrint ("pb_passwordkey \"_pbadminpw\"\n");
         UTIL_ServerPrint ("all You need to get the access from the client's CS PC is to write\n");
         UTIL_ServerPrint ("in autoexec.cfg the setinfo line (at Your client PC in \"cstrike\" or \"czero\"\n");
         UTIL_ServerPrint ("directory - depanding on the mod You are playing) as:\n");
         UTIL_ServerPrint ("setinfo \"_pbadminpw\" \"your_password\"\n");
         UTIL_ServerPrint ("The string was called above as \"your_password\" it's just Your password You need\n");
         UTIL_ServerPrint ("to write in these both places - in podbot.cfg (near pb_password) on the server\n");
         UTIL_ServerPrint ("and in autoexec.cfg (as setinfo line) on the client PC.\n");
      }
      else
      {
         UTIL_ServerPrint ("Podbot mm command list:\n"
                           "  All commands must be preceded with \"%s\"\n"
                           "    For example: \"%s %s\"\n"
                           "\n"
                           "  \"%s\"   \"%s\"   \"%s\"\n"
                           "  \"%s\"   \"%s\"   \"%s\"\n"
                           "  \"%s\"   \"%s\"   \"%s\"\n"
                           "  \"%s\"   \"%s\"   \"%s\"\n"
                           "Type \"%s %s <command>\" for more information.\n",
                           g_rgpszPbCmds[PBCMD],
                           g_rgpszPbCmds[PBCMD],
                           g_rgpszPbCmds[PBCMD_ADD],
                           g_rgpszPbCmds[PBCMD_ADD],
                           g_rgpszPbCmds[PBCMD_DEBUGGOAL],
                           g_rgpszPbCmds[PBCMD_EXPERIENCE],
                           g_rgpszPbCmds[PBCMD_FILLSERVER],
                           g_rgpszPbCmds[PBCMD_HELP],
                           g_rgpszPbCmds[PBCMD_KILLBOTS],
                           g_rgpszPbCmds[PBCMD_MENU],
                           g_rgpszPbCmds[PBCMD_REMOVE],
                           g_rgpszPbCmds[PBCMD_REMOVEBOTS],
                           g_rgpszPbCmds[PBCMD_WEAPONMODE],
                           g_rgpszPbCmds[PBCMD_WP],
                           g_rgpszPbCmds[PBCMD_WPMENU],
                           g_rgpszPbCmds[PBCMD],
                           g_rgpszPbCmds[PBCMD_HELP]);
      }
   }
   // If it were none of the above commands tell the user
   else
      UTIL_ServerPrint ("\"%s\" is not a Podbot command. Type \"%s %s\" for a list of commands\n", pcmd, g_rgpszPbCmds[PBCMD], g_rgpszPbCmds[PBCMD_HELP]);

   return;
}


void UserNewroundAll (void)
{
   // Called by UserCommand "newround"
   // Kills all Clients in Game including humans

   edict_t *pPlayer;

   for (int i = 1; i <= gpGlobals->maxClients; i++)
   {
      pPlayer = INDEXENT (i);

      // is this player slot valid
      if (!FNullEnt (pPlayer) && (pPlayer->v.flags & FL_CLIENT) && IsAlive (pPlayer))
      {
         pPlayer->v.frags++;
         MDLL_ClientKill (pPlayer);
      }
   }

   UTIL_HostPrint ("Round Restarted !\n");
}


void ShowPBKickBotMenu (edict_t *pEntity, int iMenuNum)
{
   if ((iMenuNum > 4) || (iMenuNum < 1))
      return;

   char szTemp1[512], szTemp2[512];
   int i = 0, iValidSlots = 0;
   memset (szTemp1, 0, sizeof (szTemp1));
   memset (szTemp2, 0, sizeof (szTemp2));
   const char * teamname = "UN";  // KWo - 24.04.2006
   int iTeam_nr;  // KWo - 12.02.2006

   iValidSlots = (iMenuNum == 4)? (1 << 9) : ((1 << 8) | (1 << 9));
   for (i = ((iMenuNum - 1) * 8); i < iMenuNum * 8; ++i)
   {
      if (bots[i].is_used && !FNullEnt (bots[i].pEdict))
      {
         iValidSlots |= 1 << (i - ((iMenuNum - 1) * 8));
         memmove (szTemp1, szTemp2, strlen (szTemp2));

         iTeam_nr = UTIL_GetTeam (bots[i].pEdict);  // KWo - 12.02.2006
         switch (iTeam_nr)  // KWo - 12.02.2006
         {
            case 0:
               teamname="UN";
               break;
            case 1:
               teamname="T";
               break;
            case 2:
               teamname="CT";
               break;
            case 3:
               teamname="SP";
               break;
         }
//         snprintf (szTemp2, sizeof (szTemp2), "%s %1.1d. %s (%s)\n", szTemp1, i - ((iMenuNum - 1) * 8) + 1, STRING (bots[i].pEdict->v.netname), (UTIL_GetTeam (bots[i].pEdict) == TEAM_CS_COUNTER ? "CT" : "T"));
         snprintf (szTemp2, sizeof (szTemp2), "%s %1.1d. %s (%s)\n", szTemp1, i - ((iMenuNum - 1) * 8) + 1, STRING (bots[i].pEdict->v.netname), teamname);
         memset (szTemp1, 0, sizeof (szTemp1));
      }
      else
      {
         memmove (szTemp1, szTemp2, strlen (szTemp2));
         snprintf (szTemp2, sizeof (szTemp2), "%s\\d %1.1d. Not a PodbotMM\\w\n", szTemp1, i - ((iMenuNum - 1) * 8) + 1);
         memset (szTemp1, 0, sizeof (szTemp1));
      }
   }
   memset (szTemp1, 0, sizeof (szTemp1));
   snprintf (szTemp1, sizeof (szTemp1),
      " \\yKick Bot Menu#%d:\\w\n"
      "\n"
      "\n"
      "%s"
      "\n"
      "%s"
      " 0. Back",
      iMenuNum,
      szTemp2,
      (iMenuNum == 4)? "" : " 9. More...\n");
   if (iMenuNum == 1)
   {
      menuPODBotKickBot1.ValidSlots = iValidSlots;
      menuPODBotKickBot1.szMenuText = szTemp1;
      UTIL_ShowMenu (pEntity, &menuPODBotKickBot1);
   }
   else if (iMenuNum == 2)
   {
      menuPODBotKickBot2.ValidSlots = iValidSlots;
      menuPODBotKickBot2.szMenuText = szTemp1;
      UTIL_ShowMenu (pEntity, &menuPODBotKickBot2);
   }
   else if (iMenuNum == 3)
   {
      menuPODBotKickBot3.ValidSlots = iValidSlots;
      menuPODBotKickBot3.szMenuText = szTemp1;
      UTIL_ShowMenu (pEntity, &menuPODBotKickBot3);
   }
   else if (iMenuNum == 4)
   {
      menuPODBotKickBot4.ValidSlots = iValidSlots;
      menuPODBotKickBot4.szMenuText = szTemp1;
      UTIL_ShowMenu (pEntity, &menuPODBotKickBot4);
   }
   return;
}


void GetGameDir (void)
{
   // This function fixes the erratic behaviour caused by the use of the GET_GAME_DIR engine
   // macro, which returns either an absolute directory path, or a relative one, depending on
   // whether the game server is run standalone or not. This one always return a RELATIVE path.

   unsigned char length, fieldstart, fieldstop;

   GET_GAME_DIR (g_szGameDirectory); // call the engine macro and let it mallocate for the char pointer

   length = (unsigned char)strlen (g_szGameDirectory); // get the length of the returned string
   length--; // ignore the trailing string terminator

   // format the returned string to get the last directory name
   fieldstop = length;
   while (((g_szGameDirectory[fieldstop] == '\\') || (g_szGameDirectory[fieldstop] == '/')) && (fieldstop > 0))
      fieldstop--; // shift back any trailing separator

   fieldstart = fieldstop;
   while ((g_szGameDirectory[fieldstart] != '\\') && (g_szGameDirectory[fieldstart] != '/') && (fieldstart > 0))
      fieldstart--; // shift back to the start of the last subdirectory name

   if ((g_szGameDirectory[fieldstart] == '\\') || (g_szGameDirectory[fieldstart] == '/'))
      fieldstart++; // if we reached a separator, step over it

   // now copy the formatted string back onto itself character per character
   for (length = fieldstart; length <= fieldstop; length++)
      g_szGameDirectory[length - fieldstart] = g_szGameDirectory[length];
   g_szGameDirectory[length - fieldstart] = 0; // terminate the string

   return;
}


bool IsPBAdmin (edict_t *pEdict)
{
   if ((pEdict == pHostEdict) || (clients[ENTINDEX (pEdict) - 1].iFlags & CLIENT_ADMIN))
      return (TRUE);

   return (FALSE);
}


C_DLLEXPORT int GetEntityAPI2 (DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion)
{
   gFunctionTable.pfnGameInit = GameDLLInit;
   gFunctionTable.pfnSpawn = Spawn;
   gFunctionTable.pfnClientConnect = ClientConnect;
   gFunctionTable.pfnClientDisconnect = ClientDisconnect;
   gFunctionTable.pfnClientPutInServer = ClientPutInServer;
   gFunctionTable.pfnClientCommand = ClientCommand;
   gFunctionTable.pfnServerActivate = ServerActivate;
   gFunctionTable.pfnServerDeactivate = ServerDeactivate;
   gFunctionTable.pfnStartFrame = StartFrame;
   gFunctionTable.pfnKeyValue = Pfn_KeyValue;                  // KWo - 06.03.2006
   gFunctionTable.pfnPM_Move = PM_Move;                        // KWo - 15.03.2012 - thanks to Immortal_BLG
   gFunctionTable.pfnUpdateClientData = Pfn_UpdateClientData;  // KWo - 02.03.2010

   memcpy (pFunctionTable, &gFunctionTable, sizeof (DLL_FUNCTIONS));
   return (TRUE);
}


C_DLLEXPORT int GetEntityAPI2_Post (DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion)
{
   gFunctionTable_Post.pfnSpawn = Spawn_Post;
   gFunctionTable_Post.pfnClientUserInfoChanged = ClientUserInfoChanged_Post;

   memcpy (pFunctionTable, &gFunctionTable_Post, sizeof (DLL_FUNCTIONS));
   return (TRUE);
}
