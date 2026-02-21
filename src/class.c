/* Online setting of skill/spell levels, 
 * (c) 1996 Erwin S. Andreasen <erwin@pip.dknet.dk>
 *
 */

/*
 * Copyright (C) 2007-2011 See the AUTHORS.BlinkenMUD file for details
 * By using this code, you have agreed to follow the terms of the   
 * ROT license, in the file doc/rot.license        
*/


#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"

#define MIL  MAX_INPUT_LENGTH
#define MSL  MAX_STRING_LENGTH

/* 

  Class table levels loading/saving
  
*/

/* Save this class */
void
save_class (int num)
{
  FILE *fp;
  char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  int lev, i;

  snprintf (buf, sizeof (buf), "%sclasses/%s", DATA_DIR, class_table[num].name);

  if (!(fp = fopen (buf, "w")))
    {
      snprintf (buf2, sizeof (buf2), "Could not open file %s in order to save.", buf);
      bug (buf2, 0);
      return;
    }


  for (lev = 0; lev < MAX_LEVEL; lev++)
    for (i = 0; i < MAX_SKILL; i++)
      {
	if (!skill_table[i].name || !skill_table[i].name[0])
	  continue;

	if (skill_table[i].skill_level[num] == lev)
	  fprintf (fp, "%d %s\n", lev, skill_table[i].name);
      }

  fprintf (fp, "-1");		/* EOF -1 */
  fclose (fp);
}



void
save_classes ()
{
  int i;

  for (i = 0; i < MAX_CLASS; i++)
    save_class (i);
}


/* Load a class */
void
load_class (int num)
{
  char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH];
  int level, n;
  FILE *fp;

  snprintf (buf, sizeof (buf), "%sclasses/%s", DATA_DIR, class_table[num].name);

  if (!(fp = fopen (buf, "r")))
    {
      snprintf (buf2, sizeof (buf2), "Could not open file %s in order to save.", buf);
      bug (buf2, 0);
      return;
    }

  fscanf (fp, "%d", &level);

  while (level != -1)
    {
      fscanf (fp, " %[^\n]\n", buf);	/* read name of skill into buf */

      n = skill_lookup (buf);	/* find index */

      if (n == -1)
	{
	  char buf2[200];
	  snprintf (buf2, sizeof (buf2), "Class %s: unknown spell %s", class_table[num].name,
		   buf);
	  bug (buf2, 0);
	}
      else
	skill_table[n].skill_level[num] = level;

      fscanf (fp, "%d", &level);
    }

  fclose (fp);
}

void
load_classes ()
{
  int i, j;

  for (i = 0; i < MAX_CLASS; i++)
    {
      for (j = 0; j < MAX_SKILL; j++)
	skill_table[j].skill_level[i] = MAX_LEVEL;

      load_class (i);
    }
}

void
do_modskill (CHAR_DATA * ch, char *argument)
{
  char class_name[MIL], skill_name[MIL];
  int sn, level, class_no;
  char buf[MSL], lvl[MSL];



  argument = one_argument (argument, class_name);
  argument = one_argument (argument, skill_name);

  if (!argument[0])
    {
      send_to_char ("Syntax is: SKILL <class> <skill> <level>.\n\r", ch);
      return;
    }

  level = atoi (argument);

  if (!is_number (argument) || level < 0 || level > MAX_LEVEL)
    {
      snprintf (buf, sizeof (buf), "%s", "Level range is from 0 to 310.\n\r");
      send_to_char (buf, ch);
      return;
    }


  if ((sn = skill_lookup (skill_name)) == -1)
    {
      snprintf (buf, sizeof (buf), "There is no such spell/skill as %s.\n\r",
                skill_name);
      send_to_char (buf, ch);
      return;
    }

  for (class_no = 0; class_no < MAX_CLASS; class_no++)
    if (!str_cmp (class_name, class_table[class_no].who_name))
      break;

  if (class_no == MAX_CLASS)
    {
      snprintf (buf, sizeof (buf),
                "No class named '%s' exists. Use the 3-letter WHO names"
                "(Psi, Mag etc.)\n\r", class_name);
      send_to_char (buf, ch);
      return;
    }

  skill_table[sn].skill_level[class_no] = level;

  snprintf (lvl, sizeof (lvl), "%d", level);
  snprintf (buf, sizeof (buf), "OK, %s's will now gain %s at level %s%s",
            class_table[class_no].name, skill_table[sn].name, lvl,
            level > 101 ? " (i.e. never)" : "");
  send_to_char (buf, ch);

  save_classes ();
}
