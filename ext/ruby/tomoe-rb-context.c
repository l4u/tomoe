/* -*- c-file-style: "ruby"; indent-tabs-mode: nil -*- */

#include "tomoe-rb.h"

#define _SELF(obj) RVAL2TCTX(obj)

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

static VALUE
tc_search(VALUE self, VALUE query)
{
    return GLIST2ARYF(tomoe_context_search(_SELF(self), RVAL2TQRY(query)));
}

/* static VALUE */
/* tc_advanced_search(VALUE self, VALUE rb_queries) */
/* { */
/*     return GLIST2ARYF(tomoe_context_advanced_search(_SELF(self), */
/*                                                     RVAL2GLIST(queries))); */
/* } */


void
Init_tomoe_context(VALUE mTomoe)
{
    VALUE cTomoeContext;

    cTomoeContext = G_DEF_CLASS(TOMOE_TYPE_CONTEXT, "Context", mTomoe);

    rb_define_method(cTomoeContext, "add_dict", tc_add_dict, 1);
    rb_define_method(cTomoeContext, "load_config", tc_load_config, -1);
    rb_define_method(cTomoeContext, "search", tc_search, 1);
/*     rb_define_method(cTomoeContext, "advanced_search", tc_advanced_search, 1); */
}
