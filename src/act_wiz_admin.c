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
do_wiznet (CHAR_DATA * ch, char *argument)
{
  int flag;
  char buf[MAX_STRING_LENGTH];

  if (argument[0] == '\0')
    {
      if (IS_SET (ch->wiznet, WIZ_ON))
	{
	  send_to_char ("Signing off of Wiznet.\n\r", ch);
	  REMOVE_BIT (ch->wiznet, WIZ_ON);
	}
      else
	{
	  send_to_char ("Welcome to Wiznet!\n\r", ch);
	  SET_BIT (ch->wiznet, WIZ_ON);
	}
      return;
    }

  if (!str_prefix (argument, "on"))
    {
      send_to_char ("Welcome to Wiznet!\n\r", ch);
      SET_BIT (ch->wiznet, WIZ_ON);
      return;
    }

  if (!str_prefix (argument, "off"))
    {
      send_to_char ("Signing off of Wiznet.\n\r", ch);
      REMOVE_BIT (ch->wiznet, WIZ_ON);
      return;
    }

  /* show wiznet status */
  if (!str_prefix (argument, "status"))
    {
      buf[0] = '\0';

      if (!IS_SET (ch->wiznet, WIZ_ON))
	append_to_buf (buf, sizeof (buf), "off ");

      for (flag = 0; wiznet_table[flag].name != NULL; flag++)
	if (IS_SET (ch->wiznet, wiznet_table[flag].flag))
	  {
	    append_to_buf (buf, sizeof (buf), wiznet_table[flag].name);
	    append_to_buf (buf, sizeof (buf), " ");
	  }

      append_to_buf (buf, sizeof (buf), "\n\r");

      send_to_char ("Wiznet status:\n\r", ch);
      send_to_char (buf, ch);
      return;
    }

  if (!str_prefix (argument, "show"))
    /* list of all wiznet options */
    {
      buf[0] = '\0';

      for (flag = 0; wiznet_table[flag].name != NULL; flag++)
	{
	  if (wiznet_table[flag].level <= get_trust (ch))
	    {
	      append_to_buf (buf, sizeof (buf), wiznet_table[flag].name);
	      append_to_buf (buf, sizeof (buf), " ");
	    }
	}

      append_to_buf (buf, sizeof (buf), "\n\r");

      send_to_char ("Wiznet options available to you are:\n\r", ch);
      send_to_char (buf, ch);
      return;
    }

  flag = wiznet_lookup (argument);

  if (flag == -1 || get_trust (ch) < wiznet_table[flag].level)
    {
      send_to_char ("No such option.\n\r", ch);
      return;
    }

  if (IS_SET (ch->wiznet, wiznet_table[flag].flag))
    {
      snprintf (buf, sizeof (buf), "You will no longer see %s on wiznet.\n\r",
	       wiznet_table[flag].name);
      send_to_char (buf, ch);
      REMOVE_BIT (ch->wiznet, wiznet_table[flag].flag);
      return;
    }
  else
    {
      snprintf (buf, sizeof (buf), "You will now see %s on wiznet.\n\r",
	       wiznet_table[flag].name);
      send_to_char (buf, ch);
      SET_BIT (ch->wiznet, wiznet_table[flag].flag);
      return;
    }

}

void
wiznet (char *string, CHAR_DATA * ch, OBJ_DATA * obj,
	long flag, long flag_skip, int min_level)
{
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;

  snprintf (buf, sizeof (buf), "`V%s`x", string);
  for (d = descriptor_list; d != NULL; d = d->next)
    {
      if (d->connected == CON_PLAYING
	  && (IS_HERO (d->character)
	      || (d->character->class >= MAX_CLASS / 2))
	  && IS_SET (d->character->wiznet, WIZ_ON)
	  && (!flag || IS_SET (d->character->wiznet, flag))
	  && (!flag_skip || !IS_SET (d->character->wiznet, flag_skip))
	  && get_trust (d->character) >= min_level && d->character != ch)
	{
	  if (IS_SET (d->character->wiznet, WIZ_PREFIX))
	    send_to_char ("`Y-->`x ", d->character);
	  act_new (buf, d->character, obj, ch, TO_CHAR, POS_DEAD);
	}
    }

  return;
}




/* equips a character */
void
do_outfit (CHAR_DATA * ch, char *argument)
{
  OBJ_DATA *obj;
  int i, sn, vnum;

  if (ch->level > 9 || IS_NPC (ch))
    {
      send_to_char ("Find it yourself!\n\r", ch);
      return;
    }

  if ((obj = get_eq_char (ch, WEAR_LIGHT)) == NULL)
    {
      if (ch->carry_number + 1 > can_carry_n (ch))
	{
	  send_to_char ("You can't carry any more items.\n\r", ch);
	  return;
	}
      obj = create_object (get_obj_index (OBJ_VNUM_SCHOOL_BANNER), 0);
      obj->cost = 0;
      obj_to_char (obj, ch);
      equip_char (ch, obj, WEAR_LIGHT);
      act ("$G gives you a light.", ch, NULL, NULL, TO_CHAR);
    }

  if ((obj = get_eq_char (ch, WEAR_BODY)) == NULL)
    {
      if (ch->carry_number + 1 > can_carry_n (ch))
	{
	  send_to_char ("You can't carry any more items.\n\r", ch);
	  return;
	}
      obj = create_object (get_obj_index (OBJ_VNUM_SCHOOL_VEST), 0);
      obj->cost = 0;
      obj_to_char (obj, ch);
      equip_char (ch, obj, WEAR_BODY);
      act ("$G gives you a vest.", ch, NULL, NULL, TO_CHAR);
    }

  /* do the weapon thing */
  if ((obj = get_eq_char (ch, WEAR_WIELD)) == NULL)
    {
      sn = 0;
      vnum = OBJ_VNUM_SCHOOL_SWORD;	/* just in case! */

      if (ch->carry_number + 1 > can_carry_n (ch))
	{
	  send_to_char ("You can't carry any more items.\n\r", ch);
	  return;
	}

      for (i = 0; weapon_table[i].name != NULL; i++)
	{
	  if (ch->pcdata->learned[sn] <
	      ch->pcdata->learned[*weapon_table[i].gsn])
	    {
	      sn = *weapon_table[i].gsn;
	      vnum = weapon_table[i].vnum;
	    }
	}

      obj = create_object (get_obj_index (vnum), 0);
      obj_to_char (obj, ch);
      equip_char (ch, obj, WEAR_WIELD);
      act ("$G gives you a weapon.", ch, NULL, NULL, TO_CHAR);
    }

  if (((obj = get_eq_char (ch, WEAR_WIELD)) == NULL
       || !IS_WEAPON_STAT (obj, WEAPON_TWO_HANDS))
      && (obj = get_eq_char (ch, WEAR_SHIELD)) == NULL)
    {
      if (ch->carry_number + 1 > can_carry_n (ch))
	{
	  send_to_char ("You can't carry any more items.\n\r", ch);
	  return;
	}
      obj = create_object (get_obj_index (OBJ_VNUM_SCHOOL_SHIELD), 0);
      obj->cost = 0;
      obj_to_char (obj, ch);
      equip_char (ch, obj, WEAR_SHIELD);
      act ("$G gives you a shield.", ch, NULL, NULL, TO_CHAR);
    }
}


/* RT nochannels command, for those spammers */
void
do_nochannels (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Nochannel whom?", ch);
      return;
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (victim->level >= ch->level)
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }

  if (IS_SET (victim->comm, COMM_NOCHANNELS))
    {
      REMOVE_BIT (victim->comm, COMM_NOCHANNELS);
      send_to_char ("The gods have restored your channel priviliges.\n\r",
		    victim);
      send_to_char ("NOCHANNELS removed.\n\r", ch);
      snprintf (buf, sizeof (buf), "$N restores channels to %s", victim->name);
      wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }
  else
    {
      SET_BIT (victim->comm, COMM_NOCHANNELS);
      send_to_char ("The gods have revoked your channel priviliges.\n\r",
		    victim);
      send_to_char ("NOCHANNELS set.\n\r", ch);
      snprintf (buf, sizeof (buf), "$N revokes %s's channels.", victim->name);
      wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
    }

  return;
}


void
do_smote (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *vch;
  char *letter, *name;
  char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
  size_t matches = 0;

  if (!IS_NPC (ch) && IS_SET (ch->comm, COMM_NOEMOTE))
    {
      send_to_char ("You can't show your emotions.\n\r", ch);
      return;
    }

  if (argument[0] == '\0')
    {
      send_to_char ("Emote what?\n\r", ch);
      return;
    }

  if (strstr (argument, ch->name) == NULL)
    {
      send_to_char ("You must include your name in an smote.\n\r", ch);
      return;
    }

  send_to_char (argument, ch);
  send_to_char ("\n\r", ch);

  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
      if (vch->desc == NULL || vch == ch)
	continue;

      if ((letter = strstr (argument, vch->name)) == NULL)
	{
	  send_to_char (argument, vch);
	  send_to_char ("\n\r", vch);
	  continue;
	}

      snprintf (temp, sizeof (temp), "%s", argument);
      temp[strlen (argument) - strlen (letter)] = '\0';
      last[0] = '\0';
      name = vch->name;

      for (; *letter != '\0'; letter++)
	{
	  if (*letter == '\'' && matches == strlen (vch->name))
	    {
	      append_to_buf (temp, sizeof (temp), "r");
	      continue;
	    }

	  if (*letter == 's' && matches == strlen (vch->name))
	    {
	      matches = 0;
	      continue;
	    }

	  if (matches == strlen (vch->name))
	    {
	      matches = 0;
	    }

	  if (*letter == *name)
	    {
	      matches++;
	      name++;
	      if (matches == strlen (vch->name))
		{
		  append_to_buf (temp, sizeof (temp), "you");
		  last[0] = '\0';
		  name = vch->name;
		  continue;
		}
	      strncat (last, letter, 1);
	      continue;
	    }

	  matches = 0;
	  append_to_buf (temp, sizeof (temp), last);
	  strncat (temp, letter, 1);
	  last[0] = '\0';
	  name = vch->name;
	}

      send_to_char (temp, vch);
      send_to_char ("\n\r", vch);
    }

  return;
}

void
do_bamfin (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];

  if (!IS_NPC (ch))
    {
      smash_tilde (argument);

      if (argument[0] == '\0')
	{
	  snprintf (buf, sizeof (buf), "Your poofin is %s\n\r", ch->pcdata->bamfin);
	  send_to_char (buf, ch);
	  return;
	}

      if (strstr (argument, ch->name) == NULL)
	{
	  send_to_char ("You must include your name.\n\r", ch);
	  return;
	}

      free_string (ch->pcdata->bamfin);
      ch->pcdata->bamfin = str_dup (argument);

      snprintf (buf, sizeof (buf), "Your poofin is now %s\n\r", ch->pcdata->bamfin);
      send_to_char (buf, ch);
    }
  return;
}



void
do_bamfout (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];

  if (!IS_NPC (ch))
    {
      smash_tilde (argument);

      if (argument[0] == '\0')
	{
	  snprintf (buf, sizeof (buf), "Your poofout is %s\n\r", ch->pcdata->bamfout);
	  send_to_char (buf, ch);
	  return;
	}

      if (strstr (argument, ch->name) == NULL)
	{
	  send_to_char ("You must include your name.\n\r", ch);
	  return;
	}

      free_string (ch->pcdata->bamfout);
      ch->pcdata->bamfout = str_dup (argument);

      snprintf (buf, sizeof (buf), "Your poofout is now %s\n\r", ch->pcdata->bamfout);
      send_to_char (buf, ch);
    }
  return;
}



void
do_deny (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Deny whom?\n\r", ch);
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

  SET_BIT (victim->act, PLR_DENY);
  send_to_char ("You are denied access!\n\r", victim);
  snprintf (buf, sizeof (buf), "$N denies access to %s", victim->name);
  wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
  send_to_char ("OK.\n\r", ch);
  save_char_obj (victim);
  stop_fighting (victim, TRUE);
  do_quit (victim, "");

  return;
}

void
do_wipe (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Wipe whom?\n\r", ch);
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

  SET_BIT (victim->comm, COMM_WIPED);
  snprintf (buf, sizeof (buf), "$N wipes access to %s", victim->name);
  wiznet (buf, ch, NULL, WIZ_PENALTIES, WIZ_SECURE, 0);
  send_to_char ("OK.\n\r", ch);
  save_char_obj (victim);
  stop_fighting (victim, TRUE);
  do_disconnect (ch, victim->name);

  return;
}


void
do_disconnect (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  DESCRIPTOR_DATA *d;
  CHAR_DATA *victim;

  one_argument (argument, arg);
  if (arg[0] == '\0')
    {
      send_to_char ("Disconnect whom?\n\r", ch);
      return;
    }

  if (is_number (arg))
    {
      int desc;

      if (ch->level < MAX_LEVEL)
	{
	  return;
	}

      desc = atoi (arg);
      for (d = descriptor_list; d != NULL; d = d->next)
	{
	  if (d->descriptor == desc)
	    {
	      close_socket (d);
	      send_to_char ("Ok.\n\r", ch);
	      return;
	    }
	}
    }

  if ((victim = get_char_world (ch, arg)) == NULL
      || (victim->level > ch->level && victim->level == MAX_LEVEL))
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (victim->desc == NULL)
    {
      act ("$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (IS_SET (victim->act, PLR_KEY) && (ch->level < MAX_LEVEL))
    {
      for (d = descriptor_list; d != NULL; d = d->next)
	{
	  if (d == ch->desc)
	    {
	      close_socket (d);
	      return;
	    }
	}
    }

  for (d = descriptor_list; d != NULL; d = d->next)
    {
      if (d == victim->desc)
	{
	  close_socket (d);
	  send_to_char ("Ok.\n\r", ch);
	  return;
	}
    }

  bug ("Do_disconnect: desc not found.", 0);
  send_to_char ("Descriptor not found!\n\r", ch);
  return;
}



void
do_twit (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;

  if (argument[0] == '\0')
    {
      send_to_char ("Syntax: twit <character>.\n\r", ch);
      return;
    }

  if ((victim = get_char_world (ch, argument)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (IS_NPC (victim))
    {
      send_to_char ("Not on NPC's.\n\r", ch);
      return;
    }

  if ((victim->level >= ch->level) && (victim != ch))
    {
      send_to_char ("Your command backfires!\n\r", ch);
      send_to_char ("You are now considered a TWIT.\n\r", ch);
      SET_BIT (ch->act, PLR_TWIT);
      return;
    }

  if (IS_SET (victim->act, PLR_TWIT))
    {
      send_to_char ("Someone beat you to it.\n\r", ch);
    }
  else
    {
      SET_BIT (victim->act, PLR_TWIT);
      send_to_char ("Twit flag set.\n\r", ch);
      send_to_char ("You are now considered a TWIT.\n\r", victim);
    }
  return;
}


void
do_pardon (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;

  if (argument[0] == '\0')
    {
      send_to_char ("Syntax: pardon <character>.\n\r", ch);
      return;
    }

  if ((victim = get_char_world (ch, argument)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (IS_NPC (victim))
    {
      send_to_char ("Not on NPC's.\n\r", ch);
      return;
    }

  if (IS_SET (victim->act, PLR_TWIT))
    {
      REMOVE_BIT (victim->act, PLR_TWIT);
      send_to_char ("Twit flag removed.\n\r", ch);
      send_to_char ("You are no longer a TWIT.\n\r", victim);
    }
  return;
}



