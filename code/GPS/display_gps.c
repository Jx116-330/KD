/**
 * @file display_gps.c
 * @brief GPS坐标转换与显示模块实现
 * @author JX116
 * @date 2026-03-21
 * @version 1.0
 *
 * @details 实现：
 *          - 经纬度到本地XY坐标转换
 *          - XY坐标到屏幕坐标映射
 *          - 轨迹点与当前位置绘制支持
 */

#include <GPS/display_gps.h>

/** @brief 显示区域起始X坐标 */
static int16 display_start_x = 0;
/** @brief 显示区域起始Y坐标 */
static int16 display_start_y = 0;
/** @brief 显示区域宽度 */
static int16 display_width = 0;
/** @brief 显示区域高度 */
static int16 display_high = 0;
/** @brief 当前转换后的点数量 */
static int16 transition_point_num = 0;

screen_point screen_point_data[DISPLAY_POINT_MAX];

void gps_to_xy(gps_point *gps, double *x, double *y)
{
    int i;
    double ref_lat = gps[0].lat * USER_PI / 180.0;
    double ref_lon = gps[0].lon * USER_PI / 180.0;

    for (i = 0; i < transition_point_num; i++)
    {
        double lat = gps[i].lat * USER_PI / 180.0;
        double lon = gps[i].lon * USER_PI / 180.0;

        x[i] = EARTH_RADIUS * (lon - ref_lon) * cos(ref_lat);
        y[i] = EARTH_RADIUS * (lat - ref_lat);
    }
}

void xy_to_screen(double *x, double *y, int point_num)
{
    int i;
    double min_x = x[0], max_x = x[0];
    double min_y = y[0], max_y = y[0];
    double scale_x, scale_y, scale;

    for (i = 1; i < point_num; i++)
    {
        if (x[i] < min_x) min_x = x[i];
        if (x[i] > max_x) max_x = x[i];
        if (y[i] < min_y) min_y = y[i];
        if (y[i] > max_y) max_y = y[i];
    }

    if (fabs(max_x - min_x) < EPSILON) max_x = min_x + 1.0;
    if (fabs(max_y - min_y) < EPSILON) max_y = min_y + 1.0;

    scale_x = (display_width - 10.0) / (max_x - min_x);
    scale_y = (display_high - 10.0) / (max_y - min_y);
    scale = (scale_x < scale_y) ? scale_x : scale_y;

    for (i = 0; i < point_num; i++)
    {
        double screen_x = (x[i] - min_x) * scale + display_start_x + 5;
        double screen_y = display_start_y + display_high - 5 - ((y[i] - min_y) * scale);

        if (screen_x > 32767.0) screen_x = 32767.0;
        if (screen_x < -32768.0) screen_x = -32768.0;
        if (screen_y > 32767.0) screen_y = 32767.0;
        if (screen_y < -32768.0) screen_y = -32768.0;

        screen_point_data[i].x = (int16)screen_x;
        screen_point_data[i].y = (int16)screen_y;
    }
}

void get_transition(gps_point *gps, int point_num)
{
    double x[DISPLAY_POINT_MAX];
    double y[DISPLAY_POINT_MAX];

    if (point_num <= 0) return;
    if (point_num > DISPLAY_POINT_MAX) point_num = DISPLAY_POINT_MAX;
    if (point_num > 32767) point_num = 32767;

    transition_point_num = (int16)point_num;
    gps_to_xy(gps, x, y);
    xy_to_screen(x, y, point_num);
}

void user_gps_transition(gps_point *gps, int point_num)
{
    display_start_x = 0;
    display_start_y = 0;
    display_width = 220;
    display_high = 120;
    get_transition(gps, point_num);
}

void screen_print_gps_point(void)
{
    int i;
    for (i = 0; i < transition_point_num; i++)
    {
        if (screen_point_data[i].x >= 0 && screen_point_data[i].y >= 0)
        {
            ips200_draw_point((uint16)screen_point_data[i].x, (uint16)screen_point_data[i].y, RGB565_RED);
        }
    }
}