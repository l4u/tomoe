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

    tomoe_init();
    atexit(tomoe_quit);

    mTomoe = rb_define_module("Tomoe");

    Init_tomoe_candidate(mTomoe);
    Init_tomoe_char(mTomoe);
    Init_tomoe_config(mTomoe);
    Init_tomoe_context(mTomoe);
    Init_tomoe_dict(mTomoe);
    Init_tomoe_query(mTomoe);
    Init_tomoe_reading(mTomoe);
    Init_tomoe_recognizer(mTomoe);
    Init_tomoe_writing(mTomoe);
}
