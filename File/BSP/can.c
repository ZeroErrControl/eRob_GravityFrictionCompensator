/*
 * @Author: hsq007 2267222816@qq.com
 * @Date: 2024-12-09 22:36:44
 * @LastEditors: hsq007 2267222816@qq.com
 * @LastEditTime: 2024-12-15 00:17:47
 * @FilePath: \HITCRT竞培营2018\File\BSP\can.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "stm32f10x.h"

/**
 * PB8 CAN_RX
 * PB9 CAN_TX
 * **/
void CAN1_Configuration(void)
{
    GPIO_InitTypeDef gpio;
    NVIC_InitTypeDef nvic;
    CAN_InitTypeDef  can;
    CAN_FilterInitTypeDef can_filter;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1,  ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO , ENABLE);

    GPIO_PinRemapConfig(GPIO_Remap1_CAN1,ENABLE);

    gpio.GPIO_Pin = GPIO_Pin_8;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio);

    gpio.GPIO_Pin = GPIO_Pin_9;   
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &gpio);    

    nvic.NVIC_IRQChannel = USB_HP_CAN1_TX_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    nvic.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;  
    nvic.NVIC_IRQChannelPreemptionPriority = 0;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);


    /*CAN单元初始化***/
    CAN_DeInit(CAN1);
    /*CAN主控制寄存器器（CAN_MCR）*/
    can.CAN_TTCM = DISABLE;
    can.CAN_ABOM = DISABLE;
    can.CAN_AWUM = DISABLE;
    can.CAN_NART = DISABLE;  
    can.CAN_RFLM = DISABLE;
    can.CAN_TXFP = ENABLE;
    /*CAN位时序寄存器（CAN_BTR）*/
    /*CAN1波特率=36MHz/4/(1+5+3)=1MHz*/	
    can.CAN_Mode = CAN_Mode_Normal;
    can.CAN_SJW = CAN_SJW_1tq;
    can.CAN_BS1 = CAN_BS1_5tq;
    can.CAN_BS2 = CAN_BS2_3tq;
    can.CAN_Prescaler = 4;
    CAN_Init(CAN1, &can);


    /*CAN过滤器初始化*/
    can_filter.CAN_FilterNumber = 0;
    can_filter.CAN_FilterMode = CAN_FilterMode_IdMask;
    can_filter.CAN_FilterScale = CAN_FilterScale_32bit;
    can_filter.CAN_FilterIdHigh = 0;
    can_filter.CAN_FilterIdLow = 0;
    can_filter.CAN_FilterMaskIdHigh = 0;
    can_filter.CAN_FilterMaskIdLow = 0;
    can_filter.CAN_FilterFIFOAssignment = CAN_Filter_FIFO0;
    can_filter.CAN_FilterActivation = ENABLE;
    CAN_FilterInit(&can_filter);

    CAN_ITConfig(CAN1,CAN_IT_FMP0,ENABLE);
    //CAN_ITConfig(CAN1,CAN_IT_TME,ENABLE);
}
