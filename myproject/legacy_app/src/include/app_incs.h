
#ifndef __STD_APP_INCS_H__
#define __STD_APP_INCS_H__

//Public libs
#include "base_defines/basetypes.h"
#include "base_defines/basefunc.h"
#include "base_defines/iNew.h"
#include "base_defines/iLog.h"
#include "base_defines/iMutex.h"
#include "base_defines/iThread.h"
#include "base_defines/iQueue.h"
#include "base_defines/iStack.h"
#include "base_defines/iLog4c.h"

//charger model
#include "data_model/charger_model.h"
#include "data_model/blob_model.h"

////3-rd party plug-ins
//#include "3rd_party/curl/curl.h"
//#include "3rd_party/sqlite/sqlite3.h"

#define ENV_VARIABLE_DATA_PATH		"NCCU_DATA_PATH"	// to use shell env variable.
#define ENV_VARIABLE_APP_PATH		"NCCU_APP_PATH"	// to use shell env variable.
#define ENV_VARIABLE_VAR_PATH		"VAR_DIR"		// to use shell env variable.

//for web
#define LOCALLIST_WEB_EXPORT_FILENAME       "/opt/appdata/data/locallist.export"
#define LOCALLIST_WEB_IMPORT_FILENAME       "/opt/appdata/data/locallist.import"
#define SYSDB_WEB_EXPORT_FILENAME           "/opt/appdata/data/sysdb.export"
#define SYSDB_WEB_IMPORT_FILENAME           "/opt/appdata/data/sysdb.import"
#define IMMU2_FIRMWARE_WEB_IMPORT_FILENAME  "/opt/appdata/data/immu2_firmware.import"
#define BMS_CONFIG_PARAMERTER               "/opt/appdata/data/bms_config_docum"
#define ONECLICK_FIRMWARE_WEB_IMPORT_FILENAME  "/opt/appdata/data/oneclick_firmware.import"

#define BMSHISREC_WEB_EXPORT_FILENAME           "/opt/appdata/data/hisrec.export"

#define APP_UPDATE_FLAG_FILENAME            "/opt/appdata/data/firmwareupdt.flag" //added by Jimmy 20240530

// #include "../version_patch.h"

#endif
