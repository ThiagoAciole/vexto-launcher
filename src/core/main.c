#include "../vexto-launcher.h"

static void ol_construct(XfcePanelPlugin *plugin);
XFCE_PANEL_PLUGIN_REGISTER(ol_construct);

static void ol_save(XfcePanelPlugin *plugin, VextoLauncher *ol) {
    XfceRc *rc;
    gchar  *file;

    file = xfce_panel_plugin_save_location(plugin, TRUE);
    if (G_UNLIKELY(file == NULL)) return;

    rc = xfce_rc_simple_open(file, FALSE);
    g_free(file);

    if (G_LIKELY(rc != NULL)) {
        xfce_rc_write_int_entry(rc, "icon_size", ol->icon_size);
        xfce_rc_close(rc);
    }
}

static void ol_load(VextoLauncher *ol) {
    XfceRc *rc;
    gchar  *file;

    file = xfce_panel_plugin_lookup_rc_file(ol->plugin);
    if (G_LIKELY(file != NULL)) {
        rc = xfce_rc_simple_open(file, TRUE);
        g_free(file);

        if (G_LIKELY(rc != NULL)) {
            ol->icon_size = xfce_rc_read_int_entry(rc, "icon_size", 0);
            xfce_rc_close(rc);
        }
    } else {
        ol->icon_size = 0;
    }
}

static void ol_size_changed(XfcePanelPlugin *plugin, gint size, VextoLauncher *ol) {
    gint target_size;
    if (ol->icon_size > 0) {
        target_size = ol->icon_size;
    } else {
        target_size = size - 4;
    }
    gtk_image_set_pixel_size(GTK_IMAGE(ol->icon), target_size);
}

static gboolean ol_button_press(GtkWidget *widget, GdkEventButton *event, VextoLauncher *ol) {
    if (event->button == 1) {
        if (ol->window && gtk_widget_get_visible(ol->window)) {
            vexto_launcher_hide(ol);
        } else {
            vexto_launcher_show(ol);
        }
        return TRUE;
    }
    return FALSE;
}

static void ol_icon_size_changed(GtkSpinButton *spin, VextoLauncher *ol) {
    ol->icon_size = gtk_spin_button_get_value_as_int(spin);
    ol_size_changed(ol->plugin, xfce_panel_plugin_get_size(ol->plugin), ol);
}

static void ol_configure(XfcePanelPlugin *plugin, VextoLauncher *ol) {
    GtkWidget *dialog = xfce_titled_dialog_new_with_mixed_buttons(
        "Configurações do Vexto Launcher",
        GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(plugin))),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "window-close-symbolic", "Fechar", GTK_RESPONSE_OK, NULL);

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 15);
    gtk_box_pack_start(GTK_BOX(content), vbox, TRUE, TRUE, 0);

    /* Icon Size Setting */
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    GtkWidget *label = gtk_label_new("Tamanho do Ícone (0 = Auto):");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);

    GtkWidget *spin = gtk_spin_button_new_with_range(0, 128, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), ol->icon_size);
    gtk_box_pack_start(GTK_BOX(hbox), spin, TRUE, TRUE, 0);
    g_signal_connect(spin, "value-changed", G_CALLBACK(ol_icon_size_changed), ol);

    gtk_widget_show_all(dialog);
    xfce_titled_dialog_set_subtitle(XFCE_TITLED_DIALOG(dialog), "Personalize seu menu");
    
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    
    ol_save(plugin, ol);
}

static void ol_free(XfcePanelPlugin *plugin, VextoLauncher *ol) {
    if (ol->window) gtk_widget_destroy(ol->window);
    if (ol->all_apps) g_list_free_full(ol->all_apps, g_object_unref);
    if (ol->filtered_apps) g_list_free(ol->filtered_apps);
    g_slice_free(VextoLauncher, ol);
}

static void ol_construct(XfcePanelPlugin *plugin) {
    VextoLauncher *ol = g_slice_new0(VextoLauncher);
    ol->plugin = plugin;
    
    /* Initialize fields (g_slice_new0 already zeros, but being explicit) */
    ol->all_apps = NULL;
    ol->filtered_apps = NULL;
    ol->current_page = 0;
    ol->total_pages = 0;
    ol->window = NULL;
    
    /* Load saved settings */
    ol_load(ol);

    /* Global Style Initialization */
    vexto_launcher_style_init();

    /* Panel Button */
    ol->button = xfce_panel_create_button();
    gtk_widget_set_name(ol->button, "vexto-launcher-button");
    gtk_button_set_relief(GTK_BUTTON(ol->button), GTK_RELIEF_NONE);
    
    ol->icon = gtk_image_new_from_icon_name("vexto-launcher", GTK_ICON_SIZE_BUTTON);
    gtk_container_add(GTK_CONTAINER(ol->button), ol->icon);
    
    gtk_container_add(GTK_CONTAINER(plugin), ol->button);
    xfce_panel_plugin_add_action_widget(plugin, ol->button);
    xfce_panel_plugin_menu_show_configure(plugin);
    
    g_signal_connect(G_OBJECT(ol->button), "button-press-event", G_CALLBACK(ol_button_press), ol);
    g_signal_connect(G_OBJECT(plugin), "free-data", G_CALLBACK(ol_free), ol);
    g_signal_connect(G_OBJECT(plugin), "size-changed", G_CALLBACK(ol_size_changed), ol);
    g_signal_connect(G_OBJECT(plugin), "save", G_CALLBACK(ol_save), ol);
    g_signal_connect(G_OBJECT(plugin), "configure-plugin", G_CALLBACK(ol_configure), ol);

    gtk_widget_show_all(ol->button);
}
