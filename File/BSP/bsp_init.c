/*
 * @Author: hsq007 2267222816@qq.com
 * @Date: 2024-12-09 22:36:44
 * @LastEditors: hsq007 2267222816@qq.com
 * @LastEditTime: 2024-12-15 01:21:45
 * @FilePath: \20241214-M2006电机实验\File\BSP\bso_init.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "bsp_init.h"
#include "timer.h"
#include "can.h"
#include "usart.h"
#include "stm32f10x.h"
#include "hsq_math.h"
#include "rtt.h"

/*************************************************************************
函 数 名：BSP_Iinitialization
函数功能：板级支持包初始化
备    注：
*************************************************************************/
void BSP_Iinitialization(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置NVIC中断分组
    TIM2_Configuration(200-1,360-1);//72MHz/360/200=1000Hz
    CAN1_Configuration();
    USART1_Configuration();
    rtt_init();
}

