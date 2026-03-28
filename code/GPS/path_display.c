/**
 * @file path_display.c
 * @brief Cleaned module header to avoid encoding issues.
 * @author JX116
 */

#include "path_recorder.h"
#include "display_gps.h"
#include "zf_device_ips200.h"

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

#define DISPLAY_AREA_X          10
#define DISPLAY_AREA_Y          10
#define DISPLAY_AREA_WIDTH      300
#define DISPLAY_AREA_HEIGHT     220

#define PATH_LINE_COLOR         RGB565_BLUE
#define PATH_POINT_COLOR        RGB565_RED
#define CURRENT_POS_COLOR       RGB565_GREEN
#define TEXT_COLOR              RGB565_WHITE
#define BACKGROUND_COLOR        RGB565_BLACK

#define POINT_RADIUS            3
#define CURRENT_POS_RADIUS      5

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

static uint8 display_initialized = 0;
static uint16 display_x = DISPLAY_AREA_X;
static uint16 display_y = DISPLAY_AREA_Y;
static uint16 display_width = DISPLAY_AREA_WIDTH;
static uint16 display_height = DISPLAY_AREA_HEIGHT;

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

/**
 */
void path_display_init(void)
{
    user_gps_display_init(SCREEN_IPS200_PARALLEL8, 
                         display_x, display_y, 
                         display_width, display_height);
    
    display_initialized = 1;
}

/**
 */
void path_display_trajectory(uint16 color)
{
    if (!display_initialized || path_data.point_count < 2)
    {
        return;
    }
    
    gps_point gps_points[MAX_PATH_POINTS];
    
    for (uint16 i = 0; i < path_data.point_count; i++)
    {
        gps_points[i].lat = path_data.points[i].latitude;
        gps_points[i].lon = path_data.points[i].longitude;
    }
    
    user_gps_transition(gps_points, path_data.point_count);
    user_gps_display(color);
}

/**
 */
void path_display_status(void)
{
    if (!display_initialized)
    {
        return;
    }
    
//    ips200_fill_rect(display_x, display_y + display_height + 5,
//                    display_width, 50, BACKGROUND_COLOR);
    
    uint16 y_offset = display_y + display_height + 10;
    
    char status_str[32];
    switch (path_data.state)
    {
        case PATH_STATE_IDLE:
            strcpy(status_str, "״̬: ����");
            break;
        case PATH_STATE_RECORDING:
            strcpy(status_str, "״̬: ��¼��");
            break;
        case PATH_STATE_PAUSED:
            strcpy(status_str, "״̬: ��ͣ");
            break;
        case PATH_STATE_PLAYBACK:
            strcpy(status_str, "״̬: �ط�");
            break;
        case PATH_STATE_COMPLETED:
            strcpy(status_str, "״̬: ���?);
            break;
        default:
            strcpy(status_str, "״̬: δ֪");
            break;
    }
    
    ips200_show_string(display_x, y_offset, status_str);
    
    char point_str[32];
    sprintf(point_str, "����: %d/%d", path_data.point_count, MAX_PATH_POINTS);
    ips200_show_string(display_x, y_offset + 20, point_str);
    
    float total_distance;
    uint32 total_time;
    float avg_speed;
    
    path_recorder_get_stats(&total_distance, &total_time, &avg_speed);
    
    char distance_str[32];
    sprintf(distance_str, "����: %.1fm", total_distance);
    ips200_show_string(display_x + 120, y_offset, distance_str);
    
    char time_str[32];
    sprintf(time_str, "ʱ��: %ds", (int)total_time);
    ips200_show_string(display_x + 120, y_offset + 20, time_str);
    
    char speed_str[32];
    sprintf(speed_str, "����: %.1fkm/h", avg_speed);
    ips200_show_string(display_x + 120, y_offset + 40, speed_str);
}

/**
 */
void path_display_current_position(void)
{
    if (!display_initialized || !gnss.state)
    {
        return;
    }
    
    gps_point current_gps;
    current_gps.lat = (double)gnss.latitude;  // display_gpsʹ��double����
    current_gps.lon = (double)gnss.longitude;
    
    gps_point gps_points[2];
    gps_points[0] = current_gps;
    gps_points[1] = current_gps;  // ��Ҫ����2����
    
    user_gps_transition(gps_points, 2);
    
    if (screen_point_data[0].x > 0 && screen_point_data[0].y > 0)
    {
        // uint16 x = (uint16)(screen_point_data[0].x + display_x);
        // uint16 y = (uint16)(screen_point_data[0].y + display_y);
        
//        ips200_INS_flash_draw_circle(x, y, CURRENT_POS_RADIUS, CURRENT_POS_COLOR);
        
        char pos_str[64];
        sprintf(pos_str, "Lat:%.6f Lon:%.6f", gnss.latitude, gnss.longitude);
        ips200_show_string(display_x, display_y - 20, pos_str);
        
        char sat_str[32];
        sprintf(sat_str, "SAT:%d SPD:%.1f", gnss.satellite_used, gnss.speed);
        ips200_show_string(display_x, display_y - 40, sat_str);
    }
}

/**
 */
void path_display_control_ui(void)
{
    if (!display_initialized)
    {
        return;
    }
    
    uint16 control_y = display_y + display_height + 60;
//    ips200_fill_rect(display_x, control_y, display_width, 40, BACKGROUND_COLOR);
    
    char control_str[64];
    
    switch (path_data.state)
    {
        case PATH_STATE_IDLE:
            strcpy(control_str, "��A��ʼ��¼");
            break;
        case PATH_STATE_RECORDING:
            strcpy(control_str, "��B��ͣ ��Cֹͣ");
            break;
        case PATH_STATE_PAUSED:
            strcpy(control_str, "��A�ָ� ��Cֹͣ");
            break;
        case PATH_STATE_COMPLETED:
            strcpy(control_str, "��D�ط� ��E���?);
            break;
        case PATH_STATE_PLAYBACK:
            strcpy(control_str, "��Fֹͣ�ط�");
            break;
        default:
            strcpy(control_str, "");
            break;
    }
    
    ips200_show_string(display_x, control_y, control_str);
}

/**
 */
void path_display_update(void)
{
    if (!display_initialized)
    {
        return;
    }
    
//    ips200_fill_rect(display_x, display_y, display_width, display_height, BACKGROUND_COLOR);
    
    path_display_trajectory(PATH_LINE_COLOR);
    
    path_display_current_position();
    
    path_display_status();
    
    path_display_control_ui();
}

/**
 */
void path_display_playback_point(uint16 point_index)
{
    if (!display_initialized || point_index >= path_data.point_count)
    {
        return;
    }
    
//    ips200_fill_rect(display_x, display_y, display_width, display_height, BACKGROUND_COLOR);
    
    path_display_trajectory(PATH_LINE_COLOR);
    
    gps_point playback_gps;
    playback_gps.lat = path_data.points[point_index].latitude;
    playback_gps.lon = path_data.points[point_index].longitude;
    
    gps_point gps_points[2];
    gps_points[0] = playback_gps;
    gps_points[1] = playback_gps;
    
    user_gps_transition(gps_points, 2);
    
    if (screen_point_data[0].x > 0 && screen_point_data[0].y > 0)
    {
        // uint16 x = (uint16)(screen_point_data[0].x + display_x);
        // uint16 y = (uint16)(screen_point_data[0].y + display_y);
        
//        ips200_INS_flash_draw_circle(x, y, CURRENT_POS_RADIUS, RGB565_YELLOW);
        
        char playback_str[64];
        sprintf(playback_str, "�ط�: %d/%d", point_index + 1, path_data.point_count);
        ips200_show_string(display_x, display_y - 20, playback_str);
        
        char time_str[32];
        uint32 seconds = path_data.points[point_index].timestamp / 1000;
        sprintf(time_str, "ʱ��: %ds", (int)seconds);
        ips200_show_string(display_x, display_y - 40, time_str);
    }
    
    path_display_status();
}

/**
 */
void path_display_set_area(uint16 x, uint16 y, uint16 width, uint16 height)
{
    display_x = x;
    display_y = y;
    display_width = width;
    display_height = height;
    
    user_gps_display_init(SCREEN_IPS200_PARALLEL8, x, y, width, height);
}

/**
 */
void path_display_get_area(uint16* x, uint16* y, uint16* width, uint16* height)
{
    if (x != NULL) *x = display_x;
    if (y != NULL) *y = display_y;
    if (width != NULL) *width = display_width;
    if (height != NULL) *height = display_height;
}
