// The flash loader framework API declarations.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arch.h>
#include <board.h>
#include <debug.h>
#include <param.h>

#include <flash_loader.h>
#include <flash_loader_extra.h>
#include <flash_wrapper.h>

#ifdef CONFIG_AVB_FOOTER
#include <avb_footer.h>
#endif

#ifdef CONFIG_BPT_UPDATE
#include <bpt_v2.h>
#endif

/** external data **/
extern FlashParamsHolder theFlashParams;

static flash_wrapper_t *flash_wrapper;

static flash_addr_t written_size = 0;
static flash_addr_t erase_size = 0;

static struct spi_nor *flash_loader;
static uint32_t img_size;
static void * img_flash_base;
static uint32_t img_link_base;
static uint8_t core;
static bool img_flash_base_fg;

#ifdef CONFIG_BPT_UPDATE
static bool bpt_update_fg;
#endif

#if CONFIG_CHECK_FLASH_LOAD
static uint8_t check_buffer[CONFIG_FLASHLOADER_BUFFER_SIZE];
#endif

static void PrintCurrentFlashParam();

void FlashLoaderRegister(struct spi_nor *flash)
{
    flash_loader = flash;
}

/** public functions **/
#if USE_ARGC_ARGV
uint32_t FlashInit(void *base_of_flash, uint32_t image_size,
                   uint32_t link_address, uint32_t flags,
                   int argc, char const *argv[])
{
  __iar_data_init3();
  arch_early_init();
  board_early_init();
  arch_init();
  board_init();

#ifdef CONFIG_TAISHAN_XSPI
  board_norflash_init();
#endif

  img_size = image_size;
  img_link_base = link_address;
  printf("FlashInit start, base:0x%x, link base:0x%x, size:0x%x\n",
         base_of_flash, img_link_base, img_size);

  flash_wrapper = flash_wrapper_init(flash_loader);
  if (flash_wrapper == NULL) {
    ssdk_printf(SSDK_ERR, "Flash wrapper init fail\n");
    return RESULT_ERROR;
  }

#ifdef CONFIG_BPT_UPDATE
  if (argc > 0) {
    core = atoi(argv[0]);
    bpt_update_fg = atoi(argv[1]);
  }
#endif

  printf("FlashInit Success\n");
  return RESULT_OK;
}
#else
uint32_t FlashInit(void *base_of_flash, uint32_t image_size,
                   uint32_t link_address, uint32_t flags)
{
  __iar_data_init3();
  arch_early_init();
  board_early_init();
  arch_init();
  board_init();

#ifdef CONFIG_TAISHAN_XSPI
  board_norflash_init();
#endif

  img_size = image_size;
  img_link_base = link_address;
  printf("FlashInit start, base:0x%x, link base:0x%x, size:0x%x\n",
         base_of_flash, img_link_base, img_size);

  flash_wrapper = flash_wrapper_init(flash_loader);
  if (flash_wrapper == NULL) {
    ssdk_printf(SSDK_ERR, "Flash wrapper init fail\n");
    return RESULT_ERROR;
  }

  printf("FlashInit Success\n");
  return RESULT_OK;
}
#endif

uint32_t FlashWrite(void *block_start,
                    uint32_t offset_into_block,
                    uint32_t count,
                    char const *buffer)
{
  int ret;
  flash_addr_t rx_addr = FLASH_ADDR_MASK((uint32_t)block_start) + offset_into_block;

  arch_invalidate_cache_range(buffer, count);

  ssdk_printf(SSDK_DEBUG, "FlashWrite block_start:0x%x, offset_into_block:0x%x, count:0x%x, buffer:0x%x\n",
              (uint32_t)block_start, offset_into_block, count, buffer);
  ret = flash_wrapper_write(flash_wrapper, rx_addr, (const uint8_t *)buffer,
                      (flash_size_t)count);

  written_size += (flash_size_t)count;

#if CONFIG_CHECK_FLASH_LOAD
  uint32_t check_count;
  uint8_t *buffer_ptr = (uint8_t *)buffer;
  while (count > 0) {
    check_count = MIN(count, CONFIG_FLASHLOADER_BUFFER_SIZE);
    flash_wrapper_read(flash_wrapper, rx_addr, (uint8_t *)check_buffer, (flash_size_t)check_count);
    for (uint32_t i = 0; i < check_count; i++) {
      if (buffer_ptr[i] != check_buffer[i]) {
        printf("Error@0x%x: write 0x%x, read 0x%x\n",
               (unsigned int)(rx_addr + i), buffer_ptr[i], check_buffer[i]);
      }
    }
    rx_addr += check_count;
    buffer_ptr += check_count;
    count -= check_count;
  }
#endif

  // if the whole image has been written, add the footer or update bpt
  if (written_size >= img_size && ret == 0)
  {
    ssdk_printf(SSDK_DEBUG, "Last copy done, total written size: 0x%x, image size: 0x%x\n",
           (unsigned int)written_size, img_size);
#ifdef CONFIG_AVB_FOOTER
    ret = AppendFooter(flash_wrapper, img_flash_base, img_size);
#endif
#ifdef CONFIG_BPT_UPDATE
    if (bpt_update_fg)
      ret = BptUpdate(flash_wrapper, img_flash_base, img_link_base, img_size, core);
#endif
    printf("Flashload finished, ret:%d (0 is OK)\n", ret);
  }

  if (ret < 0) {
    ssdk_printf(SSDK_ERR, "FlashWrite fail\n");
    return RESULT_ERROR;
  }

  return RESULT_OK;
}

uint32_t FlashErase(void *block_start,
                    uint32_t block_size)
{
  int ret;

  if (!img_flash_base_fg) {
    img_flash_base = block_start;
    img_flash_base_fg = true;
  }

  if ((erase_size + block_size) > img_size) {
    block_size = img_size - erase_size;
  }
  erase_size += block_size;
  block_size = ALIGN(block_size, flash_wrapper->page_size);

  ssdk_printf(SSDK_DEBUG, "FlashErase addr:0x%x, count:0x%x\n",
              (uint32_t)block_start, block_size);
  ret = flash_wrapper_erase(flash_wrapper, FLASH_ADDR_MASK((uint32_t)block_start),
                      (flash_size_t)block_size);
  if (ret < 0) {
    ssdk_printf(SSDK_ERR, "FlashErase fail\n");
    return RESULT_ERROR;
  }

  return RESULT_OK;
}

#if 0
OPTIONAL_CHECKSUM
uint32_t FlashChecksum(void const *begin, uint32_t count)
{
  return Crc16((uint8_t const *)begin, count);
}

OPTIONAL_SIGNOFF
uint32_t FlashSignoff(void)
{
  return 0;
}
#endif

static void PrintCurrentFlashParam()
{
  ssdk_printf(SSDK_DEBUG, "Current value of TheFlashParameter@0x%p:\n", &theFlashParams);
  ssdk_printf(SSDK_DEBUG, "base_ptr: 0x%x\n", (unsigned int)theFlashParams.base_ptr);
  ssdk_printf(SSDK_DEBUG, "count: 0x%x\n", (unsigned int)theFlashParams.count);
  ssdk_printf(SSDK_DEBUG, "offset_into_block: 0x%x\n", (unsigned int)theFlashParams.offset_into_block);
  ssdk_printf(SSDK_DEBUG, "buffer: 0x%p\n", theFlashParams.buffer);
  ssdk_printf(SSDK_DEBUG, "block_size: 0x%x\n", (unsigned int)theFlashParams.block_size);
}
