LOCAL_DIR := $(GET_LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(LOCAL_DIR)/Functions/inc \
	$(LOCAL_DIR)/Functions/OSUser \
	$(LOCAL_DIR)/Functions/Measurement \
	$(LOCAL_DIR)/Functions/Parameter \
	$(LOCAL_DIR)/Functions/IccMsg \
	$(LOCAL_DIR)/Functions/DataCalc \
	$(LOCAL_DIR)/Functions/Diagnosis \
	$(LOCAL_DIR)/Functions/Statistics \
	$(LOCAL_DIR)/Functions/EnergyCalc \
	$(LOCAL_DIR)/Functions/SocLib \
	$(LOCAL_DIR)/Functions/UdsUpgrade \
	$(LOCAL_DIR)/Functions/IMEU3 \
	$(LOCAL_DIR)/Functions/EventRec \
	$(LOCAL_DIR)/BSPSys/Device \
	$(LOCAL_DIR)/BSPSys/BSPCfg \
	$(LOCAL_DIR)/BSPSys/BSPUser \
	$(LOCAL_DIR)/BSPSys/Util \
	$(LOCAL_DIR)/Logics/Balance \
	$(LOCAL_DIR)/Logics/CanUser \
	$(LOCAL_DIR)/Logics/Tactics \
	$(LOCAL_DIR)/Addition/UserDef \
	$(LOCAL_DIR)/Addition/UserProto \

MODULE_DEPS += $(LOCAL_DIR)/Functions
MODULE_DEPS += $(LOCAL_DIR)/Logics
MODULE_DEPS += $(LOCAL_DIR)/BSPSys
MODULE_DEPS += $(LOCAL_DIR)/Addition


