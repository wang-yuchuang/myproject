/* ******************* 
// can uds的网络层接口
 **********************/
#include "can_uds_network_layer.h"
#include <string.h>


enum{MF_STA_EMP = 0,MF_STA_REVING,MF_STA_FINISH};                           //多帧收发的状态枚举





struct UDS_DEV_CONFIG g_uds_config;
unsigned char uds_send_flg = TRUE;



//
/*
 * send_uds_SF - 发送ISO-TP单帧(SF)
 * 说明：单帧首字节承载"帧类型+长度"信息。CAN-FD有效载荷上限62字节、普通CAN为
 *       7字节(第0字节低4位放SF_DL)；数据按SF_DL截断后填充到报文并发出。
 */
unsigned char send_uds_SF(struct SF_PACK *sf_pack)
{
    int ret = 0;
    unsigned char SF_DL = 0,i = 0;
    unsigned char databuf[64] = {0};
    unsigned char byte = 0,byte1 = 0;
    unsigned char sdl_max = 0;

    if(uds_is_canfd()){
        sdl_max = 62;
        memset(databuf,0x55,64);
    }else{
        sdl_max = 7;
        memset(databuf,0x55,8);
    }
        
    if(sf_pack->data_len > sdl_max)
        SF_DL = sdl_max;
    else
        SF_DL = sf_pack->data_len;

    for(i = 0; i < SF_DL;i++)
    {
        databuf[i] = sf_pack->data_buf[i];
    }
    
    uds_send_flg = TRUE;
    if(uds_is_canfd()){
        byte = 0x00;// CAN-FD单帧:byte0高4位=0(帧类型SF)，长度单独放在byte1
        byte1 = SF_DL;
        ret = uds_send_data64(g_uds_config.tx_id,g_uds_config.tx_id_type,
                              byte,byte1,databuf[0],databuf[1],
                              databuf[2],databuf[3],databuf[4],databuf[5],
                              &databuf[6]);
    }else{
        byte = byte | (SF_DL&0x0f);// 普通CAN单帧:byte0高4位=0(帧类型SF)，低4位=SF_DL
        ret = uds_send_data8(g_uds_config.tx_id,g_uds_config.tx_id_type,
                            byte,databuf[0],databuf[1],databuf[2],
                            databuf[3],databuf[4],databuf[5],databuf[6]);
    }
    
    return ((ret>0)?1:0);
}

/*
 * send_uds_FF - 发送ISO-TP首帧(FF)，携带多帧总长度FF_DL
 * 说明：首帧前两字节(或CAN-FD的前多字节)承载类型+12位总长度，随后紧跟首段数据；
 *       CAN-FD首段数据58字节、普通CAN为6字节。发送前将cur_sn复位为1，使后续
 *       第一帧CF的SN从1开始递增。
 */
unsigned char send_uds_FF(struct MF_PACK *mf_pack)//FF has total len
{
    int ret = 0;
    unsigned short FF_DL = 0, i = 0;
    unsigned char databuf[64] = {0};
    unsigned char byte0 = 0,byte1 = 0, byte2 = 0, byte3 = 0, byte4 = 0, byte5 = 0;
    unsigned char ff_left_bytes = 0;

    if(uds_is_canfd()){
        ff_left_bytes = 58;
    }else{
        ff_left_bytes = 6;
    }

    if(mf_pack->data_len > MAX_MF_PACK_LEN)
        return -1;
    else
        FF_DL = mf_pack->data_len;

    for(i = 0;i < ff_left_bytes;i++){// FF only have 6/59 bytes
        databuf[i] = mf_pack->data_buf[i];
    }
    
    mf_pack->cur_sn = 1;    //FF首帧，将SN状态清除，置位下一次CF为1
    uds_send_flg = TRUE;
    if(uds_is_canfd()){
        byte0 = 0x10 | (0x00);// CAN-FD首帧:byte0高4位=1(帧类型FF)，总长度放在byte4/byte5
        byte4 = ((FF_DL & 0x0f00)>>8);
        byte5 = (FF_DL & 0xff);
        ret = uds_send_data64(g_uds_config.tx_id,g_uds_config.tx_id_type,
                              byte0,byte1,byte2,byte3,
                              byte4,byte5,databuf[0],databuf[1],
                              &databuf[2]);
    }else{
        byte0 = 0x10 | ((FF_DL & 0x0f00)>>8);// 普通CAN首帧:byte0高4位=1(帧类型FF)+长度高4位，byte1=长度低8位
        byte1 = (FF_DL & 0xff);
        ret = uds_send_data8(g_uds_config.tx_id,g_uds_config.tx_id_type,
                         byte0,byte1,databuf[0],databuf[1],
                         databuf[2],databuf[3],databuf[4],databuf[5]);
    }
    
    return ((ret>0)?1:0);
}

/*
 * send_uds_CF - 发送ISO-TP连续帧(CF)
 * 说明：CF不携带长度，仅首字节高4位=2(帧类型CF)、低4位=当前SN。从cur_recv_idx
 *       起取剩余数据，每帧CAN-FD最多63字节、普通CAN最多7字节；SN按调用方维护。
 */
unsigned char send_uds_CF(struct MF_PACK *mf_pack)//CF do not have len each frame
{//对于是否CANFD，连续帧格式都是不变的，只是数据域扩展不同
    int ret = 0;
    unsigned i = 0;
    int tmp_len = 0;
    unsigned char databuf[8] = {0},byte = 0;

    if(uds_is_canfd()){
        memset(databuf,0x55,64);
    }else{
        memset(databuf,0x55,8);
    }

    if(mf_pack->cur_recv_idx < mf_pack->data_len)
        tmp_len = mf_pack->data_len - mf_pack->cur_recv_idx;// 剩余待发字节数
    if(uds_is_canfd()){
        if(tmp_len > 63)
            tmp_len = 63;// CAN-FD连续帧单帧最多63字节数据
    }else{
        if(tmp_len > 7)
            tmp_len = 7;// 普通CAN连续帧单帧最多7字节数据
    }
    

    for(i = 0; i < tmp_len;i++)
    {
        databuf[i] = mf_pack->data_buf[i+mf_pack->cur_recv_idx];// 从当前索引处取连续帧数据
    }
    byte = 0x20 | ((mf_pack->cur_sn)&0x0f);// 高4位=2(帧类型CF)，低4位=SN
    uds_send_flg = TRUE;
    if(uds_is_canfd()){
        ret = uds_send_data64(g_uds_config.tx_id,g_uds_config.tx_id_type,
                              byte,databuf[0],databuf[1],databuf[2],
                              databuf[3],databuf[4],databuf[5],databuf[6],
                              &databuf[7]);
    }else{
        ret = uds_send_data8(g_uds_config.tx_id,g_uds_config.tx_id_type,
                            byte,databuf[0],databuf[1],databuf[2],
                            databuf[3],databuf[4],databuf[5],databuf[6]);
    }
    

    return ((ret>0)?1:0);
}

/*
 * send_uds_FC - 发送ISO-TP流控帧(FC)
 * 说明：首字节高4位=3(帧类型FC)、低4位=FS(CTS=0/WT=1/OVFLW=2)，随后依次为
 *       块大小BS和最小间隔STmin；FC用于对端收到FF后告知后续CF发送节奏。
 */
unsigned char send_uds_FC(unsigned char fs,unsigned char bs,unsigned char stmin)
{//对于是否CANFD，流控帧格式都是不变的，只是数据域扩展不同
    int ret = 0;
    unsigned char databuf[64] = {0};

    uds_send_flg = TRUE;
    if(uds_is_canfd()){
        ret = uds_send_data64(g_uds_config.tx_id,g_uds_config.tx_id_type,
                                0x30 | (fs&0x0f),bs,stmin,0,// byte0=0x3x(帧类型FC+FS)，byte1=BS，byte2=STmin
                                0,0,0,0,
                                &databuf[0]);
    }else{
        ret = uds_send_data8(g_uds_config.tx_id,g_uds_config.tx_id_type,
                                0x30 | (fs&0x0f),bs,stmin,0,// 同上，FC格式与是否CAN-FD无关
                                0,0,0,0);
    }

    return ((ret>0)?1:0);
}

/*
 * check_uds_type - 判断CAN报文的UDS帧类型
 * 说明：先校验ID是否匹配配置的物理/功能接收ID，再取首字节高4位区分帧类型：
 *       0=SF、1=FF、2=CF、3=FC；ID不匹配或类型非法返回-1。
 */
unsigned char check_uds_type(struct CAN_DATA *p_can_data)// return uds type
{
    if((p_can_data->idt.id == g_uds_config.rx_phy_id) ||
        (p_can_data->idt.id == g_uds_config.rx_fuc_id))
    {// if 
        switch(((p_can_data->data[0]&0xf0)>>4))
        {
        case 0x00:// SF
            return SF_UDS_TYPE;
        case 0x01:// FF
            return FF_UDS_TYPE;
        case 0x02:// CF
            return CF_UDS_TYPE;
        case 0x03:// FC
            return FC_UDS_TYPE;
        default:
            return -1;
        }
    }
    else
        return -1;
}

/*
 * analyze_uds_SF - 解析单帧，提取有效数据到sf_pack，返回SF_DL
 * 说明：CAN-FD单帧长度位于data[1]、数据从data[2]起；普通CAN长度在data[0]低4位、
 *       数据从data[1]起。长度超过上限或为0视为非法，返回0xff。
 */
unsigned char analyze_uds_SF(struct CAN_DATA *p_can_data,struct SF_PACK *sf_pack)//return  SF_DL len
{
    unsigned char SF_DL = 0, i = 0;
    if(uds_is_canfd()){//canfd单帧的SDL位于BYTE1
        SF_DL = p_can_data->data[1];
        if(SF_DL > 62)
            return 0xff;//len error
        if(SF_DL == 0)
            return 0xff;//len is not equal 0
    }else{
        SF_DL = p_can_data->data[0] & 0x0f;
        if(SF_DL > 7)
            return 0xff;//len error
        if(SF_DL == 0)
            return 0xff;//len is not equal 0
    }
    if(uds_is_canfd())
    {
        for(i = 0;i < SF_DL; i++)
        {
            sf_pack->data_buf[i] = p_can_data->data[i+2];
        }
    }
    else
    {
        for(i = 0;i < SF_DL; i++)
        {
            sf_pack->data_buf[i] = p_can_data->data[i+1];
        }
    }

    sf_pack->data_len = SF_DL;
    return SF_DL;
}

/*
 * analyze_uds_FF - 解析首帧，填充多帧包并返回总长度FF_DL
 * 说明：收到FF后需回FC通知对端发送CF。CAN-FD长度在data[4]/data[5]、首段数据从
 *       data[6]起58字节；普通CAN长度在data[0]高4位+data[1]、首段数据从data[2]起
 *       6字节。解析后将frame_sta置为接收中、cur_sn置1等待第一帧CF。
 */
unsigned short analyze_uds_FF(struct CAN_DATA *p_can_data,struct MF_PACK *mf_pack)// return FF_DL
{// when receive FF, should triggle CF
    unsigned short FF_DL = 0;
    unsigned char i = 0;
    if(uds_is_canfd()){//canfd首帧的SDL位于BYTE2/3/4/5
        FF_DL = (p_can_data->data[4] << 8) + (p_can_data->data[5]);
        for(i = 0;i < 58;i++){//剩余58个字节
            mf_pack->data_buf[i] = p_can_data->data[i+6];
        }
        mf_pack->cur_recv_idx = 58;
    }else{
        FF_DL = ((p_can_data->data[0]&0x0f) << 8) + p_can_data->data[1];
        for(i = 0;i < 6;i++){
            mf_pack->data_buf[i] = p_can_data->data[i+2];
        }
        mf_pack->cur_recv_idx = 6;
    }
    mf_pack->data_len = FF_DL;
    mf_pack->frame_sta = MF_STA_REVING;
    mf_pack->cur_sn = 1;// this is first frame ,so , set the first continue frame's sn to 1
    return FF_DL;
}

/*
 * analyze_uds_CF - 解析连续帧，校验SN并追加数据到多帧包
 * 说明：首字节低4位为SN，需与期望值cur_sn一致，否则返回ERR_SN_WRONG；校验通过后
 *       递增cur_sn(0x0F回绕为0)。CAN-FD每帧追加63字节、普通CAN追加7字节；当接收
 *       索引达到总长时返回ERR_RECV_END表示收齐。
 */
unsigned char analyze_uds_CF(struct CAN_DATA *p_can_data,struct MF_PACK *mf_pack)// return sn
{// current do not solve over data length ###
    unsigned char SN = 0, i = 0;
    unsigned char data_len = 0;

    SN = (p_can_data->data[0]&0x0f);
    if(SN != mf_pack->cur_sn){
        return ERR_SN_WRONG;// CF SN ERROR
    }else{// checking sn auto increase
        if(mf_pack->cur_sn == 0x0f)
            mf_pack->cur_sn = 0;// 1;  when sn go to 0x0f , come to be 0
        else
            mf_pack->cur_sn++;
    }

    if(uds_is_canfd()){//连续帧只有数据长度不一样
        data_len = 63;
    }else{
        data_len = 7;
    }
    for(i = 0;i < data_len;i++){
        mf_pack->data_buf[mf_pack->cur_recv_idx] = p_can_data->data[i+1];
        mf_pack->cur_recv_idx++;
    }
    if(mf_pack->cur_recv_idx >= mf_pack->data_len){
         return ERR_RECV_END;// all data has receive
    }else{
        return SN;// return correct sn
    }
}

/*
 * analyze_uds_FC - 解析流控帧，返回FS状态(非法返回-1)
 * 说明：首字节低4位为FS(0=CTS、1=WT、2=OVFLW)，data[1]=BS块大小、data[2]=STmin
 *       最小间隔；解析结果写入fc_sta供发送状态机决定后续CF发送节奏。
 */
unsigned char analyze_uds_FC(struct CAN_DATA *p_can_data,struct FC_STATUS *fc_sta)// return fs status,if error return (-1)
{//流控帧对于是否canfd，内容完全一致
    unsigned char FS = 0;

    fc_sta->bs = p_can_data->data[1];
    fc_sta->stmin = p_can_data->data[2];
    FS = p_can_data->data[0] & 0x0f;
    switch(FS)
    {
    case 0:
        fc_sta->fs = FS;
        return FC_FS_CTS;
    case 1:
        fc_sta->fs = FS;
        return FC_FS_WT;
    case 2:
        fc_sta->fs = FS;
        return FC_FS_OVFLW;
    default:
        fc_sta->fs = FC_FS_UNUSED;// is error
        return -1;
    }
}


/*
 * reset_sf_buf - 复位单帧包：清空首字节与长度
 */
void reset_sf_buf(struct SF_PACK *sf_pack)
{
    sf_pack->data_buf[0] = 0;
    sf_pack->data_len = 0;
}

/*
 * reset_mf_buf - 复位多帧包：清空索引/SN/首字节/长度，状态回到空闲
 */
void reset_mf_buf(struct MF_PACK *mf_pack)
{
    mf_pack->cur_recv_idx = 0;
    mf_pack->cur_sn = 0;
    mf_pack->data_buf[0] = 0;
    mf_pack->data_len = 0;
    mf_pack->frame_sta = MF_STA_EMP;
}

/*
 * reset_fc_sta - 复位流控状态：清空BS/STmin，FS置为未使用
 */
void reset_fc_sta(struct FC_STATUS *fc_sta)
{
    fc_sta->bs = 0;
    fc_sta->stmin = 0;
    fc_sta->fs = FC_FS_UNUSED;
}