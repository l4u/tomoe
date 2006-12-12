#include "tomoe-rb.h"

#define TOMOE_PREFIX "Tomoe"

void
_tomoe_rb_module_load (GList *registered_types, VALUE klass,
                       const gchar *prefix)
{
    GList *node;
    gchar *full_prefix;
    gint tomoe_prefix_len;

    full_prefix = g_strconcat(TOMOE_PREFIX, prefix, NULL);
    tomoe_prefix_len = strlen(TOMOE_PREFIX);
    for (node = registered_types; node; node = g_list_next (node)) {
        const gchar *name = node->data;
        GType type;

        type = g_type_from_name (name);
        if (type && g_str_has_prefix (name, full_prefix)) {
            G_DEF_CLASS(type, name + tomoe_prefix_len, klass);
        }
    }
    g_free(full_prefix);
    g_list_free (registered_types);
}

void
_tomoe_rb_module_unload (GList *registered_types, VALUE klass,
                         const gchar *prefix)
{
    GList *node;
    gchar *full_prefix;
    gint tomoe_prefix_len;

    full_prefix = g_strconcat(TOMOE_PREFIX, prefix, NULL);
    tomoe_prefix_len = strlen(TOMOE_PREFIX);
    for (node = registered_types; node; node = g_list_next (node)) {
        const gchar *name = node->data;

        if (g_str_has_prefix(name, full_prefix)) {
            const gchar *klass_name = name + tomoe_prefix_len;
            if (rb_const_defined (klass, rb_intern(klass_name))) {
                rb_mod_remove_const (klass, rb_str_new2(klass_name));
            }
        }
    }
    g_free(full_prefix);
    g_list_free (registered_types);
}
