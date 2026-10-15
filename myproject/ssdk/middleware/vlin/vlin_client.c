/*
 * vlin_client.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Virtual LIN Client driver implements standard LIN driver
 * functions, by forwarding the calls to remote Virtual LIN server.
 *
 * Revision History:
 * -----------------
 */

#include <string.h>

#include <vlin.h>
#include <sdpe_rpc_vlin_service.h>

#include "vlin_common.h"

/**
 * @brief LIN init.
 *
 * @param[in] device lin dev
 * @param[in] config lin config
 */
void Lin_Init(const Lin_ConfigType *Config)
{
    virLin_Init(Config);
}


/**
 * @brief LIN deinit.
 *
 * @param[in] device lin dev
 */
void Lin_DeInit(void)
{
}

/**
 * @brief LIN send frame.
 *
 * @param[in] device lin dev
 * @param[in] pdu send pdu info
 * @return send result
 */
Std_ReturnType Lin_SendFrame(uint8_t Channel,
                             const Lin_PduType *PduInfoPtr)
{
    Std_ReturnType ret;

    VLIN_INFO("%s() channel:%d, Pid: %d, Cs: %d, Drc: %d, Dl: %d, Sdu: \n",
              __func__, Channel, PduInfoPtr->Pid,
              PduInfoPtr->Cs, PduInfoPtr->Drc, PduInfoPtr->Dl);

    for (uint32_t i = 0; i < PduInfoPtr->Dl; i++) {
        VLIN_INFO( "0x%x ", PduInfoPtr->SduPtr[i]);
    }

    VLIN_INFO( "\n");

    ret = virLin_SendFrame(Channel, PduInfoPtr);
    VLIN_INFO( "%s() ret: %d\n", __func__, ret);

    return ret;
}

/**
 * @brief LIN check wakeup.
 *
 * @param[in] device lin dev
 * @return check wakeup result
 */
Std_ReturnType Lin_CheckWakeup(uint8_t Channel)
{
    Std_ReturnType ret;

    VLIN_INFO( "%s() Channel: %d\n", __func__, Channel);
    ret = virLin_CheckWakeup(Channel);
    VLIN_INFO( "%s() ret: %d\n", __func__, ret);

    return ret;
}

/**
 * @brief LIN goto sleep.
 *
 * @param[in] device lin dev
 * @return goto sleep result
 */
Std_ReturnType Lin_GoToSleep(uint8_t Channel)
{
    Std_ReturnType ret;

    VLIN_INFO( "%s() Channel: %d\n", __func__, Channel);
    ret = virLin_GoToSleep(Channel);
    VLIN_INFO( "%s() ret: %d\n", __func__, ret);

    return ret;
}

/**
 * @brief LIN goto sleep internal.
 *
 * @param[in] device lin dev
 * @return goto sleep result
 */
Std_ReturnType Lin_GoToSleepInternal(uint8_t Channel)
{
    Std_ReturnType ret;

    VLIN_INFO( "%s() Channel: %d\n", __func__, Channel);
    ret = virLin_GoToSleepInternal(Channel);
    VLIN_INFO( "%s() ret: %d\n", __func__, ret);

    return ret;
}

/**
 * @brief LIN wakeup.
 *
 * @param[in] device lin dev
 * @return wakeup result
 */
Std_ReturnType Lin_Wakeup(uint8_t Channel)
{
    Std_ReturnType ret;

    VLIN_INFO( "%s() Channel: %d\n", __func__, Channel);
    ret = virLin_Wakeup(Channel);
    VLIN_INFO( "%s() ret: %d\n", __func__, ret);

    return ret;
}

/**
 * @brief LIN wakeup internal.
 *
 * @param[in] device lin dev
 * @return wakeup result
 */
Std_ReturnType Lin_WakeupInternal(uint8_t Channel)
{
    Std_ReturnType ret;

    VLIN_INFO( "%s() Channel: %d\n", __func__, Channel);
    ret = virLin_WakeupInternal(Channel);
    VLIN_INFO( "%s() ret: %d\n", __func__, ret);

    return ret;
}

/**
 * @brief LIN get status.
 *
 * @param[in] device lin dev
 * @param[out] sdu_ptr send or receive sdu
 * @return status
 */
Lin_StatusType Lin_GetStatus(uint8_t Channel, uint8_t **Lin_SduPtr)
{
    Lin_StatusType ret;
    static uint8_t SduPtr[LIN_IFC_CHN_MAX][LIN_FRAME_DATA_LENGTH];

    VLIN_INFO( "%s() Channel: %d\n", __func__, Channel);

    ret = virLin_GetStatus(Channel, SduPtr[Channel]);
    *Lin_SduPtr = SduPtr[Channel];

    VLIN_INFO( "%s() ret: %d, sdu: ", __func__, ret);

    for (uint8_t i = 0; i < 8; i++) {
        VLIN_INFO( "0x%x ", SduPtr[Channel][i]);
    }

    VLIN_INFO( "\n");

    return ret;
}
