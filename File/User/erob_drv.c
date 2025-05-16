// 零差云控CAN自定义协议驱动
// 实现电机速度和电流模式控制，实现位置、速度、电流采集
// 2025.02.23 by 黄胜全


#include "stdio.h"
#include "erob_drv.h"
#include "lpf.h"

#define EROB_LOG(format, ...) printf("[EROB]" format"\r\n", ##__VA_ARGS__)


typedef enum
{
    EROB_DRV_STATE_OFF = 0x00,
    EROB_DRV_STATE_TX_EN,
    EROB_DRV_STATE_TX_MODE,
    EROB_DRV_STATE_TX_CMD,
    EROB_DRV_STATE_TX_SPD_LIMIT,
    EROB_DRV_STATE_TX_ACK,
    EROB_DRV_STATE_RX_SPD,
    EROB_DRV_STATE_RX_SPD_ACK,
    EROB_DRV_STATE_RX_POS,
    EROB_DRV_STATE_RX_POS_ACK,
    EROB_DRV_STATE_RX_CURRENT,
    EROB_DRV_STATE_RX_CURRENT_ACK,
    EROB_DRV_STATE_RX_TORQUE,
    EROB_DRV_STATE_RX_TORQUE_ACK,
    EROB_DRV_STATE_ERR,
}EROB_DRV_state_e;

typedef struct 
{
    uint8_t en_input; // 使能输入信号
    uint8_t start; // 0x01=开始
    float dt; // 控制周期 s
    ERBO_DRV_mode_e mode; 
    EROB_DRV_state_e state; // 设置控制模式状态
    // 控制指令
    uint8_t motor_en; // 电机使能标志位
    float ref; // 控制目标值
     // 模组状态信号
    float angle; // 输出轴角度 rad
    float angle_deg; // 输出轴角度 deg
    float speed; // 输出轴转速 rad/s
    float speed_rpm; // 输出轴转速 rpm
    float current; // 电机的q轴电流 mA
    float torque; // 扭矩 N*m
    float angle_delta; // 双编码器差值 rad
    float angle_delta_deg; // 双编码器差值 deg
    // 辅助观察变量
    float angle_ref;  // 角度目标值 rad
    int32_t angle_ref_sim; // 速度目标值 线
    float speed_ref;  // 速度目标值 rad/s
    float speed_ref_rpm; // 速度目标值 rpm
    int32_t current_ref; // 电流目标值 mA
    int32_t speed_limit_raw;
    // 记录
    EROB_DRV_state_e next_state; // 下一个状态
    ERBO_DRV_mode_e mode_pre; // 当前控制模式
    uint8_t motor_en_pre;
    int32_t current_ref_pre;
    int32_t speed_ref_sim; // 速度目标值 线/s
    ST_LPF  st_lpf_current; // 电流低通滤波
    // 数据解析
    int32_t speed_raw; // can接收的速度原始数据
    int32_t angle_raw; // can接收的位置原始数据
    int32_t current_raw; // can接收的电流原始数据
    int32_t torque_raw; // can接收的扭矩传感器原始数据
    uint32_t encoder_sim; // 编码器线数
    float encoder_sim_div; // 编码器线数倒数
    float k_radps2simps; // 转换系数 rad/s -> 线/s
    // CAN 发送报文控制
    uint8_t can_tx_test; // 0x01=发送报文测试
    uint8_t motor_id; // 电机的ID 0-10
    uint8_t can_tx_data[6]; // 要发生的CAN报文数据
    uint8_t can_tx_len; // 发送的CAN报文数据长度
    uint8_t can_tx_tick; // 发生CAN报文的次数
    // CAN 接收报文控制
    uint8_t can_rx_data[8]; // 接收的CAN报文数据
    uint8_t can_rx_len; // 接收到的CAN报文长度
    uint8_t can_rx_flag; // 0x01=接受到报文
    // 报文接收出错计数
    uint16_t can_rx_err_cnt_pos;
    uint16_t can_rx_err_cnt_current;
    uint16_t can_rx_err_cnt_speed;
    uint16_t can_rx_err_cnt_torque;
}EROB_DRV_t, *EROB_DRV_h;

void EROB_DRV_can_tx(EROB_DRV_h h);


EROB_DRV_t g_erob_drv;

void EROB_DRV_init(void)
{
    EROB_DRV_h h = &g_erob_drv;
    h->motor_id = 10u;
    h->en_input = 0x01;
    h->encoder_sim = 524288u;
    h->encoder_sim_div = 1.0f / (float)(h->encoder_sim);
    h->k_radps2simps = 1.0f / HSQ_MATH_2PI * h->encoder_sim;
    h->mode = EROB_DRV_MODE_NONE;
    h->start = 0x01;
    h->speed_limit_raw = 520000;
    lpf_init(&h->st_lpf_current, 0.1f);
}

// 填充要发送的CAN报文
/**
 * len 报文的长度 最大6
 * data 报文数据数组
 * **/
void EROB_DRV_can_tx_load(EROB_DRV_h h, uint8_t len, uint8_t data[6])
{
    h->can_tx_len = len;
    for(uint8_t i=0; i<len; i++)
    {
        h->can_tx_data[i] = data[i];
    }
    h->can_tx_tick = 0x01; // 发送一次
}


// 等待指令发送成功后切换到下一个状态 next_state
void EROB_DRV_state_wait_ack(EROB_DRV_h h, EROB_DRV_state_e next_state)
{
    h->next_state = next_state;
    h->state = EROB_DRV_STATE_TX_ACK;
}

// 设置点击控制模式的状态
void EROB_DRV_step(float dt)
{
    EROB_DRV_h h = &g_erob_drv;
    h->dt =dt;
    lpf_step(&h->st_lpf_current, h->current, dt);
    if(h->can_tx_test && h->can_rx_flag)
    {
        EROB_LOG("[CAN_RX]%02x,%02x,%02x,%02x,%02x", h->can_rx_data[0], h->can_rx_data[1],h->can_rx_data[2],h->can_rx_data[3],h->can_rx_data[4]);
        h->can_rx_flag = 0x00;
    }
    switch(h->state)
    {
        case EROB_DRV_STATE_OFF:
        {
            if(h->start)
            {
                h->state = EROB_DRV_STATE_TX_EN;
            }
            break;
        }
        
        
        case EROB_DRV_STATE_TX_EN: // 发送使能信号
        {
            // 关闭电机动作
            if(h->start == 0x00)
            {
               h->motor_en = 0x00;
               h->ref = 0.0f;
            }
            // 控制模式
            if(h->motor_en != h->motor_en_pre)
            {
                uint8_t en_data[6] = {0x01, 0x00, 0x00, 0x00, 0x00, h->motor_en};
                EROB_DRV_can_tx_load(h, 0x06, en_data);
                EROB_DRV_state_wait_ack(h, EROB_DRV_STATE_TX_MODE);
                EROB_LOG("set en:%d -> %d", h->motor_en_pre, h->motor_en);
                h->motor_en_pre = h->motor_en;
            }
            else
            {
                h->state = EROB_DRV_STATE_TX_MODE;
            }
            break;
        }

        case EROB_DRV_STATE_TX_MODE: // 发送控制模式指令
        {
            if(h->mode != h->mode_pre)
            {
                uint8_t en_data[6] = {0x00, 0x4e, 0x00, 0x00, 0x00, h->mode};
                EROB_DRV_can_tx_load(h, 0x06, en_data);
                EROB_DRV_state_wait_ack(h, EROB_DRV_STATE_TX_CMD);
                EROB_LOG("set mode:%d -> %d", h->mode_pre, h->mode);
                h->mode_pre = h->mode;
            }
            else
            {
                h->state = EROB_DRV_STATE_TX_CMD;
            }
            break;
        }


        case EROB_DRV_STATE_TX_CMD: // 设置电流目标值
        {
            if(h->mode == EROB_DRV_MODE_TORQUE)
            {
                h->current_ref = h->ref;
                uint8_t en_data[6] = {0x01, 0xfe, (h->current_ref >> 24u)&0xff, (h->current_ref >> 16u)&0xff, (h->current_ref >> 0x08)&0xff, (h->current_ref & 0xFF)};
                EROB_DRV_can_tx_load(h, 0x06, en_data);
                EROB_DRV_state_wait_ack(h, EROB_DRV_STATE_TX_SPD_LIMIT);
                h->current_ref_pre = h->current_ref;
            }
            else if(h->mode == EROB_DRV_MODE_SPD)
            {
                h->speed_ref = h->ref;
                h->speed_ref_sim = h->speed_ref * h->k_radps2simps;
                h->speed_ref_rpm = h->speed_ref * HSQ_MATH_K_RADPS2RPM;
                uint8_t en_data[6] = {0x01, 0xfe, (h->speed_ref_sim >> 24u)&0xff, (h->speed_ref_sim >> 16u)&0xff, (h->speed_ref_sim >> 0x08)&0xff, (h->speed_ref_sim & 0xFF)};
                EROB_DRV_can_tx_load(h, 0x06, en_data);
                EROB_DRV_state_wait_ack(h, EROB_DRV_STATE_RX_SPD);
            }
            else if(h->mode == EROB_DRV_MODE_POS)
            {
                h->angle_ref = h->ref;
            }
            break;
        }

        case EROB_DRV_STATE_TX_SPD_LIMIT: // 设置电流目标值
        {
            if(h->mode == EROB_DRV_MODE_TORQUE)
            {
                int32_t data_raw = h->speed_limit_raw;
                uint8_t sen_data[6] = {0x02, 0x04, (data_raw >> 24u)&0xff, (data_raw >> 16u)&0xff, (data_raw >> 0x08)&0xff, (data_raw & 0xFF)};
                EROB_DRV_can_tx_load(h, 0x06, sen_data);
                EROB_DRV_state_wait_ack(h, EROB_DRV_STATE_RX_SPD);
            }
            else
            {
                EROB_DRV_state_wait_ack(h, EROB_DRV_STATE_RX_SPD);
            }
            break;
        }

        case EROB_DRV_STATE_RX_SPD: // 读取转速反馈
        {
            uint8_t en_data[4] = {0x00, 0x05, 0x00, 0x01};
            EROB_DRV_can_tx_load(h, 0x04, en_data);
            h->state = EROB_DRV_STATE_RX_SPD_ACK;
            break;
        }

        case EROB_DRV_STATE_RX_SPD_ACK: // 解析转速反馈
        {
            if(h->can_rx_flag)
            {
                h->can_rx_flag = 0x00;
                if((h->can_rx_len == 0x05)&&(h->can_rx_data[4]== 0x3e))
                {
                    h->speed_raw = (int32_t)((h->can_rx_data[0] << 24) + (h->can_rx_data[1] << 16) + (h->can_rx_data[2] << 8) + h->can_rx_data[3]);
                    h->speed = h->speed_raw * h->encoder_sim_div * HSQ_MATH_2PI;
                    h->speed_rpm = h->speed * HSQ_MATH_K_RADPS2RPM;
                    h->state = EROB_DRV_STATE_RX_POS;
                }
                else
                {
                    h->state = EROB_DRV_STATE_RX_SPD;
                    h->can_rx_err_cnt_speed ++;
                    EROB_LOG("read speed err, len:%d, %02x, %02x, %02x, %02x, %02x", h->can_rx_len, h->can_rx_data[0], h->can_rx_data[1], h->can_rx_data[2], h->can_rx_data[3], h->can_rx_data[4]);
                }
            }
            break;
        }

        
        case EROB_DRV_STATE_RX_POS: // 读取位置反馈
        {
            uint8_t en_data[2] = {0x00, 0x02};
            EROB_DRV_can_tx_load(h, 0x02, en_data);
            h->state = EROB_DRV_STATE_RX_POS_ACK;
            break;
        }

        case EROB_DRV_STATE_RX_POS_ACK: // 解析位置反馈
        {
            if(h->can_rx_flag)
            {
                h->can_rx_flag = 0x00;
                if((h->can_rx_len == 0x05)&&(h->can_rx_data[4]== 0x3e))
                {
                    h->angle_raw = (int32_t)((h->can_rx_data[0] << 24) + (h->can_rx_data[1] << 16) + (h->can_rx_data[2] << 8) + h->can_rx_data[3]);
                    h->angle = h->angle_raw * h->encoder_sim_div * HSQ_MATH_2PI;
                    h->angle_deg = h->angle * HSQ_MATH_K_RAD2DEG;
                    h->state = EROB_DRV_STATE_RX_CURRENT;
                }
                else
                {
                    h->state = EROB_DRV_STATE_RX_POS;
                    h->can_rx_err_cnt_pos ++;
                    EROB_LOG("read angle err, len:%d, %02x, %02x, %02x, %02x, %02x", h->can_rx_len, h->can_rx_data[0], h->can_rx_data[1], h->can_rx_data[2], h->can_rx_data[3], h->can_rx_data[4]);
                }
            }
            break;
        }

        case EROB_DRV_STATE_RX_CURRENT: // 读取电流反馈
        {
            uint8_t en_data[2] = {0x00, 0x08};
            EROB_DRV_can_tx_load(h, 0x02, en_data);
            h->state = EROB_DRV_STATE_RX_CURRENT_ACK;
            break;
        }

        case EROB_DRV_STATE_RX_CURRENT_ACK: // 解析电流反馈
        {
            if(h->can_rx_flag)
            {
                h->can_rx_flag = 0x00;
                if((h->can_rx_len == 0x05)&&(h->can_rx_data[4]== 0x3e))
                {
                    h->current_raw = (int32_t)((h->can_rx_data[0] << 24) + (h->can_rx_data[1] << 16) + (h->can_rx_data[2] << 8) + h->can_rx_data[3]);
                    h->current = h->current_raw;
                    h->state = EROB_DRV_STATE_RX_TORQUE;
                }
                else
                {
                    h->state = EROB_DRV_STATE_RX_CURRENT;
                    h->can_rx_err_cnt_current ++;
                    EROB_LOG("read current err, len:%d, %02x, %02x, %02x, %02x, %02x", h->can_rx_len, h->can_rx_data[0], h->can_rx_data[1], h->can_rx_data[2], h->can_rx_data[3], h->can_rx_data[4]);
                }
            }
            break;
        }

        case EROB_DRV_STATE_RX_TORQUE: // 读取扭矩
        {
            uint16_t torque_adress = 0x119; // 扭矩地址
            uint8_t en_data[2] = {torque_adress >> 0x08, torque_adress & 0xff};
            EROB_DRV_can_tx_load(h, 0x02, en_data);
            h->state = EROB_DRV_STATE_RX_TORQUE_ACK;
            break;
        }

        case EROB_DRV_STATE_RX_TORQUE_ACK: // 解析扭矩数据
        {
            if(h->can_rx_flag)
            {
                h->can_rx_flag = 0x00;
                if((h->can_rx_len == 0x05)&&(h->can_rx_data[4]== 0x3e))
                {
                    h->torque_raw = (int32_t)((h->can_rx_data[0] << 24) + (h->can_rx_data[1] << 16) + (h->can_rx_data[2] << 8) + h->can_rx_data[3]);
                    h->torque = h->torque_raw;
                    h->angle_delta = h->torque_raw * h->encoder_sim_div * HSQ_MATH_2PI;
                    h->angle_delta_deg = h->angle_delta * HSQ_MATH_K_RAD2DEG;
                    h->state = EROB_DRV_STATE_TX_EN;
                }
                else
                {
                    h->state = EROB_DRV_STATE_RX_TORQUE;
                    h->can_rx_err_cnt_torque ++;
                    EROB_LOG("read torque err, len:%d, %02x, %02x, %02x, %02x, %02x", h->can_rx_len, h->can_rx_data[0], h->can_rx_data[1], h->can_rx_data[2], h->can_rx_data[3], h->can_rx_data[4]);
                }
            }
            break;
        }
        
        case EROB_DRV_STATE_TX_ACK: // 等待指令发送成功
        {
            if(h->can_rx_flag)
            {
                // 指令成功
                h->can_rx_flag = 0x00;
                if((h->can_rx_len == 0x01)&&(h->can_rx_data[0] == 0x3e))
                {
                    h->state = h->next_state;
                }
                else // 失败
                {
                    h->state = EROB_DRV_STATE_ERR;
                    EROB_LOG("can_rx err! rx message , len:%d, %02x, %02x, %02x, %02x, %02x", h->can_rx_len, h->can_rx_data[0], h->can_rx_data[1], h->can_rx_data[2], h->can_rx_data[3], h->can_rx_data[4]);
                }
            }
            break;
        }

        case EROB_DRV_STATE_ERR: // ERR
        {
            ;
        }
    }

    // 发送CAN报文
    EROB_DRV_can_tx(h);
}   


void EROB_DRV_can_tx(EROB_DRV_h h)
{
    CanTxMsg g_tx_message = {0x00};
    CanTxMsg *ch = &g_tx_message;
    ch->StdId = h->motor_id + 0x640;
    ch->IDE = CAN_Id_Standard;
    ch->RTR = CAN_RTR_Data;
    ch->DLC = h->can_tx_len;
    for(uint8_t i=0; i< h->can_tx_len; i++)
    {
        ch->Data[i] = h->can_tx_data[i];
    }
    
    if(h->can_tx_tick > 0x00)
    {
        CAN_Transmit(CAN1, &g_tx_message);
        h->can_tx_tick --;
        if(h->can_tx_test)
        {
            EROB_LOG("[CAN_TX]%02x,%02x,%02x,%02x,%02x,%02x", ch->Data[0], ch->Data[1],ch->Data[2],ch->Data[3],ch->Data[4],ch->Data[5]);
        }
    }
}

// CAN 报文接收数据回调函数
void EROB_DRV_can_rx_callback(CanRxMsg *message)
{
    EROB_DRV_h h = &g_erob_drv;
    for(uint8_t i=0; i<0x08; i++)
    {
        h->can_rx_data[i] = 0x00;
    }
    if(message->StdId == (0x5c0+h->motor_id))
    {
        h->can_rx_len = message->DLC;
        for(uint8_t i=0; i< message->DLC; i++)
        {
            h->can_rx_data[i] = message->Data[i];
        }
        h->can_rx_flag = 0x01;
    }
}


//----------- set
// 设置电机工作模式
void EROB_DRV_set_mode(ERBO_DRV_mode_e mode)
{
    EROB_DRV_h h = &g_erob_drv;
    if(h->en_input)
    {
        h->mode = mode;
    }
}

// 设置目标值
void EROB_DRV_set_ref(float ref)
{
    EROB_DRV_h h = &g_erob_drv;
    if(h->en_input)
    {
        h->ref = ref;
    }
}

// 设置电机使能
void EROB_DRV_set_motor_en(uint8_t en)
{
    EROB_DRV_h h = &g_erob_drv;
    if(h->en_input)
    {
        h->motor_en = en;
    }
}

//------------ get

// 获取输出轴角度 rad
float EROB_DRV_get_angle(void)
{
    EROB_DRV_h h = &g_erob_drv;
    return h->angle;
}

// 获取输出轴转速 rad/s
float EROB_DRV_get_speed(void)
{
    EROB_DRV_h h = &g_erob_drv;
    return h->speed;
}

// 获取电流值 mA
float EROB_DRV_get_current(void)
{
    EROB_DRV_h h = &g_erob_drv;
    return h->current;
}


float EROB_DRV_get_speed_rpm(void)
{
    EROB_DRV_h h = &g_erob_drv;
    return h->speed_rpm;
}

// 获取双编码器角度差 deg
float EROB_DRV_get_angle_det_deg(void)
{
    EROB_DRV_h h = &g_erob_drv;
    return h->angle_delta_deg;
}
