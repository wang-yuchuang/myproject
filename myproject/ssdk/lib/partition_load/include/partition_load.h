 /*
 * SEMIDRIVE Copyright Statement
 * Copyright (c) SEMIDRIVE. All rights reserved
 *
 * This software and all rights therein are owned by SEMIDRIVE, and are
 * protected by copyright law and other relevant laws, regulations and
 * protection. Without SEMIDRIVE's prior written consent and/or related rights,
 * please do not use this software or any potion thereof in any form or by any
 * means. You may not reproduce, modify or distribute this software except in
 * compliance with the License. Unless required by applicable law or agreed to
 * in writing, software distributed under the License is distributed on
 * an AS IS basis, WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * You should have received a copy of the License along with this program.
 * If not, see <http://www.semidrive.com/licenses/>.
 */
#ifndef _PARTITION_LOAD_H_
#define _PARTITION_LOAD_H_

#include "partition_parser.h"

/* Load the image in partition.
 * If the image has avbfooter, it will load the actual size data of the image.
 * Otherwise, it loads all data of the image.
 * ptdev: partition device
 * part_name: the name of partition
 * buf: the buffer that will hold the image
 * buf_size: size of the buffer
 * loaded_size_p: a pointer to uint64_t to hold the actual data size, it could be NUL if doesn't care the size
 * return code: 0,successful;other value, fail
 * */
int partition_load(partition_device_t *ptdev, const char *part_name,
                          void *buf, uint64_t buf_size, uint64_t *loaded_size_p);

/* Load vbmeta image to buffer.
 * ptdev: partition device
 * load_addr_in: the buffer which hold the vbemeta image.
 *               If NULL, a buffer is allocated, caller should free this buffer.
 * load_buf_sz_in: the size of the buffer load_addr_in point to. If load_addr_in is NULL, load_buf_sz_in should be 0.
 *                 If load_buf_sz_in is 0, a buffer is allocated, caller should free this buffer.
 * loaded_sz_out: the actual size of vbmeta image which has been loaded.If NULL, ignore it.
 * return: If NULL, fail to load vbmeta image, otherwise the address of the buffer hold the vbemte image.
 * */
uint8_t * load_vbmeta_partition(partition_device_t *ptdev,
                           uint8_t *load_addr_in, size_t load_buf_sz_in,
                           size_t * load_sz_out);
#endif

