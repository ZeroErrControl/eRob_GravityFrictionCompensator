/*
 * @Author: 翼之道男
 * @Date: 2024-12-15 01:20:59
 * @LastEditors: 翼之道男
 * @LastEditTime: 2024-12-15 01:37:55
 * @FilePath: \File\BSP\rtt.c
 * @Description: 翼之道男
 */
#include "rtt.h"
#include "yzdn_math.h"
#include "SEGGER_RTT.h"

#define RTT_BUFFER_LEN (1024U)
uint8_t g_rtt_buffer[RTT_BUFFER_LEN];

void rtt_init(void)
{
    SEGGER_RTT_ConfigUpBuffer(0x01, "JScope_i2i2i2i2i2i2", g_rtt_buffer, RTT_BUFFER_LEN, SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
}


/**
 * 一次写入8个int16_t数据到rtt_scope
 * **/
void rtt_scope_write(int16_t data_1, int16_t data_2, int16_t data_3, int16_t data_4, int16_t data_5, int16_t data_6)
{
    int16_t data[6] = {data_1, data_2, data_3, data_4, data_5, data_6};
    SEGGER_RTT_Write(0x01, data, 12u);
}
