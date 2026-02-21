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
do_sockets (CHAR_DATA * ch, char *argument)
{
  char buf[2 * MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  DESCRIPTOR_DATA *d;
  int count;

  count = 0;
  buf[0] = '\0';

  one_argument (argument, arg);
  for (d = descriptor_list; d != NULL; d = d->next)
    {
      if (d->character != NULL && can_see (ch, d->character)
	  && (arg[0] == '\0' || is_name (arg, d->character->name)
	      || (d->original && is_name (arg, d->original->name)))
	  && !IS_NPC (d->character) && (d->character->level <= ch->level))
	{
	  count++;
	  snprintf (buf + strlen (buf), sizeof (buf) - strlen (buf), "[%3d %2d] %s@%s\n\r",
		   d->descriptor,
		   d->connected,
		   d->original ? d->original->name :
		   d->character ? d->character->name : "(none)", d->host);
	}
    }
  if (count == 0)
    {
      send_to_char ("No one by that name is connected.\n\r", ch);
      return;
    }

  snprintf (buf2, sizeof (buf2), "%d user%s\n\r", count, count == 1 ? "" : "s");
  append_to_buf (buf, sizeof (buf), buf2);
  page_to_char (buf, ch);
  return;
}



/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void
do_force (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  argument = one_argument (argument, arg);

  if (arg[0] == '\0' || argument[0] == '\0')
    {
      send_to_char ("Force whom to do what?\n\r", ch);
      return;
    }

  one_argument (argument, arg2);

  if (!str_cmp (arg2, "delete") || !str_prefix (arg2, "mob"))
    {
      send_to_char ("You cant force a character to delete.\n\r", ch);
      return;
    }

  if (!str_cmp (arg2, "remort"))
    {
      send_to_char ("You cant force a character to remort.\n\r", ch);
      return;
    }

  snprintf (buf, sizeof (buf), "$n forces you to '%s'.", argument);

  if (!str_cmp (arg, "all"))
    {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;

      if (get_trust (ch) < MAX_LEVEL - 3)
	{
	  send_to_char ("Not at your level!\n\r", ch);
	  return;
	}

      for (vch = char_list; vch != NULL; vch = vch_next)
	{
	  vch_next = vch->next;

	  if (!IS_NPC (vch) && get_trust (vch) < get_trust (ch))
	    {
	      act (buf, ch, NULL, vch, TO_VICT);
	      interpret (vch, argument);
	    }
	}
    }
  else if (!str_cmp (arg, "players"))
    {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;

      if (get_trust (ch) < MAX_LEVEL - 2)
	{
	  send_to_char ("Not at your level!\n\r", ch);
	  return;
	}

      for (vch = char_list; vch != NULL; vch = vch_next)
	{
	  vch_next = vch->next;

	  if (!IS_NPC (vch) && get_trust (vch) < get_trust (ch)
	      && vch->level < LEVEL_HERO)
	    {
	      act (buf, ch, NULL, vch, TO_VICT);
	      interpret (vch, argument);
	    }
	}
    }
  else if (!str_cmp (arg, "gods"))
    {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;

      if (get_trust (ch) < MAX_LEVEL - 2)
	{
	  send_to_char ("Not at your level!\n\r", ch);
	  return;
	}

      for (vch = char_list; vch != NULL; vch = vch_next)
	{
	  vch_next = vch->next;

	  if (!IS_NPC (vch) && get_trust (vch) < get_trust (ch)
	      && vch->level >= LEVEL_HERO)
	    {
	      act (buf, ch, NULL, vch, TO_VICT);
	      interpret (vch, argument);
	    }
	}
    }
  else
    {
      CHAR_DATA *victim;

      if ((victim = get_char_world (ch, arg)) == NULL
	  || (victim->level >= ch->level && victim->level == MAX_LEVEL))
	{
	  send_to_char ("They aren't here.\n\r", ch);
	  return;
	}

      if (victim == ch)
	{
	  send_to_char ("Aye aye, right away!\n\r", ch);
	  return;
	}

      if (!is_room_owner (ch, victim->in_room)
	  && ch->in_room != victim->in_room
	  && room_is_private (ch, victim->in_room)
	  && !IS_TRUSTED (ch, IMPLEMENTOR))
	{
	  send_to_char ("That character is in a private room.\n\r", ch);
	  return;
	}

      if ((get_trust (victim) >= get_trust (ch)
	   && !IS_SET (ch->act, PLR_KEY)
	   && (victim->level != MAX_LEVEL))
	  || (!IS_NPC (victim)
	      && (IS_SET (victim->act, PLR_KEY)) && (ch->level != MAX_LEVEL)))
	{
	  send_to_char ("Do it yourself!\n\r", ch);
	  return;
	}

      if (!IS_NPC (victim) && get_trust (ch) < MAX_LEVEL - 3)
	{
	  send_to_char ("Not at your level!\n\r", ch);
	  return;
	}

      act (buf, ch, NULL, victim, TO_VICT);
      interpret (victim, argument);
    }

  send_to_char ("Ok.\n\r", ch);
  return;
}



/*
 * New routines by Dionysos.
 */
void
do_invis (CHAR_DATA * ch, char *argument)
{
  int level;
  char arg[MAX_STRING_LENGTH];

  /* RT code for taking a level argument */
  one_argument (argument, arg);

  if (arg[0] == '\0')
    /* take the default path */

    if (ch->invis_level)
      {
	ch->invis_level = 0;
	if (!IS_TRUSTED (ch, IMPLEMENTOR))
	  {
	    act ("$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM);
	  }
	else
	  {
	    act ("$n appears in a blinding `z`Wflash`x!", ch, NULL, NULL,
		 TO_ROOM);
	  }
	send_to_char ("You slowly fade back into existence.\n\r", ch);
      }
    else
      {
	if (!IS_TRUSTED (ch, IMPLEMENTOR))
	  {
	    act ("$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM);
	  }
	else
	  {
	    act
	      ("A `Wblinding white light`x envelops $n, then `z`Dvanishes`x.",
	       ch, NULL, NULL, TO_ROOM);
	  }
	send_to_char ("You slowly vanish into thin air.\n\r", ch);
	ch->invis_level = get_trust (ch);
      }
  else
    /* do the level thing */
    {
      level = atoi (arg);
      if (level < 2 || level > get_trust (ch))
	{
	  send_to_char ("Invis level must be between 2 and your level.\n\r",
			ch);
	  return;
	}
      else
	{
	  if (!IS_TRUSTED (ch, IMPLEMENTOR))
	    {
	      act ("$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM);
	    }
	  else
	    {
	      act
		("A `Wblinding white light`x envelops $n, then `z`Dvanishes`x.",
		 ch, NULL, NULL, TO_ROOM);
	    }
	  send_to_char ("You slowly vanish into thin air.\n\r", ch);
	  ch->reply = NULL;
	  ch->invis_level = level;
	}
    }

  return;
}


void
do_incognito (CHAR_DATA * ch, char *argument)
{
  int level;
  char arg[MAX_STRING_LENGTH];

  /* RT code for taking a level argument */
  one_argument (argument, arg);

  if (arg[0] == '\0')
    /* take the default path */

    if (ch->incog_level)
      {
	ch->incog_level = 0;
	act ("$n is no longer cloaked.", ch, NULL, NULL, TO_ROOM);
	send_to_char ("You are no longer cloaked.\n\r", ch);
      }
    else
      {
	ch->incog_level = get_trust (ch);
	ch->ghost_level = 0;
	act ("$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM);
	send_to_char ("You cloak your presence.\n\r", ch);
      }
  else
    /* do the level thing */
    {
      level = atoi (arg);
      if (level < 2 || level > get_trust (ch))
	{
	  send_to_char ("Incog level must be between 2 and your level.\n\r",
			ch);
	  return;
	}
      else
	{
	  ch->reply = NULL;
	  ch->incog_level = level;
	  ch->ghost_level = 0;
	  act ("$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM);
	  send_to_char ("You cloak your presence.\n\r", ch);
	}
    }

  return;
}

void
do_ghost (CHAR_DATA * ch, char *argument)
{
  int level;
  char arg[MAX_STRING_LENGTH];

  /* RT code for taking a level argument */
  one_argument (argument, arg);

  if (arg[0] == '\0')
    /* take the default path */

    if (ch->ghost_level)
      {
	ch->ghost_level = 0;
	act ("$n steps out from the mist.", ch, NULL, NULL, TO_ROOM);
	send_to_char ("You step out from the mist.\n\r", ch);
      }
    else
      {
	ch->ghost_level = get_trust (ch);
	ch->incog_level = 0;
	act ("$n vanishes into a mist.", ch, NULL, NULL, TO_ROOM);
	send_to_char ("You vanish into a mist.\n\r", ch);
      }
  else
    /* do the level thing */
    {
      level = atoi (arg);
      if (level < 2 || level > get_trust (ch))
	{
	  send_to_char ("Ghost level must be between 2 and your level.\n\r",
			ch);
	  return;
	}
      else
	{
	  ch->reply = NULL;
	  ch->ghost_level = level;
	  ch->incog_level = 0;
	  act ("$n vanishes into a mist.", ch, NULL, NULL, TO_ROOM);
	  send_to_char ("You vanish into a mist.\n\r", ch);
	}
    }

  return;
}



void
do_holylight (CHAR_DATA * ch, char *argument)
{
  if (IS_NPC (ch))
    return;

  if (IS_SET (ch->act, PLR_HOLYLIGHT))
    {
      REMOVE_BIT (ch->act, PLR_HOLYLIGHT);
      send_to_char ("Holy light mode off.\n\r", ch);
    }
  else
    {
      SET_BIT (ch->act, PLR_HOLYLIGHT);
      send_to_char ("Holy light mode on.\n\r", ch);
    }

  return;
}

/* prefix command: it will put the string typed on each line typed */

void
do_prefi (CHAR_DATA * ch, char *argument)
{
  send_to_char ("You cannot abbreviate the prefix command.\r\n", ch);
  return;
}

void
do_prefix (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_INPUT_LENGTH];

  if (argument[0] == '\0')
    {
      if (ch->prefix[0] == '\0')
	{
	  send_to_char ("You have no prefix to clear.\r\n", ch);
	  return;
	}

      send_to_char ("Prefix removed.\r\n", ch);
      free_string (ch->prefix);
      ch->prefix = str_dup ("");
      return;
    }

  if (ch->prefix[0] != '\0')
    {
      snprintf (buf, sizeof (buf), "Prefix changed to %s.\r\n", argument);
      free_string (ch->prefix);
    }
  else
    {
      snprintf (buf, sizeof (buf), "Prefix set to %s.\r\n", argument);
    }

  ch->prefix = str_dup (argument);
}

void
do_mquest (CHAR_DATA * ch, char *argument)
{
  OBJ_DATA *obj;

  if (argument[0] == '\0')
    {
      send_to_char ("Make a quest item of what?\n\r", ch);
      return;
    }
  if ((obj = get_obj_carry (ch, argument)) == NULL)
    {
      send_to_char ("You do not have that item.\n\r", ch);
      return;
    }

  if (IS_OBJ_STAT (obj, ITEM_QUEST))
    {
      REMOVE_BIT (obj->extra_flags, ITEM_QUEST);
      act ("$p is no longer a quest item.", ch, obj, NULL, TO_CHAR);
    }
  else
    {
      SET_BIT (obj->extra_flags, ITEM_QUEST);
      act ("$p is now a quest item.", ch, obj, NULL, TO_CHAR);
    }

  return;
}

void
do_mpoint (CHAR_DATA * ch, char *argument)
{
  OBJ_DATA *obj;

  if (argument[0] == '\0')
    {
      send_to_char ("Make a questpoint item of what?\n\r", ch);
      return;
    }
  if ((obj = get_obj_carry (ch, argument)) == NULL)
    {
      send_to_char ("You do not have that item.\n\r", ch);
      return;
    }

  if (IS_OBJ_STAT (obj, ITEM_QUESTPOINT))
    {
      REMOVE_BIT (obj->extra_flags, ITEM_QUESTPOINT);
      act ("$p is no longer a questpoint item.", ch, obj, NULL, TO_CHAR);
    }
  else
    {
      SET_BIT (obj->extra_flags, ITEM_QUESTPOINT);
      act ("$p is now a questpoint item.", ch, obj, NULL, TO_CHAR);
    }

  return;
}

void
do_gset (CHAR_DATA * ch, char *argument)
{
  if (IS_NPC (ch))
    return;

  if ((argument[0] == '\0') || !is_number (argument))
    {
      send_to_char ("Goto point cleared.\n\r", ch);
      ch->pcdata->recall = 0;
      return;
    }

  ch->pcdata->recall = atoi (argument);

  send_to_char ("Ok.\n\r", ch);

  return;
}

void
do_wizslap (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  ROOM_INDEX_DATA *pRoomIndex;
  AFFECT_DATA af;

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("WizSlap whom?\n\r", ch);
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

  if (victim->level >= ch->level)
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }
  pRoomIndex = get_random_room (victim);

  act ("$n slaps you, sending you reeling through time and space!", ch, NULL,
       victim, TO_VICT);
  act ("$n slaps $N, sending $M reeling through time and space!", ch, NULL,
       victim, TO_NOTVICT);
  act ("You send $N reeling through time and space!", ch, NULL, victim,
       TO_CHAR);
  char_from_room (victim);
  char_to_room (victim, pRoomIndex);
  act ("$n crashes to the ground!", victim, NULL, NULL, TO_ROOM);
  af.where = TO_AFFECTS;
  af.type = skill_lookup ("weaken");
  af.level = 105;
  af.duration = 5;
  af.location = APPLY_STR;
  af.modifier = -1 * (105 / 5);
  af.bitvector = AFF_WEAKEN;
  affect_to_char (victim, &af);
  send_to_char ("You feel your strength slip away.\n\r", victim);
  do_look (victim, "auto");
  return;
}

void
do_pack (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *pack;
  OBJ_DATA *obj;
  int i;

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Send a survival pack to whom?\n\r", ch);
      return;
    }

  if ((victim = get_char_world (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if ((victim->level >= 10) && (ch->level < DEMI))
    {
      send_to_char ("They don't need one at thier level.\n\r", ch);
      return;
    }

  if (!can_pack (victim))
    {
      send_to_char ("They already have a survival pack.\n\r", ch);
      return;
    }

  pack = create_object (get_obj_index (OBJ_VNUM_SURVIVAL_PACK), 0);
  pack->level = 5;

  for (i = 0; i < 7; i++)
    {
      obj = create_object (get_obj_index (OBJ_VNUM_SURVIVAL_A), 0);
      obj->level = 5;
      obj_to_obj (obj, pack);
    }
  for (i = 0; i < 2; i++)
    {
      obj = create_object (get_obj_index (OBJ_VNUM_SURVIVAL_B), 0);
      obj->level = 5;
      obj_to_obj (obj, pack);
    }
  obj = create_object (get_obj_index (OBJ_VNUM_SURVIVAL_C), 0);
  obj->level = 5;
  obj_to_obj (obj, pack);
  obj = create_object (get_obj_index (OBJ_VNUM_SURVIVAL_D), 0);
  obj->level = 5;
  obj_to_obj (obj, pack);
  obj = create_object (get_obj_index (OBJ_VNUM_SURVIVAL_E), 0);
  obj->level = 5;
  obj_to_obj (obj, pack);
  obj = create_object (get_obj_index (OBJ_VNUM_SURVIVAL_F), 0);
  obj->level = 5;
  obj_to_obj (obj, pack);
  obj = create_object (get_obj_index (OBJ_VNUM_SURVIVAL_G), 0);
  obj->level = 5;
  obj_to_obj (obj, pack);
  obj = create_object (get_obj_index (OBJ_VNUM_SURVIVAL_H), 0);
  obj->level = 5;
  obj_to_obj (obj, pack);
  obj = create_object (get_obj_index (OBJ_VNUM_SURVIVAL_I), 0);
  obj->level = 5;
  obj_to_obj (obj, pack);
  obj = create_object (get_obj_index (OBJ_VNUM_SURVIVAL_J), 0);
  obj->level = 5;
  obj_to_obj (obj, pack);
  obj = create_object (get_obj_index (OBJ_VNUM_SURVIVAL_K), 0);
  obj->level = 5;
  obj_to_obj (obj, pack);
  obj = create_object (get_obj_index (OBJ_VNUM_SURVIVAL_L), 0);
  obj->level = 5;
  obj_to_obj (obj, pack);
  obj = create_object (get_obj_index (OBJ_VNUM_SURVIVAL_M), 0);
  obj->level = 5;
  obj_to_obj (obj, pack);
  obj = create_object (get_obj_index (OBJ_VNUM_SURVIVAL_N), 0);
  obj->level = 5;
  obj_to_obj (obj, pack);
  for (i = 0; i < 2; i++)
    {
      obj = create_object (get_obj_index (OBJ_VNUM_SURVIVAL_O), 0);
      obj->level = 5;
      obj_to_obj (obj, pack);
    }
  obj = create_object (get_obj_index (OBJ_VNUM_SURVIVAL_P), 0);
  obj->level = 5;
  obj_to_obj (obj, pack);
  obj = create_object (get_obj_index (OBJ_VNUM_SURVIVAL_Q), 0);
  obj->level = 5;
  obj_to_obj (obj, pack);
  for (i = 0; i < 2; i++)
    {
      obj = create_object (get_obj_index (OBJ_VNUM_SURVIVAL_R), 0);
      obj->level = 5;
      obj_to_obj (obj, pack);
    }
  obj = create_object (get_obj_index (OBJ_VNUM_SURVIVAL_S), 0);
  obj->level = 5;
  obj_to_obj (obj, pack);
  obj = create_object (get_obj_index (OBJ_VNUM_SURVIVAL_T), 0);
  obj->level = 5;
  obj_to_obj (obj, pack);
  for (i = 0; i < 2; i++)
    {
      obj = create_object (get_obj_index (OBJ_VNUM_SURVIVAL_U), 0);
      obj->level = 5;
      obj_to_obj (obj, pack);
    }
  for (i = 0; i < 2; i++)
    {
      obj = create_object (get_obj_index (OBJ_VNUM_SURVIVAL_V), 0);
      obj->level = 5;
      obj_to_obj (obj, pack);
    }
  obj = create_object (get_obj_index (OBJ_VNUM_SURVIVAL_W), 0);
  obj->level = 5;
  obj_to_obj (obj, pack);
  obj = create_object (get_obj_index (OBJ_VNUM_SURVIVAL_X), 0);
  obj->level = 5;
  obj_to_obj (obj, pack);

  obj_to_char (pack, victim);

  send_to_char ("Ok.\n\r", ch);
  act ("$p suddenly appears in your inventory.", ch, pack, victim, TO_VICT);
  return;
}


bool
can_pack (CHAR_DATA * ch)
{
  OBJ_DATA *object;
  bool found;

  if (ch->desc == NULL)
    return TRUE;

  if (ch->level > HERO)
    return TRUE;

  /*
   * search the list of objects.
   */
  found = TRUE;
  for (object = ch->carrying; object != NULL; object = object->next_content)
    {
      if (object->pIndexData->vnum == OBJ_VNUM_SURVIVAL_PACK)
	found = FALSE;
    }
  if (found)
    return TRUE;

  return FALSE;
}

void
do_dupe (CHAR_DATA * ch, char *argument)
{
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH], arg2[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int pos;
  bool found = FALSE;

  if (IS_NPC (ch))
    return;

  smash_tilde (argument);

  argument = one_argument (argument, arg);
  one_argument (argument, arg2);

  if (arg[0] == '\0')
    {
      send_to_char ("Dupe whom?\n\r", ch);
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

  if (arg2[0] == '\0')
    {
      if (victim->pcdata->dupes[0] == NULL)
	{
	  send_to_char ("They have no dupes set.\n\r", ch);
	  return;
	}
      send_to_char ("They currently have the following dupes:\n\r", ch);

      for (pos = 0; pos < MAX_DUPES; pos++)
	{
	  if (victim->pcdata->dupes[pos] == NULL)
	    break;

	  snprintf (buf, sizeof (buf), "    %s\n\r", victim->pcdata->dupes[pos]);
	  send_to_char (buf, ch);
	}
      return;
    }

  for (pos = 0; pos < MAX_DUPES; pos++)
    {
      if (victim->pcdata->dupes[pos] == NULL)
	break;

      if (!str_cmp (arg2, victim->pcdata->dupes[pos]))
	{
	  found = TRUE;
	}
    }

  if (found)
    {
      found = FALSE;
      for (pos = 0; pos < MAX_DUPES; pos++)
	{
	  if (victim->pcdata->dupes[pos] == NULL)
	    break;

	  if (found)
	    {
	      victim->pcdata->dupes[pos - 1] = victim->pcdata->dupes[pos];
	      victim->pcdata->dupes[pos] = NULL;
	      continue;
	    }

	  if (!strcmp (arg2, victim->pcdata->dupes[pos]))
	    {
	      send_to_char ("Dupe removed.\n\r", ch);
	      free_string (victim->pcdata->dupes[pos]);
	      victim->pcdata->dupes[pos] = NULL;
	      found = TRUE;
	    }
	}
      return;
    }

  for (pos = 0; pos < MAX_DUPES; pos++)
    {
      if (victim->pcdata->dupes[pos] == NULL)
	break;
    }

  if (pos >= MAX_DUPES)
    {
      send_to_char ("Sorry, they've reached the limit for dupes.\n\r", ch);
      return;
    }

  /* make a new dupe */
  victim->pcdata->dupes[pos] = str_dup (arg2);
  snprintf (buf, sizeof (buf), "%s now has the dupe %.4000s set.\n\r", victim->name, arg2);
  send_to_char (buf, ch);
}
