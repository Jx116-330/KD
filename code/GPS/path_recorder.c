/***
 * @file path_recorder.c
 * @brief GPS轨迹记录模块实现
 * @author JX116
 * @date 2026-03-21
 * @version 1.0
 *
 * @details 实现GPS轨迹记录、轨迹点管理与统计功能。
 ***/

#include "path_recorder.h"
#include "display_gps.h"
#include "zf_driver_flash.h"
#include <string.h>
#include <math.h>

#define get_current_time_ms()   system_getval_ms()

#define PATH_FLASH_SECTOR            (0U)
#define PATH_FLASH_BASE_PAGE         (120U)
#define PATH_FLASH_PAGES_PER_SLOT    (3U)
#define PATH_FLASH_SLOT_COUNT        (2U)
#define PATH_FLASH_PAGE_WORDS        (EEPROM_PAGE_LENGTH)
#define PATH_FLASH_PAGE_BYTES        (PATH_FLASH_PAGE_WORDS * sizeof(uint32))
#define PATH_FLASH_MAGIC             (0x50415448UL)
#define PATH_FLASH_VERSION           (0x00010000UL)

typedef struct
{
    uint32 magic;
    uint32 version;
    uint32 point_count;
    uint32 total_time;
    float total_distance;
    uint32 reserved;
} path_flash_header_t;

path_data_t path_data;
static uint32 path_flash_page_buffer[PATH_FLASH_PAGE_WORDS];

static uint8 is_valid_gps_fix(void);
static void update_path_statistics(path_point_t* new_point);
static uint8 path_recorder_get_slot_page(uint8 slot, uint32 *page_num);
static uint32 path_recorder_get_saved_bytes(uint16 point_count);
static void path_recorder_pack_page(uint8 *page_buffer, uint32 page_offset, const path_flash_header_t *header);
static void path_recorder_restore_runtime_fields(void);

static uint8 is_valid_gps_fix(void)
{
    return (gnss.state == 1 &&
            gnss.satellite_used >= GPS_MIN_SATELLITES &&
            gnss.speed <= MAX_RECORD_SPEED_KPH &&
            gnss.latitude != 0.0 &&
            gnss.longitude != 0.0);
}

void path_recorder_init(void)
{
    memset(&path_data, 0, sizeof(path_data));
    path_data.state = PATH_STATE_IDLE;
}

uint8 path_recorder_start(void)
{
    if (!is_valid_gps_fix())
    {
        return 0;
    }

    if (path_data.state == PATH_STATE_RECORDING)
    {
        return 1;
    }

    path_data.state = PATH_STATE_RECORDING;
    path_data.last_record_time = get_current_time_ms();
    path_data.last_latitude = (float)gnss.latitude;
    path_data.last_longitude = (float)gnss.longitude;
    return 1;
}

void path_recorder_pause(void)
{
    if (path_data.state == PATH_STATE_RECORDING)
    {
        path_data.state = PATH_STATE_PAUSED;
    }
}

void path_recorder_resume(void)
{
    if (path_data.state == PATH_STATE_PAUSED)
    {
        path_data.state = PATH_STATE_RECORDING;
        path_data.last_record_time = get_current_time_ms();
    }
}

void path_recorder_stop(void)
{
    if (path_data.state == PATH_STATE_RECORDING || path_data.state == PATH_STATE_PAUSED)
    {
        path_data.state = PATH_STATE_COMPLETED;
    }
}

void path_recorder_clear(void)
{
    memset(&path_data, 0, sizeof(path_data));
    path_data.state = PATH_STATE_IDLE;
}

uint8 path_recorder_should_record(double lat, double lon, uint32 current_time)
{
    float distance;

    if (path_data.point_count == 0)
    {
        return 1;
    }

    if (current_time - path_data.last_record_time < MIN_RECORD_INTERVAL_MS)
    {
        return 0;
    }

    distance = path_recorder_calculate_distance(path_data.last_latitude, path_data.last_longitude, lat, lon);
    return (distance >= MIN_RECORD_DISTANCE);
}

uint8 path_recorder_add_point(path_point_t* point)
{
    if (path_data.point_count >= MAX_PATH_POINTS || point == NULL)
    {
        return 0;
    }

    path_data.points[path_data.point_count] = *point;
    path_data.point_count++;
    update_path_statistics(point);
    path_data.last_latitude = point->latitude;
    path_data.last_longitude = point->longitude;
    path_data.last_record_time = point->timestamp;
    return 1;
}

void path_recorder_task(void)
{
    if (path_data.state != PATH_STATE_RECORDING)
    {
        return;
    }

    if (!is_valid_gps_fix())
    {
        return;
    }

    uint32 current_time = get_current_time_ms();
    if (path_recorder_should_record(gnss.latitude, gnss.longitude, current_time))
    {
        path_point_t point;
        point.latitude = (float)gnss.latitude;
        point.longitude = (float)gnss.longitude;
        point.timestamp = current_time;
        point.speed = gnss.speed;
        point.direction = gnss.direction;
        point.satellite_count = gnss.satellite_used;
        point.fix_quality = gnss.state;
        path_recorder_add_point(&point);
    }
}

void path_recorder_start_playback(void)
{
    if (path_data.point_count > 0)
    {
        path_data.state = PATH_STATE_PLAYBACK;
        path_data.current_index = 0;
    }
}

void path_recorder_stop_playback(void)
{
    if (path_data.state == PATH_STATE_PLAYBACK)
    {
        path_data.state = PATH_STATE_COMPLETED;
        path_data.current_index = 0;
    }
}

path_point_t* path_recorder_get_next_point(void)
{
    if (path_data.current_index < path_data.point_count)
    {
        return &path_data.points[path_data.current_index++];
    }
    return NULL;
}

path_point_t* path_recorder_get_prev_point(void)
{
    if (path_data.current_index > 0)
    {
        path_data.current_index--;
        return &path_data.points[path_data.current_index];
    }
    return NULL;
}

float path_recorder_calculate_distance(double lat1, double lon1, double lat2, double lon2)
{
    double dlat = (lat2 - lat1) * USER_PI / 180.0;
    double dlon = (lon2 - lon1) * USER_PI / 180.0;
    double a = sin(dlat / 2) * sin(dlat / 2) +
               cos(lat1 * USER_PI / 180.0) * cos(lat2 * USER_PI / 180.0) *
               sin(dlon / 2) * sin(dlon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return (float)(EARTH_RADIUS * c);
}

static void update_path_statistics(path_point_t* new_point)
{
    if (path_data.point_count > 1)
    {
        path_point_t* prev = &path_data.points[path_data.point_count - 2];
        path_data.total_distance += path_recorder_calculate_distance(
            prev->latitude, prev->longitude, new_point->latitude, new_point->longitude);
        path_data.total_time = new_point->timestamp - path_data.points[0].timestamp;
    }
}

void path_recorder_get_stats(float* distance, uint32* time, float* avg_speed)
{
    if (distance) *distance = path_data.total_distance;
    if (time) *time = path_data.total_time;
    if (avg_speed)
    {
        if (path_data.total_time > 0)
            *avg_speed = (path_data.total_distance / ((float)path_data.total_time / 1000.0f)) * 3.6f;
        else
            *avg_speed = 0.0f;
    }
}

uint8 path_recorder_save_to_flash(uint8 slot)
{
    uint32 start_page;
    uint32 page_index;
    uint32 total_bytes;
    path_flash_header_t header;

    if (!path_recorder_get_slot_page(slot, &start_page))
    {
        return 0;
    }

    header.magic = PATH_FLASH_MAGIC;
    header.version = PATH_FLASH_VERSION;
    header.point_count = path_data.point_count;
    header.total_time = path_data.total_time;
    header.total_distance = path_data.total_distance;
    header.reserved = 0;
    total_bytes = path_recorder_get_saved_bytes(path_data.point_count);

    if (total_bytes > (PATH_FLASH_PAGES_PER_SLOT * PATH_FLASH_PAGE_BYTES))
    {
        return 0;
    }

    for (page_index = 0; page_index < PATH_FLASH_PAGES_PER_SLOT; page_index++)
    {
        memset(path_flash_page_buffer, 0xFF, sizeof(path_flash_page_buffer));
        path_recorder_pack_page((uint8 *)path_flash_page_buffer, page_index * PATH_FLASH_PAGE_BYTES, &header);
        flash_write_page(PATH_FLASH_SECTOR,
                         start_page + page_index,
                         path_flash_page_buffer,
                         PATH_FLASH_PAGE_WORDS);
    }

    return 1;
}

uint8 path_recorder_load_from_flash(uint8 slot)
{
    uint32 start_page;
    uint32 page_index;
    path_flash_header_t header;
    uint32 total_bytes;
    uint32 points_bytes;

    if (!path_recorder_get_slot_page(slot, &start_page))
    {
        return 0;
    }

    flash_read_page(PATH_FLASH_SECTOR, start_page, path_flash_page_buffer, PATH_FLASH_PAGE_WORDS);
    memcpy(&header, path_flash_page_buffer, sizeof(header));

    if (header.magic != PATH_FLASH_MAGIC || header.version != PATH_FLASH_VERSION)
    {
        return 0;
    }

    if (header.point_count > MAX_PATH_POINTS)
    {
        return 0;
    }

    memset(&path_data, 0, sizeof(path_data));

    points_bytes = header.point_count * sizeof(path_point_t);
    total_bytes = path_recorder_get_saved_bytes((uint16)header.point_count);

    if (total_bytes > (PATH_FLASH_PAGES_PER_SLOT * PATH_FLASH_PAGE_BYTES))
    {
        return 0;
    }

    for (page_index = 0; page_index < PATH_FLASH_PAGES_PER_SLOT; page_index++)
    {
        uint32 page_offset = page_index * PATH_FLASH_PAGE_BYTES;
        uint32 data_begin = sizeof(path_flash_header_t);
        uint32 page_end = page_offset + PATH_FLASH_PAGE_BYTES;
        uint32 copy_begin;
        uint32 copy_end;

        if (page_offset >= total_bytes)
        {
            break;
        }

        flash_read_page(PATH_FLASH_SECTOR,
                        start_page + page_index,
                        path_flash_page_buffer,
                        PATH_FLASH_PAGE_WORDS);

        copy_begin = (page_offset > data_begin) ? page_offset : data_begin;
        copy_end = (page_end < total_bytes) ? page_end : total_bytes;

        if (copy_end > copy_begin && points_bytes > 0U)
        {
            uint32 src_offset = copy_begin - page_offset;
            uint32 dst_offset = copy_begin - data_begin;
            uint32 copy_size = copy_end - copy_begin;
            memcpy(((uint8 *)path_data.points) + dst_offset,
                   ((uint8 *)path_flash_page_buffer) + src_offset,
                   copy_size);
        }
    }

    path_data.point_count = (uint16)header.point_count;
    path_data.total_time = header.total_time;
    path_data.total_distance = header.total_distance;
    path_data.state = (path_data.point_count > 0U) ? PATH_STATE_COMPLETED : PATH_STATE_IDLE;
    path_recorder_restore_runtime_fields();
    return 1;
}

path_state_enum path_recorder_get_state(void)
{
    return path_data.state;
}

uint16 path_recorder_get_point_count(void)
{
    return path_data.point_count;
}

uint8 path_recorder_is_full(void)
{
    return (path_data.point_count >= MAX_PATH_POINTS);
}

static uint8 path_recorder_get_slot_page(uint8 slot, uint32 *page_num)
{
    if (NULL == page_num || slot >= PATH_FLASH_SLOT_COUNT)
    {
        return 0;
    }

    *page_num = PATH_FLASH_BASE_PAGE + ((uint32)slot * PATH_FLASH_PAGES_PER_SLOT);
    return 1;
}

static uint32 path_recorder_get_saved_bytes(uint16 point_count)
{
    return (uint32)sizeof(path_flash_header_t) + ((uint32)point_count * sizeof(path_point_t));
}

static void path_recorder_pack_page(uint8 *page_buffer, uint32 page_offset, const path_flash_header_t *header)
{
    uint32 header_bytes = sizeof(path_flash_header_t);
    uint32 total_bytes = path_recorder_get_saved_bytes((uint16)header->point_count);
    uint32 page_end = page_offset + PATH_FLASH_PAGE_BYTES;
    uint32 copy_begin;
    uint32 copy_end;

    if (page_offset < header_bytes)
    {
        uint32 header_copy = header_bytes - page_offset;
        if (header_copy > PATH_FLASH_PAGE_BYTES)
        {
            header_copy = PATH_FLASH_PAGE_BYTES;
        }
        memcpy(page_buffer, ((const uint8 *)header) + page_offset, header_copy);
    }

    copy_begin = (page_offset > header_bytes) ? page_offset : header_bytes;
    copy_end = (page_end < total_bytes) ? page_end : total_bytes;

    if (copy_end > copy_begin)
    {
        uint32 dst_offset = copy_begin - page_offset;
        uint32 src_offset = copy_begin - header_bytes;
        uint32 copy_size = copy_end - copy_begin;
        memcpy(page_buffer + dst_offset, ((const uint8 *)path_data.points) + src_offset, copy_size);
    }
}

static void path_recorder_restore_runtime_fields(void)
{
    path_data.current_index = 0;

    if (path_data.point_count > 0U)
    {
        path_point_t *last_point = &path_data.points[path_data.point_count - 1U];
        path_data.last_latitude = last_point->latitude;
        path_data.last_longitude = last_point->longitude;
        path_data.last_record_time = last_point->timestamp;
    }
    else
    {
        path_data.last_latitude = 0.0f;
        path_data.last_longitude = 0.0f;
        path_data.last_record_time = 0U;
    }
}
