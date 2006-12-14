#include <stdlib.h>

#include "tomoe-rb.h"

#ifndef HAVE_GOBJ2RVALU
VALUE
_tomoe_ruby_object_from_instance_with_unref(gpointer instance)
{
    VALUE result = rbgobj_ruby_object_from_instance(instance);
    if (!NIL_P(result))
        g_object_unref(instance);
    return result;
}
#endif

void
Init_tomoe(void)
{
    VALUE mTomoe;

    mTomoe = rb_define_module("Tomoe");

    _tomoe_rb_init_tomoe_candidate(mTomoe);
    _tomoe_rb_init_tomoe_char(mTomoe);
    _tomoe_rb_init_tomoe_config(mTomoe);
    _tomoe_rb_init_tomoe_context(mTomoe);
    _tomoe_rb_init_tomoe_dict(mTomoe);
    _tomoe_rb_init_tomoe_query(mTomoe);
    _tomoe_rb_init_tomoe_reading(mTomoe);
    _tomoe_rb_init_tomoe_recognizer(mTomoe);
    _tomoe_rb_init_tomoe_writing(mTomoe);


    tomoe_init();
    atexit(tomoe_quit);

    _tomoe_rb_dict_module_load();
    _tomoe_rb_recognizer_module_load();
}
