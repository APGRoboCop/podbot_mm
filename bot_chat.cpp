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
// bot_chat.cpp
//
// Contains parsing stuff & chat selection for chatting Bots

#include "bot_globals.h"


inline void StripClanTags (char *pszTemp1, char *pszReturn, char *cTag1, char *cTag2)
{
   // Strips out Words between Chars like []

   unsigned char ucLen = (unsigned char)strlen (pszTemp1);
   char* pszEndPattern;
   char* pszStartPattern = strstr (pszTemp1, cTag1);

   if (pszStartPattern)
   {
      pszStartPattern++;

      if (*pszStartPattern != 0)
      {
         pszEndPattern = strstr (pszTemp1, cTag2);

         if (pszEndPattern)
         {
            if (pszEndPattern - pszStartPattern < ucLen)
            {
               if (pszStartPattern - 1 != pszTemp1)
                  strncpy (pszReturn,pszTemp1, (pszStartPattern - pszTemp1) - 1);

               if (pszEndPattern < pszTemp1 + ucLen)
                  strcat (pszReturn,pszEndPattern + 1);
            }
         }
      }
   }
   else
      strcpy (pszReturn, pszTemp1);
}


void ConvertNameToHuman (char *pszName, char *pszReturn)
{
   // Converts given Names to a more human like style for output

   char szTemp1[80];
   char szTemp2[80];
   char* pszLast;

//   unsigned char ucLen = 1;
   unsigned char ucActLen = 1;

   memset (szTemp1, 0, sizeof (szTemp1));
   memset (szTemp2, 0, sizeof (szTemp2));
   memset (pszReturn, 0, sizeof (pszReturn));

   StripClanTags (pszName, szTemp1, "[", "]");
   StripClanTags (szTemp1, szTemp2, "(", ")");
   memset (szTemp1, 0, sizeof (szTemp1));
   StripClanTags (szTemp2, szTemp1, "{", "}");

   // strip out all spaces at the end of the name...
   ucActLen = (unsigned char)strlen (szTemp1);
   pszLast = szTemp1 + ucActLen - 1;
   while (ucActLen > 0)
   {
      if (szTemp1[ucActLen - 1] == ' ')
      {
         szTemp1[ucActLen - 1] = '\0';
         ucActLen = (unsigned char)strlen (szTemp1);

         if (g_b_DebugChat)
            ALERT (at_logged, "[DEBUG] ConvertNameToHuman - There was a space in the name %s.\n", szTemp2);
      }
      else
         ucActLen = 0;
   }
   strcpy (pszReturn, szTemp1);

   if (g_b_DebugChat)
      ALERT (at_logged, "[DEBUG] ConvertNameToHuman(3) - Player's name %s.\n", pszReturn);
}


void BotPrepareChatMessage (bot_t *pBot, char *pszText)
{
   // Parses Messages from the Botchat, replaces Keywords
   // and converts Names into a more human style

   int iLen;
   char szNamePlaceholder[80];
   int iIndex = 0;
   int i;

   memset (&pBot->szMiscStrings, 0, sizeof (pBot->szMiscStrings));
   memset (szNamePlaceholder, 0, sizeof (szNamePlaceholder));

   char *pszTextStart = pszText;
   char *pszPattern = pszText;
   edict_t *pTalkEdict = NULL;

   while (pszPattern)
   {
      // all replacement placeholders start with a %
      pszPattern = strstr (pszTextStart, "%");

      if (pszPattern)
      {
         iLen = pszPattern - pszTextStart;
         if (iLen > 0)
            strncpy (pBot->szMiscStrings, pszTextStart, iLen);
         pszPattern++;

         // Player with most frags ?
         if (*pszPattern == 'f')
         {
            int iHighestFrags = -9000; // just pick some start value
            int iCurrFrags;

            for (i = 0; i < gpGlobals->maxClients; i++)
            {
               if (!(clients[i].iFlags & CLIENT_USED)
                   || (clients[i].pEdict == pBot->pEdict))
                  continue;

               iCurrFrags = (int) clients[i].pEdict->v.frags;	// KWo - to remove warning

               if (iCurrFrags > iHighestFrags)
               {
                  iHighestFrags = iCurrFrags;
                  iIndex = i;
               }
            }

            // fix fix fix, all day long...
            if (iIndex < gpGlobals->maxClients)
            {
               pTalkEdict = clients[iIndex].pEdict;

               if (!FNullEnt (pTalkEdict))
               {
                  ConvertNameToHuman ((char *) STRING (pTalkEdict->v.netname), szNamePlaceholder);
                  strcat (pBot->szMiscStrings, szNamePlaceholder);
               }
            }
         }

         // Mapname ?
         else if (*pszPattern == 'm')
            strcat (pBot->szMiscStrings, STRING (gpGlobals->mapname));

         // Roundtime ?
         else if (*pszPattern == 'r')
         {
            char szTime[] = "000:00";
            int iTime = (int) (g_fTimeRoundEnd - gpGlobals->time);

            sprintf (szTime, "%02d:%02d", iTime / 60, iTime % 60);
            strcat (pBot->szMiscStrings, szTime);
         }

         // Chat Reply ?
         else if (*pszPattern == 's') // KWo - 09.03.2010 - rewritten...
         {
            // crash fixes, crash fixes...
            iIndex = pBot->SaytextBuffer.iEntityIndex;
            i = 0;
            if ((iIndex < 1) || (iIndex > gpGlobals->maxClients))
            {
               iIndex = (int)RANDOM_LONG(1,gpGlobals->maxClients);
               while (((iIndex == g_i_botthink_index) || !(clients[iIndex - 1].iFlags & CLIENT_USED))
                  && (i < 64))
               {
                  iIndex = (int)RANDOM_LONG(1,gpGlobals->maxClients);
                  i++;
               }
            }
            if (((iIndex == g_i_botthink_index) || !(clients[iIndex - 1].iFlags & CLIENT_USED)) && (i == 0))
            {
               i = 0;
               while (((iIndex == g_i_botthink_index) || !(clients[iIndex - 1].iFlags & CLIENT_USED))
                  && (i < 64))
               {
                  iIndex = (int)RANDOM_LONG(1,gpGlobals->maxClients);
                  i++;
               }
            }
            pBot->SaytextBuffer.iEntityIndex = iIndex;

            if ((pBot->SaytextBuffer.iEntityIndex > 0)
                && (pBot->SaytextBuffer.iEntityIndex <= gpGlobals->maxClients))
            {
               pTalkEdict = INDEXENT (pBot->SaytextBuffer.iEntityIndex);

               if (!FNullEnt (pTalkEdict))
               {
                  ConvertNameToHuman ((char *) STRING (pTalkEdict->v.netname), szNamePlaceholder);

                  if (g_b_DebugChat)
                     ALERT (at_logged, "[DEBUG] BotPrepareChatMessage(2) - Player's name %s.\n", szNamePlaceholder);

                  strcat (pBot->szMiscStrings, szNamePlaceholder);
               }
            }
         }

         // Teammate alive ?
         else if (*pszPattern == 't')
         {
            for (i = 0; i < gpGlobals->maxClients; i++)
            {
               if (!(clients[i].iFlags & CLIENT_USED)
                   || !(clients[i].iFlags & CLIENT_ALIVE)
                   || (clients[i].iTeam != pBot->bot_team)
                   || (clients[i].pEdict == pBot->pEdict))
                  continue;

               break;
            }

            if (i < gpGlobals->maxClients)
            {
               pTalkEdict = clients[i].pEdict;

               if (!FNullEnt (pTalkEdict))
               {
                  ConvertNameToHuman ((char *) STRING (pTalkEdict->v.netname), szNamePlaceholder);
                  strcat (pBot->szMiscStrings, szNamePlaceholder);
               }
            }
         }

         else if (*pszPattern == 'v')
         {
            pTalkEdict = pBot->pLastVictim;

            if (!FNullEnt (pTalkEdict))
            {
               ConvertNameToHuman ((char *) STRING (pTalkEdict->v.netname), szNamePlaceholder);
               strcat (pBot->szMiscStrings, szNamePlaceholder);
            }
         }

         pszPattern++;
         pszTextStart = pszPattern;
      }
   }

   strcat (pBot->szMiscStrings, pszTextStart);

   // removes trailing '\n'
   iLen = (int)strlen (pBot->szMiscStrings);
   while ((pBot->szMiscStrings[iLen - 1] == '\n' || pBot->szMiscStrings[iLen - 1] == ' '
      || pBot->szMiscStrings[iLen - 1] == '\r' || pBot->szMiscStrings[iLen - 1] == '\t') && (iLen > 0))
      pBot->szMiscStrings[--iLen] = 0;

   if (g_b_DebugChat)
      ALERT(at_logged, "[DEBUG] BotPrepareChatMessage - Bot %s prepared the message: %s.\n", pBot->name, pBot->szMiscStrings);

   return;
}


bool BotCheckKeywords(bot_t *pBot, char *pszMessage, char *pszReply)
{
   replynode_t *pReply = pChatReplies;
   char szKeyword[128];
   char *pszCurrKeyword;
   char *pszKeywordEnd;
   int iLen, iRandom;
   char cNumRetries;

   while (pReply != NULL)
   {
      pszCurrKeyword = (char *) &pReply->szKeywords;

      while (pszCurrKeyword)
      {
         pszKeywordEnd = strstr (pszCurrKeyword, "@");

         if (pszKeywordEnd)
         {
            iLen = pszKeywordEnd - pszCurrKeyword;
            strncpy (szKeyword, pszCurrKeyword, iLen);

            szKeyword[iLen] = 0x0;

            // Parse Text for occurences of keywords
            char *pPattern = strstr (pszMessage, szKeyword);

            if (pPattern)
            {
               STRINGNODE *pNode = pReply->pReplies;

               if (pReply->cNumReplies == 1)
                  strcpy (pszReply, pNode->szString);
               else
               {
                  cNumRetries = 0;
                  do
                  {
                     iRandom = RANDOM_LONG (1, pReply->cNumReplies);
                     cNumRetries++;
                  } while ((cNumRetries <= 10 /* pReply->cNumReplies */)
                     && ((iRandom == pReply->cLastReply[0]) || (iRandom == pReply->cLastReply[1])
                         || (iRandom == pReply->cLastReply[2]) || (iRandom == pReply->cLastReply[3]))); // KWo - 27.03.2010

                  pReply->cLastReply[3] = pReply->cLastReply[2]; // KWo - 27.03.2010
                  pReply->cLastReply[2] = pReply->cLastReply[1]; // KWo - 27.03.2010
                  pReply->cLastReply[1] = pReply->cLastReply[0]; // KWo - 27.03.2010
                  pReply->cLastReply[0] = iRandom;
                  cNumRetries = 1;

                  while (cNumRetries < iRandom)
                  {
                     pNode = pNode->Next;
                     cNumRetries++;
                  }

                  strcpy (pszReply, pNode->szString);
               }

               if (g_b_DebugChat)
                  ALERT(at_logged, "[DEBUG] BotCheckKeywords - Bot %s found a keyword: %s.\n", pBot->name, szKeyword);

               return (TRUE);
            }

            pszKeywordEnd++;

            if (*pszKeywordEnd == 0)
               pszKeywordEnd = NULL;
         }

         pszCurrKeyword = pszKeywordEnd;
      }

      pReply = pReply->pNextReplyNode;
   }

   // Didn't find a keyword ? // 50% of the time use some universal reply
   if (RANDOM_LONG (1, 100) < 50)
   {
      iRandom = RANDOM_LONG (0, iNumNoKwChats - 1);
      cNumRetries = 0;
      while ((cNumRetries < 5) 
         && ((iUsedUnknownChatIndex[0] == iRandom)
            || (iUsedUnknownChatIndex[1] == iRandom)
            || (iUsedUnknownChatIndex[2] == iRandom)
            || (iUsedUnknownChatIndex[3] == iRandom)
            || (iUsedUnknownChatIndex[4] == iRandom))) // KWo - 27.03.2010
      {
         iRandom = RANDOM_LONG (0, iNumNoKwChats - 1);
         cNumRetries++;
      }
      iUsedUnknownChatIndex[4] = iUsedUnknownChatIndex[3];
      iUsedUnknownChatIndex[3] = iUsedUnknownChatIndex[2];
      iUsedUnknownChatIndex[2] = iUsedUnknownChatIndex[1];
      iUsedUnknownChatIndex[1] = iUsedUnknownChatIndex[0];
      iUsedUnknownChatIndex[0] = iRandom;
      strcpy (pszReply, szNoKwChat[iRandom]);

      if (g_b_DebugChat)
         ALERT(at_logged, "[DEBUG] BotCheckKeywords - Bot %s didn't find a keyword; uses some universal reply %s.\n", pBot->name, pszReply);

      return (TRUE);
   }
   return (FALSE);
}


bool BotParseChat (bot_t *pBot, char *pszReply)
{
   char szMessage[512];
   int iMessageLen;
   int i = 0;

   // Copy to safe place
   strcpy (szMessage, pBot->SaytextBuffer.szSayText);

   // Text to uppercase for Keyword parsing
   iMessageLen = (int)strlen (szMessage);
   for (i = 0; i < iMessageLen; i++)
      szMessage[i] = (char) toupper ((char) szMessage[i]); // KWo it was (int)
   szMessage[i] = 0;

   // Find the : char behind the name to get the start of the real text
   if (g_bIsOldCS15)
   {
      i = 0;  // KWo - 31.03.2006
      while (i <= iMessageLen)
      {
         if (szMessage[i] == ':')
            break;
         i++;
      }

      if (g_b_DebugChat)
         ALERT(at_logged, "[DEBUG] BotParseChat - Bot %s got the message: %s.\n", pBot->name, szMessage);

      return (BotCheckKeywords (pBot, &szMessage[i], pszReply));  // KWo - 31.03.2006
   }
   return (BotCheckKeywords (pBot, &szMessage[0], pszReply));  // KWo - 31.03.2006
}


bool BotRepliesToPlayer (bot_t *pBot)
{
   char szText[256];

   if ((pBot->SaytextBuffer.iEntityIndex > 0)
       && (pBot->SaytextBuffer.iEntityIndex <= gpGlobals->maxClients)
       && (pBot->SaytextBuffer.szSayText[0] != 0))
   {
      if ((pBot->SaytextBuffer.fTimeNextChat < gpGlobals->time) 
         && (g_fLastChatTime + 2.0 < gpGlobals->time)) // KWo - 10.03.2013
      {
         if ((RANDOM_LONG (0, 100) < pBot->SaytextBuffer.cChatProbability)  // KWo - 06.03.2010
             && BotParseChat (pBot, (char *) &szText))
         {
            if (g_b_DebugChat)
               ALERT(at_logged, "[DEBUG] BotRepliesToPlayer - Bot %s wants to reply: %s.\n", pBot->name, szText);

            BotPrepareChatMessage (pBot, (char *) &szText);
            BotPushMessageQueue (pBot, MSG_CS_SAY);
            pBot->SaytextBuffer.iEntityIndex = -1;
            pBot->SaytextBuffer.szSayText[0] = 0x0;
            pBot->SaytextBuffer.fTimeNextChat = gpGlobals->time + pBot->SaytextBuffer.fChatDelay;
            pBot->f_lastchattime = gpGlobals->time;   // KWo - 27.03.2010
            g_fLastChatTime = gpGlobals->time;        // KWo - 27.03.2010
            return (TRUE);
         }

         pBot->SaytextBuffer.iEntityIndex = -1;
         pBot->SaytextBuffer.szSayText[0] = 0x0;
      }
   }

   return (FALSE);
}
