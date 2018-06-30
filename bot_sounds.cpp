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
// bot_sounds.cpp
//
// Hooks to let Bots 'hear' otherwise unnoticed things. Base code & idea from killaruna/ParaBot

#include "bot_globals.h"


void SoundAttachToThreat (edict_t *pEdict, const char *pszSample, float fVolume)
{
   // Called by the Sound Hooking Code (in EMIT_SOUND)
   // Enters the played Sound into the Array associated with the Entity

   Vector vecPosition;
   int iIndex;

   if (FNullEnt (pEdict))
      return;

   // Hit/Fall Sound ?
   if ((strncmp ("player/bhit_", pszSample, 12) == 0)
       || (strncmp ("player/headshot", pszSample, 15) == 0))
   {
      iIndex = ENTINDEX (pEdict) - 1;

      // crash fix courtesy of Wei Mingzhi
      if ((iIndex < 0) || (iIndex >= gpGlobals->maxClients))
         iIndex = UTIL_GetNearestPlayerIndex (VecBModelOrigin (pEdict));

      clients[iIndex].fHearingDistance = 768.0 * fVolume;
      clients[iIndex].fTimeSoundLasting = gpGlobals->time + 1.2;
      clients[iIndex].fMaxTimeSoundLasting = 1.2; // KWo - 01.08.2006
      clients[iIndex].vecSoundPosition = pEdict->v.origin;
   }

   // Weapon Pickup ?
   else if (strncmp ("items/gunpickup", pszSample, 15) == 0)
   {
      iIndex = ENTINDEX (pEdict) - 1;

      // crash fix courtesy of Wei Mingzhi
      if ((iIndex < 0) || (iIndex >= gpGlobals->maxClients))
         iIndex = UTIL_GetNearestPlayerIndex (VecBModelOrigin (pEdict));

      clients[iIndex].fHearingDistance = 768.0 * fVolume;
      clients[iIndex].fTimeSoundLasting = gpGlobals->time + 1.2;
      clients[iIndex].fMaxTimeSoundLasting = 1.2; // KWo - 01.08.2006
      clients[iIndex].vecSoundPosition = pEdict->v.origin;
   }

   // Sniper zooming ?
   else if (strncmp ("weapons/zoom", pszSample, 12) == 0)
   {
      iIndex = ENTINDEX (pEdict) - 1;

      // crash fix courtesy of Wei Mingzhi
      if ((iIndex < 0) || (iIndex >= gpGlobals->maxClients))
         iIndex = UTIL_GetNearestPlayerIndex (VecBModelOrigin (pEdict));

      clients[iIndex].fHearingDistance = 512.0 * fVolume;
      clients[iIndex].fTimeSoundLasting = gpGlobals->time + 1.2;
      clients[iIndex].fMaxTimeSoundLasting = 1.2; // KWo - 01.08.2006
      clients[iIndex].vecSoundPosition = pEdict->v.origin;
   }

   // Reload ?
#if 0
 // this doesn't work since the reload sound is played client-side - from Whistler
   else if (strncmp ("weapons/reload", pszSample, 14) == 0)
   {
      iIndex = ENTINDEX (pEdict) - 1;

      // crash fix courtesy of Wei Mingzhi
      if ((iIndex < 0) || (iIndex >= gpGlobals->maxClients))
         iIndex = UTIL_GetNearestPlayerIndex (VecBModelOrigin (pEdict));

      clients[iIndex].fReloadingTime = gpGlobals->time + 1.5;
//      ALERT(at_logged,"[DEBUG] client %s is realoading the weapon...\n", STRING(clients[iIndex].pEdict->v.netname));
   }
#endif

   // The following Sounds don't have the Player Entity associated
   // so we need to search the nearest Player

   // Ammo Pickup ?
   else if (strncmp ("items/9mmclip", pszSample, 13) == 0)
   {
      vecPosition = pEdict->v.origin;
      iIndex = UTIL_GetNearestPlayerIndex (vecPosition);
      clients[iIndex].fHearingDistance = 512.0 * fVolume;
      clients[iIndex].fTimeSoundLasting = gpGlobals->time + 1.2;
      clients[iIndex].fMaxTimeSoundLasting = 1.2; // KWo - 01.08.2006
      clients[iIndex].vecSoundPosition = pEdict->v.origin;
   }

   // CT used Hostage ?
   else if (strncmp ("hostage/hos", pszSample, 11) == 0)
   {
      vecPosition = VecBModelOrigin (pEdict);
      iIndex = UTIL_GetNearestPlayerIndex (vecPosition);
      clients[iIndex].fHearingDistance = 1024.0 * fVolume;
      clients[iIndex].fTimeSoundLasting = gpGlobals->time + 1.2;
      clients[iIndex].fMaxTimeSoundLasting = 1.2; // KWo - 01.08.2006
      clients[iIndex].vecSoundPosition = vecPosition;
   }

   // Broke something ?
   else if ((strncmp ("debris/bustmetal", pszSample, 16) == 0)
            || (strncmp ("debris/bustglass", pszSample, 16) == 0))
   {
      vecPosition = VecBModelOrigin (pEdict);
      iIndex = UTIL_GetNearestPlayerIndex (vecPosition);
      clients[iIndex].fHearingDistance = 1024.0 * fVolume;
      clients[iIndex].fTimeSoundLasting = gpGlobals->time + 1.2;
      clients[iIndex].fMaxTimeSoundLasting = 1.2; // KWo - 01.08.2006
      clients[iIndex].vecSoundPosition = vecPosition;
   }

   // Someone opened a door
   else if (strncmp ("doors/doormove", pszSample, 14) == 0)
   {
      vecPosition = VecBModelOrigin (pEdict);
      iIndex = UTIL_GetNearestPlayerIndex (vecPosition);
      clients[iIndex].fHearingDistance = 1024.0 * fVolume;
      clients[iIndex].fTimeSoundLasting = gpGlobals->time + 3.0;
      clients[iIndex].fMaxTimeSoundLasting = 3.0; // KWo - 01.08.2006
      clients[iIndex].vecSoundPosition = vecPosition;
   }
   return;
}


void SoundSimulateUpdate (int iPlayerIndex)
{
   // Tries to simulate playing of Sounds to let the Bots hear
   // sounds which aren't captured through Server Sound hooking

   assert(iPlayerIndex >= 0);  // KWo - from Whistler
   assert(iPlayerIndex < gpGlobals->maxClients);
   if (iPlayerIndex < 0 || iPlayerIndex >= gpGlobals->maxClients)
      return; // reliability check

   edict_t *pPlayer = clients[iPlayerIndex].pEdict;
   float fVelocity = pPlayer->v.velocity.Length2D ();
   float fHearDistance = 0.0;
   float fTimeSound = 0.0;
   float fMaxTimeSound = 0.5; // KWo - 01.08.2006
   float f_last_volume; // KWo - 01.08.2006
   float f_volume; // KWo - 01.08.2006

   // Pressed Attack Button ?
   if (pPlayer->v.oldbuttons & IN_ATTACK)
   {
      fHearDistance = 3072; // 2048.0;  // KWo 28.10.2006
      fTimeSound = gpGlobals->time + 1.2;
      fMaxTimeSound = 1.2; // KWo - 01.08.2006
   }

   // Pressed Use Button ?
   else if (pPlayer->v.oldbuttons & IN_USE)
   {
      fHearDistance = 512.0;
      fTimeSound = gpGlobals->time + 1.2;
      fMaxTimeSound = 1.2; // KWo - 01.08.2006
   }

   // Uses Ladder ?
   else if (pPlayer->v.movetype == MOVETYPE_FLY)
   {
   		if (fabs(pPlayer->v.velocity.z) > 50)
      {
         fHearDistance = 1024.0;
         fTimeSound = gpGlobals->time + 1.2;
         fMaxTimeSound = 1.2; // KWo - 01.08.2006
      }
   }

   // Moves fast enough ?
   else
   {
      if (g_b_cv_FootSteps)
      {	// KWo - 06.04.2006
         fHearDistance = 1280.0 * (fVelocity / 240.0);	// KWo - 09.10.2006
         fTimeSound = gpGlobals->time + 1.2;
         fMaxTimeSound = 1.2; // KWo - 01.08.2006
      }
   }

   // Did issue Sound ?
   if (fHearDistance > 0.0)
   {
      // Some sound already associated ?
      if (clients[iPlayerIndex].fTimeSoundLasting > gpGlobals->time)
      {
         // New Sound louder (bigger range) than old sound ?
         if (clients[iPlayerIndex].fMaxTimeSoundLasting <= 0.0) // KWo - 01.08.2006
            clients[iPlayerIndex].fMaxTimeSoundLasting = 1.2;
         // the volume is lowered down when the time of lasting sound is expiring... - KWo
         // f_last_volume = clients[iPlayerIndex].fHearingDistance * (clients[iPlayerIndex].fTimeSoundLasting - gpGlobals->time)/clients[iPlayerIndex].fMaxTimeSoundLasting; // KWo - 01.08.2006
         f_volume = fHearDistance; // KWo - 01.08.2006
         f_last_volume = clients[iPlayerIndex].fHearingDistance; // KWo - 29.12.2009 - reverting back some function...
         if (f_last_volume <= f_volume) // KWo - 01.08.2006
         {
            // Override it with new
            clients[iPlayerIndex].fHearingDistance = fHearDistance;
            clients[iPlayerIndex].fTimeSoundLasting = fTimeSound;
            clients[iPlayerIndex].fMaxTimeSoundLasting = fMaxTimeSound; // KWo - 01.08.2006
            clients[iPlayerIndex].vecSoundPosition = pPlayer->v.origin;
         }
      }
      // New sound ?
      else
      {
         // Just remember it
         clients[iPlayerIndex].fHearingDistance = fHearDistance;
         clients[iPlayerIndex].fTimeSoundLasting = fTimeSound;
         clients[iPlayerIndex].fMaxTimeSoundLasting = fMaxTimeSound; // KWo - 01.08.2006
         clients[iPlayerIndex].vecSoundPosition = pPlayer->v.origin;
      }
   }
   return;
}
