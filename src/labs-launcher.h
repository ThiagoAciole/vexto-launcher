#ifndef __labs_launcher_H__
#define __labs_launcher_H__

#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>
#include <libxfce4ui/libxfce4ui.h>
#include <libxfce4util/libxfce4util.h>

/* Constants */
#define DEFAULT_WINDOW_WIDTH 600
#define DEFAULT_WINDOW_HEIGHT 600
#define APPS_PER_PAGE 12
#define GRID_COLS 4
#define GRID_ROWS 3

/* Grid Data Structures */
typedef enum { ITEM_TYPE_APP, ITEM_TYPE_FOLDER, ITEM_TYPE_BACK } LabsItemType;

typedef struct {
  LabsItemType type;
  gchar *id;          /* Used for sorting or identifying folder ID */
  gchar *name;        /* Display name (App or Folder) */
  GIcon *icon;        /* Display icon */
  GAppInfo *app;      /* Only valid if type == ITEM_TYPE_APP */
  GList *folder_apps; /* List of (LabsGridItem*) if type == ITEM_TYPE_FOLDER */
} LabsGridItem;

typedef struct {
  XfcePanelPlugin *plugin;

  /* UI Components */
  GtkWidget *button;        /* Panel button */
  GtkWidget *icon;          /* Panel icon */
  GtkWidget *window;        /* Main launcher popup */
  GtkWidget *header;        /* Search area */
  GtkWidget *folder_header; /* < [Folder Name] section */
  GtkWidget *folder_label;  /* Label inside folder_header */
  GtkWidget *grid;          /* App grid */
  GtkWidget *search_entry;  /* Search input */
  GtkWidget *page_dots;     /* Pagination container */
  GtkWidget *footer;        /* Bottom actions area */

  /* Data */
  GList *all_items;              /* Raw ordered list of LabsGridItem* */
  GList *filtered_items;         /* Currently filtered items being paginated */
  LabsGridItem *current_folder; /* NULL if root, or pointer to active folder */
  gint current_page;
  gint total_pages;
  gint icon_size;   /* Custom icon size */
  gchar *icon_name; /* Custom icon name */
} LabsLauncher;

/* Component functions */
void labs_launcher_window_init(LabsLauncher *ol);
void labs_launcher_grid_update(LabsLauncher *ol, const gchar *search_text);
void labs_launcher_grid_set_page(LabsLauncher *ol, guint page);
void on_item_clicked(GtkWidget *btn, gpointer data);
void labs_launcher_show(LabsLauncher *ol);
void labs_launcher_hide(LabsLauncher *ol);

/* Utils */
void labs_launcher_style_init(void);
void labs_launcher_style_apply(GtkWidget *widget);

#endif /* __labs_launcher_H__ */
