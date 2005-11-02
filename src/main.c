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

static tomoe_glyph * read_glyph (void);

#define LINE_BUF_SIZE 4096
static char line_buf[LINE_BUF_SIZE];

static tomoe_glyph *
read_glyph (void)
{
    char *p = NULL;
    int stroke_num = 0;
    int point_num = 0;
    int j = 0;
    int k = 0;
    tomoe_stroke *strk = NULL;
    tomoe_point *pnt = NULL;
    FILE *fp = stdin;
    tomoe_glyph *glyph = NULL;

    glyph = calloc (1, sizeof (tomoe_glyph));

    while ((p = fgets(line_buf, LINE_BUF_SIZE, fp)) != NULL)
    {
        if (strstr(p, "EOF") != NULL)
        {
            tomoe_glyph_free (glyph);
            return NULL;
        }

        if (p[0] != ':')
        {
            continue;
        }
        sscanf(p + 1, "%d", &stroke_num);
        glyph->stroke_num = stroke_num;
        strk = calloc (stroke_num, sizeof (tomoe_stroke));
        glyph->strokes = strk;

        for (j = 0; j < stroke_num; j++)
        {
            p = fgets(line_buf, LINE_BUF_SIZE, fp);
            sscanf(p, "%d", &point_num);
            p = strchr(p, ' ');

            strk[j].point_num = point_num;
            pnt = calloc (point_num, sizeof (tomoe_point));
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

        glyph->strokes = strk;
        glyph->stroke_num = stroke_num;
        return glyph;
    }
    tomoe_glyph_free (glyph);
    return NULL;
}

int
main (int argc, char **argv)
{
    tomoe_glyph *glyph = NULL;
    tomoe_candidate **matched = NULL;
    int i, candidate_num = 0;

    /* FIXME! read arguments */

    tomoe_init ();

    while (1)
    {
        glyph = read_glyph ();

        if (!glyph)
            break;

        candidate_num = tomoe_get_matched (glyph, &matched);

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

        tomoe_glyph_free (glyph);
  
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
