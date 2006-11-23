/* -*- c-file-style: "ruby"; indent-tabs-mode: nil -*- */

#include "tomoe-rb.h"

#define _SELF(obj) RVAL2TQRY(obj)

static VALUE
tq_add_reading(VALUE self, VALUE reading)
{
    tomoe_query_add_reading(_SELF(self), RVAL2TRDG(reading));
    return Qnil;
}

static VALUE
tq_set_writing(VALUE self, VALUE writing)
{
    tomoe_query_set_writing(_SELF(self), RVAL2TWTG(writing));
    return Qnil;
}

void
Init_tomoe_query(VALUE mTomoe)
{
    VALUE cTomoeQuery;

    cTomoeQuery = G_DEF_CLASS(TOMOE_TYPE_QUERY, "Query", mTomoe);

    rb_define_method(cTomoeQuery, "add_reading", tq_add_reading, 1);
/*     rb_define_method(cTomoeQuery, "set_number_of_stroke", */
/*                      tq_set_number_of_stroke, 1); */
/*     rb_define_method(cTomoeQuery, "add_radical", tq_add_radical, 1); */
/*     rb_define_method(cTomoeQuery, "add_variant", tq_add_variant, 1); */
    rb_define_method(cTomoeQuery, "set_writing", tq_set_writing, 1);

    G_DEF_SETTERS(cTomoeQuery);
}