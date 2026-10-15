//#define _TRACE_PRINT_
#include "../fcgi_defs.h"
#define TEMP_CSV_FILE           "PCS_Export_File.csv"
#define TEMP_CSV_PATH           "/opt/appdata/data/" TEMP_CSV_FILE
#define GROUP_COUNT             (sizeof(group_info)/sizeof(ParamGroupInfo))
#define PARAM_ID_CNT_AUSTRIA    (sizeof(g_PCS_ParamId_List_Exprot_Austria)/sizeof(int))

// PCS Param_Id映射表
static const int g_PCS_ParamId_List_Exprot_Austria[] = {
    // 1. EMS CONFIGURATION INFO   8

    2117, 3160, 1127, 2105, 2114, 3121, 3122, 91011,

    // 2. Grid Voltage & Frequency Protection Parameters   18

    2120, 2121, 2122, 2123, 2124, 2125, 2126, 2127, 2128, 2129, 21210, 21211, 21212, 21213, 21214, 21215, 998, 999,
    // 3. Overvoltage/Undervoltage Control Parameters 6

    3126, 3127, 3128, 31138, 31281, 313151,

    //4. Overfrequency/Underfrequency Control Parameters  6

    3136, 3138, 31133, 3131, 3133, 31132, 

    // 5. COSP Mode Power Factor & Control Point Parameters 8

    996, 997, 3174, 3175, 31610, 31611, 31612, 31613,  

    // 6. Grid Dynamic Ride-Through & Reconnection Control Parameters   7

    31130, 31131, 3180, 31811, 31812, 31911, 31912, 

    // 7. QU Mode Reactive Power Control Parameters   12

    3163, 3164, 3165, 3166, 3167, 3168, 3169, 31614, 31615, 311315, 31158, 31159,

    //  8. Module Reconnection & Startup Parameter   7

    31110, 31111, 3118, 3119, 311101, 311111, 3115

};

static const ParamGroupInfo group_info[] = {
    {"EMS CONFIGURATION INFO", 8},
    {"Grid Voltage & Frequency Protection Parameters", 18},
    {"Overvoltage/Undervoltage Control Parameters", 6},
    {"Overfrequency/Underfrequency Control Parameters", 6},
    {"COSP Mode Power Factor & Control Point Parameters", 8},
    {"Grid Dynamic Ride-Through & Reconnection Control Parameters", 7},
    {"QU Mode Reactive Power Control Parameters", 12},
    {"Module Reconnection & Startup Parameters", 7}
};

static int Init_SystemResource(FCGI_ROUGH_DATA* pFCGI_Rough)
{
	char	szDBFile[256] = {0};
	sqlite3 *cfg_db;
	MakeFullFileName(szDBFile, \
		"/app", "config", CHARGER_CONFIG_DB);
	int rc = sqlite3_open_v2( szDBFile, &cfg_db, SQLITE_OPEN_READONLY, NULL);
	if(rc != SQLITE_OK)
	{
		LogOut_A(MAIN_MODULE_DPR, "InitLoader", LOG_TYPE_ERROR,
			"Thd CfgDB [%s] can NOT be opened, please check!", szDBFile);
		return FALSE;
	}

	sqlite3_stmt * stat;
	char sql[256];
	ZERO_POBJS(sql, 256);
	sprintf(sql, "SELECT COUNT(*) FROM Resources");
	sqlite3_prepare_v2(cfg_db, sql, -1, &stat, NULL);
	if(sqlite3_step(stat) != SQLITE_ROW)
	{
		sqlite3_close(cfg_db);
		return FALSE;
	}
	sqlite3_finalize(stat);

    ZERO_POBJS(sql, 256);
	sprintf(sql, "SELECT * FROM Resources");
	sqlite3_prepare_v2(cfg_db, sql, -1, &stat, NULL);
	memset(pFCGI_Rough->ResItem, 0, RESOURCE_ITEM_TOTAL * sizeof(ResourceItem));
	while(sqlite3_step(stat) == SQLITE_ROW)
	{
		int nResID = sqlite3_column_int(stat, 0);
		UCHAR *szName = sqlite3_column_text(stat, 4);
        pFCGI_Rough->ResItem[nResID].nResIdx++;
		strcpy(pFCGI_Rough->ResItem[nResID].szResName, szName);
	}
	sqlite3_finalize(stat);
	sqlite3_close(cfg_db);
	return TRUE;
}

static void GetCurrentTimeString(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    
    // 格式：2025-12-16 14:30:45
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", timeinfo);
}

static uint32_t SwapEndian32(uint32_t value) {
    return ((value & 0x000000FF) << 24) |
           ((value & 0x0000FF00) << 8)  |
           ((value & 0x00FF0000) >> 8)  |
           ((value & 0xFF000000) >> 24);
}

static BOOL Handle_PCSExportFile2_Download(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    if(!Init_SystemResource(pFCGI_Rough))
    {
        Make_Web_OutPut(104, "Init_SystemResource failed!", NULL, NULL);
        return TRUE;
    }

    sqlite3 *data_db = NULL;
    sqlite3_stmt *stmt = NULL;
    char szConfigDB[512] = {0};
    char sql[32768] = {0}; 
    int rc, i, j, g, p;
    char szSysSN[255] = {0};
    float fSoftWareVersion;

    if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 1, 38, 4, (void*)&fSoftWareVersion))
    {
        TRACEX("----ERRRRRR: can not get all Single Rect Data----\n");
        Make_Web_OutPut(104, "can not get all Single Rect Data!", NULL, NULL);
        return TRUE;
    }

    A_BLOB_SINGLE_RECT_INFO allsingleRectData;
    if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 101, sizeof(A_BLOB_SINGLE_RECT_INFO), (void*)&allsingleRectData))
    {
        TRACEX("----ERRRRRR: can not get all Single Rect Data----\n");
        Make_Web_OutPut(104, "can not get all Single Rect Data!", NULL, NULL);
        return TRUE;
    }

    float fDCDCVer = allsingleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][0].fRect_DCDCVer;
    float fPFCVer = allsingleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][0].fRect_PFCVer;

    int nSinglePCSModuleNum = allsingleRectData.nOnlineNum[TYPE_SIG_RECT_ACDC_GRP1];
    if(nSinglePCSModuleNum <= 0 || nSinglePCSModuleNum > 8)
    {
        TRACEX("----WARNING: SinglePCSModuleNum Error = %d, adjusting to 0----\n", nSinglePCSModuleNum);
        nSinglePCSModuleNum = 0;
    }

    A_BLOB_CHARGER_BOOTINFO allChargerBootInfo;
    if(!Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 1, 17, sizeof(A_BLOB_CHARGER_BOOTINFO), (void*)&allChargerBootInfo))
    {
        TRACEX("----ERRRRRR: can not get all Charger Boot Info----\n");
        Make_Web_OutPut(104, "can not get all Charger Boot Info!", NULL, NULL);
        return TRUE;
    }

    data_db = Open_Lock_SqliteDB();
    if (data_db == NULL)
    {
        printf("Failed to open main database!\n");
        Make_Web_OutPut(104, "Failed to open main database!", NULL, NULL);
        return TRUE;
    }

    snprintf(szConfigDB, sizeof(szConfigDB), "/app/config/System.cfg");
    if (access(szConfigDB, R_OK) != 0)
    {
        printf("Config DB file not found or no access: %s\n", szConfigDB);
        sqlite3_close(data_db);
        Close_UnLock_SqliteDB();
        Make_Web_OutPut(104, "Config DB file access failed!", NULL, NULL);
        return TRUE;
    }

    char *attach_sql = sqlite3_mprintf("ATTACH DATABASE '%q' AS orders_db;", szConfigDB);
    if (attach_sql == NULL)
    {
        printf("Failed to allocate attach SQL!\n");
        sqlite3_close(data_db);
        Close_UnLock_SqliteDB();
        return TRUE;
    }

    rc = sqlite3_exec(data_db, attach_sql, NULL, NULL, NULL);
    sqlite3_free(attach_sql); 

    if (rc != SQLITE_OK)
    {
        printf("Failed to attach config database: %s\n", sqlite3_errmsg(data_db));
        sqlite3_close(data_db);
        Close_UnLock_SqliteDB();
        Make_Web_OutPut(104, "Attach config DB failed!", NULL, NULL);
        return TRUE;
    }

    snprintf(sql, sizeof(sql), 
    "WITH ParamValueParse AS (\n"
    "    SELECT \n"
    "        u.*,\n"
    "        cds.Value_Type,\n"
    "        cds.Value_Range,\n"
    "        cds.Unit_Res,\n"
    "        CAST(\n"
    "            (CAST(substr(hex(u.Current_Value),7,2) AS INTEGER)*16777216) +\n"
    "            (CAST(substr(hex(u.Current_Value),5,2) AS INTEGER)*65536) +\n"
    "            (CAST(substr(hex(u.Current_Value),3,2) AS INTEGER)*256) +\n"
    "            CAST(substr(hex(u.Current_Value),1,2) AS INTEGER)\n"
    "        AS TEXT) AS CurrentValueNum,\n"
    "        CAST(\n"
    "            (CAST(substr(hex(u.RereadValue_SingleModule_1),7,2) AS INTEGER)*16777216) +\n"
    "            (CAST(substr(hex(u.RereadValue_SingleModule_1),5,2) AS INTEGER)*65536) +\n"
    "            (CAST(substr(hex(u.RereadValue_SingleModule_1),3,2) AS INTEGER)*256) +\n"
    "            CAST(substr(hex(u.RereadValue_SingleModule_1),1,2) AS INTEGER)\n"
    "        AS TEXT) AS Reread1Num,\n"
    "        CAST(\n"
    "            (CAST(substr(hex(u.RereadValue_SingleModule_2),7,2) AS INTEGER)*16777216) +\n"
    "            (CAST(substr(hex(u.RereadValue_SingleModule_2),5,2) AS INTEGER)*65536) +\n"
    "            (CAST(substr(hex(u.RereadValue_SingleModule_2),3,2) AS INTEGER)*256) +\n"
    "            CAST(substr(hex(u.RereadValue_SingleModule_2),1,2) AS INTEGER)\n"
    "        AS TEXT) AS Reread2Num,\n"
    "        CAST(\n"
    "            (CAST(substr(hex(u.RereadValue_SingleModule_3),7,2) AS INTEGER)*16777216) +\n"
    "            (CAST(substr(hex(u.RereadValue_SingleModule_3),5,2) AS INTEGER)*65536) +\n"
    "            (CAST(substr(hex(u.RereadValue_SingleModule_3),3,2) AS INTEGER)*256) +\n"
    "            CAST(substr(hex(u.RereadValue_SingleModule_3),1,2) AS INTEGER)\n"
    "        AS TEXT) AS Reread3Num,\n"
    "        CAST(\n"
    "            (CAST(substr(hex(u.RereadValue_SingleModule_4),7,2) AS INTEGER)*16777216) +\n"
    "            (CAST(substr(hex(u.RereadValue_SingleModule_4),5,2) AS INTEGER)*65536) +\n"
    "            (CAST(substr(hex(u.RereadValue_SingleModule_4),3,2) AS INTEGER)*256) +\n"
    "            CAST(substr(hex(u.RereadValue_SingleModule_4),1,2) AS INTEGER)\n"
    "        AS TEXT) AS Reread4Num,\n"
    "        CAST(\n"
    "            (CAST(substr(hex(u.RereadValue_SingleModule_5),7,2) AS INTEGER)*16777216) +\n"
    "            (CAST(substr(hex(u.RereadValue_SingleModule_5),5,2) AS INTEGER)*65536) +\n"
    "            (CAST(substr(hex(u.RereadValue_SingleModule_5),3,2) AS INTEGER)*256) +\n"
    "            CAST(substr(hex(u.RereadValue_SingleModule_5),1,2) AS INTEGER)\n"
    "        AS TEXT) AS Reread5Num,\n"
    "        CAST(\n"
    "            (CAST(substr(hex(u.RereadValue_SingleModule_6),7,2) AS INTEGER)*16777216) +\n"
    "            (CAST(substr(hex(u.RereadValue_SingleModule_6),5,2) AS INTEGER)*65536) +\n"
    "            (CAST(substr(hex(u.RereadValue_SingleModule_6),3,2) AS INTEGER)*256) +\n"
    "            CAST(substr(hex(u.RereadValue_SingleModule_6),1,2) AS INTEGER)\n"
    "        AS TEXT) AS Reread6Num,\n"
    "        CAST(\n"
    "            (CAST(substr(hex(u.RereadValue_SingleModule_7),7,2) AS INTEGER)*16777216) +\n"
    "            (CAST(substr(hex(u.RereadValue_SingleModule_7),5,2) AS INTEGER)*65536) +\n"
    "            (CAST(substr(hex(u.RereadValue_SingleModule_7),3,2) AS INTEGER)*256) +\n"
    "            CAST(substr(hex(u.RereadValue_SingleModule_7),1,2) AS INTEGER)\n"
    "        AS TEXT) AS Reread7Num,\n"
    "        CAST(\n"
    "            (CAST(substr(hex(u.RereadValue_SingleModule_8),7,2) AS INTEGER)*16777216) +\n"
    "            (CAST(substr(hex(u.RereadValue_SingleModule_8),5,2) AS INTEGER)*65536) +\n"
    "            (CAST(substr(hex(u.RereadValue_SingleModule_8),3,2) AS INTEGER)*256) +\n"
    "            CAST(substr(hex(u.RereadValue_SingleModule_8),1,2) AS INTEGER)\n"
    "        AS TEXT) AS Reread8Num\n"
    "    FROM main.Data_SavedSettings u\n"
    "    INNER JOIN orders_db.Charger_DeviceModel_SettingParameters cds ON u.Param_Id = cds.Parameter_ID\n"
    "    WHERE u.Device_Id BETWEEN 700 AND 704\n"
    "    AND u.Param_Id IN (\n"
    "        -- EMS CONFIGURATION INFO\n"
    "         2117, 3160, 1127, 2105, 2114,3121,3122,91011,\n"
    "        -- Grid Voltage & Frequency Protection Parameters\n"
    "        2120, 2121, 2122, 2123, 2124, 2125, 2126, 2127, 2128, 2129,\n"
    "        21210, 21211, 21212, 21213, 21214, 21215, 998, 999,\n"
    "        -- Overvoltage/Undervoltage Derating Control Parameters\n"
    "        3126, 3127, 3128, 31138, 31281,313151,\n"
    "        -- Overfrequency/Underfrequency Uprating Control Parameters\n"
    "        3136, 3138, 31133, 3131, 3133, 31132,\n"
    "        -- COSP Mode Power Factor & Control Point Parameters\n"
    "        996, 997, 3174, 3175, 31610, 31611, 31612, 31613, \n"
    "        -- Grid Dynamic Ride-Through & Reconnection Control Parameters\n"
    "        31130, 31131, 3180, 31811, 31812, 31911, 31912,\n"
    "        -- QU Mode Reactive Power Control Parameters\n"
    "        3163, 3164, 3165, 3166, 3167, 3168, 3169, 31614, 31615, 311315, 31158, 31159,\n"
    "        -- Module Reconnection & Startup Parameter\n"
    "         31110, 31111, 3118, 3119, 311101, 311111, 3115\n"
    "    )\n"
    ")\n"
    "SELECT \n"
    "    o.Text_AustrianAuth AS DeviceName,\n"
    "    oo.Text_AustrianAuth AS ParamName,\n"
    "    pvp.Value_Type AS Datatype,\n"
    "    datetime(pvp.Save_Time, 'unixepoch') AS SavedSettingTime_UTC,\n"
    "    CASE pvp.Value_Type\n"
    "        WHEN 'UINT' THEN \n"
    "            CAST(\n"
    "                (CAST(substr(hex(pvp.Current_Value), 1, 2) AS INTEGER) * 16777216) +\n"
    "                (CAST(substr(hex(pvp.Current_Value), 3, 2) AS INTEGER) * 65536) +\n"
    "                (CAST(substr(hex(pvp.Current_Value), 5, 2) AS INTEGER) * 256) +\n"
    "                CAST(substr(hex(pvp.Current_Value), 7, 2) AS INTEGER)\n"
    "            AS TEXT)\n"
    "        WHEN 'FLOAT' THEN '0x' || hex(pvp.Current_Value)\n"
    "        WHEN 'ENUM' THEN COALESCE(\n"
    "            (SELECT r.Text_AustrianAuth FROM orders_db.Resources r \n"
    "             WHERE r.Res_ID = CAST(\n"
    "                 substr(\n"
    "                     pvp.Value_Range,\n"
    "                     instr(pvp.Value_Range, pvp.CurrentValueNum || '(') + length(pvp.CurrentValueNum) + 1,\n"
    "                     instr(\n"
    "                         substr(pvp.Value_Range, instr(pvp.Value_Range, pvp.CurrentValueNum || '(') + length(pvp.CurrentValueNum) + 1),\n"
    "                         ')'\n"
    "                     ) - 1\n"
    "                 ) AS INTEGER)\n"
    "            ), 'unknown enum')\n"
    "        ELSE hex(pvp.Current_Value)\n"
    "    END AS Value,\n"
    "    CASE pvp.Value_Type\n"
    "        WHEN 'UINT' THEN CAST((CAST(substr(hex(pvp.RereadValue_SingleModule_1),1,2) AS INTEGER)*16777216)+(CAST(substr(hex(pvp.RereadValue_SingleModule_1),3,2) AS INTEGER)*65536)+(CAST(substr(hex(pvp.RereadValue_SingleModule_1),5,2) AS INTEGER)*256)+CAST(substr(hex(pvp.RereadValue_SingleModule_1),7,2) AS INTEGER) AS TEXT)\n"
    "        WHEN 'FLOAT' THEN '0x' || hex(pvp.RereadValue_SingleModule_1)\n"
    "        WHEN 'ENUM' THEN COALESCE((SELECT r.Text_AustrianAuth FROM orders_db.Resources r WHERE r.Res_ID = CAST(substr(pvp.Value_Range, instr(pvp.Value_Range, pvp.Reread1Num || '(') + length(pvp.Reread1Num) + 1, instr(substr(pvp.Value_Range, instr(pvp.Value_Range, pvp.Reread1Num || '(') + length(pvp.Reread1Num) + 1), ')') -1) AS INTEGER)), 'unknown enum')\n"
    "        ELSE hex(pvp.RereadValue_SingleModule_1)\n"
    "    END AS RereadValue_1,\n"
    "    CASE pvp.Value_Type\n"
    "        WHEN 'UINT' THEN CAST((CAST(substr(hex(pvp.RereadValue_SingleModule_2),1,2) AS INTEGER)*16777216)+(CAST(substr(hex(pvp.RereadValue_SingleModule_2),3,2) AS INTEGER)*65536)+(CAST(substr(hex(pvp.RereadValue_SingleModule_2),5,2) AS INTEGER)*256)+CAST(substr(hex(pvp.RereadValue_SingleModule_2),7,2) AS INTEGER) AS TEXT)\n"
    "        WHEN 'FLOAT' THEN '0x' || hex(pvp.RereadValue_SingleModule_2)\n"
    "        WHEN 'ENUM' THEN COALESCE((SELECT r.Text_AustrianAuth FROM orders_db.Resources r WHERE r.Res_ID = CAST(substr(pvp.Value_Range, instr(pvp.Value_Range, pvp.Reread2Num || '(') + length(pvp.Reread2Num) + 1, instr(substr(pvp.Value_Range, instr(pvp.Value_Range, pvp.Reread2Num || '(') + length(pvp.Reread2Num) + 1), ')') -1) AS INTEGER)), 'unknown enum')\n"
    "        ELSE hex(pvp.RereadValue_SingleModule_2)\n"
    "    END AS RereadValue_2,\n"
    "    CASE pvp.Value_Type\n"
    "        WHEN 'UINT' THEN CAST((CAST(substr(hex(pvp.RereadValue_SingleModule_3),1,2) AS INTEGER)*16777216)+(CAST(substr(hex(pvp.RereadValue_SingleModule_3),3,2) AS INTEGER)*65536)+(CAST(substr(hex(pvp.RereadValue_SingleModule_3),5,2) AS INTEGER)*256)+CAST(substr(hex(pvp.RereadValue_SingleModule_3),7,2) AS INTEGER) AS TEXT)\n"
    "        WHEN 'FLOAT' THEN '0x' || hex(pvp.RereadValue_SingleModule_3)\n"
    "        WHEN 'ENUM' THEN COALESCE((SELECT r.Text_AustrianAuth FROM orders_db.Resources r WHERE r.Res_ID = CAST(substr(pvp.Value_Range, instr(pvp.Value_Range, pvp.Reread3Num || '(') + length(pvp.Reread3Num) + 1, instr(substr(pvp.Value_Range, instr(pvp.Value_Range, pvp.Reread3Num || '(') + length(pvp.Reread3Num) + 1), ')') -1) AS INTEGER)), 'unknown enum')\n"
    "        ELSE hex(pvp.RereadValue_SingleModule_3)\n"
    "    END AS RereadValue_3,\n"
    "    CASE pvp.Value_Type\n"
    "        WHEN 'UINT' THEN CAST((CAST(substr(hex(pvp.RereadValue_SingleModule_4),1,2) AS INTEGER)*16777216)+(CAST(substr(hex(pvp.RereadValue_SingleModule_4),3,2) AS INTEGER)*65536)+(CAST(substr(hex(pvp.RereadValue_SingleModule_4),5,2) AS INTEGER)*256)+CAST(substr(hex(pvp.RereadValue_SingleModule_4),7,2) AS INTEGER) AS TEXT)\n"
    "        WHEN 'FLOAT' THEN '0x' || hex(pvp.RereadValue_SingleModule_4)\n"
    "        WHEN 'ENUM' THEN COALESCE((SELECT r.Text_AustrianAuth FROM orders_db.Resources r WHERE r.Res_ID = CAST(substr(pvp.Value_Range, instr(pvp.Value_Range, pvp.Reread4Num || '(') + length(pvp.Reread4Num) + 1, instr(substr(pvp.Value_Range, instr(pvp.Value_Range, pvp.Reread4Num || '(') + length(pvp.Reread4Num) + 1), ')') -1) AS INTEGER)), 'unknown enum')\n"
    "        ELSE hex(pvp.RereadValue_SingleModule_4)\n"
    "    END AS RereadValue_4,\n"
    "    CASE pvp.Value_Type\n"
    "        WHEN 'UINT' THEN CAST((CAST(substr(hex(pvp.RereadValue_SingleModule_5),1,2) AS INTEGER)*16777216)+(CAST(substr(hex(pvp.RereadValue_SingleModule_5),3,2) AS INTEGER)*65536)+(CAST(substr(hex(pvp.RereadValue_SingleModule_5),5,2) AS INTEGER)*256)+CAST(substr(hex(pvp.RereadValue_SingleModule_5),7,2) AS INTEGER) AS TEXT)\n"
    "        WHEN 'FLOAT' THEN '0x' || hex(pvp.RereadValue_SingleModule_5)\n"
    "        WHEN 'ENUM' THEN COALESCE((SELECT r.Text_AustrianAuth FROM orders_db.Resources r WHERE r.Res_ID = CAST(substr(pvp.Value_Range, instr(pvp.Value_Range, pvp.Reread5Num || '(') + length(pvp.Reread5Num) + 1, instr(substr(pvp.Value_Range, instr(pvp.Value_Range, pvp.Reread5Num || '(') + length(pvp.Reread5Num) + 1), ')') -1) AS INTEGER)), 'unknown enum')\n"
    "        ELSE hex(pvp.RereadValue_SingleModule_5)\n"
    "    END AS RereadValue_5,\n"
    "    CASE pvp.Value_Type\n"
    "        WHEN 'UINT' THEN CAST((CAST(substr(hex(pvp.RereadValue_SingleModule_6),1,2) AS INTEGER)*16777216)+(CAST(substr(hex(pvp.RereadValue_SingleModule_6),3,2) AS INTEGER)*65536)+(CAST(substr(hex(pvp.RereadValue_SingleModule_6),5,2) AS INTEGER)*256)+CAST(substr(hex(pvp.RereadValue_SingleModule_6),7,2) AS INTEGER) AS TEXT)\n"
    "        WHEN 'FLOAT' THEN '0x' || hex(pvp.RereadValue_SingleModule_6)\n"
    "        WHEN 'ENUM' THEN COALESCE((SELECT r.Text_AustrianAuth FROM orders_db.Resources r WHERE r.Res_ID = CAST(substr(pvp.Value_Range, instr(pvp.Value_Range, pvp.Reread6Num || '(') + length(pvp.Reread6Num) + 1, instr(substr(pvp.Value_Range, instr(pvp.Value_Range, pvp.Reread6Num || '(') + length(pvp.Reread6Num) + 1), ')') -1) AS INTEGER)), 'unknown enum')\n"
    "        ELSE hex(pvp.RereadValue_SingleModule_6)\n"
    "    END AS RereadValue_6,\n"
    "    CASE pvp.Value_Type\n"
    "        WHEN 'UINT' THEN CAST((CAST(substr(hex(pvp.RereadValue_SingleModule_7),1,2) AS INTEGER)*16777216)+(CAST(substr(hex(pvp.RereadValue_SingleModule_7),3,2) AS INTEGER)*65536)+(CAST(substr(hex(pvp.RereadValue_SingleModule_7),5,2) AS INTEGER)*256)+CAST(substr(hex(pvp.RereadValue_SingleModule_7),7,2) AS INTEGER) AS TEXT)\n"
    "        WHEN 'FLOAT' THEN '0x' || hex(pvp.RereadValue_SingleModule_7)\n"
    "        WHEN 'ENUM' THEN COALESCE((SELECT r.Text_AustrianAuth FROM orders_db.Resources r WHERE r.Res_ID = CAST(substr(pvp.Value_Range, instr(pvp.Value_Range, pvp.Reread7Num || '(') + length(pvp.Reread7Num) + 1, instr(substr(pvp.Value_Range, instr(pvp.Value_Range, pvp.Reread7Num || '(') + length(pvp.Reread7Num) + 1), ')') -1) AS INTEGER)), 'unknown enum')\n"
    "        ELSE hex(pvp.RereadValue_SingleModule_7)\n"
    "    END AS RereadValue_7,\n"
    "    CASE pvp.Value_Type\n"
    "        WHEN 'UINT' THEN CAST((CAST(substr(hex(pvp.RereadValue_SingleModule_8),1,2) AS INTEGER)*16777216)+(CAST(substr(hex(pvp.RereadValue_SingleModule_8),3,2) AS INTEGER)*65536)+(CAST(substr(hex(pvp.RereadValue_SingleModule_8),5,2) AS INTEGER)*256)+CAST(substr(hex(pvp.RereadValue_SingleModule_8),7,2) AS INTEGER) AS TEXT)\n"
    "        WHEN 'FLOAT' THEN '0x' || hex(pvp.RereadValue_SingleModule_8)\n"
    "        WHEN 'ENUM' THEN COALESCE((SELECT r.Text_AustrianAuth FROM orders_db.Resources r WHERE r.Res_ID = CAST(substr(pvp.Value_Range, instr(pvp.Value_Range, pvp.Reread8Num || '(') + length(pvp.Reread8Num) + 1, instr(substr(pvp.Value_Range, instr(pvp.Value_Range, pvp.Reread8Num || '(') + length(pvp.Reread8Num) + 1), ')') -1) AS INTEGER)), 'unknown enum')\n"
    "        ELSE hex(pvp.RereadValue_SingleModule_8)\n"
    "    END AS RereadValue_8,\n"
    "    pvp.Unit_Res AS Unit,\n"
    "    pvp.Param_Id AS param_id\n"
    "FROM ParamValueParse pvp\n"
    "INNER JOIN orders_db.Charger_Devices cd ON pvp.Device_Id = cd.Device_Id\n"
    "INNER JOIN orders_db.Resources o ON cd.Name_Res = o.Res_ID\n"
    "INNER JOIN orders_db.Resources oo ON oo.Res_ID = (SELECT cds2.Name_Res FROM orders_db.Charger_DeviceModel_SettingParameters cds2 WHERE cds2.Parameter_ID = pvp.Param_Id)\n"
    "ORDER BY pvp.Param_Id;\n"
    );


    // ===================== 执行SQL查询 =====================
    rc = sqlite3_prepare_v2(data_db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        printf("Failed to prepare statement: %s\n", sqlite3_errmsg(data_db));
        LogOut(MAIN_MODULE_MAIN, "export", LOG_TYPE_MILESTONE, "Failed to prepare statement: %s", sqlite3_errmsg(data_db));
        sqlite3_exec(data_db, "DETACH DATABASE orders_db;", NULL, NULL, NULL);
        sqlite3_close(data_db);
        Close_UnLock_SqliteDB();
        return TRUE;
    }

    // ===================== 创建CSV文件 =====================
    FILE *fp = fopen(TEMP_CSV_PATH, "wb");
    if (fp == NULL)
    {
        printf("Error opening file: %s\n", TEMP_CSV_PATH);
        sqlite3_finalize(stmt);
        sqlite3_exec(data_db, "DETACH DATABASE orders_db;", NULL, NULL, NULL);
        sqlite3_close(data_db);
        Close_UnLock_SqliteDB();
        Make_Web_OutPut(104, "Open CSV file failed!", NULL, NULL);
        return TRUE;
    }

    // 写入BOM头
    unsigned char bom[] = {0xEF, 0xBB, 0xBF};
    fwrite(bom, 1, sizeof(bom), fp);

    // 写入表头
    fprintf(fp, "ParamName, Value");
    for(i = 0; i < nSinglePCSModuleNum; i++)
    {
        fprintf(fp, ", RereadValue_%d", i+1);
    }
    fprintf(fp, ", Unit\n");

    char time_str[64];
    GetCurrentTimeString(time_str, sizeof(time_str));
    
    fprintf(fp, "DEVICE INFO,\n");
    fprintf(fp, "Exporttime, %s", time_str);
    for(i = 0; i < nSinglePCSModuleNum; i++) fprintf(fp, ", ");
    fprintf(fp, ", \n");
    
    //写入系统SN
    if(!Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 1, 57, 255, (void*)&szSysSN))
    {
        fprintf(fp, "System_SN, N/A");
    }
    else
    {
        fprintf(fp, "System_SN, %s", szSysSN);
    }
    for(i = 0; i < nSinglePCSModuleNum; i++) fprintf(fp, ", ");
    fprintf(fp, ", \n");
    //写入设备SN
    fprintf(fp, "Device_SN, %s", allChargerBootInfo.szChargePointSerualNuml);
    for(i = 0; i < nSinglePCSModuleNum; i++) fprintf(fp, ", ");
    fprintf(fp, ", \n");
    
    // 写入软件版本信息
    fprintf(fp, "Software_Version, V%.2f", fSoftWareVersion);
    for(i = 0; i < nSinglePCSModuleNum; i++) fprintf(fp, ", ");
    fprintf(fp, ", \n");
    
    fprintf(fp, "DCDC_Version, V%.2f", fDCDCVer);
    for(i = 0; i < nSinglePCSModuleNum; i++) fprintf(fp, ", ");
    fprintf(fp, ", \n");
    
    fprintf(fp, "PFC_Version, V%.2f", fPFCVer);
    for(i = 0; i < nSinglePCSModuleNum; i++) fprintf(fp, ", ");
    fprintf(fp, ", \n");
    
    fprintf(fp, ",\n");

    const char *param_names[PARAM_ID_CNT_AUSTRIA] = {0};
    const char *param_types[PARAM_ID_CNT_AUSTRIA] = {0};
    const char *param_values[PARAM_ID_CNT_AUSTRIA] = {0};
    const char *param_units[PARAM_ID_CNT_AUSTRIA] = {0};
    const char *param_rereads[PARAM_ID_CNT_AUSTRIA][8] = {{0}};
    int param_exists[PARAM_ID_CNT_AUSTRIA] = {0};

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        int param_id = sqlite3_column_int(stmt, 14);
        
        int idx = -1;
        for (i = 0; i < PARAM_ID_CNT_AUSTRIA; i++)
        {
            if (g_PCS_ParamId_List_Exprot_Austria[i] == param_id)
            {
                idx = i;
                break;
            }
        }
        
        if (idx < 0)
        {
            continue;
        }

        const unsigned char *param_name = sqlite3_column_text(stmt, 1);
        param_names[idx] = param_name ? strdup((const char*)param_name) : strdup("NULL");
        
        const unsigned char *param_type = sqlite3_column_text(stmt, 2);
        param_types[idx] = param_type ? strdup((const char*)param_type) : strdup("NULL");
        
        const unsigned char *unit = sqlite3_column_text(stmt, 13);
        param_units[idx] = unit ? strdup((const char*)unit) : strdup("NULL");
        
        const unsigned char *value = sqlite3_column_text(stmt, 4);
        param_values[idx] = value ? strdup((const char*)value) : strdup("NULL");
        
        for (j = 0; j < nSinglePCSModuleNum && j < 8; j++)
        {
            const unsigned char *reread_val = sqlite3_column_text(stmt, 5 + j);
            param_rereads[idx][j] = reread_val ? strdup((const char*)reread_val) : strdup("NULL");
        }
        param_exists[idx] = 1;
    }

    int param_offset = 0;
    for (g = 0; g < GROUP_COUNT; g++)
    {
        fprintf(fp, "%s,\n", group_info[g].title);
        
        for (p = 0; p < group_info[g].param_count; p++)
        {
            int idx = param_offset + p;
            if (idx >= PARAM_ID_CNT_AUSTRIA)
            {
                fprintf(fp, "ERROR: Index out of bounds idx=%d, PARAM_ID_CNT_AUSTRIA=%d,\n", idx, PARAM_ID_CNT_AUSTRIA);
                break;
            }
            
            int param_id = g_PCS_ParamId_List_Exprot_Austria[idx];
            
            int found_idx = -1;
            for (i = 0; i < PARAM_ID_CNT_AUSTRIA; i++)
            {
                if (param_exists[i] && g_PCS_ParamId_List_Exprot_Austria[i] == param_id)
                {
                    found_idx = i;
                    break;
                }
            }
            
            if (found_idx >= 0 && param_exists[found_idx])
            {
                const char *param_name = param_names[found_idx];
                const char *param_type = param_types[found_idx];
                const char *value = param_values[found_idx];
                const char *unit = param_units[found_idx];
                
                // 写入参数名称
                fprintf(fp, "%s", param_name ? param_name : "NULL");
                
                char strFloat[64] = {0};
                if (param_type && strcmp(param_type, "FLOAT") == 0 && value != NULL && strcmp(value, "NULL") != 0)
                {
                    // 解析 FLOAT 类型的值
                    unsigned int int_value = 0;
                    if (sscanf(value, "0x%X", &int_value) == 1)
                    {
                        float float_value;
                        int_value = SwapEndian32(int_value);
                        memcpy(&float_value, &int_value, sizeof(float));
                        float corrected_value = roundf(float_value * 1000.0f) / 1000.0f;
                        snprintf(strFloat, sizeof(strFloat), "%.3f", corrected_value);
                        fprintf(fp, ",%s", strFloat);
                    }
                    else
                    {
                        // 如果解析失败，显示NULL
                        fprintf(fp, ",NULL");
                    }
                }
                else
                {
                    // 其他类型或NULL值
                    fprintf(fp, ",%s", value ? value : "NULL");
                }
                
                // 写入RereadValues
                for (j = 0; j < nSinglePCSModuleNum; j++)
                {
                    const char *reread_val = param_rereads[found_idx][j];
                    
                    if (param_type && strcmp(param_type, "FLOAT") == 0 && reread_val != NULL && strcmp(reread_val, "NULL") != 0)
                    {
                        // 解析 FLOAT 类型的回读值
                        unsigned int int_val = 0;
                        if (sscanf(reread_val, "0x%X", &int_val) == 1) {
                            float f_val;
                            int_val = SwapEndian32(int_val);
                            memcpy(&f_val, &int_val, sizeof(float));
                            snprintf(strFloat, sizeof(strFloat), "%.3f", f_val);
                            fprintf(fp, ",%s", strFloat);
                        }
                        else
                        {
                            // 如果解析失败，显示NULL
                            fprintf(fp, ",NULL");
                        }
                    }
                    else
                    {
                        // 其他类型或NULL值
                        fprintf(fp, ",%s", reread_val ? reread_val : "NULL");
                    }
                }
                
                // 写入单位列
                if (unit && strcmp(unit, "NULL") != 0)
                {
                    int unit_id = atoi(unit);
                    if (unit_id > 0 && unit_id < RESOURCE_ITEM_TOTAL)
                    {
                        const char* unit_name = pFCGI_Rough->ResItem[unit_id].szResName;
                        if (unit_name && unit_name[0] != '\0')
                        {
                            fprintf(fp, ",%s\n", unit_name);
                        }
                        else
                        {
                            fprintf(fp, ",\n");
                        }
                    }
                    else
                    {
                        fprintf(fp, ",\n");
                    }
                }
                else
                {
                    fprintf(fp, ",\n");
                }
            }
            else
            {
                // 如果参数不存在，显示参数ID和NULL
                fprintf(fp, "Param_ID_%d,NULL", param_id);
                for (j = 0; j < nSinglePCSModuleNum; j++)
                {
                    fprintf(fp, ",NULL");
                }
                // 单位列为空
                fprintf(fp, ",\n");
            }
        }
        
        fprintf(fp, ",\n");
        param_offset += group_info[g].param_count;
    }

    fclose(fp);

    for (i = 0; i < PARAM_ID_CNT_AUSTRIA; i++)
    {
        if (param_names[i] && strcmp(param_names[i], "NULL") != 0)
        {
            free((void*)param_names[i]);
        }
        if (param_types[i] && strcmp(param_types[i], "NULL") != 0)
        {
            free((void*)param_types[i]);
        }
        if (param_values[i] && strcmp(param_values[i], "NULL") != 0)
        {
            free((void*)param_values[i]);
        }
        if (param_units[i] && strcmp(param_units[i], "NULL") != 0)
        {
            free((void*)param_units[i]);
        }
        for (j = 0; j < 8; j++)
        {
            if (param_rereads[i][j] && strcmp(param_rereads[i][j], "NULL") != 0)
            {
                free((void*)param_rereads[i][j]);
            }
        }
    }

    // ===================== 执行系统命令 =====================
    char szSysCmd[256];
    snprintf(szSysCmd, sizeof(szSysCmd), "cd /opt/appdata/data; datasavedsettings %s", TEMP_CSV_FILE);
    TRACEX("---Now begin run the system cmd[%s]---", szSysCmd);
    system(szSysCmd);

    // ===================== 上传文件 =====================
    if(!UploadFile_2Client(TEMP_CSV_PATH, TEMP_CSV_FILE))
    {
        unlink(TEMP_CSV_PATH);
        Make_Web_OutPut(104, "Upload process is FAILED!", NULL, NULL);
        sqlite3_finalize(stmt);
        sqlite3_exec(data_db, "DETACH DATABASE orders_db;", NULL, NULL, NULL);
        sqlite3_close(data_db);
        Close_UnLock_SqliteDB();
        return FALSE;
    }

    unlink(TEMP_CSV_PATH);
    sqlite3_finalize(stmt);
    sqlite3_exec(data_db, "DETACH DATABASE orders_db;", NULL, NULL, NULL);
    sqlite3_close(data_db);
    Close_UnLock_SqliteDB();
    return TRUE;
}

BOOL webpage_handler_export_pcs(FCGI_ROUGH_DATA* pFCGI_Rough)
{ 
    TRACE("\n----export_pcs page_handler was called!---\n");
    if(pFCGI_Rough->emHttpReqMethod == HTTP_REQMETHOD_GET)
    {
        TRACE("webpage_handler_export_pcs:HTTP_REQMETHOD_GET\n");
        return  Handle_PCSExportFile2_Download(pFCGI_Rough);;
    }

    else
    {
        TRACE("webpage_handler_export_pcs:HTTP_REQMETHOD_POST\n");
        return TRUE;
       //Handle_PCSExportFile2_Download(pFCGI_Rough);
    }
    return TRUE;
}