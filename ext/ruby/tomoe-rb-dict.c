#include "tomoe-rb.h"

#define _SELF(obj) RVAL2TDIC(obj)

static VALUE
td_get_char(VALUE self, VALUE code_point)
{
    return GOBJ2RVAL(tomoe_dict_get_char(_SELF(self), RVAL2CSTR(code_point)));
}

static VALUE
tdl_s_load(VALUE self, VALUE base_dir)
{
    tomoe_dict_loader_load(NIL_P(base_dir) ? NULL : RVAL2CSTR(base_dir));
    return Qnil;
}

static VALUE
tdl_s_unload(VALUE self)
{
    tomoe_dict_loader_unload();
    return Qnil;
}

static VALUE
tdl_s_instantiate(VALUE self, VALUE name, VALUE filename, VALUE editable)
{
    return GOBJ2RVAL(tomoe_dict_loader_instantiate(RVAL2CSTR(name),
                                                   RVAL2CSTR(filename),
                                                   RVAL2CBOOL(editable)));
}

void
Init_tomoe_dict(VALUE mTomoe)
{
    VALUE cTomoeDict, mTomoeDictLoader;

    cTomoeDict = G_DEF_CLASS(TOMOE_TYPE_DICT, "Dict", mTomoe);
    mTomoeDictLoader = rb_define_module_under(mTomoe, "DictLoader");

    rb_define_method(cTomoeDict, "[]", td_get_char, 1);

    rb_define_module_function(mTomoeDictLoader, "load", tdl_s_load, 1);
    rb_define_module_function(mTomoeDictLoader, "unload", tdl_s_unload, 0);
    rb_define_module_function(mTomoeDictLoader, "instantiate",
                              tdl_s_instantiate, 3);
}
