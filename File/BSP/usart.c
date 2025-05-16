/*
 * @Author: hsq007 2267222816@qq.com
 * @Date: 2024-12-09 22:36:44
 * @LastEditors: hsq007 2267222816@qq.com
 * @LastEditTime: 2024-12-15 00:16:57
 * @FilePath: \20241214-M2006电机实验\File\BSP\usart.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "stm32f10x.h"
#include "stdio.h"

void USART1_Configuration(void)
{
    GPIO_InitTypeDef  gpio;
    USART_InitTypeDef usart;
 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);//使能USART1，GPIOA时钟

    gpio.GPIO_Pin = GPIO_Pin_9;//USART1_TX PA.9
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;//复用推挽输出
    GPIO_Init(GPIOA, &gpio);//初始化PA9  

    gpio.GPIO_Pin = GPIO_Pin_10;//USART1_RX	PA.10
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(GPIOA, &gpio);//初始化PA10

    usart.USART_BaudRate = 9600;//一般设置为9600;
    usart.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
    usart.USART_StopBits = USART_StopBits_1;//一个停止位
    usart.USART_Parity = USART_Parity_No;//无奇偶校验位
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
    usart.USART_Mode = USART_Mode_Tx;//发送模式
    USART_Init(USART1, &usart);//初始化串口

    USART_Cmd(USART1, ENABLE);//使能串口 
}

//重定向printf
int fputc(int ch, FILE *f)
{
    USART_SendData(USART1, (uint8_t)ch);
    //等待发送完成
    while (USART_GetFlagStatus(USART1,USART_FLAG_TC) == RESET);
    return ch;
}

