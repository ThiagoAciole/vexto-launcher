#include "categories.h"
#include <gio/gdesktopappinfo.h>
#include <string.h>

/* Helper to check if string exists in comma-separated priorities list, or array
 */
static const gchar *FAVORITE_APPS[] = {"Visual Studio Code",
                                       "Antigravity",
                                       "Navegador Web",
                                       "Firefox",
                                       "Google Chrome",
                                       "Mail Reader",
                                       "Cliente de e-mail",
                                       "Webapp-manager",
                                       "Thunar File Manager",
                                       "Notes",
                                       "Calculadora",
                                       "Mousepad",
                                       "Apps Snaps",
                                       "Xfce Terminal",
                                       "Software",
                                       "Software Install",
                                       NULL};

/* Struct for mapping categories */
typedef struct {
  const gchar *id;
  const gchar *display_name;
  const gchar *icon_name;
  const gchar *freedesktop_categories;
  VextoGridItem *folder_item; /* Created dynamically */
} CategoryMapping;

static CategoryMapping category_mappings[] = {
    {"utilities", "Utilitários", "applications-utilities", "Utility;Tool;"},
    {"system", "Sistema e Ferramentas", "applications-system",
     "System;Core;Monitor;Network;"},
    {"settings", "Configurações", "preferences-system",
     "Settings;X-XFCE-SettingsDialog;DesktopSettings;"},
    {"media", "Mídia e Entretenimento", "applications-multimedia",
     "AudioVideo;Audio;Video;Graphics;"},
    {"development", "Desenvolvimento", "applications-development",
     "Development;IDE;"},
    {"office", "Escritório", "applications-office", "Office;Dictionary;"},
    {"other", "Outros", "applications-other", "Other;"}, /* Fallback */
    {NULL, NULL, NULL, NULL}};

VextoGridItem *vexto_grid_item_new_app(GAppInfo *app) {
  VextoGridItem *item = g_new0(VextoGridItem, 1);
  item->type = ITEM_TYPE_APP;
  item->app = g_object_ref(app);
  item->name = g_strdup(g_app_info_get_name(app));
  item->icon = g_app_info_get_icon(app);
  if (item->icon)
    g_object_ref(item->icon);
  return item;
}

VextoGridItem *vexto_grid_item_new_folder(const gchar *id, const gchar *name,
                                          const gchar *icon_name) {
  VextoGridItem *item = g_new0(VextoGridItem, 1);
  item->type = ITEM_TYPE_FOLDER;
  item->id = g_strdup(id);
  item->name = g_strdup(name);
  item->icon = g_themed_icon_new(icon_name);
  item->folder_apps = NULL;
  return item;
}

VextoGridItem *vexto_grid_item_new_back(void) {
  VextoGridItem *item = g_new0(VextoGridItem, 1);
  item->type = ITEM_TYPE_BACK;
  item->id = g_strdup("back");
  item->name = g_strdup("Voltar");
  item->icon = g_themed_icon_new("go-previous");
  return item;
}

void vexto_grid_item_free(VextoGridItem *item) {
  if (!item)
    return;
  g_free(item->id);
  g_free(item->name);
  if (item->icon)
    g_object_unref(item->icon);
  if (item->app)
    g_object_unref(item->app);
  if (item->folder_apps) {
    g_list_free_full(item->folder_apps, (GDestroyNotify)vexto_grid_item_free);
  }
  g_free(item);
}

static gboolean is_favorite(GAppInfo *app) {
  const gchar *app_name = g_app_info_get_name(app);
  if (!app_name)
    return FALSE;

  for (int i = 0; FAVORITE_APPS[i] != NULL; i++) {
    /* Case-insensitive partial match for safety */
    gchar *lower_app = g_utf8_strdown(app_name, -1);
    gchar *lower_fav = g_utf8_strdown(FAVORITE_APPS[i], -1);

    gboolean match = FALSE;
    if (g_strcmp0(lower_app, lower_fav) == 0) {
      match = TRUE;
    }

    g_free(lower_app);
    g_free(lower_fav);

    if (match)
      return TRUE;
  }
  return FALSE;
}

static CategoryMapping *find_best_category(GAppInfo *app) {
  const gchar *cats = NULL;
  if (G_IS_DESKTOP_APP_INFO(app)) {
    cats = g_desktop_app_info_get_categories(G_DESKTOP_APP_INFO(app));
  }

  /* Edge cases from user testing */
  const gchar *app_name = g_app_info_get_name(app);
  if (app_name && g_str_has_suffix(app_name, "Preferences")) {
    return &category_mappings[2]; /* Settings */
  }

  if (!cats)
    return &category_mappings[6]; /* Other */

  for (int i = 0; category_mappings[i].id != NULL &&
                  category_mappings[i].freedesktop_categories != NULL;
       i++) {
    gchar **tokens =
        g_strsplit(category_mappings[i].freedesktop_categories, ";", -1);
    for (int j = 0; tokens[j] != NULL && *tokens[j] != '\0'; j++) {
      if (strstr(cats, tokens[j]) != NULL) {
        g_strfreev(tokens);
        return &category_mappings[i];
      }
    }
    g_strfreev(tokens);
  }

  return &category_mappings[6]; /* Other */
}

GList *vexto_launcher_parse_and_group_apps(void) {
  GList *installed_apps = g_app_info_get_all();

  GList *favorites = NULL;

  /* Initialize folder items */
  for (int i = 0; category_mappings[i].id != NULL; i++) {
    category_mappings[i].folder_item = vexto_grid_item_new_folder(
        category_mappings[i].id, category_mappings[i].display_name,
        category_mappings[i].icon_name);
  }

  /* Process all apps */
  for (GList *l = installed_apps; l != NULL; l = l->next) {
    GAppInfo *app = G_APP_INFO(l->data);

    if (is_favorite(app)) {
      favorites = g_list_append(favorites, vexto_grid_item_new_app(app));
    } else {
      CategoryMapping *mapping = find_best_category(app);
      mapping->folder_item->folder_apps = g_list_append(
          mapping->folder_item->folder_apps, vexto_grid_item_new_app(app));
    }
  }
  g_list_free_full(installed_apps, g_object_unref);

  /* Construct final list: Favorites first, then non-empty Folders */
  GList *final_items = favorites; /* favorites is already in order of scanning,
                                     or we could sort */

  for (int i = 0; category_mappings[i].id != NULL; i++) {
    if (category_mappings[i].folder_item->folder_apps != NULL) {
      final_items =
          g_list_append(final_items, category_mappings[i].folder_item);
    } else {
      vexto_grid_item_free(category_mappings[i].folder_item);
      category_mappings[i].folder_item = NULL;
    }
  }

  return final_items;
}
