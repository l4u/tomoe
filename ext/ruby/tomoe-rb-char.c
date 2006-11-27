#include "tomoe-rb.h"

#define _SELF(obj) (TOMOE_CHAR(RVAL2GOBJ(obj)))

static VALUE
tc_get_code(VALUE self)
{
    return CSTR2RVAL(tomoe_char_get_code(_SELF(self)));
}

static VALUE
tc_get_writing(VALUE self)
{
    return GOBJ2RVAL(tomoe_char_get_writing(_SELF(self)));
}

void
Init_tomoe_char(VALUE mTomoe)
{
    VALUE cTomoeChar;

    cTomoeChar = G_DEF_CLASS(TOMOE_TYPE_CHAR, "Char", mTomoe);

    rb_define_method(cTomoeChar, "code", tc_get_code, 0);
    rb_define_method(cTomoeChar, "writing", tc_get_writing, 0);
}

