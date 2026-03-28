/**
 * @file path_display.h
 * @brief 路径显示模块头文件
 * @author 贾维斯
 * @date 2026-03-21
 * @version 1.0
 *
 * @details 本模块提供路径数据的显示功能，与现有display_gps模块集成
 */

#ifndef _PATH_DISPLAY_H_
#define _PATH_DISPLAY_H_

#include "zf_common_headfile.h"

//--------------------------------------------------------------------------------------------------
// 函数声明
//--------------------------------------------------------------------------------------------------

/**
 * @brief 初始化路径显示
 */
void path_display_init(void);

/**
 * @brief 显示路径轨迹
 * @param color 轨迹颜色
 */
void path_display_trajectory(uint16 color);

/**
 * @brief 显示当前路径状态
 */
void path_display_status(void);

/**
 * @brief 显示当前GPS位置
 */
void path_display_current_position(void);

/**
 * @brief 显示路径控制界面
 */
void path_display_control_ui(void);

/**
 * @brief 完整路径显示更新
 */
void path_display_update(void);

/**
 * @brief 显示路径回放
 * @param point_index 要显示的点索引
 */
void path_display_playback_point(uint16 point_index);

/**
 * @brief 设置显示区域
 * @param x X坐标
 * @param y Y坐标
 * @param width 宽度
 * @param height 高度
 */
void path_display_set_area(uint16 x, uint16 y, uint16 width, uint16 height);

/**
 * @brief 获取显示区域
 * @param[out] x X坐标
 * @param[out] y Y坐标
 * @param[out] width 宽度
 * @param[out] height 高度
 */
void path_display_get_area(uint16* x, uint16* y, uint16* width, uint16* height);

#endif /* _PATH_DISPLAY_H_ */
