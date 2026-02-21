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
do_set (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];

  argument = one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Syntax:\n\r", ch);
      send_to_char ("  set mob   <name> <field> <value>\n\r", ch);
      send_to_char ("  set obj   <name> <field> <value>\n\r", ch);
      send_to_char ("  set room  <room> <field> <value>\n\r", ch);
      send_to_char ("  set skill <name> <spell or skill> <value>\n\r", ch);
      send_to_char ("  set char  <name> <field> <value>\n\r", ch);
      return;
    }

  if (!str_prefix (arg, "mobile") || !str_prefix (arg, "character"))
    {
      do_mset (ch, argument);
      return;
    }

  if (!str_prefix (arg, "skill") || !str_prefix (arg, "spell"))
    {
      do_sset (ch, argument);
      return;
    }

  if (!str_prefix (arg, "object"))
    {
      do_oset (ch, argument);
      return;
    }

  if (!str_prefix (arg, "room"))
    {
      do_rset (ch, argument);
      return;
    }
  /* echo syntax */
  do_set (ch, "");
}


void
do_sset (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int value;
  int sn;
  bool fAll;

  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  argument = one_argument (argument, arg3);

  if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
    {
      send_to_char ("Syntax:\n\r", ch);
      send_to_char ("  set skill <name> <spell or skill> <value>\n\r", ch);
      send_to_char ("  set skill <name> all <value>\n\r", ch);
      send_to_char ("   (use the name of the skill, not the number)\n\r", ch);
      return;
    }

  if ((victim = get_char_world (ch, arg1)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (IS_NPC (victim))
    {
      send_to_char ("Not on NPC's.\n\r", ch);
      return;
    }

  fAll = !str_cmp (arg2, "all");
  sn = 0;
  if (!fAll && (sn = skill_lookup (arg2)) < 0)
    {
      send_to_char ("No such skill or spell.\n\r", ch);
      return;
    }

  /*
   * Snarf the value.
   */
  if (!is_number (arg3))
    {
      send_to_char ("Value must be numeric.\n\r", ch);
      return;
    }

  value = atoi (arg3);
  if (value < 0 || value > 100)
    {
      send_to_char ("Value range is 0 to 100.\n\r", ch);
      return;
    }

  if (fAll)
    {
      for (sn = 0; sn < MAX_SKILL; sn++)
	{
	  if (skill_table[sn].name != NULL)
	    victim->pcdata->learned[sn] = value;
	}
    }
  else
    {
      victim->pcdata->learned[sn] = value;
    }

  return;
}



void
do_mset (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  char buf[100];
  CHAR_DATA *victim;
  int value;

  smash_tilde (argument);
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  snprintf (arg3, sizeof (arg3), "%s", argument);

  if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
    {
      send_to_char ("Syntax:\n\r", ch);
      send_to_char ("  set char <name> <field> <value>\n\r", ch);
      send_to_char ("  Field being one of:\n\r", ch);
      send_to_char ("    str int wis dex con sex class level\n\r", ch);
      send_to_char ("    race group platinum gold silver hp\n\r", ch);
      send_to_char ("    mana move prac align train thirst\n\r", ch);
      send_to_char ("    hunger drunk full quest\n\r", ch);
      send_to_char ("    security\n\r", ch);
      return;
    }

  if ((victim = get_char_world (ch, arg1)) == NULL
      || (victim->level > ch->level && victim->level == MAX_LEVEL))
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  /* clear zones for mobs */
  victim->zone = NULL;

  /*
   * Snarf the value (which need not be numeric).
   */
  value = is_number (arg3) ? atoi (arg3) : -1;

  /*
   * Set something.
   */
  if (!str_cmp (arg2, "str"))
    {
      if (value < 3 || value > get_max_train (victim, STAT_STR))
	{
	  snprintf (buf, sizeof (buf),
		   "Strength range is 3 to %d\n\r.",
		   get_max_train (victim, STAT_STR));
	  send_to_char (buf, ch);
	  return;
	}

      victim->perm_stat[STAT_STR] = value;
      return;
    }
  if (!str_cmp (arg2, "security"))	/* OLC */
    {
      if (IS_NPC (victim))
	{
	  send_to_char ("Not on NPC's.\n\r", ch);
	  return;
	}

      if (value > ch->pcdata->security || value < 0)
	{
	  if (ch->pcdata->security != 0)
	    {
	      snprintf (buf, sizeof (buf), "Valid security is 0-%d.\n\r",
		       ch->pcdata->security);
	      send_to_char (buf, ch);
	    }
	  else
	    {
	      send_to_char ("Valid security is 0 only.\n\r", ch);
	    }
	  return;
	}
      victim->pcdata->security = value;
      return;
    }

  if (!str_cmp (arg2, "int"))
    {
      if (value < 3 || value > get_max_train (victim, STAT_INT))
	{
	  snprintf (buf, sizeof (buf),
		   "Intelligence range is 3 to %d.\n\r",
		   get_max_train (victim, STAT_INT));
	  send_to_char (buf, ch);
	  return;
	}

      victim->perm_stat[STAT_INT] = value;
      return;
    }

  if (!str_cmp (arg2, "wis"))
    {
      if (value < 3 || value > get_max_train (victim, STAT_WIS))
	{
	  snprintf (buf, sizeof (buf),
		   "Wisdom range is 3 to %d.\n\r", get_max_train (victim,
								  STAT_WIS));
	  send_to_char (buf, ch);
	  return;
	}

      victim->perm_stat[STAT_WIS] = value;
      return;
    }

  if (!str_cmp (arg2, "dex"))
    {
      if (value < 3 || value > get_max_train (victim, STAT_DEX))
	{
	  snprintf (buf, sizeof (buf),
		   "Dexterity ranges is 3 to %d.\n\r",
		   get_max_train (victim, STAT_DEX));
	  send_to_char (buf, ch);
	  return;
	}

      victim->perm_stat[STAT_DEX] = value;
      return;
    }

  if (!str_cmp (arg2, "con"))
    {
      if (value < 3 || value > get_max_train (victim, STAT_CON))
	{
	  snprintf (buf, sizeof (buf),
		   "Constitution range is 3 to %d.\n\r",
		   get_max_train (victim, STAT_CON));
	  send_to_char (buf, ch);
	  return;
	}

      victim->perm_stat[STAT_CON] = value;
      return;
    }

  if (!str_prefix (arg2, "sex"))
    {
      if (value < 0 || value > 2)
	{
	  send_to_char ("Sex range is 0 to 2.\n\r", ch);
	  return;
	}
      victim->sex = value;
      if (!IS_NPC (victim))
	victim->pcdata->true_sex = value;
      return;
    }

  if (!str_prefix (arg2, "class"))
    {
      int class;

      if (IS_NPC (victim))
	{
	  send_to_char ("Mobiles have no class.\n\r", ch);
	  return;
	}

      class = class_lookup (arg3);
      if (class == -1)
	{
	  char buf[MAX_STRING_LENGTH];

	  snprintf (buf, sizeof (buf), "%s", "Possible classes are: ");
	  for (class = 0; class < MAX_CLASS; class++)
	    {
	      if (class > 0)
		append_to_buf (buf, sizeof (buf), " ");
	      append_to_buf (buf, sizeof (buf), class_table[class].name);
	    }
	  append_to_buf (buf, sizeof (buf), ".\n\r");

	  send_to_char (buf, ch);
	  return;
	}

      victim->class = class;
      return;
    }

  if (!str_prefix (arg2, "level"))
    {
      if (!IS_NPC (victim))
	{
	  send_to_char ("Not on PC's.\n\r", ch);
	  return;
	}

      if (value < 0 || value > ch->level)
	{
	  snprintf (buf, sizeof (buf), "Level range is 0 to %d.\n\r", ch->level);
	  send_to_char (buf, ch);
	  return;
	}
      victim->level = value;
      return;
    }

  if (!str_prefix (arg2, "platinum"))
    {
      victim->platinum = value;
      return;
    }

  if (!str_prefix (arg2, "gold"))
    {
      victim->gold = value;
      return;
    }

  if (!str_prefix (arg2, "silver"))
    {
      victim->silver = value;
      return;
    }

  if (!str_prefix (arg2, "hp"))
    {
      if (value < -10 || value > 30000)
	{
	  send_to_char ("Hp range is -10 to 30,000 hit points.\n\r", ch);
	  return;
	}
      victim->max_hit = value;
      if (!IS_NPC (victim))
	victim->pcdata->perm_hit = value;
      return;
    }

  if (!str_prefix (arg2, "mana"))
    {
      if (value < 0 || value > 30000)
	{
	  send_to_char ("Mana range is 0 to 30,000 mana points.\n\r", ch);
	  return;
	}
      victim->max_mana = value;
      if (!IS_NPC (victim))
	victim->pcdata->perm_mana = value;
      return;
    }

  if (!str_prefix (arg2, "move"))
    {
      if (value < 0 || value > 30000)
	{
	  send_to_char ("Move range is 0 to 30,000 move points.\n\r", ch);
	  return;
	}
      victim->max_move = value;
      if (!IS_NPC (victim))
	victim->pcdata->perm_move = value;
      return;
    }

  if (!str_prefix (arg2, "practice"))
    {
      if (value < 0 || value > 250)
	{
	  send_to_char ("Practice range is 0 to 250 sessions.\n\r", ch);
	  return;
	}
      victim->practice = value;
      return;
    }

  if (!str_prefix (arg2, "train"))
    {
      if (value < 0 || value > 50)
	{
	  send_to_char ("Training session range is 0 to 50 sessions.\n\r",
			ch);
	  return;
	}
      victim->train = value;
      return;
    }

  if (!str_prefix (arg2, "align"))
    {
      if (value < -1000 || value > 1000)
	{
	  send_to_char ("Alignment range is -1000 to 1000.\n\r", ch);
	  return;
	}
      victim->alignment = value;
      if (victim->pet != NULL)
	victim->pet->alignment = victim->alignment;
      return;
    }

  if (!str_prefix (arg2, "thirst"))
    {
      if (IS_NPC (victim))
	{
	  send_to_char ("Not on NPC's.\n\r", ch);
	  return;
	}

      if (value < -1 || value > 100)
	{
	  send_to_char ("Thirst range is -1 to 100.\n\r", ch);
	  return;
	}

      victim->pcdata->condition[COND_THIRST] = value;
      return;
    }

  if (!str_prefix (arg2, "drunk"))
    {
      if (IS_NPC (victim))
	{
	  send_to_char ("Not on NPC's.\n\r", ch);
	  return;
	}

      if (value < -1 || value > 100)
	{
	  send_to_char ("Drunk range is -1 to 100.\n\r", ch);
	  return;
	}

      victim->pcdata->condition[COND_DRUNK] = value;
      return;
    }

  if (!str_prefix (arg2, "full"))
    {
      if (IS_NPC (victim))
	{
	  send_to_char ("Not on NPC's.\n\r", ch);
	  return;
	}

      if (value < -1 || value > 100)
	{
	  send_to_char ("Full range is -1 to 100.\n\r", ch);
	  return;
	}

      victim->pcdata->condition[COND_FULL] = value;
      return;
    }

  if (!str_prefix (arg2, "hunger"))
    {
      if (IS_NPC (victim))
	{
	  send_to_char ("Not on NPC's.\n\r", ch);
	  return;
	}

      if (value < -1 || value > 100)
	{
	  send_to_char ("Full range is -1 to 100.\n\r", ch);
	  return;
	}

      victim->pcdata->condition[COND_HUNGER] = value;
      return;
    }

  if (!str_prefix (arg2, "quest"))
    {
      if (IS_NPC (victim))
	{
	  send_to_char ("NPC's don't need quest points.\n\r", ch);
	  return;
	}

      victim->qps = value;
      return;
    }

  if (!str_prefix (arg2, "race"))
    {
      int race;

      race = race_lookup (arg3);

      if (race == 0)
	{
	  send_to_char ("That is not a valid race.\n\r", ch);
	  return;
	}

      if (!IS_NPC (victim) && !race_table[race].pc_race)
	{
	  send_to_char ("That is not a valid player race.\n\r", ch);
	  return;
	}

      victim->race = race;
      return;
    }

  if (!str_prefix (arg2, "group"))
    {
      if (!IS_NPC (victim))
	{
	  send_to_char ("Only on NPCs.\n\r", ch);
	  return;
	}
      victim->group = value;
      return;
    }


  /*
   * Generate usage message.
   */
  do_mset (ch, "");
  return;
}

void
do_string (CHAR_DATA * ch, char *argument)
{
  char type[MAX_INPUT_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  int cnt, plc;

  smash_tilde (argument);
  argument = one_argument (argument, type);
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  snprintf (arg3, sizeof (arg3), "%s", argument);

  if (type[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0')
    {
      send_to_char ("Syntax:\n\r", ch);
      send_to_char ("  string char <name> <field> <string>\n\r", ch);
      send_to_char ("    fields: name short long title who spec\n\r", ch);
      send_to_char ("  string obj  <name> <field> <string>\n\r", ch);
      send_to_char ("    fields: name short long extended\n\r", ch);
      return;
    }

  if (!str_prefix (type, "character") || !str_prefix (type, "mobile"))
    {
      if ((victim = get_char_world (ch, arg1)) == NULL)
	{
	  send_to_char ("They aren't here.\n\r", ch);
	  return;
	}

      /* clear zone for mobs */
      victim->zone = NULL;

      /* string something */

      if ((victim->level >= ch->level) && (ch != victim))
	{
	  send_to_char ("That will not be done.\n\r", ch);
	  return;
	}

      if (!str_prefix (arg2, "who"))
	{
	  if (IS_NPC (victim))
	    {
	      send_to_char ("Not on NPC's.\n\r", ch);
	      return;
	    }
	  if ((ch->level < CREATOR)
	      && (victim->level < HERO) && (victim->class < MAX_CLASS / 2))
	    {
	      send_to_char ("Not on 1st tier mortals.\n\r", ch);
	      return;
	    }
	  buf[0] = '\0';
	  buf2[0] = '\0';
	  victim->pcdata->who_descr = str_dup ("");
	  if (arg3[0] == '\0')
	    {
	      return;
	    }
	  cnt = 0;
	  for (plc = 0; plc < (int) strlen (arg3); plc++)
	    {
	      if (!IS_COLOUR_MARKER (arg3[plc]))
		{
		  if (buf[0] == '\0')
		    {
		      snprintf (buf2, sizeof (buf2), "%c", arg3[plc]);
		    }
		  else
		    {
		      snprintf (buf2, sizeof (buf2), "%.*s%c", (int) sizeof (buf2) - 2, buf, arg3[plc]);
		    }
		  snprintf (buf, sizeof (buf), "%s", buf2);
		  cnt++;
		}
	      else if (arg3[plc + 1] == arg3[plc])
		{
		  if (buf[0] == '\0')
		    {
		      snprintf (buf2, sizeof (buf2), "%c%c", arg3[plc], arg3[plc]);
		    }
		  else
		    {
		      snprintf (buf2, sizeof (buf2), "%.*s%c%c", (int) sizeof (buf2) - 3, buf, arg3[plc], arg3[plc]);
		    }
		  snprintf (buf, sizeof (buf), "%s", buf2);
		  cnt++;
		  plc++;
		}
	      else
		{
		  if (buf[0] == '\0')
		    {
		      snprintf (buf2, sizeof (buf2), "%c%c", arg3[plc], arg3[plc + 1]);
		    }
		  else
		    {
		      snprintf (buf2, sizeof (buf2), "%.*s%c%c", (int) sizeof (buf2) - 3, buf, arg3[plc], arg3[plc + 1]);
		    }
		  snprintf (buf, sizeof (buf), "%s", buf2);
		  plc++;
		}
	      if (cnt >= 10)
		{
		  plc = strlen (arg3);
		}
	    }
	  snprintf (buf2, sizeof (buf2), "%.252s`0", buf);
	  snprintf (buf, sizeof (buf), "%s", buf2);
	  while (cnt < 10)
	    {
	      snprintf (buf2, sizeof (buf2), "%.254s ", buf);
	      snprintf (buf, sizeof (buf), "%s", buf2);
	      cnt++;
	    }
	  victim->pcdata->who_descr = str_dup (buf);
	  buf[0] = '\0';
	  buf2[0] = '\0';
	  return;
	}

      if (arg3[0] == '\0')
	{
	  do_string (ch, "");
	  return;
	}

      if (!str_prefix (arg2, "name"))
	{
	  if (!IS_NPC (victim))
	    {
	      send_to_char ("Not on PC's.\n\r", ch);
	      return;
	    }
	  free_string (victim->name);
	  victim->name = str_dup (arg3);
	  return;
	}

      if (!str_prefix (arg2, "short"))
	{
	  free_string (victim->short_descr);
	  victim->short_descr = str_dup (arg3);
	  return;
	}

      if (!str_prefix (arg2, "long"))
	{
	  char long_descr[MAX_STRING_LENGTH];

	  free_string (victim->long_descr);
	  long_descr[0] = '\0';
	  append_to_buf (long_descr, sizeof (long_descr), arg3);
	  append_to_buf (long_descr, sizeof (long_descr), "\n\r");
	  victim->long_descr = str_dup (long_descr);
	  return;
	}

      if (!str_prefix (arg2, "title"))
	{
	  if (IS_NPC (victim))
	    {
	      send_to_char ("Not on NPC's.\n\r", ch);
	      return;
	    }

	  set_title (victim, arg3);
	  return;
	}

      if (!str_prefix (arg2, "spec"))
	{
	  if (!IS_NPC (victim))
	    {
	      send_to_char ("Not on PC's.\n\r", ch);
	      return;
	    }

	  if ((victim->spec_fun = spec_lookup (arg3)) == 0)
	    {
	      send_to_char ("No such spec fun.\n\r", ch);
	      return;
	    }

	  return;
	}
    }

  if (arg3[0] == '\0')
    {
      do_string (ch, "");
      return;
    }
  if (!str_prefix (type, "object"))
    {
      /* string an obj */

      if ((obj = get_obj_world (ch, arg1)) == NULL)
	{
	  send_to_char ("Nothing like that in heaven or earth.\n\r", ch);
	  return;
	}
      if (obj->item_type == ITEM_EXIT)
	{
	  send_to_char ("You cannot modify exit objects.\n\r", ch);
	  return;
	}
      if (!str_prefix (arg2, "name"))
	{
	  free_string (obj->name);
	  obj->name = str_dup (arg3);
	  return;
	}

      if (!str_prefix (arg2, "short"))
	{
	  free_string (obj->short_descr);
	  obj->short_descr = str_dup (arg3);
	  return;
	}

      if (!str_prefix (arg2, "long"))
	{
	  free_string (obj->description);
	  obj->description = str_dup (arg3);
	  return;
	}

      if (!str_prefix (arg2, "ed") || !str_prefix (arg2, "extended"))
	{
	  EXTRA_DESCR_DATA *ed;

	  argument = one_argument (argument, arg3);
	  if (argument == NULL)
	    {
	      send_to_char ("Syntax: oset <object> ed <keyword> <string>\n\r",
			    ch);
	      return;
	    }

	  {
	    char edesc[MAX_STRING_LENGTH];

	    edesc[0] = '\0';
	    append_to_buf (edesc, sizeof (edesc), argument);
	    append_to_buf (edesc, sizeof (edesc), "\n\r");

	    ed = new_extra_descr ();

	    ed->keyword = str_dup (arg3);
	    ed->description = str_dup (edesc);
	    ed->next = obj->extra_descr;
	    obj->extra_descr = ed;
	  }
	  return;
	}
    }


  /* echo bad use message */
  do_string (ch, "");
}



void
do_oset (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int value;
  int clan;
  int class;

  smash_tilde (argument);
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  snprintf (arg3, sizeof (arg3), "%s", argument);

  if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
    {
      send_to_char ("Syntax:\n\r", ch);
      send_to_char ("  set obj <object> <field> <value>\n\r", ch);
      send_to_char ("  Field being one of:\n\r", ch);
      send_to_char ("    value0 value1 value2 value3 value4 (v1-v4)\n\r", ch);
      send_to_char ("    level weight cost timer clan guild\n\r", ch);
      return;
    }

  if ((obj = get_obj_world (ch, arg1)) == NULL)
    {
      send_to_char ("Nothing like that in heaven or earth.\n\r", ch);
      return;
    }
  if (obj->item_type == ITEM_EXIT)
    {
      send_to_char ("You cannot modify exit objects.\n\r", ch);
      return;
    }

  if (!str_prefix (arg2, "clan"))
    {
      if (!str_prefix (arg3, "none"))
	{
	  obj->clan = 0;
	  return;
	}
      if ((clan = clan_lookup (arg3)) == 0)
	{
	  send_to_char ("No such clan exists.\n\r", ch);
	  return;
	}
      obj->clan = clan;
      return;
    }
  if (!str_prefix (arg2, "guild"))
    {
      if (!str_prefix (arg3, "none"))
	{
	  obj->class = 0;
	  return;
	}
      if ((class = class_lookup (arg3)) == 0)
	{
	  send_to_char ("No such guild exists.\n\r", ch);
	  return;
	}
      obj->class = class;
      return;
    }

  /*
   * Snarf the value (which need not be numeric).
   */
  value = atoi (arg3);

  /*
   * Set something.
   */
  if (!str_cmp (arg2, "value0") || !str_cmp (arg2, "v0"))
    {
      if (obj->item_type == ITEM_WEAPON)
	{
	  obj->value[0] = UMIN (MAX_WEAPON, value);
	  obj->value[0] = UMAX (0, obj->value[0]);
	  return;
	}
      if ((obj->item_type == ITEM_WAND)
	  || (obj->item_type == ITEM_STAFF)
	  || (obj->item_type == ITEM_POTION)
	  || (obj->item_type == ITEM_SCROLL) || (obj->item_type == ITEM_PILL))
	{
	  obj->value[0] = UMIN (MAX_LEVEL, value);
	  obj->value[0] = UMAX (0, obj->value[0]);
	  return;
	}
      obj->value[0] = value;
      return;
    }

  if (!str_cmp (arg2, "value1") || !str_cmp (arg2, "v1"))
    {
      obj->value[1] = value;
      return;
    }

  if (!str_cmp (arg2, "value2") || !str_cmp (arg2, "v2"))
    {
      if ((obj->item_type == ITEM_FOUNTAIN)
	  || (obj->item_type == ITEM_DRINK_CON))
	{
	  obj->value[2] = UMIN (MAX_LIQUID, value);
	  obj->value[2] = UMAX (0, obj->value[2]);
	  return;
	}
      obj->value[2] = value;
      return;
    }

  if (!str_cmp (arg2, "value3") || !str_cmp (arg2, "v3"))
    {
      if (obj->item_type == ITEM_WEAPON)
	{
	  obj->value[3] = UMIN (MAX_DAMAGE_MESSAGE, value);
	  obj->value[3] = UMAX (0, obj->value[3]);
	  return;
	}
      obj->value[3] = value;
      return;
    }

  if (!str_cmp (arg2, "value4") || !str_cmp (arg2, "v4"))
    {
      obj->value[4] = value;
      return;
    }

  if (!str_prefix (arg2, "extra"))
    {
      send_to_char ("Use the flag command instead.\n\r", ch);
      return;
    }

  if (!str_prefix (arg2, "wear"))
    {
      send_to_char ("Use the flag command instead.\n\r", ch);
      return;
    }

  if (!str_prefix (arg2, "level"))
    {
      if ((get_trust (ch) < CREATOR && (obj->pIndexData->level - 5) > value)
	  && !IS_SET (ch->act, PLR_KEY))
	{
	  send_to_char ("You may not lower an item more than 5 levels!\n\r",
			ch);
	  return;
	}
      if ((get_trust (ch) == CREATOR && (obj->pIndexData->level - 10) > value)
	  && !IS_SET (ch->act, PLR_KEY))
	{
	  send_to_char ("You may not lower an item more than 10 levels!\n\r",
			ch);
	  return;
	}
      obj->level = UMIN (MAX_LEVEL, value);
      obj->level = UMIN (0, obj->level);
      return;
    }

  if (!str_prefix (arg2, "weight"))
    {
      obj->weight = value;
      return;
    }

  if (!str_prefix (arg2, "cost"))
    {
      obj->cost = value;
      return;
    }

  if (!str_prefix (arg2, "timer"))
    {
      obj->timer = value;
      return;
    }

  /*
   * Generate usage message.
   */
  do_oset (ch, "");
  return;
}



void
do_rset (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;
  int value;

  smash_tilde (argument);
  argument = one_argument (argument, arg1);
  argument = one_argument (argument, arg2);
  snprintf (arg3, sizeof (arg3), "%s", argument);

  if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
    {
      send_to_char ("Syntax:\n\r", ch);
      send_to_char ("  set room <location> <field> <value>\n\r", ch);
      send_to_char ("  Field being one of:\n\r", ch);
      send_to_char ("    sector\n\r", ch);
      return;
    }

  if ((location = find_location (ch, arg1)) == NULL)
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

  /*
   * Snarf the value.
   */
  if (!is_number (arg3))
    {
      send_to_char ("Value must be numeric.\n\r", ch);
      return;
    }
  value = atoi (arg3);

  /*
   * Set something.
   */
  if (!str_prefix (arg2, "flags"))
    {
      send_to_char ("Use the flag command instead.\n\r", ch);
      return;
    }

  if (!str_prefix (arg2, "sector"))
    {
      location->sector_type = value;
      return;
    }

  /*
   * Generate usage message.
   */
  do_rset (ch, "");
  return;
}



