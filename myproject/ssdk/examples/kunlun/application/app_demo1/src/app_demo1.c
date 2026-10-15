#include <CLI.h>
#include <debug.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "app_demo1.h"

#include <malloc.h>
#include <driver/wav_header.h>
#include "audio_res.h"
#include "param.h"


static int app_demo1_init(){
    //init....
    ssdk_printf(SSDK_INFO,"This is app_demo1 init function.\n");
    return 0;
}

static int app_demo1_deinit(){
    //deinit....
    ssdk_printf(SSDK_INFO,"This is app_demo1 deinit function.\n");
    return 0;
}

static int app_demo1_test(){
    //tst....
    ssdk_printf(SSDK_INFO,"This is app_demo1 test function.\n");
    return 0;
}

static int app_demo1(int argc, char *argv[])
{
    if (!strcmp(argv[0], "init")) {

        ssdk_printf(SSDK_INFO,"app_demo1 init start.\n");
        app_demo1_init();
        return 0;

    } else if (!strcmp(argv[0], "deinit")) {

        ssdk_printf(SSDK_INFO,"app_demo1 deinit start.\n");
        app_demo1_deinit();
        return 0;

    } else if (!strcmp(argv[0], "tst")) {

        ssdk_printf(SSDK_INFO,"app_demo1 test start.\n");
        app_demo1_test();
        return 0;
    }
    return 0;
}

CLI_CMD("app_demo1", "ahub init/wr/deinit", app_demo1);

#if 1
#define MP_MEMBASE 0x37800000
static int test_ddr(int argc, char *argv[])
{
    volatile uint8_t *p = (uint8_t *)0x37800000ul;
    uint8_t read[4] = {0};
    int test_len = atoi(argv[0]);

    if(test_len > (256*1024*1024)){
        test_len = 256*1024*1024;
    }
    ssdk_printf(SSDK_INFO,"write ddr(%d)\n",test_len);

    for(int i=0;i<test_len;i++){
        *(p+i) = 0x5a;
    }

    for(int i=0;i<test_len;i++){
        if(p[i] != 0x5a){
            ssdk_printf(SSDK_INFO,"read value fail, 5A -> %x\n",p[i]);
        }
    }
    ssdk_printf(SSDK_INFO,"read ddr: \n");
    for(int i=0;i<test_len;i++){
        ssdk_printf(SSDK_INFO,"%x,",p[i]);
        if(((i+1)%16) == 0){
            ssdk_printf(SSDK_INFO,"\n");
        }
    }
    ssdk_printf(SSDK_INFO,"\n");
    return 0;
}

CLI_CMD("test_ddr", "test ddr", test_ddr);


extern void ExtHeapInit( uint32_t address ,uint32_t size);
extern void *ExtMallocAligned( size_t xWantedSize ,size_t xBoundary );
extern void *ExtMalloc( size_t xWantedSize );
extern void ExtFree( void *pv );

struct HEAP_TEST_NODE_T{
    uint8_t *addr;
    uint8_t is_used;
};

#define MAX_EXT_HEAP_TEST_NODE  16
struct HEAP_TEST_NODE_T heap_control[MAX_EXT_HEAP_TEST_NODE];

static int heap_create(int argc, char *argv[])
{
#define EXT_HEAP_ADDR       0x37800000ul
#define EXT_HEAP_SIZE       (64*1024*1024)  //分配64M
    ExtHeapInit(EXT_HEAP_ADDR, EXT_HEAP_SIZE);
    ssdk_printf(SSDK_INFO,"extern heap create at: %x, size: %d\n",EXT_HEAP_ADDR,EXT_HEAP_SIZE);
    for(int i=0;i<MAX_EXT_HEAP_TEST_NODE;i++){
        heap_control[i].addr = 0;
        heap_control[i].is_used = 0;
    }
    return 0;
}
CLI_CMD("heap_create", "heap create", heap_create);


extern uint32_t xTaskGetTickCount( void );
int test_memory_access(char* start_address, size_t size) 
{
    // Generate a simple seed based on time
    unsigned int orgin_tick = xTaskGetTickCount();
    unsigned int seed = (unsigned int)orgin_tick;
    
    // Write random data to the memory space
    for (size_t i = 0; i < size; i++) {
        // Simple pseudo-random number generator (Linear Congruential Generator)
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        start_address[i] = (char)(seed >> 16);  // Use higher bits for better randomness
    }
    
    // Reset seed for reproducible sequence
    seed = (unsigned int)orgin_tick;
    
    // Read back and verify the data
    for (size_t i = 0; i < size; i++) {
        // Generate the same sequence of random numbers
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        char expected_value = (char)(seed >> 16);
        
        // Check if the value read matches what we wrote
        if (start_address[i] != expected_value) {
            return 0;  // Mismatch found
        }
    }
    
    return 1;  // All values matched
}

static int heap_malloc(int argc, char *argv[])
{
    uint8_t is_find = 0;
    uint32_t malloc_size = atoi(argv[0]);
    for(int i=0;i<MAX_EXT_HEAP_TEST_NODE;i++){
        if(heap_control[i].is_used == 0){
            heap_control[i].addr = (uint8_t*)ExtMalloc(malloc_size);
            if(heap_control[i].addr != 0){
                is_find = 1;
                heap_control[i].is_used = 1;
                ssdk_printf(SSDK_INFO,"malloc success: (0x%x,%d)\n",heap_control[i].addr,malloc_size);
                int ret = test_memory_access((char*)heap_control[i].addr,malloc_size);
                if(ret){
                    ssdk_printf(SSDK_INFO,"read/write test success\n");
                }else{
                    ssdk_printf(SSDK_INFO,"read/write test fail\n");
                }
                break;
            }else{
                ssdk_printf(SSDK_INFO,"malloc fail\n");
            }
        }
    }
    if(!is_find){
        ssdk_printf(SSDK_INFO,"there is no empty node\n");
    }
    return 0;
}
CLI_CMD("heap_malloc", "heap malloc", heap_malloc);


static int heap_free(int argc, char *argv[])
{
    for(int i=0;i<MAX_EXT_HEAP_TEST_NODE;i++){
        if(heap_control[i].is_used == 1){
            ExtFree(heap_control[i].addr);
            ssdk_printf(SSDK_INFO,"free success: (0x%x)\n",heap_control[i].addr);
            heap_control[i].is_used = 0;
            heap_control[i].addr = 0;
            break;
        }
    }
    return 0;
}
CLI_CMD("heap_free", "heap free", heap_free);

static int heap_list(int argc, char *argv[])
{
    ssdk_printf(SSDK_INFO,"heap list:\n");
    for(int i=0;i<MAX_EXT_HEAP_TEST_NODE;i++){
        if(heap_control[i].is_used == 1){
            ssdk_printf(SSDK_INFO,"[%d] (0x%x)\n",i,heap_control[i].addr);
        }
    }
    return 0;
}
CLI_CMD("heap_list", "heap list", heap_list);
#endif
