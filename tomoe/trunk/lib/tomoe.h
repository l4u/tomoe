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
#include "tomoe-db.h"

/**
 * @brief Initialize tomoe library.
 */
void       tomoe_init          (void);

tomoe_db*  tomoe_simple_load   (const char* configFile);

/**
 * @brief Finalize tomoe library.
 */
void       tomoe_term          (void);

#ifdef	__cplusplus
}
#endif

#endif /* __TOMOE_H__ */
