#include <gio/gdesktopappinfo.h>
#include "vexto-launcher.h"

#define APPS_PER_PAGE 20
#define GRID_COLS 5
#define ICON_SIZE 40

const gchar* vexto_launcher_get_css(void) {
    return
    "window.launcher-window {\n"
    "  background-color: #1a1a1a;\n"
    "  border-radius: 12px;\n"
    "  border: 1px solid rgba(255, 255, 255, 0.03);\n"
    "  padding: 15px;\n"
    "}\n"
    ".search-container {\n"
    "  background-color: #262626;\n"
    "  border-radius: 10px;\n"
    "  border: 1px solid rgba(255, 255, 255, 0.05);\n"
    "  margin: 10px 5px 10px 10px;\n"
    "  padding: 5px 10px;\n"
    "}\n"
    "entry {\n"
    "  background-color: transparent;\n"
    "  background-image: none;\n"
    "  border: none;\n"
    "  font-size: 14px;\n"
    "  color: #a0a0a0;\n"
    "  caret-color: white;\n"
    "  min-height: 32px;\n"
    "  box-shadow: none;\n"
    "}\n"
    "entry:focus {\n"
    "  color: white;\n"
    "}\n"
    "button.app-item {\n"
    "  background-color: transparent;\n"
    "  background-image: none;\n"
    "  border: none;\n"
    "  padding: 10px 5px;\n"
    "  margin: 0;\n"
    "  border-radius: 10px;\n"
    "  transition: all 0.2s ease;\n"
    "}\n"
    "button.app-item:hover {\n"
    "  background-color: rgba(255, 255, 255, 0.05);\n"
    "}\n"
    "button.app-item label {\n"
    "  color: #e0e0e0;\n"
    "  font-size: 11px;\n"
    "  margin-top: 6px;\n"
    "}\n"
    ".page-dots-container {\n"
    "  margin: 10px auto 15px auto;\n"
    "}\n"
    "button.page-dot {\n"
    "  background-color: rgba(255, 255, 255, 0.2);\n"
    "  border-radius: 50%;\n"
    "  min-width: 6px;\n"
    "  min-height: 6px;\n"
    "  margin: 0 4px;\n"
    "  padding: 0;\n"
    "  border: none;\n"
    "}\n"
    "button.page-dot.active {\n"
    "  background-color: white;\n"
    "  min-width: 8px;\n"
    "  min-height: 8px;\n"
    "}\n"
    "button.nav-arrow {\n"
    "  background-color: transparent;\n"
    "  border: none;\n"
    "  color: rgba(255, 255, 255, 0.4);\n"
    "  padding: 5px;\n"
    "  margin: 5px;\n"
    "  min-width: 20px;\n"
    "  min-height: 20px;\n"
    "}\n"
    "button.nav-arrow:hover { color: white; }\n"
    "button.action-button {\n"
    "  background-color: #262626;\n"
    "  border-radius: 8px;\n"
    "  border: 1px solid rgba(255, 255, 255, 0.05);\n"
    "  color: white;\n"
    "  min-width: 36px;\n"
    "  min-height: 36px;\n"
    "  margin: 10px;\n"
    "  padding: 0;\n"
    "}\n"
    "button.action-button:hover {\n"
    "  background-color: #333333;\n"
    "}\n"
    "button.action-button image {\n"
    "  opacity: 0.9;\n"
    "}\n";
}

static void on_power_clicked(GtkWidget *widget, VextoLauncher *ol) {
    vexto_launcher_hide_grid(ol);
    g_spawn_command_line_async("xfce4-session-logout", NULL);
}

static void on_settings_clicked(GtkWidget *widget, VextoLauncher *ol) {
    vexto_launcher_hide_grid(ol);
    g_spawn_command_line_async("xfce4-settings-manager", NULL);
}

void vexto_launcher_hide_grid(VextoLauncher *ol) {
    if (ol->window) gtk_widget_hide(ol->window);
    if (ol->button) gtk_style_context_remove_class(gtk_widget_get_style_context(ol->button), "active");
}

static void on_app_clicked(GtkWidget *button, GAppInfo *app) {
    g_app_info_launch(app, NULL, NULL, NULL);
}

static void on_search_changed(GtkSearchEntry *entry, VextoLauncher *ol) {
    ol->current_page = 0;
    vexto_launcher_populate_grid(ol);
}

static void on_dot_clicked(GtkWidget *dot, VextoLauncher *ol) {
    gint page_index = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(dot), "page-index"));
    if (page_index != ol->current_page) {
        ol->current_page = page_index;
        vexto_launcher_populate_grid(ol);
    }
}

static void on_nav_arrow_clicked(GtkWidget *button, VextoLauncher *ol) {
    gboolean is_next = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(button), "is-next"));
    if (is_next) {
        if (ol->current_page < ol->total_pages - 1) ol->current_page++;
    } else {
        if (ol->current_page > 0) ol->current_page--;
    }
    vexto_launcher_populate_grid(ol);
}

void vexto_launcher_populate_grid(VextoLauncher *ol) {
    const gchar *search_text = gtk_entry_get_text(GTK_ENTRY(ol->search_entry));
    
    /* Clear Grid */
    gtk_container_foreach(GTK_CONTAINER(ol->grid), (GtkCallback)gtk_widget_destroy, NULL);
    
    GList *filtered = NULL;
    for (GList *l = ol->all_apps; l != NULL; l = l->next) {
        GAppInfo *app = l->data;
        const gchar *name = g_app_info_get_name(app);
        
        if (search_text && *search_text) {
            if (!g_str_match_string(search_text, name, TRUE)) continue;
        }
        filtered = g_list_append(filtered, app);
    }

    gint count = g_list_length(filtered);
    ol->total_pages = (count + APPS_PER_PAGE - 1) / APPS_PER_PAGE;
    if (ol->current_page >= ol->total_pages) ol->current_page = 0;

    gint start = ol->current_page * APPS_PER_PAGE;
    gint displayed = 0;
    
    for (GList *l = g_list_nth(filtered, start); l != NULL && displayed < APPS_PER_PAGE; l = l->next) {
        GAppInfo *app = l->data;
        GtkWidget *btn = gtk_button_new();
        gtk_style_context_add_class(gtk_widget_get_style_context(btn), "app-item");
        
        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        GIcon *gicon = g_app_info_get_icon(app);
        GtkWidget *img = gtk_image_new_from_gicon(gicon, GTK_ICON_SIZE_DIALOG);
        gtk_image_set_pixel_size(GTK_IMAGE(img), ICON_SIZE);
        
        GtkWidget *lbl = gtk_label_new(g_app_info_get_name(app));
        gtk_label_set_ellipsize(GTK_LABEL(lbl), PANGO_ELLIPSIZE_END);
        gtk_label_set_max_width_chars(GTK_LABEL(lbl), 11);
        gtk_widget_set_halign(lbl, GTK_ALIGN_CENTER);

        gtk_box_pack_start(GTK_BOX(box), img, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(box), lbl, FALSE, FALSE, 0);
        gtk_container_add(GTK_CONTAINER(btn), box);
        
        g_signal_connect(btn, "clicked", G_CALLBACK(on_app_clicked), app);
        
        gint row = displayed / GRID_COLS;
        gint col = displayed % GRID_COLS;
        gtk_grid_attach(GTK_GRID(ol->grid), btn, col, row, 1, 1);
        displayed++;
    }

    /* Update Dots and Arrows */
    gtk_container_foreach(GTK_CONTAINER(ol->page_dots), (GtkCallback)gtk_widget_destroy, NULL);
    
    if (ol->total_pages <= 1) return;

    /* Previous arrow */
    GtkWidget *prev_arrow = gtk_button_new();
    const gchar *prev_icon_path = g_file_test(CHEVRON_LEFT_PATH, G_FILE_TEST_EXISTS) ? CHEVRON_LEFT_PATH : CHEVRON_LEFT_LOCAL_PATH;
    gtk_button_set_image(GTK_BUTTON(prev_arrow), gtk_image_new_from_file(prev_icon_path));
    gtk_style_context_add_class(gtk_widget_get_style_context(prev_arrow), "nav-arrow");
    gtk_widget_set_valign(prev_arrow, GTK_ALIGN_CENTER);
    g_object_set_data(G_OBJECT(prev_arrow), "is-next", GINT_TO_POINTER(FALSE));
    g_signal_connect(prev_arrow, "clicked", G_CALLBACK(on_nav_arrow_clicked), ol);
    gtk_box_pack_start(GTK_BOX(ol->page_dots), prev_arrow, FALSE, FALSE, 0);
    gtk_widget_set_sensitive(prev_arrow, ol->current_page > 0);

    for (int i = 0; i < ol->total_pages; i++) {
        GtkWidget *dot = gtk_button_new();
        gtk_style_context_add_class(gtk_widget_get_style_context(dot), "page-dot");
        gtk_widget_set_valign(dot, GTK_ALIGN_CENTER);
        if (i == ol->current_page) gtk_style_context_add_class(gtk_widget_get_style_context(dot), "active");
        
        g_object_set_data(G_OBJECT(dot), "page-index", GINT_TO_POINTER(i));
        g_signal_connect(dot, "clicked", G_CALLBACK(on_dot_clicked), ol);
        
        gtk_box_pack_start(GTK_BOX(ol->page_dots), dot, FALSE, FALSE, 0);
    }

    /* Next arrow */
    GtkWidget *next_arrow = gtk_button_new();
    const gchar *next_icon_path = g_file_test(CHEVRON_RIGHT_PATH, G_FILE_TEST_EXISTS) ? CHEVRON_RIGHT_PATH : CHEVRON_RIGHT_LOCAL_PATH;
    gtk_button_set_image(GTK_BUTTON(next_arrow), gtk_image_new_from_file(next_icon_path));
    gtk_style_context_add_class(gtk_widget_get_style_context(next_arrow), "nav-arrow");
    gtk_widget_set_valign(next_arrow, GTK_ALIGN_CENTER);
    g_object_set_data(G_OBJECT(next_arrow), "is-next", GINT_TO_POINTER(TRUE));
    g_signal_connect(next_arrow, "clicked", G_CALLBACK(on_nav_arrow_clicked), ol);
    gtk_box_pack_start(GTK_BOX(ol->page_dots), next_arrow, FALSE, FALSE, 0);
    gtk_widget_set_sensitive(next_arrow, ol->current_page < ol->total_pages - 1);

    gtk_widget_show_all(ol->page_dots);

    g_list_free(filtered);
    gtk_widget_show_all(ol->window);
}

void vexto_launcher_show_grid(VextoLauncher *ol) {
    if (!ol->window) {
        ol->all_apps = g_app_info_get_all();
        ol->current_page = 0;

        ol->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_decorated(GTK_WINDOW(ol->window), FALSE);
        gtk_window_set_type_hint(GTK_WINDOW(ol->window), GDK_WINDOW_TYPE_HINT_DIALOG);
        gtk_window_set_skip_taskbar_hint(GTK_WINDOW(ol->window), TRUE);
        gtk_window_set_skip_pager_hint(GTK_WINDOW(ol->window), TRUE);
        gtk_window_set_default_size(GTK_WINDOW(ol->window), 600, 500);
        gtk_window_set_position(GTK_WINDOW(ol->window), GTK_WIN_POS_CENTER);
        gtk_style_context_add_class(gtk_widget_get_style_context(ol->window), "launcher-window");

        /* Enable Transparency */
        GdkScreen *screen = gtk_widget_get_screen(ol->window);
        GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
        if (visual && gdk_screen_is_composited(screen)) {
            gtk_widget_set_visual(ol->window, visual);
        }

        /* Apply CSS */
        GtkCssProvider *css = gtk_css_provider_new();
        gtk_css_provider_load_from_data(css, vexto_launcher_get_css(), -1, NULL);
        gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(css), GTK_STYLE_PROVIDER_PRIORITY_USER);

        GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_container_add(GTK_CONTAINER(ol->window), main_box);

        /* Header Row (Search, Settings, Power) */
        GtkWidget *header_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_box_pack_start(GTK_BOX(main_box), header_row, FALSE, FALSE, 0);

        /* Search Entry Container */
        GtkWidget *search_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_style_context_add_class(gtk_widget_get_style_context(search_box), "search-container");
        gtk_box_pack_start(GTK_BOX(header_row), search_box, TRUE, TRUE, 0);

        ol->search_entry = gtk_search_entry_new();
        gtk_entry_set_placeholder_text(GTK_ENTRY(ol->search_entry), "Pesquisar Aplicativos");
        gtk_widget_set_hexpand(ol->search_entry, TRUE);
        gtk_box_pack_start(GTK_BOX(search_box), ol->search_entry, TRUE, TRUE, 0);
        g_signal_connect(ol->search_entry, "search-changed", G_CALLBACK(on_search_changed), ol);

        /* Settings Button */
        GtkWidget *settings_btn = gtk_button_new();
        gtk_style_context_add_class(gtk_widget_get_style_context(settings_btn), "action-button");
        gtk_button_set_image(GTK_BUTTON(settings_btn), gtk_image_new_from_icon_name("preferences-system-symbolic", GTK_ICON_SIZE_BUTTON));
        g_signal_connect(settings_btn, "clicked", G_CALLBACK(on_settings_clicked), ol);
        gtk_box_pack_start(GTK_BOX(header_row), settings_btn, FALSE, FALSE, 0);

        /* Power Button */
        GtkWidget *power_btn = gtk_button_new();
        gtk_style_context_add_class(gtk_widget_get_style_context(power_btn), "action-button");
        gtk_button_set_image(GTK_BUTTON(power_btn), gtk_image_new_from_icon_name("system-shutdown-symbolic", GTK_ICON_SIZE_BUTTON));
        g_signal_connect(power_btn, "clicked", G_CALLBACK(on_power_clicked), ol);
        gtk_box_pack_start(GTK_BOX(header_row), power_btn, FALSE, FALSE, 0);

        /* Grid Padding Container */
        GtkWidget *grid_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
        gtk_container_set_border_width(GTK_CONTAINER(grid_container), 10);
        gtk_box_pack_start(GTK_BOX(main_box), grid_container, TRUE, TRUE, 0);

        /* Grid */
        ol->grid = gtk_grid_new();
        gtk_grid_set_column_spacing(GTK_GRID(ol->grid), 10);
        gtk_grid_set_row_spacing(GTK_GRID(ol->grid), 10);
        gtk_widget_set_halign(ol->grid, GTK_ALIGN_CENTER);
        gtk_widget_set_valign(ol->grid, GTK_ALIGN_CENTER);
        gtk_box_pack_start(GTK_BOX(grid_container), ol->grid, TRUE, TRUE, 0);

        /* Page Dots */
        ol->page_dots = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
        gtk_style_context_add_class(gtk_widget_get_style_context(ol->page_dots), "page-dots-container");
        gtk_widget_set_halign(ol->page_dots, GTK_ALIGN_CENTER);
        gtk_box_pack_start(GTK_BOX(main_box), ol->page_dots, FALSE, FALSE, 0);

        g_signal_connect(ol->window, "focus-out-event", G_CALLBACK(gtk_widget_hide_on_delete), NULL);
    }

    vexto_launcher_populate_grid(ol);
    if (ol->button) gtk_style_context_add_class(gtk_widget_get_style_context(ol->button), "active");
    gtk_window_present(GTK_WINDOW(ol->window));
    gtk_widget_grab_focus(ol->search_entry);
}
