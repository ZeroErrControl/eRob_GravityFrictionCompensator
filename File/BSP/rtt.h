/*
 * @Author: hsq007 2267222816@qq.com
 * @Date: 2024-12-15 01:21:02
 * @LastEditors: hsq007 2267222816@qq.com
 * @LastEditTime: 2024-12-15 01:34:43
 * @FilePath: \20241214-M2006电机实验\File\BSP\rtt.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef __RTT_H__
#define __RTT_H__
#include "hsq_math.h"

void rtt_init(void);
void rtt_scope_write(int16_t data_1, int16_t data_2, int16_t data_3, int16_t data_4, int16_t data_5, int16_t data_6);

#endif

