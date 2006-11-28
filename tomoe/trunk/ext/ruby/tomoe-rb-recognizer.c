#include "tomoe-rb.h"

static VALUE
trl_s_load(VALUE self, VALUE base_dir)
{
    tomoe_recognizer_loader_load(NIL_P(base_dir) ? NULL : RVAL2CSTR(base_dir));
    return Qnil;
}

static VALUE
trl_s_unload(VALUE self)
{
    tomoe_recognizer_loader_unload();
    return Qnil;
}

static VALUE
trl_s_instantiate(VALUE self, VALUE name)
{
    return GOBJ2RVAL(tomoe_recognizer_loader_instantiate(RVAL2CSTR(name)));
}

void
Init_tomoe_recognizer(VALUE mTomoe)
{
    VALUE cTomoeRecognizer, mTomoeRecognizerLoader;

    cTomoeRecognizer = G_DEF_CLASS(TOMOE_TYPE_RECOGNIZER, "Recognizer", mTomoe);
    mTomoeRecognizerLoader = rb_define_module_under(mTomoe, "RecognizerLoader");

    rb_define_module_function(mTomoeRecognizerLoader, "load", trl_s_load, 1);
    rb_define_module_function(mTomoeRecognizerLoader, "unload",
                              trl_s_unload, 0);
    rb_define_module_function(mTomoeRecognizerLoader, "instantiate",
                              trl_s_instantiate, 3);
}
