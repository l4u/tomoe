#include <stdlib.h>
#include <stdio.h>
#include "tomoe.h"

int
main (int argc, char **argv)
{
  glyph *test_glyph = NULL;
  candidates *matched = NULL;
  int i, candidate_num = 0;

  tomoe_init ();

  test_glyph = (glyph *) calloc (1, sizeof(glyph));

  candidate_num = tomoe_get_matched (test_glyph, &matched);
 
  if (candidate_num != 0)
  {
    if (!matched)
    {
      fprintf (stderr, "Candidate list is NULL!");
      goto END;
    }
    if (candidate_num != matched->candidate_num)
    {
      fprintf (stderr, "The number of candidates does not equals!");
      goto END;
    }
    for (i = 0; i < candidate_num; i++)
    {
      fprintf (stdout, "character:%s\tscore:%d",
	       matched->candidates[i].letter, matched->candidates[i].score);
    }
  }
  
END:
  free (test_glyph);

  if (matched)
  {
    tomoe_free_matched (matched);
  }

  tomoe_term ();
  return 0;
}
