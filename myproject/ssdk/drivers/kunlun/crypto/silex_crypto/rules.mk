SILEX_LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(SILEX_LOCAL_DIR)

GLOBAL_INCLUDES += \
	$(SILEX_LOCAL_DIR)/include/

include $(SILEX_LOCAL_DIR)/crypto_hal/rules.mk

MODULE_SRCS += \
	$(SILEX_LOCAL_DIR)/sram_conf.c \
	$(SILEX_LOCAL_DIR)/ce.c \
	$(SILEX_LOCAL_DIR)/sx_hash.c \
	$(SILEX_LOCAL_DIR)/sx_cipher.c \
	$(SILEX_LOCAL_DIR)/sx_pke_conf.c \
	$(SILEX_LOCAL_DIR)/sx_rsa_pad.c \
	$(SILEX_LOCAL_DIR)/sx_dma.c \
	$(SILEX_LOCAL_DIR)/sx_rsa.c \
	$(SILEX_LOCAL_DIR)/sx_trng.c \
	$(SILEX_LOCAL_DIR)/sx_ecc_keygen.c \
	$(SILEX_LOCAL_DIR)/sx_ecdsa.c \
	$(SILEX_LOCAL_DIR)/sx_sm2.c \
	$(SILEX_LOCAL_DIR)/sx_dsa.c \
	$(SILEX_LOCAL_DIR)/sx_srp.c \
	$(SILEX_LOCAL_DIR)/sx_eddsa.c \
	$(SILEX_LOCAL_DIR)/sx_pke_funcs.c \
	$(SILEX_LOCAL_DIR)/sx_ecc.c \
	$(SILEX_LOCAL_DIR)/sx_math.c \
	$(SILEX_LOCAL_DIR)/sx_prime.c

GLOBAL_DEFINES += \
	WAIT_PK_WITH_REGISTER_POLLING=0\
	RSA_PERFORMANCE_TEST=1

include $(MKROOT)/module.mk