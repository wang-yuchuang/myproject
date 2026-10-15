/* ******************* 
// can uds需要重载实现的接口
 **********************/
#ifndef __CAN_UDS_ARCH_H__
#define __CAN_UDS_ARCH_H__

// --- 用于配置的宏定义 ---
#define USE_SECURITY_ACCESS_SERVICE     1  //是否在升级流程中启用安全访问服务
#define app_sw_version_str      "ver1.00"  //升级查询时会询问的软件版本号，根据你自身的
#define MAX_DID_CHARACTORS_NUM  64         //最大DID字符串的长度，即22H服务最大支持查询的数据信息长度
#define MAX_NUMBERS_OF_BLOCK    512        //逻辑地址最大可写入的数据块

#include "DEVCAN.h"

// --- 需要按格式填入的CAN数据包结构 ---
struct IDT{
    unsigned long id:29;// can id , std id's length is 11bits, ext id's length is 29bits
    unsigned char port:2;// can port , port0 ~ port3, 00,01,10,11
    unsigned char type:1;//can type, std or ext
};
//can data structrue
struct CAN_DATA {
    struct IDT idt;
    unsigned char dlc;	      // can data len
    unsigned char data[64]; // can data , max 8 bytes
};


// --- 需要根据平台差异填写的函数 ---
extern e_CANNUM g_upgrade_can_port;
extern uint32_t g_uds_can_tx_fail_count;

/**
 * @brief 用于UDS CAN发送的函数, 你需要需要根据自己的软件平台，在其内部实现发送的具体源码
 * 
 * @param id , can-id
 * @param type , 0:标准帧，1:扩展帧
 * @param data0~data7, 数据域的内容
 * @return int , 返回值大于0表示发送成功
 */
int uds_send_data8(unsigned long id,unsigned char type,
                    unsigned char data0,unsigned char data1,unsigned char data2,unsigned char data3,
                    unsigned char data4,unsigned char data5,unsigned char data6,unsigned char data7);

/**
 * @brief 用于UDS CAN-FD发送的函数, CANFD支持64字节数据
 * 
 * @param id , can-id
 * @param type , 0:标准帧，1:扩展帧
 * @param data0~data7, 数据域前8字节
 * @param next_datas , 后续56字节数据指针
 * @return int , 返回值大于0表示发送成功
 */
int uds_send_data64(unsigned long id,unsigned char type,
                    unsigned char d0,unsigned char d1,unsigned char d2,unsigned char d3,
                    unsigned char d4,unsigned char d5,unsigned char d6,unsigned char d7,
                    unsigned char *next_datas);

/**
 * @brief 获取时间戳(1ms的时基)，如果所移植的系统时间片为10ms，则在函数内部返回值乘10
 * 
 * @return unsigned long ，返回时间戳
 */
unsigned long get_timer_tick(void);

/**
 * @brief 向指定的逻辑地址中写入数据，逻辑地址从0开始，不是目标升级板的某块内置或外置的具体flash地址，具体偏移及存储介质的位置
 *        由device实现方来实现
 * 
 * @param addr ，逻辑地址
 * @param msg_buff ，待写入数据指针
 * @param len ，本次写入的数据长度（字节）
 * @return int ，实际写入的数据长度
 */
int program_data_by_logic_addr(unsigned long addr,unsigned char *msg_buff,unsigned short len);

/**
 * @brief 获取种子，用于27H服务的安全访问使用
 * 
 * @return unsigned long ，返回的种子值（4字节）
 */
unsigned long uds_get_seed(void);

/**
 * @brief 根据种子生成KEY的函数，用于27H服务的安全访问使用，一般不用更改
 * 
 * @param input_seed ，输入的种子值
 * @return unsigned long ，返回的KEY值（4字节）
 */
unsigned long uds_seed_to_key(unsigned long input_seed);

/**
 * @brief crc校验函数，用于36H/31H传输校验时使用，一般不用更改
 * 
 * @param crc ，上一次crc的结果，可用于分段离散的计算crc
 * @param buffer ，待计算的输入数据缓存指针
 * @param size ，待计算的长度
 * @return unsigned long ，返回的crc32的计算值
 */
unsigned long uds_crc(unsigned long crc, const void *buffer, int size);
unsigned long uds_crc_init(unsigned long *wholepackcrc);
void uds_wholepackcrc(unsigned long *wholepackcrc,const void *buffer, int size);
/**
 * @brief 喂看门狗（对于开启了看门狗的系统），没有使用看门狗，内容填空即可
 * 
 */
void uds_feed_wdt(void);

/**
 * @brief 复位当前板子，根据平台来实现具体如何复位
 * 
 */
void uds_reset(void);

/**
 * @brief 擦除存放APP升级文件数据的区域，根据自身平台来实现
 * 
 * @return int ，小于0表示失败，等于0表示成功
 */
int uds_erase_app_data(void);

/**
 * @brief 所有升级数据保存结束后会调用此函数，有需要的情况下可以填入一些行为代码，
 *        一般是写入升级标识以及文件大小等信息，以用于后续覆盖应用代码使用
 * 
 * @return int 
 */
int uds_program_finish(void);

/**
 * @brief 确认是否为can-fd
 * 
 * @return int, 等于1表示为can-fd，等于0表示为can2.0
 */
int uds_is_canfd(void);

int uds_get_upgrade_data(unsigned long addr, unsigned long len, unsigned char **data_out);

uint32_t uds_get_upgrade_bin_len(void);
#endif