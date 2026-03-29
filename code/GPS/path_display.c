/**
 * @file path_display.c
 * @brief GPS 轨迹显示模块实现
 * @author JX116
 *
 * 这里是 GPS 界面的上层显示逻辑，主要负责：
 * 1. 把记录点绘制成轨迹线
 * 2. 在同一坐标系上叠加当前位置或回放点
 * 3. 显示状态、统计信息和控制提示
 */

#include "path_recorder.h"
#include "display_gps.h"
#include "zf_device_ips200.h"

/** 默认显示区域左上角 X */
#define DISPLAY_AREA_X          10
/** 默认显示区域左上角 Y */
#define DISPLAY_AREA_Y          10
/** 默认显示区域宽度 */
#define DISPLAY_AREA_WIDTH      300
/** 默认显示区域高度 */
#define DISPLAY_AREA_HEIGHT     220

/** 轨迹线颜色 */
#define PATH_LINE_COLOR         RGB565_BLUE
/** 当前点或回放点颜色 */
#define CURRENT_POS_COLOR       RGB565_RED
/** 地图显示边框颜色 */
#define MAP_BORDER_COLOR        RGB565_WHITE

/**
 * @brief 获取当前工程默认字体的字符宽度
 *
 * 当前菜单初始化时把屏幕字体设置成 IPS200_DEFAULT_DISPLAY_FONT，
 * 这里用同一配置计算字符串截断宽度，避免直接写死为 8。
 */
static uint16 path_display_get_font_width(void)
{
#if (IPS200_DEFAULT_DISPLAY_FONT == IPS200_6X8_FONT)
    return 6U;
#elif (IPS200_DEFAULT_DISPLAY_FONT == IPS200_16X16_FONT)
    return 16U;
#else
    return 8U;
#endif
}

/** 当前实际显示区域 X */
static uint16 display_x = DISPLAY_AREA_X;
/** 当前实际显示区域 Y */
static uint16 display_y = DISPLAY_AREA_Y;
/** 当前实际显示区域宽度 */
static uint16 display_width = DISPLAY_AREA_WIDTH;
/** 当前实际显示区域高度 */
static uint16 display_height = DISPLAY_AREA_HEIGHT;
/** 显示模块是否已初始化 */
static uint8 display_initialized = 0;
/** 地图边框是否需要重画 */
static uint8 display_border_dirty = 1;

/**
 * @brief 将显示区域裁剪到屏幕范围内
 */
static void path_display_clamp_area(void)
{
    if (0U == ips200_width_max || 0U == ips200_height_max)
    {
        return;
    }

    if (display_x >= ips200_width_max)
    {
        display_x = ips200_width_max - 1U;
    }
    if (display_y >= ips200_height_max)
    {
        display_y = ips200_height_max - 1U;
    }

    if (0U == display_width)
    {
        display_width = 1U;
    }
    if (0U == display_height)
    {
        display_height = 1U;
    }

    if ((uint32)display_x + (uint32)display_width > (uint32)ips200_width_max)
    {
        display_width = ips200_width_max - display_x;
    }
    if ((uint32)display_y + (uint32)display_height > (uint32)ips200_height_max)
    {
        display_height = ips200_height_max - display_y;
    }

    if (0U == display_width)
    {
        display_width = 1U;
    }
    if (0U == display_height)
    {
        display_height = 1U;
    }
}

/**
 * @brief 安全清空指定矩形区域
 *
 * 这个函数会先将坐标裁剪到屏幕范围内，避免底层显示函数断言。
 */
static void path_display_fill_rect_safe(int32 x_start, int32 y_start, int32 x_end, int32 y_end, uint16 color)
{
    int32 y;

    if (0U == ips200_width_max || 0U == ips200_height_max)
    {
        return;
    }

    if (x_start > x_end)
    {
        int32 temp = x_start;
        x_start = x_end;
        x_end = temp;
    }
    if (y_start > y_end)
    {
        int32 temp = y_start;
        y_start = y_end;
        y_end = temp;
    }

    if (x_end < 0 || y_end < 0)
    {
        return;
    }
    if (x_start >= (int32)ips200_width_max || y_start >= (int32)ips200_height_max)
    {
        return;
    }

    if (x_start < 0) x_start = 0;
    if (y_start < 0) y_start = 0;
    if (x_end >= (int32)ips200_width_max) x_end = (int32)ips200_width_max - 1;
    if (y_end >= (int32)ips200_height_max) y_end = (int32)ips200_height_max - 1;

    for (y = y_start; y <= y_end; y++)
    {
        ips200_draw_line((uint16)x_start, (uint16)y, (uint16)x_end, (uint16)y, color);
    }
}

/**
 * @brief 清空轨迹显示相关的所有区域
 *
 * 包括地图本体、顶部两行文本和底部状态/控制文本区域。
 */
static void path_display_clear_layout(void)
{
    int32 x_start = (int32)display_x;
    int32 x_end = (int32)display_x + (int32)display_width - 1;
    int32 y_start = (int32)display_y - 40;
    int32 y_end = (int32)display_y + (int32)display_height + 80;

    path_display_fill_rect_safe(x_start, y_start, x_end, y_end, RGB565_BLACK);
}

/**
 * @brief 在屏幕范围内安全显示字符串
 *
 * 如果起始坐标越界则直接跳过；如果字符串过长则按剩余宽度截断。
 */
static void path_display_show_string_fit(uint16 x, int32 y, const char *text)
{
    char buffer[64];
    int32 max_chars;
    int32 right_limit;
    uint16 font_width;
    uint16 i = 0U;

    if (NULL == text)
    {
        return;
    }
    if (y < 0 || y >= (int32)ips200_height_max)
    {
        return;
    }
    if (x >= ips200_width_max)
    {
        return;
    }

    font_width = path_display_get_font_width();
    if (0U == font_width)
    {
        return;
    }

    right_limit = (int32)display_x + (int32)display_width;
    if (right_limit > (int32)ips200_width_max)
    {
        right_limit = (int32)ips200_width_max;
    }

    max_chars = (right_limit - (int32)x) / (int32)font_width;
    if (max_chars <= 0)
    {
        return;
    }

    while (text[i] != '\0' && i < (uint16)max_chars && i < (sizeof(buffer) - 1U))
    {
        buffer[i] = text[i];
        i++;
    }

    while (i < (uint16)max_chars && i < (sizeof(buffer) - 1U))
    {
        buffer[i] = ' ';
        i++;
    }

    buffer[i] = '\0';

    ips200_show_string(x, (uint16)y, buffer);
}

/**
 * @brief 绘制地图显示区域边框
 *
 * 边框用于明确地图可视范围，即使当前没有轨迹点，
 * 用户也能直观看到地图区域的位置和大小。
 */
static void path_display_draw_border(void)
{
    uint16 x_end;
    uint16 y_end;

    if (!display_initialized || display_width < 2U || display_height < 2U)
    {
        return;
    }

    x_end = display_x + display_width - 1U;
    y_end = display_y + display_height - 1U;

    ips200_draw_line(display_x, display_y, x_end, display_y, MAP_BORDER_COLOR);
    ips200_draw_line(display_x, y_end, x_end, y_end, MAP_BORDER_COLOR);
    ips200_draw_line(display_x, display_y, display_x, y_end, MAP_BORDER_COLOR);
    ips200_draw_line(x_end, display_y, x_end, y_end, MAP_BORDER_COLOR);
}

/**
 * @brief 从完整轨迹中抽取用于显示的点
 *
 * 当轨迹点数超过 DISPLAY_POINT_MAX 时，不再简单截取前面的点，
 * 而是按整条轨迹均匀采样，这样自动缩放会基于整条路径而不是局部前段。
 *
 * @param gps_points 输出的 GPS 点数组
 * @return 实际写入的点数
 */
static uint16 path_display_collect_points(gps_point *gps_points)
{
    uint16 total_count;
    uint16 sample_count;
    uint16 i;

    total_count = path_data.point_count;
    if (0U == total_count)
    {
        return 0U;
    }

    sample_count = total_count;
    if (sample_count > DISPLAY_POINT_MAX)
    {
        sample_count = DISPLAY_POINT_MAX;
    }

    if (sample_count == total_count)
    {
        for (i = 0; i < sample_count; i++)
        {
            gps_points[i].lat = (double)path_data.points[i].latitude;
            gps_points[i].lon = (double)path_data.points[i].longitude;
        }
    }
    else if (sample_count <= 1U)
    {
        gps_points[0].lat = (double)path_data.points[0].latitude;
        gps_points[0].lon = (double)path_data.points[0].longitude;
        sample_count = 1U;
    }
    else
    {
        uint32 total_span = (uint32)(total_count - 1U);
        uint32 sample_span = (uint32)(sample_count - 1U);

        for (i = 0; i < sample_count; i++)
        {
            uint32 index = (i * total_span + (sample_span / 2U)) / sample_span;
            gps_points[i].lat = (double)path_data.points[index].latitude;
            gps_points[i].lon = (double)path_data.points[index].longitude;
        }
    }

    return sample_count;
}

/**
 * @brief 计算完整轨迹的局部坐标包围盒
 *
 * 自动缩放应当基于全轨迹边界，而不是只基于绘制采样点，
 * 否则极值点刚好未被采样到时，会出现缩放范围偏小的问题。
 */
static uint8 path_display_collect_bounds(double *min_x, double *max_x, double *min_y, double *max_y)
{
    double ref_lat;
    double ref_lon;
    uint16 i;

    if (NULL == min_x || NULL == max_x || NULL == min_y || NULL == max_y || path_data.point_count == 0U)
    {
        return 0;
    }

    ref_lat = (double)path_data.points[0].latitude * USER_PI / 180.0;
    ref_lon = (double)path_data.points[0].longitude * USER_PI / 180.0;

    for (i = 0; i < path_data.point_count; i++)
    {
        double lat = (double)path_data.points[i].latitude * USER_PI / 180.0;
        double lon = (double)path_data.points[i].longitude * USER_PI / 180.0;
        double x = EARTH_RADIUS * (lon - ref_lon) * cos(ref_lat);
        double y = EARTH_RADIUS * (lat - ref_lat);

        if (0U == i)
        {
            *min_x = x;
            *max_x = x;
            *min_y = y;
            *max_y = y;
        }
        else
        {
            if (x < *min_x) *min_x = x;
            if (x > *max_x) *max_x = x;
            if (y < *min_y) *min_y = y;
            if (y > *max_y) *max_y = y;
        }
    }

    return 1;
}

/**
 * @brief 判断一个屏幕点是否在当前显示区域内部
 */
static uint8 path_display_point_in_area(const screen_point *point)
{
    if (NULL == point)
    {
        return 0;
    }

    return (point->x >= 0 && point->y >= 0 &&
            point->x < (int16)display_width &&
            point->y < (int16)display_height);
}

/**
 * @brief 用十字形标记绘制当前位置或回放点
 *
 * 这里只画一个很小的十字，目的是在不遮挡轨迹线的前提下
 * 让当前位置足够醒目。
 */
static void path_display_draw_marker(const screen_point *point, uint16 color)
{
    static const int8 x_offsets[5] = {0, -1, 1, 0, 0};
    static const int8 y_offsets[5] = {0, 0, 0, -1, 1};
    uint16 i;

    for (i = 0; i < 5; i++)
    {
        screen_point draw_point = {
            (int16)(point->x + x_offsets[i]),
            (int16)(point->y + y_offsets[i])
        };

        if (path_display_point_in_area(&draw_point))
        {
            ips200_draw_point((uint16)(display_x + draw_point.x),
                              (uint16)(display_y + draw_point.y),
                              color);
        }
    }
}

void path_display_init(void)
{
    display_x = DISPLAY_AREA_X;
    display_y = DISPLAY_AREA_Y;
    display_width = DISPLAY_AREA_WIDTH;
    display_height = DISPLAY_AREA_HEIGHT;
    path_display_clamp_area();
    display_border_dirty = 1;
    display_initialized = 1;
}

void path_display_set_area(uint16 x, uint16 y, uint16 width, uint16 height)
{
    if (display_x != x || display_y != y || display_width != width || display_height != height)
    {
        display_border_dirty = 1;
    }
    display_x = x;
    display_y = y;
    display_width = width;
    display_height = height;
    path_display_clamp_area();
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
    gps_point gps_points[DISPLAY_POINT_MAX];
    double min_x;
    double max_x;
    double min_y;
    double max_y;
    uint16 i;
    uint16 count;

    if (!display_initialized)
    {
        return;
    }

    if (display_border_dirty)
    {
        path_display_draw_border();
        display_border_dirty = 0;
    }

    if (path_data.point_count == 0)
    {
        return;
    }

    /* 使用整条轨迹均匀采样后的点参与显示，保证自动缩放基于全路径。 */
    count = path_display_collect_points(gps_points);
    if (0U == count)
    {
        return;
    }

    /* 先告诉转换模块当前地图实际使用的显示范围。 */
    gps_set_display_area(0, 0, (int16)display_width, (int16)display_height);
    if (path_display_collect_bounds(&min_x, &max_x, &min_y, &max_y))
    {
        gps_set_xy_bounds(min_x, max_x, min_y, max_y);
    }
    user_gps_transition(gps_points, count);
    gps_clear_xy_bounds();

    if (count < 2)
    {
        return;
    }

    /* 使用转换后的缓存点逐段连线。 */
    for (i = 1; i < count; i++)
    {
        if (path_display_point_in_area(&screen_point_data[i - 1]) &&
            path_display_point_in_area(&screen_point_data[i]))
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
    uint16 y_offset;
    char status_str[32];
    char point_str[32];
    char distance_str[32];
    char time_str[32];
    char speed_str[32];
    float total_distance;
    uint32 total_time;
    float avg_speed;

    if (!display_initialized)
    {
        return;
    }

    y_offset = display_y + display_height + 10;

    switch (path_data.state)
    {
        case PATH_STATE_IDLE:
            strcpy(status_str, "State: IDLE");
            break;
        case PATH_STATE_RECORDING:
            strcpy(status_str, "State: REC");
            break;
        case PATH_STATE_PAUSED:
            strcpy(status_str, "State: PAUSE");
            break;
        case PATH_STATE_PLAYBACK:
            strcpy(status_str, "State: PLAY");
            break;
        case PATH_STATE_COMPLETED:
            strcpy(status_str, "State: DONE");
            break;
        default:
            strcpy(status_str, "State: UNK");
            break;
    }

    path_display_show_string_fit(display_x, (int32)y_offset, status_str);

    sprintf(point_str, "Points: %d/%d", path_data.point_count, MAX_PATH_POINTS);
    path_display_show_string_fit(display_x, (int32)y_offset + 20, point_str);

    path_recorder_get_stats(&total_distance, &total_time, &avg_speed);

    sprintf(distance_str, "Dist: %.1fm", total_distance);
    path_display_show_string_fit((uint16)(display_x + 120U), (int32)y_offset, distance_str);

    /* total_time 内部单位是毫秒，这里转成秒后再显示。 */
    sprintf(time_str, "Time: %lus", (unsigned long)(total_time / 1000U));
    path_display_show_string_fit((uint16)(display_x + 120U), (int32)y_offset + 20, time_str);

    sprintf(speed_str, "Avg: %.1fkm/h", avg_speed);
    path_display_show_string_fit((uint16)(display_x + 120U), (int32)y_offset + 40, speed_str);
}

void path_display_current_position(void)
{
    gps_point current_gps;
    screen_point current_point;
    uint8 point_ready = 0;
    char pos_str[64];
    char sat_str[32];

    if (!display_initialized || !gnss.state)
    {
        return;
    }

    current_gps.lat = (double)gnss.latitude;
    current_gps.lon = (double)gnss.longitude;

    if (path_data.point_count > 0)
    {
        /* 有轨迹时，直接复用轨迹线刚刚建立的映射关系。 */
        point_ready = gps_point_to_screen(&current_gps, &current_point);
    }
    else
    {
        /*
         * 没有轨迹时也允许显示当前位置。
         * 这里人为建立一个仅包含当前点的映射，保证界面上至少有一个点可画。
         */
        gps_set_display_area(0, 0, (int16)display_width, (int16)display_height);
        gps_clear_xy_bounds();
        user_gps_transition(&current_gps, 1);
        current_point = screen_point_data[0];
        point_ready = 1;
    }

    if (point_ready && path_display_point_in_area(&current_point))
    {
        path_display_draw_marker(&current_point, CURRENT_POS_COLOR);
    }

    sprintf(pos_str, "Lat:%.6f Lon:%.6f", gnss.latitude, gnss.longitude);
    path_display_show_string_fit(display_x, (int32)display_y - 20, pos_str);

    sprintf(sat_str, "SAT:%d SPD:%.1f", gnss.satellite_used, gnss.speed);
    path_display_show_string_fit(display_x, (int32)display_y - 40, sat_str);
}

void path_display_control_ui(void)
{
    uint16 control_y;
    char control_str[64];

    if (!display_initialized)
    {
        return;
    }

    control_y = display_y + display_height + 60;

    switch (path_data.state)
    {
        case PATH_STATE_IDLE:
            strcpy(control_str, "A Start Rec");
            break;
        case PATH_STATE_RECORDING:
            strcpy(control_str, "B Pause C Stop");
            break;
        case PATH_STATE_PAUSED:
            strcpy(control_str, "A Resume C Stop");
            break;
        case PATH_STATE_COMPLETED:
            strcpy(control_str, "D Play E Clear");
            break;
        case PATH_STATE_PLAYBACK:
            strcpy(control_str, "F Stop Play");
            break;
        default:
            strcpy(control_str, "");
            break;
    }

    path_display_show_string_fit(display_x, (int32)control_y, control_str);
}

void path_display_update(void)
{
    if (!display_initialized)
    {
        return;
    }

    path_display_clear_layout();

    /* 先画轨迹，再叠加当前位置，最后补文字信息。 */
    path_display_trajectory(PATH_LINE_COLOR);
    path_display_current_position();
    path_display_status();
    path_display_control_ui();
}

void path_display_playback_point(uint16 point_index)
{
    gps_point playback_gps;
    screen_point playback_point;
    char playback_str[64];
    char time_str[32];
    uint32 seconds;

    if (!display_initialized || point_index >= path_data.point_count)
    {
        return;
    }

    path_display_clear_layout();

    /*
     * 回放点和当前位置一样，也必须建立在轨迹线已经生成的映射上，
     * 所以这里先重画轨迹，确保坐标缓存是当前轨迹对应的那一套。
     */
    path_display_trajectory(PATH_LINE_COLOR);

    playback_gps.lat = (double)path_data.points[point_index].latitude;
    playback_gps.lon = (double)path_data.points[point_index].longitude;

    if (gps_point_to_screen(&playback_gps, &playback_point) &&
        path_display_point_in_area(&playback_point))
    {
        path_display_draw_marker(&playback_point, CURRENT_POS_COLOR);
    }

    sprintf(playback_str, "Play: %d/%d", point_index + 1, path_data.point_count);
    path_display_show_string_fit(display_x, (int32)display_y - 20, playback_str);

    seconds = path_data.points[point_index].timestamp / 1000U;
    sprintf(time_str, "T: %lus", (unsigned long)seconds);
    path_display_show_string_fit(display_x, (int32)display_y - 40, time_str);

    path_display_status();
}
