/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tomoe.h"

int
main (int argc, char **argv)
{
    int i, candidate_num = 0;
    tomoe_array* matched = NULL;
    tomoe_db* db = NULL;

    if (argc != 2) exit (1);

    db = tomoe_init ();
    if (!db) exit (1);

    matched = tomoe_db_get_reading (db, argv[1]);
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
    if (matched)
        tomoe_array_free (matched);
    tomoe_db_free (db);
    tomoe_term ();
    return 0;
}
/*
vi:ts=2:nowrap:ai:expandtab
*/
 
