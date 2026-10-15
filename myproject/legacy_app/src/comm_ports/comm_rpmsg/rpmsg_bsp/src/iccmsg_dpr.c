#include "iccmsg_common.h"
#include "iccmsg_endian.h"
#include "iccmsg_dpr.h"



int iccmsg_frm_cfg(frame_conf_t* frm_cfg, 
                    uint8_t *head, 
                    uint8_t head_len, 
                    uint8_t *tail, 
                    uint8_t tail_len, 
                    checksum_t chksum){
    frame_conf_t *p_opr = frm_cfg;
    // 指针有效性检查
    if (!p_opr || !head || !tail ){
        LOG_PRINT(LOG_ERR,"%s: null pointer!\n");
        return -1;
    }
    // 参数有效性检查
    if(head_len > ICCMSG_FRAME_HEAD_MAX_LEN ||
        tail_len > ICCMSG_FRAME_TAIL_MAX_LEN ||
        chksum > CRC32){
        LOG_PRINT(LOG_ERR,"%s: unavailable param!\n");
        return -1;
    }
    // 传参复制
    memcpy(p_opr->frame_head,head,head_len);
    p_opr->head_len = head_len;
    memcpy(p_opr->frame_tail,tail,tail_len);
    p_opr->tail_len = tail_len;
    p_opr->checksum_type = chksum;

    return 0;
}

int get_frame_data_len(const uint8_t *buf, const frame_conf_t *fc)
{
    uint8_t * p_opr = NULL;
    uint16_t data_len;
    if(NULL == buf)
        return -1;
    p_opr = &buf[fc->head_len];
    // 获取数据长度
    if(is_little_endian())
        data_len = LOAD_LE16(p_opr);
    else
        data_len = LOAD_BE16(p_opr);
    return (int)data_len;
}

int iccmsg_verify(const uint8_t *buf, uint32_t len, const frame_conf_t *fc)
{
    uint32_t i,opr_pos,data_len,rcv_checksum,calc_checksum;
    const uint8_t *payload = NULL;

    if (!buf || !fc ) return false;

    // 校验帧头
    for (i = 0; i < fc->head_len; i++) {
        if (buf[i] != fc->frame_head[i]) return false;
    }

    // 校验帧尾
    opr_pos = len - fc->tail_len;
    for (int i = 0; i < fc->tail_len; i++) {
        if (buf[opr_pos + i] != fc->frame_tail[i]) return false;
    }

    // 获取数据长度
    data_len = get_frame_data_len(buf,fc);

    if (data_len + fc->head_len + 2 + 4 + fc->tail_len > len)
            return false;

    /* 获取frame_checksum */
    opr_pos = len - fc->tail_len - 4;
    if(is_little_endian())
        rcv_checksum = LOAD_LE32(&buf[opr_pos]);
    else
        rcv_checksum = LOAD_BE32(&buf[opr_pos]);

    calc_checksum = 0;
    payload = &buf[opr_pos-data_len];
    if (fc->checksum_type == CRC32) {
        calc_checksum = sd_crc32(0, payload, data_len);
    } else if (fc->checksum_type == SUM) {
        for (uint32_t i = 0; i < data_len; i++) {
            calc_checksum += payload[i];
        }
    }
    
    if(rcv_checksum != calc_checksum){ 
        /* 完整性校验失败，丢弃 */
        LOG_PRINT(LOG_CRIT, "iccmsg_virtio_cb frame checksum error.\n");
        return false;
    }

    return true;
}

int iccmsg_pack(uint8_t *dest, 
                const uint8_t *orig_data, 
                uint32_t orig_len, 
                const frame_conf_t *fc,
                iccmsg_data_type_t data_type)
{
    uint32_t idx = 0;
    uint32_t calc_checksum = 0;
    uint16_t data_len = 0;

    if(!dest || !fc) return -1;
    if(data_type.type != DATA_TYPE_ACK){
        if(!orig_data || orig_len == 0) return -1;
        data_len = orig_len + sizeof(iccmsg_data_type_t);
    }

    //处理帧头
    memcpy(dest, fc->frame_head, fc->head_len);
    idx += fc->head_len;

    // 处理data_len
    if(is_little_endian()){
        STORE_LE16(&dest[idx], data_len);
    }
    else{
        STORE_BE16(&dest[idx], data_len);
    }
    idx += 2;

    // 处理data_type
    memcpy(&dest[idx], (uint8_t *)&data_type, sizeof(iccmsg_data_type_t));
    idx +=sizeof(iccmsg_data_type_t);

    // 除了ACK，处理原始数据
    if(data_type.type != DATA_TYPE_ACK){
        
        memcpy(&dest[idx], orig_data, orig_len);
        idx += orig_len;
    }
    // crc32校验
    if (fc->checksum_type == CRC32) {
        calc_checksum = sd_crc32(0, &dest[fc->head_len+2], data_len);
    } else if (fc->checksum_type == SUM) {
        for (uint32_t i = 0; i < (data_len); i++) {
            calc_checksum += dest[fc->head_len+2+i];
        }
    }
    LOG_PRINT(LOG_DEBUG,"iccmsg_pack: calc_checksum = 0x%x, data_len = %d, calc_pos = %d\n", calc_checksum, data_len, fc->head_len+2);

    if(is_little_endian()){
        STORE_LE32(&dest[idx], calc_checksum);
    }
    else{
        STORE_BE32(&dest[idx], calc_checksum);
    }
    idx += 4;

    //处理帧尾
    memcpy(&dest[idx], fc->frame_tail, fc->tail_len);
    idx += fc->tail_len;

    return idx; //返回帧长度
}

int get_frame_data_max_len(const frame_conf_t *fc){
    if(NULL == fc )
        return -1;
    return (ICCMSG_FRAME_MAX_LEN - fc->head_len - fc->tail_len - ICCMSG_FRAME_DATA_LEN_LEN - ICCMSG_FRAME_CHECKSUM_LEN - sizeof(iccmsg_data_type_t));
}

int get_frm_data_pos(const frame_conf_t *fc){
    if(NULL == fc)
        return -1;
    return (fc->head_len + ICCMSG_FRAME_DATA_LEN_LEN);
}

int get_origin_data_pos(const frame_conf_t *fc){
    if(NULL == fc)
        return -1;
    return (fc->head_len + ICCMSG_FRAME_DATA_LEN_LEN + sizeof(iccmsg_data_type_t));
}






