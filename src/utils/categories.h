#ifndef __labs_launcher_CATEGORIES_H__
#define __labs_launcher_CATEGORIES_H__

#include "../labs-launcher.h"

/* Creation functions */
LabsGridItem *labs_grid_item_new_app(GAppInfo *app);
LabsGridItem *labs_grid_item_new_folder(const gchar *id, const gchar *name,
                                          const gchar *icon_name);
LabsGridItem *labs_grid_item_new_back(void);

/* Free functions */
void labs_grid_item_free(LabsGridItem *item);

/* Main logic to parse apps and group them */
GList *labs_launcher_parse_and_group_apps(void);

#endif /* __labs_launcher_CATEGORIES_H__ */
