/**
 * @file path_types.h
 * @brief Cleaned module header to avoid encoding issues.
 * @author JX116
 */

#ifndef _PATH_TYPES_H_
#define _PATH_TYPES_H_

#include "zf_common_typedef.h"
#include "path_config.h"

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

typedef uint8 bool_t;
#define TRUE    1
#define FALSE   0

typedef uint32 timestamp_t;

typedef float distance_t;

typedef float speed_t;

typedef float angle_t;

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

#if USE_COMPRESSED_STORAGE

/**
 */
typedef struct
{
    int16 x;    /**< X���꣨0.1�׵�λ�� */
    int16 y;    /**< Y���꣨0.1�׵�λ�� */
} compressed_coord_t;

/**
 */
typedef struct
{
    compressed_coord_t coord;      /**< ѹ������ */
    timestamp_t timestamp;         /**< ʱ���������?*/
    uint16 speed;                  /**< �ٶȣ�0.1km/h��λ�� */
    uint8 direction;               /**< ����2�ȵ�λ��0-179�� */
    uint8 flags;                   /**< ��־λ */
} compressed_point_t;

#define POINT_SIZE_BYTES           sizeof(compressed_point_t)  // 12�ֽ�

#else

/**
 */
typedef struct
{
    float x;    /**< X���꣨�ף� */
    float y;    /**< Y���꣨�ף� */
} coord_t;

/**
 */
typedef struct
{
    coord_t coord;                 /**< ���� */
    timestamp_t timestamp;         /**< ʱ���������?*/
    speed_t speed;                 /**< �ٶȣ�km/h�� */
    angle_t direction;             /**< ���򣨶ȣ� */
    uint8 satellite_count;         /**< �������� */
    uint8 fix_quality;             /**< ��λ���� */
    uint8 flags;                   /**< ��־λ */
} path_point_t;

#define POINT_SIZE_BYTES           sizeof(path_point_t)  // 20�ֽ�

#endif

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

typedef enum
{
    POINT_FLAG_VALID        = 0x01,  /**< ����Ч */
    POINT_FLAG_FILTERED     = 0x02,  /**< �㱻���� */
    POINT_FLAG_COMPRESSED   = 0x04,  /**< ����ѹ�� */
    POINT_FLAG_TURNING      = 0x08,  /**< ת���?*/
    POINT_FLAG_STOP         = 0x10,  /**< ֹͣ�� */
    POINT_FLAG_LANDMARK     = 0x20,  /**< �ر��?*/
} point_flag_t;

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

/**
 */
typedef enum
{
    PATH_STATE_IDLE = 0,          /**< ����״̬ */
    PATH_STATE_INITIALIZING,      /**< ��ʼ���� */
    PATH_STATE_CALIBRATING,       /**< У׼�� */
    PATH_STATE_RECORDING,         /**< ���ڼ�¼ */
    PATH_STATE_PAUSED,            /**< ��ͣ��¼ */
    PATH_STATE_PLAYBACK,          /**< �ط�״̬ */
    PATH_STATE_COMPLETED,         /**< ��¼���?*/
    PATH_STATE_ERROR,             /**< ����״̬ */
    PATH_STATE_COUNT              /**< ״̬���� */
} path_state_t;

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

/**
 */
typedef enum
{
    PATH_QUALITY_POOR = 0,        /**< ����������٣�HDOP�ߣ� */
    PATH_QUALITY_FAIR,            /**< ����һ�� */
    PATH_QUALITY_GOOD,            /**< ������ */
    PATH_QUALITY_EXCELLENT,       /**< �������� */
} path_quality_t;

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

/**
 */
typedef struct
{
    distance_t total_distance;     /**< �ܾ��루�ף� */
    timestamp_t total_time;        /**< ��ʱ�䣨���룩 */
    speed_t avg_speed;             /**< ƽ���ٶȣ�km/h�� */
    speed_t max_speed;             /**< ����ٶȣ�km/h�� */
    distance_t max_displacement;   /**< ���λ�ƣ��ף�?*/
    uint16 point_count;            /**< �ܵ��� */
    uint16 valid_point_count;      /**< ��Ч���� */
    uint16 filtered_point_count;   /**< ���˵��� */
    path_quality_t quality;        /**< ·������ */
} path_stats_t;

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

/**
 */
typedef struct
{
    distance_t min_record_distance;    /**< ��С��¼���� */
    uint32 min_record_interval;        /**< ��С��¼���?*/
    uint8 min_satellites;              /**< ��С������ */
    float max_hdop;                    /**< ���HDOP */
    speed_t max_speed;                 /**< ����¼�ٶ� */
    distance_t max_position_jump;      /**< ���λ�����?*/
    uint8 compression_enabled;         /**< ����ѹ�� */
    float compression_precision;       /**< ѹ������ */
} path_config_t;

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

#if PATH_PERF_STATS_ENABLED

/**
 */
typedef struct
{
    uint32 task_exec_count;        /**< ����ִ�д��� */
    uint32 gps_check_count;        /**< GPS������ */
    uint32 point_add_count;        /**< �����Ӵ��� */
    uint32 point_filter_count;     /**< ����˴���?*/
    uint32 display_update_count;   /**< ��ʾ���´��� */
    uint32 max_task_time_us;       /**< �������ʱ�䣨΢��?*/
    uint32 avg_task_time_us;       /**< ƽ������ʱ�䣨΢�룩 */
    uint32 memory_used_bytes;      /**< �����ڴ棨�ֽڣ� */
    uint32 memory_free_bytes;      /**< �����ڴ棨�ֽڣ� */
} perf_stats_t;

#endif

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

/**
 */
typedef struct
{
    uint32 error_code;             /**< �������?*/
    timestamp_t error_time;        /**< ����ʱ�� */
    uint8 retry_count;             /**< ���Դ��� */
    char error_msg[32];            /**< ������Ϣ */
} error_info_t;

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

/**
 */
typedef void (*state_change_cb_t)(path_state_t old_state, path_state_t new_state);

/**
 */
typedef void (*point_added_cb_t)(uint16 point_index, uint16 point_count);

/**
 */
typedef void (*error_cb_t)(const error_info_t* error_info);

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

/**
 */
typedef struct
{
    state_change_cb_t state_change_cb;    /**< ״̬�ı�ص�?*/
    point_added_cb_t point_added_cb;      /**< �����ӻص� */
    error_cb_t error_cb;                  /**< ����ص�?*/
} callback_config_t;

#endif /* _PATH_TYPES_H_ */
