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
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "tables.h"


/* command procedures needed */
DECLARE_DO_FUN (do_backstab);
DECLARE_DO_FUN (do_emote);
DECLARE_DO_FUN (do_berserk);
DECLARE_DO_FUN (do_bash);
DECLARE_DO_FUN (do_trip);
DECLARE_DO_FUN (do_dirt);
DECLARE_DO_FUN (do_flee);
DECLARE_DO_FUN (do_kick);
DECLARE_DO_FUN (do_disarm);
DECLARE_DO_FUN (do_get);
DECLARE_DO_FUN (do_recall);
DECLARE_DO_FUN (do_yell);
DECLARE_DO_FUN (do_sacrifice);
DECLARE_DO_FUN (do_circle);
DECLARE_DO_FUN (do_feed);
DECLARE_DO_FUN (do_gouge);
DECLARE_DO_FUN (do_vdpi);
DECLARE_DO_FUN (do_vdtr);
DECLARE_DO_FUN (do_vdth);
DECLARE_DO_FUN (do_look);


/*
 * Local functions.
 */
void check_assist (CHAR_DATA * ch, CHAR_DATA * victim);
bool check_dodge (CHAR_DATA * ch, CHAR_DATA * victim);
bool check_parry (CHAR_DATA * ch, CHAR_DATA * victim);
bool check_shield_block (CHAR_DATA * ch, CHAR_DATA * victim);
void dam_message (CHAR_DATA * ch, CHAR_DATA * victim, int dam,
			int dt, bool immune);
void death_cry (CHAR_DATA * ch);
void group_gain (CHAR_DATA * ch, CHAR_DATA * victim);
int xp_compute (CHAR_DATA * gch, CHAR_DATA * victim, int total_levels);
bool is_safe (CHAR_DATA * ch, CHAR_DATA * victim);
bool is_safe_mock (CHAR_DATA * ch, CHAR_DATA * victim);
bool is_voodood (CHAR_DATA * ch, CHAR_DATA * victim);
void make_corpse (CHAR_DATA * ch, CHAR_DATA * killer);
void one_hit
(CHAR_DATA * ch, CHAR_DATA * victim, int dt, bool secondary);
void one_hit_mock
(CHAR_DATA * ch, CHAR_DATA * victim, int dt, bool secondary);
void mob_hit (CHAR_DATA * ch, CHAR_DATA * victim, int dt);
void raw_kill (CHAR_DATA * victim, CHAR_DATA * killer);
void set_fighting (CHAR_DATA * ch, CHAR_DATA * victim);
void disarm (CHAR_DATA * ch, CHAR_DATA * victim);



/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void
violence_update (void)
{
  CHAR_DATA *ch;
  //    CHAR_DATA *ch_next;
  CHAR_DATA *victim;

  for (ch = char_list; ch != NULL; ch = ch->next)
    {
      //ch_next = ch->next;

      if ((victim = ch->fighting) == NULL || ch->in_room == NULL)
	continue;

      if (IS_AWAKE (ch) && ch->in_room == victim->in_room)
	multi_hit (ch, victim, TYPE_UNDEFINED);
      else
	stop_fighting (ch, FALSE);

      if ((victim = ch->fighting) == NULL)
	continue;

      /*
       * Fun for the whole family!
       */
      check_assist (ch, victim);

      if (IS_NPC (ch))
	{
	  if (HAS_TRIGGER (ch, TRIG_FIGHT))
	    mp_percent_trigger (ch, victim, NULL, NULL, TRIG_FIGHT);
	  if (HAS_TRIGGER (ch, TRIG_HPCNT))
	    mp_hprct_trigger (ch, victim);
	}
    }

  return;
}

/* for auto assisting */
void
check_assist (CHAR_DATA * ch, CHAR_DATA * victim)
{
  CHAR_DATA *rch, *rch_next;

  for (rch = ch->in_room->people; rch != NULL; rch = rch_next)
    {
      rch_next = rch->next_in_room;

      if (IS_AWAKE (rch) && rch->fighting == NULL)
	{

	  /* quick check for ASSIST_PLAYER */
	  if (!IS_NPC (ch) && IS_NPC (rch)
	      && IS_SET (rch->off_flags, ASSIST_PLAYERS)
	      && rch->level + 6 > victim->level)
	    {
	      do_emote (rch, "`Rscreams and attacks!`x");
	      multi_hit (rch, victim, TYPE_UNDEFINED);
	      continue;
	    }

	  /* PCs next */
	  if (!IS_NPC (ch) || IS_AFFECTED (ch, AFF_CHARM))
	    {
	      if (((!IS_NPC (rch) && IS_SET (rch->act, PLR_AUTOASSIST))
		   || IS_AFFECTED (rch, AFF_CHARM))
		  && is_same_group (ch, rch) && !is_safe (rch, victim))
		multi_hit (rch, victim, TYPE_UNDEFINED);

	      continue;
	    }

	  /* now check the NPC cases */

	  if (IS_NPC (ch) && !IS_AFFECTED (ch, AFF_CHARM))

	    {
	      if ((IS_NPC (rch) && IS_SET (rch->off_flags, ASSIST_ALL))
		  || (IS_NPC (rch) && rch->group && rch->group == ch->group)
		  || (IS_NPC (rch) && rch->race == ch->race
		      && IS_SET (rch->off_flags, ASSIST_RACE))
		  || (IS_NPC (rch) && IS_SET (rch->off_flags, ASSIST_ALIGN)
		      && ((IS_GOOD (rch) && IS_GOOD (ch))
			  || (IS_EVIL (rch) && IS_EVIL (ch))
			  || (IS_NEUTRAL (rch) && IS_NEUTRAL (ch))))
		  || (rch->pIndexData == ch->pIndexData
		      && IS_SET (rch->off_flags, ASSIST_VNUM)))

		{
		  CHAR_DATA *vch;
		  CHAR_DATA *target;
		  int number;

		  if (number_bits (1) == 0)
		    continue;

		  target = NULL;
		  number = 0;
		  for (vch = ch->in_room->people; vch; vch = vch->next)
		    {
		      if (can_see (rch, vch)
			  && is_same_group (vch, victim)
			  && number_range (0, number) == 0)
			{
			  target = vch;
			  number++;
			}
		    }

		  if (target != NULL)
		    {
		      do_emote (rch, "`Rscreams and attacks!`x");
		      multi_hit (rch, target, TYPE_UNDEFINED);
		    }
		}
	    }
	}
    }
}


/*
 * Do one group of attacks.
 */
void
multi_hit (CHAR_DATA * ch, CHAR_DATA * victim, int dt)
{
  int chance;

  /* decrement the wait */
  if (ch->desc == NULL)
    ch->wait = UMAX (0, ch->wait - PULSE_VIOLENCE);

  if (ch->desc == NULL)
    ch->daze = UMAX (0, ch->daze - PULSE_VIOLENCE);


  /* no attacks for stunnies -- just a check */
  if (ch->position < POS_RESTING)
    return;

  if (ch->stunned)
    {
      ch->stunned--;
      if (!ch->stunned)
	{
	  send_to_char ("You regain your equilibrium.\n\r", ch);
	  act ("$n regains $m equilibrium.", ch, NULL, NULL, TO_ROOM);
	}
      return;
    }

  if (IS_NPC (ch))
    {
      mob_hit (ch, victim, dt);
      return;
    }

  one_hit (ch, victim, dt, FALSE);

  if (get_eq_char (ch, WEAR_SECONDARY))
    {
      chance = (get_skill (ch, gsn_dual_wield) / 3) * 2;
      chance += 33;
      if (number_percent () < chance)
	{
	  one_hit (ch, victim, dt, TRUE);
	  if (get_skill (ch, gsn_dual_wield) != 0 && (!IS_NPC (ch)
						      && ch->level >=
						      skill_table
						      [gsn_dual_wield].
						      skill_level[ch->class]))
	    {
	      check_improve (ch, gsn_dual_wield, TRUE, 1);
	    }
	}
      if (ch->fighting != victim)
	return;
    }

  if (ch->fighting != victim)
    return;

  if (IS_AFFECTED (ch, AFF_HASTE))
    one_hit (ch, victim, dt, FALSE);

  if (ch->fighting != victim || dt == gsn_backstab || dt == gsn_circle)
    return;

  chance = get_skill (ch, gsn_second_attack) / 2;

  if (IS_AFFECTED (ch, AFF_SLOW))
    chance /= 2;

  if (number_percent () < chance)
    {
      one_hit (ch, victim, dt, FALSE);
      check_improve (ch, gsn_second_attack, TRUE, 5);
      if (ch->fighting != victim)
	return;
    }
  else
    {
      return;
    }

  chance = get_skill (ch, gsn_third_attack) / 2;

  if (IS_AFFECTED (ch, AFF_SLOW))
    chance /= 2;

  if (number_percent () < chance)
    {
      one_hit (ch, victim, dt, FALSE);
      check_improve (ch, gsn_third_attack, TRUE, 6);
      if (ch->fighting != victim)
	return;
    }
  else
    {
      return;
    }

  chance = get_skill (ch, gsn_fourth_attack) / 2;

  if (IS_AFFECTED (ch, AFF_SLOW))
    chance /= 3;

  if (number_percent () < chance)
    {
      one_hit (ch, victim, dt, FALSE);
      check_improve (ch, gsn_fourth_attack, TRUE, 6);
      if (ch->fighting != victim)
	return;
    }
  else
    {
      return;
    }

  chance = get_skill (ch, gsn_fifth_attack) / 2;

  if (IS_AFFECTED (ch, AFF_SLOW))
    chance = 0;

  if (number_percent () < chance)
    {
      one_hit (ch, victim, dt, FALSE);
      check_improve (ch, gsn_fifth_attack, TRUE, 6);
      if (ch->fighting != victim)
	return;
    }

  return;
}

/* procedure for all mobile attacks */
void
mob_hit (CHAR_DATA * ch, CHAR_DATA * victim, int dt)
{
  int chance, number;
  CHAR_DATA *vch, *vch_next;

  one_hit (ch, victim, dt, FALSE);

  if (ch->fighting != victim)
    return;

  if (ch->stunned)
    return;

  /* Area attack -- BALLS nasty! */

  if (IS_SET (ch->off_flags, OFF_AREA_ATTACK))
    {
      for (vch = ch->in_room->people; vch != NULL; vch = vch_next)
	{
	  vch_next = vch->next;
	  if ((vch != victim && vch->fighting == ch))
	    one_hit (ch, vch, dt, FALSE);
	}
    }

  if (ch->fighting != victim)
    return;

  if (get_eq_char (ch, WEAR_SECONDARY))
    {
      chance = (get_skill (ch, gsn_dual_wield) / 3) * 2;
      chance += 33;
      if (number_percent () < chance)
	{
	  one_hit (ch, victim, dt, TRUE);
	}
      if (ch->fighting != victim)
	return;
    }

  if (IS_AFFECTED (ch, AFF_HASTE)
      || (IS_SET (ch->off_flags, OFF_FAST) && !IS_AFFECTED (ch, AFF_SLOW)))
    one_hit (ch, victim, dt, FALSE);

  if (ch->fighting != victim || dt == gsn_backstab || dt == gsn_circle)
    return;

  chance = get_skill (ch, gsn_second_attack) / 2;

  if (IS_AFFECTED (ch, AFF_SLOW) && !IS_SET (ch->off_flags, OFF_FAST))
    chance /= 2;

  if (number_percent () < chance)
    {
      one_hit (ch, victim, dt, FALSE);
      if (ch->fighting != victim)
	return;
      chance = get_skill (ch, gsn_third_attack) / 2;

      if (IS_AFFECTED (ch, AFF_SLOW) && !IS_SET (ch->off_flags, OFF_FAST))
	chance /= 2;

      if (number_percent () < chance)
	{
	  one_hit (ch, victim, dt, FALSE);
	  if (ch->fighting != victim)
	    return;

	  chance = get_skill (ch, gsn_fourth_attack) / 2;

	  if (IS_AFFECTED (ch, AFF_SLOW) && !IS_SET (ch->off_flags, OFF_FAST))
	    chance /= 3;

	  if (number_percent () < chance)
	    {
	      one_hit (ch, victim, dt, FALSE);
	      if (ch->fighting != victim)
		return;

	      chance = get_skill (ch, gsn_fifth_attack) / 2;

	      if (IS_AFFECTED (ch, AFF_SLOW)
		  && !IS_SET (ch->off_flags, OFF_FAST))
		chance = 0;

	      if (number_percent () < chance)
		{
		  one_hit (ch, victim, dt, FALSE);
		  if (ch->fighting != victim)
		    return;
		}
	    }
	}
    }

  /* oh boy!  Fun stuff! */

  if (ch->wait > 0)
    return;

  number = number_range (0, 2);

  if (number == 1 && IS_SET (ch->act, ACT_MAGE))
    {
      /*  { mob_cast_mage(ch,victim); return; } */ ;
    }

  if (number == 2 && IS_SET (ch->act, ACT_CLERIC))
    {
      /* { mob_cast_cleric(ch,victim); return; } */ ;
    }

  /* now for the skills */

  number = number_range (0, 8);

  switch (number)
    {
    case (0):
      if (IS_SET (ch->off_flags, OFF_BASH))
	do_bash (ch, "");
      break;

    case (1):
      if (IS_SET (ch->off_flags, OFF_BERSERK)
	  && !IS_AFFECTED (ch, AFF_BERSERK))
	do_berserk (ch, "");
      break;


    case (2):
      if (IS_SET (ch->off_flags, OFF_DISARM)
	  || (get_weapon_sn (ch) != gsn_hand_to_hand
	      && (IS_SET (ch->act, ACT_WARRIOR)
		  || IS_SET (ch->act, ACT_VAMPIRE)
		  || IS_SET (ch->act, ACT_THIEF))))
	do_disarm (ch, "");
      break;

    case (3):
      if (IS_SET (ch->off_flags, OFF_KICK))
	do_kick (ch, "");
      break;

    case (4):
      if (IS_SET (ch->off_flags, OFF_KICK_DIRT))
	do_dirt (ch, "");
      break;

    case (5):
      if (IS_SET (ch->off_flags, OFF_TAIL))
	{
	  /* do_tail(ch,"") */ ;
	}
      break;

    case (6):
      if (IS_SET (ch->off_flags, OFF_TRIP))
	do_trip (ch, "");
      break;

    case (7):
      if (IS_SET (ch->off_flags, OFF_CRUSH))
	{
	  /* do_crush(ch,"") */ ;
	}
      break;
    case (8):
      if (IS_SET (ch->off_flags, OFF_BACKSTAB))
	{
	  do_backstab (ch, "");
	}
    }
}


/*
 * Hit one guy once.
 */
void
one_hit (CHAR_DATA * ch, CHAR_DATA * victim, int dt, bool secondary)
{
  OBJ_DATA *wield;
  int victim_ac;
  int thac0;
  int thac0_00;
  int thac0_32;
  int dam;
  int diceroll;
  int sn, skill;
  int dam_type;
  bool result;

  sn = -1;


  /* just in case */
  if (victim == ch || ch == NULL || victim == NULL)
    return;

  /*
   * Can't beat a dead char!
   * Guard against weird room-leavings.
   */
  if (victim->position == POS_DEAD || ch->in_room != victim->in_room)
    return;

  /*
   * Figure out the type of damage message.
   * if secondary == true, use the second weapon.
   */
  if (!secondary)
    wield = get_eq_char (ch, WEAR_WIELD);
  else
    wield = get_eq_char (ch, WEAR_SECONDARY);
  if (dt == TYPE_UNDEFINED)
    {
      dt = TYPE_HIT;
      if (wield != NULL && wield->item_type == ITEM_WEAPON)
	dt += wield->value[3];
      else
	dt += ch->dam_type;
    }

  if (dt < TYPE_HIT)
    if (wield != NULL)
      dam_type = attack_table[wield->value[3]].damage;
    else
      dam_type = attack_table[ch->dam_type].damage;
  else
    dam_type = attack_table[dt - TYPE_HIT].damage;

  if (dam_type == -1)
    dam_type = DAM_BASH;

  /* get the weapon skill */
  sn = get_weapon_sn (ch);
  skill = 20 + get_weapon_skill (ch, sn);

  /*
   * Calculate to-hit-armor-class-0 versus armor.
   */
  if (IS_NPC (ch))
    {
      thac0_00 = 20;
      thac0_32 = -4;		/* as good as a thief */
      if (IS_SET (ch->act, ACT_VAMPIRE))
	thac0_32 = -30;
      else if (IS_SET (ch->act, ACT_DRUID))
	thac0_32 = 0;
      else if (IS_SET (ch->act, ACT_RANGER))
	thac0_32 = -4;
      else if (IS_SET (ch->act, ACT_WARRIOR))
	thac0_32 = -10;
      else if (IS_SET (ch->act, ACT_THIEF))
	thac0_32 = -4;
      else if (IS_SET (ch->act, ACT_CLERIC))
	thac0_32 = 2;
      else if (IS_SET (ch->act, ACT_MAGE))
	thac0_32 = 6;
    }
  else
    {
      thac0_00 = class_table[ch->class].thac0_00;
      thac0_32 = class_table[ch->class].thac0_32;
    }
  thac0 = interpolate (ch->level, thac0_00, thac0_32);

  if (thac0 < 0)
    thac0 = thac0 / 2;

  if (thac0 < -5)
    thac0 = -5 + (thac0 + 5) / 2;

  thac0 -= GET_HITROLL (ch) * skill / 100;
  thac0 += 5 * (100 - skill) / 100;

  if (dt == gsn_backstab)
    thac0 -= 10 * (100 - get_skill (ch, gsn_backstab));

  if (dt == gsn_circle)
    thac0 -= 10 * (100 - get_skill (ch, gsn_circle));

  switch (dam_type)
    {
    case (DAM_PIERCE):
      victim_ac = GET_AC (victim, AC_PIERCE) / 10;
      break;
    case (DAM_BASH):
      victim_ac = GET_AC (victim, AC_BASH) / 10;
      break;
    case (DAM_SLASH):
      victim_ac = GET_AC (victim, AC_SLASH) / 10;
      break;
    default:
      victim_ac = GET_AC (victim, AC_EXOTIC) / 10;
      break;
    };

  if (victim_ac < -15)
    victim_ac = (victim_ac + 15) / 5 - 15;

  if (!can_see (ch, victim))
    victim_ac -= 4;

  if (victim->position < POS_FIGHTING)
    victim_ac += 4;

  if (victim->position < POS_RESTING)
    victim_ac += 6;

  /*
   * The moment of excitement!
   */
  while ((diceroll = number_bits (5)) >= 20)
    ;

  if (diceroll == 0 || (diceroll != 19 && diceroll < thac0 - victim_ac))
    {
      /* Miss. */
      damage (ch, victim, 0, dt, dam_type, TRUE);
      tail_chain ();
      return;
    }

  /*
   * Hit.
   * Calc damage.
   */
  if (IS_NPC (ch) && (!ch->pIndexData->new_format || wield == NULL))
    if (!ch->pIndexData->new_format)
      {
	dam = number_range (ch->level / 2, ch->level * 3 / 2);
	if (wield != NULL)
	  dam += dam / 2;
      }
    else
      dam = dice (ch->damage[DICE_NUMBER], ch->damage[DICE_TYPE]);

  else
    {
      if (sn != -1)
	check_improve (ch, sn, TRUE, 5);
      if (wield != NULL)
	{
	  if (wield->clan)
	    {
	      dam = dice (ch->level / 3, 3) * skill / 100;
	    }
	  else
	    {
	      if (wield->pIndexData->new_format)
		dam = dice (wield->value[1], wield->value[2]) * skill / 100;
	      else
		dam = number_range (wield->value[1] * skill / 100,
				    wield->value[2] * skill / 100);
	    }

	  if (get_eq_char (ch, WEAR_SHIELD) == NULL)	/* no shield = more */
	    dam = dam * 11 / 10;

	  /* sharpness! */
	  if (IS_WEAPON_STAT (wield, WEAPON_SHARP))
	    {
	      int percent;

	      if ((percent = number_percent ()) <= (skill / 8))
		dam = 2 * dam + (dam * 2 * percent / 100);
	    }
	}
      else
	dam =
	  number_range (1 + 4 * skill / 100, 2 * ch->level / 3 * skill / 100);
    }

  /*
   * Bonuses.
   */
  if (get_skill (ch, gsn_enhanced_damage) > 0)
    {
      diceroll = number_percent ();
      if (diceroll <= get_skill (ch, gsn_enhanced_damage))
	{
	  check_improve (ch, gsn_enhanced_damage, TRUE, 6);
	  dam += 2 * (dam * diceroll / 300);
	}
    }

  if (!IS_AWAKE (victim))
    dam *= 2;
  else if (victim->position < POS_FIGHTING)
    dam = dam * 3 / 2;

  if (dt == gsn_backstab && wield != NULL)
    {
      if (wield->value[0] != 2)
	dam *= 2 + (ch->level / 10);
      else
	dam *= 2 + (ch->level / 8);
    }
  if (dt == gsn_circle && wield != NULL)
    {
      if (wield->value[0] != 2)
	dam *= 2 + (ch->level / 15);
      else
	dam *= 2 + (ch->level / 12);
    }
  dam += GET_DAMROLL (ch) * UMIN (100, skill) / 100;

  if (dam <= 0)
    dam = 1;

  result = damage (ch, victim, dam, dt, dam_type, TRUE);

  /* but do we have a funky weapon? */
  if (result && wield != NULL)
    {
      int dam;

      if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_POISON))
	{
	  int level;
	  AFFECT_DATA *poison, af;

	  if ((poison = affect_find (wield->affected, gsn_poison)) == NULL)
	    level = wield->level;
	  else
	    level = poison->level;

	  if (!saves_spell (level / 2, victim, DAM_POISON))
	    {
	      send_to_char
		("`cYou feel `ypoison`c coursing through your veins.`x",
		 victim);
	      act ("$n is `ypoisoned`x by the venom on $p.", victim, wield,
		   NULL, TO_ROOM);

	      af.where = TO_AFFECTS;
	      af.type = gsn_poison;
	      af.level = level * 3 / 4;
	      af.duration = level / 2;
	      af.location = APPLY_STR;
	      af.modifier = -1;
	      af.bitvector = AFF_POISON;
	      affect_join (victim, &af);
	    }

	  /* weaken the poison if it's temporary */
	  if (poison != NULL)
	    {
	      poison->level = UMAX (0, poison->level - 2);
	      poison->duration = UMAX (0, poison->duration - 1);

	      if (poison->level == 0 || poison->duration == 0)
		act ("The `ypoison`x on $p has worn off.", ch, wield, NULL,
		     TO_CHAR);
	    }
	}


      if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_VAMPIRIC))
	{
	  dam = number_range (1, wield->level / 5 + 1);
	  act ("`k$p draws life from $n.`x", victim, wield, NULL, TO_ROOM);
	  act ("`iYou feel $p drawing your life away.`x",
	       victim, wield, NULL, TO_CHAR);
	  damage_old (ch, victim, dam, 0, DAM_NEGATIVE, FALSE);
	  ch->alignment = UMAX (-1000, ch->alignment - 1);
	  if (ch->pet != NULL)
	    ch->pet->alignment = ch->alignment;
	  ch->hit += dam / 2;
	}

      if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_FLAMING))
	{
	  dam = number_range (1, wield->level / 4 + 1);
	  act ("`k$n is `rburned`k by $p.`x", victim, wield, NULL, TO_ROOM);
	  act ("`i$p `rsears`i your flesh.`x", victim, wield, NULL, TO_CHAR);
	  fire_effect ((void *) victim, wield->level / 2, dam, TARGET_CHAR);
	  damage (ch, victim, dam, 0, DAM_FIRE, FALSE);
	}

      if (ch->fighting == victim && IS_WEAPON_STAT (wield, WEAPON_FROST))
	{
	  dam = number_range (1, wield->level / 6 + 2);
	  act ("`k$p `cfreezes`k $n.`x", victim, wield, NULL, TO_ROOM);
	  act ("`iThe `Ccold`i touch of $p surrounds you with `Cice.`x",
	       victim, wield, NULL, TO_CHAR);
	  cold_effect (victim, wield->level / 2, dam, TARGET_CHAR);
	  damage (ch, victim, dam, 0, DAM_COLD, FALSE);
	}

      if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_SHOCKING))
	{
	  dam = number_range(1,wield->level/5 + 2);
	  act("`k$n is struck by `Ylightning`k from $p.`x",victim,wield,NULL,TO_ROOM);
	  act("`iYou are `Yshocked`i by $p.`x",victim,wield,NULL,TO_CHAR);
	    shock_effect(victim,wield->level/2,dam,TARGET_CHAR);
	    damage(ch,victim,dam,0,DAM_LIGHTNING,FALSE);
	}
    }
    if (ch->fighting == victim && result)
	{
	  if (IS_SHIELDED(victim, SHD_ICE) && !IS_SHIELDED(ch, SHD_ICE))
	    {
	      dt = skill_lookup("iceshield");
	      dam = number_range(5, 15);
	      damage_old(victim, ch, dam, dt, DAM_COLD, TRUE);
	    }
	  if (IS_SHIELDED(victim, SHD_FIRE) && !IS_SHIELDED(ch, SHD_FIRE))
	    {
	      dt = skill_lookup("fireshield");
	      dam = number_range(10, 20);
	      damage_old(victim, ch, dam, dt, DAM_FIRE, TRUE);
	    }
	  if (IS_SHIELDED(victim, SHD_SHOCK) && (!IS_SHIELDED(ch, SHD_SHOCK)))
	    {
	      dt = skill_lookup("shockshield");
	      dam = number_range(15, 25);
	      damage_old(victim, ch, dam, dt, DAM_LIGHTNING, TRUE);
	    }
	}
    tail_chain( );
    return;
}

/*
 * Mock hit one guy once.
 */
void
one_hit_mock (CHAR_DATA * ch, CHAR_DATA * victim, int dt, bool secondary)
{
  OBJ_DATA *wield;
  int victim_ac;
  int thac0;
  int thac0_00;
  int thac0_32;
  int dam;
  int diceroll;
  int sn, skill;
  int dam_type;
  //    bool result;

  sn = -1;


  /* just in case */
  if (ch == NULL || victim == NULL)
    return;

  /*
   * Can't beat a dead char!
   * Guard against weird room-leavings.
   */
  if (victim->position == POS_DEAD || ch->in_room != victim->in_room)
    return;

  /*
   * Figure out the type of damage message.
   * if secondary == true, use the second weapon.
   */
  if (!secondary)
    wield = get_eq_char (ch, WEAR_WIELD);
  else
    wield = get_eq_char (ch, WEAR_SECONDARY);
  if (dt == TYPE_UNDEFINED)
    {
      dt = TYPE_HIT;
      if (wield != NULL && wield->item_type == ITEM_WEAPON)
	dt += wield->value[3];
      else
	dt += ch->dam_type;
    }

  if (dt < TYPE_HIT)
    if (wield != NULL)
      dam_type = attack_table[wield->value[3]].damage;
    else
      dam_type = attack_table[ch->dam_type].damage;
  else
    dam_type = attack_table[dt - TYPE_HIT].damage;

  if (dam_type == -1)
    dam_type = DAM_BASH;

  /* get the weapon skill */
  sn = get_weapon_sn (ch);
  skill = 20 + get_weapon_skill (ch, sn);

  /*
   * Calculate to-hit-armor-class-0 versus armor.
   */
  if (IS_NPC (ch))
    {
      thac0_00 = 20;
      thac0_32 = -4;		/* as good as a thief */
      if (IS_SET (ch->act, ACT_VAMPIRE))
	thac0_32 = -30;
      else if (IS_SET (ch->act, ACT_DRUID))
	thac0_32 = 0;
      else if (IS_SET (ch->act, ACT_RANGER))
	thac0_32 = -4;
      else if (IS_SET (ch->act, ACT_WARRIOR))
	thac0_32 = -10;
      else if (IS_SET (ch->act, ACT_THIEF))
	thac0_32 = -4;
      else if (IS_SET (ch->act, ACT_CLERIC))
	thac0_32 = 2;
      else if (IS_SET (ch->act, ACT_MAGE))
	thac0_32 = 6;
    }
  else
    {
      thac0_00 = class_table[ch->class].thac0_00;
      thac0_32 = class_table[ch->class].thac0_32;
    }
  thac0 = interpolate (ch->level, thac0_00, thac0_32);

  if (thac0 < 0)
    thac0 = thac0 / 2;

  if (thac0 < -5)
    thac0 = -5 + (thac0 + 5) / 2;

  thac0 -= GET_HITROLL (ch) * skill / 100;
  thac0 += 5 * (100 - skill) / 100;

  if (dt == gsn_backstab)
    thac0 -= 10 * (100 - get_skill (ch, gsn_backstab));

  if (dt == gsn_circle)
    thac0 -= 10 * (100 - get_skill (ch, gsn_circle));

  switch (dam_type)
    {
    case (DAM_PIERCE):
      victim_ac = GET_AC (victim, AC_PIERCE) / 10;
      break;
    case (DAM_BASH):
      victim_ac = GET_AC (victim, AC_BASH) / 10;
      break;
    case (DAM_SLASH):
      victim_ac = GET_AC (victim, AC_SLASH) / 10;
      break;
    default:
      victim_ac = GET_AC (victim, AC_EXOTIC) / 10;
      break;
    };

  if (victim_ac < -15)
    victim_ac = (victim_ac + 15) / 5 - 15;

  if (!can_see (ch, victim))
    victim_ac -= 4;

  if (victim->position < POS_FIGHTING)
    victim_ac += 4;

  if (victim->position < POS_RESTING)
    victim_ac += 6;

  /*
   * The moment of excitement!
   */
  while ((diceroll = number_bits (5)) >= 20)
    ;

  if (diceroll == 0 || (diceroll != 19 && diceroll < thac0 - victim_ac))
    {
      /* Miss. */
      damage_mock (ch, victim, 0, dt, dam_type, TRUE);
      tail_chain ();
      return;
    }

  /*
   * Hit.
   * Calc damage.
   */
  if (IS_NPC (ch) && (!ch->pIndexData->new_format || wield == NULL))
    if (!ch->pIndexData->new_format)
      {
	dam = number_range (ch->level / 2, ch->level * 3 / 2);
	if (wield != NULL)
	  dam += dam / 2;
      }
    else
      dam = dice (ch->damage[DICE_NUMBER], ch->damage[DICE_TYPE]);

  else
    {
      if (sn != -1)
	check_improve (ch, sn, TRUE, 5);
      if (wield != NULL)
	{
	  if (wield->clan)
	    {
	      dam = dice (ch->level / 3, 3) * skill / 100;
	    }
	  else
	    {
	      if (wield->pIndexData->new_format)
		dam = dice (wield->value[1], wield->value[2]) * skill / 100;
	      else
		dam = number_range (wield->value[1] * skill / 100,
				    wield->value[2] * skill / 100);
	    }

	  if (get_eq_char (ch, WEAR_SHIELD) == NULL)	/* no shield = more */
	    dam = dam * 11 / 10;

	  /* sharpness! */
	  if (IS_WEAPON_STAT (wield, WEAPON_SHARP))
	    {
	      int percent;

	      if ((percent = number_percent ()) <= (skill / 8))
		dam = 2 * dam + (dam * 2 * percent / 100);
	    }
	}
      else
	dam =
	  number_range (1 + 4 * skill / 100, 2 * ch->level / 3 * skill / 100);
    }

  /*
   * Bonuses.
   */
  if (get_skill (ch, gsn_enhanced_damage) > 0)
    {
      diceroll = number_percent ();
      if (diceroll <= get_skill (ch, gsn_enhanced_damage))
	{
	  check_improve (ch, gsn_enhanced_damage, TRUE, 6);
	  dam += 2 * (dam * diceroll / 300);
	}
    }

  if (!IS_AWAKE (victim))
    dam *= 2;
  else if (victim->position < POS_FIGHTING)
    dam = dam * 3 / 2;

  if (dt == gsn_backstab && wield != NULL)
    {
      if (wield->value[0] != 2)
	dam *= 2 + (ch->level / 10);
      else
	dam *= 2 + (ch->level / 8);
    }
  if (dt == gsn_circle && wield != NULL)
    {
      if (wield->value[0] != 2)
	dam *= 2 + (ch->level / 15);
      else
	dam *= 2 + (ch->level / 12);
    }
  dam += GET_DAMROLL (ch) * UMIN (100, skill) / 100;

  if (dam <= 0)
    dam = 1;

  //    result = damage_mock( ch, victim, dam, dt, dam_type, TRUE );
  damage_mock (ch, victim, dam, dt, dam_type, TRUE);

  tail_chain ();
  return;
}

/*
 * Inflict damage from a hit.
 */
bool
damage (CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, int dam_type,
	bool show)
{

  OBJ_DATA *corpse;
  bool immune;

  if (victim->position == POS_DEAD)
    return FALSE;

  /*
   * Stop up any residual loopholes.
   */
  if (dam > 1200 && dt >= TYPE_HIT && !IS_IMMORTAL (ch))
    {
      bug ("Damage: %d: more than 1200 points!", dam);
      dam = 1200;
      if (!IS_IMMORTAL (ch))
	{
	  OBJ_DATA *obj;
	  obj = get_eq_char (ch, WEAR_WIELD);
	  send_to_char ("`cYou `z`Breally`x`c shouldn't cheat.`x\n\r", ch);
	  if (obj != NULL)
	    extract_obj (obj);
	}

    }


  /* damage reduction */
  if (dam > 35)
    dam = (dam - 35) / 2 + 35;
  if (dam > 80)
    dam = (dam - 80) / 2 + 80;




  if (victim != ch)
    {
      /*
       * Certain attacks are forbidden.
       * Most other attacks are returned.
       */
      if (is_safe (ch, victim))
	return FALSE;

      if (victim->position > POS_STUNNED)
	{
	  if (victim->fighting == NULL)
	    {
	      set_fighting (victim, ch);
	      if (IS_NPC (victim) && HAS_TRIGGER (victim, TRIG_KILL))
		mp_percent_trigger (victim, ch, NULL, NULL, TRIG_KILL);
	    }
	  if (victim->timer <= 4)
	    victim->position = POS_FIGHTING;
	}

      if (victim->position > POS_STUNNED)
	{
	  if (ch->fighting == NULL)
	    set_fighting (ch, victim);

	  /*
	   * If victim is charmed, ch might attack victim's master.
	   taken out by Russ! */
/*
	    if ( IS_NPC(ch)
	    &&   IS_NPC(victim)
	    &&   IS_AFFECTED(victim, AFF_CHARM)
	    &&   victim->master != NULL
	    &&   victim->master->in_room == ch->in_room
	    &&   number_bits( 3 ) == 0 )
	    {
		stop_fighting( ch, FALSE );
		multi_hit( ch, victim->master, TYPE_UNDEFINED );
		return FALSE;
	    }
*/
	}

      /*
       * More charm stuff.
       */
      if (victim->master == ch)
	stop_follower (victim);
    }

  /*
   * Inviso attacks ... not.
   */
  if (IS_SHIELDED (ch, SHD_INVISIBLE))
    {
      affect_strip (ch, gsn_invis);
      affect_strip (ch, gsn_mass_invis);
      REMOVE_BIT (ch->shielded_by, SHD_INVISIBLE);
      act ("$n fades into existence.", ch, NULL, NULL, TO_ROOM);
    }

  /*
   * Damage modifiers.
   */

  if (dam > 1 && !IS_NPC (victim)
      && victim->pcdata->condition[COND_DRUNK] > 10)
    dam = 9 * dam / 10;

  if (dam > 1 && IS_SHIELDED (victim, SHD_SANCTUARY))
    dam /= 2;

  if (dam > 1 && ((IS_SHIELDED (victim, SHD_PROTECT_EVIL) && IS_EVIL (ch))
		  || (IS_SHIELDED (victim, SHD_PROTECT_GOOD)
		      && IS_GOOD (ch))))
    dam -= dam / 4;

  immune = FALSE;


  /*
   * Check for parry, and dodge.
   */
  if (dt >= TYPE_HIT && ch != victim)
    {
      if (check_parry (ch, victim))
	return FALSE;
      if (check_dodge (ch, victim))
	return FALSE;
      if (check_shield_block (ch, victim))
	return FALSE;

    }

  switch (check_immune (victim, dam_type))
    {
    case (IS_IMMUNE):
      immune = TRUE;
      dam = 0;
      break;
    case (IS_RESISTANT):
      dam -= dam / 3;
      break;
    case (IS_VULNERABLE):
      dam += dam / 2;
      break;
    }

  if (show)
    dam_message (ch, victim, dam, dt, immune);

  if (dam == 0)
    return FALSE;

  /*
   * Hurt the victim.
   * Inform the victim of his new state.
   */
  victim->hit -= dam;
  if (!IS_NPC (victim) && victim->level >= LEVEL_IMMORTAL && victim->hit < 1)
    victim->hit = 1;
  update_pos (victim);
  if (dt == gsn_feed)
    {
      ch->hit = UMIN (ch->hit + ((dam / 3) * 2), ch->max_hit);
      update_pos (ch);
    }

  switch (victim->position)
    {
    case POS_MORTAL:
      act ("`c$n is mortally wounded, and will die soon, if not aided.`x",
	   victim, NULL, NULL, TO_ROOM);
      send_to_char
	("`cYou are mortally wounded, and will die soon, if not aided.`x\n\r",
	 victim);
      break;

    case POS_INCAP:
      act ("`c$n is incapacitated and will slowly die, if not aided.`x",
	   victim, NULL, NULL, TO_ROOM);
      send_to_char
	("`cYou are incapacitated and will slowly `z`Rdie`x`c, if not aided.`x\n\r",
	 victim);
      break;

    case POS_STUNNED:
      act ("`c$n is stunned, but will probably recover.`x",
	   victim, NULL, NULL, TO_ROOM);
      send_to_char ("`cYou are stunned, but will probably recover.`x\n\r",
		    victim);
      break;

    case POS_DEAD:
      if ((IS_NPC (victim)) && (victim->die_descr[0] != '\0'))
	{
	  act ("`c$n $T`x", victim, 0, victim->die_descr, TO_ROOM);
	}
      else
	{
	  act ("`c$n is `CDEAD!!`x", victim, 0, 0, TO_ROOM);
	}
      send_to_char ("`cYou have been `RKILLED!!`x\n\r\n\r", victim);
      break;

    default:
      if (dam > victim->max_hit / 4)
	send_to_char ("`cThat really did `RHURT!`x\n\r", victim);
      if (victim->hit < victim->max_hit / 4)
	send_to_char ("`cYou sure are `z`RBLEEDING!`x\n\r", victim);
      break;
    }

  /*
   * Sleep spells and extremely wounded folks.
   */
  if (!IS_AWAKE (victim))
    stop_fighting (victim, FALSE);

  /*
   * Payoff for killing things.
   */
  if (victim->position == POS_DEAD)
    {
      group_gain (ch, victim);

      if (!IS_NPC (victim))
	{
	  snprintf (log_buf, MAX_STRING_LENGTH, "%s killed by %s at %d",
		   victim->name,
		   (IS_NPC (ch) ? ch->short_descr : ch->name),
		   ch->in_room->vnum);
	  log_string (log_buf);

	  /*
	   * Dying penalty:
	   * 2/3 way back to previous level.
	   */
	  if (victim->exp > exp_per_level (victim, victim->pcdata->points)
	      * victim->level)
	    gain_exp (victim,
		      (5 *
		       (exp_per_level (victim, victim->pcdata->points) *
			victim->level - victim->exp) / 6) + 50);
	}

      snprintf (log_buf, MAX_STRING_LENGTH, "%s got toasted by %s at %s [room %d]",
	       (IS_NPC (victim) ? victim->short_descr : victim->name),
	       (IS_NPC (ch) ? ch->short_descr : ch->name),
	       ch->in_room->name, ch->in_room->vnum);

      if (IS_NPC (victim))
	wiznet (log_buf, NULL, NULL, WIZ_MOBDEATHS, 0, 0);
      else
	wiznet (log_buf, NULL, NULL, WIZ_DEATHS, 0, 0);

      /*
       * Death trigger
       */
      if (IS_NPC (victim) && HAS_TRIGGER (victim, TRIG_DEATH))
	{
	  victim->position = POS_STANDING;
	  mp_percent_trigger (victim, ch, NULL, NULL, TRIG_DEATH);
	}

      raw_kill (victim, ch);
      /* dump the flags */
      if (ch != victim && !IS_NPC (ch) && (!is_same_clan (ch, victim)
					   || clan_table[victim->clan].
					   independent))
	{
	  if (IS_SET (victim->act, PLR_TWIT))
	    REMOVE_BIT (victim->act, PLR_TWIT);
	}

      /* RT new auto commands */

      if (!IS_NPC (ch) && IS_NPC (victim))
	{
	  OBJ_DATA *coins;

	  corpse = get_obj_list (ch, "corpse", ch->in_room->contents);

	  if (IS_SET (ch->act, PLR_AUTOLOOT) && corpse && corpse->contains)	/* exists and not empty */
	    do_get (ch, "all corpse");

	  if (IS_SET (ch->act, PLR_AUTOGOLD) && corpse && corpse->contains &&	/* exists and not empty */
	      !IS_SET (ch->act, PLR_AUTOLOOT))
	    if ((coins = get_obj_list (ch, "gcash", corpse->contains))
		!= NULL)
	      do_get (ch, "all.gcash corpse");

	  if (IS_SET (ch->act, PLR_AUTOSAC))
	    {
	      if (IS_SET (ch->act, PLR_AUTOLOOT) && corpse
		  && corpse->contains)
		return TRUE;	/* leave if corpse has treasure */
	      else
		do_sacrifice (ch, "corpse");
	    }
	}

      return TRUE;
    }

  if (victim == ch)
    return TRUE;

  /*
   * Take care of link dead people.
   */
  if (!IS_NPC (victim) && victim->desc == NULL)
    {
      if (number_range (0, victim->wait) == 0)
	{
	  do_recall (victim, "");
	  return TRUE;
	}
    }

  /*
   * Wimp out?
   */
  if (IS_NPC (victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
    {
      if ((IS_SET (victim->act, ACT_WIMPY) && number_bits (2) == 0
	   && victim->hit < victim->max_hit / 5)
	  || (IS_AFFECTED (victim, AFF_CHARM) && victim->master != NULL
	      && victim->master->in_room != victim->in_room))
	do_flee (victim, "");
    }

  if (!IS_NPC (victim)
      && victim->hit > 0
      && victim->hit <= victim->wimpy && victim->wait < PULSE_VIOLENCE / 2)
    do_flee (victim, "");

  tail_chain ();
  return TRUE;
}

/*
 * Show damage from a mock hit.
 */
bool
damage_mock (CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt,
	     int dam_type, bool show)
{
  long immdam;
  //bool immune;
  char buf1[256], buf2[256], buf3[256];
  const char *attack;

  if (victim->position == POS_DEAD)
    return FALSE;

  if (dam > 35)
    dam = (dam - 35) / 2 + 35;
  if (dam > 80)
    dam = (dam - 80) / 2 + 80;
  if (is_safe_mock (ch, victim))
    return FALSE;
  /*
   * Damage modifiers.
   */

  if (dam > 1 && !IS_NPC (victim)
      && victim->pcdata->condition[COND_DRUNK] > 10)
    dam = 9 * dam / 10;

  if (dam > 1 && IS_SHIELDED (victim, SHD_SANCTUARY))
    dam /= 2;

  if (dam > 1 && ((IS_SHIELDED (victim, SHD_PROTECT_EVIL) && IS_EVIL (ch))
		  || (IS_SHIELDED (victim, SHD_PROTECT_GOOD)
		      && IS_GOOD (ch))))
    dam -= dam / 4;

  //    immune = FALSE;


  switch (check_immune (victim, dam_type))
    {
    case (IS_IMMUNE):
      //immune = TRUE;
      dam = 0;
      break;
    case (IS_RESISTANT):
      dam -= dam / 3;
      break;
    case (IS_VULNERABLE):
      dam += dam / 2;
      break;
    }

  if (dt >= 0 && dt < MAX_SKILL)
    attack = skill_table[dt].noun_damage;
  else if (dt >= TYPE_HIT && dt <= TYPE_HIT + MAX_DAMAGE_MESSAGE)
    attack = attack_table[dt - TYPE_HIT].noun;
  else
    {
      bug ("Dam_message: bad dt %d.", dt);
      dt = TYPE_HIT;
      attack = attack_table[0].name;
    }
  immdam = 0;
  if (ch->level == MAX_LEVEL)
    {
      immdam = dam * 63;
    }
  if (ch == victim)
    {
      snprintf (buf1, sizeof (buf1),
	       "`y$n's `gmock `B%s`g would have done `R%d hp`g damage to `y$mself`g.`x",
	       attack, dam);
      snprintf (buf2, sizeof (buf2),
	       "`yYour `gmock `B%s`g would have done `R%d hp`g damage to `yyourself`g.`x",
	       attack, dam);
      act (buf1, ch, NULL, NULL, TO_ROOM);
      act (buf2, ch, NULL, NULL, TO_CHAR);
    }
  else if (ch->level < MAX_LEVEL)
    {
      snprintf (buf1, sizeof (buf1),
	       "`y$n's `gmock `B%s`g would have done `R%d hp`g damage to `y$N`g.`x",
	       attack, dam);
      snprintf (buf2, sizeof (buf2),
	       "`yYour `gmock `B%s`g would have done `R%d hp`g damage to `y$N`g.`x",
	       attack, dam);
      snprintf (buf3, sizeof (buf3),
	       "`y$n's `gmock `B%s`g would have done `R%d hp`g damage to `yyou`g.`x",
	       attack, dam);
      act (buf1, ch, NULL, victim, TO_NOTVICT);
      act (buf2, ch, NULL, victim, TO_CHAR);
      act (buf3, ch, NULL, victim, TO_VICT);
    }
  else
    {
      snprintf (buf1, sizeof (buf1),
	       "`y$n's `gmock `B%s`g would have done `R%lu hp`g damage to `y$N`g.`x",
	       attack, immdam);
      snprintf (buf2, sizeof (buf2),
	       "`yYour `gmock `B%s`g would have done `R%lu hp`g damage to `y$N`g.`x",
	       attack, immdam);
      snprintf (buf3, sizeof (buf3),
	       "`y$n's `gmock `B%s`g would have done `R%lu hp`g damage to `yyou`g.`x",
	       attack, immdam);
      act (buf1, ch, NULL, victim, TO_NOTVICT);
      act (buf2, ch, NULL, victim, TO_CHAR);
      act (buf3, ch, NULL, victim, TO_VICT);
    }

  tail_chain ();
  return TRUE;
}

/*
 * Inflict damage from a hit.
 */
bool
damage_old (CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, int
	    dam_type, bool show)
{

  OBJ_DATA *corpse;
  bool immune;

  if (victim->position == POS_DEAD)
    return FALSE;

  /*
   * Stop up any residual loopholes.
   */

  if (dam > 1200 && dt >= TYPE_HIT && !IS_IMMORTAL (ch))
    {
      bug ("Damage: %d: more than 1200 points!", dam);
      dam = 1200;
      if (!IS_IMMORTAL (ch))
	{
	  OBJ_DATA *obj;
	  obj = get_eq_char (ch, WEAR_WIELD);
	  send_to_char ("`cYou `z`Greally`x`c shouldn't cheat.`x\n\r", ch);
	  if (obj != NULL)
	    extract_obj (obj);
	}

    }


  /* damage reduction */
  if (dam > 35)
    dam = (dam - 35) / 2 + 35;
  if (dam > 80)
    dam = (dam - 80) / 2 + 80;




  if (victim != ch)
    {
      /*
       * Certain attacks are forbidden.
       * Most other attacks are returned.
       */
      if (is_safe (ch, victim))
	return FALSE;

      if (victim->position > POS_STUNNED)
	{
	  if (victim->fighting == NULL)
	    set_fighting (victim, ch);
	  if (victim->timer <= 4)
	    victim->position = POS_FIGHTING;
	}

      if (victim->position > POS_STUNNED)
	{
	  if (ch->fighting == NULL)
	    set_fighting (ch, victim);

	  /*
	   * If victim is charmed, ch might attack victim's master.
	   */
	  if (IS_NPC (ch)
	      && IS_NPC (victim)
	      && IS_AFFECTED (victim, AFF_CHARM)
	      && victim->master != NULL
	      && victim->master->in_room == ch->in_room
	      && number_bits (3) == 0)
	    {
	      stop_fighting (ch, FALSE);
	      multi_hit (ch, victim->master, TYPE_UNDEFINED);
	      return FALSE;
	    }
	}

      /*
       * More charm stuff.
       */
      if (victim->master == ch)
	stop_follower (victim);
    }

  /*
   * Inviso attacks ... not.
   */
  if (IS_SHIELDED (ch, SHD_INVISIBLE))
    {
      affect_strip (ch, gsn_invis);
      affect_strip (ch, gsn_mass_invis);
      REMOVE_BIT (ch->shielded_by, SHD_INVISIBLE);
      act ("$n fades into existence.", ch, NULL, NULL, TO_ROOM);
    }

  /*
   * Damage modifiers.
   */

  if (dam > 1 && !IS_NPC (victim)
      && victim->pcdata->condition[COND_DRUNK] > 10)
    dam = 9 * dam / 10;

  if (dam > 1 && IS_SHIELDED (victim, SHD_SANCTUARY))
    dam /= 2;

  if (dam > 1 && ((IS_SHIELDED (victim, SHD_PROTECT_EVIL) && IS_EVIL (ch))
		  || (IS_SHIELDED (victim, SHD_PROTECT_GOOD)
		      && IS_GOOD (ch))))
    dam -= dam / 4;

  immune = FALSE;


  /*
   * Check for parry, and dodge.
   */
  if (dt >= TYPE_HIT && ch != victim)
    {
      if (check_parry (ch, victim))
	return FALSE;
      if (check_dodge (ch, victim))
	return FALSE;
      if (check_shield_block (ch, victim))
	return FALSE;

    }

  switch (check_immune (victim, dam_type))
    {
    case (IS_IMMUNE):
      immune = TRUE;
      dam = 0;
      break;
    case (IS_RESISTANT):
      dam -= dam / 3;
      break;
    case (IS_VULNERABLE):
      dam += dam / 2;
      break;
    }

  if (show)
    dam_message (ch, victim, dam, dt, immune);

  if (dam == 0)
    return FALSE;

  /*
   * Hurt the victim.
   * Inform the victim of his new state.
   */
  victim->hit -= dam;
  if (!IS_NPC (victim) && victim->level >= LEVEL_IMMORTAL && victim->hit < 1)
    victim->hit = 1;
  update_pos (victim);
  if (dt == gsn_feed)
    {
      ch->hit = UMIN (ch->hit + ((dam / 3) * 2), ch->max_hit);
      update_pos (ch);
    }

  switch (victim->position)
    {
    case POS_MORTAL:
      act ("`c$n is mortally wounded, and will die soon, if not aided.`x",
	   victim, NULL, NULL, TO_ROOM);
      send_to_char
	("`cYou are mortally wounded, and will die soon, if not aided.`x\n\r",
	 victim);
      break;

    case POS_INCAP:
      act ("`c$n is incapacitated and will slowly die, if not aided.`x",
	   victim, NULL, NULL, TO_ROOM);
      send_to_char
	("`cYou are incapacitated and will slowly `Rdie`c, if not aided.`x\n\r",
	 victim);
      break;

    case POS_STUNNED:
      act ("`c$n is stunned, but will probably recover.`x",
	   victim, NULL, NULL, TO_ROOM);
      send_to_char ("`cYou are stunned, but will probably recover.`x\n\r",
		    victim);
      break;

    case POS_DEAD:
      if ((IS_NPC (victim)) && (victim->die_descr[0] != '\0'))
	{
	  act ("`c$n $T`x", victim, 0, victim->die_descr, TO_ROOM);
	}
      else
	{
	  act ("`c$n is `CDEAD!!`x", victim, 0, 0, TO_ROOM);
	}
      send_to_char ("`cYou have been `RKILLED!!`x\n\r\n\r", victim);
      break;

    default:
      if (dam > victim->max_hit / 4)
	send_to_char ("`cThat really did `RHURT!`x\n\r", victim);
      if (victim->hit < victim->max_hit / 4)
	send_to_char ("`cYou sure are `z`RBLEEDING!`x\n\r", victim);
      break;
    }

  /*
   * Sleep spells and extremely wounded folks.
   */
  if (!IS_AWAKE (victim))
    stop_fighting (victim, FALSE);

  /*
   * Payoff for killing things.
   */
  if (victim->position == POS_DEAD)
    {
      group_gain (ch, victim);

      if (!IS_NPC (victim))
	{
	  snprintf (log_buf, MAX_STRING_LENGTH, "%s killed by %s at %d",
		   victim->name,
		   (IS_NPC (ch) ? ch->short_descr : ch->name),
		   victim->in_room->vnum);
	  log_string (log_buf);

	  /*
	   * Dying penalty:
	   * 2/3 way back to previous level.
	   */
	  if (victim->exp > exp_per_level (victim, victim->pcdata->points)
	      * victim->level)
	    gain_exp (victim,
		      (2 *
		       (exp_per_level (victim, victim->pcdata->points) *
			victim->level - victim->exp) / 3) + 50);
	}

      snprintf (log_buf, MAX_STRING_LENGTH, "%s got toasted by %s at %s [room %d]",
	       (IS_NPC (victim) ? victim->short_descr : victim->name),
	       (IS_NPC (ch) ? ch->short_descr : ch->name),
	       ch->in_room->name, ch->in_room->vnum);

      if (IS_NPC (victim))
	wiznet (log_buf, NULL, NULL, WIZ_MOBDEATHS, 0, 0);
      else
	wiznet (log_buf, NULL, NULL, WIZ_DEATHS, 0, 0);

      raw_kill (victim, ch);
      /* dump the flags */
      if (ch != victim && !IS_NPC (ch) && (!is_same_clan (ch, victim)
					   || clan_table[victim->clan].
					   independent))
	{
	  if (IS_SET (victim->act, PLR_TWIT))
	    REMOVE_BIT (victim->act, PLR_TWIT);
	}
      /* RT new auto commands */

      if (!IS_NPC (ch) && IS_NPC (victim))
	{
	  corpse = get_obj_list (ch, "corpse", ch->in_room->contents);

	  if (IS_SET (ch->act, PLR_AUTOLOOT) && corpse && corpse->contains)	/* exists and not empty */
	    do_get (ch, "all corpse");

	  if (IS_SET (ch->act, PLR_AUTOGOLD) && corpse && corpse->contains &&	/* exists and not empty */
	      !IS_SET (ch->act, PLR_AUTOLOOT))
	    do_get (ch, "gold corpse");

	  if (IS_SET (ch->act, PLR_AUTOSAC))
	    {
	      if (IS_SET (ch->act, PLR_AUTOLOOT) && corpse
		  && corpse->contains)
		return TRUE;	/* leave if corpse has treasure */
	      else
		do_sacrifice (ch, "corpse");
	    }
	}

      return TRUE;
    }

  if (victim == ch)
    return TRUE;

  /*
   * Take care of link dead people.
   */
  if (!IS_NPC (victim) && victim->desc == NULL)
    {
      if (number_range (0, victim->wait) == 0)
	{
	  do_recall (victim, "");
	  return TRUE;
	}
    }

  /*
   * Wimp out?
   */
  if (IS_NPC (victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2)
    {
      if ((IS_SET (victim->act, ACT_WIMPY) && number_bits (2) == 0
	   && victim->hit < victim->max_hit / 5)
	  || (IS_AFFECTED (victim, AFF_CHARM) && victim->master != NULL
	      && victim->master->in_room != victim->in_room))
	do_flee (victim, "");
    }

  if (!IS_NPC (victim)
      && victim->hit > 0
      && victim->hit <= victim->wimpy && victim->wait < PULSE_VIOLENCE / 2)
    do_flee (victim, "");

  tail_chain ();
  return TRUE;
}

bool
is_safe (CHAR_DATA * ch, CHAR_DATA * victim)
{
  if (victim->in_room == NULL || ch->in_room == NULL)
    return TRUE;

  if (victim->fighting == ch || victim == ch)
    return FALSE;

  if (!IS_NPC (ch) && IS_IMMORTAL (ch))
    return FALSE;

  /* killing mobiles */
  if (IS_NPC (victim))
    {

      /* safe room? */
      if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
	{
	  send_to_char ("Not in this room.\n\r", ch);
	  return TRUE;
	}

      if (victim->pIndexData->pShop != NULL)
	{
	  send_to_char ("The shopkeeper wouldn't like that.\n\r", ch);
	  return TRUE;
	}

      /* no killing healers, trainers, etc */
      if (IS_SET (victim->act, ACT_TRAIN)
	  || IS_SET (victim->act, ACT_BOUNTY)
	  || IS_SET (victim->act, ACT_PRACTICE)
	  || IS_SET (victim->act, ACT_IS_HEALER)
	  || IS_SET (victim->act, ACT_IS_CHANGER)
	  || IS_SET (victim->act, ACT_IS_SATAN)
	  || IS_SET (victim->act, ACT_IS_PRIEST))
	{
	  act ("I don't think $G would approve.", ch, NULL, NULL, TO_CHAR);
	  return TRUE;
	}

      if (!IS_NPC (ch))
	{
	  /* no pets */
	  if (IS_SET (victim->act, ACT_PET))
	    {
	      act ("But $N looks so cute and cuddly...",
		   ch, NULL, victim, TO_CHAR);
	      return TRUE;
	    }

	  /* no charmed creatures unless owner */
	  if (IS_AFFECTED (victim, AFF_CHARM) && ch != victim->master)
	    {
	      send_to_char ("You don't own that monster.\n\r", ch);
	      return TRUE;
	    }
	}
    }
  /* killing players */
  else
    {
      /* NPC doing the killing */
      if (IS_NPC (ch))
	{
	  /* safe room check */
	  if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
	    {
	      send_to_char ("Not in this room.\n\r", ch);
	      return TRUE;
	    }

	  /* charmed mobs and pets cannot attack players while owned */
	  if (IS_AFFECTED (ch, AFF_CHARM) && ch->master != NULL
	      && ch->master->fighting != victim)
	    {
	      send_to_char ("Players are your friends!\n\r", ch);
	      return TRUE;
	    }
	}
      /* player doing the killing */
      else
	{
	  if (IS_SET (victim->act, PLR_TWIT))
	    return FALSE;

	  if (((victim->level > 19)
	       || ((victim->class >= MAX_CLASS / 2)
		   && (victim->level > 14))) && (is_voodood (ch, victim)))
	    return FALSE;

	  if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
	    {
	      send_to_char ("Not in this room.\n\r", ch);
	      return TRUE;
	    }
	  if (ch->on_quest)
	    {
	      send_to_char ("Not while you are on a quest.\n\r", ch);
	      return TRUE;
	    }
	  if (victim->on_quest)
	    {
	      send_to_char ("They are on a quest, leave them alone.\n\r", ch);
	      return TRUE;
	    }
	  if (!is_clan (ch))
	    {
	      send_to_char ("Join a clan if you want to fight players.\n\r",
			    ch);
	      return TRUE;
	    }

	  if (!is_pkill (ch))
	    {
	      send_to_char ("Your clan does not allow player fighting.\n\r",
			    ch);
	      return TRUE;
	    }

	  if (!is_clan (victim))
	    {
	      send_to_char ("They aren't in a clan, leave them alone.\n\r",
			    ch);
	      return TRUE;
	    }

	  if (!is_pkill (victim))
	    {
	      send_to_char
		("They are in a no pkill clan, leave them alone.\n\r", ch);
	      return TRUE;
	    }

	  if (is_same_clan (ch, victim))
	    {
	      send_to_char ("You can't fight your own clan members.\n\r", ch);
	      return TRUE;
	    }

	  if (((ch->class < MAX_CLASS / 2)
	       && (victim->class < MAX_CLASS / 2))
	      || ((ch->class >= MAX_CLASS / 2)
		  && (victim->class >= MAX_CLASS / 2)))
	    {
	      if (ch->level > victim->level + 10)
		{
		  send_to_char ("Pick on someone your own size.\n\r", ch);
		  return TRUE;
		}
	      if (ch->level < victim->level - 10)
		{
		  send_to_char ("Pick on someone your own size.\n\r", ch);
		  return TRUE;
		}
	    }
	  else
	    {
	      send_to_char ("Pick on someone in your own tier.\n\r", ch);
	      return TRUE;
	    }
	}
    }
  return FALSE;
}

bool
is_safe_mock (CHAR_DATA * ch, CHAR_DATA * victim)
{
  if (victim->in_room == NULL || ch->in_room == NULL)
    return TRUE;
  if (!IS_NPC (ch) && IS_IMMORTAL (ch))
    return FALSE;
  if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
    {
      send_to_char ("Not in this room.\n\r", ch);
      return TRUE;
    }
  if (IS_NPC (victim))
    {
      send_to_char ("`RYou can only use this on a player.`x\n\r", ch);
      return TRUE;
    }
  return FALSE;
}

bool
is_voodood (CHAR_DATA * ch, CHAR_DATA * victim)
{
  OBJ_DATA *object;
  //    bool found;

  if (ch->level > HERO)
    return FALSE;

  //    found = FALSE;
  for (object = victim->carrying; object != NULL;
       object = object->next_content)
    {
      if (object->pIndexData->vnum == OBJ_VNUM_VOODOO)
	{
	  char arg[MAX_INPUT_LENGTH];

	  one_argument (object->name, arg);
	  if (!str_cmp (arg, ch->name))
	    {
	      return TRUE;
	    }
	}
    }
  return FALSE;
}

bool
is_safe_spell (CHAR_DATA * ch, CHAR_DATA * victim, bool area)
{
  if (victim->in_room == NULL || ch->in_room == NULL)
    return TRUE;

  if (victim == ch && area)
    return TRUE;

  if (victim->fighting == ch || victim == ch)
    return FALSE;

  if (!IS_NPC (ch) && IS_IMMORTAL (ch))
    return FALSE;

  /* killing mobiles */
  if (IS_NPC (victim))
    {
      /* safe room? */
      if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
	return TRUE;

      if (victim->pIndexData->pShop != NULL)
	return TRUE;

      /* no killing healers, trainers, etc */
      if (IS_SET (victim->act, ACT_TRAIN)
	  || IS_SET (victim->act, ACT_PRACTICE)
	  || IS_SET (victim->act, ACT_IS_HEALER)
	  || IS_SET (victim->act, ACT_IS_CHANGER)
	  || IS_SET (victim->act, ACT_IS_SATAN)
	  || IS_SET (victim->act, ACT_IS_PRIEST))
	return TRUE;

      if (!IS_NPC (ch))
	{
	  /* no pets */
	  if (IS_SET (victim->act, ACT_PET))
	    return TRUE;

	  /* no charmed creatures unless owner */
	  if (IS_AFFECTED (victim, AFF_CHARM)
	      && (area || ch != victim->master))
	    return TRUE;

	  /* legal kill? -- cannot hit mob fighting non-group member */
	  if (victim->fighting != NULL
	      && !is_same_group (ch, victim->fighting))
	    return TRUE;
	}
      else
	{
	  /* area effect spells do not hit other mobs */
	  if (area && !is_same_group (victim, ch->fighting))
	    return TRUE;
	}
    }
  /* killing players */
  else
    {
      if (area && IS_IMMORTAL (victim) && victim->level > LEVEL_IMMORTAL)
	return TRUE;

      /* NPC doing the killing */
      if (IS_NPC (ch))
	{
	  /* charmed mobs and pets cannot attack players while owned */
	  if (((IS_AFFECTED (ch, AFF_CHARM)) & (ch->master != NULL))
	      && (ch->master->fighting != victim))
	    return TRUE;

	  /* safe room? */
	  if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
	    return TRUE;

	  /* legal kill? -- mobs only hit players grouped with opponent */
	  if (ch->fighting != NULL && !is_same_group (ch->fighting, victim))
	    return TRUE;
	}

      /* player doing the killing */
      else
	{
	  if (IS_SET (victim->act, PLR_TWIT))
	    return FALSE;

	  if (((victim->level > 19)
	       || ((victim->class >= MAX_CLASS / 2)
		   && (victim->level > 14))) && (is_voodood (ch, victim)))
	    return FALSE;

	  if (!is_clan (ch))
	    return TRUE;

	  if (!is_pkill (ch))
	    return TRUE;

	  if (IS_SET (victim->in_room->room_flags, ROOM_SAFE))
	    return TRUE;

	  if (ch->on_quest)
	    return TRUE;

	  if (victim->on_quest)
	    return TRUE;

	  if (!is_clan (victim))
	    return TRUE;

	  if (!is_pkill (victim))
	    return TRUE;

	  if (is_same_clan (ch, victim))
	    return TRUE;

	  if (((ch->class < MAX_CLASS / 2)
	       && (victim->class < MAX_CLASS / 2))
	      || ((ch->class >= MAX_CLASS / 2)
		  && (victim->class >= MAX_CLASS / 2)))
	    {
	      if (ch->level > victim->level + 10)
		{
		  return TRUE;
		}
	      if (ch->level < victim->level - 10)
		{
		  return TRUE;
		}
	    }
	  else
	    {
	      return TRUE;
	    }
	}
    }
  return FALSE;
}

/*
 * Check for parry.
 */
bool
check_parry (CHAR_DATA * ch, CHAR_DATA * victim)
{
  int chance;

  if (!IS_AWAKE (victim))
    return FALSE;

  chance = get_skill (victim, gsn_parry) / 2;

  if (get_eq_char (victim, WEAR_WIELD) == NULL)
    {
      if (IS_NPC (victim))
	chance /= 2;
      else
	return FALSE;
    }

  if (victim->stunned)
    return FALSE;

  if (!can_see (ch, victim))
    chance /= 2;

  if (number_percent () >= chance + victim->level - ch->level)
    return FALSE;

  act ("`iYou parry $n's attack.`x", ch, NULL, victim, TO_VICT);
  act ("`h$N parries your attack.`x", ch, NULL, victim, TO_CHAR);
  check_improve (victim, gsn_parry, TRUE, 6);
  return TRUE;
}

/*
 * Check for shield block.
 */
bool
check_shield_block (CHAR_DATA * ch, CHAR_DATA * victim)
{
  int chance;

  if (!IS_AWAKE (victim))
    return FALSE;


  chance = get_skill (victim, gsn_shield_block) / 5 + 3;


  if (get_eq_char (victim, WEAR_SHIELD) == NULL)
    return FALSE;

  if (number_percent () >= chance + victim->level - ch->level)
    return FALSE;

  if (victim->stunned)
    return FALSE;

  act ("`iYou block $n's attack with your shield.`x", ch, NULL, victim,
       TO_VICT);
  act ("`h$N blocks your attack with a shield.`x", ch, NULL, victim, TO_CHAR);
  check_improve (victim, gsn_shield_block, TRUE, 6);
  return TRUE;
}


/*
 * Check for dodge.
 */
bool
check_dodge (CHAR_DATA * ch, CHAR_DATA * victim)
{
  int chance;

  if (!IS_AWAKE (victim))
    return FALSE;

  chance = get_skill (victim, gsn_dodge) / 2;

  if (!can_see (victim, ch))
    chance /= 2;

  if (number_percent () >= chance + victim->level - ch->level)
    return FALSE;

  if (victim->stunned)
    return FALSE;

  act ("`iYou dodge $n's attack.`x", ch, NULL, victim, TO_VICT);
  act ("`h$N dodges your attack.`x", ch, NULL, victim, TO_CHAR);
  check_improve (victim, gsn_dodge, TRUE, 6);
  return TRUE;
}



/*
 * Set position of a victim.
 */
void
update_pos (CHAR_DATA * victim)
{
  if (victim->hit > 0)
    {
      if (victim->position <= POS_STUNNED)
	victim->position = POS_STANDING;
      return;
    }

  if (IS_NPC (victim) && victim->hit < 1)
    {
      victim->position = POS_DEAD;
      return;
    }

  if (victim->hit <= -11)
    {
      victim->position = POS_DEAD;
      return;
    }

  if (victim->hit <= -6)
    victim->position = POS_MORTAL;
  else if (victim->hit <= -3)
    victim->position = POS_INCAP;
  else
    victim->position = POS_STUNNED;

  return;
}



/*
 * Start fights.
 */
void
set_fighting (CHAR_DATA * ch, CHAR_DATA * victim)
{
  if (ch->fighting != NULL)
    {
      bug ("Set_fighting: already fighting", 0);
      return;
    }

  if (IS_AFFECTED (ch, AFF_SLEEP))
    affect_strip (ch, gsn_sleep);

  ch->fighting = victim;
  ch->position = POS_FIGHTING;
  ch->stunned = 0;

  return;
}



/*
 * Stop fights.
 */
void
stop_fighting (CHAR_DATA * ch, bool fBoth)
{
  CHAR_DATA *fch;
  char buf[MAX_STRING_LENGTH];

  for (fch = char_list; fch != NULL; fch = fch->next)
    {
      if (fch == ch || (fBoth && fch->fighting == ch))
	{
	  fch->fighting = NULL;
	  fch->position = IS_NPC (fch) ? fch->default_pos : POS_STANDING;
	  fch->stunned = 0;
	  update_pos (fch);
	  if (IS_SET (fch->comm, COMM_STORE))
	    if (fch->tells)
	      {
		snprintf (buf, sizeof (buf), "You have `R%d`x tells waiting.\n\r",
			 fch->tells);
		send_to_char (buf, fch);
		send_to_char ("Type 'replay' to see tells.\n\r", fch);
	      }
	}
    }

  return;
}



/*
 * Make a corpse out of a character.
 */
void
make_corpse (CHAR_DATA * ch, CHAR_DATA * killer)
{
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *corpse;
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  char *name;
  ROOM_INDEX_DATA *location;
  location = get_room_index (ROOM_VNUM_MORGUE);

  if (IS_NPC (ch))
    {
      if (IS_SET (ch->act, ACT_NO_BODY))
	{
	  if (IS_SET (ch->act, ACT_NB_DROP))
	    {
	      for (obj = ch->carrying; obj != NULL; obj = obj_next)
		{
		  obj_next = obj->next_content;
		  obj_from_char (obj);
		  if (obj->item_type == ITEM_POTION)
		    obj->timer = number_range (500, 1000);
		  if (obj->item_type == ITEM_SCROLL)
		    obj->timer = number_range (1000, 2500);
		  if (IS_SET (obj->extra_flags, ITEM_ROT_DEATH))
		    {
		      obj->timer = number_range (5, 10);
		      REMOVE_BIT (obj->extra_flags, ITEM_ROT_DEATH);
		    }
		  REMOVE_BIT (obj->extra_flags, ITEM_VIS_DEATH);

		  if (IS_SET (obj->extra_flags, ITEM_INVENTORY))
		    extract_obj (obj);
		  act ("$p falls to the floor.", ch, obj, NULL, TO_ROOM);
		  obj_to_room (obj, ch->in_room);
		}
	    }
	  return;
	}
      name = ch->short_descr;
      corpse = create_object (get_obj_index (OBJ_VNUM_CORPSE_NPC), 0);
      corpse->timer = number_range (3, 6);
      if (ch->gold > 0 || ch->platinum > 0)
	{
	  obj_to_obj (create_money (ch->platinum, ch->gold, ch->silver),
		      corpse);
	  ch->platinum = 0;
	  ch->gold = 0;
	  ch->silver = 0;
	}
      corpse->cost = 0;
    }
  else
    {
      name = ch->name;
      corpse = create_object (get_obj_index (OBJ_VNUM_CORPSE_PC), 0);
      corpse->timer = number_range (25, 40);
      REMOVE_BIT (ch->act, PLR_CANLOOT);
      if (!is_clan (ch))
	{
	  corpse->owner = str_dup (ch->name);
	  corpse->killer = NULL;
	}
      else
	{
	  corpse->owner = str_dup (ch->name);
	  corpse->killer = str_dup (killer->name);
	  if (ch->platinum > 1 || ch->gold > 1 || ch->silver > 1)
	    {
	      obj_to_obj (create_money
			  (ch->platinum / 2, ch->gold / 2, ch->silver / 2),
			  corpse);
	      ch->platinum -= ch->platinum / 2;
	      ch->gold -= ch->gold / 2;
	      ch->silver -= ch->silver / 2;
	    }
	}

      corpse->cost = 0;
    }

  corpse->level = ch->level;

  snprintf (buf, sizeof (buf), corpse->short_descr, name);
  free_string (corpse->short_descr);
  corpse->short_descr = str_dup (buf);

  snprintf (buf, sizeof (buf), corpse->description, name);
  free_string (corpse->description);
  corpse->description = str_dup (buf);

  for (obj = ch->carrying; obj != NULL; obj = obj_next)
    {
      bool floating = FALSE;

      obj_next = obj->next_content;
      if (obj->wear_loc == WEAR_FLOAT)
	floating = TRUE;
      obj_from_char (obj);
      if (obj->item_type == ITEM_POTION)
	obj->timer = number_range (500, 1000);
      if (obj->item_type == ITEM_SCROLL)
	obj->timer = number_range (1000, 2500);
      if (IS_SET (obj->extra_flags, ITEM_ROT_DEATH) && !floating)
	{
	  obj->timer = number_range (5, 10);
	  REMOVE_BIT (obj->extra_flags, ITEM_ROT_DEATH);
	}
      REMOVE_BIT (obj->extra_flags, ITEM_VIS_DEATH);

      if (IS_SET (obj->extra_flags, ITEM_INVENTORY))
	extract_obj (obj);
      else if (floating)
	{
	  if (IS_OBJ_STAT (obj, ITEM_ROT_DEATH))	/* get rid of it! */
	    {
	      if (obj->contains != NULL)
		{
		  OBJ_DATA *in, *in_next;

		  act ("$p evaporates,scattering its contents.",
		       ch, obj, NULL, TO_ROOM);
		  for (in = obj->contains; in != NULL; in = in_next)
		    {
		      in_next = in->next_content;
		      obj_from_obj (in);
		      obj_to_room (in, ch->in_room);
		    }
		}
	      else
		act ("$p evaporates.", ch, obj, NULL, TO_ROOM);
	      extract_obj (obj);
	    }
	  else
	    {
	      act ("$p falls to the floor.", ch, obj, NULL, TO_ROOM);
	      obj_to_room (obj, ch->in_room);
	    }
	}
      else
	obj_to_obj (obj, corpse);
    }

  if (IS_NPC (ch))
    obj_to_room (corpse, ch->in_room);
  else
    obj_to_room (corpse, location);
}



/*
 * Improved Death_cry contributed by Diavolo.
 */
void
death_cry (CHAR_DATA * ch)
{
  ROOM_INDEX_DATA *was_in_room;
  char *msg;
  int door;
  int vnum;

  vnum = 0;
  msg = "You hear $n's death cry.";
  if (IS_NPC (ch))
    {
      if (!IS_SET (ch->act, ACT_NO_BODY))
	{
	  switch (number_bits (4))
	    {
	    case 0:
	      msg = "$n hits the ground ... DEAD.";
	      vnum = OBJ_VNUM_BLOOD;
	      break;
	    case 1:
	      msg = "$n splatters blood on your armor.";
	      vnum = OBJ_VNUM_BLOOD;
	      break;
	    case 2:
	      if (IS_SET (ch->parts, PART_GUTS))
		{
		  msg = "$n spills $s guts all over the floor.";
		  vnum = OBJ_VNUM_GUTS;
		}
	      break;
	    case 3:
	      if (IS_SET (ch->parts, PART_HEAD))
		{
		  msg = "$n's severed head plops on the ground.";
		  vnum = OBJ_VNUM_SEVERED_HEAD;
		}
	      break;
	    case 4:
	      if (IS_SET (ch->parts, PART_HEART))
		{
		  msg = "$n's heart is torn from $s chest.";
		  vnum = OBJ_VNUM_TORN_HEART;
		}
	      break;
	    case 5:
	      if (IS_SET (ch->parts, PART_ARMS))
		{
		  msg = "$n's arm is sliced from $s dead body.";
		  vnum = OBJ_VNUM_SLICED_ARM;
		}
	      break;
	    case 6:
	      if (IS_SET (ch->parts, PART_LEGS))
		{
		  msg = "$n's leg is sliced from $s dead body.";
		  vnum = OBJ_VNUM_SLICED_LEG;
		}
	      break;
	    case 7:
	      if (IS_SET (ch->parts, PART_BRAINS))
		{
		  msg =
		    "$n's head is shattered, and $s brains splash all over you.";
		  vnum = OBJ_VNUM_BRAINS;
		}
	      break;
	    case 8:
	      msg = "$n hits the ground ... DEAD.";
	      vnum = OBJ_VNUM_BLOOD;
	      break;
	    case 9:
	      msg = "$n hits the ground ... DEAD.";
	      vnum = OBJ_VNUM_BLOOD;
	    }
	}
    }
  else if (ch->level > 19)
    {
      switch (number_bits (4))
	{
	case 0:
	  msg = "$n hits the ground ... DEAD.";
	  vnum = OBJ_VNUM_BLOOD;
	  break;
	case 1:
	  msg = "$n splatters blood on your armor.";
	  vnum = OBJ_VNUM_BLOOD;
	  break;
	case 2:
	  if (IS_SET (ch->parts, PART_GUTS))
	    {
	      msg = "$n spills $s guts all over the floor.";
	      vnum = OBJ_VNUM_GUTS;
	    }
	  break;
	case 3:
	  if (IS_SET (ch->parts, PART_HEAD))
	    {
	      msg = "$n's severed head plops on the ground.";
	      vnum = OBJ_VNUM_SEVERED_HEAD;
	    }
	  break;
	case 4:
	  if (IS_SET (ch->parts, PART_HEART))
	    {
	      msg = "$n's heart is torn from $s chest.";
	      vnum = OBJ_VNUM_TORN_HEART;
	    }
	  break;
	case 5:
	  if (IS_SET (ch->parts, PART_ARMS))
	    {
	      msg = "$n's arm is sliced from $s dead body.";
	      vnum = OBJ_VNUM_SLICED_ARM;
	    }
	  break;
	case 6:
	  if (IS_SET (ch->parts, PART_LEGS))
	    {
	      msg = "$n's leg is sliced from $s dead body.";
	      vnum = OBJ_VNUM_SLICED_LEG;
	    }
	  break;
	case 7:
	  if (IS_SET (ch->parts, PART_BRAINS))
	    {
	      msg =
		"$n's head is shattered, and $s brains splash all over you.";
	      vnum = OBJ_VNUM_BRAINS;
	    }
	  break;
	case 8:
	  msg = "$n hits the ground ... DEAD.";
	  vnum = OBJ_VNUM_BLOOD;
	  break;
	case 9:
	  msg = "$n hits the ground ... DEAD.";
	  vnum = OBJ_VNUM_BLOOD;
	  break;
	case 10:
	  if (IS_SET (ch->parts, PART_HEAD))
	    {
	      msg = "$n's severed head plops on the ground.";
	      vnum = OBJ_VNUM_SEVERED_HEAD;
	    }
	  break;
	case 11:
	  if (IS_SET (ch->parts, PART_HEART))
	    {
	      msg = "$n's heart is torn from $s chest.";
	      vnum = OBJ_VNUM_TORN_HEART;
	    }
	  break;
	case 12:
	  if (IS_SET (ch->parts, PART_ARMS))
	    {
	      msg = "$n's arm is sliced from $s dead body.";
	      vnum = OBJ_VNUM_SLICED_ARM;
	    }
	  break;
	case 13:
	  if (IS_SET (ch->parts, PART_LEGS))
	    {
	      msg = "$n's leg is sliced from $s dead body.";
	      vnum = OBJ_VNUM_SLICED_LEG;
	    }
	  break;
	case 14:
	  if (IS_SET (ch->parts, PART_BRAINS))
	    {
	      msg =
		"$n's head is shattered, and $s brains splash all over you.";
	      vnum = OBJ_VNUM_BRAINS;
	    }
	}
    }

  act (msg, ch, NULL, NULL, TO_ROOM);

  if ((vnum == 0) && !IS_SET (ch->act, ACT_NO_BODY))
    {
      switch (number_bits (4))
	{
	case 0:
	  vnum = 0;
	  break;
	case 1:
	  vnum = OBJ_VNUM_BLOOD;
	  break;
	case 2:
	  vnum = 0;
	  break;
	case 3:
	  vnum = OBJ_VNUM_BLOOD;
	  break;
	case 4:
	  vnum = 0;
	  break;
	case 5:
	  vnum = OBJ_VNUM_BLOOD;
	  break;
	case 6:
	  vnum = 0;
	  break;
	case 7:
	  vnum = OBJ_VNUM_BLOOD;
	}
    }

  if (vnum != 0)
    {
      char buf[MAX_STRING_LENGTH];
      OBJ_DATA *obj;
      char *name;

      name = IS_NPC (ch) ? ch->short_descr : ch->name;
      obj = create_object (get_obj_index (vnum), 0);
      obj->timer = number_range (4, 7);
      if (!IS_NPC (ch))
	{
	  obj->timer = number_range (12, 18);
	}
      if (vnum == OBJ_VNUM_BLOOD)
	{
	  obj->timer = number_range (1, 4);
	}

      snprintf (buf, sizeof (buf), obj->short_descr, name);
      free_string (obj->short_descr);
      obj->short_descr = str_dup (buf);

      snprintf (buf, sizeof (buf), obj->description, name);
      free_string (obj->description);
      obj->description = str_dup (buf);

      snprintf (buf, sizeof (buf), obj->name, name);
      free_string (obj->name);
      obj->name = str_dup (buf);

      if (obj->item_type == ITEM_FOOD)
	{
	  if (IS_SET (ch->form, FORM_POISON))
	    obj->value[3] = 1;
	  else if (!IS_SET (ch->form, FORM_EDIBLE))
	    obj->item_type = ITEM_TRASH;
	}

      if (IS_NPC (ch))
	{
	  obj->value[4] = 0;
	}
      else
	{
	  obj->value[4] = 1;
	}

      obj_to_room (obj, ch->in_room);
    }

  if (IS_NPC (ch))
    msg = "You hear something's death cry.";
  else
    msg = "You hear someone's death cry.";

  was_in_room = ch->in_room;
  for (door = 0; door <= 5; door++)
    {
      EXIT_DATA *pexit;

      if ((pexit = was_in_room->exit[door]) != NULL
	  && pexit->u1.to_room != NULL && pexit->u1.to_room != was_in_room)
	{
	  ch->in_room = pexit->u1.to_room;
	  act (msg, ch, NULL, NULL, TO_ROOM);
	}
    }
  ch->in_room = was_in_room;

  return;
}


