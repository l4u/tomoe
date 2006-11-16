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

static TomoeGlyph * read_glyph (void);

#define LINE_BUF_SIZE 4096
static char line_buf[LINE_BUF_SIZE];

static TomoeGlyph *
read_glyph (void)
{
    char *p = NULL;
    FILE *fp = stdin;
    TomoeGlyph *glyph = NULL;

    glyph = tomoe_glyph_new ();

    while ((p = fgets (line_buf, LINE_BUF_SIZE, fp)) != NULL)
    {
        int stroke_num = 0;
        int j = 0;
        TomoeStroke *strk = NULL;
        TomoePoint  *pnt  = NULL;

        if (strstr (p, "EOF") != NULL)
        {
            tomoe_glyph_free (glyph);
            return NULL;
        }

        if (p[0] != ':')
        {
            continue;
        }

        sscanf (p + 1, "%d", &stroke_num);
        glyph->stroke_num = stroke_num;
        strk = calloc (stroke_num, sizeof (TomoeStroke));
        glyph->strokes = strk;

        for (j = 0; j < stroke_num; j++)
        {
            int k = 0;
            int point_num = 0;

            p = fgets (line_buf, LINE_BUF_SIZE, fp);
            sscanf (p, "%d", &point_num);
            p = strchr (p, ' ');

            strk[j].point_num = point_num;
            pnt = calloc (point_num, sizeof (TomoePoint));
            strk[j].points = pnt;
            for (k = 0; k < point_num; k++)
            {
                int x = 0, y = 0, n_matched;

                n_matched = sscanf (p, " (%d %d)", &x, &y);
                p = strchr (p, ')') + 1;
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
    /* FIXME! read arguments */

    tomoe_init ();
    TomoeDB* db = tomoe_simple_load (NULL);
    if (!db) exit (1);

    while (1)
    {
        TomoeGlyph *glyph;
        TomoeArray* matched = NULL;
        int candidate_num = 0;

        glyph = read_glyph ();

        if (!glyph)
            break;

        matched = tomoe_db_search_by_strokes (db, glyph);
        candidate_num = tomoe_array_size (matched);

        if (candidate_num != 0 && matched)
        {
            int i;

            for (i = 0; i < candidate_num; i++)
            {
#if 0
                const tomoe_candidate* p = (const tomoe_candidate*)tomoe_array_get_const (matched, i);
                if (i > 0)
                    fprintf (stdout, " ");
                fprintf (stdout, " %s", p->character->letter);
#endif
            }
            fprintf (stdout, "\n");
        }

        tomoe_glyph_free (glyph);
        tomoe_array_free (matched);
    }

    tomoe_db_free (db);
    tomoe_quit ();

    return 0;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
