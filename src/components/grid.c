#include "../utils/categories.h"
#include "../vexto-launcher.h"

/* Exposed globally so window.c can use it for the standalone back button */
void on_item_clicked(GtkWidget *btn, gpointer data) {
  VextoGridItem *item = (VextoGridItem *)data;
  VextoLauncher *ol = g_object_get_data(G_OBJECT(btn), "ol");
  if (!ol)
    return;

  if (item->type == ITEM_TYPE_APP) {
    GError *error = NULL;
    if (!g_app_info_launch(item->app, NULL, NULL, &error)) {
      g_warning("Could not launch app: %s", error->message);
      g_error_free(error);
    }
    /* Hide the grid after launch */
    vexto_launcher_hide(ol);
  } else if (item->type == ITEM_TYPE_FOLDER) {
    ol->current_folder = item;
    /* Clear search if moving into folder? Usually yes */
    if (ol->search_entry)
      gtk_entry_set_text(GTK_ENTRY(ol->search_entry), "");
    vexto_launcher_grid_update(ol, NULL);
  } else if (item->type == ITEM_TYPE_BACK) {
    ol->current_folder = NULL;
    if (ol->search_entry)
      gtk_entry_set_text(GTK_ENTRY(ol->search_entry), "");
    vexto_launcher_grid_update(ol, NULL);
  }
}

static GtkWidget *create_item_button(VextoLauncher *ol, VextoGridItem *item) {
  GtkWidget *btn = gtk_button_new();
  gtk_widget_set_name(btn, "app-button");
  gtk_style_context_add_class(gtk_widget_get_style_context(btn), "app-button");
  if (item->type == ITEM_TYPE_FOLDER) {
    gtk_style_context_add_class(gtk_widget_get_style_context(btn),
                                "folder-button");
  } else if (item->type == ITEM_TYPE_BACK) {
    gtk_style_context_add_class(gtk_widget_get_style_context(btn),
                                "back-button");
  }

  gtk_widget_set_can_focus(btn, TRUE);

  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
  gtk_container_add(GTK_CONTAINER(btn), vbox);

  GtkWidget *img = NULL;
  if (item->icon) {
    img = gtk_image_new_from_gicon(item->icon, GTK_ICON_SIZE_DIALOG);
  } else {
    img = gtk_image_new_from_icon_name("application-x-executable",
                                       GTK_ICON_SIZE_DIALOG);
  }
  gtk_box_pack_start(GTK_BOX(vbox), img, TRUE, TRUE, 0);

  GtkWidget *lbl = gtk_label_new(item->name);
  gtk_label_set_ellipsize(GTK_LABEL(lbl), PANGO_ELLIPSIZE_END);
  gtk_label_set_max_width_chars(GTK_LABEL(lbl), 12);
  gtk_label_set_lines(GTK_LABEL(lbl), 1);
  gtk_box_pack_start(GTK_BOX(vbox), lbl, FALSE, FALSE, 0);

  g_object_set_data(G_OBJECT(btn), "ol", ol);
  g_signal_connect(btn, "clicked", G_CALLBACK(on_item_clicked), item);

  return btn;
}

void vexto_launcher_grid_set_page(VextoLauncher *ol, guint page) {
  if (page < 0 || page >= ol->total_pages)
    return;
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

  for (GList *l = ol->filtered_items; l != NULL; l = l->next) {
    if (count < start) {
      count++;
      continue;
    }
    if (attached >= APPS_PER_PAGE)
      break;

    GtkWidget *btn = create_item_button(ol, (VextoGridItem *)l->data);
    gtk_grid_attach(GTK_GRID(ol->grid), btn, attached % GRID_COLS,
                    attached / GRID_COLS, 1, 1);
    attached++;
    count++;
  }

  /* Fill remaining slots with dummy boxes so the grid always has exactly
   * APPS_PER_PAGE cells. Because the grid is homogeneous and these boxes
   * expand, this forces each cell to retain its proper size and prevents
   * visible items from stretching vertically. */
  for (; attached < APPS_PER_PAGE; attached++) {
    GtkWidget *dummy = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_hexpand(dummy, TRUE);
    gtk_widget_set_vexpand(dummy, TRUE);
    gtk_grid_attach(GTK_GRID(ol->grid), dummy, attached % GRID_COLS,
                    attached / GRID_COLS, 1, 1);
  }

  gtk_widget_show_all(ol->grid);
}

static void filter_items(VextoLauncher *ol, GList *source_items,
                         const gchar *search_text) {
  for (GList *l = source_items; l != NULL; l = l->next) {
    VextoGridItem *item = (VextoGridItem *)l->data;

    if (search_text && *search_text) {
      gchar *lower_search = g_utf8_strdown(search_text, -1);
      gchar *lower_name = g_utf8_strdown(item->name, -1);
      gboolean match = (strstr(lower_name, lower_search) != NULL);
      g_free(lower_search);
      g_free(lower_name);

      if (!match)
        continue;
    }
    ol->filtered_items = g_list_prepend(ol->filtered_items, item);
  }
}

void vexto_launcher_grid_update(VextoLauncher *ol, const gchar *search_text) {
  /* Reset filter and page */
  if (ol->filtered_items) {
    /* If the first item was a dynamically created back button, free it */
    VextoGridItem *first = (VextoGridItem *)ol->filtered_items->data;
    if (first && first->type == ITEM_TYPE_BACK) {
      vexto_grid_item_free(first);
    }
    g_list_free(ol->filtered_items);
  }
  ol->filtered_items = NULL;

  if (!ol->all_items)
    ol->all_items = vexto_launcher_parse_and_group_apps();

  /* Determine source list and Header state */
  if (ol->current_folder) {
    filter_items(ol, ol->current_folder->folder_apps, search_text);
    /* Show folder header and set title */
    gtk_label_set_text(GTK_LABEL(ol->folder_label), ol->current_folder->name);
    gtk_widget_show(ol->folder_header);
    gtk_widget_show_all(ol->folder_header);
  } else {
    filter_items(ol, ol->all_items, search_text);
    /* Hide folder header */
    gtk_widget_hide(ol->folder_header);
  }

  ol->filtered_items = g_list_reverse(ol->filtered_items);

  /* Calculate pages */
  guint n_items = g_list_length(ol->filtered_items);
  ol->total_pages = (n_items + APPS_PER_PAGE - 1) / APPS_PER_PAGE;
  if (ol->total_pages == 0)
    ol->total_pages = 1;

  vexto_launcher_grid_set_page(ol, 0);
}
