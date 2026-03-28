/*********************************************************************************************************************
* TC387 Opensourec Library （TC387 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
*
* 本文件是 TC387 开源库的一部分
********************************************************************************************************************/
#include "myhead.h"
#define IPS200_TYPE     (IPS200_TYPE_SPI)                                 // 使用 SPI 屏幕接口
#pragma section all "cpu0_dsram"
// 将全局变量放在 CPU0 的 RAM 中

// **************************** 核心代码 ****************************
int core0_main(void)
{
    uint8 gps_record_started = 0;
    uint8 last_gnss_flag = 0;
    uint32 record_start_time = 0;
    uint32 record_elapsed_time = 0;
    uint32 last_ui_update_time = 0;
    char buffer[64];

    clock_init();                   // 获取时钟频率
    debug_init();                   // 初始化默认调试串口
    Beep_Init();

    ips200_set_dir(IPS200_PORTAIT);
    ips200_set_color(RGB565_RED, RGB565_BLACK);
    ips200_init(IPS200_TYPE);
    ips200_clear();

    // GPS 初始化
    gnss_init(TAU1201);

    // 轨迹记录模块初始化
    path_recorder_init();

    cpu_wait_event_ready();         // 等待所有核心初始化完成

    while (TRUE)
    {
        uint32 current_time;
        float total_distance = 0.0f;
        uint32 total_time = 0;
        float avg_speed = 0.0f;
        uint8 has_data = 0;

        gnss_data_parse();
        current_time = system_getval_ms();

        // 只要收到过串口解析标志，就认为 GPS 确实在输出数据
        if (gnss_flag)
        {
            last_gnss_flag = 1;
        }
        has_data = last_gnss_flag;

        // 自动判断是否满足开始记录条件
        if (!gps_record_started)
        {
            if ((gnss.state == 1) && (gnss.satellite_used >= 4) &&
                (gnss.latitude != 0.0) && (gnss.longitude != 0.0))
            {
                if (path_recorder_start())
                {
                    gps_record_started = 1;
                    record_start_time = current_time;
                    Beep_Ring(200);
                }
            }
        }

        // 已经开始记录后，周期性执行记录任务
        if (gps_record_started)
        {
            path_recorder_task();
        }

        if (gps_record_started)
        {
            record_elapsed_time = current_time - record_start_time;
        }
        else
        {
            record_elapsed_time = 0;
        }

        // 每 500ms 刷新一次显示
        if (current_time - last_ui_update_time >= 500)
        {
            last_ui_update_time = current_time;
            path_recorder_get_stats(&total_distance, &total_time, &avg_speed);

            ips200_clear();

            ips200_show_string(0, 0, "GPS diagnose");

            sprintf(buffer, "Data:%d Fix:%d", (int)has_data, (int)gnss.state);
            ips200_show_string(0, 20, buffer);

            sprintf(buffer, "Sat:%d Flag:%d", (int)gnss.satellite_used, (int)gnss_flag);
            ips200_show_string(0, 40, buffer);

            if (!has_data)
            {
                ips200_show_string(0, 60, "Status: no data");
            }
            else if (gnss.state != 1)
            {
                ips200_show_string(0, 60, "Status: no fix");
            }
            else if (gnss.satellite_used < 4)
            {
                ips200_show_string(0, 60, "Status: weak fix");
            }
            else if (!gps_record_started)
            {
                ips200_show_string(0, 60, "Status: ready");
            }
            else
            {
                ips200_show_string(0, 60, "Status: recording");
            }

            sprintf(buffer, "Points:%d", path_recorder_get_point_count());
            ips200_show_string(0, 80, buffer);

            sprintf(buffer, "Dist:%.2fm", total_distance);
            ips200_show_string(0, 100, buffer);

            sprintf(buffer, "Time:%lu s", (unsigned long)(record_elapsed_time / 1000));
            ips200_show_string(0, 120, buffer);

            sprintf(buffer, "Lat:%.6f", gnss.latitude);
            ips200_show_string(0, 140, buffer);

            sprintf(buffer, "Lon:%.6f", gnss.longitude);
            ips200_show_string(0, 160, buffer);

            sprintf(buffer, "Speed:%.2f", gnss.speed);
            ips200_show_string(0, 180, buffer);
        }

        system_delay_ms(10);
    }
}
#pragma section all restore
// **************************** 核心代码 ****************************