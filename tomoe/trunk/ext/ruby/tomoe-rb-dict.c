#include "tomoe-rb.h"

#define _SELF(obj) RVAL2TDIC(obj)

static VALUE
td_s_load(VALUE self, VALUE base_dir)
{
    tomoe_dict_load(NIL_P(base_dir) ? NULL : RVAL2CSTR(base_dir));
    return Qnil;
}

static VALUE
td_s_unload(VALUE self)
{
    tomoe_dict_unload();
    return Qnil;
}

static VALUE
td_s_new(VALUE self, VALUE name, VALUE filename, VALUE editable)
{
    return GOBJ2RVAL(tomoe_dict_new(RVAL2CSTR(name),
                                    "filename", RVAL2CSTR(filename),
                                    "editable", RVAL2CBOOL(editable),
                                    NULL));
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

    rb_define_singleton_method(cTomoeDict, "load", td_s_load, 1);
    rb_define_singleton_method(cTomoeDict, "unload", td_s_unload, 0);

    rb_define_singleton_method(cTomoeDict, "new", td_s_new, 3);

    rb_define_method(cTomoeDict, "[]", td_get_char, 1);
}
