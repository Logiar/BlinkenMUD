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

/* command procedures needed */
DECLARE_DO_FUN (do_split);
DECLARE_DO_FUN (do_yell);
DECLARE_DO_FUN (do_say);
DECLARE_DO_FUN (do_at);
DECLARE_DO_FUN (do_wear);



/*
 * Local functions.
 */
#define CD CHAR_DATA
#define OD OBJ_DATA
bool remove_obj args ((CHAR_DATA * ch, int iWear, bool fReplace));
void wear_obj args ((CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace));
CD *find_keeper args ((CHAR_DATA * ch));
int get_cost args ((CHAR_DATA * keeper, OBJ_DATA * obj, bool fBuy));
void obj_to_keeper args ((OBJ_DATA * obj, CHAR_DATA * ch));
OD *get_obj_keeper
args ((CHAR_DATA * ch, CHAR_DATA * keeper, char *argument));
bool can_quest args ((CHAR_DATA * ch));

#undef OD
#undef	CD

/* RT part of the corpse looting code */

CHAR_DATA *
find_keeper (CHAR_DATA * ch)
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *keeper;
  SHOP_DATA *pShop;

  pShop = NULL;
  for (keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room)
    {
      if (IS_NPC (keeper) && (pShop = keeper->pIndexData->pShop) != NULL)
	break;
    }

  if (pShop == NULL)
    {
      send_to_char ("You can't do that here.\n\r", ch);
      return NULL;
    }

  /*
   * Undesirables.
   */
  if (!IS_NPC (ch) && IS_SET (ch->act, PLR_TWIT))
    {
      do_say (keeper, "`aTwits are not welcome!`x");
      snprintf (buf, sizeof (buf), "`a%s the `z`RTWIT`x is over here!`x\n\r", ch->name);
      do_yell (keeper, buf);
      return NULL;
    }
  /*
   * Shop hours.
   */
  if (time_info.hour < pShop->open_hour)
    {
      do_say (keeper, "`aSorry, I am closed. Come back later.`x");
      return NULL;
    }

  if (time_info.hour > pShop->close_hour)
    {
      do_say (keeper, "`aSorry, I am closed. Come back tomorrow.`x");
      return NULL;
    }

  /*
   * Invisible or hidden people.
   */
  if (!can_see (keeper, ch))
    {
      do_say (keeper, "`aI don't trade with folks I can't see.`x");
      return NULL;
    }

  return keeper;
}

/* insert an object at the right spot for the keeper */
void
obj_to_keeper (OBJ_DATA * obj, CHAR_DATA * ch)
{
  OBJ_DATA *t_obj, *t_obj_next;

  /* see if any duplicates are found */
  for (t_obj = ch->carrying; t_obj != NULL; t_obj = t_obj_next)
    {
      t_obj_next = t_obj->next_content;

      if (obj->pIndexData == t_obj->pIndexData
	  && !str_cmp (obj->short_descr, t_obj->short_descr))
	{
	  /* if this is an unlimited item, destroy the new one */
	  if (IS_OBJ_STAT (t_obj, ITEM_INVENTORY))
	    {
	      extract_obj (obj);
	      return;
	    }
	  obj->cost = t_obj->cost;	/* keep it standard */
	  break;
	}
    }

  if (t_obj == NULL)
    {
      obj->next_content = ch->carrying;
      ch->carrying = obj;
    }
  else
    {
      obj->next_content = t_obj->next_content;
      t_obj->next_content = obj;
    }

  obj->carried_by = ch;
  obj->in_room = NULL;
  obj->in_obj = NULL;
  ch->carry_number += get_obj_number (obj);
  ch->carry_weight += get_obj_weight (obj);
}

/* get an object from a shopkeeper's list */
OBJ_DATA *
get_obj_keeper (CHAR_DATA * ch, CHAR_DATA * keeper, char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int number;
  int count;

  number = number_argument (argument, arg);
  count = 0;
  for (obj = keeper->carrying; obj != NULL; obj = obj->next_content)
    {
      if (obj->wear_loc == WEAR_NONE
	  && can_see_obj (keeper, obj)
	  && can_see_obj (ch, obj) && is_name (arg, obj->name))
	{
	  if (++count == number)
	    return obj;

	  /* skip other objects of the same name */
	  while (obj->next_content != NULL
		 && obj->pIndexData == obj->next_content->pIndexData
		 && !str_cmp (obj->short_descr,
			      obj->next_content->short_descr))
	    obj = obj->next_content;
	}
    }

  return NULL;
}

int
get_cost (CHAR_DATA * keeper, OBJ_DATA * obj, bool fBuy)
{
  SHOP_DATA *pShop;
  int cost;

  if (obj == NULL || (pShop = keeper->pIndexData->pShop) == NULL)
    return 0;

  if (fBuy)
    {
      cost = obj->cost * pShop->profit_buy / 100;
    }
  else
    {
      OBJ_DATA *obj2;
      int itype;

      cost = 0;
      for (itype = 0; itype < MAX_TRADE; itype++)
	{
	  if (obj->item_type == pShop->buy_type[itype])
	    {
	      cost = obj->cost * pShop->profit_sell / 100;
	      break;
	    }
	}

      if (!IS_OBJ_STAT (obj, ITEM_SELL_EXTRACT))
	for (obj2 = keeper->carrying; obj2; obj2 = obj2->next_content)
	  {
	    if (obj->pIndexData == obj2->pIndexData
		&& !str_cmp (obj->short_descr, obj2->short_descr))
	      {
		if (IS_OBJ_STAT (obj2, ITEM_INVENTORY))
		  cost /= 2;
		else
		  cost = cost * 3 / 4;
	      }
	  }
    }

  if (obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND)
    {
      if (obj->value[1] == 0)
	cost /= 4;
      else
	cost = cost * obj->value[2] / obj->value[1];
    }

  return cost;
}



void
do_buy (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  int cost, roll;
  long multicost;

  if (argument[0] == '\0')
    {
      send_to_char ("Buy what?\n\r", ch);
      return;
    }

  smash_tilde (argument);

  if (IS_SET (ch->in_room->room_flags, ROOM_PET_SHOP))
    {
      char arg[MAX_INPUT_LENGTH];
      char buf[MAX_STRING_LENGTH];
      CHAR_DATA *pet;
      ROOM_INDEX_DATA *pRoomIndexNext;
      ROOM_INDEX_DATA *in_room;

      if (IS_NPC (ch))
	return;

      argument = one_argument (argument, arg);

      /* hack to make new thalos pets work */
      if (ch->in_room->vnum == 9621)
	pRoomIndexNext = get_room_index (9706);
      else
	pRoomIndexNext = get_room_index (ch->in_room->vnum + 1);
      if (pRoomIndexNext == NULL)
	{
	  bug ("Do_buy: bad pet shop at vnum %d.", ch->in_room->vnum);
	  send_to_char ("Sorry, you can't buy that here.\n\r", ch);
	  return;
	}

      in_room = ch->in_room;
      ch->in_room = pRoomIndexNext;
      pet = get_char_room (ch, arg);
      ch->in_room = in_room;

      if (pet == NULL || !IS_SET (pet->act, ACT_PET))
	{
	  send_to_char ("Sorry, you can't buy that here.\n\r", ch);
	  return;
	}

      if (ch->pet != NULL)
	{
	  send_to_char ("You already own a pet.\n\r", ch);
	  return;
	}

      cost = 10 * pet->level * pet->level;

      if ((ch->silver + (100 * ch->gold) + (10000 * ch->platinum)) < cost)
	{
	  send_to_char ("You can't afford it.\n\r", ch);
	  return;
	}

      if (ch->level < pet->level)
	{
	  send_to_char ("You're not powerful enough to master this pet.\n\r",
			ch);
	  return;
	}

      /* haggle */
      roll = number_percent ();
      if (roll < get_skill (ch, gsn_haggle))
	{
	  cost -= cost / 2 * roll / 100;
	  snprintf (buf, sizeof (buf), "You haggle the price down to `g%d`x coins.\n\r",
		   cost);
	  send_to_char (buf, ch);
	  check_improve (ch, gsn_haggle, TRUE, 4);

	}

      deduct_cost (ch, cost, VALUE_SILVER);
      pet = create_mobile (pet->pIndexData);
      SET_BIT (pet->act, ACT_PET);
      SET_BIT (pet->affected_by, AFF_CHARM);
      pet->comm = COMM_NOTELL | COMM_NOSHOUT | COMM_NOCHANNELS;

      argument = one_argument (argument, arg);
      if (arg[0] != '\0')
	{
	  snprintf (buf, sizeof (buf), "%s %s", pet->name, arg);
	  free_string (pet->name);
	  pet->name = str_dup (buf);
	}

      snprintf (buf, sizeof (buf), "%sA neck tag says '`cI belong to %s`x'.\n\r",
	       pet->description, ch->name);
      free_string (pet->description);
      pet->description = str_dup (buf);

      char_to_room (pet, ch->in_room);
      add_follower (pet, ch);
      pet->leader = ch;
      ch->pet = pet;
      pet->alignment = ch->alignment;
      send_to_char ("Enjoy your pet.\n\r", ch);
      act ("$n bought $N as a pet.", ch, NULL, pet, TO_ROOM);
      return;
    }
  else
    {
      CHAR_DATA *keeper;
      OBJ_DATA *obj, *t_obj;
      char arg[MAX_INPUT_LENGTH];
      int number, count = 1;

      if ((keeper = find_keeper (ch)) == NULL)
	return;

      number = mult_argument (argument, arg);
      obj = get_obj_keeper (ch, keeper, arg);
      cost = get_cost (keeper, obj, TRUE);

      if (cost <= 0 || !can_see_obj (ch, obj))
	{
	  act ("$n tells you '`aI don't sell that -- try '`Mlist`a'`x'.",
	       keeper, NULL, ch, TO_VICT);
	  ch->reply = keeper;
	  return;
	}

      if (number < 0)
	{
	  act ("$n tells you '`aNice try, jackass!`x'.",
	       keeper, NULL, ch, TO_VICT);
	  ch->reply = keeper;
	  multi_hit (keeper, ch, TYPE_UNDEFINED);
	  return;
	}
      if (number == 0)
	number = 1;

      if (!IS_OBJ_STAT (obj, ITEM_INVENTORY))
	{
	  for (t_obj = obj->next_content;
	       count < number && t_obj != NULL; t_obj = t_obj->next_content)
	    {
	      if (t_obj->pIndexData == obj->pIndexData
		  && !str_cmp (t_obj->short_descr, obj->short_descr))
		count++;
	      else
		break;
	    }

	  if (count < number)
	    {
	      act ("$n tells you '`aI don't have that many in stock`x'.",
		   keeper, NULL, ch, TO_VICT);
	      ch->reply = keeper;
	      return;
	    }
	}

      if ((ch->silver + (ch->gold * 100) + (ch->platinum * 10000)) <
	  cost * number)
	{
	  if (number > 1)
	    act ("$n tells you '`aYou can't afford to buy that many`x'.",
		 keeper, obj, ch, TO_VICT);
	  else
	    act ("$n tells you '`aYou can't afford to buy $p`x'.",
		 keeper, obj, ch, TO_VICT);
	  ch->reply = keeper;
	  return;
	}

      if (((obj->level > ch->level)
	   && (ch->class < MAX_CLASS / 2)
	   && (obj->level > 19))
	  || ((obj->level > ch->level)
	      && (ch->class >= MAX_CLASS / 2) && (obj->level > 27)))
	{
	  act ("$n tells you '`aYou can't use $p `ayet`x'.",
	       keeper, obj, ch, TO_VICT);
	  ch->reply = keeper;
	  return;
	}

      if (ch->carry_number + number * get_obj_number (obj) > can_carry_n (ch))
	{
	  send_to_char ("You can't carry that many items.\n\r", ch);
	  return;
	}

      if (ch->carry_weight + number * get_obj_weight (obj) > can_carry_w (ch))
	{
	  send_to_char ("You can't carry that much weight.\n\r", ch);
	  return;
	}

      /* haggle */
      roll = number_percent ();
      if (!IS_OBJ_STAT (obj, ITEM_SELL_EXTRACT)
	  && roll < get_skill (ch, gsn_haggle))
	{
	  cost -= obj->cost / 2 * roll / 100;
	  act ("You haggle with $N.", ch, NULL, keeper, TO_CHAR);
	  check_improve (ch, gsn_haggle, TRUE, 4);
	}

      if (number > 1)
	{
	  snprintf (buf, sizeof (buf), "$n buys $p[%d].", number);
	  act (buf, ch, obj, NULL, TO_ROOM);
	  snprintf (buf, sizeof (buf), "You buy $p[%d] for `g%d`x silver.", number,
		   cost * number);
	  act (buf, ch, obj, NULL, TO_CHAR);
	}
      else
	{
	  act ("$n buys $p.", ch, obj, NULL, TO_ROOM);
	  snprintf (buf, sizeof (buf), "You buy $p for `g%d`x silver.", cost);
	  act (buf, ch, obj, NULL, TO_CHAR);
	}
      multicost = cost * number;
      while (multicost >= 100000)
	{
	  deduct_cost (ch, 10, VALUE_PLATINUM);
	  add_cost (keeper, 10, VALUE_PLATINUM);
	  multicost -= 100000;
	}
      while (multicost >= 10000)
	{
	  deduct_cost (ch, 1, VALUE_PLATINUM);
	  add_cost (keeper, 1, VALUE_PLATINUM);
	  multicost -= 10000;
	}
      while (multicost >= 1000)
	{
	  deduct_cost (ch, 10, VALUE_GOLD);
	  add_cost (keeper, 10, VALUE_GOLD);
	  multicost -= 1000;
	}
      while (multicost >= 100)
	{
	  deduct_cost (ch, 1, VALUE_GOLD);
	  add_cost (keeper, 1, VALUE_GOLD);
	  multicost -= 100;
	}
      if (multicost > 0)
	{
	  roll = multicost;
	  deduct_cost (ch, roll, VALUE_SILVER);
	  add_cost (keeper, roll, VALUE_SILVER);
	}

      for (count = 0; count < number; count++)
	{
	  if (IS_SET (obj->extra_flags, ITEM_INVENTORY))
	    t_obj = create_object (obj->pIndexData, obj->level);
	  else
	    {
	      t_obj = obj;
	      obj = obj->next_content;
	      obj_from_char (t_obj);
	    }

	  if (t_obj->timer > 0 && !IS_OBJ_STAT (t_obj, ITEM_HAD_TIMER))
	    t_obj->timer = 0;
	  REMOVE_BIT (t_obj->extra_flags, ITEM_HAD_TIMER);
	  obj_to_char (t_obj, ch);
	  if (cost < t_obj->cost)
	    t_obj->cost = cost;
	}
    }
}



void
do_list (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];

  if (IS_SET (ch->in_room->room_flags, ROOM_PET_SHOP))
    {
      ROOM_INDEX_DATA *pRoomIndexNext;
      CHAR_DATA *pet;
      bool found;

      /* hack to make new thalos pets work */
      if (ch->in_room->vnum == 9621)
	pRoomIndexNext = get_room_index (9706);
      else
	pRoomIndexNext = get_room_index (ch->in_room->vnum + 1);

      if (pRoomIndexNext == NULL)
	{
	  bug ("Do_list: bad pet shop at vnum %d.", ch->in_room->vnum);
	  send_to_char ("You can't do that here.\n\r", ch);
	  return;
	}

      found = FALSE;
      for (pet = pRoomIndexNext->people; pet; pet = pet->next_in_room)
	{
	  if (IS_SET (pet->act, ACT_PET))
	    {
	      if (!found)
		{
		  found = TRUE;
		  send_to_char ("Pets for sale:\n\r", ch);
		}
	      snprintf (buf, sizeof (buf), "[%2d] %8d - %s\n\r",
		       pet->level,
		       10 * pet->level * pet->level, pet->short_descr);
	      send_to_char (buf, ch);
	    }
	}
      if (!found)
	send_to_char ("Sorry, we're out of pets right now.\n\r", ch);
      return;
    }
  else
    {
      CHAR_DATA *keeper;
      OBJ_DATA *obj;
      int cost, count;
      bool found;
      char arg[MAX_INPUT_LENGTH];

      if ((keeper = find_keeper (ch)) == NULL)
	return;
      one_argument (argument, arg);

      found = FALSE;
      for (obj = keeper->carrying; obj; obj = obj->next_content)
	{
	  if (obj->wear_loc == WEAR_NONE
	      && can_see_obj (ch, obj)
	      && (cost = get_cost (keeper, obj, TRUE)) > 0
	      && (arg[0] == '\0' || is_name (arg, obj->name)))
	    {
	      if (!found)
		{
		  found = TRUE;
		  send_to_char ("[Lv Price Qty] Item\n\r", ch);
		}

	      if (IS_OBJ_STAT (obj, ITEM_INVENTORY))
		snprintf (buf, sizeof (buf), "[%2d %5d -- ] %s\n\r",
			 obj->level, cost, obj->short_descr);
	      else
		{
		  count = 1;

		  while (obj->next_content != NULL
			 && obj->pIndexData == obj->next_content->pIndexData
			 && !str_cmp (obj->short_descr,
				      obj->next_content->short_descr))
		    {
		      obj = obj->next_content;
		      count++;
		    }
		  snprintf (buf, sizeof (buf), "[%2d %5d %2d ] %s\n\r",
			   obj->level, cost, count, obj->short_descr);
		}
	      send_to_char (buf, ch);
	    }
	}

      if (!found)
	send_to_char ("You can't buy anything here.\n\r", ch);
      return;
    }
}



void
do_sell (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *keeper;
  OBJ_DATA *obj;
  int cost, roll;

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Sell what?\n\r", ch);
      return;
    }

  if ((keeper = find_keeper (ch)) == NULL)
    return;

  if ((obj = get_obj_carry (ch, arg)) == NULL)
    {
      act ("$n tells you '`aYou don't have that item`x'.",
	   keeper, NULL, ch, TO_VICT);
      ch->reply = keeper;
      return;
    }

  if (!can_drop_obj (ch, obj))
    {
      send_to_char ("`RYou can't let go of it`z!!`x\n\r", ch);
      return;
    }

  if (!can_see_obj (keeper, obj))
    {
      act ("$n doesn't see what you are offering.", keeper, NULL, ch,
	   TO_VICT);
      return;
    }

  if ((cost = get_cost (keeper, obj, FALSE)) <= 0)
    {
      act ("$n looks uninterested in $p.", keeper, obj, ch, TO_VICT);
      return;
    }
  if (cost >
      (keeper->silver + (100 * keeper->gold) + (10000 * keeper->platinum)))
    {
      act
	("$n tells you '`aI'm afraid I don't have enough wealth to buy $p`x'.",
	 keeper, obj, ch, TO_VICT);
      return;
    }

  act ("$n sells $p.", ch, obj, NULL, TO_ROOM);
  /* haggle */
  roll = number_percent ();
  if (!IS_OBJ_STAT (obj, ITEM_SELL_EXTRACT)
      && roll < get_skill (ch, gsn_haggle))
    {
      send_to_char ("You haggle with the shopkeeper.\n\r", ch);
      cost += obj->cost / 2 * roll / 100;
      cost = UMIN (cost, 95 * get_cost (keeper, obj, TRUE) / 100);
      cost =
	UMIN (cost,
	      (keeper->silver + (100 * keeper->gold) +
	       (10000 * keeper->platinum)));
      check_improve (ch, gsn_haggle, TRUE, 4);
    }
  snprintf (buf, sizeof (buf), "You sell $p for `g%d`x silver piece%s.",
	   cost, cost == 1 ? "" : "s");
  act (buf, ch, obj, NULL, TO_CHAR);

  while (cost >= 10000)
    {
      deduct_cost (keeper, 1, VALUE_PLATINUM);
      add_cost (ch, 1, VALUE_PLATINUM);
      cost -= 10000;
    }
  while (cost >= 1000)
    {
      deduct_cost (keeper, 10, VALUE_GOLD);
      add_cost (ch, 10, VALUE_GOLD);
      cost -= 1000;
    }
  while (cost >= 100)
    {
      deduct_cost (keeper, 1, VALUE_GOLD);
      add_cost (ch, 1, VALUE_GOLD);
      cost -= 100;
    }
  if (cost > 0)
    {
      deduct_cost (keeper, cost, VALUE_SILVER);
      add_cost (ch, cost, VALUE_SILVER);
    }

  if (obj->item_type == ITEM_TRASH || IS_OBJ_STAT (obj, ITEM_SELL_EXTRACT))
    {
      extract_obj (obj);
    }
  else
    {
      obj_from_char (obj);
      if (obj->timer)
	SET_BIT (obj->extra_flags, ITEM_HAD_TIMER);
      else
	obj->timer = number_range (50, 100);
      obj_to_keeper (obj, keeper);
    }

  return;
}



void
do_value (CHAR_DATA * ch, char *argument)
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *keeper;
  OBJ_DATA *obj;
  int cost;

  one_argument (argument, arg);

  if (arg[0] == '\0')
    {
      send_to_char ("Value what?\n\r", ch);
      return;
    }

  if ((keeper = find_keeper (ch)) == NULL)
    return;

  if ((obj = get_obj_carry (ch, arg)) == NULL)
    {
      act ("$n tells you '`aYou don't have that item`x'.",
	   keeper, NULL, ch, TO_VICT);
      ch->reply = keeper;
      return;
    }

  if (!can_see_obj (keeper, obj))
    {
      act ("$n doesn't see what you are offering.", keeper, NULL, ch,
	   TO_VICT);
      return;
    }

  if (!can_drop_obj (ch, obj))
    {
      send_to_char ("You can't let go of it.\n\r", ch);
      return;
    }

  if ((cost = get_cost (keeper, obj, FALSE)) <= 0)
    {
      act ("$n looks uninterested in $p.", keeper, obj, ch, TO_VICT);
      return;
    }

  snprintf (buf, sizeof (buf),
	   "$n tells you '`aI'll give you `g%d`a silver coin%s for $p`x'.",
	   cost, cost == 1 ? "" : "s");
  act (buf, keeper, obj, ch, TO_VICT);
  ch->reply = keeper;

  return;
}

