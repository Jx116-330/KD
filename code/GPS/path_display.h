/***
 * @file path_display.h
 * @brief 轨迹显示模块头文件
 * @author JX116
 * @date 2026-03-21
 * @version 1.0
 *
 * @details 提供轨迹、当前位置与状态信息的显示接口。
 ***/

#ifndef _PATH_DISPLAY_H_
#define _PATH_DISPLAY_H_

#include "zf_common_headfile.h"

void path_display_init(void);
void path_display_trajectory(uint16 color);
void path_display_status(void);
void path_display_current_position(void);
void path_display_control_ui(void);
void path_display_update(void);
void path_display_playback_point(uint16 point_index);
void path_display_set_area(uint16 x, uint16 y, uint16 width, uint16 height);
void path_display_get_area(uint16* x, uint16* y, uint16* width, uint16* height);

#endif
