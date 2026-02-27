#include "../vexto-launcher.h"

void vexto_launcher_style_init(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
    
    /* Try to load from the data directory */
    const gchar *style_path = "/usr/share/vexto-launcher/styles/main.css";
    GFile *file = g_file_new_for_path(style_path);
    
    if (g_file_query_exists(file, NULL)) {
        gtk_css_provider_load_from_file(provider, file, NULL);
    } else {
        /* Fallback for local development */
        gtk_css_provider_load_from_path(provider, "data/styles/main.css", NULL);
    }
    
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
    
    g_object_unref(file);
    g_object_unref(provider);
}

void vexto_launcher_style_apply(GtkWidget *widget) {
    if (!widget) return;
    /* This can be used for widget-specific providers if needed */
}
