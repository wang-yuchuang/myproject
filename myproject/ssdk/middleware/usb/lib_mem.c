/**
 * @file lib_mem.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Memory Library file for USB Host.
 *
 * Revision History:
 * -----------------
 */


#include  "lib_mem.h"
#include  "lib_math.h"


/*
*********************************************************************************************************
*                                          Mem_PoolCreate()
*
* Description : (1) Creates a memory pool :
*
*                   (a) Create    memory pool from heap or dedicated memory
*                   (b) Allocate  memory pool memory blocks
*                   (c) Configure memory pool
*
*
* Argument(s) : p_pool          Pointer to a memory pool structure to create (see Note #1).
*
*               p_mem_base      Memory pool segment base address :
*
*                                       (a)     Null address    Memory pool allocated from general-purpose heap.
*                                       (b) Non-null address    Memory pool allocated from dedicated memory
*                                                                   specified by its base address.
*
*               mem_size        Size      of memory pool segment          (in bytes).
*
*               blk_nbr         Number    of memory pool blocks to create.
*
*               blk_size        Size      of memory pool blocks to create (in bytes).
*
*               blk_align       Alignment of memory pool blocks to specific word boundary (in bytes).
*
*               p_bytes_reqd    Optional pointer to a variable to ... :
*
*                                   (a) Return the number of bytes required to successfully
*                                               allocate the memory pool, if any error(s);
*                                   (b) Return 0, otherwise.
*
*               p_err           Pointer to variable that will receive the return error code from this function :
*
*                                   LIB_MEM_ERR_NONE                    Operation was successful.
*                                   LIB_MEM_ERR_NULL_PTR                Pointer to memory pool is null.
*                                   LIB_MEM_ERR_INVALID_BLK_ALIGN       Invalid block alignment requested.
*                                   LIB_MEM_ERR_INVALID_BLK_NBR         Invalid number of blocks specified.
*                                   LIB_MEM_ERR_INVALID_BLK_SIZE        Invalid block size specified.
*                                   LIB_MEM_ERR_INVALID_SEG_SIZE        Invalid segment size.
*                                   LIB_MEM_ERR_HEAP_EMPTY              No more memory available on heap.
*                                   LIB_MEM_ERR_ADDR_OVF                Memory allocation exceeds address space.
*
*                                   ---------------RETURNED BY Mem_SegOverlapChkCritical()----------------
*                                   LIB_MEM_ERR_INVALID_SEG_EXISTS      Segment already exists.
*                                   LIB_MEM_ERR_INVALID_SEG_OVERLAP     Segment overlaps another existing segment.
*
*                                   -----------------RETURNED BY Mem_SegAllocExtCritical()-----------------
*                                   LIB_MEM_ERR_SEG_OVF                 Allocation would overflow memory segment.
*
*                                   ------------------RETURNED BY Mem_SegAllocInternal()-------------------
*                                   LIB_MEM_ERR_INVALID_MEM_ALIGN       Invalid memory block alignment requested.
*                                   LIB_MEM_ERR_INVALID_MEM_SIZE        Invalid memory block size specified.
*                                   LIB_MEM_ERR_NULL_PTR                Error or segment data pointer NULL.
*                                   LIB_MEM_ERR_SEG_OVF                 Allocation would overflow memory segment.
*
*                                   -----------------------RETURNED BY Mem_PoolClr()-----------------------
*                                   LIB_MEM_ERR_NULL_PTR                Argument 'p_pool' passed a NULL pointer.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) This function is DEPRECATED and will be removed in a future version of this product.
*                   Mem_DynPoolCreate() or Mem_DynPoolCreateHW() should be used instead.
*********************************************************************************************************
*/

void  Mem_PoolCreate (MEM_POOL          *p_pool,
                      void              *p_mem_base,
                      CPU_SIZE_T         mem_size,
                      MEM_POOL_BLK_QTY   blk_nbr,
                      CPU_SIZE_T         blk_size,
                      CPU_SIZE_T         blk_align,
                      CPU_SIZE_T        *p_bytes_reqd,
                      LIB_ERR           *p_err)
{
    uintptr_t          mem_start_align;
    uintptr_t          mem_end;
    void              *p_pool_mem;
    CPU_SIZE_T         pool_size;
    CPU_SIZE_T         tbl_size;
    CPU_SIZE_T         blk_size_align;
    CPU_ADDR           pool_addr_end;
    MEM_POOL_BLK_QTY   blk_ix;
    CPU_INT08U        *p_blk;


#if (LIB_MEM_CFG_ARG_CHK_EXT_EN == DEF_ENABLED)                 /* --------------- VALIDATE RTN ERR PTR --------------- */
    if (p_err == DEF_NULL) {
        CPU_SW_EXCEPTION(;);
    }

                                                                /* ------------- VALIDATE MEM POOL CREATE ------------- */
    if (p_pool == DEF_NULL) {
       *p_err = LIB_MEM_ERR_NULL_PTR;
        return;
    }

    if (p_mem_base != DEF_NULL) {
        if (mem_size < 1u) {
           *p_err = LIB_MEM_ERR_INVALID_SEG_SIZE;
            return;
        }
    }

    if (blk_nbr < 1u) {
       *p_err = LIB_MEM_ERR_INVALID_BLK_NBR;
        return;
    }

    if (blk_size < 1u) {
       *p_err = LIB_MEM_ERR_INVALID_BLK_SIZE;
        return;
    }

    if (MATH_IS_PWR2(blk_align) != DEF_YES) {                   /* Chk that req alignment is a pwr of 2.                */
       *p_err = LIB_MEM_ERR_INVALID_BLK_ALIGN;
        return;
    }
#endif

    Mem_PoolClr(p_pool, p_err);                                 /* Init mem pool.                                       */
    if (*p_err != LIB_MEM_ERR_NONE) {
         return;
    }

                                                                /* ---------------- ALLOC MEM FOR POOL ---------------- */
                                                                /* Calc blk size with align.                            */
    blk_size_align =  MATH_ROUND_INC_UP_PWR2(blk_size, blk_align);
    pool_size      =  blk_size_align * blk_nbr;                 /* Calc required size for pool.                         */
    tbl_size       =  blk_nbr * sizeof(void *);                 /* Calc required size for free block table.             */

                                                                /* Detect integer overflows in the size calculations.   */
    if ((blk_size_align >  (DEF_INT_CPU_U_MAX_VAL / blk_nbr       )) ||
        (blk_nbr        >  (DEF_INT_CPU_U_MAX_VAL / sizeof(void *)))) {
       *p_err = LIB_MEM_ERR_ADDR_OVF;
        return;
    }
                                                                /* ------------ ALLOC MEM FOR FREE BLK TBL ------------ */
    p_pool->BlkFreeTbl = (void **)osAllocAlign(sizeof(void *), tbl_size);
    if (p_pool->BlkFreeTbl == NULL) {
        *p_err = LIB_MEM_ERR_SEG_OVF;
        return;
    }

                                                                /* Alloc mem for pool.                                  */
    if (p_mem_base == DEF_NULL) {
        p_pool_mem = osAllocAlign(blk_align, pool_size);
    } else {
        mem_start_align = MATH_ROUND_INC_UP_PWR2((uintptr_t)p_mem_base, blk_align);
        mem_end = (uintptr_t)p_mem_base + mem_size;

        if ((mem_end > mem_start_align) &&
            (mem_end - mem_start_align >= pool_size))
            p_pool_mem = (void *)mem_start_align;
        else
            p_pool_mem = NULL;
    }

    if (p_pool_mem == NULL) {
        osFree(p_pool->BlkFreeTbl);
        *p_err = LIB_MEM_ERR_SEG_OVF;
        return;
    }

                                                                /* ------------------ INIT BLK LIST ------------------- */
    p_blk = (CPU_INT08U *)p_pool_mem;
    for (blk_ix = 0; blk_ix < blk_nbr; blk_ix++) {
        p_pool->BlkFreeTbl[blk_ix]  = p_blk;
        p_blk                      += blk_size_align;
    }


                                                                /* ------------------ INIT POOL DATA ------------------ */
    pool_addr_end         = (CPU_ADDR)p_pool_mem + (pool_size - 1u);
    p_pool->PoolAddrStart =  p_pool_mem;
    p_pool->PoolAddrEnd   = (void *)pool_addr_end;
    p_pool->BlkNbr        =  blk_nbr;
    p_pool->BlkSize       =  blk_size_align;
    p_pool->BlkFreeTblIx  =  blk_nbr;
}


/*
*********************************************************************************************************
*                                            Mem_PoolClr()
*
* Description : Clears a memory pool (see Note #1).
*
* Argument(s) : p_pool   Pointer to a memory pool structure to clear (see Note #2).
*
*               p_err    Pointer to variable that will receive the return error code from this function :
*
*                               LIB_MEM_ERR_NONE                Operation was successful.
*                               LIB_MEM_ERR_NULL_PTR            Argument 'p_pool' passed a NULL pointer.
*
* Return(s)   : none.
*
* Caller(s)   : Application,
*               Mem_PoolCreate().
*
* Note(s)     : (1) (a) Mem_PoolClr() ONLY clears a memory pool structure's variables & should ONLY be
*                       called to initialize a memory pool structure prior to calling Mem_PoolCreate().
*
*                   (b) Mem_PoolClr() does NOT deallocate memory from the memory pool or deallocate the
*                       memory pool itself & MUST NOT be called after calling Mem_PoolCreate() since
*                       this will likely corrupt the memory pool management.
*
*               (2) Assumes 'p_pool' points to a valid memory pool (if non-NULL).
*
*               (3) This function is DEPRECATED and will be removed in a future version of this product.
*********************************************************************************************************
*/

void  Mem_PoolClr (MEM_POOL  *p_pool,
                   LIB_ERR   *p_err)
{
#if (LIB_MEM_CFG_ARG_CHK_EXT_EN == DEF_ENABLED)                 /* -------------- VALIDATE RTN ERR  PTR --------------- */
    if (p_err == DEF_NULL) {
        CPU_SW_EXCEPTION(;);
    }

                                                                /* -------------- VALIDATE MEM POOL PTR --------------- */
    if (p_pool == DEF_NULL) {
       *p_err = LIB_MEM_ERR_NULL_PTR;
        return;
    }
#endif

    p_pool->PoolAddrStart = DEF_NULL;
    p_pool->PoolAddrEnd   = DEF_NULL;
    p_pool->BlkSize       = 0u;
    p_pool->BlkNbr        = 0u;
    p_pool->BlkFreeTbl    = DEF_NULL;
    p_pool->BlkFreeTblIx  = 0u;

   *p_err = LIB_MEM_ERR_NONE;
}


/*
*********************************************************************************************************
*                                          Mem_PoolBlkGet()
*
* Description : Gets a memory block from memory pool.
*
* Argument(s) : p_pool  Pointer to  memory pool to get memory block from.
*
*               size    Size of requested memory (in bytes).
*
*               p_err   Pointer to variable that will receive the return error code from this function :
*
*                           LIB_MEM_ERR_NONE                Operation was successful.
*                           LIB_MEM_ERR_INVALID_BLK_SIZE    Invalid memory pool block size requested.
*                           LIB_MEM_ERR_NULL_PTR            Argument 'p_pool' passed a NULL pointer.
*                           LIB_MEM_ERR_POOL_EMPTY          NO memory blocks available in memory pool.
*
* Return(s)   : Pointer to memory block, if NO error(s).
*
*               Pointer to NULL,         otherwise.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) This function is DEPRECATED and will be removed in a future version of this product.
*                   Mem_DynPoolBlkGet() should be used instead.
*********************************************************************************************************
*/

void  *Mem_PoolBlkGet (MEM_POOL    *p_pool,
                       CPU_SIZE_T   size,
                       LIB_ERR     *p_err)
{
    CPU_INT08U  *p_blk;
    CPU_SR_ALLOC();


#if (LIB_MEM_CFG_ARG_CHK_EXT_EN == DEF_ENABLED)                 /* -------------- VALIDATE MEM POOL GET --------------- */
    if (p_err == DEF_NULL) {                                    /* Validate err ptr.                                    */
        CPU_SW_EXCEPTION(DEF_NULL);
    }

    if (p_pool == DEF_NULL) {                                   /* Validate pool ptr.                                   */
       *p_err = LIB_MEM_ERR_NULL_PTR;
        return (DEF_NULL);
    }

    if (size < 1u) {                                            /* Validate req'd size as non-NULL.                     */
       *p_err = LIB_MEM_ERR_INVALID_BLK_SIZE;
        return (DEF_NULL);
    }

    if (size > p_pool->BlkSize) {                               /* Validate req'd size <= mem pool blk size.            */
       *p_err = LIB_MEM_ERR_INVALID_BLK_SIZE;
        return (DEF_NULL);
    }
#else
    (void)size;                                                 /* Prevent possible 'variable unused' warning.          */
#endif


                                                                /* -------------- GET MEM BLK FROM POOL --------------- */
    p_blk = DEF_NULL;
    CPU_CRITICAL_ENTER();
    if (p_pool->BlkFreeTblIx > 0u) {
        p_pool->BlkFreeTblIx                     -=  1u;
        p_blk                                     = (CPU_INT08U *)p_pool->BlkFreeTbl[p_pool->BlkFreeTblIx];
        p_pool->BlkFreeTbl[p_pool->BlkFreeTblIx]  =  DEF_NULL;
    }
    CPU_CRITICAL_EXIT();

    if (p_blk == DEF_NULL) {
       *p_err = LIB_MEM_ERR_POOL_EMPTY;
    } else {
       *p_err = LIB_MEM_ERR_NONE;
    }

    return (p_blk);
}


/*
*********************************************************************************************************
*                                          Mem_PoolBlkFree()
*
* Description : Free a memory block to memory pool.
*
* Argument(s) : p_pool  Pointer to memory pool to free memory block.
*
*               p_blk   Pointer to memory block address to free.
*
*               p_err   Pointer to variable that will receive the return error code from this function :
*
*                           LIB_MEM_ERR_NONE                        Operation was successful.
*                           LIB_MEM_ERR_NULL_PTR                    Argument 'p_pool'/'p_blk' passed
*                                                                       a NULL pointer.
*                           LIB_MEM_ERR_INVALID_BLK_ADDR            Invalid memory block address.
*                           LIB_MEM_ERR_INVALID_BLK_ADDR_IN_POOL            Memory block address already
*                                                                        in memory pool.
*                           LIB_MEM_ERR_POOL_FULL                   Pool is full.
*
* Return(s)   : none.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) This function is DEPRECATED and will be removed in a future version of this product.
*                   Mem_DynPoolBlkFree() should be used instead.
*********************************************************************************************************
*/

void  Mem_PoolBlkFree (MEM_POOL  *p_pool,
                       void      *p_blk,
                       LIB_ERR   *p_err)
{
#if (LIB_MEM_CFG_ARG_CHK_EXT_EN == DEF_ENABLED)
    CPU_SIZE_T   tbl_ix;
    CPU_BOOLEAN  addr_valid;
#endif
    CPU_SR_ALLOC();


#if (LIB_MEM_CFG_ARG_CHK_EXT_EN == DEF_ENABLED)                 /* -------------- VALIDATE MEM POOL FREE -------------- */
    if (p_err == DEF_NULL) {
        CPU_SW_EXCEPTION(;);
    }

    if (p_pool == DEF_NULL) {                                   /* Validate mem ptrs.                                   */
       *p_err = LIB_MEM_ERR_NULL_PTR;
        return;
    }

    if (p_blk == DEF_NULL) {
       *p_err = LIB_MEM_ERR_NULL_PTR;
        return;
    }

    addr_valid = Mem_PoolBlkIsValidAddr(p_pool, p_blk);         /* Validate mem blk as valid pool blk addr.             */
    if (addr_valid != DEF_OK) {
       *p_err = LIB_MEM_ERR_INVALID_BLK_ADDR;
        return;
    }

    CPU_CRITICAL_ENTER();                                       /* Make sure blk isn't already in free list.            */
    for (tbl_ix = 0u; tbl_ix < p_pool->BlkNbr; tbl_ix++) {
        if (p_pool->BlkFreeTbl[tbl_ix] == p_blk) {
            CPU_CRITICAL_EXIT();
           *p_err = LIB_MEM_ERR_INVALID_BLK_ADDR_IN_POOL;
            return;
        }
    }
#else                                                           /* Double-free possibility if not in critical section.  */
    CPU_CRITICAL_ENTER();
#endif
                                                                /* --------------- FREE MEM BLK TO POOL --------------- */
    if (p_pool->BlkFreeTblIx >= p_pool->BlkNbr) {
        CPU_CRITICAL_EXIT();
       *p_err = LIB_MEM_ERR_POOL_FULL;
        return;
    }

    p_pool->BlkFreeTbl[p_pool->BlkFreeTblIx]  = p_blk;
    p_pool->BlkFreeTblIx                     += 1u;
    CPU_CRITICAL_EXIT();

   *p_err = LIB_MEM_ERR_NONE;
}


/*
*********************************************************************************************************
*                                      Mem_PoolBlkGetNbrAvail()
*
* Description : Get memory pool's remaining number of blocks available to allocate.
*
* Argument(s) : p_pool   Pointer to a memory pool structure.
*
*               p_err    Pointer to variable that will receive the return error code from this function :
*
*                               LIB_MEM_ERR_NONE                Operation was successful.
*                               LIB_MEM_ERR_NULL_PTR            Argument 'p_pool' passed a NULL pointer.
*
* Return(s)   : Remaining memory pool blocks,   if NO error(s).
*
*               0,                              otherwise.
*
* Caller(s)   : Application.
*
* Note(s)     : (1) This function is DEPRECATED and will be removed in a future version of this product.
*                   Mem_DynPoolBlkNbrAvailGet() should be used instead.
*********************************************************************************************************
*/

MEM_POOL_BLK_QTY  Mem_PoolBlkGetNbrAvail (MEM_POOL  *p_pool,
                                          LIB_ERR   *p_err)
{
    CPU_SIZE_T  nbr_avail;
    CPU_SR_ALLOC();


#if (LIB_MEM_CFG_ARG_CHK_EXT_EN == DEF_ENABLED)
                                                                /* --------------- VALIDATE RTN ERR PTR --------------- */
    if (p_err == DEF_NULL) {
        CPU_SW_EXCEPTION(0u);
    }
                                                                /* ---------------- VALIDATE MEM POOL ----------------- */
    if (p_pool == DEF_NULL) {                                   /* Validate mem ptr.                                    */
       *p_err =  LIB_MEM_ERR_NULL_PTR;
        return (0u);
    }
#endif

    CPU_CRITICAL_ENTER();
    nbr_avail = p_pool->BlkFreeTblIx;
    CPU_CRITICAL_EXIT();

   *p_err = LIB_MEM_ERR_NONE;

    return (nbr_avail);
}


/*
*********************************************************************************************************
*                                      Mem_PoolBlkIsValidAddr()
*
* Description : Calculates if a given memory block address is valid for the memory pool.
*
* Argument(s) : p_pool   Pointer to memory pool structure to validate memory block address.
*               ------   Argument validated by caller.
*
*               p_mem    Pointer to memory block address to validate.
*               -----    Argument validated by caller.
*
* Return(s)   : DEF_YES, if valid memory pool block address.
*
*               DEF_NO,  otherwise.
*
* Caller(s)   : Mem_PoolBlkFree().
*
* Note(s)     : (1) This function is DEPRECATED and will be removed in a future version of this product.
*********************************************************************************************************
*/

#if (LIB_MEM_CFG_ARG_CHK_EXT_EN == DEF_ENABLED)
static  CPU_BOOLEAN  Mem_PoolBlkIsValidAddr (MEM_POOL  *p_pool,
                                             void      *p_mem)
{
    CPU_ADDR  pool_offset;


    if ((p_mem < p_pool->PoolAddrStart) ||
        (p_mem > p_pool->PoolAddrEnd)) {
        return (DEF_FALSE);
    }

    pool_offset = (CPU_ADDR)p_mem - (CPU_ADDR)p_pool->PoolAddrStart;
    if (pool_offset % p_pool->BlkSize != 0u) {
        return (DEF_FALSE);
    } else {
        return (DEF_TRUE);
    }
}
#endif

