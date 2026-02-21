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
*\tROM 2.4 is copyright 1993-1995 Russ Taylor\t\t\t   *
*\tROM has been brought to you by the ROM consortium\t\t   *
*\t    Russ Taylor (rtaylor@pacinfo.com)\t\t\t\t   *
*\t    Gabrielle Taylor (gtaylor@pacinfo.com)\t\t\t   *
*\t    Brian Moore (rom@rom.efn.org)\t\t\t\t   *
*\tBy using this code, you have agreed to follow the terms of the\t   *
*\tROM license, in the file Rom24/doc/rom.license\t\t\t   *
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
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "tables.h"
#include "lookup.h"


void
do_noemote (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Noemote whom?\n\r", ch);
      return;
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }


  if (get_trust (victim) >= get_trust (ch))
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }

  if (IS_SET (victim->comm, COMM_NOEMOTE))
    {
      REMOVE_BIT (victim->comm, COMM_NOEMOTE);
      send_to_char ("You can emote again.\n\r", victim);
      send_to_char ("NOEMOTE removed.\n\r", ch);
      snprintf (buf, sizeof (buf), "$N restores emotes to %s.", victim->name);
      wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }
  else
    {
      SET_BIT (victim->comm, COMM_NOEMOTE);
      send_to_char ("You can't emote!\n\r", victim);
      send_to_char ("NOEMOTE set.\n\r", ch);
      snprintf (buf, sizeof (buf), "$N revokes %s's emotes.", victim->name);
      wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }

  return;
}



void
do_noshout (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Noshout whom?\n\r", ch);
      return;
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (IS_NPC (victim))
    {
      send_to_char ("Not on NPC's.\n\r", ch);
      return;
    }

  if (get_trust (victim) >= get_trust (ch))
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }

  if (IS_SET (victim->comm, COMM_NOSHOUT))
    {
      REMOVE_BIT (victim->comm, COMM_NOSHOUT);
      send_to_char ("You can shout again.\n\r", victim);
      send_to_char ("NOSHOUT removed.\n\r", ch);
      snprintf (buf, sizeof (buf), "$N restores shouts to %s.", victim->name);
      wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }
  else
    {
      SET_BIT (victim->comm, COMM_NOSHOUT);
      send_to_char ("You can't shout!\n\r", victim);
      send_to_char ("NOSHOUT set.\n\r", ch);
      snprintf (buf, sizeof (buf), "$N revokes %s's shouts.", victim->name);
      wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }

  return;
}



void
do_notell (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Notell whom?\n\r", ch);
      return;
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (get_trust (victim) >= get_trust (ch))
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }

  if (IS_SET (victim->comm, COMM_NOTELL))
    {
      REMOVE_BIT (victim->comm, COMM_NOTELL);
      send_to_char ("You can tell again.\n\r", victim);
      send_to_char ("NOTELL removed.\n\r", ch);
      snprintf (buf, sizeof (buf), "$N restores tells to %s.", victim->name);
      wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }
  else
    {
      SET_BIT (victim->comm, COMM_NOTELL);
      send_to_char ("You can't tell!\n\r", victim);
      send_to_char ("NOTELL set.\n\r", ch);
      snprintf (buf, sizeof (buf), "$N revokes %s's tells.", victim->name);
      wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }

  return;
}



void
do_peace (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *rch;

  for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
      if (rch->fighting != NULL)
	{
	  stop_fighting (rch, TRUE);
	  if (!IS_NPC (rch))
	    {
	      send_to_char ("Ok.\n\r", ch);
	    }
	}
      if (IS_NPC (rch) && IS_SET (rch->act, ACT_AGGRESSIVE))
	REMOVE_BIT (rch->act, ACT_AGGRESSIVE);
    }
  return;
}

void
do_wizlock (CHAR_DATA * ch, char *argument)
{
  extern bool wizlock;
  wizlock = !wizlock;

  if (wizlock)
    {
      wiznet ("$N has wizlocked the game.", ch, NULL, 0, 0, 0);
      send_to_char ("Game wizlocked.\n\r", ch);
    }
  else
    {
      wiznet ("$N removes wizlock.", ch, NULL, 0, 0, 0);
      send_to_char ("Game un-wizlocked.\n\r", ch);
    }

  return;
}

/* RT anti-newbie code */

void
do_newlock (CHAR_DATA * ch, char *argument)
{
  extern bool newlock;
  newlock = !newlock;

  if (newlock)
    {
      wiznet ("$N locks out new characters.", ch, NULL, 0, 0, 0);
      send_to_char ("New characters have been locked out.\n\r", ch);
    }
  else
    {
      wiznet ("$N allows new characters back in.", ch, NULL, 0, 0, 0);
      send_to_char ("Newlock removed.\n\r", ch);
    }

  return;
}


void
do_slookup (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int sn;

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Lookup which skill or spell?\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "all"))
    {
      for (sn = 0; sn < MAX_SKILL; sn++)
	{
	  if (skill_table[sn].name == NULL)
	    break;
	  snprintf (buf, sizeof (buf), "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
		   sn, skill_table[sn].slot, skill_table[sn].name);
	  send_to_char (buf, ch);
	}
    }
  else
    {
      if ((sn = skill_lookup (arg)) < 0)
	{
	  send_to_char ("No such skill or spell.\n\r", ch);
	  return;
	}

      snprintf (buf, sizeof (buf), "Sn: %3d  Slot: %3d  Skill/spell: '%s'\n\r",
	       sn, skill_table[sn].slot, skill_table[sn].name);
      send_to_char (buf, ch);
    }

  return;
}
