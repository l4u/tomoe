/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tomoe.h"

static tomoe_glyph * read_test_data ();
void outCharInfo (tomoe_char* chr, int score);
void testStrokeMatch (tomoe_db* db);
void testReadingMatch (tomoe_db* db, const char* reading);
void testUserDB (tomoe_db* db);

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

void outCharInfo (tomoe_char* chr, int score)
{
   int j;
   tomoe_array* readings = tomoe_char_getReadings (chr);
   const char* meta = tomoe_char_getMeta (chr);

   fprintf (stdout, "character:%s [%d] ", tomoe_char_getCode (chr), score);
   fflush (stdout);
   if (readings)
   {
       int reading_num = tomoe_array_size (readings);
       for (j = 0; j < reading_num; j++)
       {
           const char* r = tomoe_array_getConst (readings, j);
           fprintf (stdout, " %s", r);
       }
       fprintf (stdout, "\n");
   }
   if (meta)
       fprintf (stdout, meta);
   fprintf (stdout, "\n");
}

void testStrokeMatch (tomoe_db* db)
{
    tomoe_glyph *test_glyph = NULL;
    int i, candidate_num = 0;
    tomoe_array* matched = NULL;

    test_glyph = read_test_data ();

    if (!test_glyph) 
        goto END;

    matched = tomoe_db_searchByStrokes (db, test_glyph);
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
            const tomoe_candidate* p = (const tomoe_candidate*)tomoe_array_getConst (matched, i);
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

void testReadingMatch (tomoe_db* db, const char* reading)
{
    tomoe_array* matched = tomoe_db_searchByReading (db, reading);
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
            tomoe_char* p = (tomoe_char*)tomoe_array_get (matched, i);
            outCharInfo (p, 0);
        }
    }
    else
        fprintf (stdout, "No Candidate found!\n");
}

void testUserDB (tomoe_db* db)
{
    tomoe_char* chr;
    tomoe_dict* myDict = tomoe_dict_new ("../data/userdb.xml", 1);
    tomoe_array* readings = tomoe_array_new ((tomoe_compare_fn)tomoe_string_compare,
                                             NULL,
                                             (tomoe_free_fn)free);

    fprintf (stdout, "dictSize %d; create character \"（＾o＾）／\" with reading \"やった\" and add to dictionary\n", 
             tomoe_dict_getSize (myDict));
    chr = tomoe_char_new (NULL);
    tomoe_char_setCode (chr, "（＾o＾）／");
    tomoe_array_append (readings, strdup ("やった"));
    tomoe_char_setReadings (chr, readings);
    tomoe_dict_addChar (myDict, chr);

    tomoe_db_addDict (db, myDict);

    fprintf (stdout, "dictSize %d; reading search with やった:\n", tomoe_dict_getSize (myDict));
    testReadingMatch (db, "やった");

    fprintf (stdout, "update character to \"\\\\（＾o＾）//\"\n");
    tomoe_char_setCode (chr, "\\\\（＾o＾）//");
    fprintf (stdout, "dictSize %d; reading search with やった:\n", tomoe_dict_getSize (myDict));
    testReadingMatch (db, "やった");

    fprintf (stdout, "update reading to \"yey\"\n");
    tomoe_array_remove (readings, tomoe_array_find (readings, "やった"));
    tomoe_array_append (readings, "yey");
    fprintf (stdout, "dictSize %d; reading search with やった:\n", tomoe_dict_getSize (myDict));
    testReadingMatch (db, "やった");
    fprintf (stdout, "dictSize %d; reading search with yey:\n", tomoe_dict_getSize (myDict));
    testReadingMatch (db, "yey");

    fprintf (stdout, "remove character \n");
    tomoe_dict_removeByChar (myDict, chr);
    fprintf (stdout, "dictSize %d; reading search with yey:\n", tomoe_dict_getSize (myDict));
    testReadingMatch (db, "yey");

    tomoe_dict_save (myDict);

    tomoe_char_free (chr);
    tomoe_dict_free (myDict);
}

int
main (int argc, char **argv)
{
    tomoe_db* db = NULL;

    tomoe_init ();

    db = tomoe_simple_load ("test-config.xml");
    if (!db) exit (1);

    if (argc == 2 && 0 == strcmp (argv[1], "stroke"))
        testStrokeMatch (db);
    else if (argc == 3 && 0 == strcmp (argv[1], "reading"))
        testReadingMatch (db, argv[2]);
    else if (argc == 2 && 0 == strcmp (argv[1], "userdb"))
        testUserDB (db);
    else
        fprintf (stdout, "testtomoe [stroke|reading|userdb]\n");

    tomoe_db_free (db);
    tomoe_term ();
    return 0;
}
/*
vi:ts=2:nowrap:ai:expandtab
*/
