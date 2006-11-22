/* -*- c-file-style: "ruby"; indent-tabs-mode: nil -*- */

#include "tomoe-rb.h"

#define _SELF(obj) RVAL2TRDG(obj)

static VALUE
tr_initialize(VALUE self, VALUE type, VALUE reading)
{
    G_INITIALIZE(self, tomoe_reading_new(RVAL2TRT(type), RVAL2CSTR(reading)));
    return Qnil;
}

void
Init_tomoe_reading(VALUE mTomoe)
{
    VALUE cTomoeReading;

    cTomoeReading = G_DEF_CLASS(TOMOE_TYPE_READING, "Reading", mTomoe);
    G_DEF_CONSTANTS(mTomoe, TOMOE_TYPE_READING_TYPE, "TOMOE_");

    rb_define_method(cTomoeReading, "initialize", tr_initialize, 2);
}
