#include "inventory.h"
#include <stdio.h>
#include <string.h>

void inventory_init(Inventory *inv) {
    if (!inv) return;
    memset(inv->items, 0, sizeof(inv->items));
    inv->count = 0;
    inv->gold = 50;  // 初始金币
}

Item* inventory_add(Inventory *inv, const char *id, const char *name, 
                    const char *desc, int value, int quantity, bool quest_item) {
    if (!inv || inv->count >= MAX_ITEMS) return NULL;

    Item *item = &inv->items[inv->count++];
    strncpy(item->id, id, MAX_NAME_LEN - 1);
    strncpy(item->name, name, MAX_NAME_LEN - 1);
    strncpy(item->description, desc, MAX_DESC_LEN - 1);
    item->value = value;
    item->quantity = quantity;
    item->quest_item = quest_item;

    return item;
}

bool inventory_remove(Inventory *inv, const char *id, int quantity) {
    if (!inv) return false;

    for (int i = 0; i < inv->count; i++) {
        if (strcmp(inv->items[i].id, id) == 0) {
            if (inv->items[i].quantity >= quantity) {
                inv->items[i].quantity -= quantity;
                if (inv->items[i].quantity <= 0) {
                    // 移除物品
                    for (int j = i; j < inv->count - 1; j++) {
                        inv->items[j] = inv->items[j + 1];
                    }
                    inv->count--;
                }
                return true;
            }
            return false;
        }
    }
    return false;
}

Item* inventory_find(Inventory *inv, const char *id) {
    if (!inv || !id) return NULL;

    for (int i = 0; i < inv->count; i++) {
        if (strcmp(inv->items[i].id, id) == 0) {
            return &inv->items[i];
        }
    }
    return NULL;
}

void inventory_display(Inventory *inv) {
    if (!inv) return;

    printf("\n========== 背包 ==========\n");
    printf("金币：%d\n", inv->gold);
    printf("物品 (%d/%d):\n", inv->count, MAX_ITEMS);
    printf("--------------------------\n");

    if (inv->count == 0) {
        printf("  背包是空的\n");
    } else {
        for (int i = 0; i < inv->count; i++) {
            printf("  [%d] %s x%d\n", i + 1, inv->items[i].name, inv->items[i].quantity);
            if (inv->items[i].quest_item) {
                printf("      (任务物品)\n");
            }
        }
    }
    printf("==========================\n\n");
}

bool inventory_has_item(Inventory *inv, const char *id) {
    return inventory_find(inv, id) != NULL;
}

int inventory_count_item(Inventory *inv, const char *id) {
    Item *item = inventory_find(inv, id);
    return item ? item->quantity : 0;
}
