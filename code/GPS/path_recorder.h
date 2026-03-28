/***
  @file path_recorder.h
  @brief GPS轨迹记录模块头文件
  @author JX116
  @date 2026-03-21
  @version 1.0

  @details 提供：
           - 实时轨迹记录
           - 轨迹点存储
           - 回放支持
           - 轨迹统计信息
 **/

#ifndef _PATH_RECORDER_H_
#define _PATH_RECORDER_H_

#include "zf_common_headfile.h"
#include "zf_device_gnss.h"
#include "zf_driver_timer.h"

#define MAX_PATH_POINTS          200
#define MIN_RECORD_DISTANCE      0.5
#define MIN_RECORD_INTERVAL      100

typedef enum
{
    PATH_STATE_IDLE = 0,
    PATH_STATE_RECORDING,
    PATH_STATE_PAUSED,
    PATH_STATE_PLAYBACK,
    PATH_STATE_COMPLETED
} path_state_enum;

typedef struct
{
    float latitude;
    float longitude;
    uint32 timestamp;
    float speed;
    float direction;
    uint8 satellite_count;
    uint8 fix_quality;
} path_point_t;

typedef struct
{
    path_point_t points[MAX_PATH_POINTS];
    uint16 point_count;
    uint16 current_index;
    path_state_enum state;
    uint32 last_record_time;
    float last_latitude;
    float last_longitude;
    float total_distance;
    uint32 total_time;
} path_data_t;

extern path_data_t path_data;

void path_recorder_init(void);
uint8 path_recorder_start(void);
void path_recorder_pause(void);
void path_recorder_resume(void);
void path_recorder_stop(void);
void path_recorder_clear(void);
void path_recorder_task(void);
void path_recorder_start_playback(void);
void path_recorder_stop_playback(void);
path_point_t* path_recorder_get_next_point(void);
path_point_t* path_recorder_get_prev_point(void);
void path_recorder_get_stats(float* distance, uint32* time, float* avg_speed);
uint8 path_recorder_save_to_flash(uint8 slot);
uint8 path_recorder_load_from_flash(uint8 slot);
uint8 path_recorder_should_record(double lat, double lon, uint32 current_time);
uint8 path_recorder_add_point(path_point_t* point);
float path_recorder_calculate_distance(double lat1, double lon1, double lat2, double lon2);
path_state_enum path_recorder_get_state(void);
uint16 path_recorder_get_point_count(void);
uint8 path_recorder_is_full(void);

#endif
