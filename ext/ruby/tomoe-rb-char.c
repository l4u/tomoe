#include "tomoe-rb.h"

#define _SELF(obj) RVAL2TCHR(obj)

static VALUE
tc_get_readings(VALUE self)
{
    return GLIST2ARY((GList *)tomoe_char_get_readings(_SELF(self)));
}

static VALUE
tc_add_reading(VALUE self, VALUE reading)
{
    tomoe_char_add_reading(_SELF(self), RVAL2TRDG(reading));
    return Qnil;
}

static VALUE
tc_get_radicals(VALUE self)
{
    return GLIST2ARY((GList *)tomoe_char_get_radicals(_SELF(self)));
}

static VALUE
tc_add_radical(VALUE self, VALUE radical)
{
    tomoe_char_add_radical(_SELF(self), RVAL2TCHR(radical));
    return Qnil;
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

    rb_define_method(cTomoeChar, "readings", tc_get_readings, 0);
    rb_define_method(cTomoeChar, "add_reading", tc_add_reading, 1);
    rb_define_method(cTomoeChar, "radicals", tc_get_radicals, 0);
    rb_define_method(cTomoeChar, "add_radical", tc_add_radical, 1);

    rb_define_method(cTomoeChar, "to_xml", tc_to_xml, 0);
}
