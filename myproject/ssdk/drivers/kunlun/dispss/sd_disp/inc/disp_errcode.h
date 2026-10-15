#ifndef __DISP_ERRORCODE_H_
#define __DISP_ERRORCODE_H_

#ifdef CONFIG_USE_SYS_ERRCODE
#include <sd_errcode.h>

#define DISP_ERRCODE_ID     MOD_ID_DC
#define DSI_ERRCODE_ID      MOD_ID_MIPI_DSI
#define disp_set_errcode(_mod_id, _mod_no, _error_code, _user_data) \
    ERRCODE_PRINT(_mod_id, _mod_no, _error_code, _user_data)
#else
#define DISP_ERRCODE_ID     10
#define DSI_ERRCODE_ID      27

#define disp_set_errcode(_mod_id, _mod_no, _error_code, _user_data)
#endif

#endif

