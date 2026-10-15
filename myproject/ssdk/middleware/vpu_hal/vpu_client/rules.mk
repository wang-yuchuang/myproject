LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_SRCS += 							\
		$(LOCAL_DIR)/vpu_proto_client.c	\
		$(LOCAL_DIR)/vpu_proto_api.c 	\
		$(LOCAL_DIR)/vpu_proto_tool.c 	\

MODULE_DEPS += \

GLOBAL_INCLUDES += 		\
	$(LOCAL_DIR)/ 		\
	$(LOCAL_DIR)/proto/ \
	$(LOCAL_DIR)/../vpu_instance\
	$(LOCAL_DIR)/../vpu_instance/protobuf-c\

include $(MKROOT)/module.mk