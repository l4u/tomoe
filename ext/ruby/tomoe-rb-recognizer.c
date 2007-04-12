#include "tomoe-rb.h"

#define _SELF(obj) RVAL2TREC(obj)

#define RECOGNIZER_PREFIX "Recognizer"

static VALUE mTomoe;

void
_tomoe_rb_recognizer_module_load(void)
{
    _tomoe_rb_module_load(tomoe_recognizer_get_registered_types(), mTomoe,
                          tomoe_recognizer_get_log_domains(),
                          RECOGNIZER_PREFIX);
}

static void
_tomoe_rb_recognizer_module_unload(void)
{
    _tomoe_rb_module_unload(tomoe_recognizer_get_registered_types(), mTomoe,
                            RECOGNIZER_PREFIX);
}

static VALUE
tr_s_load(VALUE self, VALUE base_dir)
{
    tomoe_recognizer_load(NIL_P(base_dir) ? NULL : RVAL2CSTR(base_dir));
    _tomoe_rb_recognizer_module_load();
    return Qnil;
}

static VALUE
tr_s_unload(VALUE self)
{
    _tomoe_rb_recognizer_module_unload();
    tomoe_recognizer_unload();
    return Qnil;
}

static VALUE
tr_language(VALUE self)
{
    const gchar *language;

    language = tomoe_recognizer_get_language(_SELF(self));
    if (language)
        return rb_str_new2(language);
    else
        return Qnil;
}

void
_tomoe_rb_init_tomoe_recognizer(VALUE _mTomoe)
{
    VALUE cTomoeRecognizer;

    mTomoe = _mTomoe;
    cTomoeRecognizer = G_DEF_CLASS(TOMOE_TYPE_RECOGNIZER, "Recognizer", mTomoe);

    rb_define_singleton_method(cTomoeRecognizer, "load", tr_s_load, 1);
    rb_define_singleton_method(cTomoeRecognizer, "unload", tr_s_unload, 0);

    rb_define_method(cTomoeRecognizer, "language", tr_language, 0);
}
