LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

ifeq ($(CONFIG_OSR_CRYPTO_DRIVER), y)
GLOBAL_INCLUDES += \
    $(LOCAL_DIR)/ \
    $(LOCAL_DIR)/cacc/ \

MODULE_SRCS += \
	$(LOCAL_DIR)/osr_crypto_init.c \
	$(LOCAL_DIR)/cacc/ral/hash_basic.c \
	$(LOCAL_DIR)/cacc/ral/pke_prime.c \
	$(LOCAL_DIR)/cacc/ral/pke.c \
	$(LOCAL_DIR)/cacc/ral/ske_basic.c \
	$(LOCAL_DIR)/cacc/ral/trng.c \
	$(LOCAL_DIR)/cacc/ral/eccp_curve.c \
	$(LOCAL_DIR)/cacc/ral/utility.c \

endif

ifeq ($(CONFIG_OSR_CRYPTO_DRIVER_HASH), y)
MODULE_SRCS += \
	$(LOCAL_DIR)/cacc/hash_hmac/hash.c \
	$(LOCAL_DIR)/cacc/hash_hmac/hmac_md5.c \
	$(LOCAL_DIR)/cacc/hash_hmac/hmac_sha1.c \
	$(LOCAL_DIR)/cacc/hash_hmac/hmac_sha224.c \
	$(LOCAL_DIR)/cacc/hash_hmac/hmac_sha256.c \
	$(LOCAL_DIR)/cacc/hash_hmac/hmac_sha384.c \
	$(LOCAL_DIR)/cacc/hash_hmac/hmac_sha512_224.c \
	$(LOCAL_DIR)/cacc/hash_hmac/hmac_sha512_256.c \
	$(LOCAL_DIR)/cacc/hash_hmac/hmac_sha512.c \
	$(LOCAL_DIR)/cacc/hash_hmac/hmac_sm3.c \
	$(LOCAL_DIR)/cacc/hash_hmac/hmac.c \
	$(LOCAL_DIR)/cacc/hash_hmac/md5.c \
	$(LOCAL_DIR)/cacc/hash_hmac/sha1.c \
	$(LOCAL_DIR)/cacc/hash_hmac/sha224.c \
	$(LOCAL_DIR)/cacc/hash_hmac/sha256.c \
	$(LOCAL_DIR)/cacc/hash_hmac/sha384.c \
	$(LOCAL_DIR)/cacc/hash_hmac/sha512_224.c \
	$(LOCAL_DIR)/cacc/hash_hmac/sha512_256.c \
	$(LOCAL_DIR)/cacc/hash_hmac/sha512.c \
	$(LOCAL_DIR)/cacc/hash_hmac/sm3.c \
	$(LOCAL_DIR)/cacc/hash_hmac/sha3_224.c \
	$(LOCAL_DIR)/cacc/hash_hmac/sha3_256.c \
	$(LOCAL_DIR)/cacc/hash_hmac/sha3_384.c \
	$(LOCAL_DIR)/cacc/hash_hmac/sha3_512.c \

endif

ifeq ($(CONFIG_OSR_CRYPTO_DRIVER_PKE), y)

MODULE_SRCS += \
	$(LOCAL_DIR)/cacc/pke/ecdh.c \
	$(LOCAL_DIR)/cacc/pke/ecdsa.c \
	$(LOCAL_DIR)/cacc/pke/rsa.c \
	$(LOCAL_DIR)/cacc/pke/sm2_basic.c \
	$(LOCAL_DIR)/cacc/pke/sm2.c \
	$(LOCAL_DIR)/cacc/pke/sm9.c \
	$(LOCAL_DIR)/cacc/pke/x25519.c \
	$(LOCAL_DIR)/cacc/pke/ed25519.c \

endif

ifeq ($(CONFIG_OSR_CRYPTO_DRIVER_AES), y)

MODULE_SRCS += \
	$(LOCAL_DIR)/cacc/ske/ske_cbc_mac.c \
	$(LOCAL_DIR)/cacc/ske/ske_cbc.c \
	$(LOCAL_DIR)/cacc/ske/ske_cfb.c \
	$(LOCAL_DIR)/cacc/ske/ske_cmac.c \
	$(LOCAL_DIR)/cacc/ske/ske_ctr.c \
	$(LOCAL_DIR)/cacc/ske/ske_ecb.c \
	$(LOCAL_DIR)/cacc/ske/ske_gcm.c \
	$(LOCAL_DIR)/cacc/ske/ske_ofb.c \
	$(LOCAL_DIR)/cacc/ske/ske_xts.c \
	$(LOCAL_DIR)/cacc/ske/ske.c \

endif

ifeq ($(CONFIG_OSR_CRYPTO_DRIVER_MAILBOX), y)

MODULE_SRCS += \
	$(LOCAL_DIR)/mailbox/cmd_basic_mgmt.c \
	$(LOCAL_DIR)/mailbox/cmd_eccp.c \
	$(LOCAL_DIR)/mailbox/cmd_hmac.c \
	$(LOCAL_DIR)/mailbox/cmd_ske_basic.c \
	$(LOCAL_DIR)/mailbox/cmd_ske_mac.c \
	$(LOCAL_DIR)/mailbox/cmd_sm2.c \
	$(LOCAL_DIR)/mailbox/cmd_common.c \
	$(LOCAL_DIR)/mailbox/cmd_hash.c \
	$(LOCAL_DIR)/mailbox/cmd_rsa.c \
	$(LOCAL_DIR)/mailbox/cmd_ske_gcm.c \
	$(LOCAL_DIR)/mailbox/cmd_ske_xts.c \
	$(LOCAL_DIR)/mailbox/cmd_sm9.c \
	$(LOCAL_DIR)/mailbox/cmd_trng.c \
	$(LOCAL_DIR)/mailbox/soc_otp_key.c \

endif

MODULE_COMPILEFLAGS += -Wno-format -fno-builtin -Wno-unused-variable -Wno-sign-compare -Wno-format -Wno-int-to-void-pointer-cast

include $(MKROOT)/module.mk