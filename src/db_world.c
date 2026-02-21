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

#include <stdio.h>
#include <string.h>
#include "merc.h"
#include "db.h"
#include "tables.h"


extern OBJ_DATA *obj_free;
extern CHAR_DATA *char_free;
extern DESCRIPTOR_DATA *descriptor_free;
extern PC_DATA *pcdata_free;
extern AFFECT_DATA *affect_free;
extern int top_vnum;


void
do_dump (CHAR_DATA * ch, char *argument)
{
  int count, count2, num_pcs, aff_count;
  CHAR_DATA *fch;
  MOB_INDEX_DATA *pMobIndex;
  PC_DATA *pc;
  OBJ_DATA *obj;
  OBJ_INDEX_DATA *pObjIndex;
  ROOM_INDEX_DATA *room;
  EXIT_DATA *exit;
  DESCRIPTOR_DATA *d;
  AFFECT_DATA *af;
  FILE *fp;
  int vnum, nMatch = 0;

  /* open file */
  fclose (fpReserve);
  fp = fopen ("mem.dmp", "w");

  /* report use of data structures */

  num_pcs = 0;
  aff_count = 0;

  /* mobile prototypes */
  fprintf (fp, "MobProt	%4d (%8lu bytes)\n",
	   top_mob_index, top_mob_index * (sizeof (*pMobIndex)));

  /* mobs */
  count = 0;
  count2 = 0;
  for (fch = char_list; fch != NULL; fch = fch->next)
    {
      count++;
      if (fch->pcdata != NULL)
	num_pcs++;
      for (af = fch->affected; af != NULL; af = af->next)
	aff_count++;
    }
  for (fch = char_free; fch != NULL; fch = fch->next)
    count2++;

  fprintf (fp, "Mobs	%4d (%8lu bytes), %2d free (%lu bytes)\n",
	   count, count * (sizeof (*fch)), count2, count2 * (sizeof (*fch)));

  /* pcdata */
  count = 0;
  for (pc = pcdata_free; pc != NULL; pc = pc->next)
    count++;

  fprintf (fp, "Pcdata	%4d (%8lu bytes), %2d free (%lu bytes)\n",
	   num_pcs, num_pcs * (sizeof (*pc)), count, count * (sizeof (*pc)));

  /* descriptors */
  count = 0;
  count2 = 0;
  for (d = descriptor_list; d != NULL; d = d->next)
    count++;
  for (d = descriptor_free; d != NULL; d = d->next)
    count2++;

  fprintf (fp, "Descs	%4d (%8lu bytes), %2d free (%lu bytes)\n",
	   count, count * (sizeof (*d)), count2, count2 * (sizeof (*d)));

  /* object prototypes */
  for (vnum = 0; nMatch < top_obj_index; vnum++)
    if ((pObjIndex = get_obj_index (vnum)) != NULL)
      {
	for (af = pObjIndex->affected; af != NULL; af = af->next)
	  aff_count++;
	nMatch++;
      }

  fprintf (fp, "ObjProt	%4d (%8lu bytes)\n",
	   top_obj_index, top_obj_index * (sizeof (*pObjIndex)));


  /* objects */
  count = 0;
  count2 = 0;
  for (obj = object_list; obj != NULL; obj = obj->next)
    {
      count++;
      for (af = obj->affected; af != NULL; af = af->next)
	aff_count++;
    }
  for (obj = obj_free; obj != NULL; obj = obj->next)
    count2++;

  fprintf (fp, "Objs	%4d (%8lu bytes), %2d free (%lu bytes)\n",
	   count, count * (sizeof (*obj)), count2, count2 * (sizeof (*obj)));

  /* affects */
  count = 0;
  for (af = affect_free; af != NULL; af = af->next)
    count++;

  fprintf (fp, "Affects	%4d (%8lu bytes), %2d free (%lu bytes)\n",
	   aff_count, aff_count * (sizeof (*af)), count,
	   count * (sizeof (*af)));

  /* rooms */
  fprintf (fp, "Rooms	%4d (%8lu bytes)\n",
	   top_room, top_room * (sizeof (*room)));

  /* exits */
  fprintf (fp, "Exits	%4d (%8lu bytes)\n",
	   top_exit, top_exit * (sizeof (*exit)));

  fclose (fp);

  /* start printing out mobile data */
  fp = fopen ("mob.dmp", "w");

  fprintf (fp, "\nMobile Analysis\n");
  fprintf (fp, "---------------\n");
  nMatch = 0;
  for (vnum = 0; nMatch < top_mob_index; vnum++)
    if ((pMobIndex = get_mob_index (vnum)) != NULL)
      {
	nMatch++;
	fprintf (fp, "#%-4d %3d active %3d killed     %s\n",
		 pMobIndex->vnum, pMobIndex->count,
		 pMobIndex->killed, pMobIndex->short_descr);
      }
  fclose (fp);

  /* start printing out object data */
  fp = fopen ("obj.dmp", "w");

  fprintf (fp, "\nObject Analysis\n");
  fprintf (fp, "---------------\n");
  nMatch = 0;
  for (vnum = 0; nMatch < top_obj_index; vnum++)
    if ((pObjIndex = get_obj_index (vnum)) != NULL)
      {
	nMatch++;
	fprintf (fp, "#%-4d %3d active %3d reset      %s\n",
		 pObjIndex->vnum, pObjIndex->count,
		 pObjIndex->reset_num, pObjIndex->short_descr);
      }

  /* close file */
  fclose (fp);
  fpReserve = fopen (NULL_FILE, "r");
}





void
randomize_entrances (int code)
{
  char buf[MAX_STRING_LENGTH];
  ROOM_INDEX_DATA *pRoomIndex;
  ROOM_INDEX_DATA *pToRoomIndex;
  OBJ_DATA *portal;
  OBJ_DATA *toportal;
  EXIT_DATA *pexit;
  int clannum, door, todoor;
  sh_int room, toroom;

  if (code == 0)
    chain = 3120;

  if ((code == 0) || (code == ROOM_VNUM_CLANS))
    {
      for (clannum = 0; clannum < MAX_CLAN; clannum++)
	{
	  room = clan_table[clannum].entrance;

	  if (room == ROOM_VNUM_ALTAR)
	    continue;

	  if ((pRoomIndex = get_room_index (room)) == NULL)
	    {
	      bug ("Clan Entrance: bad vnum %d.", room);
	      continue;
	    }
	  for (door = 0; door < 6; door++)
	    {
	      if (door == 5)
		todoor = 4;
	      else if (door == 4)
		todoor = 5;
	      else if (door < 2)
		todoor = door + 2;
	      else
		todoor = door - 2;
	      portal = get_obj_exit (dir_name[door], pRoomIndex->contents);
	      if ((portal != NULL) && (portal->item_type == ITEM_EXIT))
		{
		  pToRoomIndex = get_room_index (portal->value[0]);
		  if (pToRoomIndex != NULL)
		    {
		      toportal =
			get_obj_exit (dir_name[todoor],
				      pToRoomIndex->contents);
		      if ((toportal != NULL)
			  && (toportal->item_type == ITEM_EXIT))
			extract_obj (toportal);
		    }
		  extract_obj (portal);
		}
	    }
	  for (;;)
	    {
	      door = number_range (0, 5);
	      if (door == 5)
		todoor = 4;
	      else if (door == 4)
		todoor = 5;
	      else if (door < 2)
		todoor = door + 2;
	      else
		todoor = door - 2;

	      if ((pexit = pRoomIndex->exit[door]) == NULL)
		{
		  for (;;)
		    {
		      pToRoomIndex =
			get_room_index (number_range (0, top_vnum));
		      if (pToRoomIndex != NULL)
			{
			  if (!IS_SET (pToRoomIndex->room_flags, ROOM_PRIVATE)
			      && !IS_SET (pToRoomIndex->room_flags, ROOM_SAFE)
			      && !IS_SET (pToRoomIndex->room_flags,
					  ROOM_SOLITARY)
			      && !IS_SET (pToRoomIndex->room_flags,
					  ROOM_IMP_ONLY)
			      && !IS_SET (pToRoomIndex->room_flags,
					  ROOM_GODS_ONLY)
			      && !IS_SET (pToRoomIndex->room_flags,
					  ROOM_HEROES_ONLY)
			      && !IS_SET (pToRoomIndex->room_flags,
					  ROOM_NEWBIES_ONLY)
			      && !IS_SET (pToRoomIndex->room_flags, ROOM_LAW)
			      && !IS_SET (pToRoomIndex->room_flags,
					  ROOM_NOWHERE)
			      && !IS_SET (pToRoomIndex->room_flags,
					  ROOM_LOCKED)
			      && (pToRoomIndex->vnum != ROOM_VNUM_CHAIN)
			      && (pToRoomIndex->exit[todoor] == NULL)
			      && (pToRoomIndex->exit[todoor + 6] == NULL))
			    {
			      portal =
				get_obj_exit ("exit", pRoomIndex->contents);
			      if (portal == NULL)
				break;
			    }
			}
		    }
		  portal = create_object (get_obj_index (OBJ_VNUM_EXIT), 1);
		  snprintf (buf, sizeof (buf), "exit %s", dir_name[door]);
		  free_string (portal->name);
		  portal->name = str_dup (buf);
		  free_string (portal->short_descr);
		  portal->short_descr = str_dup (dir_name[door]);
		  portal->value[0] = pToRoomIndex->vnum;
		  obj_to_room (portal, pRoomIndex);
		  toportal = create_object (get_obj_index (OBJ_VNUM_EXIT), 1);
		  snprintf (buf, sizeof (buf), "exit %s", dir_name[todoor]);
		  free_string (toportal->name);
		  toportal->name = str_dup (buf);
		  free_string (toportal->short_descr);
		  toportal->short_descr = str_dup (dir_name[todoor]);
		  toportal->value[0] = pRoomIndex->vnum;
		  obj_to_room (toportal, pToRoomIndex);
		  buf[0] = '\0';
		  break;
		}
	    }
	}
      if (code != 0)
	{
	  return;
	}
    }
  if ((code == 0) || (code == ROOM_VNUM_CHAIN))
    {
      CHAR_DATA *rch;

      if (code == ROOM_VNUM_CHAIN)
	{
	  if (number_range (0, 1000) < 500)
	    return;
	}
      room = ROOM_VNUM_CHAIN;
      if ((pRoomIndex = get_room_index (room)) == NULL)
	{
	  bug ("Chain Room: bad vnum %d.", room);
	  return;
	}
      door = 5;
      todoor = 4;
      portal = get_obj_exit (dir_name[door], pRoomIndex->contents);
      if ((portal != NULL) && (portal->item_type == ITEM_EXIT))
	{
	  toroom = portal->value[0];
	  pToRoomIndex = get_room_index (toroom);
	  if (pToRoomIndex != NULL)
	    {
	      toportal = get_obj_exit ("chain", pToRoomIndex->contents);
	      if ((toportal != NULL) && (toportal->item_type == ITEM_EXIT))
		extract_obj (toportal);
	    }
	  extract_obj (portal);
	}
      else
	{
	  toroom = 3120;
	  chain = 3120;
	}
      pToRoomIndex = get_room_index (toroom);
      for (;;)
	{
	  door = number_range (0, 3);
	  if ((pexit = pToRoomIndex->exit[door]) != NULL)
	    {
	      if (((pexit->u1.to_room->vnum >= 3100)
		   && (pexit->u1.to_room->vnum <= 3141)
		   && (pexit->u1.to_room->vnum != 3106)
		   && (pexit->u1.to_room->vnum != 3110)
		   && (pexit->u1.to_room->vnum != 3114)
		   && (pexit->u1.to_room->vnum != 3137)
		   && (pexit->u1.to_room->vnum != 3138))
		  || ((pexit->u1.to_room->vnum >= 3270)
		      && (pexit->u1.to_room->vnum <= 3273))
		  || (pexit->u1.to_room->vnum == 3144)
		  || (pexit->u1.to_room->vnum == 3255))
		{
		  toroom = pexit->u1.to_room->vnum;
		  break;
		}
	    }
	}
      if (door < 2)
	todoor = door + 2;
      else
	todoor = door - 2;

      snprintf (buf, sizeof (buf), "The chain drifts off to the %s.\n\r", dir_name[door]);
      for (rch = pToRoomIndex->people; rch != NULL; rch = rch->next_in_room)
	send_to_char (buf, rch);

      pToRoomIndex = get_room_index (toroom);
      snprintf (buf, sizeof (buf), "A chain drifts in from the %s.\n\r", dir_name[todoor]);
      for (rch = pToRoomIndex->people; rch != NULL; rch = rch->next_in_room)
	send_to_char (buf, rch);

      chain = toroom;
      door = 5;
      todoor = 4;
      portal = create_object (get_obj_index (OBJ_VNUM_EXIT), 1);
      snprintf (buf, sizeof (buf), "exit %s", dir_name[door]);
      free_string (portal->name);
      portal->name = str_dup (buf);
      free_string (portal->short_descr);
      portal->short_descr = str_dup (dir_name[door]);
      portal->value[0] = pToRoomIndex->vnum;
      obj_to_room (portal, pRoomIndex);
      toportal = create_object (get_obj_index (OBJ_VNUM_CHAIN), 1);
      obj_to_room (toportal, pToRoomIndex);
      buf[0] = '\0';
      if (code != 0)
	{
	  return;
	}
    }
  return;
}
