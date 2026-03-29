/**
 * @file path_display.h
 * @brief GPS 轨迹显示模块接口
 * @author JX116
 *
 * 这个模块负责把 path_recorder 中记录下来的轨迹数据
 * 以“线 + 点 + 状态文本”的形式画到屏幕上。
 */

#ifndef _PATH_DISPLAY_H_
#define _PATH_DISPLAY_H_

#include "zf_common_headfile.h"

/** 初始化显示区域参数 */
void path_display_init(void);

/** 绘制当前轨迹线 */
void path_display_trajectory(uint16 color);

/** 绘制状态、点数、距离、时间、平均速度等文本信息 */
void path_display_status(void);

/** 绘制当前 GPS 位置及其文本信息 */
void path_display_current_position(void);

/** 绘制简单的控制提示信息 */
void path_display_control_ui(void);

/** 按标准顺序刷新整块轨迹显示区域 */
void path_display_update(void);

/** 绘制指定回放点的位置和状态信息 */
void path_display_playback_point(uint16 point_index);

/** 设置轨迹显示区域 */
void path_display_set_area(uint16 x, uint16 y, uint16 width, uint16 height);

/** 读取当前轨迹显示区域 */
void path_display_get_area(uint16* x, uint16* y, uint16* width, uint16* height);

#endif
