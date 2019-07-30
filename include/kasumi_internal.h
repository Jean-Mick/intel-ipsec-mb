/*******************************************************************************
  Copyright (c) 2009-2019, Intel Corporation

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
      * Neither the name of Intel Corporation nor the names of its contributors
        may be used to endorse or promote products derived from this software
        without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/


/*---------------------------------------------------------
* Kasumi_internal.h
*---------------------------------------------------------*/

#ifndef _KASUMI_INTERNAL_H_
#define _KASUMI_INTERNAL_H_

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "intel-ipsec-mb.h"
#include "wireless_common.h"
#include "kasumi_S97.h"

/* KASUMI cipher definitions */
#define NUM_KASUMI_ROUNDS           (8)     /* 8 rounds in the kasumi spec */
#define QWORDSIZEINBITS	            (64)
#define QWORDSIZEINBYTES            (8)
#define LAST_PADDING_BIT            (1)

#define BYTESIZE     (8)
#define BITSIZE(x)   ((int)(sizeof(x)*BYTESIZE))

/*--------- 16 bit rotate left ------------------------------------------*/
#define ROL16(a,b) (uint16_t)((a<<b)|(a>>(16-b)))

/*----- a 64-bit structure to help with kasumi endian issues -----*/
typedef union _ku64 {
	uint64_t b64[1];
	uint32_t b32[2];
	uint16_t b16[4];
	uint8_t b8[8];
} kasumi_union_t;

typedef union SafeBuffer {
        uint64_t b64;
        uint32_t b32[2];
        uint8_t b8[KASUMI_BLOCK_SIZE];
} SafeBuf;

/*---------------------------------------------------------------------
* Inline 16-bit left rotation
*---------------------------------------------------------------------*/

#define ROL16(a,b) (uint16_t)((a<<b)|(a>>(16-b)))

/* generic version based on a large table */
#define FIp1(data, key1, key2, key3)                                           \
        do {                                                                   \
                (data) ^= (key1);                                              \
                (data) = kasumi_S97[data];                                 \
                (data) ^= (key2);                                              \
                (data) = ROL16((data), 7);                                   \
                (data) = kasumi_S97[data];                                 \
                (data) ^= (key3);                                              \
        } while (0)

#define FIp2(data1, data2, key1, key2, key3, key4)                             \
        do {                                                                   \
                FIp1(data1, key1, key2, key3);                                 \
                FIp1(data2, key1, key2, key4);                                 \
        } while (0)

#define FLpi(key1, key2, res_h, res_l)                                         \
        do {                                                                   \
                uint16_t l, r;                                                 \
                r = (res_l) & (key1);                                          \
                r = (res_h) ^ ROL16(r, 1);                                   \
                l = r | (key2);                                                \
                (res_h) = (res_l) ^ ROL16(l, 1);                             \
                (res_l) = r;                                                   \
        } while (0)

#define FLp1(index, h, l)                                                      \
        do {                                                                   \
                uint16_t ka = *(index + 0);                                    \
                uint16_t kb = *(index + 1);                                    \
                FLpi(ka, kb, h, l);                                            \
        } while (0)

#define FLp2(index, h1, l1, h2, l2)                                            \
        do {                                                                   \
                uint16_t ka = *(index + 0);                                    \
                uint16_t kb = *(index + 1);                                    \
                FLpi(ka, kb, h1, l1);                                          \
                FLpi(ka, kb, h2, l2);                                          \
        } while (0)

#define FLp3(index, h1, l1, h2, l2, h3, l3)                                    \
        do {                                                                   \
                uint16_t ka = *(index + 0);                                    \
                uint16_t kb = *(index + 1);                                    \
                FLpi(ka, kb, h1, l1);                                          \
                FLpi(ka, kb, h2, l2);                                          \
                FLpi(ka, kb, h3, l3);                                          \
        } while (0)

#define FLp4(index, h1, l1, h2, l2, h3, l3, h4, l4)                            \
        do {                                                                   \
                FLp2(index, h1, l1, h2, l2);                                   \
                FLp2(index, h3, l3, h4, l4);                                   \
        } while (0)

#define FOp1(index, h, l)                                                      \
        do {                                                                   \
                FIp1(h, *(index + 2), *(index + 3), l);                        \
                FIp1(l, *(index + 4), *(index + 5), h);                        \
                FIp1(h, *(index + 6), *(index + 7), l);                        \
        } while (0)

#define FOp2(index, h1, l1, h2, l2)                                            \
        do {                                                                   \
                uint16_t ka = *(index + 2);                                    \
                uint16_t kb = *(index + 3);                                    \
                FIp2(h1, h2, ka, kb, l1, l2);                                  \
                ka = *(index + 4);                                             \
                kb = *(index + 5);                                             \
                FIp2(l1, l2, ka, kb, h1, h2);                                  \
                ka = *(index + 6);                                             \
                kb = *(index + 7);                                             \
                FIp2(h1, h2, ka, kb, l1, l2);                                  \
        } while (0)

#define FOp3(index, h1, l1, h2, l2, h3, l3)                                    \
        do {                                                                   \
                uint16_t ka = *(index + 2);                                    \
                uint16_t kb = *(index + 3);                                    \
                FIp2(h1, h2, ka, kb, l1, l2);                                  \
                FIp1(h3, ka, kb, l3);                                          \
                ka = *(index + 4);                                             \
                kb = *(index + 5);                                             \
                FIp2(l1, l2, ka, kb, h1, h2);                                  \
                FIp1(l3, ka, kb, h3);                                          \
                ka = *(index + 6);                                             \
                kb = *(index + 7);                                             \
                FIp2(h1, h2, ka, kb, l1, l2);                                  \
                FIp1(h3, ka, kb, l3);                                          \
        } while (0)

#define FOp4(index, h1, l1, h2, l2, h3, l3, h4, l4)                            \
        do {                                                                   \
                uint16_t ka = *(index + 2);                                    \
                uint16_t kb = *(index + 3);                                    \
                FIp2(h1, h2, ka, kb, l1, l2);                                  \
                FIp2(h3, h4, ka, kb, l3, l4);                                  \
                ka = *(index + 4);                                             \
                kb = *(index + 5);                                             \
                FIp2(l1, l2, ka, kb, h1, h2);                                  \
                FIp2(l3, l4, ka, kb, h3, h4);                                  \
                ka = *(index + 6);                                             \
                kb = *(index + 7);                                             \
                FIp2(h1, h2, ka, kb, l1, l2);                                  \
                FIp2(h3, h4, ka, kb, l3, l4);                                  \
        } while (0)

/**
 *******************************************************************************
 * @description
 * This function performs the Kasumi operation on the given block using the key
 * that is already scheduled in the context
 *
 * @param[in]       pContext     Context where the scheduled keys are stored
 * @param[in/out]   pData        Block to be enc/dec
 *
 ******************************************************************************/
static void kasumi_1_block(uint16_t *context, uint16_t *data)
{
    uint16_t *end = context + KASUMI_KEY_SCHEDULE_SIZE;
    uint16_t temp_l, temp_h;

    /* 4 iterations odd/even */
    do {
        temp_l = data[3];
        temp_h = data[2];
        FLp1(context, temp_h, temp_l);
        FOp1(context, temp_h, temp_l);
        context += 8;
        data[1] ^= temp_l;
        data[0] ^= temp_h;

        temp_h = data[1];
        temp_l = data[0];
        FOp1(context, temp_h, temp_l);
        FLp1(context, temp_h, temp_l);
        context += 8;
        data[3] ^= temp_h;
        data[2] ^= temp_l;
    } while (context < end);
}

/**
 ******************************************************************************
 * @description
 * This function performs the Kasumi operation on the given blocks using the key
 * that is already scheduled in the context
 *
 * @param[in]       pContext     Context where the scheduled keys are stored
 * @param[in/out]   pData1       First block to be enc/dec
 * @param[in/out]   pData2       Second block to be enc/dec
 *
 ******************************************************************************/
static void kasumi_2_blocks(uint16_t *context, uint16_t *data1, uint16_t *data2)
{
    uint16_t *end = context + KASUMI_KEY_SCHEDULE_SIZE;
    uint16_t temp1_l, temp1_h;
    uint16_t temp2_l, temp2_h;

    /* 4 iterations odd/even , with fine grain interleave */
    do {
        /* even */
        temp1_l = data1[3];
        temp1_h = data1[2];
        temp2_l = data2[3];
        temp2_h = data2[2];
        FLp2(context, temp1_h, temp1_l, temp2_h, temp2_l);
        FOp2(context, temp1_h, temp1_l, temp2_h, temp2_l);
        context += 8;
        data1[1] ^= temp1_l;
        data1[0] ^= temp1_h;
        data2[1] ^= temp2_l;
        data2[0] ^= temp2_h;

        /* odd */
        temp1_h = data1[1];
        temp1_l = data1[0];
        temp2_h = data2[1];
        temp2_l = data2[0];
        FOp2(context, temp1_h, temp1_l, temp2_h, temp2_l);
        FLp2(context, temp1_h, temp1_l, temp2_h, temp2_l);
        context += 8;
        data1[3] ^= temp1_h;
        data1[2] ^= temp1_l;
        data2[3] ^= temp2_h;
        data2[2] ^= temp2_l;
    } while (context < end);
}


/**
 *******************************************************************************
 * @description
 * This function performs the Kasumi operation on the given blocks using the key
 * that is already scheduled in the context
 *
 * @param[in]       pContext     Context where the scheduled keys are stored
 * @param[in/out]   pData1       First block to be enc/dec
 * @param[in/out]   pData2       Second block to be enc/dec
 * @param[in/out]   pData3       Third block to be enc/dec
 *
 ******************************************************************************/
static void kasumi_3_blocks(uint16_t *context, uint16_t *data1,
                            uint16_t *data2, uint16_t *data3)
{
        /* Case when the conmpiler is able to interleave efficiently */
        uint16_t *end = context + KASUMI_KEY_SCHEDULE_SIZE;
        uint16_t temp1_l, temp1_h;
        uint16_t temp2_l, temp2_h;
        uint16_t temp3_l, temp3_h;

        /* 4 iterations odd/even , with fine grain interleave */
        do {
                temp1_l = data1[3];
                temp1_h = data1[2];
                temp2_l = data2[3];
                temp2_h = data2[2];
                temp3_l = data3[3];
                temp3_h = data3[2];
                FLp3(context, temp1_h, temp1_l, temp2_h, temp2_l, temp3_h,
                     temp3_l);
                FOp3(context, temp1_h, temp1_l, temp2_h, temp2_l, temp3_h,
                     temp3_l);
                context += 8;
                data1[1] ^= temp1_l;
                data1[0] ^= temp1_h;
                data2[1] ^= temp2_l;
                data2[0] ^= temp2_h;
                data3[1] ^= temp3_l;
                data3[0] ^= temp3_h;

                temp1_h = data1[1];
                temp1_l = data1[0];
                temp2_h = data2[1];
                temp2_l = data2[0];
                temp3_h = data3[1];
                temp3_l = data3[0];
                FOp3(context, temp1_h, temp1_l, temp2_h, temp2_l, temp3_h,
                     temp3_l);
                FLp3(context, temp1_h, temp1_l, temp2_h, temp2_l, temp3_h,
                     temp3_l);
                context += 8;
                data1[3] ^= temp1_h;
                data1[2] ^= temp1_l;
                data2[3] ^= temp2_h;
                data2[2] ^= temp2_l;
                data3[3] ^= temp3_h;
                data3[2] ^= temp3_l;
        } while (context < end);
}

/**
 *******************************************************************************
 * @description
 * This function performs the Kasumi operation on the given blocks using the key
 * that is already scheduled in the context
 *
 * @param[in]       pContext    Context where the scheduled keys are stored
 * @param[in]       ppData      Pointer to an array of addresses of blocks
 *
 ******************************************************************************/
static void kasumi_4_blocks(uint16_t *context, uint16_t **ppData)
{
    /* Case when the conmpiler is unable to interleave efficiently */
    kasumi_2_blocks (context, ppData[0], ppData[1]);
    kasumi_2_blocks (context, ppData[2], ppData[3]);
}

/**
 ******************************************************************************
 * @description
 * This function performs the Kasumi operation on the given blocks using the key
 * that is already scheduled in the context
 *
 * @param[in]       pContext    Context where the scheduled keys are stored
 * @param[in]       ppData      Pointer to an array of addresses of blocks
 *
 ******************************************************************************/
static void kasumi_8_blocks(uint16_t *context, uint16_t **ppData)
{
    kasumi_4_blocks (context, &ppData[0]);
    kasumi_4_blocks (context, &ppData[4]);
}

/******************************************************************************
* @description
*   Multiple wrappers for the Kasumi rounds on up to 16 blocks of 64 bits at a
*time.
*
*   Depending on the variable packet lengths, different wrappers get called.
*   It has been measured that 1 packet is faster than 2, 2 packets is faster
*than 3
*   3 packets is faster than 4, and so on ...
*   It has also been measured that 6 = 4+2 packets is faster than 8
*   It has also been measured that 7 packets are processed faster as 8 packets,
*
*   If the assumptions are not verified, it is easy to implmement
*   the right function and reference it in wrapperArray.
*
*******************************************************************************/
static void kasumi_f8_1_buffer_wrapper(uint16_t *context, uint16_t **data)
{
        kasumi_1_block(context, data[0]);
}

static void kasumi_f8_2_buffer_wrapper(uint16_t *context, uint16_t **data)
{
        kasumi_2_blocks(context, data[0], data[1]);
}

static void kasumi_f8_3_buffer_wrapper(uint16_t *context, uint16_t **data)
{
        kasumi_3_blocks(context, data[0], data[1], data[2]);
}

static void kasumi_f8_5_buffer_wrapper(uint16_t *context, uint16_t **data)
{
        kasumi_4_blocks(context, &data[0]);
        kasumi_1_block(context, data[4]);
}

static void kasumi_f8_6_buffer_wrapper(uint16_t *context, uint16_t **data)
{
        /* It is also assumed 6 = 4+2 packets is faster than 8 */
        kasumi_4_blocks(context, &data[0]);
        kasumi_2_blocks(context, data[4], data[5]);
}

static void kasumi_f8_7_buffer_wrapper(uint16_t *context, uint16_t **data)
{
        kasumi_4_blocks(context, &data[0]);
        kasumi_3_blocks(context, data[4], data[5], data[6]);
}

static void kasumi_f8_9_buffer_wrapper(uint16_t *context, uint16_t **data)
{

        kasumi_8_blocks(context, &data[0]);
        kasumi_1_block(context, data[8]);
}

static void kasumi_f8_10_buffer_wrapper(uint16_t *context, uint16_t **data)
{
        kasumi_8_blocks(context, &data[0]);
        kasumi_2_blocks(context, data[8], data[9]);
}

static void kasumi_f8_11_buffer_wrapper(uint16_t *context, uint16_t **data)
{
        kasumi_8_blocks(context, &data[0]);
        kasumi_3_blocks(context, data[8], data[9], data[10]);
}

static void kasumi_f8_12_buffer_wrapper(uint16_t *context, uint16_t **data)
{
        kasumi_8_blocks(context, &data[0]);
        kasumi_4_blocks(context, &data[8]);
}

static void kasumi_f8_13_buffer_wrapper(uint16_t *context, uint16_t **data)
{

        kasumi_8_blocks(context, &data[0]);
        kasumi_4_blocks(context, &data[8]);
        kasumi_1_block(context, data[12]);
}

static void kasumi_f8_14_buffer_wrapper(uint16_t *context, uint16_t **data)
{
        kasumi_8_blocks(context, &data[0]);
        kasumi_4_blocks(context, &data[8]);
        kasumi_2_blocks(context, data[12], data[13]);
}

static void kasumi_f8_15_buffer_wrapper(uint16_t *context, uint16_t **data)
{
        kasumi_8_blocks(context, &data[0]);
        kasumi_4_blocks(context, &data[8]);
        kasumi_3_blocks(context, data[12], data[13], data[14]);
}

static void kasumi_f8_16_buffer_wrapper(uint16_t *context, uint16_t **data)
{
        kasumi_8_blocks(context, &data[0]);
        kasumi_8_blocks(context, &data[8]);
}

typedef void (*kasumi_wrapper_t)(uint16_t *, uint16_t **);

static kasumi_wrapper_t kasumiWrapperArray[] = {
        NULL,
        kasumi_f8_1_buffer_wrapper,
        kasumi_f8_2_buffer_wrapper,
        kasumi_f8_3_buffer_wrapper,
        kasumi_4_blocks,
        kasumi_f8_5_buffer_wrapper,
        kasumi_f8_6_buffer_wrapper,
        kasumi_f8_7_buffer_wrapper,
        kasumi_8_blocks,
        kasumi_f8_9_buffer_wrapper,
        kasumi_f8_10_buffer_wrapper,
        kasumi_f8_11_buffer_wrapper,
        kasumi_f8_12_buffer_wrapper,
        kasumi_f8_13_buffer_wrapper,
        kasumi_f8_14_buffer_wrapper,
        kasumi_f8_15_buffer_wrapper,
        kasumi_f8_16_buffer_wrapper};

static inline void
kasumi_f8_1_buffer(kasumi_key_sched_t *pCtx, uint64_t IV,
                   uint8_t *pBufferIn, uint8_t *pBufferOut,
                   uint32_t lengthInBytes)
{
        uint32_t blkcnt;
        kasumi_union_t a, b; /* the modifier */
        SafeBuf safeInBuf;

        /* IV Endianity  */
	a.b64[0] = BSWAP64(IV);

        /* First encryption to create modifier */
        kasumi_1_block(pCtx->msk16, a.b16 );

        /* Final initialisation steps */
        blkcnt = 0;
        b.b64[0] = a.b64[0];

        /* Now run the block cipher */
        while (lengthInBytes) {
                /* KASUMI it to produce the next block of keystream */
                kasumi_1_block(pCtx->sk16, b.b16 );

                if (lengthInBytes > KASUMI_BLOCK_SIZE) {
                        pBufferIn = xor_keystrm_rev(pBufferOut, pBufferIn,
                                                    b.b64[0]);
                        pBufferOut += KASUMI_BLOCK_SIZE;
                        /* loop variant */
                        /* done another 64 bits */
                        lengthInBytes -= KASUMI_BLOCK_SIZE;

                        /* apply the modifier and update the block count */
                        b.b64[0] ^= a.b64[0];
                        b.b16[0] ^= (uint16_t)++blkcnt;
                } else if (lengthInBytes < KASUMI_BLOCK_SIZE) {
                        /* end of the loop, handle the last bytes */
                        memcpy_keystrm(safeInBuf.b8, pBufferIn,
                                       lengthInBytes);
                        xor_keystrm_rev(b.b8, safeInBuf.b8, b.b64[0]);
                        memcpy_keystrm(pBufferOut, b.b8, lengthInBytes);
                        lengthInBytes = 0;
                /* lengthInBytes == KASUMI_BLOCK_SIZE */
                } else {
                        xor_keystrm_rev(pBufferOut, pBufferIn, b.b64[0]);
                        lengthInBytes = 0;
                }
        }
}

static inline void
kasumi_f8_1_buffer_bit(kasumi_key_sched_t *pCtx, uint64_t IV,
                       uint8_t *pBufferIn, uint8_t *pBufferOut,
                       uint32_t cipherLengthInBits,
                       uint32_t offsetInBits)
{
        uint32_t blkcnt;
        uint64_t shiftrem = 0;
        kasumi_union_t a, b, c; /* the modifier */
        uint8_t *pcBufferIn = pBufferIn + (offsetInBits / 8);
        uint8_t *pcBufferOut = pBufferOut + (offsetInBits / 8);
        /* Offset into the first byte (0 - 7 bits) */
        uint32_t remainOffset = offsetInBits % 8;
        uint32_t byteLength = (cipherLengthInBits + 7) / 8;
        SafeBuf safeOutBuf;
        SafeBuf safeInBuf;
        uint64_t mask;

        /* IV Endianity  */
        a.b64[0] = BSWAP64(IV);

        /* First encryption to create modifier */
        kasumi_1_block(pCtx->msk16, a.b16);

        /* Final initialisation steps */
        blkcnt = 0;
        b.b64[0] = a.b64[0];
        /* Now run the block cipher */

        /* Start with potential partial block (due to offset and length) */
        kasumi_1_block(pCtx->sk16, b.b16);
        c.b64[0] = b.b64[0] >> remainOffset;
        /* Only one block to encrypt */
        if (cipherLengthInBits < (64 - remainOffset)) {
                byteLength = (cipherLengthInBits + 7) / 8;
                memcpy_keystrm(safeInBuf.b8, pcBufferIn,
                                   byteLength);
                /* Last bits (not to cipher) need to be preserved from Input */
                mask = (uint64_t)-1 << (KASUMI_BLOCK_SIZE * 8 -
                                remainOffset - cipherLengthInBits);
                c.b64[0] &= mask;
                xor_keystrm_rev(safeOutBuf.b8, safeInBuf.b8, c.b64[0]);
                memcpy_keystrm(pcBufferOut, safeOutBuf.b8, byteLength);
                return;
        }
        /* At least 64 bits to produce (including offset) */
        pcBufferIn = xor_keystrm_rev(pcBufferOut, pcBufferIn, c.b64[0]);
        if (remainOffset != 0)
                shiftrem = b.b64[0] << (64 - remainOffset);
        cipherLengthInBits -= KASUMI_BLOCK_SIZE * 8 - remainOffset;
        pcBufferOut += KASUMI_BLOCK_SIZE;
        /* apply the modifier and update the block count */
        b.b64[0] ^= a.b64[0];
        b.b16[0] ^= (uint16_t)++blkcnt;

        while (cipherLengthInBits) {
                /* KASUMI it to produce the next block of keystream */
                kasumi_1_block(pCtx->sk16, b.b16);
                c.b64[0] = (b.b64[0] >> remainOffset) | shiftrem;
                if (remainOffset != 0)
                        shiftrem = b.b64[0] << (64 - remainOffset);
                if (cipherLengthInBits >= KASUMI_BLOCK_SIZE * 8) {
                        pcBufferIn = xor_keystrm_rev(pcBufferOut,
                                                     pcBufferIn, c.b64[0]);
                        cipherLengthInBits -= KASUMI_BLOCK_SIZE * 8;
                        pcBufferOut += KASUMI_BLOCK_SIZE;
                        /* loop variant */

                        /* apply the modifier and update the block count */
                        b.b64[0] ^= a.b64[0];
                        b.b16[0] ^= (uint16_t)++blkcnt;
                } else {
                        /* end of the loop, handle the last bytes */
                        byteLength = (cipherLengthInBits + 7) / 8;
                        memcpy_keystrm(safeInBuf.b8, pcBufferIn,
                                       byteLength);

                        /* Last bits need to be preserved from Input */
                        mask = ((uint64_t)-1) << (KASUMI_BLOCK_SIZE * 8 -
                                cipherLengthInBits);
                        c.b64[0] &= mask;
                        xor_keystrm_rev(safeOutBuf.b8, safeInBuf.b8, c.b64[0]);
                        memcpy_keystrm(pcBufferOut, safeOutBuf.b8, byteLength);
                        cipherLengthInBits = 0;
                }
        }
}

static inline void
kasumi_f8_2_buffer(kasumi_key_sched_t *pCtx, uint64_t IV1,
                   uint64_t IV2, uint8_t *pBufferIn1,
                   uint8_t *pBufferOut1, uint32_t lengthInBytes1,
                   uint8_t *pBufferIn2, uint8_t *pBufferOut2,
                   uint32_t lengthInBytes2)
{
        uint32_t blkcnt, length;
        kasumi_union_t a1, b1; /* the modifier */
        kasumi_union_t a2, b2; /* the modifier */
        SafeBuf safeInBuf;

        kasumi_union_t temp;

        /* IV Endianity  */
        a1.b64[0] = BSWAP64(IV1);
        a2.b64[0] = BSWAP64(IV2);

        kasumi_2_blocks(pCtx->msk16, a1.b16, a2.b16);

        /* Final initialisation steps */
        blkcnt = 0;
        b1.b64[0] = a1.b64[0];
        b2.b64[0] = a2.b64[0];

        /* check which packet is longer and save "common" shortest length */
        if (lengthInBytes1 > lengthInBytes2)
                length = lengthInBytes2;
        else
                length = lengthInBytes1;

        /* Round down to to a whole number of qwords. (QWORDLENGTHINBYTES-1 */
        length &= ~7;
        lengthInBytes1 -= length;
        lengthInBytes2 -= length;

        /* Now run the block cipher for common packet length, a whole number of
         * blocks */
        while (length) {
                /* KASUMI it to produce the next block of keystream for both
                 * packets */
                kasumi_2_blocks(pCtx->sk16, b1.b16, b2.b16);

                /* xor and write keystream */
                pBufferIn1 =
                    xor_keystrm_rev(pBufferOut1, pBufferIn1, b1.b64[0]);
                pBufferOut1 += KASUMI_BLOCK_SIZE;
                pBufferIn2 =
                    xor_keystrm_rev(pBufferOut2, pBufferIn2, b2.b64[0]);
                pBufferOut2 += KASUMI_BLOCK_SIZE;
                /* loop variant */
                length -= KASUMI_BLOCK_SIZE; /* done another 64 bits */

                /* apply the modifier and update the block count */
                b1.b64[0] ^= a1.b64[0];
                b1.b16[0] ^= (uint16_t)++blkcnt;
                b2.b64[0] ^= a2.b64[0];
                b2.b16[0] ^= (uint16_t)blkcnt;
        }

        /*
         * Process common part at end of first packet and second packet.
         * One of the packets has a length less than 8 bytes.
         */
        if (lengthInBytes1 > 0 && lengthInBytes2 > 0) {
                /* final round for 1 of the packets */
                kasumi_2_blocks(pCtx->sk16, b1.b16, b2.b16);
                if (lengthInBytes1 > KASUMI_BLOCK_SIZE) {
                        pBufferIn1 = xor_keystrm_rev(pBufferOut1,
                                                     pBufferIn1, b1.b64[0]);
                        pBufferOut1 += KASUMI_BLOCK_SIZE;
                        b1.b64[0] ^= a1.b64[0];
                        b1.b16[0] ^= (uint16_t)++blkcnt;
                        lengthInBytes1 -= KASUMI_BLOCK_SIZE;
                } else if (length < KASUMI_BLOCK_SIZE) {
                        memcpy_keystrm(safeInBuf.b8, pBufferIn1,
                                       lengthInBytes1);
                        xor_keystrm_rev(temp.b8, safeInBuf.b8, b1.b64[0]);
                        memcpy_keystrm(pBufferOut1, temp.b8,
                                       lengthInBytes1);
                        lengthInBytes1 = 0;
                /* length == KASUMI_BLOCK_SIZE */
                } else {
                        xor_keystrm_rev(pBufferOut1, pBufferIn1, b1.b64[0]);
                        lengthInBytes1 = 0;
                }
                if (lengthInBytes2 > KASUMI_BLOCK_SIZE) {
                        pBufferIn2 = xor_keystrm_rev(pBufferOut2,
                                                     pBufferIn2, b2.b64[0]);
                        pBufferOut2 += KASUMI_BLOCK_SIZE;
                        b2.b64[0] ^= a2.b64[0];
                        b2.b16[0] ^= (uint16_t)++blkcnt;
                        lengthInBytes2 -= KASUMI_BLOCK_SIZE;
                } else if (lengthInBytes2 < KASUMI_BLOCK_SIZE) {
                        memcpy_keystrm(safeInBuf.b8, pBufferIn2,
                                       lengthInBytes2);
                        xor_keystrm_rev(temp.b8, safeInBuf.b8, b2.b64[0]);
                        memcpy_keystrm(pBufferOut2, temp.b8,
                                       lengthInBytes2);
                        lengthInBytes2 = 0;
                } else {
                        xor_keystrm_rev(pBufferOut2, pBufferIn2, b2.b64[0]);
                        lengthInBytes2 = 0;
                }
        }

        if (lengthInBytes1 < lengthInBytes2) {
                /* packet 2 is not completed since lengthInBytes2 > 0
                *  packet 1 has less than 8 bytes.
                */
                if (lengthInBytes1) {
                        kasumi_1_block(pCtx->sk16, b1.b16);
                        xor_keystrm_rev(pBufferOut1, pBufferIn1, b1.b64[0]);
                }
                /* move pointers to right variables for packet 1 */
                lengthInBytes1 = lengthInBytes2;
                b1.b64[0] = b2.b64[0];
                a1.b64[0] = a2.b64[0];
                pBufferIn1 = pBufferIn2;
                pBufferOut1 = pBufferOut2;
        } else { /* lengthInBytes1 >= lengthInBytes2 */
                if (!lengthInBytes1)
                        /* both packets are completed */
                        return;
                /* process the remaining of packet 2 */
                if (lengthInBytes2) {
                        kasumi_1_block(pCtx->sk16, b2.b16);
                        xor_keystrm_rev(pBufferOut2, pBufferIn2, b2.b64[0]);
                }
                /* packet 1 is not completed */
        }

        /* process the length difference from ipkt1 and pkt2 */
        while (lengthInBytes1) {
                /* KASUMI it to produce the next block of keystream */
                kasumi_1_block(pCtx->sk16, b1.b16);

                if (lengthInBytes1 > KASUMI_BLOCK_SIZE) {
                        pBufferIn1 = xor_keystrm_rev(pBufferOut1,
                                                     pBufferIn1, b1.b64[0]);
                        pBufferOut1 += KASUMI_BLOCK_SIZE;
                        /* loop variant */
                        lengthInBytes1 -= KASUMI_BLOCK_SIZE;

                        /* apply the modifier and update the block count */
                        b1.b64[0] ^= a1.b64[0];
                        b1.b16[0] ^= (uint16_t)++blkcnt;
                } else if (lengthInBytes1 < KASUMI_BLOCK_SIZE) {
                        /* end of the loop, handle the last bytes */
                        memcpy_keystrm(safeInBuf.b8, pBufferIn1,
                                       lengthInBytes1);
                        xor_keystrm_rev(temp.b8, safeInBuf.b8, b1.b64[0]);
                        memcpy_keystrm(pBufferOut1, temp.b8,
                                       lengthInBytes1);
                        lengthInBytes1 = 0;
                /* lengthInBytes1 == KASUMI_BLOCK_SIZE */
                } else {
                        xor_keystrm_rev(pBufferOut1, pBufferIn1, b1.b64[0]);
                        lengthInBytes1 = 0;
                }
        }
}

static inline void
kasumi_f8_3_buffer(kasumi_key_sched_t *pCtx, uint64_t IV1,
                   uint64_t IV2, uint64_t IV3, uint8_t *pBufferIn1,
                   uint8_t *pBufferOut1, uint8_t *pBufferIn2,
                   uint8_t *pBufferOut2, uint8_t *pBufferIn3,
                   uint8_t *pBufferOut3, uint32_t lengthInBytes)
{
        uint32_t blkcnt;
        kasumi_union_t a1, b1; /* the modifier */
        kasumi_union_t a2, b2; /* the modifier */
        kasumi_union_t a3, b3; /* the modifier */
        SafeBuf safeInBuf1, safeInBuf2, safeInBuf3;

        /* IV Endianity  */
        a1.b64[0] = BSWAP64(IV1);
        a2.b64[0] = BSWAP64(IV2);
        a3.b64[0] = BSWAP64(IV3);

        kasumi_3_blocks(pCtx->msk16, a1.b16, a2.b16, a3.b16);

        /* Final initialisation steps */
        blkcnt = 0;
        b1.b64[0] = a1.b64[0];
        b2.b64[0] = a2.b64[0];
        b3.b64[0] = a3.b64[0];

        /* Now run the block cipher for common packet lengthInBytes, a whole
         * number of blocks */
        while (lengthInBytes) {
                /* KASUMI it to produce the next block of keystream for all the
                 * packets */
                kasumi_3_blocks(pCtx->sk16, b1.b16, b2.b16, b3.b16);

                if (lengthInBytes > KASUMI_BLOCK_SIZE) {
                        /* xor and write keystream */
                        pBufferIn1 = xor_keystrm_rev(pBufferOut1,
                                                     pBufferIn1, b1.b64[0]);
                        pBufferOut1 += KASUMI_BLOCK_SIZE;
                        pBufferIn2 = xor_keystrm_rev(pBufferOut2,
                                                     pBufferIn2, b2.b64[0]);
                        pBufferOut2 += KASUMI_BLOCK_SIZE;
                        pBufferIn3 = xor_keystrm_rev(pBufferOut3,
                                                     pBufferIn3, b3.b64[0]);
                        pBufferOut3 += KASUMI_BLOCK_SIZE;
                        /* loop variant */
                        lengthInBytes -= KASUMI_BLOCK_SIZE;

                        /* apply the modifier and update the block count */
                        b1.b64[0] ^= a1.b64[0];
                        b1.b16[0] ^= (uint16_t)++blkcnt;
                        b2.b64[0] ^= a2.b64[0];
                        b2.b16[0] ^= (uint16_t)blkcnt;
                        b3.b64[0] ^= a3.b64[0];
                        b3.b16[0] ^= (uint16_t)blkcnt;
                } else if (lengthInBytes < KASUMI_BLOCK_SIZE) {
                        /* end of the loop, handle the last bytes */
                        memcpy_keystrm(safeInBuf1.b8, pBufferIn1,
                                       lengthInBytes);
                        xor_keystrm_rev(b1.b8, safeInBuf1.b8, b1.b64[0]);
                        memcpy_keystrm(pBufferOut1, b1.b8, lengthInBytes);

                        memcpy_keystrm(safeInBuf2.b8, pBufferIn2,
                                       lengthInBytes);
                        xor_keystrm_rev(b2.b8, safeInBuf2.b8, b2.b64[0]);
                        memcpy_keystrm(pBufferOut2, b2.b8, lengthInBytes);

                        memcpy_keystrm(safeInBuf3.b8, pBufferIn3,
                                       lengthInBytes);
                        xor_keystrm_rev(b3.b8, safeInBuf3.b8, b3.b64[0]);
                        memcpy_keystrm(pBufferOut3, b3.b8, lengthInBytes);
                        lengthInBytes = 0;
                /* lengthInBytes == KASUMI_BLOCK_SIZE */
                } else {
                        xor_keystrm_rev(pBufferOut1, pBufferIn1, b1.b64[0]);
                        xor_keystrm_rev(pBufferOut2, pBufferIn2, b2.b64[0]);
                        xor_keystrm_rev(pBufferOut3, pBufferIn3, b3.b64[0]);
                        lengthInBytes = 0;
                }
        }
}

/*---------------------------------------------------------
* @description
*       Kasumi F8 4 packet:
*       Four packets enc/dec with the same key schedule.
*       The 4 Ivs are independent and are passed as an array of values
*       The packets are separate, the datalength is common
*---------------------------------------------------------*/

static inline void
kasumi_f8_4_buffer(kasumi_key_sched_t *pCtx, uint64_t IV1,
                   uint64_t IV2, uint64_t IV3, uint64_t IV4,
                   uint8_t *pBufferIn1, uint8_t *pBufferOut1,
                   uint8_t *pBufferIn2, uint8_t *pBufferOut2,
                   uint8_t *pBufferIn3, uint8_t *pBufferOut3,
                   uint8_t *pBufferIn4, uint8_t *pBufferOut4,
                   uint32_t lengthInBytes)
{
        uint32_t blkcnt;
        kasumi_union_t a1, b1; /* the modifier */
        kasumi_union_t a2, b2; /* the modifier */
        kasumi_union_t a3, b3; /* the modifier */
        kasumi_union_t a4, b4; /* the modifier */
        uint16_t *pTemp[4] = {b1.b16, b2.b16, b3.b16, b4.b16};
        SafeBuf safeInBuf1, safeInBuf2, safeInBuf3, safeInBuf4;

        /* IV Endianity  */
        b1.b64[0] = BSWAP64(IV1);
        b2.b64[0] = BSWAP64(IV2);
        b3.b64[0] = BSWAP64(IV3);
        b4.b64[0] = BSWAP64(IV4);

        kasumi_4_blocks(pCtx->msk16, pTemp);

        /* Final initialisation steps */
        blkcnt = 0;
        a1.b64[0] = b1.b64[0];
        a2.b64[0] = b2.b64[0];
        a3.b64[0] = b3.b64[0];
        a4.b64[0] = b4.b64[0];

        /* Now run the block cipher for common packet lengthInBytes, a whole
         * number of blocks */
        while (lengthInBytes) {
                /* KASUMI it to produce the next block of keystream for all the
                 * packets */
                kasumi_4_blocks(pCtx->sk16, pTemp);

                if (lengthInBytes > KASUMI_BLOCK_SIZE) {
                        /* xor and write keystream */
                        pBufferIn1 = xor_keystrm_rev(pBufferOut1,
                                                     pBufferIn1, b1.b64[0]);
                        pBufferOut1 += KASUMI_BLOCK_SIZE;
                        pBufferIn2 = xor_keystrm_rev(pBufferOut2,
                                                     pBufferIn2, b2.b64[0]);
                        pBufferOut2 += KASUMI_BLOCK_SIZE;
                        pBufferIn3 = xor_keystrm_rev(pBufferOut3,
                                                     pBufferIn3, b3.b64[0]);
                        pBufferOut3 += KASUMI_BLOCK_SIZE;
                        pBufferIn4 = xor_keystrm_rev(pBufferOut4,
                                                     pBufferIn4, b4.b64[0]);
                        pBufferOut4 += KASUMI_BLOCK_SIZE;
                        /* loop variant */
                        lengthInBytes -= KASUMI_BLOCK_SIZE;

                        /* apply the modifier and update the block count */
                        b1.b64[0] ^= a1.b64[0];
                        b1.b16[0] ^= (uint16_t)++blkcnt;
                        b2.b64[0] ^= a2.b64[0];
                        b2.b16[0] ^= (uint16_t)blkcnt;
                        b3.b64[0] ^= a3.b64[0];
                        b3.b16[0] ^= (uint16_t)blkcnt;
                        b4.b64[0] ^= a4.b64[0];
                        b4.b16[0] ^= (uint16_t)blkcnt;
                } else if (lengthInBytes < KASUMI_BLOCK_SIZE) {
                        /* end of the loop, handle the last bytes */
                        memcpy_keystrm(safeInBuf1.b8, pBufferIn1,
                                       lengthInBytes);
                        xor_keystrm_rev(b1.b8, safeInBuf1.b8, b1.b64[0]);
                        memcpy_keystrm(pBufferOut1, b1.b8, lengthInBytes);

                        memcpy_keystrm(safeInBuf2.b8, pBufferIn2,
                                       lengthInBytes);
                        xor_keystrm_rev(b2.b8, safeInBuf2.b8, b2.b64[0]);
                        memcpy_keystrm(pBufferOut2, b2.b8, lengthInBytes);

                        memcpy_keystrm(safeInBuf3.b8, pBufferIn3,
                                       lengthInBytes);
                        xor_keystrm_rev(b3.b8, safeInBuf3.b8, b3.b64[0]);
                        memcpy_keystrm(pBufferOut3, b3.b8, lengthInBytes);

                        memcpy_keystrm(safeInBuf4.b8, pBufferIn4,
                                       lengthInBytes);
                        xor_keystrm_rev(b4.b8, safeInBuf4.b8, b4.b64[0]);
                        memcpy_keystrm(pBufferOut4, b4.b8, lengthInBytes);
                        lengthInBytes = 0;
                /* lengthInBytes == KASUMI_BLOCK_SIZE */
                } else {
                        xor_keystrm_rev(pBufferOut1, pBufferIn1, b1.b64[0]);
                        xor_keystrm_rev(pBufferOut2, pBufferIn2, b2.b64[0]);
                        xor_keystrm_rev(pBufferOut3, pBufferIn3, b3.b64[0]);
                        xor_keystrm_rev(pBufferOut4, pBufferIn4, b4.b64[0]);
                        lengthInBytes = 0;
                }
        }
}

/*---------------------------------------------------------
* @description
*       Kasumi F8 2 packet:
*       Two packets enc/dec with the same key schedule.
*       The 2 Ivs are independent and are passed as an array of values.
*       The packets are separate, the datalength is common
*---------------------------------------------------------*/
/******************************************************************************
* @description
*       Kasumi F8 n packet:
*       Performs F8 enc/dec on [n] packets. The operation is performed in-place.
*       The input IV's are passed in Big Endian format.
*       The KeySchedule is in Little Endian format.
*******************************************************************************/

static inline void
kasumi_f8_n_buffer(kasumi_key_sched_t *pKeySchedule, uint64_t IV[],
                   uint8_t *pDataIn[], uint8_t *pDataOut[],
                   uint32_t dataLen[], uint32_t dataCount)
{
        if (dataCount > 16) {
                pDataOut[0] = NULL;
                printf("dataCount too high (%d)\n", dataCount);
                return;
        }

        kasumi_union_t A[NUM_PACKETS_16], temp[NUM_PACKETS_16], tempSort;
        uint16_t *data[NUM_PACKETS_16];
        uint8_t *srctempbuff;
        uint8_t *dsttempbuff;
        uint32_t blkcnt = 0;
        uint32_t len = 0;
        uint32_t packet_idx, inner_idx, same_size_blocks;
        int sortNeeded = 0, tempLen = 0;
        SafeBuf safeInBuf;

        /* save the IV to A for each packet */
        packet_idx = dataCount;
        while (packet_idx--) {
                /*copy IV in reverse endian order as input IV is BE */
                temp[packet_idx].b64[0] = BSWAP64(IV[packet_idx]);

                /* set LE IV pointers */
                data[packet_idx] = temp[packet_idx].b16;

                /* check if all packets are sorted by decreasing length */
                if (packet_idx > 0 &&
                    dataLen[packet_idx - 1] < dataLen[packet_idx])
                        /* this packet array is not correctly sorted  */
                        sortNeeded = 1;
        }

        /* do 1st kasumi block on A with modified key, this overwrites A */
        kasumiWrapperArray[dataCount](pKeySchedule->msk16, data);

        if (sortNeeded) {
                /* sort packets in decreasing buffer size from [0] to [n]th
                packet,
                        ** where buffer[0] will contain longest buffer and
                buffer[n] will
                contain the shortest buffer.
                4 arrays are swapped :
                - pointers to input buffers
                - pointers to output buffers
                - pointers to input IV's
                - input buffer lengths
                */
                packet_idx = dataCount;
                while (packet_idx--) {
                        inner_idx = packet_idx;
                        while (inner_idx--) {
                                if (dataLen[packet_idx] > dataLen[inner_idx]) {

                                        /* swap buffers to arrange in descending
                                         * order from [0]. */
                                        srctempbuff = pDataIn[packet_idx];
                                        dsttempbuff = pDataOut[packet_idx];
                                        tempSort = temp[packet_idx];
                                        tempLen = dataLen[packet_idx];

                                        pDataIn[packet_idx] =
                                            pDataIn[inner_idx];
                                        pDataOut[packet_idx] =
                                            pDataOut[inner_idx];
                                        temp[packet_idx] = temp[inner_idx];
                                        dataLen[packet_idx] =
                                            dataLen[inner_idx];

                                        pDataIn[inner_idx] = srctempbuff;
                                        pDataOut[inner_idx] = dsttempbuff;
                                        temp[inner_idx] = tempSort;
                                        dataLen[inner_idx] = tempLen;
                                }
                        } /* for inner packet idx (inner bubble-sort) */
                }         /* for outer packet idx (outer bubble-sort) */
        }                 /* if sortNeeded */

        packet_idx = dataCount;
        while (packet_idx--)
                /* copy the schedule */
                A[packet_idx].b64[0] = temp[packet_idx].b64[0];

        while (dataCount > 0) {
                /* max num of blocks left depends on roundUp(smallest packet),
                * The shortest stream to process is always stored at location
                * [dataCount - 1]
                */
                same_size_blocks =
                    ((dataLen[dataCount - 1] + KASUMI_BLOCK_SIZE - 1) /
                     KASUMI_BLOCK_SIZE) -
                    blkcnt;

                /* process streams of complete blocks */
                while (same_size_blocks-- > 1) {
                        /* do kasumi block encryption */
                        kasumiWrapperArray[dataCount](pKeySchedule->sk16,
                                                          data);

                        packet_idx = dataCount;
                        while (packet_idx--)
                                xor_keystrm_rev(pDataOut[packet_idx] + len,
                                                pDataIn[packet_idx] + len,
                                                temp[packet_idx].b64[0]);

                        /* length already done since the start of the packets */
                        len += KASUMI_BLOCK_SIZE;

                        /* block idx is incremented and rewritten in the
                         * keystream */
                        blkcnt += 1;
                        packet_idx = dataCount;
                        while (packet_idx--) {
                                temp[packet_idx].b64[0] ^= A[packet_idx].b64[0];
                                temp[packet_idx].b16[0] ^= (uint16_t)blkcnt;
                        } /* for packet_idx */

                } /* while same_size_blocks  (iteration on multiple blocks) */

                /* keystream for last block of all packets */
                kasumiWrapperArray[dataCount](pKeySchedule->sk16, data);

                /* process incomplete blocks without overwriting past the buffer
                 * end */
                while ((dataCount > 0) &&
                       (dataLen[dataCount - 1] < (len + KASUMI_BLOCK_SIZE))) {

                        dataCount--;
                        /* incomplete block is copied into a temp buffer */
                        memcpy_keystrm(safeInBuf.b8, pDataIn[dataCount] + len,
                                       dataLen[dataCount] - len);
                        xor_keystrm_rev(temp[dataCount].b8,
                                        safeInBuf.b8,
                                        temp[dataCount].b64[0]);

                        memcpy_keystrm(pDataOut[dataCount] + len,
                                       temp[dataCount].b8,
                                       dataLen[dataCount] - len);
                } /* while dataCount */

                /* process last blocks: it can be the last complete block of the
                packets or, if
                KASUMI_SAFE_BUFFER is defined, the last block (complete or not)
                of the packets*/
                while ((dataCount > 0) &&
                       (dataLen[dataCount - 1] <= (len + KASUMI_BLOCK_SIZE))) {

                        dataCount--;
                        xor_keystrm_rev(pDataOut[dataCount] + len,
                                        pDataIn[dataCount] + len,
                                        temp[dataCount].b64[0]);
                } /* while dataCount */
                /* block idx is incremented and rewritten in the keystream */
                blkcnt += 1;

                /* for the following packets, this block is not the last one:
                dataCount is not decremented */
                packet_idx = dataCount;
                while (packet_idx--) {

                        xor_keystrm_rev(pDataOut[packet_idx] + len,
                                        pDataIn[packet_idx] + len,
                                        temp[packet_idx].b64[0]);
                        temp[packet_idx].b64[0] ^= A[packet_idx].b64[0];
                        temp[packet_idx].b16[0] ^= (uint16_t)blkcnt;
                } /* while packet_idx */

                /* length already done since the start of the packets */
                len += KASUMI_BLOCK_SIZE;

                /* the remaining packets, if any, have now at least one valid
                block, which might be complete or not */

        } /* while (dataCount) */
}

static inline void
kasumi_f9_1_buffer(kasumi_key_sched_t *pCtx, uint8_t *dataIn,
                   uint32_t length, uint8_t *pDigest)
{
        kasumi_union_t a, b, mask;
        uint64_t *pIn = (uint64_t *)dataIn;
        SafeBuf safeBuf;

        /* Init */
        a.b64[0] = 0;
        b.b64[0] = 0;
        mask.b64[0] = -1;

        /* Now run kasumi for all 8 byte blocks */
        while (length >= 8) {

                a.b64[0] ^= BSWAP64(*(pIn++));

                /* KASUMI it */
                kasumi_1_block(pCtx->sk16, a.b16);

                /* loop variant */
                length -= 8; /* done another 64 bits */

                /* update */
                b.b64[0] ^= a.b64[0];
        }

        if (length) {
                /* Not a whole 8 byte block remaining */
                mask.b64[0] = ~(mask.b64[0] >> (BYTESIZE * length));
                memcpy(&safeBuf.b64, pIn, length);
                mask.b64[0] &= BSWAP64(safeBuf.b64);
                a.b64[0] ^= mask.b64[0];

                /* KASUMI it */
                kasumi_1_block(pCtx->sk16, a.b16);

                /* update */
                b.b64[0] ^= a.b64[0];
        }

        /* Kasumi b */
        kasumi_1_block(pCtx->msk16, b.b16);

        /* swap result */
        *(uint32_t *)pDigest = bswap4(b.b32[1]);
}

/*---------------------------------------------------------
* @description
*       Kasumi F9 1 packet with user config:
*       Single packet digest with user defined IV, and precomputed key schedule.
*
*       IV = swap32(count) << 32 | swap32(fresh)
*
*---------------------------------------------------------*/

static inline void
kasumi_f9_1_buffer_user(kasumi_key_sched_t *pCtx, uint64_t IV,
                        uint8_t *pDataIn, uint32_t lengthInBits,
                        uint8_t *pDigest, uint32_t direction)
{
        kasumi_union_t a, b, mask, message, temp;
        uint64_t *pIn = (uint64_t *)pDataIn;
        kasumi_union_t safebuff;

        a.b64[0] = 0;
        b.b64[0] = 0;

        /* Use the count and fresh for first round */
        a.b64[0] = BSWAP64(IV);
        /* KASUMI it */
        kasumi_1_block(pCtx->sk16, a.b16);
        /* update */
        b.b64[0] = a.b64[0];

        /* Now run kasumi for all 8 byte blocks */
        while (lengthInBits >= QWORDSIZEINBITS) {
                a.b64[0] ^= BSWAP64(*(pIn++));
                /* KASUMI it */
                kasumi_1_block(pCtx->sk16, a.b16);
                /* loop variant */
                lengthInBits -= 64; /* done another 64 bits */
                /* update */
                b.b64[0] ^= a.b64[0];
        }

        /* Is there any non 8 byte blocks remaining ? */
        if (lengthInBits == 0) {
                /* last block is : direct + 1 + 62 0's */
                a.b64[0] ^= ((uint64_t)direction + direction + LAST_PADDING_BIT)
                            << (QWORDSIZEINBITS - 2);
                kasumi_1_block(pCtx->sk16, a.b16);
                /* update */
                b.b64[0] ^= a.b64[0];
        } else if (lengthInBits <= (QWORDSIZEINBITS - 2)) {
                /* last block is : message + direction + LAST_PADDING_BITS(1) +
                 * less than 62 0's */
                mask.b64[0] = -1;
                temp.b64[0] = 0;
                message.b64[0] = 0;
                mask.b64[0] = ~(mask.b64[0] >> lengthInBits);
                /*round up and copy last lengthInBits */
                memcpy(&safebuff.b64[0], pIn, (lengthInBits + 7) / 8);
                message.b64[0] = BSWAP64(safebuff.b64[0]);
                temp.b64[0] = mask.b64[0] & message.b64[0];
                temp.b64[0] |=
                    ((uint64_t)direction + direction + LAST_PADDING_BIT)
                    << ((QWORDSIZEINBITS - 2) - lengthInBits);
                a.b64[0] ^= temp.b64[0];
                /* KASUMI it */
                kasumi_1_block(pCtx->sk16, a.b16);

                /* update */
                b.b64[0] ^= a.b64[0];
        } else if (lengthInBits == (QWORDSIZEINBITS - 1)) {
                /* next block is : message + direct  */
                /* last block is : 1 + 63 0's */
                a.b64[0] ^= direction | (~1 & BSWAP64(*(pIn++)));
                /* KASUMI it */
                kasumi_1_block(pCtx->sk16, a.b16);
                /* update */
                b.b64[0] ^= a.b64[0];
                a.b8[QWORDSIZEINBYTES - 1] ^= (LAST_PADDING_BIT)
                                              << (QWORDSIZEINBYTES - 1);
                /* KASUMI it */
                kasumi_1_block(pCtx->sk16, a.b16);
                /* update */
                b.b64[0] ^= a.b64[0];
        }
        /* Kasumi b */
        kasumi_1_block(pCtx->msk16, b.b16);

        /* swap result */
        *(uint32_t *)pDigest = bswap4(b.b32[1]);
}

void kasumi_f8_1_buffer_sse(kasumi_key_sched_t *pCtx, uint64_t IV,
                            uint8_t *pBufferIn, uint8_t *pBufferOut,
                            uint32_t cipherLengthInBytes);
void kasumi_f8_1_buffer_bit_sse(kasumi_key_sched_t *pCtx, uint64_t IV,
                                uint8_t *pBufferIn, uint8_t *pBufferOut,
                                uint32_t cipherLengthInBits,
                                uint32_t offsetInBits);
void kasumi_f8_2_buffer_sse(kasumi_key_sched_t *pCtx, uint64_t IV1,
                            uint64_t IV2, uint8_t *pBufferIn1,
                            uint8_t *pBufferOut1, uint32_t lengthInBytes1,
                            uint8_t *pBufferIn2, uint8_t *pBufferOut2,
                            uint32_t lengthInBytes2);
void kasumi_f8_3_buffer_sse(kasumi_key_sched_t *pCtx, uint64_t IV1,
                            uint64_t IV2, uint64_t IV3, uint8_t *pBufferIn1,
                            uint8_t *pBufferOut1, uint8_t *pBufferIn2,
                            uint8_t *pBufferOut2, uint8_t *pBufferIn3,
                            uint8_t *pBufferOut3, uint32_t lengthInBytes);
void kasumi_f8_4_buffer_sse(kasumi_key_sched_t *pCtx, uint64_t IV1,
                            uint64_t IV2, uint64_t IV3, uint64_t IV4,
                            uint8_t *pBufferIn1, uint8_t *pBufferOut1,
                            uint8_t *pBufferIn2, uint8_t *pBufferOut2,
                            uint8_t *pBufferIn3, uint8_t *pBufferOut3,
                            uint8_t *pBufferIn4, uint8_t *pBufferOut4,
                            uint32_t lengthInBytes);
void kasumi_f8_n_buffer_sse(kasumi_key_sched_t *pKeySchedule, uint64_t IV[],
                            uint8_t *pDataIn[], uint8_t *pDataOut[],
                            uint32_t dataLen[], uint32_t dataCount);



void kasumi_f9_1_buffer_sse(kasumi_key_sched_t *pCtx, uint8_t *pBufferIn,
                            uint32_t lengthInBytes, uint8_t *pDigest);

void kasumi_f9_1_buffer_user_sse(kasumi_key_sched_t *pCtx, uint64_t IV,
                                 uint8_t *pBufferIn, uint32_t lengthInBits,
                                 uint8_t *pDigest, uint32_t direction);

void kasumi_f8_1_buffer_avx(kasumi_key_sched_t *pCtx, uint64_t IV,
                            uint8_t *pBufferIn, uint8_t *pBufferOut,
                            uint32_t cipherLengthInBytes);
void kasumi_f8_1_buffer_bit_avx(kasumi_key_sched_t *pCtx, uint64_t IV,
                                uint8_t *pBufferIn, uint8_t *pBufferOut,
                                uint32_t cipherLengthInBits,
                                uint32_t offsetInBits);
void kasumi_f8_2_buffer_avx(kasumi_key_sched_t *pCtx, uint64_t IV1,
                            uint64_t IV2, uint8_t *pBufferIn1,
                            uint8_t *pBufferOut1, uint32_t lengthInBytes1,
                            uint8_t *pBufferIn2, uint8_t *pBufferOut2,
                            uint32_t lengthInBytes2);
void kasumi_f8_3_buffer_avx(kasumi_key_sched_t *pCtx, uint64_t IV1,
                            uint64_t IV2, uint64_t IV3, uint8_t *pBufferIn1,
                            uint8_t *pBufferOut1, uint8_t *pBufferIn2,
                            uint8_t *pBufferOut2, uint8_t *pBufferIn3,
                            uint8_t *pBufferOut3, uint32_t lengthInBytes);
void kasumi_f8_4_buffer_avx(kasumi_key_sched_t *pCtx, uint64_t IV1,
                            uint64_t IV2, uint64_t IV3, uint64_t IV4,
                            uint8_t *pBufferIn1, uint8_t *pBufferOut1,
                            uint8_t *pBufferIn2, uint8_t *pBufferOut2,
                            uint8_t *pBufferIn3, uint8_t *pBufferOut3,
                            uint8_t *pBufferIn4, uint8_t *pBufferOut4,
                            uint32_t lengthInBytes);
void kasumi_f8_n_buffer_avx(kasumi_key_sched_t *pKeySchedule, uint64_t IV[],
                            uint8_t *pDataIn[], uint8_t *pDataOut[],
                            uint32_t dataLen[], uint32_t dataCount);



void kasumi_f9_1_buffer_avx(kasumi_key_sched_t *pCtx, uint8_t *pBufferIn,
                            uint32_t lengthInBytes, uint8_t *pDigest);

void kasumi_f9_1_buffer_user_avx(kasumi_key_sched_t *pCtx, uint64_t IV,
                                 uint8_t *pBufferIn, uint32_t lengthInBits,
                                 uint8_t *pDigest, uint32_t direction);
#endif /*_KASUMI_INTERNAL_H_*/
