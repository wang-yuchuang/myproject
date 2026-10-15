/* ******************* 
 * can uds
 **********************/
#ifndef __CAN_UDS_SERVICES_LAYER_H__
#define __CAN_UDS_SERVICES_LAYER_H__


#define ECU_MASK		0x79014010

// diagnostic service
#define ds_diagnostic_session_ctrl      0x10	//支持
#define ds_security_access              0x27    //支持，两个安全等级
#define ds_tester_present               0x3e	//支持，仅回复
#define ds_ecu_reset                    0x11    //支持，仅一种复位方式
#define ds_communication_ctrl           0x28    //支持，目前没有实际作用
#define ds_ctrl_dtc_setting             0x85    //不支持，没用到
#define ds_read_data_by_id              0x22	//支持-- 后面追加CRC值
#define ds_write_data_by_id             0x2e	//支持-- 后面追加CRC
#define ds_read_dtc_info                0x19	//不支持，没用到
#define ds_clr_diagnostic_info          0x14	//不支持，没用到
#define ds_io_ctrl_by_id                0x2f    //不支持，没用到
#define ds_routine_ctrl                 0x31    //支持，没用到 增加擦除/启动文件完整性校验/传输模式：带CRC/不发带CRC
#define ds_request_download             0x34    //支持
#define ds_request_upload               0x35    //不支持
#define ds_transfer_data                0x36    //支持
#define ds_request_transferexit         0x37    //支持


// 响应拒绝码
#define nrc_general_reject                                  0x10
#define nrc_service_not_supported                           0x11
#define nrc_subfunction_not_support                         0x12
#define nrc_invalid_message_lenth_or_format                 0x13
#define nrc_conditions_not_correct                          0x22
#define nrc_request_sequence_error                          0x24
#define nrc_request_out_of_range                            0x31
#define nrc_security_access_denied                          0x33
#define nrc_invalid_key                                     0x35
#define nrc_exceeded_number_of_attempts                     0x36
#define nrc_required_time_delay_not_expired                 0x37
#define nrc_upload_download_not_accepted                    0x70
#define nrc_transfer_data_suspended                         0x71
#define nrc_general_programming_failure                     0x72
#define nrc_wrong_block_sequence_counter                    0x73
#define nrc_service_busy                                    0x78
#define nrc_subfunction_not_supported_in_active_session     0x7E
#define nrc_service_not_support_in_active_session           0x7F

#define nrc_single_pack_crc_check_error                     0xF2
#define nrc_whole_pack_crc_check_error                      0xF3

//#DIDs
#define app_sw_id                       0xf181              //只打开一个内容，即版本号
#define app_crc_value                   0xf182              //升级文件CRC值

#define DID_OPTION_READ                 0
#define DID_OPTION_WRITE                1

// DTC code
#define lost_with_abs_dtc               0xc12100
#define lost_with_bcm_dtc               0xc14000
#define single_wire_err_dtc             0xd70097
#define bus_off_dtc                     0xc07300
#define antenna_to_batt_dtc             0x960115
#define antenna_to_gnd_dtc              0x960112
#define batt_high_dtc                   0x911717
#define batt_low_dtc                    0x911616
#define lost_with_tcu_dtc               0xc10100
#define lost_with_ic_dtc                0xc15500

#define DEFAULT_STMIN_TIME              200                 //如果没有收到有效的STmin，默认为200ms

extern unsigned char dtc_update_flg;

enum{COMMU_EN_RX_TX = 0,COMMU_EN_RX_DIS_TX,COMMU_DIS_RX_EN_TX,COMMU_DIS_RX_TX};
extern unsigned char cur_communication_type;
extern unsigned char uds_send_flg;

//--- 函数声明 ---
void reset_uds_send_buf(void);
void uds_phy_recv(struct CAN_DATA *p_can_data,unsigned char reset);
void uds_phy_send(unsigned char reset);
void uds_upgrade_proc(void);
void uds_upgrade_reset(void);
#endif