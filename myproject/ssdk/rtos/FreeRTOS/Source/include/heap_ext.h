/**
 * heap_ext.h - External heap management functions
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Custom heap functions for DDR external memory allocation.
 */

#ifndef HEAP_EXT_H
#define HEAP_EXT_H

#include <stdint.h>
#include <stddef.h>

/**
 * Initialize the external heap with specified address and size.
 *
 * @param address - Base address of the heap memory region
 * @param size - Size of the heap memory region in bytes
 */
void ExtHeapInit(uint32_t address, uint32_t size);

/**
 * Allocate memory from external heap with alignment.
 *
 * @param xWantedSize - Size of memory to allocate
 * @param xBoundary - Alignment boundary (e.g., 32 for 32-byte alignment)
 * @return Pointer to allocated memory, or NULL if allocation failed
 */
void *ExtMallocAligned(size_t xWantedSize, size_t xBoundary);

/**
 * Allocate memory from external heap.
 *
 * @param xWantedSize - Size of memory to allocate
 * @return Pointer to allocated memory, or NULL if allocation failed
 */
void *ExtMalloc(size_t xWantedSize);

/**
 * Free memory allocated from external heap.
 *
 * @param pv - Pointer to memory to free
 */
void ExtFree(void *pv);

/**
 * Get the amount of free memory remaining in external heap.
 *
 * @return Number of free bytes
 */
size_t ExtxPortGetFreeHeapSize(void);

/**
 * Get the minimum amount of free memory that has ever existed.
 *
 * @return Minimum number of free bytes that has existed
 */
size_t ExtxPortGetMinimumEverFreeHeapSize(void);

#endif /* HEAP_EXT_H */