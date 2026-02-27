#include "one-launcher.h"

static void ol_construct(XfcePanelPlugin *plugin);

/* Register the plugin */
XFCE_PANEL_PLUGIN_REGISTER(ol_construct);

static void ol_size_changed(XfcePanelPlugin *plugin, gint size, OneLauncher *ol) {
    /* Set icon size - very little padding for a larger appearance */
    gtk_image_set_pixel_size(GTK_IMAGE(ol->icon), size - 4);
}

static void ol_orientation_changed(XfcePanelPlugin *plugin, GtkOrientation orientation, OneLauncher *ol) {
    /* Handle orientation if needed */
}

static gboolean ol_button_press(GtkWidget *widget, GdkEventButton *event, OneLauncher *ol) {
    if (event->button == 1) {
        if (ol->window && gtk_widget_get_visible(ol->window)) {
            vexto_app_launcher_hide_grid(ol);
        } else {
            vexto_app_launcher_show_grid(ol);
        }
        return TRUE;
    }
    return FALSE;
}

static void ol_configure(XfcePanelPlugin *plugin, OneLauncher *ol) {
    GtkWidget *dialog = xfce_titled_dialog_new_with_mixed_buttons(
        "Sobre o oneLauncher",
        GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(plugin))),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "window-close-symbolic", "Fechar", GTK_RESPONSE_OK, NULL);

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *label = gtk_label_new("oneLauncher - Menu de Aplicativos Minimalista.");
    gtk_container_set_border_width(GTK_CONTAINER(label), 20);
    gtk_box_pack_start(GTK_BOX(content), label, TRUE, TRUE, 0);

    gtk_widget_show_all(dialog);
    g_signal_connect(dialog, "response", G_CALLBACK(gtk_widget_destroy), NULL);
}

static void ol_free(XfcePanelPlugin *plugin, OneLauncher *ol) {
    if (ol->window) gtk_widget_destroy(ol->window);
    if (ol->all_apps) g_list_free_full(ol->all_apps, g_object_unref);
    g_slice_free(OneLauncher, ol);
}

static void ol_construct(XfcePanelPlugin *plugin) {
    OneLauncher *ol = g_slice_new0(OneLauncher);
    ol->plugin = plugin;
    
    ol->all_apps = NULL;
    ol->current_page = 0;
    ol->total_pages = 0;

    /* Create the panel button */
    ol->button = xfce_panel_create_button();
    gtk_widget_set_name(ol->button, "one-launcher-button");
    gtk_button_set_relief(GTK_BUTTON(ol->button), GTK_RELIEF_NONE);
    
    /* Use the standard app grid icon */
    ol->icon = gtk_image_new_from_icon_name("view-app-grid-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_container_add(GTK_CONTAINER(ol->button), ol->icon);
    
    gtk_container_add(GTK_CONTAINER(plugin), ol->button);
    xfce_panel_plugin_add_action_widget(plugin, ol->button);
    
    /* Show configure button */
    xfce_panel_plugin_menu_show_configure(plugin);
    
    g_signal_connect(G_OBJECT(ol->button), "button-press-event", G_CALLBACK(ol_button_press), ol);
    g_signal_connect(G_OBJECT(plugin), "free-data", G_CALLBACK(ol_free), ol);
    g_signal_connect(G_OBJECT(plugin), "size-changed", G_CALLBACK(ol_size_changed), ol);
    g_signal_connect(G_OBJECT(plugin), "orientation-changed", G_CALLBACK(ol_orientation_changed), ol);
    g_signal_connect(G_OBJECT(plugin), "configure-plugin", G_CALLBACK(ol_configure), ol);

    /* CSS for panel button */
    GtkCssProvider *css = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css, 
        "#one-launcher-button {\n"
        "  background: transparent;\n"
        "  border: none;\n"
        "  padding: 0;\n"
        "  margin: 0;\n"
    "  transition: all 0.2s ease;\n"
    "}\n"
    "#one-launcher-button:hover {\n"
    "  background-color: rgba(255, 255, 255, 0.1);\n"
    "}\n"
    "#one-launcher-button.active {\n"
    "  background-color: #3584e4 !important;\n"
    "  color: white;\n"
    "  box-shadow: inset 0 0 5px rgba(0,0,0,0.2);\n"
    "}\n", -1, NULL);
    
    GtkStyleContext *context = gtk_widget_get_style_context(ol->button);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(css), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(css);

    gtk_widget_show_all(ol->button);
}
