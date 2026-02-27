#ifndef __vexto_launcher_H__
#define __vexto_launcher_H__

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
typedef enum { ITEM_TYPE_APP, ITEM_TYPE_FOLDER, ITEM_TYPE_BACK } VextoItemType;

typedef struct {
  VextoItemType type;
  gchar *id;          /* Used for sorting or identifying folder ID */
  gchar *name;        /* Display name (App or Folder) */
  GIcon *icon;        /* Display icon */
  GAppInfo *app;      /* Only valid if type == ITEM_TYPE_APP */
  GList *folder_apps; /* List of (VextoGridItem*) if type == ITEM_TYPE_FOLDER */
} VextoGridItem;

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
  GList *all_items;              /* Raw ordered list of VextoGridItem* */
  GList *filtered_items;         /* Currently filtered items being paginated */
  VextoGridItem *current_folder; /* NULL if root, or pointer to active folder */
  gint current_page;
  gint total_pages;
  gint icon_size;   /* Custom icon size */
  gchar *icon_name; /* Custom icon name */
} VextoLauncher;

/* Component functions */
void vexto_launcher_window_init(VextoLauncher *ol);
void vexto_launcher_grid_update(VextoLauncher *ol, const gchar *search_text);
void vexto_launcher_grid_set_page(VextoLauncher *ol, guint page);
void on_item_clicked(GtkWidget *btn, gpointer data);
void vexto_launcher_show(VextoLauncher *ol);
void vexto_launcher_hide(VextoLauncher *ol);

/* Utils */
void vexto_launcher_style_init(void);
void vexto_launcher_style_apply(GtkWidget *widget);

#endif /* __vexto_launcher_H__ */
