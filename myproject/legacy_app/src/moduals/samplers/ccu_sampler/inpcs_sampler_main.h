/********************************************************************************
* Copyright(c) Shenzhen Infypower Co., Ltd.
* @author LJH936
* @brief inpcs sampler
* @history 2024-12-02 created
*******************************************************************************/

#ifndef __INPCS_SAMPLER_H
#define __INPCS_SAMPLER_H

#include "ccu_sampler_main.h"

BOOL Initialize_IN235kwPCS_StateM(HANDLE hSamplerThis);
BOOL Initialize_INPCS_StateM(HANDLE hSamplerThis);
int Sample_INPCS_StateM(HANDLE hSamplerThis);
int Sample_IN235kwPCS_StateM(HANDLE hSamplerThis);

#endif
