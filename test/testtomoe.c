#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tomoe.h"

static glyph * read_test_data ();

#define LINE_BUF_SIZE 4096
static char line_buf[LINE_BUF_SIZE];

static glyph *
read_test_data ()
{
  char *p = NULL;
  int stroke_num = 0;
  int point_num = 0;
  int j = 0;
  int k = 0;
  stroke *strk = NULL;
  point *pnt = NULL;
  FILE *fp = stdin;
  glyph *test_glyph = NULL;

  test_glyph = (glyph *) calloc (1, sizeof(glyph));

  while ((p = fgets(line_buf, LINE_BUF_SIZE, fp)) != NULL)
  {
	  if (strstr(p, "EOF") != NULL)
	  {
		  return NULL;
	  }

	  if (p[0] != ':')
	  {
		  continue;
	  }
	  sscanf(p + 1, "%d", &stroke_num);
    strk = (stroke *) calloc (stroke_num, sizeof (stroke));

	  for (j = 0; j < stroke_num; j++)
	  {
		  p = fgets(line_buf, LINE_BUF_SIZE, fp);
		  sscanf(p, "%d", &point_num);
		  p = strchr(p, ' ');

      strk[j].point_num = point_num;
      pnt = (point *) calloc (point_num, sizeof (point));
      strk[j].points = pnt;
		  for (k = 0; k < point_num; k++)
		  {
        int x, y;
			  sscanf(p, " (%d %d)", &x, &y);
			  p = strchr(p, ')') + 1;
        pnt[k].x = x;
        pnt[k].y = y;
		  }
	  }

    test_glyph->strokes = strk;
    test_glyph->stroke_num = stroke_num;
	  return test_glyph;
  }
  return NULL;
}

int
main (int argc, char **argv)
{
  glyph *test_glyph = NULL;
  candidates *matched = NULL;
  int i, candidate_num = 0;

  tomoe_init ();

  test_glyph = read_test_data (); 

  if (!test_glyph)
    exit (1);

  candidate_num = tomoe_get_matched (test_glyph, &matched);

  if (candidate_num != 0)
  {
    if (!matched)
    {
      fprintf (stderr, "Candidate list is NULL!\n");
      goto END;
    }
    if (candidate_num != matched->candidate_num)
    {
      fprintf (stdout, "The number of candidates does not equal!\n");
      goto END;
    }
    for (i = 0; i < candidate_num; i++)
    {
      fprintf (stdout, "character:%s\tscore:%d\n",
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
/*
vi:ts=2:nowrap:ai:expandtab
*/
