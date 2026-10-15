#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Iram_Ip.h"
#include <irq_num.h>
#include "driver/irq.h"
#include <CLI_type.h>
extern void FuncSafetyPkg_Run_Period_Func(void);
extern void Fabric_Inject_SigMulErr_Func(void);
extern void Iram_Mem_Inj1BitErr_Func(IramCtlType IramCtl);
extern void Iram_Mem_Inj2BitErr_Func(IramCtlType IramCtl);
extern void Iram_Inject_SigMulErr_Func(void);
extern void Sem_SigMonitor_Func(void);
extern void DualCore_LockStep_InjectError(void);
extern void Dcache_1bitErr_Injection(void);
extern void TCM_InjSigMulErr_Func(void);
extern void Run_Bist_Noc_SafeCore(void);
extern void Wdt_SelfTst_Func(void);
extern void Fsrefclk_Check_Func(void);
extern void Soc_Temp_Monitor(void);

extern void FuncSafetyPkg_Init(void);
extern void FuncSafetyPkg_Run_PowerOn_Func(void);

extern void SemOutInterruptEnable(boolean enable);
extern void arch_enable_cache(uint flags);
extern void arch_disable_cache(uint flags);

extern void print_InjErr_IramBaseAddr32(void);

extern int Sem_Isr(uint32_t irq, void *arg);
extern void Iram0_injecterr(void);
extern void Iram_Inj1BitErr_test(void);

extern void enable_mpc_err(void);
extern void enable_bipc_err(void);

extern void udelay(uint32_t usecs);
extern void sem_check_status(void);

extern void fusalib_ddr_en(void);

void usage()
{
        printf("usage:\n");
        printf("   FuncSafetyPkg_test 0:Fabric inject 1bit and 2bit error\n");
        printf("   FuncSafetyPkg_test 1:Iram inject 1bit error\n");
        printf("   FuncSafetyPkg_test 2:Iram inject 2bit error\n");
        printf("   FuncSafetyPkg_test 3:Iram inject 1bit and 2bit error \n");
        printf("   FuncSafetyPkg_test 4:sem monitor\n");
        printf("   FuncSafetyPkg_test 5:Injecting error Dual-Core lockstep\n");
        printf("   FuncSafetyPkg_test 6:Injecting 1bit error in data cache for data ram and tag ram\n");
        printf("   FuncSafetyPkg_test 7:Inject 1/2bits error in ATCM/B0TCM/B1TCM\n");
        printf("   FuncSafetyPkg_test 8:Run bist in noc of safe core : noc_saf noc_hpia noc_hpib\n");
        printf("   FuncSafetyPkg_test 9:self-test mode for wdt\n");
        printf("   FuncSafetyPkg_test 10:Fail safe reference clock check each other among safe,ap,rc clock\n");
        printf("   FuncSafetyPkg_test 11:tested by adjusting over-heat threshold \n");
}

static int FuncSafetyPkg_test(int argc, char *argv[])
{
    uint32_t unTestCase     = 0;
    uint32_t unParameter   = 0;

    if (argc == 1)
    {
        unTestCase = atoi(argv[0]);
        if (unTestCase > 10) {
            usage();
            return 0;
        }
    } else {
        usage();
        return 0;
    }

    /* Disable SEM input interrupt */
    FuncSafetyPkg_Init();
    SemOutInterruptEnable(FALSE);
    switch(unTestCase)
    {
        case 0:
        {
            /* Fabric inject 1bit and 2bit error */
            arch_disable_cache(1);
            Fabric_Inject_SigMulErr_Func();
            arch_enable_cache(1);
        }
        break;

        case 1:
        {
            /* Iram inject 1bit error */
            if (argc > 1)
            {
                unParameter=atoi(argv[1]);
            }
            arch_disable_cache(1);
            Iram_Mem_Inj1BitErr_Func((IramCtlType)unParameter);
            arch_enable_cache(1);
        }
        break;

        case 2:
        {
            /* Iram inject 2bit error */
            if (argc > 1)
            {
                unParameter=atoi(argv[1]);
            }
            arch_disable_cache(1);
            Iram_Mem_Inj2BitErr_Func((IramCtlType)unParameter);
            arch_enable_cache(1);
        }
        break;

        case 3:
        {
            /* Iram inject 1bit and 2bit error */
            arch_disable_cache(1);
            Iram_Inject_SigMulErr_Func();
            arch_enable_cache(1);
        }
        break;

        case 4:
        {
            /* sem monitor */
            Sem_SigMonitor_Func();
        }
        break;

        case 5:
        {
            /* Injecting error Dual-Core lockstep */
            DualCore_LockStep_InjectError();
        }
        break;

        case 6:
        {
            /* Injecting 1bit error in data cache for data ram and tag ram */
            arch_disable_cache(1);
            Dcache_1bitErr_Injection();
            arch_enable_cache(1);
        }
        break;

        case 7:
        {
            /* Inject 1/2bits error in ATCM/B0TCM/B1TCM */
            TCM_InjSigMulErr_Func();
        }
        break;

        case 8:
        {
            /* Run bist in noc of safe core : noc_saf noc_hpia noc_hpib */
            Run_Bist_Noc_SafeCore();
        }
        break;

        case 9:
        {
            /* self-test mode for wdt */
            Wdt_SelfTst_Func();
        }
        break;

        case 10:
        {
            /* Fail safe reference clock check each other among safe,ap,rc clock */
            Fsrefclk_Check_Func();
        }
        break;

        case 11:
        {
            /**
             *SOC temperature monitor itself,temperature sensor shall
             *be tested by adjusting over-heat threshold
             */
            Soc_Temp_Monitor();
        }
        break;
    }

    printf("FuncSafetyPkg_test, use case %d config test pass\n", unTestCase);
    /* Enable SEM input interrupt */
    SemOutInterruptEnable(TRUE);

    return 0;
}

static int  FuncSafetyPkg_Poweron(int argc, char *argv[])
{
    irq_attach(SEM1_O_SEM_INTR_NUM, Sem_Isr, 0);
    irq_set_priority(SEM1_O_SEM_INTR_NUM, 0);
    irq_enable(SEM1_O_SEM_INTR_NUM);

    FuncSafetyPkg_Init();
    FuncSafetyPkg_Run_PowerOn_Func();
    return 0;
}

CLI_CMD("Fusa_Poweron", "FuncSafetyPkg Poweron", FuncSafetyPkg_Poweron);
CLI_CMD("Fusa_test", "FuncSafetyPkg test", FuncSafetyPkg_test);