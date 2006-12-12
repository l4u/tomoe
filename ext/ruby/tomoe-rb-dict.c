#include "tomoe-rb.h"

#define _SELF(obj) RVAL2TDIC(obj)

#define TOMOE_DICT_PREFIX "TomoeDict"
#define TOMOE_DICT_PREFIX_LEN (strlen(TOMOE_DICT_PREFIX))

static VALUE cTomoeDict;

static VALUE
td_s_load(VALUE self, VALUE base_dir)
{
    tomoe_dict_load(NIL_P(base_dir) ? NULL : RVAL2CSTR(base_dir));
    _tomoe_rb_module_load(tomoe_dict_get_registered_types(), cTomoeDict,
                          TOMOE_DICT_PREFIX, TOMOE_DICT_PREFIX_LEN);
    return Qnil;
}

static VALUE
td_s_unload(VALUE self)
{
    _tomoe_rb_module_unload(tomoe_dict_get_registered_types(), cTomoeDict,
                            TOMOE_DICT_PREFIX, TOMOE_DICT_PREFIX_LEN);
    tomoe_dict_unload();
    return Qnil;
}

static VALUE
td_get_char(VALUE self, VALUE utf8)
{
    return GOBJ2RVALU(tomoe_dict_get_char(_SELF(self), RVAL2CSTR(utf8)));
}

static VALUE
td_register_char(VALUE self, VALUE chr)
{
    return CBOOL2RVAL(tomoe_dict_register_char(_SELF(self), RVAL2TCHR(chr)));
}

static VALUE
td_unregister_char(VALUE self, VALUE utf8)
{
    return CBOOL2RVAL(tomoe_dict_unregister_char(_SELF(self), RVAL2CSTR(utf8)));
}

static VALUE
td_search(VALUE self, VALUE query)
{
    return GLIST2ARYF(tomoe_dict_search(_SELF(self), RVAL2TQRY(query)));
}

static VALUE
td_flush(VALUE self)
{
    return CBOOL2RVAL(tomoe_dict_flush(_SELF(self)));
}

void
Init_tomoe_dict(VALUE mTomoe)
{
    cTomoeDict = G_DEF_CLASS(TOMOE_TYPE_DICT, "Dict", mTomoe);

    rb_define_singleton_method(cTomoeDict, "load", td_s_load, 1);
    rb_define_singleton_method(cTomoeDict, "unload", td_s_unload, 0);

    rb_define_method(cTomoeDict, "[]", td_get_char, 1);
    rb_define_method(cTomoeDict, "register", td_register_char, 1);
    rb_define_method(cTomoeDict, "unregister", td_unregister_char, 1);

    rb_define_method(cTomoeDict, "search", td_search, 1);

    rb_define_method(cTomoeDict, "flush", td_flush, 0);
}
