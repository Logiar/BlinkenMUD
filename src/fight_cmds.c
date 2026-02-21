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
#include <string.h>
#include <time.h>
#include "merc.h"
#include "tables.h"


/* command procedures needed */
DECLARE_DO_FUN (do_backstab);
DECLARE_DO_FUN (do_emote);
DECLARE_DO_FUN (do_berserk);
DECLARE_DO_FUN (do_bash);
DECLARE_DO_FUN (do_trip);
DECLARE_DO_FUN (do_dirt);
DECLARE_DO_FUN (do_flee);
DECLARE_DO_FUN (do_kick);
DECLARE_DO_FUN (do_disarm);
DECLARE_DO_FUN (do_get);
DECLARE_DO_FUN (do_recall);
DECLARE_DO_FUN (do_yell);
DECLARE_DO_FUN (do_sacrifice);
DECLARE_DO_FUN (do_circle);
DECLARE_DO_FUN (do_feed);
DECLARE_DO_FUN (do_gouge);
DECLARE_DO_FUN (do_vdpi);
DECLARE_DO_FUN (do_vdtr);
DECLARE_DO_FUN (do_vdth);
DECLARE_DO_FUN (do_look);


/*
 * Local functions.
 */
void check_assist (CHAR_DATA * ch, CHAR_DATA * victim);
bool check_dodge (CHAR_DATA * ch, CHAR_DATA * victim);
bool check_parry (CHAR_DATA * ch, CHAR_DATA * victim);
bool check_shield_block (CHAR_DATA * ch, CHAR_DATA * victim);
void dam_message (CHAR_DATA * ch, CHAR_DATA * victim, int dam,
			int dt, bool immune);
void death_cry (CHAR_DATA * ch);
void group_gain (CHAR_DATA * ch, CHAR_DATA * victim);
int xp_compute (CHAR_DATA * gch, CHAR_DATA * victim, int total_levels);
bool is_safe (CHAR_DATA * ch, CHAR_DATA * victim);
bool is_safe_mock (CHAR_DATA * ch, CHAR_DATA * victim);
bool is_voodood (CHAR_DATA * ch, CHAR_DATA * victim);
void make_corpse (CHAR_DATA * ch, CHAR_DATA * killer);
void one_hit
(CHAR_DATA * ch, CHAR_DATA * victim, int dt, bool secondary);
void one_hit_mock
(CHAR_DATA * ch, CHAR_DATA * victim, int dt, bool secondary);
void mob_hit (CHAR_DATA * ch, CHAR_DATA * victim, int dt);
void raw_kill (CHAR_DATA * victim, CHAR_DATA * killer);
void set_fighting (CHAR_DATA * ch, CHAR_DATA * victim);
void disarm (CHAR_DATA * ch, CHAR_DATA * victim);



/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void
do_berserk (CHAR_DATA * ch, char *argument)
{
  int chance, hp_percent;

  if ((chance = get_skill (ch, gsn_berserk)) == 0
      || (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_BERSERK))
      || (!IS_NPC (ch)
	  && ch->level < skill_table[gsn_berserk].skill_level[ch->class]))
    {
      send_to_char
	("`hYou turn `rred`h in the face, but nothing happens.`x\n\r", ch);
      return;
    }

  if (IS_AFFECTED (ch, AFF_BERSERK) || is_affected (ch, gsn_berserk)
      || is_affected (ch, skill_lookup ("frenzy")))
    {
      send_to_char ("`hYou get a little madder.`x\n\r", ch);
      return;
    }

  if (IS_AFFECTED (ch, AFF_CALM))
    {
      send_to_char ("`hYou're feeling to mellow to berserk.`x\n\r", ch);
      return;
    }

  if (ch->mana < 50)
    {
      send_to_char ("`hYou can't get up enough energy.`x\n\r", ch);
      return;
    }

  /* modifiers */

  /* fighting */
  if (ch->position == POS_FIGHTING)
    chance += 10;

  /* damage -- below 50% of hp helps, above hurts */
  hp_percent = 100 * ch->hit / ch->max_hit;
  chance += 25 - hp_percent / 2;

  if (number_percent () < chance)
    {
      AFFECT_DATA af;

      WAIT_STATE (ch, PULSE_VIOLENCE);
      ch->mana -= 50;
      ch->move /= 2;

      /* heal a little damage */
      ch->hit += ch->level * 2;
      ch->hit = UMIN (ch->hit, ch->max_hit);

      send_to_char ("`hYour pulse races as you are consumed by `rrage!`x\n\r",
		    ch);
      act ("`k$n gets a `cw`gi`rl`yd`k look in $s eyes.`x", ch, NULL, NULL,
	   TO_ROOM);
      check_improve (ch, gsn_berserk, TRUE, 2);

      af.where = TO_AFFECTS;
      af.type = gsn_berserk;
      af.level = ch->level;
      af.duration = number_fuzzy (ch->level / 8);
      af.modifier = UMAX (1, ch->level / 5);
      af.bitvector = AFF_BERSERK;

      af.location = APPLY_HITROLL;
      affect_to_char (ch, &af);

      af.location = APPLY_DAMROLL;
      affect_to_char (ch, &af);

      af.modifier = UMAX (10, 10 * (ch->level / 5));
      af.location = APPLY_AC;
      affect_to_char (ch, &af);
    }

  else
    {
      WAIT_STATE (ch, 3 * PULSE_VIOLENCE);
      ch->mana -= 25;
      ch->move /= 2;

      send_to_char ("`hYour pulse speeds up, but nothing happens.`x\n\r", ch);
      check_improve (ch, gsn_berserk, FALSE, 2);
    }
}

void
do_voodoo (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *doll;

  if (IS_NPC (ch))
    return;

  doll = get_eq_char (ch, WEAR_HOLD);
  if (doll == NULL || (doll->pIndexData->vnum != OBJ_VNUM_VOODOO))
    {
      send_to_char ("You are not holding a voodoo doll.\n\r", ch);
      return;
    }

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Syntax: voodoo <action>\n\r", ch);
      send_to_char ("Actions: pin trip throw\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "pin"))
    {
      do_vdpi (ch, doll->name);
      return;
    }

  if (!str_cmp (arg, "trip"))
    {
      do_vdtr (ch, doll->name);
      return;
    }

  if (!str_cmp (arg, "throw"))
    {
      do_vdth (ch, doll->name);
      return;
    }

  do_voodoo (ch, "");
}

void
do_vdpi (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  DESCRIPTOR_DATA *d;
  AFFECT_DATA af;
  bool found = FALSE;

  argument = one_argument (argument, arg1);

  for (d = descriptor_list; d != NULL; d = d->next)
    {
      CHAR_DATA *wch;

      if (d->connected != CON_PLAYING || !can_see (ch, d->character))
	continue;

      wch = (d->original != NULL) ? d->original : d->character;

      if (!can_see (ch, wch))
	continue;

      if (!str_cmp (arg1, wch->name) && !found)
	{
	  if (IS_NPC (wch))
	    continue;

	  if (IS_IMMORTAL (wch) && (wch->level > ch->level))
	    {
	      send_to_char ("That's not a good idea.\n\r", ch);
	      return;
	    }

	  if ((wch->level < 20) && !IS_IMMORTAL (ch))
	    {
	      send_to_char ("They are a little too young for that.\n\r", ch);
	      return;
	    }

	  if (IS_SHIELDED (wch, SHD_PROTECT_VOODOO))
	    {
	      send_to_char
		("They are still realing from a previous voodoo.\n\r", ch);
	      return;
	    }

	  found = TRUE;

	  send_to_char ("You stick a pin into your voodoo doll.\n\r", ch);
	  act ("$n sticks a pin into a voodoo doll.", ch, NULL, NULL,
	       TO_ROOM);
	  send_to_char
	    ("`RYou double over with a sudden pain in your gut!`x\n\r", wch);
	  act ("$n suddenly doubles over with a look of extreme pain!", wch,
	       NULL, NULL, TO_ROOM);
	  af.where = TO_SHIELDS;
	  af.type = skill_lookup ("protection voodoo");
	  af.level = wch->level;
	  af.duration = 1;
	  af.location = APPLY_NONE;
	  af.modifier = 0;
	  af.bitvector = SHD_PROTECT_VOODOO;
	  affect_to_char (wch, &af);
	  return;
	}
    }
  send_to_char ("Your victim doesn't seem to be in the realm.\n\r", ch);
  return;
}

void
do_vdtr (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  DESCRIPTOR_DATA *d;
  AFFECT_DATA af;
  bool found = FALSE;

  argument = one_argument (argument, arg1);

  for (d = descriptor_list; d != NULL; d = d->next)
    {
      CHAR_DATA *wch;

      if (d->connected != CON_PLAYING || !can_see (ch, d->character))
	continue;

      wch = (d->original != NULL) ? d->original : d->character;

      if (!can_see (ch, wch))
	continue;

      if (!str_cmp (arg1, wch->name) && !found)
	{
	  if (IS_NPC (wch))
	    continue;

	  if (IS_IMMORTAL (wch) && (wch->level > ch->level))
	    {
	      send_to_char ("That's not a good idea.\n\r", ch);
	      return;
	    }

	  if ((wch->level < 20) && !IS_IMMORTAL (ch))
	    {
	      send_to_char ("They are a little too young for that.\n\r", ch);
	      return;
	    }

	  if (IS_SHIELDED (wch, SHD_PROTECT_VOODOO))
	    {
	      send_to_char
		("They are still realing from a previous voodoo.\n\r", ch);
	      return;
	    }

	  found = TRUE;

	  send_to_char ("You slam your voodoo doll against the ground.\n\r",
			ch);
	  act ("$n slams a voodoo doll against the ground.", ch, NULL, NULL,
	       TO_ROOM);
	  send_to_char ("`RYour feet slide out from under you!`x\n\r", wch);
	  send_to_char ("`RYou hit the ground face first!`x\n\r", wch);
	  act
	    ("$n trips over $s own feet, and does a nose dive into the ground!",
	     wch, NULL, NULL, TO_ROOM);
	  af.where = TO_SHIELDS;
	  af.type = skill_lookup ("protection voodoo");
	  af.level = wch->level;
	  af.duration = 1;
	  af.location = APPLY_NONE;
	  af.modifier = 0;
	  af.bitvector = SHD_PROTECT_VOODOO;
	  affect_to_char (wch, &af);
	  return;
	}
    }
  send_to_char ("Your victim doesn't seem to be in the realm.\n\r", ch);
  return;
}

void
do_vdth (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;
  AFFECT_DATA af;
  ROOM_INDEX_DATA *was_in;
  ROOM_INDEX_DATA *now_in;
  bool found = FALSE;
  int attempt;

  argument = one_argument (argument, arg1);

  for (d = descriptor_list; d != NULL; d = d->next)
    {
      CHAR_DATA *wch;

      if (d->connected != CON_PLAYING || !can_see (ch, d->character))
	continue;

      wch = (d->original != NULL) ? d->original : d->character;

      if (!can_see (ch, wch))
	continue;

      if (!str_cmp (arg1, wch->name) && !found)
	{
	  if (IS_NPC (wch))
	    continue;

	  if (IS_IMMORTAL (wch) && (wch->level > ch->level))
	    {
	      send_to_char ("That's not a good idea.\n\r", ch);
	      return;
	    }

	  if ((wch->level < 20) && !IS_IMMORTAL (ch))
	    {
	      send_to_char ("They are a little too young for that.\n\r", ch);
	      return;
	    }

	  if (IS_SHIELDED (wch, SHD_PROTECT_VOODOO))
	    {
	      send_to_char
		("They are still realing from a previous voodoo.\n\r", ch);
	      return;
	    }

	  found = TRUE;

	  send_to_char ("You toss your voodoo doll into the air.\n\r", ch);
	  act ("$n tosses a voodoo doll into the air.", ch, NULL, NULL,
	       TO_ROOM);
	  af.where = TO_SHIELDS;
	  af.type = skill_lookup ("protection voodoo");
	  af.level = wch->level;
	  af.duration = 1;
	  af.location = APPLY_NONE;
	  af.modifier = 0;
	  af.bitvector = SHD_PROTECT_VOODOO;
	  affect_to_char (wch, &af);
	  if ((wch->fighting != NULL) || (number_percent () < 25))
	    {
	      send_to_char
		("`RA sudden gust of wind throws you through the air!`x\n\r",
		 wch);
	      send_to_char
		("`RYou slam face first into the nearest wall!`x\n\r", wch);
	      act
		("A sudden gust of wind picks up $n and throws $m into a wall!",
		 wch, NULL, NULL, TO_ROOM);
	      return;
	    }
	  wch->position = POS_STANDING;
	  was_in = wch->in_room;
	  for (attempt = 0; attempt < 6; attempt++)
	    {
	      EXIT_DATA *pexit;
	      int door;

	      door = number_door ();
	      if ((pexit = was_in->exit[door]) == 0
		  || pexit->u1.to_room == NULL
		  || IS_SET (pexit->exit_info, EX_CLOSED)
		  || (IS_NPC (wch)
		      && IS_SET (pexit->u1.to_room->room_flags, ROOM_NO_MOB)))
		continue;

	      move_char (wch, door, FALSE, TRUE);
	      if ((now_in = wch->in_room) == was_in)
		continue;

	      wch->in_room = was_in;
	      snprintf (buf, sizeof (buf),
		       "A sudden gust of wind picks up $n and throws $m to the %s.",
		       dir_name[door]);
	      act (buf, wch, NULL, NULL, TO_ROOM);
	      send_to_char
		("`RA sudden gust of wind throws you through the air!`x\n\r",
		 wch);
	      wch->in_room = now_in;
	      act ("$n sails into the room and slams face first into a wall!",
		   wch, NULL, NULL, TO_ROOM);
	      do_look (wch, "auto");
	      send_to_char
		("`RYou slam face first into the nearest wall!`x\n\r", wch);
	      return;
	    }
	  send_to_char
	    ("`RA sudden gust of wind throws you through the air!`x\n\r",
	     wch);
	  send_to_char ("`RYou slam face first into the nearest wall!`x\n\r",
			wch);
	  act ("A sudden gust of wind picks up $n and throws $m into a wall!",
	       wch, NULL, NULL, TO_ROOM);
	  return;
	}
    }
  send_to_char ("Your victim doesn't seem to be in the realm.\n\r", ch);
  return;
}

void
do_bash (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;

  one_argument (argument, arg);

  if ((chance = get_skill (ch, gsn_bash)) == 0
      || (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_BASH))
      || (!IS_NPC (ch)
	  && ch->level < skill_table[gsn_bash].skill_level[ch->class]))
    {
      send_to_char ("Bashing? What's that?\n\r", ch);
      return;
    }

  if (arg[0] == '\0')
    {
      victim = ch->fighting;
      if (victim == NULL)
	{
	  send_to_char ("But you aren't fighting anyone!\n\r", ch);
	  return;
	}
    }

  else if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (victim->position < POS_FIGHTING)
    {
      act ("You'll have to let $M get back up first.", ch, NULL, victim,
	   TO_CHAR);
      return;
    }

  if (victim == ch)
    {
      send_to_char ("You try to bash your brains out, but fail.\n\r", ch);
      return;
    }

  if (is_safe (ch, victim))
    return;

  if (IS_NPC (victim) &&
      victim->fighting != NULL && !is_same_group (ch, victim->fighting))
    {
      send_to_char ("Kill stealing is not permitted.\n\r", ch);
      return;
    }

  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
    {
      act ("But $N is your friend!", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (ch->stunned)
    {
      send_to_char ("You're still a little woozy.\n\r", ch);
      return;
    }

  if (!can_see (ch, victim))
    {
      send_to_char
	("You get a running start, and slam right into a wall.\n\r", ch);
      return;
    }

  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }

  /* modifiers */

  /* size  and weight */
  chance += ch->carry_weight / 250;
  chance -= victim->carry_weight / 200;

  if (ch->size < victim->size)
    chance += (ch->size - victim->size) * 15;
  else
    chance += (ch->size - victim->size) * 10;


  /* stats */
  chance += get_curr_stat (ch, STAT_STR);
  chance -= (get_curr_stat (victim, STAT_DEX) * 4) / 3;
  chance -= GET_AC (victim, AC_BASH) / 25;
  /* speed */
  if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
    chance += 10;
  if (IS_SET (victim->off_flags, OFF_FAST) || IS_AFFECTED (victim, AFF_HASTE))
    chance -= 30;

  /* level */
  chance += (ch->level - victim->level);

  if (!IS_NPC (victim) && chance < get_skill (victim, gsn_dodge))
    {				/*
				   act("`i$n tries to bash you, but you dodge it.`x",ch,NULL,victim,TO_VICT);
				   act("`h$N dodges your bash, you fall flat on your face.`x",ch,NULL,victim,TO_CHAR);
				   WAIT_STATE(ch,skill_table[gsn_bash].beats);
				   return; */
      chance -= 3 * (get_skill (victim, gsn_dodge) - chance);
    }

  /* now the attack */
  if (number_percent () < chance)
    {

      act ("`i$n sends you sprawling with a powerful bash!`x",
	   ch, NULL, victim, TO_VICT);
      act ("`hYou slam into $N, and send $M flying!`x", ch, NULL, victim,
	   TO_CHAR);
      act ("`k$n sends $N sprawling with a powerful bash.`x", ch, NULL,
	   victim, TO_NOTVICT);
      check_improve (ch, gsn_bash, TRUE, 1);

      DAZE_STATE (victim, 3 * PULSE_VIOLENCE);
      WAIT_STATE (ch, skill_table[gsn_bash].beats);
      victim->position = POS_RESTING;
      damage (ch, victim, number_range (2, 2 + 2 * ch->size + chance / 20),
	      gsn_bash, DAM_BASH, FALSE);
      chance = (get_skill (ch, gsn_stun) / 5);
      if (number_percent () < chance)
	{
	  chance = (get_skill (ch, gsn_stun) / 5);
	  if (number_percent () < chance)
	    {
	      victim->stunned = 2;
	    }
	  else
	    {
	      victim->stunned = 1;
	    }
	  act ("`iYou are stunned, and have trouble getting back up!`x",
	       ch, NULL, victim, TO_VICT);
	  act ("`h$N is stunned by your bash!`x", ch, NULL, victim, TO_CHAR);
	  act ("`k$N is having trouble getting back up.`x",
	       ch, NULL, victim, TO_NOTVICT);
	  check_improve (ch, gsn_stun, TRUE, 1);
	}
    }
  else
    {
      damage (ch, victim, 0, gsn_bash, DAM_BASH, FALSE);
      act ("`hYou fall flat on your face!`x", ch, NULL, victim, TO_CHAR);
      act ("`k$n falls flat on $s face.`x", ch, NULL, victim, TO_NOTVICT);
      act ("`iYou evade $n's bash, causing $m to fall flat on $s face.`x",
	   ch, NULL, victim, TO_VICT);
      check_improve (ch, gsn_bash, FALSE, 1);
      ch->position = POS_RESTING;
      WAIT_STATE (ch, skill_table[gsn_bash].beats * 3 / 2);
    }
}

void
do_dirt (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;

  one_argument (argument, arg);

  if ((chance = get_skill (ch, gsn_dirt)) == 0
      || (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_KICK_DIRT))
      || (!IS_NPC (ch)
	  && ch->level < skill_table[gsn_dirt].skill_level[ch->class]))
    {
      send_to_char ("`hYou get your feet dirty.`x\n\r", ch);
      return;
    }

  if (arg[0] == '\0')
    {
      victim = ch->fighting;
      if (victim == NULL)
	{
	  send_to_char ("But you aren't in combat!\n\r", ch);
	  return;
	}
    }

  else if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (IS_AFFECTED (victim, AFF_BLIND))
    {
      act ("`h$E's already been blinded.`x", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (victim == ch)
    {
      send_to_char ("Very funny.\n\r", ch);
      return;
    }

  if (is_safe (ch, victim))
    return;

  if (IS_NPC (victim) &&
      victim->fighting != NULL && !is_same_group (ch, victim->fighting))
    {
      send_to_char ("Kill stealing is not permitted.\n\r", ch);
      return;
    }

  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
    {
      act ("But $N is such a good friend!", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (ch->stunned)
    {
      send_to_char ("You're still a little woozy.\n\r", ch);
      return;
    }

  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }

  /* modifiers */

  /* dexterity */
  chance += get_curr_stat (ch, STAT_DEX);
  chance -= 2 * get_curr_stat (victim, STAT_DEX);

  /* speed  */
  if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
    chance += 10;
  if (IS_SET (victim->off_flags, OFF_FAST) || IS_AFFECTED (victim, AFF_HASTE))
    chance -= 25;

  /* level */
  chance += (ch->level - victim->level) * 2;

  /* sloppy hack to prevent false zeroes */
  if (chance % 5 == 0)
    chance += 1;

  /* terrain */

  switch (ch->in_room->sector_type)
    {
    case (SECT_INSIDE):
      chance -= 20;
      break;
    case (SECT_CITY):
      chance -= 10;
      break;
    case (SECT_FIELD):
      chance += 5;
      break;
    case (SECT_FOREST):
      break;
    case (SECT_HILLS):
      break;
    case (SECT_MOUNTAIN):
      chance -= 10;
      break;
    case (SECT_WATER_SWIM):
      chance = 0;
      break;
    case (SECT_WATER_NOSWIM):
      chance = 0;
      break;
    case (SECT_AIR):
      chance = 0;
      break;
    case (SECT_DESERT):
      chance += 10;
      break;
    }

  if (chance == 0)
    {
      send_to_char ("`hThere isn't any dirt to kick.`x\n\r", ch);
      return;
    }

  /* now the attack */
  if (number_percent () < chance)
    {
      AFFECT_DATA af;
      act ("`k$n is blinded by the dirt in $s eyes!`x", victim, NULL, NULL,
	   TO_ROOM);
      act ("`i$n kicks dirt in your eyes!`x", ch, NULL, victim, TO_VICT);
      damage (ch, victim, number_range (2, 5), gsn_dirt, DAM_NONE, FALSE);
      send_to_char ("`DYou can't see a thing!`x\n\r", victim);
      check_improve (ch, gsn_dirt, TRUE, 2);
      WAIT_STATE (ch, skill_table[gsn_dirt].beats);

      af.where = TO_AFFECTS;
      af.type = gsn_dirt;
      af.level = ch->level;
      af.duration = 0;
      af.location = APPLY_HITROLL;
      af.modifier = -4;
      af.bitvector = AFF_BLIND;

      affect_to_char (victim, &af);
    }
  else
    {
      damage (ch, victim, 0, gsn_dirt, DAM_NONE, TRUE);
      check_improve (ch, gsn_dirt, FALSE, 2);
      WAIT_STATE (ch, skill_table[gsn_dirt].beats);
    }
}

void
do_gouge (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;

  one_argument (argument, arg);

  if ((chance = get_skill (ch, gsn_gouge)) == 0
      || (!IS_NPC (ch)
	  && ch->level < skill_table[gsn_gouge].skill_level[ch->class]))
    {
      send_to_char ("Gouge?  What's that?`x\n\r", ch);
      return;
    }

  if (arg[0] == '\0')
    {
      victim = ch->fighting;
      if (victim == NULL)
	{
	  send_to_char ("But you aren't in combat!\n\r", ch);
	  return;
	}
    }

  else if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (IS_AFFECTED (victim, AFF_BLIND))
    {
      act ("`h$E's already been blinded.`x", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (victim == ch)
    {
      send_to_char ("Very funny.\n\r", ch);
      return;
    }

  if (is_safe (ch, victim))
    return;

  if (IS_NPC (victim) &&
      victim->fighting != NULL && !is_same_group (ch, victim->fighting))
    {
      send_to_char ("Kill stealing is not permitted.\n\r", ch);
      return;
    }

  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
    {
      act ("But $N is such a good friend!", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (ch->stunned)
    {
      send_to_char ("You're still a little woozy.\n\r", ch);
      return;
    }

  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }

  /* modifiers */

  /* dexterity */
  chance += get_curr_stat (ch, STAT_DEX);
  chance -= 2 * get_curr_stat (victim, STAT_DEX);

  /* speed  */
  if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
    chance += 10;
  if (IS_SET (victim->off_flags, OFF_FAST) || IS_AFFECTED (victim, AFF_HASTE))
    chance -= 25;

  /* level */
  chance += (ch->level - victim->level) * 2;

  /* sloppy hack to prevent false zeroes */
  if (chance % 5 == 0)
    chance += 1;

  /* now the attack */
  if (number_percent () < chance)
    {
      AFFECT_DATA af;
      act ("`k$n is blinded by a poke in the eyes!`x", victim, NULL, NULL,
	   TO_ROOM);
      act ("`i$n gouges at your eyes!`x", ch, NULL, victim, TO_VICT);
      damage (ch, victim, number_range (2, 5), gsn_gouge, DAM_NONE, FALSE);
      send_to_char ("`DYou see nothing but stars!`x\n\r", victim);
      check_improve (ch, gsn_gouge, TRUE, 2);
      WAIT_STATE (ch, skill_table[gsn_gouge].beats);

      af.where = TO_AFFECTS;
      af.type = gsn_gouge;
      af.level = ch->level;
      af.duration = 0;
      af.location = APPLY_HITROLL;
      af.modifier = -4;
      af.bitvector = AFF_BLIND;

      affect_to_char (victim, &af);
    }
  else
    {
      damage (ch, victim, 0, gsn_gouge, DAM_NONE, TRUE);
      check_improve (ch, gsn_gouge, FALSE, 2);
      WAIT_STATE (ch, skill_table[gsn_gouge].beats);
    }
}

void
do_trip (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;

  one_argument (argument, arg);

  if ((chance = get_skill (ch, gsn_trip)) == 0
      || (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_TRIP))
      || (!IS_NPC (ch)
	  && ch->level < skill_table[gsn_trip].skill_level[ch->class]))
    {
      send_to_char ("Tripping?  What's that?\n\r", ch);
      return;
    }


  if (arg[0] == '\0')
    {
      victim = ch->fighting;
      if (victim == NULL)
	{
	  send_to_char ("But you aren't fighting anyone!\n\r", ch);
	  return;
	}
    }

  else if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (is_safe (ch, victim))
    return;

  if (IS_NPC (victim) &&
      victim->fighting != NULL && !is_same_group (ch, victim->fighting))
    {
      send_to_char ("Kill stealing is not permitted.\n\r", ch);
      return;
    }

  if (ch->stunned)
    {
      send_to_char ("You're still a little woozy.\n\r", ch);
      return;
    }

  if (IS_AFFECTED (victim, AFF_FLYING))
    {
      act ("`h$S feet aren't on the ground.`x", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (victim->position < POS_FIGHTING)
    {
      act ("`h$N is already down.`c", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (victim == ch)
    {
      send_to_char ("`hYou fall flat on your face!`x\n\r", ch);
      WAIT_STATE (ch, 2 * skill_table[gsn_trip].beats);
      act ("`k$n trips over $s own feet!`x", ch, NULL, NULL, TO_ROOM);
      return;
    }

  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
    {
      act ("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
      return;
    }

  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }

  /* modifiers */

  /* size */
  if (ch->size < victim->size)
    chance += (ch->size - victim->size) * 10;	/* bigger = harder to trip */

  /* dex */
  chance += get_curr_stat (ch, STAT_DEX);
  chance -= get_curr_stat (victim, STAT_DEX) * 3 / 2;

  /* speed */
  if (IS_SET (ch->off_flags, OFF_FAST) || IS_AFFECTED (ch, AFF_HASTE))
    chance += 10;
  if (IS_SET (victim->off_flags, OFF_FAST) || IS_AFFECTED (victim, AFF_HASTE))
    chance -= 20;

  /* level */
  chance += (ch->level - victim->level) * 2;


  /* now the attack */
  if (number_percent () < chance)
    {
      act ("`i$n trips you and you go down!`x", ch, NULL, victim, TO_VICT);
      act ("`hYou trip $N and $N goes down!`x", ch, NULL, victim, TO_CHAR);
      act ("`k$n trips $N, sending $M to the ground.`x", ch, NULL, victim,
	   TO_NOTVICT);
      check_improve (ch, gsn_trip, TRUE, 1);

      DAZE_STATE (victim, 2 * PULSE_VIOLENCE);
      WAIT_STATE (ch, skill_table[gsn_trip].beats);
      victim->position = POS_RESTING;
      damage (ch, victim, number_range (2, 2 + 2 * victim->size), gsn_trip,
	      DAM_BASH, TRUE);
    }
  else
    {
      damage (ch, victim, 0, gsn_trip, DAM_BASH, TRUE);
      WAIT_STATE (ch, skill_table[gsn_trip].beats * 2 / 3);
      check_improve (ch, gsn_trip, FALSE, 1);
    }
}



void
do_kill (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Kill whom?\n\r", ch);
      return;
    }

  if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (victim == ch)
    {
      send_to_char ("`hYou hit yourself.  `z`COuch!`x\n\r", ch);
      multi_hit (ch, ch, TYPE_UNDEFINED);
      return;
    }

  if (is_safe (ch, victim))
    return;

  if (!IS_NPC (victim))
    {
      if (!IS_SET (victim->act, PLR_TWIT))
	{
	  send_to_char ("You must MURDER a player.\n\r", ch);
	  return;
	}
    }
  if (victim->fighting != NULL && !is_same_group (ch, victim->fighting))
    {
      send_to_char ("Kill stealing is not permitted.\n\r", ch);
      return;
    }

  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
    {
      act ("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (ch->position == POS_FIGHTING)
    {
      send_to_char ("You do the best you can!\n\r", ch);
      return;
    }

  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }

  WAIT_STATE (ch, 1 * PULSE_VIOLENCE);
  multi_hit (ch, victim, TYPE_UNDEFINED);
  return;
}

void
do_mock (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Mock hit whom?\n\r", ch);
      return;
    }

  if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (is_safe_mock (ch, victim))
    return;

  if (victim->fighting != NULL)
    {
      send_to_char ("`gThis player is busy at the moment.`x\n\r", ch);
      return;
    }

  if (ch->position == POS_FIGHTING)
    {
      send_to_char ("`gYou've already got your hands full!`x\n\r", ch);
      return;
    }

  one_hit_mock (ch, victim, TYPE_UNDEFINED, FALSE);

  return;
}

void
do_murde (CHAR_DATA * ch, char *argument)
{
  send_to_char ("If you want to `RMURDER`x, spell it out.\n\r", ch);
  return;
}



void
do_murder (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Murder whom?\n\r", ch);
      return;
    }

  if (IS_NPC (ch))
    return;

  if (IS_AFFECTED (ch, AFF_CHARM))
    return;

  if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (victim == ch)
    {
      send_to_char ("Suicide is a mortal sin.\n\r", ch);
      return;
    }

  if (is_safe (ch, victim))
    return;

  if (IS_NPC (victim) &&
      victim->fighting != NULL && !is_same_group (ch, victim->fighting))
    {
      send_to_char ("Kill stealing is not permitted.\n\r", ch);
      return;
    }

  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
    {
      act ("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (ch->position == POS_FIGHTING)
    {
      send_to_char ("You do the best you can!\n\r", ch);
      return;
    }

  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }

  WAIT_STATE (ch, 1 * PULSE_VIOLENCE);
  if (IS_NPC (ch))
    snprintf (buf, sizeof (buf), "Help! I am being attacked by %s!", ch->short_descr);
  else
    snprintf (buf, sizeof (buf), "Help!  I am being attacked by %s!", ch->name);
  do_yell (victim, buf);
  multi_hit (ch, victim, TYPE_UNDEFINED);
  return;
}



void
do_backstab (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Backstab whom?\n\r", ch);
      return;
    }

  if (ch->fighting != NULL)
    {
      send_to_char ("`hYou're facing the wrong end.`x\n\r", ch);
      return;
    }

  else if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (victim == ch)
    {
      send_to_char ("How can you sneak up on yourself?\n\r", ch);
      return;
    }

  if (is_safe (ch, victim))
    return;

  if (IS_NPC (victim) &&
      victim->fighting != NULL && !is_same_group (ch, victim->fighting))
    {
      send_to_char ("Kill stealing is not permitted.\n\r", ch);
      return;
    }

  if ((obj = get_eq_char (ch, WEAR_WIELD)) == NULL)
    {
      send_to_char ("`hYou need to wield a primary weapon to backstab.`x\n\r",
		    ch);
      return;
    }

  if (victim->hit < victim->max_hit / 3)
    {
      act ("$N is hurt and suspicious ... you can't sneak up.",
	   ch, NULL, victim, TO_CHAR);
      return;
    }

  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }

  WAIT_STATE (ch, skill_table[gsn_backstab].beats);
  if (number_percent () < get_skill (ch, gsn_backstab)
      || (get_skill (ch, gsn_backstab) >= 2 && !IS_AWAKE (victim)))
    {
      check_improve (ch, gsn_backstab, TRUE, 1);
      multi_hit (ch, victim, gsn_backstab);
    }
  else
    {
      check_improve (ch, gsn_backstab, FALSE, 1);
      damage (ch, victim, 0, gsn_backstab, DAM_NONE, TRUE);
    }

  return;
}

void
do_circle (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;

  if (get_skill (ch, gsn_circle) == 0
      || (!IS_NPC (ch)
	  && ch->level < skill_table[gsn_circle].skill_level[ch->class]))
    {
      send_to_char ("Circle? What's that?\n\r", ch);
      return;
    }

  if ((victim = ch->fighting) == NULL)
    {
      send_to_char ("You aren't fighting anyone.\n\r", ch);
      return;
    }

  if ((obj = get_eq_char (ch, WEAR_WIELD)) == NULL)
    {
      send_to_char ("You need to wield a primary weapon to circle.\n\r", ch);
      return;
    }

  if (victim->hit < victim->max_hit / 6)
    {
      act ("$N is hurt and suspicious ... you can't sneak around.",
	   ch, NULL, victim, TO_CHAR);
      return;
    }

  if (ch->stunned)
    {
      send_to_char ("You're still a little woozy.\n\r", ch);
      return;
    }

  if (!can_see (ch, victim))
    {
      send_to_char ("You stumble blindly into a wall.\n\r", ch);
      return;
    }

  WAIT_STATE (ch, skill_table[gsn_circle].beats);
  if (number_percent () < get_skill (ch, gsn_circle)
      || (get_skill (ch, gsn_circle) >= 2 && !IS_AWAKE (victim)))
    {
      check_improve (ch, gsn_circle, TRUE, 1);
      act ("`i$n circles around behind you.`x", ch, NULL, victim, TO_VICT);
      act ("`hYou circle around $N.`x", ch, NULL, victim, TO_CHAR);
      act ("`k$n circles around behind $N.`x", ch, NULL, victim, TO_NOTVICT);
      multi_hit (ch, victim, gsn_circle);
    }
  else
    {
      check_improve (ch, gsn_circle, FALSE, 1);
      act ("`i$n tries to circle around you.`x", ch, NULL, victim, TO_VICT);
      act ("`h$N circles with you.`x", ch, NULL, victim, TO_CHAR);
      act ("`k$n tries to circle around $N.`x", ch, NULL, victim, TO_NOTVICT);
      damage (ch, victim, 0, gsn_circle, DAM_NONE, TRUE);
    }

  return;
}

void
do_feed (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  int dam;

  if (get_skill (ch, gsn_feed) == 0
      || (!IS_NPC (ch)
	  && ch->level < skill_table[gsn_feed].skill_level[ch->class]))
    {
      send_to_char ("Feed? What's that?\n\r", ch);
      return;
    }

  if ((victim = ch->fighting) == NULL)
    {
      send_to_char ("You aren't fighting anyone.\n\r", ch);
      return;
    }

  if (victim->hit < victim->max_hit / 6)
    {
      act ("$N is hurt and suspicious ... you can't get close enough.",
	   ch, NULL, victim, TO_CHAR);
      return;
    }

  if (ch->stunned)
    {
      send_to_char ("You're still a little woozy.\n\r", ch);
      return;
    }

  WAIT_STATE (ch, skill_table[gsn_feed].beats);
  if (number_percent () < get_skill (ch, gsn_feed) / 3
      || (get_skill (ch, gsn_feed) >= 2 && !IS_AWAKE (victim)))
    {
      check_improve (ch, gsn_feed, TRUE, 1);
      act ("`i$n bites you.`x", ch, NULL, victim, TO_VICT);
      act ("`hYou bite $N.`x", ch, NULL, victim, TO_CHAR);
      act ("`k$n bites $N.`x", ch, NULL, victim, TO_NOTVICT);
      dam = number_range ((((ch->level / 2) + (victim->level / 2)) / 3),
			  (((ch->level / 2) + (victim->level / 2)) / 3) * 2);
      damage (ch, victim, dam, gsn_feed, DAM_NEGATIVE, TRUE);
    }
  else
    {
      check_improve (ch, gsn_feed, FALSE, 1);
      act ("`i$n tries to bite you, but hits only air.`x", ch, NULL, victim,
	   TO_VICT);
      act ("`hYou chomp a mouthfull of air.`x", ch, NULL, victim, TO_CHAR);
      act ("`k$n tries to bite $N.`x", ch, NULL, victim, TO_NOTVICT);
      damage (ch, victim, 0, gsn_feed, DAM_NEGATIVE, TRUE);
    }

  return;
}


void
do_flee (CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *was_in;
  ROOM_INDEX_DATA *now_in;
  CHAR_DATA *victim;
  int attempt;

  if ((victim = ch->fighting) == NULL)
    {
      if (ch->position == POS_FIGHTING)
	ch->position = POS_STANDING;
      send_to_char ("You aren't fighting anyone.\n\r", ch);
      return;
    }

  was_in = ch->in_room;
  for (attempt = 0; attempt < 6; attempt++)
    {
      EXIT_DATA *pexit;
      int door;

      door = number_door ();
      if ((pexit = was_in->exit[door]) == 0
	  || pexit->u1.to_room == NULL
	  || IS_SET (pexit->exit_info, EX_CLOSED)
	  || number_range (0, ch->daze) != 0
	  || (IS_NPC (ch)
	      && IS_SET (pexit->u1.to_room->room_flags, ROOM_NO_MOB)))
	continue;

      move_char (ch, door, FALSE, FALSE);
      if ((now_in = ch->in_room) == was_in)
	continue;

      ch->in_room = was_in;
      act ("$n has `Yfled`x!", ch, NULL, NULL, TO_ROOM);
      if (!IS_NPC (ch))
	{
	  send_to_char ("`BYou `Yflee`B from combat!`x\n\r", ch);
	  if (((ch->class == 2) || (ch->class == (MAX_CLASS / 2) + 1))
	      && (number_percent () < 3 * (ch->level / 2)))
	    {
	      if (IS_NPC (victim) || ch->attacker == FALSE)
		{
		  send_to_char ("You `Ysnuck away`x safely.\n\r", ch);
		}
	      else
		{
		  send_to_char
		    ("You feel something singe your butt on the way out.\n\r",
		     ch);
		  act
		    ("$n is nearly `Yzapped`x in the butt by a lightning bolt from above!",
		     ch, NULL, NULL, TO_ROOM);
		  ch->hit -= (ch->hit / 8);
		}
	    }
	  else
	    {
	      if (!IS_NPC (victim) && ch->attacker == TRUE)
		{
		  send_to_char
		    ("The `RWrath of Thoth `YZAPS`x your butt on the way out!\n\r",
		     ch);
		  act
		    ("$n is `Yzapped`x in the butt by a lightning bolt from above!",
		     ch, NULL, NULL, TO_ROOM);
		  ch->hit -= (ch->hit / 4);
		}
	      send_to_char ("You lost 10 exp.\n\r", ch);
	      gain_exp (ch, -10);
	    }
	}
      ch->in_room = now_in;
      stop_fighting (ch, TRUE);
      return;
    }

  send_to_char ("`z`CPANIC!`x`B You couldn't escape!`x\n\r", ch);
  return;
}



void
do_rescue (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *fch;

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Rescue whom?\n\r", ch);
      return;
    }

  if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (victim == ch)
    {
      send_to_char ("What about `Yfleeing`x instead?\n\r", ch);
      return;
    }

  if (!IS_NPC (ch) && IS_NPC (victim))
    {
      send_to_char ("Doesn't need your help!\n\r", ch);
      return;
    }

  if (ch->fighting == victim)
    {
      send_to_char ("Too late.\n\r", ch);
      return;
    }

  if ((fch = victim->fighting) == NULL)
    {
      send_to_char ("That person is not fighting right now.\n\r", ch);
      return;
    }

  if (IS_NPC (fch) && !is_same_group (ch, victim))
    {
      send_to_char ("Kill stealing is not permitted.\n\r", ch);
      return;
    }

  WAIT_STATE (ch, skill_table[gsn_rescue].beats);
  if (number_percent () > get_skill (ch, gsn_rescue))
    {
      send_to_char ("You fail the rescue.\n\r", ch);
      check_improve (ch, gsn_rescue, FALSE, 1);
      return;
    }

  act ("`yYou rescue $N!`x", ch, NULL, victim, TO_CHAR);
  act ("`y$n rescues you!`x", ch, NULL, victim, TO_VICT);
  act ("`y$n rescues $N!`x", ch, NULL, victim, TO_NOTVICT);
  check_improve (ch, gsn_rescue, TRUE, 1);

  stop_fighting (fch, FALSE);
  stop_fighting (victim, FALSE);

  set_fighting (ch, fch);
  set_fighting (fch, ch);
  return;
}



void
do_kick (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  int dam;

  if (!IS_NPC (ch)
      && ch->level < skill_table[gsn_kick].skill_level[ch->class])
    {
      send_to_char ("You better leave the martial arts to fighters.\n\r", ch);
      return;
    }

  if (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_KICK))
    return;

  if ((victim = ch->fighting) == NULL)
    {
      send_to_char ("You aren't fighting anyone.\n\r", ch);
      return;
    }

  if (ch->stunned)
    {
      send_to_char ("You're still a little woozy.\n\r", ch);
      return;
    }

  dam = number_range (1, ch->level);
  WAIT_STATE (ch, skill_table[gsn_kick].beats);
  if (get_skill (ch, gsn_kick) > number_percent ())
    {
      damage (ch, victim, number_range (dam, (ch->level * 1.5)), gsn_kick,
	      DAM_BASH, TRUE);
      check_improve (ch, gsn_kick, TRUE, 1);
    }
  else
    {
      damage (ch, victim, 0, gsn_kick, DAM_BASH, TRUE);
      check_improve (ch, gsn_kick, FALSE, 1);
    }
  return;
}




void
do_disarm (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  int chance, hth, ch_weapon, vict_weapon, ch_vict_weapon;

  hth = 0;

  if ((chance = get_skill (ch, gsn_disarm)) == 0)
    {
      send_to_char ("You don't know how to disarm opponents.\n\r", ch);
      return;
    }

  if (get_eq_char (ch, WEAR_WIELD) == NULL
      && ((hth = get_skill (ch, gsn_hand_to_hand)) == 0
	  || (IS_NPC (ch) && !IS_SET (ch->off_flags, OFF_DISARM))))
    {
      send_to_char ("You must wield a weapon to disarm.\n\r", ch);
      return;
    }

  if ((victim = ch->fighting) == NULL)
    {
      send_to_char ("You aren't fighting anyone.\n\r", ch);
      return;
    }

  if (ch->stunned)
    {
      send_to_char ("You're still a little woozy.\n\r", ch);
      return;
    }

  if ((obj = get_eq_char (victim, WEAR_WIELD)) == NULL)
    {
      send_to_char ("`hYour opponent is not wielding a weapon.`x\n\r", ch);
      return;
    }

  /* find weapon skills */
  ch_weapon = get_weapon_skill (ch, get_weapon_sn (ch));
  vict_weapon = get_weapon_skill (victim, get_weapon_sn (victim));
  ch_vict_weapon = get_weapon_skill (ch, get_weapon_sn (victim));

  /* modifiers */

  /* skill */
  if (get_eq_char (ch, WEAR_WIELD) == NULL)
    chance = chance * hth / 150;
  else
    chance = chance * ch_weapon / 100;

  chance += (ch_vict_weapon / 2 - vict_weapon) / 2;

  /* dex vs. strength */
  chance += get_curr_stat (ch, STAT_DEX);
  chance -= 2 * get_curr_stat (victim, STAT_STR);

  /* level */
  chance += (ch->level - victim->level) * 2;

  chance /= 2;

  /* and now the attack */
  if (number_percent () < chance)
    {
      if (((chance = get_skill (victim, gsn_grip)) == 0)
	  || (!IS_NPC (victim)
	      && victim->level <
	      skill_table[gsn_grip].skill_level[victim->class]))
	{
	  WAIT_STATE (ch, skill_table[gsn_disarm].beats);
	  disarm (ch, victim);
	  check_improve (ch, gsn_disarm, TRUE, 1);
	  return;
	}
      if (number_percent () > (chance / 5) * 4)
	{
	  WAIT_STATE (ch, skill_table[gsn_disarm].beats);
	  disarm (ch, victim);
	  check_improve (ch, gsn_disarm, TRUE, 1);
	  check_improve (victim, gsn_grip, FALSE, 1);
	  return;
	}
      check_improve (victim, gsn_grip, TRUE, 1);
    }
  WAIT_STATE (ch, skill_table[gsn_disarm].beats);
  act ("`hYou fail to disarm $N.`x", ch, NULL, victim, TO_CHAR);
  act ("`i$n tries to disarm you, but fails.`x", ch, NULL, victim, TO_VICT);
  act ("`k$n tries to disarm $N, but fails.`x", ch, NULL, victim, TO_NOTVICT);
  check_improve (ch, gsn_disarm, FALSE, 1);
  return;
}

void
do_surrender (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *mob;
  if ((mob = ch->fighting) == NULL)
    {
      send_to_char ("But you're not fighting!\n\r", ch);
      return;
    }
  act ("You surrender to $N!", ch, NULL, mob, TO_CHAR);
  act ("$n surrenders to you!", ch, NULL, mob, TO_VICT);
  act ("$n tries to surrender to $N!", ch, NULL, mob, TO_NOTVICT);
  stop_fighting (ch, TRUE);

  if (!IS_NPC (ch) && IS_NPC (mob)
      && (!HAS_TRIGGER (mob, TRIG_SURR)
	  || !mp_percent_trigger (mob, ch, NULL, NULL, TRIG_SURR)))
    {
      act ("$N seems to ignore your cowardly act!", ch, NULL, mob, TO_CHAR);
      multi_hit (mob, ch, TYPE_UNDEFINED);
    }
}

void
do_sla (CHAR_DATA * ch, char *argument)
{
  send_to_char ("If you want to `RSLAY`x, spell it out.\n\r", ch);
  return;
}



void
do_slay (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Slay whom?\n\r", ch);
      return;
    }

  if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (ch == victim)
    {
      send_to_char ("Suicide is a mortal sin.\n\r", ch);
      return;
    }

  if (!IS_NPC (victim) && victim->level >= get_trust (ch))
    {
      send_to_char ("`hYou failed.`c\n\r", ch);
      return;
    }

  if (IS_NPC (victim) || get_trust (ch) >= CREATOR)
    {
      act ("`hYou slay $M in cold blood!`x", ch, NULL, victim, TO_CHAR);
      act ("`i$n slays you in cold blood!`x", ch, NULL, victim, TO_VICT);
      act ("`k$n slays $N in cold blood!`x", ch, NULL, victim, TO_NOTVICT);
      raw_kill (victim, ch);
    }
  else
    {
      act ("`i$N wields a sword called '`z`RGodSlayer`i'!`x", ch, NULL,
	   victim, TO_CHAR);
      act ("`hYou wield a sword called '`z`RGodSlayer`h'!`x", ch, NULL,
	   victim, TO_VICT);
      act ("`k$N wields a sword called '`z`RGodSlayer`k'!`x", ch, NULL,
	   victim, TO_NOTVICT);
      act ("`i$N's slice takes off your left arm!`x", ch, NULL, victim,
	   TO_CHAR);
      act ("`hYour slice takes off $n's left arm!`x", ch, NULL, victim,
	   TO_VICT);
      act ("`k$N's slice takes off $n's left arm!`x", ch, NULL, victim,
	   TO_NOTVICT);
      act ("`i$N's slice takes off your right arm!`x", ch, NULL, victim,
	   TO_CHAR);
      act ("`hYour slice takes off $n's right arm!`x", ch, NULL, victim,
	   TO_VICT);
      act ("`k$N's slice takes off $n's right arm!`x", ch, NULL, victim,
	   TO_NOTVICT);
      act ("`i$N's slice cuts off both of your legs!`x", ch, NULL, victim,
	   TO_CHAR);
      act ("`hYour slice cuts off both of $n's legs!`x", ch, NULL, victim,
	   TO_VICT);
      act ("`k$N's slice cuts off both of $n's legs!`x", ch, NULL, victim,
	   TO_NOTVICT);
      act ("`i$N's slice beheads you!`x", ch, NULL, victim, TO_CHAR);
      act ("`hYour slice beheads $n!`x", ch, NULL, victim, TO_VICT);
      act ("`k$N's slice beheads $n!`x", ch, NULL, victim, TO_NOTVICT);
      act ("`iYou are DEAD!!!`x", ch, NULL, victim, TO_CHAR);
      act ("`h$n is DEAD!!!`x", ch, NULL, victim, TO_VICT);
      act ("`k$n is DEAD!!!`x", ch, NULL, victim, TO_NOTVICT);
      act ("A sword called '`z`RGodSlayer`x' vanishes.", ch, NULL, victim,
	   TO_VICT);
      act ("A sword called '`z`RGodSlayer`x' vanishes.", ch, NULL, victim,
	   TO_NOTVICT);
      raw_kill (ch, victim);
    }
  return;
}
