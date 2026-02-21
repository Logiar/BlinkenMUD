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

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"

/* command procedures needed */
DECLARE_DO_FUN (do_look);
DECLARE_DO_FUN (do_wear);

/*
 * Local functions.
 */
void say_spell args ((CHAR_DATA * ch, int sn));

/* imported functions */
bool remove_obj args ((CHAR_DATA * ch, int iWear, bool fReplace));
void wear_obj args ((CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace));



/* imported functions */
int saves_dispel (int dis_level, int spell_level, int duration);
bool check_dispel (int dis_level, CHAR_DATA * victim, int sn);
extern char *target_name;


/*
 * Lookup a skill by name.
 */
void
spell_identify (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  char buf[MAX_STRING_LENGTH];
  AFFECT_DATA *paf;

  snprintf (buf, sizeof (buf),
	   "Object '%s' is type %s, extra flags %s.\n\rWeight is %d, value is %d, level is %d.\n\r",
	   obj->name,
	   item_type_name (obj),
	   extra_bit_name (obj->extra_flags),
	   obj->weight / 10, obj->cost, obj->level);
  send_to_char (buf, ch);

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
      snprintf (buf, sizeof (buf), "Has %d charges of level %d",
	       obj->value[2], obj->value[0]);
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

    case ITEM_WEAPON:
      send_to_char ("Weapon type is ", ch);
      switch (obj->value[0])
	{
	case (WEAPON_EXOTIC):
	  send_to_char ("exotic.\n\r", ch);
	  break;
	case (WEAPON_SWORD):
	  send_to_char ("sword.\n\r", ch);
	  break;
	case (WEAPON_DAGGER):
	  send_to_char ("dagger.\n\r", ch);
	  break;
	case (WEAPON_SPEAR):
	  send_to_char ("spear/staff.\n\r", ch);
	  break;
	case (WEAPON_MACE):
	  send_to_char ("mace/club.\n\r", ch);
	  break;
	case (WEAPON_AXE):
	  send_to_char ("axe.\n\r", ch);
	  break;
	case (WEAPON_FLAIL):
	  send_to_char ("flail.\n\r", ch);
	  break;
	case (WEAPON_WHIP):
	  send_to_char ("whip.\n\r", ch);
	  break;
	case (WEAPON_POLEARM):
	  send_to_char ("polearm.\n\r", ch);
	  break;
	default:
	  send_to_char ("unknown.\n\r", ch);
	  break;
	}
      if (obj->clan)
	{
	  snprintf (buf, sizeof (buf), "Damage is variable.\n\r");
	}
      else
	{
	  if (obj->pIndexData->new_format)
	    snprintf (buf, sizeof (buf), "Damage is %dd%d (average %d).\n\r",
		     obj->value[1], obj->value[2],
		     (1 + obj->value[2]) * obj->value[1] / 2);
	  else
	    snprintf (buf, sizeof (buf), "Damage is %d to %d (average %d).\n\r",
		     obj->value[1], obj->value[2],
		     (obj->value[1] + obj->value[2]) / 2);
	}
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
		   "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic.\n\r",
		   obj->value[0], obj->value[1], obj->value[2],
		   obj->value[3]);
	}
      send_to_char (buf, ch);
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
  if (!obj->enchanted)
    for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
      {
	if (paf->location != APPLY_NONE && paf->modifier != 0)
	  {
	    snprintf (buf, sizeof (buf), "Affects %s by %d.\n\r",
		     affect_loc_name (paf->location), paf->modifier);
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
      }

  for (paf = obj->affected; paf != NULL; paf = paf->next)
    {
      if (paf->location != APPLY_NONE && paf->modifier != 0)
	{
	  snprintf (buf, sizeof (buf), "Affects %s by %d",
		   affect_loc_name (paf->location), paf->modifier);
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
		case TO_OBJECT:
		  snprintf (buf, sizeof (buf), "Adds %s object flag.\n",
			   extra_bit_name (paf->bitvector));
		  break;
		case TO_WEAPON:
		  snprintf (buf, sizeof (buf), "Adds %s weapon flags.\n",
			   weapon_bit_name (paf->bitvector));
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
    }
  return;
}



void
spell_infravision (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_AFFECTED (victim, AFF_INFRARED))
    {
      if (victim == ch)
	send_to_char ("You can already see in the dark.\n\r", ch);
      else
	act ("$N already has infravision.\n\r", ch, NULL, victim, TO_CHAR);
      return;
    }
  act ("$n's eyes glow red.\n\r", ch, NULL, NULL, TO_ROOM);

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 2 * level;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_INFRARED;
  affect_to_char (victim, &af);
  send_to_char ("Your eyes glow red.\n\r", victim);
  return;
}



void
spell_invis (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  AFFECT_DATA af;

  /* object invisibility */
  if (target == TARGET_OBJ)
    {
      obj = (OBJ_DATA *) vo;

      if (IS_OBJ_STAT (obj, ITEM_INVIS))
	{
	  act ("$p is already invisible.", ch, obj, NULL, TO_CHAR);
	  return;
	}

      af.where = TO_OBJECT;
      af.type = sn;
      af.level = level;
      af.duration = level + 12;
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = ITEM_INVIS;
      affect_to_obj (obj, &af);

      act ("$p fades out of sight.", ch, obj, NULL, TO_ALL);
      return;
    }

  /* character invisibility */
  victim = (CHAR_DATA *) vo;

  if (IS_SHIELDED (victim, SHD_INVISIBLE))
    return;

  act ("$n fades out of existence.", victim, NULL, NULL, TO_ROOM);

  af.where = TO_SHIELDS;
  af.type = sn;
  af.level = level;
  af.duration = level + 12;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = SHD_INVISIBLE;
  affect_to_char (victim, &af);
  send_to_char ("You fade out of existence.\n\r", victim);
  return;
}



void
spell_know_alignment (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  char *msg;
  int ap;

  ap = victim->alignment;

  if (ap > 700)
    msg = "$N has a pure and good aura.";
  else if (ap > 350)
    msg = "$N is of excellent moral character.";
  else if (ap > 100)
    msg = "$N is often kind and thoughtful.";
  else if (ap > -100)
    msg = "$N doesn't have a firm moral commitment.";
  else if (ap > -350)
    msg = "$N lies to $S friends.";
  else if (ap > -700)
    msg = "$N is a black-hearted murderer.";
  else
    msg = "$N is the embodiment of pure evil!.";

  act (msg, ch, NULL, victim, TO_CHAR);
  return;
}



void
spell_lightning_bolt (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  static const sh_int dam_each[] = {
    0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 25, 26, 28, 29,
    31, 32, 34, 35, 37, 38, 39, 40, 40, 40,
    41, 41, 42, 42, 42, 43, 43, 43, 44, 44,
    44, 45, 45, 45, 46, 46, 46, 47, 47, 47,
    48, 48, 48, 49, 49, 49, 50, 50, 50, 51,
    51, 51, 52, 52, 52, 53, 53, 53, 54, 54,
    54, 55, 55, 55, 56, 56, 56, 57, 57, 57,
    58, 58, 58, 59, 59, 59, 60, 60, 60, 61,
    61, 61, 62, 62, 62, 63, 63, 63, 64, 64
  };
  int dam;

  level = UMIN (level,(int) (sizeof (dam_each) / sizeof (dam_each[0]) - 1));
  level = UMAX (0, level);
  dam = number_range (dam_each[level] / 2, dam_each[level] * 2);
  if (saves_spell (level, victim, DAM_LIGHTNING))
    dam /= 2;
  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }
  damage_old (ch, victim, dam, sn, DAM_LIGHTNING, TRUE);
  return;
}



void
spell_locate_object (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  char buf[MAX_INPUT_LENGTH];
  BUFFER *buffer;
  OBJ_DATA *obj;
  OBJ_DATA *in_obj;
  bool found;
  int number = 0, max_found;

  found = FALSE;
  number = 0;
  max_found = IS_IMMORTAL (ch) ? 200 : 2 * level;

  buffer = new_buf ();

  for (obj = object_list; obj != NULL; obj = obj->next)
    {
      if (!can_see_obj (ch, obj) || !is_name (target_name, obj->name)
	  || IS_OBJ_STAT (obj, ITEM_NOLOCATE) || number_percent () > 2 * level
	  || ch->level < obj->level)
	continue;

      found = TRUE;
      number++;

      for (in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj)
	;

      if (in_obj->carried_by != NULL && can_see (ch, in_obj->carried_by))
	{
	  snprintf (buf, sizeof (buf), "one is carried by %s\n\r",
		   PERS (in_obj->carried_by, ch));
	}
      else
	{
	  if (IS_IMMORTAL (ch) && in_obj->in_room != NULL)
	    snprintf (buf, sizeof (buf), "one is in %s [Room %d]\n\r",
		     in_obj->in_room->name, in_obj->in_room->vnum);
	  else
	    snprintf (buf, sizeof (buf), "one is in %s\n\r",
		     in_obj->in_room == NULL
		     ? "somewhere" : in_obj->in_room->name);
	}

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

  return;
}



void
spell_magic_missile (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  static const sh_int dam_each[] = {
    0,
    3, 3, 3, 3, 4, 4, 4, 4, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    9, 9, 9, 9, 9, 9, 9, 9, 9, 9,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14
  };
  int dam;

  level = UMIN (level,(int) (sizeof (dam_each) / sizeof (dam_each[0]) - 1));
  level = UMAX (0, level);
  dam = number_range (dam_each[level] / 2, dam_each[level] * 2);
  if (saves_spell (level, victim, DAM_ENERGY))
    dam /= 2;
  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }
  damage_old (ch, victim, dam, sn, DAM_ENERGY, TRUE);
  return;
}

void
spell_mass_healing (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *gch;
  int heal_num, refresh_num;

  heal_num = skill_lookup ("heal");
  refresh_num = skill_lookup ("refresh");

  for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
    {
      if ((IS_NPC (ch) && IS_NPC (gch)) || (!IS_NPC (ch) && !IS_NPC (gch)))
	{
	  spell_heal (heal_num, level, ch, (void *) gch, TARGET_CHAR);
	  spell_refresh (refresh_num, level, ch, (void *) gch, TARGET_CHAR);
	}
    }
}


void
spell_mass_invis (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  AFFECT_DATA af;
  CHAR_DATA *gch;

  for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room)
    {
      if (!is_same_group (gch, ch) || IS_SHIELDED (gch, SHD_INVISIBLE))
	continue;
      act ("$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM);
      send_to_char ("You slowly fade out of existence.\n\r", gch);

      af.where = TO_SHIELDS;
      af.type = sn;
      af.level = level / 2;
      af.duration = 24;
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = SHD_INVISIBLE;
      affect_to_char (gch, &af);
    }
  send_to_char ("Ok.\n\r", ch);

  return;
}



void
spell_null (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  send_to_char ("That's not a spell!\n\r", ch);
  return;
}



void
spell_pass_door (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_AFFECTED (victim, AFF_PASS_DOOR))
    {
      if (victim == ch)
	send_to_char ("You are already out of phase.\n\r", ch);
      else
	act ("$N is already shifted out of phase.", ch, NULL, victim,
	     TO_CHAR);
      return;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = number_fuzzy (level / 4);
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_PASS_DOOR;
  affect_to_char (victim, &af);
  act ("$n turns translucent.", victim, NULL, NULL, TO_ROOM);
  send_to_char ("You turn translucent.\n\r", victim);
  return;
}

/* RT plague spell, very nasty */

void
spell_plague (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (saves_spell (level, victim, DAM_DISEASE) ||
      (IS_NPC (victim) && IS_SET (victim->act, ACT_UNDEAD)))
    {
      if (ch == victim)
	send_to_char ("You feel momentarily ill, but it passes.\n\r", ch);
      else
	act ("$N seems to be unaffected.", ch, NULL, victim, TO_CHAR);
      return;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level * 3 / 4;
  af.duration = level;
  af.location = APPLY_STR;
  af.modifier = -5;
  af.bitvector = AFF_PLAGUE;
  affect_join (victim, &af);

  send_to_char
    ("You scream in agony as plague sores erupt from your skin.\n\r", victim);
  act ("$n screams in agony as plague sores erupt from $s skin.",
       victim, NULL, NULL, TO_ROOM);
}

void
spell_poison (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  AFFECT_DATA af;


  if (target == TARGET_OBJ)
    {
      obj = (OBJ_DATA *) vo;

      if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
	{
	  if (IS_OBJ_STAT (obj, ITEM_BLESS)
	      || IS_OBJ_STAT (obj, ITEM_BURN_PROOF))
	    {
	      act ("Your spell fails to corrupt $p.", ch, obj, NULL, TO_CHAR);
	      return;
	    }
	  obj->value[3] = 1;
	  act ("$p is infused with poisonous vapors.", ch, obj, NULL, TO_ALL);
	  return;
	}

      if (obj->item_type == ITEM_WEAPON)
	{
	  if (IS_WEAPON_STAT (obj, WEAPON_FLAMING)
	      || IS_WEAPON_STAT (obj, WEAPON_FROST)
	      || IS_WEAPON_STAT (obj, WEAPON_VAMPIRIC)
	      || IS_WEAPON_STAT (obj, WEAPON_SHARP)
	      || IS_WEAPON_STAT (obj, WEAPON_VORPAL)
	      || IS_WEAPON_STAT (obj, WEAPON_SHOCKING)
	      || IS_OBJ_STAT (obj, ITEM_BLESS)
	      || IS_OBJ_STAT (obj, ITEM_BURN_PROOF))
	    {
	      act ("You can't seem to envenom $p.", ch, obj, NULL, TO_CHAR);
	      return;
	    }

	  if (IS_WEAPON_STAT (obj, WEAPON_POISON))
	    {
	      act ("$p is already envenomed.", ch, obj, NULL, TO_CHAR);
	      return;
	    }

	  af.where = TO_WEAPON;
	  af.type = sn;
	  af.level = level / 2;
	  af.duration = level / 8;
	  af.location = 0;
	  af.modifier = 0;
	  af.bitvector = WEAPON_POISON;
	  affect_to_obj (obj, &af);

	  act ("$p is coated with deadly venom.", ch, obj, NULL, TO_ALL);
	  return;
	}

      act ("You can't poison $p.", ch, obj, NULL, TO_CHAR);
      return;
    }

  victim = (CHAR_DATA *) vo;

  if (saves_spell (level, victim, DAM_POISON))
    {
      act ("$n turns slightly green, but it passes.", victim, NULL, NULL,
	   TO_ROOM);
      send_to_char ("You feel momentarily ill, but it passes.\n\r", victim);
      return;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level;
  af.location = APPLY_STR;
  af.modifier = -2;
  af.bitvector = AFF_POISON;
  affect_join (victim, &af);
  send_to_char ("You feel very sick.\n\r", victim);
  act ("$n looks very ill.", victim, NULL, NULL, TO_ROOM);
  return;
}



void
spell_protection_evil (int sn, int level, CHAR_DATA * ch, void *vo,
		       int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_SHIELDED (victim, SHD_PROTECT_EVIL)
      || IS_SHIELDED (victim, SHD_PROTECT_GOOD))
    {
      if (victim == ch)
	send_to_char ("You are already protected.\n\r", ch);
      else
	act ("$N is already protected.", ch, NULL, victim, TO_CHAR);
      return;
    }

  af.where = TO_SHIELDS;
  af.type = sn;
  af.level = level;
  af.duration = 24;
  af.location = APPLY_SAVING_SPELL;
  af.modifier = -1;
  af.bitvector = SHD_PROTECT_EVIL;
  affect_to_char (victim, &af);
  send_to_char ("You feel holy and pure.\n\r", victim);
  if (ch != victim)
    act ("$N is protected from evil.", ch, NULL, victim, TO_CHAR);
  return;
}

void
spell_protection_good (int sn, int level, CHAR_DATA * ch, void *vo,
		       int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_SHIELDED (victim, SHD_PROTECT_GOOD)
      || IS_SHIELDED (victim, SHD_PROTECT_EVIL))
    {
      if (victim == ch)
	send_to_char ("You are already protected.\n\r", ch);
      else
	act ("$N is already protected.", ch, NULL, victim, TO_CHAR);
      return;
    }

  af.where = TO_SHIELDS;
  af.type = sn;
  af.level = level;
  af.duration = 24;
  af.location = APPLY_SAVING_SPELL;
  af.modifier = -1;
  af.bitvector = SHD_PROTECT_GOOD;
  affect_to_char (victim, &af);
  send_to_char ("You feel aligned with darkness.\n\r", victim);
  if (ch != victim)
    act ("$N is protected from good.", ch, NULL, victim, TO_CHAR);
  return;
}


void
spell_ray_of_truth (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam, align;

  if (IS_EVIL (ch))
    {
      victim = ch;
      send_to_char ("The energy explodes inside you!\n\r", ch);
    }

  if (victim != ch)
    {
      act ("$n raises $s hand, and a blinding ray of light shoots forth!",
	   ch, NULL, NULL, TO_ROOM);
      send_to_char
	("You raise your hand and a blinding ray of light shoots forth!\n\r",
	 ch);
    }

  if (IS_GOOD (victim))
    {
      act ("$n seems unharmed by the light.", victim, NULL, victim, TO_ROOM);
      send_to_char ("The light seems powerless to affect you.\n\r", victim);
      return;
    }

  dam = dice (level, 10);
  if (saves_spell (level, victim, DAM_HOLY))
    dam /= 2;

  align = victim->alignment;
  align -= 350;

  if (align < -1000)
    align = -1000 + (align + 1000) / 3;

  dam = (dam * align * align) / 1000000;

  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }
  damage_old (ch, victim, dam, sn, DAM_HOLY, TRUE);
  spell_blindness (gsn_blindness,
		   3 * level / 4, ch, (void *) victim, TARGET_CHAR);
}


void
spell_recharge (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  int chance, percent;

  if ((obj->item_type != ITEM_WAND) & (obj->item_type != ITEM_STAFF))
    {
      send_to_char ("That item does not carry charges.\n\r", ch);
      return;
    }

  if (obj->value[3] >= 3 * level / 2)
    {
      send_to_char ("Your skills are not great enough for that.\n\r", ch);
      return;
    }

  if (obj->value[1] == 0)
    {
      send_to_char ("That item has already been recharged once.\n\r", ch);
      return;
    }

  chance = 40 + 2 * level;

  chance -= obj->value[3];	/* harder to do high-level spells */
  chance -= (obj->value[1] - obj->value[2]) * (obj->value[1] - obj->value[2]);

  chance = UMAX (level / 2, chance);

  percent = number_percent ();

  if (percent < chance / 2)
    {
      act ("$p glows softly.", ch, obj, NULL, TO_CHAR);
      act ("$p glows softly.", ch, obj, NULL, TO_ROOM);
      obj->value[2] = UMAX (obj->value[1], obj->value[2]);
      obj->value[1] = 0;
      return;
    }

  else if (percent <= chance)
    {
      int chargeback, chargemax;

      act ("$p glows softly.", ch, obj, NULL, TO_CHAR);
      act ("$p glows softly.", ch, obj, NULL, TO_CHAR);

      chargemax = obj->value[1] - obj->value[2];

      if (chargemax > 0)
	chargeback = UMAX (1, chargemax * percent / 100);
      else
	chargeback = 0;

      obj->value[2] += chargeback;
      obj->value[1] = 0;
      return;
    }

  else if (percent <= UMIN (95, 3 * chance / 2))
    {
      send_to_char ("Nothing seems to happen.\n\r", ch);
      if (obj->value[1] > 1)
	obj->value[1]--;
      return;
    }

  else				/* whoops! */
    {
      act ("$p glows brightly and explodes!", ch, obj, NULL, TO_CHAR);
      act ("$p glows brightly and explodes!", ch, obj, NULL, TO_ROOM);
      extract_obj (obj);
    }
}

void
spell_refresh (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  victim->move = UMIN (victim->move + level, victim->max_move);
  if (victim->max_move == victim->move)
    send_to_char ("You feel fully refreshed!\n\r", victim);
  else
    send_to_char ("You feel less tired.\n\r", victim);
  if (ch != victim)
    send_to_char ("Ok.\n\r", ch);
  return;
}

void
spell_remove_curse (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  bool found = FALSE;

  /* do object cases first */
  if (target == TARGET_OBJ)
    {
      obj = (OBJ_DATA *) vo;

      if (IS_OBJ_STAT (obj, ITEM_NODROP) || IS_OBJ_STAT (obj, ITEM_NOREMOVE))
	{
	  if (!IS_OBJ_STAT (obj, ITEM_NOUNCURSE)
	      && !saves_dispel (level + 2, obj->level, 0))
	    {
	      REMOVE_BIT (obj->extra_flags, ITEM_NODROP);
	      REMOVE_BIT (obj->extra_flags, ITEM_NOREMOVE);
	      act ("$p glows blue.", ch, obj, NULL, TO_ALL);
	      return;
	    }

	  act ("The curse on $p is beyond your power.", ch, obj, NULL,
	       TO_CHAR);
	  return;
	}
      else
	{
	  act ("There is no curse on $p.", ch, obj, NULL, TO_CHAR);
	  return;
	}
    }

  /* characters */
  victim = (CHAR_DATA *) vo;

  if (check_dispel (level, victim, gsn_curse))
    {
      send_to_char ("You feel better.\n\r", victim);
      act ("$n looks more relaxed.", victim, NULL, NULL, TO_ROOM);
    }

  for (obj = victim->carrying; (obj != NULL && !found);
       obj = obj->next_content)
    {
      if ((IS_OBJ_STAT (obj, ITEM_NODROP) || IS_OBJ_STAT (obj, ITEM_NOREMOVE))
	  && !IS_OBJ_STAT (obj, ITEM_NOUNCURSE))
	{			/* attempt to remove curse */
	  if (!saves_dispel (level, obj->level, 0))
	    {
	      found = TRUE;
	      REMOVE_BIT (obj->extra_flags, ITEM_NODROP);
	      REMOVE_BIT (obj->extra_flags, ITEM_NOREMOVE);
	      act ("Your $p glows blue.", victim, obj, NULL, TO_CHAR);
	      act ("$n's $p glows blue.", victim, obj, NULL, TO_ROOM);
	    }
	}
    }
}

void
spell_restore_mana (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  victim->mana = UMIN (victim->mana + 51, victim->max_mana);
  if (victim->max_mana == victim->mana)
    send_to_char ("You feel fully focused!\n\r", victim);
  else
    send_to_char ("You feel more focused.\n\r", victim);
  if (ch != victim)
    send_to_char ("Ok.\n\r", ch);
  return;
}

void
spell_sanctuary (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_SHIELDED (victim, SHD_SANCTUARY))
    {
      if (victim == ch)
	send_to_char ("You are already in sanctuary.\n\r", ch);
      else
	act ("$N is already in sanctuary.", ch, NULL, victim, TO_CHAR);
      return;
    }

  af.where = TO_SHIELDS;
  af.type = sn;
  af.level = level;
  af.duration = level / 6;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = SHD_SANCTUARY;
  affect_to_char (victim, &af);
  act ("$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM);
  send_to_char ("You are surrounded by a white aura.\n\r", victim);
  return;
}



void
spell_shield (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected (victim, sn))
    {
      if (victim == ch)
	send_to_char ("You are already shielded from harm.\n\r", ch);
      else
	act ("$N is already protected by a shield.", ch, NULL, victim,
	     TO_CHAR);
      return;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 8 + level;
  af.location = APPLY_AC;
  af.modifier = -20;
  af.bitvector = 0;
  affect_to_char (victim, &af);
  act ("$n is surrounded by a force shield.", victim, NULL, NULL, TO_ROOM);
  send_to_char ("You are surrounded by a force shield.\n\r", victim);
  return;
}



void
spell_shocking_grasp (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  static const int dam_each[] = {
    0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 20, 22, 25, 27, 29, 31, 33, 34,
    36, 37, 38, 39, 39, 39, 39, 39, 40, 40,
    40, 40, 41, 41, 41, 41, 42, 42, 42, 42,
    43, 43, 43, 43, 44, 44, 44, 44, 45, 45,
    45, 45, 46, 46, 46, 46, 47, 47, 47, 47,
    48, 48, 48, 48, 49, 49, 49, 49, 50, 50,
    50, 50, 51, 51, 51, 51, 52, 52, 52, 52,
    53, 53, 53, 53, 54, 54, 54, 54, 55, 55,
    55, 55, 56, 56, 56, 56, 57, 57, 57, 57
  };
  int dam;

  level = UMIN (level,(int) (sizeof (dam_each) / sizeof (dam_each[0]) - 1));
  level = UMAX (0, level);
  dam = number_range (dam_each[level] / 2, dam_each[level] * 2);
  if (saves_spell (level, victim, DAM_LIGHTNING))
    dam /= 2;
  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }
  damage_old (ch, victim, dam, sn, DAM_LIGHTNING, TRUE);
  return;
}



void
spell_sleep (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_AFFECTED (victim, AFF_SLEEP)
      || (IS_NPC (victim) && IS_SET (victim->act, ACT_UNDEAD))
      || (level + 2) < victim->level
      || saves_spell (level - 4, victim, DAM_CHARM))
    return;

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 4 + level;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_SLEEP;
  affect_join (victim, &af);

  if (IS_AWAKE (victim))
    {
      send_to_char ("You feel very sleepy ..... zzzzzz.\n\r", victim);
      act ("$n goes to sleep.", victim, NULL, NULL, TO_ROOM);
      victim->position = POS_SLEEPING;
    }
  return;
}

void
spell_slow (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected (victim, sn) || IS_AFFECTED (victim, AFF_SLOW))
    {
      if (victim == ch)
	send_to_char ("You can't move any slower!\n\r", ch);
      else
	act ("$N can't get any slower than that.", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (saves_spell (level, victim, DAM_OTHER)
      || IS_SET (victim->imm_flags, IMM_MAGIC))
    {
      if (victim != ch)
	send_to_char ("Nothing seemed to happen.\n\r", ch);
      send_to_char ("You feel momentarily lethargic.\n\r", victim);
      return;
    }

  if (IS_AFFECTED (victim, AFF_HASTE))
    {
      if (!check_dispel (level, victim, skill_lookup ("haste")))
	{
	  if (victim != ch)
	    send_to_char ("Spell failed.\n\r", ch);
	  send_to_char ("You feel momentarily slower.\n\r", victim);
	  return;
	}

      act ("$n is moving less quickly.", victim, NULL, NULL, TO_ROOM);
      return;
    }


  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level / 2;
  af.location = APPLY_DEX;
  af.modifier = -1 - (level >= 18) - (level >= 25) - (level >= 32);
  af.bitvector = AFF_SLOW;
  affect_to_char (victim, &af);
  send_to_char ("You feel yourself slowing d o w n...\n\r", victim);
  act ("$n starts to move in slow motion.", victim, NULL, NULL, TO_ROOM);
  return;
}




void
spell_stone_skin (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected (ch, sn))
    {
      if (victim == ch)
	send_to_char ("Your skin is already as hard as a rock.\n\r", ch);
      else
	act ("$N is already as hard as can be.", ch, NULL, victim, TO_CHAR);
      return;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level;
  af.location = APPLY_AC;
  af.modifier = -40;
  af.bitvector = 0;
  affect_to_char (victim, &af);
  act ("$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM);
  send_to_char ("Your skin turns to stone.\n\r", victim);
  return;
}



void
spell_summon (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim;

  if ((victim = get_char_world (ch, target_name)) == NULL
      || victim == ch
      || victim->in_room == NULL
      || IS_SET (ch->in_room->room_flags, ROOM_SAFE)
      || IS_SET (victim->in_room->room_flags, ROOM_SAFE)
      || IS_SET (victim->in_room->room_flags, ROOM_PRIVATE)
      || IS_SET (victim->in_room->room_flags, ROOM_SOLITARY)
      || IS_SET (victim->in_room->room_flags, ROOM_NO_RECALL)
      || (IS_NPC (victim) && IS_SET (victim->act, ACT_AGGRESSIVE))
      || victim->level >= level + 3
      || (is_clan (ch) && (is_clan (victim) && ((!is_same_clan (ch, victim)
						 && (clan_table[ch->clan].
						     pkill)
						 && (clan_table[victim->clan].
						     pkill))
						|| clan_table[victim->clan].
						independent)))
      || (!IS_NPC (victim) && victim->level >= LEVEL_IMMORTAL)
      || victim->fighting != NULL || (IS_NPC (victim)
				      && IS_SET (victim->imm_flags,
						 IMM_SUMMON))
      || (IS_NPC (victim) && victim->pIndexData->pShop != NULL)
      || (!IS_NPC (victim) && IS_SET (victim->act, PLR_NOSUMMON))
      || (IS_NPC (victim) && saves_spell (level, victim, DAM_OTHER)))

    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }

  act ("$n disappears suddenly.", victim, NULL, NULL, TO_ROOM);
  char_from_room (victim);
  char_to_room (victim, ch->in_room);
  act ("$n arrives suddenly.", victim, NULL, NULL, TO_ROOM);
  act ("$n has summoned you!", ch, NULL, victim, TO_VICT);
  do_look (victim, "auto");
  return;
}



void
spell_transport (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  return;
}

void
spell_teleport (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  ROOM_INDEX_DATA *pRoomIndex;

  if (victim->in_room == NULL
      || IS_SET (victim->in_room->room_flags, ROOM_NO_RECALL)
      || (victim != ch && IS_SET (victim->imm_flags, IMM_SUMMON))
      || (!IS_NPC (ch) && victim->fighting != NULL)
      || (victim != ch && (saves_spell (level - 5, victim, DAM_OTHER))))
    {
      send_to_char ("You failed.\n\r", ch);
      return;
    }

  pRoomIndex = get_random_room (victim);

  if (victim != ch)
    send_to_char ("You have been teleported!\n\r", victim);

  act ("$n vanishes!", victim, NULL, NULL, TO_ROOM);
  char_from_room (victim);
  char_to_room (victim, pRoomIndex);
  act ("$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM);
  do_look (victim, "auto");
  return;
}



void
spell_ventriloquate (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char speaker[MAX_INPUT_LENGTH];
  CHAR_DATA *vch;

  target_name = one_argument (target_name, speaker);

  snprintf (buf1, sizeof (buf1), "%s says '`S%s`x'.\n\r", speaker, target_name);
  snprintf (buf2, sizeof (buf2), "Someone makes %s say '`S%s`x'.\n\r", speaker, target_name);
  buf1[0] = UPPER (buf1[0]);

  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
      if (!is_name (speaker, vch->name))
	send_to_char (saves_spell (level, vch, DAM_OTHER) ? buf2 : buf1, vch);
    }

  return;
}



void
spell_weaken (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected (victim, sn) || saves_spell (level, victim, DAM_OTHER))
    return;

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level / 2;
  af.location = APPLY_STR;
  af.modifier = -1 * (level / 5);
  af.bitvector = AFF_WEAKEN;
  affect_to_char (victim, &af);
  send_to_char ("You feel your strength slip away.\n\r", victim);
  act ("$n looks tired and weak.", victim, NULL, NULL, TO_ROOM);
  return;
}



/* RT recall spell is back */

void
spell_word_of_recall (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  ROOM_INDEX_DATA *location;

  if (IS_NPC (victim))
    return;

  if (ch->alignment < 0)
    {
      if ((location = get_room_index (ROOM_VNUM_TEMPLEB)) == NULL)
	{
	  send_to_char ("You are completely lost.\n\r", victim);
	  return;
	}
    }
  else
    {
      if ((location = get_room_index (ROOM_VNUM_TEMPLE)) == NULL)
	{
	  send_to_char ("You are completely lost.\n\r", victim);
	  return;
	}
    }

  if (IS_SET (victim->in_room->room_flags, ROOM_NO_RECALL) ||
      IS_AFFECTED (victim, AFF_CURSE))
    {
      send_to_char ("Spell failed.\n\r", victim);
      return;
    }

  if (victim->fighting != NULL)
    stop_fighting (victim, TRUE);

  ch->move /= 2;
  act ("$n disappears.", victim, NULL, NULL, TO_ROOM);
  char_from_room (victim);
  char_to_room (victim, location);
  act ("$n appears in the room.", victim, NULL, NULL, TO_ROOM);
  do_look (victim, "auto");
}

