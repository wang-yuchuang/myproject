/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef CMD_ECCP_H
#define CMD_ECCP_H

#include "cmd_common.h"
#include "cmd_trng.h"

#define ECDSA_RAND_TEST
#define ECDSA_RAND_TEST_COUNT     (2)

typedef struct {
    uint8_t *G_x_y;
    uint8_t *k;
    uint8_t *kG_x_y;
    uint8_t *dblG_x_y;
    uint8_t *kG_plus_dblG_x_y;
} cmd_eccp_basic_vectors_t;

typedef enum {
    SECP192R1 = 0,
    SECP224R1,
    SECP256R1,
    SECP384R1,
    SECP521R1,
} crypto_ECCcurvetype;

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[2];
    uint8_t curve_id;
    uint8_t inputPoint_ptr[4];
    uint8_t outputPoint_ptr[4];
} cmd_eccp_pointdoubling_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[2];
    uint8_t curve_id;
    uint8_t inputPoint1_ptr[4];
    uint8_t inputPoint2_ptr[4];
    uint8_t outputPoint_ptr[4];
} cmd_eccp_pointaddition_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[2];
    uint8_t curve_id;
    uint8_t inputScalar_ptr[4];
    uint8_t inputPoint_ptr[4];
    uint8_t outputPoint_ptr[4];
} cmd_eccp_pointmultiplication_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[2];
    uint8_t curve_id;
    uint8_t inputPoint_ptr[4];
} cmd_eccp_pointverifying_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t curve_id;
    uint8_t key_id[2];
    uint8_t pubKey_ptr[4];
    uint8_t priKey_ptr[4];
    uint8_t aut_info_ptr[4];
} cmd_eccp_generatekey_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[1];
    uint8_t curve_id;
    uint8_t key_len;
    uint8_t key_id0[2];
    uint8_t key_id1[2];
    uint8_t selfPrikey_ptr[4];
    uint8_t PeerPubKey_ptr[4];
    uint8_t rev2[2];
    uint8_t key_id2[2];
    uint8_t key_ptr[4];
    uint8_t aut_info_ptr[4];
} cmd_ecdh_exchangekey_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[1];
    uint8_t curve_id;
    uint8_t e_len;
    uint8_t e_ptr[4];
    uint8_t key_id0[2];
    uint8_t key_id1[2];
    uint8_t priKey_ptr[4];
    uint8_t signature_ptr[4];
    uint8_t aut_info_ptr[4];
} cmd_ecdsa_generatesignature_t;

typedef struct {
    uint8_t cmd_id;
    uint8_t rev1[1];
    uint8_t curve_id;
    uint8_t e_len;
    uint8_t e_ptr[4];
    uint8_t pubKey_ptr[4];
    uint8_t signature_ptr[4];
} cmd_ecdsa_verifysignature_t;

uint32_t cmd_eccp_point_doubling(uint8_t curve_id, uint8_t *G_x_y, uint8_t *output_x_y);

uint32_t cmd_eccp_point_addition(uint8_t curve_id, uint8_t *kG_x_y, uint8_t *dblG_x_y, uint8_t *output_x_y);

uint32_t cmd_eccp_point_multiplication(uint8_t curve_id, uint8_t *G_x_y, uint8_t *k, uint8_t *output_x_y);

uint32_t cmd_eccp_point_verify(uint8_t curve_id, uint8_t *G_x_y);

uint32_t cmd_eccp_ecdh_generate_keypair(uint8_t curve_id, uint8_t *prikey, uint8_t *pubkey, cmd_key_type_e generate_key_type, uint16_t generate_key_id, uint8_t *mac_buf);

/*mac_buf len is CMD_BASIC_TWO_KEY_MAC_BUFF_LEN 64*/
uint32_t cmd_eccp_ecdh_exchange_key(uint8_t curve_id, uint8_t key_len, uint8_t *KA, uint8_t *pubkey_B, uint8_t *prikey_A,
                                    cmd_key_type_e ecdh_key_type, uint16_t ecdh_key_id, cmd_key_type_e output_key_type, uint16_t output_key_id, uint8_t *mac_buf);

uint32_t cmd_eccp_ecdsa_gen_signature(uint8_t curve_id, uint8_t *e, uint32_t e_len, cmd_key_type_e sign_key_type, uint16_t sign_key_id, uint8_t *prikey, uint8_t *signature, uint8_t *mac_buf);

uint32_t cmd_eccp_ecdsa_verify_signature(uint8_t curve_id, uint8_t *e, uint32_t e_len, uint8_t *pubkey, uint8_t *signature);

#endif