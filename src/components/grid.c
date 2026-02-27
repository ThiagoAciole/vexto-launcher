#include "../vexto-launcher.h"

static void on_app_clicked(GtkWidget *widget, GAppInfo *app) {
    GError *error = NULL;
    if (!g_app_info_launch(app, NULL, NULL, &error)) {
        g_warning("Could not launch app: %s", error->message);
        g_error_free(error);
    }
    /* Hide the grid after launch */
    VextoLauncher *ol = g_object_get_data(G_OBJECT(widget), "ol");
    if (ol) vexto_launcher_hide(ol);
}

static GtkWidget* create_app_button(VextoLauncher *ol, GAppInfo *app) {
    GtkWidget *btn = gtk_button_new();
    gtk_widget_set_name(btn, "app-button");
    gtk_style_context_add_class(gtk_widget_get_style_context(btn), "app-button");
    
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_container_add(GTK_CONTAINER(btn), vbox);
    
    GIcon *icon = g_app_info_get_icon(app);
    GtkWidget *img = gtk_image_new_from_gicon(icon, GTK_ICON_SIZE_DIALOG);
    gtk_box_pack_start(GTK_BOX(vbox), img, TRUE, TRUE, 0);
    
    GtkWidget *lbl = gtk_label_new(g_app_info_get_name(app));
    gtk_label_set_ellipsize(GTK_LABEL(lbl), PANGO_ELLIPSIZE_END);
    gtk_label_set_max_width_chars(GTK_LABEL(lbl), 12);
    gtk_label_set_lines(GTK_LABEL(lbl), 1);
    gtk_box_pack_start(GTK_BOX(vbox), lbl, FALSE, FALSE, 0);
    
    g_object_set_data(G_OBJECT(btn), "ol", ol);
    g_signal_connect(btn, "clicked", G_CALLBACK(on_app_clicked), app);
    
    return btn;
}

void vexto_launcher_grid_set_page(VextoLauncher *ol, gint page) {
    if (page < 0 || page >= ol->total_pages) return;
    ol->current_page = page;

    /* Clear existing grid */
    GList *children = gtk_container_get_children(GTK_CONTAINER(ol->grid));
    for (GList *l = children; l != NULL; l = l->next) {
        gtk_widget_destroy(GTK_WIDGET(l->data));
    }
    g_list_free(children);

    /* Populate the grid with apps for current page */
    guint start = page * APPS_PER_PAGE;
    guint count = 0;
    guint attached = 0;
    
    for (GList *l = ol->filtered_apps; l != NULL; l = l->next) {
        if (count < start) {
            count++;
            continue;
        }
        if (attached >= APPS_PER_PAGE) break;
        
        GtkWidget *btn = create_app_button(ol, l->data);
        gtk_grid_attach(GTK_GRID(ol->grid), btn, attached % GRID_COLS, attached / GRID_COLS, 1, 1);
        attached++;
        count++;
    }

    gtk_widget_show_all(ol->grid);
}

void vexto_launcher_grid_update(VextoLauncher *ol, const gchar *search_text) {
    /* Reset filter and page */
    if (ol->filtered_apps) g_list_free(ol->filtered_apps);
    ol->filtered_apps = NULL;

    if (!ol->all_apps) ol->all_apps = g_app_info_get_all();

    /* Apply filter */
    for (GList *l = ol->all_apps; l != NULL; l = l->next) {
        GAppInfo *app = l->data;
        if (search_text && *search_text) {
            if (!g_str_match_string(search_text, g_app_info_get_name(app), TRUE)) continue;
        }
        ol->filtered_apps = g_list_prepend(ol->filtered_apps, app);
    }
    ol->filtered_apps = g_list_reverse(ol->filtered_apps);

    /* Calculate pages */
    guint n_apps = g_list_length(ol->filtered_apps);
    ol->total_pages = (n_apps + APPS_PER_PAGE - 1) / APPS_PER_PAGE;
    if (ol->total_pages == 0) ol->total_pages = 1;

    vexto_launcher_grid_set_page(ol, 0);
}
