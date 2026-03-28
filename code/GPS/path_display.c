/***
 * @file path_display.c
 * @brief 轨迹显示模块实现
 * @author JX116
 * @date 2026-03-21
 * @version 1.0
 *
 * @details 实现轨迹绘制、当前位置显示、状态显示与回放显示。
 ***/

#include "path_recorder.h"
#include "display_gps.h"
#include "zf_device_ips200.h"

// -----------------------------------------------------------------------------
// 显示参数
// -----------------------------------------------------------------------------

/** @brief 显示区域X坐标 */
#define DISPLAY_AREA_X          10
/** @brief 显示区域Y坐标 */
#define DISPLAY_AREA_Y          10
/** @brief 显示区域宽度 */
#define DISPLAY_AREA_WIDTH      300
/** @brief 显示区域高度 */
#define DISPLAY_AREA_HEIGHT     220

/** @brief 轨迹线颜色 */
#define PATH_LINE_COLOR         RGB565_BLUE
/** @brief 当前点颜色 */
#define CURRENT_POS_COLOR       RGB565_RED
/** @brief 背景颜色 */
#define BACKGROUND_COLOR        RGB565_BLACK
/** @brief 当前点半径 */
#define CURRENT_POS_RADIUS      3

static uint16 display_x = DISPLAY_AREA_X;
static uint16 display_y = DISPLAY_AREA_Y;
static uint16 display_width = DISPLAY_AREA_WIDTH;
static uint16 display_height = DISPLAY_AREA_HEIGHT;
static uint8 display_initialized = 0;

void path_display_init(void)
{
    display_x = DISPLAY_AREA_X;
    display_y = DISPLAY_AREA_Y;
    display_width = DISPLAY_AREA_WIDTH;
    display_height = DISPLAY_AREA_HEIGHT;
    display_initialized = 1;
}

void path_display_set_area(uint16 x, uint16 y, uint16 width, uint16 height)
{
    display_x = x;
    display_y = y;
    display_width = width;
    display_height = height;
    display_initialized = 1;
}

void path_display_get_area(uint16* x, uint16* y, uint16* width, uint16* height)
{
    if (x) *x = display_x;
    if (y) *y = display_y;
    if (width) *width = display_width;
    if (height) *height = display_height;
}

void path_display_trajectory(uint16 color)
{
    if (!display_initialized || path_data.point_count < 2)
    {
        return;
    }

    gps_point gps_points[DISPLAY_POINT_MAX];
    uint16 i;
    uint16 count = path_data.point_count;

    if (count > DISPLAY_POINT_MAX)
    {
        count = DISPLAY_POINT_MAX;
    }

    for (i = 0; i < count; i++)
    {
        gps_points[i].lat = (double)path_data.points[i].latitude;
        gps_points[i].lon = (double)path_data.points[i].longitude;
    }

    user_gps_transition(gps_points, count);

    for (i = 1; i < count; i++)
    {
        if (screen_point_data[i - 1].x > 0 && screen_point_data[i - 1].y > 0 &&
            screen_point_data[i].x > 0 && screen_point_data[i].y > 0)
        {
            ips200_draw_line(
                (uint16)(screen_point_data[i - 1].x + display_x),
                (uint16)(screen_point_data[i - 1].y + display_y),
                (uint16)(screen_point_data[i].x + display_x),
                (uint16)(screen_point_data[i].y + display_y),
                color);
        }
    }
}

void path_display_status(void)
{
    if (!display_initialized)
    {
        return;
    }

    uint16 y_offset = display_y + display_height + 10;

    // 状态字符串：屏幕显示英文，避免编码问题
    char status_str[32];
    switch (path_data.state)
    {
        case PATH_STATE_IDLE:
            strcpy(status_str, "State: IDLE");      // 空闲
            break;
        case PATH_STATE_RECORDING:
            strcpy(status_str, "State: REC");       // 记录中
            break;
        case PATH_STATE_PAUSED:
            strcpy(status_str, "State: PAUSE");     // 暂停
            break;
        case PATH_STATE_PLAYBACK:
            strcpy(status_str, "State: PLAY");      // 回放
            break;
        case PATH_STATE_COMPLETED:
            strcpy(status_str, "State: DONE");      // 完成
            break;
        default:
            strcpy(status_str, "State: UNK");       // 未知
            break;
    }

    ips200_show_string(display_x, y_offset, status_str);

    // 点数显示：当前记录点数 / 最大点数
    char point_str[32];
    sprintf(point_str, "Points: %d/%d", path_data.point_count, MAX_PATH_POINTS);
    ips200_show_string(display_x, y_offset + 20, point_str);

    // 统计信息：距离、时间、平均速度
    float total_distance;
    uint32 total_time;
    float avg_speed;
    path_recorder_get_stats(&total_distance, &total_time, &avg_speed);

    // Dist：累计轨迹距离（米）
    char distance_str[32];
    sprintf(distance_str, "Dist: %.1fm", total_distance);
    ips200_show_string(display_x + 120, y_offset, distance_str);

    // Time：累计记录时间（秒）
    char time_str[32];
    sprintf(time_str, "Time: %ds", (int)total_time);
    ips200_show_string(display_x + 120, y_offset + 20, time_str);

    // Avg：平均速度（km/h）
    char speed_str[32];
    sprintf(speed_str, "Avg: %.1fkm/h", avg_speed);
    ips200_show_string(display_x + 120, y_offset + 40, speed_str);
}

void path_display_current_position(void)
{
    if (!display_initialized || !gnss.state)
    {
        return;
    }

    gps_point current_gps;
    current_gps.lat = (double)gnss.latitude;
    current_gps.lon = (double)gnss.longitude;

    gps_point gps_points[2];
    gps_points[0] = current_gps;
    gps_points[1] = current_gps;

    user_gps_transition(gps_points, 2);

    if (screen_point_data[0].x > 0 && screen_point_data[0].y > 0)
    {
        // Lat/Lon：当前经纬度
        char pos_str[64];
        sprintf(pos_str, "Lat:%.6f Lon:%.6f", gnss.latitude, gnss.longitude);
        ips200_show_string(display_x, display_y - 20, pos_str);

        // SAT/SPD：卫星数 / 当前速度
        char sat_str[32];
        sprintf(sat_str, "SAT:%d SPD:%.1f", gnss.satellite_used, gnss.speed);
        ips200_show_string(display_x, display_y - 40, sat_str);
    }
}

void path_display_control_ui(void)
{
    if (!display_initialized)
    {
        return;
    }

    uint16 control_y = display_y + display_height + 60;

    // 控制提示：代码注释用中文，屏幕显示仍保留英文
    char control_str[64];
    switch (path_data.state)
    {
        case PATH_STATE_IDLE:
            strcpy(control_str, "A Start Rec");      // A 开始记录
            break;
        case PATH_STATE_RECORDING:
            strcpy(control_str, "B Pause C Stop");   // B 暂停，C 停止
            break;
        case PATH_STATE_PAUSED:
            strcpy(control_str, "A Resume C Stop");  // A 恢复，C 停止
            break;
        case PATH_STATE_COMPLETED:
            strcpy(control_str, "D Play E Clear");   // D 回放，E 清空
            break;
        case PATH_STATE_PLAYBACK:
            strcpy(control_str, "F Stop Play");      // F 停止回放
            break;
        default:
            strcpy(control_str, "");
            break;
    }

    ips200_show_string(display_x, control_y, control_str);
}

void path_display_update(void)
{
    if (!display_initialized)
    {
        return;
    }

    path_display_trajectory(PATH_LINE_COLOR);
    path_display_current_position();
    path_display_status();
    path_display_control_ui();
}

void path_display_playback_point(uint16 point_index)
{
    if (!display_initialized || point_index >= path_data.point_count)
    {
        return;
    }

    gps_point playback_gps;
    playback_gps.lat = (double)path_data.points[point_index].latitude;
    playback_gps.lon = (double)path_data.points[point_index].longitude;

    gps_point gps_points[2];
    gps_points[0] = playback_gps;
    gps_points[1] = playback_gps;

    user_gps_transition(gps_points, 2);

    if (screen_point_data[0].x > 0 && screen_point_data[0].y > 0)
    {
        // Play：当前回放到第几个点
        char playback_str[64];
        sprintf(playback_str, "Play: %d/%d", point_index + 1, path_data.point_count);
        ips200_show_string(display_x, display_y - 20, playback_str);

        // T：当前回放点时间（秒）
        char time_str[32];
        uint32 seconds = path_data.points[point_index].timestamp / 1000;
        sprintf(time_str, "T: %ds", (int)seconds);
        ips200_show_string(display_x, display_y - 40, time_str);
    }

    path_display_status();
}
