#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "merc.h"
#include "tables.h"
#include "olc.h"

#define DUMMY_TABLE(name) \
  const struct flag_type name[] = { {NULL, 0, FALSE} }

DUMMY_TABLE(area_flags);
const struct flag_type sex_flags[] = {
    {"neutral", 0, TRUE},
    {"male", 1, TRUE},
    {"female", 2, TRUE},
    {NULL, 0, FALSE},
};
const struct flag_type exit_flags[] = {
    {"closed", A, TRUE},
    {"locked", B, TRUE},
    {"hidden", C, FALSE},
    {NULL, 0, FALSE},
};
DUMMY_TABLE(door_resets);
DUMMY_TABLE(room_flags);
DUMMY_TABLE(sector_flags);
DUMMY_TABLE(type_flags);
DUMMY_TABLE(extra_flags);
DUMMY_TABLE(wear_flags);
DUMMY_TABLE(act_flags);
DUMMY_TABLE(affect_flags);
DUMMY_TABLE(apply_flags);
DUMMY_TABLE(wear_loc_flags);
DUMMY_TABLE(wear_loc_strings);
DUMMY_TABLE(container_flags);
DUMMY_TABLE(form_flags);
DUMMY_TABLE(part_flags);
DUMMY_TABLE(ac_type);
DUMMY_TABLE(size_flags);
DUMMY_TABLE(position_flags);
DUMMY_TABLE(off_flags);
DUMMY_TABLE(imm_flags);
DUMMY_TABLE(res_flags);
DUMMY_TABLE(vuln_flags);
DUMMY_TABLE(weapon_class);
DUMMY_TABLE(weapon_type2);
DUMMY_TABLE(apply_types);
DUMMY_TABLE(pit_flags);

bool str_cmp(const char *astr, const char *bstr)
{
  if (astr == NULL)
    return bstr != NULL;
  if (bstr == NULL)
    return TRUE;

  while (*astr != '\0' || *bstr != '\0')
    {
      if (tolower((unsigned char) *astr) != tolower((unsigned char) *bstr))
        return TRUE;
      if (*astr != '\0')
        astr++;
      if (*bstr != '\0')
        bstr++;
    }
  return FALSE;
}

char *one_argument(char *argument, char *arg_first)
{
  while (*argument != '\0' && isspace((unsigned char) *argument))
    argument++;

  while (*argument != '\0' && !isspace((unsigned char) *argument))
    {
      *arg_first++ = (char) tolower((unsigned char) *argument++);
    }
  *arg_first = '\0';

  while (*argument != '\0' && isspace((unsigned char) *argument))
    argument++;

  return argument;
}

static void test_flag_value_for_stats(void)
{
  char input[] = "male";
  assert(flag_value(sex_flags, input) == 1);
}

static void test_flag_value_for_multi_flags(void)
{
  char input[] = "closed locked";
  int expected = A | B;
  assert(flag_value(exit_flags, input) == expected);
}

static void test_flag_value_rejects_non_settable(void)
{
  char input[] = "hidden";
  assert(flag_value(exit_flags, input) == NO_FLAG);
}

static void test_flag_string_for_flags(void)
{
  int bits = A | B;
  const char *actual = flag_string(exit_flags, bits);
  assert(strcmp(actual, "closed locked") == 0);
}

static void test_flag_string_for_stats(void)
{
  const char *actual = flag_string(sex_flags, 2);
  assert(strcmp(actual, "female") == 0);
}

int main(void)
{
  test_flag_value_for_stats();
  test_flag_value_for_multi_flags();
  test_flag_value_rejects_non_settable();
  test_flag_string_for_flags();
  test_flag_string_for_stats();

  puts("test_bit_flags: ok");
  return 0;
}
