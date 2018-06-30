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
// bot_weapons.h
//
// Contains Defines and Structures for the CS Weapons

#ifndef BOT_WEAPONS_H
#define BOT_WEAPONS_H

// Set by me to flag in-between weapon switching
#define CS_WEAPON_INSWITCH 255

// weapon ID values for Counter-Strike
#define CS_WEAPON_P228 1
#define CS_WEAPON_SHIELDGUN 2
#define CS_WEAPON_SCOUT 3
#define CS_WEAPON_HEGRENADE 4
#define CS_WEAPON_XM1014 5
#define CS_WEAPON_C4 6
#define CS_WEAPON_MAC10 7
#define CS_WEAPON_AUG 8
#define CS_WEAPON_SMOKEGRENADE 9
#define CS_WEAPON_ELITE 10
#define CS_WEAPON_FIVESEVEN 11
#define CS_WEAPON_UMP45 12
#define CS_WEAPON_SG550 13
#define CS_WEAPON_GALIL 14
#define CS_WEAPON_FAMAS 15
#define CS_WEAPON_USP 16
#define CS_WEAPON_GLOCK18 17
#define CS_WEAPON_AWP 18
#define CS_WEAPON_MP5NAVY 19
#define CS_WEAPON_M249 20
#define CS_WEAPON_M3 21
#define CS_WEAPON_M4A1 22
#define CS_WEAPON_TMP 23
#define CS_WEAPON_G3SG1 24
#define CS_WEAPON_FLASHBANG 25
#define CS_WEAPON_DEAGLE 26
#define CS_WEAPON_SG552 27
#define CS_WEAPON_AK47 28
#define CS_WEAPON_KNIFE 29
#define CS_WEAPON_P90 30

#define NUM_WEAPONS 26  // it's the number of PB weapons - used for buying and restriction - see the list below
#define NUM_EQUIPMENTS 9

#define MIN_BURST_DISTANCE 300.0 // KWo - 09.04.2010

/*
# 0 - KNIFE
# 1 - USP
# 2 - GLOCK18
# 3 - DEAGLE
# 4 - P228
# 5 - ELITE
# 6 - FIVESEVEN
# 7 - M3
# 8 - XM1014
# 9 - MP5NAVY
# 10 - TMP
# 11 - P90
# 12 - MAC10
# 13 - UMP45
# 14 - AK47
# 15 - SG552
# 16 - M4A1
# 17 - GALIL
# 18 - FAMAS
# 19 - AUG
# 20 - SCOUT
# 21 - AWP
# 22 - G3SG1
# 23 - SG550
# 24 - M249
# 25 - SHIELD
*/

// weapon numbers for podbot mm - used for buying and restriction - KWo - 11.03.2006
enum pb_weapon_index
{
// Knife
   PB_WEAPON_KNIFE = 0,
// Pistols
   PB_WEAPON_USP = 1,
   PB_WEAPON_GLOCK18 = 2,
   PB_WEAPON_DEAGLE = 3,
   PB_WEAPON_P228 = 4,
   PB_WEAPON_ELITE = 5,
   PB_WEAPON_FIVESEVEN = 6,

// Shotguns
   PB_WEAPON_M3 = 7,
   PB_WEAPON_XM1014 = 8,

// SMGs
   PB_WEAPON_MP5NAVY = 9,
   PB_WEAPON_TMP = 10,
   PB_WEAPON_P90 = 11,
   PB_WEAPON_MAC10 = 12,
   PB_WEAPON_UMP45 = 13,

// Rifles
   PB_WEAPON_AK47 = 14,
   PB_WEAPON_SG552 = 15,
   PB_WEAPON_M4A1 = 16,
   PB_WEAPON_GALIL = 17,
   PB_WEAPON_FAMAS = 18,
   PB_WEAPON_AUG = 19,
   PB_WEAPON_SCOUT = 20,
   PB_WEAPON_AWP = 21,
   PB_WEAPON_G3SG1 = 22,
   PB_WEAPON_SG550 = 23,

// Machine Gun
   PB_WEAPON_M249 = 24,

// Shield - as equivalent for primary weapon
   PB_WEAPON_SHIELDGUN =  25
};

// Equipement and ammo numbers for podbot mm - used for buying and restriction - KWo - 11.03.2006
enum pb_equipment_index
{
   PB_WEAPON_VEST = 0,
   PB_WEAPON_VESTHELM = 1,
   PB_WEAPON_FLASHBANG = 2,
   PB_WEAPON_HEGRENADE = 3,
   PB_WEAPON_SMOKEGRENADE = 4,
   PB_WEAPON_DEFUSER = 5,
   PB_WEAPON_NVGS = 6,
   PB_WEAPON_AMMO1 = 7,
   PB_WEAPON_AMMO2 = 8
};

typedef struct
{
   char szClassname[64];
   int iAmmo1; // ammo index for primary ammo
   int iAmmo1Max; // max primary ammo
   int iAmmo2; // ammo index for secondary ammo
   int iAmmo2Max; // max secondary ammo
   int iSlot; // HUD slot (0 based)
   int iPosition; // slot position
   int iId; // weapon ID
   int iFlags; // flags???
} bot_weapon_t;



typedef struct
{
   int iId;
   float primary_base_delay;
   float primary_min_delay[6];
   float primary_max_delay[6];
   float secondary_base_delay;
   float secondary_min_delay[5];
   float secondary_max_delay[5];
   int iMaxFireBullets;
   float fMinBurstPauseFactor;
} bot_fire_delay_t;


#endif // BOT_WEAPONS_H
