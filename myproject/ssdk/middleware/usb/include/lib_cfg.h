/**
 * @file lib_cfg.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Custom Library Configuration file for USB Device & Host.
 *
 * Revision History:
 * -----------------
 */


#ifndef  LIB_CFG_MODULE_PRESENT
#define  LIB_CFG_MODULE_PRESENT


/*
*********************************************************************************************************
*********************************************************************************************************
*                                    MEMORY LIBRARY CONFIGURATION
*********************************************************************************************************
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                             MEMORY LIBRARY ARGUMENT CHECK CONFIGURATION
*
* Note(s) : (1) Configure LIB_MEM_CFG_ARG_CHK_EXT_EN to enable/disable the memory library suite external
*               argument check feature :
*
*               (a) When ENABLED,     arguments received from any port interface provided by the developer
*                   or application are checked/validated.
*
*               (b) When DISABLED, NO arguments received from any port interface provided by the developer
*                   or application are checked/validated.
*********************************************************************************************************
*/

                                                                /* External argument check.                             */
                                                                /* Indicates if arguments received from any port ...    */
                                                                /* ... interface provided by the developer or ...       */
                                                                /* ... application are checked/validated.               */
#define  LIB_MEM_CFG_ARG_CHK_EXT_EN     DEF_DISABLED


#endif                                                          /* End of lib cfg module include.                       */

