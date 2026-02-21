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
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"

/* command procedures needed */
DECLARE_DO_FUN (do_look);

/* imported functions */
bool check_dispel args ((int dis_level, CHAR_DATA * victim, int sn));
bool remove_obj args ((CHAR_DATA * ch, int iWear, bool fReplace));

extern char *target_name;


/* RT ROM-style gate */

void
spell_gate (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim;
  bool gate_pet;

  if ((victim = get_char_world (ch, target_name)) == NULL || victim == ch || victim->in_room == NULL || !can_see_room (ch, victim->in_room) || IS_SET (victim->in_room->room_flags, ROOM_SAFE) || IS_SET (victim->in_room->room_flags, ROOM_PRIVATE) || IS_SET (victim->in_room->room_flags, ROOM_SOLITARY) || IS_SET (victim->in_room->room_flags, ROOM_NO_RECALL) || IS_SET (ch->in_room->room_flags, ROOM_NO_RECALL) || victim->level >= level + 3 || (is_clan (ch) && (is_clan (victim) && ((!is_same_clan (ch, victim) && (clan_table[ch->clan].pkill) && (clan_table[victim->clan].pkill)) || clan_table[victim->clan].independent))) || (!IS_NPC (victim) && victim->level >= LEVEL_HERO)	/* NOT trust */
      || (IS_NPC (victim) && IS_SET (victim->imm_flags, IMM_SUMMON))
      || (IS_NPC (victim) && saves_spell (level, victim, DAM_OTHER)))
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }
  if (ch->pet != NULL && ch->in_room == ch->pet->in_room)
    gate_pet = TRUE;
  else
    gate_pet = FALSE;

  act ("$n steps through a gate and vanishes.", ch, NULL, NULL, TO_ROOM);
  send_to_char ("You step through a gate and vanish.\n\r", ch);
  char_from_room (ch);
  char_to_room (ch, victim->in_room);

  act ("$n has arrived through a gate.", ch, NULL, NULL, TO_ROOM);
  do_look (ch, "auto");

  if (gate_pet)
    {
      act ("$n steps through a gate and vanishes.", ch->pet, NULL, NULL,
	   TO_ROOM);
      send_to_char ("You step through a gate and vanish.\n\r", ch->pet);
      char_from_room (ch->pet);
      char_to_room (ch->pet, victim->in_room);
      act ("$n has arrived through a gate.", ch->pet, NULL, NULL, TO_ROOM);
      do_look (ch->pet, "auto");
    }
}



void
spell_giant_strength (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected (victim, sn))
    {
      if (victim == ch)
	send_to_char ("You are already as strong as you can get!\n\r", ch);
      else
	act ("$N can't get any stronger.", ch, NULL, victim, TO_CHAR);
      return;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level;
  af.location = APPLY_STR;
  af.modifier = 1 + (level >= 18) + (level >= 25) + (level >= 32);
  af.bitvector = 0;
  affect_to_char (victim, &af);
  send_to_char ("Your muscles surge with heightened power!\n\r", victim);
  act ("$n's muscles surge with heightened power.", victim, NULL, NULL,
       TO_ROOM);
  return;
}



void
spell_harm (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }
  dam = UMAX (20, victim->hit - dice (1, 4));
  if (saves_spell (level, victim, DAM_HARM))
    dam = UMIN (50, dam / 2);
  dam = UMIN (100, dam);
  damage_old (ch, victim, dam, sn, DAM_HARM, TRUE);
  return;
}

/* RT haste spell */

void
spell_haste (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected (victim, sn) || IS_AFFECTED (victim, AFF_HASTE)
      || IS_SET (victim->off_flags, OFF_FAST))
    {
      if (victim == ch)
	send_to_char ("You can't move any faster!\n\r", ch);
      else
	act ("$N is already moving as fast as $E can.",
	     ch, NULL, victim, TO_CHAR);
      return;
    }

  if (IS_AFFECTED (victim, AFF_SLOW))
    {
      if (!check_dispel (level, victim, skill_lookup ("slow")))
	{
	  if (victim != ch)
	    send_to_char ("Spell failed.\n\r", ch);
	  send_to_char ("You feel momentarily faster.\n\r", victim);
	  return;
	}
      act ("$n is moving less slowly.", victim, NULL, NULL, TO_ROOM);
      return;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  if (victim == ch)
    af.duration = level / 2;
  else
    af.duration = level / 4;
  af.location = APPLY_DEX;
  af.modifier = 1 + (level >= 18) + (level >= 25) + (level >= 32);
  af.bitvector = AFF_HASTE;
  affect_to_char (victim, &af);
  send_to_char ("You feel yourself moving more quickly.\n\r", victim);
  act ("$n is moving more quickly.", victim, NULL, NULL, TO_ROOM);
  if (ch != victim)
    send_to_char ("Ok.\n\r", ch);
  return;
}



void
spell_heal (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  victim->hit = UMIN (victim->hit + 100, victim->max_hit);
  update_pos (victim);
  send_to_char ("A warm feeling fills your body.\n\r", victim);
  if (ch != victim)
    send_to_char ("Ok.\n\r", ch);
  return;
}

void
spell_heat_metal (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  OBJ_DATA *obj_lose, *obj_next;
  int dam = 0;
  bool fail = TRUE;

  if (!saves_spell (level + 2, victim, DAM_FIRE)
      && !IS_SET (victim->imm_flags, IMM_FIRE))
    {
      for (obj_lose = victim->carrying; obj_lose != NULL; obj_lose = obj_next)
	{
	  obj_next = obj_lose->next_content;
	  if (number_range (1, 2 * level) > obj_lose->level
	      && !saves_spell (level, victim, DAM_FIRE)
	      && !IS_OBJ_STAT (obj_lose, ITEM_NONMETAL)
	      && !IS_OBJ_STAT (obj_lose, ITEM_BURN_PROOF))
	    {
	      switch (obj_lose->item_type)
		{
		case ITEM_ARMOR:
		  if (obj_lose->wear_loc != -1)	/* remove the item */
		    {
		      if (can_drop_obj (victim, obj_lose)
			  && (obj_lose->weight / 10) <
			  number_range (1,
					2 * get_curr_stat (victim, STAT_DEX))
			  && remove_obj (victim, obj_lose->wear_loc, TRUE))
			{
			  act ("$n yelps and throws $p to the ground!",
			       victim, obj_lose, NULL, TO_ROOM);
			  act ("You remove and drop $p before it burns you.",
			       victim, obj_lose, NULL, TO_CHAR);
			  dam += (number_range (1, obj_lose->level) / 3);
			  obj_from_char (obj_lose);
			  obj_to_room (obj_lose, victim->in_room);
			  fail = FALSE;
			}
		      else	/* stuck on the body! ouch! */
			{
			  act ("Your skin is seared by $p!",
			       victim, obj_lose, NULL, TO_CHAR);
			  dam += (number_range (1, obj_lose->level));
			  fail = FALSE;
			}

		    }
		  else		/* drop it if we can */
		    {
		      if (can_drop_obj (victim, obj_lose))
			{
			  act ("$n yelps and throws $p to the ground!",
			       victim, obj_lose, NULL, TO_ROOM);
			  act ("You and drop $p before it burns you.",
			       victim, obj_lose, NULL, TO_CHAR);
			  dam += (number_range (1, obj_lose->level) / 6);
			  obj_from_char (obj_lose);
			  obj_to_room (obj_lose, victim->in_room);
			  fail = FALSE;
			}
		      else	/* cannot drop */
			{
			  act ("Your skin is seared by $p!",
			       victim, obj_lose, NULL, TO_CHAR);
			  dam += (number_range (1, obj_lose->level) / 2);
			  fail = FALSE;
			}
		    }
		  break;
		case ITEM_WEAPON:
		  if (obj_lose->wear_loc != -1)	/* try to drop it */
		    {
		      if (IS_WEAPON_STAT (obj_lose, WEAPON_FLAMING))
			continue;

		      if (can_drop_obj (victim, obj_lose)
			  && remove_obj (victim, obj_lose->wear_loc, TRUE))
			{
			  act
			    ("$n is burned by $p, and throws it to the ground.",
			     victim, obj_lose, NULL, TO_ROOM);
			  send_to_char
			    ("You throw your red-hot weapon to the ground!\n\r",
			     victim);
			  dam += 1;
			  obj_from_char (obj_lose);
			  obj_to_room (obj_lose, victim->in_room);
			  fail = FALSE;
			}
		      else	/* YOWCH! */
			{
			  send_to_char ("Your weapon sears your flesh!\n\r",
					victim);
			  dam += number_range (1, obj_lose->level);
			  fail = FALSE;
			}
		    }
		  else		/* drop it if we can */
		    {
		      if (can_drop_obj (victim, obj_lose))
			{
			  act ("$n throws a burning hot $p to the ground!",
			       victim, obj_lose, NULL, TO_ROOM);
			  act ("You and drop $p before it burns you.",
			       victim, obj_lose, NULL, TO_CHAR);
			  dam += (number_range (1, obj_lose->level) / 6);
			  obj_from_char (obj_lose);
			  obj_to_room (obj_lose, victim->in_room);
			  fail = FALSE;
			}
		      else	/* cannot drop */
			{
			  act ("Your skin is seared by $p!",
			       victim, obj_lose, NULL, TO_CHAR);
			  dam += (number_range (1, obj_lose->level) / 2);
			  fail = FALSE;
			}
		    }
		  break;
		}
	    }
	}
    }
  if (fail)
    {
      send_to_char ("Your spell had no effect.\n\r", ch);
      send_to_char ("You feel momentarily warmer.\n\r", victim);
    }
  else				/* damage! */
    {
      if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
	{
	  ch->attacker = TRUE;
	  victim->attacker = FALSE;
	}
      if (saves_spell (level, victim, DAM_FIRE))
	dam = 2 * dam / 3;
      damage_old (ch, victim, dam, sn, DAM_FIRE, TRUE);
    }
}

/* RT really nasty high-level attack spell */
void
spell_holy_word (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;
  int bless_num, curse_num, frenzy_num;

  bless_num = skill_lookup ("bless");
  curse_num = skill_lookup ("curse");
  frenzy_num = skill_lookup ("frenzy");

  act ("$n utters a word of divine power!", ch, NULL, NULL, TO_ROOM);
  send_to_char ("You utter a word of divine power.\n\r", ch);

  for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next_in_room;

      if ((IS_GOOD (ch) && IS_GOOD (vch)) ||
	  (IS_EVIL (ch) && IS_EVIL (vch)) ||
	  (IS_NEUTRAL (ch) && IS_NEUTRAL (vch)))
	{
	  send_to_char ("You feel full more powerful.\n\r", vch);
	  spell_frenzy (frenzy_num, level, ch, (void *) vch, TARGET_CHAR);
	  spell_bless (bless_num, level, ch, (void *) vch, TARGET_CHAR);
	}

      else if ((IS_GOOD (ch) && IS_EVIL (vch)) ||
	       (IS_EVIL (ch) && IS_GOOD (vch)))
	{
	  if (!is_safe_spell (ch, vch, TRUE))
	    {
	      if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (vch)))
		{
		  ch->attacker = TRUE;
		  vch->attacker = FALSE;
		}
	      spell_curse (curse_num, level, ch, (void *) vch, TARGET_CHAR);
	      send_to_char ("You are struck down!\n\r", vch);
	      dam = dice (level, 6);
	      damage_old (ch, vch, dam, sn, DAM_ENERGY, TRUE);
	    }
	}

      else if (IS_NEUTRAL (ch))
	{
	  if (!is_safe_spell (ch, vch, TRUE))
	    {
	      if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (vch)))
		{
		  ch->attacker = TRUE;
		  vch->attacker = FALSE;
		}
	      spell_curse (curse_num, level / 2, ch, (void *) vch,
			   TARGET_CHAR);
	      send_to_char ("You are struck down!\n\r", vch);
	      dam = dice (level, 4);
	      damage_old (ch, vch, dam, sn, DAM_ENERGY, TRUE);
	    }
	}
    }

  send_to_char ("You feel drained.\n\r", ch);
  ch->move = 0;
  ch->hit /= 2;
}

