/*
 * @Author: hsq007 2267222816@qq.com
 * @Date: 2024-12-09 22:36:44
 * @LastEditors: hsq007 2267222816@qq.com
 * @LastEditTime: 2024-12-15 00:12:51
 * @FilePath: \20241214-M2006电机实验\File\BSP\timer.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "stm32f10x.h"

/*************************************************************************
函 数 名：TIM2_Configuration
函数功能：TIM2定时器中断配置
备    注：arr：自动重装值;psc：时钟预分频数;
          定时器的计数器工作频率：72MHz/(psc-1)
		  中断频率：72MHz/(psc-1)/(arr-1)
*************************************************************************/
void TIM2_Configuration(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  tim;
    NVIC_InitTypeDef         nvic;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);//使能TIM2时钟
	
	tim.TIM_Period = arr;//设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	tim.TIM_Prescaler = psc;//设置用来作为TIM时钟频率除数的预分频值
	tim.TIM_ClockDivision = TIM_CKD_DIV1;//设置时钟分割
	tim.TIM_CounterMode = TIM_CounterMode_Up;//TIM向上计数模式

	TIM_TimeBaseInit(TIM2, &tim);//根据指定的参数初始化TIM的时基单元 
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);//使能TIM更新中断

    nvic.NVIC_IRQChannel = TIM2_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;//抢占优先级
    nvic.NVIC_IRQChannelSubPriority = 1;//子优先级
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

	TIM_Cmd(TIM2, ENABLE);//使能TIM外设
	
}
