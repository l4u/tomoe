/* -*- c-file-style: "ruby"; indent-tabs-mode: nil -*- */

#include "tomoe-rb.h"

#define _SELF(obj) RVAL2TWTG(obj)

static VALUE
tw_move_to(VALUE self, VALUE x, VALUE y)
{
    tomoe_writing_move_to(_SELF(self), NUM2INT(x), NUM2INT(y));
    return Qnil;
}

static VALUE
tw_line_to(VALUE self, VALUE x, VALUE y)
{
    tomoe_writing_line_to(_SELF(self), NUM2INT(x), NUM2INT(y));
    return Qnil;
}

static VALUE
tw_clear(VALUE self)
{
    tomoe_writing_clear(_SELF(self));
    return Qnil;
}

static VALUE
tw_get_number_of_strokes(VALUE self)
{
    return UINT2NUM(tomoe_writing_get_number_of_strokes(_SELF(self)));
}

static VALUE
tw_get_number_of_points(VALUE self, VALUE stroke)
{
    return UINT2NUM(tomoe_writing_get_number_of_points(_SELF(self),
                                                       NUM2UINT(stroke)));
}

static VALUE
tw_get_point(VALUE self, VALUE stroke, VALUE point)
{
    gint x, y;

    if (tomoe_writing_get_point(_SELF(self), NUM2UINT(stroke), NUM2UINT(point),
                              &x, &y)) {
        return rb_ary_new3(2, INT2NUM(x), INT2NUM(y));
    } else {
        return Qnil;
    }
}

static VALUE
tw_get_last_point(VALUE self)
{
    gint x, y;

    if (tomoe_writing_get_last_point(_SELF(self), &x, &y)) {
        return rb_ary_new3(2, INT2NUM(x), INT2NUM(y));
    } else {
        return Qnil;
    }
}

static VALUE
tw_remove_last_stroke(VALUE self)
{
    tomoe_writing_remove_last_stroke(_SELF(self));
    return Qnil;
}

static VALUE
tw_each(VALUE self)
{
    int i, j;
    guint number_of_strokes, number_of_points;
    TomoeWriting *writing;

    writing = _SELF(self);
    number_of_strokes = tomoe_writing_get_number_of_strokes(writing);
    for (i = 0; i < number_of_strokes; i++) {
        VALUE points;
        number_of_points = tomoe_writing_get_number_of_points(writing, i);

        points = rb_ary_new2(number_of_points);
        for (j = 0; j < number_of_points; j++) {
            gint x, y;
            if (tomoe_writing_get_point(writing, i, j, &x, &y))
                rb_ary_push(points, rb_ary_new3(2, INT2NUM(x), INT2NUM(y)));
        }
        rb_yield(points);
    }

    return Qnil;
}

void
Init_tomoe_handwrite(VALUE mTomoe)
{
    VALUE cTomoeWriting;

    cTomoeWriting = G_DEF_CLASS(TOMOE_TYPE_WRITING, "Writing", mTomoe);

    rb_include_module(cTomoeWriting, rb_mEnumerable);

    rb_define_method(cTomoeWriting, "move_to", tw_move_to, 2);
    rb_define_method(cTomoeWriting, "line_to", tw_line_to, 2);
    rb_define_method(cTomoeWriting, "clear", tw_clear, 0);
    rb_define_method(cTomoeWriting, "number_of_strokes",
                     tw_get_number_of_strokes, 0);
    rb_define_method(cTomoeWriting, "get_number_of_points",
                     tw_get_number_of_points, 1);
    rb_define_method(cTomoeWriting, "[]", tw_get_point, 2);
    rb_define_method(cTomoeWriting, "last_point", tw_get_last_point, 0);
    rb_define_method(cTomoeWriting, "remove_last_stroke",
                     tw_remove_last_stroke, 0);

    rb_define_method(cTomoeWriting, "each", tw_each, 0);
}
