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
        xfce_rc_write_entry(rc, "icon_name", ol->icon_name ? ol->icon_name : "vexto-launcher");
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
            ol->icon_size = xfce_rc_read_int_entry(rc, "icon_size", 42);
            ol->icon_name = g_strdup(xfce_rc_read_entry(rc, "icon_name", "vexto-launcher"));
            xfce_rc_close(rc);
        }
    } else {
        ol->icon_size = 42;
        ol->icon_name = g_strdup("vexto-launcher");
    }
}

static void ol_set_image_from_name_or_path(GtkImage *image, const gchar *name_or_path, gint size) {
    gboolean found = FALSE;
    // Se não houver nome, usamos o nome padrão do ícone que o Makefile instala
    gchar *actual_name = (name_or_path && strlen(name_or_path) > 0) ? g_strdup(name_or_path) : g_strdup("vexto-launcher");

    /* 1. Tenta pelo tema de ícones (Papirus, etc) */
    if (!g_path_is_absolute(actual_name)) {
        GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
        if (gtk_icon_theme_has_icon(icon_theme, actual_name)) {
            gtk_image_set_from_icon_name(image, actual_name, GTK_ICON_SIZE_BUTTON);
            found = TRUE;
        }
    }

    /* 2. Se não encontrou no tema, tenta caminhos absolutos baseados no Makefile */
    if (!found) {
        const gchar *paths_to_check[] = {
            actual_name, // Caso o usuário tenha selecionado um arquivo manualmente
            "/usr/share/icons/hicolor/48x48/apps/vexto-launcher.svg", // Onde seu Makefile instala
            "/usr/share/icons/hicolor/scalable/apps/vexto-launcher.svg",
            "/usr/share/pixmaps/vexto-launcher.svg",
            NULL
        };

        for (int i = 0; paths_to_check[i] != NULL; i++) {
            if (g_file_test(paths_to_check[i], G_FILE_TEST_EXISTS)) {
                GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale(paths_to_check[i], size, size, TRUE, NULL);
                if (pixbuf) {
                    gtk_image_set_from_pixbuf(image, pixbuf);
                    g_object_unref(pixbuf);
                    found = TRUE;
                    break;
                }
            }
        }
    }

    if (!found) {
        gtk_image_set_from_icon_name(image, "xfce4-panel-menu", GTK_ICON_SIZE_BUTTON);
    }
    
    gtk_image_set_pixel_size(image, size);
    g_free(actual_name);
}

static void ol_update_icon(VextoLauncher *ol) {
    gint size = xfce_panel_plugin_get_size(ol->plugin);
    gint target_size = (ol->icon_size > 0) ? ol->icon_size : size - 4;
    
    ol_set_image_from_name_or_path(GTK_IMAGE(ol->icon), ol->icon_name, target_size);
}

static void ol_size_changed(XfcePanelPlugin *plugin, gint size, VextoLauncher *ol) {
    ol_update_icon(ol);
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
    ol_update_icon(ol);
}

static void ol_icon_chooser_clicked(GtkWidget *button, VextoLauncher *ol) {
    /* Standard GTK Icon Chooser as fallback or replacement if XFCE one fails */
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Selecionar Arquivo de Ícone",
                                                  GTK_WINDOW(gtk_widget_get_toplevel(button)),
                                                  GTK_FILE_CHOOSER_ACTION_OPEN,
                                                  "Cancelar", GTK_RESPONSE_CANCEL,
                                                  "Selecionar", GTK_RESPONSE_ACCEPT,
                                                  NULL);

    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_add_pixbuf_formats(filter);
    gtk_file_filter_set_name(filter, "Imagens de Ícone");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        g_free(ol->icon_name);
        ol->icon_name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        ol_update_icon(ol);
        
        /* Update the icon on the button in settings */
        GtkWidget *image = gtk_bin_get_child(GTK_BIN(button));
        if (GTK_IS_IMAGE(image)) {
            ol_set_image_from_name_or_path(GTK_IMAGE(image), ol->icon_name, 24);
        }
    }
    gtk_widget_destroy(dialog);
}

static void ol_configure(XfcePanelPlugin *plugin, VextoLauncher *ol) {
    GtkWidget *dialog = xfce_titled_dialog_new_with_mixed_buttons(
        "Vexto Launcher",
        GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(plugin))),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        "window-close-symbolic", "Fechar", GTK_RESPONSE_OK, NULL);

    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);
    gtk_window_set_icon_name(GTK_WINDOW(dialog), "preferences-system");
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 300);

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 18);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 12);
    gtk_box_pack_start(GTK_BOX(content), vbox, TRUE, TRUE, 0);

    /* Section: Aparência */
    GtkWidget *section_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(section_label), "<b>Aparência</b>");
    gtk_label_set_xalign(GTK_LABEL(section_label), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), section_label, FALSE, FALSE, 0);

    GtkWidget *inner_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_start(GTK_WIDGET(inner_vbox), 12);
    gtk_box_pack_start(GTK_BOX(vbox), inner_vbox, FALSE, FALSE, 0);

    /* Icon Configuration Row */
    GtkWidget *row_icon = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_box_pack_start(GTK_BOX(inner_vbox), row_icon, FALSE, FALSE, 0);
    
    GtkWidget *lbl_icon = gtk_label_new("Ícone do Painel:");
    gtk_label_set_xalign(GTK_LABEL(lbl_icon), 0.0);
    gtk_box_pack_start(GTK_BOX(row_icon), lbl_icon, TRUE, TRUE, 0);
    
    GtkWidget *icon_button = gtk_button_new();
    GtkWidget *btn_image = gtk_image_new();
    ol_set_image_from_name_or_path(GTK_IMAGE(btn_image), ol->icon_name, 24);
    gtk_container_add(GTK_CONTAINER(icon_button), btn_image);
    gtk_box_pack_end(GTK_BOX(row_icon), icon_button, FALSE, FALSE, 0);
    g_signal_connect(icon_button, "clicked", G_CALLBACK(ol_icon_chooser_clicked), ol);

    /* Icon Size Configuration Row */
    GtkWidget *row_size = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    gtk_box_pack_start(GTK_BOX(inner_vbox), row_size, FALSE, FALSE, 0);
    
    GtkWidget *lbl_size = gtk_label_new("Tamanho do Ícone:");
    gtk_label_set_xalign(GTK_LABEL(lbl_size), 0.0);
    gtk_box_pack_start(GTK_BOX(row_size), lbl_size, TRUE, TRUE, 0);
    
    GtkWidget *spin = gtk_spin_button_new_with_range(16, 128, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), ol->icon_size);
    gtk_box_pack_end(GTK_BOX(row_size), spin, FALSE, FALSE, 0);
    g_signal_connect(spin, "value-changed", G_CALLBACK(ol_icon_size_changed), ol);

    gtk_widget_show_all(vbox);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    
    ol_save(plugin, ol);
}

static void ol_free(XfcePanelPlugin *plugin, VextoLauncher *ol) {
    if (ol->window) gtk_widget_destroy(ol->window);
    if (ol->all_apps) g_list_free_full(ol->all_apps, g_object_unref);
    if (ol->filtered_apps) g_list_free(ol->filtered_apps);
    g_free(ol->icon_name);
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
    
    ol->icon = gtk_image_new();
    ol_update_icon(ol);
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
