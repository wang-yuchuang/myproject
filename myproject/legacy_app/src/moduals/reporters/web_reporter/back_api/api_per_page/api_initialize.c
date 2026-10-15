// #define _TRACE_PRINT_
#include "../fcgi_defs.h"

typedef struct tagStaticDictioneryItem
{
    char    szFrontName[64];
    int     nResId;
}STATIC_DICT_ITEM;

#define ADD_STRJ_ITEM(_p, _name, _resId) cJSON_AddStringToObject((_p), (_name), LoadLangText_ById(_resId))
#define ADD_STRJ_ARRAY(_p, _name, _total, _pnResId) \
            do{ \
                cJSON* pJArr = cJSON_CreateArray();\
                int n; \
                for(n = 0; n < (_total); n++){ \
                    cJSON* pJ = cJSON_CreateString(LoadLangText_ById(*((_pnResId) + n))); \
                    cJSON_AddItemToArray(pJArr, pJ);\
                }\
                cJSON_AddItemToObject((_p), (_name), pJArr);\
            }while(0)
#define ADD_OBJ_ARRAY(_p, _name, _arrIt)\
            do{ \
                int n;\
                cJSON* pJArr = cJSON_CreateArray(); \
                for(n = 0; n < ITEM_OF(_arrIt); n++) \
                { \
                    cJSON* pJ = cJSON_CreateObject(); \
                    ADD_STRJ_ITEM(pJ, "name", _arrIt[n].val);  \
                    cJSON_AddNumberToObject(pJ, "val", _arrIt[n].key);\
                    cJSON_AddItemToArray(pJArr, pJ);\
                }\
                cJSON_AddItemToObject((_p), (_name), pJArr); \
            }while(0)            

#define ADD_OBJ_STR_ARRAY(_p, _name, _arrIt, flag)\
            do{ \
                int n;\
                cJSON* pJArr = cJSON_CreateArray(); \
                for(n = 0; n < ITEM_OF(_arrIt); n++) \
                { \
                    cJSON* pJ = cJSON_CreateObject(); \
                    if(flag)\
                    {\
                        ADD_STRJ_ITEM(pJ, "label", _arrIt[n].label);  \
                        cJSON_AddStringToObject(pJ, "value", _arrIt[n].value);\
                    }\
                    else\
                    {\
                        ADD_STRJ_ITEM(pJ, "title", _arrIt[n].label);  \
                        cJSON_AddStringToObject(pJ, "dataIndex", _arrIt[n].value);\
                    }\
                    cJSON_AddItemToArray(pJArr, pJ);\
                }\
                if(flag)\
                 {\
                    cJSON_AddItemToObject((_p), (_name), pJArr); \
                 }\
                else\
                 {\
                    cJSON_AddItemToArray((_p), pJArr); \
                 }\
            }while(0)  

static void Make_Static_DevParam_DicRes(FCGI_ROUGH_DATA* pFCGI_Rough, cJSON* pJDict)
{
    int aaaaa, n, m, k,SSSS;
    WEB_PARAM_VALUE* pParamVal;
    WEB_RUN_DEV* pRunDev = pFCGI_Rough->chargerDataRepo.pAllRunDevs;
    int nTotal;
    for(aaaaa = 0; aaaaa < pFCGI_Rough->chargerDataRepo.nDevInfoTotal; aaaaa++, pRunDev++)
    {
        //dev name
        Add_Trans_Into_Dictionary(pJDict, pRunDev->pDevInfo->nDevName_ResId);  
        //param's names
        for(SSSS = 0; SSSS < 3; SSSS++)
        {
            pParamVal = (SSSS == 0) ? pRunDev->pStatusParamList : 
                ((SSSS == 1) ? pRunDev->pSettParamList : pRunDev->pCtrlParamList);
            nTotal = (SSSS == 0) ? pRunDev->nStatusParamTotal :
                ((SSSS == 1) ? pRunDev->nSettParamTotal : pRunDev->nCtrlParamTotal);
            for(n = 0; n < nTotal; n++, pParamVal++)
            {
                Add_Trans_Into_Dictionary(pJDict, pParamVal->pParamInfo->nParamName_ResId);
                if(pParamVal->pParamInfo->nParamUnit_ResId > 0)
                {
                    Add_Trans_Into_Dictionary(pJDict, pParamVal->pParamInfo->nParamUnit_ResId);
                }
                //now for enum & object type
                if(pParamVal->pParamInfo->emValType == WEB_VALTYPE_ENUM)
                {
                    for(m = 0; m < pParamVal->pParamInfo->nEnumTotal; m++)
                    {
                        Add_Trans_Into_Dictionary(pJDict, pParamVal->pParamInfo->enumList[m].nEnumName_ResId);
                    }
                }
                else if(pParamVal->pParamInfo->emValType == WEB_VALTYPE_OBJECT)
                {
                    A_WEBBLOB_PARAM* pBlobParam = Get_ABlobParam_ByParamVal(pParamVal);
                    if(pBlobParam)
                    {
                        WEB_SUBBLOB_ITEM* pSubBlob = pBlobParam->pSubBlobList;
                        for(m = 0; m < pBlobParam->nSubBlobTotal; m++, pSubBlob++)
                        {
                            Add_Trans_Into_Dictionary(pJDict, pSubBlob->nName_ResId);
                            if(pSubBlob->nUnit_ResId > 0)
                            {
                                Add_Trans_Into_Dictionary(pJDict, pSubBlob->nUnit_ResId);
                            }
                            //for enum type
                            if(GetSubBlobParam_ValType(pSubBlob) == WEB_VALTYPE_ENUM)
                            {
                                for(k = 0; k < pSubBlob->nEnumTotal; k++)
                                {
                                    Add_Trans_Into_Dictionary(pJDict, pSubBlob->enumList[k].nEnumName_ResId);
                                }
                            }
                        }
                    }//if-pBlobParam              
                }//if-emValType
            }//for-n
        }//forSSS
    }//for-aaaaa
}

static void Load_DictioneryData(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    cJSON* pDict = cJSON_CreateObject();
	int n;
    //first add dic_data for all device/parameters
    cJSON* pJDictData = cJSON_CreateObject();
    cJSON* pJArr;
    Make_Static_DevParam_DicRes(pFCGI_Rough, pJDictData);
    // if (pFCGI_Rough->emSystemType >= SYSTYPE_SCADA_INPCS)
    // {
    //     cJSON_AddItemToObject(pDict, "dict_pcs_data", pJDictData);
    // }
    // else
    {
        cJSON_AddItemToObject(pDict, "dict_data", pJDictData);
    }
    // LJH241101 MU3 // LJH250205 REMS add DEV_CLASS_SEMS
    switch (pFCGI_Rough->emSystemType)
    {
    case SYSTYPE_EMS_STATION:
    // case SYSTYPE_SCADA_LOCALCTRL:
    {
        INT_KEYVAL allClasses[] = 
        {
            {DEV_CLASS_CHARGER_SYSTEM,          93},
            {DEV_CLASS_CCU,                     196},
            {DEV_CLASS_METERS,                  1041},
            {DEV_CLASS_UART_SERVER,             2493},//edit by kuangyue 20251029
            {DEV_CLASS_SEMS,                    2663},
            {DEV_CLASS_CABINET,                 22},
            {DEV_CLASS_PV_INVERTER,             8000},

        };
        ADD_OBJ_ARRAY(pDict, "dev_classes", allClasses);
    }
        break;

    // case SYSTYPE_SCADA_INPCS:
    // {
    //     INT_KEYVAL allClasses[] = 
    //     {
    //         {DEV_CLASS_CHARGER_SYSTEM,          93},
    //         {DEV_CLASS_CCU,                     196},

    //         {DEV_CLASS_SINGLE_PM,               1111},
    //         {DEV_CLASS_PCS_GROUP,               590},
    //     };
    //     ADD_OBJ_ARRAY(pDict, "dev_classes", allClasses);
    // }
    //     break;
    
    default:
    {
        // LJH250408 CtrlByREMS
        INT_KEYVAL allClasses[] = 
        {
            {DEV_CLASS_CHARGER_SYSTEM,          93},
            {DEV_CLASS_CCU,                     196},
            {DEV_CLASS_METERS,                  1041},
            {DEV_CLASS_BATTERY,                 491},
            {DEV_CLASS_BATTCAB_IMEU2,           902},

            {DEV_CLASS_POWER_IMEU,              6064},
            {DEV_CLASS_SINGLE_PM,               1111},
            {DEV_CLASS_PCS_GROUP,               590},
            {DEV_CLASS_UART_SERVER,             2493},
            {DEV_CLASS_TEMP_CONTROL,            2495},

            {DEV_CLASS_AIR_COND,                5507},
            {DEV_CLASS_LIQUIDCOOLING_PACK,      6005},
            {DEV_CLASS_PV_INVERTER,             8000},

            {DEV_CLASS_SEMS,                    2663},
        };
        ADD_OBJ_ARRAY(pDict, "dev_classes", allClasses);
    }
        break;
    }

    //-----------1. general----------
    cJSON* pJGeneral = cJSON_CreateObject();
    ADD_STRJ_ITEM(pJGeneral, "web_mgmt_sys",                    1046);    
    ADD_STRJ_ITEM(pJGeneral, "btn_log_out",                     1045);  
    //menus // LJH241101 MU3
    // int resIds_SysType[SYSTYPE_MAX] = {7203, 7203, 2663, 2664, 2665, 2666};
    int resIds_SysType[SYSTYPE_MAX] = {7203, 7203, 2663};
    ADD_STRJ_ITEM(pJGeneral, "menu_chargermgmt",                resIds_SysType[pFCGI_Rough->emSystemType]);   
    ADD_STRJ_ITEM(pJGeneral, "menu_quickset",                   /* 1036 */7190);    
    ADD_STRJ_ITEM(pJGeneral, "menu_detailinfo",                 238);
    ADD_STRJ_ITEM(pJGeneral, "menu_datachart",                  5664);
    ADD_STRJ_ITEM(pJGeneral, "menu_paramset",                   239);
    ADD_STRJ_ITEM(pJGeneral, "refresh",                         1065);
    ADD_STRJ_ITEM(pJGeneral, "batch_refresh",                   1070);
    ADD_STRJ_ITEM(pJGeneral, "export_pcs",                      1071);
    ADD_STRJ_ITEM(pJGeneral, "select_limit_warning",            3711);
    ADD_STRJ_ITEM(pJGeneral, "import_pcs",                      3712);
    ADD_STRJ_ITEM(pJGeneral, "import_bms",                      6367);

    ADD_STRJ_ITEM(pJGeneral, "menu_base",                       /* 2757 */239);
    ADD_STRJ_ITEM(pJGeneral, "menu_authentication",             2758);
    ADD_STRJ_ITEM(pJGeneral, "menu_battery",                    2759);
    ADD_STRJ_ITEM(pJGeneral, "menu_other",                      2760);
    ADD_STRJ_ITEM(pJGeneral, "menu_special",                    2761);
    ADD_STRJ_ITEM(pJGeneral, "menu_monitor",                    2762);
    
    ADD_STRJ_ITEM(pJGeneral, "menu_manualctrl",                 240);
    ADD_STRJ_ITEM(pJGeneral, "menu_dataservice",                1037);
    ADD_STRJ_ITEM(pJGeneral, "menu_firmwareupdt",               1038);
    ADD_STRJ_ITEM(pJGeneral, "menu_usermgmt",                   1029);
    ADD_STRJ_ITEM(pJGeneral, "menu_moreinformation",            6016);
    ADD_STRJ_ITEM(pJGeneral, "menu_runinfo",                    237); 
    ADD_STRJ_ITEM(pJGeneral, "query_total_items",               1015);
    ADD_STRJ_ITEM(pJGeneral, "hint_pleasewait",                 243);
    ADD_STRJ_ITEM(pJGeneral, "btn_OK",                          48);  
    ADD_STRJ_ITEM(pJGeneral, "btn_cancel",                      49);  
    //ADD_STRJ_ITEM(pJGeneral, "btn_save",                        1009);
    //ADD_STRJ_ITEM(pJGeneral, "btn_reset",                       355);
    ADD_STRJ_ITEM(pJGeneral, "form_search_btn",                 332);
    ADD_STRJ_ITEM(pJGeneral, "form_search_label",               1050);
    ADD_STRJ_ITEM(pJGeneral, "form_btn_save",                   1009);
    ADD_STRJ_ITEM(pJGeneral, "form_btn_exe",                    1009);
    ADD_STRJ_ITEM(pJGeneral, "form_btn_edit",                   1054);
    ADD_STRJ_ITEM(pJGeneral, "form_btn_reset",                  1058);
    ADD_STRJ_ITEM(pJGeneral, "form_validate_range",             252);
    ADD_STRJ_ITEM(pJGeneral, "menu_dataservice_charge",         103);
    ADD_STRJ_ITEM(pJGeneral, "menu_dataservice_history_alarm",  241);
    ADD_STRJ_ITEM(pJGeneral, "menu_dataservice_history",        1021);
    //ADD_STRJ_ITEM(pJGeneral, "menu_dataservice_history_data", 1021);
   //ADD_STRJ_ITEM(pJGeneral, "menu_dataservice_history_event", 1020);
    ADD_STRJ_ITEM(pJGeneral, "menu_dataservice_system",         1016);
    ADD_STRJ_ITEM(pJGeneral, "menu_dataservice_black_white",    354);
    ADD_STRJ_ITEM(pJGeneral, "menu_dataservice_charging_profile",    177);
    ADD_STRJ_ITEM(pJGeneral, "menu_dataservice_backup_restore",    353);
    ADD_STRJ_ITEM(pJGeneral, "menu_firmwareupdt_current",          1039);
    ADD_STRJ_ITEM(pJGeneral, "menu_firmwareupdt_history",          1034);
    ADD_STRJ_ITEM(pJGeneral, "menu_firmwareupdt_rectifier",        1074);
    ADD_STRJ_ITEM(pJGeneral, "form_upload_placeholder",            1042);
    ADD_STRJ_ITEM(pJGeneral, "form_select_placeholder",            1018);
    ADD_STRJ_ITEM(pJGeneral, "form_search_placeholder",            1018);
    ADD_STRJ_ITEM(pJGeneral, "common_waiting",                     243);
    ADD_STRJ_ITEM(pJGeneral, "form_exe_success",                   249);    
    ADD_STRJ_ITEM(pJGeneral, "form_upload_validate",               1042);
    ADD_STRJ_ITEM(pJGeneral, "common_error_network",               1056);
    ADD_STRJ_ITEM(pJGeneral, "form_validate_regexp",               245);
    ADD_STRJ_ITEM(pJGeneral, "form_validate_format_error",         245);    
    ADD_STRJ_ITEM(pJGeneral, "form_validate_integer",              245);    
    ADD_STRJ_ITEM(pJGeneral, "form_validate_float_0",              245);
    ADD_STRJ_ITEM(pJGeneral, "form_exe_confirm",                   1062);
    //for pcs monitor begins
    ADD_STRJ_ITEM(pJGeneral, "menu_firmware_pcs",                  429);
    ADD_STRJ_ITEM(pJGeneral, "menu_firmware_ems",                  1039);
    ADD_STRJ_ITEM(pJGeneral, "menu_fault",                         6895);
    ADD_STRJ_ITEM(pJGeneral, "menu_oscilloscope",                  6896);
    //for pcs monitor ends
    //for 404-page
    ADD_STRJ_ITEM(pJGeneral, "error_404_text",                     1064);
    ADD_STRJ_ITEM(pJGeneral, "error_404_back",                     63);
    ADD_STRJ_ITEM(pJGeneral, "error_404_refresh",                  1065);
    ADD_STRJ_ITEM(pJGeneral, "empty",                              2198);

    ADD_STRJ_ITEM(pJGeneral, "menu_system",                  7101);
    ADD_STRJ_ITEM(pJGeneral, "menu_ems",                     7102);
    ADD_STRJ_ITEM(pJGeneral, "menu_bms",                     7103);
    ADD_STRJ_ITEM(pJGeneral, "menu_pcs",                     7104);
    ADD_STRJ_ITEM(pJGeneral, "menu_mppt",                    7105);
    ADD_STRJ_ITEM(pJGeneral, "menu_meter",                   1041);
    ADD_STRJ_ITEM(pJGeneral, "menu_heat",                    7107);
    ADD_STRJ_ITEM(pJGeneral, "menu_firefighting",            7108);
    ADD_STRJ_ITEM(pJGeneral, "menu_other",                   7109);
    ADD_STRJ_ITEM(pJGeneral, "menu_emsmanager",              805);
    ADD_STRJ_ITEM(pJGeneral, "menu_alarm",                   7137);
    ADD_STRJ_ITEM(pJGeneral, "menu_operation",               7138);
    ADD_STRJ_ITEM(pJGeneral, "operation",                    1010);
    ADD_STRJ_ITEM(pJGeneral, "menu_info",                    7139);
    ADD_STRJ_ITEM(pJGeneral, "menu_firmwareupdt_current",    7140);
    ADD_STRJ_ITEM(pJGeneral, "menu_firmwareupdt_history",    7141);
    ADD_STRJ_ITEM(pJGeneral, "menu_bmsparamset",             6358);
    ADD_STRJ_ITEM(pJGeneral, "menu_bmsfault",                6368);
    ADD_STRJ_ITEM(pJGeneral, "menu_bmsevent",                9472);
    ADD_STRJ_ITEM(pJGeneral, "form_issue_btn",               332);

     if(pFCGI_Rough->emSystemType == SYSTYPE_EMS_STATION)
     {
        static INT_KEYVAL s_menu_info_list[] = {{0, 7101}, {1, 7102},/* {2, 7103},{3, 7104}, */{4, 7105},{5, 1041},/* {6, 7107},{7, 7108}, */{8, 7109}/* , {9, 315} */};
        static INT_KEYVAL s_menu_set_list[] = {{0, 7101}, {1, 7102},/* {2, 7103},{3, 7104}, */{4, 7105},{5, 1041}/* ,{6, 7107},{7, 7108}*/,{8, 7109} };
        ADD_OBJ_ARRAY(pJGeneral, "menu_info_list", s_menu_info_list);
	    ADD_OBJ_ARRAY(pJGeneral, "menu_set_list", s_menu_set_list);
    }
    else
    {
        static INT_KEYVAL s_menu_info_list[] = {{0, 7101}, {1, 7102},{2, 7103},{3, 7104},{4, 7105},{5, 1041},{6, 7107},{7, 7108},{8, 7109}, {9, 315}};
        static INT_KEYVAL s_menu_set_list[] = {{0, 7101}, {1, 7102},{2, 7103},{3, 7104},{4, 7105},{5, 1041},{6, 7107},{7, 7108},{8, 7109}};
        ADD_OBJ_ARRAY(pJGeneral, "menu_info_list", s_menu_info_list);
	    ADD_OBJ_ARRAY(pJGeneral, "menu_set_list", s_menu_set_list);
    }

    cJSON_AddItemToObject(pDict, "dict_general", pJGeneral);  

    cJSON* s_dict_paramset = cJSON_CreateObject();
    ADD_STRJ_ITEM(s_dict_paramset, "table_currentVal",                      332);
    ADD_STRJ_ITEM(s_dict_paramset, "table_operate",                      332);
    ADD_STRJ_ITEM(s_dict_paramset, "table_configVal",                     1009);
    cJSON_AddItemToObject(pDict, "dict_bmsparamset", s_dict_paramset);

    //---------2. login page--------
    cJSON* pJLogin = cJSON_CreateObject();
    ADD_STRJ_ITEM(pJLogin, "login_hint",                      69);
    ADD_STRJ_ITEM(pJLogin, "btn_login",                       48);
    ADD_STRJ_ITEM(pJLogin, "input_user",                      53);
    ADD_STRJ_ITEM(pJLogin, "input_password",                  64);
    ADD_STRJ_ITEM(pJLogin, "wrong_password_hint",             406);
    cJSON_AddStringToObject(pJLogin, "input_password_placeholder",  "");
    cJSON_AddStringToObject(pJLogin, "input_root_key",  "root_key");  
    ADD_STRJ_ITEM(pJLogin, "logout_confirm",                  1044);
    cJSON_AddItemToObject(pDict, "dict_login", pJLogin);   

    //---------3. home page--------
    cJSON* pJHome = cJSON_CreateObject();
    ADD_STRJ_ITEM(pJHome, "sys_datetime",                    167);
    ADD_STRJ_ITEM(pJHome, "btn_adjust_time",                 1001);
    ADD_STRJ_ITEM(pJHome, "sys_runstat",                     129);
    ADD_STRJ_ITEM(pJHome, "sys_alarmstatus",                 553); // Resource Changed by Jeffery 20240712
    ADD_STRJ_ITEM(pJHome, "server_status",                   130); // Resource Changed by Jeffery 20240712
    ADD_STRJ_ITEM(pJHome, "status_ok",                       34);
    // ADD_STRJ_ITEM(pJHome, "immu2_ver",                       125);//edit by Kuangyue
    // ADD_STRJ_ITEM(pJHome, "imsud_ver",                       319);
    // ADD_STRJ_ITEM(pJHome, "rectifier_ver",                   1077);
    ADD_STRJ_ITEM(pJHome, "total_kwh",                       1075);
    ADD_STRJ_ITEM(pJHome, "total_times",                     1049);
    ADD_STRJ_ITEM(pJHome, "statistics_type",                 415);
    ADD_STRJ_ITEM(pJHome, "current_total_output_power",      1055);
    static INT_KEYVAL s_StatTypes[] = 
    {
        {0,             992},
        {1,             993},
        {2,             994},
        {10,            995},
        {11,            996},
        {12,            997}
    };
	ADD_OBJ_ARRAY(pJHome, "statistics_types", s_StatTypes);
    //ADD_STRJ_ARRAY(pJHome, "statistics_types", 6, nStatTypes);   
    static INT_KEYVAL s_SelTimeTypes[] = { {0, 998}, {1, 999},{2, 1000} };
	ADD_OBJ_ARRAY(pJHome, "time_select_types", s_SelTimeTypes);
    //x-axis --- hour
    static char* sz24Hr[] = {"0:00", "1:00", "2:00","3:00","4:00","5:00","6:00","7:00",
                        "8:00", "9:00", "10:00", "11:00","12:00","13:00","14:00","15:00",
                        "16:00","17:00","18:00", "19:00", "20:00","21:00","22:00","23:00"};
    pJArr = cJSON_CreateArray();
    for(n = 0; n < 24; n++)
    { 
        cJSON* pJ = cJSON_CreateString(sz24Hr[n]); 
        cJSON_AddItemToArray(pJArr, pJ);
    }
    cJSON_AddItemToObject(pJHome, "x_axis_hour", pJArr);
    //x-axis --- week
    int nWeekNames[] = {1002, 1003, 1004, 1005, 1006, 1007, 1008};
    ADD_STRJ_ARRAY(pJHome, "x_axis_week", 7, nWeekNames);   
    //x-axis --- month
    static char* sz31Days[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10",
                        "11", "12", "13", "14", "15", "16", "17", "18", "19", "20",
                        "21", "22", "23", "24", "25", "26", "27", "28", "29", "30", "31"};

    pJArr = cJSON_CreateArray();
    for(n = 0; n < 31; n++)
    { 
        cJSON* pJ = cJSON_CreateString(sz31Days[n]); 
        cJSON_AddItemToArray(pJArr, pJ);
    }
    cJSON_AddItemToObject(pJHome, "x_axis_month", pJArr);
    //active alarms
    ADD_STRJ_ITEM(pJHome, "title_active_alm_table",                35);
    ADD_STRJ_ITEM(pJHome, "table_idx",                             109);
    ADD_STRJ_ITEM(pJHome, "table_almlevel",                        110);
    ADD_STRJ_ITEM(pJHome, "table_almdesp",                         112);
    ADD_STRJ_ITEM(pJHome, "table_almbegin",                        113);
    ADD_STRJ_ITEM(pJHome, "table_almstat",                         273);    
    ADD_STRJ_ITEM(pJHome, "alarm_started",                         368);
    ADD_STRJ_ITEM(pJHome, "alarm_ended",                           369);
    ADD_STRJ_ITEM(pJHome, "btn_adjust_time",                       1043);
    ADD_STRJ_ITEM(pJHome, "sys_datetime_confirm",                  48);
    ADD_STRJ_ITEM(pJHome, "sys_datetime_current_time",             167);
    ADD_STRJ_ITEM(pJHome, "sys_datetime_current_timezone",         504);
    ADD_STRJ_ITEM(pJHome, "sys_datetime_success",                  249);
    ADD_STRJ_ITEM(pJHome, "enable",                                323);
    ADD_STRJ_ITEM(pJHome, "disable",                               324);
    ADD_STRJ_ITEM(pJHome, "set_up",                                1009);

    ADD_STRJ_ITEM(pJHome, "topology",                              2070);
    ADD_STRJ_ITEM(pJHome, "scene_graph",                           2072);
    ADD_STRJ_ITEM(pJHome, "energy_stack",                          2373);
    ADD_STRJ_ITEM(pJHome, "no device",                             84);
    ADD_STRJ_ITEM(pJHome, "off_grid",                              2073);

    if(pFCGI_Rough->emSystemType == SYSTYPE_EMS_STATION)
    {
        static INT_KEYVAL home_tabs[] = {{0, 2070}};
        ADD_OBJ_ARRAY(pJHome, "home_tabs", home_tabs);
    }
    else
    {
        static INT_KEYVAL home_tabs[] = {{0, 2070}, {1, 2373},{2, 2072},{3, 2073}};
        ADD_OBJ_ARRAY(pJHome, "home_tabs", home_tabs);
    }

    cJSON_AddItemToObject(pDict, "dict_home", pJHome);  

    //---------4. quick_set page--------
    cJSON* pJQuickSet = cJSON_CreateObject();
    //int nQMenus[] = {169, 523, 404}; //404 means bussiness-admin
    //ADD_STRJ_ARRAY(pJQuickSet, "quick_menus", 3, nQMenus); 
    //static INT_KEYVAL s_QMenus[] = { {0, 169}, {1, 1072}, {2, 1063}, {3, 834} };
    static INT_KEYVAL s_QMenus[] = { {3, 834} };

	ADD_OBJ_ARRAY(pJQuickSet, "quick_menus", s_QMenus);
    //ADD_STRJ_ITEM(pJQuickSet, "btn_save",                         1009);
    //ADD_STRJ_ITEM(pJQuickSet, "btn_restore",                      355);
    cJSON_AddItemToObject(pDict, "dict_quickset", pJQuickSet);   

    //---------5. detail_info page--------
    cJSON* pJDetailInfo = cJSON_CreateObject();
    ADD_STRJ_ITEM(pJDetailInfo, "table_idx",                             109);
    ADD_STRJ_ITEM(pJDetailInfo, "table_paramName",                       250);
    ADD_STRJ_ITEM(pJDetailInfo, "table_paramVal",                        251);
    cJSON_AddItemToObject(pDict, "dict_detailinfo", pJDetailInfo);   

    //---------6. param_set page--------
    cJSON* pJParamSet = cJSON_CreateObject();
    ADD_STRJ_ITEM(pJParamSet, "table_idx",                             109);
    ADD_STRJ_ITEM(pJParamSet, "table_paramName",                       250);
    ADD_STRJ_ITEM(pJParamSet, "table_paramVal",                        251);
    ADD_STRJ_ITEM(pJParamSet, "table_rereadVal",                       3706);
    ADD_STRJ_ITEM(pJParamSet, "table_commandNumber",                   3713);
    ADD_STRJ_ITEM(pJParamSet, "table_operate",                         1010);
    ADD_STRJ_ITEM(pJParamSet, "table_paramRange",                      2196);
    ADD_STRJ_ITEM(pJParamSet, "table_paramDefault",                    2197);
    //ADD_STRJ_ITEM(pJParamSet, "table_btn_set",                         1009);
    cJSON_AddItemToObject(pDict, "dict_paramset", pJParamSet);   

    //---------7. manual_ctrl page--------
    cJSON* pJManCtrl = cJSON_CreateObject();
    ADD_STRJ_ITEM(pJManCtrl, "table_idx",                             109);
    ADD_STRJ_ITEM(pJManCtrl, "table_paramName",                       250);
    ADD_STRJ_ITEM(pJManCtrl, "table_operate",                         1010);
    ADD_STRJ_ITEM(pJManCtrl, "table_btn_set",                         1009);
    cJSON_AddItemToObject(pDict, "dict_manctrl", pJManCtrl);   

    //---------8. charge_records page--------
    cJSON* pJChgRec = cJSON_CreateObject();
    ADD_STRJ_ITEM(pJChgRec, "title_chgrec",                          103);
    ADD_STRJ_ITEM(pJChgRec, "btn_exort",                             1011);
    ADD_STRJ_ITEM(pJChgRec, "btn_import",                            1023);
    ADD_STRJ_ITEM(pJChgRec, "condition_time",                        256);
    static INT_KEYVAL s_ChgRecCondTime[] = { {0, 826}, {1, 832},{2, 1037},{3, 849},{4, 256} };
	ADD_OBJ_ARRAY(pJChgRec, "list_cond_time", s_ChgRecCondTime);
    ADD_STRJ_ITEM(pJChgRec, "title_cond_gun",                        256);
    static INT_KEYVAL s_ChgRecCondGun[] = { {0, 494}, {1, 711},{2, 708},{3, 799},{4, 976},{5, 518},{6, 587},{7, 805},{8, 1446},{9, 2097},{10, 7000},{11, 7012},{12, 7013} };
	ADD_OBJ_ARRAY(pJChgRec, "list_cond_gun", s_ChgRecCondGun);
    ADD_STRJ_ITEM(pJChgRec, "title_cond_startmethod",                256);
    static INT_KEYVAL s_ChgRecCondStartMeth[] = { {0, 256}, {1, 256},{2, 256} };
	ADD_OBJ_ARRAY(pJChgRec, "list_cond_startmethod", s_ChgRecCondStartMeth);
    ADD_STRJ_ITEM(pJChgRec, "search_input_hint",                       36);
    //ADD_STRJ_ITEM(pJChgRec, "btn_search",                              332);
    ADD_STRJ_ITEM(pJChgRec, "btn_export_meter",                        1011);
	ADD_STRJ_ITEM(pJChgRec, "btn_export_charge",                       1105);
    //table
    ADD_STRJ_ITEM(pJChgRec, "table_idx",                             109);
    ADD_STRJ_ITEM(pJChgRec, "table_sn",                              1345);
    ADD_STRJ_ITEM(pJChgRec, "table_connId",                          274);
    ADD_STRJ_ITEM(pJChgRec, "table_startMethod",                     256);
    ADD_STRJ_ITEM(pJChgRec, "table_chargeMode",                      256);
    ADD_STRJ_ITEM(pJChgRec, "table_soc",                             256);
    ADD_STRJ_ITEM(pJChgRec, "table_usedkWh",                         256);
    ADD_STRJ_ITEM(pJChgRec, "table_usedTime",                        256);
    ADD_STRJ_ITEM(pJChgRec, "table_stopReason",                      256);
    ADD_STRJ_ITEM(pJChgRec, "table_beginTime",                       256);
    ADD_STRJ_ITEM(pJChgRec, "table_endTime",                         256);
    ADD_STRJ_ITEM(pJChgRec, "import_success",                        249);
    cJSON_AddStringToObject(pJChgRec, "filename_error", "The file name does not match, please reselect the file!");
    cJSON_AddItemToObject(pDict, "dict_chargerec", pJChgRec);   

    //---------9. his_alarm page--------
    cJSON* pJHisAlarm = cJSON_CreateObject();
    ADD_STRJ_ITEM(pJHisAlarm, "title_hisalarm",                      241);
    ADD_STRJ_ITEM(pJHisAlarm, "condition_time",                      274);
    static INT_KEYVAL s_HisAlmCondTime[] = { {0, 998}, {1, 337},{2, 338},{3, 339} };
	ADD_OBJ_ARRAY(pJHisAlarm, "list_cond_time", s_HisAlmCondTime);
    ADD_STRJ_ITEM(pJHisAlarm, "title_cond_level",                        110);
    static INT_KEYVAL s_HisAlmCondLevel[] = { {0, 998}, {1, 4},{2, 5},{3, 6} };
	ADD_OBJ_ARRAY(pJHisAlarm, "list_cond_level", s_HisAlmCondLevel);
    ADD_STRJ_ITEM(pJHisAlarm, "title_cond_source",                112);
    static INT_KEYVAL s_HisAlmSource[] = { {0, 998}, {1, 93},{2, 196},{3, 1041},{4, 98},{5, 1040} };
	ADD_OBJ_ARRAY(pJHisAlarm, "list_cond_source", s_HisAlmSource);
    static INT_KEYVAL Status[] = { {0, 6890}, {1, 6891},{2, 6892}};
    ADD_OBJ_ARRAY(pJHisAlarm, "list_cond_status", Status);
    //table
    ADD_STRJ_ITEM(pJHisAlarm, "table_idx",                             109);
    ADD_STRJ_ITEM(pJHisAlarm, "table_level",                           110);
    ADD_STRJ_ITEM(pJHisAlarm, "table_source",                          112);
    ADD_STRJ_ITEM(pJHisAlarm, "table_name",                            111);
    ADD_STRJ_ITEM(pJHisAlarm, "table_beginTime",                       113);
    ADD_STRJ_ITEM(pJHisAlarm, "table_endTime",                         116);
    ADD_STRJ_ITEM(pJHisAlarm, "table_code",                            6886);
    ADD_STRJ_ITEM(pJHisAlarm, "table_status",                          6887);
    ADD_STRJ_ITEM(pJHisAlarm, "table_duration",                        6888);
    ADD_STRJ_ITEM(pJHisAlarm, "mark",                                  6893);
    ADD_STRJ_ITEM(pJHisAlarm, "mark_tip",                              6894);

    cJSON_AddItemToObject(pDict, "dict_hisalarm", pJHisAlarm);   

    //---------10. sys_log page--------
    cJSON* pJSysLogs = cJSON_CreateObject();
    ADD_STRJ_ITEM(pJSysLogs, "sys_log",                                1016);
    ADD_STRJ_ITEM(pJSysLogs, "ocpp_log",                               1017);
    ADD_STRJ_ITEM(pJSysLogs, "keyworkd_sort_hint",                     1018);
    //table
    ADD_STRJ_ITEM(pJSysLogs, "table_idx",                             109);
    ADD_STRJ_ITEM(pJSysLogs, "table_content",                         1019);
    cJSON_AddItemToObject(pDict, "dict_syslogs", pJSysLogs);   

    //---------11. his_event&his_data page--------
    cJSON* pJHisDatas = cJSON_CreateObject();
    ADD_STRJ_ITEM(pJHisDatas, "history_event",                          1020);
    ADD_STRJ_ITEM(pJHisDatas, "history_data",                           1021);
    ADD_STRJ_ITEM(pJHisDatas, "condition_time",                         274);
    //int nHisDataCondTime[] = {598, 337, 338, 339};
    //ADD_STRJ_ARRAY(pJHisDatas, "list_cond_time", 4, nHisDataCondTime); 
    static INT_KEYVAL s_HisDataCondTime[] = { {0, 998}, {1, 337},{2, 338},{3, 339} };
	ADD_OBJ_ARRAY(pJHisDatas, "list_cond_time", s_HisDataCondTime);
    ADD_STRJ_ITEM(pJHisDatas, "condition_event_type",                   1022);
    // static char* szEventType[] = { "All",
    //        "System Exit", "Firmware Update", "Diagnostic Upload",
    //        "Enter Maintain Page", "Ability Change", "Reserve for Charging", "Other"
    //     };
    // pJArr = cJSON_CreateArray();
    // for(n = 0; n < 8; n++)
    // { 
    //     //cJSON* pJ = cJSON_CreateString(szEventType[n]); 
    //     cJSON* pJ = cJSON_CreateObject(); 
    //     cJSON_AddStringToObject(pJ, "name", szEventType[n]);  
    //     cJSON_AddNumberToObject(pJ, "val", n);
    //     cJSON_AddItemToArray(pJArr, pJ);
    // }
    // cJSON_AddItemToObject(pJHisDatas, "list_cond_event_type", pJArr);
    static INT_KEYVAL s_HisEvCondTypes[] = { {0, 998}, {1, 1025}, {2, 358},{3, 1026},{4, 1027}, {5, 1028}, {6, 68}, {7, 1040}};
	ADD_OBJ_ARRAY(pJHisDatas, "list_cond_event_type", s_HisEvCondTypes);
    ADD_STRJ_ITEM(pJHisDatas, "condition_data_source",                  112);
	// static char* szHisDataSrc[] = {"All", "Connector-1", "Connector-2", "kWh-Meter-1", "kWh-Meter-2"};
    // pJArr = cJSON_CreateArray();
    // for(n = 0; n < 5; n++)
    // { 
    //     //cJSON* pJ = cJSON_CreateString(szHisDataSrc[n]); 
    //     cJSON* pJ = cJSON_CreateObject(); 
    //     cJSON_AddStringToObject(pJ, "name", szHisDataSrc[n]);  
    //     cJSON_AddNumberToObject(pJ, "val", n);
    //     cJSON_AddItemToArray(pJArr, pJ);
    // }
    // cJSON_AddItemToObject(pJHisDatas, "list_cond_data_src", pJArr);
    static INT_KEYVAL s_HisHisDataSrcTypes[] = { {0, 998}, {1, 18}, {2, 19}, {3, 96}, {4, 97}};
	ADD_OBJ_ARRAY(pJHisDatas, "list_cond_data_src", s_HisHisDataSrcTypes);
    ADD_STRJ_ITEM(pJHisDatas, "condition_data_type",                    1022);
    // static char* szHisDataType[] = {"All",  "Voltage(V)", "Current(A)"};
    // pJArr = cJSON_CreateArray();
    // for(n = 0; n < 3; n++)
    // { 
    //     //cJSON* pJ = cJSON_CreateString(szHisDataType[n]); 
    //     cJSON* pJ = cJSON_CreateObject(); 
    //     cJSON_AddStringToObject(pJ, "name", szHisDataType[n]);  
    //     cJSON_AddNumberToObject(pJ, "val", n);
    //     cJSON_AddItemToArray(pJArr, pJ);
    // }
    // cJSON_AddItemToObject(pJHisDatas, "list_cond_data_type", pJArr);
    static INT_KEYVAL s_HisHisDataTypes[] = { {0, 998}, {1, 326}, {2, 364}};
	ADD_OBJ_ARRAY(pJHisDatas, "list_cond_data_type", s_HisHisDataTypes);
    //table
    ADD_STRJ_ITEM(pJHisDatas, "table_idx",                              109);
    ADD_STRJ_ITEM(pJHisDatas, "table_eventType",                        1022);
    ADD_STRJ_ITEM(pJHisDatas, "table_eventStatus",                      273);
    ADD_STRJ_ITEM(pJHisDatas, "table_beginTime",                        113);
    ADD_STRJ_ITEM(pJHisDatas, "table_endTime",                          116);
    ADD_STRJ_ITEM(pJHisDatas, "table_dataSource",                       112);
    ADD_STRJ_ITEM(pJHisDatas, "table_dataContent",                      1019);
    cJSON_AddItemToObject(pDict, "dict_hisdatas", pJHisDatas);   

    //---------12. locallist & localcahe page--------
    cJSON* pJLocalAuth = cJSON_CreateObject();
    ADD_STRJ_ITEM(pJLocalAuth, "blackwhite_list_mgmt",                  1047);
    ADD_STRJ_ITEM(pJLocalAuth, "local_cache_mgmt",                      1048);
    ADD_STRJ_ITEM(pJLocalAuth, "version",                               1061);
    ADD_STRJ_ITEM(pJLocalAuth, "btn_exort",                             1011);
    ADD_STRJ_ITEM(pJLocalAuth, "btn_import",                            1023);
    //static INT_KEYVAL s_DiabledEnableAuth[] = { {0, 324}, {1, 323} };
	//ADD_OBJ_ARRAY(pJLocalAuth, "btn_enable_disable", s_DiabledEnableAuth);
    ADD_STRJ_ITEM(pJLocalAuth, "btn_disable",                           324);
    ADD_STRJ_ITEM(pJLocalAuth, "btn_enable",                            323);
    ADD_STRJ_ITEM(pJLocalAuth, "keyworkd_sort_hint",                    1018);
    //ADD_STRJ_ITEM(pJLocalAuth, "btn_search",                            332);
    ADD_STRJ_ITEM(pJLocalAuth, "btn_clearlocalcache",                   254);
    cJSON_AddStringToObject(pJLocalAuth, "table_idTag", "idTag"); 
    cJSON_AddStringToObject(pJLocalAuth, "table_parentIdTag", "parentIdTag"); 
    cJSON_AddStringToObject(pJLocalAuth, "valid_status", "status"); 
    cJSON_AddStringToObject(pJLocalAuth, "expire_date", "expire-date"); 
    ADD_STRJ_ITEM(pJLocalAuth, "enabled_success",                       249); 
    ADD_STRJ_ITEM(pJLocalAuth, "disabled_success",                      249); 
    ADD_STRJ_ITEM(pJLocalAuth, "clear_cache_success",                   249); 
    ADD_STRJ_ITEM(pJLocalAuth, "import_success",                        249); 
    cJSON_AddStringToObject(pJLocalAuth, "status", "status"); 
    cJSON_AddItemToObject(pDict, "dict_blackwhite", pJLocalAuth); 

    //---------13. data backup & restore page--------
    cJSON* pJDataBakRest = cJSON_CreateObject();
    ADD_STRJ_ITEM(pJDataBakRest, "backup_restore_mgmt",                  353);
    ADD_STRJ_ITEM(pJDataBakRest, "btn_backcup",                          354);
    ADD_STRJ_ITEM(pJDataBakRest, "btn_restore",                          1052);
    ADD_STRJ_ITEM(pJDataBakRest, "btn_restore_setting",                  355);
    ADD_STRJ_ITEM(pJDataBakRest, "btn_restore_all",                      998);
    ADD_STRJ_ITEM(pJDataBakRest, "btn_screenshot",                       1051);
    ADD_STRJ_ITEM(pJDataBakRest, "screenshot_success",                   249); 
    ADD_STRJ_ITEM(pJDataBakRest, "restore_success",                      249); 
    cJSON_AddItemToObject(pDict, "dict_backuprestore", pJDataBakRest); 

    //---------14. firmware mgmt page--------
    cJSON* pJFirmwareMgmt = cJSON_CreateObject();
    ADD_STRJ_ITEM(pJFirmwareMgmt, "firmware_mgmt",                        265);
    ADD_STRJ_ITEM(pJFirmwareMgmt, "menu_controller_updt",                 93);
    ADD_STRJ_ITEM(pJFirmwareMgmt, "menu_rectifier_updt",                  1074);
    ADD_STRJ_ITEM(pJFirmwareMgmt, "title_current_verInfo",                125);
    ADD_STRJ_ITEM(pJFirmwareMgmt, "title_history_verInfo",                1021);
    ADD_STRJ_ITEM(pJFirmwareMgmt, "title_sw_ver",                         125);
    ADD_STRJ_ITEM(pJFirmwareMgmt, "title_hw_ver",                         1057);
    ADD_STRJ_ITEM(pJFirmwareMgmt, "title_lccu_ver",                       319);
    ADD_STRJ_ITEM(pJFirmwareMgmt, "btn_upgrade",                          265);
    ADD_STRJ_ITEM(pJFirmwareMgmt, "btn_update",                           265);
    ADD_STRJ_ITEM(pJFirmwareMgmt, "title_dc_ver",                         468);
    ADD_STRJ_ITEM(pJFirmwareMgmt, "title_pfc_ver",                        469);
    ADD_STRJ_ITEM(pJFirmwareMgmt, "title_online_total",                   972);
    ADD_STRJ_ITEM(pJFirmwareMgmt, "title_rect_updt_mode",                 980);
    static INT_KEYVAL s_RectUpdtMode[] = { {0, 429},  {1, 978}};
	ADD_OBJ_ARRAY(pJFirmwareMgmt, "list_rect_updt_mode", s_RectUpdtMode);
    ADD_STRJ_ITEM(pJFirmwareMgmt, "title_rect_updt_type",                 983);
    static INT_KEYVAL s_RectUpdtType[] = {{0,986}, {1,985},{2,984},{3,2352},{4,2353},{5,328},{6,329},{7,2750},{8,2751},{9,6634},{10,6635}};
	ADD_OBJ_ARRAY(pJFirmwareMgmt, "list_rect_updt_type", s_RectUpdtType);
    ADD_STRJ_ITEM(pJFirmwareMgmt, "title_select_targetFile",              1024);
    ADD_STRJ_ITEM(pJFirmwareMgmt, "title_dc_targetVer",                   468);
    ADD_STRJ_ITEM(pJFirmwareMgmt, "title_pfc_targetVer",                  990);
    ADD_STRJ_ITEM(pJFirmwareMgmt, "title_rect_updt_loading",              1073);
    ADD_STRJ_ITEM(pJFirmwareMgmt, "update_success",                       249); 
    ADD_STRJ_ITEM(pJFirmwareMgmt, "tip_incompatibility",                  2394); 
    ADD_STRJ_ITEM(pJFirmwareMgmt, "tip_SameVertion",                      2395); 
    ADD_STRJ_ITEM(pJFirmwareMgmt, "title_CurrentVersion",                 2503); 
    ADD_STRJ_ITEM(pJFirmwareMgmt, "title_TargetVersion",                  988); 

    ADD_STRJ_ITEM(pJFirmwareMgmt, "title_oneclick_targetFile",            1024);  //目标文件
    ADD_STRJ_ITEM(pJFirmwareMgmt, "oneclick_mgmt",                        6677);  //一键升级按钮

    ADD_STRJ_ITEM(pJFirmwareMgmt, "title_version",                         125);
    ADD_STRJ_ITEM(pJFirmwareMgmt, "version_query",                         6682); //版本查询按钮

    cJSON_AddItemToObject(pDict, "dict_firmwaremgmt", pJFirmwareMgmt); 

    //---------15. user management page--------
    cJSON* pJUserMgmt = cJSON_CreateObject();
    ADD_STRJ_ITEM(pJUserMgmt, "user_mgmt",                            1029);
    ADD_STRJ_ITEM(pJUserMgmt, "user_engineer",                        403);
    ADD_STRJ_ITEM(pJUserMgmt, "user_admin",                           404);
    ADD_STRJ_ITEM(pJUserMgmt, "btn_change_password",                  1053);
    cJSON_AddStringToObject(pJUserMgmt, "user_root", "root");
    ADD_STRJ_ITEM(pJUserMgmt, "last_login",                           1030);
    ADD_STRJ_ITEM(pJUserMgmt, "title_oldpasswd",                      1031);
    ADD_STRJ_ITEM(pJUserMgmt, "title_newpasswd",                      1032);
    ADD_STRJ_ITEM(pJUserMgmt, "title_confirmpasswd",                  1033);
    ADD_STRJ_ITEM(pJUserMgmt, "validate_old",                         1031);
    ADD_STRJ_ITEM(pJUserMgmt, "validate_new",                         1032);
    ADD_STRJ_ITEM(pJUserMgmt, "validate_new_confirm",                 1033);
    //ADD_STRJ_ITEM(pJUserMgmt, "validate_format",                      245);
    ADD_STRJ_ITEM(pJUserMgmt, "validate_format",                      245);
    ADD_STRJ_ITEM(pJUserMgmt, "validate_new_diff",                    1033);
    ADD_STRJ_ITEM(pJUserMgmt, "title_login_status",                   273);
    cJSON_AddItemToObject(pDict, "dict_usermgmt", pJUserMgmt); 

    //---------16. run_info page--------
    // cJSON* pJRunInfo = cJSON_CreateObject();
    // static INT_KEYVAL s_RunMenus[] = { {0, 976}, {1, 1445}, {2, 1111}};
	// ADD_OBJ_ARRAY(pJRunInfo, "runinfo_menus", s_RunMenus);

    cJSON* pJRunInfo = cJSON_CreateObject();
    if (pFCGI_Rough->emSystemType == SYSTYPE_EMS_STATION)  /*站控系统*/
    {
        static INT_KEYVAL s_RunMenus_SEMS[] = { {0, 3901}, {1, 7104}};
        ADD_OBJ_ARRAY(pJRunInfo, "runinfo_menus", s_RunMenus_SEMS);

        cJSON* runinfo_columns = cJSON_CreateArray();
        static STR_KEYVAL s_columns_0[] = {{"grp", 109}, {"rate_pwr", 118}, {"rate_curr", 3990}, {"max_curr", 123}, \
                                           {"mode", 1920}, {"ctrl_volt", 115}, {"ctrl_curr", 1445}, {"rtime_volt", 121},\
                                           {"rtime_curr", 454}, {"online", 2805}};
                                           
        ADD_OBJ_STR_ARRAY(runinfo_columns, "children_0", s_columns_0, FALSE);

        static STR_KEYVAL s_columns_1[] = {{"Grp", 590}, {"Index", 109},{"version", 125}, {"barcode", 315}};

        ADD_OBJ_STR_ARRAY(runinfo_columns, "children_1", s_columns_1, FALSE);

        cJSON_AddItemToObject(pJRunInfo, "runinfo_columns", runinfo_columns);

        ADD_STRJ_ITEM(pJRunInfo, "table_grp",                             109);   // 编号
        ADD_STRJ_ITEM(pJRunInfo, "table_rate_pwr",                        118);   // IP Address
        ADD_STRJ_ITEM(pJRunInfo, "table_rate_curr",                       3990);  // 有功功率需求
        ADD_STRJ_ITEM(pJRunInfo, "table_max_curr",                        123);   // 最大输出功率
        ADD_STRJ_ITEM(pJRunInfo, "table_mode",                            1920);  // 工作模式
        ADD_STRJ_ITEM(pJRunInfo, "table_ctrl_volt",                       115);   // SOC
        ADD_STRJ_ITEM(pJRunInfo, "table_ctrl_curr",                       1445);  // 功率分配
        ADD_STRJ_ITEM(pJRunInfo, "table_rtime_volt",                      121);   // 输出电压
        ADD_STRJ_ITEM(pJRunInfo, "table_rtime_curr",                      454);   // 输出电流
        ADD_STRJ_ITEM(pJRunInfo, "table_online",                          2805);  // 输出功率
    }
    else
    {
        // static INT_KEYVAL s_RunMenus[] = { {0, 976}, {1, 1445}, {2, 1111}, {3, 9002}};
        static INT_KEYVAL s_RunMenus[] = {
            {0, 9002}, // 所有模块}
            {1, 1445}, // 功率分配
            {2, 9012}, // ACDC组
            {3, 9013}, // DCDC组
            {4, 9014}, // MPPT组
            {5, 1111}, // 单模块
        };

        ADD_OBJ_ARRAY(pJRunInfo, "runinfo_menus", s_RunMenus);

        cJSON* runinfo_columns = cJSON_CreateArray();
        /*所有模块*/
        static STR_KEYVAL s_columns_0[] = {{"mode", 9003}, {"ctrl", 465}, {"reason", 9004}, {"grp_cfg", 9005}, {"grp_ol", 9006}};

        ADD_OBJ_STR_ARRAY(runinfo_columns, "children_0", s_columns_0, FALSE);
        /*功率分配*/
        static STR_KEYVAL s_columns_1[] = {{"idx", 109}, {"paramName", 250}, {"paramVal1", 305}, {"paramVal2", 306}, \
                                           {"paramVal3", 307}, {"paramVal4", 308}};

        ADD_OBJ_STR_ARRAY(runinfo_columns, "children_1", s_columns_1, FALSE);
        /*ACDC组*/
        static STR_KEYVAL s_columns_2[] = {{"grp", 109}, {"ol_num", 2385}, {"rated_power", 2381}, {"rated_curr", 2382}, \
                                           {"max_curr", 122}, {"ctrl_stats", 9033}, {"ctrl_dc_volt", 9015}, \
                                           {"ctrl_dc_curr", 9016}, {"ctrl_power", 9017}, {"run_stats", 9018}, \
                                           {"rt_dc_volt", 9019}, {"ctrl_dc_tol_curr", 9020}, {"rt_sys_power", 9021}};

        ADD_OBJ_STR_ARRAY(runinfo_columns, "children_2", s_columns_2, FALSE);

        /*DCDC组*/
        static STR_KEYVAL s_columns_3[] = {{"grp", 109}, {"ol_num", 2385}, {"rated_power", 2381}, {"rated_curr", 2382}, \
                                           {"max_curr", 122}, {"ctrl_stats", 9033}, {"ctrl_volt", 2383}, \
                                           {"ctrl_batt_cutoff_volt", 9022}, {"ctrl_batt_curr_lmt", 9023},  \
                                           {"ctrl_bus_curr_lmt", 9024}, {"run_stats", 9018}, {"rt_batt_volt", 9025}, \
                                           {"rt_batt_tol_curr", 9026}, {"rt_bus_volt", 9027}};

        ADD_OBJ_STR_ARRAY(runinfo_columns, "children_3", s_columns_3, FALSE);

        /*MPPT组*/
        static STR_KEYVAL s_columns_4[] = {{"grp", 109}, {"ol_num", 2385}, {"rated_power", 2381}, {"rated_curr", 2382}, \
                                           {"max_curr", 122}, {"run_stats", 9018}, {"sys_volt", 9028}, \
                                           {"sys_tol_curr", 9029}};

        ADD_OBJ_STR_ARRAY(runinfo_columns, "children_4", s_columns_4, FALSE);

        /*单模块*/
        static STR_KEYVAL s_columns_5[] = {{"grp", 455}, {"addr", 2390}, /* {"comm_status", 273}, */ {"p", 159}, \
                                           {"q", 160}, {"out_ui", 1702}, {"in_ui", 1701}, {"temp", 610},\
                                           {"freq", 143}, {"pfc_dcdc_ver", 1703}, {"err_code", 30},  {"bar_code", 315},\
                                           };

        ADD_OBJ_STR_ARRAY(runinfo_columns, "children_5", s_columns_5, FALSE);

#if 0

        static STR_KEYVAL s_columns_0[] = {{"grp", 455}, {"rate_pwr", 2381}, {"rate_curr", 2382}, {"max_curr", 122}, \
                                           {"mode", 1920}, {"ctrl_volt", 2383}, {"ctrl_curr", 2384}, {"rtime_volt", 453},\
                                           {"rtime_curr", 454}, {"online", 2385}};
        ADD_OBJ_STR_ARRAY(runinfo_columns, "children_0", s_columns_0, FALSE);


        static STR_KEYVAL s_columns_1[] = {{"idx", 109}, {"paramName", 250}, {"paramVal1", 305}, {"paramVal2", 306}, \
                                           {"paramVal3", 307}, {"paramVal4", 308}};

        ADD_OBJ_STR_ARRAY(runinfo_columns, "children_1", s_columns_1, FALSE);

        static STR_KEYVAL s_columns_2[] = {{"grp", 455}, {"addr", 2390}, /* {"comm_status", 273}, */ {"p", 159}, \
                                           {"q", 160}, {"out_ui", 1702}, {"in_ui", 1701}, {"temp", 610},\
                                           {"freq", 143}, {"pfc_dcdc_ver", 1703}, {"err_code", 30},  {"bar_code", 315},\
                                           };

        ADD_OBJ_STR_ARRAY(runinfo_columns, "children_2", s_columns_2, FALSE);

        // static STR_KEYVAL s_columns_3[] = {{"mode", 9003}, {"ctrl", 465}, {"reason", 9004}, {"grp_cfg", 9005}, {"grp_ol", 9006}};

        // ADD_OBJ_STR_ARRAY(runinfo_columns, "children_3", s_columns_3, FALSE);

#endif


        cJSON_AddItemToObject(pJRunInfo, "runinfo_columns", runinfo_columns);

        ADD_STRJ_ITEM(pJRunInfo, "table_grp",                             455);
        ADD_STRJ_ITEM(pJRunInfo, "table_rate_pwr",                        2381);
        ADD_STRJ_ITEM(pJRunInfo, "table_rate_curr",                       2382);
        ADD_STRJ_ITEM(pJRunInfo, "table_max_curr",                        122);
        ADD_STRJ_ITEM(pJRunInfo, "table_mode",                            1920);
        ADD_STRJ_ITEM(pJRunInfo, "table_ctrl_volt",                       2383);
        ADD_STRJ_ITEM(pJRunInfo, "table_ctrl_curr",                       2384);
        ADD_STRJ_ITEM(pJRunInfo, "table_rtime_volt",                      453);
        ADD_STRJ_ITEM(pJRunInfo, "table_rtime_curr",                      454);
        ADD_STRJ_ITEM(pJRunInfo, "table_online",                          2385);

        ADD_STRJ_ITEM(pJRunInfo, "table_idx",                             109);
        ADD_STRJ_ITEM(pJRunInfo, "table_paramName",                       250);
        //ADD_STRJ_ITEM(pJRunInfo, "table_paramVal",                        251);
        ADD_STRJ_ITEM(pJRunInfo, "table_paramVal1",                       305);
        ADD_STRJ_ITEM(pJRunInfo, "table_paramVal2",                       306);
        ADD_STRJ_ITEM(pJRunInfo, "table_paramVal3",                       307);
        ADD_STRJ_ITEM(pJRunInfo, "table_paramVal4",                       308);

        ADD_STRJ_ITEM(pJRunInfo, "table_addr",                            2390);
        ADD_STRJ_ITEM(pJRunInfo, "table_comm_status",                     273);
        ADD_STRJ_ITEM(pJRunInfo, "table_p",                               159);
        ADD_STRJ_ITEM(pJRunInfo, "table_q",                               160);
        ADD_STRJ_ITEM(pJRunInfo, "table_out_ui",                          1702);
        ADD_STRJ_ITEM(pJRunInfo, "table_in_ui",                           1701);
        ADD_STRJ_ITEM(pJRunInfo, "table_temp",                            610);
        ADD_STRJ_ITEM(pJRunInfo, "table_freq",                            143);
        ADD_STRJ_ITEM(pJRunInfo, "table_err_code",                        30);
        ADD_STRJ_ITEM(pJRunInfo, "table_pfc_dcdc_ver",                    1703);
        ADD_STRJ_ITEM(pJRunInfo, "table_bar_code",                        315);
        ADD_STRJ_ITEM(pJRunInfo, "grp_title",                             456);
        ADD_STRJ_ITEM(pJRunInfo, "details",                               238);
    }
    cJSON_AddItemToObject(pDict, "dict_runinfo", pJRunInfo);

    cJSON* pAlarm = cJSON_CreateObject();

    static INT_KEYVAL s_alarm_tabs[] = {{0, 35}, {1, 241}};
    ADD_OBJ_ARRAY(pAlarm, "alarm_tabs", s_alarm_tabs);

    ADD_STRJ_ITEM(pAlarm, "condition_time",    274);

    static INT_KEYVAL s_list_cond_time[] = {{0, 998}, {1, 337}, {2, 338}, {3, 339}};
    ADD_OBJ_ARRAY(pAlarm, "list_cond_time", s_list_cond_time);
    ADD_STRJ_ITEM(pAlarm, "title_cond_level",    110);


    static INT_KEYVAL s_list_cond_level[] = {{0, 998}, {1, 4}, {2, 5}, {3, 6}};
    ADD_OBJ_ARRAY(pAlarm, "list_cond_level", s_list_cond_level);

    ADD_STRJ_ITEM(pAlarm, "title_cond_source",    112);

    static INT_KEYVAL s_list_cond_source[] = {{0, 998}, {1, 93}, {2, 196}, {3, 1041}, {4, 98}, {5, 1040}};
    ADD_OBJ_ARRAY(pAlarm, "list_cond_source", s_list_cond_source);

    ADD_STRJ_ITEM(pAlarm, "table_idx",          109);
    ADD_STRJ_ITEM(pAlarm, "table_level",        110);
    ADD_STRJ_ITEM(pAlarm, "table_source",       112);
    ADD_STRJ_ITEM(pAlarm, "table_name",         111);
    ADD_STRJ_ITEM(pAlarm, "table_beginTime",    113);
    ADD_STRJ_ITEM(pAlarm, "table_endTime",      116);
    ADD_STRJ_ITEM(pAlarm, "table_code",         6886);
    ADD_STRJ_ITEM(pAlarm, "table_status",       6887);
    ADD_STRJ_ITEM(pAlarm, "table_duration",     6888);
    //ADD_STRJ_ITEM(pAlarm, "title_cond_pcs",    112);
    ADD_STRJ_ITEM(pAlarm, "process",            7205);
    ADD_STRJ_ITEM(pAlarm, "process_tip",        6894);
    ADD_STRJ_ITEM(pAlarm, "omit",               7206);
    ADD_STRJ_ITEM(pAlarm, "omit_tip",           7207);

    static INT_KEYVAL s_list_cond_status[] = {{0, 6890}, {1, 6891}, {2, 6892}};
    ADD_OBJ_ARRAY(pAlarm, "list_cond_status", s_list_cond_status);

    cJSON_AddItemToObject(pDict, "dict_alarm", pAlarm);

    cJSON* pOperationlog = cJSON_CreateObject();

    ADD_STRJ_ITEM(pOperationlog, "title_cond_system",           7208);

    cJSON* list_cond_system = cJSON_CreateArray();
    cJSON_AddItemToObject(pOperationlog, "list_cond_system", list_cond_system);
    
    cJSON* condem_EMS = cJSON_CreateObject();
    cJSON_AddStringToObject(condem_EMS, "label", "EMS");
    cJSON_AddStringToObject(condem_EMS, "value", "0-0");

    static STR_KEYVAL s_EMS_children[] = {{"0-1", 103}, {"0-2", 849}, {"0-3", 711}, {"0-4", 708}, 
                                           {"0-5", 799}, {"0-6", 976}, {"0-7", 518}, {"0-8", 587},
                                           {"0-9", 805}, {"0-10", 1446}, {"0-11", 2097}, {"0-12", 7013}};
    ADD_OBJ_STR_ARRAY(condem_EMS, "children", s_EMS_children, TRUE);
    cJSON_AddItemToArray(list_cond_system, condem_EMS);

    cJSON* condem_BMS = cJSON_CreateObject();
    cJSON_AddStringToObject(condem_BMS, "label", "BMS");
    cJSON_AddStringToObject(condem_BMS, "value", "1-0");

    static STR_KEYVAL s_BMS_children[] = {{"1-1", 832}, {"1-2", 7012}, {"1-3", 7000}, {"1-4", 7005}, {"1-5", 6887}, {"1-6", 9444}};
    ADD_OBJ_STR_ARRAY(condem_BMS, "children", s_BMS_children, TRUE);

    cJSON_AddItemToArray(list_cond_system, condem_BMS);

    cJSON* real_PCS = cJSON_CreateObject();
    cJSON_AddStringToObject(real_PCS, "label", "PCS");
    cJSON_AddStringToObject(real_PCS, "value", "2-0");
    cJSON_AddItemToArray(list_cond_system, real_PCS);

    ADD_STRJ_ITEM(pOperationlog, "title_cond_time",           274);
    ADD_STRJ_ITEM(pOperationlog, "table_idx",           109);
    ADD_STRJ_ITEM(pOperationlog, "table_date",           7209);
    ADD_STRJ_ITEM(pOperationlog, "table_filename",           7210);

    cJSON_AddItemToObject(pDict, "dict_operationlog", pOperationlog);
    
    cJSON* Other = cJSON_CreateObject();

    ADD_STRJ_ITEM(Other, "screenshot",               7211);
    ADD_STRJ_ITEM(Other, "btn_screenshot",           1051);
    ADD_STRJ_ITEM(Other, "backup",                   354);
    ADD_STRJ_ITEM(Other, "recover",                  1052);
    ADD_STRJ_ITEM(Other, "btn_restore_setting",      355);
    ADD_STRJ_ITEM(Other, "btn_restore_all",          998);
    ADD_STRJ_ITEM(Other, "certification",            177);
    cJSON_AddStringToObject(Other, "filename_error", "The file name does not match, please reselect the file!");
    
    cJSON_AddItemToObject(pDict, "dict_other", Other);
    
    cJSON* firmware = cJSON_CreateObject();

    ADD_STRJ_ITEM(firmware, "table_idx",                    109);
    ADD_STRJ_ITEM(firmware, "table_firmware_name",          7212);
    ADD_STRJ_ITEM(firmware, "table_old_version",            7213);
    ADD_STRJ_ITEM(firmware, "table_new_version",            7214);
    ADD_STRJ_ITEM(firmware, "table_source",                 7215);
    ADD_STRJ_ITEM(firmware, "table_start_time",             113);
    ADD_STRJ_ITEM(firmware, "table_end_time",               116);
    ADD_STRJ_ITEM(firmware, "table_result",                 7216);
    ADD_STRJ_ITEM(firmware, "table_version",                2503);
    ADD_STRJ_ITEM(firmware, "table_target_file",            1024);
    ADD_STRJ_ITEM(firmware, "upgrade",                      7218);

    static INT_KEYVAL s_list_cond_result[] = {{0, 7219}, {1, 249}};
    ADD_OBJ_ARRAY(firmware, "list_cond_result", s_list_cond_result);

    cJSON_AddItemToObject(pDict, "dict_firmware", firmware);
    {
        //---------17. data chart page--------
        cJSON* pJDataChart = cJSON_CreateObject();
        static INT_KEYVAL s_DataChartMenus[] = { {0, 826}, {1, 827}, {2, 830}, {3, 6017}, {4, 6019}, {5, 1446}};
        ADD_OBJ_ARRAY(pJDataChart, "datachart_menus", s_DataChartMenus);
        //ADD_STRJ_ITEM(pJDataChart, "details",                               238);

        //---  Stat Graph Histogram DataInfo ---//
        cJSON* pJSStatGraphHistogramTotalInfo = cJSON_CreateArray();

        cJSON* pJSStatGraphHistogramInfo = cJSON_CreateObject();
        cJSON_AddStringToObject(pJSStatGraphHistogramInfo, "name", "AC Input");
        cJSON_AddNumberToObject(pJSStatGraphHistogramInfo, "val", 0);
        cJSON_AddItemToArray(pJSStatGraphHistogramTotalInfo, pJSStatGraphHistogramInfo);

        pJSStatGraphHistogramInfo = cJSON_CreateObject();
        cJSON_AddStringToObject(pJSStatGraphHistogramInfo, "name", "Mppt_1 Output");
        cJSON_AddNumberToObject(pJSStatGraphHistogramInfo, "val", 1);
        cJSON_AddItemToArray(pJSStatGraphHistogramTotalInfo, pJSStatGraphHistogramInfo);

        pJSStatGraphHistogramInfo = cJSON_CreateObject();
        cJSON_AddStringToObject(pJSStatGraphHistogramInfo, "name", "Mppt_2 Output");
        cJSON_AddNumberToObject(pJSStatGraphHistogramInfo, "val", 2);
        cJSON_AddItemToArray(pJSStatGraphHistogramTotalInfo, pJSStatGraphHistogramInfo);

        pJSStatGraphHistogramInfo = cJSON_CreateObject();
        cJSON_AddStringToObject(pJSStatGraphHistogramInfo, "name", "Batt_1 Charged");
        cJSON_AddNumberToObject(pJSStatGraphHistogramInfo, "val", 3);
        cJSON_AddItemToArray(pJSStatGraphHistogramTotalInfo, pJSStatGraphHistogramInfo);

        pJSStatGraphHistogramInfo = cJSON_CreateObject();
        cJSON_AddStringToObject(pJSStatGraphHistogramInfo, "name", "Batt_1 DisCharged");
        cJSON_AddNumberToObject(pJSStatGraphHistogramInfo, "val", 4);
        cJSON_AddItemToArray(pJSStatGraphHistogramTotalInfo, pJSStatGraphHistogramInfo);

        pJSStatGraphHistogramInfo = cJSON_CreateObject();
        cJSON_AddStringToObject(pJSStatGraphHistogramInfo, "name", "Batt_2 Charged");
        cJSON_AddNumberToObject(pJSStatGraphHistogramInfo, "val", 5);
        cJSON_AddItemToArray(pJSStatGraphHistogramTotalInfo, pJSStatGraphHistogramInfo);

        pJSStatGraphHistogramInfo = cJSON_CreateObject();
        cJSON_AddStringToObject(pJSStatGraphHistogramInfo, "name", "Batt_2 DisCharged");
        cJSON_AddNumberToObject(pJSStatGraphHistogramInfo, "val", 6);
        cJSON_AddItemToArray(pJSStatGraphHistogramTotalInfo, pJSStatGraphHistogramInfo);

        cJSON_AddItemToObject(pJDataChart, "histogram_select", pJSStatGraphHistogramTotalInfo);

        //---  Time Range DataInfo ---//
        cJSON* pJSStatGraphTimeRangeTotalInfo = cJSON_CreateArray();

        cJSON* pJSStatGraphTimeRangeInfo = cJSON_CreateObject();
        cJSON_AddStringToObject(pJSStatGraphTimeRangeInfo, "name", LoadLangText_ById(824));
        cJSON_AddNumberToObject(pJSStatGraphTimeRangeInfo, "val", 0);
        cJSON_AddItemToArray(pJSStatGraphTimeRangeTotalInfo, pJSStatGraphTimeRangeInfo);

        pJSStatGraphTimeRangeInfo = cJSON_CreateObject();
        cJSON_AddStringToObject(pJSStatGraphTimeRangeInfo, "name", LoadLangText_ById(823));
        cJSON_AddNumberToObject(pJSStatGraphTimeRangeInfo, "val", 1);
        cJSON_AddItemToArray(pJSStatGraphTimeRangeTotalInfo, pJSStatGraphTimeRangeInfo);

        cJSON_AddItemToObject(pJDataChart, "time_select", pJSStatGraphTimeRangeTotalInfo);

        //---  Time Range 2 DataInfo ---//
        cJSON* pJSStatGraphTimeRange2TotalInfo = cJSON_CreateArray();

        cJSON* pJSStatGraphTimeRange2Info = cJSON_CreateObject();
        cJSON_AddStringToObject(pJSStatGraphTimeRange2Info, "name", LoadLangText_ById(823));
        cJSON_AddNumberToObject(pJSStatGraphTimeRange2Info, "val", 0);
        cJSON_AddItemToArray(pJSStatGraphTimeRange2TotalInfo, pJSStatGraphTimeRange2Info);

        pJSStatGraphTimeRange2Info = cJSON_CreateObject();
        cJSON_AddStringToObject(pJSStatGraphTimeRange2Info, "name", LoadLangText_ById(6020));
        cJSON_AddNumberToObject(pJSStatGraphTimeRange2Info, "val", 1);
        cJSON_AddItemToArray(pJSStatGraphTimeRange2TotalInfo, pJSStatGraphTimeRange2Info);

        pJSStatGraphTimeRange2Info = cJSON_CreateObject();
        cJSON_AddStringToObject(pJSStatGraphTimeRange2Info, "name", LoadLangText_ById(825));
        cJSON_AddNumberToObject(pJSStatGraphTimeRange2Info, "val", 2);
        cJSON_AddItemToArray(pJSStatGraphTimeRange2TotalInfo, pJSStatGraphTimeRange2Info);

        cJSON_AddItemToObject(pJDataChart, "time_select2", pJSStatGraphTimeRange2TotalInfo);

        //---  Stat Graph Temp Curve DataInfo ---//
        cJSON* pJSStatGraphTempCurveTotalInfo = cJSON_CreateArray();

        cJSON* pJSStatGraphTempCurveInfo = cJSON_CreateObject();
        cJSON_AddStringToObject(pJSStatGraphTempCurveInfo, "name", LoadLangText_ById(346));
        cJSON_AddNumberToObject(pJSStatGraphTempCurveInfo, "val", 0);
        cJSON_AddItemToArray(pJSStatGraphTempCurveTotalInfo, pJSStatGraphTempCurveInfo);

        pJSStatGraphTempCurveInfo = cJSON_CreateObject();
        cJSON_AddStringToObject(pJSStatGraphTempCurveInfo, "name", LoadLangText_ById(347));
        cJSON_AddNumberToObject(pJSStatGraphTempCurveInfo, "val", 1);
        cJSON_AddItemToArray(pJSStatGraphTempCurveTotalInfo, pJSStatGraphTempCurveInfo);

        pJSStatGraphTempCurveInfo = cJSON_CreateObject();
        cJSON_AddStringToObject(pJSStatGraphTempCurveInfo, "name", LoadLangText_ById(348));
        cJSON_AddNumberToObject(pJSStatGraphTempCurveInfo, "val", 2);
        cJSON_AddItemToArray(pJSStatGraphTempCurveTotalInfo, pJSStatGraphTempCurveInfo);

        pJSStatGraphTempCurveInfo = cJSON_CreateObject();
        cJSON_AddStringToObject(pJSStatGraphTempCurveInfo, "name", LoadLangText_ById(349));
        cJSON_AddNumberToObject(pJSStatGraphTempCurveInfo, "val", 3);
        cJSON_AddItemToArray(pJSStatGraphTempCurveTotalInfo, pJSStatGraphTempCurveInfo);

        cJSON_AddItemToObject(pJDataChart, "battery_select", pJSStatGraphTempCurveTotalInfo);

        cJSON_AddStringToObject(pJDataChart, "tem_tip", LoadLangText_ById(6564));

        cJSON_AddStringToObject(pJDataChart, "time_tip", LoadLangText_ById(6565));
        
        cJSON_AddItemToObject(pDict, "dict_datachart", pJDataChart);
    }

    //Following code is for PCS Monitor Web
    //---------18. pcs home page--------
    cJSON* pJPcsHome = cJSON_CreateObject();

    cJSON_AddStringToObject(pJPcsHome, "tip", LoadLangText_ById(6867));
    
    cJSON_AddStringToObject(pJPcsHome, "pcs", LoadLangText_ById(6868));

    cJSON_AddStringToObject(pJPcsHome, "onoff", LoadLangText_ById(459));

    cJSON_AddStringToObject(pJPcsHome, "on", LoadLangText_ById(90));

    cJSON_AddStringToObject(pJPcsHome, "off", LoadLangText_ById(91));

    cJSON_AddStringToObject(pJPcsHome, "power", LoadLangText_ById(1961));

    cJSON* pJSPcsHomePowerList = cJSON_CreateArray();
    cJSON* pJSPcsHomePowerMode = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsHomePowerMode, "label", LoadLangText_ById(1864));
    cJSON_AddNumberToObject(pJSPcsHomePowerMode, "value", 0);
    cJSON_AddItemToArray(pJSPcsHomePowerList, pJSPcsHomePowerMode);

    pJSPcsHomePowerMode = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsHomePowerMode, "label", LoadLangText_ById(1865));
    cJSON_AddNumberToObject(pJSPcsHomePowerMode, "value", 1);
    cJSON_AddItemToArray(pJSPcsHomePowerList, pJSPcsHomePowerMode);

    cJSON_AddItemToObject(pJPcsHome, "powerList", pJSPcsHomePowerList);

    cJSON_AddStringToObject(pJPcsHome, "electricity", LoadLangText_ById(1962));

    cJSON* pJSPcsHomeElectricityList = cJSON_CreateArray();
    cJSON* pJSPcsHomeElectricityMode = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsHomeElectricityMode, "label", LoadLangText_ById(1866));
    cJSON_AddNumberToObject(pJSPcsHomeElectricityMode, "value", 0);
    cJSON_AddItemToArray(pJSPcsHomeElectricityList, pJSPcsHomeElectricityMode);

    pJSPcsHomeElectricityMode = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsHomeElectricityMode, "label", LoadLangText_ById(1868));
    cJSON_AddNumberToObject(pJSPcsHomeElectricityMode, "value", 1);
    cJSON_AddItemToArray(pJSPcsHomeElectricityList, pJSPcsHomeElectricityMode);

    pJSPcsHomeElectricityMode = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsHomeElectricityMode, "label", LoadLangText_ById(1867));
    cJSON_AddNumberToObject(pJSPcsHomeElectricityMode, "value", 2);
    cJSON_AddItemToArray(pJSPcsHomeElectricityList, pJSPcsHomeElectricityMode);

    cJSON_AddItemToObject(pJPcsHome, "electeicityList", pJSPcsHomeElectricityList);

    cJSON_AddStringToObject(pJPcsHome, "alarm_tip", LoadLangText_ById(6869));

    cJSON_AddStringToObject(pJPcsHome, "set_tip", LoadLangText_ById(6870));

    cJSON_AddStringToObject(pJPcsHome, "table_paramType", LoadLangText_ById(6871));

    cJSON_AddStringToObject(pJPcsHome, "table_paramName", LoadLangText_ById(250));

    cJSON_AddStringToObject(pJPcsHome, "table_paramValue", LoadLangText_ById(251));

    cJSON_AddStringToObject(pJPcsHome, "success", LoadLangText_ById(249));

    cJSON* pJSPcsHomeFiltersList = cJSON_CreateArray();
    cJSON* pJSPcsHomeFilters = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsHomeFilters, "text", LoadLangText_ById(1702));
    cJSON_AddStringToObject(pJSPcsHomeFilters, "value", LoadLangText_ById(1702));
    cJSON_AddItemToArray(pJSPcsHomeFiltersList, pJSPcsHomeFilters);

    pJSPcsHomeFilters = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsHomeFilters, "text", LoadLangText_ById(1701));
    cJSON_AddStringToObject(pJSPcsHomeFilters, "value", LoadLangText_ById(1701));
    cJSON_AddItemToArray(pJSPcsHomeFiltersList, pJSPcsHomeFilters);

    pJSPcsHomeFilters = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsHomeFilters, "text", LoadLangText_ById(6872));
    cJSON_AddStringToObject(pJSPcsHomeFilters, "value", LoadLangText_ById(6872));
    cJSON_AddItemToArray(pJSPcsHomeFiltersList, pJSPcsHomeFilters);

    pJSPcsHomeFilters = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsHomeFilters, "text", LoadLangText_ById(6873));
    cJSON_AddStringToObject(pJSPcsHomeFilters, "value", LoadLangText_ById(6873));
    cJSON_AddItemToArray(pJSPcsHomeFiltersList, pJSPcsHomeFilters);

    cJSON_AddItemToObject(pJPcsHome, "filters", pJSPcsHomeFiltersList);

    cJSON_AddStringToObject(pJPcsHome, "on_off_grid", LoadLangText_ById(2073));

    cJSON* pJSPcsHomeOnOffGridList = cJSON_CreateArray();
    cJSON* pJSPcsHomeOnOffGrid = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsHomeOnOffGrid, "label", LoadLangText_ById(701));
    cJSON_AddNumberToObject(pJSPcsHomeOnOffGrid, "value", 0);
    cJSON_AddItemToArray(pJSPcsHomePowerList, pJSPcsHomeOnOffGrid);

    pJSPcsHomeOnOffGrid = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsHomeOnOffGrid, "label", LoadLangText_ById(702));
    cJSON_AddNumberToObject(pJSPcsHomeOnOffGrid, "value", 1);
    cJSON_AddItemToArray(pJSPcsHomeOnOffGridList, pJSPcsHomeOnOffGrid);

    cJSON_AddItemToObject(pJPcsHome, "on_off_gridList", pJSPcsHomeOnOffGridList);

    cJSON_AddStringToObject(pJPcsHome, "control_model", LoadLangText_ById(465));

    cJSON* pJSPcsHomeControlModelList = cJSON_CreateArray();
    cJSON* pJSPcsHomeControlModel = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsHomeControlModel, "label", LoadLangText_ById(222));
    cJSON_AddNumberToObject(pJSPcsHomeControlModel, "value", 0);
    cJSON_AddItemToArray(pJSPcsHomeControlModelList, pJSPcsHomeControlModel);

    pJSPcsHomeControlModel = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsHomeControlModel, "label", LoadLangText_ById(2687));
    cJSON_AddNumberToObject(pJSPcsHomeControlModel, "value", 1);
    cJSON_AddItemToArray(pJSPcsHomeControlModelList, pJSPcsHomeControlModel);

    pJSPcsHomeControlModel = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsHomeControlModel, "label", LoadLangText_ById(2688));
    cJSON_AddNumberToObject(pJSPcsHomeControlModel, "value", 2);
    cJSON_AddItemToArray(pJSPcsHomeControlModelList, pJSPcsHomeControlModel);

    cJSON_AddItemToObject(pJPcsHome, "control_modelList", pJSPcsHomeControlModelList);

    cJSON* pJSPcsHomeControlModel1List = cJSON_CreateArray();
    cJSON* pJSPcsHomeControlModel1 = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsHomeControlModel1, "label", LoadLangText_ById(91));
    cJSON_AddNumberToObject(pJSPcsHomeControlModel1, "value", 0);
    cJSON_AddItemToArray(pJSPcsHomeControlModel1List, pJSPcsHomeControlModel1);

    pJSPcsHomeControlModel1 = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsHomeControlModel1, "label", LoadLangText_ById(90));
    cJSON_AddNumberToObject(pJSPcsHomeControlModel1, "value", 1);
    cJSON_AddItemToArray(pJSPcsHomeControlModel1List, pJSPcsHomeControlModel1);

    cJSON_AddItemToObject(pJPcsHome, "control_modelList1", pJSPcsHomeControlModel1List);

    cJSON_AddStringToObject(pJPcsHome, "constant", LoadLangText_ById(1962));

    cJSON* pJSPcsHomeConstantList = cJSON_CreateArray();
    cJSON* pJSPcsHomeConstant = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsHomeConstant, "label", LoadLangText_ById(1866));
    cJSON_AddNumberToObject(pJSPcsHomeConstant, "value", 0);
    cJSON_AddItemToArray(pJSPcsHomeConstantList, pJSPcsHomeConstant);

    pJSPcsHomeConstant = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsHomeConstant, "label", LoadLangText_ById(1867));
    cJSON_AddNumberToObject(pJSPcsHomeConstant, "value", 1);
    cJSON_AddItemToArray(pJSPcsHomeConstantList, pJSPcsHomeConstant);

    cJSON_AddItemToObject(pJPcsHome, "constantList", pJSPcsHomeConstantList);

    cJSON_AddStringToObject(pJPcsHome, "current", LoadLangText_ById(364));

    cJSON_AddStringToObject(pJPcsHome, "capacity", LoadLangText_ById(6897));

    cJSON_AddStringToObject(pJPcsHome, "voltage", LoadLangText_ById(326));

    cJSON_AddStringToObject(pJPcsHome, "frequency", LoadLangText_ById(143));

    cJSON_AddItemToObject(pDict, "dict_pcs_home", pJPcsHome);

    //---------19. pcs his_alarm page--------
    cJSON* pJPcsHisAlarm = cJSON_CreateObject();
    ADD_STRJ_ITEM(pJPcsHisAlarm, "title_hisalarm",                      241);
    ADD_STRJ_ITEM(pJPcsHisAlarm, "condition_time",                      274);
    static INT_KEYVAL s_PcsHisAlmCondTime[] = { {0, 998}, {1, 337},{2, 338},{3, 339} };
	ADD_OBJ_ARRAY(pJPcsHisAlarm, "list_cond_time", s_PcsHisAlmCondTime);
    ADD_STRJ_ITEM(pJPcsHisAlarm, "title_cond_level",                        110);
    static INT_KEYVAL s_PcsHisAlmCondLevel[] = { {0, 998}, {1, 4},{2, 5},{3, 6} };
	ADD_OBJ_ARRAY(pJPcsHisAlarm, "list_cond_level", s_PcsHisAlmCondLevel);
    ADD_STRJ_ITEM(pJPcsHisAlarm, "title_cond_source",                112);
    static INT_KEYVAL s_PcsHisAlmSource[] = { {0, 998}, {1, 93},{2, 196},{3, 1041},{4, 98},{5, 1040} };
	ADD_OBJ_ARRAY(pJPcsHisAlarm, "list_cond_source", s_PcsHisAlmSource);
    //table
    ADD_STRJ_ITEM(pJPcsHisAlarm, "table_idx",                             109);
    ADD_STRJ_ITEM(pJPcsHisAlarm, "table_level",                           110);
    ADD_STRJ_ITEM(pJPcsHisAlarm, "table_source",                          112);
    ADD_STRJ_ITEM(pJPcsHisAlarm, "table_name",                            111);
    ADD_STRJ_ITEM(pJPcsHisAlarm, "table_beginTime",                       113);
    ADD_STRJ_ITEM(pJPcsHisAlarm, "table_endTime",                         116);
    ADD_STRJ_ITEM(pJPcsHisAlarm, "table_code",                           6886);
    ADD_STRJ_ITEM(pJPcsHisAlarm, "table_status",                         6887);
    ADD_STRJ_ITEM(pJPcsHisAlarm, "table_duration",                       6888);
    ADD_STRJ_ITEM(pJPcsHisAlarm, "table_operation",                      1010);
    ADD_STRJ_ITEM(pJPcsHisAlarm, "title_cond_pcs",                       6889);
    ADD_STRJ_ITEM(pJPcsHisAlarm, "mark",                                 6893);
    ADD_STRJ_ITEM(pJPcsHisAlarm, "mark_tip",                             6894);

    cJSON* pJSPcsHisAlarmCondList = cJSON_CreateArray();
    cJSON* pJSPcsHisAlarmCond = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsHisAlarmCond, "name", LoadLangText_ById(721));
    cJSON_AddNumberToObject(pJSPcsHisAlarmCond, "val", 0);
    cJSON_AddItemToArray(pJSPcsHisAlarmCondList, pJSPcsHisAlarmCond);

    pJSPcsHisAlarmCond = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsHisAlarmCond, "name", LoadLangText_ById(722));
    cJSON_AddNumberToObject(pJSPcsHisAlarmCond, "val", 1);
    cJSON_AddItemToArray(pJSPcsHisAlarmCondList, pJSPcsHisAlarmCond);

    cJSON_AddItemToObject(pJPcsHisAlarm, "list_cond_pcs", pJSPcsHisAlarmCondList);

    cJSON* pJSPcsHisAlarmStatusList = cJSON_CreateArray();
    cJSON* pJSPcsHisAlarmStatus = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsHisAlarmStatus, "name", LoadLangText_ById(6890));
    cJSON_AddNumberToObject(pJSPcsHisAlarmStatus, "val", 0);
    cJSON_AddItemToArray(pJSPcsHisAlarmStatusList, pJSPcsHisAlarmStatus);

    pJSPcsHisAlarmStatus = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsHisAlarmStatus, "name", LoadLangText_ById(6891));
    cJSON_AddNumberToObject(pJSPcsHisAlarmStatus, "val", 1);
    cJSON_AddItemToArray(pJSPcsHisAlarmStatusList, pJSPcsHisAlarmStatus);

    pJSPcsHisAlarmStatus = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsHisAlarmStatus, "name", LoadLangText_ById(6892));
    cJSON_AddNumberToObject(pJSPcsHisAlarmStatus, "val", 2);
    cJSON_AddItemToArray(pJSPcsHisAlarmStatusList, pJSPcsHisAlarmStatus);

    cJSON_AddItemToObject(pJPcsHisAlarm, "list_cond_status", pJSPcsHisAlarmStatusList);

    cJSON_AddItemToObject(pDict, "dict_pcs_hisalarm", pJPcsHisAlarm);   

    //---------20. pcs run_info page--------
    cJSON* pJPcsRunInfo = cJSON_CreateObject();

    ADD_STRJ_ITEM(pJPcsRunInfo, "title_cond_pcs",                       6889);
    ADD_STRJ_ITEM(pJPcsRunInfo, "table_idx",                             109);
    ADD_STRJ_ITEM(pJPcsRunInfo, "table_voltage",                         148);
    ADD_STRJ_ITEM(pJPcsRunInfo, "table_electric_current",                149);
    ADD_STRJ_ITEM(pJPcsRunInfo, "table_working_mode",                   1920);
    ADD_STRJ_ITEM(pJPcsRunInfo, "table_ambient_temperature",            6135);
    ADD_STRJ_ITEM(pJPcsRunInfo, "table_module_temperature",             6279);
    ADD_STRJ_ITEM(pJPcsRunInfo, "table_time",                            274);
    

    cJSON* pJSPcsRunInfoCondList = cJSON_CreateArray();
    cJSON* pJSPcsRunInfoCond = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsRunInfoCond, "name", LoadLangText_ById(721));
    cJSON_AddNumberToObject(pJSPcsRunInfoCond, "val", 0);
    cJSON_AddItemToArray(pJSPcsRunInfoCondList, pJSPcsRunInfoCond);

    pJSPcsRunInfoCond = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsRunInfoCond, "name", LoadLangText_ById(722));
    cJSON_AddNumberToObject(pJSPcsRunInfoCond, "val", 1);
    cJSON_AddItemToArray(pJSPcsRunInfoCondList, pJSPcsRunInfoCond);

    cJSON_AddItemToObject(pJPcsRunInfo, "list_cond_pcs", pJSPcsRunInfoCondList);

    cJSON* pJSPcsRunInfoWorkModeList = cJSON_CreateArray();
    cJSON* pJSPcsRunInfoWorkMode = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsRunInfoWorkMode, "name", LoadLangText_ById(91));
    cJSON_AddNumberToObject(pJSPcsRunInfoWorkMode, "val", 0);
    cJSON_AddItemToArray(pJSPcsRunInfoWorkModeList, pJSPcsRunInfoWorkMode);

    pJSPcsRunInfoWorkMode = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsRunInfoWorkMode, "name", LoadLangText_ById(709));
    cJSON_AddNumberToObject(pJSPcsRunInfoWorkMode, "val", 1);
    cJSON_AddItemToArray(pJSPcsRunInfoWorkModeList, pJSPcsRunInfoWorkMode);

    pJSPcsRunInfoWorkMode = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSPcsRunInfoWorkMode, "name", LoadLangText_ById(710));
    cJSON_AddNumberToObject(pJSPcsRunInfoWorkMode, "val", 2);
    cJSON_AddItemToArray(pJSPcsRunInfoWorkModeList, pJSPcsRunInfoWorkMode);

    cJSON_AddItemToObject(pJPcsRunInfo, "list_work_mode", pJSPcsRunInfoWorkModeList);

    cJSON_AddItemToObject(pDict, "dict_pcs_runinfo", pJPcsRunInfo);
    
    cJSON* s_dict_bmsfault = cJSON_CreateObject();
    ADD_STRJ_ITEM(s_dict_bmsfault, "table_idx",            9334);   // 编号
    ADD_STRJ_ITEM(s_dict_bmsfault, "table_time",           9335);   // 故障时间
    ADD_STRJ_ITEM(s_dict_bmsfault, "table_name",           9336);   // 故障名称
    ADD_STRJ_ITEM(s_dict_bmsfault, "table_state",          9337);   // 故障状态
    ADD_STRJ_ITEM(s_dict_bmsfault, "table_protect_state",  9338);   // 保护状态
    ADD_STRJ_ITEM(s_dict_bmsfault, "table_work_state",     9339);   // 工作状态
    ADD_STRJ_ITEM(s_dict_bmsfault, "table_output_current", 9340);   // 输出电流
    ADD_STRJ_ITEM(s_dict_bmsfault, "table_custom_3",       9341);   // 附件信息3
    ADD_STRJ_ITEM(s_dict_bmsfault, "table_custom_4",       9342);   // 附件信息4
    ADD_STRJ_ITEM(s_dict_bmsfault, "table_custom_5",       9343);   // 附件信息5
    ADD_STRJ_ITEM(s_dict_bmsfault, "table_group",          9344);   // 组
    cJSON* pJGroupList = cJSON_CreateArray();
    cJSON* pJItem;
    pJItem = cJSON_CreateObject(); ADD_STRJ_ITEM(pJItem, "name", 998); cJSON_AddNumberToObject(pJItem, "val", 0); cJSON_AddItemToArray(pJGroupList, pJItem);   // 所有
    pJItem = cJSON_CreateObject(); ADD_STRJ_ITEM(pJItem, "name", 9346); cJSON_AddNumberToObject(pJItem, "val", 1); cJSON_AddItemToArray(pJGroupList, pJItem);   // 组1
    pJItem = cJSON_CreateObject(); ADD_STRJ_ITEM(pJItem, "name", 9347); cJSON_AddNumberToObject(pJItem, "val", 2); cJSON_AddItemToArray(pJGroupList, pJItem);   // 组2
    cJSON_AddItemToObject(s_dict_bmsfault, "list_cond_group", pJGroupList);
    cJSON* pJFtypeList = cJSON_CreateArray();
    pJItem = cJSON_CreateObject(); ADD_STRJ_ITEM(pJItem, "name", 998); cJSON_AddNumberToObject(pJItem, "val", 0); cJSON_AddItemToArray(pJFtypeList, pJItem);   // 所有
    pJItem = cJSON_CreateObject(); ADD_STRJ_ITEM(pJItem, "name", 9314); cJSON_AddNumberToObject(pJItem, "val", 1); cJSON_AddItemToArray(pJFtypeList, pJItem);   // 告警产生
    pJItem = cJSON_CreateObject(); ADD_STRJ_ITEM(pJItem, "name", 9348); cJSON_AddNumberToObject(pJItem, "val", 2); cJSON_AddItemToArray(pJFtypeList, pJItem);   // 告警清除
    pJItem = cJSON_CreateObject(); ADD_STRJ_ITEM(pJItem, "name", 9316); cJSON_AddNumberToObject(pJItem, "val", 3); cJSON_AddItemToArray(pJFtypeList, pJItem);   // 故障产生
    pJItem = cJSON_CreateObject(); ADD_STRJ_ITEM(pJItem, "name", 9317); cJSON_AddNumberToObject(pJItem, "val", 4); cJSON_AddItemToArray(pJFtypeList, pJItem);   // 故障消除
    cJSON_AddItemToObject(s_dict_bmsfault, "list_cond_fstatus", pJFtypeList);
    cJSON* pJFaultTabList = cJSON_CreateArray();
    pJItem = cJSON_CreateObject(); ADD_STRJ_ITEM(pJItem, "name", 9349); cJSON_AddNumberToObject(pJItem, "val", 0); cJSON_AddItemToArray(pJFaultTabList, pJItem);   // 当前故障
    pJItem = cJSON_CreateObject(); ADD_STRJ_ITEM(pJItem, "name", 9350); cJSON_AddNumberToObject(pJItem, "val", 1); cJSON_AddItemToArray(pJFaultTabList, pJItem);   // 历史故障
    cJSON_AddItemToObject(s_dict_bmsfault, "fault_tabs", pJFaultTabList);
    cJSON_AddItemToObject(pDict, "dict_bms_fault", s_dict_bmsfault);
    
    cJSON* s_dict_bmsevent = cJSON_CreateObject();
    ADD_STRJ_ITEM(s_dict_bmsevent, "table_idx",            9334);   // 编号
    ADD_STRJ_ITEM(s_dict_bmsevent, "table_time",           9445);   // 事件时间
    ADD_STRJ_ITEM(s_dict_bmsevent, "table_type",           9446);   // 事件类型
    ADD_STRJ_ITEM(s_dict_bmsevent, "table_name",           9447);   // 事件名称
    ADD_STRJ_ITEM(s_dict_bmsevent, "table_protect_state",  9338);   // 保护状态
    ADD_STRJ_ITEM(s_dict_bmsevent, "table_work_state",     9339);   // 工作状态
    ADD_STRJ_ITEM(s_dict_bmsevent, "table_output_current", 9340);   // 输出电流
    ADD_STRJ_ITEM(s_dict_bmsevent, "table_custom_3",       9341);   // 附件信息3
    ADD_STRJ_ITEM(s_dict_bmsevent, "table_custom_4",       9342);   // 附件信息4
    ADD_STRJ_ITEM(s_dict_bmsevent, "table_custom_5",       9343);   // 附件信息5
    ADD_STRJ_ITEM(s_dict_bmsevent, "table_group",          9344);   // 组
    cJSON* pJEventGroupList = cJSON_CreateArray();
    cJSON* pJEventItem;
    pJEventItem = cJSON_CreateObject(); ADD_STRJ_ITEM(pJEventItem, "name", 998); cJSON_AddNumberToObject(pJEventItem, "val", 0); cJSON_AddItemToArray(pJEventGroupList, pJEventItem);   // 所有
    pJEventItem = cJSON_CreateObject(); ADD_STRJ_ITEM(pJEventItem, "name", 9346); cJSON_AddNumberToObject(pJEventItem, "val", 1); cJSON_AddItemToArray(pJEventGroupList, pJEventItem);   // 组1
    pJEventItem = cJSON_CreateObject(); ADD_STRJ_ITEM(pJEventItem, "name", 9347); cJSON_AddNumberToObject(pJEventItem, "val", 2); cJSON_AddItemToArray(pJEventGroupList, pJEventItem);   // 组2
    cJSON_AddItemToObject(s_dict_bmsevent, "list_cond_group", pJEventGroupList);
    cJSON* pJEventTypeList = cJSON_CreateArray();
    pJEventItem = cJSON_CreateObject(); ADD_STRJ_ITEM(pJEventItem, "name", 998); cJSON_AddNumberToObject(pJEventItem, "val", 0); cJSON_AddItemToArray(pJEventTypeList, pJEventItem);   // 所有
    pJEventItem = cJSON_CreateObject(); ADD_STRJ_ITEM(pJEventItem, "name", 9448); cJSON_AddNumberToObject(pJEventItem, "val", 5); cJSON_AddItemToArray(pJEventTypeList, pJEventItem);   // 输出口变化
    pJEventItem = cJSON_CreateObject(); ADD_STRJ_ITEM(pJEventItem, "name", 9449); cJSON_AddNumberToObject(pJEventItem, "val", 6); cJSON_AddItemToArray(pJEventTypeList, pJEventItem);   // 输入口变化
    pJEventItem = cJSON_CreateObject(); ADD_STRJ_ITEM(pJEventItem, "name", 9450); cJSON_AddNumberToObject(pJEventItem, "val", 7); cJSON_AddItemToArray(pJEventTypeList, pJEventItem);   // 复用输出口变化
    pJEventItem = cJSON_CreateObject(); ADD_STRJ_ITEM(pJEventItem, "name", 9451); cJSON_AddNumberToObject(pJEventItem, "val", 8); cJSON_AddItemToArray(pJEventTypeList, pJEventItem);   // 复用输入口1变化
    pJEventItem = cJSON_CreateObject(); ADD_STRJ_ITEM(pJEventItem, "name", 9452); cJSON_AddNumberToObject(pJEventItem, "val", 9); cJSON_AddItemToArray(pJEventTypeList, pJEventItem);   // 复用输入口2变化
    pJEventItem = cJSON_CreateObject(); ADD_STRJ_ITEM(pJEventItem, "name", 9453); cJSON_AddNumberToObject(pJEventItem, "val", 10); cJSON_AddItemToArray(pJEventTypeList, pJEventItem);  // 状态量变化
    pJEventItem = cJSON_CreateObject(); ADD_STRJ_ITEM(pJEventItem, "name", 9454); cJSON_AddNumberToObject(pJEventItem, "val", 11); cJSON_AddItemToArray(pJEventTypeList, pJEventItem);  // 接收控制命令
    pJEventItem = cJSON_CreateObject(); ADD_STRJ_ITEM(pJEventItem, "name", 9455); cJSON_AddNumberToObject(pJEventItem, "val", 12); cJSON_AddItemToArray(pJEventTypeList, pJEventItem);  // 接收调试命令
    pJEventItem = cJSON_CreateObject(); ADD_STRJ_ITEM(pJEventItem, "name", 9456); cJSON_AddNumberToObject(pJEventItem, "val", 13); cJSON_AddItemToArray(pJEventTypeList, pJEventItem);  // 接收参数配置
    pJEventItem = cJSON_CreateObject(); ADD_STRJ_ITEM(pJEventItem, "name", 9457); cJSON_AddNumberToObject(pJEventItem, "val", 14); cJSON_AddItemToArray(pJEventTypeList, pJEventItem);  // 内部事件
    pJEventItem = cJSON_CreateObject(); ADD_STRJ_ITEM(pJEventItem, "name", 9458); cJSON_AddNumberToObject(pJEventItem, "val", 100); cJSON_AddItemToArray(pJEventTypeList, pJEventItem); // 用户自定义
    cJSON_AddItemToObject(s_dict_bmsevent, "list_cond_etype", pJEventTypeList);
    cJSON_AddItemToObject(pDict, "dict_bms_event", s_dict_bmsevent);

    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "dictionary", pDict);   
}

static void Load_SpportedLangList(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    cJSON* pLangList = cJSON_CreateArray();
    int n;
    LANG_INFO* pLang;
    for(n = 0; n < (int)pFCGI_Rough->stResInfo.nLangTotal; n++)
    {
        cJSON* pJsLang = cJSON_CreateObject();
        pLang = &pFCGI_Rough->stResInfo.langInfos[n];
        cJSON_AddNumberToObject(pJsLang, "lang", pLang->nLangId);
        cJSON_AddStringToObject(pJsLang, "lang_name", LoadLangText_ById(pLang->nLangName_ResId));
        cJSON_AddItemToArray(pLangList, pJsLang);
    }
    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "suported_langs", pLangList);   
}

static void Load_SpportedUserList(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    cJSON* pSupporteUserList = cJSON_CreateArray();
    //added by Jimmy 20210907: for NPS, only apply ROOT
    if(!pFCGI_Rough->bIsWebClient_FromNPS)
    {
        //engineer && admin
        cJSON* pEngineer = cJSON_CreateObject();
        cJSON_AddNumberToObject(pEngineer, "level", USER_LEVEL_ENGINEER);
        cJSON_AddStringToObject(pEngineer, "user_name", LoadLangText_ById(403));
        cJSON_AddItemToArray(pSupporteUserList, pEngineer);
        cJSON* pAdmin = cJSON_CreateObject();
        cJSON_AddNumberToObject(pAdmin, "level", USER_LEVEL_ADMIN);
        cJSON_AddStringToObject(pAdmin, "user_name", LoadLangText_ById(404));
        cJSON_AddItemToArray(pSupporteUserList, pAdmin);        
    }
    //root
    //static BOOL s_bTest = FALSE;
    //if(pFCGI_Rough->bIsKeyPass_4Root)
    //if(s_bTest)//alwasy show root on web-client
    {
        cJSON* pRoot = cJSON_CreateObject();
        cJSON_AddNumberToObject(pRoot, "level", USER_LEVEL_ROOT);
        //cJSON_AddStringToObject(pRoot, "user_name", "root");
        cJSON_AddStringToObject(pRoot, "user_name", pFCGI_Rough->bIsWebClient_FromNPS ? "INFY-MBE" : "ROOT");
        cJSON_AddItemToArray(pSupporteUserList, pRoot);
    }
    //s_bTest = s_bTest ? FALSE : TRUE;

    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "supported_user_levels", pSupporteUserList);   
}

static void Parse_and_ReloadReources(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    //default use ENGLISH, in case data error!
    int nLang = LANG_TYPE_ENGLISH;    
    cJSON* pJ = pFCGI_Rough->pJSON_Resp;
    //when sesssion already created
    //if(pFCGI_Rough->pCurrSess != NULL)
    BOOL bHasLoggedIn = FALSE;
    if((pFCGI_Rough->pStrToken != NULL) && (pFCGI_Rough->pCurrSess != NULL)/* || (strlen(pFCGIRough->pStrToken) < 10)*/) 
    {
        if(strcmp(pFCGI_Rough->pStrToken, pFCGI_Rough->pCurrSess->szToken) == 0)
        {
            nLang = pFCGI_Rough->pCurrSess->pCurrDict->nLangId;
            cJSON_AddNumberToObject(pJ, "login_userlev", pFCGI_Rough->pCurrSess->emUserLvel);  
            bHasLoggedIn = TRUE;        
        }
    }
    //else
    if(!bHasLoggedIn)
    {
        char *pos = NULL;
        pos = strstr(pFCGI_Rough->szQueryRaw, "lang=");
        if(pos!= NULL)
        {
            pos += strlen("lang=");
            if(*pos != '\0')
            {
                nLang = atoi(pos);
                TRACEX("--------get the lang -> %d----------\n", nLang);
            }
            //if data error, we use english 
            if((nLang < 0) || (nLang >= LANG_TYPE_MAX))
            {
                nLang = LANG_TYPE_ENGLISH;
            }    
        }    
    }

    //need reload the lang resource to be sure it is ready
    //but we do not judge it loaded OK here
    Load_LangResources(pFCGI_Rough, nLang);

    pFCGI_Rough->nInitLangSel = nLang;
    
    cJSON_AddNumberToObject(pJ, "curr_lang", nLang);
}

BOOL webpage_handler_init(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    TRACEX("\n----INITILIZATION page_handler was called!---\n");
    if(pFCGI_Rough->emHttpReqMethod == HTTP_REQMETHOD_GET)
    {
        // LJH241101 MU3
        cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "system_type", pFCGI_Rough->emSystemType);
        int web_type = 0;   // 储能网页
        BOOL bIsCellInfo = TRUE;
        if (pFCGI_Rough->emSystemType == SYSTYPE_EMS_STATION)
        {
            web_type = 1;   // 站控网页
            bIsCellInfo = FALSE;
        }
        cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "web_type", web_type);
        cJSON_AddBoolToObject(pFCGI_Rough->pJSON_Resp, "cell_info", bIsCellInfo);
        //now parse langsel
        Parse_and_ReloadReources(pFCGI_Rough);
        //pSessionInfo->nCurrLangSel = Parse_LangSel(szQury);
        Load_DictioneryData(pFCGI_Rough);        
        //now add supported lang list
        Load_SpportedLangList(pFCGI_Rough);        
        //load supported user list
        Load_SpportedUserList(pFCGI_Rough);    
        //added by Jimmy 20210901 to add a title for WEB
        char szDispTitleSN[32] = {0};
        A_BLOB_CHARGER_BOOTINFO bootInfo;
        if(Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 1, 17, sizeof(A_BLOB_CHARGER_BOOTINFO), (void*)&bootInfo))
        {
            snprintf(szDispTitleSN, 32, "%s", bootInfo.szChargePointSerualNuml);
        }        
        cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "charger_disp_title", szDispTitleSN);
    }
    else //post not allowed
    {
        Make_Web_OutPut(104, "Init page does NOT allow POST!", NULL, NULL);
        return FALSE;
    }

    return TRUE;
}