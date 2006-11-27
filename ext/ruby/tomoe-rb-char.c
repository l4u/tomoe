#include "tomoe-rb.h"

#define _SELF(obj) (TOMOE_CHAR(RVAL2GOBJ(obj)))

static VALUE
tc_get_utf8(VALUE self)
{
    return CSTR2RVAL(tomoe_char_get_utf8(_SELF(self)));
}

static VALUE
tc_set_utf8(VALUE self, VALUE utf8)
{
    tomoe_char_set_utf8(_SELF(self), RVAL2CSTR(utf8));
    return Qnil;
}

static VALUE
tc_get_n_strokes(VALUE self)
{
    return INT2NUM(tomoe_char_get_n_strokes(_SELF(self)));
}

static VALUE
tc_set_n_strokes(VALUE self, VALUE n_strokes)
{
    tomoe_char_set_n_strokes(_SELF(self), NUM2INT(n_strokes));
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

    rb_define_method(cTomoeChar, "utf8", tc_get_utf8, 0);
    rb_define_method(cTomoeChar, "utf8=", tc_set_utf8, 1);
    rb_define_method(cTomoeChar, "n_strokes", tc_get_n_strokes, 0);
    rb_define_method(cTomoeChar, "n_strokes=", tc_set_n_strokes, 1);
    rb_define_method(cTomoeChar, "writing", tc_get_writing, 0);

    rb_define_method(cTomoeChar, "to_xml", tc_to_xml, 0);
}

