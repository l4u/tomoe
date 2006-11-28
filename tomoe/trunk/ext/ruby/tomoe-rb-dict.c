#include "tomoe-rb.h"

#define _SELF(obj) RVAL2TDIC(obj)

static VALUE
td_initialize(VALUE self, VALUE filename, VALUE editable,
              VALUE rb_base_dir, VALUE rb_name)
{
    gchar *base_dir, *name;

    base_dir = NIL_P(rb_base_dir) ? NULL : RVAL2CSTR(rb_base_dir);
    name = NIL_P(rb_name) ? NULL : RVAL2CSTR(rb_name);
    G_INITIALIZE(self, tomoe_dict_new(RVAL2CSTR(filename),
                                      RVAL2CBOOL(editable),
                                      base_dir, name));
    return Qnil;
}

static VALUE
td_get_char(VALUE self, VALUE code_point)
{
    return GOBJ2RVAL(tomoe_dict_get_char(_SELF(self), RVAL2CSTR(code_point)));
}


void
Init_tomoe_dict(VALUE mTomoe)
{
    VALUE cTomoeDict;

    cTomoeDict = G_DEF_CLASS(TOMOE_TYPE_DICT, "Dict", mTomoe);

    rb_define_method(cTomoeDict, "initialize", td_initialize, 4);

    rb_define_method(cTomoeDict, "[]", td_get_char, 1);
}
