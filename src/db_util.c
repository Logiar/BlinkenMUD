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
*	    Russ Taylor (rtaylor@pacinfo.com)			   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)			   *
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "merc.h"
#include "db.h"
#include "recycle.h"
#include "music.h"
#include "tables.h"
#include "lookup.h"

#if !defined(OLD_RAND)
long random ();
void srandom (unsigned int);
int getpid ();
time_t time (time_t * tloc);
#endif


extern char boot_buf[];
extern FILE *fpArea;
extern char strArea[];

static void append_to_buf (char *buf, size_t buf_size, const char *text);

static void
append_to_buf (char *buf, size_t buf_size, const char *text)
{
  size_t buf_len;

  if (buf_size == 0 || text == NULL)
    return;

  buf_len = strlen (buf);
  if (buf_len >= buf_size - 1)
    return;

  snprintf (buf + buf_len, buf_size - buf_len, "%s", text);
}

/*
 * Stick a little fuzz on a number.
 */
int
number_fuzzy (int number)
{
  switch (number_bits (2))
    {
    case 0:
      number -= 1;
      break;
    case 3:
      number += 1;
      break;
    }

  return UMAX (1, number);
}



/*
 * Generate a random number.
 */
int
number_range (int from, int to)
{
  int power;
  int number;

  if (from == 0 && to == 0)
    return 0;

  if ((to = to - from + 1) <= 1)
    return from;

  for (power = 2; power < to; power <<= 1)
    ;

  while ((number = number_mm () & (power - 1)) >= to)
    ;

  return from + number;
}



/*
 * Generate a percentile roll.
 */
int
number_percent (void)
{
  int percent;

  while ((percent = number_mm () & (128 - 1)) > 99)
    ;

  return 1 + percent;
}



/*
 * Generate a random door.
 */
int
number_door (void)
{
  int door;

  while ((door = number_mm () & (8 - 1)) > 11)
    ;

  return door;
}

int
number_bits (int width)
{
  return number_mm () & ((1 << width) - 1);
}




/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */

/* I noticed streaking with this random number generator, so I switched
   back to the system srandom call.  If this doesn't work for you, 
   define OLD_RAND to use the old system -- Alander */

#if defined (OLD_RAND)
static int rgiState[2 + 55];
#endif

void
init_mm ()
{
#if defined (OLD_RAND)
  int *piState;
  int iState;

  piState = &rgiState[2];

  piState[-2] = 55 - 55;
  piState[-1] = 55 - 24;

  piState[0] = ((int) current_time) & ((1 << 30) - 1);
  piState[1] = 1;
  for (iState = 2; iState < 55; iState++)
    {
      piState[iState] = (piState[iState - 1] + piState[iState - 2])
	& ((1 << 30) - 1);
    }
#else
  srandom (time (NULL) ^ getpid ());
#endif
  append_to_buf (boot_buf, MAX_STRING_LENGTH, "sign ");
  return;
}



long
number_mm (void)
{
#if defined (OLD_RAND)
  int *piState;
  int iState1;
  int iState2;
  int iRand;

  piState = &rgiState[2];
  iState1 = piState[-2];
  iState2 = piState[-1];
  iRand = (piState[iState1] + piState[iState2]) & ((1 << 30) - 1);
  piState[iState1] = iRand;
  if (++iState1 == 55)
    iState1 = 0;
  if (++iState2 == 55)
    iState2 = 0;
  piState[-2] = iState1;
  piState[-1] = iState2;
  return iRand >> 6;
#else
  return random () >> 6;
#endif
}


/*
 * Roll some dice.
 */
int
dice (int number, int size)
{
  int idice;
  int sum;

  switch (size)
    {
    case 0:
      return 0;
    case 1:
      return number;
    }

  for (idice = 0, sum = 0; idice < number; idice++)
    sum += number_range (1, size);

  return sum;
}



/*
 * Simple linear interpolation.
 */
int
interpolate (int level, int value_00, int value_32)
{
  return value_00 + level * (value_32 - value_00) / 32;
}



/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void
smash_tilde (char *str)
{
  for (; *str != '\0'; str++)
    {
      if (*str == '~')
	*str = '-';
    }

  return;
}



/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool
str_cmp (const char *astr, const char *bstr)
{
  if (astr == NULL)
    {
      bug ("Str_cmp: null astr.", 0);
      return TRUE;
    }

  if (bstr == NULL)
    {
      bug ("Str_cmp: null bstr.", 0);
      return TRUE;
    }

  for (; *astr || *bstr; astr++, bstr++)
    {
      if (LOWER (*astr) != LOWER (*bstr))
	return TRUE;
    }

  return FALSE;
}



/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool
str_prefix (const char *astr, const char *bstr)
{
  if (astr == NULL)
    {
      bug ("Strn_cmp: null astr.", 0);
      return TRUE;
    }

  if (bstr == NULL)
    {
      bug ("Strn_cmp: null bstr.", 0);
      return TRUE;
    }

  for (; *astr; astr++, bstr++)
    {
      if (LOWER (*astr) != LOWER (*bstr))
	return TRUE;
    }

  return FALSE;
}

/*
 * Compare strings, case sensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool
str_prefix_c (const char *astr, const char *bstr)
{
  if (astr == NULL)
    {
      bug ("Strn_cmp: null astr.", 0);
      return TRUE;
    }

  if (bstr == NULL)
    {
      bug ("Strn_cmp: null bstr.", 0);
      return TRUE;
    }

  for (; *astr; astr++, bstr++)
    {
      if (*astr != *bstr)
	return TRUE;
    }

  return FALSE;
}

/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool
str_infix (const char *astr, const char *bstr)
{
  int sstr1;
  int sstr2;
  int ichar;
  char c0;

  if ((c0 = LOWER (astr[0])) == '\0')
    return FALSE;

  sstr1 = strlen (astr);
  sstr2 = strlen (bstr);

  for (ichar = 0; ichar <= sstr2 - sstr1; ichar++)
    {
      if (c0 == LOWER (bstr[ichar]) && !str_prefix (astr, bstr + ichar))
	return FALSE;
    }

  return TRUE;
}


/*
 * Compare strings, case sensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool
str_infix_c (const char *astr, const char *bstr)
{
  int sstr1;
  int sstr2;
  int ichar;
  char c0;

  if ((c0 = astr[0]) == '\0')
    return FALSE;

  sstr1 = strlen (astr);
  sstr2 = strlen (bstr);

  for (ichar = 0; ichar <= sstr2 - sstr1; ichar++)
    {
      if (c0 == bstr[ichar] && !str_prefix_c (astr, bstr + ichar))
	return FALSE;
    }

  return TRUE;
}


/*
 * Replace a substring in a string, case insensitive...Russ Walsh
 * looks for bstr within astr and replaces it with cstr.
 */
char *
str_replace (char *astr, char *bstr, char *cstr)
{
  char newstr[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  bool found = FALSE;
  int sstr1, sstr2;
  int ichar, jchar;
  char c0, c1, c2;

  if (((c0 = LOWER (astr[0])) == '\0')
      || ((c1 = LOWER (bstr[0])) == '\0') || ((c2 = LOWER (cstr[0])) == '\0'))
    return astr;

  if (str_infix (bstr, astr))
    return astr;

/* make sure we don't start an infinite loop */
  if (!str_infix (bstr, cstr))
    return astr;

  sstr1 = strlen (astr);
  sstr2 = strlen (bstr);
  jchar = 0;

  if (sstr1 < sstr2)
    return astr;

  for (ichar = 0; ichar <= sstr1 - sstr2; ichar++)
    {
      if (c1 == LOWER (astr[ichar]) && !str_prefix (bstr, astr + ichar))
	{
	  found = TRUE;
	  jchar = ichar;
	  ichar = sstr1;
	}
    }
  if (found)
    {
      buf[0] = '\0';
      for (ichar = 0; ichar < jchar; ichar++)
	{
	  snprintf (newstr, sizeof (newstr), "%c", astr[ichar]);
	  append_to_buf (buf, sizeof (buf), newstr);
	}
      append_to_buf (buf, sizeof (buf), cstr);
      for (ichar = jchar + sstr2; ichar < sstr1; ichar++)
	{
	  snprintf (newstr, sizeof (newstr), "%c", astr[ichar]);
	  append_to_buf (buf, sizeof (buf), newstr);
	}
      {
        char *replaced = str_replace (buf, bstr, cstr);

        memmove (astr, replaced, strlen (replaced) + 1);
      }
      return astr;
    }
  return astr;
}

/*
 * Replace a substring in a string, case sensitive...Russ Walsh
 * looks for bstr within astr and replaces it with cstr.
 */
char *
str_replace_c (char *astr, char *bstr, char *cstr)
{
  char newstr[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  bool found = FALSE;
  int sstr1, sstr2;
  int ichar, jchar;
  char c0, c1, c2;

  if (((c0 = astr[0]) == '\0')
      || ((c1 = bstr[0]) == '\0') || ((c2 = cstr[0]) == '\0'))
    return astr;

  if (str_infix_c (bstr, astr))
    return astr;

/* make sure we don't start an infinite loop */
  if (!str_infix_c (bstr, cstr))
    return astr;

  sstr1 = strlen (astr);
  sstr2 = strlen (bstr);
  jchar = 0;

  if (sstr1 < sstr2)
    return astr;

  for (ichar = 0; ichar <= sstr1 - sstr2; ichar++)
    {
      if (c1 == astr[ichar] && !str_prefix_c (bstr, astr + ichar))
	{
	  found = TRUE;
	  jchar = ichar;
	  ichar = sstr1;
	}
    }
  if (found)
    {
      buf[0] = '\0';
      for (ichar = 0; ichar < jchar; ichar++)
	{
	  snprintf (newstr, sizeof (newstr), "%c", astr[ichar]);
	  append_to_buf (buf, sizeof (buf), newstr);
	}
      append_to_buf (buf, sizeof (buf), cstr);
      for (ichar = jchar + sstr2; ichar < sstr1; ichar++)
	{
	  snprintf (newstr, sizeof (newstr), "%c", astr[ichar]);
	  append_to_buf (buf, sizeof (buf), newstr);
	}
      {
        char *replaced = str_replace_c (buf, bstr, cstr);

        memmove (astr, replaced, strlen (replaced) + 1);
      }
      return astr;
    }
  return astr;
}


/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool
str_suffix (const char *astr, const char *bstr)
{
  int sstr1;
  int sstr2;

  sstr1 = strlen (astr);
  sstr2 = strlen (bstr);
  if (sstr1 <= sstr2 && !str_cmp (astr, bstr + sstr2 - sstr1))
    return FALSE;
  else
    return TRUE;
}



/*
 * Returns an initial-capped string.
 */
char *
capitalize (const char *str)
{
  static char strcap[MAX_STRING_LENGTH];
  int i;

  for (i = 0; str[i] != '\0'; i++)
    strcap[i] = LOWER (str[i]);
  strcap[i] = '\0';
  strcap[0] = UPPER (strcap[0]);
  return strcap;
}

/* Returns a string all lowercase - Fallon*/
char *
lowercase (const char *str)
{
  static char strlow[MAX_STRING_LENGTH];
  int i;

  if (!str)
    return NULL;

  for (i = 0; str[i] != '\0'; i++)
    strlow[i] = LOWER (str[i]);
  strlow[i] = '\0';
  return strlow;
}

/* Returns a string with the first letter lowercased - Fallon */
char *
decap (const char *str)
{
  static char firstlow[MAX_STRING_LENGTH];
  int i;

  for (i = 0; str[i] != '\0'; i++)
    firstlow[i] = str[i];
  firstlow[i] = '\0';
  firstlow[0] = LOWER (firstlow[0]);
  return firstlow;
}

/* Returns a string with returns inserted - Fallon */
char *
wrapstr (CHAR_DATA * ch, const char *str)
{
  static char strwrap[MAX_STRING_LENGTH];
  size_t i;
  int count = strlen (IS_NPC (ch) ? ch->short_descr : ch->name);

  for (i = 0; i < strlen (str); i++)
    {
      count++;
      if (count > 66 && str[i] == ' ')
	{
	  strwrap[i] = '\n';
	  strwrap[i + 1] = '\r';
	  count = 0;
	}
      else
	{
	  strwrap[i] = str[i];
	}
    }
  strwrap[i] = '\0';
  return strwrap;
}

/*
 * Append a string to a file.
 */
void
append_file (CHAR_DATA * ch, char *file, char *str)
{
  FILE *fp;

  if (IS_NPC (ch) || str[0] == '\0')
    return;

  fclose (fpReserve);
  if ((fp = fopen (file, "a")) == NULL)
    {
      perror (file);
      send_to_char ("Could not open the file!\n\r", ch);
    }
  else
    {
      fprintf (fp, "[%5d] %s: %s\n",
	       ch->in_room ? ch->in_room->vnum : 0, ch->name, str);
      fclose (fp);
    }

  fpReserve = fopen (NULL_FILE, "r");
  return;
}



/*
 * Reports a bug.
 */
void
bug (const char *str, int param)
{
  char buf[MAX_STRING_LENGTH];

  if (fpArea != NULL)
    {
      int iLine;
      int iChar;

      if (fpArea == stdin)
	{
	  iLine = 0;
	}
      else
	{
	  iChar = ftell (fpArea);
	  fseek (fpArea, 0, 0);
	  for (iLine = 0; ftell (fpArea) < iChar; iLine++)
	    {
	      while (getc (fpArea) != '\n')
		;
	    }
	  fseek (fpArea, iChar, 0);
	}

      snprintf (buf, sizeof (buf), "[*****] FILE: %s LINE: %d", strArea, iLine);
      log_string (buf);
/* RT removed because we don't want bugs shutting the mud 
	if ( ( fp = fopen( "shutdown.txt", "a" ) ) != NULL )
	{
	    fprintf( fp, "[*****] %s\n", buf );
	    fclose( fp );
	}
*/
    }

  snprintf (buf, sizeof (buf), "[*****] BUG: ");
  snprintf (buf + strlen (buf), sizeof (buf) - strlen (buf), str, param);
  log_string (buf);
/* RT removed due to bug-file spamming 
    fclose( fpReserve );
    if ( ( fp = fopen( BUG_FILE, "a" ) ) != NULL )
    {
	fprintf( fp, "%s\n", buf );
	fclose( fp );
    }
    fpReserve = fopen( NULL_FILE, "r" );
*/

  return;
}



/*
 * Writes a string to the log.
 */
void
log_string (const char *str)
{
  char *strtime;

  strtime = ctime (&current_time);
  strtime[strlen (strtime) - 1] = '\0';
  fprintf (stderr, "%s :: %s\n", strtime, str);
  return;
}



/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void
tail_chain (void)
{
  return;
}
