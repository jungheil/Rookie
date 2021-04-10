/*************************************************************************
 * @brief         上位机与下位机串口通信
 * @version        1.0.0.1
 * @authors        何武军
 * -----------------------------------------------------------------------	
 *   Change Hisitory ：
 *   <Date>     | <Verision> | <Author> |<Descripition>
 * -----------------------------------------------------------------------
 *   2019/02/16 |  1.0.0.1  |   何武军   | 修改宏定义以及代码规范的命名格式
 *
 ************************************************************************/
#ifndef T_DT2019VISION_USART_H
#define T_DT2019VISION_USART_H
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> //文件控制定义
#include <termios.h>//终端控制定义
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <mutex>
#include <vector>

//设备名称
const char kDevice[] = "/dev/serial/by-id/usb-1a86_USB_Serial-if00-port0";
const int  kTotalSendLength = 6;
const int  kReceiveLength = 11;
using namespace std;


class Usart{
public:
    static int serial_fd_;//设备名称
    Usart();//串口初始化

    int UsartSend(unsigned char* data);
    int UsartSend(uint16_t * data);
    /**
    * @brief    串口接收数据
    *            要求启动后，在pc端发送ascii文件
    */
    int UsartRecv(volatile int *data);
    uint16_t GetCRC16CheckSum(uint8_t *pchMessage,uint32_t dwLength,uint16_t wCRC);
    uint32_t VerifyCRC16CheckSum(uint8_t *pchMessage, uint32_t dwLength);
    void AppendCRC16CheckSum(uint8_t  * pchMessage,uint32_t dwLength);
};

#endif //T_DT2019VISION_USART_H
