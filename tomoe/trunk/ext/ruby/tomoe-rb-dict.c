#include "tomoe-rb.h"

void
Init_tomoe_dict(VALUE mTomoe)
{
    VALUE cTomoeDict;

    cTomoeDict = G_DEF_CLASS(TOMOE_TYPE_DICT, "Dict", mTomoe);
}

