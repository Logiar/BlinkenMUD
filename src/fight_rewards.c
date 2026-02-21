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
#include <stdio.h>
#include <string.h>
#include "merc.h"
#include "tables.h"


void death_cry (CHAR_DATA * ch);
void make_corpse (CHAR_DATA * ch, CHAR_DATA * killer);
int xp_compute (CHAR_DATA * gch, CHAR_DATA * victim, int total_levels);


void
raw_kill (CHAR_DATA * victim, CHAR_DATA * killer)
{
  int i;

  death_cry (victim);
  stop_fighting (victim, TRUE);
  make_corpse (victim, killer);

  if (IS_NPC (victim))
    {
      victim->pIndexData->killed++;
      kill_table[URANGE (0, victim->level, MAX_LEVEL - 1)].killed++;
      extract_char (victim, TRUE);
      return;
    }

  extract_char (victim, FALSE);
  while (victim->affected)
    affect_remove (victim, victim->affected);
  victim->affected_by = race_table[victim->race].aff;
  victim->shielded_by = race_table[victim->race].shd;
  for (i = 0; i < 4; i++)
    victim->armor[i] = 100;
  victim->position = POS_RESTING;
  victim->hit = UMAX (1, victim->hit);
  victim->mana = UMAX (1, victim->mana);
  victim->move = UMAX (1, victim->move);
/*  save_char_obj( victim ); we're stable enough to not need this :) */
  return;
}



void
group_gain (CHAR_DATA * ch, CHAR_DATA * victim)
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *gch;
  //    CHAR_DATA *lch;
  int xp;
  int members = 0;
  int group_levels = 0;

  /*
   * Monsters don't get kill xp's or alignment changes.
   * P-killing doesn't help either.
   * Dying of mortal wounds or poison doesn't give xp to anyone!
   */
  if (victim == ch)
    return;

  for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
    {
      if (is_same_group (gch, ch))
	{
	  members++;
	  group_levels += IS_NPC (gch) ? gch->level / 2 : gch->level;
	}
    }

  if (members == 0)
    {
      bug ("Group_gain: members.", members);
      members = 1;
      group_levels = ch->level;
    }

  //lch = (ch->leader != NULL) ? ch->leader : ch;

  for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
    {
      OBJ_DATA *obj;
      OBJ_DATA *obj_next;

      if (!is_same_group (gch, ch) || IS_NPC (gch))
	continue;

/*	Taken out, add it back if you want it
	if ( gch->level - lch->level >= 5 )
	{
	    send_to_char( "You are too high for this group.\n\r", gch );
	    continue;
	}

	if ( gch->level - lch->level <= -5 )
	{
	    send_to_char( "You are too low for this group.\n\r", gch );
	    continue;
	}
*/


      xp = xp_compute (gch, victim, group_levels);
      snprintf (buf, sizeof (buf), "`BYou receive `W%d`B experience points.`x\n\r", xp);
      send_to_char (buf, gch);
      gain_exp (gch, xp);

      for (obj = ch->carrying; obj != NULL; obj = obj_next)
	{
	  obj_next = obj->next_content;
	  if (obj->wear_loc == WEAR_NONE)
	    continue;

	  if ((IS_OBJ_STAT (obj, ITEM_ANTI_EVIL) && IS_EVIL (ch))
	      || (IS_OBJ_STAT (obj, ITEM_ANTI_GOOD) && IS_GOOD (ch))
	      || (IS_OBJ_STAT (obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL (ch)))
	    {
	      act ("`cYou are `Wzapped`c by $p.`x", ch, obj, NULL, TO_CHAR);
	      act ("$n is `Wzapped`x by $p.", ch, obj, NULL, TO_ROOM);
	      obj_from_char (obj);
	      obj_to_room (obj, ch->in_room);
	    }
	}
      /* QUEST COMPLETED */
      if (IS_NPC (victim)
	  && IS_SET (ch->act, PLR_QUESTOR)
	  && (ch->questmob == victim->pIndexData->vnum))
	{
	  act ("`2QUEST`x almost `4COMPLETED`x", ch, NULL, NULL, TO_CHAR);
	  act
	    ("`7Return to the questmaster at once to `4complete`x your quest",
	     ch, NULL, NULL, TO_CHAR);
	  act ("a `2Quest`x is being completed", ch, NULL, NULL, TO_ROOM);
	  ch->questmob = -1;
	  ch->countdown += number_range (0, 5);	//Give some time to get back
	}

    }

  return;
}



/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int
xp_compute (CHAR_DATA * gch, CHAR_DATA * victim, int total_levels)
{
  int xp, base_exp;
  int align, level_range;
  int change;
  int time_per_level;

  level_range = victim->level - gch->level;
  if (!IS_NPC (gch))
    {
      if (gch->class >= MAX_CLASS / 2)
	level_range -= 5;
    }

  if (!IS_NPC (gch) && !IS_NPC (victim))
    {
      xp = 1;
      return xp;
    }
  /* compute the base exp */
  switch (level_range)
    {
    default:
      base_exp = 0;
      break;
    case -9:
      base_exp = 1;
      break;
    case -8:
      base_exp = 2;
      break;
    case -7:
      base_exp = 5;
      break;
    case -6:
      base_exp = 9;
      break;
    case -5:
      base_exp = 11;
      break;
    case -4:
      base_exp = 22;
      break;
    case -3:
      base_exp = 33;
      break;
    case -2:
      base_exp = 50;
      break;
    case -1:
      base_exp = 66;
      break;
    case 0:
      base_exp = 83;
      break;
    case 1:
      base_exp = 99;
      break;
    case 2:
      base_exp = 121;
      break;
    case 3:
      base_exp = 143;
      break;
    case 4:
      base_exp = 165;
      break;
    }

  if (level_range > 4)
    base_exp = 160 + 20 * (level_range - 4);

  /* do alignment computations */

  align = victim->alignment - gch->alignment;

  if (IS_SET (victim->act, ACT_NOALIGN))
    {
      /* no change */
    }

  else if (align > 500)		/* monster is more good than slayer */
    {
      change = (align - 500) * base_exp / 500 * gch->level / total_levels;
      change = UMAX (1, change);
      gch->alignment = UMAX (-1000, gch->alignment - change);
      if (gch->pet != NULL)
	gch->pet->alignment = gch->alignment;
    }

  else if (align < -500)	/* monster is more evil than slayer */
    {
      change =
	(-1 * align - 500) * base_exp / 500 * gch->level / total_levels;
      change = UMAX (1, change);
      gch->alignment = UMIN (1000, gch->alignment + change);
      if (gch->pet != NULL)
	gch->pet->alignment = gch->alignment;
    }

  else				/* improve this someday */
    {
      change = gch->alignment * base_exp / 500 * gch->level / total_levels;
      gch->alignment -= change;
      if (gch->pet != NULL)
	gch->pet->alignment = gch->alignment;
    }

  /* calculate exp multiplier */
  if (IS_SET (victim->act, ACT_NOALIGN))
    xp = base_exp;

  else if (gch->alignment > 500)	/* for goodie two shoes */
    {
      if (victim->alignment < -750)
	xp = (base_exp * 4) / 3;

      else if (victim->alignment < -500)
	xp = (base_exp * 5) / 4;

      else if (victim->alignment < -250)
	xp = (base_exp * 3) / 4;

      else if (victim->alignment > 750)
	xp = base_exp / 4;

      else if (victim->alignment > 500)
	xp = base_exp / 2;

      else
	xp = base_exp;
    }

  else if (gch->alignment < -500)	/* for baddies */
    {
      if (victim->alignment > 750)
	xp = (base_exp * 5) / 4;

      else if (victim->alignment > 500)
	xp = (base_exp * 11) / 10;

      else if (victim->alignment < -750)
	xp = base_exp / 2;

      else if (victim->alignment < -500)
	xp = (base_exp * 3) / 4;

      else if (victim->alignment < -250)
	xp = (base_exp * 9) / 10;

      else
	xp = base_exp;
    }

  else if (gch->alignment > 200)	/* a little good */
    {

      if (victim->alignment < -500)
	xp = (base_exp * 6) / 5;

      else if (victim->alignment > 750)
	xp = base_exp / 2;

      else if (victim->alignment > 0)
	xp = (base_exp * 3) / 4;

      else
	xp = base_exp;
    }

  else if (gch->alignment < -200)	/* a little bad */
    {
      if (victim->alignment > 500)
	xp = (base_exp * 6) / 5;

      else if (victim->alignment < -750)
	xp = base_exp / 2;

      else if (victim->alignment < 0)
	xp = (base_exp * 3) / 4;

      else
	xp = base_exp;
    }

  else				/* neutral */
    {

      if (victim->alignment > 500 || victim->alignment < -500)
	xp = (base_exp * 4) / 3;

      else if (victim->alignment < 200 && victim->alignment > -200)
	xp = base_exp / 2;

      else
	xp = base_exp;
    }

  /* more exp at the low levels */
  if (gch->level < 11)
    xp = 15 * xp / (gch->level + 4);

  /* less at high */
  if (gch->level > 60)
    xp = 15 * xp / (gch->level - 25);

  /* reduce for playing time */


  {
    /* compute quarter-hours per level */
    time_per_level = 4 *
      (gch->played + (int) (current_time - gch->logon)) / 3600 / gch->level;

    time_per_level = URANGE (2, time_per_level, 12);
    if (gch->level < 25)	/* make it a curve */
      time_per_level = UMAX (time_per_level, (25 - gch->level));
/*
 *	xp = xp * time_per_level / 12;
 */
  }
  xp = xp * .75;

  /* randomize the rewards */
  xp = number_range (xp * 3 / 4, xp * 5 / 4);

  /* adjust for grouping */
  xp = xp * gch->level / (UMAX (1, total_levels - 1));

  return xp;
}


void
dam_message (CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, bool immune)
{
  char buf1[256], buf2[256], buf3[256];
  const char *vs;
  const char *vp;
  const char *attack;
  char punct;

  if (ch == NULL || victim == NULL)
    return;

  if (dam == 0)
    {
      vs = "miss";
      vp = "misses";
    }
  else if (dam <= 4)
    {
      vs = "scratch";
      vp = "scratches";
    }
  else if (dam <= 8)
    {
      vs = "graze";
      vp = "grazes";
    }
  else if (dam <= 12)
    {
      vs = "hit";
      vp = "hits";
    }
  else if (dam <= 16)
    {
      vs = "injure";
      vp = "injures";
    }
  else if (dam <= 20)
    {
      vs = "wound";
      vp = "wounds";
    }
  else if (dam <= 24)
    {
      vs = "maul";
      vp = "mauls";
    }
  else if (dam <= 28)
    {
      vs = "decimate";
      vp = "decimates";
    }
  else if (dam <= 32)
    {
      vs = "devastate";
      vp = "devastates";
    }
  else if (dam <= 36)
    {
      vs = "maim";
      vp = "maims";
    }
  else if (dam <= 40)
    {
      vs = "MUTILATE";
      vp = "MUTILATES";
    }
  else if (dam <= 44)
    {
      vs = "DISEMBOWEL";
      vp = "DISEMBOWELS";
    }
  else if (dam <= 48)
    {
      vs = "DISMEMBER";
      vp = "DISMEMBERS";
    }
  else if (dam <= 52)
    {
      vs = "MASSACRE";
      vp = "MASSACRES";
    }
  else if (dam <= 56)
    {
      vs = "MANGLE";
      vp = "MANGLES";
    }
  else if (dam <= 60)
    {
      vs = "*** DEMOLISH ***";
      vp = "*** DEMOLISHES ***";
    }
  else if (dam <= 75)
    {
      vs = "*** DEVASTATE ***";
      vp = "*** DEVASTATES ***";
    }
  else if (dam <= 100)
    {
      vs = "=== OBLITERATE ===";
      vp = "=== OBLITERATES ===";
    }
  else if (dam <= 125)
    {
      vs = ">>> ANNIHILATE <<<";
      vp = ">>> ANNIHILATES <<<";
    }
  else if (dam <= 150)
    {
      vs = "<<< ERADICATE >>>";
      vp = "<<< ERADICATES >>>";
    }
  else
    {
      vs = "do UNSPEAKABLE things to";
      vp = "does UNSPEAKABLE things to";
    }

  punct = (dam <= 24) ? '.' : '!';

  if (dt == TYPE_HIT)
    {
      if (ch == victim)
	{
	  snprintf (buf1, sizeof (buf1), "`k$n %s $melf%c`x", vp, punct);
	  snprintf (buf2, sizeof (buf2), "`hYou %s yourself%c`x", vs, punct);
	}
      else
	{
	  snprintf (buf1, sizeof (buf1), "`k$n %s $N%c`x", vp, punct);
	  snprintf (buf2, sizeof (buf2), "`hYou %s $N%c`x", vs, punct);
	  snprintf (buf3, sizeof (buf3), "`i$n %s you%c`x", vp, punct);
	}
    }
  else
    {
      if (dt >= 0 && dt < MAX_SKILL)
	attack = skill_table[dt].noun_damage;
      else if (dt >= TYPE_HIT && dt < TYPE_HIT + MAX_DAMAGE_MESSAGE)
	attack = attack_table[dt - TYPE_HIT].noun;
      else
	{
	  bug ("Dam_message: bad dt %d.", dt);
	  dt = TYPE_HIT;
	  attack = attack_table[0].name;
	}

      if (immune)
	{
	  if (ch == victim)
	    {
	      snprintf (buf1, sizeof (buf1), "`k$n is unaffected by $s own %s.`x", attack);
	      snprintf (buf2, sizeof (buf2), "`hLuckily, you are immune to that.`x");
	    }
	  else
	    {
	      snprintf (buf1, sizeof (buf1), "`k$N is unaffected by $n's %s!`x", attack);
	      snprintf (buf2, sizeof (buf2), "`h$N is unaffected by your %s!`x", attack);
	      snprintf (buf3, sizeof (buf3), "`i$n's %s is powerless against you.`x", attack);
	    }
	}
      else
	{
	  if (ch == victim)
	    {
	      snprintf (buf1, sizeof (buf1), "`k$n's %s %s $m%c`x", attack, vp, punct);
	      snprintf (buf2, sizeof (buf2), "`hYour %s %s you%c`x", attack, vp, punct);
	    }
	  else
	    {
	      snprintf (buf1, sizeof (buf1), "`k$n's %s %s $N%c`x", attack, vp, punct);
	      snprintf (buf2, sizeof (buf2), "`hYour %s %s $N%c`x", attack, vp, punct);
	      snprintf (buf3, sizeof (buf3), "`i$n's %s %s you%c`x", attack, vp, punct);
	    }
	}
    }

  if (ch == victim)
    {
      act (buf1, ch, NULL, NULL, TO_ROOM);
      act (buf2, ch, NULL, NULL, TO_CHAR);
    }
  else
    {
      act (buf1, ch, NULL, victim, TO_NOTVICT);
      act (buf2, ch, NULL, victim, TO_CHAR);
      act (buf3, ch, NULL, victim, TO_VICT);
    }

  return;
}



/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void
disarm (CHAR_DATA * ch, CHAR_DATA * victim)
{
  OBJ_DATA *obj;

  if ((obj = get_eq_char (victim, WEAR_WIELD)) == NULL)
    return;

  if (IS_OBJ_STAT (obj, ITEM_NOREMOVE))
    {
      act ("`j$S weapon won't budge!`x", ch, NULL, victim, TO_CHAR);
      act ("`j$n tries to disarm you, but your weapon won't budge!`x",
	   ch, NULL, victim, TO_VICT);
      act ("`k$n tries to disarm $N, but fails.`x", ch, NULL, victim,
	   TO_NOTVICT);
      return;
    }

  act ("`j$n DISARMS you and sends your weapon flying!`x",
       ch, NULL, victim, TO_VICT);
  act ("`jYou disarm $N!`x", ch, NULL, victim, TO_CHAR);
  act ("`k$n disarms $N!`x", ch, NULL, victim, TO_NOTVICT);

  obj_from_char (obj);
  if (IS_OBJ_STAT (obj, ITEM_NODROP) || IS_OBJ_STAT (obj, ITEM_INVENTORY))
    obj_to_char (obj, victim);
  else
    {
      obj_to_room (obj, victim->in_room);
      if (IS_NPC (victim) && victim->wait == 0 && can_see_obj (victim, obj))
	get_obj (victim, obj, NULL);
    }

  return;
}

