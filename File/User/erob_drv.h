#ifndef __EROB_DRV_H__
#define __EROB_DRV_H__

#include "hsq_math.h"
#include "stm32f10x.h"

// 关节模组控制模式
typedef enum
{
    EROB_DRV_MODE_NONE = 0x00, // 无
    EROB_DRV_MODE_TORQUE = 0x01, // 力矩模式
    EROB_DRV_MODE_SPD = 0x02, // 速度模式
    EROB_DRV_MODE_POS = 0x03, // 位置模式
}ERBO_DRV_mode_e;


void EROB_DRV_init(void);
void EROB_DRV_step(float dt);
void EROB_DRV_can_rx_callback(CanRxMsg *message);
void EROB_DRV_set_ref(float ref);
void EROB_DRV_set_mode(ERBO_DRV_mode_e mode);
void EROB_DRV_set_motor_en(uint8_t en);
float EROB_DRV_get_angle(void);
float EROB_DRV_get_speed(void);
float EROB_DRV_get_speed_rpm(void);
float EROB_DRV_get_angle_det_deg(void);
float EROB_DRV_get_current(void);

#endif


