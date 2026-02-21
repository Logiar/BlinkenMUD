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
/*
 * NPC spells.
 */
void
spell_acid_breath (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam, hp_dam, dice_dam, hpch;

  act ("$n spits acid at $N.", ch, NULL, victim, TO_NOTVICT);
  act ("$n spits a stream of corrosive acid at you.", ch, NULL, victim,
       TO_VICT);
  act ("You spit acid at $N.", ch, NULL, victim, TO_CHAR);

  hpch = UMAX (12, ch->hit);
  hp_dam = number_range (hpch / 11 + 1, hpch / 6);
  dice_dam = dice (level, 16);

  dam = UMAX (hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);

  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }
  if (saves_spell (level, victim, DAM_ACID))
    {
      acid_effect (victim, level / 2, dam / 4, TARGET_CHAR);
      damage_old (ch, victim, dam / 2, sn, DAM_ACID, TRUE);
    }
  else
    {
      acid_effect (victim, level, dam, TARGET_CHAR);
      damage_old (ch, victim, dam, sn, DAM_ACID, TRUE);
    }
}



void
spell_fire_breath (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  CHAR_DATA *vch, *vch_next;
  int dam, hp_dam, dice_dam;
  int hpch;

  act ("$n breathes forth a cone of fire.", ch, NULL, victim, TO_NOTVICT);
  act ("$n breathes a cone of hot fire over you!", ch, NULL, victim, TO_VICT);
  act ("You breath forth a cone of fire.", ch, NULL, NULL, TO_CHAR);

  hpch = UMAX (10, ch->hit);
  hp_dam = number_range (hpch / 9 + 1, hpch / 5);
  dice_dam = dice (level, 20);

  dam = UMAX (hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);
  fire_effect (victim->in_room, level, dam / 2, TARGET_ROOM);

  for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next_in_room;

      if (is_safe_spell (ch, vch, TRUE)
	  || (IS_NPC (vch) && IS_NPC (ch)
	      && (ch->fighting != vch || vch->fighting != ch)))
	continue;

      if (vch == victim)	/* full damage */
	{
	  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
	    {
	      ch->attacker = TRUE;
	      victim->attacker = FALSE;
	    }
	  if (saves_spell (level, vch, DAM_FIRE))
	    {
	      fire_effect (vch, level / 2, dam / 4, TARGET_CHAR);
	      damage_old (ch, vch, dam / 2, sn, DAM_FIRE, TRUE);
	    }
	  else
	    {
	      fire_effect (vch, level, dam, TARGET_CHAR);
	      damage_old (ch, vch, dam, sn, DAM_FIRE, TRUE);
	    }
	}
      else			/* partial damage */
	{
	  if (saves_spell (level - 2, vch, DAM_FIRE))
	    {
	      fire_effect (vch, level / 4, dam / 8, TARGET_CHAR);
	      damage_old (ch, vch, dam / 4, sn, DAM_FIRE, TRUE);
	    }
	  else
	    {
	      fire_effect (vch, level / 2, dam / 4, TARGET_CHAR);
	      damage_old (ch, vch, dam / 2, sn, DAM_FIRE, TRUE);
	    }
	}
    }
}

void
spell_frost_breath (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  CHAR_DATA *vch, *vch_next;
  int dam, hp_dam, dice_dam, hpch;

  act ("$n breathes out a freezing cone of frost!", ch, NULL, victim,
       TO_NOTVICT);
  act ("$n breathes a freezing cone of frost over you!", ch, NULL, victim,
       TO_VICT);
  act ("You breath out a cone of frost.", ch, NULL, NULL, TO_CHAR);

  hpch = UMAX (12, ch->hit);
  hp_dam = number_range (hpch / 11 + 1, hpch / 6);
  dice_dam = dice (level, 16);

  dam = UMAX (hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);
  cold_effect (victim->in_room, level, dam / 2, TARGET_ROOM);

  for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next_in_room;

      if (is_safe_spell (ch, vch, TRUE)
	  || (IS_NPC (vch) && IS_NPC (ch)
	      && (ch->fighting != vch || vch->fighting != ch)))
	continue;

      if (vch == victim)	/* full damage */
	{
	  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
	    {
	      ch->attacker = TRUE;
	      victim->attacker = FALSE;
	    }
	  if (saves_spell (level, vch, DAM_COLD))
	    {
	      cold_effect (vch, level / 2, dam / 4, TARGET_CHAR);
	      damage_old (ch, vch, dam / 2, sn, DAM_COLD, TRUE);
	    }
	  else
	    {
	      cold_effect (vch, level, dam, TARGET_CHAR);
	      damage_old (ch, vch, dam, sn, DAM_COLD, TRUE);
	    }
	}
      else
	{
	  if (saves_spell (level - 2, vch, DAM_COLD))
	    {
	      cold_effect (vch, level / 4, dam / 8, TARGET_CHAR);
	      damage_old (ch, vch, dam / 4, sn, DAM_COLD, TRUE);
	    }
	  else
	    {
	      cold_effect (vch, level / 2, dam / 4, TARGET_CHAR);
	      damage_old (ch, vch, dam / 2, sn, DAM_COLD, TRUE);
	    }
	}
    }
}


void
spell_gas_breath (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam, hp_dam, dice_dam, hpch;

  act ("$n breathes out a cloud of poisonous gas!", ch, NULL, NULL, TO_ROOM);
  act ("You breath out a cloud of poisonous gas.", ch, NULL, NULL, TO_CHAR);

  hpch = UMAX (16, ch->hit);
  hp_dam = number_range (hpch / 15 + 1, 8);
  dice_dam = dice (level, 12);

  dam = UMAX (hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);
  poison_effect (ch->in_room, level, dam, TARGET_ROOM);

  for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
    {
      vch_next = vch->next_in_room;

      if (is_safe_spell (ch, vch, TRUE)
	  || (IS_NPC (ch) && IS_NPC (vch)
	      && (ch->fighting == vch || vch->fighting == ch)))
	continue;

      if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (vch)))
	{
	  ch->attacker = TRUE;
	  vch->attacker = FALSE;
	}
      if (saves_spell (level, vch, DAM_POISON))
	{
	  poison_effect (vch, level / 2, dam / 4, TARGET_CHAR);
	  damage_old (ch, vch, dam / 2, sn, DAM_POISON, TRUE);
	}
      else
	{
	  poison_effect (vch, level, dam, TARGET_CHAR);
	  damage_old (ch, vch, dam, sn, DAM_POISON, TRUE);
	}
    }
}

void
spell_lightning_breath (int sn, int level, CHAR_DATA * ch, void *vo,
			int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam, hp_dam, dice_dam, hpch;

  act ("$n breathes a bolt of lightning at $N.", ch, NULL, victim,
       TO_NOTVICT);
  act ("$n breathes a bolt of lightning at you!", ch, NULL, victim, TO_VICT);
  act ("You breathe a bolt of lightning at $N.", ch, NULL, victim, TO_CHAR);

  hpch = UMAX (10, ch->hit);
  hp_dam = number_range (hpch / 9 + 1, hpch / 5);
  dice_dam = dice (level, 20);

  dam = UMAX (hp_dam + dice_dam / 10, dice_dam + hp_dam / 10);

  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }
  if (saves_spell (level, victim, DAM_LIGHTNING))
    {
      shock_effect (victim, level / 2, dam / 4, TARGET_CHAR);
      damage_old (ch, victim, dam / 2, sn, DAM_LIGHTNING, TRUE);
    }
  else
    {
      shock_effect (victim, level, dam, TARGET_CHAR);
      damage_old (ch, victim, dam, sn, DAM_LIGHTNING, TRUE);
    }
}

/*
 * Spells for mega1.are from Glop/Erkenbrand.
 */
void
spell_general_purpose (int sn, int level, CHAR_DATA * ch, void *vo,
		       int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  dam = number_range (25, 100);
  if (saves_spell (level, victim, DAM_PIERCE))
    dam /= 2;
  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }
  damage_old (ch, victim, dam, sn, DAM_PIERCE, TRUE);
  return;
}

void
spell_high_explosive (int sn, int level, CHAR_DATA * ch, void *vo, int target)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  dam = number_range (30, 120);
  if (saves_spell (level, victim, DAM_PIERCE))
    dam /= 2;
  if ((ch->fighting == NULL) && (!IS_NPC (ch)) && (!IS_NPC (victim)))
    {
      ch->attacker = TRUE;
      victim->attacker = FALSE;
    }
  damage_old (ch, victim, dam, sn, DAM_PIERCE, TRUE);
  return;
}
