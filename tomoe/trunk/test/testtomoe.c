/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include "tomoe.h"

static TomoeGlyph * read_test_data ();
void outCharInfo (TomoeChar* chr, int score);
void testStrokeMatch (TomoeContext* ctx);
void testReadingMatch (TomoeContext* ctx, const char* reading);
void testUserDict (TomoeContext* ctx);

#define LINE_BUF_SIZE 4096
static char line_buf[LINE_BUF_SIZE];

static TomoeGlyph *
read_test_data ()
{
    char *p = NULL;
    int stroke_num = 0;
    int point_num  = 0;
    int j = 0;
    int k = 0;
    TomoeStroke *strk = NULL;
    TomoePoint  *pnt  = NULL;
    FILE *fp = stdin;
    TomoeGlyph *test_glyph = NULL;

    test_glyph = calloc (1, sizeof (TomoeGlyph));

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
        strk = calloc (stroke_num, sizeof (TomoeStroke));

        for (j = 0; j < stroke_num; j++)
        {
            p = fgets (line_buf, LINE_BUF_SIZE, fp);
            sscanf (p, "%d", &point_num);
            p = strchr (p, ' ');

            strk[j].point_num = point_num;
            pnt = calloc (point_num, sizeof (TomoePoint));
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

void outCharInfo (TomoeChar* chr, int score)
{
   int j;
   GPtrArray* readings = tomoe_char_get_readings (chr);
   const char* meta = tomoe_char_get_meta (chr);

   fprintf (stdout, "character:%s [%d] ", tomoe_char_get_code (chr), score);
   fflush (stdout);
   if (readings->len)
   {
       for (j = 0; j < readings->len; j++)
       {
           const char* r = g_ptr_array_index (readings, j);
           fprintf (stdout, " %s", r);
       }
       fprintf (stdout, "\n");
   }
   if (meta)
       fprintf (stdout, meta);
   fprintf (stdout, "\n");
}

void testStrokeMatch (TomoeContext* ctx)
{
    TomoeGlyph *test_glyph = NULL;
    int i, candidate_num = 0;
    TomoeArray* matched = NULL;

    test_glyph = read_test_data ();

    if (!test_glyph) 
        goto END;

    matched = tomoe_context_search_by_strokes (ctx, test_glyph);
    candidate_num = tomoe_array_size (matched);

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
            const TomoeCandidate* p = (const TomoeCandidate*)tomoe_array_get_const (matched, i);
            outCharInfo (p->character, p->score);
        }
    }
    else
        fprintf (stdout, "No Candidate found!\n");

END:
    if (!test_glyph)
        tomoe_glyph_free (test_glyph);
    if (matched)
        tomoe_array_free (matched);
}

void testReadingMatch (TomoeContext* ctx, const char* reading)
{
    TomoeArray* matched = tomoe_context_search_by_reading (ctx, reading);
    int candidate_num = tomoe_array_size(matched);

    if (candidate_num != 0)
    {
        int i;

        if (!matched)
        {
            fprintf (stderr, "Candidate list is NULL!\n");
            return;
        }

        fprintf (stdout, "The number of matched characters: %d\n",
                 candidate_num);
        for (i = 0; i < candidate_num; i++)
        {
            TomoeChar* p = (TomoeChar*)tomoe_array_get (matched, i);
            outCharInfo (p, 0);
        }
    }
    else
        fprintf (stdout, "No Candidate found!\n");
}

void testUserDict (TomoeContext* ctx)
{
    TomoeChar* chr;
    TomoeDict* myDict = tomoe_dict_new ("../data/userdb.xml", 1);
    GPtrArray* readings = g_ptr_array_new ();
    gint i;

    fprintf (stdout, "dictSize %d; create character \"（＾o＾）／\" with reading \"やった\" and add to dictionary\n", 
             tomoe_dict_get_size (myDict));
    chr = tomoe_char_new (NULL);
    tomoe_char_set_code (chr, "（＾o＾）／");
    g_ptr_array_add (readings, g_strdup ("やった"));
    tomoe_char_set_readings (chr, readings);
    tomoe_dict_add_char (myDict, chr);

    tomoe_context_add_dict (ctx, myDict);

    fprintf (stdout, "dictSize %d; reading search with やった:\n", tomoe_dict_get_size (myDict));
    testReadingMatch (ctx, "やった");

    fprintf (stdout, "update character to \"\\\\（＾o＾）//\"\n");
    tomoe_char_set_code (chr, "\\\\（＾o＾）//");
    fprintf (stdout, "dictSize %d; reading search with やった:\n", tomoe_dict_get_size (myDict));
    testReadingMatch (ctx, "やった");

    fprintf (stdout, "update reading to \"yey\"\n");
# warning FIXME! we need some new nice interface
    for (i = 0; readings->len; i++) {
    	const gchar *remove_reading = "やった";
	gchar *reading = g_ptr_array_index (readings, i);
	if (reading && !strcmp (reading, remove_reading))
	{
		g_ptr_array_remove (readings, reading);
		g_free (reading);
	}
    }
    g_ptr_array_add (readings, g_strdup ("yey"));
    fprintf (stdout, "dictSize %d; reading search with やった:\n", tomoe_dict_get_size (myDict));
    testReadingMatch (ctx, "やった");
    fprintf (stdout, "dictSize %d; reading search with yey:\n", tomoe_dict_get_size (myDict));
    testReadingMatch (ctx, "yey");

    fprintf (stdout, "remove character \n");
    tomoe_dict_remove_by_char (myDict, chr);
    fprintf (stdout, "dictSize %d; reading search with yey:\n", tomoe_dict_get_size (myDict));
    testReadingMatch (ctx, "yey");

    tomoe_dict_save (myDict);

    tomoe_char_free (chr);
    tomoe_dict_free (myDict);
}

int
main (int argc, char **argv)
{
    TomoeContext* ctx = NULL;

    tomoe_init ();

    ctx = tomoe_simple_load ("test-config.xml");
    if (!ctx) exit (1);

    if (argc == 2 && 0 == strcmp (argv[1], "stroke"))
        testStrokeMatch (ctx);
    else if (argc == 3 && 0 == strcmp (argv[1], "reading"))
        testReadingMatch (ctx, argv[2]);
    else if (argc == 2 && 0 == strcmp (argv[1], "userdict"))
        testUserDict (ctx);
    else
        fprintf (stdout, "testtomoe [stroke|reading|userdict]\n");

    tomoe_context_free (ctx);
    tomoe_quit ();
    return 0;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
