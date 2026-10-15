LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR) \
	$(LOCAL_DIR)/SystemInfo \
	$(LOCAL_DIR)/OSUser \
	$(LOCAL_DIR)/Measurement \
	$(LOCAL_DIR)/Parameter \
	$(LOCAL_DIR)/IccMsg \
	$(LOCAL_DIR)/DataCalc \
	$(LOCAL_DIR)/Diagnosis \
	$(LOCAL_DIR)/Statistics \
	$(LOCAL_DIR)/SocLib \
	$(LOCAL_DIR)/EnergyCalc \
	$(LOCAL_DIR)/IMEU3 \
	$(LOCAL_DIR)/IMIU4 \
	$(LOCAL_DIR)/UdsUpgrade \
	$(LOCAL_DIR)/ModbusLib \
	$(LOCAL_DIR)/EventRec \

MODULE_SRCS += \
	$(LOCAL_DIR)/SystemInfo/GroupInfo.c \
	$(LOCAL_DIR)/SystemInfo/PublicInfo.c \
	$(LOCAL_DIR)/OSUser/OSUserHook.c \
	$(LOCAL_DIR)/OSUser/TaskMonitor.c \
	$(LOCAL_DIR)/Parameter/GroupPara.c \
	$(LOCAL_DIR)/Parameter/ParaExtEep.c \
	$(LOCAL_DIR)/Parameter/ParaInterface.c \
	$(LOCAL_DIR)/Parameter/ParaRead.c \
	$(LOCAL_DIR)/Parameter/ParaWrite.c \
	$(LOCAL_DIR)/Parameter/PublicPara.c \
    $(LOCAL_DIR)/IccMsg/IccMsgCreate.c \
    $(LOCAL_DIR)/IccMsg/IccMsgHandle.c \
    $(LOCAL_DIR)/IccMsg/IccMsgInterface.c \
    $(LOCAL_DIR)/IccMsg/IccMsgTabOper.c \
    $(LOCAL_DIR)/IccMsg/IccMsgTxCycle.c \
    $(LOCAL_DIR)/IccMsg/IccMsgUser.c \
    $(LOCAL_DIR)/IccMsg/IccMsgVerSrl.c \
	$(LOCAL_DIR)/Measurement/CellAFESamp.c \
	$(LOCAL_DIR)/Measurement/PortCheck.c \
	$(LOCAL_DIR)/Measurement/CurrSample.c \
	$(LOCAL_DIR)/Measurement/InsuCheck.c \
	$(LOCAL_DIR)/DataCalc/CalcInterface.c \
	$(LOCAL_DIR)/DataCalc/CalcUtil.c \
	$(LOCAL_DIR)/DataCalc/CharactCalc.c \
	$(LOCAL_DIR)/DataCalc/CurrLimCalc.c \
	$(LOCAL_DIR)/DataCalc/CurrLimFall.c \
	$(LOCAL_DIR)/DataCalc/CurrLimPID.c \
	$(LOCAL_DIR)/DataCalc/VoltLimCalc.c \
	$(LOCAL_DIR)/Diagnosis/DiagCheck.c \
	$(LOCAL_DIR)/Diagnosis/DiagFilter.c \
	$(LOCAL_DIR)/Diagnosis/DiagInterface.c \
	$(LOCAL_DIR)/Diagnosis/DiagMaker.c \
	$(LOCAL_DIR)/Diagnosis/DiagSos.c \
	$(LOCAL_DIR)/Diagnosis/DiagUser.c \
	$(LOCAL_DIR)/Diagnosis/DiagAsyncLog.c \
	$(LOCAL_DIR)/Statistics/DodRcdStatis.c \
	$(LOCAL_DIR)/Statistics/EnergyStatis.c \
	$(LOCAL_DIR)/Statistics/StaInterface.c \
	$(LOCAL_DIR)/Statistics/StatisUser.c \
	$(LOCAL_DIR)/Statistics/TimesStatis.c \
	$(LOCAL_DIR)/SocLib/CapEnerCorr.c \
	$(LOCAL_DIR)/SocLib/SocInterface.c \
	$(LOCAL_DIR)/SocLib/SocSoeCorr.c \
	$(LOCAL_DIR)/SocLib/SocSoeSync.c \
	$(LOCAL_DIR)/SocLib/SocUserAPI.c \
	$(LOCAL_DIR)/EnergyCalc/CapInfoCalc.c \
	$(LOCAL_DIR)/EnergyCalc/CellFadeCalc.c \
	$(LOCAL_DIR)/EnergyCalc/CurrIntegral.c \
	$(LOCAL_DIR)/EnergyCalc/EnerInfoCalc.c \
	$(LOCAL_DIR)/EnergyCalc/EnerInterface.c \
	$(LOCAL_DIR)/EnergyCalc/EnerUser.c \
	$(LOCAL_DIR)/EnergyCalc/SocDisplay.c \
	$(LOCAL_DIR)/EnergyCalc/SoeDisplay.c \
	$(LOCAL_DIR)/EnergyCalc/SohDisplay.c \
	$(LOCAL_DIR)/IMEU3/IMEU3RcvHandle.c \
	$(LOCAL_DIR)/IMEU3/IMEU3SendInterface.c \
	$(LOCAL_DIR)/IMIU4/IMIU4RcvHandle.c \
	$(LOCAL_DIR)/IMIU4/IMIU4SendInterface.c \
	$(LOCAL_DIR)/UdsUpgrade/can_uds_arch.c \
	$(LOCAL_DIR)/UdsUpgrade/can_uds_interface.c \
	$(LOCAL_DIR)/UdsUpgrade/can_uds_network_layer.c \
	$(LOCAL_DIR)/UdsUpgrade/can_uds_services_layer.c \
	$(LOCAL_DIR)/UdsUpgrade/can_uds_upgrade_procedure.c \
	$(LOCAL_DIR)/UdsUpgrade/sid10_diagnostic_session_ctrl.c \
	$(LOCAL_DIR)/UdsUpgrade/sid11_ecu_reset.c \
	$(LOCAL_DIR)/UdsUpgrade/sid22_read_data_by_id.c \
	$(LOCAL_DIR)/UdsUpgrade/sid27_security_access.c \
	$(LOCAL_DIR)/UdsUpgrade/sid31_routine_ctrl.c \
	$(LOCAL_DIR)/UdsUpgrade/sid34_request_download.c \
	$(LOCAL_DIR)/UdsUpgrade/sid36_transfer_data.c \
	$(LOCAL_DIR)/UdsUpgrade/icc_upgrade_handler.c \
	$(LOCAL_DIR)/UdsUpgrade/uds_upgrade_manager.c \
	$(LOCAL_DIR)/UdsUpgrade/uds_upgrade_task.c \
	$(LOCAL_DIR)/EventRec/EventRec.c \

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-unused-but-set-variable

include $(MKROOT)/module.mk
