/**
 * @file path_recorder.c
 * @brief Cleaned module header to avoid encoding issues.
 * @author JX116
 */

#include "path_recorder.h"
#include <string.h>  // ����memset�Ⱥ���֧��
#include <math.h>    // ��ѧ������Ƕ��ʽϵͳ�п�����Ҫ����֧��

#define get_current_time_ms()   system_getval_ms()

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

path_data_t path_data;

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

static uint8 is_valid_gps_fix(void);
static void update_path_statistics(path_point_t* new_point);

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

/**
 */
void path_recorder_init(void)
{
    memset(&path_data, 0, sizeof(path_data_t));
    
    path_data.state = PATH_STATE_IDLE;
    path_data.point_count = 0;
    path_data.current_index = 0;
    path_data.last_record_time = 0;
    path_data.last_latitude = 0.0;
    path_data.last_longitude = 0.0;
    path_data.total_distance = 0.0f;
    path_data.total_time = 0;
    
    for (uint16 i = 0; i < MAX_PATH_POINTS; i++)
    {
        path_data.points[i].latitude = 0.0;
        path_data.points[i].longitude = 0.0;
        path_data.points[i].timestamp = 0;
        path_data.points[i].speed = 0.0f;
        path_data.points[i].direction = 0.0f;
        path_data.points[i].satellite_count = 0;
        path_data.points[i].fix_quality = 0;
    }
    
}

/**
 */
static uint8 is_valid_gps_fix(void)
{
    if (gnss.state != 1)  // ��λ��Ч
    {
        return 0;
    }
    
    if (gnss.satellite_used < 4)  // ������Ҫ4������
    {
        return 0;
    }
    
    if (gnss.latitude == 0.0 || gnss.longitude == 0.0)
    {
        return 0;
    }
    
    if (gnss.latitude < 3.0 || gnss.latitude > 54.0 ||  // �й�γ�ȷ�Χ
        gnss.longitude < 73.0 || gnss.longitude > 136.0) // �й����ȷ�Χ
    {
        return 0;  // ����������Χ
    }
    
    return 1;
}

/**
 */
uint8 path_recorder_start(void)
{
    if (path_data.state != PATH_STATE_IDLE && 
        path_data.state != PATH_STATE_PAUSED)
    {
        return 0;  // ״̬����ȷ
    }
    
    if (!is_valid_gps_fix())
    {
        return 0;  // GPS��Ч
    }
    
    if (path_data.state == PATH_STATE_IDLE)
    {
        path_recorder_clear();
        
        path_point_t first_point;
        first_point.latitude = (float)gnss.latitude;
        first_point.longitude = (float)gnss.longitude;
        first_point.timestamp = get_current_time_ms();
        first_point.speed = gnss.speed;
        first_point.direction = gnss.direction;
        first_point.satellite_count = gnss.satellite_used;
        first_point.fix_quality = gnss.state;
        
        if (path_recorder_add_point(&first_point))
        {
            path_data.last_latitude = (float)gnss.latitude;
            path_data.last_longitude = (float)gnss.longitude;
            path_data.last_record_time = first_point.timestamp;
            path_data.state = PATH_STATE_RECORDING;
            return 1;
        }
    }
    else if (path_data.state == PATH_STATE_PAUSED)
    {
        path_data.state = PATH_STATE_RECORDING;
        return 1;
    }
    
    return 0;
}

/**
 */
void path_recorder_pause(void)
{
    if (path_data.state == PATH_STATE_RECORDING)
    {
        path_data.state = PATH_STATE_PAUSED;
    }
}

/**
 */
void path_recorder_resume(void)
{
    if (path_data.state == PATH_STATE_PAUSED)
    {
        path_data.state = PATH_STATE_RECORDING;
    }
}

/**
 */
void path_recorder_stop(void)
{
    if (path_data.state == PATH_STATE_RECORDING || 
        path_data.state == PATH_STATE_PAUSED)
    {
        path_data.state = PATH_STATE_COMPLETED;
    }
}

/**
 */
void path_recorder_clear(void)
{
    path_data.point_count = 0;
    path_data.current_index = 0;
    path_data.last_record_time = 0;
    path_data.last_latitude = 0.0;
    path_data.last_longitude = 0.0;
    path_data.total_distance = 0.0f;
    path_data.total_time = 0;
    
    for (uint16 i = 0; i < MAX_PATH_POINTS; i++)
    {
        path_data.points[i].latitude = 0.0;
        path_data.points[i].longitude = 0.0;
        path_data.points[i].timestamp = 0;
        path_data.points[i].speed = 0.0f;
        path_data.points[i].direction = 0.0f;
        path_data.points[i].satellite_count = 0;
        path_data.points[i].fix_quality = 0;
    }
    
    path_data.state = PATH_STATE_IDLE;
}

/**
 */
uint8 path_recorder_should_record(float lat, float lon, uint32 current_time)
{
    if (current_time - path_data.last_record_time < MIN_RECORD_INTERVAL)
    {
        return 0;
    }
    
    if (path_data.point_count > 0)
    {
        float distance = path_recorder_calculate_distance(
            lat, lon, 
            path_data.last_latitude, path_data.last_longitude);
        
        if (distance < MIN_RECORD_DISTANCE)
        {
            return 0;  // �ƶ�����̫С
        }
    }
    
    return 1;
}

/**
 */
uint8 path_recorder_add_point(path_point_t* point)
{
    if (path_data.point_count >= MAX_PATH_POINTS)
    {
        return 0;  // ·������
    }
    
    if (point == NULL)
    {
        return 0;  // ��Чָ��
    }
    
    path_data.points[path_data.point_count] = *point;
    path_data.point_count++;
    
    update_path_statistics(point);
    
    return 1;
}

/**
 */
static void update_path_statistics(path_point_t* new_point)
{
    if (path_data.point_count > 1)
    {
        path_point_t* prev_point = &path_data.points[path_data.point_count - 2];
        float distance = path_recorder_calculate_distance(
            new_point->latitude, new_point->longitude,
            prev_point->latitude, prev_point->longitude);
        
        path_data.total_distance += distance;
        
        uint32 time_diff = new_point->timestamp - prev_point->timestamp;
        path_data.total_time += time_diff;
    }
}

/**
 */
void path_recorder_task(void)
{
    static uint32 last_task_time = 0;
    uint32 current_time = get_current_time_ms();
    
    if (current_time - last_task_time < 100)
    {
        return;
    }
    last_task_time = current_time;
    
    if (path_data.state != PATH_STATE_RECORDING)
    {
        return;
    }
    
    if (!is_valid_gps_fix())
    {
        return;
    }
    
    if (!path_recorder_should_record((float)gnss.latitude, (float)gnss.longitude, current_time))
    {
        return;
    }
    
    path_point_t new_point;
    new_point.latitude = (float)gnss.latitude;
    new_point.longitude = (float)gnss.longitude;
    new_point.timestamp = current_time;
    new_point.speed = gnss.speed;
    new_point.direction = gnss.direction;
    new_point.satellite_count = gnss.satellite_used;
    new_point.fix_quality = gnss.state;
    
    if (path_recorder_add_point(&new_point))
    {
        path_data.last_latitude = (float)gnss.latitude;
        path_data.last_longitude = (float)gnss.longitude;
        path_data.last_record_time = current_time;
    }
}

/**
 * 
 */
float path_recorder_calculate_distance(float lat1, float lon1, float lat2, float lon2)
{
    const float METERS_PER_DEGREE_LAT = 111319.9f;  // γ��ÿ�ȶ�Ӧ������
    float avg_lat = (lat1 + lat2) * 0.5f;
    float METERS_PER_DEGREE_LON = 111319.9f * cosf(avg_lat * 3.14159265f / 180.0f);
    
    float dlat = lat2 - lat1;  // γ�Ȳ�ȣ�
    float dlon = lon2 - lon1;  // ���Ȳ�ȣ�
    
    float distance = sqrtf(dlat * dlat * METERS_PER_DEGREE_LAT * METERS_PER_DEGREE_LAT + 
                          dlon * dlon * METERS_PER_DEGREE_LON * METERS_PER_DEGREE_LON);
    
    return distance;
}

/**
 */
void path_recorder_start_playback(void)
{
    if (path_data.point_count == 0)
    {
        return;  // û��·������
    }
    
    path_data.state = PATH_STATE_PLAYBACK;
    path_data.current_index = 0;
}

/**
 */
void path_recorder_stop_playback(void)
{
    if (path_data.state == PATH_STATE_PLAYBACK)
    {
        path_data.state = PATH_STATE_COMPLETED;
    }
}

/**
 */
path_point_t* path_recorder_get_next_point(void)
{
    if (path_data.state != PATH_STATE_PLAYBACK)
    {
        return NULL;
    }
    
    if (path_data.current_index >= path_data.point_count)
    {
        return NULL;  // �ѵ�ĩβ
    }
    
    path_point_t* point = &path_data.points[path_data.current_index];
    path_data.current_index++;
    
    return point;
}

/**
 */
path_point_t* path_recorder_get_prev_point(void)
{
    if (path_data.state != PATH_STATE_PLAYBACK)
    {
        return NULL;
    }
    
    if (path_data.current_index == 0)
    {
        return NULL;  // ���ڿ�ͷ
    }
    
    path_data.current_index--;
    return &path_data.points[path_data.current_index];
}

/**
 */
void path_recorder_get_stats(float* distance, uint32* time, float* avg_speed)
{
    if (distance != NULL)
    {
        *distance = path_data.total_distance;
    }
    
    if (time != NULL)
    {
    }
    
    if (avg_speed != NULL)
    {
        if (path_data.total_time > 0)
        {
            float avg_speed_ms = path_data.total_distance / (path_data.total_time / 1000.0f);
            *avg_speed = avg_speed_ms * 3.6f;
        }
        else
        {
            *avg_speed = 0.0f;
        }
    }
}

/**
 */
uint8 path_recorder_save_to_flash(uint8 slot)
{
    
    (void)slot;  // ����δʹ�ò�������
    return 1;
}

/**
 */
uint8 path_recorder_load_from_flash(uint8 slot)
{
    
    (void)slot;  // ����δʹ�ò�������
    return 1;
}

/**
 */
path_state_enum path_recorder_get_state(void)
{
    return path_data.state;
}

/**
 */
uint16 path_recorder_get_point_count(void)
{
    return path_data.point_count;
}

/**
 */
uint8 path_recorder_is_full(void)
{
    return (path_data.point_count >= MAX_PATH_POINTS) ? 1 : 0;
}
