#include "MyEncoder.h"

int switch_encoder_num = 0;
int switch_encoder_change_num = 0;
uint8 switch_encode_bring_flag = 0;
uint8 switch_encode_change_get_buff_flag = 0;

#define SWITCH_ENCODER_STEP_THRESHOLD    2

void MyEncoder_Init(void)
{
    encoder_quad_init(TIM6_ENCODER, Switch_ENCODER_L, Switch_ENCODER_R);
}

void Get_Switch_Num(void)
{
    int tmp = 0;
    static int i = 0;
    static int encoder_cnt = 0;
    static int timer_cnt = 0;
    static int last_switch_encoder_num = 0;

    timer_cnt = -My_Switch_encoder_get_count(TIM6_ENCODER);
    encoder_clear_count(TIM6_ENCODER);

    if(abs(timer_cnt) < SWITCH_ENCODER_STEP_THRESHOLD)
    {
        encoder_cnt += timer_cnt;
    }
    else
    {
        tmp = timer_cnt / SWITCH_ENCODER_STEP_THRESHOLD;
        switch_encoder_num += tmp;
        tmp = timer_cnt % SWITCH_ENCODER_STEP_THRESHOLD;
        encoder_cnt += tmp;
    }

    if(abs(encoder_cnt) >= SWITCH_ENCODER_STEP_THRESHOLD)
    {
        tmp = encoder_cnt / SWITCH_ENCODER_STEP_THRESHOLD;
        switch_encoder_num += tmp;
        tmp = encoder_cnt % SWITCH_ENCODER_STEP_THRESHOLD;
        encoder_cnt = 0;
        encoder_cnt += tmp;
    }

    if((last_switch_encoder_num != switch_encoder_num) && (switch_encode_change_get_buff_flag == 0))
    {
        switch_encode_change_get_buff_flag = 1;
        switch_encoder_change_num = switch_encoder_num - last_switch_encoder_num;
    }
    else if((last_switch_encoder_num != switch_encoder_num) && (switch_encode_change_get_buff_flag == 1))
    {
        switch_encoder_change_num = switch_encoder_change_num + switch_encoder_num - last_switch_encoder_num;
    }

    if((last_switch_encoder_num == switch_encoder_num) && (switch_encode_change_get_buff_flag == 0))
    {
        switch_encoder_change_num = 0;
    }

    last_switch_encoder_num = switch_encoder_num;

    if(encoder_cnt != 0)
    {
        i++;
        if(i > 200)
        {
            i = 0;
            encoder_cnt = 0;
        }
    }
    else
    {
        i = 0;
    }
}

int16 My_Switch_encoder_get_count(encoder_index_enum encoder_n)
{
    int16 encoder_data = 0;

    switch(encoder_n)
    {
        case TIM2_ENCODER: encoder_data = (int16)IfxGpt12_T2_getTimerValue(&MODULE_GPT120); break;
        case TIM3_ENCODER: encoder_data = (int16)IfxGpt12_T3_getTimerValue(&MODULE_GPT120); break;
        case TIM4_ENCODER: encoder_data = (int16)IfxGpt12_T4_getTimerValue(&MODULE_GPT120); break;
        case TIM5_ENCODER: encoder_data = (int16)IfxGpt12_T5_getTimerValue(&MODULE_GPT120); break;
        case TIM6_ENCODER: encoder_data = (int16)IfxGpt12_T6_getTimerValue(&MODULE_GPT120); break;
        default: encoder_data = 0; break;
    }

    return encoder_data;
}

uint8 If_Switch_Encoder_Change(void)
{
    switch_encode_change_get_buff_flag = 0;
    if(switch_encoder_change_num != 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
