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
// bot_globals.h
//
// Only File to include in the Code (does include all the rest)

#ifndef BOT_GLOBALS_H
#define BOT_GLOBALS_H

#ifndef _WIN32
#include <string.h>
#include <ctype.h>
#endif

#include <extdll.h>
#include <dllapi.h>
#include <h_export.h>
#include <meta_api.h>
//#include <entity_state.h> // Removed by THE STORM

#include "bot.h"
#include "bot_weapons.h"
#include "bot_globals_wp.h"
#include "waypoint.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <com_model.h>
#include <pm_defs.h>

#ifdef _WIN32
#ifndef FORCEINLINE
#define  FORCEINLINE         __forceinline
#endif
#else
#define  FORCEINLINE         inline
#endif



#define SVC_PINGS		17 // KWo - 02.03.2010 (missing in HLSDK!)

inline void UTIL_ClampAngle (float *fAngle)
{
   // Whistler, TEST your bugfixes before submitting them!!! :D
   if (*fAngle >= 180)
      *fAngle -= 360 * ((int) (*fAngle / 360) + 1); // and not 0.5
   if (*fAngle < -180)
      *fAngle += 360 * ((int) (-*fAngle / 360) + 1); // and not 0.5

   if ((*fAngle >= 180) || (*fAngle < -180))
      *fAngle = 0; // heck, if we're still above the limit then something's REALLY fuckedup!
   return;
}
/*
inline void UTIL_ClampAngle (float *fAngle) // KWo - 17.02.2008 (inline again)
{
   // this function adds or substracts 360 enough times needed to the given angle in
   // order to set it into the range [-180, 180) and returns the resulting angle. letting
   // the engine have a hand on angles that are outside these bounds may cause the game
   // to freeze by screwing up the engine math code.

   *fAngle = (360.0 / 65536.0) * (static_cast <int> ((*fAngle + 180.0) * (65536.0 / 360.0)) & 65535) - 180.0;
   if (*fAngle == -180.0)
      *fAngle = 180.0;
   return;
}
*/
/*
inline void UTIL_ClampVector (Vector *vecAngles) // KWo - 17.02.2008 (inline again)
{
   vecAngles->x = (180.0 / 65536.0) * (static_cast <int> ((vecAngles->x + 90.0) * (65536.0 / 180.0)) & 65535) - 90.0;
   vecAngles->y = (360.0 / 65536.0) * (static_cast <int> ((vecAngles->y + 180.0) * (65536.0 / 360.0)) & 65535) - 180.0;
   if (vecAngles->x > 89.0)
      vecAngles->x = 89.0;
   else if (vecAngles->x < -89.0)
      vecAngles->x = -89.0;
   if (vecAngles->y > 180.0)
      vecAngles->y = 180.0;
   else if (vecAngles->y == -180.0)
      vecAngles->y = 180.0;
   else if (vecAngles->y < -179.0)
      vecAngles->y = -179.0;
   vecAngles->z = 0.0;
   return;
}
*/
inline void UTIL_ClampVector (Vector *vecAngles)
{
   // Whistler, TEST your bugfixes before submitting them!!! :D
   if (vecAngles->x >= 180)
      vecAngles->x -= 360 * ((int) (vecAngles->x / 360) + 1); // and not 0.5
   if (vecAngles->x < -180)
      vecAngles->x += 360 * ((int) (-vecAngles->x / 360) + 1); // and not 0.5
   if (vecAngles->y >= 180)
      vecAngles->y -= 360 * ((int) (vecAngles->y / 360) + 1); // and not 0.5
   if (vecAngles->y < -180)
      vecAngles->y += 360 * ((int) (-vecAngles->y / 360) + 1); // and not 0.5
   vecAngles->z = 0.0;

   if (vecAngles->x > 89)
      vecAngles->x = 89;
   else if (vecAngles->x < -89)
      vecAngles->x = -89;

   if ((vecAngles->x >= 180) || (vecAngles->x < -180))
      vecAngles->x = 0; // heck, if we're still above the limit then something's REALLY fuckedup!
   if ((vecAngles->y >= 180) || (vecAngles->y < -180))
      vecAngles->y = 0; // heck, if we're still above the limit then something's REALLY fuckedup!
   return;
}

extern float g_fTimeNextBombUpdate;
extern bool g_bLeaderChosenT;
extern bool g_bLeaderChosenCT;
extern int iNumBotNames;
extern int iNumKillChats;
extern int iNumBombChats;
extern int iNumDeadChats;
extern int iNumNoKwChats;
extern int g_iPeoBotsKept;
extern int g_iMin_bots;
extern int g_iMax_bots;
extern float botcreation_time;
extern float g_fLastKickedBotTime;
extern bool g_bIgnoreEnemies;
extern float g_fLastChatTime;
extern float g_fTimeRoundStart;
extern float g_fTimeRoundEnd;
extern float g_fTimeRoundMid;
extern bool g_bRoundEnded;                      // KWo - 30.09.2010
extern float g_fTimeNextBombUpdate;
extern int g_iLastBombPoint;
extern bool g_bBombPlanted;
extern float g_fTimeBombPlanted;
extern bool g_bBombDefusing;                    // KWo - 13.07.2007
extern int g_iDefuser;                          // KWo - 13.07.2007
extern bool g_bBombSayString;
extern bool g_bBotsCanPause;
extern bool g_bHostageRescued;
extern int iRadioSelect[32];
extern int g_rgfLastRadio[2];
extern float g_rgfLastRadioTime[2];
extern char g_szWaypointMessage[512];
extern const char *g_szSettingsMessage;         // KWo - 17.05.2008
extern const char *g_szCommandsMessage;         // KWo - 17.05.2008
extern const char *g_szWelcomeMessage;
extern int g_iNumLogos;
extern int state;
extern Vector g_vecBomb;
extern edict_t *pHostEdict;
extern bool g_bIsOldCS15;
extern DLL_FUNCTIONS gFunctionTable;
extern DLL_FUNCTIONS gFunctionTable_Post;
extern enginefuncs_t g_engfuncs;
extern globalvars_t  *gpGlobals;
extern int gmsgFlashlight;                      // KWo - 25.05.2008
extern int gmsgNVGToggle;                       // KWo - 26.05.2008
extern char g_argv[1024];
extern botname_t *g_pszBotNames;
extern char szKillChat[100][256];
extern char szBombChat[100][256];
extern char szDeadChat[100][256];
extern char szNoKwChat[100][256];
extern const char *szUsedBotNames[32];
extern int iUsedDeadChatIndex[10];             // KWo - 23.03.2010
extern int iUsedUnknownChatIndex[5];          // KWo - 27.03.2010
extern replynode_t *pChatReplies;
extern createbot_t BotCreateTab[32];
extern client_t clients[32];
extern bool g_bEditNoclip;
extern int g_pSpriteTexture;
extern bool isFakeClientCommand;
extern int fake_arg_count;
extern int g_iNum_bots;
extern int g_iNum_players;
extern int g_iNum_humans;
extern int g_iNum_hum_tm;                       // KWo - 08.03.2010
extern int g_iNum_hum_tm_prev;                  // KWo - 04.03.2010
extern int g_i_MapBuying;                       // KWo - 07.06.2010
extern bool g_bResetHud;                        // KWo - 04.03.2010
extern bool g_GameRules;
extern char g_cStoreAddbotSkill[4];
extern char g_cStoreAddbotPersonality[2];	      // KWo - 08.01.2006
extern char g_cStoreAddbotTeam[2];	            // KWo - 08.01.2006
extern char g_cStoreFillServerSkill[4];
extern char g_cStoreFillServerPersonality[2];	// KWo - 08.01.2006
extern char g_cStoreFillServerTeam[2];	         // KWo - 08.01.2006
extern FILE *fp;
extern bool file_opened;
extern bot_t bots[32];
extern int msecnum;
extern float msecdel;
extern float msecval;
extern void (*botMsgFunction) (void *, int);
extern int botMsgIndex;
extern Vector g_vecHostOrigin;                  // KWo - 20.04.2013
extern float g_fTimeRestartServer;
extern char g_szGameDirectory[128];
extern bool g_bBotSettings;
extern bool g_bWeaponStrip;                     // KWo - 10.03.2013
extern int g_iNumButtons;                       // KWo - 07.02.2006
extern int g_iNumBreakables;                    // KWo - 07.02.2006
extern int g_iNumHostages;                      // KWo - 16.05.2006
extern int g_iWeaponRestricted[NUM_WEAPONS];    // KWo - 10.03.2006
extern int g_iEquipAmmoRestricted[NUM_EQUIPMENTS];   // KWo - 10.03.2006

extern bool g_bPathWasCalculated;               // KWo - 24.03.2007
extern bool g_bSaveVisTable;                    // KWo - 24.03.2007
extern int g_iAliveCTs;                         // KWo - 19.01.2008
extern int g_iAliveTs;                          // KWo - 19.01.2008
extern saved_buttons_t ButtonsData[MAX_BUTTONS]; // KWo 07.02.2006
extern saved_break_t BreakablesData[MAX_BREAKABLES];  // KWo 10.02.2006
extern saved_hostages_t HostagesData[MAX_HOSTAGES];   // KWo - 16.05.2006
extern hudtextparms_t g_hudset;                 // KWo - 16.01.2010

extern int g_i_cv_FpsMax;                       // KWo - 06.04.2006
extern float g_f_cv_Gravity;                    // KWo - 16.11.2006
extern float g_f_cv_FreezeTime;                 // KWo - 17.11.2006
extern bool g_b_cv_FriendlyFire;                // KWo - 06.04.2006
extern bool g_b_cv_FootSteps;                   // KWo - 06.04.2006
extern float g_f_cv_c4timer;                    // KWo - 17.04.2006
extern bool g_b_cv_csdm_active;                 // KWo - 15.04.2008
extern bool g_b_cv_flash_allowed;               // KWo - 25.05.2008
extern float g_f_cv_skycolor;                   // KWo - 26.05.2008
extern bool g_b_cv_Parachute;                   // KWo - 07.03.2010

extern int g_i_cv_MinBots;                      // KWo - 06.04.2006
extern int g_i_cv_MaxBots;                      // KWo - 06.04.2006
extern int g_i_cv_BotsQuotaMatch;               // KWo - 16.09.2006
extern int g_i_cv_BotsJoinTeam;                 // KWo - 16.09.2006
extern float g_f_cv_MapStartBotJoinDelay;       // KWo - 17.05.2008
extern float g_f_cv_timer_sound;                // KWo - 06.04.2006
extern float g_f_cv_timer_pickup;               // KWo - 06.04.2006
extern float g_f_cv_timer_grenade;              // KWo - 06.04.2006
extern int g_i_cv_skin;                         // KWo - 18.11.2006
extern bool g_b_cv_spray;                       // KWo - 06.04.2006
extern bool g_b_cv_shootthruwalls;              // KWo - 06.04.2006
extern int g_i_cv_debuglevel;                   // KWo - 20.04.2013
extern int g_i_cv_detailnames;                  // KWo - 22.03.2008
extern bool g_b_cv_UseSpeech;                   // KWo - 07.10.2006
extern int g_i_cv_numfollowuser;                // KWo - 06.04.2006
extern int g_i_cv_maxweaponpickup;              // KWo - 06.04.2006
extern bool g_b_cv_jasonmode;                   // KWo - 06.04.2006
extern float g_f_cv_dangerfactor;               // KWo - 06.04.2006
extern bool g_b_cv_chat;                        // KWo - 06.04.2006
extern int g_i_cv_latencybot;                   // KWo - 02.03.2010
extern bool g_b_cv_autokill;                    // KWo - 02.05.2006
extern float g_f_cv_autokilldelay;              // KWo - 02.05.2006
extern float g_f_cv_maxcamptime;                // KWo - 23.03.2008
extern bool g_b_cv_ffa;                         // KWo - 04.10.2006
extern bool g_b_cv_firsthumanrestart;           // KWo - 04.10.2010
extern bool g_b_cv_radio;                       // KWo - 03.02.2007
extern int g_i_cv_aim_type;                     // KWo - 06.04.2006
extern float g_f_cv_aim_spring_stiffness_x;     // KWo - 06.04.2006
extern float g_f_cv_aim_spring_stiffness_y;     // KWo - 06.04.2006
extern float g_f_cv_aim_damper_coefficient_x;   // KWo - 06.04.2006
extern float g_f_cv_aim_damper_coefficient_y;   // KWo - 06.04.2006
extern float g_f_cv_aim_deviation_x;            // KWo - 06.04.2006
extern float g_f_cv_aim_deviation_y;            // KWo - 06.04.2006
extern float g_f_cv_aim_influence_x_on_y;       // KWo - 06.04.2006
extern float g_f_cv_aim_influence_y_on_x;       // KWo - 06.04.2006
extern float g_f_cv_aim_offset_delay;           // KWo - 06.04.2006
extern float g_f_cv_aim_notarget_slowdown_ratio;   // KWo - 06.04.2006
extern float g_f_cv_aim_target_anticipation_ratio; // KWo - 06.04.2006
extern const char *g_sz_cv_PasswordField;       // KWo - 06.04.2006
extern const char *g_sz_cv_Password;            // KWo - 06.04.2006
extern const char *g_sz_cv_WPT_Folder;          // KWo - 17.11.2006

extern float g_f_cvars_upd_time;                // KWo - 02.05.2006
extern float g_f_host_upd_time;                 // KWo - 18.05.2006
extern float g_fTimeAvoidGrenade;               // KWo - 29.01.2008

extern float g_fAutoKillTime;                   // KWo - 02.05.2006
extern bool g_bAliveHumansPrevious;             // KWo - 02.05.2006
extern int g_iUpdGlExpState;                    // KWo - 02.05.2006
extern bool g_bRecalcKills;                     // KWo - 02.05.2006
extern bool g_bDeathMessageSent;                // KWo - 02.03.2010

extern const char *g_TaskNames[19];             // KWo - 06.01.2008
extern const char *g_ItemTypeNames[7];          // KWo - 28.08.2008
extern const char *g_CollideMoveNames[8];       // KWo - 27.03.2010
extern const char *g_rgpszPbCmds[NUM_PBCOMMANDS + 1];
extern const char *g_rgpszPbCvars[NUM_PBCVARS + 1];
extern cvar_t *g_p_cv_csdm_active;              // KWo - 15.04.2008
extern cvar_t *g_rgcvarPointer[NUM_PBCVARS];    // KWo - 13.10.2006
extern bot_weapon_select_t cs_weapon_select[NUM_WEAPONS + 1];
extern bot_fire_delay_t cs_fire_delay[NUM_WEAPONS + 1];
extern botaim_t BotAimTab[6];
extern bot_weapon_t weapon_defs[MAX_WEAPONS];
extern int weapon_selectIndex[MAX_WEAPONS];     // KWo - 05.04.2010
extern int weapon_maxClip[MAX_WEAPONS];         // KWo - 10.01.2007
extern skilldelay_t BotSkillDelays[6];
extern turnspeed_t BotTurnSpeeds[6];
extern bottask_t taskFilters[];
extern int NormalWeaponPrefs[NUM_WEAPONS];
extern int AgressiveWeaponPrefs[NUM_WEAPONS];
extern int DefensiveWeaponPrefs[NUM_WEAPONS];
extern int *ptrWeaponPrefs[];
extern char szSprayNames[NUM_SPRAYPAINTS][20];
extern const char szSpeechSentences[16][80];

extern menutext_t menuPODBotMain;
extern menutext_t menuPODBotAddBotSkill;
extern menutext_t menuPODBotAddBotPersonality;
extern menutext_t menuPODBotAddBotTeam;
extern menutext_t menuPODBotAddBotTModel;
extern menutext_t menuPODBotAddBotCTModel;
extern menutext_t menuPODBotFillServerSkill;
extern menutext_t menuPODBotFillServerPersonality;
extern menutext_t menuPODBotFillServerTeam;
extern menutext_t menuPODBotFillServerTModel;
extern menutext_t menuPODBotFillServerCTModel;
extern menutext_t menuPODBotKickBot1;
extern menutext_t menuPODBotKickBot2;
extern menutext_t menuPODBotKickBot3;
extern menutext_t menuPODBotKickBot4;
extern menutext_t menuPODBotWeaponMode;
extern menutext_t menuWpMain;
extern menutext_t menuWpAdd;
extern menutext_t menuWpDelete;
extern menutext_t menuWpSetRadius;
extern menutext_t menuWpSetFlags;
extern menutext_t menuWpSetTeam;
extern menutext_t menuWpAddPath;
extern menutext_t menuWpDeletePath;
extern menutext_t menuWpSave;
extern menutext_t menuWpOptions1;               // KWo - 29.03.2008
extern menutext_t menuWpOptions2;               // KWo - 29.03.2008
extern menutext_t menuWpAutoPathMaxDistance;

// KWo - 13.03.2012 - Thanks to Immortal_BLG for light code

#define   SURF_PLANEBACK      2
#define   SURF_DRAWSKY        4
#define   SURF_DRAWSPRITE     8
#define   SURF_DRAWTURB       0x10
#define   SURF_DRAWTILED      0x20
#define   SURF_DRAWBACKGROUND 0x40

#ifdef __linux__
// Linux doesn't have this function so this emulates its functionality
inline void *GetModuleHandle(const char *name)
{
   void *handle;

   if( name == NULL )
   {
      // hmm, how can this be handled under linux....
      // is it even needed?
      return NULL;
   }

    if( (handle=dlopen(name, RTLD_NOW))==NULL)
    {
            // couldn't open this file
            return NULL;
    }

   // read "man dlopen" for details
   // in short dlopen() inc a ref count
   // so dec the ref count by performing the close
   dlclose(handle);
   return handle;
}
#endif

struct GL_msurface_t
{
#define SURF_UNDERWATER      0x80   // ONLY FOR OpenGL!!!
//#define SURF_DONTWARP      0x100   // ONLY FOR OpenGL!!! (EXISTS?!?!?!?!?!??!?!?!?!?!?!??!)

/*off=0(0)*/   int         visframe;      // should be drawn when node is crossed
/*off=4(1)*/   mplane_t   *plane;         // pointer to shared plane
/*off=8(2)*/   int         flags;         // see SURF_* #defines

/*off=12(3)*/   int         firstedge;   // look up in model->surfedges[], negative numbers are backwards edges
/*off=16(4)*/   int         numedges;

/*off=20(5)*/   short      texturemins[2]; // smallest s/t position on the surface.
/*off=24(6)*/   short      extents[2];      // ?? s/t texture size, 1..256 for all non-sky surfaces

/*off=28(7)*/   int         light_s, light_t;   // gl lightmap coordinates
/*off=36(9)*/   struct glpoly_t   *polys;            // multiple if warped
/*off=40(10)*/   msurface_t   *texturechain;
/*off=44(11)*/   mtexinfo_t   *texinfo;

// lighting info
/*off=48(12)*/   int         dlightframe;   // last frame the surface was checked by an animated light
/*off=52(13)*/   int         dlightbits;      // dynamically generated. Indicates if the surface illumination
                        // is modified by an animated light.
/*off=56(14)*/   int         lightmaptexturenum;
/*off=60(15)*/   unsigned char      styles[MAXLIGHTMAPS]; // index into d_lightstylevalue[] for animated lights
                             // no one surface can be effected by more than 4
                             // animated lights.

/*off=64(16)*/   int         cached_light[MAXLIGHTMAPS];   // values currently used in lightmap
/*off=80(20)*/   qboolean   cached_dlight;            // true if dynamic light in cache

/*off=84(21)*/   color24      *samples;   // [numstyles*surfsize]

/*off=88(22)*/   decal_t      *pdecals;
};   // sizeof (GL_msurface_t) == 92 (23)
struct GL_mnode_t
{
   enum ChildrenType_t
   {
      ChildrenType_Front,
      ChildrenType_Back,

      ChildrenType_Total
   };

// common with leaf
/*! Off=0(0)*/   int         contents;      // 0, to differentiate from leafs
/*! Off=4(1)*/   int         visframe;      // node needs to be traversed if current

/*! Off=8(2)*/   Vector      mins, maxs;      // for bounding box culling

/*! Off=32(8)*/   mnode_t   *parent;

// node specific
/*! Off=36(9)*/   mplane_t   *plane;
/*! Off=40(10)*/   mnode_t   *children[ChildrenType_Total];

/*! Off=48(12)*/   unsigned short      firstsurface;
/*! Off=50(12.5)*/   unsigned short      numsurfaces;
};   // sizeof (GL_mnode_t) == 52 (13)

#define   MAX_LIGHTSTYLES   64
#define   MAX_LIGHTSTYLEVALUE   256

struct lightstyle_t
{
   int  length;
   char map[MAX_LIGHTSTYLES];
};

extern lightstyle_t cl_lightstyle[MAX_LIGHTSTYLES];
extern int          d_lightstylevalue[MAX_LIGHTSTYLEVALUE];   // 8.8 fraction of base light value
extern model_t     *sv_worldmodel;   // Analog of sv.worldmodel.

struct Color
{
   int red;
   int green;
   int blue;

   inline       void         Reset  (void)       { red = green = blue = 0; }
   inline const unsigned int GetAvg (void) const { return (red + green + blue) / (sizeof (Color) / sizeof (int)); }
};

/*
=============================================================================

LIGHT SAMPLING

=============================================================================
*/


namespace Light
{
//extern const mplane_t *lightplane (NULL);
//extern Vector lightspot;
   extern Color g_pointColor;

   template <typename nodeType, typename surfaceType> extern const  bool RecursiveLightPoint (const nodeType *const node, const Vector  &start, const Vector &end)
   {
      float front, back, frac;
      int side;
      mplane_t *plane;
      Vector mid;
      surfaceType *surf;
      int s, t, ds, dt;
      int i;
      mtexinfo_t *tex;
      color24 *lightmap;
      unsigned int scale;
      unsigned char maps;

      // Reliability check.
      assert (node != NULL);

      if (node->contents < 0)
         return false;   // didn't hit anything

      // Determine which side of the node plane our points are on
      // FIXME: optimize for axial
      plane = node->plane;
      front = DotProduct (start, plane->normal) - plane->dist;
      back = DotProduct (end, plane->normal) - plane->dist;
      side = front < 0.0f;

      // If they're both on the same side of the plane, don't bother to split just check the appropriate child
      if ((back < 0.0f) == side)
         return RecursiveLightPoint <nodeType, surfaceType>  (reinterpret_cast <nodeType *> (node->children[side]), start,  end);

      // calculate mid point
      frac = front / (front - back);
      mid = start + (end - start) * frac;

      // go down front side
      if (RecursiveLightPoint <nodeType, surfaceType>  (reinterpret_cast <nodeType *> (node->children[side]), start,  mid))
         return true;   // hit something

      // Blow it off if it doesn't split the plane...
      if ((back < 0.0f) == side)
         return false;   // didn't hit anything

      // check for impact on this node
   //   lightspot = mid;
   //   lightplane = plane;

      surf = reinterpret_cast <surfaceType *> (sv_worldmodel->surfaces) + node->firstsurface;
      for (i = 0; i < node->numsurfaces; ++i, ++surf)
      {
         if (surf->flags & SURF_DRAWTILED)
            continue;   // no lightmaps

         tex = surf->texinfo;

         // See where in lightmap space our intersection point is
         s = static_cast <int> (DotProduct (mid, Vector (tex->vecs[0])) + tex->vecs[0][3]);
         t = static_cast <int> (DotProduct (mid, Vector (tex->vecs[1])) + tex->vecs[1][3]);

         // Not in the bounds of our lightmap? punt...
         if (s < surf->texturemins[0] || t < surf->texturemins[1])
            continue;

         // assuming a square lightmap (FIXME: which ain't always the case),
         // lets see if it lies in that rectangle. If not, punt...
         ds = s - surf->texturemins[0];
         dt = t - surf->texturemins[1];

         if (ds > surf->extents[0] || dt > surf->extents[1])
            continue;

         if (surf->samples == NULL)
            return true;

         ds >>= 4;
         dt >>= 4;

         g_pointColor.Reset ();   // Reset point color.

         const int smax ((surf->extents[0] >> 4) + 1);
         const int tmax ((surf->extents[1] >> 4) + 1);
         const int size (smax * tmax);

         lightmap = surf->samples + dt * smax + ds;

         // Compute the lightmap color at a particular point
         for (maps = 0u; maps < MAXLIGHTMAPS && surf->styles[maps] != 255u; ++maps)
         {
            scale = d_lightstylevalue[surf->styles[maps]];

            g_pointColor.red += lightmap->r * scale;
            g_pointColor.green += lightmap->g * scale;
            g_pointColor.blue += lightmap->b * scale;

            lightmap += size;   // skip to next lightmap
         }

         g_pointColor.red >>= 8u;
         g_pointColor.green >>= 8u;
         g_pointColor.blue >>= 8u;

         return true;
      }

      // go down back side
      return RecursiveLightPoint <nodeType, surfaceType>  (reinterpret_cast <nodeType *> (node->children[!side]), mid,  end);
   }

   inline const bool IsSoftwareDrawingMode (void)
   {
      static const bool isSoftwareDrawingMode (IS_DEDICATED_SERVER () || GetModuleHandle ("sw.dll") != NULL);

      return isSoftwareDrawingMode;
   }

   inline const bool ActualRecursiveLightPoint (const Vector &start, const Vector &end)
   {
      return IsSoftwareDrawingMode () ?
         RecursiveLightPoint <mnode_t, msurface_t> (sv_worldmodel->nodes, start, end) :
         RecursiveLightPoint <GL_mnode_t, GL_msurface_t>  (reinterpret_cast <GL_mnode_t *> (sv_worldmodel->nodes), start,  end);
   }

   inline const unsigned char R_LightPoint (const Vector &p)
   {
   // Reliability check.
      if (sv_worldmodel == NULL)
         return 0u;

      if (sv_worldmodel->lightdata == NULL)
         return 255u;

      Vector end (p);

      end.z -= 2048.0f;

      return ActualRecursiveLightPoint (p, end) == false ? 0u : static_cast <unsigned char> (g_pointColor.GetAvg ());
   }
}

#endif
