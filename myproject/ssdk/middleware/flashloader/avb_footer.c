
#include <stdio.h>
#include <stdlib.h>
#include <debug.h>
#include <cmsis_os2.h>

#include <flash_wrapper.h>

#ifdef CONFIG_AVB_FOOTER
#include <avb_footer.h>
#endif

__big_endian __packed static struct AvbFooter avb_footer;

static void FooterInit(struct AvbFooter *footer, uint32_t img_size)
{
    memcpy((void *)footer->magic,(void *)(AVB_FOOTER_MAGIC),
            AVB_FOOTER_MAGIC_LEN);
    footer->version_major = AVB_FOOTER_VERSION_MAJOR;
    footer->version_minor = AVB_FOOTER_VERSION_MINOR;
    footer->original_image_size = img_size;
}

int AppendFooter(flash_wrapper_t *flash_wrapper, void *img_base, uint32_t img_size)
{
    struct AvbFooter *footer = (struct AvbFooter *)&avb_footer;
    uint32_t page_size = flash_wrapper->page_size;
    uint32_t block_size = flash_wrapper->sector_size;
    int ret;

    if (page_size < AVB_FOOTER_SIZE) {
        ssdk_printf(SSDK_ERR, "AppendFooter:page size error\n");
        return -1;
    }

    FooterInit(footer, img_size);

    uint8_t * footer_buffer = (uint8_t *)osAllocAlign(CONFIG_ARCH_CACHE_LINE, page_size);
    if (footer_buffer == NULL)
        return -1;

    // calc the flash address of the footer page to be erased and written
    flash_addr_t footer_write_base =
        (flash_addr_t)img_base + CONFIG_IMG_PARTITION_SIZE - page_size;
    flash_addr_t footer_erase_base =
        (flash_addr_t)img_base + CONFIG_IMG_PARTITION_SIZE - block_size;

    // place the footer with offset in the footer page buffer
    uint32_t footer_offset = page_size - AVB_FOOTER_SIZE;
    memcpy((void *)(footer_buffer + footer_offset), (void *)footer, AVB_FOOTER_SIZE);

    // erase block and write the footer page to flash
    ssdk_printf(SSDK_DEBUG, "Appending image footer...\n");
    ret = flash_wrapper_erase(flash_wrapper, footer_erase_base, block_size);
    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "AppendFooter:erase error\n");
        return -1;
    }

    ret = flash_wrapper_write(flash_wrapper, footer_write_base, footer_buffer, page_size);
    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "AppendFooter:write error\n");
        return -1;
    }

#if CONFIG_CHECK_FLASH_LOAD
    uint8_t * footer_chk_buffer = (uint8_t *)osAllocAlign(CONFIG_ARCH_CACHE_LINE, page_size);
    ssdk_printf(SSDK_DEBUG, "footer_chk_buffer @ 0x%p\n", footer_chk_buffer);
    if (footer_chk_buffer != NULL)
    {
        flash_wrapper_read(flash_wrapper, footer_write_base, footer_chk_buffer, page_size);
        printf("Checking the image footer in flash (no ERR! is OK):\n");
        for (uint32_t i = 0; i < AVB_FOOTER_SIZE; i++)
        {
            if (i % 16 == 0)
                printf("0x%08x: ", (unsigned int)(footer_write_base + footer_offset + i));
            if ((footer_chk_buffer + footer_offset)[i] == (footer_buffer + footer_offset)[i])
                printf("0x%02x ", (footer_chk_buffer + footer_offset)[i]);
            else
                printf("ERR!(W:0x%02x,R:0x%02x)\n",
                    (footer_buffer + footer_offset)[i],
                    (footer_chk_buffer + footer_offset)[i]);
            if (i % 16 == 15)
                printf("\n");
        }
        osFree(footer_chk_buffer);
    }
    else
        printf("Fail to do osAllocAlign for footer_chk_buffer\n");
#endif
    osFree(footer_buffer);
    return 0;
}