#include "tomoe-rb.h"

static VALUE
tr_s_load(VALUE self, VALUE base_dir)
{
    tomoe_recognizer_load(NIL_P(base_dir) ? NULL : RVAL2CSTR(base_dir));
    return Qnil;
}

static VALUE
tr_s_unload(VALUE self)
{
    tomoe_recognizer_unload();
    return Qnil;
}

static VALUE
tr_s_new(int argc, VALUE *argv, VALUE self)
{
    TomoeRecognizer *recognizer;
    VALUE name, dict;

    rb_scan_args(argc, argv, "11", &name, &dict);

    recognizer = tomoe_recognizer_new(RVAL2CSTR(name),
                                      "dictionary", RVAL2TDIC(dict),
                                      NULL);
    return GOBJ2RVALU(recognizer);
}

void
Init_tomoe_recognizer(VALUE mTomoe)
{
    VALUE cTomoeRecognizer;

    cTomoeRecognizer = G_DEF_CLASS(TOMOE_TYPE_RECOGNIZER, "Recognizer", mTomoe);

    rb_define_singleton_method(cTomoeRecognizer, "load", tr_s_load, 1);
    rb_define_singleton_method(cTomoeRecognizer, "unload", tr_s_unload, 0);

    rb_define_singleton_method(cTomoeRecognizer, "new", tr_s_new, -1);
}
