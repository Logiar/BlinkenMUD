/***************************************************************************
 *  File: olc_medit.c                                                      *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/



#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "olc.h"
#include "tables.h"
#include "recycle.h"
#include "lookup.h"

/* Return TRUE if area changed, FALSE if not. */
#define REDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define OEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define MEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )
#define AEDIT( fun )		bool fun( CHAR_DATA *ch, char *argument )


/*
 * Mobile Editor Functions.
 */
MEDIT (medit_show)
{
  MOB_INDEX_DATA *pMob;
  char buf[MAX_STRING_LENGTH];
  MPROG_LIST *list;

  EDIT_MOB (ch, pMob);

  snprintf (buf, sizeof (buf), "Name:        [%s]\n\rArea:        [%5d] %s\n\r",
	   pMob->player_name,
	   !pMob->area ? -1 : pMob->area->vnum,
	   !pMob->area ? "No Area" : pMob->area->name);
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf), "Act:         [%s]\n\r", flag_string (act_flags, pMob->act));
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf), "Vnum:        [%5d]\n\rSex:         [%s]\n\r", pMob->vnum, pMob->sex == SEX_MALE ? "male" : pMob->sex == SEX_FEMALE ? "female" : pMob->sex == 3 ? "random" : "neutral");	/* ROM magic number */
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf), "Group:       [%5d]\n\r", pMob->group);
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf), "Race:        [%s]\n\r",	/* ROM OLC */
	   race_table[pMob->race].name);
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf),
	   "Level:       [%2d]\n\rAlign:       [%4d]\n\r",
	   pMob->level, pMob->alignment);
  send_to_char (buf, ch);

/* ROM values: */

  snprintf (buf, sizeof (buf), "Hitroll:     [%d]\n\r", pMob->hitroll);
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf), "Dam Type:    [%s]\n\r", attack_table[pMob->dam_type].name);
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf), "Hit dice:    [%2dd%-3d+%4d]\n\r",
	   pMob->hit[DICE_NUMBER],
	   pMob->hit[DICE_TYPE], pMob->hit[DICE_BONUS]);
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf), "Damage dice: [%2dd%-3d+%4d]\n\r",
	   pMob->damage[DICE_NUMBER],
	   pMob->damage[DICE_TYPE], pMob->damage[DICE_BONUS]);
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf), "Mana dice:   [%2dd%-3d+%4d]\n\r",
	   pMob->mana[DICE_NUMBER],
	   pMob->mana[DICE_TYPE], pMob->mana[DICE_BONUS]);
  send_to_char (buf, ch);

/* ROM values end */

  snprintf (buf, sizeof (buf), "Affected by: [%s]\n\r",
	   flag_string (affect_flags, pMob->affected_by));
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf), "Shields    : [%s]\n\r",
	   flag_string (shield_flags, pMob->shielded_by));
  send_to_char (buf, ch);

/* ROM values: */

  snprintf (buf, sizeof (buf),
	   "Armor:       [pierce: %d  bash: %d  slash: %d  magic: %d]\n\r",
	   pMob->ac[AC_PIERCE], pMob->ac[AC_BASH], pMob->ac[AC_SLASH],
	   pMob->ac[AC_EXOTIC]);
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf), "Form:        [%s]\n\r",
	   flag_string (form_flags, pMob->form));
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf), "Parts:       [%s]\n\r",
	   flag_string (part_flags, pMob->parts));
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf), "Imm:         [%s]\n\r",
	   flag_string (imm_flags, pMob->imm_flags));
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf), "Res:         [%s]\n\r",
	   flag_string (res_flags, pMob->res_flags));
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf), "Vuln:        [%s]\n\r",
	   flag_string (vuln_flags, pMob->vuln_flags));
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf), "Off:         [%s]\n\r",
	   flag_string (off_flags, pMob->off_flags));
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf), "Size:        [%s]\n\r",
	   flag_string (size_flags, pMob->size));
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf), "Material:    [%s]\n\r", pMob->material);
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf), "Start pos.   [%s]\n\r",
	   flag_string (position_flags, pMob->start_pos));
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf), "Default pos  [%s]\n\r",
	   flag_string (position_flags, pMob->default_pos));
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf), "Wealth:      [%5ld]\n\r", pMob->wealth);
  send_to_char (buf, ch);

/* ROM values end */

  if (pMob->spec_fun)
    {
      snprintf (buf, sizeof (buf), "Spec fun:    [%s]\n\r", spec_name (pMob->spec_fun));
      send_to_char (buf, ch);
    }

  snprintf (buf, sizeof (buf), "Short descr: %s\n\rLong descr:\n\r%s",
	   pMob->short_descr, pMob->long_descr);
  send_to_char (buf, ch);

  snprintf (buf, sizeof (buf), "Description:\n\r%s", pMob->description);
  send_to_char (buf, ch);

  if (pMob->pShop)
    {
      SHOP_DATA *pShop;
      int iTrade;

      pShop = pMob->pShop;

      snprintf (buf, sizeof (buf),
	       "Shop data for [%5d]:\n\r"
	       "  Markup for purchaser: %d%%\n\r"
	       "  Markdown for seller:  %d%%\n\r",
	       pShop->keeper, pShop->profit_buy, pShop->profit_sell);
      send_to_char (buf, ch);
      snprintf (buf, sizeof (buf), "  Hours: %d to %d.\n\r",
	       pShop->open_hour, pShop->close_hour);
      send_to_char (buf, ch);

      for (iTrade = 0; iTrade < MAX_TRADE; iTrade++)
	{
	  if (pShop->buy_type[iTrade] != 0)
	    {
	      if (iTrade == 0)
		{
		  send_to_char ("  Number Trades Type\n\r", ch);
		  send_to_char ("  ------ -----------\n\r", ch);
		}
	      snprintf (buf, sizeof (buf), "  [%4d] %s\n\r", iTrade,
		       flag_string (type_flags, pShop->buy_type[iTrade]));
	      send_to_char (buf, ch);
	    }
	}
    }

  if (pMob->mprogs)
    {
      int cnt;

      snprintf (buf, sizeof (buf), "\n\rMOBPrograms for [%5d]:\n\r", pMob->vnum);
      send_to_char (buf, ch);

      for (cnt = 0, list = pMob->mprogs; list; list = list->next)
	{
	  if (cnt == 0)
	    {
	      send_to_char (" Number Vnum Trigger Phrase\n\r", ch);
	      send_to_char (" ------ ---- ------- ------\n\r", ch);
	    }

	  snprintf (buf, sizeof (buf), "[%5d] %4d %7s %s\n\r", cnt,
		   list->vnum, mprog_type_to_name (list->trig_type),
		   list->trig_phrase);
	  send_to_char (buf, ch);
	  cnt++;
	}
    }

  return FALSE;
}



MEDIT (medit_create)
{
  MOB_INDEX_DATA *pMob;
  AREA_DATA *pArea;
  int value;
  int iHash;

  value = atoi (argument);
  if (argument[0] == '\0' || value == 0)
    {
      send_to_char ("Syntax:  medit create [vnum]\n\r", ch);
      return FALSE;
    }

  pArea = get_vnum_area (value);

  if (!pArea)
    {
      send_to_char ("MEdit:  That vnum is not assigned an area.\n\r", ch);
      return FALSE;
    }

  if (!IS_BUILDER (ch, pArea))
    {
      send_to_char ("MEdit:  Vnum in an area you cannot build in.\n\r", ch);
      return FALSE;
    }

  if (get_mob_index (value))
    {
      send_to_char ("MEdit:  Mobile vnum already exists.\n\r", ch);
      return FALSE;
    }

  pMob = new_mob_index ();
  pMob->vnum = value;
  pMob->area = pArea;

  if (value > top_vnum_mob)
    top_vnum_mob = value;

  pMob->act = ACT_IS_NPC;
  iHash = value % MAX_KEY_HASH;
  pMob->next = mob_index_hash[iHash];
  mob_index_hash[iHash] = pMob;
  ch->desc->pEdit = (void *) pMob;

  send_to_char ("Mobile Created.\n\r", ch);
  return TRUE;
}



MEDIT (medit_spec)
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB (ch, pMob);

  if (argument[0] == '\0')
    {
      send_to_char ("Syntax:  spec [special function]\n\r", ch);
      return FALSE;
    }


  if (!str_cmp (argument, "none"))
    {
      pMob->spec_fun = NULL;

      send_to_char ("Spec removed.\n\r", ch);
      return TRUE;
    }

  if (spec_lookup (argument))
    {
      pMob->spec_fun = spec_lookup (argument);
      send_to_char ("Spec set.\n\r", ch);
      return TRUE;
    }

  send_to_char ("MEdit: No such special function.\n\r", ch);
  return FALSE;
}

MEDIT (medit_damtype)
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB (ch, pMob);

  if (argument[0] == '\0')
    {
      send_to_char ("Syntax:  damtype [damage message]\n\r", ch);
      send_to_char
	("Para ver una lista de tipos de mensajes, pon '? weapon'.\n\r", ch);
      return FALSE;
    }

  pMob->dam_type = attack_lookup (argument);
  send_to_char ("Damage type set.\n\r", ch);
  return TRUE;
}


MEDIT (medit_align)
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB (ch, pMob);

  if (argument[0] == '\0' || !is_number (argument))
    {
      send_to_char ("Syntax:  alignment [number]\n\r", ch);
      return FALSE;
    }

  pMob->alignment = atoi (argument);

  send_to_char ("Alignment set.\n\r", ch);
  return TRUE;
}



MEDIT (medit_level)
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB (ch, pMob);

  if (argument[0] == '\0' || !is_number (argument))
    {
      send_to_char ("Syntax:  level [number]\n\r", ch);
      return FALSE;
    }

  pMob->level = atoi (argument);

  send_to_char ("Level set.\n\r", ch);
  return TRUE;
}



MEDIT (medit_desc)
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB (ch, pMob);

  if (argument[0] == '\0')
    {
      string_append (ch, &pMob->description);
      return TRUE;
    }

  send_to_char ("Syntax:  desc    - line edit\n\r", ch);
  return FALSE;
}




MEDIT (medit_long)
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB (ch, pMob);

  if (argument[0] == '\0')
    {
      send_to_char ("Syntax:  long [string]\n\r", ch);
      return FALSE;
    }

  free_string (pMob->long_descr);
  {
    char long_descr[MAX_STRING_LENGTH];

    snprintf (long_descr, sizeof (long_descr), "%s\n\r", argument);
    pMob->long_descr = str_dup (long_descr);
  }
  pMob->long_descr[0] = UPPER (pMob->long_descr[0]);

  send_to_char ("Long description set.\n\r", ch);
  return TRUE;
}



MEDIT (medit_short)
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB (ch, pMob);

  if (argument[0] == '\0')
    {
      send_to_char ("Syntax:  short [string]\n\r", ch);
      return FALSE;
    }

  free_string (pMob->short_descr);
  pMob->short_descr = str_dup (argument);

  send_to_char ("Short description set.\n\r", ch);
  return TRUE;
}



MEDIT (medit_name)
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB (ch, pMob);

  if (argument[0] == '\0')
    {
      send_to_char ("Syntax:  name [string]\n\r", ch);
      return FALSE;
    }

  free_string (pMob->player_name);
  pMob->player_name = str_dup (argument);

  send_to_char ("Name set.\n\r", ch);
  return TRUE;
}

MEDIT (medit_shop)
{
  MOB_INDEX_DATA *pMob;
  char command[MAX_INPUT_LENGTH];
  char arg1[MAX_INPUT_LENGTH];

  argument = one_argument (argument, command);
  argument = one_argument (argument, arg1);

  EDIT_MOB (ch, pMob);

  if (command[0] == '\0')
    {
      send_to_char ("Syntax:  shop hours [#xopening] [#xclosing]\n\r", ch);
      send_to_char ("         shop profit [#xbuying%] [#xselling%]\n\r", ch);
      send_to_char ("         shop type [#x0-4] [item type]\n\r", ch);
      send_to_char ("         shop assign\n\r", ch);
      send_to_char ("         shop remove\n\r", ch);
      return FALSE;
    }


  if (!str_cmp (command, "hours"))
    {
      if (arg1[0] == '\0' || !is_number (arg1)
	  || argument[0] == '\0' || !is_number (argument))
	{
	  send_to_char ("Syntax:  shop hours [#xopening] [#xclosing]\n\r",
			ch);
	  return FALSE;
	}

      if (!pMob->pShop)
	{
	  send_to_char
	    ("MEdit:  Debes crear un shop primero (shop assign).\n\r", ch);
	  return FALSE;
	}

      pMob->pShop->open_hour = atoi (arg1);
      pMob->pShop->close_hour = atoi (argument);

      send_to_char ("Shop hours set.\n\r", ch);
      return TRUE;
    }


  if (!str_cmp (command, "profit"))
    {
      if (arg1[0] == '\0' || !is_number (arg1)
	  || argument[0] == '\0' || !is_number (argument))
	{
	  send_to_char ("Syntax:  shop profit [#xbuying%] [#xselling%]\n\r",
			ch);
	  return FALSE;
	}

      if (!pMob->pShop)
	{
	  send_to_char
	    ("MEdit:  Debes crear un shop primero (shop assign).\n\r", ch);
	  return FALSE;
	}

      pMob->pShop->profit_buy = atoi (arg1);
      pMob->pShop->profit_sell = atoi (argument);

      send_to_char ("Shop profit set.\n\r", ch);
      return TRUE;
    }


  if (!str_cmp (command, "type"))
    {
      char buf[MAX_INPUT_LENGTH];
      int value;

      if (arg1[0] == '\0' || !is_number (arg1) || argument[0] == '\0')
	{
	  send_to_char ("Syntax:  shop type [#x0-4] [item type]\n\r", ch);
	  return FALSE;
	}

      if (atoi (arg1) >= MAX_TRADE)
	{
	  snprintf (buf, sizeof (buf), "MEdit:  May sell %d items max.\n\r", MAX_TRADE);
	  send_to_char (buf, ch);
	  return FALSE;
	}

      if (!pMob->pShop)
	{
	  send_to_char
	    ("MEdit:  Debes crear un shop primero (shop assign).\n\r", ch);
	  return FALSE;
	}

      if ((value = flag_value (type_flags, argument)) == NO_FLAG)
	{
	  send_to_char ("MEdit:  That type of item is not known.\n\r", ch);
	  return FALSE;
	}

      pMob->pShop->buy_type[atoi (arg1)] = value;

      send_to_char ("Shop type set.\n\r", ch);
      return TRUE;
    }

  /* shop assign && shop delete by Phoenix */

  if (!str_prefix (command, "assign"))
    {
      if (pMob->pShop)
	{
	  send_to_char ("Mob already has a shop assigned to it.\n\r", ch);
	  return FALSE;
	}

      pMob->pShop = new_shop ();
      if (!shop_first)
	shop_first = pMob->pShop;
      if (shop_last)
	shop_last->next = pMob->pShop;
      shop_last = pMob->pShop;

      pMob->pShop->keeper = pMob->vnum;

      send_to_char ("New shop assigned to mobile.\n\r", ch);
      return TRUE;
    }

  if (!str_prefix (command, "remove"))
    {
      SHOP_DATA *pShop;

      pShop = pMob->pShop;
      pMob->pShop = NULL;

      if (pShop == shop_first)
	{
	  if (!pShop->next)
	    {
	      shop_first = NULL;
	      shop_last = NULL;
	    }
	  else
	    shop_first = pShop->next;
	}
      else
	{
	  SHOP_DATA *ipShop;

	  for (ipShop = shop_first; ipShop; ipShop = ipShop->next)
	    {
	      if (ipShop->next == pShop)
		{
		  if (!pShop->next)
		    {
		      shop_last = ipShop;
		      shop_last->next = NULL;
		    }
		  else
		    ipShop->next = pShop->next;
		}
	    }
	}

      free_shop (pShop);

      send_to_char ("Mobile is no longer a shopkeeper.\n\r", ch);
      return TRUE;
    }

  medit_shop (ch, "");
  return FALSE;
}


/* ROM medit functions: */


MEDIT (medit_sex)		/* Moved out of medit() due to naming conflicts -- Hugin */
{
  MOB_INDEX_DATA *pMob;
  int value;

  if (argument[0] != '\0')
    {
      EDIT_MOB (ch, pMob);

      if ((value = flag_value (sex_flags, argument)) != NO_FLAG)
	{
	  pMob->sex = value;

	  send_to_char ("Sex set.\n\r", ch);
	  return TRUE;
	}
    }

  send_to_char ("Syntax: sex [sex]\n\r"
		"Type '? sex' for a list of flags.\n\r", ch);
  return FALSE;
}


MEDIT (medit_act)		/* Moved out of medit() due to naming conflicts -- Hugin */
{
  MOB_INDEX_DATA *pMob;
  int value;

  if (argument[0] != '\0')
    {
      EDIT_MOB (ch, pMob);

      if ((value = flag_value (act_flags, argument)) != NO_FLAG)
	{
	  pMob->act ^= value;
	  SET_BIT (pMob->act, ACT_IS_NPC);

	  send_to_char ("Act flag toggled.\n\r", ch);
	  return TRUE;
	}
    }

  send_to_char ("Syntax: act [flag]\n\r"
		"Type '? act' for a list of flags.\n\r", ch);
  return FALSE;
}


MEDIT (medit_affect)		/* Moved out of medit() due to naming conflicts -- Hugin */
{
  MOB_INDEX_DATA *pMob;
  int value;

  if (argument[0] != '\0')
    {
      EDIT_MOB (ch, pMob);

      if ((value = flag_value (affect_flags, argument)) != NO_FLAG)
	{
	  pMob->affected_by ^= value;

	  send_to_char ("Affect flag toggled.\n\r", ch);
	  return TRUE;
	}
    }

  send_to_char ("Syntax: affect [flag]\n\r"
		"Type '? affect' for a list of flags.\n\r", ch);
  return FALSE;
}

MEDIT (medit_shield)
{
  MOB_INDEX_DATA *pMob;
  int value;

  if (argument[0] != '\0')
    {
      EDIT_MOB (ch, pMob);

      if ((value = flag_value (shield_flags, argument)) != NO_FLAG)
	{
	  pMob->shielded_by ^= value;

	  send_to_char ("Shield flag toggled.\n\r", ch);
	  return TRUE;
	}
    }
  send_to_char ("Syntax: shield [flag]\n\r"
		"Type '? shield' for a list of flags.\n\r", ch);
  return FALSE;
}


MEDIT (medit_ac)
{
  MOB_INDEX_DATA *pMob;
  char arg[MAX_INPUT_LENGTH];
  int pierce, bash, slash, exotic;

  do				/* So that I can use break and send the syntax in one place */
    {
      if (argument[0] == '\0')
	break;

      EDIT_MOB (ch, pMob);
      argument = one_argument (argument, arg);

      if (!is_number (arg))
	break;
      pierce = atoi (arg);
      argument = one_argument (argument, arg);

      if (arg[0] != '\0')
	{
	  if (!is_number (arg))
	    break;
	  bash = atoi (arg);
	  argument = one_argument (argument, arg);
	}
      else
	bash = pMob->ac[AC_BASH];

      if (arg[0] != '\0')
	{
	  if (!is_number (arg))
	    break;
	  slash = atoi (arg);
	  argument = one_argument (argument, arg);
	}
      else
	slash = pMob->ac[AC_SLASH];

      if (arg[0] != '\0')
	{
	  if (!is_number (arg))
	    break;
	  exotic = atoi (arg);
	}
      else
	exotic = pMob->ac[AC_EXOTIC];

      pMob->ac[AC_PIERCE] = pierce;
      pMob->ac[AC_BASH] = bash;
      pMob->ac[AC_SLASH] = slash;
      pMob->ac[AC_EXOTIC] = exotic;

      send_to_char ("Ac set.\n\r", ch);
      return TRUE;
    }
  while (FALSE);		/* Just do it once.. */

  send_to_char ("Syntax:  ac [ac-pierce [ac-bash [ac-slash [ac-exotic]]]]\n\r"
		"help MOB_AC  gives a list of reasonable ac-values.\n\r", ch);
  return FALSE;
}

MEDIT (medit_form)
{
  MOB_INDEX_DATA *pMob;
  int value;

  if (argument[0] != '\0')
    {
      EDIT_MOB (ch, pMob);

      if ((value = flag_value (form_flags, argument)) != NO_FLAG)
	{
	  pMob->form ^= value;
	  send_to_char ("Form toggled.\n\r", ch);
	  return TRUE;
	}
    }

  send_to_char ("Syntax: form [flags]\n\r"
		"Type '? form' for a list of flags.\n\r", ch);
  return FALSE;
}

MEDIT (medit_part)
{
  MOB_INDEX_DATA *pMob;
  int value;

  if (argument[0] != '\0')
    {
      EDIT_MOB (ch, pMob);

      if ((value = flag_value (part_flags, argument)) != NO_FLAG)
	{
	  pMob->parts ^= value;
	  send_to_char ("Parts toggled.\n\r", ch);
	  return TRUE;
	}
    }

  send_to_char ("Syntax: part [flags]\n\r"
		"Type '? part' for a list of flags.\n\r", ch);
  return FALSE;
}

MEDIT (medit_imm)
{
  MOB_INDEX_DATA *pMob;
  int value;

  if (argument[0] != '\0')
    {
      EDIT_MOB (ch, pMob);

      if ((value = flag_value (imm_flags, argument)) != NO_FLAG)
	{
	  pMob->imm_flags ^= value;
	  send_to_char ("Immunity toggled.\n\r", ch);
	  return TRUE;
	}
    }

  send_to_char ("Syntax: imm [flags]\n\r"
		"Type '? imm' for a list of flags.\n\r", ch);
  return FALSE;
}

MEDIT (medit_res)
{
  MOB_INDEX_DATA *pMob;
  int value;

  if (argument[0] != '\0')
    {
      EDIT_MOB (ch, pMob);

      if ((value = flag_value (res_flags, argument)) != NO_FLAG)
	{
	  pMob->res_flags ^= value;
	  send_to_char ("Resistance toggled.\n\r", ch);
	  return TRUE;
	}
    }

  send_to_char ("Syntax: res [flags]\n\r"
		"Type '? res' for a list of flags.\n\r", ch);
  return FALSE;
}

MEDIT (medit_vuln)
{
  MOB_INDEX_DATA *pMob;
  int value;

  if (argument[0] != '\0')
    {
      EDIT_MOB (ch, pMob);

      if ((value = flag_value (vuln_flags, argument)) != NO_FLAG)
	{
	  pMob->vuln_flags ^= value;
	  send_to_char ("Vulnerability toggled.\n\r", ch);
	  return TRUE;
	}
    }

  send_to_char ("Syntax: vuln [flags]\n\r"
		"Type '? vuln' for a list of flags.\n\r", ch);
  return FALSE;
}

MEDIT (medit_material)
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB (ch, pMob);

  if (argument[0] == '\0')
    {
      send_to_char ("Syntax:  material [string]\n\r", ch);
      return FALSE;
    }

  free_string (pMob->material);
  pMob->material = str_dup (argument);

  send_to_char ("Material set.\n\r", ch);
  return TRUE;
}

MEDIT (medit_off)
{
  MOB_INDEX_DATA *pMob;
  int value;

  if (argument[0] != '\0')
    {
      EDIT_MOB (ch, pMob);

      if ((value = flag_value (off_flags, argument)) != NO_FLAG)
	{
	  pMob->off_flags ^= value;
	  send_to_char ("Offensive behaviour toggled.\n\r", ch);
	  return TRUE;
	}
    }

  send_to_char ("Syntax: off [flags]\n\r"
		"Type '? off' for a list of flags.\n\r", ch);
  return FALSE;
}

MEDIT (medit_size)
{
  MOB_INDEX_DATA *pMob;
  int value;

  if (argument[0] != '\0')
    {
      EDIT_MOB (ch, pMob);

      if ((value = flag_value (size_flags, argument)) != NO_FLAG)
	{
	  pMob->size = value;
	  send_to_char ("Size set.\n\r", ch);
	  return TRUE;
	}
    }

  send_to_char ("Syntax: size [size]\n\r"
		"Type '? size' for a list of sizes.\n\r", ch);
  return FALSE;
}

MEDIT (medit_hitdice)
{
  static char syntax[] = "Syntax:  hitdice <number> d <type> + <bonus>\n\r";
  char *num, *type, *bonus, *cp;
  MOB_INDEX_DATA *pMob;

  EDIT_MOB (ch, pMob);

  if (argument[0] == '\0')
    {
      send_to_char (syntax, ch);
      return FALSE;
    }

  num = cp = argument;

  while (isdigit (*cp))
    ++cp;
  while (*cp != '\0' && !isdigit (*cp))
    *(cp++) = '\0';

  type = cp;

  while (isdigit (*cp))
    ++cp;
  while (*cp != '\0' && !isdigit (*cp))
    *(cp++) = '\0';

  bonus = cp;

  while (isdigit (*cp))
    ++cp;
  if (*cp != '\0')
    *cp = '\0';

  if ((!is_number (num) || atoi (num) < 1)
      || (!is_number (type) || atoi (type) < 1)
      || (!is_number (bonus) || atoi (bonus) < 0))
    {
      send_to_char (syntax, ch);
      return FALSE;
    }

  pMob->hit[DICE_NUMBER] = atoi (num);
  pMob->hit[DICE_TYPE] = atoi (type);
  pMob->hit[DICE_BONUS] = atoi (bonus);

  send_to_char ("Hitdice set.\n\r", ch);
  return TRUE;
}

MEDIT (medit_manadice)
{
  static char syntax[] = "Syntax:  manadice <number> d <type> + <bonus>\n\r";
  char *num, *type, *bonus, *cp;
  MOB_INDEX_DATA *pMob;

  EDIT_MOB (ch, pMob);

  if (argument[0] == '\0')
    {
      send_to_char (syntax, ch);
      return FALSE;
    }

  num = cp = argument;

  while (isdigit (*cp))
    ++cp;
  while (*cp != '\0' && !isdigit (*cp))
    *(cp++) = '\0';

  type = cp;

  while (isdigit (*cp))
    ++cp;
  while (*cp != '\0' && !isdigit (*cp))
    *(cp++) = '\0';

  bonus = cp;

  while (isdigit (*cp))
    ++cp;
  if (*cp != '\0')
    *cp = '\0';

  if (!(is_number (num) && is_number (type) && is_number (bonus)))
    {
      send_to_char (syntax, ch);
      return FALSE;
    }

  if ((!is_number (num) || atoi (num) < 1)
      || (!is_number (type) || atoi (type) < 1)
      || (!is_number (bonus) || atoi (bonus) < 0))
    {
      send_to_char (syntax, ch);
      return FALSE;
    }

  pMob->mana[DICE_NUMBER] = atoi (num);
  pMob->mana[DICE_TYPE] = atoi (type);
  pMob->mana[DICE_BONUS] = atoi (bonus);

  send_to_char ("Manadice set.\n\r", ch);
  return TRUE;
}

MEDIT (medit_damdice)
{
  static char syntax[] = "Syntax:  damdice <number> d <type> + <bonus>\n\r";
  char *num, *type, *bonus, *cp;
  MOB_INDEX_DATA *pMob;

  EDIT_MOB (ch, pMob);

  if (argument[0] == '\0')
    {
      send_to_char (syntax, ch);
      return FALSE;
    }

  num = cp = argument;

  while (isdigit (*cp))
    ++cp;
  while (*cp != '\0' && !isdigit (*cp))
    *(cp++) = '\0';

  type = cp;

  while (isdigit (*cp))
    ++cp;
  while (*cp != '\0' && !isdigit (*cp))
    *(cp++) = '\0';

  bonus = cp;

  while (isdigit (*cp))
    ++cp;
  if (*cp != '\0')
    *cp = '\0';

  if (!(is_number (num) && is_number (type) && is_number (bonus)))
    {
      send_to_char (syntax, ch);
      return FALSE;
    }

  if ((!is_number (num) || atoi (num) < 1)
      || (!is_number (type) || atoi (type) < 1)
      || (!is_number (bonus) || atoi (bonus) < 0))
    {
      send_to_char (syntax, ch);
      return FALSE;
    }

  pMob->damage[DICE_NUMBER] = atoi (num);
  pMob->damage[DICE_TYPE] = atoi (type);
  pMob->damage[DICE_BONUS] = atoi (bonus);

  send_to_char ("Damdice set.\n\r", ch);
  return TRUE;
}


MEDIT (medit_race)
{
  MOB_INDEX_DATA *pMob;
  int race;

  if (argument[0] != '\0' && (race = race_lookup (argument)) != 0)
    {
      EDIT_MOB (ch, pMob);

      pMob->race = race;
      pMob->act |= race_table[race].act;
      pMob->affected_by |= race_table[race].aff;
      pMob->shielded_by |= race_table[race].shd;
      pMob->off_flags |= race_table[race].off;
      pMob->imm_flags |= race_table[race].imm;
      pMob->res_flags |= race_table[race].res;
      pMob->vuln_flags |= race_table[race].vuln;
      pMob->form |= race_table[race].form;
      pMob->parts |= race_table[race].parts;

      send_to_char ("Race set.\n\r", ch);
      return TRUE;
    }

  if (argument[0] == '?')
    {
      char buf[MAX_STRING_LENGTH];

      send_to_char ("Available races are:", ch);

      for (race = 0; race_table[race].name != NULL; race++)
	{
	  if ((race % 3) == 0)
	    send_to_char ("\n\r", ch);
	  snprintf (buf, sizeof (buf), " %-15s", race_table[race].name);
	  send_to_char (buf, ch);
	}

      send_to_char ("\n\r", ch);
      return FALSE;
    }

  send_to_char ("Syntax:  race [race]\n\r"
		"Type 'race ?' for a list of races.\n\r", ch);
  return FALSE;
}


MEDIT (medit_position)
{
  MOB_INDEX_DATA *pMob;
  char arg[MAX_INPUT_LENGTH];
  int value;

  argument = one_argument (argument, arg);

  switch (arg[0])
    {
    default:
      break;

    case 'S':
    case 's':
      if (str_prefix (arg, "start"))
	break;

      if ((value = flag_value (position_flags, argument)) == NO_FLAG)
	break;

      EDIT_MOB (ch, pMob);

      pMob->start_pos = value;
      send_to_char ("Start position set.\n\r", ch);
      return TRUE;

    case 'D':
    case 'd':
      if (str_prefix (arg, "default"))
	break;

      if ((value = flag_value (position_flags, argument)) == NO_FLAG)
	break;

      EDIT_MOB (ch, pMob);

      pMob->default_pos = value;
      send_to_char ("Default position set.\n\r", ch);
      return TRUE;
    }

  send_to_char ("Syntax:  position [start/default] [position]\n\r"
		"Type '? position' for a list of positions.\n\r", ch);
  return FALSE;
}


MEDIT (medit_gold)
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB (ch, pMob);

  if (argument[0] == '\0' || !is_number (argument))
    {
      send_to_char ("Syntax:  wealth [number]\n\r", ch);
      return FALSE;
    }

  pMob->wealth = atoi (argument);

  send_to_char ("Wealth set.\n\r", ch);
  return TRUE;
}

MEDIT (medit_hitroll)
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB (ch, pMob);

  if (argument[0] == '\0' || !is_number (argument))
    {
      send_to_char ("Syntax:  hitroll [number]\n\r", ch);
      return FALSE;
    }

  pMob->hitroll = atoi (argument);

  send_to_char ("Hitroll set.\n\r", ch);
  return TRUE;
}

void
show_liqlist (CHAR_DATA * ch)
{
  int liq;
  BUFFER *buffer;
  char buf[MAX_STRING_LENGTH];

  buffer = new_buf ();

  for (liq = 0; liq_table[liq].liq_name != NULL; liq++)
    {
      if ((liq % 21) == 0)
	add_buf (buffer,
		 "Name                 Color          Proof Full Thirst Food Ssize\n\r");

      snprintf (buf, sizeof (buf), "%-20s %-14s %5d %4d %6d %4d %5d\n\r",
	       liq_table[liq].liq_name, liq_table[liq].liq_color,
	       liq_table[liq].liq_affect[0], liq_table[liq].liq_affect[1],
	       liq_table[liq].liq_affect[2], liq_table[liq].liq_affect[3],
	       liq_table[liq].liq_affect[4]);
      add_buf (buffer, buf);
    }

  page_to_char (buf_string (buffer), ch);
  free_buf (buffer);

  return;
}

void
show_damlist (CHAR_DATA * ch)
{
  int att;
  BUFFER *buffer;
  char buf[MAX_STRING_LENGTH];

  buffer = new_buf ();

  for (att = 0; attack_table[att].name != NULL; att++)
    {
      if ((att % 21) == 0)
	add_buf (buffer, "Name                 Noun\n\r");

      snprintf (buf, sizeof (buf), "%-20s %-20s\n\r",
	       attack_table[att].name, attack_table[att].noun);
      add_buf (buffer, buf);
    }

  page_to_char (buf_string (buffer), ch);
  free_buf (buffer);

  return;
}

MEDIT (medit_group)
{
  MOB_INDEX_DATA *pMob;
  MOB_INDEX_DATA *pMTemp;
  char arg[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int temp;
  BUFFER *buffer;
  bool found = FALSE;

  EDIT_MOB (ch, pMob);

  if (argument[0] == '\0')
    {
      send_to_char ("Syntax: group [number]\n\r", ch);
      send_to_char ("        group show [number]\n\r", ch);
      return FALSE;
    }

  if (is_number (argument))
    {
      pMob->group = atoi (argument);
      send_to_char ("Group set.\n\r", ch);
      return TRUE;
    }

  argument = one_argument (argument, arg);

  if (!strcmp (arg, "show") && is_number (argument))
    {
      if (atoi (argument) == 0)
	{
	  send_to_char ("Are you crazy?\n\r", ch);
	  return FALSE;
	}

      buffer = new_buf ();

      for (temp = 0; temp < 65536; temp++)
	{
	  pMTemp = get_mob_index (temp);
	  if (pMTemp && (pMTemp->group == atoi (argument)))
	    {
	      found = TRUE;
	      snprintf (buf, sizeof (buf), "[%5d] %s\n\r", pMTemp->vnum,
		       pMTemp->player_name);
	      add_buf (buffer, buf);
	    }
	}

      if (found)
	page_to_char (buf_string (buffer), ch);
      else
	send_to_char ("No mobs in that group.\n\r", ch);

      free_buf (buffer);
      return FALSE;
    }

  return FALSE;
}

REDIT (redit_owner)
{
  ROOM_INDEX_DATA *pRoom;

  EDIT_ROOM (ch, pRoom);

  if (argument[0] == '\0')
    {
      send_to_char ("Syntax:  owner [owner]\n\r", ch);
      send_to_char ("         owner none\n\r", ch);
      return FALSE;
    }

  free_string (pRoom->owner);
  if (!str_cmp (argument, "none"))
    pRoom->owner = str_dup ("");
  else
    pRoom->owner = str_dup (argument);

  send_to_char ("Owner set.\n\r", ch);
  return TRUE;
}

MEDIT (medit_addmprog)
{
  int value;
  MOB_INDEX_DATA *pMob;
  MPROG_LIST *list;
  MPROG_CODE *code;
  char trigger[MAX_STRING_LENGTH];
  char phrase[MAX_STRING_LENGTH];
  char num[MAX_STRING_LENGTH];

  EDIT_MOB (ch, pMob);
  argument = one_argument (argument, num);
  argument = one_argument (argument, trigger);
  argument = one_argument (argument, phrase);

  if (!is_number (num) || trigger[0] == '\0' || phrase[0] == '\0')
    {
      send_to_char ("Syntax:   addmprog [vnum] [trigger] [phrase]\n\r", ch);
      return FALSE;
    }

  if ((value = flag_value (mprog_flags, trigger)) == NO_FLAG)
    {
      send_to_char ("Valid flags are:\n\r", ch);
      show_help (ch, "mprog");
      return FALSE;
    }

  if ((code = get_mprog_index (atoi (num))) == NULL)
    {
      send_to_char ("No such MOBProgram.\n\r", ch);
      return FALSE;
    }

  list = new_mprog ();
  list->vnum = atoi (num);
  list->trig_type = value;
  list->trig_phrase = str_dup (phrase);
  list->code = code->code;
  SET_BIT (pMob->mprog_flags, value);
  list->next = pMob->mprogs;
  pMob->mprogs = list;

  send_to_char ("Mprog Added.\n\r", ch);
  return TRUE;
}

MEDIT (medit_delmprog)
{
  MOB_INDEX_DATA *pMob;
  MPROG_LIST *list;
  MPROG_LIST *list_next;
  char mprog[MAX_STRING_LENGTH];
  int value;
  int cnt = 0;

  EDIT_MOB (ch, pMob);

  one_argument (argument, mprog);
  if (!is_number (mprog) || mprog[0] == '\0')
    {
      send_to_char ("Syntax:  delmprog [#mprog]\n\r", ch);
      return FALSE;
    }

  value = atoi (mprog);

  if (value < 0)
    {
      send_to_char ("Only non-negative mprog-numbers allowed.\n\r", ch);
      return FALSE;
    }

  if (!(list = pMob->mprogs))
    {
      send_to_char ("MEdit:  Non existant mprog.\n\r", ch);
      return FALSE;
    }

  if (value == 0)
    {
      REMOVE_BIT (pMob->mprog_flags, pMob->mprogs->trig_type);
      list = pMob->mprogs;
      pMob->mprogs = list->next;
      free_mprog (list);
    }
  else
    {
      while ((list_next = list->next) && (++cnt < value))
	list = list_next;

      if (list_next)
	{
	  REMOVE_BIT (pMob->mprog_flags, list_next->trig_type);
	  list->next = list_next->next;
	  free_mprog (list_next);
	}
      else
	{
	  send_to_char ("No such mprog.\n\r", ch);
	  return FALSE;
	}
    }

  send_to_char ("Mprog removed.\n\r", ch);
  return TRUE;
}
