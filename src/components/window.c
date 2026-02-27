#include "../vexto-launcher.h"

static const struct { gchar *icon; gchar *cmd; } actions_data[] = {
    {"system-shutdown-symbolic", "xfce4-session-logout --halt"},
    {"emblem-system-symbolic", "xfce4-settings-manager"},
    {"system-search-symbolic", "focus-search"},
    {"system-log-out-symbolic", "xfce4-session-logout --logout"}
};

static void on_search_changed(GtkSearchEntry *entry, VextoLauncher *ol) {
    vexto_launcher_grid_update(ol, gtk_entry_get_text(GTK_ENTRY(entry)));
}

static void on_action_clicked(GtkWidget *btn, gpointer data) {
    const gchar *cmd = (const gchar *)data;
    if (g_strcmp0(cmd, "focus-search") == 0) return;
    g_spawn_command_line_async(cmd, NULL);
}

static gboolean on_window_focus_out(GtkWidget *widget, GdkEventFocus *event, VextoLauncher *ol) {
    if (ol->window && gtk_widget_get_visible(ol->window)) {
        vexto_launcher_hide(ol);
    }
    return FALSE;
}

static void on_page_prev(GtkWidget *btn, VextoLauncher *ol) {
    vexto_launcher_grid_set_page(ol, ol->current_page - 1);
}

static void on_page_next(GtkWidget *btn, VextoLauncher *ol) {
    vexto_launcher_grid_set_page(ol, ol->current_page + 1);
}

void vexto_launcher_window_init(VextoLauncher *ol) {
    ol->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_name(ol->window, "vexto-launcher-window");
    gtk_window_set_decorated(GTK_WINDOW(ol->window), FALSE);
    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(ol->window), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(ol->window), DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
    gtk_window_set_type_hint(GTK_WINDOW(ol->window), GDK_WINDOW_TYPE_HINT_POPUP_MENU);

    /* RGBA visual ensures rounded corners from CSS don't show black edges */
    GdkScreen *screen = gtk_widget_get_screen(ol->window);
    GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
    if (visual) gtk_widget_set_visual(ol->window, visual);

    GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(ol->window), main_vbox);

    /* Header with Search */
    ol->header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_name(ol->header, "vexto-launcher-header");
    gtk_box_pack_start(GTK_BOX(main_vbox), ol->header, FALSE, FALSE, 0);

    ol->search_entry = gtk_search_entry_new();
    gtk_widget_set_name(ol->search_entry, "vexto-launcher-search");
    gtk_box_pack_start(GTK_BOX(ol->header), ol->search_entry, TRUE, TRUE, 0);
    g_signal_connect(ol->search_entry, "search-changed", G_CALLBACK(on_search_changed), ol);

    /* Grid Area */
    ol->grid = gtk_grid_new();
    gtk_widget_set_name(ol->grid, "vexto-launcher-grid");
    gtk_grid_set_column_homogeneous(GTK_GRID(ol->grid), TRUE);
    gtk_grid_set_row_homogeneous(GTK_GRID(ol->grid), TRUE);
    gtk_grid_set_column_spacing(GTK_GRID(ol->grid), 12);
    gtk_grid_set_row_spacing(GTK_GRID(ol->grid), 12);
    gtk_box_pack_start(GTK_BOX(main_vbox), ol->grid, TRUE, TRUE, 0);

    /* Footer with Pagination and Actions */
    ol->footer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_name(ol->footer, "vexto-launcher-footer");
    gtk_container_set_border_width(GTK_CONTAINER(ol->footer), 8);
    gtk_box_pack_start(GTK_BOX(main_vbox), ol->footer, FALSE, FALSE, 0);

    /* Navigation Group (Arrows) - Left Aligned */
    GtkWidget *nav_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_widget_set_halign(nav_box, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(ol->footer), nav_box, FALSE, FALSE, 0);

    /* Prev Button (Chevron) */
    GtkWidget *btn_prev = gtk_button_new_from_icon_name("pan-start-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_prev), "nav-btn");
    g_signal_connect(btn_prev, "clicked", G_CALLBACK(on_page_prev), ol);
    gtk_box_pack_start(GTK_BOX(nav_box), btn_prev, FALSE, FALSE, 0);

    /* Next Button (Chevron) */
    GtkWidget *btn_next = gtk_button_new_from_icon_name("pan-end-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_next), "nav-btn");
    g_signal_connect(btn_next, "clicked", G_CALLBACK(on_page_next), ol);
    gtk_box_pack_start(GTK_BOX(nav_box), btn_next, FALSE, FALSE, 0);

    /* Actions on the Right */
    GtkWidget *action_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    gtk_box_pack_end(GTK_BOX(ol->footer), action_box, FALSE, FALSE, 0);

    for (int i = 0; i < 4; i++) {
        GtkWidget *btn = gtk_button_new_from_icon_name(actions_data[i].icon, GTK_ICON_SIZE_BUTTON);
        gtk_style_context_add_class(gtk_widget_get_style_context(btn), "action-btn");
        if (g_strcmp0(actions_data[i].cmd, "focus-search") == 0) {
            g_signal_connect_swapped(btn, "clicked", G_CALLBACK(gtk_widget_grab_focus), ol->search_entry);
        } else {
            g_signal_connect(btn, "clicked", G_CALLBACK(on_action_clicked), (gpointer)actions_data[i].cmd);
        }
        gtk_box_pack_end(GTK_BOX(action_box), btn, FALSE, FALSE, 0);
    }

    g_signal_connect(ol->window, "focus-out-event", G_CALLBACK(on_window_focus_out), ol);
}

void vexto_launcher_show(VextoLauncher *ol) {
    if (!ol->window) vexto_launcher_window_init(ol);
    
    gint x = 0, y = 0;
    GtkAllocation alloc;
    gtk_widget_get_allocation(ol->button, &alloc);
    
    GdkWindow *gdk_win = gtk_widget_get_window(ol->button);
    if (gdk_win) {
        gdk_window_get_origin(gdk_win, &x, &y);
        
        /* Position exactly above the panel (assuming bottom panel) */
        /* Margin 8px above the panel for better aesthetics */
        y -= (DEFAULT_WINDOW_HEIGHT + 8);
    }
    
    gtk_window_move(GTK_WINDOW(ol->window), x, y);
    
    vexto_launcher_grid_update(ol, NULL);
    gtk_widget_show_all(ol->window);
    gtk_widget_grab_focus(ol->search_entry);
    gtk_style_context_add_class(gtk_widget_get_style_context(ol->button), "active");
}

void vexto_launcher_hide(VextoLauncher *ol) {
    if (ol->window) gtk_widget_hide(ol->window);
    gtk_style_context_remove_class(gtk_widget_get_style_context(ol->button), "active");
}
