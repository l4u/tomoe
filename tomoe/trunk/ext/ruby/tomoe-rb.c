#include <stdlib.h>

#include "tomoe-rb.h"

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
    Init_tomoe_handwrite(mTomoe);
    Init_tomoe_recognizer(mTomoe);
}
