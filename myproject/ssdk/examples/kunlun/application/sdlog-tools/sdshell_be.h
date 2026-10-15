#ifndef __SDSHELL_BE_H__
#define __SDSHELL_BE_H__

#ifdef CONFIG_SDSHELL_BACKEND
void board_sdshell_be_init(void);
#else
void board_sdshell_be_init(void) {}
#endif

#endif /* __SDSHELL_BE_H__ */
