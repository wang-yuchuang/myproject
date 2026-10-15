
#include <assert.h>
#include <bits.h>
#include <debug.h>
#include <reg.h>
#include <types.h>

#include "driver/taishan/access_mac/mac.h"
#include "regs_base.h"
void mac_set_mda(uint32_t msid, uint32_t did)
{
    if (readl(MAC_MDA_ADDR(msid)) & BM_MDA_LOCK)
        return;

    uint32_t data = did | BM_MDA_LOCK;
    writel(data, MAC_MDA_ADDR(msid));
}
void mac_unlock_mda(uint32_t msid)
{
    uint32_t val = readl(MAC_MDA_ADDR(msid));
    val |= BM_MDA_LOCK;
    writel(val, MAC_MDA_ADDR(msid));
}
void mac_set_maa_pri(uint32_t msid, uint32_t data)
{
    uint32_t val = readl(MAC_MAA_ADDR(msid));
    val &= ~(BM_MAA_SEC | BM_MAA_SEC_OV_EN);
    val |= (data | BM_MAA_SEC_LOCK);
    writel(val, MAC_MAA_ADDR(msid));
}
void mac_set_maa_secure(uint32_t msid, uint32_t data)
{
    uint32_t val = readl(MAC_MAA_ADDR(msid));
    val &= ~(BM_MAA_PRI | BM_MAA_PRI_OV_EN);
    writel(val, MAC_MAA_ADDR(msid));
}
void mac_set_acs(uint32_t did)
{
    uint32_t val = readl(MAC_BASE + ACS_DID_OFF);
    val &= ~FM_ACS_DID_DID;
    val |= did;
    writel(val, MAC_BASE + ACS_DID_OFF);
}