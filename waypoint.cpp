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
// bot_navigate.cpp
//
// Features the Waypoint Code (Editor)

#include "bot_globals.h"
#include <vector>

// adds a path between 2 waypoints
void WaypointAddPath (short int add_index, short int path_index, float fDistance)
{
   PATH *p;
   int i;

   g_bWaypointsChanged = TRUE;

   p = paths[add_index];

   // Don't allow Paths get connected to the same Waypoint
   if (add_index == path_index)
   {
      UTIL_ServerPrint ("Denied path creation from %d to %d (same waypoint)\n", add_index, path_index);
      return;
   }

   // Don't allow Paths get connected twice
   for (i = 0; i < MAX_PATH_INDEX; i++)
      if (p->index[i] == path_index)
      {
         UTIL_ServerPrint ("Denied path creation from %d to %d (path already exists)\n", add_index, path_index);
         return;
      }

   // Check for free space in the Connection indices
   for (i = 0; i < MAX_PATH_INDEX; i++)
   {
      if (p->index[i] == -1)
      {
         p->index[i] = path_index;
         p->distance[i] = (int) fabs (fDistance);	// KWo - to remove warning

         UTIL_ServerPrint ("Path added from %d to %d\n", add_index, path_index);
         return;
      }
   }

   // There wasn't any free space. Try exchanging it with a long-distance path
   for (i = 0; i < MAX_PATH_INDEX; i++)
   {
      if (p->distance[i] > fabs (fDistance))
      {
         UTIL_ServerPrint ("Path added from %d to %d\n", add_index, path_index);

         p->index[i] = path_index;
         p->distance[i] = (int) fabs (fDistance);	// KWo - to remove warning
         return;
      }
   }

   return;
}


// find the nearest waypoint to the player and return the index (-1 if not found)
int WaypointFindNearest (void)
{
   int i;
   int index = -1;  // KWo - to remove warning uninitialised
   float distance;
   float min_distance;

   // find the nearest waypoint...
   min_distance = 9999.0;

   for (i = 0; i < g_iNumWaypoints; i++)
   {
      distance = (paths[i]->origin - pHostEdict->v.origin).Length ();

      if (distance < min_distance)
      {
         index = i;
         min_distance = distance;
      }
   }

   // if not close enough to a waypoint then just return
   if (min_distance > 50)
      return (-1);

   return (index);
}

// Shows waypoints under WP editor
void WaypointDrawBeam (Vector start, Vector end, int width, int red, int green, int blue)
{
   if (FNullEnt (pHostEdict))
      return;

   MESSAGE_BEGIN (MSG_ONE_UNRELIABLE, SVC_TEMPENTITY, NULL, pHostEdict);
   WRITE_BYTE (TE_BEAMPOINTS);
   WRITE_COORD (start.x);
   WRITE_COORD (start.y);
   WRITE_COORD (start.z);
   WRITE_COORD (end.x);
   WRITE_COORD (end.y);
   WRITE_COORD (end.z);
   WRITE_SHORT (g_pSpriteTexture);
   WRITE_BYTE (1); // framestart
   WRITE_BYTE (10); // framerate
   WRITE_BYTE (3); // life in 0.1's KWo - 28.02.2008
   WRITE_BYTE (width); // width
   WRITE_BYTE (0); // noise
   WRITE_BYTE (red); // r, g, b
   WRITE_BYTE (green); // r, g, b
   WRITE_BYTE (blue); // r, g, b
   WRITE_BYTE (255); // brightness
   WRITE_BYTE (0); // speed
   MESSAGE_END ();

   return;
}

// Checks if the node is reachable (for "waypoint add" function)
bool WaypointNodeReachable (int i_src, int i_dest) // KWo - 06.01.2008
{
   TraceResult tr;
   float height, last_height;
   Vector v_dest, v_src;
   IGNORE_MONSTERS igm; // KWo - 13.01.2008

   if ((i_src < 0) || (i_src > g_iNumWaypoints) || (i_dest < 0) || (i_dest > g_iNumWaypoints))
      return (FALSE);

   v_dest = paths[i_dest]->origin; // KWo - 06.01.2008
   v_src = paths[i_src]->origin; // KWo - 06.01.2008

   if (paths[i_src]->flags & W_FL_LADDER) // KWo - 06.01.2008
      v_src = v_src + Vector(0.0, 0.0, 16.0);

   float distance = (v_dest - v_src).Length ();

   // is the destination NOT close enough?
   if (g_bWaypointOn) // 13.01.2008
   {
      if ((distance > g_fAutoPathMaxDistance) || ((g_bAutoWaypoint) && (distance > 160.0))) // KWo - 09.11.2007
         return (FALSE);
   }

   igm = g_bWaypointOn ? ignore_monsters : dont_ignore_monsters; // KWo - 13.01.2008
   // check if we go through a func_illusionary, in which case return FALSE
   TRACE_HULL (v_src, v_dest, igm, head_hull, pHostEdict, &tr); // KWo - 13.01.2008
   if (!FNullEnt (tr.pHit))
   {
      if (strcmp ("func_illusionary", STRING (tr.pHit->v.classname)) == 0)
         return (FALSE); // don't add pathwaypoints through func_illusionaries
   }

   // check if this waypoint is "visible"...
   TRACE_LINE (v_src, v_dest, igm, pHostEdict, &tr);  // KWo - 13.01.2008

   // if waypoint is visible from current position (even behind head)...
   if ((tr.flFraction >= 1.0) || (strncmp ("func_door", STRING (tr.pHit->v.classname), 9) == 0))
   {
      // If it's a door check if nothing blocks behind
      if (strncmp ("func_door", STRING (tr.pHit->v.classname), 9) == 0)
      {
         Vector vDoorEnd = tr.vecEndPos;

         TRACE_LINE (vDoorEnd, v_dest, igm, tr.pHit, &tr);  // KWo - 13.01.2008
         if (tr.flFraction < 1.0)
            return (FALSE);
      }

      // check for special case of both waypoints being in water...
      if ((POINT_CONTENTS (v_src) == CONTENTS_WATER)
          && (POINT_CONTENTS (v_dest) == CONTENTS_WATER))
          return (TRUE); // then they're reachable each other

      // check for special case of waypoint being suspended in mid-air...

      // is dest waypoint higher than src? (45 is max jump height)
      if (v_dest.z > v_src.z + 45.0)
      {
         Vector v_new_src = v_dest;
         Vector v_new_dest = v_dest;

         v_new_dest.z = v_new_dest.z - 50; // straight down 50 units

         TRACE_LINE (v_new_src, v_new_dest, igm, pHostEdict, &tr); // KWo - 13.01.2008

         // check if we didn't hit anything, if not then it's in mid-air
         if (tr.flFraction >= 1.0)
            return (FALSE); // can't reach this one
      }

      // check if distance to ground drops more than step height at points
      // between source and destination...

      Vector v_direction = (v_dest - v_src).Normalize(); // 1 unit long
      Vector v_check = v_src;
      Vector v_down = v_src;

      distance = (v_dest - v_check).Length (); // distance from goal

      if ((paths[i_src]->flags & W_FL_LADDER) && (distance > 32.0)) // KWo - 06.01.2008
      {
         v_check = v_check + (v_direction * 32.0);
         v_down = v_check;
      }

      v_down.z = v_down.z - 1000.0; // straight down 1000 units

      TRACE_LINE (v_check, v_down, igm, pHostEdict, &tr); // KWo - 13.01.2008

      last_height = tr.flFraction * 1000.0; // height from ground

      while (distance > 10.0)
      {
         // move 10 units closer to the goal...
         v_check = v_check + (v_direction * 10.0);

         v_down = v_check;
         v_down.z = v_down.z - 1000.0; // straight down 1000 units

         TRACE_LINE (v_check, v_down, igm, pHostEdict, &tr); // KWo - 13.01.2008

         height = tr.flFraction * 1000.0; // height from ground

         // is the current height greater than the step height?
         if (height < last_height - 18.0)
            return (FALSE); // can't get there without jumping...

         last_height = height;

         distance = (v_dest - v_check).Length (); // distance from goal
      }

      return (TRUE);
   }

   return (FALSE);
}

// Calculates the Waypoint wayzone
void CalculateWaypointWayzone (void)
{
   PATH *p;
   Vector start;
   Vector vRadiusEnd;
   Vector v_direction;
   TraceResult tr;
   int iScanDistance;
   float fRadCircle;
   bool bWayBlocked;
   int index;
   int x;

   index = WaypointFindNearest ();
   p = paths[index];

   if ((p->flags & W_FL_LADDER)
       || (p->flags & W_FL_GOAL)
       || (p->flags & W_FL_CAMP)
       || (p->flags & W_FL_RESCUE)
       || (p->flags & W_FL_CROUCH)
       || g_bLearnJumpWaypoint)
   {
      p->Radius = 0;
      return;
   }

   for (x = 0; x < MAX_PATH_INDEX; x++)
   {
      if ((p->index[x] != -1) && (paths[p->index[x]]->flags & W_FL_LADDER))
      {
         p->Radius = 0;
         return;
      }
   }

   bWayBlocked = FALSE;

   for (iScanDistance = 32; iScanDistance < 128; iScanDistance += 16)
   {
      start = p->origin;
      MAKE_VECTORS (g_vecZero);
      vRadiusEnd = start + (gpGlobals->v_forward * iScanDistance);
      v_direction = vRadiusEnd - start;
      v_direction = UTIL_VecToAngles (v_direction);
      p->Radius = iScanDistance;

      for (fRadCircle = 0.0; fRadCircle < 360.0; fRadCircle += 20)
      {
         MAKE_VECTORS (v_direction);
         vRadiusEnd = start + (gpGlobals->v_forward * iScanDistance);
         TRACE_LINE (start, vRadiusEnd, ignore_monsters, pHostEdict, &tr);

         if (tr.flFraction < 1.0)
         {
            if (strncmp ("func_door", STRING (tr.pHit->v.classname), 9) == 0)
            {
               p->Radius = 0;
               bWayBlocked = TRUE;
               break;
            }

            bWayBlocked = TRUE;
            p->Radius -= 16;
            break;
         }

         vRadiusEnd.z += 34;
         TRACE_LINE (start, vRadiusEnd, ignore_monsters, pHostEdict, &tr);

         if (tr.flFraction < 1.0)
         {
            bWayBlocked = TRUE;
            p->Radius -= 16;
            break;
         }

         v_direction.y += fRadCircle;
         UTIL_ClampAngle (&v_direction.y);
      }

      if (bWayBlocked)
         break;
   }

   p->Radius -= 16;
   if (p->Radius < 0)
      p->Radius = 0;

   return;
}

// adds a new waypoint
void WaypointAdd (int wpt_type)
{
   int index, i, j, k;
   float distance;
   float min_distance;
   float fLongestDist = 0.0;
   PATH *pPath, *pPrev; // KWo - 06.01.2008 changed names...
   bool bPlaceNew = TRUE;
   Vector vecNewWaypoint;
   TraceResult tr;
   int iDestIndex;
   int flags;
   int TempInd[MAX_PATH_INDEX];
   int TempDistance[MAX_PATH_INDEX];
   float TempAngleY[MAX_PATH_INDEX];
   int Temp_ind;
   int Temp_dist;
   float Temp_ang;
   Vector Temp_vector_origin; // KWo - 06.01.2008
   Vector vec_dir_ang;
   int iPathIndexOfLongest = -1;
   bool bLongestCalculated = FALSE;
   bool bSorting;

   g_bWaypointsChanged = TRUE;
   vecNewWaypoint = pHostEdict->v.origin;
   index = WaypointFindNearest ();
   pPath = NULL; // KWo - 11.04.2010

   if (wpt_type == WAYPOINT_ADD_CAMP_START)
   {
      if (index != -1)
      {
         pPath = paths[index];

         if (pPath->flags & W_FL_CAMP)
         {
            pPath->fcampstartx = pHostEdict->v.v_angle.x;
            pPath->fcampstarty = pHostEdict->v.v_angle.y;
            EMIT_SOUND_DYN2 (pHostEdict, CHAN_WEAPON, "common/wpn_hudon.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
            return;
         }
      }
   }

   else if (wpt_type == WAYPOINT_ADD_CAMP_END)
   {
      if (index != -1)
      {
         pPath = paths[index];

         if (!(pPath->flags & W_FL_CAMP))
         {
            UTIL_ServerPrint ("This is no Camping Waypoint !\n");
            return;
         }

         pPath->fcampendx = pHostEdict->v.v_angle.x;
         pPath->fcampendy = pHostEdict->v.v_angle.y;
      }

      EMIT_SOUND_DYN2 (pHostEdict, CHAN_WEAPON, "common/wpn_hudon.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
      return;
   }

   else if (wpt_type == WAYPOINT_ADD_JUMP_START)
   {
      if (index != -1)
      {
         distance = (paths[index]->origin - pHostEdict->v.origin).Length ();

         if (distance < 50)
         {
            bPlaceNew = FALSE;
            pPath = paths[index];
            pPath->origin = (pPath->origin + g_vecLearnPos) / 2;
         }
      }
      else
         vecNewWaypoint = g_vecLearnPos;
   }

   else if (wpt_type == WAYPOINT_ADD_JUMP_END)
   {
      if (index != -1)
      {
         distance = (paths[index]->origin - pHostEdict->v.origin).Length ();

         if (distance < 50)
         {
            bPlaceNew = FALSE;
            pPath = paths[index];
            flags = 0;

            for (i = 0; i < MAX_PATH_INDEX; i++)
               flags += pPath->connectflag[i];

            if (flags == 0)
               pPath->origin = (pPath->origin + pHostEdict->v.origin) / 2;
         }
      }
   }

   if (bPlaceNew)
   {
      if (g_iNumWaypoints + 1 >= MAX_WAYPOINTS)
      {
         UTIL_HostPrint ("Max. Waypoint reached! Can't add Waypoint!\n");
         EMIT_SOUND_DYN2 (pHostEdict, CHAN_WEAPON, "debris/bustglass1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
         return;
      }

      index = 0;

      // find the next available slot for the new waypoint...
      pPath = paths[0];
      pPrev = NULL;

      // find an empty slot for new path_index...
      while (pPath != NULL)
      {
         pPrev = pPath;
         pPath = pPath->next;
         index++;
      }

      paths[index] = new PATH;
      if (paths[index] == NULL)
         return; // ERROR ALLOCATING MEMORY!!!

      pPath = paths[index];

      if (pPrev)
         pPrev->next = pPath;

      // increment total number of waypoints
      g_iNumWaypoints++;
      pPath->iPathNumber = index;
      pPath->flags = 0;

      // store the origin (location) of this waypoint
      pPath->origin = vecNewWaypoint;
      pPath->fcampstartx = 0;
      pPath->fcampstarty = 0;
      pPath->fcampendx = 0;
      pPath->fcampendy = 0;

      for (i = 0; i < MAX_PATH_INDEX; i++)
      {
         pPath->index[i] = -1;
         pPath->distance[i] = 0;
         pPath->connectflag[i] = 0;
         pPath->vecConnectVel[i] = g_vecZero;
      }

      pPath->next = NULL;

      // store the last used waypoint for the auto waypoint code...
      g_vecLastWaypoint = pHostEdict->v.origin;
   }

   if (wpt_type == WAYPOINT_ADD_JUMP_START)
      g_iLastJumpWaypoint = index;

   else if (wpt_type == WAYPOINT_ADD_JUMP_END)
   {
      distance = (paths[g_iLastJumpWaypoint]->origin - pHostEdict->v.origin).Length ();
      WaypointAddPath (g_iLastJumpWaypoint, index, distance);

      for (i = 0; i < MAX_PATH_INDEX; i++)
      {
         if (paths[g_iLastJumpWaypoint]->index[i] == index)
         {
            paths[g_iLastJumpWaypoint]->connectflag[i] |= C_FL_JUMP;
            paths[g_iLastJumpWaypoint]->vecConnectVel[i] = g_vecLearnVelocity;
            break;
         }
      }

      CalculateWaypointWayzone ();
      EMIT_SOUND_DYN2 (pHostEdict, CHAN_WEAPON, "weapons/xbow_hit1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
      return;
   }

   if (pPath == NULL)
      return;

   if (pHostEdict->v.flags & FL_DUCKING)
      pPath->flags |= W_FL_CROUCH; // set a crouch waypoint

   // *******************************************************
   // look for buttons, lift, ammo, flag, health, armor, etc.
   // *******************************************************

   if (wpt_type == WAYPOINT_ADD_TERRORIST)
      pPath->flags |= W_FL_TERRORIST;
   else if (wpt_type == WAYPOINT_ADD_COUNTER)
      pPath->flags |= W_FL_COUNTER;
   else if (wpt_type == WAYPOINT_ADD_LADDER)
      pPath->flags |= W_FL_LADDER;
   else if (wpt_type == WAYPOINT_ADD_RESCUE)
      pPath->flags |= W_FL_RESCUE;
   else if (wpt_type == WAYPOINT_ADD_CAMP_START)
   {
      pPath->flags |= W_FL_CAMP;
      pPath->fcampstartx = pHostEdict->v.v_angle.x;
      pPath->fcampstarty = pHostEdict->v.v_angle.y;
   }
   else if (wpt_type == WAYPOINT_ADD_GOAL)
      pPath->flags |= W_FL_GOAL;

   // Ladder waypoints need careful connections
   if (wpt_type == WAYPOINT_ADD_LADDER)
   {
      min_distance = 9999.0;
      iDestIndex = -1;

      // calculate all the paths to this new waypoint
      for (i = 0; i < g_iNumWaypoints; i++)
      {
         if (i == index)
            continue; // skip the waypoint that was just added

         // Other ladder waypoints should connect to this
         if (paths[i]->flags & W_FL_LADDER)
         {
            // check if the waypoint is reachable from the new one
            TRACE_LINE (vecNewWaypoint, paths[i]->origin, ignore_monsters, pHostEdict, &tr);
            if ((tr.flFraction == 1.0)
                && (fabs (vecNewWaypoint.x - paths[i]->origin.x) < 32)
                && (fabs (vecNewWaypoint.y - paths[i]->origin.y) < 32)
                && (fabs (vecNewWaypoint.z - paths[i]->origin.z) < g_fAutoPathMaxDistance))
            {
               distance = (paths[i]->origin - vecNewWaypoint).Length ();
               WaypointAddPath (index, i, distance);
               WaypointAddPath (i, index, distance);
            }
         }
         else
         {
            // check if the waypoint is reachable from the new one
            if ((WaypointNodeReachable (index, i)
                || (WaypointNodeReachable (i, index)) && (fabs (vecNewWaypoint.z + 16.0 - paths[i]->origin.z) < 64.0))) // KWo - 16.12.2007 - I don't like stupid connections...
            {
               distance = (paths[i]->origin - vecNewWaypoint).Length ();

               if (distance < min_distance)
               {
                  iDestIndex = i;
                  min_distance = distance;
               }
            }
         }
      }

      if ((iDestIndex > -1) && (iDestIndex < g_iNumWaypoints))
      {
         // check if the waypoint is reachable from the new one (one-way)
         if (WaypointNodeReachable (index, iDestIndex))
         {
            distance = (paths[iDestIndex]->origin - vecNewWaypoint).Length ();
            WaypointAddPath (index, iDestIndex, distance);
         }

         // check if the new one is reachable from the waypoint (other way)
         if (WaypointNodeReachable (iDestIndex, index))
         {
            distance = (paths[iDestIndex]->origin - vecNewWaypoint).Length ();
            WaypointAddPath (iDestIndex, index, distance);
         }
      }
   }
   else // KWo - 07.05.2013 - rewritten again the code to prevent making stupid connections
   {
      // calculate all the paths to this new waypoint
      j = 0;
      for (k = 0; k < MAX_PATH_INDEX; k++)
      {
         TempInd[k] = -1;
         TempDistance[k] = 9999;
         TempAngleY[k] = 360.0;
      }

      for (i = 0; i < g_iNumWaypoints; i++) // this loop is to find 8 closests waypoints to just added WP...
      {
         if (i == index)
            continue; // skip the waypoint that was just added

         // check if the waypoint is reachable from the new one (one-way)
         if (WaypointNodeReachable (index, i))
         {
            distance = (paths[i]->origin - vecNewWaypoint).Length ();
            if (j < MAX_PATH_INDEX)
            {
               TempInd[j] = i;
               TempDistance[j] = (int)distance;
               if (fLongestDist < distance)
               {
                  fLongestDist = distance;
                  iPathIndexOfLongest = j;
                  bLongestCalculated = TRUE;
               }
               j++;
            }
            else
            {
               if ((bLongestCalculated) && (iPathIndexOfLongest > -1)) // we are trying to find 8 closest waypoints to connect
               {
                  if (distance < fLongestDist)
                  {
                     fLongestDist = 0.0;
                     TempInd[iPathIndexOfLongest] = i; // here we are replacing the longest path by the closer one...
                     TempDistance[iPathIndexOfLongest] = (int)distance;
                     bLongestCalculated = FALSE;
                     fLongestDist = 0.0;
                     for (k = 0; k < MAX_PATH_INDEX; k++)
                     {
                        if (TempDistance[k] > (int)fLongestDist)
                        {
                           fLongestDist = (float)TempDistance[k];
                           iPathIndexOfLongest = k;
                           bLongestCalculated = TRUE;
                        }
                     }
                  }
               }
            }
         }
      }

      do // this code sorts paths from the closest WP to the farest away one...
      {
         bSorting = FALSE;
         for (k = 0; k < MAX_PATH_INDEX - 1; k++)
         {
            if (TempDistance[k] > TempDistance[k + 1])
            {
               Temp_ind = TempInd[k + 1];
               Temp_dist = TempDistance[k + 1];
               TempInd[k + 1] = TempInd[k];
               TempDistance[k + 1] = TempDistance[k];
               TempInd[k] = Temp_ind;
               TempDistance[k] = Temp_dist;
               bSorting = TRUE;
            }
         }
      } while (bSorting);

      if (g_b_DebugWpEdit)
      {
         for (k = 0; k < MAX_PATH_INDEX; k++)
         {
            if (j > 0)
            {
               ALERT(at_logged,"[DEBUG] WaypointAdd - %d closest WPs to added %d index are:\n",
                  (j - 1), index);
               if (TempInd[k] > -1)
                  ALERT(at_logged,"[DEBUG] WaypointAdd - WP %d (distance = %d).\n",
                     TempInd[k], TempDistance[k]);
            }
         }
      }

      for (k = 0; k < MAX_PATH_INDEX; k++)
      {
         if (TempInd[k] > -1)
         {
         // now calculate angles related to the angle created by the added WP and the closest connected one
            vec_dir_ang = UTIL_VecToAngles(paths[TempInd[k]]->origin - vecNewWaypoint) 
               - UTIL_VecToAngles(paths[TempInd[0]]->origin - vecNewWaypoint);
            if (vec_dir_ang.y < 0.0)
               vec_dir_ang.y += 360.0;
            TempAngleY[k] = vec_dir_ang.y;
         }
      }

      do // this code sorts the paths from the lowest to the highest angle...
      {
         bSorting = FALSE;
         for (k = 0; k < MAX_PATH_INDEX - 1; k++)
         {
            if (TempAngleY[k] > TempAngleY[k + 1])
            {
               Temp_ind = TempInd[k + 1];
               Temp_dist = TempDistance[k + 1];
               Temp_ang = TempAngleY[k + 1];
               TempInd[k + 1] = TempInd[k];
               TempDistance[k + 1] = TempDistance[k];
               TempAngleY[k + 1] = TempAngleY[k];
               TempInd[k] = Temp_ind;
               TempDistance[k] = Temp_dist;
               TempAngleY[k] = Temp_ang;
               bSorting = TRUE;
            }
         }
      } while (bSorting);

      if (g_b_DebugWpEdit)
      {
         for (k = 0; k < MAX_PATH_INDEX; k++)
         {
            ALERT(at_logged,"[DEBUG] WaypointAdd - index %d - checked to connect with WP nr %d (angle = %.1f).\n",
               index, TempInd[k], TempAngleY[k]);
         }
      }

      Temp_ind = -1;
      Temp_dist = 0;
      Temp_ang = 0.0;

      for(k = 2; k < MAX_PATH_INDEX; k++)
      {
testagain:
         if ((TempInd[k] == -1) || (TempInd[k - 1] == -1) || (TempInd[k - 2] == -1))
            continue;
         Temp_ind = TempInd[k]; // store the highest index which should be tested later...
         Temp_dist = TempDistance[k];
         Temp_ang = TempAngleY[k];
         if (TempAngleY[k] - TempAngleY[k - 2] < 80.0)
         {
            if ((1.1 * (TempDistance[k] + TempDistance[k - 2])/2.0 < (float)TempDistance[k - 1])
               && !(paths[TempInd[k - 1]]->flags & W_FL_LADDER))
            {
               if (g_b_DebugWpEdit)
                  ALERT(at_logged,"[DEBUG] WaypointAdd - Prevent making a stupid connection (1) from index = %d to %d.\n",
                     index, TempInd[k - 1]);
               TempInd[k - 1] = -1; // prevent making an unnecessary connection... :)
               for (j = k - 1; j < MAX_PATH_INDEX - 1; j++)
               {
                  TempInd[j] = TempInd[j + 1];
                  TempDistance[j] = TempDistance[j+1];
                  TempAngleY[j] = TempAngleY[j+1];
               }
               TempInd[MAX_PATH_INDEX - 1] = -1;
               goto testagain;
            }
         }
      }
      if ((Temp_ind > -1) && (TempInd[0] > -1) && (TempInd[1] > -1))
      {
         if ((TempAngleY[1] - Temp_ang < 80.0) || (360.0 - (TempAngleY[1] - Temp_ang) < 80.0))
         {
            if ((1.1 * (TempDistance[1] + Temp_dist)/2.0 < (float)TempDistance[0])
               && !(paths[TempInd[0]]->flags & W_FL_LADDER))
            {
               if (g_b_DebugWpEdit)
                  ALERT(at_logged,"[DEBUG] WaypointAdd - Prevent making a stupid connection (2) from index = %d to %d.\n",
                     index, TempInd[0]);
               TempInd[0] = -1; // prevent making an unnecessary connection... :)
            }
         }
      }

      for (k = 0; k < MAX_PATH_INDEX; k++)
      {
         i = TempInd[k];
         if (i > -1)
         {
            distance = (float)TempDistance[k];
            WaypointAddPath (index, i, distance);
            if (g_b_DebugWpEdit)
               ALERT(at_logged,"[DEBUG] WaypointAdd - Adding path from index = %i to %i, distance = %i, angle = %.1f.\n",
                  index, i, TempDistance[k], TempAngleY[k]);

         // check if the new one is reachable from the waypoint (other way)
            if (WaypointNodeReachable (i, index))
            {
               distance = (paths[i]->origin - vecNewWaypoint).Length ();
               WaypointAddPath (i, index, distance);
            }
         }
      }
      WaypointCleanUnnessPaths(index);
   }

   CalculateWaypointWayzone ();
   EMIT_SOUND_DYN2 (pHostEdict, CHAN_WEAPON, "weapons/xbow_hit1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
   return;
}

// deletes an existing waypoint
void WaypointDelete (void)
{
   PATH *pPrev = NULL;  // KWo - 06.01.2008 changed name
   PATH *pPath;         // KWo - 06.01.2008 changed name
   int count;
   int wpt_index;
   int i, ix;

   if (g_iNumWaypoints < 1)
      return;

   wpt_index = WaypointFindNearest ();

   if ((wpt_index < 0) || (wpt_index >= g_iNumWaypoints))
   {
      UTIL_HostPrint ("No Waypoint nearby!\n");
      return;
   }

   if ((paths[wpt_index] != NULL) && (wpt_index > 0))
      pPrev = paths[wpt_index - 1];

   count = 0;

   // delete all references to Node
   for (i = 0; i < g_iNumWaypoints; i++)
   {
      pPath = paths[i];

      for (ix = 0; ix < MAX_PATH_INDEX; ix++)
      {
         if (pPath->index[ix] == wpt_index)
         {
            pPath->index[ix] = -1; // unassign this path
            pPath->connectflag[ix] = 0;
            pPath->distance[ix] = 0;
            pPath->vecConnectVel[ix] = g_vecZero;
         }
      }
   }

   // delete all connections to Node
   for (i = 0; i < g_iNumWaypoints; i++)
   {
      pPath = paths[i];

      // if Pathnumber bigger than deleted Node Number=Number -1
      if (pPath->iPathNumber > wpt_index)
         pPath->iPathNumber--;

      for (ix = 0; ix < MAX_PATH_INDEX; ix++)
         if (pPath->index[ix] > wpt_index)
            pPath->index[ix]--;
   }

   // free deleted node
   delete paths[wpt_index];
   paths[wpt_index] = NULL;

   // Rotate Path Array down
   for (i = wpt_index; i < g_iNumWaypoints - 1; i++)
      paths[i] = paths[i + 1];

   if (pPrev)
      pPrev->next = paths[wpt_index];

   g_iNumWaypoints--;

   EMIT_SOUND_DYN2 (pHostEdict, CHAN_WEAPON, "weapons/mine_activate.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
   g_bWaypointsChanged = TRUE;
   return;
}

// Remember a closest Waypoint
void WaypointCache (void)
{
   int iNode;

   iNode = WaypointFindNearest ();

   if (iNode == -1)
   {
      g_iCachedWaypoint = -1;
      UTIL_HostPrint ("Cache cleared (no Waypoint nearby!)\n");
      return;
   }

   g_iCachedWaypoint = iNode;
   UTIL_HostPrint ("Waypoint #%d has been put into memory\n", g_iCachedWaypoint);
   return;
}

// Move a remembered Waypoint
void WaypointMoveToPosition (void) // KWo - 09.11.2007
{
   if (g_iCachedWaypoint == -1)
   {
      UTIL_HostPrint ("Nothing to Move (no Waypoint nearby!)\n");
      return;
   }
   if (fabs((paths[g_iCachedWaypoint]->origin - pHostEdict->v.origin).Length()) > 256.0)
   {
      UTIL_HostPrint ("Waypoint #%d couldn't be moved to this postion (too far away!)\n");
      return;
   }
   paths[g_iCachedWaypoint]->origin = pHostEdict->v.origin;
   g_bWaypointsChanged = TRUE;
   UTIL_HostPrint ("Waypoint #%d has been moved to the new position\n", g_iCachedWaypoint);
   g_iCachedWaypoint = -1;
   return;
}

// change a waypoint's radius
void WaypointChangeRadius (float radius)
{
   int wpt_index;

   wpt_index = WaypointFindNearest ();

   if ((wpt_index < 0) || (wpt_index >= g_iNumWaypoints))
   {
      UTIL_HostPrint ("No Waypoint nearby!\n");
      return;
   }

   paths[wpt_index]->Radius = radius;

   EMIT_SOUND_DYN2 (pHostEdict, CHAN_WEAPON, "common/wpn_hudon.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
   g_bWaypointsChanged = TRUE;
   return;
}


// switch a waypoint's flag on/off
void WaypointChangeFlag (int flag, char status)
{
   int wpt_index;

   wpt_index = WaypointFindNearest ();

   if ((wpt_index < 0) || (wpt_index >= g_iNumWaypoints))
   {
      UTIL_HostPrint ("No Waypoint nearby!\n");
      return;
   }

   if (status == FLAG_SET)
      paths[wpt_index]->flags |= flag; // set flag
   else if (status == FLAG_CLEAR)
      paths[wpt_index]->flags &= ~flag; // reset flag
   else if (status == FLAG_TOGGLE)
      paths[wpt_index]->flags ^= flag; // Switch flag on/off (XOR it)

   EMIT_SOUND_DYN2 (pHostEdict, CHAN_WEAPON, "common/wpn_hudon.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
   g_bWaypointsChanged = TRUE;
   return;
}

int WaypointLookAt (void) // KWo - 04.10.2006
{
   // find the waypoint the user is pointing at
   int j;
   float distance;
   Vector vToWaypoint;
   Vector vWaypointBound;
   float fCone1, fCone2, fCone3, fCone4, fCone5; // KWo - 11.02.2008
   float fConeMax = 0.0; // KWo - 11.02.2008

   g_iPointedWpIndex = -1;

   for (j = 0; j < g_iNumWaypoints; j++)
   {
      vToWaypoint = paths[j]->origin - pHostEdict->v.origin;
      distance = vToWaypoint.Length ();

      if (distance > 500)
         continue;

      fCone1 = GetShootingConeDeviation (pHostEdict, &paths[j]->origin);

      if (paths[j]->flags & W_FL_CROUCH)
         vWaypointBound = paths[j]->origin - Vector (0, 0, 6); // KWo - 11.02.2008
      else
         vWaypointBound = paths[j]->origin - Vector (0, 0, 12); // KWo - 11.02.2008

      fCone2 = GetShootingConeDeviation (pHostEdict, &vWaypointBound);

      if (paths[j]->flags & W_FL_CROUCH)
         vWaypointBound = paths[j]->origin - Vector (0, 0, 12); // KWo - 11.02.2008
      else
         vWaypointBound = paths[j]->origin - Vector (0, 0, 24); // KWo - 11.02.2008

      fCone3 = GetShootingConeDeviation (pHostEdict, &vWaypointBound);

      if (paths[j]->flags & W_FL_CROUCH)
         vWaypointBound = paths[j]->origin + Vector (0, 0, 6); // KWo - 11.02.2008
      else
         vWaypointBound = paths[j]->origin + Vector (0, 0, 12); // KWo - 11.02.2008

      fCone4 = GetShootingConeDeviation (pHostEdict, &vWaypointBound); // KWo - 11.02.2008

      if (paths[j]->flags & W_FL_CROUCH)
         vWaypointBound = paths[j]->origin + Vector (0, 0, 12); // KWo - 11.02.2008
      else
         vWaypointBound = paths[j]->origin + Vector (0, 0, 24); // KWo - 11.02.2008

      fCone5 = GetShootingConeDeviation (pHostEdict, &vWaypointBound); // KWo - 11.02.2008

      if ((fCone1 < 0.9992) && (fCone2 < 0.9992) && (fCone3 < 0.9992)
         && (fCone4 < 0.9992) && (fCone5 < 0.9992)) // KWo - 11.02.2008
         continue;

      if ((fCone1 > fConeMax) || (fCone2 > fConeMax) || (fCone3 > fConeMax)
         || (fCone4 > fConeMax) || (fCone5 > fConeMax)) // KWo - 11.02.2008
      {
         if ((fCone1 > fCone2) && (fCone1 > fCone4))
         {
            fConeMax = fCone1;
         }
         else if ((fCone2 > fCone1) && (fCone2 > fCone3))
         {
            fConeMax = fCone2;
         }
         else if ((fCone3 > fCone2) && (fCone3 > fCone5))
         {
            fConeMax = fCone3;
         }
         else if ((fCone4 > fCone1) && (fCone4 > fCone5))
         {
            fConeMax = fCone4;
         }
         else if ((fCone5 > fCone4) && (fCone5 > fCone3))
         {
            fConeMax = fCone5;
         }
         g_iPointedWpIndex = j;
      }
   }
   return (g_iPointedWpIndex);
}

// allows a waypointer to manually create a path from one waypoint to another
void WaypointCreatePath (char direction)
{
   int iNodeFrom;
   int iNodeTo;
   float distance;

   iNodeFrom = WaypointFindNearest ();
   iNodeTo = -1;

   if (iNodeFrom == -1)
   {
      UTIL_HostPrint ("No Waypoint nearby!\n");
      return;
   }

   iNodeTo = WaypointLookAt(); // KWo - 04.10.2006

   if ((iNodeTo < 0) || (iNodeTo >= g_iNumWaypoints))
   {
      if ((g_iCachedWaypoint >= 0) && (g_iCachedWaypoint < g_iNumWaypoints))
         iNodeTo = g_iCachedWaypoint;
      else
      {
         UTIL_HostPrint ("Destination Waypoint not found!\n");
         return;
      }
   }

   if (iNodeTo == iNodeFrom)
   {
      UTIL_HostPrint ("Can't create Path to the same Waypoint!\n");
      return;
   }

   distance = (paths[iNodeTo]->origin - paths[iNodeFrom]->origin).Length ();

   if (direction == PATH_OUTGOING)
      WaypointAddPath (iNodeFrom, iNodeTo, distance);
   else if (direction == PATH_INCOMING)
      WaypointAddPath (iNodeTo, iNodeFrom, distance);
   else
   {
      WaypointAddPath (iNodeFrom, iNodeTo, distance);
      WaypointAddPath (iNodeTo, iNodeFrom, distance);
   }

   EMIT_SOUND_DYN2 (pHostEdict, CHAN_WEAPON, "common/wpn_hudon.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
   g_bWaypointsChanged = TRUE;
   return;
}


// allows a waypointer to manually remove a path from one waypoint to another
void WaypointDeletePath (void)
{
   int index;
   int iNodeFrom;
   int iNodeTo;

   iNodeFrom = WaypointFindNearest ();
   iNodeTo = -1;

   if (iNodeFrom == -1)
   {
      UTIL_HostPrint ("No Waypoint nearby!\n");
      return;
   }

   iNodeTo = WaypointLookAt(); // KWo - 04.10.2006

   if ((iNodeTo < 0) || (iNodeTo >= g_iNumWaypoints))
   {
      if ((g_iCachedWaypoint >= 0) && (g_iCachedWaypoint < g_iNumWaypoints))
         iNodeTo = g_iCachedWaypoint;
      else
      {
         UTIL_HostPrint ("Destination Waypoint not found!\n");
         return;
      }
   }

   for (index = 0; index < MAX_PATH_INDEX; index++)
   {
      if (paths[iNodeFrom]->index[index] == iNodeTo)
      {
         paths[iNodeFrom]->index[index] = -1; // unassign this path
         paths[iNodeFrom]->connectflag[index] = 0;
         paths[iNodeFrom]->vecConnectVel[index] = g_vecZero;
         paths[iNodeFrom]->distance[index] = 0;

         EMIT_SOUND_DYN2 (pHostEdict, CHAN_WEAPON, "weapons/mine_activate.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
         g_bWaypointsChanged = TRUE;
         return;
      }
   }

   // not found this way ? check for incoming connections then
   index = iNodeFrom;
   iNodeFrom = iNodeTo;
   iNodeTo = index;

   for (index = 0; index < MAX_PATH_INDEX; index++)
   {
      if (paths[iNodeFrom]->index[index] == iNodeTo)
      {
         paths[iNodeFrom]->index[index] = -1; // unassign this path
         paths[iNodeFrom]->connectflag[index] = 0;
         paths[iNodeFrom]->vecConnectVel[index] = g_vecZero;
         paths[iNodeFrom]->distance[index] = 0;

         EMIT_SOUND_DYN2 (pHostEdict, CHAN_WEAPON, "weapons/mine_activate.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
         g_bWaypointsChanged = TRUE;
         return;
      }
   }

   UTIL_HostPrint ("Already no Path to this Waypoint!\n");
   return;
}

// Clean unnessesary paths from/to the waypoint
void WaypointCleanUnnessPaths (int index) // KWo - added 10.11.2007, rewritten 07.05.2013
{
   int j, k;
   short int TempInd[MAX_PATH_INDEX];
   int TempPathNr[MAX_PATH_INDEX];
   int TempDistance[MAX_PATH_INDEX];
   float TempAngleY[MAX_PATH_INDEX];
   short int Temp_ind;
   int Temp_dist;
   float Temp_ang;
   int Temp_path_nr;
   Vector vec_dir_ang;
   bool bSorting;

	if ((index < 0) || (index >= g_iNumWaypoints))
      return;

   Temp_ind = -1;
   Temp_dist = 9999;
   Temp_ang = 0.0;
   Temp_path_nr = -1;

   for (k = 0; k < MAX_PATH_INDEX; k++)
   {
      TempPathNr[k] = k;
      TempInd[k] = paths[index]->index[k];
      TempDistance[k] = paths[index]->distance[k];
      if (TempInd[k] == -1)
      {
         TempDistance[k] = 9999;
      }

      if (TempDistance[k] < Temp_dist)
      {
         Temp_dist = paths[index]->distance[k];
         Temp_path_nr = k;
         Temp_ind = TempInd[k];
      }
   }

   if (Temp_path_nr == -1)
   {
      if (g_iNumWaypoints > 1)
         UTIL_HostPrint ("Can't find Path to the closest connected Waypoint to WP nr %d!\n", index);
      return;
   }
   else if (g_b_DebugWpEdit)
      ALERT(at_logged,"[DEBUG] WaypointCleanUnnessPaths - the closest WP to checked %d index is - %d (distance = %d).\n",
               index, Temp_ind, Temp_dist);

   do // This code sorts paths from the closest WP to the farest away one...
   {
      bSorting = FALSE;
      for (k = 0; k < MAX_PATH_INDEX - 1; k++)
      {
         if (TempDistance[k] > TempDistance[k + 1])
         {
            Temp_ind = TempInd[k + 1];
            Temp_dist = TempDistance[k + 1];
            Temp_path_nr = TempPathNr[k + 1];
            TempInd[k + 1] = TempInd[k];
            TempDistance[k + 1] = TempDistance[k];
            TempPathNr[k + 1] = TempPathNr[k];
            TempInd[k] = Temp_ind;
            TempDistance[k] = Temp_dist;
            TempPathNr[k] = Temp_path_nr;
            bSorting = TRUE;
         }
      }
   } while (bSorting);

   if (g_b_DebugWpEdit)
      ALERT(at_logged,"[DEBUG] WaypointCleanUnnessPaths - the closest WP to checked %d index is - %d (distance = %d).\n",
               index, TempInd[0], TempDistance[0]);

   for (k = 0; k < MAX_PATH_INDEX; k++)
   {
      if (TempInd[k] == -1)
      {
         TempDistance[k] = 9999;
         TempAngleY[k] = 360.0;
      }
      else if ((TempInd[k] > -1) && (TempInd[k] < g_iNumWaypoints))
      {
         // calculate angles related to the angle of the closeset connected WP
         vec_dir_ang = UTIL_VecToAngles(paths[TempInd[k]]->origin - paths[index]->origin) 
            - UTIL_VecToAngles(paths[TempInd[0]]->origin - paths[index]->origin);
         if (vec_dir_ang.y < 0.0)
            vec_dir_ang.y += 360.0;
         TempAngleY[k] = vec_dir_ang.y;
         if (g_b_DebugWpEdit)
            ALERT(at_logged,"[DEBUG] WaypointCleanUnnessPaths - index %d connected to %d [%d] with angle = %.1f and distance = %d.\n",
               index, TempInd[k], k, TempAngleY[k], TempDistance[k]);
      }
   }

   do // This code sorts the paths from the lowest to the highest angle (related to the vector closest WP - checked index)...
   {
      bSorting = FALSE;
      for (k = 0; k < MAX_PATH_INDEX - 1; k++)
      {
         if (TempInd[k] == -1)
            continue;

         if (TempAngleY[k] > TempAngleY[k + 1])
         {
            Temp_ind = TempInd[k + 1];
            Temp_dist = TempDistance[k + 1];
            Temp_ang = TempAngleY[k + 1];
            Temp_path_nr = TempPathNr[k + 1];
            TempInd[k + 1] = TempInd[k];
            TempDistance[k + 1] = TempDistance[k];
            TempAngleY[k + 1] = TempAngleY[k];
            TempPathNr[k + 1] = TempPathNr[k];
            TempInd[k] = Temp_ind;
            TempDistance[k] = Temp_dist;
            TempAngleY[k] = Temp_ang;
            TempPathNr[k] = Temp_path_nr;
            bSorting = TRUE;
         }
      }
   } while (bSorting);

   if (g_b_DebugWpEdit)
   {
      for (k = 0; k < MAX_PATH_INDEX; k++)
      {
         ALERT(at_logged,"[DEBUG] WaypointCleanUnnessPaths - index %d - connected with WP nr %d (angle = %.1f).\n",
            index, TempInd[k], TempAngleY[k]);
      }
   }

   // now we should start testing connections from the closest WP if all calculation have been executed right until now...
   Temp_ind = -1;
   Temp_dist = 0;
   Temp_ang = 0.0;
   Temp_path_nr = 0;

   for (k = 2; k < MAX_PATH_INDEX; k++)
   {
testagain:
      if ((TempInd[k] == -1) || (TempInd[k - 1] == -1) || (TempInd[k - 2] == -1))
         continue;

      Temp_ind = TempInd[k]; // store the highest index which should be tested later...
      Temp_dist = TempDistance[k];
      Temp_ang = TempAngleY[k];
      if (TempAngleY[k] - TempAngleY[k - 2] < 80.0)
      {
         if (((paths[index]->flags & W_FL_LADDER) && (paths[TempInd[k - 1]]->flags & W_FL_LADDER)) // leave alone ladder connections...
            || (paths[index]->connectflag[TempPathNr[k - 1]] & C_FL_JUMP)) // and don't remove jump connections..
            continue;

         if (1.1 * (TempDistance[k] + TempDistance[k - 2])/2.0 < (float)TempDistance[k - 1])
         {
            if (paths[index]->index[TempPathNr[k - 1]] == TempInd[k - 1])
            {
               if (g_b_DebugWpEdit)
                  ALERT(at_logged,"[DEBUG] WaypointCleanUnnessPaths - Removing a stupid connection from index = %d to %d.\n",
                     index, TempInd[k - 1]);
               paths[index]->index[TempPathNr[k - 1]] = -1; // unassign this path
               paths[index]->connectflag[TempPathNr[k - 1]] = 0;
               paths[index]->vecConnectVel[TempPathNr[k - 1]] = g_vecZero;
               paths[index]->distance[TempPathNr[k - 1]] = 0;

               g_bWaypointsChanged = TRUE;
               for (j = 0; j < MAX_PATH_INDEX; j++)
               {
                  if ((paths[TempInd[k - 1]]->index[j] == index)
                        && !(paths[TempInd[k - 1]]->connectflag[j] & C_FL_JUMP))
                  {
                     if (g_b_DebugWpEdit)
                        ALERT(at_logged,"[DEBUG] WaypointCleanUnnessPaths - Removing also a stupid connection from index = %d to %d.\n",
                           TempInd[k - 1], index);
                     paths[TempInd[k - 1]]->index[j] = -1; // unassign this path
                     paths[TempInd[k - 1]]->connectflag[j] = 0;
                     paths[TempInd[k - 1]]->vecConnectVel[j] = g_vecZero;
                     paths[TempInd[k - 1]]->distance[j] = 0;
                  }
               }
               TempInd[k - 1] = -1;
               for (j = k - 1; j < MAX_PATH_INDEX - 1; j++)
               {
                  TempInd[j] = TempInd[j + 1];
                  TempDistance[j] = TempDistance[j + 1];
                  TempAngleY[j] = TempAngleY[j + 1];
                  TempPathNr[j] = TempPathNr[j + 1];
               }
               TempInd[MAX_PATH_INDEX - 1] = -1;
               goto testagain;
            }
            else
            {
               if (g_b_DebugWpEdit)
                  ALERT(at_logged,"[DEBUG] WaypointCleanUnnessPaths - Failed to remove a stupid connection from index = %d to %d.\n",
                     index, TempInd[k - 1]);
               return;
            }
         }
      }
   }
   if ((Temp_ind > -1) && (TempInd[0] > -1) && (TempInd[1] > -1))
   {
      if (((TempAngleY[1] - Temp_ang < 80.0) || (360.0 - (TempAngleY[1] - Temp_ang) < 80.0))
         && (!(paths[TempInd[0]]->flags & W_FL_LADDER) || !(paths[index]->flags & W_FL_LADDER))
         && (!(paths[index]->connectflag[TempPathNr[0]] & C_FL_JUMP)))
      {
         if (1.1 * (TempDistance[1] + Temp_dist)/2.0 < (float)TempDistance[0])
         {
            if (paths[index]->index[TempPathNr[0]] == TempInd[0])
            {
               if (g_b_DebugWpEdit)
                  ALERT(at_logged,"[DEBUG] WaypointCleanUnnessPaths - Removing (2) a stupid connection from index = %d to %d.\n",
                     index, TempInd[0]);
               paths[index]->index[TempPathNr[0]] = -1; // unassign this path
               paths[index]->connectflag[TempPathNr[0]] = 0;
               paths[index]->vecConnectVel[TempPathNr[0]] = g_vecZero;
               paths[index]->distance[TempPathNr[0]] = 0;

               g_bWaypointsChanged = TRUE;
               for (j = 0; j < MAX_PATH_INDEX; j++)
               {
                  if ((paths[TempInd[0]]->index[j] == index)
                        && !(paths[TempInd[0]]->connectflag[j] & C_FL_JUMP))
                  {
                     if (g_b_DebugWpEdit)
                        ALERT(at_logged,"[DEBUG] WaypointCleanUnnessPaths - Removing (2) also a stupid connection from index = %d to %d.\n",
                           TempInd[0], index);
                     paths[TempInd[0]]->index[j] = -1; // unassign this path
                     paths[TempInd[0]]->connectflag[j] = 0;
                     paths[TempInd[0]]->vecConnectVel[j] = g_vecZero;
                     paths[TempInd[0]]->distance[j] = 0;
                  }
               }
               TempInd[0] = -1;
               for (j = 0; j < MAX_PATH_INDEX - 1; j++)
               {
                  TempInd[j] = TempInd[j + 1];
                  TempDistance[j] = TempDistance[j + 1];
                  TempAngleY[j] = TempAngleY[j + 1];
                  TempPathNr[j] = TempPathNr[j + 1];
               }
               TempInd[MAX_PATH_INDEX - 1] = -1;
            }
            else
            {
               if (g_b_DebugWpEdit)
                  ALERT(at_logged,"[DEBUG] WaypointCleanUnnessPaths - Failed to remove (2) a stupid connection from index = %d to %d.\n",
                     TempInd[0], index);
               return;
            }
         }
      }
   }

   Temp_ind = -1;
   Temp_dist = 0;
   Temp_ang = 0.0;
   Temp_path_nr = 0;

   for (k = 1; k < MAX_PATH_INDEX; k++)
   {
testagain2:
      if ((TempInd[k] == -1) || (TempInd[k - 1] == -1))
         continue;

      if (TempAngleY[k] - TempAngleY[k - 1] < 40.0)
      {
         if ((float)TempDistance[k] > 1.1 * TempDistance[k - 1])
         {
            if (((paths[index]->flags & W_FL_LADDER) && (paths[TempInd[k]]->flags & W_FL_LADDER)) // leave alone ladder connections...
              || (paths[index]->connectflag[TempPathNr[k]] & C_FL_JUMP)) // and don't remove jump connections..
              continue;

            if (paths[index]->index[TempPathNr[k]] == TempInd[k])
            {
               if (g_b_DebugWpEdit)
                  ALERT(at_logged,"[DEBUG] WaypointCleanUnnessPaths - Removing (3) a stupid connection from index = %d to %d.\n",
                     index, TempInd[k]);
               paths[index]->index[TempPathNr[k]] = -1; // unassign this path
               paths[index]->connectflag[TempPathNr[k]] = 0;
               paths[index]->vecConnectVel[TempPathNr[k]] = g_vecZero;
               paths[index]->distance[TempPathNr[k]] = 0;

               g_bWaypointsChanged = TRUE;
               for (j = 0; j < MAX_PATH_INDEX; j++)
               {
                  if ((paths[TempInd[k]]->index[j] == index)
                        && !(paths[TempInd[k]]->connectflag[j] & C_FL_JUMP))
                  {
                     if (g_b_DebugWpEdit)
                        ALERT(at_logged,"[DEBUG] WaypointCleanUnnessPaths - Removing (3) also a stupid connection from index = %d to %d.\n",
                           TempInd[k], index);
                     paths[TempInd[k]]->index[j] = -1; // unassign this path
                     paths[TempInd[k]]->connectflag[j] = 0;
                     paths[TempInd[k]]->vecConnectVel[j] = g_vecZero;
                     paths[TempInd[k]]->distance[j] = 0;
                  }
               }
               TempInd[k] = -1;
               for (j = k; j < MAX_PATH_INDEX - 1; j++)
               {
                  TempInd[j] = TempInd[j + 1];
                  TempDistance[j] = TempDistance[j + 1];
                  TempAngleY[j] = TempAngleY[j + 1];
                  TempPathNr[j] = TempPathNr[j + 1];
               }
               TempInd[MAX_PATH_INDEX - 1] = -1;
               goto testagain2;
            }
            else
            {
               if (g_b_DebugWpEdit)
                  ALERT(at_logged,"[DEBUG] WaypointCleanUnnessPaths - Failed to remove (3) a stupid connection from index = %d to %d.\n",
                     index, TempInd[k]);
               return;
            }
         }
         else if ((float)TempDistance[k - 1] > 1.1 * TempDistance[k])
         {
            if (((paths[index]->flags & W_FL_LADDER) && (paths[TempInd[k - 1]]->flags & W_FL_LADDER)) // leave alone ladder connections...
               || (paths[index]->connectflag[TempPathNr[k - 1]] & C_FL_JUMP)) // and don't remove jump connections..
               continue;

            if (paths[index]->index[TempPathNr[k - 1]] == TempInd[k - 1])
            {
               if (g_b_DebugWpEdit)
                  ALERT(at_logged,"[DEBUG] WaypointCleanUnnessPaths - Removing (4) a stupid connection from index = %d to %d.\n",
                     index, TempInd[k - 1]);
               paths[index]->index[TempPathNr[k - 1]] = -1; // unassign this path
               paths[index]->connectflag[TempPathNr[k - 1]] = 0;
               paths[index]->vecConnectVel[TempPathNr[k - 1]] = g_vecZero;
               paths[index]->distance[TempPathNr[k - 1]] = 0;

               g_bWaypointsChanged = TRUE;
               for (j = 0; j < MAX_PATH_INDEX; j++)
               {
                  if ((paths[TempInd[k - 1]]->index[j] == index)
                        && !(paths[TempInd[k - 1]]->connectflag[j] & C_FL_JUMP))
                  {
                     if (g_b_DebugWpEdit)
                        ALERT(at_logged,"[DEBUG] WaypointCleanUnnessPaths - Removing (4) also a stupid connection from index = %d to %d.\n",
                           TempInd[k - 1], index);
                     paths[TempInd[k - 1]]->index[j] = -1; // unassign this path
                     paths[TempInd[k - 1]]->connectflag[j] = 0;
                     paths[TempInd[k - 1]]->vecConnectVel[j] = g_vecZero;
                     paths[TempInd[k - 1]]->distance[j] = 0;
                  }
               }
               TempInd[k - 1] = -1;
               for (j = k - 1; j < MAX_PATH_INDEX - 1; j++)
               {
                  TempInd[j] = TempInd[j + 1];
                  TempDistance[j] = TempDistance[j + 1];
                  TempAngleY[j] = TempAngleY[j + 1];
                  TempPathNr[j] = TempPathNr[j + 1];
               }
               TempInd[MAX_PATH_INDEX - 1] = -1;
               goto testagain2;
            }
            else
            {
               if (g_b_DebugWpEdit)
                  ALERT(at_logged,"[DEBUG] WaypointCleanUnnessPaths - Failed to remove (4) a stupid connection from index = %d to %d.\n",
                     index, TempInd[k -1]);
               return;
            }
         }
      }
      else
      {
         Temp_ind = TempInd[k]; // store the highest index which should be tested later...
         Temp_dist = TempDistance[k];
         Temp_ang = TempAngleY[k];
         Temp_path_nr = TempPathNr[k];
      }
   }

   if (g_b_DebugWpEdit)
   {
      for (k = 0; k < MAX_PATH_INDEX; k++)
      {
         if (TempInd[k] > -1)
         {
            ALERT(at_logged,"[DEBUG] - WaypointCleanUnnessPaths - we still have connection between index = %d and %d [%d] with angle = %d and distance = %d.\n",
               index, TempInd[k], k, int(TempAngleY[k]), int(TempDistance[k]));
         }
      }
   }

   if ((Temp_ind > -1) && (TempInd[0] > -1))
   {
      if (((Temp_ang - TempAngleY[0] < 40.0) || (360.0 - (Temp_ang - TempAngleY[0]) < 40.0))
            && (!(paths[TempInd[0]]->flags & W_FL_LADDER) || !(paths[index]->flags & W_FL_LADDER)))
      {
         if ((1.1 * Temp_dist < (float)TempDistance[0]) && (!(paths[index]->connectflag[TempPathNr[0]] & C_FL_JUMP)))
         {
            if (paths[index]->index[TempPathNr[0]] == TempInd[0])
            {
               if (g_b_DebugWpEdit)
                  ALERT(at_logged,"[DEBUG] WaypointCleanUnnessPaths - Removing (5) a stupid connection from index = %d to %d.\n",
                     index, TempInd[0]);
               paths[index]->index[TempPathNr[0]] = -1; // unassign this path
               paths[index]->connectflag[TempPathNr[0]] = 0;
               paths[index]->vecConnectVel[TempPathNr[0]] = g_vecZero;
               paths[index]->distance[TempPathNr[0]] = 0;

               g_bWaypointsChanged = TRUE;
               for (j = 0; j < MAX_PATH_INDEX; j++)
               {
                  if ((paths[TempInd[0]]->index[j] == index)
                        && !(paths[TempInd[0]]->connectflag[j] & C_FL_JUMP))
                  {
                     if (g_b_DebugWpEdit)
                        ALERT(at_logged,"[DEBUG] WaypointCleanUnnessPaths - Removing (5) also a stupid connection from index = %d to %d.\n",
                           TempInd[0], index);
                     paths[TempInd[0]]->index[j] = -1; // unassign this path
                     paths[TempInd[0]]->connectflag[j] = 0;
                     paths[TempInd[0]]->vecConnectVel[j] = g_vecZero;
                     paths[TempInd[0]]->distance[j] = 0;
                  }
               }
               TempInd[0] = -1;
               for (j = 0; j < MAX_PATH_INDEX - 1; j++)
               {
                  TempInd[j] = TempInd[j + 1];
                  TempDistance[j] = TempDistance[j + 1];
                  TempAngleY[j] = TempAngleY[j + 1];
                  TempPathNr[j] = TempPathNr[j + 1];
               }
               TempInd[MAX_PATH_INDEX - 1] = -1;
            }
            else
            {
               if (g_b_DebugWpEdit)
                  ALERT(at_logged,"[DEBUG] WaypointCleanUnnessPaths - Failed to remove (5) a stupid connection from index = %d to %d.\n",
                     TempInd[0], index);
               return;
            }
         }
         else if ((1.1 * TempDistance[0] < (float) Temp_dist) && (!(paths[index]->connectflag[Temp_path_nr] & C_FL_JUMP)))
         {
            if (paths[index]->index[Temp_path_nr] == Temp_ind)  
            {
               if (g_b_DebugWpEdit)
                  ALERT(at_logged,"[DEBUG] WaypointCleanUnnessPaths - Removing (6) a stupid connection from index = %d to %d.\n",
                     index, TempInd[0]);
               paths[index]->index[Temp_path_nr] = -1; // unassign this path
               paths[index]->connectflag[Temp_path_nr] = 0;
               paths[index]->vecConnectVel[Temp_path_nr] = g_vecZero;
               paths[index]->distance[Temp_path_nr] = 0;

               g_bWaypointsChanged = TRUE;
               for (j = 0; j < MAX_PATH_INDEX; j++)
               {
                  if ((paths[Temp_ind]->index[j] == index)
                        && !(paths[Temp_ind]->connectflag[j] & C_FL_JUMP))
                  {
                     if (g_b_DebugWpEdit)
                        ALERT(at_logged,"[DEBUG] WaypointCleanUnnessPaths - Removing (6) also a stupid connection from index = %d to %d.\n",
                           TempInd[0], index);
                     paths[Temp_ind]->index[j] = -1; // unassign this path
                     paths[Temp_ind]->connectflag[j] = 0;
                     paths[Temp_ind]->vecConnectVel[j] = g_vecZero;
                     paths[Temp_ind]->distance[j] = 0;
                  }
               }
               TempInd[Temp_path_nr] = -1;
            }
            else
            {
               if (g_b_DebugWpEdit)
                  ALERT(at_logged,"[DEBUG] WaypointCleanUnnessPaths - Failed to remove (6) a stupid connection from index = %d to %d.\n",
                     TempInd[0], index);
               return;
            }
         }
      }
   }
	return;
}

// Clean unnessesary paths from/to the waypoint
void WaypointFixOldCampType (int index) // KWo - 21.05.2013
{
   Vector vecViewAngle;
   Vector vecOrigin;
   Vector vecDirectionStart;
   Vector vecDirectionEnd;
   Vector vecViewOffset;

	if ((index < 0) || (index >= g_iNumWaypoints))
      return;

   if (!(paths[index]->flags & W_FL_CAMP))
      return;

   vecViewOffset.x = 0.0;
   vecViewOffset.y = 0.0;
   vecViewOffset.z = 17.0;

   if (paths[index]->flags & W_FL_CROUCH)
      vecViewOffset.z = 15.0;

   vecDirectionStart = Vector (paths[index]->fcampstartx, paths[index]->fcampstarty, paths[index]->origin.z) - (paths[index]->origin + vecViewOffset);
   vecDirectionEnd = Vector (paths[index]->fcampendx, paths[index]->fcampendy, paths[index]->origin.z) - (paths[index]->origin + vecViewOffset);

   vecViewAngle = UTIL_VecToAngles(vecDirectionStart);
   vecViewAngle.x = -vecViewAngle.x;
   UTIL_ClampVector(&vecViewAngle);
   paths[index]->fcampstartx = vecViewAngle.x;
   paths[index]->fcampstarty = vecViewAngle.y;

   vecViewAngle = UTIL_VecToAngles(vecDirectionEnd);
   vecViewAngle.x = -vecViewAngle.x;
   UTIL_ClampVector(&vecViewAngle);
   paths[index]->fcampendx = vecViewAngle.x;
   paths[index]->fcampendy = vecViewAngle.y;

   return;
}

// Checks if Waypoint A has a Connection to Waypoint Nr. B
bool ConnectedToWaypoint (int a, int b)
{
   int ix;

   for (ix = 0; ix < MAX_PATH_INDEX; ix++)
      if (paths[a]->index[ix] == b)
         return (TRUE);

   return (FALSE);
}


// saves the experience table to the file
void SaveExperienceTab (void)
{
   char filename[256];
   EXPERIENCE_HDR header;
   experiencesave_t *pExperienceSave;
   int iResult;
   int i, j;

//   if ((g_iNumWaypoints <= 0) || g_bWaypointsChanged /* || g_bWaypointOn */)
//      return;
   if (g_iNumWaypoints <= 0) // KWo - 08.10.2010
   {
      ALERT (at_logged, "Podbot mm - Experience Data not saved - number of waypoints %i.\n", g_iNumWaypoints);
      return;
   }
   if (g_bWaypointsChanged) // KWo - 08.10.2010
   {
      ALERT (at_logged, "Podbot mm - Experience Data not saved - waypoints have been changed.\n");
      return;
   }

   strncpy (header.filetype, "PODEXP!", sizeof (header.filetype));
   header.experiencedata_file_version = EXPERIENCE_VERSION;
   header.number_of_waypoints = g_iNumWaypoints;

   snprintf (filename, sizeof (filename), "%s/addons/podbot/%s/%s.pxp", g_szGameDirectory, g_sz_cv_WPT_Folder, STRING (gpGlobals->mapname));

   pExperienceSave = new experiencesave_t[g_iNumWaypoints * g_iNumWaypoints];

   if (pExperienceSave == NULL)
   {
      ALERT(at_logged, "Podbot mm - ERROR: Couldn't allocate Memory for saving Experience Data!\n");
      return;
   }

   UTIL_ServerPrint ("Compressing & saving Experience Data...this may take a while!\n");

   for (i = 0; i < g_iNumWaypoints; i++)
   {
      for (j = 0; j < g_iNumWaypoints; j++)
      {
         (pExperienceSave + (i * g_iNumWaypoints) + j)->uTeam0Damage = (pBotExperienceData + (i * g_iNumWaypoints) + j)->uTeam0Damage >> 3;
         (pExperienceSave + (i * g_iNumWaypoints) + j)->uTeam1Damage = (pBotExperienceData + (i * g_iNumWaypoints) + j)->uTeam1Damage >> 3;
         (pExperienceSave + (i * g_iNumWaypoints) + j)->cTeam0Value = (pBotExperienceData + (i * g_iNumWaypoints) + j)->wTeam0Value / 8;
         (pExperienceSave + (i * g_iNumWaypoints) + j)->cTeam1Value = (pBotExperienceData + (i * g_iNumWaypoints) + j)->wTeam1Value / 8;
      }
   }

   iResult = Encode (filename, (unsigned char *) &header, sizeof (EXPERIENCE_HDR), (unsigned char *) pExperienceSave, g_iNumWaypoints * g_iNumWaypoints * sizeof (experiencesave_t));

   if (pExperienceSave != NULL)
      delete [](pExperienceSave);
   pExperienceSave = NULL;

   if (iResult == -1)
   {
      ALERT(at_logged, "Podbot mm - ERROR: Couldn't save Experience Data!\n");
      return;
   }

   ALERT (at_logged, "Podbot mm - Experience Data saved...\n");
   return;
}

// initiates the experience table (at the map start)
void InitExperienceTab (void)
{
   FILE *bfp = NULL;
   experiencesave_t *pExperienceLoad;
   int iCompare;
   bool bDataLoaded = FALSE;
   bool bExperienceExists = FALSE;
   int i, j;
   EXPERIENCE_HDR header;
   char filename[256];
   char wptfilename[256];

   if (pBotExperienceData != NULL)
      delete [](pBotExperienceData);
   pBotExperienceData = NULL;

   if (g_iNumWaypoints == 0)
      return;

   snprintf (filename, sizeof (filename), "addons/podbot/%s/%s.pxp", g_sz_cv_WPT_Folder, STRING (gpGlobals->mapname));
   snprintf (wptfilename, sizeof (wptfilename), "addons/podbot/%s/%s.pwf", g_sz_cv_WPT_Folder, STRING (gpGlobals->mapname));

   pBotExperienceData = new experience_t[g_iNumWaypoints * g_iNumWaypoints];

   if (pBotExperienceData == NULL)
   {
      UTIL_ServerPrint ("ERROR: Couldn't allocate Memory for Experience Data !\n");
      return;
   }

   // Does the Experience File exist & is newer than waypoint file ?
   if (COMPARE_FILE_TIME (filename, wptfilename, &iCompare))
   {
      if (iCompare > 0)
         bExperienceExists = TRUE;
   }

   if (bExperienceExists)
   {
      // Now build the real filename
      snprintf (filename, sizeof (filename), "%s/addons/podbot/%s/%s.pxp", g_szGameDirectory, g_sz_cv_WPT_Folder, STRING (gpGlobals->mapname));
      bfp = fopen (filename, "rb");

      // if file exists, read the experience Data from it
      if (bfp != NULL)
      {
         fread (&header, sizeof (EXPERIENCE_HDR), 1, bfp);
         fclose (bfp);

         header.filetype[7] = 0;

         if (strcmp (header.filetype, "PODEXP!") == 0)
         {
            if ((header.experiencedata_file_version == EXPERIENCE_VERSION)
                && (header.number_of_waypoints == g_iNumWaypoints))
            {
               UTIL_ServerPrint ("Loading & decompressing Experience Data\n");

               pExperienceLoad = new experiencesave_t[g_iNumWaypoints * g_iNumWaypoints];

               if (pExperienceLoad == NULL)
               {
                  UTIL_ServerPrint ("ERROR: Couldn't allocate Memory for Experience Data !\n");
                  return;
               }

               Decode (filename, sizeof (EXPERIENCE_HDR), (unsigned char *) pExperienceLoad, g_iNumWaypoints * g_iNumWaypoints * sizeof (experiencesave_t));

               g_iHighestDamageT = 1;     // KWo 09.04.2006
               g_iHighestDamageCT = 1;    // KWo 09.04.2006
               g_iHighestDamageWpT = -1;  // KWo 05.01.2008
               g_iHighestDamageWpCT = -1; // KWo 05.01.2008

               for (i = 0; i < g_iNumWaypoints; i++)
               {
                  for (j = 0; j < g_iNumWaypoints; j++)
                  {
                     if (i == j)
                     {
                        (pBotExperienceData + (i * g_iNumWaypoints) + j)->uTeam0Damage = (unsigned short) ((pExperienceLoad + (i * g_iNumWaypoints) + j)->uTeam0Damage);
                        (pBotExperienceData + (i * g_iNumWaypoints) + j)->uTeam1Damage = (unsigned short) ((pExperienceLoad + (i * g_iNumWaypoints) + j)->uTeam1Damage);
                        if ((pBotExperienceData + (i * g_iNumWaypoints) + j)->uTeam0Damage > g_iHighestDamageT )  // KWo 09.04.2006
                        {
                           g_iHighestDamageT = (pBotExperienceData + (i * g_iNumWaypoints) + j)->uTeam0Damage;
                           g_iHighestDamageWpT = j; // KWo - 05.01.2008
                        }
                        if ((pBotExperienceData + (i * g_iNumWaypoints) + j)->uTeam1Damage > g_iHighestDamageCT )  // KWo 09.04.2006
                        {
                           g_iHighestDamageCT = (pBotExperienceData + (i * g_iNumWaypoints) + j)->uTeam1Damage;
                           g_iHighestDamageWpCT = j; // KWo - 05.01.2008
                        }
                     }
                     else
                     {
                        (pBotExperienceData + (i * g_iNumWaypoints) + j)->uTeam0Damage = (unsigned short) ((pExperienceLoad + (i * g_iNumWaypoints) + j)->uTeam0Damage) << 3;
                        (pBotExperienceData + (i * g_iNumWaypoints) + j)->uTeam1Damage = (unsigned short) ((pExperienceLoad + (i * g_iNumWaypoints) + j)->uTeam1Damage) << 3;
                     }

                     (pBotExperienceData + (i * g_iNumWaypoints) + j)->wTeam0Value = (signed short) ((pExperienceLoad + (i * g_iNumWaypoints) + j)->cTeam0Value) * 8;
                     (pBotExperienceData + (i * g_iNumWaypoints) + j)->wTeam1Value = (signed short) ((pExperienceLoad + (i * g_iNumWaypoints) + j)->cTeam1Value) * 8;
                     (pBotExperienceData + (i * g_iNumWaypoints) + j)->iTeam0_danger_index = -1;
                     (pBotExperienceData + (i * g_iNumWaypoints) + j)->iTeam1_danger_index = -1;
                  }
               }

               if (pExperienceLoad != NULL)
                  delete [](pExperienceLoad);
               pExperienceLoad = NULL;

               bDataLoaded = TRUE;
            }
         }
      }
   }

   if (!bDataLoaded)
   {
      UTIL_ServerPrint ("No Experience Data File or old one - starting new !\n");

      g_iHighestDamageT = 1;  // KWo 09.04.2006
      g_iHighestDamageCT = 1;  // KWo 09.04.2006
      g_iHighestDamageWpT = -1;  // KWo 05.01.2008
      g_iHighestDamageWpCT = -1;  // KWo 05.01.2008

      // initialize table by hand to correct values, and NOT zero it out, got it Markus ? ;)
      for (i = 0; i < g_iNumWaypoints; i++)
      {
         for (j = 0; j < g_iNumWaypoints; j++)
         {
            (pBotExperienceData + (i * g_iNumWaypoints) + j)->iTeam0_danger_index = -1;
            (pBotExperienceData + (i * g_iNumWaypoints) + j)->iTeam1_danger_index = -1;
            (pBotExperienceData + (i * g_iNumWaypoints) + j)->uTeam0Damage = 0;
            (pBotExperienceData + (i * g_iNumWaypoints) + j)->uTeam1Damage = 0;
            (pBotExperienceData + (i * g_iNumWaypoints) + j)->wTeam0Value = 0;
            (pBotExperienceData + (i * g_iNumWaypoints) + j)->wTeam1Value = 0;
         }
      }
   }
   else
      UTIL_ServerPrint ("Experience Data loaded from File...\n");

   return;
}

// saves the visibility table (after recalculating the visibility - when pwf file was changed)
void SaveVisTab (void)
{
   char filename[256];
   VISTABLE_HDR header;
   int iResult;

   if (g_iNumWaypoints <= 0) // KWo - 08.10.2010
   {
      ALERT (at_logged, "Podbot mm - Visibility Table not saved - number of Waypoints - %i.\n", g_iNumWaypoints);
      return;
   }
   if (g_bWaypointsChanged) // KWo - 08.10.2010
   {
      ALERT (at_logged, "Podbot mm - Visibility Table not saved - Waypoints have been changed.\n");
      return;
   }
   if (g_bWaypointOn) // KWo - 08.10.2010
   {
      ALERT (at_logged, "Podbot mm - Visibility Table not saved - Waypoints Editor is on.\n");
      return;
   }
   if (!g_bSaveVisTable) // KWo - 08.10.2010
   {
      if (g_bRecalcVis)
      {
         ALERT (at_logged, "Podbot mm - Visibility Table not saved - Table not ready to be saved.\n");
         return;
      }
      else
      {
         ALERT (at_logged, "Podbot mm - Visibility Table not saved - Table doesn't need to be saved now.\n");
         return;
      }
   }

   strncpy (header.filetype, "PODVIS!", sizeof (header.filetype));
   header.vistable_file_version = VISTABLE_VERSION;
   header.number_of_waypoints = g_iNumWaypoints;

   snprintf (filename, sizeof (filename), "%s/addons/podbot/%s/%s.pvi", g_szGameDirectory, g_sz_cv_WPT_Folder, STRING (gpGlobals->mapname));

   UTIL_ServerPrint ("Compressing & saving Visibility Table...this may take a while!\n");

   iResult = Encode (filename, (unsigned char *) &header, sizeof (VISTABLE_HDR), (unsigned char *) g_rgbyVisLUT, MAX_WAYPOINTS * (MAX_WAYPOINTS / 4) * sizeof (unsigned char));

   g_bSaveVisTable = FALSE; // KWo - 24.03.2007

   if (iResult == -1)
   {
      ALERT (at_logged, "Podbot mm - ERROR: Couldn't save Visibility Table!\n");
      return;
   }

   ALERT (at_logged, "Podbot mm - Visibility Table saved...\n");
   return;
}

// initiates the visibility table (at the map start)
void InitVisTab (void)
{
   FILE *bfp = NULL;
   int iCompare;
   bool bVisTableLoaded = FALSE;
   bool bVisTableExists = FALSE;
   VISTABLE_HDR header;
   char filename[256];
   char wptfilename[256];

   if (g_iNumWaypoints == 0)
      return;

   snprintf (filename, sizeof (filename), "addons/podbot/%s/%s.pvi", g_sz_cv_WPT_Folder, STRING (gpGlobals->mapname));
   snprintf (wptfilename, sizeof (wptfilename), "addons/podbot/%s/%s.pwf", g_sz_cv_WPT_Folder, STRING (gpGlobals->mapname));

   // Does the Visibility Table exist & is newer than waypoint file ?
   if (COMPARE_FILE_TIME (filename, wptfilename, &iCompare))
   {
      if (iCompare > 0)
      {
         bVisTableExists = TRUE;
         ALERT (at_logged, "Podbot mm - Visibility Table file (pvi) exists and is newer than the waypoint file (pwf).\n");
      }
   }

   if (bVisTableExists)
   {
      // Now build the real filename
      snprintf (filename, sizeof (filename), "%s/addons/podbot/%s/%s.pvi", g_szGameDirectory, g_sz_cv_WPT_Folder, STRING (gpGlobals->mapname));
      bfp = fopen (filename, "rb");

      // if file exists, read the experience Data from it
      if (bfp != NULL)
      {
         fread (&header, sizeof (VISTABLE_HDR), 1, bfp);
         fclose (bfp);

         header.filetype[7] = 0;

         if (strcmp (header.filetype, "PODVIS!") == 0)
         {
            if ((header.vistable_file_version == VISTABLE_VERSION)
                && (header.number_of_waypoints == g_iNumWaypoints))
            {
               ALERT (at_logged, "Podbot mm - Loading & decompressing Visibility Table\n");

               Decode (filename, sizeof (VISTABLE_HDR), (unsigned char *) g_rgbyVisLUT, MAX_WAYPOINTS * (MAX_WAYPOINTS / 4) * sizeof (unsigned char));
               bVisTableLoaded = TRUE;
            }
            else if (header.number_of_waypoints != g_iNumWaypoints) // KWo - 07.10.2010
            {
               ALERT (at_logged, "Podbot mm - Number of waypoints in the pwf file different than in the pvi file - pvi file not loaded.\n");
            }
            else // KWo - 07.10.2010
            {
               ALERT (at_logged, "Podbot mm - Wrong Visibility Table Version in the file - pvi file not loaded.\n");
            }
         }
         else // KWo - 07.10.2010
         {
            ALERT (at_logged, "Podbot mm - Wrong header type of Visibility Table file - pvi file not loaded.\n");
         }
      }
   }

   if (!bVisTableLoaded)
   {
      ALERT (at_logged, "Podbot mm - No Visibility Table File or old one - starting the new one!\n");

      memset (g_rgbyVisLUT, 0, sizeof (g_rgbyVisLUT));

      g_iCurrVisIndex = 0;
      g_iRowVisIndex = 0; // KWo - 10.09.2006
      g_bRecalcVis = TRUE;
      g_fTimeDisplayVisTableMsg = gpGlobals->time;
   }
   else
      ALERT (at_logged, "Podbot mm - Visibility Table loaded from File...\n");

   return;
}

// initiates waypoint types at pwf load
void InitWaypointTypes (void)
{
   int index;

   g_iNumTerrorPoints = 0;
   g_iNumCTPoints = 0;
   g_iNumGoalPoints = 0;
   g_iNumCampPoints = 0;
   g_iNumRescuePoints = 0; // KWo - 24.12.2006

   memset (g_rgiTerrorWaypoints, 0, sizeof (g_rgiTerrorWaypoints));
   memset (g_rgiCTWaypoints, 0, sizeof (g_rgiCTWaypoints));
   memset (g_rgiGoalWaypoints, 0, sizeof (g_rgiGoalWaypoints));
   memset (g_rgiCampWaypoints, 0, sizeof (g_rgiCampWaypoints));
   memset (g_rgiRescueWaypoints, 0, sizeof (g_rgiRescueWaypoints)); // KWo - 24.12.2006

   for (index = 0; index < g_iNumWaypoints; index++)
   {
      if (paths[index]->flags & W_FL_TERRORIST)
      {
         g_rgiTerrorWaypoints[g_iNumTerrorPoints] = index;
         g_iNumTerrorPoints++;
      }

      else if (paths[index]->flags & W_FL_COUNTER)
      {
         g_rgiCTWaypoints[g_iNumCTPoints] = index;
         g_iNumCTPoints++;
      }

      else if (paths[index]->flags & W_FL_GOAL)
      {
         g_rgiGoalWaypoints[g_iNumGoalPoints] = index;
         g_iNumGoalPoints++;
      }

      else if (paths[index]->flags & W_FL_CAMP)
      {
         g_rgiCampWaypoints[g_iNumCampPoints] = index;
         g_iNumCampPoints++;
      }

      else if (paths[index]->flags & W_FL_RESCUE) // KWo - 24.12.2006
      {
         g_rgiRescueWaypoints[g_iNumRescuePoints] = index;
         g_iNumRescuePoints++;
      }
   }

   return;
}

// initialises the path matrix
void InitPathMatrix (void)
{
   int i, j, k;
   PATH *p;

   g_pFloydDistanceMatrix = new int[g_iNumWaypoints * g_iNumWaypoints];
   g_pFloydPathMatrix = new int[g_iNumWaypoints * g_iNumWaypoints];

   for (i = 0; i < g_iNumWaypoints; i++)
   {
      for (j = 0; j < g_iNumWaypoints; j++)
      {
         *(g_pFloydDistanceMatrix + (i * g_iNumWaypoints) + j) = 999999;
         *(g_pFloydPathMatrix + (i * g_iNumWaypoints) + j) = -1;
      }
   }

   for (i = 0; i < g_iNumWaypoints; i++)
   {
      for (j = 0; j < MAX_PATH_INDEX; j++)
      {
         if (paths[i]->index[j] != -1)
         {
            *(g_pFloydDistanceMatrix + (paths[i]->iPathNumber * g_iNumWaypoints) + (paths[i]->index[j])) = paths[i]->distance[j];
            *(g_pFloydPathMatrix + (paths[i]->iPathNumber * g_iNumWaypoints) + (paths[i]->index[j])) = paths[i]->index[j];
         }
      }
   }

   for (i = 0; i < g_iNumWaypoints; i++)
      *(g_pFloydDistanceMatrix + (i * g_iNumWaypoints) + i) = 0;

   for (k = 0; k < g_iNumWaypoints; k++)
   {
      for (i = 0; i < g_iNumWaypoints; i++)
      {
         for (j = 0; j < g_iNumWaypoints; j++)
         {
            if (*(g_pFloydDistanceMatrix + (i * g_iNumWaypoints) + k) + *(g_pFloydDistanceMatrix + (k * g_iNumWaypoints) + j)
                < (*(g_pFloydDistanceMatrix + (i * g_iNumWaypoints) + j)))
            {
               *(g_pFloydDistanceMatrix + (i * g_iNumWaypoints) + j) = *(g_pFloydDistanceMatrix + (i * g_iNumWaypoints) + k) + *(g_pFloydDistanceMatrix + (k * g_iNumWaypoints) + j);
               *(g_pFloydPathMatrix + (i * g_iNumWaypoints) + j) = *(g_pFloydPathMatrix + (i * g_iNumWaypoints) + k);
            }
         }
      }
   }

   if (g_iMapType & MAP_CS)
   {
      g_pWithHostageDistMatrix = new int[g_iNumWaypoints * g_iNumWaypoints];
      g_pWithHostagePathMatrix = new int[g_iNumWaypoints * g_iNumWaypoints];

      for (i = 0; i < g_iNumWaypoints; i++)
      {
         for (j = 0; j < g_iNumWaypoints; j++)
         {
            *(g_pWithHostageDistMatrix + (i * g_iNumWaypoints) + j) = 999999;
            *(g_pWithHostagePathMatrix + (i * g_iNumWaypoints) + j) = -1;
         }
      }

      for (i = 0; i < g_iNumWaypoints; i++)
      {
         for (j = 0; j < 8; j++)
         {
            if (paths[i]->index[j] != -1)
            {
               p = paths[paths[i]->index[j]];

               if ((p->flags & W_FL_NOHOSTAGE) == 0)
               {
                  *(g_pWithHostageDistMatrix + (paths[i]->iPathNumber * g_iNumWaypoints) + (paths[i]->index[j])) = paths[i]->distance[j];
                  *(g_pWithHostagePathMatrix + (paths[i]->iPathNumber * g_iNumWaypoints) + (paths[i]->index[j])) = paths[i]->index[j];
               }
            }
         }
      }

      for (i = 0; i < g_iNumWaypoints; i++)
         *(g_pWithHostageDistMatrix + (i * g_iNumWaypoints) + i) = 0;

      for (k = 0; k < g_iNumWaypoints; k++)
      {
         for (i = 0; i < g_iNumWaypoints; i++)
         {
            for (j = 0; j < g_iNumWaypoints; j++)
            {
               if (*(g_pWithHostageDistMatrix + (i * g_iNumWaypoints) + k) + *(g_pWithHostageDistMatrix + (k * g_iNumWaypoints) + j)
                   < (*(g_pWithHostageDistMatrix + (i * g_iNumWaypoints) + j)))
               {
                  *(g_pWithHostageDistMatrix + (i * g_iNumWaypoints) + j) = *(g_pWithHostageDistMatrix + (i * g_iNumWaypoints) + k) + *(g_pWithHostageDistMatrix + (k * g_iNumWaypoints) + j);
                  *(g_pWithHostagePathMatrix + (i * g_iNumWaypoints) + j) = *(g_pWithHostagePathMatrix + (i * g_iNumWaypoints) + k);
               }
            }
         }
      }
   }

   // Free up the hostage distance matrix
   if (g_pWithHostageDistMatrix != NULL)
      delete [](g_pWithHostageDistMatrix);
   g_pWithHostageDistMatrix = NULL;

   g_cKillHistory = 0;

   return;
}


// loads a waypoint file
bool WaypointLoad (void)
{
   FILE *bfp = NULL;
   char filename[256];
   WAYPOINT_HDR header;
   int index;
   bool bOldWaypointFormat = FALSE;

   g_bMapInitialised = FALSE;
   g_bRecalcVis = FALSE;
   g_fTimeDisplayVisTableMsg = 0;
   g_bWaypointsSaved = FALSE;

   snprintf (filename, sizeof (filename), "%s/addons/podbot/%s/%s.pwf", g_szGameDirectory, g_sz_cv_WPT_Folder, STRING (gpGlobals->mapname));

   bfp = fopen (filename, "rb");
   if (bfp == NULL)
   {
      UTIL_ServerPrint ("Waypoint file %s does not exist!\n", filename);
      snprintf (g_szWaypointMessage, sizeof (g_szWaypointMessage), "Waypoint file %s does not exist!\n(you can't add Bots!)\n", filename);
      return (FALSE);
   }

   // if file exists, read the waypoint structure from it
   fread (&header, sizeof (header), 1, bfp);
   header.filetype[7] = 0;
   header.mapname[31] = 0;

   if (strcmp (header.filetype, "PODWAY!") != 0)
   {
      UTIL_ServerPrint ("%s is not a POD Bot waypoint file!\n", filename);
      snprintf (g_szWaypointMessage, sizeof (g_szWaypointMessage), "Waypoint file %s does not exist!\n", filename);
      fclose (bfp);
      return (FALSE);
   }

   if (header.waypoint_file_version != WAYPOINT_VERSION7)
   {
      if ((header.waypoint_file_version == WAYPOINT_VERSION6)
          || (header.waypoint_file_version == WAYPOINT_VERSION5))
      {
         UTIL_ServerPrint ("Old POD Bot waypoint file version (V%d)!\nTrying to convert...\n", header.waypoint_file_version);
         bOldWaypointFormat = TRUE;
      }
      else
      {
         UTIL_ServerPrint ("%s Incompatible POD Bot waypoint file version!\nWaypoints not loaded!\n",filename);
         snprintf (g_szWaypointMessage, sizeof (g_szWaypointMessage), "%s Incompatible POD Bot waypoint file version!\nWaypoints not loaded!\n", filename);
         fclose (bfp);
         return (FALSE);
      }
   }

   if (strcmp (header.mapname, STRING (gpGlobals->mapname)) != 0)
   {
      UTIL_ServerPrint ("%s POD Bot waypoints are not for this map!\n", filename);
      snprintf (g_szWaypointMessage, sizeof (g_szWaypointMessage), "%s POD Bot waypoints are not for this map!\n", filename);
      fclose (bfp);
      return (FALSE);
   }

   g_iNumWaypoints = header.number_of_waypoints;

   // read and add waypoint paths...
   for (index = 0; index < g_iNumWaypoints; index++)
   {
      // Oldest Format to convert
      if (header.waypoint_file_version == WAYPOINT_VERSION5)
      {
         PATH5 convpath;

         paths[index] = new PATH;

         // read 1 oldpath
         fread (&convpath, SIZEOF_PATH5, 1, bfp);

         // Convert old to new
         paths[index]->iPathNumber = convpath.iPathNumber;
         paths[index]->flags = convpath.flags;
         paths[index]->origin = convpath.origin;
         paths[index]->Radius = convpath.Radius;
         paths[index]->fcampstartx = convpath.fcampstartx;
         paths[index]->fcampstarty = convpath.fcampstarty;
         paths[index]->fcampendx = convpath.fcampendx;
         paths[index]->fcampendy = convpath.fcampendy;
         paths[index]->index[0] = convpath.index[0];
         paths[index]->index[1] = convpath.index[1];
         paths[index]->index[2] = convpath.index[2];
         paths[index]->index[3] = convpath.index[3];
         paths[index]->index[4] = -1;
         paths[index]->index[5] = -1;
         paths[index]->index[6] = -1;
         paths[index]->index[7] = -1;
         paths[index]->distance[0] = convpath.distance[0];
         paths[index]->distance[1] = convpath.distance[1];
         paths[index]->distance[2] = convpath.distance[2];
         paths[index]->distance[3] = convpath.distance[3];
         paths[index]->distance[4] = 0;
         paths[index]->distance[5] = 0;
         paths[index]->distance[6] = 0;
         paths[index]->distance[7] = 0;
         paths[index]->connectflag[0] = 0;
         paths[index]->connectflag[1] = 0;
         paths[index]->connectflag[2] = 0;
         paths[index]->connectflag[3] = 0;
         paths[index]->connectflag[4] = 0;
         paths[index]->connectflag[5] = 0;
         paths[index]->connectflag[6] = 0;
         paths[index]->connectflag[7] = 0;
         paths[index]->vecConnectVel[0] = g_vecZero;
         paths[index]->vecConnectVel[1] = g_vecZero;
         paths[index]->vecConnectVel[2] = g_vecZero;
         paths[index]->vecConnectVel[3] = g_vecZero;
         paths[index]->vecConnectVel[4] = g_vecZero;
         paths[index]->vecConnectVel[5] = g_vecZero;
         paths[index]->vecConnectVel[6] = g_vecZero;
         paths[index]->vecConnectVel[7] = g_vecZero;
      }
      else if (header.waypoint_file_version == WAYPOINT_VERSION6)
      {
         PATH6 convpath;

         paths[index] = new PATH;

         // read 1 oldpath
         fread (&convpath, SIZEOF_PATH6, 1, bfp);

         // Convert old to new
         paths[index]->iPathNumber = convpath.iPathNumber;
         paths[index]->flags = convpath.flags;
         paths[index]->origin = convpath.origin;
         paths[index]->Radius = convpath.Radius;
         paths[index]->fcampstartx = convpath.fcampstartx;
         paths[index]->fcampstarty = convpath.fcampstarty;
         paths[index]->fcampendx = convpath.fcampendx;
         paths[index]->fcampendy = convpath.fcampendy;
         paths[index]->index[0] = convpath.index[0];
         paths[index]->index[1] = convpath.index[1];
         paths[index]->index[2] = convpath.index[2];
         paths[index]->index[3] = convpath.index[3];
         paths[index]->index[4] = convpath.index[4];
         paths[index]->index[5] = convpath.index[5];
         paths[index]->index[6] = convpath.index[6];
         paths[index]->index[7] = convpath.index[7];
         paths[index]->distance[0] = convpath.distance[0];
         paths[index]->distance[1] = convpath.distance[1];
         paths[index]->distance[2] = convpath.distance[2];
         paths[index]->distance[3] = convpath.distance[3];
         paths[index]->distance[4] = convpath.distance[4];
         paths[index]->distance[5] = convpath.distance[5];
         paths[index]->distance[6] = convpath.distance[6];
         paths[index]->distance[7] = convpath.distance[7];
         paths[index]->connectflag[0] = 0;
         paths[index]->connectflag[1] = 0;
         paths[index]->connectflag[2] = 0;
         paths[index]->connectflag[3] = 0;
         paths[index]->connectflag[4] = 0;
         paths[index]->connectflag[5] = 0;
         paths[index]->connectflag[6] = 0;
         paths[index]->connectflag[7] = 0;
         paths[index]->vecConnectVel[0] = g_vecZero;
         paths[index]->vecConnectVel[1] = g_vecZero;
         paths[index]->vecConnectVel[2] = g_vecZero;
         paths[index]->vecConnectVel[3] = g_vecZero;
         paths[index]->vecConnectVel[4] = g_vecZero;
         paths[index]->vecConnectVel[5] = g_vecZero;
         paths[index]->vecConnectVel[6] = g_vecZero;
         paths[index]->vecConnectVel[7] = g_vecZero;
      }
      else
      {
         paths[index] = new PATH;
         fread (paths[index], SIZEOF_PATH, 1, bfp); // read the number of paths from this node...
      }
   }

   fclose (bfp);

   for (index = 0; index < g_iNumWaypoints; index++)
      paths[index]->next = paths[index + 1];
   paths[index - 1]->next = NULL;

   snprintf (g_szWaypointMessage, sizeof (g_szWaypointMessage), "Waypoints created by %s\n", header.creatorname);

   InitWaypointTypes ();
   InitPathMatrix ();

   g_bWaypointsChanged = FALSE;

   return (TRUE);
}

// saves the wpf file
void WaypointSave (void)
{
   FILE *bfp = NULL;
   char filename[256];
   WAYPOINT_HDR header;
   int i;
   PATH *p;

   g_bWaypointsChanged = TRUE;

   strncpy (header.filetype, "PODWAY!", sizeof (header.filetype));
   header.waypoint_file_version = WAYPOINT_VERSION7;
   header.number_of_waypoints = g_iNumWaypoints;

   memset (header.mapname, 0, sizeof (header.mapname));
   memset (header.creatorname, 0, sizeof (header.creatorname));
   strncpy (header.mapname, STRING (gpGlobals->mapname), sizeof (header.mapname));
   header.mapname[31] = 0;
   strncpy (header.creatorname, STRING (pHostEdict->v.netname), sizeof (header.creatorname));

   snprintf (filename, sizeof (filename), "%s/addons/podbot/%s/%s.pwf", g_szGameDirectory, g_sz_cv_WPT_Folder, header.mapname);

   bfp = fopen (filename, "wb");

   if (bfp == NULL)
   {
      UTIL_ServerPrint ("Error opening .pwf file for writing! Waypoints NOT saved!\n");
      return;
   }

   // write the waypoint header to the file...
   fwrite (&header, sizeof (header), 1, bfp);

   p = paths[0];

   // save the waypoint paths...
   for (i = 0; i < g_iNumWaypoints; i++)
   {
      fwrite (p, SIZEOF_PATH, 1, bfp);
      p = p->next;
   }

   fclose (bfp);

   g_bWaypointsSaved = TRUE;
   return;
}

// calculates the visibility table
void WaypointCalcVisibility (void)
{
   TraceResult tr;
   unsigned char byRes;
   unsigned char byShift;
   Vector vecDest;
   int i,j;

   if ((g_iCurrVisIndex < 0) || (g_iCurrVisIndex > g_iNumWaypoints))
      g_iCurrVisIndex = 0;

   if ((g_iRowVisIndex < 0) || (g_iRowVisIndex > g_iNumWaypoints))
      g_iRowVisIndex = 0;

   Vector vecSourceDuck = paths[g_iCurrVisIndex]->origin;
   Vector vecSourceStand = paths[g_iCurrVisIndex]->origin;

   if (paths[g_iCurrVisIndex]->flags & W_FL_CROUCH)
   {
      vecSourceDuck.z += 12.0;
      vecSourceStand.z += 18.0 + 28.0;
   }
   else
   {
      vecSourceDuck.z += -18.0 + 12.0;
      vecSourceStand.z += 28.0;
   }

   j = g_iRowVisIndex + 200; // KWo - 10.09.2006
   if (j > g_iNumWaypoints) // KWo - 10.09.2006
      j = g_iNumWaypoints;


   for (i = g_iRowVisIndex; i < j; i++)  // KWo - 10.09.2006
   {
      // First check ducked Visibility

      vecDest = paths[i]->origin;

      TRACE_LINE (vecSourceDuck, vecDest, ignore_monsters, NULL, &tr);

      // check if line of sight to object is not blocked (i.e. visible)
      if (tr.flFraction != 1.0)
         byRes = 1;
      else
         byRes = 0;
      byRes <<= 1;

      TRACE_LINE (vecSourceStand, vecDest, ignore_monsters, NULL, &tr);

      // check if line of sight to object is not blocked (i.e. visible)
      if (tr.flFraction != 1.0)
         byRes |= 1;

      if (byRes != 0)
      {
         vecDest = paths[i]->origin;
         // First check ducked Visibility
         if (paths[i]->flags & W_FL_CROUCH)
            vecDest.z += 18.0 + 28.0;
         else
            vecDest.z += 28.0;

         TRACE_LINE (vecSourceDuck, vecDest, ignore_monsters, NULL, &tr);

         // check if line of sight to object is not blocked (i.e. visible)
         if (tr.flFraction != 1.0)
            byRes |= 2;
         else
            byRes &= 1;

         TRACE_LINE (vecSourceStand, vecDest, ignore_monsters, NULL, &tr);

         // check if line of sight to object is not blocked (i.e. visible)
         if (tr.flFraction != 1.0)
            byRes |= 1;
         else
            byRes &= 2;

      }

      byShift = (i % 4) << 1;
      g_rgbyVisLUT[g_iCurrVisIndex][i >> 2] &= ~(3 << byShift);
      g_rgbyVisLUT[g_iCurrVisIndex][i >> 2] |= byRes << byShift;
   }

   if (j == g_iNumWaypoints) // KWo - 10.09.2006
   {
      g_iRowVisIndex = 0;
      g_iCurrVisIndex++;
   }
   else
   {
      g_iRowVisIndex += 200; // KWo - 10.09.2006
   }


   if ((g_fTimeDisplayVisTableMsg > 0) && (g_fTimeDisplayVisTableMsg < gpGlobals->time) && (j == g_iNumWaypoints))
   {
      UTIL_HostPrint ("Visibility Table out of Date. Rebuilding... (%d%%)\n", (g_iCurrVisIndex * 100) / g_iNumWaypoints);
      g_fTimeDisplayVisTableMsg = gpGlobals->time + 0.5;
   }

   if ((g_iCurrVisIndex == g_iNumWaypoints) && (j == g_iNumWaypoints))
   {
      UTIL_HostPrint ("Done rebuilding Visibility Table.\n");
      g_bRecalcVis = FALSE;
      g_fTimeDisplayVisTableMsg = 0;
      g_bSaveVisTable = TRUE; // KWo - 24.03.2007
   }

   return;
}

// refreshes waypoints under WP editor (to let waypointer see them)
void WaypointThink (void)
{
   PATH *pPath;         // KWo - 05.01.2008
   PATH *pCachedPath;   // KWo - 05.01.2008
   PATH *pFacedPath;    // KWo - 05.01.2008
   float distance, min_distance;
   Vector start, end;
   int i, index = 0; // KWo - to remove warning uninitialised
   bool bNodeVisible;    // KWo - 05.01.2008
   char szMessage[2048];
   float fRadCircle;
   Vector vRadiusStart;
   Vector vRadiusEnd;
   Vector v_direction;
   bool isJumpWaypoint;
   int iNodeFrom;
   int iNodeTo;
   int iWaypointWidth = 30;

   // check if it's time to add jump waypoint
   if (g_bLearnJumpWaypoint) // KWo - 05.01.2008 (moved here, like in YapB)
   {
      if (!g_bEndJumpPoint)
      {
         if (pHostEdict->v.button & IN_JUMP)
         {
            WaypointAdd (WAYPOINT_ADD_JUMP_START);
            g_fTimeJumpStarted = gpGlobals->time;
            g_bEndJumpPoint = TRUE;
         }
         else
         {
            g_vecLearnVelocity = pHostEdict->v.velocity;
            g_vecLearnPos = pHostEdict->v.origin;
         }
      }

      else if (((pHostEdict->v.flags & FL_ONGROUND) || (pHostEdict->v.movetype == MOVETYPE_FLY))
               && (g_fTimeJumpStarted + 0.1 < gpGlobals->time) && g_bEndJumpPoint)
      {
         WaypointAdd (WAYPOINT_ADD_JUMP_END);
         g_bLearnJumpWaypoint = FALSE;
         g_bEndJumpPoint = FALSE;
         UTIL_HostPrint ("Observation Check off\n");
         if (g_b_cv_UseSpeech)
            SERVER_COMMAND ("speak \"movement check over\"\n");
      }
   }

   // is auto waypoint on?
   if ((g_bAutoWaypoint) && (pHostEdict->v.flags & (FL_ONGROUND | FL_PARTIALGROUND))) // KWo - 05.01.2008 (like in YapB)
   {
      // find the distance from the last used waypoint
      distance = (g_vecLastWaypoint - pHostEdict->v.origin).Length ();

      if (distance > 128)
      {
         min_distance = 9999.0;

         // check that no other reachable waypoints are nearby...
         for (i = 0; i < g_iNumWaypoints; i++)
         {
            if (WaypointReachableByEnt (pHostEdict->v.origin, paths[i]->origin, pHostEdict)) // KWo - 30.07.2006
            {
               distance = (paths[i]->origin - pHostEdict->v.origin).Length ();

               if (distance < min_distance)
                  min_distance = distance;
            }
         }

         // make sure nearest waypoint is far enough away...
         if (min_distance >= 128)
            WaypointAdd (WAYPOINT_ADD_NORMAL); // place a waypoint here
      }
   }

   min_distance = 9999.0;

   // START - Make waypoint bigger when pointed at code
   iNodeFrom = WaypointFindNearest ();
   iNodeTo = -1;

   if (iNodeFrom != -1)
   {
      iNodeTo = WaypointLookAt(); // KWo - 04.10.2006
   }
   // END - Make waypoint bigger when pointed at code

// draw waypoints
   for (i = 0; i < g_iNumWaypoints; i++)
   {
      distance = (paths[i]->origin - pHostEdict->v.origin).Length ();

      if (distance < 500)
      {
         if (distance < min_distance)
         {
            index = i; // store index of nearest waypoint
            min_distance = distance;
         }

         if (g_fWPDisplayTime[i] < gpGlobals->time)
         {
            if (iNodeTo == i)
               iWaypointWidth = 72; // KWo - 05.01.2008
            else
               iWaypointWidth = 36; // KWo - 05.01.2008

            if (paths[i]->flags & W_FL_CROUCH)
            {
               start = paths[i]->origin - Vector (0, 0, 17);
               end = start + Vector (0, 0, 34);
            }
            else
            {
               start = paths[i]->origin - Vector (0, 0, 34);
               end = start + Vector (0, 0, 68);
            }
            // KWo - 05.01.2008 - trying to fire up less tracelines...
            bNodeVisible = false;
            if (FVisible (start, pHostEdict))
               bNodeVisible = true;
            if (!bNodeVisible)
               if (FVisible (end, pHostEdict))
                  bNodeVisible = true;
            if (!bNodeVisible)
               if (FVisible (paths[i]->origin, pHostEdict))
                  bNodeVisible = true;

            if (!bNodeVisible)
               continue;

            if (paths[i]->flags & W_FL_GOAL)
               WaypointDrawBeam (start, end, iWaypointWidth, 128, 0, 255);

            else if (paths[i]->flags & W_FL_LADDER)
               WaypointDrawBeam (start, end, iWaypointWidth, 255, 0, 255);

            else if (paths[i]->flags & W_FL_RESCUE)
               WaypointDrawBeam (start, end, iWaypointWidth, 255, 255, 255);

            else if (paths[i]->flags & W_FL_CAMP)
            {
               if (paths[i]->flags & W_FL_TERRORIST)
                  WaypointDrawBeam (start, end, iWaypointWidth, 255, 160, 160);
               else if (paths[i]->flags & W_FL_COUNTER)
                  WaypointDrawBeam (start, end, iWaypointWidth, 255, 160, 255);
               else
                  WaypointDrawBeam (start, end, iWaypointWidth, 0, 255, 255);
            }

            else
            {
               if (paths[i]->flags & W_FL_TERRORIST)
                  WaypointDrawBeam (start, end, iWaypointWidth, 255, 0, 0);
               else if (paths[i]->flags & W_FL_COUNTER)
                  WaypointDrawBeam (start, end, iWaypointWidth, 0, 0, 255);
               else
                  WaypointDrawBeam (start, end, iWaypointWidth, 0, 192, 0);
            }

            if (iWaypointWidth > 36) // show the faced waypoint
            {
               WaypointDrawBeam (end + Vector (-8, 0, 3), end + Vector (8, 0, 3), 30, 0, 0, 255);
               WaypointDrawBeam (end + Vector (0, -8, 3), end + Vector (0, 8, 3), 30, 0, 0, 255);
            }

            if (g_bShowWpFlags)
            {
               if (paths[i]->flags & W_FL_NOHOSTAGE)
               {
                  WaypointDrawBeam (end + Vector (-8, 0, 0), end + Vector (8, 0, 0), 30, 255, 0, 0);
                  WaypointDrawBeam (end + Vector (0, -8, 0), end + Vector (0, 8, 0), 30, 255, 0, 0);
               }

               if (paths[i]->flags & W_FL_USE_BUTTON)
               {
                  WaypointDrawBeam (end + Vector (-8, 0, -3), end + Vector (8, 0, -3), 30, 0, 255, 0);
                  WaypointDrawBeam (end + Vector (0, -8, -3), end + Vector (0, 8, -3), 30, 0, 255, 0);
               }

               if (paths[i]->flags & W_FL_LIFT)
               {
                  WaypointDrawBeam (end + Vector (-8, 0, -6), end + Vector (8, 0, -3), 30, 255, 0, 255);
                  WaypointDrawBeam (end + Vector (0, -8, -6), end + Vector (0, 8, -3), 30, 255, 0, 255);
               }
            }
            g_fWPDisplayTime[i] = gpGlobals->time + 0.2; // KWo - 11.02.2008
         }
      }
   }

   if (index == -1) // KWo - 05.01.2008
      return;

   // check if player is close enough to a waypoint and time to draw path and danger...
   if ((min_distance < 50) && (g_fPathDisplayTime[index] < gpGlobals->time)) // KWo - 05.01.2008 - based on YapB code - new style of info
   {
      g_fPathDisplayTime[index] = gpGlobals->time + 0.2;
      pPath = paths[index];

// draw paths
      if (pPath->flags & W_FL_CAMP)
      {
         if (pPath->flags & W_FL_CROUCH)
            start = pPath->origin - Vector (0, 0, 17) + Vector (0, 0, 34);
         else
            start = pPath->origin - Vector (0, 0, 34) + Vector (0, 0, 68);

         MAKE_VECTORS (Vector (pPath->fcampstartx, pPath->fcampstarty, 0));
         end = pPath->origin + gpGlobals->v_forward * 500;
         WaypointDrawBeam (start, end, 30, 255, 192, 0);

         MAKE_VECTORS (Vector (pPath->fcampendx, pPath->fcampendy, 0));
         end = pPath->origin + gpGlobals->v_forward * 500;
         WaypointDrawBeam (start, end, 30, 192, 96, 0);
      }

      for (i = 0; i < MAX_PATH_INDEX; i++)
      {
         if (pPath->index[i] != -1)
         {
            if (pPath->connectflag[i] & C_FL_JUMP)
               WaypointDrawBeam (pPath->origin, paths[pPath->index[i]]->origin, 10, 255, 0, 0);

            // If 2-way connection draw a yellow line to this index's waypoint
            else if (ConnectedToWaypoint (pPath->index[i], pPath->iPathNumber))
               WaypointDrawBeam (pPath->origin, paths[pPath->index[i]]->origin, 10, 255, 255, 0);

            // draw a white line to this index's waypoint
            else
               WaypointDrawBeam (pPath->origin, paths[pPath->index[i]]->origin, 10, 255, 255, 255);
         }
      }

      // now look for one-way incoming connections
      for (i = 0; i < g_iNumWaypoints; i++)
         if (ConnectedToWaypoint (paths[i]->iPathNumber, pPath->iPathNumber)
             && !ConnectedToWaypoint (pPath->iPathNumber, paths[i]->iPathNumber))
            WaypointDrawBeam (pPath->origin, paths[i]->origin, 10, 0, 192, 96);

      v_direction = g_vecZero;

      for (fRadCircle = 0.0; fRadCircle <= 180.0; fRadCircle += 22.5)
      {
         MAKE_VECTORS (v_direction);
         vRadiusStart = pPath->origin - (gpGlobals->v_forward * pPath->Radius);
         vRadiusEnd = pPath->origin + (gpGlobals->v_forward * pPath->Radius);
         WaypointDrawBeam (vRadiusStart, vRadiusEnd, 30, 0, 0, 255);

         v_direction.y = fRadCircle;
      }

      if (!g_bWaypointsChanged)
      {
         // draw a red line to this index's danger point
         if ((pBotExperienceData + (index * g_iNumWaypoints) + index)->iTeam0_danger_index != -1)
         {
            WaypointDrawBeam (paths[index]->origin,
                              paths[(pBotExperienceData + (index * g_iNumWaypoints) + index)->iTeam0_danger_index]->origin,
                              75, 128, 0, 0);
         }

         // draw a blue line to this index's danger point
         if ((pBotExperienceData + (index * g_iNumWaypoints) + index)->iTeam1_danger_index != -1)
         {
            WaypointDrawBeam (paths[index]->origin,
                              paths[(pBotExperienceData + (index * g_iNumWaypoints) + index)->iTeam1_danger_index]->origin,
                              75, 0, 0, 192);
         }
      }

// draw some info
      isJumpWaypoint = FALSE;
      for (i = 0; i < MAX_PATH_INDEX; i++)
         if ((pPath->index[i] != -1) && (pPath->connectflag[i] & C_FL_JUMP))
            isJumpWaypoint = TRUE;

      int iLength = sprintf (szMessage,
                    "\n"
                    "\n"
                    "\n"
                    "\n"
                    "Current Waypoint Information:\n"
                    " Index Nr.: %d of %d, Radius: %d\n"
                    " Flags:%s%s%s%s%s%s%s%s%s%s%s%s%s\n"
                    " Origin: [%d, %d, %d]\n",
                    index,
                    g_iNumWaypoints,
                    (int) pPath->Radius,
                    (((pPath->flags == 0) && (!isJumpWaypoint))? " none" : ""),
                    (pPath->flags & W_FL_USE_BUTTON ? " | USE_BUTTON" : ""),
                    (pPath->flags & W_FL_LIFT ? " | LIFT" : ""),
                    (pPath->flags & W_FL_CROUCH ? " | CROUCH" : ""),
                    (pPath->flags & W_FL_CROSSING ? " | CROSSING" : ""),
                    (pPath->flags & W_FL_GOAL ? " | GOAL" : ""),
                    (pPath->flags & W_FL_LADDER ? " | LADDER" : ""),
                    (pPath->flags & W_FL_RESCUE ? " | RESCUE" : ""),
                    (pPath->flags & W_FL_CAMP ? " | CAMP" : ""),
                    (pPath->flags & W_FL_NOHOSTAGE ? " | NOHOSTAGE" : ""),
                    (pPath->flags & W_FL_TERRORIST ? " | TERRORIST" : ""),
                    (pPath->flags & W_FL_COUNTER ? " | COUNTER" : ""),
                    (isJumpWaypoint ? " | JUMP" : ""),
                    (int) pPath->origin.x,
                    (int) pPath->origin.y,
                    (int) pPath->origin.z);

      // if waypoint is not changed display experience also
      if (!g_bWaypointsChanged)
      {
         int iDangerIndexCT = (pBotExperienceData + index * g_iNumWaypoints + index)->iTeam1_danger_index;
         int iDangerIndexT = (pBotExperienceData + index * g_iNumWaypoints + index)->iTeam0_danger_index;

         iLength += sprintf (&szMessage[iLength],
            " Experience Info [(DWP for CWP/EXP),(HDWP/HD)] :\n"
            "  CT: (%d / %u) , (%d / %d)\n"
            "  T:  (%d / %u) , (%d / %d)\n",
            iDangerIndexCT,
            iDangerIndexCT != -1 ? (pBotExperienceData + index * g_iNumWaypoints + iDangerIndexCT)->uTeam1Damage : 0,
            g_iHighestDamageWpCT,
            g_iHighestDamageCT,
            iDangerIndexT,
            iDangerIndexT != -1 ? (pBotExperienceData + index * g_iNumWaypoints + iDangerIndexT)->uTeam0Damage : 0,
            g_iHighestDamageWpT,
            g_iHighestDamageT);
      }

      // check if we need to show the cached point index
      if (g_iCachedWaypoint != -1)
      {
         isJumpWaypoint = FALSE;
         pCachedPath = paths[g_iCachedWaypoint];
         for (i = 0; i < MAX_PATH_INDEX; i++)
            if ((pCachedPath->index[i] != -1) && (pCachedPath->connectflag[i] & C_FL_JUMP))
               isJumpWaypoint = TRUE;

         iLength += sprintf (&szMessage[iLength],
            "\n"
            "Cached Waypoint Information:\n"
            " Index Nr.: %d of %d, Radius: %d\n"
/*            " Flags:%s%s%s%s%s%s%s%s%s%s%s%s%s\n" */
            " Origin: [%d, %d, %d]\n",
            g_iCachedWaypoint,
            g_iNumWaypoints,
            (int) pCachedPath->Radius,
/*            (((pCachedPath->flags == 0) && (!isJumpWaypoint)) ? " none" : ""),
            (pCachedPath->flags & W_FL_USE_BUTTON ? " | USE_BUTTON" : ""),
            (pCachedPath->flags & W_FL_LIFT ? " | LIFT" : ""),
            (pCachedPath->flags & W_FL_CROUCH ? " | CROUCH" : ""),
            (pCachedPath->flags & W_FL_CROSSING ? " | CROSSING" : ""),
            (pCachedPath->flags & W_FL_GOAL ? " | GOAL" : ""),
            (pCachedPath->flags & W_FL_LADDER ? " | LADDER" : ""),
            (pCachedPath->flags & W_FL_RESCUE ? " | RESCUE" : ""),
            (pCachedPath->flags & W_FL_CAMP ? " | CAMP" : ""),
            (pCachedPath->flags & W_FL_NOHOSTAGE ? " | NOHOSTAGE" : ""),
            (pCachedPath->flags & W_FL_TERRORIST ? " | TERRORIST" : ""),
            (pCachedPath->flags & W_FL_COUNTER ? " | COUNTER" : ""),
            (isJumpWaypoint ? " | JUMP" : ""), */
            (int) pCachedPath->origin.x,
            (int) pCachedPath->origin.y,
            (int) pCachedPath->origin.z);
      }

      // check if we need to show the facing point index
      if (iNodeTo != -1)
      {
         isJumpWaypoint = FALSE;
         pFacedPath = paths[iNodeTo];
         for (i = 0; i < MAX_PATH_INDEX; i++)
            if ((pFacedPath->index[i] != -1) && (pFacedPath->connectflag[i] & C_FL_JUMP))
               isJumpWaypoint = TRUE;

         iLength += sprintf (&szMessage[iLength],
            "\n"
            "Faced Waypoint Information:\n"
            " Index Nr.: %d of %d, Radius: %d\n"
/*            " Flags:%s%s%s%s%s%s%s%s%s%s%s%s%s\n", */
            " Origin: [%d, %d, %d]\n",
            iNodeTo,
            g_iNumWaypoints,
            (int) pFacedPath->Radius,
/*            (((pFacedPath->flags == 0) && (!isJumpWaypoint)) ? " none" : ""),
            (pFacedPath->flags & W_FL_USE_BUTTON ? " | USE_BUTTON" : ""),
            (pFacedPath->flags & W_FL_LIFT ? " | LIFT" : ""),
            (pFacedPath->flags & W_FL_CROUCH ? " | CROUCH" : ""),
            (pFacedPath->flags & W_FL_CROSSING ? " | CROSSING" : ""),
            (pFacedPath->flags & W_FL_GOAL ? " | GOAL" : ""),
            (pFacedPath->flags & W_FL_LADDER ? " | LADDER" : ""),
            (pFacedPath->flags & W_FL_RESCUE ? " | RESCUE" : ""),
            (pFacedPath->flags & W_FL_CAMP ? " | CAMP" : ""),
            (pFacedPath->flags & W_FL_NOHOSTAGE ? " | NOHOSTAGE" : ""),
            (pFacedPath->flags & W_FL_TERRORIST ? " | TERRORIST" : ""),
            (pFacedPath->flags & W_FL_COUNTER ? " | COUNTER" : ""),
            (isJumpWaypoint ? " | JUMP" : "") */
            (int) pFacedPath->origin.x,
            (int) pFacedPath->origin.y,
            (int) pFacedPath->origin.z);
      }

      if (iLength > 485)
      {
         szMessage[485] = '\n';
         szMessage[486] = '\0';
//         ALERT(at_logged,"[DEBUG] WP info message too long = %d.\n", iLength);
         for (i = 487; i < 2047; i++)
         {
            szMessage[i] = 0;
         }
//         return;
      }
// KWo - 16.01.2010 -
      g_hudset.x = 0.033;
      g_hudset.y = 0.061;
//      g_hudset.a1 = 1;
      g_hudset.holdTime = 1.055;
      g_hudset.channel = 1;

      UTIL_HudMessage(pHostEdict, g_hudset, szMessage);

/*
      MESSAGE_BEGIN (MSG_ONE_UNRELIABLE, SVC_TEMPENTITY, NULL, pHostEdict);
      WRITE_BYTE (TE_TEXTMESSAGE);
      WRITE_BYTE (1); // channel
      WRITE_SHORT (270); // x coordinates * 8192 // KWo - 06.01.2008
      WRITE_SHORT (500); // y coordinates * 8192 // KWo - 06.01.2008
      WRITE_BYTE (0); // effect (fade in/out)
      WRITE_BYTE (255); // initial RED
      WRITE_BYTE (255); // initial GREEN
      WRITE_BYTE (255); // initial BLUE
      WRITE_BYTE (1); // initial ALPHA
      WRITE_BYTE (255); // effect RED
      WRITE_BYTE (255); // effect GREEN
      WRITE_BYTE (255); // effect BLUE
      WRITE_BYTE (1); // effect ALPHA
      WRITE_SHORT (0); // fade-in time in seconds * 256
      WRITE_SHORT (0); // fade-out time in seconds * 256
      WRITE_SHORT (270); // hold time in seconds * 256 // KWo - 05.01.2007
      WRITE_STRING (szMessage); // write message
      MESSAGE_END (); // end
*/
   }
   return;
}

// checks if the waypoint is connected somewhere
bool WaypointIsConnected (int iNum)
{
   int i, n;

   for (i = 0; i < g_iNumWaypoints; i++)
      if (i != iNum)
         for (n = 0; n < MAX_PATH_INDEX; n++)
            if (paths[i]->index[n] == iNum)
               return (TRUE);

   return (FALSE);
}

// checks if all nodes are valid - everything connected
bool WaypointNodesValid (void)
{
   TraceResult tr;
//   PATHNODE *pStartNode, *pDelNode;
//   bool bPathValid;
   int iTPoints = 0;
   int iCTPoints = 0;
   int iGoalPoints = 0;
   int iRescuePoints = 0;
   int iConnections;
   int i, j, n, x;

   for (i = 0; i < g_iNumWaypoints; i++)
   {
      iConnections = 0;

      for (n = 0; n < MAX_PATH_INDEX; n++)
      {
         if (paths[i]->index[n] != -1)
         {
            iConnections++;
            break;
         }
      }

      if (iConnections == 0)
      {
         if (!WaypointIsConnected (i))
         {
            UTIL_HostPrint ("Node %d not connected to any Waypoint!\n", i);
            EMIT_SOUND_DYN2 (pHostEdict, CHAN_WEAPON, "debris/bustglass1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
            TRACE_HULL (paths[i]->origin + Vector (0, 0, 32),
                        paths[i]->origin + Vector (0, 0, -32),
                        ignore_monsters, human_hull, pHostEdict, &tr);
            SET_ORIGIN (pHostEdict, tr.vecEndPos);
            return (FALSE);
         }
      }

      if (paths[i]->iPathNumber != i)
      {
         UTIL_HostPrint ("Node %d Pathnumber differs from Index!\n", i);
         EMIT_SOUND_DYN2 (pHostEdict, CHAN_WEAPON, "debris/bustglass1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
         TRACE_HULL (paths[i]->origin + Vector (0, 0, 32),
                     paths[i]->origin + Vector (0, 0, -32),
                     ignore_monsters, human_hull, pHostEdict, &tr);
         SET_ORIGIN (pHostEdict, tr.vecEndPos);
         return (FALSE);
      }

      if ((paths[i]->next == NULL) && (i != g_iNumWaypoints - 1))
      {
         UTIL_HostPrint ("Node %d not connected!\n", i);
         EMIT_SOUND_DYN2 (pHostEdict, CHAN_WEAPON, "debris/bustglass1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
         TRACE_HULL (paths[i]->origin + Vector (0, 0, 32),
                     paths[i]->origin + Vector (0, 0, -32),
                     ignore_monsters, human_hull, pHostEdict, &tr);
         SET_ORIGIN (pHostEdict, tr.vecEndPos);
         return (FALSE);
      }

      if (paths[i]->flags & W_FL_CAMP)
      {
         if ((paths[i]->fcampstartx == 0.0) && (paths[i]->fcampstarty == 0.0))
         {
            UTIL_HostPrint ("Node %d Camp Start Position not set!\n", i);
            EMIT_SOUND_DYN2 (pHostEdict, CHAN_WEAPON, "debris/bustglass1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
            TRACE_HULL (paths[i]->origin + Vector (0, 0, 32),
                        paths[i]->origin + Vector (0, 0, -32),
                        ignore_monsters, human_hull, pHostEdict, &tr);
            SET_ORIGIN (pHostEdict, tr.vecEndPos);
            return (FALSE);
         }

         else if ((paths[i]->fcampendx == 0.0) && (paths[i]->fcampendy == 0.0))
         {
            UTIL_HostPrint ("Node %d Camp End Position not set !\n", i);
            EMIT_SOUND_DYN2 (pHostEdict, CHAN_WEAPON, "debris/bustglass1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
            TRACE_HULL (paths[i]->origin + Vector (0, 0, 32),
                        paths[i]->origin + Vector (0, 0, -32),
                        ignore_monsters, human_hull, pHostEdict, &tr);
            SET_ORIGIN (pHostEdict, tr.vecEndPos);
            return (FALSE);
         }
      }

      else if (paths[i]->flags & W_FL_TERRORIST)
         iTPoints++;
      else if (paths[i]->flags & W_FL_COUNTER)
         iCTPoints++;
      else if (paths[i]->flags & W_FL_GOAL)
         iGoalPoints++;
      else if (paths[i]->flags & W_FL_RESCUE)
         iRescuePoints++;

      for (x = 0; x < MAX_PATH_INDEX; x++)
      {
         if (paths[i]->index[x] != -1)
         {
            if ((paths[i]->index[x] >= g_iNumWaypoints) || (paths[i]->index[x] < -1))
            {
               UTIL_HostPrint ("Node %d - Path Index %d out of Range!\n", i, x);
               EMIT_SOUND_DYN2 (pHostEdict, CHAN_WEAPON, "debris/bustglass1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
               TRACE_HULL (paths[i]->origin + Vector (0, 0, 32),
                           paths[i]->origin + Vector (0, 0, -32),
                           ignore_monsters, human_hull, pHostEdict, &tr);
               SET_ORIGIN (pHostEdict, tr.vecEndPos);
               return (FALSE);
            }

            else if (paths[i]->index[x] == paths[i]->iPathNumber)
            {
               UTIL_HostPrint ("Node %d - Path Index %d points to itself!\n", i, x);
               EMIT_SOUND_DYN2 (pHostEdict, CHAN_WEAPON, "debris/bustglass1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
               TRACE_HULL (paths[i]->origin + Vector (0, 0, 32),
                           paths[i]->origin + Vector (0, 0, -32),
                           ignore_monsters, human_hull, pHostEdict, &tr);
               SET_ORIGIN (pHostEdict, tr.vecEndPos);
               return (FALSE);
            }
         }
      }
   }

   if (g_iMapType & MAP_CS)
   {
      if (iRescuePoints == 0)
      {
         UTIL_HostPrint ("You didn't set a Rescue Point!\n");
         EMIT_SOUND_DYN2 (pHostEdict, CHAN_WEAPON, "debris/bustglass1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
         return (FALSE);
      }
   }

   if (iTPoints == 0)
   {
      UTIL_HostPrint ("You didn't set any Terrorist Point!\n");
      EMIT_SOUND_DYN2 (pHostEdict, CHAN_WEAPON, "debris/bustglass1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
      return (FALSE);
   }

   else if (iCTPoints == 0)
   {
      UTIL_HostPrint ("You didn't set any Counter Point!\n");
      EMIT_SOUND_DYN2 (pHostEdict, CHAN_WEAPON, "debris/bustglass1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
      return (FALSE);
   }

   else if (iGoalPoints == 0)
   {
      UTIL_HostPrint ("You didn't set any Goal Points!\n");
      EMIT_SOUND_DYN2 (pHostEdict, CHAN_WEAPON, "debris/bustglass1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
      return (FALSE);
   }


/*
// Old Count Floyd method to initialise paths - no longer used since A* implemented by PMB
// Init Floyd Matrix to use Floyd Pathfinder
   InitPathMatrix ();

   // Now check if each and every Path is valid
   for (i = 0; i < g_iNumWaypoints; i++)
   {
      for (n = 0; n < g_iNumWaypoints; n++)
      {
         if (i == n)
            continue;

         pStartNode = FindShortestPath (i, n, &bPathValid);

         while (pStartNode != NULL)
         {
            pDelNode = pStartNode->NextNode;
            delete pStartNode;
            pStartNode = pDelNode;
         }

         pStartNode = NULL;
         pDelNode = NULL;

         // No path from A to B ?
         if (!bPathValid)
         {
            UTIL_HostPrint ("No Path from %d to %d! One or both are unconnected!\n", i, n);
            EMIT_SOUND_DYN2 (pHostEdict, CHAN_WEAPON, "debris/bustglass1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
            if (n != 0)
               TRACE_HULL (paths[n]->origin + Vector (0, 0, 32),
                           paths[n]->origin + Vector (0, 0, -32),
                           ignore_monsters, human_hull, pHostEdict, &tr);
            else
               TRACE_HULL (paths[i]->origin + Vector (0, 0, 32),
                           paths[i]->origin + Vector (0, 0, -32),
                           ignore_monsters, human_hull, pHostEdict, &tr);
            SET_ORIGIN (pHostEdict, tr.vecEndPos);
            return (FALSE);
         }
      }
   }*/

   bool rgbVisited[MAX_WAYPOINTS];
   PATHNODE *stack = NULL;

   // first check incoming connectivity
   // initialize the "visited" table
   for (i = 0; i < g_iNumWaypoints; i++)
      rgbVisited[i] = FALSE;

   // check from Waypoint nr. 0
   stack = (PATHNODE *)malloc(sizeof(PATHNODE));
   stack->NextNode = NULL;
   stack->iIndex = 0;
   int iIndex;

   while (stack)
   {
      // pop a node from the stack
      PATHNODE *current = stack;
      stack = stack->NextNode;

      rgbVisited[current->iIndex] = TRUE;

      for (j = 0; j < MAX_PATH_INDEX; j++)
      {
         iIndex = paths[current->iIndex]->index[j];
         if (rgbVisited[iIndex])
            continue; // skip this waypoint as it's already visited
         if (iIndex >= 0 && iIndex < g_iNumWaypoints)
         {
            PATHNODE *newnode = (PATHNODE *)malloc(sizeof(PATHNODE));
            newnode->NextNode = stack;
            newnode->iIndex = iIndex;
            stack = newnode;
         }
      }

      free(current);
   }

   for (i = 0; i < g_iNumWaypoints; i++)
   {
      if (!rgbVisited[i])
      {
         UTIL_HostPrint ("Path broken from Waypoint Nr. 0 to Waypoint Nr. %d!\n", i);
         EMIT_SOUND_DYN2 (pHostEdict, CHAN_WEAPON, "debris/bustglass1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
         TRACE_HULL (paths[i]->origin + Vector (0, 0, 32),
                     paths[i]->origin + Vector (0, 0, -32),
                     ignore_monsters, human_hull, pHostEdict, &tr);
         SET_ORIGIN (pHostEdict, tr.vecEndPos);
         return (FALSE);
      }
   }

   // then check outgoing connectivity
   std::vector<int> in_paths[MAX_WAYPOINTS]; // store incoming paths for speedup

   for (i = 0; i < g_iNumWaypoints; i++)
      for (j = 0; j < MAX_PATH_INDEX; j++)
         if (paths[i]->index[j] >= 0 && paths[i]->index[j] < g_iNumWaypoints)
            in_paths[paths[i]->index[j]].push_back(i);

   // initialize the "visited" table
   for (i = 0; i < g_iNumWaypoints; i++)
      rgbVisited[i] = FALSE;

   // check from Waypoint nr. 0
   stack = (PATHNODE *)malloc(sizeof(PATHNODE));
   stack->NextNode = NULL;
   stack->iIndex = 0;

   while (stack)
   {
      // pop a node from the stack
      PATHNODE *current = stack;
      stack = stack->NextNode;

      rgbVisited[current->iIndex] = TRUE;

      for (j = 0; j < (int)in_paths[current->iIndex].size(); j++)
      {
         if (rgbVisited[in_paths[current->iIndex][j]])
            continue; // skip this waypoint as it's already visited
         PATHNODE *newnode = (PATHNODE *)malloc(sizeof(PATHNODE));
         newnode->NextNode = stack;
         newnode->iIndex = in_paths[current->iIndex][j];
         stack = newnode;
      }
      free(current);
   }

   for (i = 0; i < g_iNumWaypoints; i++)
   {
      if (!rgbVisited[i])
      {
         UTIL_HostPrint ("Path broken from Waypoint Nr. %d to Waypoint Nr. 0!\n", i);
         EMIT_SOUND_DYN2 (pHostEdict, CHAN_WEAPON, "debris/bustglass1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
         TRACE_HULL (paths[i]->origin + Vector (0, 0, 32),
                     paths[i]->origin + Vector (0, 0, -32),
                     ignore_monsters, human_hull, pHostEdict, &tr);
         SET_ORIGIN (pHostEdict, tr.vecEndPos);
         return (FALSE);
      }
   }

   EMIT_SOUND_DYN2 (pHostEdict, CHAN_WEAPON, "weapons/he_bounce-1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
   return (TRUE);
}
