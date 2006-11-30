#include "tomoe-rb.h"

#define _SELF(obj) RVAL2TCHR(obj)

static VALUE
tc_initialize(int argc, VALUE *argv, VALUE self)
{
    VALUE xml;
    TomoeChar *chr;

    rb_scan_args (argc, argv, "01", &xml);

    if (NIL_P(xml)) {
        chr = tomoe_char_new();
    } else {
        chr = tomoe_char_new_from_xml_data(RVAL2CSTR(xml), RSTRING(xml)->len);
    }

    G_INITIALIZE(self, chr);
    return Qnil;
}

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
    VALUE radicals;
    const GList *node;

    radicals = rb_ary_new ();
    for (node = tomoe_char_get_radicals(_SELF(self));
         node;
         node = g_list_next (node)) {
        rb_ary_push (radicals, CSTR2RVAL (node->data));
    }
    return radicals;
}

static VALUE
tc_add_radical(VALUE self, VALUE radical)
{
    tomoe_char_add_radical(_SELF(self), RVAL2CSTR(radical));
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

static VALUE
tc_compare(VALUE self, VALUE other)
{
    return INT2NUM(tomoe_char_compare(_SELF(self), RVAL2TCHR(other)));
}

void
Init_tomoe_char(VALUE mTomoe)
{
    VALUE cTomoeChar;

    cTomoeChar = G_DEF_CLASS(TOMOE_TYPE_CHAR, "Char", mTomoe);
    rb_include_module(cTomoeChar, rb_mComparable);

    rb_define_method(cTomoeChar, "initialize", tc_initialize, -1);

    rb_define_method(cTomoeChar, "<=>", tc_compare, 1);

    rb_define_method(cTomoeChar, "readings", tc_get_readings, 0);
    rb_define_method(cTomoeChar, "add_reading", tc_add_reading, 1);
    rb_define_method(cTomoeChar, "radicals", tc_get_radicals, 0);
    rb_define_method(cTomoeChar, "add_radical", tc_add_radical, 1);

    rb_define_method(cTomoeChar, "to_xml", tc_to_xml, 0);
}
