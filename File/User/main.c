// 零差云控合作项目 谐波模组建模
// 实现CAN自定义通信协议和重力补偿模型
// 摩擦力模型正在开发中
// 2025.02.23 by 黄胜全

#include "stm32f10x.h"
#include "bsp_init.h"
#include "log.h"
#include "erob_drv.h"
#include "math.h"
#include "friction_identify.h"

// #define FRIC_IDENTIFY_EN // 测试摩擦力辨识
#define FRIC_GRAVITY_COMPENSATE // 使用摩擦和重力补偿

void Friction_Identify_init(void);
void Friction_Identify_step(float dt);


int main(void)
{
    BSP_Iinitialization();
    EROB_DRV_init();
    MAIN_LOG("bsp init ok!");
    MAIN_LOG("bsp init ok!");
    MAIN_LOG("bsp init ok!");
    MAIN_LOG("start morot!");
    #ifdef FRIC_IDENTIFY_EN
    //  开机后进调试模式手动设置
    Friction_Identify_init();
    #endif
    #ifdef FRIC_GRAVITY_COMPENSATE
    EROB_DRV_set_mode(EROB_DRV_MODE_TORQUE);
    EROB_DRV_set_ref(0.0f);
    EROB_DRV_set_motor_en(0x01);
    #endif
    
    while(1)
    {
        ;
    }
}

// 重力补偿
float k_gravity = -330.0f; // 重力补偿系数 
float i_gravity; // 重力补偿电流 A
float gravity_gain = 1.0f; // 重力补偿调整系数
// 摩擦力补偿
float i_fric; // 摩擦力补偿电流 A
float fric_gain = 1.05f;


// 重力补偿 angle 输出轴角度 rad
float gravity_compensate_step(float angle)
{
    // 重力补偿计
    float i_gravity = k_gravity * sin(angle);
    return i_gravity;
}


// 这个线程不能被阻塞 1ms 执行一次
void app_1ms_task(void)
{
    float dt = 0.001f;
    #ifdef FRIC_GRAVITY_COMPENSATE
    // 摩擦力和重力补偿计算
    float spd_fbk = EROB_DRV_get_speed_rpm();
    float angle = EROB_DRV_get_angle();
    i_fric = fric_gain * fric_compensate_step(spd_fbk);
    i_gravity = gravity_gain * gravity_compensate_step(angle);
    float current = i_fric + i_gravity;
    EROB_DRV_set_ref(current);
    #endif
    
    #ifdef FRIC_IDENTIFY_EN
    Friction_Identify_step(dt);
    #endif

    // 电机驱动
    EROB_DRV_step(dt);
}


