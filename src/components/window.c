#include "../utils/categories.h"
#include "../labs-launcher.h"

/* ── Callbacks ─────────────────────────────────────────── */

static void on_search_changed(GtkSearchEntry *entry, LabsLauncher *ol) {
  labs_launcher_grid_update(ol, gtk_entry_get_text(GTK_ENTRY(entry)));
}

static void on_action_clicked(GtkWidget *btn, gpointer data) {
  g_spawn_command_line_async((const gchar *)data, NULL);
}

static void on_logout_clicked(GtkWidget *btn, LabsLauncher *ol) {
  GtkWidget *dialog = gtk_message_dialog_new(
      GTK_WINDOW(ol->window),
      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
      GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
      "Encerrar a sess\u00e3o atual?");
  gtk_message_dialog_format_secondary_text(
      GTK_MESSAGE_DIALOG(dialog),
      "Todos os programas abertos ser\u00e3o fechados.");
  gtk_dialog_add_button(GTK_DIALOG(dialog), "Cancelar", GTK_RESPONSE_CANCEL);
  gtk_dialog_add_button(GTK_DIALOG(dialog), "Encerrar Sess\u00e3o", GTK_RESPONSE_OK);
  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    labs_launcher_hide(ol);
    g_spawn_command_line_async("xfce4-session-logout --logout", NULL);
  } else {
    gtk_widget_destroy(dialog);
  }
}

static void on_power_clicked(GtkWidget *btn, LabsLauncher *ol) {
  GtkWidget *dialog = gtk_message_dialog_new(
      GTK_WINDOW(ol->window),
      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
      GTK_MESSAGE_QUESTION, GTK_BUTTONS_NONE,
      "Desligar o computador?");
  gtk_message_dialog_format_secondary_text(
      GTK_MESSAGE_DIALOG(dialog),
      "O sistema ser\u00e1 encerrado imediatamente.");
  gtk_dialog_add_button(GTK_DIALOG(dialog), "Cancelar", GTK_RESPONSE_CANCEL);
  gtk_dialog_add_button(GTK_DIALOG(dialog), "Desligar", GTK_RESPONSE_OK);
  gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
  gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
    gtk_widget_destroy(dialog);
    labs_launcher_hide(ol);
    g_spawn_command_line_async("xfce4-session-logout", NULL);
  } else {
    gtk_widget_destroy(dialog);
  }
}

static gboolean on_window_focus_out(GtkWidget *widget, GdkEventFocus *event,
                                    LabsLauncher *ol) {
  if (ol->window && gtk_widget_get_visible(ol->window))
    labs_launcher_hide(ol);
  return FALSE;
}

static void on_page_prev(GtkWidget *btn, LabsLauncher *ol) {
  labs_launcher_grid_set_page(ol, ol->current_page - 1);
}

static void on_page_next(GtkWidget *btn, LabsLauncher *ol) {
  labs_launcher_grid_set_page(ol, ol->current_page + 1);
}

static gboolean on_window_scroll(GtkWidget *widget, GdkEventScroll *event,
                                 LabsLauncher *ol) {
  if (event->direction == GDK_SCROLL_UP) {
    labs_launcher_grid_set_page(ol, ol->current_page - 1);
    return TRUE;
  } else if (event->direction == GDK_SCROLL_DOWN) {
    labs_launcher_grid_set_page(ol, ol->current_page + 1);
    return TRUE;
  }
  return FALSE;
}

static gboolean on_window_key_press(GtkWidget *widget, GdkEventKey *event,
                                    LabsLauncher *ol) {
  if (event->keyval == GDK_KEY_Page_Up) {
    labs_launcher_grid_set_page(ol, ol->current_page - 1);
    return TRUE;
  } else if (event->keyval == GDK_KEY_Page_Down) {
    labs_launcher_grid_set_page(ol, ol->current_page + 1);
    return TRUE;
  } else if (event->keyval == GDK_KEY_Escape) {
    labs_launcher_hide(ol);
    return TRUE;
  }
  return FALSE;
}

static gboolean on_search_key_press(GtkWidget *widget, GdkEventKey *event,
                                    LabsLauncher *ol) {
  if (event->keyval == GDK_KEY_Down && ol->grid) {
    GList *children = gtk_container_get_children(GTK_CONTAINER(ol->grid));
    if (children && children->data)
      gtk_widget_grab_focus(GTK_WIDGET(children->data));
    g_list_free(children);
    return TRUE;
  }
  return FALSE;
}

/* ── Window Init ───────────────────────────────────────── */

void labs_launcher_window_init(LabsLauncher *ol) {
  ol->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_widget_set_name(ol->window, "labs-launcher-window");
  gtk_window_set_decorated(GTK_WINDOW(ol->window), FALSE);
  gtk_window_set_skip_taskbar_hint(GTK_WINDOW(ol->window), TRUE);
  gtk_window_set_default_size(GTK_WINDOW(ol->window), DEFAULT_WINDOW_WIDTH,
                              DEFAULT_WINDOW_HEIGHT);
  gtk_window_set_type_hint(GTK_WINDOW(ol->window),
                           GDK_WINDOW_TYPE_HINT_POPUP_MENU);

  GdkScreen *screen = gtk_widget_get_screen(ol->window);
  GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
  if (visual)
    gtk_widget_set_visual(ol->window, visual);

  GtkWidget *main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  gtk_container_add(GTK_CONTAINER(ol->window), main_vbox);

  /* ── HEADER: Search + Settings ─────────────────────── */
  ol->header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
  gtk_widget_set_name(ol->header, "labs-launcher-header");
  gtk_box_pack_start(GTK_BOX(main_vbox), ol->header, FALSE, FALSE, 0);

  ol->search_entry = gtk_search_entry_new();
  gtk_widget_set_name(ol->search_entry, "labs-launcher-search");
  gtk_box_pack_start(GTK_BOX(ol->header), ol->search_entry, TRUE, TRUE, 0);
  g_signal_connect(ol->search_entry, "search-changed",
                   G_CALLBACK(on_search_changed), ol);
  g_signal_connect(ol->search_entry, "key-press-event",
                   G_CALLBACK(on_search_key_press), ol);

  GtkWidget *settings_btn = gtk_button_new_from_icon_name(
      "preferences-system-symbolic", GTK_ICON_SIZE_BUTTON);
  gtk_widget_set_name(settings_btn, "labs-settings-btn");
  gtk_style_context_add_class(gtk_widget_get_style_context(settings_btn),
                              "header-action-btn");
  gtk_widget_set_tooltip_text(settings_btn, "Configurações");
  g_signal_connect(settings_btn, "clicked", G_CALLBACK(on_action_clicked),
                   (gpointer) "xfce4-settings-manager");
  gtk_box_pack_end(GTK_BOX(ol->header), settings_btn, FALSE, FALSE, 0);

  /* ── FOLDER HEADER (hidden by default) ─────────────── */
  ol->folder_header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
  gtk_widget_set_name(ol->folder_header, "labs-launcher-folder-header");
  gtk_box_pack_start(GTK_BOX(main_vbox), ol->folder_header, FALSE, FALSE, 0);

  GtkWidget *back_btn = gtk_button_new_from_icon_name("go-previous-symbolic",
                                                      GTK_ICON_SIZE_BUTTON);
  gtk_style_context_add_class(gtk_widget_get_style_context(back_btn),
                              "back-nav-btn");
  gtk_button_set_relief(GTK_BUTTON(back_btn), GTK_RELIEF_NONE);
  gtk_box_pack_start(GTK_BOX(ol->folder_header), back_btn, FALSE, FALSE, 0);

  ol->folder_label = gtk_label_new("Categoria");
  gtk_label_set_xalign(GTK_LABEL(ol->folder_label), 0.0);
  gtk_style_context_add_class(gtk_widget_get_style_context(ol->folder_label),
                              "folder-title");
  gtk_box_pack_start(GTK_BOX(ol->folder_header), ol->folder_label, TRUE, TRUE,
                     0);

  LabsGridItem *back_item = labs_grid_item_new_back();
  g_object_set_data(G_OBJECT(back_btn), "ol", ol);
  g_signal_connect(back_btn, "clicked", G_CALLBACK(on_item_clicked), back_item);

  /* ── GRID inside ScrolledWindow ─────────────────────── */
  GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_set_name(scrolled, "labs-launcher-scroll");
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                 GTK_POLICY_AUTOMATIC, GTK_POLICY_NEVER);
  gtk_box_pack_start(GTK_BOX(main_vbox), scrolled, TRUE, TRUE, 0);

  ol->grid = gtk_grid_new();
  gtk_widget_set_name(ol->grid, "labs-launcher-grid");
  gtk_grid_set_column_homogeneous(GTK_GRID(ol->grid), TRUE);
  gtk_grid_set_row_homogeneous(GTK_GRID(ol->grid), TRUE);
  gtk_grid_set_column_spacing(GTK_GRID(ol->grid), 12);
  gtk_grid_set_row_spacing(GTK_GRID(ol->grid), 12);
  gtk_container_add(GTK_CONTAINER(scrolled), ol->grid);

  /* ── FOOTER: User Info + Action Group ───────────────── */
  ol->footer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_widget_set_name(ol->footer, "labs-launcher-footer");
  gtk_container_set_border_width(GTK_CONTAINER(ol->footer), 10);
  gtk_box_pack_start(GTK_BOX(main_vbox), ol->footer, FALSE, FALSE, 0);

  /* User info — left side */
  GtkWidget *user_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
  gtk_box_pack_start(GTK_BOX(ol->footer), user_box, TRUE, TRUE, 0);

  GtkWidget *user_icon = gtk_image_new_from_icon_name(
      "avatar-default-symbolic", GTK_ICON_SIZE_DND);
  gtk_widget_set_name(user_icon, "labs-user-avatar");
  gtk_box_pack_start(GTK_BOX(user_box), user_icon, FALSE, FALSE, 0);

  const gchar *real_name = g_get_real_name();
  const gchar *display_name =
      (real_name && g_strcmp0(real_name, "Unknown") != 0) ? real_name
                                                           : g_get_user_name();
  GtkWidget *user_label = gtk_label_new(display_name);
  gtk_style_context_add_class(gtk_widget_get_style_context(user_label),
                              "user-label");
  gtk_widget_set_valign(user_label, GTK_ALIGN_CENTER);
  gtk_box_pack_start(GTK_BOX(user_box), user_label, FALSE, FALSE, 0);

  /* Action group — right side */
  GtkWidget *action_group = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_style_context_add_class(gtk_widget_get_style_context(action_group),
                              "action-group");
  gtk_widget_set_valign(action_group, GTK_ALIGN_CENTER);
  gtk_box_pack_end(GTK_BOX(ol->footer), action_group, FALSE, FALSE, 0);

  GtkWidget *logout_btn = gtk_button_new_from_icon_name(
      "system-log-out-symbolic", GTK_ICON_SIZE_BUTTON);
  gtk_style_context_add_class(gtk_widget_get_style_context(logout_btn),
                              "group-btn");
  gtk_widget_set_tooltip_text(logout_btn, "Encerrar Sessão");
  g_signal_connect(logout_btn, "clicked", G_CALLBACK(on_logout_clicked), ol);
  gtk_box_pack_start(GTK_BOX(action_group), logout_btn, FALSE, FALSE, 0);

  GtkWidget *power_btn = gtk_button_new_from_icon_name(
      "system-shutdown-symbolic", GTK_ICON_SIZE_BUTTON);
  gtk_style_context_add_class(gtk_widget_get_style_context(power_btn),
                              "group-btn");
  gtk_widget_set_tooltip_text(power_btn, "Desligar");
  g_signal_connect(power_btn, "clicked", G_CALLBACK(on_power_clicked), ol);
  gtk_box_pack_start(GTK_BOX(action_group), power_btn, FALSE, FALSE, 0);

  /* ── Window Signals ──────────────────────────────────── */
  g_signal_connect(ol->window, "focus-out-event",
                   G_CALLBACK(on_window_focus_out), ol);
  g_signal_connect(ol->window, "scroll-event", G_CALLBACK(on_window_scroll),
                   ol);
  g_signal_connect(ol->window, "key-press-event",
                   G_CALLBACK(on_window_key_press), ol);
  gtk_widget_add_events(ol->window, GDK_SCROLL_MASK | GDK_KEY_PRESS_MASK);
}

/* ── Show / Hide ───────────────────────────────────────── */

void labs_launcher_show(LabsLauncher *ol) {
  if (!ol->window)
    labs_launcher_window_init(ol);

  gint x = 0, y = 0;
  GdkWindow *gdk_win = gtk_widget_get_window(ol->button);
  if (gdk_win) {
    GtkAllocation alloc;
    gtk_widget_get_allocation(ol->button, &alloc);
    gdk_window_get_origin(gdk_win, &x, &y);
    y -= (DEFAULT_WINDOW_HEIGHT + 8);
  }

  gtk_window_move(GTK_WINDOW(ol->window), x, y);
  gtk_widget_show_all(ol->window);
  labs_launcher_grid_update(ol, NULL);
  gtk_widget_grab_focus(ol->search_entry);
  gtk_style_context_add_class(gtk_widget_get_style_context(ol->button),
                              "active");
}

void labs_launcher_hide(LabsLauncher *ol) {
  if (ol->window)
    gtk_widget_hide(ol->window);
  if (ol->button)
    gtk_style_context_remove_class(gtk_widget_get_style_context(ol->button),
                                   "active");

  ol->current_folder = NULL;
  if (ol->search_entry)
    gtk_entry_set_text(GTK_ENTRY(ol->search_entry), "");
}
