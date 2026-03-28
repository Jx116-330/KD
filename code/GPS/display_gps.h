/***
 * @file display_gps.h
 * @brief GPS坐标转换与显示模块头文件
 * @author JX116
 * @date 2026-03-21
 * @version 1.0
 *
 * @details 提供：
 *          - 经纬度到本地平面坐标的转换
 *          - 平面坐标到屏幕坐标的转换
 *          - 轨迹点与当前位置显示支持
 ***/

#ifndef _SHOW_GPS_H_
#define _SHOW_GPS_H_

#include "zf_common_headfile.h"

/** @brief 地球半径，单位米 */
#define EARTH_RADIUS   (6371000.0)

/** @brief 浮点比较使用的误差值 */
#define EPSILON   (1e-9)

/** @brief 圆周率常量 */
#define USER_PI     (3.1415926535898)

/** @brief 最多显示的轨迹点数量 */
#define DISPLAY_POINT_MAX  (50)

/**
 * @brief GPS经纬度点结构体
 */
typedef struct
{
    double lat;    /**< 纬度 */
    double lon;    /**< 经度 */
} gps_point;

/**
 * @brief 屏幕坐标点结构体
 */
typedef struct
{
    int16 x;       /**< 屏幕X坐标 */
    int16 y;       /**< 屏幕Y坐标 */
} screen_point;

extern screen_point screen_point_data[DISPLAY_POINT_MAX];

void user_gps_transition(gps_point *gps, int point_num);
void gps_to_xy(gps_point *gps, double *x, double *y);
void xy_to_screen(double *x, double *y, int point_num);
void get_transition(gps_point *gps, int point_num);
void screen_print_gps_point(void);

#endif
