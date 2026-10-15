/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#include <driver/taishan/crypto/osr_crypto/cacc/hash_hmac/hash.h>

#define LOCAL_TRACE 0

/*HASH IV definition*/
#ifndef HASH_CPU_BIG_ENDIAN

#ifdef SUPPORT_HASH_SM3
uint32_t const SM3_IV[8]         = {0x6F168073, 0xB9B21449, 0xD7422417, 0x00068ADA, 0xBC306FA9, 0xAA383116, 0x4DEE8DE3, 0x4E0EFBB0,};
#endif

#ifdef SUPPORT_HASH_MD5
uint32_t const MD5_IV[4]         = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476,};
#endif

#ifdef SUPPORT_HASH_SHA256
uint32_t const SHA256_IV[8]      = {0x67E6096A, 0x85AE67BB, 0x72F36E3C, 0x3AF54FA5, 0x7F520E51, 0x8C68059B, 0xABD9831F, 0x19CDE05B,};
#endif

#ifdef SUPPORT_HASH_SHA384
uint32_t const SHA384_IV[16]     = {0x5D9DBBCB, 0xD89E05C1, 0x2A299A62, 0x07D57C36, 0x5A015991, 0x17DD7030, 0xD8EC2F15, 0x39590EF7,
                                    0x67263367, 0x310BC0FF, 0x874AB48E, 0x11155868, 0x0D2E0CDB, 0xA78FF964, 0x1D48B547, 0xA44FFABE,
                                   };
#endif

#ifdef SUPPORT_HASH_SHA512
uint32_t const SHA512_IV[16]     = {0x67E6096A, 0x08C9BCF3, 0x85AE67BB, 0x3BA7CA84, 0x72F36E3C, 0x2BF894FE, 0x3AF54FA5, 0xF1361D5F,
                                    0x7F520E51, 0xD182E6AD, 0x8C68059B, 0x1F6C3E2B, 0xABD9831F, 0x6BBD41FB, 0x19CDE05B, 0x79217E13,
                                   };
#endif

#ifdef SUPPORT_HASH_SHA1
uint32_t const SHA1_IV[5]        = {0x01234567, 0x89ABCDEF, 0xFEDCBA98, 0x76543210, 0xF0E1D2C3,};
#endif

#ifdef SUPPORT_HASH_SHA224
uint32_t const SHA224_IV[8]      = {0xD89E05C1, 0x07D57C36, 0x17DD7030, 0x39590EF7, 0x310BC0FF, 0x11155868, 0xA78FF964, 0xA44FFABE,};
#endif

#ifdef SUPPORT_HASH_SHA512_224
uint32_t const SHA512_224_IV[16] = {0xC8373D8C, 0xA24D5419, 0x6699E173, 0xD6D4DC89, 0xAEB7FA1D, 0x829CFF32, 0x14D59D67, 0xCF9F2F58,
                                    0x692B6D0F, 0xA84DD47B, 0x736FE377, 0x4289C404, 0xA8859D3F, 0xC8361D6A, 0xADE61211, 0xA192D691,
                                   };
#endif

#ifdef SUPPORT_HASH_SHA512_256
uint32_t const SHA512_256_IV[16] = {0x94213122, 0x2CF72BFC, 0xA35F559F, 0xC2644CC8, 0x6BB89323, 0x51B1536F, 0x19773896, 0xBDEA4059,
                                    0xE23E2896, 0xE3FF8EA8, 0x251E5EBE, 0x92398653, 0xFC99012B, 0xAAB8852C, 0xDC2DB70E, 0xA22CC581,
                                   };
#endif

#else

#ifdef SUPPORT_HASH_SM3
uint32_t const SM3_IV[8]         = {0x7380166f, 0x4914b2b9, 0x172442d7, 0xda8a0600, 0xa96f30bc, 0x163138aa, 0xe38dee4d, 0xb0fb0e4e,};
#endif

#ifdef SUPPORT_HASH_MD5
uint32_t const MD5_IV[4]         = {0x01234567, 0x89ABCDEF, 0xFEDCBA98, 0x76543210,};
#endif

#ifdef SUPPORT_HASH_SHA256
uint32_t const SHA256_IV[8]      = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19,};
#endif

#ifdef SUPPORT_HASH_SHA384
uint32_t const SHA384_IV[16]     = {0xcbbb9d5d, 0xc1059ed8, 0x629a292a, 0x367cd507, 0x9159015a, 0x3070dd17, 0x152fecd8, 0xf70e5939,
                                    0x67332667, 0xffc00b31, 0x8eb44a87, 0x68581511, 0xdb0c2e0d, 0x64f98fa7, 0x47b5481d, 0xbefa4fa4,
                                   };
#endif

#ifdef SUPPORT_HASH_SHA512
uint32_t const SHA512_IV[16]     = {0x6a09e667, 0xf3bcc908, 0xbb67ae85, 0x84caa73b, 0x3c6ef372, 0xfe94f82b, 0xa54ff53a, 0x5f1d36f1,
                                    0x510e527f, 0xade682d1, 0x9b05688c, 0x2b3e6c1f, 0x1f83d9ab, 0xfb41bd6b, 0x5be0cd19, 0x137e2179,
                                   };
#endif

#ifdef SUPPORT_HASH_SHA1
uint32_t const SHA1_IV[5]        = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0,};
#endif

#ifdef SUPPORT_HASH_SHA224
uint32_t const SHA224_IV[8]      = {0xc1059ed8, 0x367cd507, 0x3070dd17, 0xf70e5939, 0xffc00b31, 0x68581511, 0x64f98fa7, 0xbefa4fa4,};
#endif

#ifdef SUPPORT_HASH_SHA512_224
uint32_t const SHA512_224_IV[16] = {0x8C3D37C8, 0x19544DA2, 0x73E19966, 0x89DCD4D6, 0x1DFAB7AE, 0x32FF9C82, 0x679DD514, 0x582F9FCF,
                                    0x0F6D2B69, 0x7BD44DA8, 0x77E36F73, 0x04C48942, 0x3F9D85A8, 0x6A1D36C8, 0x1112E6AD, 0x91D692A1,
                                   };
#endif

#ifdef SUPPORT_HASH_SHA512_256
uint32_t const SHA512_256_IV[16] = {0x22312194, 0xFC2BF72C, 0x9F555FA3, 0xC84C64C2, 0x2393B86B, 0x6F53B151, 0x96387719, 0x5940EABD,
                                    0x96283EE2, 0xA88EFFE3, 0xBE5E1E25, 0x53863992, 0x2B0199FC, 0x2C85B8AA, 0x0EB72DDC, 0x81C52CA2,
                                   };
#endif

#endif

/* function: check whether the hash algorithm is valid or not
 * parameters:
 *     hash_alg ------------------- input, specific hash algorithm
 * return: HASH_SUCCESS(valid), other(invalid)
 * caution:
 *     1.
 */
uint8_t check_hash_alg(HASH_ALG hash_alg)
{
    uint8_t ret;

    switch (hash_alg) {
#ifdef SUPPORT_HASH_SM3
        case HASH_SM3:
#endif

#ifdef SUPPORT_HASH_MD5
        case HASH_MD5:
#endif

#ifdef SUPPORT_HASH_SHA256
        case HASH_SHA256:
#endif

#ifdef SUPPORT_HASH_SHA384
        case HASH_SHA384:
#endif

#ifdef SUPPORT_HASH_SHA512
        case HASH_SHA512:
#endif

#ifdef SUPPORT_HASH_SHA1
        case HASH_SHA1:
#endif

#ifdef SUPPORT_HASH_SHA224
        case HASH_SHA224:
#endif

#ifdef SUPPORT_HASH_SHA512_224
        case HASH_SHA512_224:
#endif

#ifdef SUPPORT_HASH_SHA512_256
        case HASH_SHA512_256:
#endif

#ifdef SUPPORT_HASH_SHA3_224
        case HASH_SHA3_224:
#endif

#ifdef SUPPORT_HASH_SHA3_256
        case HASH_SHA3_256:
#endif

#ifdef SUPPORT_HASH_SHA3_384
        case HASH_SHA3_384:
#endif

#ifdef SUPPORT_HASH_SHA3_512
        case HASH_SHA3_512:
#endif
            ret = HASH_SUCCESS;
            break;

        default:
            ret = HASH_INPUT_INVALID;
            break;
    }

    return ret;
}


/* function: get hash block word length
 * parameters:
 *     hash_alg ------------------- input, specific hash algorithm
 * return: hash block word length
 * caution:
 *     1. please make sure hash_alg is valid
 */
uint8_t hash_get_block_word_len(HASH_ALG hash_alg)
{
    uint8_t block_words;

    switch (hash_alg) {
#ifdef SUPPORT_HASH_SM3
        case HASH_SM3:
#endif
#ifdef SUPPORT_HASH_MD5
        case HASH_MD5:
#endif
#ifdef SUPPORT_HASH_SHA1
        case HASH_SHA1:
#endif
#ifdef SUPPORT_HASH_SHA256
        case HASH_SHA256:
#endif
#ifdef SUPPORT_HASH_SHA224
        case HASH_SHA224:
#endif
#if (defined(SUPPORT_HASH_SM3) || defined(SUPPORT_HASH_MD5) || defined(SUPPORT_HASH_SHA1) || defined(SUPPORT_HASH_SHA256) || defined(SUPPORT_HASH_SHA224))
            block_words = 16;
            break;
#endif

#ifdef SUPPORT_HASH_SHA384
        case HASH_SHA384:
#endif
#ifdef SUPPORT_HASH_SHA512
        case HASH_SHA512:
#endif
#ifdef SUPPORT_HASH_SHA512_224
        case HASH_SHA512_224:
#endif
#ifdef SUPPORT_HASH_SHA512_256
        case HASH_SHA512_256:
#endif
#if (defined(SUPPORT_HASH_SHA384) || defined(SUPPORT_HASH_SHA512) || defined(SUPPORT_HASH_SHA512_224) || defined(SUPPORT_HASH_SHA512_256))
            block_words = 32;
            break;
#endif

#ifdef SUPPORT_HASH_SHA3_224
        case HASH_SHA3_224:
            block_words = 36;
            break;
#endif

#ifdef SUPPORT_HASH_SHA3_256
        case HASH_SHA3_256:
            block_words = 34;
            break;
#endif

#ifdef SUPPORT_HASH_SHA3_384
        case HASH_SHA3_384:
            block_words = 26;
            break;
#endif

#ifdef SUPPORT_HASH_SHA3_512
        case HASH_SHA3_512:
            block_words = 18;
            break;
#endif

        default:
            block_words = 0;
            break;
    }

    return block_words;
}


/* function: get hash iterator word length
 * parameters:
 *     hash_alg ------------------- input, specific hash algorithm
 * return: hash iterator word length
 * caution:
 *     1. please make sure hash_alg is valid
 */
uint8_t hash_get_iterator_word_len(HASH_ALG hash_alg)
{
    uint8_t iterator_words;

    switch (hash_alg) {
#ifdef SUPPORT_HASH_MD5
        case HASH_MD5:
            iterator_words = 4;
            break;
#endif

#ifdef SUPPORT_HASH_SHA1
        case HASH_SHA1:
            iterator_words = 5;
            break;
#endif

#ifdef SUPPORT_HASH_SM3
        case HASH_SM3:
#endif
#ifdef SUPPORT_HASH_SHA256
        case HASH_SHA256:
#endif
#ifdef SUPPORT_HASH_SHA224
        case HASH_SHA224:
#endif
#if (defined(SUPPORT_HASH_SM3) || defined(SUPPORT_HASH_SHA256) || defined(SUPPORT_HASH_SHA224))
            iterator_words = 8;
            break;
#endif

#ifdef SUPPORT_HASH_SHA384
        case HASH_SHA384:
#endif
#ifdef SUPPORT_HASH_SHA512
        case HASH_SHA512:
#endif
#ifdef SUPPORT_HASH_SHA512_224
        case HASH_SHA512_224:
#endif
#ifdef SUPPORT_HASH_SHA512_256
        case HASH_SHA512_256:
#endif
#if (defined(SUPPORT_HASH_SHA384) || defined(SUPPORT_HASH_SHA512) || defined(SUPPORT_HASH_SHA512_224) || defined(SUPPORT_HASH_SHA512_256))
            iterator_words = 16;
            break;
#endif

#ifdef SUPPORT_HASH_SHA3_224
        case HASH_SHA3_224:
#endif

#ifdef SUPPORT_HASH_SHA3_256
        case HASH_SHA3_256:
#endif

#ifdef SUPPORT_HASH_SHA3_384
        case HASH_SHA3_384:
#endif

#ifdef SUPPORT_HASH_SHA3_512
        case HASH_SHA3_512:
#endif

#if (defined(SUPPORT_HASH_SHA3_224) || defined(SUPPORT_HASH_SHA3_256) ||defined(SUPPORT_HASH_SHA3_384) ||defined(SUPPORT_HASH_SHA3_512))
            iterator_words = 50;
            break;
#endif

        default:
            iterator_words = 0;
            break;
    }

    return iterator_words;
}


/* function: get hash digest word length
 * parameters:
 *     hash_alg ------------------- input, specific hash algorithm
 * return: hash digest word length
 * caution:
 *     1. please make sure hash_alg is valid
 */
uint8_t hash_get_digest_word_len(HASH_ALG hash_alg)
{
    uint8_t digest_words;

    switch (hash_alg) {
#ifdef SUPPORT_HASH_MD5
        case HASH_MD5:
            digest_words = 4;
            break;
#endif

#ifdef SUPPORT_HASH_SHA1
        case HASH_SHA1:
            digest_words = 5;
            break;
#endif

#ifdef SUPPORT_HASH_SHA224
        case HASH_SHA224:
#endif
#ifdef SUPPORT_HASH_SHA512_224
        case HASH_SHA512_224:
#endif
#if (defined(SUPPORT_HASH_SHA224) || defined(SUPPORT_HASH_SHA512_224))
            digest_words = 7;
            break;
#endif

#ifdef SUPPORT_HASH_SM3
        case HASH_SM3:
#endif
#ifdef SUPPORT_HASH_SHA256
        case HASH_SHA256:
#endif
#ifdef SUPPORT_HASH_SHA512_256
        case HASH_SHA512_256:
#endif
#if (defined(SUPPORT_HASH_SM3) || defined(SUPPORT_HASH_SHA256) || defined(SUPPORT_HASH_SHA512_256))
            digest_words = 8;
            break;
#endif

#ifdef SUPPORT_HASH_SHA384
        case HASH_SHA384:
            digest_words = 12;
            break;
#endif

#ifdef SUPPORT_HASH_SHA512
        case HASH_SHA512:
            digest_words = 16;
            break;
#endif

#ifdef SUPPORT_HASH_SHA3_224
        case HASH_SHA3_224:
            digest_words = 7;
            break;
#endif
#ifdef SUPPORT_HASH_SHA3_256
        case HASH_SHA3_256:
            digest_words = 8;
            break;
#endif
#ifdef SUPPORT_HASH_SHA3_384
        case HASH_SHA3_384:
            digest_words = 12;
            break;
#endif
#ifdef SUPPORT_HASH_SHA3_512
        case HASH_SHA3_512:
            digest_words = 16;
            break;
#endif

        default:
            digest_words = 0;
            break;
    }

    return digest_words;
}


/* function: hash message total byte length a = a+b
 * parameters:
 *     a -------------------------- input&output, big number a, total byte length of hash message
 *     a_words -------------------- input, word length of buffer a
 *     b -------------------------- input, integer to be added to a
 * return: 0(success), other(error, hash total length overflow)
 * caution:
 */
uint8_t hash_total_len_add_uint32(uint32_t *a, uint32_t a_words, uint32_t b)
{
    uint32_t i;

    for (i = 0; i < a_words; i++) {
        a[i] += b;

        if (a[i] < b) {
            b = 1;
        }
        else {
            break;
        }
    }

    if (i == a_words) {
        return 1;
    }
    else if (a[a_words - 1] & 0xE0000000) {
        /*bit length overflow*/
        return 1;
    }
    else {
        return 0;
    }
}


/* function: transform hash message total byte length to bit length
 * parameters:
 *     a -------------------------- input&output, big number a
 *     a_words -------------------- input, word length of buffer a
 * return: none
 * caution:
 */
void hash_total_bytelen_2_bitlen(uint32_t *a, uint32_t a_words)
{
    int32_t i;

    for (i = a_words - 1; i > 0; i--) {
        a[i] <<= 3;
        a[i] |= a[i - 1] >> (32 - 3);
    }

    a[i] <<= 3;
}

/* function: input actual bit length of current input message
 * parameters:
 *     bits ----------------------- input, actual bit length of current input message
 * return: none
 * caution:
 *     1. before starting please set the actual message bit length
 */
void hash_set_tx_bit_len(hash_ctx_t *ctx, uint32_t block_count)
{
    uint32_t tmp = ctx->block_byte_len * block_count;

    hash_set_msg_len(tmp);
}

/* function: input hash IV
 * parameters:
 *     hash_alg ------------------- input, specific hash algorithm
 *     hash_iterator_words -------- input, iterator word length
 * return: none
 * caution:
 *     1.
 */
void hash_set_IV(HASH_ALG hash_alg, uint8_t hash_iterator_words)
{
    uint32_t *iv;

    switch (hash_alg) {
#ifdef SUPPORT_HASH_SM3
        case HASH_SM3:
            iv = (uint32_t *)SM3_IV;
            break;
#endif

#ifdef SUPPORT_HASH_MD5
        case HASH_MD5:
            iv = (uint32_t *)MD5_IV;
            break;
#endif

#ifdef SUPPORT_HASH_SHA256
        case HASH_SHA256:
            iv = (uint32_t *)SHA256_IV;
            break;
#endif

#ifdef SUPPORT_HASH_SHA384
        case HASH_SHA384:
            iv = (uint32_t *)SHA384_IV;
            break;
#endif

#ifdef SUPPORT_HASH_SHA1
        case HASH_SHA1:
            iv = (uint32_t *)SHA1_IV;
            break;
#endif

#ifdef SUPPORT_HASH_SHA512
        case HASH_SHA512:
            iv = (uint32_t *)SHA512_IV;
            break;
#endif

#ifdef SUPPORT_HASH_SHA224
        case HASH_SHA224:
            iv = (uint32_t *)SHA224_IV;
            break;
#endif

#ifdef SUPPORT_HASH_SHA512_224
        case HASH_SHA512_224:
            iv = (uint32_t *)SHA512_224_IV;
            break;
#endif

#ifdef SUPPORT_HASH_SHA512_256
        case HASH_SHA512_256:
            iv = (uint32_t *)SHA512_256_IV;
            break;
#endif

        //here iv = NULL means SHA3 IV is zero of 1600 bits
        default:
            iv = NULL;
            break;
    }

    hash_set_iterator(iv, hash_iterator_words);
}


/* function: start HASH iteration calc
 * parameters:
 *     ctx ------------------------ input, hash_ctx_t context pointer
 * return: none
 * caution:
 */
void hash_start_calculate(hash_ctx_t *ctx)
{
    if (ctx->first_update_flag) {
        hash_set_IV(ctx->hash_alg, ctx->iterator_word_len);
        ctx->first_update_flag = 0;
    }
    else
    {;}

    hash_start();
}


/* function: init HASH
 * parameters:
 *     ctx ------------------------ input, hash_ctx_t context pointer
 *     hash_alg ------------------- input, specific hash algorithm
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure hash_alg is valid
 */
uint32_t hash_init(hash_ctx_t *ctx, HASH_ALG hash_alg)
{
    if (NULL == ctx) {
        return HASH_BUFFER_NULL;
    }
    else if (HASH_SUCCESS != check_hash_alg(hash_alg)) {
        return HASH_INPUT_INVALID;
    }
    else
    {;}

    memset_(ctx, 0, sizeof(hash_ctx_t));

    hash_set_cpu_mode();

    hash_set_hash_mode();

    hash_set_endian_uint32();

    hash_clear_msg_len();

    hash_disable_cpu_interruption();

    hash_set_last_block(0);

    hash_set_alg(hash_alg);

    hash_update_config();

    ctx->hash_alg = hash_alg;
    ctx->block_byte_len = hash_get_block_word_len(hash_alg) << 2;
    ctx->iterator_word_len = hash_get_iterator_word_len(hash_alg);
    ctx->digest_byte_len = hash_get_digest_word_len(hash_alg) << 2;
    ctx->status.busy = 0;
    ctx->first_update_flag = 1;
    ctx->finish_flag = 0;
    uint32_clear(ctx->total, ctx->block_byte_len / 32);

    return HASH_SUCCESS;
}


/* function: hash iterate calc with some blocks
 * parameters:
 *     ctx ------------------------ input, hash_ctx_t context pointer
 *     msg ------------------------ input, message of some blocks
 *     block_count ---------------- input, count of blocks
 * return: none
 * caution:
 *     1. please make sure the three parameters is valid
 */
static void hash_calc_blocks(hash_ctx_t *ctx, const uint8_t *msg, uint32_t block_count)
{
    uint8_t block_word_len = (ctx->block_byte_len) >> 2;

#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
    /*set the input iterator data*/
    if (1 != ctx->first_update_flag) {
        hash_set_iterator(ctx->iterator, ctx->iterator_word_len);
    }
    else
    {;}

#endif
    /*set the bit length of the input blocks*/
    hash_set_tx_bit_len(ctx, block_count);

    hash_start_calculate(ctx);

    while (block_count--) {
        /*input the block message*/
        hash_input_msg((uint8_t *)msg, block_word_len);
        msg += ctx->block_byte_len;
    }

    hash_wait_till_done();

#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
    /*if message update not done, get the new iterator hash value*/
    if (1 != ctx->finish_flag) {
        hash_get_iterator((uint8_t *)(ctx->iterator), ctx->iterator_word_len);
    }
    else
    {;}

#endif
}


/* function: hash iterate calc with padding
 * parameters:
 *     ctx ------------------------ input, hash_ctx_t context pointer
 *     msg ------------------------ input, message that contains the last block(maybe not full)
 *     bytes ---------------------- input, byte length of msg
 * return: none
 * caution:
 *     1. msg contains the last byte of the total message while the total message length is not a
 *        multiple of hash block length, otherwise byte length of msg is zero.
 *     2. at present this function does not support the case that byte length of msg is a multiple
 *        of hash block length. actually msg_bytes here must be less than the hash block byte length,
 *        namely, this function is just for the remainder message, and will do padding, finally get
 *        digest.
 *     3. before calling this function, some blocks(could be 0 block) must be calculated.
 */
static void hash_calc_rand_len_msg(hash_ctx_t *ctx, const uint8_t *msg, uint32_t msg_bytes)
{

#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
    /*set the input iterator data*/
    if (1 != ctx->first_update_flag) {
        hash_set_iterator(ctx->iterator, ctx->iterator_word_len);
    }
    else
    {;}
#endif

    hash_set_last_block(1);

    hash_start_calculate(ctx);

    hash_input_msg((uint8_t *)msg, (msg_bytes + 3) / 4);

    hash_wait_till_done();
}


/* function: hash update message
 * parameters:
 *     ctx ------------------------ input, hash_ctx_t context pointer
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the three parameters are valid, and ctx is initialized
 */
uint32_t hash_update(hash_ctx_t *ctx, const uint8_t *msg, uint32_t msg_bytes)
{
    uint32_t count;
    uint8_t left, fill;

    if ((NULL == ctx)) {
        return HASH_BUFFER_NULL;
    }
    else if ((NULL == msg) || (0 == msg_bytes)) {
        return HASH_SUCCESS;
    }
    else
    {;}

    /*start to update processing*/
    ctx->status.busy = 1;

#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
    hash_set_cpu_mode();
    hash_set_hash_mode();

    hash_set_endian_uint32();

    hash_disable_cpu_interruption();

    /*set not the last block*/
    hash_set_last_block(0);

    hash_set_alg(ctx->hash_alg);

    hash_update_config();
#endif
    /*byte length of valid message left in block buffer*/
    /*byte length that block buffer need to fill a block*/
    left = ctx->total[0] % (ctx->block_byte_len);
    fill = (ctx->block_byte_len) - left;

    if(hash_check_whether_sha3_alg(ctx->hash_alg)) {
        ctx->total[0] = (left + msg_bytes) % (ctx->block_byte_len);
    }
    else
    {
        /*update total byte length*/
        if (hash_total_len_add_uint32(ctx->total, ctx->block_byte_len / 32, msg_bytes)) {
            return HASH_LEN_OVERFLOW;
        }
        else
        {;}
    }

    if (left) {
        if (msg_bytes >= fill) {
            memcpy_(ctx->hash_buffer + left, (uint8_t *)msg, fill);
            hash_calc_blocks(ctx, ctx->hash_buffer, 1);
            msg_bytes -= fill;
            msg += fill;
        }
        else {
            memcpy_(ctx->hash_buffer + left, (uint8_t *)msg, msg_bytes);
            goto end;
        }
    }
    else
    {;}

    /*process some blocks*/
    count = msg_bytes / (ctx->block_byte_len);

    if (count) {
        hash_calc_blocks(ctx, msg, count);
    }
    else
    {;}

    /*process the remainder*/
    msg += (ctx->block_byte_len) * count;
    msg_bytes = msg_bytes % (ctx->block_byte_len);

    if (msg_bytes) {
        memcpy_(ctx->hash_buffer, (uint8_t *)msg, msg_bytes);
    }
    else
    {;}

end:
    /*update end, status becomes idle*/
    ctx->status.busy = 0;

    return HASH_SUCCESS;
}


/* function: message update done, get the digest
 * parameters:
 *     ctx ------------------------ input, hash_ctx_t context pointer
 *     digest --------------------- output, hash digest
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the ctx is valid and initialized
 *     2. please make sure the digest buffer is sufficient
 */
uint32_t hash_final(hash_ctx_t *ctx, uint8_t *digest)
{
    uint8_t tmp;

    if ((NULL == ctx) || (NULL == digest)) {
        return HASH_BUFFER_NULL;
    }
    else
    {;}

#ifdef CONFIG_HASH_SUPPORT_MUL_THREAD
    hash_set_cpu_mode();

    hash_set_endian_uint32();

    hash_disable_cpu_interruption();
    /*set not the last block*/
    hash_set_last_block(0);

    hash_set_alg(ctx->hash_alg);

    hash_update_config();
#endif
    /*the last block calc*/
    ctx->finish_flag = 1;

    /*get the byte length of the remainder msg(less than one block)*/
    tmp = ctx->total[0] % (ctx->block_byte_len);

    if(hash_check_whether_sha3_alg(ctx->hash_alg)) {
        /*set remainder msg bit length*/
        hash_set_msg_len(tmp);
    }
    else {
        /*set total msg bit length*/
        hash_total_bytelen_2_bitlen(ctx->total, (ctx->block_byte_len) / 32);
        hash_set_msg_total_bit_len(ctx->total, ctx->block_byte_len);
    }

    /*input the remainder msg(less than one block)*/
    hash_calc_rand_len_msg(ctx, ctx->hash_buffer, tmp);

    /*get the hash result*/
    hash_get_iterator(digest, (ctx->digest_byte_len) >> 2);

    /*clear the context*/
    memset_(ctx, 0, sizeof(hash_ctx_t));

    return HASH_SUCCESS;
}


/* function: input whole message and get its digest
 * parameters:
 *     hash_alg ------------------- input, specific hash algorithm
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the input message, it could be 0
 *     digest --------------------- output, hash digest
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the digest buffer is sufficient
 */
uint32_t hash(HASH_ALG hash_alg, uint8_t *msg, uint32_t msg_bytes, uint8_t *digest)
{
    hash_ctx_t ctx[1];
    uint32_t ret;

    ret = hash_init(ctx,  hash_alg);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ret = hash_update(ctx, msg, msg_bytes);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    ret = hash_final(ctx, digest);

    if (HASH_SUCCESS != ret) {
        memset_(digest, 0, ctx->digest_byte_len);
    }
    else
    {;}

    /*clear the context*/
    memset_(ctx, 0, sizeof(hash_ctx_t));

    return ret;
}


#ifdef HASH_DMA_FUNCTION
/* function: init dma hash
 * parameters:
 *     ctx ------------------------ input, hash_dma_ctx_t context pointer
 *     hash_alg ------------------- input, specific hash algorithm
 *     callback ------------------- callback function pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 */
uint32_t hash_dma_init(hash_dma_ctx_t *ctx, HASH_ALG hash_alg, HASH_CALLBACK callback)
{
    if (NULL == ctx || NULL == callback) {
        return HASH_BUFFER_NULL;
    }
    else if (HASH_SUCCESS != check_hash_alg(hash_alg)) {
        return HASH_INPUT_INVALID;
    }
    else
    {;}

    hash_set_cpu_mode();
    hash_set_hash_mode();
    hash_set_endian_uint32();
    hash_clear_msg_len();
    hash_disable_cpu_interruption();
    hash_set_last_block(0);
    hash_set_alg(hash_alg);
    hash_update_config();
    hash_clear_risp();

    hash_set_dma_wlen(hash_get_digest_word_len(hash_alg) * 32);

    hash_set_dma_mode();

    /*init context*/
    ctx->hash_alg = hash_alg;
    ctx->block_word_len = hash_get_block_word_len(hash_alg);
    ctx->callback = callback;
    uint32_clear(ctx->total, sizeof(ctx->total) / 4);

    /*set IV*/
    hash_set_IV(hash_alg, hash_get_iterator_word_len(hash_alg));

    return HASH_SUCCESS;
}


/* function: dma hash update some message blocks
 * parameters:
 *     ctx ------------------------ input, hash_dma_ctx_t context pointer
 *     msg ------------------------ input, message blocks
 *     msg_words ------------------ input, word length of the input message, must be multiple of hash block word length
 *     iterator ------------------- output, hash temporary result
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the four parameters are valid, and ctx is initialized
 */
uint32_t hash_dma_update_blocks(hash_dma_ctx_t *ctx, uint32_t *msg, uint32_t msg_words, uint32_t *iterator)
{
    uint32_t msgByteLen;

    if ((NULL == ctx) || (NULL == iterator)) {
        return HASH_BUFFER_NULL;
    }
    else if ((NULL == msg) || (0 == msg_words)) {
        return HASH_SUCCESS;
    }
    else if (msg_words % ctx->block_word_len) {
        return HASH_INPUT_INVALID;
    }
    else
    {;}

    msgByteLen = msg_words * 4;
    if(!hash_check_whether_sha3_alg(ctx->hash_alg))
    {
        /*update total byte length*/
        if (hash_total_len_add_uint32(ctx->total, ctx->block_word_len / 8, msgByteLen)) {
            return HASH_LEN_OVERFLOW;
        }
    }

    hash_set_msg_len(msgByteLen);

    hash_dma_operate(msg, iterator, msgByteLen * 8, ctx->callback);

    return HASH_SUCCESS;
}


/* function: dma hash final(input the remainder message and get the digest)
 * parameters:
 *     ctx ------------------------ input, hash_dma_ctx_t context pointer
 *     remainder_msg -------------- input, remainder message
 *     remainder_bytes ------------ input, byte length of the remainder message, must be in [0, BLOCK_BYTE_LEN-1],
 *                                  here BLOCK_BYTE_LEN is block byte length of HASH
 *     digest --------------------- output, hash digest
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the four parameters are valid, and ctx is initialized
 */
uint32_t hash_dma_final(hash_dma_ctx_t *ctx, uint32_t *remainder_msg, uint32_t remainder_bytes, uint32_t *digest)
{
    if ((NULL == ctx) || (NULL == digest)) {
        return HASH_BUFFER_NULL;
    }
    else
    {;}

    if ((NULL == remainder_msg)) {
        remainder_bytes = 0;
    }
    else
    {;}

    if (remainder_bytes >= (ctx->block_word_len << 2)) {
        return HASH_INPUT_INVALID;
    }
    else
    {;}

    /*set the last block*/
    hash_set_last_block(1);

    if(hash_check_whether_sha3_alg(ctx->hash_alg)) {
        hash_set_msg_len(remainder_bytes);
    }
    else {
        /*update total byte length*/
        if (hash_total_len_add_uint32(ctx->total, ctx->block_word_len / 8, remainder_bytes)) {
            return HASH_LEN_OVERFLOW;
        }
        else
        {;}



        /*set total length of message*/
        hash_total_bytelen_2_bitlen(ctx->total, (ctx->block_word_len) / 8);

        hash_set_msg_total_bit_len(ctx->total, ctx->block_word_len * 4);
    }
    hash_dma_operate(remainder_msg, digest, remainder_bytes * 8, NULL);
    memset_(ctx, 0, sizeof(hash_dma_ctx_t));

    return HASH_SUCCESS;
}


/* function: dma hash digest calculate
 * parameters:
 *     hash_alg ------------------- input, specific hash algorithm
 *     msg ------------------------ input, message
 *     msg_bytes ------------------ input, byte length of the message, it could be 0
 *     digest --------------------- output, hash digest
 *     callback ------------------- callback function pointer
 * return: HASH_SUCCESS(success), other(error)
 * caution:
 *     1. please make sure the four parameters are valid
 */
uint32_t hash_dma(HASH_ALG hash_alg, uint32_t *msg, uint32_t msg_bytes, uint32_t *digest,
                  HASH_CALLBACK callback)
{
    uint32_t blocks_words, remainder_bytes;
    uint32_t ret;
    hash_dma_ctx_t ctx[1];

    if ((NULL == msg)) {
        msg_bytes = 0;
    }
    else
    {;}

    if (NULL == digest) {
        return HASH_BUFFER_NULL;
    }
    else
    {;}

    ret = hash_dma_init(ctx, hash_alg, callback);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else
    {;}

    remainder_bytes = msg_bytes % (ctx->block_word_len << 2);
    blocks_words = (msg_bytes - remainder_bytes) / 4;

    ret = hash_dma_update_blocks(ctx, (uint32_t *)msg, blocks_words, digest);

    if (HASH_SUCCESS != ret) {
        return ret;
    }
    else {
        return hash_dma_final(ctx, (uint32_t *)(msg + blocks_words), remainder_bytes, digest);
    }
}
#endif
