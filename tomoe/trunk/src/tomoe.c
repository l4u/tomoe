/*
 *  Copyright (C) 2000 - 2004 Hiroyuki Komatsu <komatsu@taiyaki.org>
 *  Copyright (C) 2004 Hiroaki Nakamura <hnakamur@good-day.co.jp>
 *  Copyright (C) 2005 Hiroyuki Ikezoe <poincare@ikezoe.net>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>
#include "tomoe.h"

#define LINE_BUF_SIZE 4096
#define DICT_LETTER_INITIAL_SIZE 3049
#define DICT_LETTER_EXPAND_SIZE 10
#define LIMIT_LENGTH ((300 * 0.25) * (300 * 0.25))

typedef struct _letter letter;
typedef struct _dictionary dictionary;

struct dictionary
{
  char *file_name;
  char *dictionary_name;
  char *encoding;
  char *lang;
};

struct _letter
{
  char *character;
  int stroke_num;
  stroke *strokes;
};


/* 
 * Initialize tomoe 
 */
void
tomoe_init (void)
{
  /* load all available dictionaries */
  /* Not implemented yet*/
}

/* 
 * get matched characters 
 *
 * matched:
 * Return value: the number of matched characters
 *
 */
int
tomoe_get_matched (glyph *input, candidates **matched)
{
  int matched_num = 0;
  /* Not implemented yet*/
  return matched_num;
}

/* 
 * free matched characters 
 *
 * matched:
 *
 */
void
tomoe_free_matched (candidates *matched)
{
  /* Not implemented yet*/
}

/* register characters with stroke */
tomoe_bool
tomoe_data_register (glyph *input, char *data)
{
  tomoe_bool ret = TRUE;
  /* Not implemented yet*/
  return ret;
}

/* finalize tomoe */
void 
tomoe_term (void)
{
  /* Not implemented yet*/
}

