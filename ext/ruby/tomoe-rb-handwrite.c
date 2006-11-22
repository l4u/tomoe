/* -*- c-file-style: "ruby"; indent-tabs-mode: nil -*- */

#include "tomoe-rb.h"

#define _SELF(obj) RVAL2TGLYPH(obj)

static VALUE
tg_move_to(VALUE self, VALUE x, VALUE y)
{
    tomoe_glyph_move_to(_SELF(self), NUM2INT(x), NUM2INT(y));
    return Qnil;
}

static VALUE
tg_line_to(VALUE self, VALUE x, VALUE y)
{
    tomoe_glyph_line_to(_SELF(self), NUM2INT(x), NUM2INT(y));
    return Qnil;
}

static VALUE
tg_clear(VALUE self)
{
    tomoe_glyph_clear(_SELF(self));
    return Qnil;
}

static VALUE
tg_get_number_of_strokes(VALUE self)
{
    return UINT2NUM(tomoe_glyph_get_number_of_strokes(_SELF(self)));
}

static VALUE
tg_get_number_of_points(VALUE self, VALUE stroke)
{
    return UINT2NUM(tomoe_glyph_get_number_of_points(_SELF(self),
                                                     NUM2UINT(stroke)));
}

static VALUE
tg_get_point(VALUE self, VALUE stroke, VALUE point)
{
    gint x, y;

    if (tomoe_glyph_get_point(_SELF(self), NUM2UINT(stroke), NUM2UINT(point),
                              &x, &y)) {
        return rb_ary_new3(2, INT2NUM(x), INT2NUM(y));
    } else {
        return Qnil;
    }
}

static VALUE
tg_get_last_point(VALUE self)
{
    gint x, y;

    if (tomoe_glyph_get_last_point(_SELF(self), &x, &y)) {
        return rb_ary_new3(2, INT2NUM(x), INT2NUM(y));
    } else {
        return Qnil;
    }
}

static VALUE
tg_remove_last_stroke(VALUE self)
{
    tomoe_glyph_remove_last_stroke(_SELF(self));
    return Qnil;
}

static VALUE
tg_each(VALUE self)
{
    int i, j;
    guint number_of_strokes, number_of_points;
    TomoeGlyph *glyph;

    glyph = _SELF(self);
    number_of_strokes = tomoe_glyph_get_number_of_strokes(glyph);
    for (i = 0; i < number_of_strokes; i++) {
        VALUE points;
        number_of_points = tomoe_glyph_get_number_of_points(glyph, i);

        points = rb_ary_new2(number_of_points);
        for (j = 0; j < number_of_points; j++) {
            gint x, y;
            if (tomoe_glyph_get_point(glyph, i, j, &x, &y))
                rb_ary_push(points, rb_ary_new3(2, INT2NUM(x), INT2NUM(y)));
        }
        rb_yield(points);
    }

    return Qnil;
}

void
Init_tomoe_handwrite(VALUE mTomoe)
{
    VALUE cTomoeGlyph;

    cTomoeGlyph = G_DEF_CLASS(TOMOE_TYPE_GLYPH, "Glyph", mTomoe);

    rb_include_module(cTomoeGlyph, rb_mEnumerable);

    rb_define_method(cTomoeGlyph, "move_to", tg_move_to, 2);
    rb_define_method(cTomoeGlyph, "line_to", tg_line_to, 2);
    rb_define_method(cTomoeGlyph, "clear", tg_clear, 0);
    rb_define_method(cTomoeGlyph, "number_of_strokes",
                     tg_get_number_of_strokes, 0);
    rb_define_method(cTomoeGlyph, "get_number_of_points",
                     tg_get_number_of_points, 1);
    rb_define_method(cTomoeGlyph, "[]", tg_get_point, 2);
    rb_define_method(cTomoeGlyph, "last_point", tg_get_last_point, 0);
    rb_define_method(cTomoeGlyph, "remove_last_stroke",
                     tg_remove_last_stroke, 0);

    rb_define_method(cTomoeGlyph, "each", tg_each, 0);
}
