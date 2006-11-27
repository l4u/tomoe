#include "tomoe-rb.h"

#define _SELF(obj) (TOMOE_CHAR(RVAL2GOBJ(obj)))

static VALUE
tc_get_code(VALUE self)
{
    return CSTR2RVAL(tomoe_char_get_code(_SELF(self)));
}

static VALUE
tc_set_code(VALUE self, VALUE code)
{
    tomoe_char_set_code(_SELF(self), RVAL2CSTR(code));
    return Qnil;
}

static VALUE
tc_get_writing(VALUE self)
{
    return GOBJ2RVAL(tomoe_char_get_writing(_SELF(self)));
}

static VALUE
tc_to_xml(VALUE self)
{
    gchar *xml;

    xml = tomoe_char_to_xml(_SELF(self));
    if (xml) {
        VALUE result;

        result = CSTR2RVAL(xml);
        g_free(xml);
        return result;
    } else {
        return Qnil;
    }
}

void
Init_tomoe_char(VALUE mTomoe)
{
    VALUE cTomoeChar;

    cTomoeChar = G_DEF_CLASS(TOMOE_TYPE_CHAR, "Char", mTomoe);

    rb_define_method(cTomoeChar, "code", tc_get_code, 0);
    rb_define_method(cTomoeChar, "code=", tc_set_code, 1);
    rb_define_method(cTomoeChar, "writing", tc_get_writing, 0);
    rb_define_method(cTomoeChar, "to_xml", tc_to_xml, 0);
}

