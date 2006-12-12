#include "tomoe-rb.h"

void
_tomoe_rb_module_load (GList *registered_types, VALUE klass,
                       const gchar *prefix, gint prefix_len)
{
    GList *node;

    for (node = registered_types; node; node = g_list_next (node)) {
        const gchar *name = node->data;
        GType type;

        type = g_type_from_name (name);
        if (type && g_str_has_prefix (name, prefix)) {
            G_DEF_CLASS(type, name + prefix_len, klass);
        }
    }
    g_list_free (registered_types);
}

void
_tomoe_rb_module_unload (GList *registered_types, VALUE klass,
                         const gchar *prefix, gint prefix_len)
{
    GList *node;

    for (node = registered_types; node; node = g_list_next (node)) {
        const gchar *name = node->data;

        if (g_str_has_prefix(name, prefix)) {
            const gchar *klass_name = name + prefix_len;
            if (rb_const_defined (klass, rb_intern(klass_name))) {
                rb_mod_remove_const (klass, rb_str_new2(klass_name));
            }
        }
    }
    g_list_free (registered_types);
}
