#include "tomoe-rb.h"

#define _SELF(obj) RVAL2TDIC(obj)

static VALUE
td_initialize(VALUE self, VALUE filename, VALUE editable)
{
    G_INITIALIZE(self, tomoe_dict_new(RVAL2CSTR(filename),
                                      RVAL2CBOOL(editable)));
    return Qnil;
}

static VALUE
td_save(VALUE self)
{
    tomoe_dict_save(_SELF(self));
    return Qnil;
}


void
Init_tomoe_dict(VALUE mTomoe)
{
    VALUE cTomoeDict;

    cTomoeDict = G_DEF_CLASS(TOMOE_TYPE_DICT, "Dict", mTomoe);

    rb_define_method(cTomoeDict, "initialize", td_initialize, 2);

    rb_define_method(cTomoeDict, "save", td_save, 0);
}
