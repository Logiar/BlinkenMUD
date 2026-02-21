/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

/*************************************************************************** 
*       ROT 1.4 is copyright 1996-1997 by Russ Walsh                       * 
*       By using this code, you have agreed to follow the terms of the     * 
*       ROT license, in the file doc/rot.license                           * 
***************************************************************************/
/*
 * Copyright (C) 2007-2011 See the AUTHORS.BlinkenMUD file for details
 * By using this code, you have agreed to follow the terms of the   
 * ROT license, in the file doc/rot.license        
*/

#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "interp.h"


/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */
#define SLOT(n)	n

const struct skill_type skill_table[MAX_SKILL] = {

/*
 * Magic spells.
 */

/*
    {
	"NAME",
	{ SKILL_LEVELS 1st TIER,
	  SKILL_LEVELS 2nd TIER },
	{ DIFFICULTY 1st TIER,
	  DIFFICULTY 2nd TIER },
	SPELL_FUN,		TARGET,		MIN_POSITION,
	GSN,		SOCKET,	SLOT(#),	MIN_MANA,	BEATS,
	"DAMAGE NOUN",	"OFF MESSAGE",	"OFF MESSAGE (OBJ)"
    }

	NAME		Name of Spell/Skill
	SKILL_LEVELS	Levels to obtain spell/skill per class
	DIFFICULTY	Spells: multiplier for base mana
			Skills: charge for gaining skill
	SPELL_FUN	Routine to call for spells
	TARGET		Legal targets
	MIN_POSTITION	Position for caster/user
	GSN		gsn for skills and some spells
	SOCKET		Can spell be cast on person from same socket
			(to limit multiplaying)
	SLOT(#)		A unique slot number for spells
	MIN_MANA	Base mana for spells (multiplied by DIFFICULTY)
	BEATS		Waiting time after use
	DAMAGE NOUN	Damage Message
	OFF MESSAGE	Wear off message
	OFF MESSAGE (OBJ)	Wear off message for objects
*/

  {
   "reserved",
   {199, 199, 199, 199, 199, 199, 199,
    199, 199, 199, 199, 199, 199, 199},
   {99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99},
   0, TAR_IGNORE, POS_STANDING,
   NULL, TRUE, SLOT (0), 0, 0,
   "", "", ""},

  {
   "acid blast",
   {55, 93, 103, 103, 100, 45, 65,
    45, 83, 103, 103, 90, 35, 55},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_acid_blast, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (70), 20, 12,
   "acid blast", "!Acid Blast!", ""},

  {
   "animate",
   {103, 103, 103, 103, 103, 103, 45,
    103, 103, 103, 103, 103, 103, 35},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_animate, TAR_OBJ_INV, POS_STANDING,
   NULL, TRUE, SLOT (239), 20, 12,
   "", "!Animate!", ""},

  {
   "armor",
   {13, 2, 102, 102, 102, 23, 102,
    3, 1, 102, 102, 102, 13, 102},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_armor, TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL, FALSE, SLOT (1), 5, 12,
   "", "You feel less armored.", ""},

  {
   "bless",
   {102, 13, 102, 102, 102, 16, 102,
    75, 3, 102, 102, 102, 6, 102},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_bless, TAR_OBJ_CHAR_DEF, POS_STANDING,
   NULL, FALSE, SLOT (3), 5, 12,
   "", "You feel less righteous.",
   "$p's holy aura fades."},

  {
   "blindness",
   {24, 15, 103, 103, 65, 14, 34,
    14, 5, 103, 103, 55, 4, 24},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_blindness, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   &gsn_blindness, TRUE, SLOT (4), 5, 12,
   "", "You can see again.", ""},

  {
   "burning hands",
   {13, 87, 103, 103, 103, 23, 103,
    3, 77, 103, 103, 103, 13, 85},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_burning_hands, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (5), 15, 12,
   "burning hands", "!Burning Hands!", ""},

  {
   "call lightning",
   {103, 36, 103, 103, 30, 48, 103,
    92, 26, 103, 103, 20, 38, 103},
   {1, 1, 2, 2, 1, 1, 1,
    1, 1, 2, 2, 1, 1, 1},
   spell_call_lightning, TAR_IGNORE, POS_FIGHTING,
   NULL, TRUE, SLOT (6), 15, 12,
   "lightning bolt", "!Call Lightning!", ""},

  {
   "calm",
   {95, 32, 102, 102, 90, 46, 59,
    85, 22, 102, 102, 80, 36, 49},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_calm, TAR_IGNORE, POS_FIGHTING,
   NULL, TRUE, SLOT (509), 30, 12,
   "", "You have lost your peace of mind.", ""},

  {
   "cancellation",
   {51, 51, 102, 102, 102, 46, 102,
    41, 41, 102, 102, 102, 36, 102},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_cancellation, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
   NULL, FALSE, SLOT (507), 20, 12,
   "", "!cancellation!", ""},

  {
   "cause critical",
   {45, 45, 103, 103, 103, 103, 21,
    35, 35, 103, 103, 103, 103, 11},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_cause_critical, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (63), 20, 12,
   "spell", "!Cause Critical!", ""},

  {
   "cause light",
   {5, 2, 103, 103, 103, 103, 5,
    1, 1, 103, 103, 103, 90, 4},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_cause_light, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (62), 15, 12,
   "spell", "!Cause Light!", ""},

  {
   "cause serious",
   {20, 23, 103, 103, 103, 103, 11,
    10, 13, 103, 103, 103, 100, 9},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_cause_serious, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (64), 17, 12,
   "spell", "!Cause Serious!", ""},

  {
   "chain lightning",
   {66, 103, 103, 103, 103, 103, 103,
    55, 103, 103, 103, 103, 103, 103},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_chain_lightning, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (500), 25, 12,
   "lightning", "!Chain Lightning!", ""},

  {
   "change sex",
   {103, 103, 103, 103, 103, 103, 103,
    103, 103, 103, 103, 103, 103, 103},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_change_sex, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (82), 15, 12,
   "", "Your body feels familiar again.", ""},

  {
   "charm person",
   {40, 30, 50, 103, 50, 103, 43,
    30, 20, 40, 103, 40, 103, 33},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_charm_person, TAR_CHAR_OFFENSIVE, POS_STANDING,
   &gsn_charm_person, TRUE, SLOT (7), 5, 12,
   "", "You feel more self-confident.", ""},

  {
   "chill touch",
   {12, 103, 103, 103, 103, 22, 40,
    2, 100, 103, 103, 103, 12, 30},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_chill_touch, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (8), 15, 12,
   "chilling touch", "You feel less cold.", ""},

  {
   "colour spray",
   {32, 103, 103, 103, 103, 20, 103,
    22, 103, 103, 103, 103, 10, 103},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_colour_spray, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (10), 15, 12,
   "colour spray", "!Colour Spray!", ""},

  {
   "conjure",
   {85, 102, 102, 102, 102, 95, 102,
    59, 102, 102, 102, 102, 71, 102},
   {1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1},
   spell_conjure, TAR_IGNORE, POS_STANDING,
   NULL, TRUE, SLOT (280), 100, 12,
   "", "!Conjure!", ""},

  {
   "continual light",
   {11, 8, 102, 102, 7, 5, 102,
    1, 7, 102, 102, 1, 4, 102},
   {1, 1, 2, 2, 1, 1, 2,
    1, 1, 2, 2, 1, 1, 2},
   spell_continual_light, TAR_IGNORE, POS_STANDING,
   NULL, TRUE, SLOT (57), 7, 12,
   "", "!Continual Light!", ""},

  {
   "control weather",
   {102, 48, 102, 102, 32, 20, 102,
    102, 38, 102, 102, 22, 10, 102},
   {1, 1, 2, 2, 1, 1, 2,
    1, 1, 2, 2, 1, 1, 2},
   spell_control_weather, TAR_IGNORE, POS_STANDING,
   NULL, TRUE, SLOT (11), 25, 12,
   "", "!Control Weather!", ""},

  {
   "create food",
   {19, 10, 102, 102, 6, 7, 102,
    9, 1, 102, 102, 1, 6, 102},
   {1, 1, 2, 2, 1, 1, 2,
    1, 1, 2, 2, 1, 1, 2},
   spell_create_food, TAR_IGNORE, POS_STANDING,
   NULL, TRUE, SLOT (12), 5, 12,
   "", "!Create Food!", ""},

  {
   "create rose",
   {32, 21, 102, 102, 16, 11, 102,
    22, 11, 102, 102, 6, 1, 102},
   {1, 1, 2, 2, 1, 1, 2,
    1, 1, 2, 2, 1, 1, 2},
   spell_create_rose, TAR_IGNORE, POS_STANDING,
   NULL, TRUE, SLOT (511), 30, 12,
   "", "!Create Rose!", ""},

  {
   "create spring",
   {28, 34, 102, 102, 26, 24, 102,
    18, 24, 102, 102, 16, 14, 102},
   {1, 1, 2, 2, 1, 1, 2,
    1, 1, 2, 2, 1, 1, 2},
   spell_create_spring, TAR_IGNORE, POS_STANDING,
   NULL, TRUE, SLOT (80), 20, 12,
   "", "!Create Spring!", ""},

  {
   "create water",
   {15, 5, 102, 102, 2, 3, 102,
    5, 1, 102, 102, 1, 2, 102},
   {1, 1, 2, 2, 1, 1, 2,
    1, 1, 2, 2, 1, 1, 2},
   spell_create_water, TAR_OBJ_INV, POS_STANDING,
   NULL, TRUE, SLOT (13), 5, 12,
   "", "!Create Water!", ""},

  {
   "cure blindness",
   {102, 11, 102, 102, 13, 16, 102,
    90, 1, 102, 102, 3, 6, 102},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_cure_blindness, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
   NULL, FALSE, SLOT (14), 5, 12,
   "", "!Cure Blindness!", ""},

  {
   "cure critical",
   {102, 25, 102, 102, 49, 37, 102,
    102, 15, 102, 102, 39, 27, 102},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_cure_critical, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
   NULL, FALSE, SLOT (15), 20, 12,
   "", "!Cure Critical!", ""},

  {
   "cure disease",
   {102, 26, 102, 102, 28, 32, 102,
    102, 16, 102, 102, 18, 22, 102},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_cure_disease, TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL, FALSE, SLOT (501), 20, 12,
   "", "!Cure Disease!", ""},

  {
   "cure light",
   {102, 4, 102, 102, 3, 8, 102,
    102, 3, 102, 102, 1, 1, 102},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_cure_light, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
   NULL, FALSE, SLOT (16), 10, 12,
   "", "!Cure Light!", ""},

  {
   "cure poison",
   {102, 28, 102, 102, 37, 33, 102,
    102, 18, 102, 102, 27, 23, 102},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_cure_poison, TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL, FALSE, SLOT (43), 5, 12,
   "", "!Cure Poison!", ""},

  {
   "cure serious",
   {102, 14, 102, 102, 34, 24, 102,
    102, 4, 102, 102, 24, 14, 102},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_cure_serious, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
   NULL, FALSE, SLOT (61), 15, 12,
   "", "!Cure Serious!", ""},

  {
   "curse",
   {35, 35, 103, 103, 103, 38, 45,
    25, 25, 103, 103, 103, 28, 35},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_curse, TAR_OBJ_CHAR_OFF, POS_FIGHTING,
   &gsn_curse, TRUE, SLOT (17), 20, 12,
   "curse", "The curse wears off.",
   "$p is no longer impure."},

  {
   "demonfire",
   {76, 68, 103, 103, 103, 103, 77,
    66, 58, 103, 103, 103, 103, 67},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_demonfire, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (505), 20, 12,
   "torments", "!Demonfire!", ""},

  {
   "detect evil",
   {22, 22, 102, 102, 10, 102, 17,
    12, 12, 102, 102, 1, 85, 7},
   {1, 1, 2, 2, 1, 1, 1,
    1, 1, 2, 2, 1, 1, 1},
   spell_detect_evil, TAR_CHAR_SELF, POS_STANDING,
   NULL, FALSE, SLOT (18), 5, 12,
   "", "The red in your vision disappears.", ""},

  {
   "detect good",
   {23, 22, 102, 102, 10, 102, 17,
    13, 12, 102, 102, 1, 102, 7},
   {1, 1, 2, 2, 1, 1, 1,
    1, 1, 2, 2, 1, 1, 1},
   spell_detect_good, TAR_CHAR_SELF, POS_STANDING,
   NULL, FALSE, SLOT (513), 5, 12,
   "", "The gold in your vision disappears.", ""},

  {
   "detect hidden",
   {24, 22, 24, 102, 29, 26, 18,
    14, 12, 14, 102, 19, 16, 8},
   {1, 1, 2, 2, 1, 1, 1,
    1, 1, 2, 2, 1, 1, 1},
   spell_detect_hidden, TAR_CHAR_SELF, POS_STANDING,
   NULL, FALSE, SLOT (44), 5, 12,
   "", "You feel less aware of your surroundings.",
   ""},

  {
   "detect invis",
   {25, 16, 11, 102, 25, 25, 4,
    15, 6, 1, 102, 15, 15, 3},
   {1, 1, 2, 2, 1, 1, 1,
    1, 1, 2, 2, 1, 1, 1},
   spell_detect_invis, TAR_CHAR_SELF, POS_STANDING,
   NULL, FALSE, SLOT (19), 5, 12,
   "", "You no longer see invisible objects.",
   ""},

  {
   "detect magic",
   {26, 11, 8, 102, 24, 21, 3,
    16, 1, 1, 102, 14, 11, 2},
   {1, 1, 2, 2, 1, 1, 1,
    1, 1, 2, 2, 1, 1, 1},
   spell_detect_magic, TAR_CHAR_SELF, POS_STANDING,
   NULL, FALSE, SLOT (20), 5, 12,
   "", "The detect magic wears off.", ""},

  {
   "detect poison",
   {27, 14, 18, 102, 8, 11, 11,
    17, 4, 8, 102, 1, 1, 1},
   {1, 1, 2, 2, 1, 1, 1,
    1, 1, 2, 2, 1, 1, 1},
   spell_detect_poison, TAR_OBJ_INV, POS_STANDING,
   NULL, TRUE, SLOT (21), 5, 12,
   "", "!Detect Poison!", ""},

  {
   "dispel evil",
   {103, 30, 103, 103, 103, 35, 53,
    103, 20, 103, 103, 103, 25, 43},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_dispel_evil, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (22), 15, 12,
   "dispel evil", "!Dispel Evil!", ""},

  {
   "dispel good",
   {103, 30, 103, 103, 103, 35, 53,
    103, 20, 103, 103, 103, 25, 43},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_dispel_good, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (512), 15, 12,
   "dispel good", "!Dispel Good!", ""},

  {
   "dispel magic",
   {48, 48, 103, 103, 103, 36, 103,
    38, 38, 103, 103, 103, 35, 103},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_dispel_magic, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (59), 15, 12,
   "", "!Dispel Magic!", ""},

  {
   "earthquake",
   {83, 20, 103, 103, 36, 11, 76,
    73, 10, 103, 103, 26, 1, 66},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_earthquake, TAR_IGNORE, POS_FIGHTING,
   NULL, TRUE, SLOT (23), 15, 12,
   "earthquake", "!Earthquake!", ""},

  {
   "empower",
   {36, 40, 103, 103, 103, 42, 103,
    26, 30, 103, 103, 103, 32, 103},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_empower, TAR_IGNORE, POS_RESTING,
   NULL, TRUE, SLOT (234), 40, 12,
   "", "!Empower!", ""},

  {
   "enchant armor",
   {31, 90, 102, 102, 102, 102, 102,
    21, 80, 102, 102, 101, 100, 102},
   {2, 2, 4, 4, 4, 2, 4,
    2, 2, 4, 4, 4, 2, 4},
   spell_enchant_armor, TAR_OBJ_INV, POS_STANDING,
   NULL, TRUE, SLOT (510), 100, 24,
   "", "!Enchant Armor!", ""},

  {
   "enchant weapon",
   {32, 90, 102, 102, 102, 102, 102,
    22, 80, 102, 102, 101, 100, 102},
   {2, 2, 4, 4, 4, 2, 4,
    2, 2, 4, 4, 4, 2, 4},
   spell_enchant_weapon, TAR_OBJ_INV, POS_STANDING,
   NULL, TRUE, SLOT (24), 100, 24,
   "", "!Enchant Weapon!", ""},

  {
   "energy drain",
   {38, 44, 103, 103, 103, 103, 10,
    28, 34, 103, 103, 103, 103, 1},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_energy_drain, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (25), 35, 12,
   "energy drain", "!Energy Drain!", ""},

  {
   "faerie fire",
   {12, 5, 103, 103, 17, 2, 92,
    2, 4, 103, 103, 7, 1, 82},
   {1, 1, 2, 2, 1, 1, 2,
    1, 1, 2, 2, 1, 1, 2},
   spell_faerie_fire, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (72), 5, 12,
   "faerie fire", "The pink aura around you fades away.",
   ""},

  {
   "faerie fog",
   {28, 41, 103, 103, 35, 18, 103,
    18, 31, 103, 103, 25, 8, 103},
   {1, 1, 2, 2, 1, 1, 2,
    1, 1, 2, 2, 1, 1, 2},
   spell_faerie_fog, TAR_IGNORE, POS_STANDING,
   NULL, TRUE, SLOT (73), 12, 12,
   "faerie fog", "!Faerie Fog!", ""},

  {
   "farsight",
   {4, 8, 103, 103, 15, 9, 6,
    3, 7, 103, 103, 5, 8, 5},
   {1, 1, 2, 2, 1, 1, 1,
    1, 1, 2, 2, 1, 1, 1},
   spell_farsight, TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL, FALSE, SLOT (521), 5, 12,
   "", "The green in your vision disappears.",
   ""},

  {
   "fireball",
   {44, 89, 103, 103, 103, 44, 81,
    34, 79, 103, 103, 103, 34, 71},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_fireball, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (26), 15, 12,
   "fireball", "!Fireball!", ""},

  {
   "fireproof",
   {25, 24, 102, 102, 102, 21, 102,
    15, 14, 102, 102, 102, 11, 102},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_fireproof, TAR_OBJ_INV, POS_STANDING,
   NULL, TRUE, SLOT (523), 10, 12,
   "", "", "$p's protective aura fades."},

  {
   "fireshield",
   {50, 60, 102, 102, 102, 55, 102,
    40, 50, 102, 102, 102, 45, 102},
   {3, 3, 5, 5, 5, 3, 5,
    3, 3, 5, 5, 5, 3, 5},
   spell_fireshield, TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL, FALSE, SLOT (411), 75, 12,
   "fireball", "Your firey shield gutters out.",
   ""},

  {
   "flamestrike",
   {34, 40, 103, 103, 103, 30, 47,
    24, 30, 103, 103, 103, 20, 37},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_flamestrike, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (65), 20, 12,
   "flamestrike", "!Flamestrike!", ""},

  {
   "floating disc",
   {7, 19, 102, 102, 45, 15, 102,
    6, 9, 102, 102, 35, 5, 102},
   {1, 1, 2, 2, 1, 1, 2,
    1, 1, 2, 2, 1, 1, 2},
   spell_floating_disc, TAR_IGNORE, POS_STANDING,
   NULL, TRUE, SLOT (522), 40, 24,
   "", "!Floating disc!", ""},

  {
   "fly",
   {19, 36, 102, 102, 31, 66, 33,
    9, 26, 102, 102, 21, 56, 23},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_fly, TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL, FALSE, SLOT (56), 10, 18,
   "", "You slowly float to the ground.", ""},

  {
   "frenzy",
   {66, 48, 103, 103, 57, 103, 55,
    56, 38, 103, 103, 47, 103, 45},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_frenzy, TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL, FALSE, SLOT (504), 30, 24,
   "", "Your rage ebbs.", ""},

  {
   "gate",
   {54, 34, 102, 102, 42, 81, 56,
    44, 24, 102, 102, 32, 71, 46},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_gate, TAR_IGNORE, POS_FIGHTING,
   NULL, FALSE, SLOT (83), 80, 12,
   "", "!Gate!", ""},

  {
   "giant strength",
   {41, 42, 102, 102, 37, 31, 37,
    31, 32, 102, 102, 27, 21, 27},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_giant_strength, TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL, FALSE, SLOT (39), 20, 12,
   "", "You feel weaker.", ""},

  {
   "harm",
   {43, 46, 103, 103, 103, 41, 103,
    33, 36, 103, 103, 103, 31, 103},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_harm, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (27), 35, 12,
   "harm spell", "!Harm!", ""},

  {
   "haste",
   {42, 54, 103, 103, 60, 26, 52,
    32, 44, 103, 103, 50, 16, 42},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_haste, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
   NULL, FALSE, SLOT (502), 30, 12,
   "", "You feel yourself slow down.", ""},

  {
   "heal",
   {102, 32, 102, 102, 65, 55, 102,
    102, 22, 102, 102, 55, 45, 102},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_heal, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
   NULL, FALSE, SLOT (28), 50, 12,
   "", "!Heal!", ""},

  {
   "heat metal",
   {73, 42, 103, 103, 103, 22, 103,
    63, 32, 103, 103, 103, 12, 103},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_heat_metal, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (516), 25, 18,
   "spell", "!Heat Metal!", ""},

  {
   "holy word",
   {103, 71, 103, 103, 103, 77, 103,
    103, 61, 103, 103, 103, 67, 103},
   {2, 2, 4, 4, 4, 2, 4,
    2, 2, 4, 4, 4, 2, 4},
   spell_holy_word, TAR_IGNORE, POS_FIGHTING,
   NULL, TRUE, SLOT (506), 200, 24,
   "divine wrath", "!Holy Word!", ""},

  {
   "iceshield",
   {30, 30, 102, 102, 102, 35, 102,
    20, 20, 102, 102, 102, 25, 102},
   {3, 3, 5, 5, 5, 3, 5,
    3, 3, 5, 5, 5, 3, 5},
   spell_iceshield, TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL, FALSE, SLOT (410), 75, 12,
   "chilling touch", "Your icy shield slowly melts away.",
   ""},

  {
   "identify",
   {30, 32, 35, 102, 46, 42, 28,
    20, 22, 25, 102, 36, 32, 18},
   {1, 1, 2, 2, 1, 1, 1,
    1, 1, 2, 2, 1, 1, 1},
   spell_identify, TAR_OBJ_INV, POS_STANDING,
   NULL, TRUE, SLOT (53), 12, 24,
   "", "!Identify!", ""},

  {
   "infravision",
   {18, 26, 102, 102, 102, 18, 12,
    8, 16, 102, 102, 102, 8, 2},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_infravision, TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL, FALSE, SLOT (77), 5, 18,
   "", "You no longer see in the dark.", ""},

  {
   "invisibility",
   {9, 37, 17, 102, 12, 39, 13,
    8, 27, 7, 102, 2, 29, 3},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_invis, TAR_OBJ_CHAR_DEF, POS_STANDING,
   &gsn_invis, FALSE, SLOT (29), 5, 12,
   "", "You are no longer invisible.",
   "$p fades into view."},

  {
   "know alignment",
   {24, 17, 102, 102, 36, 27, 33,
    14, 7, 102, 102, 26, 17, 23},
   {1, 1, 2, 2, 1, 1, 1,
    1, 1, 2, 2, 1, 1, 1},
   spell_know_alignment, TAR_CHAR_DEFENSIVE, POS_FIGHTING,
   NULL, FALSE, SLOT (58), 9, 12,
   "", "!Know Alignment!", ""},

  {
   "lightning bolt",
   {25, 46, 103, 103, 40, 42, 91,
    15, 36, 103, 103, 30, 32, 81},
   {1, 1, 2, 2, 1, 1, 1,
    1, 1, 2, 2, 1, 1, 1},
   spell_lightning_bolt, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (30), 15, 12,
   "lightning bolt", "!Lightning Bolt!", ""},

  {
   "locate object",
   {17, 30, 102, 102, 38, 35, 24,
    7, 20, 102, 102, 28, 25, 14},
   {1, 1, 2, 2, 1, 1, 1,
    1, 1, 2, 2, 1, 1, 1},
   spell_locate_object, TAR_IGNORE, POS_STANDING,
   NULL, TRUE, SLOT (31), 20, 18,
   "", "!Locate Object!", ""},

  {
   "magic missile",
   {1, 103, 103, 103, 103, 12, 103,
    1, 103, 103, 103, 103, 2, 103},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_magic_missile, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (32), 15, 12,
   "magic missile", "!Magic Missile!", ""},

  {
   "mass healing",
   {102, 75, 102, 102, 85, 85, 102,
    102, 65, 102, 102, 75, 75, 102},
   {2, 2, 4, 4, 2, 2, 2,
    2, 2, 4, 4, 2, 2, 2},
   spell_mass_healing, TAR_IGNORE, POS_STANDING,
   NULL, FALSE, SLOT (508), 100, 36,
   "", "!Mass Healing!", ""},

  {
   "mass invis",
   {43, 49, 103, 103, 55, 103, 54,
    33, 39, 103, 103, 45, 103, 44},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_mass_invis, TAR_IGNORE, POS_STANDING,
   &gsn_mass_invis, FALSE, SLOT (69), 20, 24,
   "", "You are no longer invisible.", ""},

  {
   "nexus",
   {79, 69, 102, 102, 81, 102, 92,
    69, 59, 102, 102, 71, 102, 82},
   {2, 2, 4, 4, 4, 2, 4,
    2, 2, 4, 4, 4, 2, 4},
   spell_nexus, TAR_IGNORE, POS_STANDING,
   NULL, FALSE, SLOT (520), 150, 36,
   "", "!Nexus!", ""},

  {
   "pass door",
   {48, 63, 49, 102, 60, 102, 42,
    38, 53, 39, 102, 50, 102, 32},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_pass_door, TAR_CHAR_SELF, POS_STANDING,
   NULL, FALSE, SLOT (74), 20, 12,
   "", "You feel solid again.", ""},

  {
   "plague",
   {46, 34, 103, 103, 59, 39, 65,
    36, 24, 103, 103, 49, 29, 55},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_plague, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   &gsn_plague, TRUE, SLOT (503), 20, 12,
   "sickness", "Your sores vanish.", ""},

  {
   "poison",
   {33, 24, 103, 103, 49, 27, 22,
    23, 14, 103, 103, 39, 17, 12},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_poison, TAR_OBJ_CHAR_OFF, POS_FIGHTING,
   &gsn_poison, TRUE, SLOT (33), 10, 12,
   "poison", "You feel less sick.",
   "The poison on $p dries up."},

  {
   "portal",
   {70, 59, 102, 102, 72, 102, 83,
    60, 49, 102, 102, 62, 102, 73},
   {2, 2, 4, 4, 4, 2, 4,
    2, 2, 4, 4, 4, 2, 4},
   spell_portal, TAR_IGNORE, POS_STANDING,
   NULL, FALSE, SLOT (519), 100, 24,
   "", "!Portal!", ""},

  {
   "protection evil",
   {24, 18, 102, 102, 64, 19, 66,
    14, 8, 102, 102, 54, 9, 56},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_protection_evil, TAR_CHAR_SELF, POS_STANDING,
   NULL, FALSE, SLOT (34), 5, 12,
   "", "You feel less protected.", ""},

  {
   "protection good",
   {24, 18, 102, 102, 64, 19, 66,
    14, 8, 102, 102, 54, 9, 56},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_protection_good, TAR_CHAR_SELF, POS_STANDING,
   NULL, FALSE, SLOT (514), 5, 12,
   "", "You feel less protected.", ""},

  {
   "protection voodoo",
   {102, 102, 102, 102, 102, 102, 102,
    102, 102, 102, 102, 102, 102, 102},
   {1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1},
   spell_protection_voodoo, TAR_CHAR_SELF, POS_STANDING,
   NULL, FALSE, SLOT (345), 5, 12,
   "", "", ""},

  {
   "quest pill",
   {110, 110, 110, 110, 110, 110, 110,
    110, 110, 110, 110, 110, 110, 110},
   {1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1},
   spell_quest_pill, TAR_CHAR_SELF, POS_STANDING,
   NULL, TRUE, SLOT (530), 5, 18,
   "", "!Quest Pill!", ""},

  {
   "ray of truth",
   {103, 69, 103, 103, 103, 96, 103,
    103, 59, 103, 103, 103, 86, 103},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_ray_of_truth, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (518), 20, 12,
   "ray of truth", "!Ray of Truth!", ""},

  {
   "recharge",
   {18, 63, 103, 103, 103, 52, 103,
    8, 53, 103, 103, 103, 42, 103},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_recharge, TAR_OBJ_INV, POS_STANDING,
   NULL, TRUE, SLOT (517), 60, 24,
   "", "!Recharge!", ""},

  {
   "refresh",
   {16, 9, 102, 102, 18, 10, 16,
    6, 4, 102, 102, 8, 1, 6},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_refresh, TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL, FALSE, SLOT (81), 12, 18,
   "refresh", "!Refresh!", ""},

  {
   "remove curse",
   {102, 35, 102, 102, 93, 49, 102,
    93, 25, 102, 102, 83, 39, 102},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_remove_curse, TAR_OBJ_CHAR_DEF, POS_STANDING,
   NULL, FALSE, SLOT (35), 5, 12,
   "", "!Remove Curse!", ""},

  {
   "restore mana",
   {102, 102, 102, 102, 102, 102, 102,
    102, 102, 102, 102, 102, 102, 102},
   {1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1},
   spell_restore_mana, TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL, FALSE, SLOT (251), 1, 18,
   "restore mana", "!Restore Mana!", ""},

  {
   "resurrect",
   {103, 103, 103, 103, 103, 103, 28,
    103, 103, 103, 103, 103, 103, 18},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_resurrect, TAR_IGNORE, POS_STANDING,
   NULL, TRUE, SLOT (235), 35, 12,
   "", "!Resurrect!", ""},

  {
   "sanctuary",
   {52, 39, 103, 103, 98, 40, 103,
    42, 29, 103, 103, 57, 30, 103},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_sanctuary, TAR_CHAR_DEFENSIVE, POS_STANDING,
   &gsn_sanctuary, FALSE, SLOT (36), 75, 12,
   "", "The white aura around your body fades.",
   ""},

  {
   "shield",
   {40, 70, 102, 102, 88, 80, 102,
    30, 60, 102, 102, 78, 70, 102},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_shield, TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL, FALSE, SLOT (67), 12, 18,
   "", "Your force shield shimmers then fades away.",
   ""},

  {
   "shocking grasp",
   {20, 103, 103, 103, 103, 30, 27,
    10, 103, 103, 103, 103, 20, 17},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_shocking_grasp, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (53), 15, 12,
   "shocking grasp", "!Shocking Grasp!", ""},

  {
   "shockshield",
   {60, 40, 102, 102, 102, 90, 102,
    50, 30, 102, 102, 102, 80, 102},
   {3, 3, 5, 5, 5, 3, 5,
    3, 3, 5, 5, 5, 3, 5},
   spell_shockshield, TAR_CHAR_DEFENSIVE, POS_STANDING,
   NULL, FALSE, SLOT (412), 75, 12,
   "lightning bolt", "Your crackling shield sizzles and fades.",
   ""},

  {
   "sleep",
   {20, 103, 103, 103, 21, 103, 15,
    10, 95, 103, 103, 11, 103, 5},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_sleep, TAR_CHAR_OFFENSIVE, POS_STANDING,
   &gsn_sleep, TRUE, SLOT (38), 15, 12,
   "", "You feel less tired.", ""},

  {
   "slow",
   {45, 40, 103, 103, 60, 66, 51,
    35, 30, 103, 103, 50, 56, 41},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_slow, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (515), 30, 12,
   "", "You feel yourself speed up.", ""},

  {
   "stone skin",
   {49, 70, 102, 102, 100, 90, 102,
    39, 60, 102, 102, 90, 80, 102},
   {1, 1, 2, 2, 2, 1, 2,
    1, 1, 2, 2, 2, 1, 2},
   spell_stone_skin, TAR_CHAR_SELF, POS_STANDING,
   NULL, FALSE, SLOT (66), 12, 18,
   "", "Your skin feels soft again.", ""},

  {
   "summon",
   {48, 23, 102, 102, 33, 102, 51,
    38, 13, 102, 102, 23, 102, 41},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_summon, TAR_IGNORE, POS_STANDING,
   NULL, FALSE, SLOT (40), 50, 12,
   "", "!Summon!", ""},

  {
   "teleport",
   {24, 44, 102, 102, 59, 102, 59,
    14, 34, 102, 102, 49, 102, 49},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_teleport, TAR_CHAR_SELF, POS_FIGHTING,
   NULL, TRUE, SLOT (2), 35, 12,
   "", "!Teleport!", ""},

  {
   "transport",
   {25, 46, 102, 102, 45, 53, 51,
    15, 36, 102, 102, 35, 43, 41},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_transport, TAR_OBJ_TRAN, POS_STANDING,
   NULL, FALSE, SLOT (524), 30, 12,
   "", "!Transport!", ""},

  {
   "ventriloquate",
   {1, 102, 2, 102, 5, 102, 4,
    1, 102, 1, 102, 1, 102, 3},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_ventriloquate, TAR_IGNORE, POS_STANDING,
   NULL, TRUE, SLOT (41), 5, 12,
   "", "!Ventriloquate!", ""},

  {
   "voodoo",
   {102, 80, 102, 102, 102, 102, 65,
    102, 45, 102, 102, 102, 102, 30},
   {1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1},
   spell_voodoo, TAR_IGNORE, POS_STANDING,
   NULL, TRUE, SLOT (286), 80, 12,
   "", "!Voodoo!", ""},

  {
   "weaken",
   {21, 28, 103, 103, 40, 31, 25,
    11, 18, 103, 103, 30, 21, 15},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_weaken, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (68), 20, 12,
   "spell", "You feel stronger.", ""},

  {
   "word of recall",
   {63, 56, 103, 103, 47, 103, 73,
    53, 46, 103, 103, 37, 103, 63},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_word_of_recall, TAR_CHAR_SELF, POS_RESTING,
   NULL, TRUE, SLOT (42), 5, 12,
   "", "!Word of Recall!", ""},

/*
 * Dragon breath
 */
  {
   "acid breath",
   {62, 103, 103, 103, 103, 103, 103,
    52, 103, 103, 103, 103, 103, 103},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_acid_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (200), 100, 24,
   "blast of acid", "!Acid Breath!", ""},

  {
   "fire breath",
   {80, 90, 103, 103, 103, 103, 103,
    70, 80, 103, 103, 103, 103, 103},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_fire_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (201), 200, 24,
   "blast of flame", "The smoke leaves your eyes.", ""},

  {
   "frost breath",
   {67, 103, 103, 103, 103, 103, 103,
    57, 103, 103, 103, 103, 103, 103},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_frost_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (202), 125, 24,
   "blast of frost", "!Frost Breath!", ""},

  {
   "gas breath",
   {77, 103, 103, 103, 103, 103, 103,
    67, 103, 103, 103, 103, 103, 103},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_gas_breath, TAR_IGNORE, POS_FIGHTING,
   NULL, TRUE, SLOT (203), 175, 24,
   "blast of gas", "!Gas Breath!", ""},

  {
   "lightning breath",
   {74, 103, 103, 103, 103, 103, 103,
    64, 103, 103, 103, 103, 103, 103},
   {1, 1, 2, 2, 2, 1, 1,
    1, 1, 2, 2, 2, 1, 1},
   spell_lightning_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (204), 150, 24,
   "blast of lightning", "!Lightning Breath!", ""},

/*
 * Spells for mega1.are from Glop/Erkenbrand.
 */
  {
   "general purpose",
   {104, 104, 104, 104, 104, 104, 104,
    104, 104, 104, 104, 104, 104, 104},
   {0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0},
   spell_general_purpose, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (401), 0, 12,
   "general purpose ammo", "!General Purpose Ammo!", ""},

  {
   "high explosive",
   {104, 104, 104, 104, 104, 104, 104,
    104, 104, 104, 104, 104, 104, 104},
   {0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0},
   spell_high_explosive, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   NULL, TRUE, SLOT (402), 0, 12,
   "high explosive ammo", "!High Explosive Ammo!", ""},


/* combat and weapons skills */


  {
   "axe",
   {1, 2, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1},
   {6, 6, 5, 3, 4, 6, 5,
    5, 5, 5, 2, 3, 5, 4},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_axe, TRUE, SLOT (0), 0, 0,
   "", "!Axe!", ""},

  {
   "dagger",
   {1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1},
   {2, 3, 2, 1, 2, 3, 1,
    2, 2, 1, 1, 1, 2, 1},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_dagger, TRUE, SLOT (0), 0, 0,
   "", "!Dagger!", ""},

  {
   "flail",
   {1, 2, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1},
   {6, 3, 6, 3, 4, 3, 5,
    5, 2, 5, 2, 3, 2, 4},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_flail, TRUE, SLOT (0), 0, 0,
   "", "!Flail!", ""},

  {
   "mace",
   {1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1},
   {5, 1, 3, 2, 3, 2, 3,
    4, 1, 3, 1, 2, 1, 2},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_mace, TRUE, SLOT (0), 0, 0,
   "", "!Mace!", ""},

  {
   "polearm",
   {1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1},
   {6, 6, 6, 3, 3, 2, 5,
    5, 5, 5, 2, 2, 1, 4},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_polearm, TRUE, SLOT (0), 0, 0,
   "", "!Polearm!", ""},

  {
   "shield block",
   {1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1},
   {6, 4, 6, 1, 2, 4, 3,
    5, 3, 5, 1, 1, 3, 2},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_shield_block, TRUE, SLOT (0), 0, 0,
   "", "!Shield!", ""},

  {
   "spear",
   {1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1},
   {4, 4, 4, 2, 1, 3, 5,
    3, 3, 3, 1, 1, 2, 4},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_spear, TRUE, SLOT (0), 0, 0,
   "", "!Spear!", ""},

  {
   "sword",
   {1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1},
   {5, 6, 3, 1, 2, 4, 3,
    4, 5, 2, 1, 1, 3, 2},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_sword, TRUE, SLOT (0), 0, 0,
   "", "!sword!", ""},

  {
   "whip",
   {1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1},
   {6, 5, 5, 3, 3, 4, 5,
    5, 4, 4, 3, 2, 3, 4},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_whip, TRUE, SLOT (0), 0, 0,
   "", "!Whip!", ""},

  {
   "backstab",
   {103, 103, 1, 50, 7, 103, 7,
    103, 103, 1, 40, 1, 103, 6},
   {0, 0, 2, 4, 4, 0, 2,
    0, 0, 2, 3, 3, 0, 1},
   spell_null, TAR_IGNORE, POS_STANDING,
   &gsn_backstab, TRUE, SLOT (0), 0, 24,
   "backstab", "!Backstab!", ""},

  {
   "bash",
   {103, 103, 103, 1, 103, 103, 103,
    103, 103, 103, 1, 103, 103, 103},
   {0, 0, 0, 2, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_bash, TRUE, SLOT (0), 0, 24,
   "bash", "!Bash!", ""},

  {
   "berserk",
   {103, 103, 103, 27, 103, 103, 103,
    103, 103, 103, 17, 103, 103, 103},
   {0, 0, 0, 2, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_berserk, TRUE, SLOT (0), 0, 24,
   "", "You feel your pulse slow down.", ""},

  {
   "circle",
   {103, 103, 38, 103, 103, 103, 98,
    103, 103, 26, 103, 103, 103, 85},
   {0, 0, 4, 0, 0, 0, 5,
    0, 0, 3, 0, 0, 0, 4},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_circle, TRUE, SLOT (0), 0, 24,
   "circle", "!Circle!", ""},

  {
   "dirt kicking",
   {103, 103, 4, 4, 1, 9, 3,
    103, 103, 1, 1, 1, 8, 2},
   {0, 0, 4, 4, 4, 6, 2,
    0, 0, 3, 3, 3, 5, 1},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_dirt, TRUE, SLOT (0), 0, 24,
   "kicked dirt", "You rub the dirt out of your eyes.", ""},

  {
   "disarm",
   {103, 103, 18, 18, 32, 103, 20,
    103, 103, 8, 8, 22, 103, 10},
   {0, 0, 6, 3, 5, 0, 3,
    0, 0, 5, 2, 4, 0, 2},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_disarm, TRUE, SLOT (0), 0, 24,
   "", "!Disarm!", ""},

  {
   "dodge",
   {103, 103, 1, 19, 10, 103, 10,
    103, 103, 1, 9, 1, 103, 1},
   {0, 0, 4, 6, 5, 0, 2,
    0, 0, 3, 5, 4, 0, 1},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_dodge, TRUE, SLOT (0), 0, 0,
   "", "!Dodge!", ""},

  {
   "dual wield",
   {103, 103, 35, 3, 29, 103, 103,
    103, 103, 8, 1, 15, 103, 103},
   {0, 0, 3, 2, 3, 0, 0,
    0, 0, 2, 1, 3, 0, 0},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_dual_wield, TRUE, SLOT (0), 0, 0,
   "", "!Dual Wield!", ""},

  {
   "enhanced damage",
   {103, 103, 103, 1, 30, 103, 33,
    100, 103, 103, 1, 20, 103, 32},
   {0, 0, 0, 3, 6, 0, 7,
    0, 0, 0, 2, 5, 0, 6},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_enhanced_damage, TRUE, SLOT (0), 0, 0,
   "", "!Enhanced Damage!", ""},

  {
   "envenom",
   {103, 103, 15, 90, 19, 103, 9,
    103, 103, 5, 80, 9, 103, 1},
   {0, 0, 4, 6, 1, 0, 3,
    0, 0, 3, 5, 1, 0, 2},
   spell_null, TAR_IGNORE, POS_RESTING,
   &gsn_envenom, TRUE, SLOT (0), 0, 36,
   "", "!Envenom!", ""},

  {
   "feed",
   {103, 103, 103, 103, 103, 103, 29,
    103, 103, 103, 103, 103, 103, 11},
   {0, 0, 0, 0, 0, 0, 2,
    0, 0, 0, 0, 0, 0, 1},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_feed, TRUE, SLOT (0), 0, 24,
   "feed", "!Feed!", ""},

  {
   "gouge",
   {103, 103, 52, 103, 103, 103, 103,
    103, 103, 24, 103, 103, 103, 103},
   {0, 0, 2, 0, 0, 0, 0,
    0, 0, 1, 0, 0, 0, 0},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_gouge, TRUE, SLOT (0), 0, 24,
   "gouge", "Your vision clears.", ""},

  {
   "grip",
   {103, 103, 103, 25, 103, 103, 103,
    103, 103, 103, 10, 103, 103, 103},
   {0, 0, 0, 2, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_grip, TRUE, SLOT (0), 0, 0,
   "", "!Grip!", ""},

  {
   "hand to hand",
   {103, 103, 22, 9, 22, 103, 12,
    103, 103, 12, 8, 12, 103, 2},
   {0, 0, 4, 2, 4, 0, 2,
    0, 0, 3, 1, 3, 0, 1},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_hand_to_hand, TRUE, SLOT (0), 0, 0,
   "", "!Hand to Hand!", ""},

  {
   "kick",
   {103, 18, 21, 12, 20, 17, 15,
    103, 8, 11, 2, 10, 7, 5},
   {0, 4, 6, 3, 4, 6, 2,
    0, 3, 5, 2, 3, 5, 1},
   spell_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING,
   &gsn_kick, TRUE, SLOT (0), 0, 12,
   "kick", "!Kick!", ""},

  {
   "parry",
   {103, 103, 19, 1, 15, 27, 17,
    103, 103, 9, 1, 5, 17, 7},
   {0, 0, 4, 4, 5, 5, 2,
    0, 0, 3, 3, 4, 4, 1},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_parry, TRUE, SLOT (0), 0, 0,
   "", "!Parry!", ""},

  {
   "rescue",
   {103, 103, 103, 1, 13, 103, 103,
    103, 103, 103, 1, 3, 103, 103},
   {0, 0, 0, 4, 3, 0, 0,
    0, 0, 0, 3, 2, 0, 0},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_rescue, TRUE, SLOT (0), 0, 12,
   "", "!Rescue!", ""},

  {
   "trip",
   {103, 103, 2, 22, 25, 103, 5,
    103, 103, 1, 12, 15, 103, 1},
   {0, 0, 4, 8, 5, 0, 3,
    0, 0, 3, 7, 4, 0, 2},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_trip, TRUE, SLOT (0), 0, 24,
   "trip", "!Trip!", ""},

  {
   "stun",
   {103, 103, 103, 45, 103, 103, 103,
    103, 103, 103, 23, 103, 103, 103},
   {0, 0, 0, 3, 0, 0, 0,
    0, 0, 0, 2, 0, 0, 0},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_stun, TRUE, SLOT (0), 0, 0,
   "", "!Stun!", ""},

  {
   "second attack",
   {45, 36, 18, 7, 1, 25, 20,
    35, 26, 8, 6, 1, 15, 10},
   {7, 8, 5, 3, 4, 8, 2,
    6, 7, 4, 2, 3, 7, 1},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_second_attack, TRUE, SLOT (0), 0, 0,
   "", "!Second Attack!", ""},

  {
   "third attack",
   {103, 103, 36, 18, 29, 103, 30,
    103, 103, 26, 8, 19, 103, 20},
   {0, 0, 9, 4, 7, 0, 3,
    0, 0, 8, 3, 6, 0, 2},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_third_attack, TRUE, SLOT (0), 0, 0,
   "", "!Third Attack!", ""},

  {
   "fourth attack",
   {103, 103, 90, 65, 80, 103, 103,
    103, 103, 80, 55, 70, 103, 103},
   {0, 0, 10, 4, 5, 0, 0,
    0, 0, 9, 3, 4, 0, 0},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_fourth_attack, TRUE, SLOT (0), 0, 0,
   "", "!Fourth Attack!", ""},

  {
   "fifth attack",
   {103, 103, 103, 80, 95, 103, 103,
    103, 103, 103, 70, 85, 103, 103},
   {0, 0, 0, 4, 5, 0, 0,
    0, 0, 0, 3, 4, 0, 0},
   spell_null, TAR_IGNORE, POS_FIGHTING,
   &gsn_fifth_attack, TRUE, SLOT (0), 0, 0,
   "", "!Fifth Attack!", ""},

/* non-combat skills */

  {
   "fast healing",
   {102, 13, 102, 9, 55, 34, 2,
    102, 12, 102, 1, 45, 24, 1},
   {0, 5, 0, 4, 8, 5, 1,
    0, 4, 0, 3, 7, 4, 1},
   spell_null, TAR_IGNORE, POS_SLEEPING,
   &gsn_fast_healing, TRUE, SLOT (0), 0, 0,
   "", "!Fast Healing!", ""},

  {
   "haggle",
   {102, 102, 1, 102, 42, 102, 102,
    102, 102, 1, 102, 32, 102, 102},
   {0, 0, 3, 0, 5, 0, 0,
    0, 0, 2, 0, 4, 0, 0},
   spell_null, TAR_IGNORE, POS_RESTING,
   &gsn_haggle, TRUE, SLOT (0), 0, 0,
   "", "!Haggle!", ""},

  {
   "hide",
   {102, 102, 1, 18, 5, 102, 1,
    102, 102, 1, 8, 1, 102, 1},
   {0, 0, 4, 6, 5, 0, 3,
    0, 0, 3, 5, 4, 0, 2},
   spell_null, TAR_IGNORE, POS_RESTING,
   &gsn_hide, TRUE, SLOT (0), 0, 12,
   "", "!Hide!", ""},

  {
   "lore",
   {15, 15, 9, 102, 5, 14, 102,
    5, 5, 1, 102, 4, 4, 102},
   {6, 6, 4, 0, 4, 5, 0,
    5, 5, 3, 0, 3, 4, 0},
   spell_null, TAR_IGNORE, POS_RESTING,
   &gsn_lore, TRUE, SLOT (0), 0, 36,
   "", "!Lore!", ""},

  {
   "meditation",
   {9, 9, 102, 102, 15, 13, 102,
    8, 1, 102, 102, 5, 3, 70},
   {5, 5, 0, 0, 6, 6, 0,
    4, 4, 0, 0, 5, 5, 6},
   spell_null, TAR_IGNORE, POS_SLEEPING,
   &gsn_meditation, TRUE, SLOT (0), 0, 0,
   "", "Meditation", ""},

  {
   "peek",
   {102, 102, 1, 102, 102, 102, 4,
    90, 102, 1, 102, 102, 102, 1},
   {0, 0, 3, 0, 0, 0, 4,
    6, 0, 2, 0, 0, 0, 3},
   spell_null, TAR_IGNORE, POS_STANDING,
   &gsn_peek, TRUE, SLOT (0), 0, 0,
   "", "!Peek!", ""},

  {
   "pick lock",
   {102, 102, 10, 102, 102, 102, 37,
    102, 102, 6, 102, 102, 102, 27},
   {0, 0, 4, 0, 0, 0, 8,
    0, 0, 3, 0, 0, 0, 7},
   spell_null, TAR_IGNORE, POS_STANDING,
   &gsn_pick_lock, TRUE, SLOT (0), 0, 12,
   "", "!Pick!", ""},

  {
   "sneak",
   {102, 102, 6, 15, 2, 102, 2,
    102, 102, 5, 5, 1, 102, 1},
   {0, 0, 4, 6, 3, 0, 1,
    0, 0, 3, 5, 2, 0, 1},
   spell_null, TAR_IGNORE, POS_STANDING,
   &gsn_sneak, TRUE, SLOT (0), 0, 12,
   "", "You no longer feel stealthy.", ""},

  {
   "steal",
   {102, 102, 7, 102, 102, 102, 102,
    102, 102, 5, 102, 102, 102, 102},
   {0, 0, 4, 0, 0, 0, 0,
    0, 0, 3, 0, 0, 0, 0},
   spell_null, TAR_IGNORE, POS_STANDING,
   &gsn_steal, TRUE, SLOT (0), 0, 24,
   "", "!Steal!", ""},

  {
   "scrolls",
   {2, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1},
   {2, 3, 5, 8, 7, 3, 8,
    1, 2, 4, 7, 6, 2, 7},
   spell_null, TAR_IGNORE, POS_STANDING,
   &gsn_scrolls, TRUE, SLOT (0), 0, 24,
   "", "!Scrolls!", ""},

  {
   "staves",
   {1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1},
   {2, 3, 5, 8, 7, 3, 8,
    1, 2, 4, 7, 6, 2, 7},
   spell_null, TAR_IGNORE, POS_STANDING,
   &gsn_staves, TRUE, SLOT (0), 0, 12,
   "", "!Staves!", ""},

  {
   "track",
   {102, 102, 102, 102, 20, 102, 102,
    102, 102, 102, 102, 10, 102, 102},
   {0, 0, 0, 0, 2, 0, 0,
    0, 0, 0, 0, 1, 0, 0},
   spell_null, TAR_IGNORE, POS_STANDING,
   &gsn_track, TRUE, SLOT (0), 0, 12,
   "", "!Track!", ""},

  {
   "wands",
   {1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1},
   {2, 3, 5, 8, 7, 3, 8,
    1, 2, 5, 7, 6, 2, 7},
   spell_null, TAR_IGNORE, POS_STANDING,
   &gsn_wands, TRUE, SLOT (0), 0, 12,
   "", "!Wands!", ""},

  {
   "recall",
   {1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1},
   {2, 2, 2, 2, 2, 2, 2,
    1, 1, 2, 1, 1, 1, 1},
   spell_null, TAR_IGNORE, POS_STANDING,
   &gsn_recall, TRUE, SLOT (0), 0, 12,
   "", "!Recall!", ""},

  {
    "throw",
    {1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1},
   {2, 2, 2, 2, 2, 2, 2,
    1, 1, 2, 1, 1, 1, 1},
    spell_null, TAR_IGNORE, POS_FIGHTING,
    &gsn_throw, TRUE, SLOT(0), 0, 0,
    "throw", "!throw!", ""},

  {
    "sharpen",
    {102, 102,  60, 65, 55, 102, 102,
     102, 102, 102, 65, 50, 102, 102},
    {  0,   0,   3,  2,  5,   0,   0,
       0,   0,   0,  2,  6,   0,   0},
    spell_null, TAR_IGNORE, POS_RESTING,
    &gsn_sharpen, TRUE, SLOT(0), 0, 36,
    "", "!Sharpen!", ""}
    
};

const struct group_type group_table[MAX_GROUP] = {

  {
   "rom basics",
   {0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0},
   {"scrolls", "staves", "wands", "recall"}
   },

  {
   "mage basics",
   {0, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1},
   {"dagger"}
   },

  {
   "cleric basics",
   {-1, 0, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1},
   {"mace"}
   },

  {
   "thief basics",
   {-1, -1, 0, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1},
   {"dagger", "steal", "pass door"}
   },

  {
   "warrior basics",
   {-1, -1, -1, 0, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1},
   {"sword", "second attack", "dual wield"}
   },

  {
   "ranger basics",
   {-1, -1, -1, -1, 0, -1, -1,
    -1, -1, -1, -1, -1, -1, -1},
   {"spear", "second attack", "track"}
   },

  {
   "druid basics",
   {-1, -1, -1, -1, -1, 0, -1,
    -1, -1, -1, -1, -1, -1, -1},
   {"polearm", "invisibility"}
   },

  {
   "vampire basics",
   {-1, -1, -1, -1, -1, -1, 0,
    -1, -1, -1, -1, -1, -1, -1},
   {"dagger", "hide", "sneak"}
   },

  {
   "wizard basics",
   {-1, -1, -1, -1, -1, -1, -1,
    0, -1, -1, -1, -1, -1, -1},
   {"dagger"}
   },

  {
   "priest basics",
   {-1, -1, -1, -1, -1, -1, -1,
    -1, 0, -1, -1, -1, -1, -1},
   {"mace"}
   },

  {
   "mercenary basics",
   {-1, -1, -1, -1, -1, -1, -1,
    -1, -1, 0, -1, -1, -1, -1},
   {"dagger", "steal", "grip", "pass door"}
   },

  {
   "gladiator basics",
   {-1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, 0, -1, -1, -1},
   {"sword", "second attack", "dual wield"}
   },

  {
   "strider basics",
   {-1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, 0, -1, -1},
   {"dagger", "second attack", "track"}
   },

  {
   "sage basics",
   {-1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, 0, -1},
   {"polearm", "invisibility"}
   },

  {
   "lich basics",
   {-1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, 0},
   {"dagger", "hide", "sneak", "feed"}
   },

  {
   "mage default",
   {40, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1},
   {
    "lore",
    "beguiling",
    "combat",
    "detection",
    "enhancement",
    "illusion",
    "maladictions",
    "protective",
    "shielding",
    "transportation",
    "weather"}
   },

  {
   "cleric default",
   {-1, 40, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1},
   {
    "flail",
    "attack",
    "benedictions",
    "creation",
    "curative",
    "detection",
    "healing",
    "maladictions",
    "protective",
    "shield block",
    "transportation",
    "weather"}
   },

  {
   "thief default",
   {-1, -1, 40, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1},
   {
    "backstab",
    "circle",
    "disarm",
    "dodge",
    "hide",
    "mace",
    "peek",
    "pick lock",
    "sneak",
    "sword",
    "trip",
    "second attack",
    "charm person"}
   },

  {
   "warrior default",
   {-1, -1, -1, 40, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1},
   {
    "weaponsmaster",
    "bash",
    "disarm",
    "enhanced damage",
    "grip",
    "parry",
    "rescue",
    "shield block",
    "third attack",
    "fourth attack"}
   },

  {
   "ranger default",
   {-1, -1, -1, -1, 40, -1, -1,
    -1, -1, -1, -1, -1, -1, -1},
   {
    "weaponsmaster",
    "dirt kick",
    "enhanced damage",
    "envenom",
    "hand to hand",
    "kick",
    "parry",
    "shield block",
    "third attack",
    "curative",
    "healing",
    "transportation",
    "earthquake"}
   },

  {
   "druid default",
   {-1, -1, -1, -1, -1, 40, -1,
    -1, -1, -1, -1, -1, -1, -1},
   {
    "lore",
    "shield block",
    "second attack",
    "attack",
    "benedictions",
    "combat",
    "creation",
    "curative",
    "healing",
    "protective",
    "weather",
    "harm"}
   },

  {
   "vampire default",
   {-1, -1, -1, -1, -1, -1, 40,
    -1, -1, -1, -1, -1, -1, -1},
   {
    "backstab",
    "disarm",
    "dodge",
    "fast healing",
    "feed",
    "hand to hand",
    "shield block",
    "beguiling",
    "detection",
    "enhancement",
    "illusion",
    "maladictions",
    "transportation"}
   },

  {
   "wizard default",
   {-1, -1, -1, -1, -1, -1, -1,
    40, -1, -1, -1, -1, -1, -1},
   {
    "lore",
    "beguiling",
    "combat",
    "detection",
    "enhancement",
    "illusion",
    "maladictions",
    "protective",
    "shielding",
    "transportation",
    "weather",
    "cure blindness"}
   },

  {
   "priest default",
   {-1, -1, -1, -1, -1, -1, -1,
    -1, 40, -1, -1, -1, -1, -1},
   {
    "flail",
    "shield block",
    "attack",
    "benedictions",
    "creation",
    "curative",
    "detection",
    "healing",
    "maladictions",
    "protective",
    "transportation",
    "weather"}
   },

  {
   "mercenary default",
   {-1, -1, -1, -1, -1, -1, -1,
    -1, -1, 40, -1, -1, -1, -1},
   {
    "backstab",
    "circle",
    "disarm",
    "dodge",
    "dual wield",
    "hide",
    "mace",
    "peek",
    "pick lock",
    "sneak",
    "sword",
    "trip",
    "second attack",
    "charm person"}
   },

  {
   "gladiator default",
   {-1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, 40, -1, -1, -1},
   {
    "weaponsmaster",
    "bash",
    "disarm",
    "enhanced damage",
    "parry",
    "rescue",
    "shield block",
    "third attack",
    "fourth attack"}
   },

  {
   "strider default",
   {-1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, 40, -1, -1},
   {
    "weaponsmaster",
    "dual wield",
    "enhanced damage",
    "envenom",
    "hand to hand",
    "kick",
    "parry",
    "third attack",
    "beguiling",
    "curative",
    "healing",
    "protective",
    "transportation"}
   },

  {
   "sage default",
   {-1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, 40, -1},
   {
    "lore",
    "shield block",
    "second attack",
    "attack",
    "benedictions",
    "combat",
    "creation",
    "curative",
    "healing",
    "protective",
    "weather"}
   },

  {
   "lich default",
   {-1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, 40},
   {
    "backstab",
    "detection",
    "disarm",
    "dodge",
    "fast healing",
    "hand to hand",
    "shield block",
    "beguiling",
    "enhancement",
    "illusion",
    "maladictions",
    "transportation"}
   },

  {
   "weaponsmaster",
   {40, 40, 40, 20, 30, 40, 30,
    40, 40, 40, 20, 30, 40, 30},
   {"axe", "dagger", "flail", "mace", "polearm", "spear", "sword",
    "whip"}
   },

  {
   "attack",
   {6, 5, -1, -1, -1, 6, 5,
    5, 4, -1, -1, -1, 5, 4},
   {"demonfire", "dispel evil", "dispel good", "earthquake",
    "flamestrike", "heat metal", "ray of truth"}
   },

  {
   "beguiling",
   {5, 4, -1, -1, 5, -1, 6,
    4, 3, -1, -1, 4, -1, 5},
   {"animate", "calm", "charm person", "resurrect", "sleep"}
   },

  {
   "benedictions",
   {2, 4, -1, -1, 5, 4, 3,
    1, 3, -1, -1, 4, 3, 2},
   {"bless", "calm", "frenzy", "holy word", "remove curse"}
   },

  {
   "combat",
   {5, 5, -1, -1, 4, 6, 5,
    4, 4, -1, -1, 3, 5, 4},
   {"acid blast", "burning hands", "chain lightning", "chill touch",
    "colour spray", "fireball", "lightning bolt", "magic missile",
    "shocking grasp"}
   },

  {
   "creation",
   {4, 4, -1, -1, 7, 4, -1,
    3, 3, -1, -1, 6, 3, -1},
   {"continual light", "create food", "create spring", "create water",
    "create rose", "conjure", "empower", "floating disc"}
   },

  {
   "curative",
   {-1, 4, -1, -1, 7, 5, -1,
    -1, 3, -1, -1, 6, 4, -1},
   {"cure blindness", "cure disease", "cure poison"}
   },

  {
   "detection",
   {4, 3, 6, -1, 3, 6, 4,
    3, 2, 5, -1, 2, 5, 3},
   {"detect evil", "detect good", "detect hidden", "detect invis",
    "detect magic", "detect poison", "farsight", "identify",
    "know alignment", "locate object"}
   },

  {
   "draconian",
   {8, 4, -1, -1, -1, -1, -1,
    7, 3, -1, -1, -1, -1, -1},
   {"acid breath", "fire breath", "frost breath", "gas breath",
    "lightning breath"}
   },

  {
   "enchantment",
   {4, 7, -1, -1, -1, 3, -1,
    3, 6, -1, -1, 3, 3, -1},
   {"enchant armor", "enchant weapon", "fireproof", "recharge"}
   },

  {
   "enhancement",
   {5, 6, -1, -1, 4, 4, 6,
    4, 5, -1, -1, 3, 3, 5},
   {"giant strength", "haste", "infravision", "refresh"}
   },

  {
   "harmful",
   {3, 4, -1, -1, -1, -1, 6,
    2, 3, -1, -1, -1, 4, 5},
   {"cause critical", "cause light", "cause serious", "harm"}
   },

  {
   "healing",
   {-1, 3, -1, -1, 5, 4, -1,
    -1, 2, -1, -1, 4, 3, -1},
   {"cure critical", "cure light", "cure serious", "heal",
    "mass healing"}
   },

  {
   "illusion",
   {4, 6, 7, -1, 7, -1, 4,
    3, 5, 6, -1, 6, -1, 3},
   {"invis", "mass invis", "ventriloquate"}
   },

  {
   "maladictions",
   {5, 5, -1, -1, 6, 8, 5,
    4, 4, -1, -1, 5, 7, 4},
   {"blindness", "change sex", "curse", "energy drain", "plague",
    "poison", "slow", "voodoo", "weaken"}
   },

  {
   "protective",
   {4, 4, -1, -1, 7, 5, 7,
    3, 3, -1, -1, 6, 4, 6},
   {"armor", "cancellation", "dispel magic", "fireproof",
    "protection evil", "protection good", "sanctuary", "shield",
    "stone skin"}
   },

  {
   "shielding",
   {8, 8, -1, -1, -1, 8, -1,
    6, 7, -1, -1, -1, 7, -1},
   {"iceshield", "fireshield", "shockshield"}
   },

  {
   "transportation",
   {4, 4, -1, -1, 5, 7, 4,
    3, 3, -1, -1, 4, 6, 3},
   {"fly", "gate", "nexus", "pass door", "portal", "summon", "teleport",
    "transport", "word of recall"}
   },

  {
   "weather",
   {4, 4, -1, -1, 5, 5, 7,
    3, 3, -1, -1, 4, 4, 6},
   {"call lightning", "control weather", "faerie fire", "faerie fog",
    "lightning bolt"}
   }

};
