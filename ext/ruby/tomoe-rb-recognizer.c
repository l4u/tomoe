#include "tomoe-rb.h"

#define TOMOE_RECOGNIZER_PREFIX "TomoeRecognizer"
#define TOMOE_RECOGNIZER_PREFIX_LEN (strlen(TOMOE_RECOGNIZER_PREFIX))

static VALUE cTomoeRecognizer;

static VALUE
tr_s_load(VALUE self, VALUE base_dir)
{
    tomoe_recognizer_load(NIL_P(base_dir) ? NULL : RVAL2CSTR(base_dir));
    _tomoe_rb_module_load(tomoe_recognizer_get_registered_types(),
                          cTomoeRecognizer,
                          TOMOE_RECOGNIZER_PREFIX,
                          TOMOE_RECOGNIZER_PREFIX_LEN);
    return Qnil;
}

static VALUE
tr_s_unload(VALUE self)
{
    _tomoe_rb_module_unload(tomoe_recognizer_get_registered_types(),
                            cTomoeRecognizer,
                            TOMOE_RECOGNIZER_PREFIX,
                            TOMOE_RECOGNIZER_PREFIX_LEN);
    tomoe_recognizer_unload();
    return Qnil;
}

void
Init_tomoe_recognizer(VALUE mTomoe)
{
    cTomoeRecognizer = G_DEF_CLASS(TOMOE_TYPE_RECOGNIZER, "Recognizer", mTomoe);

    rb_define_singleton_method(cTomoeRecognizer, "load", tr_s_load, 1);
    rb_define_singleton_method(cTomoeRecognizer, "unload", tr_s_unload, 0);
}
