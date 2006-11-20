#include "tomoe-rb.h"

void
Init_tomoe_recognizer(VALUE mTomoe)
{
    VALUE cTomoeRecognizer;

    cTomoeRecognizer = G_DEF_CLASS(TOMOE_TYPE_RECOGNIZER, "Recognizer", mTomoe);
}
