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
    FILE *fp = stdin;
    TomoeGlyph *test_glyph = NULL;

    test_glyph = tomoe_glyph_new ();

    while ((p = fgets (line_buf, LINE_BUF_SIZE, fp)) != NULL) {
        if (strstr (p, "EOF") != NULL) {
            return NULL;
        }

        if (p[0] != ':') continue;

        sscanf (p + 1, "%d", &stroke_num);

        for (j = 0; j < stroke_num; j++) {
            p = fgets (line_buf, LINE_BUF_SIZE, fp);
            sscanf (p, "%d", &point_num);
            p = strchr (p, ' ');

            for (k = 0; k < point_num; k++) {
                int x, y;
                sscanf (p, " (%d %d)", &x, &y);
                p = strchr (p, ')') + 1;
                if (k == 0)
                    tomoe_glyph_move_to (test_glyph, x, y);
                else
                    tomoe_glyph_line_to (test_glyph, x, y);
            }
        }

        return test_glyph;
    }
    return NULL;
}

static void
output_character_meta_data (gpointer key, gpointer value, gpointer user_data)
{
    const gchar *meta_key = key;
    const gchar *meta_value = value;
    FILE *output = user_data;

    fprintf (output, "%s: %s\n", meta_key, meta_value);
}

void outCharInfo (TomoeChar* chr, int score)
{
   unsigned int j;
   GPtrArray *readings = tomoe_char_get_readings (chr);

   fprintf (stdout, "character:%s [%d] ", tomoe_char_get_code (chr), score);
   fflush (stdout);
   if (readings->len) {
       for (j = 0; j < readings->len; j++) {
           const char* r = g_ptr_array_index (readings, j);
           fprintf (stdout, " %s", r);
       }
       fprintf (stdout, "\n");
   }
   tomoe_char_meta_data_foreach (chr, output_character_meta_data, stdout);
   fprintf (stdout, "\n");
}

void testStrokeMatch (TomoeContext* ctx)
{
    TomoeGlyph *test_glyph = NULL;
    GList *matched = NULL;

    test_glyph = read_test_data ();

    if (!test_glyph) 
        goto END;

    matched = tomoe_context_search_by_strokes (ctx, test_glyph);

    if (!matched) {
        fprintf (stdout, "No Candidate found!\n");
    } else {
        GList *candidate;
        fprintf (stdout, "The number of matched characters: %d\n",
                 g_list_length (matched));
        for (candidate = matched; candidate; candidate = candidate->next) {
            TomoeCandidate* p = candidate->data;
            outCharInfo (tomoe_candidate_get_character (p),
                         tomoe_candidate_get_score (p));
        }
    }
#warning FIXME! plug memory leak!
END:
    if (!test_glyph)
        g_object_unref (G_OBJECT (test_glyph));
    if (matched) {
        g_list_foreach (matched, (GFunc) g_object_unref, NULL);
        g_list_free (matched);
    }
}

void testReadingMatch (TomoeContext* ctx, const char* reading)
{
    GList *results = tomoe_context_search_by_reading (ctx, reading);
    guint candidate_num = g_list_length(results);

    if (candidate_num != 0) {
        GList *result;

        if (!results) {
            fprintf (stderr, "Candidate list is NULL!\n");
            return;
        }

        fprintf (stdout, "The number of matched characters: %d\n",
                 candidate_num);
        for (result = results; result; result = result->next) {
            TomoeCandidate *p = result->data;
            outCharInfo (tomoe_candidate_get_character (p),
                         tomoe_candidate_get_score (p));
        }
    } else {
        fprintf (stdout, "No Candidate found!\n");
    }

    g_list_foreach (results, (GFunc) g_object_unref, NULL);
    g_list_free (results);
}

void testUserDict (TomoeContext* ctx)
{
    TomoeChar* chr;
    TomoeDict* myDict = tomoe_dict_new ("../data/userdb.xml", 1);
    GPtrArray* readings = g_ptr_array_new ();
    gint i;

    fprintf (stdout, "dictSize %d; create character \"（＾o＾）／\" with reading \"やった\" and add to dictionary\n", 
             tomoe_dict_get_size (myDict));
    chr = tomoe_char_new ();
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
    for (i = readings->len - 1; i >= 0; i--) {
    	const gchar *remove_reading = "やった";
        gchar *reading = g_ptr_array_index (readings, i);
        if (reading && !strcmp (reading, remove_reading)) {
            gchar *p = g_ptr_array_remove_index (readings, i);
            g_free (p);
        }
    }
    g_ptr_array_add (readings, g_strdup ("yey"));
    tomoe_char_set_readings (chr, readings);
    g_ptr_array_foreach (readings, (GFunc) g_free, NULL); 
    g_ptr_array_free (readings, TRUE);

    fprintf (stdout, "dictSize %d; reading search with やった:\n", tomoe_dict_get_size (myDict));
    testReadingMatch (ctx, "やった");
    fprintf (stdout, "dictSize %d; reading search with yey:\n", tomoe_dict_get_size (myDict));
    testReadingMatch (ctx, "yey");

    fprintf (stdout, "remove character \n");
    tomoe_dict_remove_by_char (myDict, chr);
    fprintf (stdout, "dictSize %d; reading search with yey:\n", tomoe_dict_get_size (myDict));
    testReadingMatch (ctx, "yey");

    g_object_unref (G_OBJECT (chr));
    tomoe_dict_save (myDict);
    g_object_unref (myDict);
}

int
main (int argc, char **argv)
{
    TomoeContext* ctx = NULL;

    tomoe_init ();

    ctx = tomoe_context_new();
    if (!ctx) exit (1);
    tomoe_context_load_config (ctx, "test-config.xml");

    if (argc == 2 && 0 == strcmp (argv[1], "stroke"))
        testStrokeMatch (ctx);
    else if (argc == 3 && 0 == strcmp (argv[1], "reading"))
        testReadingMatch (ctx, argv[2]);
    else if (argc == 2 && 0 == strcmp (argv[1], "userdict"))
        testUserDict (ctx);
    else
        fprintf (stdout, "testtomoe [stroke|reading|userdict]\n");

    g_object_unref (ctx);
    tomoe_quit ();
    return 0;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
