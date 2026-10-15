/**
 * @file usbd_cfg.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: USB Device Configuration File For Mass Storage Application.
 *
 * Revision History:
 * -----------------
 */


#ifndef  USBD_CFG_MODULE_PRESENT
#define  USBD_CFG_MODULE_PRESENT


/*
*********************************************************************************************************
*                                  USB DEVICE GENERIC CONFIGURATION
*********************************************************************************************************
*/

                                                                /* uC/USB-Device Code Optimization.                     */
#define  USBD_CFG_OPTIMIZE_SPD                  DEF_ENABLED
                                                                /* DEF_ENABLED  Optimizes uC/USBD for speed performance.*/
                                                                /* DEF_DISABLED Optimizes uC/USBD for memory footprint. */

                                                                /* Maximum Number of Devices.                           */
#define  USBD_CFG_MAX_NBR_DEV                              1u
                                                                /* Must be between 1u and 255u.                         */

                                                                /* Buffer Alignment in uC/USB-Device.                   */
#define  USBD_CFG_BUF_ALIGN_OCTETS                         4u
                                                                /* Must be between 1u and 2^(CPU_CFG_DATA_SIZE * 8).    */


/*
*********************************************************************************************************
*                                USB DEVICE ARGUMENT CHECK CONFIGURATION
*
* Note(s) : (1) Configure USBD_ERR_CFG_ARG_CHK_EXT_EN to enable or disable argument checks.
*
*               (a) When DEF_ENABLED,  ALL arguments received from any API provided by the developer
*                   or application are checked/validated.
*               (b) When DEF_DISABLED, NO  arguments received from any API provided by the developer
*                   or application are checked/validated.
*********************************************************************************************************
*/

                                                                /* Argument Check Feature.                              */
#define  USBD_CFG_ERR_ARG_CHK_EXT_EN            DEF_ENABLED
                                                                /* See Note #1.                                         */


/*
*********************************************************************************************************
*                          USB DEVICE MICROSOFT OS DESCRIPTOR CONFIGURATION
*
* Note(s) : (1) Configure USBD_CFG_MS_OS_DESC_EN to enable or disable Microsoft OS descriptor.
*
*               (a) When DEF_ENABLED,  Microsoft descriptors and MS OS string descriptor are enabled.
*               (b) When DEF_DISABLED, Microsoft descriptors and MS OS string descriptor are disabled.
*********************************************************************************************************
*/

                                                                /* Microsoft Descriptor Support.                        */
#define  USBD_CFG_MS_OS_DESC_EN                 DEF_DISABLED
                                                                /* See Note #1.                                         */


/*
*********************************************************************************************************
*                                      USB DEVICE CONFIGURATIONS
*
* Note(s) : (1) This configuration should be set to DEF_ENABLED if the USB device controller supports
*               high-speed, or to DEF_DISABLED if otherwise.
*********************************************************************************************************
*/

                                                                /* Maximum Number of Configurations.                    */
#define  USBD_CFG_MAX_NBR_CFG                              2u
                                                                /* Must be between 1u and 255u.                         */

                                                                /* Configure Isochronous Support in uC/USB-Device.      */
#define  USBD_CFG_EP_ISOC_EN                    DEF_DISABLED
                                                                /* DEF_ENABLED  Isochronous enpoints are     available. */
                                                                /* DEF_DISABLED Isochronous enpoints are not available. */

                                                                /* Configure High-Speed Support in uC/USB-Device.       */
#define  USBD_CFG_HS_EN                         DEF_ENABLED
                                                                /* See Note #1.                                         */

                                                                /* Timeout for Data/Status Phases of Control Transfer.  */
#define  USBD_CFG_CTRL_REQ_TIMEOUT_mS                   5000u
                                                                /* Must be between 1u and 65535u.                       */


/*
*********************************************************************************************************
*                                 USB DEVICE INTERFACES CONFIGURATION
*********************************************************************************************************
*/

                                                                /* Maximum Number of Interfaces.                        */
#define  USBD_CFG_MAX_NBR_IF                               2u
                                                                /* Must be between 1u and 255u.                         */

                                                                /* Maximum Number of Alternate Interfaces.              */
#define  USBD_CFG_MAX_NBR_IF_ALT                           2u
                                                                /* Must be between 1u and 255u.                         */

                                                                /* Maximum Number of Interface Groups.                  */
#define  USBD_CFG_MAX_NBR_IF_GRP                           1u
                                                                /* Must be between 0u and 255u.                         */

                                                                /* Maximum Number of Endpoint Descriptors.              */
#define  USBD_CFG_MAX_NBR_EP_DESC                          4u
                                                                /* Must be between 1u and 255u.                         */

                                                                /* Maximum Number of Opened Endpoints.                  */
#define  USBD_CFG_MAX_NBR_EP_OPEN                          4u
                                                                /* Must be between 2u and 255u.                         */

                                                                /* Maximum Number of Additional URBs.                   */
                                                                /* These URBs are used for async queueing.              */
#define  USBD_CFG_MAX_NBR_URB_EXTRA                        0u
                                                                /* Must be between 0u and 255u.                         */


/*
*********************************************************************************************************
*                                   USB DEVICE STRING CONFIGURATION
*********************************************************************************************************
*/

                                                                /* Maximum Number of String Descriptors.                */
#define  USBD_CFG_MAX_NBR_STR                             10u
                                                                /* Must be between 0u and 255u.                         */


/*
*********************************************************************************************************
*                                   USB DEVICE DEBUG CONFIGURATION
*********************************************************************************************************
*/

                                                                /* Debug Module Trace Support.                          */
#define  USBD_CFG_DBG_TRACE_EN                  DEF_DISABLED
                                                                /* DEF_ENABLED  Trace debug module is     available.    */
                                                                /* DEF_DISABLED Trace debug module is not available.    */

                                                                /* Maximum Number of Trace Events.                      */
#define  USBD_CFG_DBG_TRACE_NBR_EVENTS                    10u
                                                                /* Must be between 1u and 255u.                         */

                                                                /* Debug Module Built-In Statistics Support.            */
#define  USBD_CFG_DBG_STATS_EN                  DEF_DISABLED
                                                                /* DEF_ENABLED  Built-in statistics are     available.  */
                                                                /* DEF_DISABLED Built-in statistics are not available.  */

                                                                /* Debug Module Built-In Statistics Counter Data Type.  */
#define  USBD_CFG_DBG_STATS_CNT_TYPE            CPU_INT08U
                                                                /* CPU_INT08U, CPU_INT16U or CPU_INT32U.                */


/*
*********************************************************************************************************
*                               MASS STORAGE CLASS (MSC) CONFIGURATION
*
* Note(s) : (1) USBD_MSC_CFG_MICRIUM_FS is used to enable the Mass Storage Class interface with the
*               various storage media supported by uC/FS such as SD-Card, NAND, etc.
*
*               DEF_ENABLED      Use the uC/FS' MSC class interface.
*               DEF_DISABLED     Use uC/USB-Device's own RAMDisk implementation without a File System.
*
*           (2) USBD_MSC_CFG_FS_REFRESH_TASK_EN is only used when MSC class interfaces with uC/FS. It
*               should be DEF_ENABLED when using a removable media such as SD card. Fixed media such as
*               RAMDisk, NAND should have this constant set to DEF_DISABLED.
*
*               DEF_ENABLED      Create FS Refresh task for polling media status.
*               DEF_DISABLED     Do not create FS Refresh task.
*
*           (3) USBD_RAMDISK_CFG_BASE_ADDR is used to define the data area of the RAM disk. If it is
*               defined with a value other than 0, the RAM disk data area will be set from this base
*               address directly. Conversely, if it is equal to 0, the RAM disk data area will be
*               represented as a table from the program's data area.
*********************************************************************************************************
*/

                                                                /* Maximum Number of MSC Class Instances.               */
#define  USBD_MSC_CFG_MAX_NBR_DEV                          1u
                                                                /* Must be between 1u and 255u.                         */

                                                                /* Maximum Number of Configurations per Class Instance. */
#define  USBD_MSC_CFG_MAX_NBR_CFG                          2u
                                                                /* Must be between 1u and 255u.                         */

                                                                /* Maximum Number of Logical Units.                     */
#define  USBD_MSC_CFG_MAX_LUN                              1u
                                                                /* Must be between 1u and 255u.                         */

                                                                /* Data Buffer Length, in octets.                       */
#define  USBD_MSC_CFG_DATA_LEN                          (16*1024u)
                                                                /* Must be between 1u and DEF_INT_32U_MAX_VAL.          */

                                                                /* Use uC/FS MSC class interface.                       */
#define  USBD_MSC_CFG_MICRIUM_FS                DEF_DISABLED
                                                                /* See Note #1.                                         */

                                                                /* Removable Device Refresh Task.                       */
#define  USBD_MSC_CFG_FS_REFRESH_TASK_EN        DEF_DISABLED
                                                                /* See Note #2.                                         */

                                                                /* Removable Device Refresh Task Polling Delay.         */
#define  USBD_MSC_CFG_DEV_POLL_DLY_mS                    100u
                                                                /* Must be between 1u and DEF_INT_32U_MAX_VAL.          */

                                                                /* Number of RAMDisk units.                             */
#define  USBD_RAMDISK_CFG_NBR_UNITS                        1u
                                                                /* Must be at least 1.                                  */

                                                                /* RAMDisk block size.                                  */
#define  USBD_RAMDISK_CFG_BLK_SIZE                       512u
                                                                /* Must be at least 1.                                  */

                                                                /* RAMDisk number of blocks.                            */
#define  USBD_RAMDISK_CFG_NBR_BLKS                        (128*1024*2u)
                                                                /* Must be at least 1.                                  */

                                                                /* RAMDisk base address in memory.                      */
#define  USBD_RAMDISK_CFG_BASE_ADDR                        0u
                                                                /* See Note #3.                                         */


#endif

