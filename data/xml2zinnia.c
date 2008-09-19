/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#if HAVE_UNISTD_H
#  include <unistd.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <glib/gstdio.h>

#include <tomoe.h>
#include <tomoe-dict.h>

#include <zinnia.h>

static void
generate_binary_data (zinnia_trainer_t *trainer, const gchar *model_file)
{
    gchar *text_file;
    gint fd;
    GError *error = NULL;

    fd = g_file_open_tmp (NULL, &text_file, &error);
    if (fd == -1) {
        g_warning ("g_file_open_tmp() is failed: %s", error->message);
        g_error_free (error);
        return;
    }
    close (fd);

    zinnia_trainer_train (trainer, text_file);
    zinnia_trainer_convert_model (text_file, model_file, 0.001);
    g_remove (text_file);
}

int
main (gint argc, gchar **argv)
{
    TomoeDict *xml_dict;
    TomoeQuery *query;
    GList *candidates, *node;
    zinnia_trainer_t *trainer;

    if (argc != 3) {
        g_print ("Usage: %s XXX.xml XXX.model\n", argv[0]);
        exit (EXIT_FAILURE);
    }

    tomoe_init ();

    xml_dict = tomoe_dict_new ("xml",
                               "filename", argv[1],
                               "editable", FALSE,
                               NULL);
    if (!xml_dict)
        return EXIT_FAILURE;
    trainer = zinnia_trainer_new ();

    query = tomoe_query_new ();
    candidates = tomoe_dict_search (xml_dict, query);
    for (node = candidates; node; node = g_list_next (node)) {
        TomoeCandidate *candidate = node->data;
        TomoeChar *tomoe_char;
        TomoeWriting *writing;
        zinnia_character_t *character;
        const GList *strokes;
        gsize i;

        tomoe_char = tomoe_candidate_get_char (candidate);
        writing = tomoe_char_get_writing (tomoe_char);
        character = zinnia_character_new ();
        zinnia_character_clear (character);
        zinnia_character_set_width (character, TOMOE_WRITING_WIDTH);
        zinnia_character_set_height (character, TOMOE_WRITING_HEIGHT);
        zinnia_character_set_value (character, tomoe_char_get_utf8 (tomoe_char));
        for (strokes = tomoe_writing_get_strokes(writing), i = 0;
             strokes;
             strokes = g_list_next (strokes), i++) {
            GList *points;

            for (points = strokes->data; points; points = g_list_next (points)) {
                TomoePoint *point = points->data;
                if (!zinnia_character_add (character, i, point->x, point->y))
                    g_warning ("%s", zinnia_character_strerror (character));
            }
        }
        if (!zinnia_trainer_add (trainer, character))
            g_warning ("%s", zinnia_trainer_strerror (trainer));
        zinnia_character_destroy (character);
    }
    g_list_foreach (candidates, (GFunc)g_object_unref, NULL);
    g_list_free (candidates);
    g_object_unref (xml_dict);

    generate_binary_data (trainer, argv[2]);
    zinnia_trainer_destroy (trainer);

    tomoe_quit ();

    return EXIT_SUCCESS;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
