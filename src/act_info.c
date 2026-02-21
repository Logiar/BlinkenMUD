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
#include <ctype.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"

/* command procedures needed */
DECLARE_DO_FUN (do_exits);
DECLARE_DO_FUN (do_look);
DECLARE_DO_FUN (do_help);
DECLARE_DO_FUN (do_affects);
DECLARE_DO_FUN (do_play);
DECLARE_DO_FUN (do_inventory);
DECLARE_DO_FUN (do_recall);




char *const where_name[] = {
  "`G<`Cused as light`G>`x     ",
  "`G<`Cworn on finger`G>`x    ",
  "`G<`Cworn on finger`G>`x    ",
  "`G<`Cworn around neck`G>`x  ",
  "`G<`Cworn around neck`G>`x  ",
  "`G<`Cworn on torso`G>`x     ",
  "`G<`Cworn on head`G>`x      ",
  "`G<`Cworn on legs`G>`x      ",
  "`G<`Cworn on feet`G>`x      ",
  "`G<`Cworn on hands`G>`x     ",
  "`G<`Cworn on arms`G>`x      ",
  "`G<`Cworn as shield`G>`x    ",
  "`G<`Cworn about body`G>`x   ",
  "`G<`Cworn about waist`G>`x  ",
  "`G<`Cworn around wrist`G>`x ",
  "`G<`Cworn around wrist`G>`x ",
  "`G<`Cprimary wield`G>`x     ",
  "`G<`Cheld`G>`x              ",
  "`G<`Cfloating nearby`G>`x   ",
  "`G<`Csecondary wield`G>`x   ",
  "`G<`Cworn on face`G>`x      "
};

sh_int const where_order[] = {
  1, 2, 3, 4, 5,
  6, 20, 7, 8, 9,
  10, 11, 12, 13, 14,
  15, 16, 19, 17, 18,
  0
};


/* for do_count */
int max_on = 0;
bool is_pm = FALSE;


/*
 * Local functions.
 */
char *format_obj_to_char args ((OBJ_DATA * obj, CHAR_DATA * ch, bool fShort));
BUFFER *show_list_to_char args ((OBJ_DATA * list, CHAR_DATA * ch,
				 bool fShort, bool fShowNothing));
void show_char_to_char_0 args ((CHAR_DATA * victim, CHAR_DATA * ch));
void show_char_to_char_1 args ((CHAR_DATA * victim, CHAR_DATA * ch));
void show_char_to_char args ((CHAR_DATA * list, CHAR_DATA * ch));
bool check_blind args ((CHAR_DATA * ch));
void display_map (CHAR_DATA * ch);
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

char *
format_obj_to_char (OBJ_DATA * obj, CHAR_DATA * ch, bool fShort)
{
  static char buf[MAX_STRING_LENGTH];

  buf[0] = '\0';

  if ((fShort && (obj->short_descr == NULL || obj->short_descr[0] == '\0'))
      || (obj->description == NULL || obj->description[0] == '\0'))
    return buf;

  if (obj->pIndexData->vnum == ch->pcdata->bounty_obj)
    {
      if (ch->pcdata->hunt_time < current_time)
	reset_hunt (ch);
      else
	append_to_buf (buf, sizeof (buf), "`D(`rS`Rt`Wolen Au`Rr`ra`D)`x ");
    }
  else
   if (IS_OBJ_STAT (obj, ITEM_SHARP))
    append_to_buf (buf, sizeof (buf), "`C(`DS`dh`w`War`Dp`C)`x");

  if (!IS_SET (ch->comm, COMM_LONG))
    {
      append_to_buf (buf, sizeof (buf), "`x[`y.`R.`B.`M.`Y.`W.`G.`C.`x]");
      if (IS_OBJ_STAT (obj, ITEM_INVIS))
	buf[5] = 'V';
      if (IS_AFFECTED (ch, AFF_DETECT_EVIL) && IS_OBJ_STAT (obj, ITEM_EVIL))
	buf[8] = 'E';
      if (IS_AFFECTED (ch, AFF_DETECT_GOOD) && IS_OBJ_STAT (obj, ITEM_BLESS))
	buf[11] = 'B';
      if (IS_AFFECTED (ch, AFF_DETECT_MAGIC) && IS_OBJ_STAT (obj, ITEM_MAGIC))
	buf[14] = 'M';
      if (IS_OBJ_STAT (obj, ITEM_GLOW))
	buf[17] = 'G';
      if (IS_OBJ_STAT (obj, ITEM_HUM))
	buf[20] = 'H';
      if (IS_OBJ_STAT (obj, ITEM_QUEST))
	buf[23] = 'Q';
      if (IS_OBJ_STAT (obj, ITEM_SHARP))
	buf[26] = 'S';
      if (!strcmp (buf, "`x[`y.`R.`B.`M.`Y.`W.`G.`C.`x]"))
	buf[0] = '\0';
    }
  else
    {
      if (IS_OBJ_STAT (obj, ITEM_INVIS))
	append_to_buf (buf, sizeof (buf), "(`yInvis`x)");
      if (IS_OBJ_STAT (obj, ITEM_DARK))
	append_to_buf (buf, sizeof (buf), "(`DHidden`x)");
      if (IS_AFFECTED (ch, AFF_DETECT_EVIL) && IS_OBJ_STAT (obj, ITEM_EVIL))
	append_to_buf (buf, sizeof (buf), "(`RRed Aura`x)");
      if (IS_AFFECTED (ch, AFF_DETECT_GOOD) && IS_OBJ_STAT (obj, ITEM_BLESS))
	append_to_buf (buf, sizeof (buf), "(`BBlue Aura`x)");
      if (IS_AFFECTED (ch, AFF_DETECT_MAGIC) && IS_OBJ_STAT (obj, ITEM_MAGIC))
	append_to_buf (buf, sizeof (buf), "(`yMagical`x)");
      if (IS_OBJ_STAT (obj, ITEM_GLOW))
	append_to_buf (buf, sizeof (buf), "(`YGlowing`x)");
      if (IS_OBJ_STAT (obj, ITEM_HUM))
	append_to_buf (buf, sizeof (buf), "(`yHumming`x)");
      if (IS_OBJ_STAT (obj, ITEM_QUEST))
	append_to_buf (buf, sizeof (buf), "(`GQuest`x)");
      if (IS_OBJ_STAT (obj, ITEM_SHARP))
	append_to_buf (buf, sizeof (buf), "`C(`DS`dh`w`War`Dp`C)`x");
    }

  if (buf[0] != '\0')
    {
      append_to_buf (buf, sizeof (buf), " ");
    }

  if (fShort)
    {
      if (obj->short_descr != NULL)
	append_to_buf (buf, sizeof (buf), obj->short_descr);
    }
  else
    {
      if (obj->description != NULL)
	append_to_buf (buf, sizeof (buf), obj->description);
    }
  if (strlen (buf) <= 0)
    append_to_buf (buf, sizeof (buf), "This object has no description. Please inform the IMP.");

  return buf;
}



/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
BUFFER *
show_list_to_char (OBJ_DATA * list, CHAR_DATA * ch, bool fShort,
		   bool fShowNothing)
{
  char buf[MAX_STRING_LENGTH];
  BUFFER *output;
  char **prgpstrShow;
  int *prgnShow;
  char *pstrShow;
  OBJ_DATA *obj;
  int nShow;
  int iShow;
  int count;
  bool fCombine;

  /*
   * Alloc space for output lines.
   */
  output = new_buf ();
  count = 0;
  for (obj = list; obj != NULL; obj = obj->next_content)
    count++;
  prgpstrShow = alloc_mem (count * sizeof (char *));
  prgnShow = alloc_mem (count * sizeof (int));
  nShow = 0;

  /*
   * Format the list of objects.
   */
  for (obj = list; obj != NULL; obj = obj->next_content)
    {
      if (obj->wear_loc == WEAR_NONE && can_see_obj (ch, obj))
	{
	  pstrShow = format_obj_to_char (obj, ch, fShort);

	  fCombine = FALSE;

	  if (IS_NPC (ch) || IS_SET (ch->comm, COMM_COMBINE))
	    {
	      /*
	       * Look for duplicates, case sensitive.
	       * Matches tend to be near end so run loop backwords.
	       */
	      for (iShow = nShow - 1; iShow >= 0; iShow--)
		{
		  if (!strcmp (prgpstrShow[iShow], pstrShow))
		    {
		      prgnShow[iShow]++;
		      fCombine = TRUE;
		      break;
		    }
		}
	    }

	  /*
	   * Couldn't combine, or didn't want to.
	   */
	  if (!fCombine)
	    {
	      prgpstrShow[nShow] = str_dup (pstrShow);
	      prgnShow[nShow] = 1;
	      nShow++;
	    }
	}
    }

  /*
   * Output the formatted list.
   */
  for (iShow = 0; iShow < nShow; iShow++)
    {
      if (prgpstrShow[iShow][0] == '\0')
	{
	  free_string (prgpstrShow[iShow]);
	  continue;
	}

      if (IS_NPC (ch) || IS_SET (ch->comm, COMM_COMBINE))
	{
	  if (prgnShow[iShow] != 1)
	    {
	      snprintf (buf, sizeof (buf), "(%2d) ", prgnShow[iShow]);
	      add_buf (output, buf);
	    }
	  else
	    {
	      add_buf (output, "     ");
	    }
	}
      add_buf (output, prgpstrShow[iShow]);
      add_buf (output, "\n\r");
      free_string (prgpstrShow[iShow]);
    }

  if (fShowNothing && nShow == 0)
    {
      if (IS_NPC (ch) || IS_SET (ch->comm, COMM_COMBINE))
	send_to_char ("     ", ch);
      send_to_char ("Nothing.\n\r", ch);
    }
  /*
   * Clean up.
   */
  free_mem (prgpstrShow, count * sizeof (char *));
  free_mem (prgnShow, count * sizeof (int));

  return output;
}



void
show_char_to_char_0 (CHAR_DATA * victim, CHAR_DATA * ch)
{
  char buf[MAX_STRING_LENGTH], message[MAX_STRING_LENGTH];

  buf[0] = '\0';

  if (!IS_SET (ch->comm, COMM_LONG))
    {
      append_to_buf (buf, sizeof (buf), "`x[`y.`D.`c.`b.`w.`C.`r.`B.`R.`Y.`W.`G.`x]");
      if (IS_SHIELDED (victim, SHD_INVISIBLE))
	buf[5] = 'V';
      if (IS_AFFECTED (victim, AFF_HIDE))
	buf[8] = 'H';
      if (IS_AFFECTED (victim, AFF_CHARM))
	buf[11] = 'C';
      if (IS_AFFECTED (victim, AFF_PASS_DOOR))
	buf[14] = 'T';
      if (IS_AFFECTED (victim, AFF_FAERIE_FIRE))
	buf[17] = 'P';
      if (IS_SHIELDED (victim, SHD_ICE))
	buf[20] = 'I';
      if (IS_SHIELDED (victim, SHD_FIRE))
	buf[23] = 'F';
      if (IS_SHIELDED (victim, SHD_SHOCK))
	buf[26] = 'L';
      if (IS_EVIL (victim) && IS_AFFECTED (ch, AFF_DETECT_EVIL))
	buf[29] = 'E';
      if (IS_GOOD (victim) && IS_AFFECTED (ch, AFF_DETECT_GOOD))
	buf[32] = 'G';
      if (IS_SHIELDED (victim, SHD_SANCTUARY))
	buf[35] = 'S';
      if (victim->on_quest)
	buf[38] = 'Q';
      if (!strcmp (buf, "`x[`y.`D.`c.`b.`w.`C.`r.`B.`R.`Y.`W.`G.`x]"))
	buf[0] = '\0';
      if (IS_SET (victim->comm, COMM_AFK))
	append_to_buf (buf, sizeof (buf), "[`yAFK`x]");
      if (victim->invis_level >= LEVEL_HERO)
	append_to_buf (buf, sizeof (buf), "(`WWizi`x)");
    }
  else
    {
      if (IS_SET (victim->comm, COMM_AFK))
	append_to_buf (buf, sizeof (buf), "[`yAFK`x]");
      if (IS_SHIELDED (victim, SHD_INVISIBLE))
	append_to_buf (buf, sizeof (buf), "(`yInvis`x)");
      if (victim->invis_level >= LEVEL_HERO)
	append_to_buf (buf, sizeof (buf), "(`WWizi`x)");
      if (IS_AFFECTED (victim, AFF_HIDE))
	append_to_buf (buf, sizeof (buf), "(`DHide`x)");
      if (IS_AFFECTED (victim, AFF_CHARM))
	append_to_buf (buf, sizeof (buf), "(`cCharmed`x)");
      if (IS_AFFECTED (victim, AFF_PASS_DOOR))
	append_to_buf (buf, sizeof (buf), "(`bTranslucent`x)");
      if (IS_AFFECTED (victim, AFF_FAERIE_FIRE))
	append_to_buf (buf, sizeof (buf), "(`wPink Aura`x)");
      if (IS_SHIELDED (victim, SHD_ICE))
	append_to_buf (buf, sizeof (buf), "(`DGrey Aura`x)");
      if (IS_SHIELDED (victim, SHD_FIRE))
	append_to_buf (buf, sizeof (buf), "(`rOrange Aura`x)");
      if (IS_SHIELDED (victim, SHD_SHOCK))
	append_to_buf (buf, sizeof (buf), "(`BBlue Aura`x)");
      if (IS_EVIL (victim) && IS_AFFECTED (ch, AFF_DETECT_EVIL))
	append_to_buf (buf, sizeof (buf), "(`RRed Aura`x)");
      if (IS_GOOD (victim) && IS_AFFECTED (ch, AFF_DETECT_GOOD))
	append_to_buf (buf, sizeof (buf), "(`YGolden Aura`x)");
      if (IS_SHIELDED (victim, SHD_SANCTUARY))
	append_to_buf (buf, sizeof (buf), "(`WWhite Aura`x)");
      if (victim->on_quest)
	append_to_buf (buf, sizeof (buf), "(`GQuest`x)");
    }

  if (IS_NPC (victim) && ch->questmob > 0
      && victim->pIndexData->vnum == ch->questmob)
    append_to_buf (buf, sizeof (buf), "[TARGET] ");
  if (!IS_NPC (victim) && IS_SET (victim->act, PLR_TWIT))
    append_to_buf (buf, sizeof (buf), "(`rTWIT`x)");
  if (buf[0] != '\0')
    {
      append_to_buf (buf, sizeof (buf), " ");
    }
  if (victim->position == victim->start_pos && victim->long_descr[0] != '\0')
    {
      append_to_buf (buf, sizeof (buf), victim->long_descr);
      send_to_char (buf, ch);
      return;
    }

  append_to_buf (buf, sizeof (buf), PERS (victim, ch));
  if (!IS_NPC (victim) && !IS_SET (ch->comm, COMM_BRIEF)
      && victim->position == POS_STANDING && ch->on == NULL)
    append_to_buf (buf, sizeof (buf), victim->pcdata->title);

  switch (victim->position)
    {
    case POS_DEAD:
      append_to_buf (buf, sizeof (buf), " is DEAD!!");
      break;
    case POS_MORTAL:
      append_to_buf (buf, sizeof (buf), " is mortally wounded.");
      break;
    case POS_INCAP:
      append_to_buf (buf, sizeof (buf), " is incapacitated.");
      break;
    case POS_STUNNED:
      append_to_buf (buf, sizeof (buf), " is lying here stunned.");
      break;
    case POS_SLEEPING:
      if (victim->on != NULL)
	{
	  if (IS_SET (victim->on->value[2], SLEEP_AT))
	    {
	      snprintf (message, sizeof (message), " is sleeping at %s.",
		       victim->on->short_descr);
	      append_to_buf (buf, sizeof (buf), message);
	    }
	  else if (IS_SET (victim->on->value[2], SLEEP_ON))
	    {
	      snprintf (message, sizeof (message), " is sleeping on %s.",
		       victim->on->short_descr);
	      append_to_buf (buf, sizeof (buf), message);
	    }
	  else
	    {
	      snprintf (message, sizeof (message), " is sleeping in %s.",
		       victim->on->short_descr);
	      append_to_buf (buf, sizeof (buf), message);
	    }
	}
      else
	append_to_buf (buf, sizeof (buf), " is sleeping here.");
      break;
    case POS_RESTING:
      if (victim->on != NULL)
	{
	  if (IS_SET (victim->on->value[2], REST_AT))
	    {
	      snprintf (message, sizeof (message), " is resting at %s.",
		       victim->on->short_descr);
	      append_to_buf (buf, sizeof (buf), message);
	    }
	  else if (IS_SET (victim->on->value[2], REST_ON))
	    {
	      snprintf (message, sizeof (message), " is resting on %s.",
		       victim->on->short_descr);
	      append_to_buf (buf, sizeof (buf), message);
	    }
	  else
	    {
	      snprintf (message, sizeof (message), " is resting in %s.",
		       victim->on->short_descr);
	      append_to_buf (buf, sizeof (buf), message);
	    }
	}
      else
	append_to_buf (buf, sizeof (buf), " is resting here.");
      break;
    case POS_SITTING:
      if (victim->on != NULL)
	{
	  if (IS_SET (victim->on->value[2], SIT_AT))
	    {
	      snprintf (message, sizeof (message), " is sitting at %s.",
		       victim->on->short_descr);
	      append_to_buf (buf, sizeof (buf), message);
	    }
	  else if (IS_SET (victim->on->value[2], SIT_ON))
	    {
	      snprintf (message, sizeof (message), " is sitting on %s.",
		       victim->on->short_descr);
	      append_to_buf (buf, sizeof (buf), message);
	    }
	  else
	    {
	      snprintf (message, sizeof (message), " is sitting in %s.",
		       victim->on->short_descr);
	      append_to_buf (buf, sizeof (buf), message);
	    }
	}
      else
	append_to_buf (buf, sizeof (buf), " is sitting here.");
      break;
    case POS_STANDING:
      if (victim->on != NULL)
	{
	  if (IS_SET (victim->on->value[2], STAND_AT))
	    {
	      snprintf (message, sizeof (message), " is standing at %s.",
		       victim->on->short_descr);
	      append_to_buf (buf, sizeof (buf), message);
	    }
	  else if (IS_SET (victim->on->value[2], STAND_ON))
	    {
	      snprintf (message, sizeof (message), " is standing on %s.",
		       victim->on->short_descr);
	      append_to_buf (buf, sizeof (buf), message);
	    }
	  else
	    {
	      snprintf (message, sizeof (message), " is standing in %s.",
		       victim->on->short_descr);
	      append_to_buf (buf, sizeof (buf), message);
	    }
	}
      else
	append_to_buf (buf, sizeof (buf), " is here.");
      break;
    case POS_FIGHTING:
      append_to_buf (buf, sizeof (buf), " is here, fighting ");
      if (victim->fighting == NULL)
	append_to_buf (buf, sizeof (buf), "thin air??");
      else if (victim->fighting == ch)
	append_to_buf (buf, sizeof (buf), "YOU!");
      else if (victim->in_room == victim->fighting->in_room)
	{
	  append_to_buf (buf, sizeof (buf), PERS (victim->fighting, ch));
	  append_to_buf (buf, sizeof (buf), ".");
	}
      else
	append_to_buf (buf, sizeof (buf), "someone who left??");
      break;
    }

  append_to_buf (buf, sizeof (buf), "\n\r");
  buf[0] = UPPER (buf[0]);
  send_to_char (buf, ch);
  return;
}



void
show_char_to_char_1 (CHAR_DATA * victim, CHAR_DATA * ch)
{
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj;
  BUFFER *output;
  BUFFER *outlist;
  int iWear;
  int oWear;
  int percent;
  bool found;

  if (can_see (victim, ch) && get_trust (victim) >= ch->ghost_level)
    {
      if (ch == victim)
	act ("$n looks at $mself.", ch, NULL, NULL, TO_ROOM);
      else
	{
	  act ("$n looks at you.", ch, NULL, victim, TO_VICT);
	  act ("$n looks at $N.", ch, NULL, victim, TO_NOTVICT);
	}
    }

  output = new_buf ();
  if (victim->description[0] != '\0')
    {
      snprintf (buf, sizeof (buf), "`C%s`x", victim->description);
    }
  else
    {
      snprintf (buf, sizeof (buf), "`CYou see nothing special about %s`x\n\r", victim->name);
    }

  add_buf (output, buf);

  if (victim->max_hit > 0)
    percent = (100 * victim->hit) / victim->max_hit;
  else
    percent = -1;

  buf[0] = '\0';
  snprintf (buf, sizeof (buf), "%s", PERS (victim, ch));

  if (percent >= 100)
    append_to_buf (buf, sizeof (buf), " `fis in excellent condition.`x\n\r");
  else if (percent >= 90)
    append_to_buf (buf, sizeof (buf), " `fhas a few scratches.`x\n\r");
  else if (percent >= 75)
    append_to_buf (buf, sizeof (buf), " `fhas some small wounds and bruises.`x\n\r");
  else if (percent >= 50)
    append_to_buf (buf, sizeof (buf), " `fhas quite a few wounds.`x\n\r");
  else if (percent >= 30)
    append_to_buf (buf, sizeof (buf), " `fhas some big nasty wounds and scratches.`x\n\r");
  else if (percent >= 15)
    append_to_buf (buf, sizeof (buf), " `flooks pretty hurt.`x\n\r");
  else if (percent >= 0)
    append_to_buf (buf, sizeof (buf), " `fis in awful condition.`x\n\r");
  else
    append_to_buf (buf, sizeof (buf), " `fis bleeding to death.`x\n\r");

  buf[0] = UPPER (buf[0]);
  add_buf (output, buf);

  if (IS_SHIELDED (victim, SHD_ICE))
    {
      snprintf (buf, sizeof (buf), "%s is surrounded by an `Cicy`x shield.\n\r",
	       PERS (victim, ch));
      buf[0] = UPPER (buf[0]);
      add_buf (output, buf);
    }
  if (IS_SHIELDED (victim, SHD_FIRE))
    {
      snprintf (buf, sizeof (buf), "%s is surrounded by a `Rfiery`x shield.\n\r",
	       PERS (victim, ch));
      buf[0] = UPPER (buf[0]);
      add_buf (output, buf);
    }
  if (IS_SHIELDED (victim, SHD_SHOCK))
    {
      snprintf (buf, sizeof (buf), "%s is surrounded by a `Bcrackling`x shield.\n\r",
	       PERS (victim, ch));
      buf[0] = UPPER (buf[0]);
      add_buf (output, buf);
    }

  found = FALSE;
  for (oWear = 0; oWear < MAX_WEAR; oWear++)
    {
      iWear = where_order[oWear];
      if ((obj = get_eq_char (victim, iWear)) != NULL
	  && can_see_obj (ch, obj))
	{
	  if (!found)
	    {
	      snprintf (buf, sizeof (buf), "\n\r");
	      add_buf (output, buf);
	      snprintf (buf, sizeof (buf), "`G%s is using:`x\n\r", victim->name);
	      add_buf (output, buf);
	      found = TRUE;
	    }
	  snprintf (buf, sizeof (buf), "%s%s\n\r", where_name[iWear],
		   format_obj_to_char (obj, ch, TRUE));
	  add_buf (output, buf);
	}
    }

  if (victim != ch
      && !IS_NPC (ch)
      && number_percent () < get_skill (ch, gsn_peek)
      && IS_SET (ch->act, PLR_AUTOPEEK))
    {
      snprintf (buf, sizeof (buf), "\n\r`GYou peek at the inventory:`x\n\r");
      add_buf (output, buf);
      check_improve (ch, gsn_peek, TRUE, 4);
      outlist = show_list_to_char (victim->carrying, ch, TRUE, TRUE);
      add_buf (output, buf_string (outlist));
      free_buf (outlist);
    }
  page_to_char (buf_string (output), ch);
  free_buf (output);
  return;
}



void
show_char_to_char (CHAR_DATA * list, CHAR_DATA * ch)
{
  CHAR_DATA *rch;

  for (rch = list; rch != NULL; rch = rch->next_in_room)
    {
      if (rch == ch)
	continue;

      if (get_trust (ch) < rch->invis_level)
	continue;

      if (get_trust (ch) < rch->ghost_level)
	continue;

      if (can_see (ch, rch))
	{
	  show_char_to_char_0 (rch, ch);
	}
      else if (room_is_dark (ch->in_room) && IS_AFFECTED (rch, AFF_INFRARED))
	{
	  send_to_char ("You see `Rglowing red`x eyes watching YOU!\n\r", ch);
	}
    }

  return;
}

void
do_peek (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  BUFFER *output;
  BUFFER *outlist;
  CHAR_DATA *victim;

  one_argument (argument, arg);

  if (IS_NPC (ch))
    return;

  if (arg[0] == '\0')
    {
      send_to_char ("Peek at who?\n\r", ch);
      return;
    }

  if ((victim = get_char_room (ch, arg)) == NULL)
    {
      send_to_char ("They aren't here.\n\r", ch);
      return;
    }

  if (victim == ch)
    {
      do_inventory (ch, "");
      return;
    }

  if (can_see (victim, ch) && get_trust (victim) >= ch->ghost_level)
    {
      act ("$n peers intently at you.", ch, NULL, victim, TO_VICT);
      act ("$n peers intently at $N.", ch, NULL, victim, TO_NOTVICT);
    }

  output = new_buf ();

  if (number_percent () < get_skill (ch, gsn_peek))
    {
      snprintf (buf, sizeof (buf), "\n\r`GYou peek at the inventory:`x\n\r");
      add_buf (output, buf);
      check_improve (ch, gsn_peek, TRUE, 4);
      outlist = show_list_to_char (victim->carrying, ch, TRUE, TRUE);
      add_buf (output, buf_string (outlist));
      free_buf (outlist);
    }
  else
    {
      snprintf (buf, sizeof (buf), "`RYou fail to see anything.`x\n\r");
      add_buf (output, buf);
      check_improve (ch, gsn_peek, FALSE, 2);
    }
  page_to_char (buf_string (output), ch);
  free_buf (output);
  return;
}

bool
check_blind (CHAR_DATA * ch)
{

  if (!IS_NPC (ch) && IS_SET (ch->act, PLR_HOLYLIGHT))
    return TRUE;

  if (IS_AFFECTED (ch, AFF_BLIND))
    {
      send_to_char ("You can't see a thing!\n\r", ch);
      return FALSE;
    }

  return TRUE;
}

/* changes your scroll */
void
do_scroll (CHAR_DATA * ch, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[100];
  int lines;

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      if (ch->lines == 0)
	send_to_char ("You do not page long messages.\n\r", ch);
      else
	{
	  snprintf (buf, sizeof (buf), "You currently display %d lines per page.\n\r",
		   ch->lines + 2);
	  send_to_char (buf, ch);
	}
      return;
    }

  if (!is_number (arg))
    {
      send_to_char ("You must provide a number.\n\r", ch);
      return;
    }

  lines = atoi (arg);

  if (lines == 0)
    {
      send_to_char ("Paging disabled.\n\r", ch);
      ch->lines = 0;
      return;
    }

  if (lines < 10 || lines > 100)
    {
      send_to_char ("You must provide a reasonable number.\n\r", ch);
      return;
    }

  snprintf (buf, sizeof (buf), "Scroll set to %d lines.\n\r", lines);
  send_to_char (buf, ch);
  ch->lines = lines - 2;
}

/* RT does socials */
void
do_socials (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  int iSocial;
  int col;

  col = 0;

  for (iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++)
    {
      snprintf (buf, sizeof (buf), "%-12.12s", social_table[iSocial].name);
      send_to_char (buf, ch);
      if (++col % 6 == 0)
	send_to_char ("\n\r", ch);
    }

  if (col % 6 != 0)
    send_to_char ("\n\r", ch);
  return;
}




/* RT Commands to replace news, motd, imotd, etc from ROM */

void
do_motd (CHAR_DATA * ch, char *argument)
{
  do_help (ch, "motd");
}

void
do_imotd (CHAR_DATA * ch, char *argument)
{
  do_help (ch, "imotd");
}

void
do_rules (CHAR_DATA * ch, char *argument)
{
  do_help (ch, "rules");
}

void
do_story (CHAR_DATA * ch, char *argument)
{
  do_help (ch, "story");
}

/* RT this following section holds all the auto commands from ROM, as well as
   replacements for config */

void
do_autolist (CHAR_DATA * ch, char *argument)
{
  /* lists most player flags */
  if (IS_NPC (ch))
    return;

  send_to_char ("   action     status\n\r", ch);
  send_to_char ("---------------------\n\r", ch);

  send_to_char ("autoassist     ", ch);
  if (IS_SET (ch->act, PLR_AUTOASSIST))
    send_to_char ("ON\n\r", ch);
  else
    send_to_char ("OFF\n\r", ch);

  send_to_char ("autoexit       ", ch);
  if (IS_SET (ch->act, PLR_AUTOEXIT))
    send_to_char ("ON\n\r", ch);
  else
    send_to_char ("OFF\n\r", ch);

  send_to_char ("autogold       ", ch);
  if (IS_SET (ch->act, PLR_AUTOGOLD))
    send_to_char ("ON\n\r", ch);
  else
    send_to_char ("OFF\n\r", ch);

  send_to_char ("autoloot       ", ch);
  if (IS_SET (ch->act, PLR_AUTOLOOT))
    send_to_char ("ON\n\r", ch);
  else
    send_to_char ("OFF\n\r", ch);

  send_to_char ("autosac        ", ch);
  if (IS_SET (ch->act, PLR_AUTOSAC))
    send_to_char ("ON\n\r", ch);
  else
    send_to_char ("OFF\n\r", ch);

  send_to_char ("autosplit      ", ch);
  if (IS_SET (ch->act, PLR_AUTOSPLIT))
    send_to_char ("ON\n\r", ch);
  else
    send_to_char ("OFF\n\r", ch);

  send_to_char ("autopeek       ", ch);
  if (IS_SET (ch->act, PLR_AUTOPEEK))
    send_to_char ("ON\n\r", ch);
  else
    send_to_char ("OFF\n\r", ch);

  send_to_char ("autostore      ", ch);
  if (IS_SET (ch->comm, COMM_STORE))
    send_to_char ("ON\n\r", ch);
  else
    send_to_char ("OFF\n\r", ch);

  send_to_char ("compact mode   ", ch);
  if (IS_SET (ch->comm, COMM_COMPACT))
    send_to_char ("ON\n\r", ch);
  else
    send_to_char ("OFF\n\r", ch);

  send_to_char ("prompt         ", ch);
  if (IS_SET (ch->comm, COMM_PROMPT))
    send_to_char ("ON\n\r", ch);
  else
    send_to_char ("OFF\n\r", ch);

  send_to_char ("combine items  ", ch);
  if (IS_SET (ch->comm, COMM_COMBINE))
    send_to_char ("ON\n\r", ch);
  else
    send_to_char ("OFF\n\r", ch);

  send_to_char ("long flags     ", ch);
  if (IS_SET (ch->comm, COMM_LONG))
    send_to_char ("ON\n\r", ch);
  else
    send_to_char ("OFF\n\r", ch);

  if (!IS_SET (ch->act, PLR_CANLOOT))
    send_to_char ("Your corpse is safe from thieves.\n\r", ch);
  else
    send_to_char ("Your corpse may be looted.\n\r", ch);

  if (IS_SET (ch->act, PLR_NOSUMMON))
    send_to_char ("You cannot be summoned.\n\r", ch);
  else
    send_to_char ("You can be summoned.\n\r", ch);

  if (IS_SET (ch->act, PLR_NOFOLLOW))
    send_to_char ("You do not welcome followers.\n\r", ch);
  else
    send_to_char ("You accept followers.\n\r", ch);
}



/* 
 * Lore written by Rahl (Daniel Anderson).
 * Can use on object anywhere in the world, but you also get less info than 
 * identify 
 */
void
do_lore (CHAR_DATA * ch, char *argument)
{
  OBJ_DATA *obj;
  char arg[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];

  one_argument (argument, arg);

  obj = get_obj_world (ch, arg);

  /* 
   * <blush> oops. Dunno how I forgot this the first time around
   * -Rahl
   */

  if (obj == NULL)
    {
      snprintf (buf, sizeof (buf), "You've never heard of a %.4000s.\n\r", arg);
      send_to_char (buf, ch);
      return;
    }

  if (get_skill (ch, gsn_lore) == 0)
    {
      send_to_char ("You don't know anything about it.\n\r", ch);
      return;
    }

  if (arg[0] == '\0')
    {
      send_to_char ("What do you want information on?\n\r", ch);
      return;
    }

  if (number_percent () < get_skill (ch, gsn_lore))
    {
      snprintf (buf, sizeof (buf),
		"'%s' is type %s, extra flags %s.\n\rLevel %d.\n\r",
		obj->name, item_type_name (obj),
		extra_bit_name (obj->extra_flags), obj->level);
      send_to_char (buf, ch);

      check_improve (ch, gsn_lore, TRUE, 1);
    }
  else
    {
      send_to_char ("You can't remember a thing about it.\n\r", ch);
      check_improve (ch, gsn_lore, FALSE, 1);
    }

  return;
}

