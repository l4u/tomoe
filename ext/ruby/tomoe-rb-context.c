/* -*- c-file-style: "ruby"; indent-tabs-mode: nil -*- */

#include "tomoe-rb.h"

#define _SELF(obj) (TOMOE_CONTEXT(RVAL2GOBJ(obj)))

static VALUE
tc_add_dict(VALUE self, VALUE dict)
{
    tomoe_context_add_dict(_SELF(self), RVAL2TDIC(dict));
    return Qnil;
}

static VALUE
tc_load_config(int argc, VALUE *argv, VALUE self)
{
    VALUE filename;

    rb_scan_args(argc, argv, "01", &filename);

    tomoe_context_load_config(_SELF(self),
                              NIL_P(filename) ? NULL : RVAL2CSTR(filename));
    return Qnil;
}


void
Init_tomoe_context(VALUE mTomoe)
{
    VALUE cTomoeContext;

    cTomoeContext = G_DEF_CLASS(TOMOE_TYPE_CONTEXT, "Context", mTomoe);

    rb_define_method(cTomoeContext, "add_dict", tc_add_dict, 1);
    rb_define_method(cTomoeContext, "load_config", tc_load_config, -1);
}
