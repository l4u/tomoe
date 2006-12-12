#include "tomoe-rb.h"

void
_tomoe_rb_init_tomoe_config(VALUE mTomoe)
{
    VALUE cTomoeConfig;

    cTomoeConfig = G_DEF_CLASS(TOMOE_TYPE_CONFIG, "Config", mTomoe);
}

