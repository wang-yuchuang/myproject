#include "../ess_sampler_main.h"


// BOOL Initialize(HANDLE hSamplerSelf)
// {

//     SAMPLER* pSampler = (SAMPLER*)hSamplerSelf;
//     ROUGH_CCU_DATA* pCCU = (ROUGH_CCU_DATA*)pSampler->pDataHeap;

//     int nErrCode;
// 	nErrCode = pCCU->pDPR->Get((HANDLE)pSampler, GETTYPE_COMMPORT,
// 		5, NULL, &pCCU->pComm, sizeof(COMMPORT));

// 	// printf("pComm pointer = %p\n", (void*)pCCU->pComm);
// 	// if (pCCU.pComm == NULL) {
// 	// 	printf("pComm is NULL!\n");
// 	// 	// 处理错误
// 	// }
// 	printf("111111111111\n");
// 	if(nErrCode != ERR_DPR_OK)
// 	{
// 		printf("ccu get can fail\n");
// 	}
// 	else
// 	{
// 		COMM_CAN_BAUDRATE	emLevel = COMM_CAN_BAUDRATE_500K;

// 		printf("szSamplerSetting = %s\n", pSampler->info.szSamplerSetting);

// 		pCCU->hCanDevHanle = pCCU->pComm.Open(
// 			CANPORT,
// 			pSampler->info.szSamplerSetting,
// 			emLevel, 50, &nErrCode);

// 		if(nErrCode != ERR_COMM_OK)
// 		{
// 			printf("Open CAN1 Failed !!! nErrCode=[%d]\n", nErrCode);
// 		}
// 	}
// 	printf("22222222222222\n");



// 	return TRUE;
// }

static void YFY_ReadAndParseCanData(ROUGH_CCU_DATA *pCCU)
{
    int nFrmCount;
    int n = 0, nReadLen = 0;
    can_frame_t RcvMCan;       // 假设该结构包含 uint32_t id; uint8_t len; uint8_t data[8];
    BYTE *byRecvBuff = pCCU->abyRcvBuf;

    memset(byRecvBuff, 0, MAX_READ_BUFF_SIZE * CAN_FRAME_LEN);
    nReadLen = pCCU->pComm.Read(pCCU->hCanDevHanle, (void*)byRecvBuff, 
                                (ssize_t)(CAN_FRAME_LEN * MAX_READ_BUFF_SIZE));

    // 调试打印（可选）
    // printf("YFY BMS Recv: \n");
    // for (n = 0; n < nReadLen; n++) {
    //     printf("%02X ", byRecvBuff[n]);
    // }
    // printf("\n");

    if (nReadLen >= CAN_FRAME_LEN) {
        nFrmCount = nReadLen / CAN_FRAME_LEN;
        for (n = 0; n < nFrmCount; n++) {
            // 当前帧变长数据起始地址
            BYTE *pFrame = byRecvBuff + n * CAN_FRAME_LEN;

            // 1. 读取 DLC（第1字节）
            uint8_t dlc = pFrame[0];
            if (dlc > 8) dlc = 8;   // 安全限制

            // 2. 组装 CAN ID（大端序，pFrame[1] 为最高字节）
            uint32_t id = ((uint32_t)pFrame[1] << 24) |
                          ((uint32_t)pFrame[2] << 16) |
                          ((uint32_t)pFrame[3] << 8)  |
                          (uint32_t)pFrame[4];

            // 3. 复制数据（只复制有效长度）
            RcvMCan.id  = id;
            RcvMCan.len = dlc;
            memcpy(RcvMCan.data, &pFrame[5], dlc);
            // 如果 dlc < 8，剩余字节置零（可选）
            if (dlc < 8) {
                memset(&RcvMCan.data[dlc], 0, 8 - dlc);
            }
            // 调试打印（可选）
            // printf("  ID(大端): %02X %02X %02X %02X -> 0x%08X, DLC=%d\n",
            //        pFrame[1], pFrame[2], pFrame[3], pFrame[4], id, dlc);
            // printf("  Data: ");
            // for (int i = 0; i < dlc; i++) {
            //     printf("%02X ", RcvMCan.data[i]);
            // }
            // printf("\n");

            // 调用原有业务处理函数（需要传入正确的 can_frame_t）
            CanRcvModuleMsgHandle_API(pCCU, &RcvMCan);
        }
    }
    return;
}



int CAN_Samples(HANDLE hSamplerThis)
{
	// printf(">>> CAN Sample_ESS >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	ROUGH_CCU_DATA* pCCU = (ROUGH_CCU_DATA*)pSampler->pDataHeap;

	TASK_N2402_Proc(pCCU);
	YFY_ReadAndParseCanData(pCCU);
	Data_Swap1(pCCU);

	// {
    //    InitCCUStuffData(pCCU);  //注意   通信中断 需要把数据清除一下
    // }

	//TRACE("Info:---Running Here stateM---\n");
	Refresh_SysAvailState(pCCU);
	
	//TRACE("emCCUState=%d, emSystemFailReason=%d \n", pCCU->emCCUState, pCCU->emSystemFailReason );
 
	// A_BLOB_SINGLE_STBATT_RUNINFO
	Update_SysDataInfo(pCCU);
	pCCU->bQuickChannle = FALSE;
	Ems_Mgmt_Main_ESS(pCCU);
	pCCU->emErrCode_MgmtCfg = Check_MgmtPlanCfg(pCCU);
	
 	CalcMPPTGenerateKWH(pCCU);
	
	CalcDCLoadCostKWH(pCCU);
	PushPCSData(pCCU);
	Prepare_ESRunData(pCCU);
 
	Manage_ESStoreData(pCCU);
 
	return ERR_SAMPLE_OK;
}