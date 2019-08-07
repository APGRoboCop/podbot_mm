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
// waypoint.cpp
//
// Features the Waypoint Code (Bot Navigation)

#include "bot_globals.h"
#include <vector>

// find the nearest waypoint to that Origin and return the index
int WaypointFindNearestToMove (edict_t *pEnt, Vector vOrigin)
{
   int i, index = -1;  // KWo - to remove warning uninitialised
   float fDistance;
   float fMinDistance;
   TraceResult tr;  // KWo - 19.04.2006
   fMinDistance = 600.0f;

   for (i = 0; i < g_iNumWaypoints; i++)
   {
      fDistance = (paths[i]->origin - vOrigin).Length ();
      if ((fDistance < fMinDistance) && (fabs(paths[i]->origin.z - vOrigin.z) < 40.0f)) // KWo - 14.08.2008
      {
         TRACE_LINE (vOrigin, paths[i]->origin, ignore_monsters, pEnt, &tr); // KWo - 17.04.2008
         if (tr.flFraction >= (g_vecBomb == vOrigin) ? 0.85f : 0.95f) // KWo - 05.09.2008
         {
            index = i;
            fMinDistance = fDistance;
         }
      }
   }
   if (index > -1)
      return (index); // return visible WP index KWo - 19.04.2006

   index = 0;

   fMinDistance = 9999.0f;
   for (i = 0; i < g_iNumWaypoints; i++)
   {
      fDistance = (paths[i]->origin - vOrigin).Length ();

      if (fDistance < fMinDistance)
      {
         index = i;
         fMinDistance = fDistance;
      }
   }
   return (index);
}

// Returns all Waypoints within Radius from Position
void WaypointFindInRadius (Vector vecPos, float fRadius, int *pTab, int *iCount)
{
   int i, iMaxCount;
   float distance;

   iMaxCount = *iCount;
   *iCount = 0;

   for (i = 0; i < g_iNumWaypoints; i++)
   {
      distance = (paths[i]->origin - vecPos).Length ();

      if (distance < fRadius)
      {
         *pTab++ = i;
         (*iCount)++;

         if (*iCount == iMaxCount)
            break;
      }
   }

   (*iCount)--;
   return;
}

// Returns 2D Traveltime to a Position
float GetTravelTime (float fMaxSpeed, Vector vecSource, Vector vecPosition)
{
   float fDistance = (vecPosition - vecSource).Length2D ();
   return (fDistance / fMaxSpeed);
}

// Returns bool value if the waypoint is reachable by an entity (for example by host)
bool WaypointReachableByEnt (Vector v_src, Vector v_dest, edict_t *pEntity) // KWo - 30.07.2006
{
   TraceResult tr;
   float distance = (v_dest - v_src).Length ();

   // is the destination close enough?
   if (distance < 400) // KWo - 29.07.2006
   {
      // are we in water ?
      if ((pEntity->v.waterlevel == 2) || (pEntity->v.waterlevel == 3))
      {
      // is dest waypoint higher than src? (62 is max jump height)
         if (v_dest.z > v_src.z + 62.0f)
            return (FALSE); // can't reach this one

      // is dest waypoint lower than src?
         if (v_dest.z < v_src.z - 100.0f)
            return (FALSE); // can't reach this one
      }
      // check if this waypoint is "visible"...
      TRACE_LINE (v_src, v_dest, ignore_monsters, pEntity, &tr);

      // if waypoint is visible from current position (even behind head)...
      if (tr.flFraction >= 1.0f)
         return (TRUE);
   }
   return (FALSE);
}

// Returns bool value if the waypoint is reachable for a bot
bool WaypointReachable (bot_t *pBot, int WP_Index) // KWo - 30.07.2006
{
   TraceResult tr;
   float distance;
   float distance2D;
   Vector v_src = pBot->pEdict->v.origin;
   Vector v_dest;
   edict_t *pEntity = pBot->pEdict;
   bool b_WP_IsLadder;

   if ((WP_Index < 0) || (WP_Index >= g_iNumWaypoints)) // KWo - 30.07.2006
      return (FALSE); // can't reach this one

   b_WP_IsLadder = false;
   if (paths[WP_Index]->flags & W_FL_LADDER)
      b_WP_IsLadder = true;

   v_dest = paths[WP_Index]->origin;
   distance = (v_dest - v_src).Length ();
   distance2D = (v_dest - v_src).Length2D ();


   // is the destination close enough?
   if (distance < 600.0f) // KWo - 25.02.2008
   {
      // are we in water ?
//      if ((pEntity->v.waterlevel == 2) || (pEntity->v.waterlevel == 3))
      if (pEntity->v.waterlevel < 2) // KWo - in water shouldn't be a problem to reach a WP, so check except of the water
      {
      // is dest waypoint higher than src? (62 is max jump height)
         if ((v_dest.z > v_src.z + 62.0f) && (!(b_WP_IsLadder) || (distance2D > 16.0f))) // KWo - 30.07.2006 added a check for a ladder ;)
            return (FALSE); // can't reach this one

      // is dest waypoint lower than src?
         if ((v_dest.z < v_src.z - 100.0f) && (!(b_WP_IsLadder) || (distance2D > 16.0f))) // KWo - 30.07.2006 added a check for a ladder ;)
            return (FALSE); // can't reach this one
      }
      // check if this waypoint is "visible"...
      TRACE_LINE (v_src, v_dest, ignore_monsters, pEntity, &tr);

      // if waypoint is visible from current position (even behind head)...
      if (tr.flFraction >= 1.0f)
         return (TRUE);
   }
   return (FALSE);
}

// checks if the waypoint is visible
bool WaypointIsVisible (int iSourceIndex, int iDestIndex)
{
   unsigned char byRes = g_rgbyVisLUT[iSourceIndex][iDestIndex >> 2];
   byRes >>= (iDestIndex % 4) << 1;

   return (!((byRes & 3) == 3));
}

// deletes all nodes of the way the bot should've to pass
void DeleteSearchNodes (bot_t *pBot)
{
   PATHNODE *NextNode;

   while (pBot->pWayNodesStart != NULL)
   {
      NextNode = pBot->pWayNodesStart->NextNode;
      delete (pBot->pWayNodesStart);
      pBot->pWayNodesStart = NextNode;

      pBot->i_PathDeep--; // KWo - 30.08.2006 - stack tests
      if (pBot->i_PathDeep < -999999) // KWo - 30.08.2006 - stack tests
         pBot->i_PathDeep = -999999;
   }
   pBot->pWayNodesStart = NULL;
   pBot->pWaypointNodes = NULL;
   pBot->chosengoal_index = -1; // KWo - 27.06.2006

   if (g_b_DebugNavig)
      ALERT(at_logged, "[DEBUG] DeleteSearchNodes called for bot %s.\n", pBot->name);
   return;
}

// returns the "standerd" distance between 2 waypoints
int GetPathDistance (int iSourceWaypoint, int iDestWaypoint)
{
   return (*(g_pFloydDistanceMatrix + (iSourceWaypoint * g_iNumWaypoints) + iDestWaypoint));
}

// called when the bot has to find some closest waypoint
void BotChangeWptIndex (bot_t *pBot, int iWptIndex)
{
   pBot->prev_wpt_index[4] = pBot->prev_wpt_index[3];
   pBot->prev_wpt_index[3] = pBot->prev_wpt_index[2];
   pBot->prev_wpt_index[2] = pBot->prev_wpt_index[1];
   pBot->prev_wpt_index[1] = pBot->prev_wpt_index[0];
   pBot->prev_wpt_index[0] = pBot->curr_wpt_index;

   pBot->curr_wpt_index = iWptIndex;
   pBot->f_wpt_timeset = gpGlobals->time;

   // Get current Waypoint Flags
   // FIXME: Would make more sense to store it each time
   // in the Bot struct when a Bot gets a new wpt instead doing this here
   if ((iWptIndex >= 0) && (iWptIndex < g_iNumWaypoints)) // 17.04.2016
   {
      pBot->iWPTFlags = paths[pBot->curr_wpt_index]->flags;
      pBot->wpt_origin = paths[iWptIndex]->origin;

   }
   else
      pBot->iWPTFlags = 0;

      pBot->dest_origin = pBot->wpt_origin; // KWo - 17.04.2016
   if (g_b_DebugNavig)  // KWo - 17.04.2016
   {
      ALERT(at_logged, "[DEBUG] BotChangeWptIndex - Bot %s sets the dest origin as WP origin.\n", pBot->name);
      ALERT(at_logged, "[DEBUG] BotChangeWptIndex - Bot %s changes the WP index: current = %d, previous = %d.\n",
         pBot->name, pBot->curr_wpt_index, pBot->prev_wpt_index[0]);
   }
   return;
}

// Return the most distant waypoint which is seen from the Bot to
// the Target and is within iCount
int GetAimingWaypoint (bot_t *pBot, Vector vecTargetPos, int iCount) // KWo - used nowhere...
{
   if (pBot->curr_wpt_index == -1)
      BotChangeWptIndex (pBot, WaypointFindNearestToMove (pBot->pEdict, pBot->pEdict->v.origin)); // KWo - 17.04.2008

   int iSourceIndex = pBot->curr_wpt_index;
   int iDestIndex = WaypointFindNearestToMove (pBot->pEdict, vecTargetPos); // KWo - 17.04.2008
   int iBestIndex = iSourceIndex;
   PATHNODE *pStartNode, *pNode;

   pNode = new PATHNODE;
   pNode->iIndex = iDestIndex;
   pNode->NextNode = NULL;
   pStartNode = pNode;

   while (iDestIndex != iSourceIndex)
   {
      iDestIndex = *(g_pFloydPathMatrix + (iDestIndex * g_iNumWaypoints) + iSourceIndex);

      if (iDestIndex < 0)
         break;

      pNode->NextNode = new PATHNODE;
      pNode = pNode->NextNode;
      pNode->iIndex = iDestIndex;
      pNode->NextNode = NULL;

      if (WaypointIsVisible (pBot->curr_wpt_index, iDestIndex))
      {
         iBestIndex = iDestIndex;
         break;
      }
   }

   while (pStartNode != NULL)
   {
      pNode = pStartNode->NextNode;
      delete pStartNode;
      pStartNode = pNode;
   }

   return (iBestIndex);
}


PATHNODE *FindShortestPath (int iSourceIndex, int iDestIndex, bool *bValid)
{
   PATHNODE *StartNode, *Node;
   Node = new PATHNODE;

   bots[g_i_botthink_index].i_PathDeep++; // KWo - 30.08.2006 - stack tests
   if (bots[g_i_botthink_index].i_PathDeep > 999999) // KWo - 30.08.2006 - stack tests
      bots[g_i_botthink_index].i_PathDeep = 999999;

   g_bPathWasCalculated = TRUE; // KWo 24.03.2007

   Node->iIndex = iSourceIndex;
   Node->NextNode = NULL;
   StartNode = Node;
   *bValid = FALSE;

   while (iSourceIndex != iDestIndex)
   {
      iSourceIndex = *(g_pFloydPathMatrix + (iSourceIndex * g_iNumWaypoints) + iDestIndex);

      if (iSourceIndex < 0)
         return (StartNode);

      Node->NextNode = new PATHNODE;
      Node = Node->NextNode;
      Node->iIndex = iSourceIndex;
      Node->NextNode = NULL;

      bots[g_i_botthink_index].i_PathDeep++; // KWo - 30.08.2006 - stack tests
      if (bots[g_i_botthink_index].i_PathDeep > 999999) // KWo - 30.08.2006 - stack tests
         bots[g_i_botthink_index].i_PathDeep = 999999;

   }

   *bValid = TRUE;

   return (StartNode);
}


// Test function to view the calculated A* Path
void TestAPath (int iTeam, int iWithHostage, int iSourceIndex, int iDestIndex, unsigned char byPathType)
{
   PATHNODE *root;
   PATHNODE *path = NULL;  // KWo - to remove warning uninitialised
   PATHNODE *p;
   bool bWithHostage = (iWithHostage > 0);
   g_iSearchGoalIndex = iDestIndex;

   // allocate and setup the root node
   root = new PATHNODE;

   root->iIndex = iSourceIndex;
   root->parent = NULL;
   root->NextNode = NULL;
   root->prev = NULL;
   root->g = 0;
   if (byPathType == 2)
      root->h = hfunctionNone(root);
   else
      root->h = hfunctionSquareDist (root);

   root->f = root->g + root->h;
   root->id = 0;
   root->depth = 0;
   root->state = OPEN;

   if (byPathType == 0) // KWo - 22.02.2007
   {
      if (iTeam == TEAM_CS_TERRORIST)
         path = AStarSearch (root, hfunctionNone, hfunctionSquareDist, goal, makeChildren, nodeEqual);
      else if (bWithHostage)
         path = AStarSearch (root, hfunctionNone, hfunctionSquareDistWithHostage, goal, makeChildren, nodeEqual);
      else
         path = AStarSearch (root, hfunctionNone, hfunctionSquareDist, goal, makeChildren, nodeEqual);
   }
   if (byPathType == 1) // KWo - 22.02.2007
   {
      if (iTeam == TEAM_CS_TERRORIST)
         path = AStarSearch (root, gfunctionKillsDistT, hfunctionSquareDist, goal, makeChildren, nodeEqual);
      else if (bWithHostage)
         path = AStarSearch (root, gfunctionKillsDistCTWithHostage, hfunctionSquareDist, goal, makeChildren, nodeEqual);
      else
         path = AStarSearch (root, gfunctionKillsDistCT, hfunctionSquareDist, goal, makeChildren, nodeEqual);
   }
   if (byPathType == 2)  // KWo - 12.02.2006
   {
      if (iTeam == TEAM_CS_TERRORIST)
         path = AStarSearch (root, gfunctionKillsT, hfunctionNone, goal, makeChildren, nodeEqual);
      else if (bWithHostage)
         path = AStarSearch (root, gfunctionKillsCTWithHostage, hfunctionNone, goal, makeChildren, nodeEqual);
      else
         path = AStarSearch (root, gfunctionKillsCT, hfunctionNone, goal, makeChildren, nodeEqual);
   }


   // A* returned failure
   if (path == NULL)
      UTIL_HostPrint ("NO PATH FOUND!\n");

   // otherwise, we had a successful search
   p = path;

   Vector vecSource;
   Vector vecDest;

   while (p != NULL)
   {
      vecSource = paths[p->iIndex]->origin;
      p = (PATHNODE *) p->NextNode;

      if ((p != NULL) && !FNullEnt (pHostEdict))
      {
         vecDest = paths[p->iIndex]->origin;
         MESSAGE_BEGIN (MSG_ONE_UNRELIABLE, SVC_TEMPENTITY, NULL, pHostEdict);
         WRITE_BYTE (TE_SHOWLINE);
         WRITE_COORD (vecSource.x);
         WRITE_COORD (vecSource.y);
         WRITE_COORD (vecSource.z);
         WRITE_COORD (vecDest.x);
         WRITE_COORD (vecDest.y);
         WRITE_COORD (vecDest.z);
         MESSAGE_END ();
      }
   }

   // delete the nodes on the path (which should delete root as well)
   while (path != NULL)
   {
      p = (PATHNODE *) path->NextNode;
      delete (path);
      path = p;
   }
   return;
}


PATHNODE *FindLeastCostPath (bot_t *pBot, int iSourceIndex, int iDestIndex)
{
   PATHNODE *root;
   PATHNODE *path = NULL; // KWo - to remove warning uninitialised
   unsigned char byPathType = 0; // KWo - 22.02.2007
   int iApproach; // KWo - 24.02.2007

   if ((iDestIndex > g_iNumWaypoints - 1) || (iDestIndex < 0))
      return (NULL);
   else if ((iSourceIndex > g_iNumWaypoints - 1) || (iSourceIndex < 0))
      return (NULL);

   g_bPathWasCalculated = TRUE; // KWo 24.03.2007

   g_iSearchGoalIndex = iDestIndex;

   // allocate and setup the root node
   root = new PATHNODE;

   pBot->i_PathDeep++; // KWo - 30.08.2006 - stack tests
   if (pBot->i_PathDeep > 999999) // KWo - 30.08.2006 - stack tests
      pBot->i_PathDeep = 999999;

   root->iIndex = iSourceIndex;
   root->parent = NULL;
   root->NextNode = NULL;
   root->prev = NULL;
   root->g = 0;

   iApproach = (int) (pBot->pEdict->v.health * pBot->fAgressionLevel); // KWo - 24.02.2007
   if (iApproach < 34)
      byPathType = 2;
   else if ((iApproach >= 34) && (iApproach < 66))
      byPathType = 1;
   else if (iApproach >= 66)
      byPathType = 0;

   if (pBot->bot_team == TEAM_CS_COUNTER)  // KWo - 06.07.2008
   {
      if ((g_bBombPlanted) && (g_iAliveTs == 0))
         byPathType = 0;
      else
         byPathType = (unsigned char)(RANDOM_LONG(0,1));
   }
   if ((paths[iDestIndex]->origin - paths[iSourceIndex]->origin).Length() < 600.0f)  // KWo - 22.02.2007
      byPathType = 0;

   if (byPathType == 2)
      root->h = hfunctionNone(root);
   else
      root->h = hfunctionSquareDist (root);

   root->f = root->g + root->h;
   root->id = 0;
   root->depth = 0;
   root->state = OPEN;

/*
bot personality - normal - byPathType 1 and 2
                - psycho - byPathType 0
                - coward - byPathType 2
*/

   if (byPathType == 0)  // KWo - 22.02.2007
   {
      if (pBot->bot_team == TEAM_CS_TERRORIST)
         path = AStarSearch (root, hfunctionNone, hfunctionSquareDist, goal, makeChildren, nodeEqual);
      else if (BotHasHostage (pBot)) // KWo - 22.02.2007
         path = AStarSearch (root, hfunctionNone, hfunctionSquareDistWithHostage, goal, makeChildren, nodeEqual);
      else
         path = AStarSearch (root, hfunctionNone, hfunctionSquareDist, goal, makeChildren, nodeEqual);
   }
   if (byPathType == 1)  // KWo - 22.02.2007
   {
      if (pBot->bot_team == TEAM_CS_TERRORIST)
         path = AStarSearch (root, gfunctionKillsDistT, hfunctionSquareDist, goal, makeChildren, nodeEqual);
      else if (BotHasHostage (pBot))
         path = AStarSearch (root, gfunctionKillsDistCTWithHostage, hfunctionSquareDist, goal, makeChildren, nodeEqual);
      else
         path = AStarSearch (root, gfunctionKillsDistCT, hfunctionSquareDist, goal, makeChildren, nodeEqual);
   }
   if (byPathType == 2)  // KWo - 12.02.2006
   {
      if (pBot->bot_team == TEAM_CS_TERRORIST)
         path = AStarSearch (root, gfunctionKillsT, hfunctionNone, goal, makeChildren, nodeEqual);
      else if (BotHasHostage (pBot))
         path = AStarSearch (root, gfunctionKillsCTWithHostage, hfunctionNone, goal, makeChildren, nodeEqual);
      else
         path = AStarSearch (root, gfunctionKillsCT, hfunctionNone, goal, makeChildren, nodeEqual);
   }

   // A* returned failure
   if (path == NULL)
   {
   			// Search for a broken path  KWo - 15.01.2006
      int iConnections;
   		int i, n;
      for (i = 0; i < g_iNumWaypoints; i++)
      {
         iConnections = 0;
         for (n = 0; n < MAX_PATH_INDEX; n++)
         {
            if (paths[i]->index[n] != -1)
            {
               iConnections++;
               break;  // Found some connection, check another WP
            }
         }
         if (iConnections == 0)
         {
            if (!WaypointIsConnected (i))
            {
               UTIL_HostPrint ("Waypoint problem! No path found from node %d!\n", i);
               g_bPathWasCalculated = FALSE; // KWo 24.03.2007
               break;
            }
         }
       }
       if (i == g_iNumWaypoints)
       {
          g_bPathWasCalculated = TRUE; // KWo 24.03.2007
          UTIL_HostPrint ("Waypoint Problem! No path found from node %d to node %d!\n", iSourceIndex, iDestIndex); // KWo - 16.05.2008
       }
    }
   // otherwise, we had a successful search
   return (path);
}


// This is a general A* function, so the user defines all of the
// necessary function calls for calculating path costs and generating
// the children of a particular node.
//
// root: Node*, root node from which to begin the search.
//
// gcalc: Takes a Node* as argument and returns a double indicating
// the g value (cost from initial state).
//
// hcalc: Takes a Node* as argument and returns a double indicating
// the h value (estimated cost to the goal).
//
// goalNode: Takes a Node* as argument and returns 1 if the node is
// the goal node, 0 otherwise.
//
// children: Takes a Node* as argument and returns a linked list of
// the children of that node, or NULL if there are none.
//
// nodeEqual: Takes two Node* as arguments and returns a 1 if the
// nodes are equivalent states, 0 otherwise.
//
// The function returns a Node*, which is the root node of a linked
// list (follow the "next" fields) that specifies the path from the
// root node to the goal node.

PATHNODE *AStarSearch (PATHNODE *root, int (*gcalc) (PATHNODE *), int (*hcalc) (PATHNODE *),
                       int (*goalNode) (PATHNODE *), PATHNODE * (*children) (PATHNODE *),
                       int (*nodeEqual) (PATHNODE *, PATHNODE *))
{
   PATHNODE *openList;
   PATHNODE *closedList;
   PATHNODE *current;
   PATHNODE *childList;
   PATHNODE *curChild;
   PATHNODE *p, *q;
   PATHNODE *path;
   static int gblID = 1;
   static int gblExpand = 0;

   // generate the open list
   openList = NULL;

   // generate the closed list
   closedList = NULL;

   // put the root node on the open list
   root->NextNode = NULL;
   root->prev = NULL;
   openList = root;

   while (openList != NULL)
   {
      // remove the first node from the open list, as it will always be sorted

      current = openList;
      openList = (PATHNODE *) openList->NextNode;

      if (openList != NULL)
         openList->prev = NULL;

      gblExpand++;

      // is the current node the goal node?
      if (goalNode (current))
      {
         // build the complete path to return
         current->NextNode = NULL;
         path = current;
         p = (PATHNODE *) current->parent;

         //printf ("Goal state reached with %d nodes created and %d nodes expanded\n", gblID, gblExpand);

         while (p != NULL)
         {
            // remove the parent node from the closed list (where it has to be)
            if (p->prev != NULL)
               ((PATHNODE *) p->prev)->NextNode = p->NextNode;

            if (p->NextNode != NULL)
               ((PATHNODE *) p->NextNode)->prev = p->prev;

            // check if we're romoving the top of the list
            if (p == closedList)
               closedList = (PATHNODE *)p->NextNode;

            // set it up in the path
            p->NextNode = path;
            path = p;

            p = (PATHNODE *) p->parent;
         }

         // now delete all nodes on OPEN
         while (openList != NULL)
         {
            p = (PATHNODE *) openList->NextNode;
            delete (openList);
            openList = p;

            bots[g_i_botthink_index].i_PathDeep--; // KWo - 30.08.2006 - stack tests
            if (bots[g_i_botthink_index].i_PathDeep < -999999) // KWo - 30.08.2006 - stack tests
               bots[g_i_botthink_index].i_PathDeep = -999999;

         }

         // now delete all nodes on CLOSED
         while (closedList != NULL)
         {
            p = (PATHNODE *) closedList->NextNode;
            delete (closedList);
            closedList = p;

            bots[g_i_botthink_index].i_PathDeep--; // KWo - 30.08.2006 - stack tests
            if (bots[g_i_botthink_index].i_PathDeep < -999999) // KWo - 30.08.2006 - stack tests
               bots[g_i_botthink_index].i_PathDeep = -999999;

         }

         // now return the path
         return (path);
      }

      // now expand the current node
      childList = children (current);

      // insert the children into the OPEN list according to their f values
      while (childList != NULL)
      {
         curChild = childList;
         childList = (PATHNODE *) childList->NextNode;

         // set up the rest of the child node
         curChild->parent = current;
         curChild->state = OPEN;
         curChild->depth = current->depth + 1;
         curChild->id = gblID++;
         curChild->NextNode = NULL;
         curChild->prev = NULL;

         // calculate the f value as f = g + h
         curChild->g = gcalc (curChild);
         curChild->h = hcalc (curChild);
         curChild->f = curChild->g + curChild->h;

         // forbidden value for g ?
//         if (curChild->g == -1)
         if (curChild->g == -1)
         {
            curChild->g = 9e+99;
            curChild->h = 9e+99;
            curChild->f = 9e+99; // max out all the costs for this child
         }

         // test whether the child is in the closed list (already been there)
         if (closedList != NULL)
         {
            p = closedList;

            while (p != NULL)
            {
               if (nodeEqual (p, curChild))
               {
                  // if so, check if the f value is lower
                  if (p->f <= curChild->f)
                  {
                     // if the f value of the older node is lower, delete the new child
                     delete (curChild);
                     curChild = NULL;

                     bots[g_i_botthink_index].i_PathDeep--; // KWo - 30.08.2006 - stack tests
                     if (bots[g_i_botthink_index].i_PathDeep < -999999) // KWo - 30.08.2006 - stack tests
                        bots[g_i_botthink_index].i_PathDeep = -999999;

                     break;
                  }
                  else
                  {
                     // the child is a shorter path to this point, delete p from the closed list
                     // This works so long as the new child is put in the OPEN list.
                     // Another solution is to just update all of the descendents of this node
                     // with the new f values.
                     if (p->prev != NULL)
                        ((PATHNODE *) p->prev)->NextNode = p->NextNode;

                     if (p->NextNode != NULL)
                        ((PATHNODE *) p->NextNode)->prev = p->prev;

                     if (p == closedList)
                        closedList = (PATHNODE *) p->NextNode;

                     delete (p);

                     bots[g_i_botthink_index].i_PathDeep--; // KWo - 30.08.2006 - stack tests
                     if (bots[g_i_botthink_index].i_PathDeep < -999999) // KWo - 30.08.2006 - stack tests
                        bots[g_i_botthink_index].i_PathDeep = -999999;

                     break;
                  }
               }

               p = (PATHNODE *) p->NextNode;
            }
         }

         if (curChild != NULL)
         {
            // check if the child is already on the open list
            p = openList;

            while (p != NULL)
            {
               if (nodeEqual (p, curChild))
               {
                  // child is on the OPEN list
                  if (p->f <= curChild->f)
                  {
                     // child is a longer path to the same place so delete it
                     delete (curChild);
                     curChild = NULL;

                     bots[g_i_botthink_index].i_PathDeep--; // KWo - 30.08.2006 - stack tests
                     if (bots[g_i_botthink_index].i_PathDeep < -999999) // KWo - 30.08.2006 - stack tests
                        bots[g_i_botthink_index].i_PathDeep = -999999;

                     break;
                  }
                  else
                  {
                     // child is a shorter path to the same place, remove the duplicate node
                     if (p->prev != NULL)
                        ((PATHNODE *) p->prev)->NextNode = p->NextNode;

                     if (p->NextNode != NULL)
                        ((PATHNODE *) p->NextNode)->prev = p->prev;

                     if (p == openList)
                        openList = (PATHNODE *) p->NextNode;

                     break;
                  }
               }

               p = (PATHNODE *)p->NextNode;
            }

            if (curChild != NULL)
            {
               // now insert the child into the list according to the f values
               p = openList;
               q = p;

               while (p != NULL)
               {
                  if (p->f >= curChild->f)
                  {
                     // insert before p
                     // test head case

                     if (p == openList)
                        openList = curChild;

                     // insert the node
                     curChild->NextNode = p;
                     curChild->prev = p->prev;
                     p->prev = curChild;

                     if (curChild->prev != NULL)
                        ((PATHNODE *) curChild->prev)->NextNode = curChild;
                     break;
                  }

                  q = p;
                  p = (PATHNODE *)p->NextNode;
               }

               if (p == NULL)
               {
                  // insert at the end
                  if (q != NULL)
                  {
                     q->NextNode = curChild;
                     curChild->prev = q;
                  }
                  else
                     openList = curChild; // insert at the beginning
               }
            } // end if child is not NULL (better duplicate on OPEN list)
         } // end if child is not NULL (better duplicate on CLOSED list)
      } // end of child list loop

      // put the current node onto the closed list
      current->NextNode = closedList;
      if (closedList != NULL)
         closedList->prev = current;
      closedList = current;

      current->prev = NULL;
      current->state = CLOSED;
   }

   // if we got here, then there is no path to the goal

   // delete all nodes on CLOSED since OPEN is now empty
   while (closedList != NULL)
   {
      p = (PATHNODE *) closedList->NextNode;
      delete (closedList);

      bots[g_i_botthink_index].i_PathDeep--; // KWo - 30.08.2006 - stack tests
      if (bots[g_i_botthink_index].i_PathDeep < -999999) // KWo - 30.08.2006 - stack tests
         bots[g_i_botthink_index].i_PathDeep = -999999;

      closedList = p;
   }

   return (NULL);
}


// Least Kills and Number of Nodes to Goal for a Team
int gfunctionKillsDistT (PATHNODE* p)
{
   int i;

   if (p == NULL)
      return (-1);

   int iThisIndex = p->iIndex;
   int IPrevIndex = -1;  // KWo - 09.04.2006
   if (p->prev != NULL)  // KWo - 09.04.2006
      IPrevIndex = p->prev->iIndex;

   int iCost = (pBotExperienceData + (iThisIndex * g_iNumWaypoints) + iThisIndex)->uTeam0Damage;	// KWo 14.01.2006
   int iNeighbour;

   for (i = 0; i < MAX_PATH_INDEX; i++)
   {
      iNeighbour = paths[iThisIndex]->index[i];

      if (iNeighbour != -1)
         iCost += (int) ((pBotExperienceData + (iNeighbour * g_iNumWaypoints) + iNeighbour)->uTeam0Damage * 0.3);	// KWo it was float , but to remove warning should be int
   }

   if (paths[iThisIndex]->flags & W_FL_CROUCH)
      iCost *= 2;

   if ((IPrevIndex < 0) || (IPrevIndex > g_iNumWaypoints) || (IPrevIndex == iThisIndex)) // KWo - 09.04.2006
      return (iCost * (int)((g_f_cv_dangerfactor * 20 / (2 * (int)g_iHighestDamageT))));	// KWo - 25.01.2010

   iCost = GetPathDistance(IPrevIndex, iThisIndex) + (int) (iCost * 10 * g_f_cv_dangerfactor); // KWo - 25.01.2010

   return (iCost);
}


int gfunctionKillsDistCT (PATHNODE *p)
{
   int i;

   if (p == NULL)
      return (-1);

   int iThisIndex = p->iIndex;
   int IPrevIndex = -1;  // KWo - 09.04.2006
   if (p->prev != NULL)  // KWo - 09.04.2006
      IPrevIndex = p->prev->iIndex;

   int iCost = (pBotExperienceData + (iThisIndex * g_iNumWaypoints) + iThisIndex)->uTeam1Damage;	// KWo 14.01.2006
   int iNeighbour;

   for (i = 0; i < MAX_PATH_INDEX; i++)
   {
      iNeighbour = paths[iThisIndex]->index[i];

      if (iNeighbour != -1)
         iCost += (int) ((pBotExperienceData + (iNeighbour * g_iNumWaypoints) + iNeighbour)->uTeam1Damage * 0.3);	// KWo - it was float , but should be int to remove warning
   }

   if (paths[iThisIndex]->flags & W_FL_CROUCH)
      iCost *= 2;

   if ((IPrevIndex < 0) || (IPrevIndex > g_iNumWaypoints) || (IPrevIndex == iThisIndex)) // KWo - 09.04.2006
      return (iCost * (int)((g_f_cv_dangerfactor * 20 / (2 * (int)g_iHighestDamageCT))));	// KWo - 25.01.2010

   iCost = GetPathDistance(IPrevIndex, iThisIndex) + (int)(iCost * 10 * g_f_cv_dangerfactor); // KWo - 25.01.2010

   return (iCost);
}


int gfunctionKillsDistCTWithHostage (PATHNODE *p)
{
   int i;

   if (p == NULL)
      return (-1);

   int iThisIndex = p->iIndex;
   int IPrevIndex = -1;  // KWo - 09.04.2006
   if (p->prev != NULL)  // KWo - 09.04.2006
      IPrevIndex = p->prev->iIndex;

   if (paths[iThisIndex]->flags & W_FL_NOHOSTAGE)
      return (-1);

   int iCost = (pBotExperienceData + (iThisIndex * g_iNumWaypoints) + iThisIndex)->uTeam1Damage;	// KWo 14.01.2006
   int iNeighbour;

   for (i = 0; i < MAX_PATH_INDEX; i++)
   {
      iNeighbour = paths[iThisIndex]->index[i];

      if (iNeighbour != -1)
         iCost += (int) ((pBotExperienceData + (iNeighbour * g_iNumWaypoints) + iNeighbour)->uTeam1Damage * 0.3);	// KWo - it was float , but should be int to remove warning
   }

   if (paths[iThisIndex]->flags & W_FL_CROUCH)
      iCost *= 2;

   if ((IPrevIndex < 0) || (IPrevIndex > g_iNumWaypoints) || (IPrevIndex == iThisIndex)) // KWo - 09.04.2006
      return (iCost * (int)((10 * g_f_cv_dangerfactor / (int)g_iHighestDamageCT)));	// KWo - 25.01.2010

   iCost = GetPathDistance(IPrevIndex, iThisIndex) + (int)(iCost * 10 * g_f_cv_dangerfactor); // KWo - 25.01.2010

   return (iCost);
}


// Least Kills to Goal for a Team
int gfunctionKillsT (PATHNODE* p)
{
   int i;

   if (p == NULL)
      return (-1);

   int iThisIndex = p->iIndex;

   int iCost = (pBotExperienceData + (iThisIndex * g_iNumWaypoints) + iThisIndex)->uTeam0Damage;
   int iNeighbour;

   for (i = 0; i < MAX_PATH_INDEX; i++)
   {
      iNeighbour = paths[iThisIndex]->index[i];

      if (iNeighbour != -1)
         iCost += (int) ((pBotExperienceData + (iNeighbour * g_iNumWaypoints) + iNeighbour)->uTeam0Damage * 0.3);	// KWo - it was float, but should be int to remove warning
   }

   if (paths[iThisIndex]->flags & W_FL_CROUCH)
      iCost *= 2;
   return (iCost * (int)((10 * g_f_cv_dangerfactor)));	// KWo - 25.01.2010
}


int gfunctionKillsCT (PATHNODE *p)
{
   int i;

   if (p == NULL)
      return (-1);

   int iThisIndex = p->iIndex;
   int iCost = (pBotExperienceData + (iThisIndex * g_iNumWaypoints) + iThisIndex)->uTeam1Damage;
   int iNeighbour;

   for (i = 0; i < MAX_PATH_INDEX; i++)
   {
      iNeighbour = paths[iThisIndex]->index[i];

      if (iNeighbour != -1)
         iCost += (int) ((pBotExperienceData + (iNeighbour * g_iNumWaypoints) + iNeighbour)->uTeam1Damage * 0.3);	// KWo - it was float, but should be int to remove warning
   }

   if (paths[iThisIndex]->flags & W_FL_CROUCH)
      iCost *= 2;

   return (iCost * (int)((10 * g_f_cv_dangerfactor)));	// KWo - 25.01.2010
}


int gfunctionKillsCTWithHostage (PATHNODE *p)
{
   int i;

   if (p == NULL)
      return (-1);

   int iThisIndex = p->iIndex;

   if (paths[iThisIndex]->flags & W_FL_NOHOSTAGE)
      return (-1);

   int iCost = (pBotExperienceData + (iThisIndex * g_iNumWaypoints) + iThisIndex)->uTeam1Damage;
   int iNeighbour;

   for (i = 0; i < MAX_PATH_INDEX; i++)
   {
      iNeighbour = paths[iThisIndex]->index[i];

      if (iNeighbour != -1)
         iCost += (int) ((pBotExperienceData + (iNeighbour * g_iNumWaypoints) + iNeighbour)->uTeam1Damage * 0.3);	// KWo - it was float , but should be int to remove warning
   }

   if (paths[iThisIndex]->flags & W_FL_CROUCH)
      iCost *= 2;
   return (iCost * (int) ((10 * g_f_cv_dangerfactor)));	// KWo - 25.01.2010
}


// No heurist (greedy) !!
int hfunctionNone (PATHNODE *p)
{
   if (p == NULL)
      return (-1);

   return (0);
}


// Square Distance Heuristic
int hfunctionSquareDist (PATHNODE *p)
{
   int deltaX = abs ((int) paths[g_iSearchGoalIndex]->origin.x - (int) paths[p->iIndex]->origin.x);
   int deltaY = abs ((int) paths[g_iSearchGoalIndex]->origin.y - (int) paths[p->iIndex]->origin.y);
   int deltaZ = abs ((int) paths[g_iSearchGoalIndex]->origin.z - (int) paths[p->iIndex]->origin.z);

   return (deltaX + deltaY + deltaZ);
}

// Square Distance Heuristic with Hostages
int hfunctionSquareDistWithHostage (PATHNODE *p)
{
   int deltaX = abs ((int) paths[g_iSearchGoalIndex]->origin.x - (int) paths[p->iIndex]->origin.x);
   int deltaY = abs ((int) paths[g_iSearchGoalIndex]->origin.y - (int) paths[p->iIndex]->origin.y);
   int deltaZ = abs ((int) paths[g_iSearchGoalIndex]->origin.z - (int) paths[p->iIndex]->origin.z);
   if (paths[p->iIndex]->flags & W_FL_NOHOSTAGE)
      return (8192);
   return (deltaX + deltaY + deltaZ);
}

// define the goalNode function
int goal (PATHNODE *p)
{
   if (p->iIndex == g_iSearchGoalIndex)
      return (1);

   return (0);
}


// define the children function
PATHNODE *makeChildren (PATHNODE *parent)
{
   int i;
   PATHNODE *p, *q;

   // initialize the return list
   q = NULL;
   int iParentIndex = parent->iIndex;

   for (i = 0; i < MAX_PATH_INDEX; i++)
   {
      if (paths[iParentIndex]->index[i] != -1)
      {
         p = new PATHNODE;
         p->iIndex = paths[iParentIndex]->index[i];
         p->parent = parent;
         p->NextNode = q;
         q = p;

         bots[g_i_botthink_index].i_PathDeep++; // KWo - 30.08.2006 - stack tests
         if (bots[g_i_botthink_index].i_PathDeep > 999999) // KWo - 30.08.2006 - stack tests
            bots[g_i_botthink_index].i_PathDeep = 999999;

      }
   }

   return (q);
}


// Test for node equality
int nodeEqual (PATHNODE *a, PATHNODE *b)
{
   if ((a == NULL) && (b == NULL))
      return (1);
   else if (a == NULL)
      return (0);
   else if (b == NULL)
      return (0);

   return (a->iIndex == b->iIndex);
}

bool BotFindWaypoint (bot_t *pBot)
{
   // Finds a Waypoint in the near of the Bot if he lost his Path or if Pathfinding needs
   // to be started over again

   int i, c, wpt_index[3], select_index;
   int covered_wpt = -1;
   float distance, min_distance[3];
   Vector v_src, v_dest, v_WPOrigin;
   TraceResult tr;
   bool bWaypointInUse;
   bot_t *pOtherBot;
   edict_t *pEdict = pBot->pEdict;
   bool bHasHostage = BotHasHostage (pBot);

   for (i = 0; i < 3; i++)
   {
      wpt_index[i] = -1;
      min_distance[i] = 999.0f;
   }

   for (i = 0; i < g_iNumWaypoints; i++)
   {
      // ignore current waypoint and previous recent waypoints...
/*
      if (((i == pBot->curr_wpt_index)
          || (i == pBot->prev_wpt_index[0])
          || (i == pBot->prev_wpt_index[1])
          || (i == pBot->prev_wpt_index[2])
          || (i == pBot->prev_wpt_index[3])
          || (i == pBot->prev_wpt_index[4])) && !(paths[pBot->curr_wpt_index]->flags & W_FL_LIFT) && !(paths[pBot->curr_wpt_index]->flags & W_FL_LADDER) && (g_iDebugGoalIndex == -1)
          || (paths[i]->flags & W_FL_LIFT)) // KWo - 23.04.2006

         continue;
*/
      if (WaypointReachable (pBot, i)) // KWo - 30.07.2006
      {
         // Don't use duck Waypoints if Bot got a hostage
         if (bHasHostage && (paths[i]->flags & W_FL_NOHOSTAGE))
            continue;

         bWaypointInUse = FALSE;
         v_WPOrigin = paths[i]->origin;  // KWo - 15.01.2008

         // check if this Waypoint is already in use by another bot
         for (c = 0; c < gpGlobals->maxClients; c++)
         {
            pOtherBot = &bots[c];

            if (pOtherBot->is_used && !pOtherBot->bDead
                && (pOtherBot != pBot) && (pOtherBot->curr_wpt_index == i))
            {
               covered_wpt = i;
               bWaypointInUse = TRUE;
               break;
            }
            if ((clients[c].iFlags & CLIENT_USED) && (clients[c].iFlags & CLIENT_ALIVE) // KWo - 15.01.2008
                && ((clients[c].iTeam == pBot->bot_team) && (!g_b_cv_ffa))
                && (clients[c].pEdict != pBot->pEdict))
            {
               if ((clients[c].pEdict->v.origin - v_WPOrigin).Length() < 50.0f)
               {
                  bWaypointInUse = TRUE;
                  break;
               }
            }
         }

         if (bWaypointInUse)
            continue;

         distance = (paths[i]->origin - pEdict->v.origin).Length ();

         if (IsDeadlyDrop(pBot, paths[i]->origin)) // KWo - 26.07.2006 (must be here - cannot be tested above because of the performance)
            continue;

         if (distance < min_distance[0])
         {
            wpt_index[0] = i;
            min_distance[0] = distance;
         }
         else if (distance < min_distance[1])
         {
            wpt_index[1] = i;
            min_distance[1] = distance;
         }
         else if (distance < min_distance[2])
         {
            wpt_index[2] = i;
            min_distance[2] = distance;
         }
      }
   }

   select_index = -1;

   if (wpt_index[2] != -1)
      i = RANDOM_LONG (0, 2);
   else if (wpt_index[1] != -1)
      i = RANDOM_LONG (0, 1);
   else if (wpt_index[0] != -1)
      i = 0;
   else if (covered_wpt != -1)
   {
      wpt_index[0] = covered_wpt;
      i = 0;
   }
   else
   {
      wpt_index[0] = RANDOM_LONG (0, g_iNumWaypoints - 1);
      i = 0;

      if (g_b_DebugNavig)
         ALERT(at_logged, "[DEBUG] BotFindWaypoint - Bot %s didn't find a good waypoint - need to use a random one - %i.\n",
            pBot->name, wpt_index[0]);
   }

   select_index = wpt_index[i];

   if (g_b_DebugNavig)
      ALERT(at_logged, "[DEBUG] BotFindWaypoint - Bot %s found a new waypoint %i and changes wpt index.\n", pBot->name, select_index);

   BotChangeWptIndex (pBot, select_index);
   return (TRUE);
}


void CTBombPointClear (int iIndex)
{
   // Little Helper routine to tell that a Bomb Waypoint just got visited

   int i;

   for (i = 0; i < MAXNUMBOMBSPOTS; i++)
   {
      if ((g_rgiBombSpotsVisited[i] == -1) && (paths[iIndex]->flags & W_FL_GOAL)) // KWo - 05.09.2008
      {
         g_rgiBombSpotsVisited[i] = iIndex;
         return;
      }

      else if (g_rgiBombSpotsVisited[i] == iIndex)
         return;
   }
}


bool WasBombPointVisited (int iIndex)
{
   // Little Helper routine to check if a Bomb Waypoint got visited

   int i;

   for (i = 0; i < MAXNUMBOMBSPOTS; i++)
   {
      if (g_rgiBombSpotsVisited[i] == -1)
         return (FALSE);

      else if (g_rgiBombSpotsVisited[i] == iIndex)
         return (TRUE);
   }

   return (FALSE);
}


Vector GetBombPosition (void)
{
   // Stores the Bomb Position as a Vector

   Vector vecBomb = g_vecZero;
   edict_t *pent = NULL;

   while (!FNullEnt (pent = FIND_ENTITY_BY_STRING (pent, "classname", "grenade")))
   {
      if (FStrEq (STRING (pent->v.model), "models/w_c4.mdl")
          || (pent->v.dmg >= 100)) // KWo - 11.04.2008
      {
         vecBomb = pent->v.origin;
         break;
      }
   }

   assert (vecBomb != g_vecZero);
   return (vecBomb);
}


bool BotHearsBomb (Vector vecBotPos)
{
   // Returns if Bomb is hearable and if so the exact Position as a Vector

   if (g_vecBomb == g_vecZero)
      g_vecBomb = GetBombPosition ();

   if ((vecBotPos - g_vecBomb).Length () < BOMBMAXHEARDISTANCE)
      return (TRUE);

   return (FALSE);
}


int BotChooseBombWaypoint (bot_t *pBot)
{
   // Finds the Best Goal (Bomb) Waypoint for CTs when searching for a planted Bomb

   float min_distance = 9999.0f;
   float act_distance;
   int iGoal = 0;
   int iCount = 0;
   int iCount2 = 0;        // KWo - 20.05.2008
   int i;
   int iGoalChoices[16];   // KWo - 20.05.2008
   edict_t *pEdict = pBot->pEdict;
   Vector vecPosition;
   bool bHearsBomb = false;

   if (BotHearsBomb (pEdict->v.origin))
   {
      vecPosition = g_vecBomb;
      bHearsBomb = true;
   }
   else
      vecPosition = pEdict->v.origin;

   // Find nearest Goal Waypoint either to Bomb (if "heard" or Player)
   for (i = 0; i < g_iNumGoalPoints; i++)
   {
      act_distance = (paths[g_rgiGoalWaypoints[i]]->origin - vecPosition).Length ();

      if ((act_distance < min_distance) && ((bHearsBomb) || (!bHearsBomb && (act_distance > BOMBMAXHEARDISTANCE)))) // KWo 13.11.2006
      {
         min_distance = act_distance;
         iGoal = g_rgiGoalWaypoints[i];
         if (iCount2 < 16) // KWo - 20.05.2008
         {
            iGoalChoices[iCount2] = iGoal;
            iCount2++;
         }
      }
   }

   while (WasBombPointVisited (iGoal))
   {
      if (g_iNumGoalPoints == 1)
         iGoal = g_rgiGoalWaypoints[0];
      else
         iGoal = iGoalChoices[RANDOM_LONG (0, iCount2 - 1)]; // KWo - 20.05.2008

      iCount++;

      if (iCount == iCount2)
         break;
   }

   return (iGoal);
}


int BotFindDefendWaypoint (bot_t *pBot, Vector vecPosition)
{
   // Tries to find a good waypoint which
   // a) has a Line of Sight to vecPosition and
   // b) provides enough cover
   // c) is far away from the defending position

   int wpt_index[8];
   int min_distance[8];
   int iDistance, i, j;
   float fMin = 9999.0f;
   float fPosMin = 9999.0f;
   float fDistance;
   int iSourceIndex = 0;		// KWo - to remove warning uninitialised
   int iPosIndex = 0;		// KWo - to remove warning uninitialised
   edict_t *pEdict = pBot->pEdict;
   TraceResult tr;
   Vector vecSource;
   Vector vecDest;
   int iExp;	// KWo 11.01.2006
   int index1;
   int index2;
   int tempindex;
   bool bOrderchange;
   bool bSkipUsed; // KWo - 07.07.2008

   for (i = 0; i < 8; i++)
   {
      wpt_index[i] = -1;
      min_distance[i] = 128;
   }

   // Get nearest waypoint to Bot & Position
   for (i = 0; i < g_iNumWaypoints; i++)
   {
      fDistance = (paths[i]->origin - pEdict->v.origin).Length ();

      if (fDistance < fMin)
      {
         iSourceIndex = i;
         fMin = fDistance;
      }

      fDistance = (paths[i]->origin - vecPosition).Length ();
      if (fDistance < fPosMin)
      {
         iPosIndex = i;
         fPosMin = fDistance;
      }
   }

   vecDest = paths[iPosIndex]->origin;

   // Find Best Waypoint now
   for (i = 0; i < g_iNumWaypoints; i++)
   {
      // Exclude Ladder, Goal, Rescue & current Waypoints
      if ((paths[i]->flags & W_FL_LADDER) || (i == iSourceIndex) || !WaypointIsVisible (i, iPosIndex))
         continue;

      // Use the 'real' Pathfinding Distances
      iDistance = GetPathDistance (iSourceIndex, i);

      if (iDistance < 1024)
      {
         vecSource = paths[i]->origin;
         UTIL_TraceLine (vecSource, vecDest, dont_ignore_monsters, ignore_glass, pEdict, &tr); // KWo - 06.07.2008
         if (tr.flFraction != 1.0f)
            continue;

         bSkipUsed = false; // KWo - 07.07.2008

         for (j = 0; j < gpGlobals->maxClients; j++) // KWo - 07.07.2008
         {
            if ((bots[j].is_used) && (clients[j].iFlags & CLIENT_ALIVE) && (pBot->bot_team == bots[j].bot_team)
               && (bots[j].pEdict != pBot->pEdict))
            {
               if ((BotGetSafeTask(&bots[j])->iData == i) || (bots[j].curr_wpt_index == i))
               {
                  bSkipUsed = true;
                  break;
               }
            }
         }

         if (bSkipUsed) // KWo - 07.07.2008
            continue;

         for (j = 0; j < 8; j++)
         {
            if (iDistance > min_distance[j])
            {
               wpt_index[j] = i;
               min_distance[j] = iDistance;
               break;
            }
         }
      }
   }

   // Use statistics if we have them...
   for (i = 0; i < 8; i++)	// KWo - 09.04.2006
   {
      if (wpt_index[i] != -1)
      {
         if (pBot->bot_team == TEAM_CS_TERRORIST)
         {
            iExp = (pBotExperienceData + (wpt_index[i] * g_iNumWaypoints) + wpt_index[i])->uTeam0Damage;
            iExp = (iExp * 100) / g_iHighestDamageT; // KWo - 09.04.2006
            min_distance[i] = iExp / 8192;
            min_distance[i] += iExp;
            iExp >>= 1;
         }
         else
         {
            iExp = (pBotExperienceData + (wpt_index[i] * g_iNumWaypoints) + wpt_index[i])->uTeam1Damage;
            iExp = (iExp * 100) / g_iHighestDamageCT; // KWo - 09.04.2006
            min_distance[i] = iExp / 8192;
            min_distance[i] += iExp;
            iExp >>= 1;
         }
      }
   }	// KWo 11.01.2006

   // Sort resulting Waypoints for farest distance
   do
   {
      bOrderchange = FALSE;

      for (i = 0; i < 7; i++)
      {
         index1 = wpt_index[i];
         index2 = wpt_index[i + 1];

         if ((index1 != -1) && (index2 != -1))
         {
            if (min_distance[i] > min_distance[i + 1])
            {
               tempindex = wpt_index[i];
               wpt_index[i] = wpt_index[i + 1];
               wpt_index[i + 1] = tempindex;
               tempindex = min_distance[i];
               min_distance[i] = min_distance[i + 1];
               min_distance[i + 1] = tempindex;
               bOrderchange = TRUE;
            }
         }
      }
   }
   while (bOrderchange);

   for (i = 0; i < 8; i++)
   {
      if (wpt_index[i] != -1)
         return (wpt_index[i]);
   }

   // Worst case: If no waypoint was found, just use a random one
   return (RANDOM_LONG (0, g_iNumWaypoints - 1));
}


int BotFindCoverWaypoint (bot_t *pBot, float maxdistance)
{
   // Tries to find a good Cover Waypoint if Bot wants to hide
   edict_t *pEdict = pBot->pEdict;
   TraceResult tr;
   bool bOrderChange;
   bool bNeighbourVisible;
   float f_min = 9999.0f;
   float f_enemymin = 9999.0f;
   float f_enemymin2 = 9999.0f; // KWo - 19.04.2008
   float f_distance;
   int i, j, k;
   int wpt_index[8];
   int distance,enemydistance;
   int iEnemyWPT = -1;
   int iEnemyWPT2 = -1;
   int rgiEnemyIndices[MAX_PATH_INDEX];
   int iEnemyNeighbours = 0;
   int min_distance[8];
   int iSourceIndex = pBot->curr_wpt_index;
   int iExp;
   int index1;
   int index2;
   int tempindex;
   Vector v_source;
   Vector v_dest;
   Vector vecEnemy = pBot->vecLastEnemyOrigin;
   // Get Enemies Distance
   // FIXME: Would be better to use the actual Distance returned
   // from Pathfinding because we might get wrong distances here
   f_distance = (vecEnemy - pEdict->v.origin).Length ();

   // Don't move to a position nearer the enemy
   if (maxdistance > f_distance)
      maxdistance = f_distance;
   if (maxdistance < 300)
      maxdistance = 300;

   for (i = 0; i < 8; i++)
   {
      wpt_index[i] = -1;
      min_distance[i] = (int) maxdistance; // KWo - to remove warning
   }

   // Get nearest waypoint to enemy & Bot
   for (i = 0; i < g_iNumWaypoints; i++)
   {
      f_distance = (paths[i]->origin - pEdict->v.origin).Length ();

      if (f_distance < f_min)
      {
         if (WaypointReachable (pBot, i)) // KWo - 19.04.2008
         {
            if (IsDeadlyDrop(pBot, paths[i]->origin))
               continue;

            iSourceIndex = i;
            f_min = f_distance;
         }
      }

      f_distance = (paths[i]->origin - vecEnemy).Length ();

      if (f_distance < f_enemymin)
      {
         if (WaypointReachableByEnt(vecEnemy, paths[i]->origin, pBot->pLastEnemy)) // KWo - 19.04.2008
         {
            iEnemyWPT = i;
            f_enemymin = f_distance;
         }
      }
      if (f_distance < f_enemymin2)
      {
         iEnemyWPT2 = i;
         f_enemymin2 = f_distance;
      }
   }

   if (iEnemyWPT == -1) // KWo - 19.04.2008
   {
      if (iEnemyWPT2 >= 0)
         iEnemyWPT = iEnemyWPT2;
      else
         iEnemyWPT = iSourceIndex;
   }

   // Now Get Enemies Neigbouring Waypoints
   for (i = 0; i < MAX_PATH_INDEX; i++)
   {
      if (paths[iEnemyWPT]->index[i] != -1)
      {
         rgiEnemyIndices[iEnemyNeighbours] = paths[iEnemyWPT]->index[i];
         iEnemyNeighbours++;
      }
   }

   BotChangeWptIndex (pBot, iSourceIndex);

   // Find Best Waypoint now
   for (i = 0; i < g_iNumWaypoints; i++)
   {
      // Exclude Ladder, current Waypoints
      if ((paths[i]->flags & W_FL_LADDER) || (i == iSourceIndex) || WaypointIsVisible (iEnemyWPT, i))
         continue;

      // Now check neighbour Waypoints for Visibility
      bNeighbourVisible = FALSE;
      for (j = 0; j < iEnemyNeighbours; j++)
      {
         if (WaypointIsVisible (rgiEnemyIndices[j], i))
         {
            bNeighbourVisible = TRUE;
            break;
         }
      }

      if (bNeighbourVisible)
         continue;

      // Use the 'real' Pathfinding Distances
      distance = GetPathDistance (iSourceIndex, i);
      enemydistance = GetPathDistance (iEnemyWPT, i);

      if (distance < enemydistance)
      {
         for (j = 0; j < 8; j++)
         {
            if (distance < min_distance[j])
            {
               for (k = 6; k + 1 > j; k--) // KWo - 27.04.2010
               {
                  wpt_index[k + 1] = wpt_index[k];
                  min_distance[k + 1] = min_distance[k];
               }
               wpt_index[j] = i;
               min_distance[j] = distance;
               break;
            }
         }
      }
   }

   // Use statistics if we have them...
   if (pBot->bot_team == TEAM_CS_TERRORIST)
   {
      for (i = 0; i < 8; i++) // KWo - 09.04.2006
      {
         if (wpt_index[i]!= -1)
         {
            iExp = (pBotExperienceData + (wpt_index[i] * g_iNumWaypoints) + wpt_index[i])->uTeam0Damage;
            iExp = (iExp * 100) / g_iHighestDamageT; // KWo - 09.04.2006
            min_distance[i] = (iExp * 100) / 8192;
            min_distance[i] += iExp;
            iExp >>= 1;
         }
      }
   }
   else
   {
      for (i = 0; i < 8; i++)
      {
         if (wpt_index[i]!= -1)
         {
            iExp = (pBotExperienceData + (wpt_index[i] * g_iNumWaypoints) + wpt_index[i])->uTeam1Damage;
            iExp = (iExp * 100) / g_iHighestDamageCT; // KWo - 09.04.2006
            min_distance[i] = (iExp * 100) / 8192;
            min_distance[i] += iExp;
            iExp >>= 1;
         }
      }
   }

   // Sort resulting Waypoints for nearest distance
   do
   {
      bOrderChange = FALSE;
      for (i = 0; i < 7; i++)
      {
         index1 = wpt_index[i];
         index2 = wpt_index[i + 1];

         if ((index1 != -1) && (index2 != -1))
         {
            if (min_distance[i] > min_distance[i + 1])
            {
               tempindex = wpt_index[i];
               wpt_index[i] = wpt_index[i + 1];
               wpt_index[i + 1] = tempindex;
               tempindex = min_distance[i];
               min_distance[i] = min_distance[i + 1];
               min_distance[i + 1] = tempindex;
               bOrderChange = TRUE;
            }
         }
      }
   }
   while (bOrderChange);

   // Take the first one which isn't spotted by the enemy
   for (i = 0; i < 8; i++)
   {
      if (wpt_index[i] != -1)
      {
         v_source = pBot->vecLastEnemyOrigin + Vector (0, 0, 36);
         v_dest = paths[wpt_index[i]]->origin;

         UTIL_TraceLine (v_source, v_dest, ignore_monsters, ignore_glass, pEdict, &tr);
         if (tr.flFraction < 1.0f)
            return (wpt_index[i]);
      }
   }

   // If all are seen by the enemy, take the first one
   if (wpt_index[0] != -1)
      return (wpt_index[0]);

   // Worst case: if no waypoint was found, just use a random one
   return (RANDOM_LONG (0, g_iNumWaypoints - 1));
}


bool GetBestNextWaypoint (bot_t *pBot)
{
   // Does a realtime postprocessing of Waypoints returned from
   // Pathfinding, to vary Paths and find best Waypoints on the way

   static bot_t *pOtherBot;
   static bool bWaypointInUse;
   static bool bWaypointNodeNextReachable;   // KWo - 02.11.2009
   static bool bWaypointNodePrevReachable;   // KWo - 02.11.2009
//   static bool bCurrentWaypointIndexOK;      // KWo - 09.04.2016
   static int c;
   static int iCount;
   static int iCurrentWaypointIndex;
   static int iNextWaypointIndex;
   static int iPrevWaypointIndex;
   static int iUsedWaypoints[32];
   static int num;
   static Vector vecWPOrigin;

   if ((g_iDebugGoalIndex != -1) && (pBot->curr_wpt_index == g_iDebugGoalIndex)) // KWo - 08.01.2008
      return(TRUE);


   // Get waypoints used by other Bots
   for (c = 0; c < gpGlobals->maxClients; c++)
   {
      pOtherBot = &bots[c];
      iUsedWaypoints[c] = -1;
      if (pOtherBot->is_used && !pOtherBot->bDead && (pOtherBot != pBot)
         && (!g_b_cv_ffa) && (pBot->bot_team == pOtherBot->bot_team)) // KWo - 16.12.2007

         iUsedWaypoints[c] = pOtherBot->curr_wpt_index;
   }

   iCurrentWaypointIndex = pBot->pWaypointNodes->iIndex;
//   bCurrentWaypointIndexOK = (iCurrentWaypointIndex >= 0) && (iCurrentWaypointIndex < g_iNumWaypoints); // KWo - 09.04.2016
   vecWPOrigin = g_vecZero;
   bWaypointInUse = FALSE;
/*
   if (bCurrentWaypointIndexOK)
   {
      vecWPOrigin = paths[iCurrentWaypointIndex]->origin;
   }

   for (c = 0; c < gpGlobals->maxClients; c++)
   {
      if ((iUsedWaypoints[c] == iCurrentWaypointIndex) && (bCurrentWaypointIndexOK)) // KWo - 07.04.2016
      {
*/
/*
         if (!paths[iCurrentWaypointIndex]->flags & W_FL_LADDER)
         {
            bWaypointInUse = TRUE;
            break;
         }
*/
/*
         bWaypointInUse = TRUE;
         break;
      }
      if ((clients[c].iFlags & CLIENT_USED) && (clients[c].iFlags & CLIENT_ALIVE) // KWo - 07.01.2008
          && ((clients[c].iTeam == pBot->bot_team) && (!g_b_cv_ffa))
          && (clients[c].pEdict != pBot->pEdict))
      {
         if ((clients[c].pEdict->v.origin - vecWPOrigin).Length() < 50.0f)
         {
            bWaypointInUse = TRUE;
            break;
         }
      }
   }
*/


   if (bWaypointInUse)
   {
      if (iCurrentWaypointIndex == BotGetSafeTask(pBot)->iData)   // KWo - 19.01.2012
      {
         BotGetSafeTask(pBot)->iData = pBot->curr_wpt_index;
         return (FALSE);
      }

      iNextWaypointIndex = pBot->pWaypointNodes->NextNode->iIndex;
      iPrevWaypointIndex = pBot->curr_wpt_index;
      iCount = 0;
      while (iCount < MAX_PATH_INDEX)
      {
         num = paths[iPrevWaypointIndex]->index[iCount];
         if (num != -1)
         {
/*
            if (IsConnectedWithWaypoint (num, iNextWaypointIndex)
                && IsConnectedWithWaypoint (num, iPrevWaypointIndex))
*/
            bWaypointNodeNextReachable = IsConnectedWithWaypoint (num, iNextWaypointIndex); // KWo - 02.11.2009
            bWaypointNodePrevReachable = IsConnectedWithWaypoint (num, iPrevWaypointIndex); // KWo - 02.11.2009

            if (!WaypointIsVisible(iPrevWaypointIndex, iNextWaypointIndex)) // KWo - 11.11.2009
            {
               iCount++;
               continue;
            }

            if (!bWaypointNodeNextReachable) // KWo - 02.11.2009
               bWaypointNodeNextReachable = WaypointNodeReachable (num, iNextWaypointIndex);
            if (!bWaypointNodePrevReachable) // KWo - 02.11.2009
               bWaypointNodePrevReachable = WaypointNodeReachable (num, iPrevWaypointIndex);
/*
            if (WaypointNodeReachable (num, iNextWaypointIndex) // KWo - 07.01.2008
                && WaypointNodeReachable (num, iPrevWaypointIndex))
*/
            if (bWaypointNodePrevReachable && bWaypointNodeNextReachable) // KWo - 02.11.2009
            {

               // Don't use ladder waypoints as alternative - if is placed higher or lower than the bot's origin
               if ((paths[num]->flags & W_FL_LADDER)
                  && (fabs(paths[num]->origin.z - pBot->pEdict->v.origin.z) > 60.0f)) // KWo - 05.04.2016
               {
                  iCount++;
                  continue;
               }

               // check if this Waypoint is already in use by another bot
               bWaypointInUse = FALSE;
               vecWPOrigin = paths[num]->origin; // KWo - 07.01.2008

               for (c = 0; c < gpGlobals->maxClients; c++)
               {
                  if (iUsedWaypoints[c] == num)
                  {
                     bWaypointInUse = TRUE;
                     break;
                  }
                  if ((clients[c].iFlags & CLIENT_USED) && (clients[c].iFlags & CLIENT_ALIVE) // KWo - 07.01.2008
                     && ((clients[c].iTeam == pBot->bot_team) && (!g_b_cv_ffa))
                     && (clients[c].pEdict != pBot->pEdict))
                  {
                     if ((clients[c].pEdict->v.origin - vecWPOrigin).Length() < 50.0f)
                     {
                        bWaypointInUse = TRUE;
                        break;
                     }
                  }
               }

               // Waypoint not used by another Bot - feel free to use it
               if (!bWaypointInUse)
               {
                  pBot->pWaypointNodes->iIndex = num;
                  return (TRUE);
               }
            }
         }
         iCount++;
      }
   }

   return (FALSE);
}

bool BotHeadTowardWaypoint (bot_t *pBot)
{
   // Advances in our Pathfinding list and sets the appropiate Destination Origins for this Bot

   static edict_t *pEdict;
   static TraceResult tr;
   static bool bWill_jump;
   static float fKills;
   static float fTime;
   static float fJump_distance;
   static int i;
   static int iWaypoint;
   static long max_random;          // KWo - 19.02.2008
   static Vector v_src, v_dest;
   static PATH *pPath;

   // Check if old waypoints is still reliable
   GetValidWaypoint (pBot);

   if ((paths[pBot->curr_wpt_index]->flags & W_FL_USE_BUTTON) && (g_iNumButtons > 0)
      && (pBot->fButtonPushTime < gpGlobals->time) && (pBot->vecEntity == pBot->dest_origin) && (pBot->bButtonPushDecided))
      return (FALSE);

   // No Waypoints from pathfinding ?
   if (pBot->pWaypointNodes == NULL)
   {
      if (g_b_DebugNavig)
         ALERT (at_logged,"[DEBUG] BotHeadTowardWaypoint - Bot %s has no waypoint from pathfinding.\n", pBot->name);
      return (FALSE);
   }

   if (g_b_DebugNavig)
      ALERT (at_logged,"[DEBUG] BotHeadTowardWaypoint called for Bot %s.\n", pBot->name);

   pEdict = pBot->pEdict;

   // Reset Travel Flags (jumping etc.)
   pBot->curr_travel_flags = 0;

   // Advance in List
   pBot->pWaypointNodes = pBot->pWaypointNodes->NextNode;

   // We're not at the end of the List ?
   if (pBot->pWaypointNodes != NULL)
   {
      // If in between a route, postprocess the waypoint (find better alternatives)
      if ((pBot->pWaypointNodes != pBot->pWayNodesStart)
           && (pBot->pWaypointNodes->NextNode != NULL))      // KWo - 04.04.2012
      {
         if (g_b_DebugNavig)
            ALERT (at_logged,"[DEBUG] BotHeadTowardWaypoint - Bot %s checks the next alternative waypoint.\n", pBot->name);

         GetBestNextWaypoint (pBot);
         if (BotGetSafeTask(pBot)->iData == pBot->curr_wpt_index) // KWo - 19.01.2012
         {
            pBot->pWaypointNodes = pBot->pWaypointNodes->prev;
            return (FALSE);
         }

         if (pBot->pWaypointNodes->NextNode != NULL)              // KWo - 19.01.2012
         {
            pBot->fMinSpeed = pEdict->v.maxspeed;
            if ((BotGetSafeTask (pBot)->iTask == TASK_NORMAL) && !g_bBombPlanted
               && !BotHasHostage(pBot) && !(pBot->pEdict->v.weapons & (1 << CS_WEAPON_C4))) // KWo - 18.03.2010
            {
               pBot->iCampButtons = 0;
               iWaypoint = pBot->pWaypointNodes->NextNode->iIndex;

               if (pBot->bot_team == TEAM_CS_TERRORIST)
                  fKills = (pBotExperienceData + (iWaypoint * g_iNumWaypoints) + iWaypoint)->uTeam0Damage / g_iHighestDamageT; // KWo - 15.03.2010
               else
                  fKills = (pBotExperienceData + (iWaypoint * g_iNumWaypoints) + iWaypoint)->uTeam1Damage / g_iHighestDamageCT; // KWo - 15.03.2010

               switch (pBot->bot_personality) // KWo - 19.02.2008 moved here
               {
                  case 1: // Psycho
                     fKills *= 0.3333f;
                     break;
                  default:
                     fKills *= 0.5f;
               }


               if ((fKills > 0.15f) && (g_fTimeRoundMid > gpGlobals->time)
                  && (BotHasCampWeapon (pBot)) && (pBot->current_weapon.iAmmo1 > 0)
                  && (!pBot->bIsChickenOrZombie)) // KWo - 18.01.2011
               {

/*
                  // KWo - 19.02.2008 - now fKills is already a value below 1.0...
                  fKills = (fKills * 100) / g_cKillHistory;
                  fKills /= 100;
                  g_cKillHistory used only to reduce the remembered values of damages after few rounds...
*/

                  if ((pBot->fBaseAgressionLevel < fKills) && (BotGetSafeTask(pBot)->iTask != TASK_MOVETOPOSITION)
                       && (pBot->fTimeCamping + 20.0f < gpGlobals->time)
                     && (g_fTimeRoundMid > gpGlobals->time)) // 23.03.2008
                  {
                     fTime = pBot->fFearLevel * (g_fTimeRoundMid - gpGlobals->time) * 0.5f;
                     if (fTime > g_f_cv_maxcamptime) // KWo - 23.03.2008
                        fTime = g_f_cv_maxcamptime;

                     if (g_b_DebugNavig || g_b_DebugTasks)
                     {
                        ALERT(at_logged, "[DEBUG] BotHeadTowardWaypoint - Bot's %s agression = %f, fKills = %f.\n",
                           pBot->name, pBot->fBaseAgressionLevel, fKills);
                     }

                     // KWo - I need to check this fKills and fBaseAggressionLevel if it works correcty here...
                     // I feel if the area is really bad (a lot of danger values around), the bot may camp in that area
                     // forever (until middle of the round) instead go away from it...)

                     // Push camp task on to stack
                     pBot->fTimeCamping = gpGlobals->time + fTime; // KWo - 23.03.2008
                     bottask_t TempTask = {NULL, NULL, TASK_CAMP, TASKPRI_CAMP, -1, pBot->fTimeCamping, TRUE}; // KWo - 23.03.2008
                     BotPushTask (pBot, &TempTask);

                     // Push Move Command
                     TempTask.iTask = TASK_MOVETOPOSITION;
                     TempTask.fDesire = TASKPRI_MOVETOPOSITION;
                     TempTask.iData = BotFindDefendWaypoint (pBot, paths[iWaypoint]->origin);

                     if (g_b_DebugTasks || g_b_DebugCombat)
                        ALERT(at_logged,"[DEBUG] BotHeadTowardWaypoint - The bot %s will camp because of the fkills...\n",
                           pBot->name);

                     BotPushTask (pBot, &TempTask);
                     if (RANDOM_LONG(0,100) > 50) // KWo - 19.02.2008
                        pBot->iCampButtons |= IN_DUCK;
                     else
                        pBot->iCampButtons = 0;
                  }
               }
               else if (g_bBotsCanPause && !pBot->bOnLadder
                     && !pBot->bInWater && (pBot->curr_travel_flags == 0)
                     && (pEdict->v.flags & FL_ONGROUND))
               {
                  if (fKills >= pBot->fBaseAgressionLevel)
                  {
                     pBot->f_ducktime = gpGlobals->time + 2.5f; // KWo - 19.02.2008
                     pEdict->v.button |= IN_DUCK;
//                   pBot->iCampButtons |= IN_DUCK;
                  }
                  else
                  {
                     max_random = 0;
                     switch (pBot->bot_personality) // KWo - 19.02.2008 - more clean now...
                     {
                        case 0: // Normal
                        {
                           max_random = long (pBot->bot_skill * 0.2);
                           break;
                        }
                        case 1: // Psycho
                        {
                           max_random = long (pBot->bot_skill * 0.1);
                           break;
                        }
                        case 2: // Coward
                        {
                           max_random = long (pBot->bot_skill * 0.5f);
                           break;
                        }
                     }

                     if (RANDOM_LONG (1, 100) < RANDOM_LONG (1, max_random))	// KWo 19.02.2008
                        pBot->fMinSpeed = 120.0f;
                  }
               }
            }
         }
      }

      if (pBot->pWaypointNodes != NULL)
      {
         // Find out about connection flags
         if (pBot->curr_wpt_index != -1)
         {
            pPath = paths[pBot->curr_wpt_index];

            for (i = 0; i < MAX_PATH_INDEX; i++)
            {
               if (pPath->index[i] == pBot->pWaypointNodes->iIndex)
               {
                  pBot->curr_travel_flags = pPath->connectflag[i];
                  pBot->vecDesiredVelocity = pPath->vecConnectVel[i];
                  pBot->bJumpDone = FALSE;
                  break;
               }
            }

            // Find out about FUTURE connection flags
            bWill_jump = FALSE;
            fJump_distance = 0.0f;
            if (pBot->pWaypointNodes->NextNode != NULL)
            {
               for (i = 0; i < MAX_PATH_INDEX; i++)
               {
                  if ((paths[pBot->pWaypointNodes->iIndex]->index[i] == pBot->pWaypointNodes->NextNode->iIndex)
                      && (paths[pBot->pWaypointNodes->iIndex]->connectflag[i] & C_FL_JUMP))
                  {
                     bWill_jump = TRUE;
                     v_src = paths[pBot->pWaypointNodes->iIndex]->origin;
                     v_dest = paths[pBot->pWaypointNodes->NextNode->iIndex]->origin;
                     fJump_distance = (paths[pBot->pWaypointNodes->iIndex]->origin - paths[pBot->pWaypointNodes->NextNode->iIndex]->origin).Length ();
                     break;
                  }
               }
            }

            // is there a jump waypoint right ahead and do we need to draw out the knife ?
            if (bWill_jump
                && ((((fJump_distance > 220.0f) && (v_dest.z  - 10.0f > v_src.z))
                    || ((v_dest.z - 32.0f > v_src.z) && (fJump_distance > 150.0f))
                   /*  || (((v_dest - v_src).Length2D () < 50.0f) && (fJump_distance > 60.0f)) */ )
                && FNullEnt (pBot->pBotEnemy)
                && (pBot->current_weapon.iId != CS_WEAPON_KNIFE))) // KWo - 08.04.2010
               SelectWeaponByName (pBot, "weapon_knife"); // draw out the knife if needed

         }

         if (g_b_DebugNavig)
         {
            ALERT(at_logged, "[DEBUG] BotHeadTowardWaypoint - Bot %s is about to change wpt index.\n", pBot->name);
         }

         BotChangeWptIndex (pBot, pBot->pWaypointNodes->iIndex);
      }
   }

   pBot->wpt_origin = paths[pBot->curr_wpt_index]->origin;

   // If wayzone radius != 0 vary origin a bit depending on body angles
//   if (paths[pBot->curr_wpt_index]->Radius != 0.0f)
//   {
//      MAKE_VECTORS (pEdict->v.angles);
//      Vector v_x = pBot->wpt_origin + gpGlobals->v_right * RANDOM_FLOAT (-paths[pBot->curr_wpt_index]->Radius, paths[pBot->curr_wpt_index]->Radius);
//      Vector v_y = pBot->wpt_origin + gpGlobals->v_forward * RANDOM_FLOAT (0, paths[pBot->curr_wpt_index]->Radius);
//      pBot->wpt_origin = (v_x + v_y) * 0.5f;
//      pBot->wpt_origin.z = paths[pBot->curr_wpt_index]->origin.z;
//   }

//   if (!g_bIsDedicatedServer && g_bWaypointOn && !FNullEnt (pHostEdict))
//   {
//      float distance = (paths[pBot->curr_wpt_index]->origin - pHostEdict->v.origin).Length ();

//      if (distance < 500)
//      {
//         if (g_fWPDisplayTime[pBot->curr_wpt_index] + 0.18f > gpGlobals->time)
//         {
//            WaypointDrawBeam (pBot->wpt_origin - Vector (0, 0, 34), pBot->wpt_origin + Vector (0, 0, 34), 36, 255, 0, 0);
//         }
//      }
//   }

   // Bot on Ladder ?
   if (pBot->bOnLadder)
   {
      v_src = pEdict->v.origin;
      v_src.z = pEdict->v.absmin.z;
      v_dest = pBot->wpt_origin;

      // Is the Bot inside a Ladder Cage ? Then we need to adjust
      // the Waypoint Origin to make sure Bot doesn't get stuck
      TRACE_LINE (v_src, v_dest, ignore_monsters, pEdict, &tr);
      if (tr.flFraction < 1.0f)
         pBot->wpt_origin = paths[pBot->curr_wpt_index]->origin + (pEdict->v.origin - pBot->wpt_origin) * 0.5f + Vector (0, 0, 32);

   }

   // Uncomment this to have a temp line showing the direction to the destination waypoint
   //WaypointDrawBeam (pEdict->v.origin, pBot->wpt_origin, 10, 255, 255, 255);
   //WaypointDrawBeam (pBot->wpt_origin + Vector (0, 0, -20), pBot->wpt_origin + Vector (0, 0, 20), 10, 255, 255, 255);

   if (pBot->iAimFlags == AIM_DEST) // KWo - 12.09.2008 looking only at the dest while changing the waypoint
      pBot->fChangeAimDirectionTime = gpGlobals->time - 0.1f;

   pBot->f_wpt_timeset = gpGlobals->time;

   return (TRUE);
}


bool BotCantMoveForward (bot_t *pBot, Vector vNormal)  // KWo - 13.07.2006
{
   // Checks if Bot is blocked in his movement direction (excluding doors)
   // use some TraceLines to determine if anything is blocking the current path of the bot.

   TraceResult tr;
   edict_t *pEdict = pBot->pEdict;
   Vector v_src, v_forward, v_center;

   // first do a trace from the bot's eyes forward...

   v_src = GetGunPosition (pEdict); // EyePosition ()
   v_forward = v_src + vNormal * 24;

   // trace from the bot's eyes straight forward...
   TRACE_HULL (v_src, v_forward, dont_ignore_monsters, head_hull, pEdict, &tr); // KWo - 13.01.2008

   // check if the trace hit something...
   if ((tr.flFraction < 1.0f) && (!FNullEnt(tr.pHit)))
   {
      if ((strncmp ("func_door", STRING (tr.pHit->v.classname), 9) == 0)
            || ((pBot->bot_team == TEAM_CS_COUNTER) && (FStrEq ("hostage_entity", STRING (tr.pHit->v.classname))))) // KWo - 13.01.2008
         return (FALSE);

      if (g_b_DebugStuck)
         ALERT(at_logged, "[DEBUG] Bot %s can't move forward - its head may hit someting.\n", pBot->name);

      return (TRUE); // bot's head will hit something
   }

   // bot's head is clear, check at shoulder level...
   // trace from the bot's shoulder left diagonal forward to the right shoulder...
   v_center = pEdict->v.angles;
   v_center.z = 0;
   v_center.x = 0;
   MAKE_VECTORS (v_center);

   v_src = GetGunPosition (pEdict) + Vector (0, 0, -16) + gpGlobals->v_right * -16;
   v_forward = GetGunPosition (pEdict) + Vector (0, 0, -16) + gpGlobals->v_right * 16 + vNormal * 24;

   TRACE_LINE (v_src, v_forward, dont_ignore_monsters, pEdict, &tr); // KWo - 13.01.2008

   // check if the trace hit something...
   if ((tr.flFraction < 1.0f) && (strncmp ("func_door", STRING (tr.pHit->v.classname), 9) != 0)
      && ((pBot->bot_team == TEAM_CS_TERRORIST) || !(FStrEq ("hostage_entity", STRING (tr.pHit->v.classname))))) // KWo - 13.01.2008
   {
      if (g_b_DebugStuck)
         ALERT(at_logged, "[DEBUG] Bot %s can't move forward - its shoulder may hit something on the right.\n", pBot->name);

      return (TRUE); // bot's body will hit something
   }

   // bot's head is clear, check at shoulder level...
   // trace from the bot's shoulder right diagonal forward to the left shoulder...
   v_src = GetGunPosition (pEdict) + Vector (0, 0, -16) + gpGlobals->v_right * 16;
   v_forward = GetGunPosition (pEdict) + Vector (0, 0, -16) + gpGlobals->v_right * -16 + vNormal * 24;

   TRACE_LINE (v_src, v_forward, dont_ignore_monsters, pEdict, &tr); // KWo - 13.01.2008

   // check if the trace hit something...
   if ((tr.flFraction < 1.0f) && (strncmp ("func_door", STRING (tr.pHit->v.classname), 9) != 0)
      && ((pBot->bot_team == TEAM_CS_TERRORIST) || !(FStrEq ("hostage_entity", STRING (tr.pHit->v.classname))))) // KWo - 13.01.2008
   {

      if (g_b_DebugStuck)
         ALERT(at_logged, "[DEBUG] Bot %s can't move forward - its shoulder may hit something on the left.\n", pBot->name);

      return (TRUE); // bot's body will hit something
   }

   // Now check below Waist

   if (pEdict->v.flags & FL_DUCKING)
   {
      v_src = pEdict->v.origin + Vector (0, 0, -19 + 19);
      v_forward = v_src + Vector (0, 0, 10) + vNormal * 24;

      TRACE_LINE (v_src, v_forward, dont_ignore_monsters, pEdict, &tr); // KWo - 13.01.2008

      // check if the trace hit something...
      if ((tr.flFraction < 1.0f) && (strncmp ("func_door", STRING (tr.pHit->v.classname), 9) != 0)
         && ((pBot->bot_team == TEAM_CS_TERRORIST) || !(FStrEq ("hostage_entity", STRING (tr.pHit->v.classname))))) // KWo - 13.01.2008
      {
         if (g_b_DebugStuck)
            ALERT(at_logged, "[DEBUG] Bot %s can't move forward - its body may hit something centrally (1).\n", pBot->name);

         return (TRUE); // bot's body will hit something
      }

      v_src = pEdict->v.origin + Vector (0, 0, -19 + 19);
      v_forward = v_src + vNormal * 24;

      TRACE_LINE (v_src, v_forward, dont_ignore_monsters, pEdict, &tr); // KWo - 13.01.2008

      // check if the trace hit something...
      if ((tr.flFraction < 1.0f) && (strncmp ("func_door", STRING (tr.pHit->v.classname), 9) != 0)
         && ((pBot->bot_team == TEAM_CS_TERRORIST) || !(FStrEq ("hostage_entity", STRING (tr.pHit->v.classname))))) // KWo - 13.01.2008
      {
         if (g_b_DebugStuck)
            ALERT(at_logged, "[DEBUG] Bot %s can't move forward - its body may hit something centrally (2).\n", pBot->name);

         return (TRUE); // bot's body will hit something
      }
   }
   else
   {
      // Trace from the left Waist to the right forward Waist Pos
      v_src = pEdict->v.origin + Vector (0, 0, -17) + gpGlobals->v_right * -16;
      v_forward = pEdict->v.origin + Vector (0, 0, -17) + gpGlobals->v_right * 16 + vNormal * 24;

      // trace from the bot's waist straight forward...
      TRACE_LINE (v_src, v_forward, dont_ignore_monsters, pEdict, &tr); // KWo - 13.01.2008

      // check if the trace hit something...
      if ((tr.flFraction < 1.0f) && (strncmp ("func_door", STRING (tr.pHit->v.classname), 9) != 0)
         && ((pBot->bot_team == TEAM_CS_TERRORIST) || !(FStrEq ("hostage_entity", STRING (tr.pHit->v.classname))))) // KWo - 13.01.2008
      {
         if (g_b_DebugStuck)
            ALERT(at_logged, "[DEBUG] Bot %s can't move forward - its waits may hit something on the right.\n", pBot->name);

         return (TRUE); // bot's body will hit something
      }

      // Trace from the left Waist to the right forward Waist Pos
      v_src = pEdict->v.origin + Vector (0, 0, -17) + gpGlobals->v_right * 16;
      v_forward = pEdict->v.origin + Vector (0, 0, -17) + gpGlobals->v_right * -16 + vNormal * 24;

      TRACE_LINE (v_src, v_forward, dont_ignore_monsters, pEdict, &tr); // KWo - 13.01.2008

      // check if the trace hit something...
      if ((tr.flFraction < 1.0f) && (strncmp ("func_door", STRING (tr.pHit->v.classname), 9) != 0)
         && ((pBot->bot_team == TEAM_CS_TERRORIST) || !(FStrEq ("hostage_entity", STRING (tr.pHit->v.classname))))) // KWo - 13.01.2008
      {
         if (g_b_DebugStuck)
            ALERT(at_logged, "[DEBUG] Bot %s can't move forward - its waits may hit something on the left.\n", pBot->name);

         return (TRUE); // bot's body will hit something
      }
   }

   return (FALSE); // bot can move forward, return (FALSE)
}

bool BotCantMoveBack (bot_t *pBot, Vector vNormal)  // KWo - 16.09.2006
{
   // Checks if Bot is blocked in his back movement direction (excluding doors)
   // use some TraceLines to determine if anything is blocking the current path of the bot.

   TraceResult tr;
   edict_t *pEdict = pBot->pEdict;
   Vector v_src, v_back, v_center;

   // first do a trace from the bot's eyes forward...

   v_src = GetGunPosition (pEdict); // EyePosition ()
   v_back = v_src + (-vNormal) * 24;

   // trace from the bot's eyes straight forward...
   TRACE_HULL (v_src, v_back, dont_ignore_monsters, head_hull, pEdict, &tr);  // KWo - 13.01.2008

   // check if the trace hit something...
   if ((tr.flFraction < 1.0f) && (!FNullEnt(tr.pHit)))
   {
      if ((strncmp ("func_door", STRING (tr.pHit->v.classname), 9) == 0)
            || ((pBot->bot_team == TEAM_CS_COUNTER) && (FStrEq ("hostage_entity", STRING (tr.pHit->v.classname))))) // KWo - 13.01.2008
         return (FALSE);

      if (g_b_DebugStuck)
         ALERT(at_logged, "[DEBUG] Bot %s can't move back - its head may hit someting.\n", pBot->name);

      return (TRUE); // bot's head will hit something
   }

   // bot's head is clear, check at shoulder level...
   // trace from the bot's shoulder left diagonal forward to the right shoulder...
   v_center = pEdict->v.angles;
   v_center.z = 0;
   v_center.x = 0;
   MAKE_VECTORS (v_center);

   v_src = GetGunPosition (pEdict) + Vector (0, 0, -16) + gpGlobals->v_right * -16;
   v_back = GetGunPosition (pEdict) + Vector (0, 0, -16) + gpGlobals->v_right * 16 + (-vNormal) * 24;

   TRACE_LINE (v_src, v_back, dont_ignore_monsters, pEdict, &tr); // KWo - 13.01.2008

   // check if the trace hit something...
   if ((tr.flFraction < 1.0f) && (strncmp ("func_door", STRING (tr.pHit->v.classname), 9) != 0)
      && ((pBot->bot_team == TEAM_CS_TERRORIST) || !(FStrEq ("hostage_entity", STRING (tr.pHit->v.classname))))) // KWo - 13.01.2008
   {
      if (g_b_DebugStuck)
         ALERT(at_logged, "[DEBUG] Bot %s can't move back - its shoulder may hit something on the right.\n", pBot->name);

      return (TRUE); // bot's body will hit something
   }
   // bot's head is clear, check at shoulder level...
   // trace from the bot's shoulder right diagonal back to the left shoulder...
   v_src = GetGunPosition (pEdict) + Vector (0, 0, -16) + gpGlobals->v_right * 16;
   v_back = GetGunPosition (pEdict) + Vector (0, 0, -16) + gpGlobals->v_right * -16 + (-vNormal) * 24;

   TRACE_LINE (v_src, v_back, dont_ignore_monsters, pEdict, &tr); // KWo - 13.01.2008

   // check if the trace hit something...
   if ((tr.flFraction < 1.0f) && (strncmp ("func_door", STRING (tr.pHit->v.classname), 9) != 0)
      && ((pBot->bot_team == TEAM_CS_TERRORIST) || !(FStrEq ("hostage_entity", STRING (tr.pHit->v.classname))))) // KWo - 13.01.2008
   {
      if (g_b_DebugStuck)
         ALERT(at_logged, "[DEBUG] Bot %s can't move back - its shoulder may hit something on the left.\n", pBot->name);

      return (TRUE); // bot's body will hit something
   }

   // Now check below Waist
   if (pEdict->v.flags & FL_DUCKING)
   {
      v_src = pEdict->v.origin + Vector (0, 0, -19 + 19);
      v_back = v_src + Vector (0, 0, 10) + (-vNormal) * 24;

      TRACE_LINE (v_src, v_back, dont_ignore_monsters, pEdict, &tr); // KWo - 13.01.2008

      // check if the trace hit something...
      if ((tr.flFraction < 1.0f) && (strncmp ("func_door", STRING (tr.pHit->v.classname), 9) != 0)
         && ((pBot->bot_team == TEAM_CS_TERRORIST) || !(FStrEq ("hostage_entity", STRING (tr.pHit->v.classname))))) // KWo - 13.01.2008
      {
         if (g_b_DebugStuck)
            ALERT(at_logged, "[DEBUG] Bot %s can't move back - its body may hit something centrally(1).\n", pBot->name);

         return (TRUE); // bot's body will hit something
      }

      v_src = pEdict->v.origin + Vector (0, 0, -19 + 19);
      v_back = v_src + (-vNormal) * 24;

      TRACE_LINE (v_src, v_back, dont_ignore_monsters, pEdict, &tr); // KWo - 13.01.2008

      // check if the trace hit something...
      if ((tr.flFraction < 1.0f) && (strncmp ("func_door", STRING (tr.pHit->v.classname), 9) != 0)
         && ((pBot->bot_team == TEAM_CS_TERRORIST) || !(FStrEq ("hostage_entity", STRING (tr.pHit->v.classname))))) // KWo - 13.01.2008
      {
         if (g_b_DebugStuck)
            ALERT(at_logged, "[DEBUG] Bot %s can't move back - its body may hit something centrally(2).\n", pBot->name);

         return (TRUE); // bot's body will hit something
      }
   }
   else
   {
      // Trace from the left Waist to the right back Waist Pos
      v_src = pEdict->v.origin + Vector (0, 0, -17) + gpGlobals->v_right * -16;
      v_back = pEdict->v.origin + Vector (0, 0, -17) + gpGlobals->v_right * 16 + (-vNormal) * 24;

      // trace from the bot's waist straight back...
      TRACE_LINE (v_src, v_back, dont_ignore_monsters, pEdict, &tr); // KWo - 13.01.2008

      // check if the trace hit something...
      if ((tr.flFraction < 1.0f) && (strncmp ("func_door", STRING (tr.pHit->v.classname), 9) != 0)
         && ((pBot->bot_team == TEAM_CS_TERRORIST) || !(FStrEq ("hostage_entity", STRING (tr.pHit->v.classname))))) // KWo - 13.01.2008
      {
         if (g_b_DebugStuck)
            ALERT(at_logged, "[DEBUG] Bot %s can't move back - its waits may hit something on the right.\n", pBot->name);

         return (TRUE); // bot's body will hit something
      }
      // Trace from the left Waist to the right forward Waist Pos
      v_src = pEdict->v.origin + Vector (0, 0, -17) + gpGlobals->v_right * 16;
      v_back = pEdict->v.origin + Vector (0, 0, -17) + gpGlobals->v_right * -16 + (-vNormal) * 24;

      TRACE_LINE (v_src, v_back, dont_ignore_monsters, pEdict, &tr); // KWo - 13.01.2008

      // check if the trace hit something...
      if ((tr.flFraction < 1.0f) && (strncmp ("func_door", STRING (tr.pHit->v.classname), 9) != 0)
         && ((pBot->bot_team == TEAM_CS_TERRORIST) || !(FStrEq ("hostage_entity", STRING (tr.pHit->v.classname))))) // KWo - 13.01.2008
      {
         if (g_b_DebugStuck)
            ALERT(at_logged, "[DEBUG] Bot %s can't move back - its waits may hit something on the left.\n", pBot->name);

         return (TRUE); // bot's body will hit something
      }
   }

   return (FALSE); // bot can move back, return (FALSE)
}


bool BotCanStrafeLeft (bot_t *pBot, TraceResult *tr)
{
   // Check if Bot can move sideways

   edict_t *pEdict = pBot->pEdict;
   Vector v_src;
   Vector v_left;
   Vector v_angle;

   v_angle = pEdict->v.v_angle;
   MAKE_VECTORS (v_angle);
   v_src = pEdict->v.origin;
   v_left = v_src + gpGlobals->v_right * -40;

   // trace from the bot's waist straight left...
   TRACE_LINE (v_src, v_left, ignore_monsters, pEdict, tr);

   // check if the trace hit something...
   if ((tr->flFraction < 1.0f) && !(FStrEq ("hostage_entity", STRING (tr->pHit->v.classname)))) // KWo - 14.08.2006
      return (FALSE); // bot's body will hit something

   v_src = v_left; // KWo - 09.07.2006
   v_left = v_left + gpGlobals->v_forward * 40;

   // trace from the strafe pos straight forward...
   TRACE_LINE (v_src, v_left, ignore_monsters, pEdict, tr);

   // check if the trace hit something...
   if ((tr->flFraction < 1.0f) && !(FStrEq ("hostage_entity", STRING (tr->pHit->v.classname)))) // KWo - 14.08.2006
      return (FALSE); // bot's body will hit something

   return (TRUE);
}


bool BotCanStrafeRight (bot_t *pBot, TraceResult *tr)
{
   // Check if Bot can move sideways

   edict_t *pEdict = pBot->pEdict;
   Vector v_src;
   Vector v_right;
   Vector v_angle;

   v_angle = pEdict->v.v_angle;
   MAKE_VECTORS (v_angle);
   v_src = pEdict->v.origin;
   v_right = v_src + gpGlobals->v_right * 40;

   // trace from the bot's waist straight right...
   TRACE_LINE (v_src, v_right, ignore_monsters, pEdict, tr);

   // check if the trace hit something...
   if ((tr->flFraction < 1.0f) && !(FStrEq ("hostage_entity", STRING (tr->pHit->v.classname))))
      return (FALSE); // bot's body will hit something


   v_src = v_right; // KWo - 09.07.2006
   v_right = v_right + gpGlobals->v_forward * 40;

   // trace from the strafe pos straight forward...
   TRACE_LINE (v_src, v_right, ignore_monsters, pEdict, tr);

   // check if the trace hit something...
   if ((tr->flFraction < 1.0f) && !(FStrEq ("hostage_entity", STRING (tr->pHit->v.classname))))
      return (FALSE); // bot's body will hit something

   return (TRUE);
}


bool BotCanJumpUp (bot_t *pBot, Vector vNormal)
{
   // Check if Bot can jump over some obstacle

   TraceResult tr;
   Vector v_jump;
   Vector v_source;
   Vector v_dest;
   edict_t *pEdict = pBot->pEdict;

   // Can't jump if not on ground and not on ladder/swimming
   if (!(pEdict->v.flags & (FL_ONGROUND | FL_PARTIALGROUND)) && (pBot->bOnLadder || !pBot->bInWater))
      return (FALSE);

   // convert current view angle to vectors for TraceLine math...

   v_jump = pEdict->v.angles;
   v_jump.x = 0; // reset pitch to 0 (level horizontally)
   v_jump.z = 0; // reset roll to 0 (straight up and down)

   MAKE_VECTORS (v_jump);

   // Check for normal jump height first...

   v_source = pEdict->v.origin + Vector (0, 0, -36 + 45);
   v_dest = v_source + vNormal * 32;

   // trace a line forward at maximum jump height...
   TRACE_LINE (v_source, v_dest, ignore_monsters, pEdict, &tr);

   if (tr.flFraction < 1.0f)
      goto CheckDuckJump;
   else
   {
      // now trace from jump height upward to check for obstructions...
      v_source = v_dest;
      v_dest.z = v_dest.z + 37;

      TRACE_LINE (v_source, v_dest, ignore_monsters, pEdict, &tr);

      if (tr.flFraction < 1.0f)
         return (FALSE);
   }

   // now check same height to one side of the bot...
   v_source = pEdict->v.origin + gpGlobals->v_right * 16 + Vector (0, 0, -36 + 45);
   v_dest = v_source + vNormal * 32;

   // trace a line forward at maximum jump height...
   TRACE_LINE (v_source, v_dest, ignore_monsters, pEdict, &tr);

   // if trace hit something, return (FALSE)
   if (tr.flFraction < 1.0f)
      goto CheckDuckJump;

   // now trace from jump height upward to check for obstructions...
   v_source = v_dest;
   v_dest.z = v_dest.z + 37;

   TRACE_LINE (v_source, v_dest, ignore_monsters, pEdict, &tr);

   // if trace hit something, return (FALSE)
   if (tr.flFraction < 1.0f)
      return (FALSE);

   // now check same height on the other side of the bot...
   v_source = pEdict->v.origin + (-gpGlobals->v_right * 16) + Vector (0, 0, -36 + 45);
   v_dest = v_source + vNormal * 32;

   // trace a line forward at maximum jump height...
   TRACE_LINE (v_source, v_dest, ignore_monsters, pEdict, &tr);

   // if trace hit something, return (FALSE)
   if (tr.flFraction < 1.0f)
      goto CheckDuckJump;

   // now trace from jump height upward to check for obstructions...
   v_source = v_dest;
   v_dest.z = v_dest.z + 37;

   TRACE_LINE (v_source, v_dest, ignore_monsters, pEdict, &tr);

   // if trace hit something, return (FALSE)
   if (tr.flFraction < 1.0f)
      return (FALSE);

   return (TRUE);

   // Here we check if a Duck Jump would work...
CheckDuckJump:
   // use center of the body first...

   // maximum duck jump height is 62, so check one unit above that (63)
   v_source = pEdict->v.origin + Vector (0, 0, -36 + 63);
   v_dest = v_source + vNormal * 32;

   // trace a line forward at maximum jump height...
   TRACE_LINE (v_source, v_dest, ignore_monsters, pEdict, &tr);

   if (tr.flFraction < 1.0f)
      return (FALSE);
   else
   {
      // now trace from jump height upward to check for obstructions...
      v_source = v_dest;
      v_dest.z = v_dest.z + 37;

      TRACE_LINE (v_source, v_dest, ignore_monsters, pEdict, &tr);

      // if trace hit something, check duckjump
      if (tr.flFraction < 1.0f)
         return (FALSE);
   }

   // now check same height to one side of the bot...
   v_source = pEdict->v.origin + gpGlobals->v_right * 16 + Vector (0, 0, -36 + 63);
   v_dest = v_source + vNormal * 32;

   // trace a line forward at maximum jump height...
   TRACE_LINE (v_source, v_dest, ignore_monsters, pEdict, &tr);

   // if trace hit something, return (FALSE)
   if (tr.flFraction < 1.0f)
      return (FALSE);

   // now trace from jump height upward to check for obstructions...
   v_source = v_dest;
   v_dest.z = v_dest.z + 37;

   TRACE_LINE (v_source, v_dest, ignore_monsters, pEdict, &tr);

   // if trace hit something, return (FALSE)
   if (tr.flFraction < 1.0f)
      return (FALSE);

   // now check same height on the other side of the bot...
   v_source = pEdict->v.origin + (-gpGlobals->v_right * 16) + Vector (0, 0, -36 + 63);
   v_dest = v_source + vNormal * 32;

   // trace a line forward at maximum jump height...
   TRACE_LINE (v_source, v_dest, ignore_monsters, pEdict, &tr);

   // if trace hit something, return (FALSE)
   if (tr.flFraction < 1.0f)
      return (FALSE);

   // now trace from jump height upward to check for obstructions...
   v_source = v_dest;
   v_dest.z = v_dest.z + 37;

   TRACE_LINE (v_source, v_dest, ignore_monsters, pEdict, &tr);

   // if trace hit something, return (FALSE)
   if (tr.flFraction < 1.0f)
      return (FALSE);

   return (TRUE);
}


bool BotCanDuckUnder (bot_t *pBot, Vector vNormal)
{
   // Check if Bot can duck under Obstacle

   TraceResult tr;
   Vector v_duck, v_baseheight,v_source, v_dest;
   edict_t *pEdict = pBot->pEdict;

   // convert current view angle to vectors for TraceLine math...

   v_duck = pEdict->v.angles;
   v_duck.x = 0; // reset pitch to 0 (level horizontally)
   v_duck.z = 0; // reset roll to 0 (straight up and down)

   MAKE_VECTORS (v_duck);

   // use center of the body first...

   if (pEdict->v.flags & FL_DUCKING)
      v_baseheight = pEdict->v.origin + Vector (0, 0, -17);
   else
      v_baseheight = pEdict->v.origin;

   v_source = v_baseheight;
   v_dest = v_source + vNormal * 32;

   // trace a line forward at duck height...
   TRACE_LINE (v_source, v_dest, ignore_monsters, pEdict, &tr);

   // if trace hit something, return (FALSE)
   if (tr.flFraction < 1.0f)
      return (FALSE);

   // now check same height to one side of the bot...
   v_source = v_baseheight + gpGlobals->v_right * 16;
   v_dest = v_source + vNormal * 32;

   // trace a line forward at duck height...
   TRACE_LINE (v_source, v_dest, ignore_monsters, pEdict, &tr);

   // if trace hit something, return (FALSE)
   if (tr.flFraction < 1.0f)
      return (FALSE);

   // now check same height on the other side of the bot...
   v_source = v_baseheight + (-gpGlobals->v_right * 16);
   v_dest = v_source + vNormal * 32;

   // trace a line forward at duck height...
   TRACE_LINE (v_source, v_dest, ignore_monsters, pEdict, &tr);

   // if trace hit something, return (FALSE)
   if (tr.flFraction < 1.0f)
      return (FALSE);

   return (TRUE);
}


bool BotIsBlockedLeft (bot_t *pBot)
{
   edict_t *pEdict = pBot->pEdict;
   Vector v_src, v_left;
   TraceResult tr;

   int iDirection = 48;

   if (pBot->f_move_speed < 0.0f)
      iDirection = -48;

   MAKE_VECTORS (pEdict->v.angles);

   // do a trace to the left...

   v_src = pEdict->v.origin;
   v_left = v_src + (gpGlobals->v_forward * iDirection) + (gpGlobals->v_right * -48); // 48 units to the left

   TRACE_LINE (v_src, v_left, ignore_monsters, pEdict, &tr);

   // check if the trace hit something...
   if ((tr.flFraction < 1.0f) && (strncmp ("func_door", STRING (tr.pHit->v.classname), 9) != 0))
      return (TRUE);

   return (FALSE);
}


bool BotIsBlockedRight (bot_t *pBot)
{
   edict_t *pEdict = pBot->pEdict;
   Vector v_src, v_left;
   TraceResult tr;

   int iDirection = 48;

   if (pBot->f_move_speed < 0.0f)
      iDirection = -48;
   MAKE_VECTORS (pEdict->v.angles);

   // do a trace to the left...

   v_src = pEdict->v.origin;
   v_left = v_src + (gpGlobals->v_forward * iDirection) + (gpGlobals->v_right * 48); // 48 units to the right

   TRACE_LINE (v_src, v_left, ignore_monsters, pEdict, &tr);

   // check if the trace hit something...
   if ((tr.flFraction < 1.0f) && (strncmp ("func_door", STRING (tr.pHit->v.classname), 9) != 0))
      return (TRUE);

   return (FALSE);
}


bool BotCheckWallOnLeft (bot_t *pBot)
{
   edict_t *pEdict = pBot->pEdict;
   Vector v_src, v_left;
   TraceResult tr;

   MAKE_VECTORS (pEdict->v.angles);

   // do a trace to the left...

   v_src = pEdict->v.origin;
   v_left = v_src + (-gpGlobals->v_right * 40); // 40 units to the left

   TRACE_LINE (v_src, v_left, ignore_monsters, pEdict, &tr);

   // check if the trace hit something...
   if (tr.flFraction < 1.0f)
      return (TRUE);

   return (FALSE);
}


bool BotCheckWallOnRight (bot_t *pBot)
{
   edict_t *pEdict = pBot->pEdict;
   Vector v_src, v_right;
   TraceResult tr;

   MAKE_VECTORS (pEdict->v.angles);

   // do a trace to the right...

   v_src = pEdict->v.origin;
   v_right = v_src + gpGlobals->v_right * 40; // 40 units to the right

   TRACE_LINE (v_src, v_right, ignore_monsters, pEdict, &tr);

   // check if the trace hit something...
   if (tr.flFraction < 1.0f)
      return (TRUE);

   return (FALSE);
}

void BotSetStrafeSpeed (bot_t *pBot, float fStrafeSpeed) // KWo - 25.07.2006
{

   edict_t *pEdict = pBot->pEdict;

   Vector vecRight;
   Vector vecForward;
   Vector vecDestination = g_vecZero;

   if (fStrafeSpeed == 0)
      return;
   MAKE_VECTORS (pEdict->v.angles);

   vecForward = gpGlobals->v_forward;
   vecRight = gpGlobals->v_right;

   if (fStrafeSpeed > 0.0f)
      vecDestination = pEdict->v.origin + 30.0f * vecRight;
   else
      vecDestination = pEdict->v.origin + (-30.0f) * vecRight;

   if (!IsDeadlyDrop (pBot, vecDestination))
   {
      pBot->f_sidemove_speed = fStrafeSpeed;
   }
   return;
}


void BotGetCampDirection (bot_t *pBot, Vector *vecDest)
{
   // Check if View on last Enemy Position is blocked - replace with better Vector then
   // Mostly used for getting a good Camping Direction Vector if not camping on a camp waypoint

   TraceResult tr;
   edict_t *pEdict;
   Vector vecSource;
   float distance1;
   float min_distance1;
   float distance2;
   float min_distance2;
   float f_length;
   int iLookAtWaypoint;
   int indexbot = 1;  // KWo - to remove warning uninitialised
   int indexenemy = 1;  // KWo - to remove warning uninitialised
   int i;

   pEdict = pBot->pEdict;
   vecSource = GetGunPosition (pEdict); // EyePosition ()

   TRACE_LINE (vecSource, *vecDest, ignore_monsters, pEdict, &tr);

   // check if the trace hit something...
   if (tr.flFraction < 1.0f)
   {
      min_distance1 = 9999.0f;
      min_distance2 = 9999.0f;
      f_length = (tr.vecEndPos - vecSource).Length ();

      if (f_length > 300.0f) // KWo - 18.02.2008
         return;

      // Find Nearest Waypoint to Bot and Position
      for (i = 0; i < g_iNumWaypoints; i++)
      {
         distance1 = (paths[i]->origin - pEdict->v.origin).Length ();

         if (distance1 < min_distance1)
         {
            min_distance1 = distance1;
            indexbot = i;
         }

         distance2 = (paths[i]->origin - *vecDest).Length ();

         if (distance2 < min_distance2)
         {
            min_distance2 = distance2;
            indexenemy = i;
         }
      }

      min_distance1 = 9999.0f;
      iLookAtWaypoint = -1;

      for (i = 0; i < MAX_PATH_INDEX; i++)
      {
         if (paths[indexbot]->index[i] == -1)
            continue;

         distance1 = GetPathDistance (paths[indexbot]->index[i], indexenemy);

         if (distance1 < min_distance1)
         {
            min_distance1 = distance1;
            iLookAtWaypoint = paths[indexbot]->index[i]; // KWo - 18.02.2008 - strange bug since old PB... :)
         }
      }

      if ((iLookAtWaypoint != -1) && (iLookAtWaypoint < g_iNumWaypoints))
         *vecDest = paths[iLookAtWaypoint]->origin + pEdict->v.view_ofs;  // KWo - 14.08.2007
   }
}

void UpdateGlobalExperienceData (void)
{
   // Called after each End of the Round to update knowledge about
   // the most dangerous waypoints for each Team.

   unsigned short max_damage;	// KWo 14.01.2006 - changed the name of variable
   unsigned short act_damage;
   int iBestIndex, i, j;
   int iClip;

   // No waypoints, no experience used or waypoints edited OR being edited ?
   if ((g_iNumWaypoints < 1) || g_bWaypointsChanged || /* g_bWaypointOn || */ (g_iUpdGlExpState == 0))
   {
      g_bRecalcKills = FALSE;
      g_iUpdGlExpState = 0;
      return;
   }

   switch(g_iUpdGlExpState)    // KWo - 02.05.2006
   {
   case 1:

      // Get the most dangerous Waypoint for this Position for Team 0
      for (i = 0; i < g_iNumWaypoints; i++)
      {
         max_damage = 0;
         iBestIndex = -1;

         for (j = 0; j < g_iNumWaypoints; j++)
         {
            if (i == j)
            {
               if ((pBotExperienceData + (i * g_iNumWaypoints) + j)->uTeam0Damage >= MAX_DAMAGE_VAL)  // KWo - 09.04.2006
                  g_bRecalcKills = TRUE;
               continue;
            }

            act_damage = (pBotExperienceData + (i * g_iNumWaypoints) + j)->uTeam0Damage;

            if (act_damage > max_damage)
            {
               max_damage = act_damage;
               iBestIndex = j;
            }
         }

         if (max_damage >= MAX_DAMAGE_VAL)  // KWo - 23.01.2006
            g_bRecalcKills = TRUE;

         (pBotExperienceData + (i * g_iNumWaypoints) + i)->iTeam0_danger_index = (short) iBestIndex;
      }
      g_iUpdGlExpState++;
      break;

   case 2:
      // Get the most dangerous Waypoint for this Position for Team 1
      for (i = 0; i < g_iNumWaypoints; i++)
      {
         max_damage = 0;
         iBestIndex = -1;

         for (j = 0; j < g_iNumWaypoints; j++)
         {
            if (i == j)
            {
               if ((pBotExperienceData + (i * g_iNumWaypoints) + j)->uTeam1Damage >= MAX_DAMAGE_VAL)  // KWo - 09.04.2006
                  g_bRecalcKills = TRUE;
               continue;
            }

            act_damage = (pBotExperienceData + (i * g_iNumWaypoints) + j)->uTeam1Damage;

            if (act_damage > max_damage)
            {
               max_damage = act_damage;
               iBestIndex = j;
            }
         }

         if (max_damage >= MAX_DAMAGE_VAL)
            g_bRecalcKills = TRUE;

         (pBotExperienceData + (i * g_iNumWaypoints) + i)->iTeam1_danger_index = (short) iBestIndex;
      }
      g_iUpdGlExpState++;
      break;

   case 3:
      // Adjust Values if overflow is about to happen
      if (g_bRecalcKills)
      {
         for (i = 0; i < g_iNumWaypoints; i++)
         {
            for (j = 0; j < g_iNumWaypoints; j++)
            {
               iClip = (pBotExperienceData + (i * g_iNumWaypoints) + j)->uTeam0Damage;
               iClip -= (int) (MAX_DAMAGE_VAL * 0.5f);	// KWo - to remove warning

               if (iClip < 0)
                  iClip = 0;

               (pBotExperienceData + (i * g_iNumWaypoints) + j)->uTeam0Damage = (unsigned short) iClip;

               iClip = (pBotExperienceData + (i * g_iNumWaypoints) + j)->uTeam1Damage;
               iClip -= (int) (MAX_DAMAGE_VAL * 0.5f);	// KWo - to remove warning

               if (iClip < 0)
                  iClip = 0;

               (pBotExperienceData + (i * g_iNumWaypoints) + j)->uTeam1Damage = (unsigned short) iClip;

            }
         }
// 	KWo - 09.04.2006
         iClip = (int) g_iHighestDamageT - (int) (MAX_DAMAGE_VAL * 0.5f);
         if (iClip < 1)
            iClip = 1;
         g_iHighestDamageT = (unsigned short) iClip;

         iClip = (int) g_iHighestDamageCT - (int) (MAX_DAMAGE_VAL * 0.5f);
         if (iClip < 1)
            iClip = 1;
         g_iHighestDamageCT = (unsigned short) iClip;
      }
      g_iUpdGlExpState++;
      break;

   case 4:
      g_cKillHistory++;

      if (g_cKillHistory == MAX_KILL_HIST)
      {
         for (i = 0; i < g_iNumWaypoints; i++)
         {
            (pBotExperienceData + (i * g_iNumWaypoints) + i)->uTeam0Damage /= (unsigned short) (gpGlobals->maxClients * 0.5f); // KWo - to remove warning
            (pBotExperienceData + (i * g_iNumWaypoints) + i)->uTeam1Damage /= (unsigned short) (gpGlobals->maxClients * 0.5f); // KWo - to remove warning
         }
// KWo - 09.04.2006
         g_iHighestDamageT /= (unsigned short) (gpGlobals->maxClients * 0.5f);
         if (g_iHighestDamageT < 1)
            g_iHighestDamageT = 1;
         g_iHighestDamageCT /= (unsigned short) (gpGlobals->maxClients * 0.5f);
         if (g_iHighestDamageCT < 1)
            g_iHighestDamageCT = 1;

         g_cKillHistory = 1;
      }
      g_iUpdGlExpState = 0;

      if (g_b_DebugNavig)
         ALERT(at_logged, "[DEBUG] UpdateGlobalExperienceData executed in 4 steps.\n");

      break;
   }
   return;
}

void BotCollectGoalExperience (bot_t *pBot, int iDamage)
{
   // Gets called each time a Bot gets damaged by some enemy.
   // Tries to achieve a statistic about most/less dangerous waypoints
   // for a destination goal used for pathfinding

   int iWPTValue;

   if ((g_iNumWaypoints < 1) || g_bWaypointsChanged || (BotGetSafeTask (pBot)->iTask == TASK_FOLLOWUSER)
       || (pBot->chosengoal_index < 0) || (pBot->chosengoal_index >= g_iNumWaypoints) || (pBot->prev_goal_index < 0)) // KWo - 30.06.2006
      return;

   // Only rate Goal Waypoint if Bot died because of the damage
   // FIXME: Could be done a lot better, however this cares most
   // about damage done by sniping or really deadly weapons
   if (pBot->pEdict->v.health - iDamage <= 0)
   {
      if (pBot->bot_team == TEAM_CS_TERRORIST)
      {
         iWPTValue = (int) ((pBotExperienceData + (pBot->chosengoal_index * g_iNumWaypoints) + pBot->prev_goal_index)->wTeam0Value - (pBot->pEdict->v.health * 0.05));
         if (iWPTValue < -MAX_GOAL_VAL)
            iWPTValue = -MAX_GOAL_VAL;
         else if (iWPTValue > MAX_GOAL_VAL)
            iWPTValue = MAX_GOAL_VAL;
         (pBotExperienceData + (pBot->chosengoal_index * g_iNumWaypoints) + pBot->prev_goal_index)->wTeam0Value = (signed short) iWPTValue;
      }
      else
      {
         iWPTValue = (int) ((pBotExperienceData + (pBot->chosengoal_index * g_iNumWaypoints) + pBot->prev_goal_index)->wTeam1Value - (pBot->pEdict->v.health * 0.05));
         if (iWPTValue<-MAX_GOAL_VAL)
            iWPTValue = -MAX_GOAL_VAL;
         else if (iWPTValue>MAX_GOAL_VAL)
            iWPTValue = MAX_GOAL_VAL;
         (pBotExperienceData + (pBot->chosengoal_index * g_iNumWaypoints) + pBot->prev_goal_index)->wTeam1Value = (signed short) iWPTValue;
      }
   }
}


void BotCollectExperienceData (edict_t *pVictimEdict, edict_t *pAttackerEdict, int iDamage)
{
   // Gets called each time a Bot gets damaged by some enemy.
   // Stores the damage (teamspecific) done to the Victim
   // FIXME: Should probably rate damage done by humans higher...

   if (FNullEnt (pVictimEdict) || FNullEnt (pAttackerEdict) || g_bWaypointsChanged) // KWo - 25.01.2010
      return;

   int iVictimTeam = UTIL_GetTeam (pVictimEdict);
   int iAttackerTeam = UTIL_GetTeam (pAttackerEdict);

   if ((iVictimTeam == iAttackerTeam) && !g_b_cv_ffa) // KWo - 25.01.2010
      return;

   float distance;
   float min_distance_victim = 9999.0f;
   float min_distance_attacker = 9999.0f;

   int VictimIndex = 0;	// KWo - 15.01.2006
   int AttackerIndex = 0;	// KWo - 15.01.2006
   int i;

   int iDangerIndexCT = -1;   // KWo - 25.01.2010
   int iDangerIndexT = -1;    // KWo - 25.01.2010
   int iDamageDITeamCT = 0;   // KWo - 25.01.2010
   int iDamageDITeamT = 0;    // KWo - 25.01.2010

   int iValue;

   bot_t *pBot = UTIL_GetBotPointer (pVictimEdict);

   // If these are Bots also remember damage to rank the destination of the Bot
   if (pBot != NULL)
   {
      pBot->f_goal_value -= (float)iDamage;
      if ((pBot->curr_wpt_index >= 0) && (pBot->curr_wpt_index < g_iNumWaypoints)
         && (pBot->prev_wpt_index[0] >= 0) && (pBot->prev_wpt_index[0] < g_iNumWaypoints)) // KWo - 26.01.2010
      {
         VictimIndex = pBot->curr_wpt_index;
         min_distance_victim = (paths[pBot->curr_wpt_index]->origin - pVictimEdict->v.origin).Length ();
         distance = (paths[pBot->prev_wpt_index[0]]->origin - pVictimEdict->v.origin).Length ();
         if (distance < min_distance_victim)
         {
            min_distance_victim = distance;
            VictimIndex = pBot->prev_wpt_index[0];
         }
         if (distance > 200.0f)
         {
            min_distance_victim = 9999.9f;
            VictimIndex = 0;
         }
      }
   }

   pBot = UTIL_GetBotPointer (pAttackerEdict);
   if (pBot != NULL)
   {
      pBot->f_goal_value += (float)iDamage;
      if ((pBot->curr_wpt_index >= 0) && (pBot->curr_wpt_index < g_iNumWaypoints)
         && (pBot->prev_wpt_index[0] >= 0) && (pBot->prev_wpt_index[0] < g_iNumWaypoints))  // KWo - 26.01.2010
      {
         AttackerIndex = pBot->curr_wpt_index;
         min_distance_attacker = (paths[pBot->curr_wpt_index]->origin - pVictimEdict->v.origin).Length ();
         distance = (paths[pBot->prev_wpt_index[0]]->origin - pVictimEdict->v.origin).Length ();
         if (distance < min_distance_attacker)
         {
            min_distance_attacker = distance;
            AttackerIndex = pBot->prev_wpt_index[0];
         }
         if (distance > 200.0f)
         {
            min_distance_attacker = 9999.9f;
            AttackerIndex = 0;
         }
      }
   }

   if ((VictimIndex == 0) && (AttackerIndex == 0)) // KWo - 26.01.2010
   {
   // Find Nearest Waypoint to Attacker/Victim
      for (i = 0; i < g_iNumWaypoints; i++)
      {
         distance = (paths[i]->origin - pVictimEdict->v.origin).Length ();
         if (distance < min_distance_victim)
         {
            min_distance_victim = distance;
            VictimIndex = i;
         }

         distance = (paths[i]->origin - pAttackerEdict->v.origin).Length ();
         if (distance < min_distance_attacker)
         {
            min_distance_attacker = distance;
            AttackerIndex = i;
         }
      }
   }
   else if (VictimIndex == 0)
   {
      for (i = 0; i < g_iNumWaypoints; i++)
      {
         distance = (paths[i]->origin - pVictimEdict->v.origin).Length ();
         if (distance < min_distance_victim)
         {
            min_distance_victim = distance;
            VictimIndex = i;
         }
      }
   }
   else if (AttackerIndex == 0)
   {
      for (i = 0; i < g_iNumWaypoints; i++)
      {
         distance = (paths[i]->origin - pAttackerEdict->v.origin).Length ();
         if (distance < min_distance_attacker)
         {
            min_distance_attacker = distance;
            AttackerIndex = i;
         }
      }
   }

   if (g_b_DebugNavig)
      ALERT(at_logged, "[DEBUG] BotCollectExperienceData - Bot %s collects damage %d experience.\n", STRING(pVictimEdict->v.netname), iDamage);

   if (iDamage > 20)	// KWo - 12.02.2008
   {                // WPs nearby - affected 0.1 * iDamage - should it works better now?
      PATH *p;
      short int path_index;
      if (iVictimTeam == TEAM_CS_TERRORIST)
      {
         iValue = (pBotExperienceData + (VictimIndex * g_iNumWaypoints) + VictimIndex)->uTeam0Damage;
         iValue += (int) ((float) iDamage * 1.0f); // KWo 09.04.2006
         if (iValue > MAX_DAMAGE_VAL)
         {
            iValue = MAX_DAMAGE_VAL;
            // Update Global Experience Data if overflow is about to happen
            if (g_iUpdGlExpState == 0) // KWo - 28.12.2009
               g_iUpdGlExpState = 1;
         }

         if (iValue > g_iHighestDamageT) // KWo - 09.04.2006
         {
            g_iHighestDamageT = (unsigned short)iValue;
            g_iHighestDamageWpT = VictimIndex; // KWo - 05.01.2008
         }

         (pBotExperienceData + (VictimIndex * g_iNumWaypoints) + VictimIndex)->uTeam0Damage = (unsigned short)iValue;
         p = paths[VictimIndex];
         for (i = 0; i < MAX_PATH_INDEX; i++)
         {
            if ((p->index[i] > -1) && (p->index[i] < MAX_WAYPOINTS) )
            {
               path_index = p->index[i];
               iValue = (pBotExperienceData + (path_index * g_iNumWaypoints) + path_index)->uTeam0Damage;
               iValue += (int) ((float) iDamage * 0.1); // KWo - 09.04.2006
               if (iValue > MAX_DAMAGE_VAL)
               {
                  iValue = MAX_DAMAGE_VAL;
                  // Update Global Experience Data if overflow is about to happen
                  if (g_iUpdGlExpState == 0) // KWo - 28.12.2009
                     g_iUpdGlExpState = 1;
               }

               if (iValue > g_iHighestDamageT) // KWo - 09.04.2006
               {
                  g_iHighestDamageT = (unsigned short)iValue;
                  g_iHighestDamageWpT = path_index; // KWo - 05.01.2008
               }

               (pBotExperienceData + (path_index * g_iNumWaypoints) + path_index)->uTeam0Damage = (unsigned short)iValue;
            }
         }
      }
      else
      {
         iValue = (pBotExperienceData + (VictimIndex * g_iNumWaypoints) + VictimIndex)->uTeam1Damage;
         iValue += (int) ((float) iDamage * 1.0f); // KWo - 09.04.2006
         if (iValue > MAX_DAMAGE_VAL)
         {
            iValue = MAX_DAMAGE_VAL;
            // Update Global Experience Data if overflow is about to happen
            if (g_iUpdGlExpState == 0) // KWo - 28.12.2009
               g_iUpdGlExpState = 1;
         }

         if (iValue > g_iHighestDamageCT) // KWo - 09.04.2006
         {
            g_iHighestDamageCT = (unsigned short)iValue;
            g_iHighestDamageWpCT = VictimIndex; // KWo - 05.01.2008
         }

         (pBotExperienceData + (VictimIndex * g_iNumWaypoints) + VictimIndex)->uTeam1Damage = (unsigned short)iValue;
         p = paths[VictimIndex];
         for (i = 0; i < MAX_PATH_INDEX; i++)
         {
            if ((p->index[i] > -1) && (p->index[i] < MAX_WAYPOINTS) )
            {
               path_index = p->index[i];
               iValue = (pBotExperienceData + (path_index * g_iNumWaypoints) + path_index)->uTeam1Damage;
               iValue += (int) ((float) iDamage * 0.1f);  // KWo - 09.04.2006
               if (iValue > MAX_DAMAGE_VAL)
               {
                   iValue = MAX_DAMAGE_VAL;
                  // Update Global Experience Data if overflow is about to happen
                  if (g_iUpdGlExpState == 0) // KWo - 28.12.2009
                     g_iUpdGlExpState = 1;
               }

               if (iValue > g_iHighestDamageCT) // KWo - 09.04.2006
               {
                  g_iHighestDamageCT = (unsigned short)iValue;
                  g_iHighestDamageWpCT = path_index; // KWo - 05.01.2008
               }

               (pBotExperienceData + (path_index * g_iNumWaypoints) + path_index)->uTeam1Damage = (unsigned short)iValue;
            }
         }
      }
   }

   if (VictimIndex == AttackerIndex)
      return;

   // Store away the damage done
   if (iVictimTeam == TEAM_CS_TERRORIST)
   {
      iDangerIndexT = (pBotExperienceData + VictimIndex * g_iNumWaypoints + VictimIndex)->iTeam0_danger_index; // KWo - 25.01.2010
      if ((iDangerIndexT > -1) && (iDangerIndexT < g_iNumWaypoints))
         iDamageDITeamT = (pBotExperienceData + (VictimIndex * g_iNumWaypoints) + iDangerIndexT)->uTeam0Damage;   // KWo - 25.01.2010

      iValue = (pBotExperienceData + (VictimIndex * g_iNumWaypoints) + AttackerIndex)->uTeam0Damage; // KWo - 25.01.2010
      iValue += (int) ((float) iDamage * 1.0f);	// KWo - to remove warning

      if (iValue >= iDamageDITeamT) // KWo - 25.01.2010
      {
         iDangerIndexT = AttackerIndex;
         iDamageDITeamT = iValue;
         if (iDamageDITeamT > MAX_DAMAGE_VAL)
            iDamageDITeamT = MAX_DAMAGE_VAL;
         (pBotExperienceData + (VictimIndex * g_iNumWaypoints) + iDangerIndexT)->uTeam0Damage = (unsigned short)iDamageDITeamT;
         (pBotExperienceData + (VictimIndex * g_iNumWaypoints) + VictimIndex)->iTeam0_danger_index = iDangerIndexT;
      }

      if (iValue > MAX_DAMAGE_VAL)
      {
         iValue = MAX_DAMAGE_VAL;
         // Update Global Experience Data if overflow is about to happen
         if (g_iUpdGlExpState == 0) // KWo - 28.12.2009
            g_iUpdGlExpState = 1;
      }

      (pBotExperienceData + (VictimIndex * g_iNumWaypoints) + AttackerIndex)->uTeam0Damage = (unsigned short)iValue;
   }
   else
   {
      iDangerIndexCT = (pBotExperienceData + VictimIndex * g_iNumWaypoints + VictimIndex)->iTeam1_danger_index;   // KWo - 25.01.2010
      if ((iDangerIndexCT > -1) && (iDangerIndexCT < g_iNumWaypoints))  // KWo - 25.01.2010
         iDamageDITeamCT = (pBotExperienceData + (VictimIndex * g_iNumWaypoints) + iDangerIndexCT)->uTeam1Damage;

      iValue = (pBotExperienceData + (VictimIndex * g_iNumWaypoints) + AttackerIndex)->uTeam1Damage;
      iValue += (int) ((float) iDamage * 1.0f);	// KWo - to remove warning

      if (iValue >= iDamageDITeamCT)   // KWo - 25.01.2010
      {
         iDangerIndexCT = AttackerIndex;
         iDamageDITeamCT = iValue;
         if (iDamageDITeamCT > MAX_DAMAGE_VAL)
            iDamageDITeamCT = MAX_DAMAGE_VAL;
         (pBotExperienceData + (VictimIndex * g_iNumWaypoints) + iDangerIndexCT)->uTeam1Damage = (unsigned short)iDamageDITeamCT;
         (pBotExperienceData + (VictimIndex * g_iNumWaypoints) + VictimIndex)->iTeam1_danger_index = iDangerIndexCT;
      }

      if (iValue > MAX_DAMAGE_VAL)
      {
         iValue = MAX_DAMAGE_VAL;
         // Update Global Experience Data if overflow is about to happen
         if (g_iUpdGlExpState == 0) // KWo - 28.12.2009
            g_iUpdGlExpState = 1;
      }

      (pBotExperienceData + (VictimIndex * g_iNumWaypoints) + AttackerIndex)->uTeam1Damage = (unsigned short)iValue;
   }

   return;
}

bool BotDoWaypointNav (bot_t *pBot)
{
   // Does the main Path Navigation...
   static edict_t *pEdict;
   static TraceResult tr;
   static TraceResult tr2;
   static TraceResult tr3;
   static TraceResult tr4;
   static bool bFoundGround;
   static bool bLiftExistDoorClosed;
   static bool bNeedWait;
   static bool bLiftUsed;
   static bool bPathValid;
   static bool bDoorHasTargetButton;
   static bool bPreviousWP_OK;
   static float f_button_distance;
   static float f_button_min_distance;
   static float fDistance;
   static float fDesiredDistance;
   static float wpt_distance;
   static float flDot;
   static float fCone;
   static float f_tr2FlFraction;
   static float f_tr3FlFraction;
   static float fOwnLightLevel;
   static int i;
   static int iWPTValue;
   static int iStartIndex;
   static int iGoalIndex;
   static int iWPTIndex;
   static int iTempIndex;
   static int iPreviousWpt;
   static int iAmountLooped;
   static int iButtonPushProp;
   static int EntHitIndex;
   static size_t iStrLen;
   static signed char cNearestButtonIndex;
   static char cCollStateIndex;
   static Vector v_trace_start;
   static Vector v_trace_end;
   static Vector vecDirection;
   static Vector v_button_origin;
   static Vector vecOrg;
   static Vector vBotOrigin;
   static Vector vWptLadder;
   static Vector vPlayerOrigin;
   static Vector v_src;
   static Vector v_tr23_start;
   static Vector v_OriginNextFrame;

   pEdict = pBot->pEdict;
   iAmountLooped = 0;
   bPreviousWP_OK = ((pBot->prev_wpt_index[0] >= 0) && (pBot->prev_wpt_index[0] < g_iNumWaypoints));
   if (pBot->curr_wpt_index == -1)
      GetValidWaypoint (pBot); // check if we need to find a waypoint...

   if ( ((pBot->f_timeHitDoor < gpGlobals->time)
       || ((pBot->cCollideMoves[(int) pBot->cCollStateIndex] != COLLISION_GOBACK)
           && (pBot->cCollideMoves[(int) pBot->cCollStateIndex] != COLLISION_STRAFELEFT)
           && (pBot->cCollideMoves[(int) pBot->cCollStateIndex] != COLLISION_STRAFERIGHT))
            || (pBot->f_probe_time + 0.1 < gpGlobals->time))
       && (pBot->f_timeDoorOpen <= gpGlobals->time) && (pBot->fButtonPushTime - 1.95 <= gpGlobals->time)) // KWo - 30.12.2009
   {
      pBot->dest_origin = pBot->wpt_origin;

      if (g_b_DebugNavig)
      {
         ALERT (at_logged, "[DEBUG] BotDoWaypointNav - Bot %s sets the dest origin as WP origin.\n", pBot->name);
      }
   }
   else if (bPreviousWP_OK && (pBot->cCollideMoves[(int) pBot->cCollStateIndex] == COLLISION_GOBACK)) // KWo - 14.08.2006
      pBot->dest_origin = paths[pBot->prev_wpt_index[0]]->origin;

   wpt_distance = (pEdict->v.origin - pBot->wpt_origin).Length ();

   if (g_b_DebugNavig)
   {
      ALERT (at_logged, "[DEBUG] BotDoWaypointNav - Bot %s wpt distance %i, dest. distance = %i.\n", pBot->name,
      (int) wpt_distance, (int) (pEdict->v.origin - pBot->dest_origin).Length ());
   }

   // Initialize the radius for a special waypoint type, where the wpt
   // is considered to be reached
   if ((!(paths[pBot->curr_wpt_index]->flags & W_FL_CROUCH) && (pEdict->v.flags & FL_DUCKING))
      || (paths[pBot->curr_wpt_index]->flags & W_FL_GOAL)) // KWo - 15.02.2008
      fDesiredDistance = 25.0f;
   else if (pBot->bOnLadder)
      fDesiredDistance = 15.0f;
   else if (paths[pBot->curr_wpt_index]->flags & W_FL_USE_BUTTON) // KWo - 09.07.2006
   {
      if (!(pBot->bNewButtonNoticed)) // KWo - 14.07.2006
         pBot->fButtonNoticedTime = gpGlobals->time;
      fOwnLightLevel = UTIL_IlluminationOf(pEdict); // KWo - 29.03.2008
      iButtonPushProp = ((fOwnLightLevel < 10.0f) ? 10 : 40);
      if ((RANDOM_LONG(0,100) > iButtonPushProp) && !(pBot->bNewButtonNoticed)) // KWo - 29.03.2008
      {
         pBot->bButtonPushDecided = true;
      }
      pBot->bNewButtonNoticed = true;

      if ((pBot->bButtonPushDecided) && (pBot->fButtonPushTime < gpGlobals->time))
         fDesiredDistance = 0.0f;
      else
         fDesiredDistance = 64.0f;
   }
   else if (paths[pBot->curr_wpt_index]->flags & W_FL_CROUCH) // KWo - 15.02.2008
      fDesiredDistance = 5.0f;
   else
   {
      fDesiredDistance = paths[pBot->curr_wpt_index]->Radius;
   }
   if (!(paths[pBot->curr_wpt_index]->flags & W_FL_USE_BUTTON)) // KWo - 09.07.2006
   {
      pBot->bButtonPushDecided = false;
      pBot->bNewButtonNoticed = false;
   }

   cCollStateIndex = pBot->cCollideMoves[(int) pBot->cCollStateIndex];   // KWo - 09.01.2008
   if (!(paths[pBot->curr_wpt_index]->flags & W_FL_CROUCH) && pBot->bBotNeedsObviateObstacle
      && (pBot->cCollisionState == COLLISION_PROBING)                         // KWo - 14.02.2008
      && ((cCollStateIndex == COLLISION_STRAFELEFT)
           || (cCollStateIndex == COLLISION_STRAFERIGHT)
           || (cCollStateIndex == COLLISION_JUMP))
      && (fDesiredDistance < 64.0f))
         fDesiredDistance = 64.0f;


   // This waypoint has additional Travel Flags - care about them
   if (pBot->curr_travel_flags & C_FL_JUMP)
   {
      // Not jumped yet ?
      if (!pBot->bJumpDone)
      {
         fDesiredDistance = 0;

         // If Bot's on ground or on ladder we're free to jump.
         // Yes, I'm cheating here by setting the correct velocity for the jump. Pressing
         // the jump button gives the illusion of the Bot actual jumping
         if ((pEdict->v.flags & (FL_ONGROUND | FL_PARTIALGROUND)) || pBot->bOnLadder)
         {
            pEdict->v.velocity = pBot->vecDesiredVelocity;
            pEdict->v.button |= IN_JUMP;
            pBot->f_jumptime = gpGlobals->time + 1.0f;
            pBot->bJumpDone = TRUE;
            pBot->vecDesiredVelocity = g_vecZero;
            pBot->bCheckTerrain = FALSE;
         }
      }

      // Jump made
      else
      {
         // if bot was doing a knife-jump, switch back to normal weapon
         if (!(g_b_cv_jasonmode) && (pBot->current_weapon.iId == CS_WEAPON_KNIFE)  // KWo - 06.04.2006
             && (pEdict->v.flags & (FL_ONGROUND | FL_PARTIALGROUND)))
            BotSelectBestWeapon (pBot);
      }
   }

   // Special Ladder Handling
   if (paths[pBot->curr_wpt_index]->flags & W_FL_LADDER)
   {
      fDesiredDistance = 5.0f;
      if (pBot->pWaypointNodes != NULL)  // KWo - 22.01.2012
      {
         if (pBot->pWaypointNodes->NextNode != NULL)
         {
            if ((paths[pBot->pWaypointNodes->NextNode->iIndex]->flags & W_FL_LADDER) || pBot->bOnLadder)
               fDesiredDistance = 17.0f;
         }
      }

      if (bPreviousWP_OK)  // KWo - 14.04.2016
         if (!(paths[pBot->prev_wpt_index[0]]->flags & W_FL_LADDER)
            && (fabs(pBot->wpt_origin.z - pEdict->v.origin.z) > 5.0f))
            pBot->wpt_origin.z += pEdict->v.origin.z - pBot->wpt_origin.z;
/*
         if (!(paths[pBot->prev_wpt_index[0]]->flags & W_FL_LADDER)
            && (pBot->wpt_origin.z > (pEdict->v.origin.z + 16.0f)))
            pBot->wpt_origin = paths[pBot->curr_wpt_index]->origin - Vector (0, 0, 16);
         if (!(paths[pBot->prev_wpt_index[0]]->flags & W_FL_LADDER)
            && (pBot->wpt_origin.z < (pEdict->v.origin.z - 16.0f)))
            pBot->wpt_origin = paths[pBot->curr_wpt_index]->origin + Vector (0, 0, 16);
*/
      pBot->dest_origin = pBot->wpt_origin;

//    KWo - 10.09.2006 - special detection if someone is using the ladder (to prevent to have bots-towers on ladders)
      for (i = 0; i < gpGlobals->maxClients; i++)
      {
         if (!(clients[i].iFlags & CLIENT_USED)
             || !(clients[i].iFlags & CLIENT_ALIVE)
/*             || (clients[i].pEdict->v.movetype != MOVETYPE_FLY) */
             || (clients[i].pEdict == NULL)
             || (clients[i].pEdict == pEdict))
            continue;

         vBotOrigin = pEdict->v.origin;
         vBotOrigin.z = 0.0f;
         vPlayerOrigin = clients[i].pEdict->v.origin;
         vPlayerOrigin.z = 0.0f;
         vWptLadder = paths[pBot->curr_wpt_index]->origin;
         vWptLadder.z = 0.0f;
         bFoundGround = false;

         if ((vPlayerOrigin - vWptLadder).Length() < 40.0f) // more than likely someone is already using our ladder...
         {
            if ( ((clients[i].iTeam != pBot->bot_team) || (g_b_cv_ffa)) && (!g_bIgnoreEnemies)) // KWo - 16.12.2007
            {
               vecOrg = GetGunPosition (pEdict);
               TRACE_LINE (vecOrg, clients[i].pEdict->v.origin, dont_ignore_monsters, pEdict, &tr);
               if (tr.pHit == clients[i].pEdict) // bot found an enemy on his ladder - he should see him...
               {
                  pBot->pBotEnemy = clients[i].pEdict;
                  pBot->pLastEnemy = clients[i].pEdict;
                  pBot->vecLastEnemyOrigin = clients[i].pEdict->v.origin;
                  pBot->ucVisibility = 0;
                  pBot->ucVisibility |= HEAD_VISIBLE;
                  pBot->ucVisibility |= WAIST_VISIBLE;
                  pBot->iStates |= STATE_SEEINGENEMY;
                  pBot->f_bot_see_enemy_time = gpGlobals->time;
                  break;
               }
            }
            else
            {
//               if ((pBot->prev_wpt_index[0] >= 0) && (pBot->prev_wpt_index[0] < g_iNumWaypoints))
//                  vecOrg = paths[pBot->prev_wpt_index[0]]->origin;
//               else
               vecOrg = GetGunPosition (pEdict);
               TRACE_HULL (vecOrg, paths[pBot->curr_wpt_index]->origin, dont_ignore_monsters, (pEdict->v.flags & FL_DUCKING) ? head_hull : human_hull, pEdict, &tr);
               if ((tr.pHit == clients[i].pEdict) && (fabs(pEdict->v.origin.z - clients[i].pEdict->v.origin.z) > 15.0f)  // someone is above or below us
                 /* && (clients[i].pEdict->v.movetype == MOVETYPE_FLY) */)    // and is using the ladder already  KWo - 05.01.2008
               {
                  if ((pBot->prev_wpt_index[0] >= 0) && (pBot->prev_wpt_index[0] < g_iNumWaypoints))
                  {
                     if (!(paths[pBot->prev_wpt_index[0]]->flags & W_FL_LADDER))
                     {
                        bFoundGround = true;
                        iPreviousWpt = pBot->prev_wpt_index[0]; // KWo - 05.01.2008
                     }
                     else if ((pBot->prev_wpt_index[1] >= 0) && (pBot->prev_wpt_index[1] < g_iNumWaypoints))
                     {
                        if (!(paths[pBot->prev_wpt_index[1]]->flags & W_FL_LADDER))
                        {
                           bFoundGround = true;
                           iPreviousWpt = pBot->prev_wpt_index[1]; // KWo - 05.01.2008
                        }
                        else if ((pBot->prev_wpt_index[2] >= 0) && (pBot->prev_wpt_index[2] < g_iNumWaypoints))
                        {
                           if (!(paths[pBot->prev_wpt_index[2]]->flags & W_FL_LADDER))
                           {
                              bFoundGround = true;
                              iPreviousWpt = pBot->prev_wpt_index[2]; // KWo - 05.01.2008
                           }
                           else if ((pBot->prev_wpt_index[3] >= 0) && (pBot->prev_wpt_index[3] < g_iNumWaypoints))
                           {
                              if (!(paths[pBot->prev_wpt_index[3]]->flags & W_FL_LADDER))
                              {
                                 bFoundGround = true;
                                 iPreviousWpt = pBot->prev_wpt_index[3]; // KWo - 05.01.2008
                              }
                           }
                        }
                     }
                  }
                  if (bFoundGround) // KWo - 05.01.2008
                  {
                     if ((BotGetSafeTask(pBot)->iTask != TASK_MOVETOPOSITION) || (BotGetSafeTask(pBot)->fDesire != TASKPRI_PLANTBOMB))
                     {
                        if (g_b_DebugNavig)
                           ALERT(at_logged, "[DEBUG] BotDoWaypointNav - Bot %s forced to go away from the ladder - WPTindex = %i, out index = %i...\n",
                              pBot->name, BotGetSafeTask(pBot)->iData, iPreviousWpt);

                        pBot->curr_wpt_index = pBot->prev_wpt_index[0];
                        bottask_t TempTask = {NULL, NULL, TASK_MOVETOPOSITION, TASKPRI_PLANTBOMB, iPreviousWpt, 0.0f, TRUE};
                        BotPushTask (pBot, &TempTask);
                     }
                     break;
                  }
               }
            }
         }
      }
   }

   // Special Lift Handling
   if (paths[pBot->curr_wpt_index]->flags & W_FL_LIFT)
   {
      fDesiredDistance = 15.0f;
      pBot->f_wpt_timeset = gpGlobals->time;

      TRACE_LINE (paths[pBot->curr_wpt_index]->origin, paths[pBot->curr_wpt_index]->origin + Vector (0, 0, -50), ignore_monsters, pEdict, &tr);
      const char *szClassname = STRING(tr.pHit->v.classname); // KWo - 14.08.2007
      EntHitIndex = ENTINDEX(tr.pHit);
      bLiftExistDoorClosed = false; // KWo - 14.10.2006

      TRACE_LINE (pEdict->v.origin, paths[pBot->curr_wpt_index]->origin, ignore_monsters, pEdict, &tr2); // KWo - 14.10.2006

      if ((tr2.flFraction < 1.0f) && (FStrEq(STRING(tr2.pHit->v.classname), "func_door")) // KWo - 18.11.2006
         && ((pBot->iLiftUsageState == LIFT_NO_NEARBY) || (pBot->iLiftUsageState == LIFT_WAITING_FOR) || (pBot->iLiftUsageState == LIFT_LOOKING_BUTTON_OUTSIDE))
         && (pEdict->v.groundentity != tr2.pHit))
      {
         if (pBot->iLiftUsageState == LIFT_NO_NEARBY)
         {
            pBot->iLiftUsageState = LIFT_LOOKING_BUTTON_OUTSIDE;
            pBot->f_UsageLiftTime = gpGlobals->time + 7.0f;
         }
         bLiftExistDoorClosed = true;
      }

      // lift found at waypoint ?
      if (g_b_DebugNavig)
         ALERT (at_logged, "[DEBUG] BotDoWaypointNav - Bot %s is near WP with lift flag - WP index %i, state = %i.\n",
            pBot->name, pBot->curr_wpt_index, pBot->iLiftUsageState);

      if ((tr.flFraction < 1.0f) && (EntHitIndex > 32) && (pBot->pWaypointNodes!=NULL)
         && ((FStrEq(szClassname, "func_door")) || (FStrEq(szClassname, "func_plat"))
         || (FStrEq(szClassname, "func_train"))) && !bLiftExistDoorClosed) // KWo - 14.10.2006
      {
         if (((pBot->iLiftUsageState == LIFT_NO_NEARBY) || (pBot->iLiftUsageState == LIFT_WAITING_FOR)
            || (pBot->iLiftUsageState == LIFT_LOOKING_BUTTON_OUTSIDE)) && (tr.pHit->v.velocity.z == 0))
         {
            if (fabs(pEdict->v.origin.z - tr.vecEndPos.z) < 70.0f)
            {
               if (g_b_DebugNavig)
                  ALERT (at_logged, "[DEBUG] BotDoWaypointNav - Bot %s found a lift - ENTINDEX %i.\n", pBot->name, ENTINDEX(pBot->pLift));

               pBot->pLift = tr.pHit;
               pBot->iLiftUsageState = LIFT_GO_IN;
               pBot->v_LiftTravelPos = paths[pBot->curr_wpt_index]->origin;
               pBot->f_UsageLiftTime = gpGlobals->time + 5.0f;
            }
         }
         else if (pBot->iLiftUsageState == LIFT_TRAVELING_BY)
         {
            if (g_b_DebugNavig)
               ALERT (at_logged, "[DEBUG] BotDoWaypointNav - Bot %s reached target traveling by the lift.\n", pBot->name);

            pBot->iLiftUsageState = LIFT_LEAVING;
            pBot->f_UsageLiftTime = gpGlobals->time + 7.0f;
         }
      }
      // No lift found at waypoint
      else if (pBot->pWaypointNodes != NULL)
      {
         if (((pBot->iLiftUsageState == LIFT_NO_NEARBY)
            || (pBot->iLiftUsageState == LIFT_WAITING_FOR)) && (pBot->pWaypointNodes->NextNode != NULL))
         {
            iTempIndex = pBot->pWaypointNodes->NextNode->iIndex;
            if ((iTempIndex >= 0) && (iTempIndex < g_iNumWaypoints))
            {
               if (paths[iTempIndex]->flags & W_FL_LIFT)
               {
                  TRACE_LINE(paths[pBot->curr_wpt_index]->origin, paths[iTempIndex]->origin, ignore_monsters, pEdict, &tr);
                  const char *szClassname1 = STRING(tr.pHit->v.classname); // KWo - 14.08.2007
                  EntHitIndex = ENTINDEX(tr.pHit);
                  if (!FNullEnt(tr.pHit) && (EntHitIndex > 32) && (tr.flFraction < 1.0f)
                     && ((FStrEq(szClassname1, "func_door")) || (FStrEq(szClassname1, "func_plat"))
                     || (FStrEq(szClassname1, "func_train")))) // KWo - 14.10.2006
                  {
                     pBot->pLift = tr.pHit;
                  }
               }
            }
            pBot->iLiftUsageState = LIFT_LOOKING_BUTTON_OUTSIDE;
            pBot->f_UsageLiftTime = gpGlobals->time + 15.0f;
         }
      }

      if (pBot->iLiftUsageState == LIFT_GO_IN)
      {
         pBot->dest_origin = pBot->v_LiftTravelPos;
         if ((pEdict->v.origin - pBot->dest_origin).Length() < 15.0f)
         {
            pBot->f_move_speed = 0.0f;
            pBot->f_sidemove_speed = 0.0f;
            bNeedWait = false;
            // if some bot is following a bot going into lift - he should take the same lift to go
            for (i = 0; i < gpGlobals->maxClients; i++)
            {
               if (bots[i].bDead || (bots[i].bot_team != pBot->bot_team) || !(bots[i].is_used)
                  || (bots[i].pEdict == pEdict)
                  || (bots[i].pBotUser != pEdict) || (bots[i].pTasks->iTask != TASK_FOLLOWUSER)
//                  && ((bots[i].pLift != pBot->pLift)
                  || ((bots[i].pEdict->v.groundentity == pBot->pLift)
                     && (bots[i].pEdict->v.flags & FL_ONGROUND)))
               {
                  continue;
               }
               else  // KWo - 11.04.2010
               {
                  bots[i].pLift = pBot->pLift;
                  bots[i].iLiftUsageState = LIFT_GO_IN;
                  bots[i].v_LiftTravelPos = pBot->v_LiftTravelPos;
                  bNeedWait = true;
               }
            }
            if (bNeedWait)
            {
               pBot->iLiftUsageState = LIFT_WAITING_FOR_TEAMNATES;
               pBot->f_UsageLiftTime = gpGlobals->time + 8.0f;
            }
            else
            {
               pBot->iLiftUsageState = LIFT_LOOKING_BUTTON_INSIDE;
               pBot->f_UsageLiftTime = gpGlobals->time + 10.0f;
            }
         }
      }

      if (pBot->iLiftUsageState == LIFT_WAITING_FOR_TEAMNATES)
      {
         bNeedWait = false;
         for (i = 0; i < gpGlobals->maxClients; i++)
         {
            if (bots[i].bDead || (bots[i].bot_team != pBot->bot_team) || !(bots[i].is_used)
               || (bots[i].pBotUser != pEdict) || (bots[i].pTasks->iTask != TASK_FOLLOWUSER)
               || (bots[i].pLift != pBot->pLift))
               continue;
            if ((bots[i].pEdict->v.groundentity != pBot->pLift) || !(bots[i].pEdict->v.flags & FL_ONGROUND))
            {
               bNeedWait = true;
               break;
            }
         }

         if (bNeedWait)
         {
            if (g_b_DebugNavig)
               ALERT (at_logged, "[DEBUG] BotDoWaypointNav - Bot %s is waiting for someone in the lift.\n", pBot->name);

            pBot->dest_origin = pBot->v_LiftTravelPos;
            if ((pEdict->v.origin - pBot->dest_origin).Length() < 15.0f)
            {
               pBot->f_move_speed = 0.0f;
               pBot->f_sidemove_speed = 0.0f;
            }
         }

         if ((!bNeedWait) || (pBot->f_UsageLiftTime < gpGlobals->time))
         {
            pBot->iLiftUsageState = LIFT_LOOKING_BUTTON_INSIDE;
            pBot->f_UsageLiftTime = gpGlobals->time + 10.0f;
         }
      }

      if (pBot->iLiftUsageState == LIFT_LOOKING_BUTTON_INSIDE)
      {
         if (g_b_DebugNavig)
            ALERT (at_logged, "[DEBUG] BotDoWaypointNav - Bot %s is trying to find a button triggering the lift (inside).\n", pBot->name);

         cNearestButtonIndex = -1;
         f_button_min_distance = 250.0f;
         for (i = 0; i < g_iNumButtons; i++)
         {
            v_button_origin = ButtonsData[i].origin;
            f_button_distance = (v_button_origin - pEdict->v.origin).Length ();
            if (f_button_distance < f_button_min_distance)
            {
               if ((fabs(pEdict->v.origin.z - v_button_origin.z) < 30.0f)
                  && (FStrEq(STRING(pBot->pLift->v.targetname), ButtonsData[i].target)))
               {
                  cNearestButtonIndex = i;
                  f_button_min_distance = f_button_distance;
               }
            }
         }
                  // not found targeting one?
         if (cNearestButtonIndex == -1)
         {
            f_button_min_distance = 150.0f;
            for (i = 0; i < g_iNumButtons; i++)
            {
               v_button_origin = ButtonsData[i].origin;
               f_button_distance = (v_button_origin - pEdict->v.origin).Length ();
               if (f_button_distance < f_button_min_distance)
               {
                  if (fabs(pEdict->v.origin.z - v_button_origin.z) < 30.0f)
                  {
                     cNearestButtonIndex = i;
                     f_button_min_distance = f_button_distance;
                  }
               }
            }
         }

         // found one ?
         if (cNearestButtonIndex > -1)
         {
            v_button_origin = ButtonsData[cNearestButtonIndex].origin;
            fDesiredDistance = 0.0f;
            v_src = v_button_origin;
            fDistance = 70.0f;
            flDot = GetShootingConeDeviation (pEdict, &v_src);

            TRACE_LINE(pEdict->v.origin, v_button_origin, ignore_monsters, pEdict, &tr);
            EntHitIndex = ENTINDEX(tr.pHit);
            if ((tr.flFraction > 0.95f) || (EntHitIndex == ButtonsData[cNearestButtonIndex].EntIndex))
               fDistance = 30.0f;
            if (pBot->fButtonPushTime < gpGlobals->time) // KWo - 09.02.2008
               pBot->iAimFlags = AIM_ENTITY; // look at button and only at it (so bot can trigger it)
            if (pBot->fButtonPushTime < gpGlobals->time)
               pBot->dest_origin = v_button_origin;
            pBot->vecEntity = v_button_origin;
            pBot->bCheckTerrain = FALSE;
            pBot->bCanChooseAimDirection = FALSE;

            if (g_b_DebugNavig)
               ALERT (at_logged, "[DEBUG] BotDoWaypointNav - Bot %s found a lift button inside the lift and is trying to reach it.\n", pBot->name);

            if (((v_button_origin - pEdict->v.origin).Length () < fDistance) && (pEdict->v.groundentity == pBot->pLift)
               && (pBot->fButtonPushTime + 1.0f < gpGlobals->time)
               && (pBot->pLift->v.velocity.z == 0) && (pEdict->v.flags & FL_ONGROUND) && (flDot >=0.90f))
            {
               pEdict->v.button |= IN_USE;
               pBot->fButtonPushTime = gpGlobals->time + 3.0f;
               pBot->iAimFlags &= ~AIM_ENTITY;
               pBot->dest_origin = pBot->v_LiftTravelPos;

               if (g_b_DebugNavig || g_b_DebugTasks)
                  ALERT (at_logged, "[DEBUG] BotDoWaypointNav - Bot %s is pushing a lift button from inside and waits.\n", pBot->name);

               bottask_t TempTask = {NULL, NULL, TASK_PAUSE, TASKPRI_PAUSE, -1, gpGlobals->time + 0.5f, FALSE};
               BotPushTask (pBot, &TempTask);
            }

            else if ((g_b_DebugNavig) && (pBot->fButtonPushTime + 1.0f < gpGlobals->time))
            {
               ALERT (at_logged, "[DEBUG] BotDoWaypointNav - Bot %s can't reach the lift button from inside - distance %f.\n", pBot->name, (v_button_origin - pEdict->v.origin).Length ());
            }

         }
      }
         // is lift activated AND bot is standing on it AND lift is moving ?
      if ((pBot->iLiftUsageState == LIFT_LOOKING_BUTTON_INSIDE) || (pBot->iLiftUsageState == LIFT_GO_IN)
          || (pBot->iLiftUsageState == LIFT_WAITING_FOR_TEAMNATES) || (pBot->iLiftUsageState == LIFT_WAITING_FOR))
      {
         if ((pEdict->v.groundentity == pBot->pLift)
             && (pBot->pLift->v.velocity.z != 0) && (pEdict->v.flags & FL_ONGROUND)
             && ((paths[pBot->prev_wpt_index[0]]->flags & W_FL_LIFT) || !FNullEnt(pBot->pBotUser)))
         {
            pBot->iLiftUsageState = LIFT_TRAVELING_BY;
            pBot->f_UsageLiftTime = gpGlobals->time + 14.0f;

            if (g_b_DebugNavig)
               ALERT (at_logged, "[DEBUG] BotDoWaypointNav - Bot %s is traveling with a lift.\n", pBot->name);

            if ((pEdict->v.origin - pBot->dest_origin).Length() < 15.0f)
            {
               pBot->f_move_speed = 0.0f;
               pBot->f_sidemove_speed = 0.0f;
            }
         }
      }

      if (pBot->iLiftUsageState == LIFT_TRAVELING_BY)
      {
         pBot->dest_origin.x = pBot->v_LiftTravelPos.x;
         pBot->dest_origin.y = pBot->v_LiftTravelPos.y;
         pBot->dest_origin.z = pEdict->v.origin.z;

         if ((pEdict->v.origin - pBot->dest_origin).Length() < 15.0f)
         {
            pBot->f_move_speed = 0.0f;
            pBot->f_sidemove_speed = 0.0f;
         }
      }

      // need to find a button outside the lift
      if (pBot->iLiftUsageState == LIFT_LOOKING_BUTTON_OUTSIDE)
      {
         bLiftUsed = false;

         // button has been pressed, lift should come
         if (pBot->fButtonPushTime + 8.0f >= gpGlobals->time)
         {
            if ((pBot->prev_wpt_index[0] >=0) && (pBot->prev_wpt_index[0] < g_iNumWaypoints))
               pBot->dest_origin = paths[pBot->prev_wpt_index[0]]->origin;
            else
               pBot->dest_origin = pEdict->v.origin;

            if ((pEdict->v.origin - pBot->dest_origin).Length() < 15.0f)
            {
               pBot->f_move_speed = 0.0f;
               pBot->f_sidemove_speed = 0.0f;
            }
         }
         else
         {
            cNearestButtonIndex = -1;
            f_button_min_distance = 150.0f;
            for (i = 0; i < g_iNumButtons; i++)
            {
               v_button_origin = ButtonsData[i].origin;
               f_button_distance = (v_button_origin - pEdict->v.origin).Length ();
               if ((f_button_distance < f_button_min_distance)
                   && (fabs(pEdict->v.origin.z - v_button_origin.z) < 30.0f))
               {
                  TRACE_LINE (pEdict->v.origin, v_button_origin, ignore_monsters, pEdict, &tr);
                  EntHitIndex = ENTINDEX(tr.pHit);  // KWo - 10.02.2006
                  if ((EntHitIndex == ButtonsData[i].EntIndex) || (tr.flFraction > 0.95))
                  {
                     if (!FNullEnt(pBot->pLift) && !bLiftExistDoorClosed) // KWo - 18.11.2006
                     {
                       if (FStrEq(STRING(pBot->pLift->v.targetname), ButtonsData[i].target))
                       {
                          cNearestButtonIndex = i;
                          f_button_min_distance = f_button_distance;
                       }
                     }
                     else
                     {
                        cNearestButtonIndex = i;
                        f_button_min_distance = f_button_distance;
                     }
                  }
               }
            }
            // not found
            if (cNearestButtonIndex == -1)
            {
               f_button_min_distance = 150.0f;
               for (i = 0; i < g_iNumButtons; i++)
               {
                  v_button_origin = ButtonsData[i].origin;
                  f_button_distance = (v_button_origin - pEdict->v.origin).Length ();
                  if ((f_button_distance < f_button_min_distance)
                      && (fabs(pEdict->v.origin.z - v_button_origin.z) < 30.0f))
                  {
                     TRACE_LINE (pEdict->v.origin, v_button_origin, ignore_monsters, pEdict, &tr);
                     EntHitIndex = ENTINDEX(tr.pHit);  // KWo - 10.02.2006
                     if ((EntHitIndex == ButtonsData[i].EntIndex) || (tr.flFraction > 0.95f))
                     {
                        cNearestButtonIndex = i;
                        f_button_min_distance = f_button_distance;
                     }
                  }
               }
            }

            // found one ?
            if (cNearestButtonIndex > -1)
            {
               if (g_b_DebugNavig)
                  ALERT (at_logged, "[DEBUG] BotDoWaypointNav - Bot %s found a lift button (outside).\n", pBot->name);

               for (i = 0; i < gpGlobals->maxClients; i++)
               {
                  if (!(clients[i].iFlags & CLIENT_USED)
                     || !(clients[i].iFlags & CLIENT_ALIVE)
                     || (clients[i].iTeam != pBot->bot_team)
                     || (clients[i].pEdict == pEdict)
                     || FNullEnt(clients[i].pEdict->v.groundentity)
                     || bLiftExistDoorClosed) // KWo - 18.11.2006
                     continue;
                  if (FStrEq(STRING(clients[i].pEdict->v.groundentity->v.targetname), ButtonsData[cNearestButtonIndex].target))
                  {
                     if (clients[i].pEdict->v.groundentity == pBot->pLift)
                     {
                        if (g_b_DebugNavig)
                           ALERT (at_logged, "[DEBUG] BotDoWaypointNav - Bot %s found somebody using already the lift.\n", pBot->name);

                        bLiftUsed = true;
                        break;
                     }
                  }
               }
               if (bLiftUsed)
               {
                  if ((pBot->prev_wpt_index[0] >=0) && (pBot->prev_wpt_index[0] < g_iNumWaypoints))
                     pBot->dest_origin = paths[pBot->prev_wpt_index[0]]->origin;
                  else
                     pBot->dest_origin = ButtonsData[cNearestButtonIndex].origin;

                  if ((pEdict->v.origin - pBot->dest_origin).Length() < 15.0f)
                  {
                     pBot->f_move_speed = 0.0f;
                     pBot->f_sidemove_speed = 0.0f;
                  }
               }
               else
               {
                  v_button_origin = ButtonsData[cNearestButtonIndex].origin;
                  fDesiredDistance = 0.0f;
                  if (pBot->fButtonPushTime < gpGlobals->time) // KWo - 09.02.2008
                     pBot->iAimFlags = AIM_ENTITY; // look at button and only at it (so bot can trigger it)
                  pBot->dest_origin = v_button_origin;
                  v_src = v_button_origin;
                  flDot = GetShootingConeDeviation (pEdict, &v_src);
                  pBot->vecEntity = v_button_origin;
                  pBot->bCheckTerrain = FALSE;
                  pBot->bCanChooseAimDirection = FALSE;
                  if (((v_button_origin - pEdict->v.origin).Length () < 60.0f) && (flDot >= 0.90f))
                  {
                     pEdict->v.button |= IN_USE;
                     pBot->fButtonPushTime = gpGlobals->time + 2.0f;
                     pBot->iAimFlags &= ~AIM_ENTITY;
                     bottask_t TempTask = {NULL, NULL, TASK_PAUSE, TASKPRI_PAUSE, -1, gpGlobals->time + 0.5f, FALSE};

                     if (g_b_DebugNavig || g_b_DebugTasks)
                        ALERT (at_logged, "[DEBUG] BotDoWaypointNav - Bot %s is pushing a lift button (outside).\n", pBot->name);

                     BotPushTask (pBot, &TempTask);

                     pBot->iLiftUsageState = LIFT_WAITING_FOR;
                     pBot->f_UsageLiftTime = gpGlobals->time + 20.0f;
                  }
               }
            }
            else
            {
               pBot->iLiftUsageState = LIFT_WAITING_FOR;
               pBot->f_UsageLiftTime = gpGlobals->time + 15.0f;
            }
         }
      }

      if (pBot->iLiftUsageState == LIFT_WAITING_FOR)
      {
         if (g_b_DebugNavig)
            ALERT (at_logged, "[DEBUG] BotDoWaypointNav - Bot %s is waiting for a lift.\n", pBot->name);

         if ((pBot->prev_wpt_index[0] >= 0) && (pBot->prev_wpt_index[0] < g_iNumWaypoints))
         {
            if (!(paths[pBot->prev_wpt_index[0]]->flags & W_FL_LIFT))
               pBot->dest_origin = paths[pBot->prev_wpt_index[0]]->origin;
            else if ((pBot->prev_wpt_index[1] >= 0) && (pBot->prev_wpt_index[0] < g_iNumWaypoints))
               pBot->dest_origin = paths[pBot->prev_wpt_index[1]]->origin;
         }
         if ((pEdict->v.origin - pBot->dest_origin).Length() < 10.0f)
         {
            pBot->f_move_speed = 0.0f;
            pBot->f_sidemove_speed = 0.0f;
         }
      }
      if ((pBot->iLiftUsageState == LIFT_WAITING_FOR) || (pBot->iLiftUsageState == LIFT_GO_IN))
      {
         // Bot fall down somewhere inside the lift's groove :)
         if ((pEdict->v.groundentity != pBot->pLift) && (pBot->prev_wpt_index[0] >= 0)
            && (pBot->prev_wpt_index[0] < g_iNumWaypoints))
         {
            if ((paths[pBot->prev_wpt_index[0]]->flags & W_FL_LIFT)
                && ((paths[pBot->curr_wpt_index]->origin.z - pEdict->v.origin.z) > 50.0f)
                && ((paths[pBot->prev_wpt_index[0]]->origin.z - pEdict->v.origin.z) > 50.0f))
            {
               DeleteSearchNodes (pBot);
               pBot->iLiftUsageState = LIFT_NO_NEARBY;
               pBot->pLift = NULL;
               pBot->f_UsageLiftTime = 0.0f;
               BotFindWaypoint (pBot);
               bPathValid = false;
               if ((pBot->prev_wpt_index[2] >= 0) && (pBot->prev_wpt_index[2] < g_iNumWaypoints))
               {
                  iTempIndex = pBot->prev_wpt_index[2];
                  pBot->pWayNodesStart = FindShortestPath (pBot->curr_wpt_index, iTempIndex, &bPathValid);
               }
               return (FALSE);
            }
         }
      }

   } // W_FL_LIFT END , but lift code is continued...

   if ((!FNullEnt(pBot->pLift)) && !(paths[pBot->curr_wpt_index]->flags & W_FL_LIFT))
   {
      if (pBot->iLiftUsageState == LIFT_TRAVELING_BY)
      {
         if (g_b_DebugNavig)
            ALERT (at_logged, "[DEBUG] BotDoWaypointNav - Bot %s is leaving the lift.\n", pBot->name);

         pBot->iLiftUsageState = LIFT_LEAVING;
         pBot->f_UsageLiftTime = gpGlobals->time + 10.0f;
      }
      if ((pBot->iLiftUsageState == LIFT_LEAVING) && (pBot->f_UsageLiftTime < gpGlobals->time)
         && (pEdict->v.groundentity != pBot->pLift))
      {
         if (g_b_DebugNavig)
            ALERT (at_logged, "[DEBUG] BotDoWaypointNav - Bot %s finished the travel by a lift.\n", pBot->name);

         pBot->iLiftUsageState = LIFT_NO_NEARBY;
         pBot->f_UsageLiftTime = 0.0f;
         pBot->pLift = NULL;
      }
   }

   if ((pBot->f_UsageLiftTime < gpGlobals->time) && (pBot->f_UsageLiftTime != 0.0f))
   {
      if (g_b_DebugNavig)
         ALERT (at_logged, "[DEBUG] BotDoWaypointNav - Bot %s is no longer waiting for a lift - too long time; Lift state %i.\n", pBot->name, pBot->iLiftUsageState);

      pBot->pLift = NULL;
      pBot->iLiftUsageState = LIFT_NO_NEARBY;
      pBot->f_UsageLiftTime = 0.0f;
      DeleteSearchNodes (pBot);

      if ((pBot->prev_wpt_index[0] >= 0) && (pBot->prev_wpt_index[0] < g_iNumWaypoints))
      {
         if (!(paths[pBot->prev_wpt_index[0]]->flags & W_FL_LIFT))
            BotChangeWptIndex (pBot, pBot->prev_wpt_index[0]);
         else
            BotFindWaypoint (pBot);
      }
      else
         BotFindWaypoint (pBot);
      return (FALSE);
   }
// End of lift code


   // Special Button Handling - KWo - a lot of code here has been rewritten
   if ((paths[pBot->curr_wpt_index]->flags & W_FL_USE_BUTTON) && (g_iNumButtons > 0) && (pBot->fButtonPushTime < gpGlobals->time) && (pBot->bButtonPushDecided))  // KWo - 09.07.2006
   {
      cNearestButtonIndex = -1;   // KWo - 10.02.2006
      f_button_min_distance = 100.0f;
      v_button_origin = g_vecZero;
      f_button_distance = 100.0f;
      EntHitIndex = 0;   // KWo - 10.02.2006
//      signed char i;   // KWo - 10.02.2006
      // find the closest reachable button
      for (i = 0; i < g_iNumButtons; i++)  // KWo - 10.02.2006
      {
         v_button_origin = ButtonsData[i].origin;
         f_button_distance = (v_button_origin - pBot->wpt_origin).Length ();
         if (f_button_distance < f_button_min_distance)
         {
            TRACE_LINE (pBot->wpt_origin, v_button_origin, ignore_monsters, pEdict, &tr);
            EntHitIndex = ENTINDEX( tr.pHit );  // KWo - 10.02.2006
            if ((EntHitIndex == ButtonsData[i].EntIndex) || (tr.flFraction > 0.95))
            {
                cNearestButtonIndex = i;
                f_button_min_distance = f_button_distance;
            }
         }
      }
      // found one ?
      if (cNearestButtonIndex > -1)  // KWo - 10.02.2006
      {
         v_button_origin = ButtonsData[cNearestButtonIndex].origin;
//         fDesiredDistance = 0.0f;
         if (pBot->fButtonPushTime < gpGlobals->time) // KWo - 09.02.2008
            pBot->iAimFlags = AIM_ENTITY; // look at button and only at it (so bot can trigger it)
         pBot->dest_origin = v_button_origin;
         pBot->vecEntity = v_button_origin;
         pBot->f_wpt_timeset = gpGlobals->time;
         pBot->bCheckTerrain = TRUE;
         pBot->bCanChooseAimDirection = FALSE;
//         pBot->vecLookAt = v_button_origin;
         v_src = v_button_origin;  // KWo - 08.02.2006
         // reached it?

         flDot = GetShootingConeDeviation (pEdict, &v_src);

         TRACE_LINE (pBot->wpt_origin, v_button_origin, ignore_monsters, pEdict, &tr2);
         EntHitIndex = ENTINDEX( tr2.pHit );  // KWo - 10.02.2006

         if (g_b_DebugNavig)
            ALERT (at_logged, "[DEBUG] BotDoWaypointNav - Bot %s found a button EntIndex %i .\n", pBot->name, EntHitIndex);

         if ((( (tr2.flFraction > 0.95) && (v_button_origin - pEdict->v.origin).Length () < 50.0f)
            || ((EntHitIndex == ButtonsData[cNearestButtonIndex].EntIndex) && ((tr2.vecEndPos - GetGunPosition (pEdict)).Length() < 50.0f))))
          {
            if (flDot > 0.92)
            {
               if (g_b_DebugNavig || g_b_DebugTasks)
                  ALERT (at_logged, "[DEBUG] Bot %s is using a button, task = %i, WP index = %i.\n",
                     pBot->name, BotGetSafeTask(pBot)->iTask, pBot->curr_wpt_index);

               pEdict->v.button |= IN_USE;
               fDesiredDistance = 64.0f; // KWo - 08.07.2006
               pBot->fButtonPushTime = gpGlobals->time + 2.0f;
               pBot->iAimFlags &= ~AIM_ENTITY;
            }
         }
      }
   }


   if ((pBot->prev_wpt_index[0] >= 0) && (pBot->prev_wpt_index[0] < g_iNumWaypoints))
      v_trace_start = paths[pBot->prev_wpt_index[0]]->origin;
   else
      v_trace_start = pEdict->v.origin;

   v_trace_end = paths[pBot->curr_wpt_index]->origin; // KWo - 15.04.2013

   if (g_i_botthink_index == g_iFrameCounter ) // KWo - 23.03.2007
   {
      if ((pBot->bOnLadder) || (pBot->curr_travel_flags && W_FL_LADDER)) // 16.12.2007
         TRACE_LINE (v_trace_start, v_trace_end, ignore_monsters, pEdict, &tr);
      else
         TRACE_HULL (v_trace_start, v_trace_end, ignore_monsters,
           ((pEdict->v.flags & IN_DUCK) || paths[pBot->curr_wpt_index]->flags & W_FL_CROUCH) ? point_hull : head_hull, pEdict, &tr); // KWo - 17.04.2013
      pBot->pHit = tr.pHit; // KWo - 23.03.2007
   }
   tr.pHit = pBot->pHit; // KWo - 23.03.2007

breakabledoorcheck:
   ++iAmountLooped;

   // REMINDER: TRACE_HULL crashes without this... but not TRACE_LINE
   if (!FNullEnt (tr.pHit))
   {
      // Special Door Button Handling
      if ((strncmp (STRING (tr.pHit->v.classname), "func_door", 9) == 0)  && (pBot->iLiftUsageState == LIFT_NO_NEARBY)) // KWo - 23.04.2006
      {
         fDesiredDistance = 0;
         pBot->iAimFlags = AIM_DEST; // look at door and only at it (so it opens in the right direction)
         pBot->bCanChooseAimDirection = FALSE;
         bDoorHasTargetButton = false; // KWo - 02.02.2006
         // does this door has a target button ?
         const char* vtargetname = STRING(tr.pHit->v.targetname);
         if (!FStrEq(vtargetname, ""))	// KWo - to remove warning cast to a pointer
         {
            cNearestButtonIndex = -1;   // KWo - 10.02.2006
            v_button_origin = g_vecZero;
            f_button_min_distance = 300.0f;   // KWo - 10.02.2006
            f_button_distance = 300.0f;   // KWo - 10.02.2006

            f_tr2FlFraction = 0.0f; // KWo - 29.03.2008
            f_tr3FlFraction = 0.0f; // KWo - 29.03.2008
            // find a reachable target button
            for (i = 0; i < g_iNumButtons; i++)  // KWo - 10.02.2006
            {
               const char* vtarget2 = ButtonsData[i].target;
               iStrLen = strlen(vtarget2);  // KWo - 05.10.2008
               if ((iStrLen > 2) && (strncmp (vtarget2, vtargetname, iStrLen) == 0)) // KWo - 05.10.2008
//               if (FStrEq(vtarget2, vtargetname))
               {
                  bDoorHasTargetButton = true;
                  if (pBot->fButtonPushTime < gpGlobals->time)
                  {
//                     cNearestButtonIndex = -1; // KWo - 16.07.2006
                     pBot->fButtonNoticedTime = gpGlobals->time; // KWo - 16.07.2006
                     v_button_origin = ButtonsData[i].origin;
                     v_tr23_start = pEdict->v.origin;
                     TRACE_LINE (v_tr23_start, v_button_origin, ignore_monsters, pEdict, &tr2);
                     if ((pBot->prev_wpt_index[0] >= 0) && (pBot->prev_wpt_index[0] < g_iNumWaypoints))
                     {
                        v_tr23_start = paths[pBot->prev_wpt_index[0]]->origin;
                     }
                     else
                     {
                        v_tr23_start = pEdict->v.origin;
                     }
                     TRACE_LINE (v_tr23_start, v_button_origin, ignore_monsters, pEdict, &tr3);
                     if ((ENTINDEX(tr2.pHit) == ButtonsData[i].EntIndex)
                        || (ENTINDEX(tr3.pHit) == ButtonsData[i].EntIndex))  // KWo - 29.03.2008
                     {
                        cNearestButtonIndex = i;
                        f_tr2FlFraction = tr2.flFraction;
                        f_tr3FlFraction = tr3.flFraction;
                        break;
                     }
                     else if ((tr2.flFraction > 0.95f) || (tr3.flFraction > 0.95f))
                     {
                        f_button_distance = (v_button_origin - v_trace_start).Length ();  // distance between button and the WP tracehull is started
                        if (f_button_distance < f_button_min_distance)
                        {
                           cNearestButtonIndex = i;
                           f_button_min_distance = f_button_distance;
                           f_tr2FlFraction = tr2.flFraction;
                           f_tr3FlFraction = tr3.flFraction;
                        }
                     }
                  }
               }
            }

            if ((cNearestButtonIndex == -1) && (pBot->f_timeHitDoor < gpGlobals->time) && (iAmountLooped == 1)
                && (pBot->fButtonNoticedTime + 3.0f < gpGlobals->time) && (pBot->fButtonPushTime < gpGlobals->time)
                && (pBot->cCollideMoves[(int) pBot->cCollStateIndex] != COLLISION_GOBACK)) // KWo - 14.08.2006
            {
               pBot->dest_origin = tr.vecEndPos;
            }

                  // found one ?
            if ((cNearestButtonIndex > -1) && (pBot->fButtonPushTime < gpGlobals->time)) // KWo - 10.02.2006
            {
               if (g_b_DebugNavig)
                  ALERT (at_logged, "[DEBUG] BotDoWaypointNav - Bot %s found a button.\n", pBot->name);

               v_button_origin = ButtonsData[cNearestButtonIndex].origin;
               if (FStrEq ("func_button", ButtonsData[cNearestButtonIndex].classname)) // KWo - 29.03.2008
               {
                  if ((f_tr2FlFraction > 0.99f) || (f_tr3FlFraction > 0.99f))
                     f_button_min_distance = 40.0f;
                  else
                     f_button_min_distance = 60.0f;
               }
               else
                  f_button_min_distance = 20.0f;

               if (pBot->fButtonPushTime < gpGlobals->time) // KWo - 09.02.2008
                  pBot->iAimFlags = AIM_ENTITY; // look at button and only at it (so bot can trigger it)

               pBot->bCanChooseAimDirection = FALSE;
               pBot->dest_origin = v_button_origin;
               pBot->f_wpt_timeset = gpGlobals->time;
               pBot->bCheckTerrain = TRUE; // KWo - 29.03.2008
               pBot->vecEntity = v_button_origin;
               v_src = v_button_origin;
               flDot = GetShootingConeDeviation (pEdict, &v_src);

               if (g_b_DebugNavig)
                  ALERT (at_logged, "[DEBUG] BotDoWaypointNav - Bot %s found a button targetting the door and faces it with flDot = %f .\n",
                     pBot->name, flDot);

               if (((GetGunPosition (pEdict) - v_button_origin).Length() < f_button_min_distance)
                    &&  (flDot >= 0.92f)) // Is Bot facing the Button ?
               {
                  pBot->fButtonPushTime = gpGlobals->time + 1.5f;
                  pEdict->v.button |= IN_USE;
                  if ((pBot->prev_wpt_index[0] >= 0) && (pBot->prev_wpt_index[0] < g_iNumWaypoints))
                  {
                     pBot->dest_origin = paths[pBot->prev_wpt_index[0]]->origin;
                  }
                  else
                  {
                     pBot->dest_origin = v_button_origin;
                  }
//                  pBot->f_timeDoorOpen = gpGlobals->time + pevDoor->nextthink - pevDoor->ltime;
                  pBot->f_timeDoorOpen = gpGlobals->time + 2.0f;
//                  pBot->iAimFlags &= ~AIM_ENTITY;
               }
            }

            fCone = 2;  // KWo - 10.02.2006
            for (i = 0; i < g_iNumBreakables; i++)  // KWo - 10.02.2006
            {
               v_button_origin = BreakablesData[i].origin;
               if ((pEdict->v.origin - BreakablesData[i].origin).Length() < 100.0f)
               {
                  if ((pBot->prev_wpt_index[0] >= 0) && (pBot->prev_wpt_index[0] < g_iNumWaypoints))
                  {
                     fCone = UTIL_GetVectorsCone (pBot->wpt_origin, paths[pBot->prev_wpt_index[0]]->origin,
                               v_button_origin, paths[pBot->prev_wpt_index[0]]->origin);  // KWo - 13.02.2006
                  }
                  else
                  {
                     fCone = 0.05f;
                  }
                  if (fCone < 0.6f)
                  {
                     pBot->iAimFlags |= AIM_ENTITY;
                     pBot->vecEntity = v_button_origin;
                     pBot->f_wpt_timeset = gpGlobals->time;
                     pBot->bCheckTerrain = FALSE;
                     BotSelectBestWeapon (pBot);

                     if (!(pEdict->v.oldbuttons & IN_ATTACK) && (GetShootingConeDeviation (pEdict, &v_button_origin) > 0.98))
                     {
                        pEdict->v.button |= IN_ATTACK;
                        pBot->iAimFlags &= ~AIM_ENTITY;
                     }
                  }
               }
            }
         }


         // if pushed button recently and door still not open wait for door to open
         if (!(FStrEq(vtargetname, "")) && (pBot->f_timeDoorOpen > gpGlobals->time) && bDoorHasTargetButton)  // KWo - 10.02.2006
         {
            if ((pBot->prev_wpt_index[0] >= 0) && (pBot->prev_wpt_index[0] < g_iNumWaypoints))
            {
               if (g_b_DebugNavig)
                  ALERT (at_logged, "[DEBUG] BotDoWaypointNav - Bot %s is trying to reach the previous WP while waiting for the door open.\n", pBot->name);

               pBot->dest_origin = paths[pBot->prev_wpt_index[0]]->origin;
               if ((pEdict->v.origin - pBot->dest_origin).Length() < 10.0f)
               {
                  if (g_b_DebugNavig || g_b_DebugTasks)
                     ALERT (at_logged, "[DEBUG] BotDoWaypointNav - Bot %s has reached the previous WP while waiting for the door open (pause 1).\n", pBot->name);

                   bottask_t TempTask = {NULL, NULL, TASK_PAUSE, TASKPRI_PAUSE, -1, gpGlobals->time + 1.0f, FALSE};  // KWo 08.02.2006
                   BotPushTask (pBot, &TempTask);
               }
            }
            else
            {
               if (g_b_DebugNavig || g_b_DebugTasks)
                  ALERT (at_logged, "[DEBUG] BotDoWaypointNav - Bot %s haven't had the previous WP to wait for the door open (pause 2).\n", pBot->name);

               bottask_t TempTask = {NULL, NULL, TASK_PAUSE, TASKPRI_PAUSE, -1, gpGlobals->time + 1.5f, FALSE};  // KWo 06.01.2006
               BotPushTask (pBot, &TempTask);
			   }
         }

         // if door has no button but needs to be "USE"ed to be opened
         else if ((tr.pHit->v.spawnflags & (1 << 8))
                 && (((tr.pHit->v.origin - pEdict->v.origin).Length() < 80.0f)
                 || ((tr.vecEndPos - GetGunPosition (pEdict)).Length() < 80.0f)))
         {
            if (g_b_DebugNavig || g_b_DebugTasks)
               ALERT (at_logged, "[DEBUG] BotDoWaypointNav - Bot %s is USE'ing the door to open it (pause 3).\n", pBot->name);

            pEdict->v.button |= IN_USE;
            bottask_t TempTask = {NULL, NULL, TASK_PAUSE, TASKPRI_PAUSE, -1, gpGlobals->time + 1.0f, FALSE};
            BotPushTask (pBot, &TempTask);
         }

         // if bot hits the door, then it opens, so wait a bit to let it open safely (normal door without target)
         if ((pEdict->v.velocity.Length2D () < 2) && (pBot->f_timeDoorOpen < gpGlobals->time)
             && (pBot->fButtonPushTime < gpGlobals->time) && (!bDoorHasTargetButton)
             && ((!pBot->bHitDoor) || (pBot->f_timeHitDoor + 3.0f > gpGlobals->time))) // KWo - 17.01.2010
         {
            if (g_b_DebugNavig || g_b_DebugTasks)
               ALERT (at_logged, "[DEBUG] BotDoWaypointNav - Bot %s is waiting until the door is open (pause 4).\n", pBot->name);

            bottask_t TempTask = {NULL, NULL, TASK_PAUSE, TASKPRI_PAUSE, -1, gpGlobals->time + 0.5f, FALSE};
            BotPushTask (pBot, &TempTask);
            pBot->f_timeDoorOpen = gpGlobals->time + 1.0f; // retry in 1 sec until door is open
            if (!pBot->bHitDoor) // KWo - 17.01.2010
            {
               pBot->f_timeHitDoor = gpGlobals->time;
               pBot->bHitDoor = true;
            }
         }
      }

      // Special Breakable Handling
      else if ( IsShootableBreakable (tr.pHit)
              || (((iAmountLooped == 1) || (iAmountLooped == 4))
              && (FStrEq(STRING(tr.pHit->v.classname),"func_breakable")) && (tr.pHit != pBot->pBreakableIgnore)
              && (tr.pHit->v.takedamage > 0) && ((tr.pHit->v.impulse == 0) || (iAmountLooped == 1))
              && (tr.pHit->v.health < 1000))) // KWo - 12.03.2010
      {
         fCone = 2;
         if ((pBot->prev_wpt_index[0] >= 0) && (pBot->prev_wpt_index[0] < g_iNumWaypoints))
         {
            fCone = UTIL_GetVectorsCone (pBot->wpt_origin, paths[pBot->prev_wpt_index[0]]->origin,
                     VecBModelOrigin (tr.pHit), paths[pBot->prev_wpt_index[0]]->origin);  // KWo - 13.02.2006
         }
         else
         {
            fCone = 0.1;
         }
         if (fCone < 0.6f) // KWo - 30.01.2006
         {
            if (g_b_DebugNavig || g_b_DebugTasks || g_b_DebugEntities)
               ALERT(at_logged, "[DEBUG] BotDoWaypointNav - Bot %s found a breakable (1) - faces with fCone = %f; LoopCheck = %i.\n",
                  pBot->name, fCone, iAmountLooped);

            pBot->pShootBreakable = tr.pHit;
            pBot->iCampButtons = pEdict->v.button & IN_DUCK;
            bottask_t TempTask = {NULL, NULL, TASK_SHOOTBREAKABLE, TASKPRI_SHOOTBREAKABLE, -1, 0.0f, FALSE};
            BotPushTask (pBot, &TempTask);
         }
         else
         {
            if ((pBot->prev_wpt_index[0] >= 0) && (pBot->prev_wpt_index[0] < g_iNumWaypoints))  // KWo - 31.01.2006
            {
               TRACE_LINE (paths[pBot->prev_wpt_index[0]]->origin, pBot->wpt_origin, ignore_monsters, pEdict, &tr4);
               if ( IsShootableBreakable (tr4.pHit)
                   || (((iAmountLooped == 1) || (iAmountLooped == 4))
                        && (FStrEq(STRING(tr4.pHit->v.classname),"func_breakable")) && (tr4.pHit != pBot->pBreakableIgnore)
                        && (tr4.pHit->v.takedamage > 0) && ((tr4.pHit->v.impulse == 0) || (iAmountLooped == 1))
                        && (tr4.pHit->v.health < 1000))) // KWo - 12.03.2010
               {
                  if (g_b_DebugNavig || g_b_DebugTasks || g_b_DebugEntities)
                     ALERT(at_logged, "[DEBUG] BotDoWaypointNav - Bot %s found a breakable (2) - faces with fCone = %f.\n",
                        pBot->name, fCone);

                  pBot->pShootBreakable = tr4.pHit;
                  pBot->iCampButtons = pEdict->v.button & IN_DUCK;
                  bottask_t TempTask = {NULL, NULL, TASK_SHOOTBREAKABLE, TASKPRI_SHOOTBREAKABLE, -1, 0.0f, FALSE};
                  BotPushTask (pBot, &TempTask);
               }
            }
         }
      }
      else if (iAmountLooped < 4)
      {
         if ((pBot->prev_wpt_index[0] >= 0) && (pBot->prev_wpt_index[0] < g_iNumWaypoints))
            v_trace_start = paths[pBot->prev_wpt_index[0]]->origin;
         else
            v_trace_start = pEdict->v.origin;

         v_trace_end = paths[pBot->curr_wpt_index]->origin; // KWo - 15.04.2013

         if (iAmountLooped == 1)
         {
            v_trace_start = v_trace_start + pEdict->v.view_ofs;
            v_trace_end = v_trace_end + pEdict->v.view_ofs;
         }
         else if (iAmountLooped == 2)
         {
            v_trace_start = v_trace_start - pEdict->v.view_ofs;
            v_trace_end = v_trace_end - pEdict->v.view_ofs;
         }
         else if ((iAmountLooped == 3) && (pBot->prev_wpt_index[0] >= 0) && (pBot->prev_wpt_index[0] < g_iNumWaypoints))   // KWo - 17.02.2006
         {
            v_trace_start = paths[pBot->prev_wpt_index[0]]->origin;
            v_trace_end = paths[pBot->curr_wpt_index]->origin; // KWo - 15.04.2013
            vecDirection = (v_trace_end - v_trace_start).Normalize ();
            v_trace_end = v_trace_start + vecDirection * 32;
         }

         if (g_i_botthink_index == g_iFrameCounter) // KWo - 23.03.2007
         {
            TRACE_LINE (v_trace_start, v_trace_end, ignore_monsters, pEdict, &tr); // KWo - 15.10.2006
            pBot->pHit = tr.pHit; // KWo - 23.03.2007
            goto breakabledoorcheck;
         }
         tr.pHit = pBot->pHit; // KWo - 23.03.2007
      }
   }

   // Check if Waypoint has a special Travelflag, so they need to be reached more precisely
   for (i = 0; i < MAX_PATH_INDEX; i++)
   {
      if (paths[pBot->curr_wpt_index]->connectflag[i] != 0)
      {
         fDesiredDistance = 0.0f;
         break;
      }
   }

   // Needs precise placement - check if we get past the point
   if ((fDesiredDistance < 20.0f) && (wpt_distance < 30.0f))
   {
      v_OriginNextFrame = pEdict->v.origin + (pEdict->v.velocity * pBot->fTimeFrameInterval);  // KWo - 17.10.2006 - reverted back

      if ((v_OriginNextFrame - pBot->wpt_origin).Length () > wpt_distance)
         fDesiredDistance = wpt_distance + 1;
   }

   if (wpt_distance <= fDesiredDistance)
   {
      // Did we reach a destination Waypoint ?
      if (BotGetSafeTask (pBot)->iData == pBot->curr_wpt_index)
      {
         // Add Goal Values
         if ((pBot->chosengoal_index != -1) && (pBot->chosengoal_index < g_iNumWaypoints) && (BotGetSafeTask(pBot)->iTask != TASK_FOLLOWUSER)) // KWo - 27.06.2006
         {
            // Changed goal WP with current WP in goal experience collecting - KWo - 28.12.2009
            iGoalIndex = pBot->chosengoal_index;
            iStartIndex = pBot->curr_wpt_index;

            if (pBot->bot_team == TEAM_CS_TERRORIST)
            {
               iWPTValue = (pBotExperienceData + (iStartIndex * g_iNumWaypoints) + iGoalIndex)->wTeam0Value;
               iWPTValue += (int) (pBot->pEdict->v.health * 0.05f);	// KWo - to remove warning
               iWPTValue += (int) (pBot->f_goal_value * 0.05f);	// KWo - to remove warning

               if (iWPTValue < -MAX_GOAL_VAL)
                  iWPTValue = -MAX_GOAL_VAL;
               else if (iWPTValue > MAX_GOAL_VAL)
                  iWPTValue = MAX_GOAL_VAL;

               (pBotExperienceData + (iStartIndex * g_iNumWaypoints) + iGoalIndex)->wTeam0Value = iWPTValue;
            }
            else
            {
               iWPTValue = (pBotExperienceData + (iStartIndex * g_iNumWaypoints) + iGoalIndex)->wTeam1Value;
               iWPTValue += (int) (pBot->pEdict->v.health * 0.05f);	// KWo - to remove warning
               iWPTValue += (int) (pBot->f_goal_value * 0.05f);	// KWo - to remove warning

               if (iWPTValue < -MAX_GOAL_VAL)
                  iWPTValue = -MAX_GOAL_VAL;
               else if (iWPTValue > MAX_GOAL_VAL)
                  iWPTValue = MAX_GOAL_VAL;

               (pBotExperienceData + (iStartIndex * g_iNumWaypoints) + iGoalIndex)->wTeam1Value = iWPTValue;
            }
         }

         if (g_b_DebugNavig)
            ALERT(at_logged, "[DEBUG] BotDoWaypointNav - Bot %s reached the dest. waypoint.\n", pBot->name);

         pBot->prev_time = gpGlobals->time + 0.5f;     // KWo - 25.05.2010
         pBot->f_moved_distance = 15.0f;               // KWo - 25.05.2010
         return (TRUE);
      }

      else if (pBot->pWaypointNodes == NULL)
      {
         if (g_b_DebugNavig)
            ALERT(at_logged, "[DEBUG] BotDoWaypointNav - Bot %s has no nodes to follow.\n", pBot->name);

         return (FALSE);
      }

      // Defusion Map ?
      if (g_iMapType & MAP_DE)
      {
         // Bomb planted and CT ?
         if (g_bBombPlanted && (pBot->bot_team == TEAM_CS_COUNTER))
         {
            iWPTIndex = BotGetSafeTask (pBot)->iData;

            if (iWPTIndex != -1)
            {
               fDistance = (pEdict->v.origin - paths[iWPTIndex]->origin).Length ();

               // Bot within 'hearable' Bomb Tick Noises ?
               if (fDistance < 0.5f * BOMBMAXHEARDISTANCE) // KWo - 07.02.2007
               {
                  // Does hear Bomb ?
                  if (BotHearsBomb (pEdict->v.origin))
                  {
                     fDistance = (g_vecBomb - paths[iWPTIndex]->origin).Length ();

                     if (fDistance > 512.0f)
                     {
                        // Doesn't hear so not a good goal
                        CTBombPointClear (iWPTIndex);
                     }
                  }
                  else
                  {
                     // Doesn't hear so not a good goal
                     CTBombPointClear (iWPTIndex);

// KWo - 07.02.2007 - if the bot is somewhere near other goal waypoints and it cannot hear the bomb
// - these bomb waypoints should be also marked as visited...
                     for (i = 0; i < g_iNumGoalPoints; i++)
                     {
                        if (g_rgiGoalWaypoints[i] == iWPTIndex)
                           continue;

                        fDistance = (paths[g_rgiGoalWaypoints[i]]->origin - pEdict->v.origin).Length ();
                        if (fDistance < 0.9 * BOMBMAXHEARDISTANCE)
                        {
                           iTempIndex = g_rgiGoalWaypoints[i];
                           CTBombPointClear (iTempIndex);
                        }
                      }
                      DeleteSearchNodes (pBot); // KWo this needs to be heavily tested...
                      BotGetSafeTask (pBot)->iData = -1;
                      pBot->chosengoal_index = -1;
                  }
               }
            }
         }
      }
      // Do the actual movement checking
      BotHeadTowardWaypoint (pBot);
   }
   return (FALSE);
}

int BotFindGoal (bot_t *pBot)
{
   // Chooses a Destination (Goal) Waypoint for a Bot

   int iTactic;
   int iOffensive;
   int iDefensive;
   int iGoalDesire;
   int iForwardDesire;
   int iCampDesire;
   int iBackoffDesire;
   int iTacticChoice;
   int *pOffensiveWPTS = NULL;
   int iNumOffensives;
   int *pDefensiveWPTS = NULL;
   int iNumDefensives;
   bool bHasHostage = BotHasHostage (pBot);
   int index;
   int index2; // KWo - 14.04.2008
   int min_index;
   float distance;
   float min_distance;
   int iGoalChoices[4];
//   float iGoalDistances[4];
   edict_t *pEdict;
   int iBotIndex = pBot->curr_wpt_index;  // KWo - 14.02.2006
   int iTestIndex; // KWo - 14.02.2006
   bool bSorting; // KWo - 14.02.2006
   bool bGoalNearby = false;  // KWo - 12.02.2006
   int iClosestGoalIndex = -1;  // KWo - 12.02.2006
   int iRescueIndex; // KWo - 24.12.2206
   pEdict = pBot->pEdict;

   // if there is debugoal enabled - return the index of the debugged goal
   if (g_iDebugGoalIndex != -1) // KWo - 25.07.2006
   {
      pBot->chosengoal_index = g_iDebugGoalIndex;
         return (g_iDebugGoalIndex);
   }

   // Pathfinding Behaviour depending on Maptype
   if (pBot->bot_team == TEAM_CS_TERRORIST)  // KWo - 19.01.2011
   {
      if (g_iMapType & MAP_DE)
      {
         pOffensiveWPTS = &g_rgiTerrorWaypoints[0];
         iNumOffensives = g_iNumTerrorPoints;
         pDefensiveWPTS = &g_rgiCTWaypoints[0];
         iNumDefensives = g_iNumCTPoints;
      }
      else
      {
         pOffensiveWPTS = &g_rgiCTWaypoints[0];
         iNumOffensives = g_iNumCTPoints;
         pDefensiveWPTS = &g_rgiTerrorWaypoints[0];
         iNumDefensives = g_iNumTerrorPoints;
      }
   }
   else
   {
      if (g_iMapType & MAP_DE)
      {
         pOffensiveWPTS = &g_rgiTerrorWaypoints[0];
         iNumOffensives = g_iNumTerrorPoints;
         pDefensiveWPTS = &g_rgiCTWaypoints[0];
         iNumDefensives = g_iNumCTPoints;
      }
      else
      {
         pOffensiveWPTS = &g_rgiCTWaypoints[0];
         iNumOffensives = g_iNumCTPoints;
         pDefensiveWPTS = &g_rgiTerrorWaypoints[0];
         iNumDefensives = g_iNumTerrorPoints;
      }
   }

   // Terrorist carrying the C4 ?
   if ((pBot->pEdict->v.weapons & (1 << CS_WEAPON_C4))
       || pBot->bIsVIP)
   {
      iTactic = 3;
      goto tacticchosen;
   }

   else if (bHasHostage && (pBot->bot_team == TEAM_CS_COUNTER))
   {
      min_distance = 9999.0f;
      min_index = -1;

      for (index = 0; index < g_iNumRescuePoints; index++) // KWo - 24.12.2006
      {
         iRescueIndex = g_rgiRescueWaypoints[index];
         distance = (paths[iRescueIndex]->origin - pEdict->v.origin).Length();

         if (distance < min_distance)
         {
            min_distance = distance;
            min_index = index;
         }
      }

      if (min_index != -1)
      {
         pBot->chosengoal_index = g_rgiRescueWaypoints[min_index]; // KWo - 27.02.2007
         if (pBot->chosengoal_index != pBot->curr_wpt_index) // KWo - 27.02.2007
            pBot->fTimeHostageRescue = 0.0f;

         if (g_b_DebugNavig)
            ALERT(at_logged, "[DEBUG] BotFindGoal - Bot %s has a hostage and chose a goal = %d, flags = %d.\n",
               pBot->name, pBot->chosengoal_index, paths[pBot->chosengoal_index]->flags);

         return (g_rgiRescueWaypoints[min_index]);
      }
   }

   iOffensive = (int) pBot->fAgressionLevel * 100;
   iDefensive = (int) pBot->fFearLevel * 100;

   if (g_b_DebugNavig)
      ALERT(at_logged, "[DEBUG] BotFindGoal - Bot's %s AgressionLevel = %f, FearLevel = %f.\n", pBot->name, pBot->fAgressionLevel, pBot->fFearLevel);

   if ((g_iMapType & MAP_AS) || (g_iMapType & MAP_CS))
   {
      if (pBot->bot_team == TEAM_CS_TERRORIST)
      {
         iDefensive += 30;
         iOffensive -= 30;
      }
      else // KWo - 27.06.2006
      {
         iDefensive -= 30;
         iOffensive += 30;
      }
   }

   if (g_iMapType & MAP_DE) // KWo - 19.01.2011
   {
      if (pBot->bot_team == TEAM_CS_COUNTER)
      {
         if (g_bBombPlanted)
         {
            if ((g_b_cv_chat) && (g_bBombSayString) && (g_iAliveCTs > 1)) // KWo - 16.05.2008
            {
               BotPrepareChatMessage (pBot, szBombChat[RANDOM_LONG (0, iNumBombChats - 1)]);
               BotPushMessageQueue (pBot, MSG_CS_SAY);
               g_bBombSayString = FALSE;
            }
            int iTemp = BotChooseBombWaypoint (pBot); // KWo - 27.06.2006
            pBot->chosengoal_index = iTemp; // KWo - 27.06.2006
            return (iTemp); // KWo - 27.06.2006
         }

         iDefensive += 30;
         iOffensive -= 30;
      }
      else // KWo - 27.06.2006
      {
         iDefensive -= 30;
         iOffensive += 30;
      }
   }
/*
   iGoalDesire = RANDOM_LONG (0, 100) + iOffensive;
   iForwardDesire = RANDOM_LONG (0, 100) + iOffensive;
   iCampDesire = RANDOM_LONG (0, 100) + iDefensive;
   iBackoffDesire = RANDOM_LONG (0, 100) + iDefensive;
*/
   iGoalDesire = RANDOM_LONG (0, 70) + iOffensive;
   iForwardDesire = RANDOM_LONG (0, 50) + iOffensive;
   iCampDesire = RANDOM_LONG (0, 70) + iDefensive;
   iBackoffDesire = RANDOM_LONG (0, 50) + iDefensive;

   if (!BotHasCampWeapon(pBot)) // KWo - 19.05.2010
      iCampDesire = 0;
   else if (BotHasSniperWeapon(pBot))
      iCampDesire = (int)(RANDOM_FLOAT(1.5f, 2.5f) * (float)(iCampDesire));

   if (g_b_DebugNavig)
      ALERT(at_logged, "[DEBUG] BotFindGoal - Bot's %s GoalDesire = %d, ForwardDesire = %d, iCampDesire = %d, iBackoffDesire = %d.\n",
         pBot->name, iGoalDesire, iForwardDesire, iCampDesire, iBackoffDesire);

   iTacticChoice = iBackoffDesire;
   iTactic = 0;

   if (iCampDesire > iTacticChoice)
   {
      iTacticChoice = iCampDesire;
      iTactic = 1;
   }
   if (iForwardDesire > iTacticChoice)
   {
      iTacticChoice = iForwardDesire;
      iTactic = 2;
   }
   if (iGoalDesire > iTacticChoice)
   {
      iTacticChoice = iGoalDesire;
      iTactic = 3;
   }

tacticchosen:
   for (index = 0; index < 4; index++)
   {
      iGoalChoices[index] = -1;
//      iGoalDistances[index] = 9999.0f;
   }

   // Defensive Goal
   if (iTactic == 0)
   {
      for (index = 0; index < 4; index++)
      {
         iGoalChoices[index] = pDefensiveWPTS[RANDOM_LONG (0, iNumDefensives - 1)];
         assert ((iGoalChoices[index] >= 0) && (iGoalChoices[index] < g_iNumWaypoints));
      }
   }

   // Camp Waypoint Goal
   else if (iTactic == 1)
   {
      for (index = 0; index < 4; index++)
      {
         iGoalChoices[index] = g_rgiCampWaypoints[RANDOM_LONG (0, g_iNumCampPoints - 1)];
         assert ((iGoalChoices[index] >= 0) && (iGoalChoices[index] < g_iNumWaypoints));
      }
   }

   // Offensive Goal
   else if (iTactic == 2)
   {
      for (index = 0; index < 4; index++)
      {
         iGoalChoices[index] = pOffensiveWPTS[RANDOM_LONG (0, iNumOffensives - 1)];
         assert ((iGoalChoices[index] >= 0) && (iGoalChoices[index] < g_iNumWaypoints));
      }
   }

   // Map Goal Waypoint
   else if (iTactic == 3)
   {
      Vector vBotOrigin = pBot->pEdict->v.origin;  // KWo - 12.02.2006
      index2 = 0;
      for (index = 0; index < g_iNumGoalPoints; index++)  // KWo - 12.02.2006
      {
         iClosestGoalIndex = g_rgiGoalWaypoints[index];
         if ((iClosestGoalIndex >= 0) && (iClosestGoalIndex < g_iNumWaypoints))
         {
            if (((vBotOrigin - paths[iClosestGoalIndex]->origin).Length() < 1000)  // KWo - 14.04.2008
                && (!IsGroupOfEnemies (pBot, paths[iClosestGoalIndex]->origin)))
            {
               iGoalChoices[index2] = iClosestGoalIndex;
               bGoalNearby = true;
               index2++;
               if (index2 > 3)
                  break;
            }
         }
      }
      if (!bGoalNearby)
      {
         for (index = 0; index < 4; index++)
         {
            iGoalChoices[index] = g_rgiGoalWaypoints[RANDOM_LONG (0, g_iNumGoalPoints - 1)];
            assert ((iGoalChoices[index] >= 0) && (iGoalChoices[index] < g_iNumWaypoints));
         }
      }
      else if (pBot->pEdict->v.weapons & (1 << CS_WEAPON_C4))  // KWo - 14.04.2008
      {
         iClosestGoalIndex = iGoalChoices[RANDOM_LONG (0, index2 - 1)];
         pBot->chosengoal_index = iClosestGoalIndex;
         return (iClosestGoalIndex);
      }
   }


   if ((pBot->curr_wpt_index < 0) || (pBot->curr_wpt_index >= g_iNumWaypoints))
      BotChangeWptIndex (pBot, WaypointFindNearestToMove (pBot->pEdict, pBot->pEdict->v.origin)); // KWo - 17.04.2008


   if (pBot->bot_personality == 1) // Psycho doesn't care about danger :)  KWo - 14.02.2006
   {
      iTestIndex = iGoalChoices[RANDOM_LONG (0, 3)];
      if ((iTestIndex >=0) && (iTestIndex < g_iNumWaypoints))
      {
         pBot->chosengoal_index = iTestIndex;  // KWo - 27.06.2006

         if (g_b_DebugNavig)
            ALERT(at_logged, "[DEBUG] BotFindGoal - Bot's %s chose goal = %d, flags = %d.\n",
            pBot->name, pBot->chosengoal_index, paths[pBot->chosengoal_index]->flags);

         return (iTestIndex);
      }
   }

   do
   {
      bSorting = FALSE;
      for (index = 0; index < 3; index++)
      {
         iTestIndex = iGoalChoices[index + 1];
         if (iTestIndex < 0)
            break;

         if (pBot->bot_team == TEAM_CS_TERRORIST)
         {
            if ((pBotExperienceData + (iBotIndex * g_iNumWaypoints) + iGoalChoices[index])->wTeam0Value
                < (pBotExperienceData + (iBotIndex * g_iNumWaypoints) + iTestIndex)->wTeam0Value)
            {
               iGoalChoices[index + 1] = iGoalChoices[index];
               iGoalChoices[index] = iTestIndex;
               bSorting = TRUE;
            }
         }
         else
         {
            if ((pBotExperienceData + (iBotIndex * g_iNumWaypoints) + iGoalChoices[index])->wTeam1Value
                < (pBotExperienceData + (iBotIndex * g_iNumWaypoints) + iTestIndex)->wTeam1Value)
            {
               iGoalChoices[index + 1] = iGoalChoices[index];
               iGoalChoices[index] = iTestIndex;
               bSorting = TRUE;
            }
         }
      }
   } while (bSorting);

   pBot->chosengoal_index = iGoalChoices[0]; // KWo - 27.06.2006

   if (g_b_DebugNavig)
      ALERT(at_logged, "[DEBUG] BotFindGoal - Bot's %s chose goal = %d, flags %d.\n",
         pBot->name, pBot->chosengoal_index, paths[pBot->chosengoal_index]->flags);

   return (iGoalChoices[0]);
}

void BotResetCollideState (bot_t *pBot)
{
   int i;  // KWo - 09.07.2006
   pBot->f_probe_time = 0.0f;
   pBot->cCollisionProbeBits = 0;
   pBot->cCollisionState = COLLISION_NOTDECIDED;
   pBot->cCollStateIndex = 0;
   for (i = 0; i < 5; i++)                // KWo - 09.07.2006
   {
      pBot->cCollideMoves[i] = 0;
   }
   pBot->bBotNeedsObviateObstacle = false;   // KWo - 16.07.2006
   pBot->bPlayerCollision = false;        // KWo - 27.02.2007
   pBot->bHitDoor = false;                // KWo - 24.10.2009

   if (g_b_DebugStuck)
   {
      if ((pHostEdict->v.origin - pBot->pEdict->v.origin).Length() < 30.0f)
         ALERT(at_logged, "[DEBUG] BotResetCollideState - Bot %s resets collide state.\n", pBot->name);
   }

   return;
}

// the function checks the possibility to avoid getting stuck (0 - do nothing, 1 - go right, -1 - go left
int BotCheckCollisionWithPlayer (bot_t *pBot, edict_t *pPlayer) // KWo - 01.04.2010
{
   edict_t *pEdict = pBot->pEdict;
   int i;
   int iTask = BotGetSafeTask(pBot)->iTask;
   float f_Distance;
   float f_DistanceL;
   float f_DistanceR;
   Vector v_Direction;
   Vector v_MoveAngles;
   Vector v_BotVelocity;
   Vector2D v2_BotPosition;
   Vector2D v2_PlayerPosition;
   Vector2D v2_BotVelocity;
   Vector2D v2_BotVelocityL;
   Vector2D v2_BotVelocityR;
   Vector2D v2_PlayerVelocity;
   Vector2D v2_BotRightSide;
   Vector2D v2_BotFuturePosition;
   Vector2D v2_PlayerFuturePosition;

   Vector2D v2_BotDestination;

   if (FNullEnt(pEdict) || FNullEnt(pPlayer))
      return (0);
   if (pBot->bOnLadder)
      return (0);
   if (!IsAlive (pPlayer))
      return (0);
   f_Distance = (pPlayer->v.origin - pEdict->v.origin).Length ();
   if (f_Distance > 240.0f)
      return (0);
   if (fabs(pPlayer->v.origin.z - pEdict->v.origin.z) > 60.0f)
      return (0);

   if ((iTask == TASK_ATTACK) || (pBot->f_move_speed == 0.0f)) // KWo - 07.04.2010
      v_Direction = pBot->vecLookAt - pEdict->v.origin;
   else
      v_Direction = pBot->dest_origin - pEdict->v.origin;

   v_Direction = v_Direction.Normalize ();

   v_MoveAngles = UTIL_VecToAngles (v_Direction);
   v_MoveAngles.x = -v_MoveAngles.x;
   v_MoveAngles.z = 0.0f;
   UTIL_ClampVector (&v_MoveAngles);
   MAKE_VECTORS (v_MoveAngles);
   v2_BotRightSide = (gpGlobals->v_right).Make2D();

   v2_PlayerVelocity = (pPlayer->v.velocity).Make2D();
   v_BotVelocity = v_Direction * pBot->f_move_speed;
   v2_BotVelocity = (v_BotVelocity).Make2D();
   v2_BotVelocityL = ((-pEdict->v.maxspeed) * v2_BotRightSide) + v2_BotVelocity;
   v2_BotVelocityR = ((pEdict->v.maxspeed) * v2_BotRightSide) + v2_BotVelocity;

   v2_BotPosition = (pEdict->v.origin).Make2D();
   v2_PlayerPosition = (pPlayer->v.origin).Make2D();

   f_DistanceL = 300.0f;
   f_DistanceR = 300.0f;

   for (i = 1; i <= 10; i++)
   {
      v2_BotFuturePosition = v2_BotVelocityL * 0.1f * i + v2_BotPosition;
      v2_PlayerFuturePosition = v2_PlayerVelocity * 0.1f * i + v2_PlayerPosition;
      f_Distance = (v2_BotFuturePosition - v2_PlayerFuturePosition).Length();
      if (f_Distance < f_DistanceL)
         f_DistanceL = f_Distance;
   }

   for (i = 1; i <= 10; i++)
   {
      v2_BotFuturePosition = v2_BotVelocityR * 0.1f * i + v2_BotPosition;
      v2_PlayerFuturePosition = v2_PlayerVelocity * 0.1f * i + v2_PlayerPosition;
      f_Distance = (v2_BotFuturePosition - v2_PlayerFuturePosition).Length();
      if (f_Distance < f_DistanceR)
         f_DistanceR = f_Distance;
   }

   if ((f_DistanceR == 300.0f) && (f_DistanceL == 300.0f))
      return (0);
   else if ((f_DistanceL <= f_DistanceR) && (f_DistanceL < (((pEdict->v.velocity).Length() > 160.0f)? 100.0f : 50.0f))) // KWo - 28.04.2010
      return (1); // go right
   else if ((f_DistanceL > f_DistanceR) && (f_DistanceR < (((pEdict->v.velocity).Length() > 160.0f)? 100.0f : 50.0f))) // KWo - 28.04.2010
      return (-1); // go left

   return (0);
}

// the function lets the bot to unstuck
void BotCheckTerrain (bot_t *pBot) // KWo - 19.07.2006
{
   edict_t *pent;
   edict_t *pEdict = pBot->pEdict;
   edict_t *pNearestPlayer = NULL;
   bool bBypassPlayer = FALSE;
   bool bBotIsStuck = FALSE;
   bool bCurrWptIndexOK = ((pBot->curr_wpt_index >= 0) && (pBot->curr_wpt_index < g_iNumWaypoints));
   bool bPrevWptIndexOK = ((pBot->prev_wpt_index[0] >= 0) && (pBot->prev_wpt_index[0] < g_iNumWaypoints));
   bool bCurrentWpGoal = FALSE;              // KWo - 19.04.2010
   bool bCurrentTravelJump = FALSE;          // KWo - 19.04.2010
   int Ent_Index; // KWo - 06.03.2006
   int i;
   int iTask = BotGetSafeTask(pBot)->iTask;  // KWo - 17.01.2010
   int iBotBypassPlayerColl = 0;                // KWo - 01.04.2010
   int iMoveBack = 1;                        // KWo - 07.04.2010
   int index;
   Vector v_diff;
   Vector v_src;
   Vector v_forward;  // KWo - 29.07.2018
   Vector v_center;   // KWo - 29.07.2018
   Vector v_dest;
   Vector v_direction;
   Vector vecDirectionNormal;
   Vector vecDirection;
   Vector vecMoveAngles;
   Vector v_BotRightSide; // KWo - 02.04.2010
   Vector v_BotVelocity;  // KWo - 02.04.2010
   Vector v_BotVelocityL; // KWo - 02.04.2010
   Vector v_BotVelocityR; // KWo - 02.04.2010
   Vector v_BotTargetPos; // KWo - 02.04.2010
   TraceResult tr;
   TraceResult tr2;
   float f_nearestdistance = 240.0f; // KWo - 01.04.2010
   float f_distance_now;
   float fDistToCurWP;              // KWo - 27.01.2012
   float fDistToNextWP;             // KWo - 27.01.2012
   float fDistToPrevWP;             // KWo - 27.01.2012


   if ((iTask == TASK_ATTACK) || (pBot->dest_origin == pBot->pEdict->v.origin) || (pBot->f_move_speed == 0.0f)) // KWo - 13.10.2011
      v_direction = pBot->vecLookAt - pEdict->v.origin;
   else
      v_direction = pBot->dest_origin - pEdict->v.origin;

   vecDirectionNormal = v_direction.Normalize ();
   vecDirection = vecDirectionNormal;
   vecDirectionNormal.z = 0.0f;

   vecMoveAngles = UTIL_VecToAngles (v_direction);
   vecMoveAngles.x = -vecMoveAngles.x;
   vecMoveAngles.z = 0.0f;
   UTIL_ClampVector (&vecMoveAngles);


   if (g_b_DebugStuck)
      ALERT(at_logged, "[DEBUG] BotCheckTerrain called for bot %s.\n", pBot->name);

   // Test if there's a shootable breakable in our way
   v_src = GetGunPosition (pEdict); // EyePosition ()
   v_dest = v_src + vecDirection * 32;  // KWo - 08.02.2006

   if (((pBot->cCollisionState == COLLISION_NOTDECIDED) || (pBot->f_probe_time < gpGlobals->time))
        && ((g_i_botthink_index % 4) == (g_iFrameCounter % 4))) // KWo - 22.10.2011
   {
      TRACE_HULL (v_src, v_dest, dont_ignore_monsters,
         (((pEdict->v.flags & IN_DUCK) || paths[pBot->curr_wpt_index]->flags & W_FL_CROUCH) ? point_hull : head_hull), pEdict, &tr);   // KWo - 17.04.2013

      if ((tr.flFraction != 1.0f) && (!FNullEnt (tr.pHit)))
      {
         if ( IsShootableBreakable (tr.pHit)
            || ((FStrEq(STRING(tr.pHit->v.classname),"func_breakable")) && (tr.pHit != pBot->pBreakableIgnore)
                && (tr.pHit->v.takedamage > 0) && (tr.pHit->v.impulse == 0) && (tr.pHit->v.health < 1000))) // KWo - 12.03.2010
         {
            pent = tr.pHit;
            Ent_Index = ENTINDEX(pent); // KWo - 06.03.2006
            float fCone = 2.0f;  // KWo - 31.01.2006
            if (bPrevWptIndexOK)
            {
               fCone = UTIL_GetVectorsCone (pBot->wpt_origin, paths[pBot->prev_wpt_index[0]]->origin,
                        VecBModelOrigin (tr.pHit), paths[pBot->prev_wpt_index[0]]->origin); // KWo - 13.02.2006
               for (i = 0; i < g_iNumBreakables; i++) // KWo - 06.03.2006
               {
                  if (Ent_Index == BreakablesData[i].EntIndex)
                  {
                     if (BreakablesData[i].ignored)
                     {
                        fCone = 1.0f;
                        break;
                     }
                  }
               }
            }
            else
            {
               fCone = 0.1f;
            }

            if (fCone < 0.6f) // KWo - 31.01.2006 - shoot only breakable in some certain directions (if needed)
            {
               pBot->pShootBreakable = pent;
               pBot->iCampButtons = pEdict->v.button & IN_DUCK;
               bottask_t TempTask = {NULL, NULL, TASK_SHOOTBREAKABLE, TASKPRI_SHOOTBREAKABLE, -1, 0.0f, FALSE};

               if (g_b_DebugEntities || g_b_DebugStuck || g_b_DebugTasks)
                  ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s found a breakable (1) - faces with fCone = %f.\n",
                     pBot->name, fCone);

               BotPushTask (pBot, &TempTask);
               BotResetCollideState (pBot); // KWo - 09.07.2006
            }
         }
      }
      else
      {
         v_src = pEdict->v.origin;
         v_dest = v_src + vecDirection * 48;   // KWo - 08.02.2006
         TRACE_HULL (v_src, v_dest, dont_ignore_monsters,
            (((pEdict->v.flags & IN_DUCK) || paths[pBot->curr_wpt_index]->flags & W_FL_CROUCH) ? point_hull : head_hull), pEdict, &tr);   // KWo - 17.04.2013

         if ((tr.flFraction != 1.0f) && (!FNullEnt (tr.pHit)))  // KWo - 26.01.2006
         {
            if ( IsShootableBreakable (tr.pHit)
               || ((FStrEq(STRING(tr.pHit->v.classname),"func_breakable")) && (tr.pHit != pBot->pBreakableIgnore)
                && (tr.pHit->v.takedamage > 0) && (tr.pHit->v.impulse == 0) && (tr.pHit->v.health < 1000))) // KWo - 12.03.2010
            {
               pent = tr.pHit;
               Ent_Index = ENTINDEX(pent); // KWo - 06.03.2006
               float fCone = 2.0f;  // KWo - 31.01.2006
               if (bPrevWptIndexOK)
               {
                  fCone = UTIL_GetVectorsCone (pBot->wpt_origin, paths[pBot->prev_wpt_index[0]]->origin,
                          VecBModelOrigin (tr.pHit), paths[pBot->prev_wpt_index[0]]->origin); // KWo - 13.02.2006
                  for (i = 0; i < g_iNumBreakables; i++) // KWo - 06.03.2006
                  {
                     if (Ent_Index == BreakablesData[i].EntIndex)
                     {
                        if (BreakablesData[i].ignored)
                        {
                           fCone = 1.0f;
                           break;
                        }
                     }
                  }
               }
               else
               {
                  fCone = 0.1f;
               }
               if (fCone < 0.6f) // KWo - 31.01.2006 - shoot only breakable in some certain directions (if needed)
               {
               // Check if this isn't a triggered (bomb) breakable and
               // if it takes damage. If true, shoot the crap!
                  pBot->pShootBreakable = pent;
                  pBot->iCampButtons = pEdict->v.button & IN_DUCK;
                  bottask_t TempTask = {NULL, NULL, TASK_SHOOTBREAKABLE, TASKPRI_SHOOTBREAKABLE, -1, 0.0f, FALSE};

                  if (g_b_DebugEntities || g_b_DebugStuck || g_b_DebugTasks)
                     ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s found a breakable (2) - faces with fCone = %f.\n", pBot->name, fCone);

                  BotPushTask (pBot, &TempTask);
                  BotResetCollideState (pBot); // KWo - 09.07.2006
               }
            }
         }
      }
   }
   if (((pBot->cCollisionState == COLLISION_NOTDECIDED) || (pBot->f_probe_time < gpGlobals->time))
       && (bPrevWptIndexOK) && (bCurrWptIndexOK) && (FNullEnt (pBot->pShootBreakable))
       && ((g_i_botthink_index % 4) == (g_iFrameCounter % 4))
       && ((iTask != TASK_ATTACK) || (pBot->byFightStyle != 1))) // KWo - 22.10.2011
   {
      v_src = paths[pBot->prev_wpt_index[0]]->origin;
      v_dest = paths[pBot->curr_wpt_index]->origin;
      TRACE_LINE (v_src, v_dest, dont_ignore_monsters, pEdict, &tr);
      if ((tr.flFraction != 1.0f) && (!FNullEnt (tr.pHit)))  // KWo - 26.01.2006
      {
         if ( IsShootableBreakable (tr.pHit)
            || ((FStrEq(STRING(tr.pHit->v.classname),"func_breakable")) && (tr.pHit != pBot->pBreakableIgnore)
             && (tr.pHit->v.takedamage > 0) && (tr.pHit->v.impulse == 0) && (tr.pHit->v.health < 1000))) // KWo - 12.03.2010
         {
            pBot->pShootBreakable = tr.pHit;
            pBot->iCampButtons = pEdict->v.button & IN_DUCK;
            bottask_t TempTask = {NULL, NULL, TASK_SHOOTBREAKABLE, TASKPRI_SHOOTBREAKABLE, -1, 0.0f, FALSE};

            if (g_b_DebugEntities || g_b_DebugStuck || g_b_DebugTasks)
               ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s found a breakable (3).\n", pBot->name);

            BotPushTask (pBot, &TempTask);
            BotResetCollideState (pBot);
         }
      }
   }

   // No Breakable blocking ?
   if (FNullEnt (pBot->pShootBreakable))
   {
      // Standing still, no need to check ?
      // FIXME: Doesn't care for ladder movement (handled separately)
      // should be included in some way
      if ((pBot->f_move_speed != 0.0f) || (pBot->f_sidemove_speed != 0.0f) || (true)) // KWo - 17.01.2010
      {
         pent = NULL;
         pNearestPlayer = NULL;
         index = 0;
         if (bCurrWptIndexOK)
         {
            if (BotGetSafeTask(pBot)->iData == pBot->curr_wpt_index)
               bCurrentWpGoal = TRUE;
            if (pBot->curr_travel_flags & C_FL_JUMP)
               bCurrentTravelJump = TRUE;
         }
         // Find nearest player to Bot
         for (i = 0; i < gpGlobals->maxClients; i++)  // KWo - 30.04.2006
         {
            if (!(clients[i].iFlags & CLIENT_USED)
               || !(clients[i].iFlags & CLIENT_ALIVE)
               || (clients[i].pEdict == pEdict)
               || (clients[i].pEdict->v.effects & EF_NODRAW))
               continue;

            if (fabs(clients[i].pEdict->v.origin.z - pEdict->v.origin.z) < 32)
            {
               v_src = clients[i].pEdict->v.origin;
               v_dest = pEdict->v.origin;
            }
            else if ((clients[i].pEdict->v.origin.z - pEdict->v.origin.z) > 0)
            {
               v_src = clients[i].pEdict->v.origin + Vector(0.0f, 0.0f, -16.0f);
               v_dest = pEdict->v.origin + Vector(0.0f, 0.0f, 16.0f);
            }
            else
            {
               v_src = clients[i].pEdict->v.origin + Vector(0.0f, 0.0f, 16.0f);
               v_dest = pEdict->v.origin + Vector(0.0f, 0.0f, -16.0f);
            }

            f_distance_now = (v_src - v_dest).Length ();

            if (f_distance_now < f_nearestdistance)
            {
               pNearestPlayer = clients[i].pEdict;
               f_nearestdistance = f_distance_now;
               index = i;
            }
         }

         // Found somebody ?
         if (!FNullEnt (pNearestPlayer) && (pBot->cCollisionState == COLLISION_NOTDECIDED) /* && FInViewCone (&pNearestPlayer->v.origin, pEdict) */)
         {
            // bot found a visible teammate
            if (g_b_DebugStuck)
               ALERT (at_logged, "[DEBUG] BotCheckTerrain - Bot %s is about collision with %s, distance = %.0f, forward speed = %f.\n",
                  pBot->name, STRING(clients[index].pEdict->v.netname), f_nearestdistance, pBot->f_move_speed);

            iBotBypassPlayerColl = BotCheckCollisionWithPlayer(pBot, pNearestPlayer);

            if (g_b_DebugStuck)
               ALERT (at_logged, "[DEBUG] BotCheckTerrain - Bot %s is close to %s; c_result = %d!\n",
                  pBot->name, STRING(clients[index].pEdict->v.netname), iBotBypassPlayerColl);

            MAKE_VECTORS (vecMoveAngles);
            v_BotRightSide = gpGlobals->v_right;

            if (((!pBot->b_bomb_blinking) || (!bCurrentWpGoal)) && (!bCurrentTravelJump)
               && ((pBot->current_weapon.iId != CS_WEAPON_KNIFE)
                     || ((pBot->bot_team == clients[index].iTeam) && (!g_b_cv_ffa)))
                  && (f_nearestdistance < 240.0f) && (f_nearestdistance >= 40.0f)
                  && (iBotBypassPlayerColl != 0))  // KWo - 20.04.2010
            {
               bBypassPlayer = TRUE;
               TRACE_HULL(pEdict->v.origin + pEdict->v.view_ofs, pBot->dest_origin, dont_ignore_monsters,
                  (pEdict->v.flags & FL_DUCKING) ? head_hull : human_hull, pEdict, &tr2); // KWo - 27.04.2010
               if (tr2.flFraction == 1.0f)
                  bBypassPlayer = FALSE;
               if (!bBypassPlayer) goto nobypass;

               if ((pEdict->v.oldbuttons & IN_FORWARD) || (pEdict->v.oldbuttons & IN_BACK))
                  v_BotVelocity = vecDirection * pBot->f_move_speed;
               else
                  v_BotVelocity = g_vecZero;

               v_BotVelocityL = ((-pEdict->v.maxspeed) * v_BotRightSide) + v_BotVelocity;
               v_BotVelocityR = ((pEdict->v.maxspeed) * v_BotRightSide) + v_BotVelocity;

               if (iBotBypassPlayerColl < 0)
               {
                  if (!(pEdict->v.oldbuttons & IN_BACK)) // KWo - 07.04.2010 - bot goes forward or doesn't move at all
                  {
                     v_BotTargetPos = pEdict->v.origin + 0.2f * v_BotVelocityL;
                     if (!IsDeadlyDropAtPos(pBot, v_BotTargetPos))
                     {
                        TRACE_LINE(pEdict->v.origin, v_BotTargetPos, ignore_monsters, pEdict, &tr2); // KWo - 27.04.2010
                        if ((!FStrEq ("worldspawn", STRING (tr2.pHit->v.classname))) || (tr2.flFraction == 1.0f))
                           pBot->f_sidemove_speed = -pEdict->v.maxspeed;
                     }
                     else // KWo - 07.04.2010
                     {
                        v_BotTargetPos = pEdict->v.origin - 0.2f * vecDirection * pEdict->v.maxspeed;
                        if (!IsDeadlyDropAtPos(pBot, v_BotTargetPos))
                        {
                           pBot->f_move_speed = -pEdict->v.maxspeed;
                           pBot->f_sidemove_speed = 0.0f;
                        }
                     }
                  }
                  else
                  {
                     v_BotTargetPos = pEdict->v.origin + 0.2f * v_BotVelocityL;
                     if (!IsDeadlyDropAtPos(pBot, v_BotTargetPos))
                     {
                        TRACE_LINE(pEdict->v.origin, v_BotTargetPos, ignore_monsters, pEdict, &tr2); // KWo - 27.04.2010
                        if ((!FStrEq ("worldspawn", STRING (tr2.pHit->v.classname))) || (tr2.flFraction == 1.0f))
                           pBot->f_sidemove_speed = pEdict->v.maxspeed;
                     }
                     else // KWo - 07.04.2010
                     {
                        v_BotTargetPos = pEdict->v.origin + 0.2f * vecDirection * pEdict->v.maxspeed;
                        if (!IsDeadlyDropAtPos(pBot, v_BotTargetPos))
                        {
                           pBot->f_move_speed = pEdict->v.maxspeed;
                           pBot->f_sidemove_speed = 0.0f;
                        }
                     }
                  }
               }
               else if (iBotBypassPlayerColl > 0) // KWo - 07.04.2010
               {
                  if (!(pEdict->v.oldbuttons & IN_BACK)) // goes forward or doesn't move at all
                  {
                     v_BotTargetPos = pEdict->v.origin + 0.2f * v_BotVelocityR;
                     if (!IsDeadlyDropAtPos(pBot, v_BotTargetPos))
                     {
                        TRACE_LINE(pEdict->v.origin, v_BotTargetPos, ignore_monsters, pEdict, &tr2); // KWo - 27.04.2010
                        if ((!FStrEq ("worldspawn", STRING (tr2.pHit->v.classname))) || (tr2.flFraction == 1.0f))
                           pBot->f_sidemove_speed = pEdict->v.maxspeed;
                     }
                     else
                     {
                        v_BotTargetPos = pEdict->v.origin - 0.2f * vecDirection * pEdict->v.maxspeed;
                        if (!IsDeadlyDropAtPos(pBot, v_BotTargetPos))
                        {
                           pBot->f_move_speed = -pEdict->v.maxspeed;
                           pBot->f_sidemove_speed = 0.0f;
                        }
                     }
                  }
                  else
                  {
                     v_BotTargetPos = pEdict->v.origin + 0.2f * v_BotVelocityR;
                     if (!IsDeadlyDropAtPos(pBot, v_BotTargetPos))
                     {
                        TRACE_LINE(pEdict->v.origin, v_BotTargetPos, ignore_monsters, pEdict, &tr2); // KWo - 27.04.2010
                        if ((!FStrEq ("worldspawn", STRING (tr2.pHit->v.classname))) || (tr2.flFraction == 1.0f))
                           pBot->f_sidemove_speed = -pEdict->v.maxspeed;
                     }
                     else
                     {
                        v_BotTargetPos = pEdict->v.origin + 0.2f * vecDirection * pEdict->v.maxspeed;
                        if (!IsDeadlyDropAtPos(pBot, v_BotTargetPos))
                        {
                           pBot->f_move_speed = pEdict->v.maxspeed;
                           pBot->f_sidemove_speed = 0.0f;
                        }
                     }
                  }
               }
               if ((iBotBypassPlayerColl != 0) && (pBot->f_sidemove_speed != 0.0f) && (bPrevWptIndexOK) && (bCurrWptIndexOK)
                  && (pBot->pWaypointNodes != NULL) && (!g_bRecalcVis) && (pBot->bMoveToGoal)
                  && (BotGetSafeTask(pBot)->iData != pBot->curr_wpt_index))   // KWo - 04.04.2012
               {
                  if (pBot->pWaypointNodes->NextNode != NULL)
                  {
                     fDistToCurWP = (pEdict->v.origin - paths[pBot->curr_wpt_index]->origin).Length();
                     fDistToPrevWP = (pEdict->v.origin - paths[pBot->prev_wpt_index[0]]->origin).Length();
                     fDistToNextWP = (pEdict->v.origin - paths[pBot->pWaypointNodes->NextNode->iIndex]->origin).Length();
                     if ((fDistToCurWP < fDistToPrevWP) && (fDistToCurWP < fDistToNextWP)
                        && ((pNearestPlayer->v.origin - paths[pBot->curr_wpt_index]->origin).Length() < 80.0f)
                        && (fDistToCurWP < 80.0f))
                     {
                        // Advance in waypoints List
                        if (g_b_DebugStuck || g_b_DebugNavig)
                           ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s is by-passing another player - current WP = %d, previous WP = %d.\n",
                              pBot->name,pBot->curr_wpt_index, pBot->prev_wpt_index[0]);

                        BotHeadTowardWaypoint(pBot);
                     }
                  }
               }


               if (g_b_DebugStuck)
                  ALERT (at_logged, "[DEBUG] BotCheckTerrain - Bot %s is about collision with %s (distance = %f) and needs strafe %s.\n",
                     pBot->name, STRING(clients[index].pEdict->v.netname), f_nearestdistance, (pBot->f_sidemove_speed > 0.0f) ? "right" : "left");

// jump label
nobypass:
               if (bBypassPlayer)
                  BotResetCollideState (pBot);
            }

            if ((f_nearestdistance < 40.0f) && (pBot->f_moved_distance < ((pEdict->v.flags & FL_DUCKING)? 3.0f : 10.0f))
               && ((pBot->pEdict->v.origin - pBot->dest_origin).Length() > 20.0f))
            {
//               TRACE_HULL(pEdict->v.origin + pEdict->v.view_ofs, pBot->dest_origin, dont_ignore_monsters, (pEdict->v.flags & FL_DUCKING) ? head_hull : human_hull, pEdict, &tr2); // KWo - 28.01.2012
               pBot->bPlayerCollision = TRUE;
               if (/* (tr2.flFraction != 1.0f) && */ (pBot->cCollisionState == COLLISION_NOTDECIDED))  // KWo - 02.04.2010
               {
                  v_src = pEdict->v.origin;
                  v_dest = v_src - vecDirection * 32;
                  iMoveBack = 1; // KWo - 07.04.2010
                  if ((pNearestPlayer->v.origin - v_dest).Length() > f_nearestdistance) // KWo - 07.04.2010
                     iMoveBack = -1;
                  else
                     v_dest = v_src + vecDirection * 32;
                  TRACE_LINE (v_src, v_dest, dont_ignore_monsters, pEdict, &tr2);
                  if (tr2.flFraction == 1.0f)
                  {
                     if (!IsDeadlyDropAtPos (pBot, v_dest))
                        pBot->f_move_speed = iMoveBack * pEdict->v.maxspeed;
                     v_dest = v_src + iMoveBack * vecDirection * 32;
                  }
                  else
                     v_dest = v_src;

                  if (iBotBypassPlayerColl > 0)
                  {
                     v_dest = v_dest + 32 * v_BotRightSide;
                     if (!IsDeadlyDropAtPos (pBot, v_dest))
                        pBot->f_sidemove_speed = pEdict->v.maxspeed;
                  }
                  else if (iBotBypassPlayerColl < 0)
                  {
                     v_dest = v_dest - 32 * v_BotRightSide;
                     if (!IsDeadlyDropAtPos (pBot, v_dest))
                        pBot->f_sidemove_speed = -pEdict->v.maxspeed;
                  }
               }
            }
            else
               pBot->bPlayerCollision = FALSE;


            if (g_b_DebugStuck)
            {
               ALERT (at_logged,"[DEBUG] BotCheckTerrain - Bot %s %s stuck with player - %s; dist = %i, mov dist = %0.1f. NoCollTime %s gl_time, TASK = %s.\n",
                     pBot->name, pBot->bPlayerCollision ? "is" : "is not", STRING (pNearestPlayer->v.netname), (int)f_nearestdistance, pBot->f_moved_distance,
                     (pBot->fNoCollTime <= gpGlobals->time) ? "<=" : ">", g_TaskNames[pBot->pTasks->iTask]);

/*
                  char szMessage[2048];
                  sprintf (szMessage,  "Bot %s %s stuck with player - %s;\n"
                    "dist = %i, mov dist = %0.1f. NoCollTime %s gl_time, TASK = %s.\n",
                    pBot->name, pBot->bPlayerCollision ? "is" : "is not", STRING (pNearestPlayer->v.netname), (int)f_nearestdistance, pBot->f_moved_distance,
                    (pBot->fNoCollTime <= gpGlobals->time) ? "<=" : ">", g_TaskNames[pBot->pTasks->iTask]);

                  g_hudset.x = 0.033;
                  g_hudset.y = 0.85;
                  g_hudset.holdTime = 1.055;
                  g_hudset.channel = 2;
                  UTIL_HudMessage(pHostEdict, g_hudset, szMessage);
                  UTIL_SplitHudMessage(szMessage);
*/
            }
         }
      }


      if (g_b_DebugStuck)
      {
         ALERT (at_logged,"[DEBUG] BotCheckTerrain - Bot %s moved distance = %i, dist to dest = %i, speed = %.0f, TASK = %s, timers %s.\n",
                  pBot->name, (int)pBot->f_moved_distance, (int)(pBot->pEdict->v.origin - pBot->dest_origin).Length(),
                  pBot->f_move_speed, g_TaskNames[iTask],
                  ((pBot->fNoCollTime < gpGlobals->time) && (pBot->f_wpt_tim_str_chg < gpGlobals->time)) ? "OK" : "NOT OK");

/*
         char szMessage[2048];
         sprintf (szMessage, "Bot %s mov dist = %i, to dest = %i, v = %.0f, TASK = %s, timers %s.\n",
                  pBot->name, (int)pBot->f_moved_distance, (int)(pBot->pEdict->v.origin - pBot->dest_origin).Length(),
                  pBot->f_move_speed, g_TaskNames[iTask],
                  ((pBot->fNoCollTime < gpGlobals->time) && (pBot->f_wpt_tim_str_chg < gpGlobals->time)) ? "OK" : "NOT OK");

         UTIL_SplitHudMessage(szMessage);

         g_hudset.x = 0.033;
         g_hudset.y = 0.70;
         g_hudset.channel = 2;
         UTIL_HudMessage(pHostEdict, g_hudset, szMessage);
*/
      }

      // No Player collision ?
      if (!bBypassPlayer
          && ((pBot->cCollisionState == COLLISION_PROBING)
             || (((g_i_botthink_index % 4) == (g_iFrameCounter % 4))
                && (((pBot->f_wpt_tim_str_chg < gpGlobals->time)
                && ((pBot->f_move_speed != 0.0f) || (pBot->f_sidemove_speed != 0.0f)
                   || ((pBot->bHitDoor) && (pBot->f_timeHitDoor + 3.5f < gpGlobals->time)))
                && (pBot->fNoCollTime < gpGlobals->time)
                && ((iTask != TASK_ATTACK) || (pBot->byFightStyle != 1)))
          || (pBot->bPlayerCollision))))) // KWo - 22.10.2011
      {
         if (pBot->cCollisionState == COLLISION_NOTDECIDED) // KWo - 23.03.2007
         {
            if (g_b_DebugStuck)
            {
               ALERT (at_logged,"[DEBUG] BotCheckTerrain - NOT_DECID - Bot %s moved distance = %i, dist to dest = %i, speed = %.0f, TASK = %s.\n",
                     pBot->name, (int)pBot->f_moved_distance, (int)(pBot->pEdict->v.origin - pBot->dest_origin).Length(),
                     pBot->f_move_speed, g_TaskNames[iTask]);
            }

         // Didn't we move enough previously ?
            if ((((pBot->f_moved_distance < ((pEdict->v.flags & FL_DUCKING)? 3.0f : 4.0f)) && ((pBot->pEdict->v.origin - pBot->dest_origin).Length() > 20.0f))
               || (pBot->bPlayerCollision))
                  && (iTask != TASK_CAMP)
                  && ((iTask != TASK_ATTACK) || (pBot->byFightStyle != 1) || (pBot->bPlayerCollision)
                        || ((pBot->bHitDoor) && (pBot->f_timeHitDoor + 3.5f < gpGlobals->time)))
                  && (pBot->f_UsageLiftTime == 0.0f)) // KWo - 17.01.2010
            {
               // Then consider being stuck
//               pBot->prev_time = gpGlobals->time;
               bBotIsStuck = TRUE;

               if (g_b_DebugStuck)
                  ALERT (at_logged,"[DEBUG] BotCheckTerrain - NOT_DECID - Bot %s moved distance = %i, dist to dest = %i, speed = %.0f, TASK = %s, time = %.2f.\n",
                     pBot->name, (int)pBot->f_moved_distance, (int)(pBot->pEdict->v.origin - pBot->dest_origin).Length(),
                     pBot->f_move_speed, g_TaskNames[iTask], gpGlobals->time);

               if (pBot->f_firstcollide_time == 0.0f)
                  pBot->f_firstcollide_time = gpGlobals->time + 0.2f;

               if (g_b_DebugStuck)
                  ALERT (at_logged,"[DEBUG] BotCheckTerrain - Bot %s din't move enough or is stuck with another player.\n",
                     pBot->name);
            }

            // Not stuck yet
            else if ((g_i_botthink_index % 4) == (g_iFrameCounter % 4)) // 24.10.2009
            {
               bool bOnLadderWPT = FALSE;

               if (bCurrWptIndexOK && bPrevWptIndexOK)
               {
                  if ((paths[pBot->curr_wpt_index]->flags & W_FL_LADDER) && (paths[pBot->prev_wpt_index[0]]->flags & W_FL_LADDER))
                     bOnLadderWPT = TRUE;
               }

               // Test if there's something ahead blocking the way
               if (BotCantMoveForward (pBot, vecDirection) && !pBot->bOnLadder && !bOnLadderWPT) // KWo - 13.07.2006
               {
                  if (g_b_DebugStuck)
                     ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s can't move forward, WP index = %i, time = %.2f.\n",
                        pBot->name, pBot->curr_wpt_index, gpGlobals->time);

                  if (pBot->f_firstcollide_time == 0.0f)
                     pBot->f_firstcollide_time = gpGlobals->time + 0.2;
                  else if (pBot->f_firstcollide_time <= gpGlobals->time)
                     bBotIsStuck = TRUE;
               }
               else
                  pBot->f_firstcollide_time = 0.0f;
            }
         }
         else if (pBot->bPlayerCollision || (pBot->f_probe_time >= gpGlobals->time)) // KWo - 27.02.2007
         {
            bBotIsStuck = TRUE;
         }
         else if ((pBot->f_probe_time + 3.0f < gpGlobals->time) && (pBot->f_probe_time != 0.0f)) // KWo - 27.02.2007
         {
            if (g_b_DebugStuck)
               ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s resets its collision state because it was too long time in probing state.\n", pBot->name);

            BotResetCollideState (pBot);
         }

         // Not stuck ?
         if (!bBotIsStuck)
         {
            if (g_b_DebugStuck)
               ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s is no longer stuck.\n", pBot->name);

            if (pBot->f_probe_time + 1.6f < gpGlobals->time)
            {
               if (g_b_DebugStuck)
                  ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s is no longer stuck and it resets its collision state.\n", pBot->name);

               // Reset Collision Memory if not being stuck for 1.6 secs
               BotResetCollideState(pBot);
            }
/*
            else
            {
               // Remember to keep pressing duck if it was necessary ago (KWo - ducktime should take care about it already...)
               if (pBot->cCollideMoves[(int) pBot->cCollStateIndex] == (char) COLLISION_DUCK)  // KWo - to remove warning
               {
                  pEdict->v.button |= IN_DUCK;
                  pBot->f_ducktime = gpGlobals->time + 1.0f;  // KWo - 13.10.2011
               }
            }
*/
         }

         // Bot is stuck !!
         else if ((g_i_botthink_index % 4) == (g_iFrameCounter % 4)) // 24.10.2009
         {
            // Not yet decided what to do ?

            if (g_b_DebugStuck)
            {
               ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s is stuck. Collision state = %s.\n",
                  pBot->name, g_CollideMoveNames[(int) pBot->cCollisionState]);
//             char szMessage[2048];
//             sprintf (szMessage,  "Bot %s is stuck. Collision state = %i.\n", pBot->name, pBot->cCollisionState);

//             UTIL_SplitHudMessage(szMessage);

//             g_hudset.x = 0.033;
//             g_hudset.y = 0.80;
//             g_hudset.channel = 4;
//             UTIL_HudMessage(pHostEdict, g_hudset, szMessage);
            }

            bool bBotHitDoor = false;     // KWo - 21.04.2006

            bool bBotHitTeamnate = false;
            bool bBotHitHostage = false;  // KWo - 18.07.2006
            bool bAlreadyBack = false;    // KWo - 14.08.2006

            Vector v_trace_start = pEdict->v.origin; // KWo - 16.07.2006
            Vector v_trace_end = pBot->dest_origin; // KWo - 16.07.2006
            TRACE_HULL (v_trace_start + pEdict->v.view_ofs, v_trace_end, dont_ignore_monsters, ((pEdict->v.flags & IN_DUCK) ? head_hull : human_hull), pEdict, &tr); // KWo - 16.07.2006

            if (pBot->cCollisionState == COLLISION_NOTDECIDED)
            {
               pBot->bBotNeedsObviateObstacle = false; // KWo - 13.01.2008
               if (!FNullEnt(tr.pHit)) // KWo - 18.07.2006
               {
                  if (FStrEq(STRING(tr.pHit->v.classname),"player"))
                  {

                     int iEntIndex = ENTINDEX(tr.pHit);
                     if ((iEntIndex > 0) && (iEntIndex <= gpGlobals->maxClients))
                     {
                        if (((clients[iEntIndex-1].iTeam == pBot->bot_team) || g_bIgnoreEnemies) && (!g_b_cv_ffa)) // KWo - 05.10.2006
                           bBotHitTeamnate = true;
                     }
                     if ((bBotHitTeamnate && ((tr.pHit->v.origin - pEdict->v.origin).Length() < 50.0f)) || pBot->bPlayerCollision) // KWO - 27.02.2007
                     {
                        pBot->bBotNeedsObviateObstacle = true;

                        if (g_b_DebugStuck && pBot->bPlayerCollision)
                           ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s needs bypass player %s.\n",
                              pBot->name, STRING(tr.pHit->v.netname));
                     }
                  }

                  else if ((FStrEq(STRING(tr.pHit->v.classname),"hostage_entity")) && (pBot->bot_team == TEAM_CS_TERRORIST)) // KWo - 08.01.2008
                  {
                     bBotHitHostage = true;
                     pBot->bBotNeedsObviateObstacle = true;
                     pBot->iCampButtons = 0; // KWo - 17.02.2008

                     if (g_b_DebugStuck)
                        ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s needs bypass a hostage.\n", pBot->name);
                  }
               }

               char cBits = 0;
               bool bTryJump = FALSE;

               if (pBot->bInWater)
               {
                  cBits |= PROBE_JUMP;
                  cBits |= PROBE_STRAFE;
               }
               else
               {
                  if (bCurrWptIndexOK)       // KWo - 21.04.2006
                  {
                     v_dest = paths[pBot->curr_wpt_index]->origin;
                     float f_dist_traced;    // KWo - 15.08.2006
                     TRACE_LINE (pEdict->v.origin, v_dest, dont_ignore_monsters, pEdict, &tr2); // KWo - 17.01.2008
//                     TRACE_HULL (pEdict->v.origin, v_dest, dont_ignore_monsters, head_hull, pEdict, &tr2);    // KWo - 23.05.2010
                     f_dist_traced = (tr2.flFraction) * (v_dest - pEdict->v.origin).Length();      // KWo - 15.08.2006
                     if (!FNullEnt(tr2.pHit))
                     {
                        if ((((tr2.flFraction < 1.0f) && (strncmp ("func_door", STRING (tr2.pHit->v.classname), 9) == 0) && (f_dist_traced < 20.0f)) || bBotHitDoor)
                           && (pBot->f_UsageLiftTime == 0) && (pBot->f_timeHitDoor + 3.5f < gpGlobals->time) )
                        {
                           bBotHitDoor = TRUE;
                           pBot->bHitDoor = TRUE;  // KWo - 24.10.2009
                           pBot->iCampButtons = 0; // KWo - 17.02.2008

                           if (g_b_DebugStuck)
                           {
                              ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s hits a door.\n", pBot->name);
/*
                              char szMessage[2048];
                              sprintf (szMessage,  "Bot %s hits a door.\n", pBot->name);

                              UTIL_SplitHudMessage(szMessage);

                              g_hudset.x = 0.033;
                              g_hudset.y = 0.80;
                              g_hudset.channel = 1;
                              UTIL_HudMessage(pHostEdict, g_hudset, szMessage);
*/
                           }

                           cBits |= PROBE_GOBACK;
                           if (bPrevWptIndexOK) // KWo - 14.08.2006
                           {
                              if ((pBot->pEdict->v.origin - paths[pBot->prev_wpt_index[0]]->origin).Length() < 15.0f)
                              {
                                 bBotHitDoor = FALSE;
                                 pBot->bHitDoor = FALSE;  // KWo - 24.10.2009
                                 bAlreadyBack = TRUE;
                                 cBits &= ~PROBE_GOBACK;
                                 pBot->f_timeHitDoor = gpGlobals->time - 3.0f;
                              }
                              else
                              {
                                 pBot->f_timeHitDoor = gpGlobals->time + 1.2f;
                                 cBits &= ~PROBE_JUMP;
                              }
                           }
                           else
                           {
                              pBot->f_timeHitDoor = gpGlobals->time + 1.2f;
                              cBits &= ~PROBE_JUMP;
                           }
                        }
                     }
                     else if ((pBot->f_moved_distance < 10.0f) && (!bBotHitHostage) && (tr2.flFraction >= 1.0f)
                        && !(pEdict->v.flags & FL_DUCKING)) // not blocked in the normal way, but maybe legs - needs overjump something?
                     {
                        v_center = pEdict->v.angles;
                        v_center.z = 0;
                        v_center.x = 0;
                        bTryJump = TRUE;

                        if (g_b_DebugStuck)
                           ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s checks if it can overjump an obstacle (1).\n",
                              pBot->name);

                        // first do a trace from the bot's eyes forward...
                        v_src = GetGunPosition (pEdict); // EyePosition ()
                        v_forward = v_src + vecDirection * 32;

                        MAKE_VECTORS (v_center);
                        // trace from the bot's eyes straight forward...
                        TRACE_LINE (v_src, v_forward, dont_ignore_monsters, pEdict, &tr); // KWo - 17.01.2008
                           // check if the trace hit something...
                        if (tr.flFraction < 1.0f)
                           bTryJump = false; // bot's body will hit something
                        else
                        {
                        // bot's head is clear, check at shoulder level...
                        // trace from the bot's shoulder left diagonal forward to the right shoulder... KWo - 08.01.2008 (changed -16 to - 18...)
                           v_src = GetGunPosition (pEdict) + Vector (0, 0, -18) + (-16) * gpGlobals->v_right;
                           v_forward = GetGunPosition (pEdict) + Vector (0, 0, -18) + (16) * gpGlobals->v_right + vecDirection * 32;

                           TRACE_LINE (v_src, v_forward, dont_ignore_monsters, pEdict, &tr); // KWo - 17.01.2008
                            // check if the trace hit something...
                           if (tr.flFraction < 1.0f)
                              bTryJump = false; // bot's body will hit something
                           else
                           {
                           // bot's head is clear, check at shoulder level...
                           // trace from the bot's shoulder right diagonal forward to the left shoulder... KWo - 08.01.2008 (changed -16 to - 18...)
                              v_src = GetGunPosition (pEdict) + Vector (0, 0, -18) + (16) * gpGlobals->v_right;
                              v_forward = GetGunPosition (pEdict) + Vector (0, 0, -18) + (-16) * gpGlobals->v_right  + vecDirection * 32;

                              TRACE_LINE (v_src, v_forward, dont_ignore_monsters, pEdict, &tr); // KWo - 17.01.2008
                              if (tr.flFraction < 1.0f)
                                 bTryJump = false; // bot's body will hit something

                              else
                              {
                              // Trace from the left Waist to the right forward Waist Pos
                                 v_src = pEdict->v.origin + Vector (0, 0, -33) + (-16) * gpGlobals->v_right;
                                 v_forward = pEdict->v.origin + Vector (0, 0, -16) + (16) * gpGlobals->v_right + vecDirection * 24;

                              // trace from the bot's waist straight forward...
                                 TRACE_LINE (v_src, v_forward, ignore_monsters, pEdict, &tr);

                              // check if the trace hit something...
                                 if (tr.flFraction >= 1.0f)
                                 {
                              // Trace from the right Waist to the left forward Waist Pos
                                    v_src = pEdict->v.origin + Vector (0, 0, -33) + (16) * gpGlobals->v_right;
                                    v_forward = pEdict->v.origin + Vector (0, 0, -16) + (-16) * gpGlobals->v_right + vecDirection * 24;

                              // check if the trace hit something...
                                    TRACE_LINE (v_src, v_forward, ignore_monsters, pEdict, &tr);
                                    if (tr.flFraction >= 1.0f)
                                       bTryJump = false;
                                 }
                              }

                              if (g_b_DebugStuck && bTryJump)
                                 ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s wants to overjump an obstacle (1).\n", pBot->name);
                           }
                        }
                     }
                     else if ((pBot->f_moved_distance < 3.0f) && (tr2.flFraction >= 1.0f) && (pEdict->v.flags & FL_DUCKING)) // not blocked in the normal way, but maybe legs - needs overjump something?
                     {
                        v_center = pEdict->v.angles;
                        v_center.z = 0;
                        v_center.x = 0;
                        bTryJump = TRUE;

                        if (g_b_DebugStuck)
                           ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s checks if it can overjump an obstacle(2).\n", pBot->name);

                        // first do a trace from the bot's eyes forward...
                        v_src = GetGunPosition (pEdict); // EyePosition ()
                        v_forward = v_src + vecDirection * 32;

                        MAKE_VECTORS (v_center);
                        // trace from the bot's eyes straight forward...
                        TRACE_LINE (v_src, v_forward, dont_ignore_monsters, pEdict, &tr); // KWo - 17.01.2008
                           // check if the trace hit something...
                        if (tr.flFraction < 1.0f)
                           bTryJump = false; // bot's body will hit something
                        else
                        {
                        // bot's head is clear, check at shoulder level...
                        // trace from the bot's shoulder left diagonal forward to the right shoulder...
                           v_src = GetGunPosition (pEdict) + Vector (0, 0, -16) + gpGlobals->v_right * -16;
                           v_forward = GetGunPosition (pEdict) + Vector (0, 0, -16) + gpGlobals->v_right * 16 + vecDirection * 32;

                           TRACE_LINE (v_src, v_forward, dont_ignore_monsters, pEdict, &tr); // KWo - 17.01.2008
                            // check if the trace hit something...
                           if (tr.flFraction < 1.0f)
                              bTryJump = false; // bot's body will hit something
                           else
                           {
                           // bot's head is clear, check at shoulder level...
                           // trace from the bot's shoulder right diagonal forward to the left shoulder...
                              v_src = GetGunPosition (pEdict) + Vector (0, 0, -16) + gpGlobals->v_right * 16;
                              v_forward = GetGunPosition (pEdict) + Vector (0, 0, -16) + gpGlobals->v_right * -16 + vecDirection * 32;

                              TRACE_LINE (v_src, v_forward, dont_ignore_monsters, pEdict, &tr); // KWo - 17.01.2008
                              if (tr.flFraction < 1.0f)
                                 bTryJump = false; // bot's body will hit something
                              else
                              {
                              // Trace from the left Waist to the right forward Waist Pos
                                 v_src = pEdict->v.origin + Vector (0, 0, -16) + (-16) * gpGlobals->v_right;
                                 v_forward = pEdict->v.origin + Vector (0, 0, -16) + (16) * gpGlobals->v_right + vecDirection * 32;

                              // trace from the bot's waist straight forward...
                                 TRACE_LINE (v_src, v_forward, dont_ignore_monsters, pEdict, &tr); // KWo - 17.01.2008

                              // check if the trace hit something...
                                 if (tr.flFraction >= 1.0f)
                                 {
                              // Trace from the right Waist to the left forward Waist Pos
                                    v_src = pEdict->v.origin + Vector (0, 0, -16) + (16) * gpGlobals->v_right;
                                    v_forward = pEdict->v.origin + Vector (0, 0, -16) + (-16) * gpGlobals->v_right + vecDirection * 32;

                              // check if the trace hit something...
                                    TRACE_LINE (v_src, v_forward, dont_ignore_monsters, pEdict, &tr); // KWo - 17.01.2008
                                    if (tr.flFraction >= 1.0f) // KWo - 27.05.2010
                                       bTryJump = false;
                                    else
                                       pBot->iCampButtons = 0; // KWo - 17.02.2008
                                 }
                              }
                           }
                        }
                     }
                  }
                  if (pBot->bPlayerCollision && (!BotCantMoveBack (pBot, vecDirection))) // KWo - 27.02.2007
                     cBits |= PROBE_GOBACK;

                  if ((((pBot->f_moved_distance < 2.0f) && ((!bBotHitHostage) && (pBot->bBotNeedsObviateObstacle)))
                         || (f_nearestdistance < 20)) && pBot->bIsLeader)
                  {
                     if (g_b_DebugStuck)
                        ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s is a leader and is too close to another player - wants to jump.\n", pBot->name);

                     bTryJump = TRUE;
                  }
                  if ((!bTryJump) && (bPrevWptIndexOK))  // KWo - 04.10.2009
                  {
                     for (i = 0; i < MAX_PATH_INDEX; i++)
                     {
                        if ((paths[pBot->prev_wpt_index[0]]->index[i] == pBot->curr_wpt_index)
                           && (paths[pBot->prev_wpt_index[0]]->connectflag[i] & C_FL_JUMP))
                        {
                           bTryJump = TRUE;
                           break;
                        }
                     }
                  }

                  if (bTryJump)
                  {
                     if (g_b_DebugStuck)
                        ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s wants to overjump an obstacle (2).\n", pBot->name);

                     cBits |= PROBE_JUMP;
                     cBits |= PROBE_DUCK;
                  }
                  else
                  {
                     TRACE_LINE (pBot->pEdict->v.origin, pBot->wpt_origin, dont_ignore_monsters, pBot->pEdict, &tr2); // KWo - 14.07.2006
                     float fRandomProbeJump = RANDOM_FLOAT (0.0f, 10.0f); // KWo - 26.06.2006 to prevent stupid jumping
                     if ((((fRandomProbeJump > 9.0f) || ((!bBotHitHostage) && (fRandomProbeJump > 9.0f)
                          && (BotCantMoveForward(pBot, vecDirection)) && (tr2.flFraction == 1.0f)))
                           || (pBot->dest_origin.z >= pEdict->v.origin.z + 18.0f))
                          && (!bBotHitDoor) && (pBot->f_timeHitDoor + 2.0f < gpGlobals->time)) // KWo - 26.06.2006
                     {
                        if ((pBot->f_moved_distance < 2.0f) && ((pBot->pEdict->v.origin - pBot->dest_origin).Length() > 15.0f)
                           && (pBot->f_UsageLiftTime == 0.0f)) // KWo - 08.10.2006
                        {
                           if (g_b_DebugStuck)
                              ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s wants to overjump an obstacle (3).\n", pBot->name);

                           cBits |= PROBE_JUMP;
                        }
                     }
                     if ((pBot->f_moved_distance < 2.0f) && (pBot->bBotNeedsObviateObstacle) && (f_nearestdistance < 30) && (fRandomProbeJump > 9.0f))
                     {
                        cBits |= PROBE_JUMP;

                        if (g_b_DebugStuck)
                           ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s is too close to another player, wants to jump...\n",
                              pBot->name);
                     }

                     cBits |= PROBE_DUCK;
                     cBits |= PROBE_STRAFE;
                     if ((!BotCantMoveBack(pBot, vecDirection)) && (!bBotHitHostage) && (!bAlreadyBack) && (pBot->f_timeHitDoor + 5.0f < gpGlobals->time)) // KWo - 15.01.2007
                        cBits |= PROBE_GOBACK;
                  }
               }

               if (g_b_DebugStuck)
                  ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s is stuck, cBits = %s %s %s %s.\n",
                     pBot->name, (cBits & PROBE_JUMP) ? "PROBE_JUMP,":"", (cBits & PROBE_DUCK) ? "PROBE_DUCK,":"",
                     (cBits & PROBE_STRAFE) ? "PROBE_STRAFE,":"", (cBits & PROBE_GOBACK) ? "PROBE_GOBACK":"");

               // Collision check allowed if not flying through the air
               if ((pEdict->v.flags & (FL_ONGROUND | FL_PARTIALGROUND)) || pBot->bPlayerCollision)  // KWo - 27.02.2007
               {
                  char cState[10];
                  i = 0;

                  // First 5 Entries hold the possible
                  // Collision States

                  if (cBits & PROBE_JUMP)  // KWo - 25.06.2006
                     cState[i] = COLLISION_JUMP;
                  else
                     cState[i] = COLLISION_NOMOVE;

                  i++;

                  if (cBits & PROBE_DUCK)  // KWo - 25.06.2006
                     cState[i] = COLLISION_DUCK;
                  else
                     cState[i] = COLLISION_NOMOVE;

                  i++;

                  if (cBits & PROBE_STRAFE)  // KWo - 25.06.2006
                     cState[i] = COLLISION_STRAFELEFT;
                  else
                     cState[i] = COLLISION_NOMOVE;

                  i++;

                  if (cBits & PROBE_STRAFE)  // KWo - 25.06.2006
                     cState[i] = COLLISION_STRAFERIGHT;
                  else
                     cState[i] = COLLISION_NOMOVE;

                  i++;
//                  cBits &= ~PROBE_GOBACK;       //  TEST !!!!
                  if (cBits & PROBE_GOBACK)  // KWo - 25.06.2006
                     cState[i] = COLLISION_GOBACK;
                  else
                     cState[i] = COLLISION_NOMOVE;

                  i++;

                  // Now weight all possible States
                  if (cBits & PROBE_JUMP)
                  {
                     cState[i] = 0;
                     TRACE_LINE (pBot->pEdict->v.origin, pBot->wpt_origin, ignore_monsters, pBot->pEdict, &tr2); // KWo - 14.07.2006
                     if (BotCanJumpUp (pBot, vecDirectionNormal))
                     {
                        pBot->iCampButtons = 0; // KWo - 17.02.2008
                        cState[i] += 10;
                     }
                     if ((pBot->dest_origin.z >= pEdict->v.origin.z + 18.0f) || ((!bBotHitHostage) && (tr2.flFraction == 1.0f) && BotCantMoveForward(pBot, vecDirection))
                        || (f_nearestdistance <= 35.0f)) // KWo - 26.02.2007
                        cState[i] += 30;
                     if (bTryJump)
                     {
                        cState[i] += 70;
                        pBot->iCampButtons = 0; // KWo - 17.02.2008
                     }

                     if (BotEntityIsVisible (pBot, pBot->dest_origin))
                     {
                        MAKE_VECTORS (vecMoveAngles);
                        v_src = GetGunPosition (pEdict);
                        v_src = v_src + gpGlobals->v_right * 15;
                        UTIL_TraceLine (v_src, pBot->dest_origin, ignore_monsters, ignore_glass, pEdict, &tr2);

                        if (tr2.flFraction >= 1.0f)
                        {
                           v_src = GetGunPosition (pEdict);
                           v_src = v_src + (-gpGlobals->v_right * 15);
                           UTIL_TraceLine (v_src, pBot->dest_origin, ignore_monsters, ignore_glass, pEdict, &tr2);

                           if (tr2.flFraction >= 1.0f)
                              cState[i] += 5;
                        }
                     }

                     if (pEdict->v.flags & FL_DUCKING)
                        v_src = pEdict->v.origin;
                     else
                        v_src = pEdict->v.origin + Vector (0, 0, -17);
                     v_dest = v_src + vecDirectionNormal * 30;
                     UTIL_TraceLine (v_src, v_dest, ignore_monsters, ignore_glass, pEdict, &tr2);

                     if (tr2.flFraction != 1.0f)
                        cState[i] += 10;
                  }
                  else
                     cState[i] = 0;

                  i++;

                  if (cBits & PROBE_DUCK)
                  {
                     cState[i] = 0;

                     if (BotCanDuckUnder (pBot, vecDirectionNormal))
                        cState[i] += 20;

                     if ((pBot->dest_origin.z + 36.0f <= pEdict->v.origin.z)
                         && BotEntityIsVisible (pBot, pBot->dest_origin))
                        cState[i] += 25;
                  }
                  else
                     cState[i] = 0;

                  i++;

                  if (cBits & PROBE_STRAFE)
                  {
                     cState[i] = 0;
                     cState[i + 1] = 0;
                     Vector2D vec2DirToPoint;
                     Vector2D vec2RightSide;
                     float fDotPr;

                     // to start strafing, we have to first figure out if the target is on the left side or right side
                     MAKE_VECTORS (vecMoveAngles);
                     if (bCurrWptIndexOK && bPrevWptIndexOK) // KWo - 15.07.2006
                        vec2DirToPoint = (paths[pBot->curr_wpt_index]->origin - paths[pBot->prev_wpt_index[0]]->origin).Make2D ();
                     else
                        vec2DirToPoint =  (pBot->dest_origin - pEdict->v.origin).Make2D ();
                     vec2DirToPoint = vec2DirToPoint.Normalize ();

                     vec2RightSide = gpGlobals->v_right.Make2D ();
                     vec2RightSide = vec2RightSide.Normalize ();

                     bool bDirRight = FALSE;
                     bool bDirLeft = FALSE;
                     bool bBlockedLeft = FALSE;
                     bool bBlockedRight = FALSE;
                     pBot->str_l_origin = g_vecZero;
                     pBot->str_r_origin = g_vecZero;

                     if (pBot->f_move_speed >= 0.0f)
                        vecDirection = gpGlobals->v_forward;
                     else
                        vecDirection = -gpGlobals->v_forward;

                     fDotPr = DotProduct  (vec2DirToPoint, vec2RightSide);

                     if (fDotPr < 0.0f)
                     {
                        if (!pBot->bBotNeedsObviateObstacle)
                           bDirRight = TRUE;
                        else
                           bDirLeft = TRUE;
                     }
                     else
                     {
                        if (!pBot->bBotNeedsObviateObstacle)
                           bDirLeft = TRUE;
                        else
                           bDirRight = TRUE;
                     }

                     // Now check which side is blocked
                     v_dest = pEdict->v.origin + gpGlobals->v_right * (pBot->bBotNeedsObviateObstacle ? 50 : (pEdict->v.flags & FL_DUCKING) ? 10 : 20);
                     v_src = pEdict->v.origin;

                     TRACE_LINE (v_src, v_dest, dont_ignore_monsters, pEdict, &tr2);  // KWo - 01.04.2010

                     if ((tr2.flFraction != 1.0f) || IsDeadlyDropAtPos (pBot, v_dest)) // KWo - 02.04.2010
                     {
                        bBlockedRight = TRUE;
                        cState[3] = COLLISION_NOMOVE;  // KWo - 25.06.2006
                     }
                     else
                        pBot->str_r_origin = v_dest;

                     v_dest = pEdict->v.origin + -gpGlobals->v_right * (pBot->bBotNeedsObviateObstacle ? 50 : (pEdict->v.flags & FL_DUCKING) ? 10 : 20);
                     v_src = pEdict->v.origin;

                     TRACE_LINE (v_src, v_dest, dont_ignore_monsters, pEdict, &tr2);  // KWo - 01.04.2010

                     if ((tr2.flFraction != 1.0f) || IsDeadlyDropAtPos (pBot, v_dest)) // KWo - 02.04.2010
                     {
                        bBlockedLeft = TRUE;
                        cState[2] = COLLISION_NOMOVE;  // KWo - 25.06.2006
                     }
                     else
                        pBot->str_l_origin = v_dest;

                     if (bDirLeft && bBlockedLeft && !bBlockedRight)
                     {
                        bDirLeft = false;
                        bDirRight = true;
                     }

                     if (bDirRight && bBlockedRight && !bBlockedLeft)
                     {
                        bDirRight = false;
                        bDirLeft = true;
                     }

                     if (bDirLeft && !bBlockedLeft)
                        cState[i] += 50;  // KWo - 21.04.2006

                     if (!bDirLeft)
                        cState[i] -= 5;

                     if (cState[i] < 0) // KWo - 27.02.2007
                         cState[2] = COLLISION_NOMOVE;

                     i++;

                     if (bDirRight && !bBlockedRight)
                        cState[i] += 50;  // KWo - 21.04.2006

                     if (!bDirRight)
                        cState[i] -= 5;

                     if (cState[i] < 0) // KWo - 27.02.2007
                         cState[3] = COLLISION_NOMOVE;

                  }
                  else
                  {
                     cState[i] = 0;
                     i++;
                     cState[i] = 0;
                  }

                  i++;                       // KWo - 25.06.2006

                  if (cBits & PROBE_GOBACK)  // KWo - 25.06.2006
                  {
                     cState[i] = 0;
/*
                     if (iTask == TASK_ATTACK) // KWo - 16.09.2008
                        v_direction = pBot->vecLookAt - pEdict->v.origin;
                     else
                        v_direction = pBot->dest_origin - pEdict->v.origin;

                     v_direction = v_direction.Normalize ();
                     v_direction.z = 0.0f;
*/
                     MAKE_VECTORS(pEdict->v.angles);
                     v_direction = gpGlobals->v_forward;
                     v_src = pEdict->v.origin;
                     v_dest = v_src - v_direction * 32;
                     TRACE_LINE (v_src, v_dest, dont_ignore_monsters, pEdict, &tr2);  // KWo - 01.04.2010

                     if ((tr2.flFraction != 1.0f) || IsDeadlyDropAtPos (pBot, v_dest)) // KWo - 02.04.2010
                     {
                        cState[i] -= 70;
                        cState[4] = COLLISION_NOMOVE;  // KWo - 25.06.2006
                     }
                     else
                     {
                        cState[i] += 40;
                        if ((bBotHitDoor) || (f_nearestdistance < 50))
                        {
                           cState[i] += 40;
                        }
                     }
                   }


                  // Weighted all possible Moves, now sort them to start with most probable
                  char cTemp;
                  bool bSorting;
                  do
                  {
                     bSorting = FALSE;
                     for (i = 0; i < 4; i++)  // KWo - 25.06.2006
                     {
                        if (cState[i + 5] < cState[i + 1 + 5])  // KWo - 25.06.2006
                        {
                           cTemp = cState[i];
                           cState[i] = cState[i + 1];
                           cState[i + 1] = cTemp;
                           cTemp = cState[i + 5];  // KWo - 25.06.2006
                           cState[i + 5] = cState[i + 1 + 5];  // KWo - 25.06.2006
                           cState[i + 1 + 5] = cTemp;  // KWo - 25.06.2006
                           bSorting = TRUE;
                        }
                     }
                  } while (bSorting);

                  for (i = 0; i < 5; i++)  // KWo - 25.06.2006
                  {
                     pBot->cCollideMoves[i] = cState[i];

                     if (g_b_DebugStuck)
                        ALERT (at_logged, "[DEBUG] BotCheckTerrain - Bot %s is stuck, Collide Move state = %s, weight = %i.\n",
                           pBot->name, g_CollideMoveNames[(int) pBot->cCollideMoves[i]], cState[i + 5]);
                  }

                  if ((pBot->cCollideMoves[0] == COLLISION_STRAFELEFT)
                     || (pBot->cCollideMoves[0] == COLLISION_STRAFERIGHT))
                  {
                     pBot->f_probe_time = gpGlobals->time + 1.6f;
                  }
                  else if (pBot->cCollideMoves[0] == COLLISION_GOBACK)
                     pBot->f_probe_time = gpGlobals->time + 0.8f;
                  else
                  {
                     pBot->f_probe_time = gpGlobals->time + 0.5f;
                  }
                  pBot->cCollisionProbeBits = cBits;
                  pBot->cCollisionState = COLLISION_PROBING;
                  pBot->cCollStateIndex = 0;
               }
            }
         } // KWo - 24.10.2009
         else if ((g_i_botthink_index % 4) != (g_iFrameCounter % 4)) // 24.10.2009
            bBotIsStuck = FALSE;

         if ((bBotIsStuck) || (pBot->cCollisionState == COLLISION_PROBING)) // KWo - 11.11.2009
         {
            if (g_b_DebugStuck)
            {
               ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s is stuck, cCollStateIndex = %d - %s, time %.2f.\n",
                  pBot->name, pBot->cCollStateIndex, g_CollideMoveNames[(int) pBot->cCollideMoves[(int) pBot->cCollStateIndex]], gpGlobals->time);
/*
               char szMessage[2048];
               sprintf (szMessage, "Bot %s is stuck, mov dist = %d, dist to dest = %d, cCollMov = %s, TASK = %s, time %.2f.\n",
                  pBot->name, (int)pBot->f_moved_distance, (int)(pBot->pEdict->v.origin - pBot->dest_origin).Length(),
                  g_CollideMoveNames[pBot->cCollideMoves[(int) pBot->cCollStateIndex]], g_TaskNames[iTask], gpGlobals->time);

               UTIL_SplitHudMessage(szMessage);

               g_hudset.x = -1;
               g_hudset.y = 0.75;
               g_hudset.channel = 1;
               g_hudset.holdTime = 0.1;
               UTIL_HudMessage(pHostEdict, g_hudset, szMessage);
*/
            }

            if (pBot->cCollisionState == COLLISION_PROBING)
            {
               if (pBot->f_probe_time < gpGlobals->time)
               {
                  pBot->cCollStateIndex++;

                  if (g_b_DebugStuck)
                     ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s is stuck, incrementing cCollStateIndex = %d - %s, speed = %d.\n",
                        pBot->name, pBot->cCollStateIndex, g_CollideMoveNames[(int) pBot->cCollideMoves[(int) pBot->cCollStateIndex]], (int)pBot->f_move_speed);

                  if (pBot->cCollStateIndex <= 5)  // KWo - 25.06.2006
                  {
                     if ((pBot->cCollideMoves[(int) pBot->cCollStateIndex] == COLLISION_STRAFELEFT)
                        || (pBot->cCollideMoves[(int) pBot->cCollStateIndex] == COLLISION_STRAFERIGHT))
                        pBot->f_probe_time = gpGlobals->time + 1.6f;
                     else if (pBot->cCollideMoves[(int) pBot->cCollStateIndex] == COLLISION_GOBACK)
                        pBot->f_probe_time = gpGlobals->time + 0.8f;
                     else if ((pBot->cCollideMoves[(int) pBot->cCollStateIndex] == COLLISION_JUMP)
                        || (pBot->cCollideMoves[(int) pBot->cCollStateIndex] == COLLISION_DUCK))
                        pBot->f_probe_time = gpGlobals->time + 0.5f;
                  }

                  if (pBot->cCollStateIndex > 5)  // KWo - 25.06.2006
                  {
                     BotResetCollideState (pBot);
                  }
               }
               if (pBot->cCollStateIndex <= 5)  // KWo - 25.06.2006
               {
                  if (g_b_DebugStuck)
                  {
                     ALERT (at_logged, "[DEBUG] BotCheckTerrain - Bot %s is stuck, Collide Move = %s.\n",
                        pBot->name, g_CollideMoveNames[(int) pBot->cCollideMoves[(int) pBot->cCollStateIndex]]);
/*
                     char szMessage[2048];
                     sprintf (szMessage,  "Bot %s is stuck. Collision Move = %s.\n", pBot->name, g_CollideMoveNames[pBot->cCollideMoves[(int) pBot->cCollStateIndex]]);

                     UTIL_SplitHudMessage(szMessage);

                     g_hudset.x = 0.033f;
                     g_hudset.y = 0.75f;
                     g_hudset.channel = 3;
                     UTIL_HudMessage(pHostEdict, g_hudset, szMessage);
*/
                  }

                  switch (pBot->cCollideMoves[(int) pBot->cCollStateIndex])    // KWo - to remove warning
                  {
                     case COLLISION_JUMP:
                     {
                        if (((pEdict->v.flags & FL_ONGROUND) || pBot->bInWater)
                            && !pBot->bHitDoor && (!pBot->bOnLadder) && (pBot->f_timeHitDoor < gpGlobals->time)) // KWo - 17.04.2016
                        {
                           if (g_b_DebugStuck)
                              ALERT (at_logged, "[DEBUG] BotCheckTerrain - Bot %s is stuck, trying to jump; time %f.\n", pBot->name, gpGlobals->time);

                           pEdict->v.button |= IN_JUMP;
                           pBot->f_jumptime = gpGlobals->time + 1.0f;
                           pBot->f_move_speed = pEdict->v.maxspeed;
                        }
                        break;
                     }
                     case COLLISION_DUCK:
                     {
                        if (g_b_DebugStuck)
                           ALERT (at_logged, "[DEBUG] BotCheckTerrain - Bot %s is stuck, trying to duck; time %f.\n", pBot->name, gpGlobals->time);
                        pBot->f_ducktime = gpGlobals->time + 1.0f; // KWo - 17.02.2008
                        pEdict->v.button |= IN_DUCK;
                        break;
                     }
                     case COLLISION_STRAFELEFT:
                     {
                        if (g_b_DebugStuck)
                           ALERT (at_logged, "[DEBUG] BotCheckTerrain - Bot %s is stuck, trying to strafe left; time %f.\n", pBot->name, gpGlobals->time);

                        float fDot3;  // KWo - 09.01.2008
                        float fSideSpeed;  // KWo - 09.01.2008
                        float fStrafeDistance;  // KWo - 09.01.2008

                        if (!pBot->bBotNeedsObviateObstacle)
                        {
                           if (g_b_DebugStuck)
                              ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s strafes left - doesn't need to bypass an obstacle.\n", pBot->name);

                           if (bPrevWptIndexOK && bCurrWptIndexOK)   // KWo - 14.01.2008
                           {
                              if ((pBot->f_timeHitDoor + 3.0f > gpGlobals->time) || (pBot->f_timeHitDoor < gpGlobals->time)) // KWo - 17.09.2006
                              {
                                 Vector2D vec2DirToPrevWP;
                                 Vector2D Vector2DirFromPoint;
                                 Vector vecPosCentr;
                                 vecPosCentr = (paths[pBot->prev_wpt_index[0]]->origin - paths[pBot->curr_wpt_index]->origin)/3.f + paths[pBot->curr_wpt_index]->origin;
                                 Vector2DirFromPoint = (pEdict->v.origin - vecPosCentr).Make2D ();
                                 Vector2DirFromPoint = Vector2DirFromPoint.Normalize ();
                                 vec2DirToPrevWP = (paths[pBot->prev_wpt_index[0]]->origin - vecPosCentr).Make2D ();
                                 vec2DirToPrevWP = vec2DirToPrevWP.Normalize ();

                                 fDot3 = fabs(DotProduct  (Vector2DirFromPoint, vec2DirToPrevWP));  // KWo - 09.01.2008

                                 if (g_b_DebugStuck)
                                    ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s is trying to strafe left; DotProduct = %f.\n", pBot->name, fDot3);

                                 if (fDot3  < 0.99f)
                                 {
                                    fSideSpeed = (pEdict->v.flags & FL_DUCKING) ? (pEdict->v.maxspeed) : (pEdict->v.maxspeed / 2.0f);          // KWo - 09.01.2008
                                    pBot->f_move_speed = 0.0f;
                                    BotSetStrafeSpeed (pBot, -fSideSpeed);          // KWo - 09.01.2008

                                    if (g_b_DebugStuck)
                                       ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s strafes left (1).\n", pBot->name);
                                 }
                                 else if (fDot3  > 0.992f)
                                    pBot->f_probe_time = gpGlobals->time;
                              }
                              else if (pBot->str_l_origin != g_vecZero)
                              {
                                 fStrafeDistance = (pBot->str_l_origin - pBot->pEdict->v.origin).Length(); // KWo - 09.01.2008
                                 if (fStrafeDistance > 10.0f) // KWo - 09.01.2008
                                 {
                                    fSideSpeed = (pEdict->v.flags & FL_DUCKING) ? (pEdict->v.maxspeed) : ((fStrafeDistance / 50.0f) * pEdict->v.maxspeed);    // KWo - 09.01.2008
                                    if (fSideSpeed > pEdict->v.maxspeed / 2.0f)                     // KWo - 09.01.2008
                                       fSideSpeed = (pEdict->v.flags & FL_DUCKING) ? (pEdict->v.maxspeed) : (pEdict->v.maxspeed / 2.0f);
                                    BotSetStrafeSpeed (pBot, -fSideSpeed);                         // KWo - 09.01.2008

                                    if (g_b_DebugStuck)
                                       ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s strafes left (2).\n", pBot->name);
                                 }
                                 else                                                              // KWo - 13.01.2008
                                    pBot->f_probe_time = gpGlobals->time;
                              }
                              else
                                 pBot->f_probe_time = gpGlobals->time;
                           }
                           else if (pBot->str_l_origin != g_vecZero)
                           {
                              fStrafeDistance = (pBot->str_l_origin - pBot->pEdict->v.origin).Length();  // KWo - 09.01.2008
//                              pBot->dest_origin = pBot->str_l_origin;
                              if (fStrafeDistance > 10.0f) // KWo - 09.01.2008
                              {
                                 fSideSpeed = (pEdict->v.flags & FL_DUCKING) ? (pEdict->v.maxspeed) : ((fStrafeDistance / 50.0f) * pEdict->v.maxspeed);             // KWo - 09.01.2008
                                 if (fSideSpeed > pEdict->v.maxspeed / 2.0f)                              // KWo - 09.01.2008
                                    fSideSpeed = (pEdict->v.flags & FL_DUCKING) ? (pEdict->v.maxspeed) : (pEdict->v.maxspeed / 2.0f);
                                 BotSetStrafeSpeed (pBot, -fSideSpeed);                                  // KWo - 09.01.2008

                                 if (g_b_DebugStuck)
                                    ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s strafes left (3).\n", pBot->name);
                              }
                              else                                                                       // KWo - 13.01.2008
                                 pBot->f_probe_time = gpGlobals->time;
                           }
                           else
                              pBot->f_probe_time = gpGlobals->time;
                        }
                        else if (pBot->str_l_origin != g_vecZero)
                        {
                           if (g_b_DebugStuck)
                              ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s strafes left - needs to bypass an obstacle.\n", pBot->name);

//                           if (pBot->bPlayerCollision) // KWo - 27.02.2007
                           fStrafeDistance = (pBot->str_l_origin - pBot->pEdict->v.origin).Length();  // KWo - 09.01.2008
//                           pBot->dest_origin = pBot->str_l_origin;
                           pBot->f_move_speed = 0.0f;
                           if (fStrafeDistance > 10.0f)                                                // KWo - 09.01.2008
                           {
                              fSideSpeed = (pEdict->v.flags & FL_DUCKING) ? (pEdict->v.maxspeed) : ((fStrafeDistance / 50.0f) * pEdict->v.maxspeed);             // KWo - 09.01.2008
                              if (fSideSpeed > pEdict->v.maxspeed / 2.0f)                              // KWo - 09.01.2008
                                 fSideSpeed = (pEdict->v.flags & FL_DUCKING) ? (pEdict->v.maxspeed) : (pEdict->v.maxspeed / 2.0f);
                              BotSetStrafeSpeed (pBot, -fSideSpeed);                                  // KWo - 09.01.2008

                              if (g_b_DebugStuck)
                                 ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s strafes left (4).\n", pBot->name);
                           }
                           else                                                                       // KWo - 13.01.2008
                              pBot->f_probe_time = gpGlobals->time;
                        }
                        else
                           pBot->f_probe_time = gpGlobals->time;
                        break;
                     }
                     case COLLISION_STRAFERIGHT:
                     {
                        if (g_b_DebugStuck)
                           ALERT (at_logged, "[DEBUG] BotCheckTerrain - Bot %s is stuck, trying to strafe right; time %f.\n", pBot->name, gpGlobals->time);

                        float fDot3;  // KWo - 09.01.2008
                        float fSideSpeed;  // KWo - 09.01.2008
                        float fStrafeDistance;  // KWo - 09.01.2008

                        if (!pBot->bBotNeedsObviateObstacle)
                        {
                           if (g_b_DebugStuck)
                              ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s strafes right - doesn't need to bypass an obstacle.\n", pBot->name);

                           if (bPrevWptIndexOK && bCurrWptIndexOK)   // KWo - 14.01.2008
                           {
                              if ((pBot->f_timeHitDoor + 3.0f > gpGlobals->time) || (pBot->f_timeHitDoor < gpGlobals->time)) // KWo - 17.09.2006
                              {
                                 Vector2D vec2DirToPrevWP;
                                 Vector2D Vector2DirFromPoint;
                                 Vector vecPosCentr;
                                 vecPosCentr = (paths[pBot->prev_wpt_index[0]]->origin - paths[pBot->curr_wpt_index]->origin)/3.f + paths[pBot->curr_wpt_index]->origin;
                                 Vector2DirFromPoint = (pEdict->v.origin - vecPosCentr).Make2D ();
                                 Vector2DirFromPoint = Vector2DirFromPoint.Normalize ();
                                 vec2DirToPrevWP = (paths[pBot->prev_wpt_index[0]]->origin - vecPosCentr).Make2D ();
                                 vec2DirToPrevWP = vec2DirToPrevWP.Normalize ();

                                 fDot3 = fabs(DotProduct  (Vector2DirFromPoint, vec2DirToPrevWP));  // KWo - 09.01.2008

                                 if (g_b_DebugStuck)
                                    ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s is trying to strafe right; DotProduct = %f.\n", pBot->name, fDot3);

                                 if (fDot3 < 0.99)
                                 {
                                    fSideSpeed = (pEdict->v.flags & FL_DUCKING) ? (pEdict->v.maxspeed) : (pEdict->v.maxspeed / 2.0f);          // KWo - 09.01.2008
                                    BotSetStrafeSpeed (pBot, fSideSpeed);           // KWo - 09.01.2008
                                    pBot->f_move_speed = 0.0f;

                                    if (g_b_DebugStuck)
                                       ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s strafes right (1).\n", pBot->name);
                                 }
                                 else if (fDot3  > 0.992f)
                                    pBot->f_probe_time = gpGlobals->time;
                              }
                              else if (pBot->str_r_origin != g_vecZero)
                              {
                                 fStrafeDistance = (pBot->str_r_origin - pBot->pEdict->v.origin).Length(); // KWo - 09.01.2008
                                 if (fStrafeDistance > 10.0f) // KWo - 09.01.2008
                                 {
                                    fSideSpeed = (pEdict->v.flags & FL_DUCKING) ? (pEdict->v.maxspeed) : ((fStrafeDistance / 50.0f) * pEdict->v.maxspeed);    // KWo - 09.01.2008
                                    if (fSideSpeed > pEdict->v.maxspeed / 2.0f)                     // KWo - 09.01.2008
                                       fSideSpeed = (pEdict->v.flags & FL_DUCKING) ? (pEdict->v.maxspeed) : (pEdict->v.maxspeed / 2.0f);
                                    BotSetStrafeSpeed (pBot, fSideSpeed);                          // KWo - 09.01.2008

                                    if (g_b_DebugStuck)
                                       ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s strafes right (2).\n", pBot->name);
                                 }
                                 else                                                              // KWo - 13.01.2008
                                    pBot->f_probe_time = gpGlobals->time;
                              }
                              else
                                 pBot->f_probe_time = gpGlobals->time;
                           }
                           else if (pBot->str_r_origin != g_vecZero)
                           {
                              fStrafeDistance = (pBot->str_r_origin - pBot->pEdict->v.origin).Length(); // KWo - 09.01.2008
//                              pBot->dest_origin = pBot->str_r_origin;
                              if (fStrafeDistance > 10.0f)                                               // KWo - 09.01.2008
                              {
                                 fSideSpeed = (pEdict->v.flags & FL_DUCKING) ? (pEdict->v.maxspeed) : ((fStrafeDistance / 50.0f) * pEdict->v.maxspeed);            // KWo - 09.01.2008
                                 if (fSideSpeed > pEdict->v.maxspeed / 2.0f)                             // KWo - 09.01.2008
                                    fSideSpeed = (pEdict->v.flags & FL_DUCKING) ? (pEdict->v.maxspeed) : (pEdict->v.maxspeed / 2.0f);
                                 BotSetStrafeSpeed (pBot, fSideSpeed);                                  // KWo - 09.01.2008

                                 if (g_b_DebugStuck)
                                    ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s strafes right (3).\n", pBot->name);
                              }
                              else                                                                      // KWo - 13.01.2008
                                 pBot->f_probe_time = gpGlobals->time;
                           }
                           else
                              pBot->f_probe_time = gpGlobals->time;
                        }
                        else if (pBot->str_r_origin != g_vecZero)
                        {
                           if (g_b_DebugStuck)
                              ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s strafes right - needs to bypass an obstacle.\n", pBot->name);
//                           if (pBot->bPlayerCollision) // KWo - 27.02.2007
                           fStrafeDistance = (pBot->str_r_origin - pBot->pEdict->v.origin).Length();  // KWo - 09.01.2008
//                           pBot->dest_origin = pBot->str_r_origin;
                           pBot->f_move_speed = 0.0f;
                           if (fStrafeDistance > 10.0f)                                                // KWo - 09.01.2008
                           {
                              fSideSpeed = (pEdict->v.flags & FL_DUCKING) ? (pEdict->v.maxspeed) : ((fStrafeDistance / 50.0f) * pEdict->v.maxspeed);             // KWo - 09.01.2008
                              if (fSideSpeed > pEdict->v.maxspeed / 2.0f)                              // KWo - 09.01.2008
                                 fSideSpeed = (pEdict->v.flags & FL_DUCKING) ? (pEdict->v.maxspeed) : (pEdict->v.maxspeed / 2.0f);
                              BotSetStrafeSpeed (pBot, fSideSpeed);                                   // KWo - 09.01.2008

                              if (g_b_DebugStuck)
                                 ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s strafes right (4).\n", pBot->name);
                           }
                           else
                              pBot->f_probe_time = gpGlobals->time;
                        }
                        else
                           pBot->f_probe_time = gpGlobals->time;
                        break;
                     }
                     case COLLISION_GOBACK:
                     {
                        if (g_b_DebugStuck)
                           ALERT (at_logged, "[DEBUG] BotCheckTerrain - Bot %s is stuck, trying to go back; time %f.\n", pBot->name, gpGlobals->time);

                        if ((pBot->f_timeHitDoor > gpGlobals->time) && (pBot->prev_wpt_index[0] >= 0) && (pBot->prev_wpt_index[0] < g_iNumWaypoints))  // 14.07.2006
                        {
                           pBot->dest_origin = paths[pBot->prev_wpt_index[0]]->origin;

                           if (g_b_DebugStuck)
                              ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s goes back (1).\n", pBot->name);

                           if ((pEdict->v.origin - pBot->dest_origin).Length() < 15.0f) // KWo - 14.08.2006
                           {
                              pBot->f_timeHitDoor = gpGlobals->time - 1.0f;
                              pBot->f_probe_time = gpGlobals->time - 1.0f;
                           }
                        }
                        else
                        {
                           Vector vecRight;
                           Vector vecForward;
                           Vector vecDestination = g_vecZero;
                           MAKE_VECTORS (pEdict->v.angles);
                           vecForward = gpGlobals->v_forward;
                           vecDestination = pEdict->v.origin + (-40.0f) * vecForward; // KWo - 07.04.2010

                           if (!IsDeadlyDropAtPos (pBot, vecDestination))
                           {
                              pBot->f_move_speed = -pEdict->v.maxspeed;

                              if (g_b_DebugStuck)
                                 ALERT(at_logged, "[DEBUG] BotCheckTerrain - Bot %s goes back (2).\n", pBot->name);
                           }

                           pBot->f_sidemove_speed = 0.0f;
                        }
                        break;
                     }
                  }
               }
            }
         }
      }
   }
   return;
}

// no idea what is this for...?
STRINGNODE *GetNodeSTRING (STRINGNODE *pNode, int NodeNum)
{
   STRINGNODE *pTempNode = pNode;
   int i = 0;

   while (i < NodeNum)
   {
      pTempNode = pTempNode->Next;
      assert (pTempNode != NULL);

      if (pTempNode == NULL)
         break;
      i++;
   }

   return (pTempNode);
}
