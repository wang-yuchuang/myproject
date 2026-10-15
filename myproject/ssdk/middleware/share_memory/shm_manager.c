#include <CLI_type.h>
#include <cmsis_os2.h>
#include <errcode.h>
#include <errno.h>
#include <lib/list.h>
#include <sd_errcode.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <share_memory.h>
#include <shm_service.h>

typedef struct {
    struct list_node list;
    uint64_t addr;
    uint32_t size;
    char name[SHM_NAME_MAXLEN + 1];
    uint32_t refcnt;
    int own;
} shm_mem_t;

typedef struct {
    osMutexId_t mlock;
    struct list_node mlist;
    const shm_ops_t *ops;
    int ready;
    shm_handle_t handle;
} shm_manager_t;

static shm_manager_t shm_manager = {
    .ready = 0,
};

int shm_alloc(uint32_t align, uint32_t size, uint16_t flags, uint16_t mode,
              const char *name, uint64_t *addr) {
    shm_manager_t *shmm = &shm_manager;
    shm_mem_t *mem;
    int ret = -ENOMEM;

    if (!shmm->ready) {
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_SHMEM, 0, SHM_NOT_READY, 0);
        return -ENODEV;
    }

    if (!size)
        return -EINVAL;

    if (!addr)
        return -EINVAL;

    osMutexAcquire(shmm->mlock, osWaitForever);

    /* check name conflict */
    if (name && strlen(name)) {
        list_for_every_entry(&shmm->mlist, mem, shm_mem_t, list) {
            if (!strncmp(name, mem->name, SHM_NAME_MAXLEN)) {
                ret = -EINVAL;
                goto release;
            }
        }
    }

    mem = malloc(sizeof(shm_mem_t));
    if (!mem) {
        ret = -ENOMEM;
        goto release;
    }

    /* do allocation */
    ret = shmm->ops->alloc_mem(shmm->handle, align, size, flags, mode, name,
                               addr);
    if (ret) {
        free(mem);
        goto release;
    }

    /* add to local list */
    list_initialize(&mem->list);
    mem->addr = *addr;
    mem->size = size;
    if (name) {
        strncpy(mem->name, name, SHM_NAME_MAXLEN);
        mem->name[SHM_NAME_MAXLEN] = 0;
    } else {
        mem->name[0] = 0;
    }
    mem->refcnt = 1;
    mem->own = 1;
    list_add_tail(&shmm->mlist, &mem->list);

release:
    osMutexRelease(shmm->mlock);

    if (ret)
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_SHMEM, 0, SHM_ALLOC_FAIL, size);

    return ret;
}

void shm_free(uint64_t addr) {
    shm_manager_t *shmm = &shm_manager;
    shm_mem_t *mem;
    int found = 0;

    if (!shmm->ready) {
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_SHMEM, 0, SHM_NOT_READY, 0);
        return;
    }

    osMutexAcquire(shmm->mlock, osWaitForever);
    list_for_every_entry(&shmm->mlist, mem, shm_mem_t, list) {
        if (addr == mem->addr) {
            found = 1;
            break;
        }
    }

    if (!found)
        goto release;

    /* not allocated by us */
    if (!mem->own)
        goto release;

    mem->refcnt--;

    /* check if someone is still using it */
    if (mem->refcnt)
        goto release;

    /* free and remove from list */
    shmm->ops->free_mem(shmm->handle, addr);
    list_delete(&mem->list);
    free(mem);

release:
    osMutexRelease(shmm->mlock);
}

int shm_find(const char *name, uint64_t *addr) {
    shm_manager_t *shmm = &shm_manager;
    shm_mem_t *mem;
    int ret = -ENOMEM;

    if (!shmm->ready) {
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_SHMEM, 0, SHM_NOT_READY, 0);
        return -ENODEV;
    }

    if (!name)
        return -EINVAL;

    if (!addr)
        return -EINVAL;

    osMutexAcquire(shmm->mlock, osWaitForever);

    /* search local list */
    list_for_every_entry(&shmm->mlist, mem, shm_mem_t, list) {
        if (!strcmp(name, mem->name)) {
            /* find it */
            *addr = mem->addr;
            ret = 0;
            goto release;
        }
    }

    ret = shmm->ops->find_mem(shmm->handle, name, addr);

release:
    osMutexRelease(shmm->mlock);

    if (ret)
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_SHMEM, 0, SHM_FIND_FAIL, 0);

    return ret;
}

int shm_lock(uint64_t addr) {
    shm_manager_t *shmm = &shm_manager;
    shm_mem_t *mem;
    int ret = -ENOMEM;

    if (!shmm->ready) {
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_SHMEM, 0, SHM_NOT_READY, 0);
        return -ENODEV;
    }

    osMutexAcquire(shmm->mlock, osWaitForever);

    /* search local list */
    list_for_every_entry(&shmm->mlist, mem, shm_mem_t, list) {
        if (addr == mem->addr) {
            mem->refcnt++;
            ret = 0;
            goto release;
        }
    }

    mem = malloc(sizeof(shm_mem_t));
    if (!mem) {
        ret = -ENOMEM;
        goto release;
    }

    /* try to access */
    ret = shmm->ops->begin_access(shmm->handle, addr, &mem->size, mem->name);
    if (ret) {
        free(mem);
        goto release;
    }

    /* add to local list */
    list_initialize(&mem->list);
    mem->addr = addr;
    mem->refcnt = 1;
    mem->own = 0;
    list_add_tail(&shmm->mlist, &mem->list);

release:
    osMutexRelease(shmm->mlock);

    if (ret)
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_SHMEM, 0, SHM_LOCK_FAIL, 0);

    return ret;
}

void shm_unlock(uint64_t addr) {
    shm_manager_t *shmm = &shm_manager;
    shm_mem_t *mem;
    int found = 0;

    if (!shmm->ready) {
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_SHMEM, 0, SHM_NOT_READY, 0);
        return;
    }

    osMutexAcquire(shmm->mlock, osWaitForever);
    list_for_every_entry(&shmm->mlist, mem, shm_mem_t, list) {
        if (addr == mem->addr) {
            found = 1;
            break;
        }
    }

    if (!found)
        goto release;

    mem->refcnt--;

    if (mem->refcnt)
        goto release;

    /* if we allocated it, we should de-allocate it now */
    if (mem->own)
        shmm->ops->free_mem(shmm->handle, addr);
    else
        shmm->ops->end_access(shmm->handle, addr);
    list_delete(&mem->list);
    free(mem);

release:
    osMutexRelease(shmm->mlock);
}

void shm_init(void) {
    shm_manager_t *shmm = &shm_manager;

    shmm->mlock = osMutexNew(NULL);
    list_initialize(&shmm->mlist);

    shmm->ops = shm_get_ops();
    shmm->ready = !shmm->ops->init(&shmm->handle);

    if (!shmm->ready)
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_SHMEM, 0, SHM_INIT_FAIL, 0);
}

#if defined(__SHARE_MEMORY_DEBUG)
static int shm_usage(void) {
    printf("Usage: shm CMD OPTIONS\n");
    printf("\n");
    printf("Available CMDs:\n");
    printf(" - list\n");
    printf("        list all share memory\n");
    printf(" - alloc <align> <size> [name]\n");
    printf("        allocate share memory, start addr aligned\n");
    printf(" - free <addr>\n");
    printf("        free share memory at given addr\n");
    printf(" - lock <addr>\n");
    printf("        lock share memory at given addr\n");
    printf(" - unlock <addr>\n");
    printf("        unlock share memory at given addr\n");
    printf(" - find <name>\n");
    printf("        find share memory with name\n");

    return 0;
}

static int shm_debug(int argc, char *argv[]) {
    shm_manager_t *shmm = &shm_manager;
    shm_mem_t *mem;
    char *_unused;

    /* someone's already removed argv[0] for us */
    argc += 1;
    argv -= 1;

    if (!shmm->ready) {
        printf("share memory manager not ready\n");
        return 0;
    }

    if (argc < 2)
        return shm_usage();

    if (!strncmp(argv[1], "list", 4)) {
#define __unit_cnt 5
        static const char *__units[__unit_cnt] = {"B", "KB", "MB", "GB", "TB"};
        char size_buf[10];
        uint32_t size;

        printf("======= local memory list =======\n");
        printf("ADDR\t\t\tSIZE\t\t\tNAME\t\tREFCNT\tOWN\n");
        osMutexAcquire(shmm->mlock, osWaitForever);
        list_for_every_entry(&shmm->mlist, mem, shm_mem_t, list) {
            size = mem->size;
            for (int i = 0; i < __unit_cnt; i++) {
                if (size < 1024u) {
                    snprintf(size_buf, sizeof(size_buf), "%u%s", size,
                             __units[i]);
                    break;
                }

                size /= 1024u;
            }

            printf("0x%016llx\t0x%08x (%s)\t%-16s%-8u%d\n", mem->addr,
                   mem->size, size_buf, mem->name, mem->refcnt, mem->own);
        }
        osMutexRelease(shmm->mlock);
        printf("\n");
#undef __unit_cnt

        /* let's see if service could dump something */
        if (shmm->ops->dump)
            shmm->ops->dump(shmm->handle);

    } else if (!strncmp(argv[1], "alloc", 5)) {
        if (argc < 4) {
            printf("Too few arguments\n");
            return shm_usage();
        }

        uint64_t addr;
        uint32_t size = strtoul(argv[3], &_unused, 0);
        int ret = shm_alloc(strtoul(argv[2], &_unused, 0), size, 0, 0,
                            argc > 4 ? argv[4] : NULL, &addr);
        if (ret < 0)
            printf("Fail to alloc %u bytes, ret %d\n", size, ret);
        else
            printf("Allocated %u bytes at 0x%016llx\n", size, addr);
    } else if (!strncmp(argv[1], "free", 4)) {
        if (argc < 3) {
            printf("Too few arguments\n");
            return shm_usage();
        }

        shm_free(strtoul(argv[2], &_unused, 0));
    } else if (!strncmp(argv[1], "lock", 4)) {
        if (argc < 3) {
            printf("Too few arguments\n");
            return shm_usage();
        }

        uint64_t addr = strtoul(argv[2], &_unused, 0);
        int ret = shm_lock(addr);
        if (ret < 0)
            printf("Fail to lock 0x%016llx, ret %d\n", addr, ret);
        else
            printf("Share memory at 0x%016llx locked\n", addr);
    } else if (!strncmp(argv[1], "unlock", 6)) {
        if (argc < 3) {
            printf("Too few arguments\n");
            return shm_usage();
        }

        shm_unlock(strtoul(argv[2], &_unused, 0));
    } else if (!strncmp(argv[1], "find", 4)) {
        if (argc < 3) {
            printf("Too few arguments\n");
            return shm_usage();
        }

        uint64_t addr;
        int ret = shm_find(argv[2], &addr);
        if (ret < 0)
            printf("Fail to find '%s', ret %d\n", argv[2], ret);
        else
            printf("Share memory '%s' is at 0x%016llx\n", argv[2], addr);
    } else {
        printf("Unknown CMD '%s'\n", argv[1]);
        return shm_usage();
    }

    return 0;
}

CLI_CMD("shm", "share memory debug commands", shm_debug);
#endif
