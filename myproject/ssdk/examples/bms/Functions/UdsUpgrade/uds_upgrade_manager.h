#ifndef __UDS_UPGRADE_MANAGER_H__
#define __UDS_UPGRADE_MANAGER_H__

#include "icc_upgrade_handler.h"

void uds_upgrade_manager_proc(void);

int uds_upgrade_manager_is_active(void);

upgrade_state_t uds_upgrade_manager_get_state(void);

#endif
