#include "friction_identify.h"
#include "erob_drv.h"
#include "log.h"

// 摩擦力辨识数据
#define FRIC_TABLE_LEN (37)

float spd_x[FRIC_TABLE_LEN] = {0.1f,0.72f,1.45f,2.18f,2.90f,3.63f,4.35f,5.08f,5.80f,6.53f,
    7.25f,7.98f,8.70f,9.43f,10.15f,10.87f,11.59f,12.33f,13.05f,13.78f,
    14.50f,15.23f,15.95f,16.68f,17.40f,18.13f,18.85f,19.58f,20.30f,21.03f,
    21.75f,22.48f,23.23f,23.93f,24.66f,25.38f,26.11f};
float current_y[FRIC_TABLE_LEN] = {50.0f,160.50f,179.24f,197.73f,207.33f,215.97f,221.32f,226.32f,233.56f,240.11f,
    244.70f,249.98f,254.18f,258.07f,260.24f,265.76f,265.76f,272.05f,274.25f,278.42f,
    280.92f,283.45f,286.11f,290.84f,292.40f,295.50f,301.21f,301.63f,300.50f,304.15f,
    305.28f,303.67f,306.44f,308.27f,313.76f,317.24f,316.09f};

// 摩擦力补偿 spd_fbk 速度反馈 rpm
float fric_compensate_step(float spd_fbk)
{
    float dir = 0.0f;
    if(spd_fbk > 0.01f)
    {
        dir = 1.0f;
    }
    else if(spd_fbk < -0.01f)
    {
        dir = -1.0f;
    }
    else
    {
        dir = 0.0f;
    }
    float current_fric = dir * MATH_linear_table_interpolation(spd_x, current_y, FRIC_TABLE_LEN, MATH_absf(spd_fbk));
    return current_fric;
}

typedef struct{
    float dt; // 运行周期
    ERBO_DRV_mode_e mode; // 辨识模式
    uint8_t state; // 状态
    uint8_t start; // 开始测量 0x01
    float ref; // 目标值
    float ref_dir; // 方向
    uint8_t ref_idx; // 当前采集第几个点的数据
    float ref_min; // 最小目标值
    float ref_max; // 最大目标值
    uint8_t data_num; // 采集的数据点个数
    float spd; // 模组转速 rpm
    float current; // 电流 mA
    float angle_det; // 双编码器插值 deg
    // 采集平均值
    uint16_t cnt;
    float spd_avr; // 模组转速 rpm
    float current_avr; // 电流 mA
    float angle_det_avr; // 双编码器插值 deg
    float delay; // 延时 s
    float stable_time; // 稳定时间 s
    float stop_time; // 从最大转速停止的时间 s
}ST_FRIC_IDEN_t, *ST_FRIC_IDEN_h;


ST_FRIC_IDEN_t g_fric_ident;

void Friction_Identify_init(void)
{
    ST_FRIC_IDEN_h h = &g_fric_ident;
    h->data_num = 40u;
    h->ref_min = 0.0f;
    h->ref_max = 29.0f*HSQ_MATH_K_RPM2RADPS;
    h->stop_time = 20.0f;
    h->start = 0x00;
    h->stable_time = 5.0f;
    h->ref_dir = 1.0f;
    h->mode = EROB_DRV_MODE_SPD;
    EROB_DRV_set_mode(h->mode);
    EROB_DRV_set_ref(0.0f);
    EROB_DRV_set_motor_en(0x01);
}

// 辨识摩擦力模型-采集数据
void Friction_Identify_step(float dt)
{
    ST_FRIC_IDEN_h h = &g_fric_ident;
    h->dt = dt;
    h->spd = EROB_DRV_get_speed_rpm();
    h->current = EROB_DRV_get_current();
    h->angle_det = EROB_DRV_get_angle_det_deg();
    switch(h->state)
    {
        case 0x00: // 空闲
        {
            if(h->start)
            {
                h->state = 10u;
                h->start = 0x00;
                FRIC_LOG("%d, spd(rpm):%.3f, current(mA):%.3f, angle_det(deg):%.3f, current_fric(mA)", h->ref_idx, h->spd_avr, h->current_avr, h->angle_det_avr);
            }
            break;
        }

        case 10: // 设置目标转速 等待转速稳定
        {
            h->ref = h->ref_dir * (h->ref_min + h->ref_idx * (h->ref_max - h->ref_min) / (float)h->data_num);
            
            h->delay += h->dt;
            if(h->delay > h->stable_time)
            {
                h->delay = 0.0f;
                if(h->ref_idx < h->data_num)
                {
                    h->ref_idx ++;
                    h->state = 20u;
                }
                else
                {
                    h->ref_idx = 0x00;
                    h->ref = 0.0f;
                    if(h->ref_dir > 0.0f)
                    {
                        h->state = 30u;
                        h->ref_dir = -1.0f;
                    }
                    else
                    {
                        h->state = 0x00;
                    }
                }
            }
            break;
        }

        case 20: // 采集数据
        {
            
            h->spd_avr += h->spd;
            h->angle_det_avr += h->angle_det;
            h->current_avr +=h->current;
            h->cnt ++;
            uint16_t data_num = 5000u;
            if(h->cnt > data_num)
            {
                h->spd_avr /= data_num;
                h->angle_det_avr /= data_num;
                h->current_avr /= data_num;
                h->cnt = 0x00;
                h->state = 10u;
                float ic_firc = fric_compensate_step(h->spd_avr);
                float i_err = ic_firc - h->current_avr;
                FRIC_LOG("%d, %.3f, %.1f, %.1f, %.1f, %.1f", h->ref_idx, h->spd_avr, h->current_avr, h->angle_det_avr, ic_firc, i_err);
                h->spd_avr = 0.0f;
                h->angle_det_avr  = 0.0f;
                h->current_avr = 0.0f;
            }
            break;
        }

        case 30:// 减速到0
        {
            h->delay += h->dt;
            if(h->delay > h->stop_time)
            {
                h->delay = 0.0f;
                h->state = 10u;
            }
        }

        default:
            break;
    }
     EROB_DRV_set_ref(h->ref);
}

