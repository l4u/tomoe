/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) 2007 Kouhei Sutou <kou@cozmixng.org>
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
 *  $Id: tomoe-unihan.h 1490 2007-06-16 10:21:33Z ikezoe $
 */

#ifndef __TOMOE_UNIHAN_DATA_H__
#define __TOMOE_UNIHAN_DATA_H__

#include "tomoe-unihan.h"

#include <tomoe-char.h>

G_BEGIN_DECLS

typedef struct _TomoeUnihanReading TomoeUnihanReading;
typedef struct _TomoeUnihanMetaData TomoeUnihanMetaData;
typedef struct _TomoeUnihanInfo TomoeUnihanInfo;

struct _TomoeUnihanReading {
    TomoeReadingType  type;
    gchar            *reading;
};

struct _TomoeUnihanMetaData {
    gchar *key;
    gchar *value;
};

struct _TomoeUnihanInfo {
    gchar                *utf8;
    gint                  n_strokes;
    TomoeUnihanReading   *readings;
    gint                  readings_size;
    gchar               **radicals;
    gint                  radicals_size;
    gchar               **variants;
    gint                  variants_size;
    TomoeUnihanMetaData  *meta_data;
    gint                  meta_data_size;
};

TomoeUnihanInfo *_tomoe_unihan_data0(void);
gint			 _tomoe_unihan_data0_size(void);
TomoeUnihanInfo *_tomoe_unihan_data1(void);
gint			 _tomoe_unihan_data1_size(void);
TomoeUnihanInfo *_tomoe_unihan_data2(void);
gint			 _tomoe_unihan_data2_size(void);
TomoeUnihanInfo *_tomoe_unihan_data3(void);
gint			 _tomoe_unihan_data3_size(void);
TomoeUnihanInfo *_tomoe_unihan_data4(void);
gint			 _tomoe_unihan_data4_size(void);
TomoeUnihanInfo *_tomoe_unihan_data5(void);
gint			 _tomoe_unihan_data5_size(void);
TomoeUnihanInfo *_tomoe_unihan_data6(void);
gint			 _tomoe_unihan_data6_size(void);
TomoeUnihanInfo *_tomoe_unihan_data7(void);
gint			 _tomoe_unihan_data7_size(void);
TomoeUnihanInfo *_tomoe_unihan_data8(void);
gint			 _tomoe_unihan_data8_size(void);
TomoeUnihanInfo *_tomoe_unihan_data9(void);
gint			 _tomoe_unihan_data9_size(void);

G_END_DECLS

#endif /*  __TOMOE_UNIHAN_DATA_H__ */

/*
 * vi:ts=4:nowrap:ai:expandtab
 * */
