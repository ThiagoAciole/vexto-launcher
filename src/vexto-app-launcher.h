#ifndef __VEXTO_APP_LAUNCHER_H__
#define __VEXTO_APP_LAUNCHER_H__

#include <gtk/gtk.h>
#include <libxfce4panel/libxfce4panel.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>

/* Constants */
#define DEFAULT_WINDOW_WIDTH 600
#define DEFAULT_WINDOW_HEIGHT 600
#define APPS_PER_PAGE 12
#define GRID_COLS 4
#define GRID_ROWS 3

typedef struct {
    XfcePanelPlugin *plugin;
    
    /* UI Components */
    GtkWidget       *button;        /* Panel button */
    GtkWidget       *icon;          /* Panel icon */
    GtkWidget       *window;        /* Main launcher popup */
    GtkWidget       *header;        /* Search area */
    GtkWidget       *grid;          /* App grid */
    GtkWidget       *search_entry;  /* Search input */
    GtkWidget       *page_dots;     /* Pagination container */
    GtkWidget       *footer;        /* Bottom actions area */
    
    /* Data */
    GList           *all_apps;
    GList           *filtered_apps; /* Currently filtered set */
    gint            current_page;
    gint            total_pages;
} VextoAppLauncher;

/* Component functions */
void vexto_app_launcher_window_init(VextoAppLauncher *ol);
void vexto_app_launcher_grid_update(VextoAppLauncher *ol, const gchar *search_text);
void vexto_app_launcher_grid_set_page(VextoAppLauncher *ol, gint page);
void vexto_app_launcher_show(VextoAppLauncher *ol);
void vexto_app_launcher_hide(VextoAppLauncher *ol);

/* Utils */
void vexto_app_launcher_style_init(void);
void vexto_app_launcher_style_apply(GtkWidget *widget);

#endif /* __VEXTO_APP_LAUNCHER_H__ */
