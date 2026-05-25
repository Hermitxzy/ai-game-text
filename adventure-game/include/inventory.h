#ifndef INVENTORY_H
#define INVENTORY_H

#include "types.h"

void inventory_init(Inventory *inv);
Item* inventory_add(Inventory *inv, const char *id, const char *name, 
                    const char *desc, int value, int quantity, bool quest_item);
bool inventory_remove(Inventory *inv, const char *id, int quantity);
Item* inventory_find(Inventory *inv, const char *id);
void inventory_display(Inventory *inv);
bool inventory_has_item(Inventory *inv, const char *id);
int inventory_count_item(Inventory *inv, const char *id);

#endif
