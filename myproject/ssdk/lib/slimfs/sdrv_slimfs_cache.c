/**
 * @file sdrv_slimfs_cache.c
 *
 * Copyright (c) 2023 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: slimfs item name and item Id pair cache implement file
 *
 * Revision History:
 * -----------------
 */
/* =========================== include file session ======================== */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "sdrv_slimfs_log.h"
#include "sdrv_slimfs_cache.h"
#include "common_utility.h"
/* =========================== marco define session ======================== */
/* =========================== enumeration define session ================== */
/* =========================== structure define session ==================== */
/* =========================== function declare session ==================== */
/* =========================== global value define session ================= */
/* =========================== static value define session ================= */
/* =========================== function implementation session ============= */
struct cache_pairt {
    uint32_t id;
    uint8_t name[CONFIG_SLIMFS_ITEM_NAME_LEN];
};
void dump_cache_ctx(void* cacheCtx)
{
    struct cache_pairt* pair = (struct cache_pairt *)cacheCtx;
    printf("  --id: 0x%x, name: %s\n", pair->id, pair->name);

}
void slimfs_cache_dump(slimfs_cache_hdl hdl)
{
    if (hdl == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "invalid parameter for %s.\n", __func__);
        return;
    }
    slimfs_cache_t* cache_hdl = (slimfs_cache_t *)hdl;

    if (SLIMFS_LOG_LEV >= SLIMFS_LOG_DEBUG) {
        printf("list queue info:\n");
        printf("cache number: %d\n", cache_hdl->used_num);
        printf("rear        : %d\n", cache_hdl->rear);
        printf("front       : %d\n", cache_hdl->front);
        for (int32_t i = 0; i < CONFIG_SLIMFS_CACHE_NUM; i++) {
            printf("cache node inde: %d\n", cache_hdl->cache_node[i].index);
            printf("  --pre : %d\n", cache_hdl->cache_node[i].pre);
            printf("  --next: %d\n", cache_hdl->cache_node[i].next);
        }
    }

    printf("=======================\n");
    if (cache_hdl->rear != -1) {
        slimfs_cache_node_t* node = &cache_hdl->cache_node[cache_hdl->rear];
        while (node->pre != -1) {
            printf("cache node inde: %d\n", node->index);
            printf("  --pre : %d\n", node->pre);
            printf("  --next: %d\n", node->next);
            dump_cache_ctx(node->cache_ctx);
            node = &cache_hdl->cache_node[node->pre];
        }

        printf("cache node inde: %d\n", node->index);
        printf("  --pre : %d\n", node->pre);
        printf("  --next: %d\n", node->next);
        dump_cache_ctx(node->cache_ctx);
    }
    printf("=======================\n");
}

static slimfs_cache_node_t* list_queue_search(slimfs_cache_hdl hdl, const void* matchCtx)
{
    bool find = false;
    slimfs_cache_t* cache_hdl = (slimfs_cache_t *)hdl;

    SLIMFS_PRINT(SLIMFS_LOG_DEBUG, "entry %s\n", __func__);
    if (SLIMFS_LOG_LEV >= SLIMFS_LOG_DEBUG) {
        slimfs_cache_dump(hdl);
    }

    if (cache_hdl->used_num == 0) {
        // cache node all invalid
        return NULL;
    }

    slimfs_cache_node_t* node = &cache_hdl->cache_node[cache_hdl->rear];
    while (node->pre != -1) {
        if (cache_hdl->cb) {
            if (cache_hdl->cb(matchCtx, node->cache_ctx)) {
                find = true;
                break;
            }
        }
        node = &cache_hdl->cache_node[node->pre];
    }

    if (find) {
        return node;
    }

    // match the front node
    if (cache_hdl->cb &&
        cache_hdl->cb(matchCtx, node->cache_ctx)) {
        return node;
    }

    return NULL;
}

static int32_t list_queue_add_rear(slimfs_cache_hdl hdl, slimfs_cache_node_t* node)
{
    if (hdl == NULL || node == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "invalid parameter for %s.\n", __func__);
        return -1;
    }
    SLIMFS_PRINT(SLIMFS_LOG_DEBUG, "entry %s\n", __func__);
    if (SLIMFS_LOG_LEV >= SLIMFS_LOG_DEBUG) {
        slimfs_cache_dump(hdl);
    }

    slimfs_cache_t* cache_hdl = (slimfs_cache_t *)hdl;
    if (cache_hdl->rear == -1 && cache_hdl->front == -1) {
        // the first node, front & rear node is the one node
        cache_hdl->rear = cache_hdl->front = node->index;
        cache_hdl->used_num++;
        return 0;
    }

    cache_hdl->cache_node[cache_hdl->rear].next = node->index;
    node->pre = cache_hdl->cache_node[cache_hdl->rear].index;
    cache_hdl->rear = node->index;
    cache_hdl->used_num++;

    SLIMFS_PRINT(SLIMFS_LOG_DEBUG, "exit %s\n", __func__);
    if (SLIMFS_LOG_LEV >= SLIMFS_LOG_DEBUG) {
        slimfs_cache_dump(hdl);
    }
    return 0;
}

static slimfs_cache_node_t* list_queue_get_empty_node(slimfs_cache_hdl hdl)
{
    if (hdl == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "invalid parameter for %s.\n", __func__);
        return NULL;
    }

    SLIMFS_PRINT(SLIMFS_LOG_DEBUG, "entry %s\n", __func__);

    slimfs_cache_t* cache_hdl = (slimfs_cache_t *)hdl;
    slimfs_cache_node_t* node = NULL;

    for (int32_t i = 0; i < CONFIG_SLIMFS_CACHE_NUM; i++) {
        node = &cache_hdl->cache_node[i];
        if (node->next == -1 &&
            node->pre == -1 &&
            node->index != cache_hdl->front &&
            node->index != cache_hdl->rear) {
            SLIMFS_PRINT(SLIMFS_LOG_DEBUG, "get node index: %d for %s.\n",node->index, __func__);
            return node;
        }
    }

    return NULL;
}

static slimfs_cache_node_t* list_queue_get_oldest_node(slimfs_cache_hdl hdl)
{
    if (hdl == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "invalid parameter for %s.\n", __func__);
        return NULL;
    }
    slimfs_cache_t* cache_hdl = (slimfs_cache_t *)hdl;

    SLIMFS_PRINT(SLIMFS_LOG_DEBUG, "entry %s\n", __func__);

    if (cache_hdl->front >= CONFIG_SLIMFS_CACHE_NUM || cache_hdl->front < 0) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "front value is invalid: %d, cache size: %d\n"
                    , cache_hdl->front
                    , CONFIG_SLIMFS_CACHE_NUM);
        return NULL;
    }
    slimfs_cache_node_t* node = &cache_hdl->cache_node[cache_hdl->front];

    SLIMFS_PRINT(SLIMFS_LOG_DEBUG, "get node index: %d for %s.\n", node->index, __func__);
    return node;
}

static int32_t list_queue_remove(slimfs_cache_hdl hdl, slimfs_cache_node_t* node)
{
    if (hdl == NULL || node == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "invalid parameter for %s.\n", __func__);
        return -1;
    }

    SLIMFS_PRINT(SLIMFS_LOG_DEBUG, "entry %s\n", __func__);

    if (SLIMFS_LOG_LEV >= SLIMFS_LOG_DEBUG) {
        slimfs_cache_dump(hdl);
    }

    slimfs_cache_t* cache_hdl = (slimfs_cache_t *)hdl;
    if (cache_hdl->used_num == 0) {
        SLIMFS_PRINT(SLIMFS_LOG_WARN, "current cache line is empty.\n");
        return 0;
    }


    if (node->next == -1 || node->pre == -1) {
        if (node->next == -1 && node->pre == -1) {
            SLIMFS_PRINT(SLIMFS_LOG_WARN, "current node: %d is the last in queue.\n"
                         , node->index);
            cache_hdl->front = cache_hdl->rear = -1;
        }
        else if (node->next == -1) {
            // current node is rear
            cache_hdl->cache_node[node->pre].next = -1;
            cache_hdl->rear = cache_hdl->cache_node[node->pre].index;
        }
        else if (node->pre == -1) {
            // current node is front
            cache_hdl->cache_node[node->next].pre = -1;
            cache_hdl->front = cache_hdl->cache_node[node->next].index;
        }
    }
    else {
        cache_hdl->cache_node[node->next].pre = cache_hdl->cache_node[node->pre].index;
        cache_hdl->cache_node[node->pre].next = cache_hdl->cache_node[node->next].index;
     }

    node->next = node->pre = -1;
    cache_hdl->used_num--;

    SLIMFS_PRINT(SLIMFS_LOG_DEBUG, "exit %s\n", __func__);
    if (SLIMFS_LOG_LEV >= SLIMFS_LOG_DEBUG) {
        slimfs_cache_dump(hdl);
    }

    return 0;
}

/******************************************************************************
**@brief slimfs cache init
******************************************************************************/
slimfs_cache_hdl slimfs_cache_init(int32_t ctxSize, matching_fun_t cb)
{
    slimfs_cache_t* cache_hdl = malloc(sizeof(slimfs_cache_t));
    if (cache_hdl == NULL) {
        return NULL;
    }

    memset(cache_hdl, 0x00, sizeof(slimfs_cache_t));
    for (int32_t i = 0; i < CONFIG_SLIMFS_CACHE_NUM; i++) {
        cache_hdl->cache_node[i].index = i;
        cache_hdl->cache_node[i].pre = -1;
        cache_hdl->cache_node[i].next = -1;
        cache_hdl->cache_node[i].cache_ctx = malloc(ctxSize);
        if (cache_hdl->cache_node[i].cache_ctx == NULL) {
            goto err;
        }
    }

    cache_hdl->cb = cb;
    cache_hdl->cache_ctx_size = ctxSize;
    cache_hdl->front = cache_hdl->rear = -1;
    cache_hdl->used_num = 0;

    return cache_hdl;

err:
    slimfs_cache_deinit(cache_hdl);
    return NULL;
}

/******************************************************************************
**@brief slimfs cache deinit
******************************************************************************/
void slimfs_cache_deinit(slimfs_cache_hdl hdl)
{
    if (hdl == NULL) {
        return;
    }

    slimfs_cache_t* cache_hdl = (slimfs_cache_t *)hdl;
    for (int32_t i = 0; i < CONFIG_SLIMFS_CACHE_NUM; i++) {
        if (cache_hdl->cache_node[i].cache_ctx) {
            free(cache_hdl->cache_node[i].cache_ctx);
        }
    }
    free(cache_hdl);

    return;
}

/******************************************************************************
**@brief slimfs cache add context
******************************************************************************/
int32_t slimfs_cache_add(slimfs_cache_hdl hdl, const void* matchCtx, void* cacheCtx)
{
    if (hdl == NULL || matchCtx == NULL || cacheCtx == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "invalid parameter for %s.\n", __func__);
        return -1;
    }
    slimfs_cache_t* cache_hdl = (slimfs_cache_t *)hdl;

    slimfs_cache_node_t* node = NULL;
    if (list_queue_search(hdl, matchCtx) != NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "cache line match current matchCtx.\n");
        return -1;
    }

    // get empty cache node or oldest cache node
    if ((node = list_queue_get_empty_node(hdl)) == NULL) {
        // no empty cache node, get oldest cache node
        node = list_queue_get_oldest_node(hdl);
        list_queue_remove(hdl, node);
    }

    if (node == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "get empty or oldest node failed for %s.\n", __func__);
        return -1;
    }

    // copy cache contex to node
    memcpy(node->cache_ctx, cacheCtx, cache_hdl->cache_ctx_size);

    // list queue add
    return list_queue_add_rear(hdl, node);
}

/******************************************************************************
**@brief slimfs cache remove
******************************************************************************/
int32_t slimfs_cache_remove(slimfs_cache_hdl hdl, const void* matchCtx)
{
    if (hdl == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "invalid parameter for %s.\n", __func__);
        return -1;
    }

    slimfs_cache_node_t* node = NULL;

    // serach the cache node
    node = list_queue_search(hdl, matchCtx);
    if (node == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_WARN, "serach cache failed for %s\n", __func__);
        return -1;
    }

    return list_queue_remove(hdl, node);
}

/******************************************************************************
**@brief slimfs cache match
******************************************************************************/
int32_t slimfs_cache_match(slimfs_cache_hdl hdl, const void* matchCtx, void* cacheCtx)
{
    if (hdl == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_ERR, "invalid parameter for %s.\n", __func__);
        return -1;
    }

    slimfs_cache_t* cache_hdl = (slimfs_cache_t *)hdl;
    slimfs_cache_node_t* node = NULL;

    // search cache node
    node = list_queue_search(hdl, matchCtx);
    if (node == NULL) {
        SLIMFS_PRINT(SLIMFS_LOG_DEBUG, "serach cache failed for %s\n", __func__);
        return -1;
    }
    memcpy(cacheCtx, node->cache_ctx, cache_hdl->cache_ctx_size);

    if (cache_hdl->used_num > 1) {
        // adjust current node pri
        list_queue_remove(hdl, node);
        list_queue_add_rear(hdl, node);
    }

    return 0;
}

#if 0
/******************************************************************************
**@brief test slimfs cache function code
******************************************************************************/
struct cache_pair {
    uint32_t id;
    uint8_t name[16];
};

bool match_cache(const void* matchCtx, void* cacheCtx)
{
    if (cacheCtx == NULL ||
        matchCtx == NULL) {
        return false;
    }
    struct cache_pair* cpair = (struct cache_pair*)cacheCtx;
    char* name = (char *)matchCtx;

    if (strcmp((char *)cpair->name, name) == 0) {
        return true;
    }
    else {
        return false;
    }
}

int32_t args_parser(slimfs_cache_hdl hdl, int32_t argc, char* argv[])
{
    int32_t ret = 0;

    if (strcmp(argv[0], "add") == 0) {
        char *name = argv[1];
        int32_t id = strtodigit(argv[2], NULL, 0);

        struct cache_pair pair;
        strcpy((char *)pair.name, name);
        pair.id = id;

        ret = slimfs_cache_add(hdl, name, &pair);
        if (ret == 0) {
            printf("add name: %s, id: 0x%x\n", name, id);
        }
        else {
            printf("add failed\n");
        }
        slimfs_cache_dump(hdl);
    }
    else if (strcmp(argv[0], "rm") == 0) {
        char *name = argv[1];
        ret = slimfs_cache_remove(hdl, name);
        if (ret == 0) {
            printf("remove name: %s\n", name);
        }
        else {
            printf("remove failed\n");
        }
        slimfs_cache_dump(hdl);
    }
    else if (strcmp(argv[0], "match") == 0) {
        char *name = argv[1];
        struct cache_pair pair = {
            .id = 0x0,
        };
        ret = slimfs_cache_match(hdl, name, &pair);
        if (ret == 0) {
            printf("match name: %s, match id: 0x%x\n", pair.name, pair.id);
        }
        else {
            printf("match failed\n");
        }
        slimfs_cache_dump(hdl);
    }
    else if (strcmp(argv[0], "exit") == 0) {
        return -1;
    }
    else {
        printf("invalid cmd\n");
    }

    printf("\n");
    return 0;
}

int32_t main(int32_t argc, const char* argv[])
{
    slimfs_cache_hdl hdl = NULL;
    hdl = slimfs_cache_init(sizeof(struct cache_pair), match_cache);
    if (hdl == NULL) {
        printf("create cache handle failed.\n");
        return -1;
    }
    char buf[64] = "";
    int32_t ret = 0;
    char*   args[10];
    int32_t argn = 0;

    while(1) {
        printf("#input cmd:");
        ret = get_line(buf, 64);
        if (ret != 0) {
            continue;
        }
        printf("%s\n", buf);

        argn = split_str(buf, ' ', args);
        ret = args_parser(hdl, argn, args);
        if (ret != 0) {
            break;
        }
    }

    slimfs_cache_deinit(hdl);
    return 0;
}
#endif /* end #if 0 */

/* end of file sdrv_slimfs_cache.c */