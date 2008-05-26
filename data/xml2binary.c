/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <stdlib.h>
#include <string.h>
#include <glib/gstdio.h>

#include <tomoe.h>
#include <tomoe-dict.h>

int
main (gint argc, gchar **argv)
{
    TomoeDict *xml_dict, *binary_dict;
    gchar *binary_file_name;

    if (argc != 2) {
        g_print ("Usage: %s XXX.xml\n", argv[0]);
        exit (EXIT_FAILURE);
    }

    tomoe_init ();

    xml_dict = tomoe_dict_new ("xml",
                               "filename", argv[1],
                               "editable", FALSE,
                               NULL);

    if (g_str_has_suffix (argv[1], ".xml")) {
        gchar *base_name;
        base_name = g_strndup (argv[1], strlen (argv[1]) - strlen (".xml"));
        binary_file_name = g_strconcat (base_name, ".bin", NULL);
        g_free (base_name);
    } else {
        binary_file_name = g_strconcat (argv[1], ".bin", NULL);
    }
    binary_dict = tomoe_dict_new ("binary",
                                  "filename", binary_file_name,
                                  "editable", TRUE,
                                  NULL);
    g_free(binary_file_name);

    tomoe_dict_copy (xml_dict, binary_dict);

    g_object_unref (xml_dict);
    g_object_unref (binary_dict);

    tomoe_quit ();

    return EXIT_SUCCESS;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
