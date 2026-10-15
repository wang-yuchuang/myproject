#include <stdio.h>
#include <string.h>
#include <FreeRTOS.h>
#include <task.h>
#include <heap_ext.h>
#include "boot_splash.h"

#define SPLASH_WIDTH      800
#define SPLASH_HEIGHT     480
#define SPLASH_TIMEOUT_MS 3000

static void boot_splash_task(void *arg)
{
    uint8_t *bmp_data = NULL;
    
    printf("[BootSplash] Starting...\n");
    
    bmp_data = ExtMallocAligned(SPLASH_WIDTH * SPLASH_HEIGHT * 4, 32);
    if (!bmp_data) {
        printf("[BootSplash] ExtMallocAligned failed\n");
        goto exit;
    }
    
    memset(bmp_data, 0, SPLASH_WIDTH * SPLASH_HEIGHT * 4);
    
    printf("[BootSplash] DDR heap buffer allocated at 0x%x (size=%d)\n", 
           (unsigned int)bmp_data, SPLASH_WIDTH * SPLASH_HEIGHT * 4);
    
    printf("[BootSplash] Boot splash placeholder running for %d ms...\n", SPLASH_TIMEOUT_MS);
    vTaskDelay(pdMS_TO_TICKS(SPLASH_TIMEOUT_MS));
    
    printf("[BootSplash] Boot splash completed\n");
    
exit:
    if (bmp_data) {
        ExtFree(bmp_data);
    }
    
    vTaskDelete(NULL);
}

void boot_splash_show(void)
{
    TaskHandle_t task_hdl;
    
    xTaskCreate(boot_splash_task, "boot_splash", 
                2048, NULL, 3, &task_hdl);
}