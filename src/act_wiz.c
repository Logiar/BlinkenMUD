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
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"

/* command procedures needed */
DECLARE_DO_FUN (do_at);
DECLARE_DO_FUN (do_rstat);
DECLARE_DO_FUN (do_mstat);
DECLARE_DO_FUN (do_ostat);
DECLARE_DO_FUN (do_rset);
DECLARE_DO_FUN (do_mset);
DECLARE_DO_FUN (do_oset);
DECLARE_DO_FUN (do_sset);
DECLARE_DO_FUN (do_mfind);
DECLARE_DO_FUN (do_ofind);
DECLARE_DO_FUN (do_slookup);
DECLARE_DO_FUN (do_mload);
DECLARE_DO_FUN (do_oload);
DECLARE_DO_FUN (do_vload);
DECLARE_DO_FUN (do_force);
DECLARE_DO_FUN (do_quit);
DECLARE_DO_FUN (do_save);
DECLARE_DO_FUN (do_transfer);
DECLARE_DO_FUN (do_look);
DECLARE_DO_FUN (do_force);
DECLARE_DO_FUN (do_stand);
DECLARE_DO_FUN (do_disconnect);
DECLARE_DO_FUN (do_restore);
DECLARE_DO_FUN (do_allpeace);



/*
 * Local functions.
 */
static void append_to_buf (char *buf, size_t buf_size, const char *text);


static void
append_to_buf (char *buf, size_t buf_size, const char *text)
{
  size_t buf_len;

  if (buf_size == 0 || text == NULL)
    return;

  buf_len = strlen (buf);
  if (buf_len >= buf_size - 1)
    return;

  snprintf (buf + buf_len, buf_size - buf_len, "%s", text);
}


void
do_echo (CHAR_DATA * ch, char *argument)
{
  DESCRIPTOR_DATA *d;

  if (argument[0] == '\0')
    {
      send_to_char ("Global echo what?\n\r", ch);
      return;
    }

  for (d = descriptor_list; d; d = d->next)
    {
      if (d->connected == CON_PLAYING)
	{
	  if (d->character->level >= ch->level)
	    send_to_char ("global> ", d->character);
	  send_to_char (argument, d->character);
	  send_to_char ("\n\r", d->character);
	}
    }

  return;
}

void
do_wecho (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];

  if (argument[0] == '\0')
    {
      send_to_char ("Warn echo what?\n\r", ch);
      return;
    }

  snprintf (buf, sizeof (buf), "`z`B***`x `R%s`x `z`B***`x", argument);
  do_echo (ch, buf);
  do_echo (ch, buf);
  do_echo (ch, buf);
  do_restore (ch, "all");
  do_allpeace (ch, "");
  return;
}

void
do_recho (CHAR_DATA * ch, char *argument)
{
  DESCRIPTOR_DATA *d;

  if (argument[0] == '\0')
    {
      send_to_char ("Local echo what?\n\r", ch);

      return;
    }

  for (d = descriptor_list; d; d = d->next)
    {
      if (d->connected == CON_PLAYING && d->character->in_room == ch->in_room)
	{
	  if (d->character->level >= ch->level)
	    send_to_char ("local> ", d->character);
	  send_to_char (argument, d->character);
	  send_to_char ("\n\r", d->character);
	}
    }

  return;
}

void
do_zecho (CHAR_DATA * ch, char *argument)
{
  DESCRIPTOR_DATA *d;

  if (argument[0] == '\0')
    {
      send_to_char ("Zone echo what?\n\r", ch);
      return;
    }

  for (d = descriptor_list; d; d = d->next)
    {
      if (d->connected == CON_PLAYING
	  && d->character->in_room != NULL && ch->in_room != NULL
	  && d->character->in_room->area == ch->in_room->area)
	{
	  if (d->character->level >= ch->level)
	    send_to_char ("zone> ", d->character);
	  send_to_char (argument, d->character);
	  send_to_char ("\n\r", d->character);
	}
    }
}

void
do_pecho (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  argument = one_argument (argument, arg);

  if (argument[0] == '\0' || arg[0] == '\0')
    {
      send_to_char ("Personal echo what?\n\r", ch);
      return;
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("Target not found.\n\r", ch);
      return;
    }

  if (victim->level >= ch->level)
    send_to_char ("personal> ", victim);

  send_to_char (argument, victim);
  send_to_char ("\n\r", victim);
  send_to_char ("personal> ", ch);
  send_to_char (argument, ch);
  send_to_char ("\n\r", ch);
}


ROOM_INDEX_DATA *
find_location (CHAR_DATA * ch, char *arg)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;

  if (is_number (arg))
    return get_room_index (atoi (arg));

  if ((victim = get_char_world (ch, arg)) != NULL)
    return victim->in_room;

  if ((obj = get_obj_world (ch, arg)) != NULL)
    return obj->in_room;

  return NULL;
}

void
do_corner (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Corner whom?\n\r", ch);
      return;
    }

  snprintf (buf, sizeof (buf), "%s %d", arg, ROOM_VNUM_CORNER);
  do_transfer (ch, buf);

  return;
}

void
do_transfer (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;
  DESCRIPTOR_DATA *d;
  CHAR_DATA *victim;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (arg1[0] == '\0')
    {
      send_to_char ("Transfer whom (and where)?\n\r", ch);
      return;
    }

  if (!str_cmp (arg1, "all") && (ch->level >= CREATOR))
    {
      for (d = descriptor_list; d != NULL; d = d->next)
	{
	  if (d->connected == CON_PLAYING
	      && d->character != ch
	      && d->character->in_room != NULL
	      && ch->level >= d->character->ghost_level
	      && can_see (ch, d->character))
	    {
	      char buf[MAX_STRING_LENGTH];
	      snprintf (buf, sizeof (buf), "%s %s", d->character->name, arg2);
	      do_transfer (ch, buf);
	    }
	}
      return;
    }

  /*
   * Thanks to Grodyn for the optional location parameter.
   */
  if (arg2[0] == '\0')
    {
      location = ch->in_room;
    }
  else
    {
      if ((location = find_location (ch, arg2)) == NULL)
	{
	  send_to_char ("No such location.\n\r", ch);
	  return;
	}

      if (!is_room_owner (ch, location) && room_is_private (ch, location)
	  && ch->level < MAX_LEVEL)
	{
	  send_to_char ("That room is private right now.\n\r", ch);
	  return;
	}
    }

  if ((victim = get_char_world (ch, arg1)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if ((victim->level > ch->level
       && !IS_SET (ch->act, PLR_KEY)
       && (victim->level != MAX_LEVEL))
      || ((IS_SET (victim->act, PLR_KEY)) && (ch->level != MAX_LEVEL)))
    {
      send_to_char ("You failed!\n\r", ch);
      return;
    }

  if (victim->in_room == NULL)
    {
      send_to_char ("They are in limbo.\n\r", ch);
      return;
    }

  if (victim->fighting != NULL)
    stop_fighting (victim, TRUE);
  act ("$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM);
  char_from_room (victim);
  char_to_room (victim, location);
  act ("$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM);
  if (ch != victim)
    act ("$n has transferred you.", ch, NULL, victim, TO_VICT);
  do_look (victim, "auto");
  send_to_char ("Ok.\n\r", ch);
}

void
do_allpeace (CHAR_DATA * ch, char *argument)
{
  DESCRIPTOR_DATA *d;

  for (d = descriptor_list; d != NULL; d = d->next)
    {
      if (d->connected == CON_PLAYING
	  && d->character != ch
	  && d->character->in_room != NULL
	  && ch->level >= d->character->ghost_level
	  && can_see (ch, d->character))
	{
	  char buf[MAX_STRING_LENGTH];
	  snprintf (buf, sizeof (buf), "%s peace", d->character->name);
	  do_at (ch, buf);
	}
    }
}

void
do_wedpost (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  argument = one_argument (argument, arg1);

  if (arg1[0] == '\0')
    {
      send_to_char ("Syntax: wedpost <char>\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg1)) == NULL)
    {
      send_to_char ("They aren't playing.\n\r", ch);
      return;
    }

  if (victim->wedpost)
    {
      send_to_char
	("They are no longer allowed to post wedding announcements.\n\r", ch);
      victim->wedpost = FALSE;
    }
  else
    {
      send_to_char ("They are now allowed to post wedding announcements.\n\r",
		    ch);
      victim->wedpost = TRUE;
    }
}

void
do_recover (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;
  CHAR_DATA *victim;

  argument = one_argument (argument, arg1);

  if (arg1[0] == '\0')
    {
      send_to_char ("Recover whom?\n\r", ch);
      return;
    }

  if ((victim = get_char_world (ch, arg1)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (IS_NPC (victim) && !IS_SET (ch->act, ACT_PET))
    {
      send_to_char ("You can't recover NPC's.\n\r", ch);
      return;
    }

  if ((get_trust (victim) > get_trust (ch)
       && !IS_SET (ch->act, PLR_KEY)
       && (victim->level != MAX_LEVEL))
      || ((IS_SET (victim->act, PLR_KEY)) && (ch->level != MAX_LEVEL)))
    {
      send_to_char ("You failed!\n\r", ch);
      return;
    }

  if (victim->in_room == NULL)
    {
      send_to_char ("They are in limbo.\n\r", ch);
      return;
    }

  if (victim->fighting != NULL)
    {
      send_to_char ("They are fighting.\n\r", ch);
      return;
    }

  if (victim->alignment < 0)
    {
      if ((location = get_room_index (ROOM_VNUM_TEMPLEB)) == NULL)
	{
	  send_to_char ("The recall point seems to be missing.\n\r", ch);
	  return;
	}
    }
  else
    {
      if ((location = get_room_index (ROOM_VNUM_TEMPLE)) == NULL)
	{
	  send_to_char ("The recall point seems to be missing.\n\r", ch);
	  return;
	}
    }

  if (is_clan (victim)
      && (clan_table[victim->clan].hall != ROOM_VNUM_ALTAR)
      && !IS_SET (victim->act, PLR_TWIT))
    location = get_room_index (clan_table[victim->clan].hall);

  if (IS_NPC (victim) && IS_SET (ch->act, ACT_PET)
      && is_clan (victim->master)
      && (clan_table[victim->master->clan].hall != ROOM_VNUM_ALTAR)
      && !IS_SET (victim->master->act, PLR_TWIT))
    location = get_room_index (clan_table[victim->master->clan].hall);

  if (victim->in_room == location)
    {
      act ("$N does not need recovering.", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (!IS_SET (victim->in_room->room_flags, ROOM_NO_RECALL)
      && !IS_AFFECTED (victim, AFF_CURSE))
    {
      act ("$N does not need recovering.", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (victim->fighting != NULL)
    stop_fighting (victim, TRUE);
  act ("$n disappears in a flash.", victim, NULL, NULL, TO_ROOM);
  char_from_room (victim);
  char_to_room (victim, location);
  act ("$n arrives from a flash of light.", victim, NULL, NULL, TO_ROOM);
  if (ch != victim)
    act ("$n has recovered you.", ch, NULL, victim, TO_VICT);
  do_look (victim, "auto");
  act ("$N has been recovered.", ch, NULL, victim, TO_CHAR);
  if (victim->pet != NULL)
    do_recover (victim->pet, "");
}

void
do_at (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;
  ROOM_INDEX_DATA *original;
  OBJ_DATA *on;
  CHAR_DATA *wch;

  if (IS_NPC (ch))
    {
      send_to_char ("NPC's cannot use this command.\n\r", ch);
      return;
    }
  argument = one_argument (argument, arg);

  if (arg[0] == '\0' || argument[0] == '\0')
    {
      send_to_char ("At where what?\n\r", ch);
      return;
    }

  if ((location = find_location (ch, arg)) == NULL)
    {
      send_to_char ("No such location.\n\r", ch);
      return;
    }

  if (!is_room_owner (ch, location) && room_is_private (ch, location)
      && ch->level < MAX_LEVEL)
    {
      send_to_char ("That room is private right now.\n\r", ch);
      return;
    }

  original = ch->in_room;
  on = ch->on;
  char_from_room (ch);
  char_to_room (ch, location);
  interpret (ch, argument);

  /*
   * See if 'ch' still exists before continuing!
   * Handles 'at XXXX quit' case.
   */
  for (wch = char_list; wch != NULL; wch = wch->next)
    {
      if (wch == ch)
	{
	  char_from_room (ch);
	  char_to_room (ch, original);
	  ch->on = on;
	  break;
	}
    }

  return;
}



void
do_goto (CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *location;
  CHAR_DATA *rch;
  char arg[MAX_INPUT_LENGTH];
  int count = 0;

  if ((argument[0] == '\0') && (IS_NPC (ch)))
    {
      send_to_char ("Goto where?\n\r", ch);
      return;
    }
  if ((argument[0] == '\0') && (!ch->pcdata->recall))
    {
      send_to_char ("Goto where?\n\r", ch);
      return;
    }
  if ((argument[0] == '\0') && (ch->pcdata->recall))
    {
      snprintf (arg, sizeof (arg), "%d", ch->pcdata->recall);
    }
  else
    {
      snprintf (arg, sizeof (arg), "%s", argument);
    }
  if ((location = find_location (ch, arg)) == NULL)
    {
      send_to_char ("No such location.\n\r", ch);
      return;
    }

  count = 0;
  for (rch = location->people; rch != NULL; rch = rch->next_in_room)
    count++;

  if (!is_room_owner (ch, location) && room_is_private (ch, location)
      && (count > 1 || ch->level < MAX_LEVEL))
    {
      send_to_char ("That room is private right now.\n\r", ch);
      return;
    }

  if (ch->fighting != NULL)
    stop_fighting (ch, TRUE);

  for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
      if ((rch->level >= ch->invis_level) && (rch->level >= ch->ghost_level))
	{
	  if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
	    act ("$t", ch, ch->pcdata->bamfout, rch, TO_VICT);
	  else
	    act ("$n leaves in a swirling mist.", ch, NULL, rch, TO_VICT);
	}
    }

  char_from_room (ch);
  char_to_room (ch, location);


  for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
      if ((rch->level >= ch->invis_level) && (rch->level >= ch->ghost_level))
	{
	  if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
	    act ("$t", ch, ch->pcdata->bamfin, rch, TO_VICT);
	  else
	    act ("$n appears in a swirling mist.", ch, NULL, rch, TO_VICT);
	}
    }
  if ((argument[0] == '\0') && (ch->pet != NULL))
    {
      char_from_room (ch->pet);
      char_to_room (ch->pet, location);
    }
  do_look (ch, "auto");
  return;
}

void
do_violate (CHAR_DATA * ch, char *argument)
{
  ROOM_INDEX_DATA *location;
  CHAR_DATA *rch;

  if (argument[0] == '\0')
    {
      send_to_char ("Goto where?\n\r", ch);
      return;
    }

  if ((location = find_location (ch, argument)) == NULL)
    {
      send_to_char ("No such location.\n\r", ch);
      return;
    }

  if (!room_is_private (ch, location))
    {
      send_to_char ("That room isn't private, use goto.\n\r", ch);
      return;
    }

  if (ch->fighting != NULL)
    stop_fighting (ch, TRUE);

  for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
      if ((rch->level >= ch->invis_level) && (rch->level >= ch->ghost_level))
	{
	  if (ch->pcdata != NULL && ch->pcdata->bamfout[0] != '\0')
	    act ("$t", ch, ch->pcdata->bamfout, rch, TO_VICT);
	  else
	    act ("$n leaves in a swirling mist.", ch, NULL, rch, TO_VICT);
	}
    }

  char_from_room (ch);
  char_to_room (ch, location);


  for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
    {
      if ((rch->level >= ch->invis_level) && (rch->level >= ch->ghost_level))
	{
	  if (ch->pcdata != NULL && ch->pcdata->bamfin[0] != '\0')
	    act ("$t", ch, ch->pcdata->bamfin, rch, TO_VICT);
	  else
	    act ("$n appears in a swirling mist.", ch, NULL, rch, TO_VICT);
	}
    }

  do_look (ch, "auto");
  return;
}

/* RT to replace the 3 stat commands */

void
do_stat (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char *string;
  OBJ_DATA *obj;
  ROOM_INDEX_DATA *location;
  CHAR_DATA *victim;

  string = one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Syntax:\n\r", ch);
      send_to_char ("  stat <name>\n\r", ch);
      send_to_char ("  stat obj <name>\n\r", ch);
      send_to_char ("  stat mob <name>\n\r", ch);
      send_to_char ("  stat room <number>\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "room"))
    {
      do_rstat (ch, string);
      return;
    }

  if (!str_cmp (arg, "obj"))
    {
      do_ostat (ch, string);
      return;
    }

  if (!str_cmp (arg, "char") || !str_cmp (arg, "mob"))
    {
      do_mstat (ch, string);
      return;
    }

  /* do it the old way */

  obj = get_obj_world (ch, argument);
  if (obj != NULL)
    {
      do_ostat (ch, argument);
      return;
    }

  victim = get_char_world (ch, argument);
  if (victim != NULL)
    {
      do_mstat (ch, argument);
      return;
    }

  location = find_location (ch, argument);
  if (location != NULL)
    {
      do_rstat (ch, argument);
      return;
    }

  send_to_char ("Nothing by that name found anywhere.\n\r", ch);
}

void
do_rstat (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;
  OBJ_DATA *obj;
  CHAR_DATA *rch;
  int door;

  one_argument (argument, arg);
  location = (arg[0] == '\0') ? ch->in_room : find_location (ch, arg);
  if (location == NULL)
    {
      send_to_char ("No such location.\n\r", ch);
      return;
    }

  if (!is_room_owner (ch, location) && ch->in_room != location
      && room_is_private (ch, location) && !IS_TRUSTED (ch, IMPLEMENTOR))
    {
      send_to_char ("That room is private right now.\n\r", ch);
      return;
    }

  snprintf (buf, sizeof (buf), "Name: '%s'\n\rArea: '%s'\n\r",
	   location->name, location->area->name);
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf),
	   "Vnum: %d  Sector: %d  Light: %d  Healing: %d  Mana: %d\n\r",
	   location->vnum,
	   location->sector_type,
	   location->light, location->heal_rate, location->mana_rate);
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf),
	   "Room flags: %s.\n\rDescription:\n\r%s",
	   room_bit_name (location->room_flags), location->description);
  send_to_char (buf, ch);

  if (location->extra_descr != NULL)
    {
      EXTRA_DESCR_DATA *ed;

      send_to_char ("Extra description keywords: '", ch);
      for (ed = location->extra_descr; ed; ed = ed->next)
	{
	  send_to_char (ed->keyword, ch);
	  if (ed->next != NULL)
	    send_to_char (" ", ch);
	}
      send_to_char ("'.\n\r", ch);
    }

  send_to_char ("Characters:", ch);
  for (rch = location->people; rch; rch = rch->next_in_room)
    {
      if ((get_trust (ch) >= rch->ghost_level) && (can_see (ch, rch)))
	{
	  send_to_char (" ", ch);
	  one_argument (rch->name, buf);
	  send_to_char (buf, ch);
	}
    }

  send_to_char (".\n\rObjects:   ", ch);
  for (obj = location->contents; obj; obj = obj->next_content)
    {
      send_to_char (" ", ch);
      one_argument (obj->name, buf);
      send_to_char (buf, ch);
    }
  send_to_char (".\n\r", ch);

  for (door = 0; door <= 5; door++)
    {
      EXIT_DATA *pexit;

      if ((pexit = location->exit[door]) != NULL)
	{
	  snprintf (buf, sizeof (buf),
		   "Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\n\rKeyword: '%s'.  Description: %s",
		   door,
		   (pexit->u1.to_room == NULL ? -1 : pexit->u1.to_room->vnum),
		   pexit->key,
		   pexit->exit_info,
		   pexit->keyword,
		   pexit->description[0] != '\0'
		   ? pexit->description : "(none).\n\r");
	  send_to_char (buf, ch);
	}
    }

  return;
}



void
do_ostat (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  AFFECT_DATA *paf;
  OBJ_DATA *obj;

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Stat what?\n\r", ch);
      return;
    }

  if ((obj = get_obj_world (ch, argument)) == NULL)
    {
      send_to_char ("Nothing like that in hell, earth, or heaven.\n\r", ch);
      return;
    }

  snprintf (buf, sizeof (buf), "Name(s): %s\n\r", obj->name);
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf), "Vnum: %d  Format: %s  Type: %s  Resets: %d\n\r",
	   obj->pIndexData->vnum, obj->pIndexData->new_format ? "new" : "old",
	   item_type_name (obj), obj->pIndexData->reset_num);
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf), "Short description: %s\n\rLong description: %s\n\r",
	   obj->short_descr, obj->description);
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf), "Wear bits: %s\n\rExtra bits: %s\n\r",
	   wear_bit_name (obj->wear_flags),
	   extra_bit_name (obj->extra_flags));
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf), "Number: %d/%d  Weight: %d/%d/%d (10th pounds)\n\r",
	   1, get_obj_number (obj),
	   obj->weight, get_obj_weight (obj), get_true_weight (obj));
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf), "Level: %d  Cost: %d  Condition: %d  Timer: %d\n\r",
	   obj->level, obj->cost, obj->condition, obj->timer);
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf),
	   "In room: %d  In object: %s  Carried by: %s  Wear_loc: %d\n\r",
	   obj->in_room == NULL ? 0 : obj->in_room->vnum,
	   obj->in_obj == NULL ? "(none)" : obj->in_obj->short_descr,
	   obj->carried_by == NULL ? "(none)" :
	   can_see (ch, obj->carried_by) ? obj->carried_by->name
	   : "someone", obj->wear_loc);
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf), "Values: %d %d %d %d %d\n\r",
	   obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	   obj->value[4]);
  send_to_char (buf, ch);

  /* now give out vital statistics as per identify */

  switch (obj->item_type)
    {
    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_PILL:
      snprintf (buf, sizeof (buf), "Level %d spells of:", obj->value[0]);
      send_to_char (buf, ch);

      if (obj->value[1] >= 0 && obj->value[1] < MAX_SKILL)
	{
	  send_to_char (" '", ch);
	  send_to_char (skill_table[obj->value[1]].name, ch);
	  send_to_char ("'", ch);
	}

      if (obj->value[2] >= 0 && obj->value[2] < MAX_SKILL)
	{
	  send_to_char (" '", ch);
	  send_to_char (skill_table[obj->value[2]].name, ch);
	  send_to_char ("'", ch);
	}

      if (obj->value[3] >= 0 && obj->value[3] < MAX_SKILL)
	{
	  send_to_char (" '", ch);
	  send_to_char (skill_table[obj->value[3]].name, ch);
	  send_to_char ("'", ch);
	}

      if (obj->value[4] >= 0 && obj->value[4] < MAX_SKILL)
	{
	  send_to_char (" '", ch);
	  send_to_char (skill_table[obj->value[4]].name, ch);
	  send_to_char ("'", ch);
	}

      send_to_char (".\n\r", ch);
      break;

    case ITEM_WAND:
    case ITEM_STAFF:
      snprintf (buf, sizeof (buf), "Has %d(%d) charges of level %d",
	       obj->value[1], obj->value[2], obj->value[0]);
      send_to_char (buf, ch);

      if (obj->value[3] >= 0 && obj->value[3] < MAX_SKILL)
	{
	  send_to_char (" '", ch);
	  send_to_char (skill_table[obj->value[3]].name, ch);
	  send_to_char ("'", ch);
	}

      send_to_char (".\n\r", ch);
      break;

    case ITEM_DRINK_CON:
      snprintf (buf, sizeof (buf), "It holds %s-colored %s.\n\r",
	       liq_table[obj->value[2]].liq_color,
	       liq_table[obj->value[2]].liq_name);
      send_to_char (buf, ch);
      break;


    case ITEM_WEAPON:
      send_to_char ("Weapon type is ", ch);
      switch (obj->value[0])
	{
	case (WEAPON_EXOTIC):
	  send_to_char ("exotic\n\r", ch);
	  break;
	case (WEAPON_SWORD):
	  send_to_char ("sword\n\r", ch);
	  break;
	case (WEAPON_DAGGER):
	  send_to_char ("dagger\n\r", ch);
	  break;
	case (WEAPON_SPEAR):
	  send_to_char ("spear/staff\n\r", ch);
	  break;
	case (WEAPON_MACE):
	  send_to_char ("mace/club\n\r", ch);
	  break;
	case (WEAPON_AXE):
	  send_to_char ("axe\n\r", ch);
	  break;
	case (WEAPON_FLAIL):
	  send_to_char ("flail\n\r", ch);
	  break;
	case (WEAPON_WHIP):
	  send_to_char ("whip\n\r", ch);
	  break;
	case (WEAPON_POLEARM):
	  send_to_char ("polearm\n\r", ch);
	  break;
	default:
	  send_to_char ("unknown\n\r", ch);
	  break;
	}
      if (obj->clan)
	{
	  snprintf (buf, sizeof (buf), "Damage is variable.\n\r");
	}
      else
	{
	  if (obj->pIndexData->new_format)
	    snprintf (buf, sizeof (buf), "Damage is %dd%d (average %d)\n\r",
		     obj->value[1], obj->value[2],
		     (1 + obj->value[2]) * obj->value[1] / 2);
	  else
	    snprintf (buf, sizeof (buf), "Damage is %d to %d (average %d)\n\r",
		     obj->value[1], obj->value[2],
		     (obj->value[1] + obj->value[2]) / 2);
	}
      send_to_char (buf, ch);

      snprintf (buf, sizeof (buf), "Damage noun is %s.\n\r",
	       attack_table[obj->value[3]].noun);
      send_to_char (buf, ch);

      if (obj->value[4])	/* weapon flags */
	{
	  snprintf (buf, sizeof (buf), "Weapons flags: %s\n\r",
		   weapon_bit_name (obj->value[4]));
	  send_to_char (buf, ch);
	}
      break;

    case ITEM_ARMOR:
      if (obj->clan)
	{
	  snprintf (buf, sizeof (buf), "Armor class is variable.\n\r");
	}
      else
	{
	  snprintf (buf, sizeof (buf),
		   "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic\n\r",
		   obj->value[0], obj->value[1], obj->value[2],
		   obj->value[3]);
	}
      send_to_char (buf, ch);
      break;

    case ITEM_CONTAINER:
    case ITEM_PIT:
      snprintf (buf, sizeof (buf), "Capacity: %d#  Maximum weight: %d#  flags: %s\n\r",
	       obj->value[0], obj->value[3], cont_bit_name (obj->value[1]));
      send_to_char (buf, ch);
      if (obj->value[4] != 100)
	{
	  snprintf (buf, sizeof (buf), "Weight multiplier: %d%%\n\r", obj->value[4]);
	  send_to_char (buf, ch);
	}
      break;
    }

  if (is_clan_obj (obj))
    {
      snprintf (buf, sizeof (buf), "This object is owned by the [`%s%s`x] clan.\n\r",
	       clan_table[obj->clan].pkill ? "B" : "M",
	       clan_table[obj->clan].who_name);
      send_to_char (buf, ch);
    }

  if (is_class_obj (obj))
    {
      snprintf (buf, sizeof (buf), "This object may only be used by a %s.\n\r",
	       class_table[obj->class].name);
      send_to_char (buf, ch);
    }

  if (obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL)
    {
      EXTRA_DESCR_DATA *ed;

      send_to_char ("Extra description keywords: '", ch);

      for (ed = obj->extra_descr; ed != NULL; ed = ed->next)
	{
	  send_to_char (ed->keyword, ch);
	  if (ed->next != NULL)
	    send_to_char (" ", ch);
	}

      for (ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next)
	{
	  send_to_char (ed->keyword, ch);
	  if (ed->next != NULL)
	    send_to_char (" ", ch);
	}

      send_to_char ("'\n\r", ch);
    }

  for (paf = obj->affected; paf != NULL; paf = paf->next)
    {
      snprintf (buf, sizeof (buf), "Affects %s by %d, level %d",
	       affect_loc_name (paf->location), paf->modifier, paf->level);
      send_to_char (buf, ch);
      if (paf->duration > -1)
	snprintf (buf, sizeof (buf), ", %d hours.\n\r", paf->duration);
      else
	snprintf (buf, sizeof (buf), ".\n\r");
      send_to_char (buf, ch);
      if (paf->bitvector)
	{
	  switch (paf->where)
	    {
	    case TO_AFFECTS:
	      snprintf (buf, sizeof (buf), "Adds %s affect.\n",
		       affect_bit_name (paf->bitvector));
	      break;
	    case TO_WEAPON:
	      snprintf (buf, sizeof (buf), "Adds %s weapon flags.\n",
		       weapon_bit_name (paf->bitvector));
	      break;
	    case TO_OBJECT:
	      snprintf (buf, sizeof (buf), "Adds %s object flag.\n",
		       extra_bit_name (paf->bitvector));
	      break;
	    case TO_IMMUNE:
	      snprintf (buf, sizeof (buf), "Adds immunity to %s.\n",
		       imm_bit_name (paf->bitvector));
	      break;
	    case TO_RESIST:
	      snprintf (buf, sizeof (buf), "Adds resistance to %s.\n\r",
		       imm_bit_name (paf->bitvector));
	      break;
	    case TO_VULN:
	      snprintf (buf, sizeof (buf), "Adds vulnerability to %s.\n\r",
		       imm_bit_name (paf->bitvector));
	      break;
	    case TO_SHIELDS:
	      snprintf (buf, sizeof (buf), "Adds %s shield.\n",
		       shield_bit_name (paf->bitvector));
	      break;
	    default:
	      snprintf (buf, sizeof (buf), "Unknown bit %d: %d\n\r",
		       paf->where, paf->bitvector);
	      break;
	    }
	  send_to_char (buf, ch);
	}
    }

  if (!obj->enchanted)
    for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
      {
	snprintf (buf, sizeof (buf), "Affects %s by %d, level %d.\n\r",
		 affect_loc_name (paf->location), paf->modifier, paf->level);
	send_to_char (buf, ch);
	if (paf->bitvector)
	  {
	    switch (paf->where)
	      {
	      case TO_AFFECTS:
		snprintf (buf, sizeof (buf), "Adds %s affect.\n",
			 affect_bit_name (paf->bitvector));
		break;
	      case TO_OBJECT:
		snprintf (buf, sizeof (buf), "Adds %s object flag.\n",
			 extra_bit_name (paf->bitvector));
		break;
	      case TO_IMMUNE:
		snprintf (buf, sizeof (buf), "Adds immunity to %s.\n",
			 imm_bit_name (paf->bitvector));
		break;
	      case TO_RESIST:
		snprintf (buf, sizeof (buf), "Adds resistance to %s.\n\r",
			 imm_bit_name (paf->bitvector));
		break;
	      case TO_VULN:
		snprintf (buf, sizeof (buf), "Adds vulnerability to %s.\n\r",
			 imm_bit_name (paf->bitvector));
		break;
	      case TO_SHIELDS:
		snprintf (buf, sizeof (buf), "Adds %s shield.\n",
			 shield_bit_name (paf->bitvector));
		break;
	      default:
		snprintf (buf, sizeof (buf), "Unknown bit %d: %d\n\r",
			 paf->where, paf->bitvector);
		break;
	      }
	    send_to_char (buf, ch);
	  }
      }

  return;
}



void
do_mstat (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  BUFFER *output;
  AFFECT_DATA *paf;
  CHAR_DATA *victim;

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Stat whom?\n\r", ch);
      return;
    }

  if ((victim = get_char_world (ch, argument)) == NULL
      || (victim->level > ch->level && victim->level == MAX_LEVEL))
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  output = new_buf ();

  if (!IS_NPC (victim))
    {
      snprintf (buf, sizeof (buf), "Name: %s\n\rSocket: %s\n\r",
	       victim->name, victim->pcdata->socket);
    }
  else
    {
      snprintf (buf, sizeof (buf), "Name: %s\n\rSocket: <mobile>\n\r", victim->name);
    }
  add_buf (output, buf);

  snprintf (buf, sizeof (buf),
	   "Vnum: %d  Format: %s  Race: %s  Group: %d  Sex: %s  Room: %d\n\r",
	   IS_NPC (victim) ? victim->pIndexData->vnum : 0,
	   IS_NPC (victim) ? victim->pIndexData->
	   new_format ? "new" : "old" : "pc", race_table[victim->race].name,
	   IS_NPC (victim) ? victim->group : 0, sex_table[victim->sex].name,
	   victim->in_room == NULL ? 0 : victim->in_room->vnum);
  add_buf (output, buf);

  if (IS_NPC (victim))
    {
      snprintf (buf, sizeof (buf), "Count: %d  Killed: %d\n\r",
	       victim->pIndexData->count, victim->pIndexData->killed);
      add_buf (output, buf);
    }

  snprintf (buf, sizeof (buf),
	   "Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d)\n\r",
	   victim->perm_stat[STAT_STR],
	   get_curr_stat (victim, STAT_STR),
	   victim->perm_stat[STAT_INT],
	   get_curr_stat (victim, STAT_INT),
	   victim->perm_stat[STAT_WIS],
	   get_curr_stat (victim, STAT_WIS),
	   victim->perm_stat[STAT_DEX],
	   get_curr_stat (victim, STAT_DEX),
	   victim->perm_stat[STAT_CON], get_curr_stat (victim, STAT_CON));
  add_buf (output, buf);

  snprintf (buf, sizeof (buf), "Hp: %d/%d  Mana: %d/%d  Move: %d/%d  Practices: %d\n\r",
	   victim->hit, victim->max_hit,
	   victim->mana, victim->max_mana,
	   victim->move, victim->max_move,
	   IS_NPC (ch) ? 0 : victim->practice);
  add_buf (output, buf);

  snprintf (buf, sizeof (buf),
	   "Lv: %d  Class: %s  Align: %d  Exp: %ld\n\r",
	   victim->level,
	   IS_NPC (victim) ? "mobile" : class_table[victim->class].name,
	   victim->alignment, victim->exp);
  add_buf (output, buf);

  snprintf (buf, sizeof (buf),
	   "Platinum: %ld  Gold: %ld  Silver: %ld\n\r",
	   victim->platinum, victim->gold, victim->silver);
  add_buf (output, buf);

  snprintf (buf, sizeof (buf), "Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n\r",
	   GET_AC (victim, AC_PIERCE), GET_AC (victim, AC_BASH),
	   GET_AC (victim, AC_SLASH), GET_AC (victim, AC_EXOTIC));
  add_buf (output, buf);

  snprintf (buf, sizeof (buf),
	   "Hit: %d  Dam: %d  Saves: %d  Size: %s  Position: %s  Wimpy: %d\n\r",
	   GET_HITROLL (victim), GET_DAMROLL (victim), victim->saving_throw,
	   size_table[victim->size].name,
	   position_table[victim->position].name, victim->wimpy);
  add_buf (output, buf);

  if (IS_NPC (victim) && victim->pIndexData->new_format)
    {
      snprintf (buf, sizeof (buf), "Damage: %dd%d  Message:  %s\n\r",
	       victim->damage[DICE_NUMBER], victim->damage[DICE_TYPE],
	       attack_table[victim->dam_type].noun);
      add_buf (output, buf);
    }
  snprintf (buf, sizeof (buf), "Fighting: %s\n\r",
	   victim->fighting ? victim->fighting->name : "(none)");
  add_buf (output, buf);

  if (!IS_NPC (victim))
    {
      snprintf (buf, sizeof (buf),
	       "Thirst: %d  Hunger: %d  Full: %d  Drunk: %d  Quest: %d\n\r",
	       victim->pcdata->condition[COND_THIRST],
	       victim->pcdata->condition[COND_HUNGER],
	       victim->pcdata->condition[COND_FULL],
	       victim->pcdata->condition[COND_DRUNK], victim->qps);
      add_buf (output, buf);
    }

  snprintf (buf, sizeof (buf), "Carry number: %d  Carry weight: %ld\n\r",
	   victim->carry_number, get_carry_weight (victim) / 10);
  add_buf (output, buf);

  if (!IS_NPC (victim))
    {
      snprintf (buf, sizeof (buf),
	       "Age: %d  Played: %d  Last Level: %d  Timer: %d\n\r",
	       get_age (victim),
	       (int) (victim->played + current_time - victim->logon) / 3600,
	       victim->pcdata->last_level, victim->timer);
      add_buf (output, buf);
    }

  snprintf (buf, sizeof (buf), "Act: %s\n\r", act_bit_name (victim->act));
  add_buf (output, buf);

  if (victim->comm)
    {
      snprintf (buf, sizeof (buf), "Comm: %s\n\r", comm_bit_name (victim->comm));
      add_buf (output, buf);
    }

  if (IS_NPC (victim) && victim->off_flags)
    {
      snprintf (buf, sizeof (buf), "Offense: %s\n\r", off_bit_name (victim->off_flags));
      add_buf (output, buf);
    }

  if (victim->imm_flags)
    {
      snprintf (buf, sizeof (buf), "Immune: %s\n\r", imm_bit_name (victim->imm_flags));
      add_buf (output, buf);
    }

  if (victim->res_flags)
    {
      snprintf (buf, sizeof (buf), "Resist: %s\n\r", imm_bit_name (victim->res_flags));
      add_buf (output, buf);
    }

  if (victim->vuln_flags)
    {
      snprintf (buf, sizeof (buf), "Vulnerable: %s\n\r", imm_bit_name (victim->vuln_flags));
      add_buf (output, buf);
    }

  snprintf (buf, sizeof (buf), "Form: %s\n\rParts: %s\n\r",
	   form_bit_name (victim->form), part_bit_name (victim->parts));
  add_buf (output, buf);

  if (victim->affected_by)
    {
      snprintf (buf, sizeof (buf), "Affected by %s\n\r",
	       affect_bit_name (victim->affected_by));
      add_buf (output, buf);
    }

  if (victim->shielded_by)
    {
      snprintf (buf, sizeof (buf), "Shielded by %s\n\r",
	       shield_bit_name (victim->shielded_by));
      add_buf (output, buf);
    }

  snprintf (buf, sizeof (buf), "Master: %s  Leader: %s  Pet: %s\n\r",
	   victim->master ? victim->master->name : "(none)",
	   victim->leader ? victim->leader->name : "(none)",
	   victim->pet ? victim->pet->name : "(none)");
  add_buf (output, buf);

  if (!IS_NPC (victim))
    {
      snprintf (buf, sizeof (buf), "Security: %d.\n\r", victim->pcdata->security);	/* OLC */
      send_to_char (buf, ch);	/* OLC */
    }

  snprintf (buf, sizeof (buf), "Short description: %s\n\rLong  description: %s",
	   victim->short_descr,
	   victim->long_descr[0] != '\0' ? victim->long_descr : "(none)\n\r");
  add_buf (output, buf);

  if (IS_NPC (victim) && victim->spec_fun != 0)
    {
      snprintf (buf, sizeof (buf), "Mobile has special procedure %s.\n\r",
	       spec_name (victim->spec_fun));
      add_buf (output, buf);
    }

  for (paf = victim->affected; paf != NULL; paf = paf->next)
    {
      snprintf (buf, sizeof (buf),
	       "Spell: '%s' modifies %s by %d for %d hours with bits %s, level %d.\n\r",
	       skill_table[(int) paf->type].name,
	       affect_loc_name (paf->location),
	       paf->modifier,
	       paf->duration,
	       paf->where == TO_SHIELDS ?
	       shield_bit_name (paf->bitvector) :
	       affect_bit_name (paf->bitvector), paf->level);
      add_buf (output, buf);
    }
  page_to_char (buf_string (output), ch);
  free_buf (output);
  return;
}

/* ofind and mfind replaced with vnum, vnum skill also added */

void
do_vnum (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  BUFFER *output;
  AREA_DATA *pArea1;
  AREA_DATA *pArea2;
  int iArea;
  int iAreaHalf;
  char *string;

  string = one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Syntax:\n\r", ch);
      send_to_char ("  vnum obj <name>\n\r", ch);
      send_to_char ("  vnum mob <name>\n\r", ch);
      send_to_char ("  vnum skill <skill or spell>\n\r", ch);
      send_to_char ("  vnum areas\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "obj"))
    {
      do_ofind (ch, string);
      return;
    }

  if (!str_cmp (arg, "mob") || !str_cmp (arg, "char"))
    {
      do_mfind (ch, string);
      return;
    }

  if (!str_cmp (arg, "skill") || !str_cmp (arg, "spell"))
    {
      do_slookup (ch, string);
      return;
    }

  if (!str_cmp (arg, "areas") || !str_cmp (arg, "area"))
    {
      output = new_buf ();
      iAreaHalf = (top_area + 1) / 2;
      pArea1 = area_first;
      pArea2 = area_first;
      for (iArea = 0; iArea < iAreaHalf; iArea++)
	pArea2 = pArea2->next;

      for (iArea = 0; iArea < iAreaHalf; iArea++)
	{
	  snprintf (buf, sizeof (buf), "%-26s `R%5d %5d`x  %-26s `R%5d %5d`x\n\r",
		   pArea1->name, pArea1->min_vnum, pArea1->max_vnum,
		   (pArea2 != NULL) ? pArea2->name : "",
		   (pArea2 != NULL) ? pArea2->min_vnum : 0,
		   (pArea2 != NULL) ? pArea2->max_vnum : 0);
	  add_buf (output, buf);
	  pArea1 = pArea1->next;
	  if (pArea2 != NULL)
	    pArea2 = pArea2->next;
	}
      page_to_char (buf_string (output), ch);
      free_buf (output);
      return;
    }

  /* do both */
  do_mfind (ch, argument);
  do_ofind (ch, argument);
}


void
do_mfind (CHAR_DATA * ch, char *argument)
{
  extern int top_mob_index;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  BUFFER *output;
  MOB_INDEX_DATA *pMobIndex;
  int vnum;
  int nMatch;
  bool fAll;
  bool found;

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Find whom?\n\r", ch);
      return;
    }

  fAll = FALSE;			/* !str_cmp( arg, "all" ); */
  found = FALSE;
  nMatch = 0;
  output = new_buf ();

  /*
   * Yeah, so iterating over all vnum's takes 10,000 loops.
   * Get_mob_index is fast, and I don't feel like threading another link.
   * Do you?
   * -- Furey
   */
  for (vnum = 0; nMatch < top_mob_index; vnum++)
    {
      if ((pMobIndex = get_mob_index (vnum)) != NULL)
	{
	  nMatch++;
	  if (fAll || is_name (argument, pMobIndex->player_name))
	    {
	      found = TRUE;
	      snprintf (buf, sizeof (buf), "[%5d] %s\n\r",
		       pMobIndex->vnum, pMobIndex->short_descr);
	      add_buf (output, buf);
	    }
	}
    }

  if (!found)
    {
      send_to_char ("No mobiles by that name.\n\r", ch);
    }
  else
    {
      page_to_char (buf_string (output), ch);
    }
  free_buf (output);
  return;
}



void
do_ofind (CHAR_DATA * ch, char *argument)
{
  extern int top_obj_index;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  BUFFER *output;
  OBJ_INDEX_DATA *pObjIndex;
  int vnum;
  int nMatch;
  bool fAll;
  bool found;

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Find what?\n\r", ch);
      return;
    }

  fAll = FALSE;			/* !str_cmp( arg, "all" ); */
  found = FALSE;
  nMatch = 0;
  output = new_buf ();

  /*
   * Yeah, so iterating over all vnum's takes 10,000 loops.
   * Get_obj_index is fast, and I don't feel like threading another link.
   * Do you?
   * -- Furey
   */
  for (vnum = 0; nMatch < top_obj_index; vnum++)
    {
      if ((pObjIndex = get_obj_index (vnum)) != NULL)
	{
	  nMatch++;
	  if (fAll || is_name (argument, pObjIndex->name))
	    {
	      found = TRUE;
	      snprintf (buf, sizeof (buf), "[%5d] %s\n\r",
		       pObjIndex->vnum, pObjIndex->short_descr);
	      add_buf (output, buf);
	    }
	}
    }

  if (!found)
    {
      send_to_char ("No objects by that name.\n\r", ch);
    }
  else
    {
      page_to_char (buf_string (output), ch);
    }
  free_buf (output);
  return;
}


void
do_owhere (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_INPUT_LENGTH];
  BUFFER *buffer;
  OBJ_DATA *obj;
  OBJ_DATA *in_obj;
  bool found;
  int number = 0, max_found;

  found = FALSE;
  number = 0;
  max_found = 200;

  buffer = new_buf ();

  if (argument[0] == '\0')
    {
      send_to_char ("Find what?\n\r", ch);
      return;
    }

  for (obj = object_list; obj != NULL; obj = obj->next)
    {
      if (!can_see_obj (ch, obj) || !is_name (argument, obj->name)
	  || ch->level < obj->level
	  || (obj->carried_by != NULL && !can_see (ch, obj->carried_by)))
	continue;

      found = TRUE;
      number++;

      for (in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj)
	;

      if (in_obj->carried_by != NULL && can_see (ch, in_obj->carried_by)
	  && in_obj->carried_by->in_room != NULL)
	snprintf (buf, sizeof (buf), "%3d) %s is carried by %s [Room %d]\n\r",
		 number, obj->short_descr, PERS (in_obj->carried_by, ch),
		 in_obj->carried_by->in_room->vnum);
      else if (in_obj->in_room != NULL && can_see_room (ch, in_obj->in_room))
	snprintf (buf, sizeof (buf), "%3d) %s is in %s [Room %d]\n\r",
		 number, obj->short_descr, in_obj->in_room->name,
		 in_obj->in_room->vnum);
      else
	snprintf (buf, sizeof (buf), "%3d) %s is somewhere\n\r", number, obj->short_descr);

      buf[0] = UPPER (buf[0]);
      add_buf (buffer, buf);

      if (number >= max_found)
	break;
    }

  if (!found)
    send_to_char ("Nothing like that in heaven or earth.\n\r", ch);
  else
    page_to_char (buf_string (buffer), ch);

  free_buf (buffer);
}


void
do_mwhere (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  BUFFER *buffer;
  CHAR_DATA *victim;
  bool found;
  int count = 0;

  if (argument[0] == '\0')
    {
      DESCRIPTOR_DATA *d;

      /* show characters logged */

      buffer = new_buf ();
      for (d = descriptor_list; d != NULL; d = d->next)
	{
	  if (d->character != NULL && d->connected == CON_PLAYING
	      && d->character->in_room != NULL && can_see (ch, d->character)
	      && can_see_room (ch, d->character->in_room))
	    {
	      victim = d->character;
	      if ((victim->level <= CREATOR && ch->level <= CREATOR)
		  || ch->level > CREATOR)
		{
		  count++;
		  if (d->original != NULL)
		    snprintf (buf, sizeof (buf),
			     "%3d) %s (in the body of %s) is in %s [%d]\n\r",
			     count, d->original->name, victim->short_descr,
			     victim->in_room->name, victim->in_room->vnum);
		  else
		    snprintf (buf, sizeof (buf), "%3d) %s is in %s [%d]\n\r",
			     count, victim->name, victim->in_room->name,
			     victim->in_room->vnum);
		  add_buf (buffer, buf);
		}
	    }
	}

      page_to_char (buf_string (buffer), ch);
      free_buf (buffer);
      return;
    }

  found = FALSE;
  buffer = new_buf ();
  for (victim = char_list; victim != NULL; victim = victim->next)
    {
      if (victim->in_room != NULL && is_name (argument, victim->name))
	{
	  if ((victim->level <= CREATOR && ch->level <= CREATOR)
	      || ch->level > CREATOR)
	    {
	      found = TRUE;
	      count++;
	      snprintf (buf, sizeof (buf), "%3d) [%5d] %-28s [%5d] %s\n\r", count,
		       IS_NPC (victim) ? victim->pIndexData->vnum : 0,
		       IS_NPC (victim) ? victim->short_descr : victim->name,
		       victim->in_room->vnum, victim->in_room->name);
	      add_buf (buffer, buf);
	    }
	}
    }

  if (!found)
    act ("You didn't find any $T.", ch, NULL, argument, TO_CHAR);
  else
    page_to_char (buf_string (buffer), ch);

  free_buf (buffer);

  return;
}



void
do_reboo (CHAR_DATA * ch, char *argument)
{
  send_to_char ("If you want to REBOOT, spell it out.\n\r", ch);
  return;
}



/*void do_reboot( CHAR_DATA *ch, char *argument )
{
    char buf[MAX_STRING_LENGTH];
    extern bool merc_down;
    DESCRIPTOR_DATA *d,*d_next;

    if (ch->invis_level < LEVEL_HERO)
    {
    	snprintf( buf, sizeof(buf), "Reboot by %s.", ch->name );
    	do_echo( ch, buf );
    }
    do_force ( ch, "all save");
    do_save (ch, "");
    merc_down = TRUE;
    for ( d = descriptor_list; d != NULL; d = d_next )
    {
	d_next = d->next;
    	close_socket(d);
    }
    
    return;
}*/

void
do_reboot (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];

  argument = one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Usage: reboot now\n\r", ch);
      send_to_char ("Usage: reboot <ticks to reboot>\n\r", ch);
      send_to_char ("Usage: reboot cancel\n\r", ch);
      send_to_char ("Usage: reboot status\n\r", ch);
      return;
    }

  if (is_name (arg, "cancel"))
    {
      reboot_counter = -1;
      send_to_char ("Reboot canceled.\n\r", ch);
      return;
    }

  if (is_name (arg, "now"))
    {
      reboot_rot ();
      return;
    }

  if (is_name (arg, "status"))
    {
      if (reboot_counter == -1)
	snprintf (buf, sizeof (buf), "Automatic rebooting is inactive.\n\r");
      else
	snprintf (buf, sizeof (buf), "Reboot in %i minutes.\n\r", reboot_counter);
      send_to_char (buf, ch);
      return;
    }

  if (is_number (arg))
    {
      reboot_counter = atoi (arg);
      snprintf (buf, sizeof (buf), "BlinkenMuD will reboot in %i ticks.\n\r",
	       reboot_counter);
      send_to_char (buf, ch);
      return;
    }

  do_reboot (ch, "");
}


void
reboot_rot (void)
{
  extern bool merc_down;
  DESCRIPTOR_DATA *d, *d_next;

  snprintf (log_buf, MAX_STRING_LENGTH, "Rebooting BlinkenMuD.");
  log_string (log_buf);
  for (d = descriptor_list; d != NULL; d = d_next)
    {
      d_next = d->next;
      write_to_buffer (d, "BlinkenMuD is going down for rebooting NOW!", 0);
      if (d->character != NULL)
	save_char_obj (d->character);
      close_socket (d);
    }
  merc_down = TRUE;
  return;
}




void
do_shutdow (CHAR_DATA * ch, char *argument)
{
  send_to_char ("If you want to SHUTDOWN, spell it out.\n\r", ch);
  return;
}



void
do_shutdown (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  extern bool merc_down;
  DESCRIPTOR_DATA *d, *d_next;

  if (ch->invis_level < LEVEL_HERO)
    snprintf (buf, sizeof (buf), "Shutdown by %s.", ch->name);
  append_file (ch, SHUTDOWN_FILE, buf);
  append_to_buf (buf, sizeof (buf), "\n\r");
  if (ch->invis_level < LEVEL_HERO)
    do_echo (ch, buf);
  do_force (ch, "all save");
  do_save (ch, "");
  merc_down = TRUE;
  for (d = descriptor_list; d != NULL; d = d_next)
    {
      d_next = d->next;
      close_socket (d);
    }
  return;
}

void
do_protect (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;

  if (argument[0] == '\0')
    {
      send_to_char ("Protect whom from snooping?\n\r", ch);
      return;
    }

  if ((victim = get_char_world (ch, argument)) == NULL)
    {
      send_to_char ("You can't find them.\n\r", ch);
      return;
    }

  if (IS_SET (victim->comm, COMM_SNOOP_PROOF))
    {
      act_new ("$N is no longer snoop-proof.", ch, NULL, victim, TO_CHAR,
	       POS_DEAD);
      send_to_char ("Your snoop-proofing was just removed.\n\r", victim);
      REMOVE_BIT (victim->comm, COMM_SNOOP_PROOF);
    }
  else
    {
      act_new ("$N is now snoop-proof.", ch, NULL, victim, TO_CHAR, POS_DEAD);
      send_to_char ("You are now immune to snooping.\n\r", victim);
      SET_BIT (victim->comm, COMM_SNOOP_PROOF);
    }
}



void
do_snoop (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  DESCRIPTOR_DATA *d;
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Snoop whom?\n\r", ch);
      return;
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (victim->desc == NULL)
    {
      send_to_char ("No descriptor to snoop.\n\r", ch);
      return;
    }

  if (victim == ch)
    {
      send_to_char ("Cancelling all snoops.\n\r", ch);
      if (!IS_TRUSTED (ch, IMPLEMENTOR))
	{
	  wiznet ("$N stops being such a snoop.",
		  ch, NULL, WIZ_SNOOPS, WIZ_SECURE, get_trust (ch));
	}
      for (d = descriptor_list; d != NULL; d = d->next)
	{
	  if (d->snoop_by == ch->desc)
	    d->snoop_by = NULL;
	}
      return;
    }

  if (victim->desc->snoop_by != NULL)
    {
      send_to_char ("Busy already.\n\r", ch);
      return;
    }

  if (!is_room_owner (ch, victim->in_room) && ch->in_room != victim->in_room
      && room_is_private (ch, victim->in_room)
      && !IS_TRUSTED (ch, IMPLEMENTOR))
    {
      send_to_char ("That character is in a private room.\n\r", ch);
      return;
    }

  if (get_trust (victim) >= get_trust (ch)
      || (IS_SET (victim->comm, COMM_SNOOP_PROOF)
	  && !IS_TRUSTED (ch, IMPLEMENTOR)))
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }

  if (ch->desc != NULL)
    {
      for (d = ch->desc->snoop_by; d != NULL; d = d->snoop_by)
	{
	  if (d->character == victim || d->original == victim)
	    {
	      send_to_char ("No snoop loops.\n\r", ch);
	      return;
	    }
	}
    }

  victim->desc->snoop_by = ch->desc;
  if (!IS_TRUSTED (ch, IMPLEMENTOR))
    {
      snprintf (buf, sizeof (buf), "$N starts snooping on %s",
	       (IS_NPC (ch) ? victim->short_descr : victim->name));
      wiznet (buf, ch, NULL, WIZ_SNOOPS, WIZ_SECURE, get_trust (ch));
    }
  send_to_char ("Ok.\n\r", ch);
  return;
}



void
do_switch (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Switch into whom?\n\r", ch);
      return;
    }

  if (ch->desc == NULL)
    return;

  if (ch->desc->original != NULL)
    {
      send_to_char ("You are already switched.\n\r", ch);
      return;
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (victim == ch)
    {
      send_to_char ("Ok.\n\r", ch);
      return;
    }

  if (!IS_NPC (victim))
    {
      send_to_char ("You can only switch into mobiles.\n\r", ch);
      return;
    }

  if (victim->level > ch->level)
    {
      send_to_char ("That character is too powerful for you to handle.\n\r",
		    ch);
      return;
    }

  if (!is_room_owner (ch, victim->in_room) && ch->in_room != victim->in_room
      && room_is_private (ch, victim->in_room)
      && !IS_TRUSTED (ch, IMPLEMENTOR))
    {
      send_to_char ("That character is in a private room.\n\r", ch);
      return;
    }

  if (victim->desc != NULL)
    {
      send_to_char ("Character in use.\n\r", ch);
      return;
    }

  snprintf (buf, sizeof (buf), "$N switches into %s", victim->short_descr);
  wiznet (buf, ch, NULL, WIZ_SWITCHES, WIZ_SECURE, get_trust (ch));

  ch->desc->character = victim;
  ch->desc->original = ch;
  victim->desc = ch->desc;
  ch->desc = NULL;
  /* change communications to match */
  if (ch->prompt != NULL)
    victim->prompt = str_dup (ch->prompt);
  victim->comm = ch->comm;
  victim->lines = ch->lines;
  send_to_char ("Ok.\n\r", victim);
  return;
}



void
do_return (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];

  if (ch->desc == NULL)
    return;

  if (ch->desc->original == NULL)
    {
      send_to_char ("You aren't switched.\n\r", ch);
      return;
    }

  send_to_char
    ("You return to your original body. Type replay to see any missed tells.\n\r",
     ch);
  if (ch->prompt != NULL)
    {
      free_string (ch->prompt);
      ch->prompt = NULL;
    }

  snprintf (buf, sizeof (buf), "$N returns from %s.", ch->short_descr);
  wiznet (buf, ch->desc->original, 0, WIZ_SWITCHES, WIZ_SECURE,
	  get_trust (ch));
  ch->desc->character = ch->desc->original;
  ch->desc->original = NULL;
  ch->desc->character->desc = ch->desc;
  ch->desc = NULL;
  return;
}

/* trust levels for load and clone */
bool
obj_check (CHAR_DATA * ch, OBJ_DATA * obj)
{
  if (IS_TRUSTED (ch, GOD)
      || (IS_TRUSTED (ch, IMMORTAL) && obj->level <= 105)
      || (IS_TRUSTED (ch, DEMI) && obj->level <= 100)
      || (IS_TRUSTED (ch, KNIGHT) && obj->level <= 20)
      || (IS_TRUSTED (ch, SQUIRE) && obj->level == 5))
    return TRUE;
  else
    return FALSE;
}

/* for clone, to insure that cloning goes many levels deep */
void
recursive_clone (CHAR_DATA * ch, OBJ_DATA * obj, OBJ_DATA * clone)
{
  OBJ_DATA *c_obj, *t_obj;


  for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content)
    {
      if (obj_check (ch, c_obj))
	{
	  t_obj = create_object (c_obj->pIndexData, 0);
	  clone_object (c_obj, t_obj);
	  obj_to_obj (t_obj, clone);
	  recursive_clone (ch, c_obj, t_obj);
	}
    }
}

/* command that is similar to load */
void
do_clone (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char *rest;
  CHAR_DATA *mob;
  OBJ_DATA *obj;

  rest = one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Clone what?\n\r", ch);
      return;
    }

  if (!str_prefix (arg, "object"))
    {
      mob = NULL;
      obj = get_obj_here (ch, rest);
      if (obj == NULL)
	{
	  send_to_char ("You don't see that here.\n\r", ch);
	  return;
	}
    }
  else if (!str_prefix (arg, "mobile") || !str_prefix (arg, "character"))
    {
      obj = NULL;
      mob = get_char_room (ch, rest);
      if (mob == NULL)
	{
	  send_to_char ("You don't see that here.\n\r", ch);
	  return;
	}
    }
  else				/* find both */
    {
      mob = get_char_room (ch, argument);
      obj = get_obj_here (ch, argument);
      if (mob == NULL && obj == NULL)
	{
	  send_to_char ("You don't see that here.\n\r", ch);
	  return;
	}
    }

  /* clone an object */
  if (obj != NULL)
    {
      OBJ_DATA *clone;

      if (!obj_check (ch, obj))
	{
	  send_to_char
	    ("Your powers are not great enough for such a task.\n\r", ch);
	  return;
	}
      if (obj->item_type == ITEM_EXIT)
	{
	  send_to_char ("You cannot clone an exit object.\n\r", ch);
	  return;
	}
      clone = create_object (obj->pIndexData, 0);
      clone_object (obj, clone);
      if (obj->carried_by != NULL)
	obj_to_char (clone, ch);
      else
	obj_to_room (clone, ch->in_room);
      recursive_clone (ch, obj, clone);

      act ("$n has created $p.", ch, clone, NULL, TO_ROOM);
      act ("You clone $p.", ch, clone, NULL, TO_CHAR);
      wiznet ("$N clones $p.", ch, clone, WIZ_LOAD, WIZ_SECURE,
	      get_trust (ch));
      return;
    }
  else if (mob != NULL)
    {
      CHAR_DATA *clone;
      OBJ_DATA *new_obj;
      char buf[MAX_STRING_LENGTH];

      if (!IS_NPC (mob))
	{
	  send_to_char ("You can only clone mobiles.\n\r", ch);
	  return;
	}

      if ((mob->level > 100 && !IS_TRUSTED (ch, GOD))
	  || (mob->level > 90 && !IS_TRUSTED (ch, IMMORTAL))
	  || (mob->level > 85 && !IS_TRUSTED (ch, DEMI))
	  || (mob->level > 0 && !IS_TRUSTED (ch, KNIGHT))
	  || !IS_TRUSTED (ch, SQUIRE))
	{
	  send_to_char
	    ("Your powers are not great enough for such a task.\n\r", ch);
	  return;
	}

      clone = create_mobile (mob->pIndexData);
      clone_mobile (mob, clone);

      for (obj = mob->carrying; obj != NULL; obj = obj->next_content)
	{
	  if (obj_check (ch, obj))
	    {
	      new_obj = create_object (obj->pIndexData, 0);
	      clone_object (obj, new_obj);
	      recursive_clone (ch, obj, new_obj);
	      obj_to_char (new_obj, clone);
	      new_obj->wear_loc = obj->wear_loc;
	    }
	}
      char_to_room (clone, ch->in_room);
      act ("$n has created $N.", ch, NULL, clone, TO_ROOM);
      act ("You clone $N.", ch, NULL, clone, TO_CHAR);
      snprintf (buf, sizeof (buf), "$N clones %s.", clone->short_descr);
      wiznet (buf, ch, NULL, WIZ_LOAD, WIZ_SECURE, get_trust (ch));
      return;
    }
}

/* RT to replace the two load commands */

void
do_load (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];

  argument = one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Syntax:\n\r", ch);
      send_to_char ("  load mob <vnum>\n\r", ch);
      send_to_char ("  load obj <vnum> <level>\n\r", ch);
      if (ch->level >= CREATOR)
	send_to_char ("  load voodoo <player>\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "mob") || !str_cmp (arg, "char"))
    {
      do_mload (ch, argument);
      return;
    }

  if (!str_cmp (arg, "obj"))
    {
      do_oload (ch, argument);
      return;
    }

  if (!str_cmp (arg, "voodoo") && (ch->level >= CREATOR))
    {
      do_vload (ch, argument);
      return;
    }
  /* echo syntax */
  do_load (ch, "");
}


void
do_mload (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  MOB_INDEX_DATA *pMobIndex;
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];

  one_argument (argument, arg);

  if (arg[0] == '\0' || !is_number (arg))
    {
      send_to_char ("Syntax: load mob <vnum>.\n\r", ch);
      return;
    }

  if ((pMobIndex = get_mob_index (atoi (arg))) == NULL)
    {
      send_to_char ("No mob has that vnum.\n\r", ch);
      return;
    }

  victim = create_mobile (pMobIndex);
  char_to_room (victim, ch->in_room);
  act ("$n has created $N!", ch, NULL, victim, TO_ROOM);
  snprintf (buf, sizeof (buf), "$N loads %s.", victim->short_descr);
  wiznet (buf, ch, NULL, WIZ_LOAD, WIZ_SECURE, get_trust (ch));
  send_to_char ("Ok.\n\r", ch);
  return;
}

void
do_oload (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  OBJ_INDEX_DATA *pObjIndex;
  OBJ_DATA *obj;
  int level;

  argument = one_argument (argument, arg1);
  one_argument (argument, arg2);

  if (arg1[0] == '\0' || !is_number (arg1))
    {
      send_to_char ("Syntax: load obj <vnum> <level>.\n\r", ch);
      return;
    }

  level = get_trust (ch);	/* default */

  if (arg2[0] != '\0')		/* load with a level */
    {
      if (!is_number (arg2))
	{
	  send_to_char ("Syntax: oload <vnum> <level>.\n\r", ch);
	  return;
	}
      level = atoi (arg2);
      if (level < 0 || level > get_trust (ch))
	{
	  send_to_char ("Level must be be between 0 and your level.\n\r", ch);
	  return;
	}
    }

  if ((pObjIndex = get_obj_index (atoi (arg1))) == NULL)
    {
      send_to_char ("No object has that vnum.\n\r", ch);
      return;
    }
  if (pObjIndex->item_type == ITEM_EXIT)
    {
      send_to_char ("You cannot load an exit object.\n\r", ch);
      return;
    }

  obj = create_object (pObjIndex, level);
  if (CAN_WEAR (obj, ITEM_TAKE))
    obj_to_char (obj, ch);
  else
    obj_to_room (obj, ch->in_room);
  act ("$n has created $p!", ch, obj, NULL, TO_ROOM);
  wiznet ("$N loads $p.", ch, obj, WIZ_LOAD, WIZ_SECURE, get_trust (ch));
  send_to_char ("Ok.\n\r", ch);
  return;
}

void
do_vload (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  OBJ_INDEX_DATA *pObjIndex;
  OBJ_DATA *obj;
  DESCRIPTOR_DATA *d;
  bool found = FALSE;
  char *name;

  argument = one_argument (argument, arg1);

  if (arg1[0] == '\0')
    {
      send_to_char ("Syntax: load voodoo <player>\n\r", ch);
      return;
    }

  for (d = descriptor_list; d != NULL; d = d->next)
    {
      CHAR_DATA *wch;

      if (d->connected != CON_PLAYING || !can_see (ch, d->character))
	continue;

      wch = (d->original != NULL) ? d->original : d->character;

      if (!can_see (ch, wch))
	continue;

      if (!str_prefix (arg1, wch->name) && !found)
	{
	  if (IS_NPC (wch))
	    continue;

	  if (wch->level > ch->level)
	    continue;

	  found = TRUE;

	  if ((pObjIndex = get_obj_index (OBJ_VNUM_VOODOO)) == NULL)
	    {
	      send_to_char ("Cannot find the voodoo doll vnum.\n\r", ch);
	      return;
	    }
	  obj = create_object (pObjIndex, 0);
	  name = wch->name;
	  snprintf (buf, sizeof (buf), obj->short_descr, name);
	  free_string (obj->short_descr);
	  obj->short_descr = str_dup (buf);
	  snprintf (buf, sizeof (buf), obj->description, name);
	  free_string (obj->description);
	  obj->description = str_dup (buf);
	  snprintf (buf, sizeof (buf), obj->name, name);
	  free_string (obj->name);
	  obj->name = str_dup (buf);
	  if (CAN_WEAR (obj, ITEM_TAKE))
	    obj_to_char (obj, ch);
	  else
	    obj_to_room (obj, ch->in_room);
	  act ("$n has created $p!", ch, obj, NULL, TO_ROOM);
	  wiznet ("$N loads $p.", ch, obj, WIZ_LOAD, WIZ_SECURE,
		  get_trust (ch));
	  send_to_char ("Ok.\n\r", ch);
	  return;
	}
    }
  send_to_char ("No one of that name is playing.\n\r", ch);
  return;
}

void
do_randclan (CHAR_DATA * ch, char *argument)
{
  randomize_entrances (ROOM_VNUM_CLANS);
  send_to_char ("Clan entrances have been moved.\n\r", ch);
  return;
}

void
do_purge (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[100];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  DESCRIPTOR_DATA *d;

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      /* 'purge' */
      CHAR_DATA *vnext;
      OBJ_DATA *obj_next;

      for (victim = ch->in_room->people; victim != NULL; victim = vnext)
	{
	  vnext = victim->next_in_room;
	  if (IS_NPC (victim) && !IS_SET (victim->act, ACT_NOPURGE)
	      && victim != ch /* safety precaution */ )
	    extract_char (victim, TRUE);
	}

      for (obj = ch->in_room->contents; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  if (!IS_OBJ_STAT (obj, ITEM_NOPURGE))
	    extract_obj (obj);
	}

      act ("$n purges the room!", ch, NULL, NULL, TO_ROOM);
      send_to_char ("Ok.\n\r", ch);
      return;
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (!IS_NPC (victim))
    {

      if (ch == victim)
	{
	  send_to_char ("Ho ho ho.\n\r", ch);
	  return;
	}

      if (get_trust (ch) <= get_trust (victim))
	{
	  send_to_char ("Maybe that wasn't a good idea...\n\r", ch);
	  snprintf (buf, sizeof (buf), "%s tried to purge you!\n\r", ch->name);
	  send_to_char (buf, victim);
	  return;
	}

      if (get_trust (ch) <= DEITY)
	{
	  send_to_char ("Not against PC's!\n\r", ch);
	  return;
	}

      act ("$n disintegrates $N.", ch, 0, victim, TO_NOTVICT);

      if (victim->level > 1)
	save_char_obj (victim);
      d = victim->desc;
      extract_char (victim, TRUE);
      if (d != NULL)
	close_socket (d);

      return;
    }

  act ("$n purges $N.", ch, NULL, victim, TO_NOTVICT);
  extract_char (victim, TRUE);
  return;
}



void
do_advance (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int level;
  int iLevel;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number (arg2))
    {
      send_to_char ("Syntax: advance <char> <level>.\n\r", ch);
      return;
    }

  if ((victim = get_char_world (ch, arg1)) == NULL)
    {
      send_to_char ("That player is not here.\n\r", ch);
      return;
    }

  if (IS_NPC (victim))
    {
      send_to_char ("Not on NPC's.\n\r", ch);
      return;
    }

  if ((level = atoi (arg2)) < 1 || level > 110)
    {
      send_to_char ("Level must be 1 to 110.\n\r", ch);
      return;
    }

  if (level > get_trust (ch))
    {
      send_to_char ("Limited to your trust level.\n\r", ch);
      return;
    }

  /*
   * Lower level:
   *   Reset to level 1.
   *   Then raise again.
   *   Currently, an imp can lower another imp.
   *   -- Swiftest
   */
  if (level <= victim->level)
    {
      int temp_prac;

      send_to_char ("Lowering a player's level!\n\r", ch);
      send_to_char ("`R******** `GOOOOHHHHHHHHHH  NNNNOOOO `R*******`x\n\r",
		    victim);
      snprintf (buf, sizeof (buf), "`R**** `WYou've been demoted to level %d `R****`x\n\r",
	       level);
      send_to_char (buf, victim);
      if ((victim->level > HERO) || (level > HERO))
	{
	  update_wizlist (victim, level);
	}
      temp_prac = victim->practice;
      victim->level = 1;
      victim->exp = exp_per_level (victim, victim->pcdata->points);
      victim->max_hit = 100;
      victim->max_mana = 100;
      victim->max_move = 100;
      victim->practice = 0;
      victim->hit = victim->max_hit;
      victim->mana = victim->max_mana;
      victim->move = victim->max_move;
      advance_level_quiet (victim);
      victim->practice = temp_prac;
    }
  else
    {
      send_to_char ("Raising a player's level!\n\r", ch);
      send_to_char ("`B******* `GOOOOHHHHHHHHHH  YYYYEEEESSS `B******`x\n\r",
		    victim);
      snprintf (buf, sizeof (buf), "`B**** `WYou've been advanced to level %d `B****`x\n\r",
	       level);
      send_to_char (buf, victim);
      if ((victim->level > HERO) || (level > HERO))
	{
	  update_wizlist (victim, level);
	}
    }

  for (iLevel = victim->level; iLevel < level; iLevel++)
    {
      victim->level += 1;
      advance_level_quiet (victim);
    }
  victim->exp = exp_per_level (victim, victim->pcdata->points)
    * UMAX (1, victim->level);
  victim->trust = 0;
  save_char_obj (victim);
  return;
}


void
do_knight (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int level;
  int iLevel;

  argument = one_argument (argument, arg1);

  if (!IS_SET (ch->act, PLR_KEY))
    {
      send_to_char ("This function is not currently implemented.\n\r", ch);
      return;
    }

  if (arg1[0] == '\0')
    {
      send_to_char ("Syntax: knight <char>.\n\r", ch);
      return;
    }

  if ((victim = get_char_room (ch, arg1)) == NULL)
    {
      send_to_char ("That player is not here.\n\r", ch);
      return;
    }

  if (IS_NPC (victim))
    {
      send_to_char ("Not on NPC's.\n\r", ch);
      return;
    }

  level = 103;

  if (level <= victim->level)
    {
      return;
    }
  else
    {
      act ("You touch $Ns shoulder with a sword called `GKnight's Faith`x.",
	   ch, NULL, victim, TO_CHAR);
      act ("$n touches your shoulder with a sword called `GKnight's Faith`x.",
	   ch, NULL, victim, TO_VICT);
      act ("$n touches $Ns shoulder with a sword called `GKnight's Faith`x.",
	   ch, NULL, victim, TO_NOTVICT);
      act ("$N glows with an unearthly light as $S mortality slips away.", ch,
	   NULL, victim, TO_NOTVICT);
    }
  update_wizlist (victim, level);
  for (iLevel = victim->level; iLevel < level; iLevel++)
    {
      send_to_char ("You raise a level!!  ", victim);
      victim->level += 1;
      advance_level (victim);
    }
  victim->exp = exp_per_level (victim, victim->pcdata->points)
    * UMAX (1, victim->level);
  victim->trust = 0;
  save_char_obj (victim);
  return;
}

void
do_squire (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int level;
  int iLevel;

  argument = one_argument (argument, arg1);

  if (!IS_SET (ch->act, PLR_KEY))
    {
      send_to_char ("This function is not currently implemented.\n\r", ch);
      return;
    }

  if (arg1[0] == '\0')
    {
      send_to_char ("Syntax: squire <char>.\n\r", ch);
      return;
    }

  if ((victim = get_char_room (ch, arg1)) == NULL)
    {
      send_to_char ("That player is not here.\n\r", ch);
      return;
    }

  if (IS_NPC (victim))
    {
      send_to_char ("Not on NPC's.\n\r", ch);
      return;
    }

  level = 102;

  if (level <= victim->level)
    {
      return;
    }
  else
    {
      act ("You touch $Ns shoulder with a sword called `BSquire's Faith`x.",
	   ch, NULL, victim, TO_CHAR);
      act ("$n touches your shoulder with a sword called `BSquire's Faith`x.",
	   ch, NULL, victim, TO_VICT);
      act ("$n touches $Ns shoulder with a sword called `BSquire's Faith`x.",
	   ch, NULL, victim, TO_NOTVICT);
      act ("$N glows with an unearthly light as $S mortality slips away.", ch,
	   NULL, victim, TO_NOTVICT);
    }
  update_wizlist (victim, level);
  for (iLevel = victim->level; iLevel < level; iLevel++)
    {
      send_to_char ("You raise a level!!  ", victim);
      victim->level += 1;
      advance_level (victim);
    }
  victim->exp = exp_per_level (victim, victim->pcdata->points)
    * UMAX (1, victim->level);
  victim->trust = 0;
  save_char_obj (victim);
  return;
}



void
do_trust (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int level;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);

  if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number (arg2))
    {
      send_to_char ("Syntax: trust <char> <level>.\n\r", ch);
      return;
    }

  if ((victim = get_char_world (ch, arg1)) == NULL)
    {
      send_to_char ("That player is not here.\n\r", ch);
      return;
    }

  if ((level = atoi (arg2)) < 0 || level > 110)
    {
      send_to_char ("Level must be 0 (reset) or 1 to 110.\n\r", ch);
      return;
    }

  if (level > get_trust (ch))
    {
      send_to_char ("Limited to your trust.\n\r", ch);
      return;
    }

  victim->trust = level;
  return;
}



void
do_restore (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *vch;
  DESCRIPTOR_DATA *d;

  one_argument (argument, arg);
  if (arg[0] == '\0' || !str_cmp (arg, "room"))
    {
      /* cure room */

      for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{
	  if (IS_SET (vch->act, PLR_NORESTORE))
	    {
	      act ("$n attempts to restore you, but fails.", ch, NULL, vch,
		   TO_VICT);
	    }
	  else
	    {
	      affect_strip (vch, gsn_plague);
	      affect_strip (vch, gsn_poison);
	      affect_strip (vch, gsn_blindness);
	      affect_strip (vch, gsn_sleep);
	      affect_strip (vch, gsn_curse);

	      vch->hit = vch->max_hit;
	      vch->mana = vch->max_mana;
	      vch->move = vch->max_move;
	      update_pos (vch);
	      act ("$n has restored you.", ch, NULL, vch, TO_VICT);
	    }
	}

      snprintf (buf, sizeof (buf), "$N restored room %d.", ch->in_room->vnum);
      wiznet (buf, ch, NULL, WIZ_RESTORE, WIZ_SECURE, get_trust (ch));

      send_to_char ("Room restored.\n\r", ch);
      return;

    }

  if (get_trust (ch) >= MAX_LEVEL - 2 && !str_cmp (arg, "all"))
    {
      /* cure all */

      for (d = descriptor_list; d != NULL; d = d->next)
	{
	  victim = d->character;

	  if (victim == NULL || IS_NPC (victim))
	    continue;

	  if (IS_SET (victim->act, PLR_NORESTORE))
	    {
	      act ("$n attempts to restore you, but fails.", ch, NULL, victim,
		   TO_VICT);
	    }
	  else
	    {
	      affect_strip (victim, gsn_plague);
	      affect_strip (victim, gsn_poison);
	      affect_strip (victim, gsn_blindness);
	      affect_strip (victim, gsn_sleep);
	      affect_strip (victim, gsn_curse);

	      victim->hit = victim->max_hit;
	      victim->mana = victim->max_mana;
	      victim->move = victim->max_move;
	      update_pos (victim);
	      if (victim->in_room != NULL)
		act ("$n has restored you.", ch, NULL, victim, TO_VICT);
	    }
	}
      send_to_char ("All active players restored.\n\r", ch);
      return;
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (IS_SET (victim->act, PLR_NORESTORE))
    {
      act ("$n attempts to restore you, but fails.", ch, NULL, victim,
	   TO_VICT);
      send_to_char ("You failed.\n\r", ch);
      return;
    }
  affect_strip (victim, gsn_plague);
  affect_strip (victim, gsn_poison);
  affect_strip (victim, gsn_blindness);
  affect_strip (victim, gsn_sleep);
  affect_strip (victim, gsn_curse);
  victim->hit = victim->max_hit;
  victim->mana = victim->max_mana;
  victim->move = victim->max_move;
  update_pos (victim);
  act ("$n has restored you.", ch, NULL, victim, TO_VICT);
  snprintf (buf, sizeof (buf), "$N restored %s",
	   IS_NPC (victim) ? victim->short_descr : victim->name);
  wiznet (buf, ch, NULL, WIZ_RESTORE, WIZ_SECURE, get_trust (ch));
  send_to_char ("Ok.\n\r", ch);
  return;
}

void
do_immkiss (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Who do you want to kiss?\n\r", ch);
      return;
    }
  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }
  if (ch->in_room != victim->in_room)
    {
      send_to_char ("Your lips aren't that long!\n\r", ch);
      return;
    }
  affect_strip (victim, gsn_plague);
  affect_strip (victim, gsn_poison);
  affect_strip (victim, gsn_blindness);
  affect_strip (victim, gsn_sleep);
  affect_strip (victim, gsn_curse);
  victim->hit = victim->max_hit;
  victim->mana = victim->max_mana;
  victim->move = victim->max_move;
  update_pos (victim);
  act ("$n kisses you, and you feel a sudden rush of adrenaline.", ch, NULL,
       victim, TO_VICT);
  send_to_char ("You feel MUCH better now!\n\r", victim);
  send_to_char ("They feel MUCH better now!\n\r", ch);
  snprintf (buf, sizeof (buf), "$N immkissed %s",
	   IS_NPC (victim) ? victim->short_descr : victim->name);
  wiznet (buf, ch, NULL, WIZ_RESTORE, WIZ_SECURE, get_trust (ch));
  return;
}


void
do_freeze (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Freeze whom?\n\r", ch);
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

  if (IS_SET (victim->act, PLR_FREEZE))
    {
      REMOVE_BIT (victim->act, PLR_FREEZE);
      send_to_char ("You can play again.\n\r", victim);
      send_to_char ("FREEZE removed.\n\r", ch);
      snprintf (buf, sizeof (buf), "$N thaws %s.", victim->name);
      wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }
  else
    {
      SET_BIT (victim->act, PLR_FREEZE);
      send_to_char ("You can't do ANYthing!\n\r", victim);
      send_to_char ("FREEZE set.\n\r", ch);
      snprintf (buf, sizeof (buf), "$N puts %s in the deep freeze.", victim->name);
      wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }

  save_char_obj (victim);

  return;
}

void
do_norestore (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Norestore whom?\n\r", ch);
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

  if (IS_SET (victim->act, PLR_NORESTORE))
    {
      REMOVE_BIT (victim->act, PLR_NORESTORE);
      send_to_char ("NORESTORE removed.\n\r", ch);
      snprintf (buf, sizeof (buf), "$N allows %s restores.", victim->name);
      wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }
  else
    {
      SET_BIT (victim->act, PLR_NORESTORE);
      send_to_char ("NORESTORE set.\n\r", ch);
      snprintf (buf, sizeof (buf), "$N denys %s restores.", victim->name);
      wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }

  save_char_obj (victim);

  return;
}


void
do_notitle (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Notitle whom?\n\r", ch);
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

  if (IS_SET (victim->act, PLR_NOTITLE))
    {
      REMOVE_BIT (victim->act, PLR_NOTITLE);
      send_to_char ("NOTITLE removed.\n\r", ch);
      snprintf (buf, sizeof (buf), "$N allows %s title.", victim->name);
      wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }
  else
    {
      SET_BIT (victim->act, PLR_NOTITLE);
      send_to_char ("NOTITLE set.\n\r", ch);
      snprintf (buf, sizeof (buf), "$N denys %s title.", victim->name);
      wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }

  save_char_obj (victim);

  return;
}

void
do_log (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Log whom?\n\r", ch);
      return;
    }

  if (!str_cmp (arg, "all"))
    {
      if (fLogAll)
	{
	  fLogAll = FALSE;
	  send_to_char ("Log ALL off.\n\r", ch);
	}
      else
	{
	  fLogAll = TRUE;
	  send_to_char ("Log ALL on.\n\r", ch);
	}
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

  /*
   * No level check, gods can log anyone.
   */
  if (IS_SET (victim->act, PLR_LOG))
    {
      REMOVE_BIT (victim->act, PLR_LOG);
      send_to_char ("LOG removed.\n\r", ch);
    }
  else
    {
      SET_BIT (victim->act, PLR_LOG);
      send_to_char ("LOG set.\n\r", ch);
    }

  return;
}



/* RT set replaces sset, mset, oset, and rset */

