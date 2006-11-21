/* -*- c-file-style: "ruby"; indent-tabs-mode: nil -*- */

#include "tomoe-rb.h"

#define _SELF(obj) (TOMOE_CONTEXT(RVAL2GOBJ(obj)))
#define RVAL2TGLYPH(obj) (tc_glyph_from_rval(obj))

static TomoeGlyph *
tc_glyph_from_rval(VALUE rb_glyph)
{
    int i, glyph_len;
    TomoeGlyph *glyph;

    glyph_len = RARRAY(rb_glyph)->len;

    glyph = tomoe_glyph_new();
    tomoe_glyph_init(glyph, glyph_len);

    for (i = 0; i < glyph_len; i++) {
        int j, stroke_len;
        VALUE rb_stroke;
        TomoeStroke *stroke;

        rb_stroke = RARRAY(rb_glyph)->ptr[i];
        stroke_len = RARRAY(rb_stroke)->len;
        stroke = &(glyph->strokes[i]);
        tomoe_stroke_init(stroke, stroke_len);

        for (j = 0; j < stroke_len; j++) {
            VALUE rb_point;
            rb_point = RARRAY(rb_stroke)->ptr[j];
            stroke->points[j].x = NUM2INT(RARRAY(rb_point)->ptr[0]);
            stroke->points[j].y = NUM2INT(RARRAY(rb_point)->ptr[1]);
        }

    }

    return glyph;
}

static VALUE
tc_add_dict(VALUE self, VALUE dict)
{
    tomoe_context_add_dict(_SELF(self), RVAL2TDIC(dict));
    return Qnil;
}

static VALUE
tc_load_config(int argc, VALUE *argv, VALUE self)
{
    VALUE filename;

    rb_scan_args(argc, argv, "01", &filename);

    tomoe_context_load_config(_SELF(self),
                              NIL_P(filename) ? NULL : RVAL2CSTR(filename));
    return Qnil;
}

static VALUE
tc_search_by_strokes(VALUE self, VALUE input)
{
    GList *result;
    TomoeGlyph *glyph;

    glyph = RVAL2TGLYPH(input);
    result = tomoe_context_search_by_strokes(_SELF(self), glyph);
    tomoe_glyph_free(glyph);
    return GLIST2ARYF(result);
}


void
Init_tomoe_context(VALUE mTomoe)
{
    VALUE cTomoeContext;

    cTomoeContext = G_DEF_CLASS(TOMOE_TYPE_CONTEXT, "Context", mTomoe);

    rb_define_method(cTomoeContext, "add_dict", tc_add_dict, 1);
    rb_define_method(cTomoeContext, "load_config", tc_load_config, -1);
    rb_define_method(cTomoeContext, "search_by_strokes", tc_search_by_strokes,
                     1);
}
