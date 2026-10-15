/**
 * @file disk_cache_noop.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <param.h>
#include <types.h>

#include <lib/disk/disk.h>
#include <lib/disk/disk_cache.h>
#include <lib/disk/disk_cache_noop.h>

struct cache_noop_node {
    struct list_node addr_node;
    struct list_node create_node;
    struct list_node write_node;

    /* data info */
    uint64_t cache_addr;
    uint32_t cache_size;
    uint8_t *cache_buff;

    /* node attribute */
    uint32_t flags;
};

/* 基于地址匹配数据节点 */
struct cache_noop_node *disk_node_match_addr(struct list_node *list,
        uint32_t addr)
{
    struct cache_noop_node *node = NULL;

    /* Iterate over the address list */
    list_for_every_entry(list, node,
                         struct cache_noop_node, addr_node) {
    }
    /* NULL node */
    return NULL;
}

/* 申请数据节点,读取数据 */
struct cache_noop_node *disk_node_malloc(struct disk_cache_noop *cache_noop,
        uint32_t addr, uint32_t size)
{
    struct cache_noop_node *node = NULL;

    /* 地址块大小对齐 */
    /* 大小块大小对齐 */

    /* 判断缓冲区大小 */

    /* 申请ndoe */

    /* 申请buff */

    /* 读取数据 */
    disk_cache_dev_read(&cache_noop->cache, node->cache_buff, node->cache_addr,
                        node->cache_size);

    /* 插入addr链表*/

    /* 插入create链表 */

    /* 配置cache缓存大小 */
    return node;
}

/* 释放数据节点 */
struct cache_noop_node *disk_node_free(struct disk_cache_noop *cache_noop,
                                       struct cache_noop_node *node)
{
    /* 配置cache缓存大小 */

    /* 释放buff */

    /* 链表addr & create节点删除 */

    /* 释放node */
    return node;
}

/* 同步节点数据,写入数据 */
int disk_node_sync(struct disk_cache_noop *cache_noop,
                   struct cache_noop_node *node)
{
    /* 实际写入数据 */
    disk_cache_dev_write(&cache_noop->cache, node->cache_buff, node->cache_addr,
                         node->cache_size);

    /* 删除write链表节点 */

    /* 取消CACHE_NODE_WRITE */
    return 0;
}

int disk_node_read(struct cache_noop_node *node, uint8_t *dst, uint32_t addr,
                   uint32_t size)
{
    int rlen = 0;
    /* 基于节点信息,读取数据到缓冲区 */
    return rlen;
}

int disk_node_write(struct cache_noop_node *node, uint8_t *src, uint32_t addr,
                    uint32_t size)
{
    int rlen = 0;
    /* 基于节点信息，写入数据到缓冲区 */
    /* 加入write list */
    return rlen;
}

int disk_node_erase(struct cache_noop_node *node, uint8_t *src, uint32_t addr,
                    uint32_t size)
{
    int rlen = 0;
    /* 基于节点信息，写入0x0 | 0xff到缓冲区 */
    return rlen;
}

int disk_cache_noop_read(struct disk_cache *cache, uint8_t *dst,
                         disk_addr_t addr, disk_size_t size)
{
    /* 基于地址查找节点 */
    /* 申请&读取节点 */
    return 0;
}

int disk_cache_noop_write(struct disk_cache *cache, const uint8_t *src,
                          disk_addr_t addr, disk_size_t size)
{
    /* 基于地址查找节点 */
    /* 申请&写入节点 */
    return 0;
}

int disk_cache_noop_erase(struct disk_cache *cache, disk_addr_t addr,
                          disk_size_t size, uint32_t flag)
{
    /* 基于地址查找节点 */
    /* 申请&写入节点 */
    return 0;
}

int disk_cache_noop_sync(struct disk_cache *cache)
{
    /* 遍历节点,sync节点 */
    return 0;

}

int disk_cache_noop_sync_addr(struct disk_cache *cache, disk_addr_t addr,
                              disk_size_t size)
{
    /* 基于地址查找节点 */
    /* sync节点 */
    return 0;
}

/* TODO */
struct disk_cache_operations cache_noop_ops = {
    .disk_cache_read = disk_cache_noop_read,
    .disk_cache_write = disk_cache_noop_write,
    .disk_cache_erase = disk_cache_noop_erase,
    .disk_cache_sync = disk_cache_noop_sync,
    .disk_cache_sync_addr = disk_cache_noop_sync_addr,
};

int register_disk_cache_noop(struct disk_cache_noop *cache_noop)
{
    struct disk_cache *cache = &cache_noop->cache;
    cache->disk_cache_ops = &cache_noop_ops;
    cache->private_data = cache_noop;
    return 0;
}

int unregister_disk_cache_noop(struct disk_cache_noop *cache_noop)
{
    return 0;
}
