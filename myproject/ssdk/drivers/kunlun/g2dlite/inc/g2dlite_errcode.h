#ifndef __DISP_ERRORCODE_H_
#define __DISP_ERRORCODE_H_

#define G2DLITE_MODULE_NO 1

#ifdef CONFIG_USE_SYS_ERRCODE
#include <sd_errcode.h>

#define g2d_set_errcode(_mod_no, _error_code, _user_data) \
    ERRCODE_PRINT(MOD_ID_G2D, _mod_no, _error_code, _user_data)
#else
#define g2d_set_errcode(_mod_no, _error_code, _user_data)
#endif

#endif

