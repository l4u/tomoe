/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2000 - 2004 Hiroyuki Komatsu <komatsu@taiyaki.org>
 *  Copyright (C) 2004 Hiroaki Nakamura <hnakamur@good-day.co.jp>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program; if not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 *  Boston, MA  02111-1307  USA
 *
 *  $Id$
 */

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
    int point_num = 0;
    int j = 0;
    int k = 0;
    tomoe_stroke *strk = NULL;
    tomoe_point *pnt = NULL;
    FILE *fp = stdin;
    tomoe_glyph *test_glyph = NULL;

    test_glyph = (tomoe_glyph *) calloc (1, sizeof (tomoe_glyph));

    while ((p = fgets(line_buf, LINE_BUF_SIZE, fp)) != NULL)
    {
        if (strstr(p, "EOF") != NULL)
        {
            tomoe_glyph_free (test_glyph);
            return NULL;
        }

        if (p[0] != ':')
        {
            continue;
        }
        sscanf(p + 1, "%d", &stroke_num);
        test_glyph->stroke_num = stroke_num;
        strk = (tomoe_stroke *) calloc (stroke_num, sizeof (tomoe_stroke));
        test_glyph->strokes = strk;

        for (j = 0; j < stroke_num; j++)
        {
            p = fgets(line_buf, LINE_BUF_SIZE, fp);
            sscanf(p, "%d", &point_num);
            p = strchr(p, ' ');

            strk[j].point_num = point_num;
            pnt = (tomoe_point *) calloc (point_num, sizeof (tomoe_point));
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
    tomoe_glyph_free (test_glyph);
    return NULL;
}

#if 0
static glyph * 
get_stroke ()
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
    /* printf("------------------ get_stroke() ----------------------\n"); */
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
        test_glyph->stroke_num = stroke_num;
        /* printf("input_stroke_num=%d\n", stroke_num); */
        strk = (stroke *) calloc (stroke_num, sizeof (stroke));
        test_glyph->strokes = strk;
        /* letter_alloc_contents(&input, stroke_num); */

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
        return test_glyph;
    }
    return NULL;
}
#endif

int
main (int argc, char **argv)
{
    tomoe_glyph *test_glyph = NULL;
    tomoe_candidate **matched = NULL;
    int i, candidate_num = 0;

    tomoe_init ();

    while (1)
    {
        test_glyph = read_test_data ();

        if (!test_glyph)
            break;

        candidate_num = tomoe_get_matched (test_glyph, &matched);

        if (candidate_num != 0)
        {
            if (matched)
            {
                for (i = 0; i < candidate_num; i++)
                {
                    if (i > 0)
                        fprintf (stdout, " ");
                    fprintf (stdout, " %s", matched[i]->letter);
                }
                fprintf (stdout, "\n");
            }
        }

        tomoe_glyph_free (test_glyph);
  
        if (matched)
        {
            tomoe_free_matched (matched, candidate_num);
        }
    }

    tomoe_term ();
    return 0;
}
/*
vi:ts=4:nowrap:ai:expandtab
*/
