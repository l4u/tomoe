#include "tomoe-rb.h"

#define _SELF(obj) (TOMOE_CANDIDATE(RVAL2GOBJ(obj)))

static VALUE
tc_get_char(VALUE self)
{
    return GOBJ2RVAL(tomoe_candidate_get_char(_SELF(self)));
}

void
Init_tomoe_candidate(VALUE mTomoe)
{
    VALUE cTomoeCandidate;

    cTomoeCandidate = G_DEF_CLASS(TOMOE_TYPE_CANDIDATE, "Candidate", mTomoe);

    rb_define_method(cTomoeCandidate, "char", tc_get_char, 0);
}

