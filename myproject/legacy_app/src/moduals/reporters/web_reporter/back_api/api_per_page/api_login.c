#include "../fcgi_defs.h"
#include "openssl/rsa.h"

static int s_nCurrent_MBELoginLevel = MBE_REMOTE_USERLEVEL_1;

#define RSA_PKCS1_PADDING 1
# define OPENSSL_INIT_LOAD_SSL_STRINGS       0x00200000L
BOOL RSA_Decrypt_File(const char* key_path, const char* in_path, const char* out_path)
{
    OPENSSL_init_crypto(OPENSSL_INIT_LOAD_CRYPTO_STRINGS | OPENSSL_INIT_LOAD_SSL_STRINGS, NULL);

    // 初始化资源指针
    FILE* key_fp = NULL;
    FILE* in_fp = NULL;
    FILE* out_fp = NULL;
    RSA* rsa = NULL;
    unsigned char* in_buf = NULL;
    unsigned char* out_buf = NULL;
    BOOL ret = FALSE;

    // 1. 检查输入路径
    if (key_path == NULL || in_path == NULL || out_path == NULL) {
        printf("RSA_Decrypt: Error - key/in/out path is NULL\n");
        goto cleanup;
    }
    if (access(key_path, F_OK) != 0) {
        printf("RSA_Decrypt: Error - key file not exist or no permission [%s]\n", key_path);
        goto cleanup;
    }
    if (access(in_path, F_OK) != 0) {
        printf("RSA_Decrypt: Error - input file not exist or no permission [%s]\n", in_path);
        goto cleanup;
    }

    // 2. 打开并读取私钥文件
    key_fp = fopen(key_path, "r");
    if (!key_fp) {
        printf("RSA_Decrypt: Error - open key file failed [%s]\n", key_path);
        goto cleanup;
    }
    rsa = PEM_read_RSAPrivateKey(key_fp, NULL, NULL, NULL);
    fclose(key_fp); key_fp = NULL; // 及时关闭，避免泄漏
    if (!rsa) {
        printf("RSA_Decrypt: Error - read RSA private key failed\n");
        ERR_print_errors_fp(stderr); // 打印OpenSSL具体错误（如密钥格式错误）
        goto cleanup;
    }
    int rsa_size = RSA_size(rsa);

    // 3. 打开加密文件（读）和输出文件（写）
    in_fp = fopen(in_path, "rb");
    if (!in_fp) {
        printf("RSA_Decrypt: Error - open input file failed [%s]\n", in_path);
        goto cleanup;
    }
    out_fp = fopen(out_path, "wb");
    if (!out_fp) {
        printf("RSA_Decrypt: Error - open output file failed [%s]\n", out_path);
        goto cleanup;
    }

    // 4. 分配缓冲区（按RSA密钥长度）
    in_buf = (unsigned char*)malloc(rsa_size);
    out_buf = (unsigned char*)malloc(rsa_size);
    if (!in_buf || !out_buf) {
        printf("RSA_Decrypt: Error - malloc buffer failed (need [%d] bytes)\n", rsa_size);
        goto cleanup;
    }

    // 5. 分块解密（核心逻辑）
    int read_len, dec_len, write_len;
    int block_idx = 0;
    while ((read_len = fread(in_buf, 1, rsa_size, in_fp)) > 0) {
        block_idx++;
        // 校验读取长度（PKCS1加密后每块必为rsa_size，避免异常数据）
        if (read_len != rsa_size && !feof(in_fp)) {
            printf("RSA_Decrypt: Error - invalid block size (read [%d], expected [%d])\n", read_len, rsa_size);
            goto cleanup;
        }

        // 执行RSA私钥解密
        dec_len = RSA_private_decrypt(read_len, in_buf, out_buf, rsa, RSA_PKCS1_PADDING);
        if (dec_len < 0) {
            printf("RSA_Decrypt: Error - decrypt block [%d] failed\n", block_idx);
            ERR_print_errors_fp(stderr); // 打印OpenSSL解密错误（如数据篡改、密钥不匹配）
            goto cleanup;
        }

        // 写入解密数据（校验写入结果，避免磁盘满或权限问题）
        write_len = fwrite(out_buf, 1, dec_len, out_fp);
        if (write_len != dec_len) {
            printf("RSA_Decrypt: Error - write block [%d] failed (written [%d], expected [%d])\n", block_idx, write_len, dec_len);
            goto cleanup;
        }
        fflush(out_fp);
    }

    // 6. 检查读取是否正常结束（无IO错误）
    if (ferror(in_fp)) {
        printf("RSA_Decrypt: Error - read input file IO failed\n");
        goto cleanup;
    }
    
    // 7. 验证输出文件是否存在且非空
    if (access(out_path, F_OK) != 0) {
        printf("RSA_Decrypt: Error - decrypt done but output file not found [%s]\n", out_path);
        goto cleanup;
    }

    fflush(out_fp);
    struct stat out_stat;
    stat(out_path, &out_stat);
    if (out_stat.st_size == 0) {
        printf("RSA_Decrypt: Error - output file is empty [%s]\n", out_path);
        goto cleanup;
    }
    
    ret = TRUE;
    
cleanup:
    // 统一释放资源（避免泄漏）
    free(in_buf);
    free(out_buf);
    if (key_fp) fclose(key_fp);
    if (in_fp) fclose(in_fp);
    if (out_fp) fclose(out_fp);
    if (rsa) RSA_free(rsa);

    return ret;
}

static BOOL Check_PSW_4Login(FCGI_ROUGH_DATA* pFCGI_Rough, int nUserLev, char* szPasswd)
{
    char szPassword[32] = {0};
    if((nUserLev == USER_LEVEL_ENGINEER)/* && (strcmp(szPasswd, "151719") == 0)*/)
    {
        //added by Jimmy 20210907: only allow ROOT for login
        if(pFCGI_Rough->bIsWebClient_FromNPS)
        {
            return FALSE;
        }

        if(!Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 1, 4, 32, (void*)szPassword))
        {
            TRACEX("----CAN not get the engineer password from API---\n");
            return FALSE;
        }
        if(strcmp(szPasswd, szPassword) != 0)
        {
            TRACEX("----Engineer Pwss not match [%s]->[%s]---\n", szPasswd, szPassword);
            return FALSE;
        }
        return TRUE;
    }
    else if((nUserLev == USER_LEVEL_ADMIN)/* && (strcmp(szPasswd, "135792068") == 0)*/)
    {
        //added by Jimmy 20210907: only allow ROOT for login
        if(pFCGI_Rough->bIsWebClient_FromNPS)
        {
            return FALSE;
        }

        if(!Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 1, 24, 32, (void*)szPassword))
        {
            TRACEX("----CAN not get the admin password from API---\n");
            return FALSE;
        }
        if(strcmp(szPasswd, szPassword) != 0)
        {
            TRACEX("----Admin Pwss not match [%s]->[%s]---\n", szPasswd, szPassword);
            return FALSE;
        }
        return TRUE;
    }
    // else if((nUserLev == USER_LEVEL_ROOT) && (strcmp(szPasswd, "123") == 0))
    // {
    //     return TRUE;
    // }

    //now for root
#define ROOT_KEY_FILE_TMP_PATH      "/opt/appdata/data/.ks"
#define PARSED_ROOT_PSD_TEMP_PATH   "/opt/appdata/data/.tmpsd"
    if(!pFCGI_Rough->stMultiFormData.bHasParsed_File1)
    {
        return FALSE;
    }

    char szCmd[256];
    // LJH241101 MU3
// #ifdef IMMU3
    if(access(ROOT_KEY_FILE_TMP_PATH, F_OK) != 0)
    {
        const char szRootKeyPrivRSA[] = "-----BEGIN RSA PRIVATE KEY-----\n"
            "MIICXAIBAAKBgQCbR6daomLVbSKTZfgXvHBPg3odnbis3QOMOekW1z/kTnNVO1k4\n"
            "6PLQou9sZxoTOy/aYVebG+FpcWhPXrBpgsyQaFvge3LyTCxTFM+1C63UVAoQ8Mn0\n"
            "RLPi0sALRdn2xRZo2+se1NxxQlWoWMHTsa8a7bW/Si+4KU/7SbqMNJIBGQIDAQAB\n"
            "AoGAAyRKAI10jnjs0CqqAGDxrMouOFcum2DFZ5rsWmf0hPwJXVocDgi4kPwa+AiG\n"
            "AImcCEEb8ql3wBhpUeHKk/fvh5xjQRl2Xy9ePbLfS4sdvEZGUzfE66Gogc6zBgqI\n"
            "XevwwIRG6ifstAzccUmfEdjL8c3GyEPDoK7FiMt4I4pyOlkCQQDKfghpom74rXIB\n"
            "v0Dv7uZsG3Zk/LKUlvk3Ygn8uLb76Moo24PEZkmLLdgUDgcMZMW7jPMor/p8Ph7c\n"
            "IixbtKn3AkEAxE/ZeciqX/ZfHzp19aim23XYTsR9z7+S4d4OHQhzFLp3gJaqWOML\n"
            "/6bRzO9dd8SeOiehjT+O4MS4UyiW8yppbwJBAJt11WmcToxXRR74QZezl25KS0RP\n"
            "CZ+wpRcTCs3e1oVifMqPdYN0cGQNR0ZmWpz2RSSvmwOSz8FRRAoHpnyloJMCQAfk\n"
            "mf7ehZMkKcDsrc95v3O1ckQAfHp83l/8eJ7MNyxPLvzZkfHO9y2JPIg724s0OZRn\n"
            "AYTXWFxLLIoTaLZQLzsCQHBVdHscllX+CWONN28yuyCiqpuK97gibeKmsMzjXzPa\n"
            "TuGp5odBU6COcaL1dxIoDPLj6bivhDaYQt365u49cCs=\n"
            "-----END RSA PRIVATE KEY-----";
        snprintf(szCmd, 2048, "touch %s", ROOT_KEY_FILE_TMP_PATH);
         _SYSTEM(szCmd);
        int nPos = 0;
        int nTempLen = 0;
        int nLen = strlen(szRootKeyPrivRSA);
        char szScetor[200];        
        while(nPos < nLen)
        {
            snprintf(szScetor, 200, "%s", szRootKeyPrivRSA + nPos);
            snprintf(szCmd, 256, "echo \"%s\" >> %s \n", szScetor, ROOT_KEY_FILE_TMP_PATH);
            nTempLen = strlen(szScetor);
            nPos += nTempLen;
            if(nTempLen <= 0)
            {
                break;
            }
            _SYSTEM(szCmd);
        }
    }
// #endif
    // snprintf(szCmd, 256, "openssl rsautl -decrypt -inkey %s -in %s -out %s", 
    //         ROOT_KEY_FILE_TMP_PATH,
    //         pFCGI_Rough->stMultiFormData.szPath_4File1,
    //         PARSED_ROOT_PSD_TEMP_PATH);
    // _SYSTEM(szCmd);

    if (!RSA_Decrypt_File(
        ROOT_KEY_FILE_TMP_PATH,
        pFCGI_Rough->stMultiFormData.szPath_4File1,
        PARSED_ROOT_PSD_TEMP_PATH)) 
    {
        printf("ROOT login: RSA decrypt failed\n");
        unlink(ROOT_KEY_FILE_TMP_PATH);
        unlink(pFCGI_Rough->stMultiFormData.szPath_4File1);
        pFCGI_Rough->stMultiFormData.bHasParsed_File1 = FALSE;
        return FALSE;
    }

    FILE* fp = fopen(PARSED_ROOT_PSD_TEMP_PATH, "r");
    if (fp == NULL)
	{
		return FALSE;
	}
    char szTtt[64];
    memset(szTtt, 0, 64);
    fread(szTtt, 63, 1, fp);
    fclose(fp);
    unlink(PARSED_ROOT_PSD_TEMP_PATH);
    BOOL bMatched = TRUE;
    //added by Jimmy 20210907: only allow ROOT for login
    if(pFCGI_Rough->bIsWebClient_FromNPS)
    {
        s_nCurrent_MBELoginLevel = MBE_REMOTE_USERLEVEL_0; //defauult login level
        if(strlen(pFCGI_Rough->szMBE_NewToken) == 0)
        {
            char szNewRoot4MBE[64];
            snprintf(szNewRoot4MBE, 64, "IF_%s_be!", szTtt);
            if(strcmp(szNewRoot4MBE, szPasswd) != 0)
            {
                //return FALSE;
                bMatched = FALSE;
            }  
        }
        else
        {
			if(strcmp(szTtt, "123") != 0) //added by 2024-05-29 because I found any root_key will pass the verification which should be avoided!
            {
                //return FALSE;
                bMatched = FALSE;
            } 
            else
            {
            char szBasee64Passwd_AndLevel[256];
            char szMD5[33] = {0};
            int n;        
            bMatched = FALSE;    
            char szBase64Passwd[128] = {0};
            BOOL bCalcBase64 = TRUE;
            int nLen = 128;
            printf("--------Now begin calc the Base64 for psd->[%s]----\n", szPasswd);
            if(!Base64_Encode(szPasswd, strlen(szPasswd), szBase64Passwd, &nLen))
            {
                printf("----CAN NOT calc the base64 for passwd[%s]----\n", szPasswd);
                bCalcBase64 = FALSE;
            }
            printf("--------Now end calc the Base64 for psd->[%s]----\n", szPasswd);
            
            if(bCalcBase64)
            {
                for(n = MBE_REMOTE_USERLEVEL_0; n <= MBE_REMOTE_USERLEVEL_2; n++)
                {
                    snprintf(szBasee64Passwd_AndLevel, 256, "{%d,%s}", n, szBase64Passwd);
                    if(!MD5_Encode(szBasee64Passwd_AndLevel, strlen(szBasee64Passwd_AndLevel), szMD5, 33))
                    {
                        printf("----CAN NOT calc the MD5 for szPasswd_AndLevel[%s]----\n", szBasee64Passwd_AndLevel);
                        continue;
                    }
                    //now check if the MD5 matches the header so that to think the user is VALID
                    //ConvertString_2UpperCase(szMD5); //convert -> upper for comparation, already upper
                    printf("------The lev-passwd->[%s], calc-md5->[%s], header-md5->[%s]------\n",
                        szBasee64Passwd_AndLevel, szMD5, pFCGI_Rough->szMBE_NewToken);
                    if(strncmp(szMD5, pFCGI_Rough->szMBE_NewToken, 33) == 0)
                    {
                        bMatched = TRUE;
                        s_nCurrent_MBELoginLevel = n;
                        break;
                    }  
                }                    
            }
		  }
        }
    }
    else
    {
        //judge the psz match
        TRACEX("---pWWWWS[%s]->[%s]----\n", szTtt, szPasswd);
        if(strcmp(szTtt, szPasswd) != 0)
        {
            //return FALSE;
            bMatched = FALSE;
        }        
    }

    unlink(pFCGI_Rough->stMultiFormData.szPath_4File1);
    pFCGI_Rough->stMultiFormData.bHasParsed_File1 = FALSE;

    //return TRUE;
    return bMatched;
}

static BOOL Parse_LogInInfo_FromBody(FCGI_ROUGH_DATA* pFCGI_Rough, int* pnLang, int* pnUserLevel, char* szPasswd, int nPsdMaxLen)
{
    if((pFCGI_Rough->nBodyLen < 3) && (!pFCGI_Rough->stMultiFormData.bHasExternalBody))
    {
        Make_Web_OutPut(104, "NO login information provided!", NULL, NULL);
        return FALSE;
    }

    char* pszFromWhat = pFCGI_Rough->szBodyRaw;
    if(pFCGI_Rough->stMultiFormData.bHasExternalBody) //for multi
    {
        pszFromWhat = pFCGI_Rough->stMultiFormData.szExternBody;
    }
    cJSON* pJLogin = cJSON_Parse(pszFromWhat);
    if(pJLogin == NULL)
    {
        Make_Web_OutPut(104, "The login info is NOT valid Json string!", NULL, NULL);
        return FALSE;
    }

    cJSON* temp;
    //parse lang
    if ((temp = cJSON_GetObjectItem(pJLogin, "lang")) == NULL)
    {
        Make_Web_OutPut(104, "The 'lang' is NOT provided!", NULL, NULL);
        cJSON_Delete(pJLogin);
        return FALSE;
    }  
    *pnLang = (temp->type == cJSON_String) ? atoi(temp->valuestring) : temp->valueint;  

    //parse userLevel
    if ((temp = cJSON_GetObjectItem(pJLogin, "u_lev")) == NULL)
    {
        Make_Web_OutPut(104, "The 'u_lev' is NOT provided!", NULL, NULL);
        cJSON_Delete(pJLogin);
        return FALSE;
    }  
    *pnUserLevel = (temp->type == cJSON_String) ? atoi(temp->valuestring) : temp->valueint;  

    //pase passWd
    if ((temp = cJSON_GetObjectItem(pJLogin, "psw")) == NULL)
    {
        Make_Web_OutPut(104, "The 'psw' is NOT provided!", NULL, NULL);
        cJSON_Delete(pJLogin);
        return FALSE;
    }  

    if(temp->type != cJSON_String)
    {
        Make_Web_OutPut(104, "The 'psw' is NOT a string!", NULL, NULL);
        cJSON_Delete(pJLogin);
        return FALSE;
    }

    snprintf(szPasswd, nPsdMaxLen, "%s", temp->valuestring);

    cJSON_Delete(pJLogin);
    return TRUE;    
}

BOOL webpage_handler_login(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    TRACEX("\n----LOGIN page_handler was called!---\n");    
    if(pFCGI_Rough->emHttpReqMethod == HTTP_REQMETHOD_GET)
    {
        Make_Web_OutPut(104, "Login page does NOT allow GET!", NULL, NULL);
        return FALSE;
    }

    //added for logout 
    int nExit = -1;
    char* szPostStr = Percent_DecodeUrl(pFCGI_Rough->szQueryRaw);
    if(ParseInt_FromQuery(szPostStr, "exit=", &nExit))
    {
        //always return TRUE
        Delete_Curr_Session(pFCGI_Rough);
        return TRUE;
    }

    // TRACEX("--the parsed percent-decoded szPostStr is[%s]--\n", szPostStr);
    // //now parse lang/user/passwd
    // int nLang = LANG_TYPE_ENGLISH;
    // if(!ParseInt_FromQuery(szPostStr, "lang=", &nLang))
    // {
    //     Make_Web_OutPut(104, "The 'lang' NOT provided!", NULL, NULL);
    //     return FALSE;  
    // }
    // else if((nLang < 0) || (nLang >= LANG_TYPE_MAX))
    // {
    //     Make_Web_OutPut(104, "The 'lang' is INVALID!", NULL, NULL);
    //     return FALSE;          
    // }

    // int nUserLevel = USER_LEVEL_ENGINEER;
    // if(!ParseInt_FromQuery(szPostStr, "u_lev=", &nUserLevel))
    // {
    //     Make_Web_OutPut(104, "The 'u_lev' NOT provided!", NULL, NULL);
    //     return FALSE;  
    // }
    // else if((nUserLevel < USER_LEVEL_ENGINEER) || (nUserLevel > USER_LEVEL_ROOT))
    // {
    //     Make_Web_OutPut(104, "The 'u_lev' is INVALID!", NULL, NULL);
    //     return FALSE;          
    // }
    // //root user has to match SD key first
    // else if((nUserLevel == USER_LEVEL_ROOT) && (!pFCGI_Rough->bIsKeyPass_4Root))
    // {
    //     Make_Web_OutPut(104, "Access denied for root!", NULL, NULL);
    //     return FALSE;        
    // }

    // char szPasswd[33] = {0};
    // char *pos = NULL;
    // pos = strstr(szPostStr, "psw=");
    // if(pos!= NULL)
    // {
    //     pos += strlen("psw=");
    //     if(*pos != '"')
    //     {
    //         Make_Web_OutPut(104, "The 'pwd' format is INVALID!", NULL, NULL);
    //         return FALSE;            
    //     }
    //     //now parse the password
    //     int n;
    //     int nPos = 0;
    //     for(n = 1; n <= 32; n++)
    //     {
    //         if((pos[n] == '\0') || (pos[n] == '"'))
    //         {
    //             szPasswd[nPos] = 0;
    //             break;
    //         }
    //         szPasswd[nPos] = pos[n];
    //         nPos++;
    //     }
    // }

    int nLang, nUserLevel;
    char szPasswd[33] = {0};
    if(!Parse_LogInInfo_FromBody(pFCGI_Rough, &nLang, &nUserLevel, szPasswd, 33))
    {
        return FALSE;
    }

    //judge lang
    if((nLang < 0) || (nLang >= LANG_TYPE_MAX))
    {
        Make_Web_OutPut(104, "The 'lang' is INVALID!", NULL, NULL);
        return FALSE;          
    }

    //judge level
    if((nUserLevel < USER_LEVEL_ENGINEER) || (nUserLevel > USER_LEVEL_ROOT))
    {
        Make_Web_OutPut(104, "The 'u_lev' is INVALID!", NULL, NULL);
        return FALSE;          
    }

    if(strlen(szPasswd) < 1)
    {
        Make_Web_OutPut(101, NULL, "Password NOT correct!", NULL);
        return FALSE;            
    }

    if(!Check_PSW_4Login(pFCGI_Rough, nUserLevel, szPasswd))
    {
        Make_Web_OutPut(101, NULL, "Wrong user or password!", NULL);
        return FALSE;            
    }

    //now make the token and creat a new session
    int rc = AddNewSession(pFCGI_Rough, nUserLevel, nLang, s_nCurrent_MBELoginLevel);
    if(rc == ADDSESS_TOO_MANY_USER)
    {
        Make_Web_OutPut(103, NULL, "Too many users, please try 1min later!", NULL);
        return FALSE;
    }
    else if(rc == ADDSESS_LOGIN_TOO_FREQUENT)
    {
        Make_Web_OutPut(103, NULL, "Login too frequently, please retry later!", NULL);
        return FALSE;
    }
    else if(rc != ADDSESS_OK)
    {
        Make_Web_OutPut(101, NULL, "Unkown ERROR!", NULL);
        return FALSE;
    }

    //now send the token
    cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "token", pFCGI_Rough->pCurrSess->szToken);

    return TRUE;
}