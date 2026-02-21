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



/*
 * Lookup a skill by name.
 */
int
skill_lookup (const char *name)
{
  int sn;

  for (sn = 0; sn < MAX_SKILL; sn++)
    {
      if (skill_table[sn].name == NULL)
	break;
      if (LOWER (name[0]) == LOWER (skill_table[sn].name[0])
	  && !str_prefix (name, skill_table[sn].name))
	return sn;
    }

  return -1;
}

int
find_spell (CHAR_DATA * ch, const char *name)
{
  /* finds a spell the character can cast if possible */
  int sn, found = -1;

  if (IS_NPC (ch))
    return skill_lookup (name);

  for (sn = 0; sn < MAX_SKILL; sn++)
    {
      if (skill_table[sn].name == NULL)
	break;
      if (LOWER (name[0]) == LOWER (skill_table[sn].name[0])
	  && !str_prefix (name, skill_table[sn].name))
	{
	  if (found == -1)
	    found = sn;
	  if (ch->level >= skill_table[sn].skill_level[ch->class]
	      && ch->pcdata->learned[sn] > 0)
	    return sn;
	}
    }
  return found;
}



/*
 * Lookup a skill by slot number.
 * Used for object loading.
 */
int
slot_lookup (int slot)
{
  extern bool fBootDb;
  int sn;

  if (slot <= 0)
    return -1;

  for (sn = 0; sn < MAX_SKILL; sn++)
    {
      if (slot == skill_table[sn].slot)
	return sn;
    }

  if (fBootDb)
    {
      bug ("Slot_lookup: bad slot %d.", slot);
      abort ();
    }

  return -1;
}



/*
 * Utter mystical words for an sn.
 */
void
say_spell (CHAR_DATA * ch, int sn)
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  CHAR_DATA *rch;
  char *pName;
  int iSyl;
  int length;

  struct syl_type
  {
    char *old;
    char *new;
  };

  static const struct syl_type syl_table[] = {
    {" ", " "},
    {"ar", "abra"},
    {"au", "kada"},
    {"bless", "fido"},
    {"blind", "nose"},
    {"bur", "mosa"},
    {"cu", "judi"},
    {"de", "oculo"},
    {"en", "unso"},
    {"light", "dies"},
    {"lo", "hi"},
    {"mor", "zak"},
    {"move", "sido"},
    {"ness", "lacri"},
    {"ning", "illa"},
    {"per", "duda"},
    {"ra", "gru"},
    {"fresh", "ima"},
    {"re", "candus"},
    {"son", "sabru"},
    {"tect", "infra"},
    {"tri", "cula"},
    {"ven", "nofo"},
    {"a", "a"}, {"b", "b"}, {"c", "q"}, {"d", "e"},
    {"e", "z"}, {"f", "y"}, {"g", "o"}, {"h", "p"},
    {"i", "u"}, {"j", "y"}, {"k", "t"}, {"l", "r"},
    {"m", "w"}, {"n", "i"}, {"o", "a"}, {"p", "s"},
    {"q", "d"}, {"r", "f"}, {"s", "g"}, {"t", "h"},
    {"u", "j"}, {"v", "z"}, {"w", "x"}, {"x", "n"},
    {"y", "l"}, {"z", "k"},
    {"", ""}
  };

  buf[0] = '\0';
  for (pName = skill_table[sn].name; *pName != '\0'; pName += length)
    {
      for (iSyl = 0; (length = strlen (syl_table[iSyl].old)) != 0; iSyl++)
	{
	  if (!str_prefix (syl_table[iSyl].old, pName))
	    {
	      { size_t bl = strlen (buf); if (bl < sizeof (buf) - 1) snprintf (buf + bl, sizeof (buf) - bl, "%s", syl_table[iSyl].new); }
	      break;
	    }
	}

      if (length == 0)
	length = 1;
    }

  snprintf (buf2, sizeof (buf2), "$n utters the words, '%.4000s'.", buf);
  snprintf (buf, sizeof (buf), "$n utters the words, '%.4000s'.", skill_table[sn].name);

  for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
    {
      if (rch != ch)
	act (ch->class == rch->class ? buf : buf2, ch, NULL, rch, TO_VICT);
    }

  return;
}



/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool
saves_spell (int level, CHAR_DATA * victim, int dam_type)
{
  int save;

  save = 50 + (victim->level - level) * 5 - victim->saving_throw * 2;
  if (IS_AFFECTED (victim, AFF_BERSERK))
    save += victim->level / 2;

  switch (check_immune (victim, dam_type))
    {
    case IS_IMMUNE:
      return TRUE;
    case IS_RESISTANT:
      save += 2;
      break;
    case IS_VULNERABLE:
      save -= 2;
      break;
    }

  if (!IS_NPC (victim) && class_table[victim->class].fMana)
    save = 9 * save / 10;
  save = URANGE (5, save, 95);
  return number_percent () < save;
}

/* RT save for dispels */

bool
saves_dispel (int dis_level, int spell_level, int duration)
{
  int save;

  if (duration == -1)
    spell_level += 5;
  /* very hard to dispel permanent effects */

  save = 50 + (spell_level - dis_level) * 5;
  save = URANGE (5, save, 95);
  return number_percent () < save;
}

/* co-routine for dispel magic and cancellation */

bool
check_dispel (int dis_level, CHAR_DATA * victim, int sn)
{
  AFFECT_DATA *af;

  if (is_affected (victim, sn))
    {
      for (af = victim->affected; af != NULL; af = af->next)
	{
	  if (af->type == sn)
	    {
	      if (!saves_dispel (dis_level, af->level, af->duration))
		{
		  affect_strip (victim, sn);
		  if (skill_table[sn].msg_off)
		    {
		      send_to_char (skill_table[sn].msg_off, victim);
		      send_to_char ("\n\r", victim);
		    }
		  return TRUE;
		}
	      else
		af->level--;
	    }
	}
    }
  return FALSE;
}

/* for finding mana costs -- temporary version */
int
mana_cost (CHAR_DATA * ch, int min_mana, int level)
{
  if (ch->level + 2 == level)
    return 1000;
  return UMAX (min_mana, (100 / (2 + ch->level - level)));
}



/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char *target_name;
char *third_name;

void
do_cast (CHAR_DATA * ch, char *argument)
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  void *vo;
  int mana;
  int sn;
  int target;

  /*
   * Switched NPC's can cast spells, but others can't.
   */
  if (IS_NPC (ch) && ch->desc == NULL)
    return;

  target_name = one_argument (argument, arg1);
  third_name = one_argument (target_name, arg2);
  memmove (target_name, arg2, strlen (arg2) + 1);
  one_argument (third_name, arg3);

  if (arg1[0] == '\0')
    {
      send_to_char ("Cast which what where?\n\r", ch);
      return;
    }

  if (ch->stunned)
    {
      send_to_char ("You're still a little woozy.\n\r", ch);
      return;
    }

  if ((sn = find_spell (ch, arg1)) < 0
      || (!IS_NPC (ch) && (ch->level < skill_table[sn].skill_level[ch->class]
			   || ch->pcdata->learned[sn] == 0)))
    {
      send_to_char ("You don't know any spells of that name.\n\r", ch);
      return;
    }

  if (ch->position < skill_table[sn].minimum_position)
    {
      send_to_char ("You can't concentrate enough.\n\r", ch);
      return;
    }

  if (ch->level + 2 == skill_table[sn].skill_level[ch->class])
    mana = 50;
  else
    mana = UMAX (skill_table[sn].min_mana,
		 100 / (2 + ch->level -
			skill_table[sn].skill_level[ch->class]));

  if (!strcmp (skill_table[sn].name, "restore mana"))
    mana = 1;

  /*
   * Locate targets.
   */
  victim = NULL;
  obj = NULL;
  vo = NULL;
  target = TARGET_NONE;

  switch (skill_table[sn].target)
    {
    default:
      bug ("Do_cast: bad target for sn %d.", sn);
      return;

    case TAR_IGNORE:
      break;

    case TAR_CHAR_OFFENSIVE:
      if (arg2[0] == '\0')
	{
	  if ((victim = ch->fighting) == NULL)
	    {
	      send_to_char ("Cast the spell on whom?\n\r", ch);
	      return;
	    }
	}
      else
	{
	  if ((victim = get_char_room (ch, target_name)) == NULL)
	    {
	      send_to_char ("They aren't here.\n\r", ch);
	      return;
	    }
	}
/*
        if ( ch == victim )
        {
            send_to_char( "You can't do that to yourself.\n\r", ch );
            return;
        }
*/


      if (!IS_NPC (ch))
	{

	  if (is_safe (ch, victim) && victim != ch)
	    {
	      send_to_char ("Not on that target.\n\r", ch);
	      return;
	    }
	}

      if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
	{
	  send_to_char ("You can't do that on your own follower.\n\r", ch);
	  return;
	}

      if ((!IS_NPC (ch) && !IS_NPC (victim))
	  && (!IS_IMMORTAL (ch))
	  && (!IS_IMMORTAL (victim))
	  && (ch != victim)
	  && (!skill_table[sn].socket)
	  && (!strcmp (ch->pcdata->socket, victim->pcdata->socket)))
	{
	  send_to_char ("Spell failed.\n\r", ch);
	  return;
	}

      vo = (void *) victim;
      target = TARGET_CHAR;
      break;

    case TAR_CHAR_DEFENSIVE:
      if (arg2[0] == '\0')
	{
	  victim = ch;
	}
      else
	{
	  if ((victim = get_char_room (ch, target_name)) == NULL)
	    {
	      send_to_char ("They aren't here.\n\r", ch);
	      return;
	    }
	}

      if ((!IS_NPC (ch) && !IS_NPC (victim))
	  && (!IS_IMMORTAL (ch))
	  && (!IS_IMMORTAL (victim))
	  && (ch != victim)
	  && (!skill_table[sn].socket)
	  && (!strcmp (ch->pcdata->socket, victim->pcdata->socket)))
	{
	  send_to_char ("Spell failed.\n\r", ch);
	  return;
	}

      vo = (void *) victim;
      target = TARGET_CHAR;
      break;

    case TAR_CHAR_SELF:
      if (arg2[0] != '\0' && !is_name (target_name, ch->name))
	{
	  send_to_char ("You cannot cast this spell on another.\n\r", ch);
	  return;
	}

      vo = (void *) ch;
      target = TARGET_CHAR;
      break;

    case TAR_OBJ_INV:
      if (arg2[0] == '\0')
	{
	  send_to_char ("What should the spell be cast upon?\n\r", ch);
	  return;
	}

      if ((obj = get_obj_carry (ch, target_name)) == NULL)
	{
	  send_to_char ("You are not carrying that.\n\r", ch);
	  return;
	}

      vo = (void *) obj;
      target = TARGET_OBJ;
      break;

    case TAR_OBJ_CHAR_OFF:
      if (arg2[0] == '\0')
	{
	  if ((victim = ch->fighting) == NULL)
	    {
	      send_to_char ("Cast the spell on whom or what?\n\r", ch);
	      return;
	    }

	  target = TARGET_CHAR;
	}
      else if ((victim = get_char_room (ch, target_name)) != NULL)
	{
	  target = TARGET_CHAR;
	}

      if (target == TARGET_CHAR)	/* check the sanity of the attack */
	{
	  if (is_safe_spell (ch, victim, FALSE) && victim != ch)
	    {
	      send_to_char ("Not on that target.\n\r", ch);
	      return;
	    }

	  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)
	    {
	      send_to_char ("You can't do that on your own follower.\n\r",
			    ch);
	      return;
	    }

	  if ((!IS_NPC (ch) && !IS_NPC (victim))
	      && (!IS_IMMORTAL (ch))
	      && (!IS_IMMORTAL (victim))
	      && (ch != victim)
	      && (!skill_table[sn].socket)
	      && (!strcmp (ch->pcdata->socket, victim->pcdata->socket)))
	    {
	      send_to_char ("Spell failed.\n\r", ch);
	      return;
	    }

	  vo = (void *) victim;
	}
      else if ((obj = get_obj_here (ch, target_name)) != NULL)
	{
	  vo = (void *) obj;
	  target = TARGET_OBJ;
	}
      else
	{
	  send_to_char ("You don't see that here.\n\r", ch);
	  return;
	}
      break;

    case TAR_OBJ_CHAR_DEF:
      if (arg2[0] == '\0')
	{
	  vo = (void *) ch;
	  target = TARGET_CHAR;
	}
      else if ((victim = get_char_room (ch, target_name)) != NULL)
	{

	  if ((!IS_NPC (ch) && !IS_NPC (victim))
	      && (!IS_IMMORTAL (ch))
	      && (!IS_IMMORTAL (victim))
	      && (ch != victim)
	      && (!skill_table[sn].socket)
	      && (!strcmp (ch->pcdata->socket, victim->pcdata->socket)))
	    {
	      send_to_char ("Spell failed.\n\r", ch);
	      return;
	    }

	  vo = (void *) victim;
	  target = TARGET_CHAR;
	}
      else if ((obj = get_obj_carry (ch, target_name)) != NULL)
	{
	  vo = (void *) obj;
	  target = TARGET_OBJ;
	}
      else
	{
	  send_to_char ("You don't see that here.\n\r", ch);
	  return;
	}
      break;

    case TAR_OBJ_TRAN:
      if (arg2[0] == '\0')
	{
	  send_to_char ("Transport what to whom?\n\r", ch);
	  return;
	}
      if (arg3[0] == '\0')
	{
	  send_to_char ("Transport it to whom?\n\r", ch);
	  return;
	}
      if ((obj = get_obj_carry (ch, target_name)) == NULL)
	{
	  send_to_char ("You are not carrying that.\n\r", ch);
	  return;
	}
      if ((victim = get_char_world (ch, third_name)) == NULL
	  || IS_NPC (victim))
	{
	  send_to_char ("They aren't here.\n\r", ch);
	  return;
	}
      if (!IS_NPC (ch) && ch->mana < mana)
	{
	  send_to_char ("You don't have enough mana.\n\r", ch);
	  return;
	}
      if (obj->wear_loc != WEAR_NONE)
	{
	  send_to_char ("You must remove it first.\n\r", ch);
	  return;
	}
      if (IS_SET (victim->act, PLR_NOTRAN) && ch->level < SQUIRE)
	{
	  send_to_char ("They don't want it.\n\r", ch);
	  return;
	}
      if (victim->carry_number + get_obj_number (obj) > can_carry_n (victim))
	{
	  act ("$N has $S hands full.", ch, NULL, victim, TO_CHAR);
	  return;
	}
      if (get_carry_weight (victim) + get_obj_weight (obj) >
	  can_carry_w (victim))
	{
	  act ("$N can't carry that much weight.", ch, NULL, victim, TO_CHAR);
	  return;
	}
      if (!can_see_obj (victim, obj))
	{
	  act ("$N can't see it.", ch, NULL, victim, TO_CHAR);
	  return;
	}
      if ((!IS_NPC (ch) && !IS_NPC (victim))
	  && (!IS_IMMORTAL (ch))
	  && (!IS_IMMORTAL (victim))
	  && (ch != victim)
	  && (!skill_table[sn].socket)
	  && (!strcmp (ch->pcdata->socket, victim->pcdata->socket)))
	{
	  send_to_char ("Spell failed.\n\r", ch);
	  return;
	}
      WAIT_STATE (ch, skill_table[sn].beats);
      if (!can_drop_obj (ch, obj) || IS_OBJ_STAT (obj, ITEM_QUEST))
	{
	  send_to_char ("It seems happy where it is.\n\r", ch);
	  check_improve (ch, sn, FALSE, 1);
	  ch->mana -= mana / 3;
	  return;
	}
      if ((obj->pIndexData->vnum == OBJ_VNUM_VOODOO) && (ch->level <= HERO))
	{
	  send_to_char ("You can't transport voodoo dolls.\n\r", ch);
	  check_improve (ch, sn, FALSE, 1);
	  ch->mana -= mana / 3;
	  return;
	}

      if (number_percent () > get_skill (ch, sn))
	{
	  send_to_char ("You lost your concentration.\n\r", ch);
	  check_improve (ch, sn, FALSE, 1);
	  ch->mana -= mana / 2;
	}
      else
	{
	  ch->mana -= mana;
	  obj_from_char (obj);
	  obj_to_char (obj, victim);
	  act ("$p glows `Ggreen`x, then disappears.", ch, obj, victim,
	       TO_CHAR);
	  act ("$p suddenly appears in your inventory.", ch, obj, victim,
	       TO_VICT);
	  act ("$p glows `Ggreen`x, then disappears from $n's inventory.", ch,
	       obj, victim, TO_NOTVICT);
	  check_improve (ch, sn, TRUE, 1);
	  if (IS_OBJ_STAT (obj, ITEM_FORCED) && (victim->level <= HERO))
	    {
	      do_wear (victim, obj->name);
	    }
	}
      return;
      break;
    }

  if (!IS_NPC (ch) && ch->mana < mana)
    {
      send_to_char ("You don't have enough mana.\n\r", ch);
      return;
    }

  if (str_cmp (skill_table[sn].name, "ventriloquate"))
    say_spell (ch, sn);

  WAIT_STATE (ch, skill_table[sn].beats);

  if (number_percent () > get_skill (ch, sn))
    {
      send_to_char ("You lost your concentration.\n\r", ch);
      check_improve (ch, sn, FALSE, 1);
      ch->mana -= mana / 2;
    }
  else
    {
      ch->mana -= mana;
      if (IS_NPC (ch) || class_table[ch->class].fMana)
	/* class has spells */
	(*skill_table[sn].spell_fun) (sn, ch->level, ch, vo, target);
      else
	(*skill_table[sn].spell_fun) (sn, 3 * ch->level / 4, ch, vo, target);
      check_improve (ch, sn, TRUE, 1);
    }

  if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
       || (skill_table[sn].target == TAR_OBJ_CHAR_OFF
	   && target == TARGET_CHAR)) && victim != ch && victim->master != ch)
    {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;

      for (vch = ch->in_room->people; vch; vch = vch_next)
	{
	  vch_next = vch->next_in_room;
	  if (victim == vch && victim->fighting == NULL)
	    {
	      multi_hit (victim, ch, TYPE_UNDEFINED);
	      break;
	    }
	}
    }
  return;
}



/*
 * Cast spells at targets using a magical object.
 */
void
obj_cast_spell (int sn, int level, CHAR_DATA * ch, CHAR_DATA * victim,
		OBJ_DATA * obj)
{
  void *vo;
  int target = TARGET_NONE;

  if (sn <= 0)
    return;

  if (sn >= MAX_SKILL || skill_table[sn].spell_fun == 0)
    {
      bug ("Obj_cast_spell: bad sn %d.", sn);
      return;
    }

  if (ch->fighting != NULL)
    {
      WAIT_STATE (ch, skill_table[sn].beats);
    }

  switch (skill_table[sn].target)
    {
    default:
      bug ("Obj_cast_spell: bad target for sn %d.", sn);
      return;

    case TAR_IGNORE:
      vo = NULL;
      break;

    case TAR_CHAR_OFFENSIVE:
      if (victim == NULL)
	victim = ch->fighting;
      if (victim == NULL)
	{
	  send_to_char ("You can't do that.\n\r", ch);
	  return;
	}
      if (is_safe (ch, victim) && ch != victim)
	{
	  send_to_char ("Something isn't right...\n\r", ch);
	  return;
	}
      vo = (void *) victim;
      target = TARGET_CHAR;
      break;

    case TAR_CHAR_DEFENSIVE:
    case TAR_CHAR_SELF:
      if (victim == NULL)
	victim = ch;
      vo = (void *) victim;
      target = TARGET_CHAR;
      break;

    case TAR_OBJ_INV:
      if (obj == NULL)
	{
	  send_to_char ("You can't do that.\n\r", ch);
	  return;
	}
      vo = (void *) obj;
      target = TARGET_OBJ;
      break;

    case TAR_OBJ_CHAR_OFF:
      if (victim == NULL && obj == NULL)
	{
	  if (ch->fighting != NULL)
	    victim = ch->fighting;
	  else
	    {
	      send_to_char ("You can't do that.\n\r", ch);
	      return;
	    }
	}
      if (victim != NULL)
	{
	  if (is_safe_spell (ch, victim, FALSE) && ch != victim)
	    {
	      send_to_char ("Somehting isn't right...\n\r", ch);
	      return;
	    }

	  vo = (void *) victim;
	  target = TARGET_CHAR;
	}
      else
	{
	  vo = (void *) obj;
	  target = TARGET_OBJ;
	}
      break;


    case TAR_OBJ_CHAR_DEF:
      if (victim == NULL && obj == NULL)
	{
	  vo = (void *) ch;
	  target = TARGET_CHAR;
	}
      else if (victim != NULL)
	{
	  vo = (void *) victim;
	  target = TARGET_CHAR;
	}
      else
	{
	  vo = (void *) obj;
	  target = TARGET_OBJ;
	}

      break;
    }

  target_name = "";
  (*skill_table[sn].spell_fun) (sn, level, ch, vo, target);



  if ((skill_table[sn].target == TAR_CHAR_OFFENSIVE
       || (skill_table[sn].target == TAR_OBJ_CHAR_OFF
	   && target == TARGET_CHAR)) && victim != ch && victim->master != ch)
    {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;

      for (vch = ch->in_room->people; vch; vch = vch_next)
	{
	  vch_next = vch->next_in_room;
	  if (victim == vch && victim->fighting == NULL)
	    {
	      multi_hit (victim, ch, TYPE_UNDEFINED);
	      break;
	    }
	}
    }

  return;
}



/*
 * Spell functions.
 */
void
spell_acid_blast (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }

  dam = dice (level, 12);
  if (saves_spell (level, victim, DAM_ACID))
    dam /= 2;
  damage_old (ch, victim, dam, sn, DAM_ACID, TRUE);
  return;
}



void
spell_armor (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected (victim, sn))
    {
      if (victim == ch)
	send_to_char ("You are already armored.\n\r", ch);
      else
	act ("$N is already armored.", ch, NULL, victim, TO_CHAR);
      return;
    }
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 24;
  af.modifier = -20;
  af.location = APPLY_AC;
  af.bitvector = 0;
  affect_to_char (victim, &af);
  send_to_char ("You feel someone protecting you.\n\r", victim);
  if (ch != victim)
    act ("$N is protected by your magic.", ch, NULL, victim, TO_CHAR);
  return;
}



void
spell_bless (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  AFFECT_DATA af;

  /* deal with the object case first */
  if (target == TARGET_OBJ)
    {
      obj = (OBJ_DATA *) vo;
      if (IS_OBJ_STAT (obj, ITEM_BLESS))
	{
	  act ("$p is already blessed.", ch, obj, NULL, TO_CHAR);
	  return;
	}

      if (IS_OBJ_STAT (obj, ITEM_EVIL))
	{
	  AFFECT_DATA *paf;

	  paf = affect_find (obj->affected, gsn_curse);
	  if (!saves_dispel (level, paf != NULL ? paf->level : obj->level, 0))
	    {
	      if (paf != NULL)
		affect_remove_obj (obj, paf);
	      act ("$p glows a pale blue.", ch, obj, NULL, TO_ALL);
	      REMOVE_BIT (obj->extra_flags, ITEM_EVIL);
	      return;
	    }
	  else
	    {
	      act ("The evil of $p is too powerful for you to overcome.",
		   ch, obj, NULL, TO_CHAR);
	      return;
	    }
	}

      af.where = TO_OBJECT;
      af.type = sn;
      af.level = level;
      af.duration = 6 + level;
      af.location = APPLY_SAVES;
      af.modifier = -1;
      af.bitvector = ITEM_BLESS;
      affect_to_obj (obj, &af);

      act ("$p glows with a holy aura.", ch, obj, NULL, TO_ALL);
      return;
    }

  /* character target */
  victim = (CHAR_DATA *) vo;


  if (victim->position == POS_FIGHTING || is_affected (victim, sn))
    {
      if (victim == ch)
	send_to_char ("You are already blessed.\n\r", ch);
      else
	act ("$N already has divine favor.", ch, NULL, victim, TO_CHAR);
      return;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 6 + level;
  af.location = APPLY_HITROLL;
  af.modifier = level / 8;
  af.bitvector = 0;
  affect_to_char (victim, &af);

  af.location = APPLY_SAVING_SPELL;
  af.modifier = 0 - level / 8;
  affect_to_char (victim, &af);
  send_to_char ("You feel righteous.\n\r", victim);
  if (ch != victim)
    act ("You grant $N the favor of your god.", ch, NULL, victim, TO_CHAR);
  return;
}



void
spell_blindness (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_AFFECTED (victim, AFF_BLIND)
      || saves_spell (level, victim, DAM_OTHER))
    return;


  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.location = APPLY_HITROLL;
  af.modifier = -4;
  af.duration = 1 + level;
  af.bitvector = AFF_BLIND;
  affect_to_char (victim, &af);
  send_to_char ("You are blinded!\n\r", victim);
  act ("$n appears to be blinded.", victim, NULL, NULL, TO_ROOM);
  return;
}



void
spell_burning_hands (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  static const sh_int dam_each[] = {
    0,
    0, 0, 0, 0, 0, 0, 0, 0, 14, 15,
    17, 18, 20, 21, 23, 24, 26, 27, 28, 29,
    29, 29, 29, 29, 30, 30, 30, 30, 31, 31,
    31, 31, 32, 32, 32, 32, 33, 33, 33, 33,
    34, 34, 34, 34, 35, 35, 35, 35, 36, 36,
    36, 36, 37, 37, 37, 37, 38, 38, 38, 38,
    39, 39, 39, 39, 40, 40, 40, 40, 41, 41,
    41, 41, 42, 42, 42, 42, 43, 43, 43, 43,
    44, 44, 44, 44, 45, 45, 45, 45, 46, 46,
    46, 46, 47, 47, 47, 47, 48, 48, 48, 48
  };
  int dam;

  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }

  level = UMIN (level, (int) (sizeof (dam_each) / sizeof (dam_each[0]) - 1));
  level = UMAX (0, level);
  dam = number_range (dam_each[level] / 2, dam_each[level] * 2);
  if (saves_spell (level, victim, DAM_FIRE))
    dam /= 2;
  damage_old (ch, victim, dam, sn, DAM_FIRE, TRUE);
  return;
}



void
spell_call_lightning (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;

  if (!IS_OUTSIDE (ch))
    {
      send_to_char ("You must be out of doors.\n\r", ch);
      return;
    }

  if (weather_info.sky < SKY_RAINING)
    {
      send_to_char ("You need bad weather.\n\r", ch);
      return;
    }

  dam = dice (level / 2, 8);

  act ("$G's `Ylightning`x strikes your foes!", ch, NULL, NULL, TO_CHAR);
  act ("$n calls $G's `Ylightning`x to strike $s foes!",
       ch, NULL, NULL, TO_ROOM);

  for (vch = char_list; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next;
      if (vch->in_room == NULL)
	continue;
      if (vch->in_room == ch->in_room)
	{
	  if (vch != ch && (IS_NPC (ch) ? !IS_NPC (vch) : IS_NPC (vch)))
	    {
	      if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (vch)))
		{
		  ch->attacker = TRUE;
		  vch->attacker = FALSE;
		}
	      damage_old (ch, vch, saves_spell (level, vch, DAM_LIGHTNING)
			  ? dam / 2 : dam, sn, DAM_LIGHTNING, TRUE);
	    }
	  continue;
	}

      if (vch->in_room->area == ch->in_room->area
	  && IS_OUTSIDE (vch) && IS_AWAKE (vch))
	send_to_char ("`z`YLightning`x flashes in the sky.\n\r", vch);
    }

  return;
}

/* RT calm spell stops all fighting in the room */

void
spell_calm (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *vch;
  int mlevel = 0;
  int count = 0;
  int high_level = 0;
  int chance;
  AFFECT_DATA af;

  /* get sum of all mobile levels in the room */
  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
    {
      if (vch->position == POS_FIGHTING)
	{
	  count++;
	  if (IS_NPC (vch))
	    mlevel += vch->level;
	  else
	    mlevel += vch->level / 2;
	  high_level = UMAX (high_level, vch->level);
	}
    }

  /* compute chance of stopping combat */
  chance = 4 * level - high_level + 2 * count;

  if (IS_IMMORTAL (ch))		/* always works */
    mlevel = 0;

  if (number_range (0, chance) >= mlevel)	/* hard to stop large fights */
    {
      for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{
	  if (IS_NPC (vch) && (IS_SET (vch->imm_flags, IMM_MAGIC) ||
			       IS_SET (vch->act, ACT_UNDEAD)))
	    return;

	  if (IS_AFFECTED (vch, AFF_CALM) || IS_AFFECTED (vch, AFF_BERSERK)
	      || is_affected (vch, skill_lookup ("frenzy")))
	    return;

	  send_to_char ("A wave of calm passes over you.\n\r", vch);

	  if (vch->fighting || vch->position == POS_FIGHTING)
	    stop_fighting (vch, FALSE);


	  af.where = TO_AFFECTS;
	  af.type = sn;
	  af.level = level;
	  af.duration = level / 4;
	  af.location = APPLY_HITROLL;
	  if (!IS_NPC (vch))
	    af.modifier = -5;
	  else
	    af.modifier = -2;
	  af.bitvector = AFF_CALM;
	  affect_to_char (vch, &af);

	  af.location = APPLY_DAMROLL;
	  affect_to_char (vch, &af);
	}
    }
}

void
spell_cancellation (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  bool found = FALSE;

  level += 2;

  if ((!IS_NPC (ch) && IS_NPC (victim) &&
       !(IS_AFFECTED (ch, AFF_CHARM) && ch->master == victim)) ||
      (IS_NPC (ch) && !IS_NPC (victim)))
    {
      send_to_char ("You failed, try dispel magic.\n\r", ch);
      return;
    }

  /* unlike dispel magic, the victim gets NO save */

  /* begin running through the spells */

  if (check_dispel (level, victim, skill_lookup ("armor")))
    found = TRUE;

  if (check_dispel (level, victim, skill_lookup ("bless")))
    found = TRUE;

  if (check_dispel (level, victim, skill_lookup ("blindness")))
    {
      found = TRUE;
      act ("$n is no longer blinded.", victim, NULL, NULL, TO_ROOM);
    }

  if (check_dispel (level, victim, skill_lookup ("calm")))
    {
      found = TRUE;
      act ("$n no longer looks so peaceful...", victim, NULL, NULL, TO_ROOM);
    }

  if (check_dispel (level, victim, skill_lookup ("change sex")))
    {
      found = TRUE;
      act ("$n looks more like $mself again.", victim, NULL, NULL, TO_ROOM);
    }

  if (check_dispel (level, victim, skill_lookup ("charm person")))
    {
      found = TRUE;
      act ("$n regains $s free will.", victim, NULL, NULL, TO_ROOM);
    }

  if (check_dispel (level, victim, skill_lookup ("chill touch")))
    {
      found = TRUE;
      act ("$n looks warmer.", victim, NULL, NULL, TO_ROOM);
    }

  if (check_dispel (level, victim, skill_lookup ("curse")))
    found = TRUE;

  if (check_dispel (level, victim, skill_lookup ("detect evil")))
    found = TRUE;

  if (check_dispel (level, victim, skill_lookup ("detect good")))
    found = TRUE;

  if (check_dispel (level, victim, skill_lookup ("detect hidden")))
    found = TRUE;

  if (check_dispel (level, victim, skill_lookup ("detect invis")))
    found = TRUE;

  if (check_dispel (level, victim, skill_lookup ("detect hidden")))
    found = TRUE;

  if (check_dispel (level, victim, skill_lookup ("detect magic")))
    found = TRUE;

  if (check_dispel (level, victim, skill_lookup ("faerie fire")))
    {
      act ("$n's outline fades.", victim, NULL, NULL, TO_ROOM);
      found = TRUE;
    }

  if (check_dispel (level, victim, skill_lookup ("fly")))
    {
      act ("$n falls to the ground!", victim, NULL, NULL, TO_ROOM);
      found = TRUE;
    }

  if (check_dispel (level, victim, skill_lookup ("frenzy")))
    {
      act ("$n no longer looks so wild.", victim, NULL, NULL, TO_ROOM);;
      found = TRUE;
    }

  if (check_dispel (level, victim, skill_lookup ("giant strength")))
    {
      act ("$n no longer looks so mighty.", victim, NULL, NULL, TO_ROOM);
      found = TRUE;
    }

  if (check_dispel (level, victim, skill_lookup ("haste")))
    {
      act ("$n is no longer moving so quickly.", victim, NULL, NULL, TO_ROOM);
      found = TRUE;
    }

  if (check_dispel (level, victim, skill_lookup ("infravision")))
    found = TRUE;

  if (check_dispel (level, victim, skill_lookup ("invis")))
    {
      act ("$n fades into existance.", victim, NULL, NULL, TO_ROOM);
      found = TRUE;
    }

  if (check_dispel (level, victim, skill_lookup ("mass invis")))
    {
      act ("$n fades into existance.", victim, NULL, NULL, TO_ROOM);
      found = TRUE;
    }

  if (check_dispel (level, victim, skill_lookup ("pass door")))
    found = TRUE;

  if (check_dispel (level, victim, skill_lookup ("protection evil")))
    found = TRUE;

  if (check_dispel (level, victim, skill_lookup ("protection good")))
    found = TRUE;

  if (check_dispel (level, victim, skill_lookup ("sanctuary")))
    {
      act ("The white aura around $n's body vanishes.",
	   victim, NULL, NULL, TO_ROOM);
      found = TRUE;
    }

  if (check_dispel (level, victim, skill_lookup ("shield")))
    {
      act ("The shield protecting $n vanishes.", victim, NULL, NULL, TO_ROOM);
      found = TRUE;
    }

  if (check_dispel (level, victim, skill_lookup ("sleep")))
    found = TRUE;

  if (check_dispel (level, victim, skill_lookup ("slow")))
    {
      act ("$n is no longer moving so slowly.", victim, NULL, NULL, TO_ROOM);
      found = TRUE;
    }

  if (check_dispel (level, victim, skill_lookup ("stone skin")))
    {
      act ("$n's skin regains its normal texture.", victim, NULL, NULL,
	   TO_ROOM);
      found = TRUE;
    }

  if (check_dispel (level, victim, skill_lookup ("weaken")))
    {
      act ("$n looks stronger.", victim, NULL, NULL, TO_ROOM);
      found = TRUE;
    }

  if (found)
    send_to_char ("Ok.\n\r", ch);
  else
    send_to_char ("Spell failed.\n\r", ch);
}

void
spell_cause_light (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }
  damage_old (ch, victim, dice (1, 8) + level / 3, sn, DAM_HARM, TRUE);
  return;
}



void
spell_cause_critical (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }
  damage_old (ch, victim, dice (3, 8) + level - 6, sn, DAM_HARM, TRUE);
  return;
}



void
spell_cause_serious (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }
  damage_old (ch, victim, dice (2, 8) + level / 2, sn, DAM_HARM, TRUE);
  return;
}

void
spell_chain_lightning (int sn, int level, CHAR_DATA * ch, void *vo,
		       int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  CHAR_DATA *tmp_vict, *last_vict, *next_vict;
  bool found;
  int dam;

  /* first strike */

  act ("A lightning bolt leaps from $n's hand and arcs to $N.",
       ch, NULL, victim, TO_ROOM);
  act ("A lightning bolt leaps from your hand and arcs to $N.",
       ch, NULL, victim, TO_CHAR);
  act ("A lightning bolt leaps from $n's hand and hits you!",
       ch, NULL, victim, TO_VICT);

  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }
  dam = dice (level, 6);
  if (saves_spell (level, victim, DAM_LIGHTNING))
    dam /= 3;
  damage_old (ch, victim, dam, sn, DAM_LIGHTNING, TRUE);
  last_vict = victim;
  level -= 4;			/* decrement damage */

  /* new targets */
  while (level > 0)
    {
      found = FALSE;
      for (tmp_vict = ch->in_room->people;
	   tmp_vict != NULL; tmp_vict = next_vict)
	{
	  next_vict = tmp_vict->next_in_room;
	  if (!is_safe_spell (ch, tmp_vict, TRUE) && tmp_vict != last_vict)
	    {
	      found = TRUE;
	      last_vict = tmp_vict;
	      act ("The bolt arcs to $n!", tmp_vict, NULL, NULL, TO_ROOM);
	      act ("The bolt hits you!", tmp_vict, NULL, NULL, TO_CHAR);
	      dam = dice (level, 6);
	      if (saves_spell (level, tmp_vict, DAM_LIGHTNING))
		dam /= 3;
	      damage_old (ch, tmp_vict, dam, sn, DAM_LIGHTNING, TRUE);
	      level -= 4;	/* decrement damage */
	    }
	}			/* end target searching loop */

      if (!found)		/* no target found, hit the caster */
	{
	  if (ch == NULL)
	    return;

	  if (last_vict == ch)	/* no double hits */
	    {
	      act ("The bolt seems to have fizzled out.", ch, NULL, NULL,
		   TO_ROOM);
	      act ("The bolt grounds out through your body.", ch, NULL, NULL,
		   TO_CHAR);
	      return;
	    }

	  last_vict = ch;
	  act ("The bolt arcs to $n...whoops!", ch, NULL, NULL, TO_ROOM);
	  send_to_char ("You are struck by your own lightning!\n\r", ch);
	  dam = dice (level, 6);
	  if (saves_spell (level, ch, DAM_LIGHTNING))
	    dam /= 3;
	  damage_old (ch, ch, dam, sn, DAM_LIGHTNING, TRUE);
	  level -= 4;		/* decrement damage */
	  if (ch == NULL)
	    return;
	}
      /* now go back and find more targets */
    }
}


void
spell_change_sex (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected (victim, sn))
    {
      if (victim == ch)
	send_to_char ("You've already been changed.\n\r", ch);
      else
	act ("$N has already had $s(?) sex changed.", ch, NULL, victim,
	     TO_CHAR);
      return;
    }
  if (saves_spell (level, victim, DAM_OTHER))
    return;
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 2 * level;
  af.location = APPLY_SEX;
  do
    {
      af.modifier = number_range (0, 2) - victim->sex;
    }
  while (af.modifier == 0);
  af.bitvector = 0;
  affect_to_char (victim, &af);
  send_to_char ("You feel different.\n\r", victim);
  act ("$n doesn't look like $mself anymore...", victim, NULL, NULL, TO_ROOM);
  return;
}



void
spell_charm_person (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_safe (ch, victim))
    return;

  if (victim == ch)
    {
      send_to_char ("You like yourself even better!\n\r", ch);
      return;
    }

  if (victim->position == POS_SLEEPING)
    {
      send_to_char ("You can not get your victim's attention.\n\r", ch);
      send_to_char ("Your slumbers are briefly troubled.\n\r", victim);
      return;
    }

  if (IS_AFFECTED (victim, AFF_CHARM)
      || IS_AFFECTED (ch, AFF_CHARM)
      || level < victim->level
      || IS_SET (victim->imm_flags, IMM_CHARM)
      || saves_spell (level, victim, DAM_CHARM))
    return;


  if (IS_SET (victim->in_room->room_flags, ROOM_LAW))
    {
      send_to_char
	("The mayor does not allow charming in the city limits.\n\r", ch);
      return;
    }

  if (victim->master)
    stop_follower (victim);
  add_follower (victim, ch);
  victim->leader = ch;
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = number_fuzzy (level / 4);
  af.location = 0;
  af.modifier = 0;
  af.bitvector = AFF_CHARM;
  affect_to_char (victim, &af);
  act ("Isn't $n just so nice?", ch, NULL, victim, TO_VICT);
  if (ch != victim)
    act ("$N looks at you with adoring eyes.", ch, NULL, victim, TO_CHAR);
  return;
}



void
spell_chill_touch (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  static const sh_int dam_each[] = {
    0,
    0, 0, 0, 0, 6, 6, 7, 7, 8, 8,
    9, 10, 12, 12, 13, 13, 13, 13, 13, 13,
    14, 14, 14, 14, 14, 14, 15, 15, 15, 15,
    15, 15, 16, 16, 16, 16, 16, 16, 17, 17,
    17, 17, 17, 17, 18, 18, 18, 18, 18, 18,
    19, 19, 19, 19, 19, 19, 20, 20, 20, 20,
    20, 20, 21, 21, 21, 21, 21, 21, 22, 22,
    22, 22, 22, 22, 23, 23, 23, 23, 23, 23,
    24, 24, 24, 24, 24, 24, 25, 25, 25, 25,
    25, 25, 26, 26, 26, 26, 26, 26, 27, 27
  };
  AFFECT_DATA af;
  int dam;

  level = UMIN (level,(int) (sizeof (dam_each) / sizeof (dam_each[0]) - 1));
  level = UMAX (0, level);
  dam = number_range (dam_each[level] / 2, dam_each[level] * 2);
  if (!saves_spell (level, victim, DAM_COLD))
    {
      act ("$n turns blue and shivers.", victim, NULL, NULL, TO_ROOM);
      af.where = TO_AFFECTS;
      af.type = sn;
      af.level = level;
      af.duration = 6;
      af.location = APPLY_STR;
      af.modifier = -1;
      af.bitvector = 0;
      affect_join (victim, &af);
    }
  else
    {
      dam /= 2;
    }
  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }

  damage_old (ch, victim, dam, sn, DAM_COLD, TRUE);
  return;
}



void
spell_colour_spray (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  static const sh_int dam_each[] = {
    0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    30, 32, 35, 37, 40, 42, 45, 47, 50, 52,
    55, 55, 55, 55, 55, 55, 56, 56, 57, 57,
    58, 58, 58, 58, 59, 59, 60, 60, 61, 61,
    61, 61, 62, 62, 63, 63, 64, 64, 64, 64,
    65, 65, 66, 66, 67, 67, 67, 67, 68, 68,
    69, 69, 70, 70, 70, 70, 71, 71, 72, 72,
    73, 73, 73, 73, 74, 74, 75, 75, 76, 76,
    76, 76, 77, 77, 78, 78, 79, 79, 79, 79
  };
  int dam;

  level = UMIN (level, (int) (sizeof (dam_each) / sizeof (dam_each[0]) - 1));
  level = UMAX (0, level);
  dam = number_range (dam_each[level] / 2, dam_each[level] * 2);
  if (saves_spell (level, victim, DAM_LIGHT))
    dam /= 2;
  else
    spell_blindness (skill_lookup ("blindness"),
		     level / 2, ch, (void *) victim, TARGET_CHAR);

  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }
  damage_old (ch, victim, dam, sn, DAM_LIGHT, TRUE);
  return;
}



void
spell_continual_light (int sn, int level, CHAR_DATA * ch, void *vo,
		       int target)
{
  OBJ_DATA *light;

  if (target_name[0] != '\0')	/* do a glow on some object */
    {
      light = get_obj_carry (ch, target_name);

      if (light == NULL)
	{
	  send_to_char ("You don't see that here.\n\r", ch);
	  return;
	}

      if (IS_OBJ_STAT (light, ITEM_GLOW))
	{
	  act ("$p is already glowing.", ch, light, NULL, TO_CHAR);
	  return;
	}

      SET_BIT (light->extra_flags, ITEM_GLOW);
      act ("$p glows with a white light.", ch, light, NULL, TO_ALL);
      return;
    }

  light = create_object (get_obj_index (OBJ_VNUM_LIGHT_BALL), 0);
  obj_to_room (light, ch->in_room);
  act ("$n twiddles $s thumbs and $p appears.", ch, light, NULL, TO_ROOM);
  act ("You twiddle your thumbs and $p appears.", ch, light, NULL, TO_CHAR);
  return;
}



void
spell_control_weather (int sn, int level, CHAR_DATA * ch, void *vo,
		       int target)
{
  if (!str_cmp (target_name, "better"))
    weather_info.change += dice (level / 3, 4);
  else if (!str_cmp (target_name, "worse"))
    weather_info.change -= dice (level / 3, 4);
  else
    send_to_char ("Do you want it to get better or worse?\n\r", ch);

  send_to_char ("Ok.\n\r", ch);
  return;
}



void
spell_create_food (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *mushroom;

  mushroom = create_object (get_obj_index (OBJ_VNUM_MUSHROOM), 0);
  mushroom->value[0] = level / 2;
  mushroom->value[1] = level;
  obj_to_room (mushroom, ch->in_room);
  act ("$p suddenly appears.", ch, mushroom, NULL, TO_ROOM);
  act ("$p suddenly appears.", ch, mushroom, NULL, TO_CHAR);
  return;
}

void
spell_create_rose (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *rose;
  rose = create_object (get_obj_index (OBJ_VNUM_ROSE), 0);
  act ("$n has created a beautiful `Rred rose`x.", ch, rose, NULL, TO_ROOM);
  send_to_char ("You create a beautiful `Rred rose`x.\n\r", ch);
  obj_to_char (rose, ch);
  return;
}

void
spell_create_spring (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *spring;

  spring = create_object (get_obj_index (OBJ_VNUM_SPRING), 0);
  spring->timer = level;
  obj_to_room (spring, ch->in_room);
  act ("$p flows from the ground.", ch, spring, NULL, TO_ROOM);
  act ("$p flows from the ground.", ch, spring, NULL, TO_CHAR);
  return;
}



void
spell_create_water (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  int water;

  if (obj->item_type != ITEM_DRINK_CON)
    {
      send_to_char ("It is unable to hold water.\n\r", ch);
      return;
    }

  if (obj->value[2] != LIQ_WATER && obj->value[1] != 0)
    {
      send_to_char ("It contains some other liquid.\n\r", ch);
      return;
    }

  water = UMIN (level * (weather_info.sky >= SKY_RAINING ? 4 : 2),
		obj->value[0] - obj->value[1]);

  if (water > 0)
    {
      obj->value[2] = LIQ_WATER;
      obj->value[1] += water;
      if (!is_name ("water", obj->name))
	{
	  char buf[MAX_STRING_LENGTH];

	  snprintf (buf, sizeof (buf), "%s water", obj->name);
	  free_string (obj->name);
	  obj->name = str_dup (buf);
	}
      act ("$p is filled.", ch, obj, NULL, TO_CHAR);
    }

  return;
}



void
spell_cure_blindness (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if (!is_affected (victim, gsn_blindness))
    {
      if (victim == ch)
	send_to_char ("You aren't blind.\n\r", ch);
      else
	act ("$N doesn't appear to be blinded.", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (check_dispel (level, victim, gsn_blindness))
    {
      send_to_char ("Your vision returns!\n\r", victim);
      act ("$n is no longer blinded.", victim, NULL, NULL, TO_ROOM);
    }
  else
    send_to_char ("Spell failed.\n\r", ch);
}



void
spell_cure_critical (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int heal;

  heal = dice (3, 8) + level - 6;
  victim->hit = UMIN (victim->hit + heal, victim->max_hit);
  update_pos (victim);
  send_to_char ("You feel better!\n\r", victim);
  if (ch != victim)
    send_to_char ("Ok.\n\r", ch);
  return;
}

/* RT added to cure plague */
void
spell_cure_disease (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if (!is_affected (victim, gsn_plague))
    {
      if (victim == ch)
	send_to_char ("You aren't ill.\n\r", ch);
      else
	act ("$N doesn't appear to be diseased.", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (check_dispel (level, victim, gsn_plague))
    {
      send_to_char ("Your sores vanish.\n\r", victim);
      act ("$n looks relieved as $s sores vanish.", victim, NULL, NULL,
	   TO_ROOM);
    }
  else
    send_to_char ("Spell failed.\n\r", ch);
}



void
spell_cure_light (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int heal;

  heal = dice (1, 8) + level / 3;
  victim->hit = UMIN (victim->hit + heal, victim->max_hit);
  update_pos (victim);
  send_to_char ("You feel better!\n\r", victim);
  if (ch != victim)
    send_to_char ("Ok.\n\r", ch);
  return;
}



void
spell_cure_poison (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if (!is_affected (victim, gsn_poison))
    {
      if (victim == ch)
	send_to_char ("You aren't poisoned.\n\r", ch);
      else
	act ("$N doesn't appear to be poisoned.", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (check_dispel (level, victim, gsn_poison))
    {
      send_to_char ("A warm feeling runs through your body.\n\r", victim);
      act ("$n looks much better.", victim, NULL, NULL, TO_ROOM);
    }
  else
    send_to_char ("Spell failed.\n\r", ch);
}

void
spell_cure_serious (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int heal;

  heal = dice (2, 8) + level / 2;
  victim->hit = UMIN (victim->hit + heal, victim->max_hit);
  update_pos (victim);
  send_to_char ("You feel better!\n\r", victim);
  if (ch != victim)
    send_to_char ("Ok.\n\r", ch);
  return;
}



void
spell_curse (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  AFFECT_DATA af;

  /* deal with the object case first */
  if (target == TARGET_OBJ)
    {
      obj = (OBJ_DATA *) vo;
      if (IS_OBJ_STAT (obj, ITEM_EVIL))
	{
	  act ("$p is already filled with evil.", ch, obj, NULL, TO_CHAR);
	  return;
	}

      if (IS_OBJ_STAT (obj, ITEM_BLESS))
	{
	  AFFECT_DATA *paf;

	  paf = affect_find (obj->affected, skill_lookup ("bless"));
	  if (!saves_dispel (level, paf != NULL ? paf->level : obj->level, 0))
	    {
	      if (paf != NULL)
		affect_remove_obj (obj, paf);
	      act ("$p glows with a red aura.", ch, obj, NULL, TO_ALL);
	      REMOVE_BIT (obj->extra_flags, ITEM_BLESS);
	      return;
	    }
	  else
	    {
	      act ("The holy aura of $p is too powerful for you to overcome.",
		   ch, obj, NULL, TO_CHAR);
	      return;
	    }
	}

      af.where = TO_OBJECT;
      af.type = sn;
      af.level = level;
      af.duration = 2 * level;
      af.location = APPLY_SAVES;
      af.modifier = +1;
      af.bitvector = ITEM_EVIL;
      affect_to_obj (obj, &af);

      act ("$p glows with a malevolent aura.", ch, obj, NULL, TO_ALL);
      return;
    }

  /* character curses */
  victim = (CHAR_DATA *) vo;

  if (IS_AFFECTED (victim, AFF_CURSE)
      || saves_spell (level, victim, DAM_NEGATIVE))
    return;
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = 2 * level;
  af.location = APPLY_HITROLL;
  af.modifier = -1 * (level / 8);
  af.bitvector = AFF_CURSE;
  affect_to_char (victim, &af);

  af.location = APPLY_SAVING_SPELL;
  af.modifier = level / 8;
  affect_to_char (victim, &af);

  send_to_char ("You feel unclean.\n\r", victim);
  if (ch != victim)
    act ("$N looks very uncomfortable.", ch, NULL, victim, TO_CHAR);
  return;
}

/* RT replacement demonfire spell */

void
spell_demonfire (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if (!IS_NPC (ch) && !IS_EVIL (ch))
    {
      victim = ch;
      send_to_char ("The demons turn upon you!\n\r", ch);
    }

  ch->alignment = UMAX (-1000, ch->alignment - 50);
  if (ch->pet != NULL)
    ch->pet->alignment = ch->alignment;

  if (victim != ch)
    {
      act ("$n calls forth the demons of Hell upon $N!",
	   ch, NULL, victim, TO_ROOM);
      act ("$n has assailed you with the demons of Hell!",
	   ch, NULL, victim, TO_VICT);
      send_to_char ("You conjure forth the demons of hell!\n\r", ch);
    }
  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }
  dam = dice (level, 10);
  if (saves_spell (level, victim, DAM_NEGATIVE))
    dam /= 2;
  damage_old (ch, victim, dam, sn, DAM_NEGATIVE, TRUE);
  spell_curse (gsn_curse, 3 * level / 4, ch, (void *) victim, TARGET_CHAR);
}

void
spell_detect_evil (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_AFFECTED (victim, AFF_DETECT_EVIL))
    {
      if (victim == ch)
	send_to_char ("You can already sense evil.\n\r", ch);
      else
	act ("$N can already detect evil.", ch, NULL, victim, TO_CHAR);
      return;
    }
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_DETECT_EVIL;
  affect_to_char (victim, &af);
  send_to_char ("Your eyes tingle.\n\r", victim);
  if (ch != victim)
    send_to_char ("Ok.\n\r", ch);
  return;
}


void
spell_detect_good (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_AFFECTED (victim, AFF_DETECT_GOOD))
    {
      if (victim == ch)
	send_to_char ("You can already sense good.\n\r", ch);
      else
	act ("$N can already detect good.", ch, NULL, victim, TO_CHAR);
      return;
    }
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_DETECT_GOOD;
  affect_to_char (victim, &af);
  send_to_char ("Your eyes tingle.\n\r", victim);
  if (ch != victim)
    send_to_char ("Ok.\n\r", ch);
  return;
}



void
spell_detect_hidden (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_AFFECTED (victim, AFF_DETECT_HIDDEN))
    {
      if (victim == ch)
	send_to_char ("You are already as alert as you can be. \n\r", ch);
      else
	act ("$N can already sense hidden lifeforms.", ch, NULL, victim,
	     TO_CHAR);
      return;
    }
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level;
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = AFF_DETECT_HIDDEN;
  affect_to_char (victim, &af);
  send_to_char ("Your awareness improves.\n\r", victim);
  if (ch != victim)
    send_to_char ("Ok.\n\r", ch);
  return;
}



void
spell_detect_invis (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_AFFECTED (victim, AFF_DETECT_INVIS))
    {
      if (victim == ch)
	send_to_char ("You can already see invisible.\n\r", ch);
      else
	act ("$N can already see invisible things.", ch, NULL, victim,
	     TO_CHAR);
      return;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_DETECT_INVIS;
  affect_to_char (victim, &af);
  send_to_char ("Your eyes tingle.\n\r", victim);
  if (ch != victim)
    send_to_char ("Ok.\n\r", ch);
  return;
}



void
spell_detect_magic (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_AFFECTED (victim, AFF_DETECT_MAGIC))
    {
      if (victim == ch)
	send_to_char ("You can already sense magical auras.\n\r", ch);
      else
	act ("$N can already detect magic.", ch, NULL, victim, TO_CHAR);
      return;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level;
  af.modifier = 0;
  af.location = APPLY_NONE;
  af.bitvector = AFF_DETECT_MAGIC;
  affect_to_char (victim, &af);
  send_to_char ("Your eyes tingle.\n\r", victim);
  if (ch != victim)
    send_to_char ("Ok.\n\r", ch);
  return;
}



void
spell_detect_poison (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;

  if (obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD)
    {
      if (obj->value[3] != 0)
	send_to_char ("You smell poisonous fumes.\n\r", ch);
      else
	send_to_char ("It looks delicious.\n\r", ch);
    }
  else
    {
      send_to_char ("It doesn't look poisoned.\n\r", ch);
    }

  return;
}



void
spell_dispel_evil (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if (!IS_NPC (ch) && IS_EVIL (ch))
    victim = ch;

  if (IS_GOOD (victim))
    {
      act ("Thoth protects $N.", ch, NULL, victim, TO_ROOM);
      return;
    }

  if (IS_NEUTRAL (victim))
    {
      act ("$N does not seem to be affected.", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (victim->hit > (ch->level * 4))
    dam = dice (level, 4);
  else
    dam = UMAX (victim->hit, dice (level, 4));
  if (saves_spell (level, victim, DAM_HOLY))
    dam /= 2;
  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }
  damage_old (ch, victim, dam, sn, DAM_HOLY, TRUE);
  return;
}


void
spell_dispel_good (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if (!IS_NPC (ch) && IS_GOOD (ch))
    victim = ch;

  if (IS_EVIL (victim))
    {
      act ("Belan protects $N.", ch, NULL, victim, TO_ROOM);
      return;
    }

  if (IS_NEUTRAL (victim))
    {
      act ("$N does not seem to be affected.", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (victim->hit > (ch->level * 4))
    dam = dice (level, 4);
  else
    dam = UMAX (victim->hit, dice (level, 4));
  if (saves_spell (level, victim, DAM_NEGATIVE))
    dam /= 2;
  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }
  damage_old (ch, victim, dam, sn, DAM_NEGATIVE, TRUE);
  return;
}


/* modified for enhanced use */

void
spell_dispel_magic (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  bool found = FALSE;

  if (saves_spell (level, victim, DAM_OTHER))
    {
      send_to_char ("You feel a brief tingling sensation.\n\r", victim);
      send_to_char ("You failed.\n\r", ch);
      return;
    }

  /* begin running through the spells */

  if (check_dispel (level, victim, skill_lookup ("armor")))
    found = TRUE;

  if (check_dispel (level, victim, skill_lookup ("bless")))
    found = TRUE;

  if (check_dispel (level, victim, skill_lookup ("blindness")))
    {
      found = TRUE;
      act ("$n is no longer blinded.", victim, NULL, NULL, TO_ROOM);
    }

  if (check_dispel (level, victim, skill_lookup ("calm")))
    {
      found = TRUE;
      act ("$n no longer looks so peaceful...", victim, NULL, NULL, TO_ROOM);
    }

  if (check_dispel (level, victim, skill_lookup ("change sex")))
    {
      found = TRUE;
      act ("$n looks more like $mself again.", victim, NULL, NULL, TO_ROOM);
    }

  if (check_dispel (level, victim, skill_lookup ("charm person")))
    {
      found = TRUE;
      act ("$n regains $s free will.", victim, NULL, NULL, TO_ROOM);
    }

  if (check_dispel (level, victim, skill_lookup ("chill touch")))
    {
      found = TRUE;
      act ("$n looks warmer.", victim, NULL, NULL, TO_ROOM);
    }

  if (check_dispel (level, victim, skill_lookup ("curse")))
    found = TRUE;

  if (check_dispel (level, victim, skill_lookup ("detect evil")))
    found = TRUE;

  if (check_dispel (level, victim, skill_lookup ("detect good")))
    found = TRUE;

  if (check_dispel (level, victim, skill_lookup ("detect hidden")))
    found = TRUE;

  if (check_dispel (level, victim, skill_lookup ("detect invis")))
    found = TRUE;

  found = TRUE;

  if (check_dispel (level, victim, skill_lookup ("detect hidden")))
    found = TRUE;

  if (check_dispel (level, victim, skill_lookup ("detect magic")))
    found = TRUE;

  if (check_dispel (level, victim, skill_lookup ("faerie fire")))
    {
      act ("$n's outline fades.", victim, NULL, NULL, TO_ROOM);
      found = TRUE;
    }

  if (check_dispel (level, victim, skill_lookup ("fly")))
    {
      act ("$n falls to the ground!", victim, NULL, NULL, TO_ROOM);
      found = TRUE;
    }

  if (check_dispel (level, victim, skill_lookup ("frenzy")))
    {
      act ("$n no longer looks so wild.", victim, NULL, NULL, TO_ROOM);;
      found = TRUE;
    }

  if (check_dispel (level, victim, skill_lookup ("giant strength")))
    {
      act ("$n no longer looks so mighty.", victim, NULL, NULL, TO_ROOM);
      found = TRUE;
    }

  if (check_dispel (level, victim, skill_lookup ("haste")))
    {
      act ("$n is no longer moving so quickly.", victim, NULL, NULL, TO_ROOM);
      found = TRUE;
    }

  if (check_dispel (level, victim, skill_lookup ("infravision")))
    found = TRUE;

  if (check_dispel (level, victim, skill_lookup ("invis")))
    {
      act ("$n fades into existance.", victim, NULL, NULL, TO_ROOM);
      found = TRUE;
    }

  if (check_dispel (level, victim, skill_lookup ("mass invis")))
    {
      act ("$n fades into existance.", victim, NULL, NULL, TO_ROOM);
      found = TRUE;
    }

  if (check_dispel (level, victim, skill_lookup ("pass door")))
    found = TRUE;


  if (check_dispel (level, victim, skill_lookup ("protection evil")))
    found = TRUE;

  if (check_dispel (level, victim, skill_lookup ("protection good")))
    found = TRUE;

  if (check_dispel (level, victim, skill_lookup ("sanctuary")))
    {
      act ("The white aura around $n's body vanishes.",
	   victim, NULL, NULL, TO_ROOM);
      found = TRUE;
    }

  if (IS_SHIELDED (victim, SHD_SANCTUARY)
      && !saves_dispel (level, victim->level, -1)
      && !is_affected (victim, skill_lookup ("sanctuary")))
    {
      REMOVE_BIT (victim->shielded_by, SHD_SANCTUARY);
      act ("The white aura around $n's body vanishes.",
	   victim, NULL, NULL, TO_ROOM);
      found = TRUE;
    }

  if (IS_SHIELDED (victim, SHD_SANCTUARY)
      && !saves_dispel (level, victim->level, -1)
      && !is_affected (victim, skill_lookup ("sanctuary")))
    {
      REMOVE_BIT (victim->shielded_by, SHD_SANCTUARY);
      act ("The white aura around $n's body vanishes.",
	   victim, NULL, NULL, TO_ROOM);
      found = TRUE;
    }

  if (check_dispel (level, victim, skill_lookup ("shield")))
    {
      act ("The shield protecting $n vanishes.", victim, NULL, NULL, TO_ROOM);
      found = TRUE;
    }

  if (check_dispel (level, victim, skill_lookup ("sleep")))
    found = TRUE;

  if (check_dispel (level, victim, skill_lookup ("slow")))
    {
      act ("$n is no longer moving so slowly.", victim, NULL, NULL, TO_ROOM);
      found = TRUE;
    }

  if (check_dispel (level, victim, skill_lookup ("stone skin")))
    {
      act ("$n's skin regains its normal texture.", victim, NULL, NULL,
	   TO_ROOM);
      found = TRUE;
    }

  if (check_dispel (level, victim, skill_lookup ("weaken")))
    {
      act ("$n looks stronger.", victim, NULL, NULL, TO_ROOM);
      found = TRUE;
    }

  if (found)
    send_to_char ("Ok.\n\r", ch);
  else
    send_to_char ("Spell failed.\n\r", ch);
  return;
}

void
spell_earthquake (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;

  send_to_char ("The earth trembles beneath your feet!\n\r", ch);
  act ("$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM);

  for (vch = char_list; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next;
      if (vch->in_room == NULL)
	continue;
      if (vch->in_room == ch->in_room)
	{
	  if (vch != ch && !is_safe_spell (ch, vch, TRUE))
	    {
	      if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (vch)))
		{
		  ch->attacker = TRUE;
		  vch->attacker = FALSE;
		}
	      if (IS_AFFECTED (vch, AFF_FLYING))
		damage_old (ch, vch, 0, sn, DAM_BASH, TRUE);
	      else
		damage_old (ch, vch, level + dice (2, 8), sn, DAM_BASH, TRUE);
	    }
	  continue;
	}

      if (vch->in_room->area == ch->in_room->area)
	send_to_char ("The earth trembles and shivers.\n\r", vch);
    }

  return;
}

void
spell_enchant_armor (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  AFFECT_DATA *paf;
  int result, fail;
  int ac_bonus, added;
  bool ac_found = FALSE;

  if (obj->item_type != ITEM_ARMOR)
    {
      send_to_char ("That isn't an armor.\n\r", ch);
      return;
    }

  if (obj->wear_loc != -1)
    {
      send_to_char ("The item must be carried to be enchanted.\n\r", ch);
      return;
    }

  /* this means they have no bonus */
  ac_bonus = 0;
  fail = 25;			/* base 25% chance of failure */

  /* find the bonuses */

  if (!obj->enchanted)
    for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
      {
	if (paf->location == APPLY_AC)
	  {
	    ac_bonus = paf->modifier;
	    ac_found = TRUE;
	    fail += 5 * (ac_bonus * ac_bonus);
	  }

	else			/* things get a little harder */
	  fail += 20;
      }

  for (paf = obj->affected; paf != NULL; paf = paf->next)
    {
      if (paf->location == APPLY_AC)
	{
	  ac_bonus = paf->modifier;
	  ac_found = TRUE;
	  fail += 5 * (ac_bonus * ac_bonus);
	}

      else			/* things get a little harder */
	fail += 20;
    }

  /* apply other modifiers */
  fail -= level;

  if (IS_OBJ_STAT (obj, ITEM_BLESS))
    fail -= 15;
  if (IS_OBJ_STAT (obj, ITEM_GLOW))
    fail -= 5;

  fail = URANGE (5, fail, 85);

  result = number_percent ();

  /* the moment of truth */
  if (result < (fail / 5))	/* item destroyed */
    {
      act ("$p flares blindingly... and evaporates!", ch, obj, NULL, TO_CHAR);
      act ("$p flares blindingly... and evaporates!", ch, obj, NULL, TO_ROOM);
      extract_obj (obj);
      return;
    }

  if (result < (fail / 3))	/* item disenchanted */
    {
      AFFECT_DATA *paf_next;

      act ("$p glows brightly, then fades...oops.", ch, obj, NULL, TO_CHAR);
      act ("$p glows brightly, then fades.", ch, obj, NULL, TO_ROOM);
      obj->enchanted = TRUE;

      /* remove all affects */
      for (paf = obj->affected; paf != NULL; paf = paf_next)
	{
	  paf_next = paf->next;
	  free_affect (paf);
	}
      obj->affected = NULL;

      /* clear all flags */
      obj->extra_flags = 0;
      return;
    }

  if (result <= fail)		/* failed, no bad result */
    {
      send_to_char ("Nothing seemed to happen.\n\r", ch);
      return;
    }

  /* okay, move all the old flags into new vectors if we have to */
  if (!obj->enchanted)
    {
      AFFECT_DATA *af_new;
      obj->enchanted = TRUE;

      for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
	{
	  af_new = new_affect ();

	  af_new->next = obj->affected;
	  obj->affected = af_new;

	  af_new->where = paf->where;
	  af_new->type = UMAX (0, paf->type);
	  af_new->level = paf->level;
	  af_new->duration = paf->duration;
	  af_new->location = paf->location;
	  af_new->modifier = paf->modifier;
	  af_new->bitvector = paf->bitvector;
	}
    }

  if (result <= (90 - level / 5))	/* success! */
    {
      act ("$p shimmers with a gold aura.", ch, obj, NULL, TO_CHAR);
      act ("$p shimmers with a gold aura.", ch, obj, NULL, TO_ROOM);
      SET_BIT (obj->extra_flags, ITEM_MAGIC);
      added = -1;
    }

  else				/* exceptional enchant */
    {
      act ("$p glows a brillant gold!", ch, obj, NULL, TO_CHAR);
      act ("$p glows a brillant gold!", ch, obj, NULL, TO_ROOM);
      SET_BIT (obj->extra_flags, ITEM_MAGIC);
      SET_BIT (obj->extra_flags, ITEM_GLOW);
      added = -2;
    }

  /* now add the enchantments */

  if (obj->level < LEVEL_HERO)
    obj->level = UMIN (LEVEL_HERO - 1, obj->level + 1);

  if (ac_found)
    {
      for (paf = obj->affected; paf != NULL; paf = paf->next)
	{
	  if (paf->location == APPLY_AC)
	    {
	      paf->type = sn;
	      paf->modifier += added;
	      paf->level = UMAX (paf->level, level);
	    }
	}
    }
  else				/* add a new affect */
    {
      paf = new_affect ();

      paf->where = TO_OBJECT;
      paf->type = sn;
      paf->level = level;
      paf->duration = -1;
      paf->location = APPLY_AC;
      paf->modifier = added;
      paf->bitvector = 0;
      paf->next = obj->affected;
      obj->affected = paf;
    }

}




void
spell_enchant_weapon (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  AFFECT_DATA *paf;
  int result, fail;
  int hit_bonus, dam_bonus, added;
  bool hit_found = FALSE, dam_found = FALSE;

  if (obj->item_type != ITEM_WEAPON)
    {
      send_to_char ("That isn't a weapon.\n\r", ch);
      return;
    }

  if (obj->wear_loc != -1)
    {
      send_to_char ("The item must be carried to be enchanted.\n\r", ch);
      return;
    }

  /* this means they have no bonus */
  hit_bonus = 0;
  dam_bonus = 0;
  fail = 25;			/* base 25% chance of failure */

  /* find the bonuses */

  if (!obj->enchanted)
    for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
      {
	if (paf->location == APPLY_HITROLL)
	  {
	    hit_bonus = paf->modifier;
	    hit_found = TRUE;
	    fail += 2 * (hit_bonus * hit_bonus);
	  }

	else if (paf->location == APPLY_DAMROLL)
	  {
	    dam_bonus = paf->modifier;
	    dam_found = TRUE;
	    fail += 2 * (dam_bonus * dam_bonus);
	  }

	else			/* things get a little harder */
	  fail += 25;
      }

  for (paf = obj->affected; paf != NULL; paf = paf->next)
    {
      if (paf->location == APPLY_HITROLL)
	{
	  hit_bonus = paf->modifier;
	  hit_found = TRUE;
	  fail += 2 * (hit_bonus * hit_bonus);
	}

      else if (paf->location == APPLY_DAMROLL)
	{
	  dam_bonus = paf->modifier;
	  dam_found = TRUE;
	  fail += 2 * (dam_bonus * dam_bonus);
	}

      else			/* things get a little harder */
	fail += 25;
    }

  /* apply other modifiers */
  fail -= 3 * level / 2;

  if (IS_OBJ_STAT (obj, ITEM_BLESS))
    fail -= 15;
  if (IS_OBJ_STAT (obj, ITEM_GLOW))
    fail -= 5;

  fail = URANGE (5, fail, 95);

  result = number_percent ();

  /* the moment of truth */
  if (result < (fail / 5))	/* item destroyed */
    {
      act ("$p shivers violently and explodes!", ch, obj, NULL, TO_CHAR);
      act ("$p shivers violently and explodeds!", ch, obj, NULL, TO_ROOM);
      extract_obj (obj);
      return;
    }

  if (result < (fail / 2))	/* item disenchanted */
    {
      AFFECT_DATA *paf_next;

      act ("$p glows brightly, then fades...oops.", ch, obj, NULL, TO_CHAR);
      act ("$p glows brightly, then fades.", ch, obj, NULL, TO_ROOM);
      obj->enchanted = TRUE;

      /* remove all affects */
      for (paf = obj->affected; paf != NULL; paf = paf_next)
	{
	  paf_next = paf->next;
	  free_affect (paf);
	}
      obj->affected = NULL;

      /* clear all flags */
      obj->extra_flags = 0;
      return;
    }

  if (result <= fail)		/* failed, no bad result */
    {
      send_to_char ("Nothing seemed to happen.\n\r", ch);
      return;
    }

  /* okay, move all the old flags into new vectors if we have to */
  if (!obj->enchanted)
    {
      AFFECT_DATA *af_new;
      obj->enchanted = TRUE;

      for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
	{
	  af_new = new_affect ();

	  af_new->next = obj->affected;
	  obj->affected = af_new;

	  af_new->where = paf->where;
	  af_new->type = UMAX (0, paf->type);
	  af_new->level = paf->level;
	  af_new->duration = paf->duration;
	  af_new->location = paf->location;
	  af_new->modifier = paf->modifier;
	  af_new->bitvector = paf->bitvector;
	}
    }

  if (result <= (100 - level / 5))	/* success! */
    {
      act ("$p glows blue.", ch, obj, NULL, TO_CHAR);
      act ("$p glows blue.", ch, obj, NULL, TO_ROOM);
      SET_BIT (obj->extra_flags, ITEM_MAGIC);
      added = 1;
    }

  else				/* exceptional enchant */
    {
      act ("$p glows a brillant blue!", ch, obj, NULL, TO_CHAR);
      act ("$p glows a brillant blue!", ch, obj, NULL, TO_ROOM);
      SET_BIT (obj->extra_flags, ITEM_MAGIC);
      SET_BIT (obj->extra_flags, ITEM_GLOW);
      added = 2;
    }

  /* now add the enchantments */

  if (obj->level < LEVEL_HERO - 1)
    obj->level = UMIN (LEVEL_HERO - 1, obj->level + 1);

  if (dam_found)
    {
      for (paf = obj->affected; paf != NULL; paf = paf->next)
	{
	  if (paf->location == APPLY_DAMROLL)
	    {
	      paf->type = sn;
	      paf->modifier += added;
	      paf->level = UMAX (paf->level, level);
	      if (paf->modifier > 4)
		SET_BIT (obj->extra_flags, ITEM_HUM);
	    }
	}
    }
  else				/* add a new affect */
    {
      paf = new_affect ();

      paf->where = TO_OBJECT;
      paf->type = sn;
      paf->level = level;
      paf->duration = -1;
      paf->location = APPLY_DAMROLL;
      paf->modifier = added;
      paf->bitvector = 0;
      paf->next = obj->affected;
      obj->affected = paf;
    }

  if (hit_found)
    {
      for (paf = obj->affected; paf != NULL; paf = paf->next)
	{
	  if (paf->location == APPLY_HITROLL)
	    {
	      paf->type = sn;
	      paf->modifier += added;
	      paf->level = UMAX (paf->level, level);
	      if (paf->modifier > 4)
		SET_BIT (obj->extra_flags, ITEM_HUM);
	    }
	}
    }
  else				/* add a new affect */
    {
      paf = new_affect ();

      paf->type = sn;
      paf->level = level;
      paf->duration = -1;
      paf->location = APPLY_HITROLL;
      paf->modifier = added;
      paf->bitvector = 0;
      paf->next = obj->affected;
      obj->affected = paf;
    }

}



/*
 * Drain XP, MANA, HP.
 * Caster gains HP.
 */
void
spell_energy_drain (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if (victim != ch)
    {
      ch->alignment = UMAX (-1000, ch->alignment - 50);
      if (ch->pet != NULL)
	ch->pet->alignment = ch->alignment;
    }

  if (saves_spell (level, victim, DAM_NEGATIVE))
    {
      send_to_char ("You feel a momentary chill.\n\r", victim);
      return;
    }


  if (victim->level <= 2)
    {
      dam = ch->hit + 1;
    }
  else
    {
      gain_exp (victim, 0 - number_range (level / 2, 3 * level / 2));
      victim->mana /= 2;
      victim->move /= 2;
      dam = dice (1, level);
      ch->hit += dam;
    }
  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }

  send_to_char ("You feel your life slipping away!\n\r", victim);
  send_to_char ("Wow....what a rush!\n\r", ch);
  damage_old (ch, victim, dam, sn, DAM_NEGATIVE, TRUE);

  return;
}



void
spell_fireball (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  static const sh_int dam_each[] = {
    0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 30, 32,
    35, 37, 40, 42, 45, 47, 50, 52, 55, 57,
    60, 62, 65, 67, 70, 72, 75, 77, 80, 81,
    82, 83, 84, 85, 86, 87, 88, 89, 90, 91,
    92, 93, 94, 95, 96, 97, 98, 99, 100, 101,
    102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
    112, 113, 114, 115, 116, 117, 118, 119, 120, 121,
    122, 123, 124, 125, 126, 127, 128, 129, 130, 131
  };
  int dam;

  level = UMIN (level, (int) (sizeof (dam_each) / sizeof (dam_each[0]) - 1));
  level = UMAX (0, level);
  dam = number_range (dam_each[level] / 2, dam_each[level] * 2);
  if (saves_spell (level, victim, DAM_FIRE))
    dam /= 2;
  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }
  damage_old (ch, victim, dam, sn, DAM_FIRE, TRUE);
  return;
}


void
spell_fireproof (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  AFFECT_DATA af;

  if (IS_OBJ_STAT (obj, ITEM_BURN_PROOF))
    {
      act ("$p is already protected from burning.", ch, obj, NULL, TO_CHAR);
      return;
    }

  af.where = TO_OBJECT;
  af.type = sn;
  af.level = level;
  af.duration = number_fuzzy (level / 4);
  af.location = APPLY_NONE;
  af.modifier = 0;
  af.bitvector = ITEM_BURN_PROOF;

  affect_to_obj (obj, &af);

  act ("You protect $p from fire.", ch, obj, NULL, TO_CHAR);
  act ("$p is surrounded by a protective aura.", ch, obj, NULL, TO_ROOM);
}



void
spell_flamestrike (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }
  dam = dice (6 + level / 2, 8);
  if (saves_spell (level, victim, DAM_FIRE))
    dam /= 2;
  damage_old (ch, victim, dam, sn, DAM_FIRE, TRUE);
  return;
}



void
spell_faerie_fire (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_AFFECTED (victim, AFF_FAERIE_FIRE))
    return;
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level;
  af.location = APPLY_AC;
  af.modifier = 2 * level;
  af.bitvector = AFF_FAERIE_FIRE;
  affect_to_char (victim, &af);
  send_to_char ("You are surrounded by a pink outline.\n\r", victim);
  act ("$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM);
  return;
}



void
spell_faerie_fog (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *ich;

  act ("$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM);
  send_to_char ("You conjure a cloud of purple smoke.\n\r", ch);

  for (ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room)
    {
      if (ich->invis_level > 0)
	continue;

      if (ich == ch || saves_spell (level, ich, DAM_OTHER))
	continue;

      affect_strip (ich, gsn_invis);
      affect_strip (ich, gsn_mass_invis);
      affect_strip (ich, gsn_sneak);
      REMOVE_BIT (ich->affected_by, AFF_HIDE);
      REMOVE_BIT (ich->shielded_by, SHD_INVISIBLE);
      REMOVE_BIT (ich->affected_by, AFF_SNEAK);
      act ("$n is revealed!", ich, NULL, NULL, TO_ROOM);
      send_to_char ("You are revealed!\n\r", ich);
    }

  return;
}

void
spell_floating_disc (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  OBJ_DATA *disc, *floating;

  floating = get_eq_char (ch, WEAR_FLOAT);
  if (floating != NULL && IS_OBJ_STAT (floating, ITEM_NOREMOVE))
    {
      act ("You can't remove $p.", ch, floating, NULL, TO_CHAR);
      return;
    }

  disc = create_object (get_obj_index (OBJ_VNUM_DISC), 0);
  disc->value[0] = ch->level * 10;	/* 10 pounds per level capacity */
  disc->value[3] = ch->level * 5;	/* 5 pounds per level max per item */
  disc->timer = ch->level * 2 - number_range (0, level / 2);

  act ("$n has created a floating black disc.", ch, NULL, NULL, TO_ROOM);
  send_to_char ("You create a floating disc.\n\r", ch);
  obj_to_char (disc, ch);
  wear_obj (ch, disc, TRUE);
  return;
}


void
spell_fly (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (IS_AFFECTED (victim, AFF_FLYING))
    {
      if (victim == ch)
	send_to_char ("You are already airborne.\n\r", ch);
      else
	act ("$N doesn't need your help to fly.", ch, NULL, victim, TO_CHAR);
      return;
    }
  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level + 3;
  af.location = 0;
  af.modifier = 0;
  af.bitvector = AFF_FLYING;
  affect_to_char (victim, &af);
  send_to_char ("Your feet rise off the ground.\n\r", victim);
  act ("$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM);
  return;
}

/* RT clerical berserking spell */

void
spell_frenzy (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected (victim, sn) || IS_AFFECTED (victim, AFF_BERSERK))
    {
      if (victim == ch)
	send_to_char ("You are already in a frenzy.\n\r", ch);
      else
	act ("$N is already in a frenzy.", ch, NULL, victim, TO_CHAR);
      return;
    }

  if (is_affected (victim, skill_lookup ("calm")))
    {
      if (victim == ch)
	send_to_char ("Why don't you just relax for a while?\n\r", ch);
      else
	act ("$N doesn't look like $e wants to fight anymore.",
	     ch, NULL, victim, TO_CHAR);
      return;
    }

  if ((IS_GOOD (ch) && !IS_GOOD (victim)) ||
      (IS_NEUTRAL (ch) && !IS_NEUTRAL (victim)) ||
      (IS_EVIL (ch) && !IS_EVIL (victim)))
    {
      act ("Your god doesn't seem to like $N", ch, NULL, victim, TO_CHAR);
      return;
    }

  af.where = TO_AFFECTS;
  af.type = sn;
  af.level = level;
  af.duration = level / 3;
  af.modifier = level / 6;
  af.bitvector = 0;

  af.location = APPLY_HITROLL;
  affect_to_char (victim, &af);

  af.location = APPLY_DAMROLL;
  affect_to_char (victim, &af);

  af.modifier = 10 * (level / 12);
  af.location = APPLY_AC;
  affect_to_char (victim, &af);

  send_to_char ("You are filled with holy wrath!\n\r", victim);
  act ("$n gets a wild look in $s eyes!", victim, NULL, NULL, TO_ROOM);
}
