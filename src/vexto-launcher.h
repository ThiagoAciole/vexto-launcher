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

typedef struct {
  XfcePanelPlugin *plugin;

  /* UI Components */
  GtkWidget *button;       /* Panel button */
  GtkWidget *icon;         /* Panel icon */
  GtkWidget *window;       /* Main launcher popup */
  GtkWidget *header;       /* Search area */
  GtkWidget *grid;         /* App grid */
  GtkWidget *search_entry; /* Search input */
  GtkWidget *page_dots;    /* Pagination container */
  GtkWidget *footer;       /* Bottom actions area */

  /* Data */
  GList *all_apps;
  GList *filtered_apps; /* Currently filtered set */
  gint current_page;
  gint total_pages;
  gint icon_size;   /* Custom icon size */
  gchar *icon_name; /* Custom icon name */
} VextoLauncher;

/* Component functions */
void vexto_launcher_window_init(VextoLauncher *ol);
void vexto_launcher_grid_update(VextoLauncher *ol, const gchar *search_text);
void vexto_launcher_grid_set_page(VextoLauncher *ol, gint page);
void vexto_launcher_show(VextoLauncher *ol);
void vexto_launcher_hide(VextoLauncher *ol);

/* Utils */
void vexto_launcher_style_init(void);
void vexto_launcher_style_apply(GtkWidget *widget);

#endif /* __vexto_launcher_H__ */
