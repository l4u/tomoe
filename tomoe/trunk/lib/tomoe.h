/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2000 - 2004 Hiroyuki Komatsu <komatsu@taiyaki.org>
 *  Copyright (C) 2004 Hiroaki Nakamura <hnakamur@good-day.co.jp>
 *  Copyright (C) 2005 Hiroyuki Ikezoe <poincare@ikezoe.net>
 *  Copyright (C) 2005 Takuro Ashie <ashie@homa.ne.jp>
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

/** @file tomoe.h
 *  @brief Provide a set of API for handwriting recognition.
 */

#ifndef __TOMOE_H__
#define __TOMOE_H__

#ifdef	__cplusplus
extern "C" {
#endif

#include "tomoe-data-types.h"
#include "tomoe-dict.h"

/**
 * @brief Initialize tomoe library.
 */
void       tomoe_init          (void);

/**
 * @brief Finalize tomoe library.
 */
void       tomoe_term          (void);


#if 1
/**
 * @brief Get matched characters 
 * @param input    - matched candidates
 * @param matched  - 
 * @return         - the number of matched characters
 */
int        tomoe_get_matched   (tomoe_glyph       *input,
                                tomoe_candidate ***matched);
/**
 * @brief Free matched characters.
 * @param matched - Array of pointer to matched candidates to free.
 * @param len     - Length of candidates array.
 */
void       tomoe_free_matched  (tomoe_candidate  **matched,
                                int                len);

/**
 * @brief Register to the current user dictionary.
 * @param input - Stroke data.
 * @param data  - Characters to register to the current user dictionary.
 * @return      - 
 */
tomoe_bool tomoe_data_register (tomoe_glyph       *input,
                                char              *data);
#endif

#ifdef	__cplusplus
}
#endif

#endif /* __TOMOE_H__ */
