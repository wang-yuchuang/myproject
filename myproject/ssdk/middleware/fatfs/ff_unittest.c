#include <CLI.h>
#include <cmsis_os2.h>
#include <debug.h>
#include <stdlib.h>
#include <string.h>

#include "ff.h"

static FATFS fatfs;

/*
 ****************************Partition mounting instructions********************
 * VolumeStr[FF_VOLUMES] = "mmc0", "mmc1", "norflash0", "norflash1", "memdisk0",
 "usb0";

 * f_mount(&fatfs, "mmc0:1:", 1) "mmc0:1:" represents the first FAT partition to
 mount mmc0 disk

 * f_mount(&fatfs, "flash0:2:", 1) "flash0:2:" represents the second FAT
 partition to mount flash1 disk
 */

static int ff_test_mount(int argc, char *argv[])
{
    ssdk_printf(SSDK_CRIT, "ff mount test start diskid = %s!\n", argv[0]);
    FRESULT rc;

    rc = f_mount(&fatfs, argv[0], 1);
    if (rc) {
        printf("ERROR: f_mount returned %d\r\n", rc);
        return -1;
    }
    printf("f_mount success\r\n");

    ssdk_printf(SSDK_CRIT, "ff mount test end!\n");

    return 0;
}

 /* test read file
  * ff_test_read mmc0:1:file_name 0
  */
static int ff_test_read(int argc, char *argv[])
{
    ssdk_printf(SSDK_CRIT, "ff read test start!\n");
    FIL fil;
    FRESULT rc;
    UINT br;
    uint32_t offset = strtoul(argv[1], NULL, 16);
    uint8_t *buf = osAllocAlign(512, 512 * 16);

    rc = f_open(&fil, argv[0], FA_READ);
    if (rc) {
        printf("ERROR:f_open returned %d\r\n", rc);
        return -1;
    }
    rc = f_lseek(&fil, offset);
    if (rc) {
        printf("ERROR:f_lseek returned %d\r\n", rc);
        return -1;
    }
    rc = f_read(&fil, (void *)buf, 512 * 16, &br);
    if (rc) {
        printf("ERROR:f_read returned %d\r\n", rc);
        return -1;
    }
    rc = f_close(&fil);
    if (rc) {
        printf("ERROR:f_open returned %d\r\n", rc);
        return -1;
    }

    printf("the read string is: %s\n", buf);
    osFree(buf);
    ssdk_printf(SSDK_CRIT, "ff read test end!\n");

    return 0;
}

 /* test write file
  * ff_test_write mmc0:1:file_name 0
  */
static int ff_test_write(int argc, char *argv[])
{
    ssdk_printf(SSDK_CRIT, "ff write test start!\n");
    FIL fil;
    FRESULT rc;
    UINT br;
    uint32_t offset = strtoul(argv[1], NULL, 16);
    const char *buf = "fat: helloworld\n";

    rc = f_open(&fil, argv[0], FA_CREATE_ALWAYS);
    if (rc) {
        printf("ERROR:f_open returned %d\r\n", rc);
        return -1;
    }

    rc = f_close(&fil);
    if (rc) {
        printf("ERROR:f_close returned %d\r\n", rc);
        return -1;
    }

    rc = f_open(&fil, argv[0], FA_WRITE);
    if (rc) {
        printf("ERROR:f_open returned %d\r\n", rc);
        return -1;
    }

    rc = f_lseek(&fil, offset);
    if (rc) {
        printf("ERROR:f_lseek returned %d\r\n", rc);
        return -1;
    }
    rc = f_write(&fil, (void *)buf, strlen(buf) + 1, &br);
    if (rc) {
        printf("ERROR:f_write returned %d\r\n", rc);
        return -1;
    }
    rc = f_close(&fil);
    if (rc) {
        printf("ERROR:f_open returned %d\r\n", rc);
        return -1;
    }

    printf("the write string is: %s\n", buf);
    ssdk_printf(SSDK_CRIT, "ff write test end!\n");
    return 0;
}

static void ff_read_thread(void *arg)
{
    char *argv[3] = { NULL };

    argv[0] = arg;
    argv[1] = "0";

    ff_test_read(3, argv);

    osThreadExit();
}

static void ff_write_thread(void *arg)
{
    char *argv[3] = { NULL };

    argv[0] = arg;
    argv[1] = "0";

    ff_test_write(3, argv);

    osThreadExit();
}

static int ff_test_task(int argc, char *argv[])
{
    if (argc == 1) {
        osThreadNew(ff_write_thread, (void *)argv[0], NULL);
        osThreadNew(ff_read_thread, (void *)argv[0], NULL);
    } else if (argc == 2) {
        osThreadNew(ff_write_thread, (void *)argv[1], NULL);
        osThreadNew(ff_read_thread, (void *)argv[0], NULL);
    }

    return 0;
}

CLI_CMD("ff_test_mount", "ff mount unittest case",
        ff_test_mount);
CLI_CMD("ff_test_read", "ff read file unittest case",
        ff_test_read);
CLI_CMD("ff_test_write", "ff write file unittest case",
        ff_test_write);
CLI_CMD("ff_test_task", "ff concurrency unittest case",
        ff_test_task);
