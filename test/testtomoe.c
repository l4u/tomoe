/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tomoe.h"

static tomoe_glyph * read_test_data ();

#define LINE_BUF_SIZE 4096
static char line_buf[LINE_BUF_SIZE];

static tomoe_glyph *
read_test_data ()
{
    char *p = NULL;
    int stroke_num = 0;
    int point_num  = 0;
    int j = 0;
    int k = 0;
    tomoe_stroke *strk = NULL;
    tomoe_point  *pnt  = NULL;
    FILE *fp = stdin;
    tomoe_glyph *test_glyph = NULL;

    test_glyph = calloc (1, sizeof (tomoe_glyph));

    while ((p = fgets (line_buf, LINE_BUF_SIZE, fp)) != NULL)
    {
        if (strstr (p, "EOF") != NULL)
        {
            return NULL;
        }

        if (p[0] != ':')
        {
            continue;
        }
        sscanf (p + 1, "%d", &stroke_num);
        strk = calloc (stroke_num, sizeof (tomoe_stroke));

        for (j = 0; j < stroke_num; j++)
        {
            p = fgets (line_buf, LINE_BUF_SIZE, fp);
            sscanf (p, "%d", &point_num);
            p = strchr (p, ' ');

            strk[j].point_num = point_num;
            pnt = calloc (point_num, sizeof (tomoe_point));
            strk[j].points = pnt;
            for (k = 0; k < point_num; k++)
            {
                int x, y;
                sscanf (p, " (%d %d)", &x, &y);
                p = strchr (p, ')') + 1;
                pnt[k].x = x;
                pnt[k].y = y;
            }
        }

        test_glyph->strokes    = strk;
        test_glyph->stroke_num = stroke_num;
        return test_glyph;
    }
    return NULL;
}

int
main (int argc, char **argv)
{
    tomoe_glyph *test_glyph = NULL;
    int i, candidate_num = 0;
    tomoe_array* matched = NULL;
    tomoe_db* db = NULL;

    fprintf (stdout, "init tomoe ... ");
    fflush (stdout);
    db = tomoe_init ();
    if (!db) exit (1);
    fprintf (stdout, "ok\n");

    test_glyph = read_test_data (); 

    if (!test_glyph) 
        goto END;

    matched = tomoe_db_get_matched (db, test_glyph);
    candidate_num = tomoe_array_size(matched);

    if (candidate_num != 0)
    {
        if (!matched)
        {
            fprintf (stderr, "Candidate list is NULL!\n");
            goto END;
        }

        fprintf (stdout, "The number of matched characters: %d\n",
                 candidate_num);
        for (i = 0; i < candidate_num; i++)
        {
            tomoe_candidate* p = (tomoe_candidate*)tomoe_array_get(matched, i);
            fprintf (stdout, "character:%s\tscore:%d\n",
                     p->letter, p->score);
        }
    }
    else
        fprintf (stdout, "No Candidate found!\n");
  
END:
    if (!test_glyph)
        tomoe_glyph_free (test_glyph);
    if (matched)
        tomoe_array_free (matched);
    tomoe_db_free (db);
    tomoe_term ();
    return 0;
}
/*
vi:ts=2:nowrap:ai:expandtab
*/
