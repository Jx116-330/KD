/**
 * @file display_gps.h
 * @brief GPS坐标转换与显示模块头文件
 * @author 用户
 * @date 2026-03-21
 * @version 1.0
 *
 * @details 该模块提供GPS坐标系的转换和显示功能，包括：
 *          - 经纬度坐标到平面坐标的转换
 *          - 平面坐标到屏幕坐标的转换
 *          - 多种屏幕类型的支持
 */

#ifndef _SHOW_GPS_H_
#define _SHOW_GPS_H_

#include "zf_common_headfile.h"

/** @brief 地球半径，单位：米 */
#define EARTH_RADIUS   (6371000.0)

/** @brief 浮点数比较的精度阈值 */
#define EPSILON   (1e-9)

/** @brief 圆周率常量 */
#define USER_PI     (3.1415926535898)

/** @brief 最大显示点数 */
#define DISPLAY_POINT_MAX  (50)


/**
 * @brief GPS经纬度坐标点结构体
 */
typedef struct
{
    double lat;    /**< 纬度，单位：度 */
    double lon;     /**< 经度，单位：度 */
} gps_point;

/**
 * @brief 平面坐标系点结构体
 * @details 以第一个GPS点为原点，东方向为X轴，北方向为Y轴
 */
typedef struct
{
    double x;    /**< 东方向坐标，单位：米，相对于第一个点 */
    double y;   /**< 北方向坐标，单位：米，相对于第一个点 */
} plane_point;

/**
 * @brief 屏幕坐标系点结构体
 */
typedef struct
{
    double x;      /**< 屏幕X坐标，像素单位 */
    double y;     /**< 屏幕Y坐标，像素单位 */
} screen_point;

/**
 * @brief 屏幕类型枚举
 * @details 支持多种屏幕显示类型
 */
typedef enum
{
    SCREEN_IPS114,              /**< IPS114屏幕 */
    SCREEN_IPS200_SPI,          /**< IPS200屏幕，SPI接口 */
    SCREEN_IPS200_PARALLEL8,    /**< IPS200屏幕，8位并行接口 */
    SCREEN_TFT180,              /**< TFT180屏幕 */
}screen_type_enum;

/** @brief GPS坐标数据数组，存储原始经纬度坐标 */
extern gps_point gps_point_data[DISPLAY_POINT_MAX];

/** @brief 平面坐标数据数组，存储转换后的平面坐标 */
extern plane_point plane_point_data[DISPLAY_POINT_MAX];

/** @brief 屏幕坐标数据数组，存储最终显示坐标 */
extern screen_point screen_point_data[DISPLAY_POINT_MAX];

/**
 * @brief 在IPS200屏幕上绘制圆形
 * @param x_center 圆心X坐标
 * @param y_center 圆心Y坐标
 * @param radius 圆半径
 * @param color 颜色值
 */
void ips200_INS_flash_draw_circle(uint16 x_center, uint16 y_center, uint16 radius, const uint16 color);

/**
 * @brief GPS坐标转换主函数
 * @param gps_point_input GPS坐标数组指针
 * @param point_num 坐标点数量
 * @details 执行经纬度到平面坐标再到屏幕坐标的完整转换流程
 */
void user_gps_transition(gps_point *gps_point_input, int16 point_num);

/**
 * @brief GPS坐标显示函数
 * @param color 显示颜色
 * @details 在屏幕上显示转换后的GPS坐标点
 */
void user_gps_display(uint16 color);

/**
 * @brief GPS坐标显示初始化
 * @param screen_type 屏幕类型
 * @param start_x 显示区域起始X坐标
 * @param start_y 显示区域起始Y坐标
 * @param width 显示区域宽度
 * @param high 显示区域高度
 * @details 初始化显示参数，设置显示区域
 */
void user_gps_display_init(screen_type_enum screen_type, int16 start_x, int16 start_y, int16 width, int16 high);

/**
 * @brief GPS坐标显示综合函数
 * @param gps_point_data GPS坐标数组
 * @param point_num 坐标点数量
 * @param x 显示区域X坐标
 * @param y 显示区域Y坐标
 * @param R 显示半径
 * @param line_color 连线颜色
 * @param point_color 点颜色
 * @details 一站式GPS坐标转换和显示函数
 */
void gps_display(gps_point *gps_point_data,int16 point_num,uint16 x,uint16 y,int16 R,const uint16 line_color,const uint16 point_color);

#endif
