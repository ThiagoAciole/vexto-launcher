#ifndef __vexto_launcher_CATEGORIES_H__
#define __vexto_launcher_CATEGORIES_H__

#include "../vexto-launcher.h"

/* Creation functions */
VextoGridItem *vexto_grid_item_new_app(GAppInfo *app);
VextoGridItem *vexto_grid_item_new_folder(const gchar *id, const gchar *name,
                                          const gchar *icon_name);
VextoGridItem *vexto_grid_item_new_back(void);

/* Free functions */
void vexto_grid_item_free(VextoGridItem *item);

/* Main logic to parse apps and group them */
GList *vexto_launcher_parse_and_group_apps(void);

#endif /* __vexto_launcher_CATEGORIES_H__ */
