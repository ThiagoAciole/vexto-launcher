#include "../vexto-app-launcher.h"

static void ol_construct(XfcePanelPlugin *plugin);
XFCE_PANEL_PLUGIN_REGISTER(ol_construct);

static void ol_size_changed(XfcePanelPlugin *plugin, gint size, VextoAppLauncher *ol) {
    gtk_image_set_pixel_size(GTK_IMAGE(ol->icon), size - 4);
}

static gboolean ol_button_press(GtkWidget *widget, GdkEventButton *event, VextoAppLauncher *ol) {
    if (event->button == 1) {
        if (ol->window && gtk_widget_get_visible(ol->window)) {
            vexto_app_launcher_hide(ol);
        } else {
            vexto_app_launcher_show(ol);
        }
        return TRUE;
    }
    return FALSE;
}

static void ol_configure(XfcePanelPlugin *plugin, VextoAppLauncher *ol) {
    GtkWidget *dialog = xfce_titled_dialog_new_with_mixed_buttons(
        "Sobre o Vexto App Launcher",
        GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(plugin))),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "window-close-symbolic", "Fechar", GTK_RESPONSE_OK, NULL);

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *label = gtk_label_new("Vexto App Launcher - Menu de Aplicativos Minimalista.");
    gtk_container_set_border_width(GTK_CONTAINER(label), 20);
    gtk_box_pack_start(GTK_BOX(content), label, TRUE, TRUE, 0);

    gtk_widget_show_all(dialog);
    g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy), NULL);
}

static void ol_free(XfcePanelPlugin *plugin, VextoAppLauncher *ol) {
    if (ol->window) gtk_widget_destroy(ol->window);
    if (ol->all_apps) g_list_free_full(ol->all_apps, g_object_unref);
    if (ol->filtered_apps) g_list_free(ol->filtered_apps);
    g_slice_free(VextoAppLauncher, ol);
}

static void ol_construct(XfcePanelPlugin *plugin) {
    VextoAppLauncher *ol = g_slice_new0(VextoAppLauncher);
    ol->plugin = plugin;
    
    /* Initialize fields (g_slice_new0 already zeros, but being explicit) */
    ol->all_apps = NULL;
    ol->filtered_apps = NULL;
    ol->current_page = 0;
    ol->total_pages = 0;
    ol->window = NULL;
    
    /* Global Style Initialization */
    vexto_app_launcher_style_init();

    /* Panel Button */
    ol->button = xfce_panel_create_button();
    gtk_widget_set_name(ol->button, "vexto-app-launcher-button");
    gtk_button_set_relief(GTK_BUTTON(ol->button), GTK_RELIEF_NONE);
    
    ol->icon = gtk_image_new_from_icon_name("vexto-app-launcher", GTK_ICON_SIZE_BUTTON);
    gtk_container_add(GTK_CONTAINER(ol->button), ol->icon);
    
    gtk_container_add(GTK_CONTAINER(plugin), ol->button);
    xfce_panel_plugin_add_action_widget(plugin, ol->button);
    xfce_panel_plugin_menu_show_configure(plugin);
    
    g_signal_connect(G_OBJECT(ol->button), "button-press-event", G_CALLBACK(ol_button_press), ol);
    g_signal_connect(G_OBJECT(plugin), "free-data", G_CALLBACK(ol_free), ol);
    g_signal_connect(G_OBJECT(plugin), "size-changed", G_CALLBACK(ol_size_changed), ol);
    g_signal_connect(G_OBJECT(plugin), "configure-plugin", G_CALLBACK(ol_configure), ol);

    gtk_widget_show_all(ol->button);
}
