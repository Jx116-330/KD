/**
 * @file path_types.h
 * @brief 路径记录模块数据类型定义
 * @author 贾维斯
 * @date 2026-03-21
 * @version 2.0
 *
 * @details 定义优化的数据结构，支持压缩存储和高效访问
 */

#ifndef _PATH_TYPES_H_
#define _PATH_TYPES_H_

#include "zf_common_typedef.h"
#include "path_config.h"

//--------------------------------------------------------------------------------------------------
// 基本类型定义
//--------------------------------------------------------------------------------------------------

/** @brief 布尔类型 */
typedef uint8 bool_t;
#define TRUE    1
#define FALSE   0

/** @brief 时间戳类型（毫秒） */
typedef uint32 timestamp_t;

/** @brief 距离类型（米） */
typedef float distance_t;

/** @brief 速度类型（km/h） */
typedef float speed_t;

/** @brief 角度类型（度） */
typedef float angle_t;

//--------------------------------------------------------------------------------------------------
// 压缩坐标类型（节省内存）
//--------------------------------------------------------------------------------------------------

#if USE_COMPRESSED_STORAGE

/**
 * @brief 压缩坐标结构（16位）
 * @details 使用相对坐标和缩放，精度0.1米，范围±3.2公里
 */
typedef struct
{
    int16 x;    /**< X坐标（0.1米单位） */
    int16 y;    /**< Y坐标（0.1米单位） */
} compressed_coord_t;

/**
 * @brief 压缩路径点结构（12字节）
 */
typedef struct
{
    compressed_coord_t coord;      /**< 压缩坐标 */
    timestamp_t timestamp;         /**< 时间戳（毫秒） */
    uint16 speed;                  /**< 速度（0.1km/h单位） */
    uint8 direction;               /**< 方向（2度单位，0-179） */
    uint8 flags;                   /**< 标志位 */
} compressed_point_t;

#define POINT_SIZE_BYTES           sizeof(compressed_point_t)  // 12字节

#else

/**
 * @brief 标准坐标结构
 */
typedef struct
{
    float x;    /**< X坐标（米） */
    float y;    /**< Y坐标（米） */
} coord_t;

/**
 * @brief 标准路径点结构（20字节）
 */
typedef struct
{
    coord_t coord;                 /**< 坐标 */
    timestamp_t timestamp;         /**< 时间戳（毫秒） */
    speed_t speed;                 /**< 速度（km/h） */
    angle_t direction;             /**< 方向（度） */
    uint8 satellite_count;         /**< 卫星数量 */
    uint8 fix_quality;             /**< 定位质量 */
    uint8 flags;                   /**< 标志位 */
} path_point_t;

#define POINT_SIZE_BYTES           sizeof(path_point_t)  // 20字节

#endif

//--------------------------------------------------------------------------------------------------
// 路径点标志位定义
//--------------------------------------------------------------------------------------------------

/** @brief 标志位掩码定义 */
typedef enum
{
    POINT_FLAG_VALID        = 0x01,  /**< 点有效 */
    POINT_FLAG_FILTERED     = 0x02,  /**< 点被过滤 */
    POINT_FLAG_COMPRESSED   = 0x04,  /**< 点已压缩 */
    POINT_FLAG_TURNING      = 0x08,  /**< 转弯点 */
    POINT_FLAG_STOP         = 0x10,  /**< 停止点 */
    POINT_FLAG_LANDMARK     = 0x20,  /**< 地标点 */
} point_flag_t;

//--------------------------------------------------------------------------------------------------
// 路径状态枚举
//--------------------------------------------------------------------------------------------------

/**
 * @brief 路径状态枚举
 */
typedef enum
{
    PATH_STATE_IDLE = 0,          /**< 空闲状态 */
    PATH_STATE_INITIALIZING,      /**< 初始化中 */
    PATH_STATE_CALIBRATING,       /**< 校准中 */
    PATH_STATE_RECORDING,         /**< 正在记录 */
    PATH_STATE_PAUSED,            /**< 暂停记录 */
    PATH_STATE_PLAYBACK,          /**< 回放状态 */
    PATH_STATE_COMPLETED,         /**< 记录完成 */
    PATH_STATE_ERROR,             /**< 错误状态 */
    PATH_STATE_COUNT              /**< 状态数量 */
} path_state_t;

//--------------------------------------------------------------------------------------------------
// 路径质量枚举
//--------------------------------------------------------------------------------------------------

/**
 * @brief 路径质量等级
 */
typedef enum
{
    PATH_QUALITY_POOR = 0,        /**< 质量差（卫星少，HDOP高） */
    PATH_QUALITY_FAIR,            /**< 质量一般 */
    PATH_QUALITY_GOOD,            /**< 质量好 */
    PATH_QUALITY_EXCELLENT,       /**< 质量优秀 */
} path_quality_t;

//--------------------------------------------------------------------------------------------------
// 路径统计结构
//--------------------------------------------------------------------------------------------------

/**
 * @brief 路径统计信息
 */
typedef struct
{
    distance_t total_distance;     /**< 总距离（米） */
    timestamp_t total_time;        /**< 总时间（毫秒） */
    speed_t avg_speed;             /**< 平均速度（km/h） */
    speed_t max_speed;             /**< 最大速度（km/h） */
    distance_t max_displacement;   /**< 最大位移（米） */
    uint16 point_count;            /**< 总点数 */
    uint16 valid_point_count;      /**< 有效点数 */
    uint16 filtered_point_count;   /**< 过滤点数 */
    path_quality_t quality;        /**< 路径质量 */
} path_stats_t;

//--------------------------------------------------------------------------------------------------
// 路径配置结构
//--------------------------------------------------------------------------------------------------

/**
 * @brief 路径记录配置
 */
typedef struct
{
    distance_t min_record_distance;    /**< 最小记录距离 */
    uint32 min_record_interval;        /**< 最小记录间隔 */
    uint8 min_satellites;              /**< 最小卫星数 */
    float max_hdop;                    /**< 最大HDOP */
    speed_t max_speed;                 /**< 最大记录速度 */
    distance_t max_position_jump;      /**< 最大位置跳跃 */
    uint8 compression_enabled;         /**< 启用压缩 */
    float compression_precision;       /**< 压缩精度 */
} path_config_t;

//--------------------------------------------------------------------------------------------------
// 性能统计结构
//--------------------------------------------------------------------------------------------------

#if PATH_PERF_STATS_ENABLED

/**
 * @brief 性能统计信息
 */
typedef struct
{
    uint32 task_exec_count;        /**< 任务执行次数 */
    uint32 gps_check_count;        /**< GPS检查次数 */
    uint32 point_add_count;        /**< 点添加次数 */
    uint32 point_filter_count;     /**< 点过滤次数 */
    uint32 display_update_count;   /**< 显示更新次数 */
    uint32 max_task_time_us;       /**< 最大任务时间（微秒） */
    uint32 avg_task_time_us;       /**< 平均任务时间（微秒） */
    uint32 memory_used_bytes;      /**< 已用内存（字节） */
    uint32 memory_free_bytes;      /**< 空闲内存（字节） */
} perf_stats_t;

#endif

//--------------------------------------------------------------------------------------------------
// 错误信息结构
//--------------------------------------------------------------------------------------------------

/**
 * @brief 错误信息
 */
typedef struct
{
    uint32 error_code;             /**< 错误代码 */
    timestamp_t error_time;        /**< 错误时间 */
    uint8 retry_count;             /**< 重试次数 */
    char error_msg[32];            /**< 错误信息 */
} error_info_t;

//--------------------------------------------------------------------------------------------------
// 回调函数类型定义
//--------------------------------------------------------------------------------------------------

/**
 * @brief 状态改变回调函数
 * @param old_state 旧状态
 * @param new_state 新状态
 */
typedef void (*state_change_cb_t)(path_state_t old_state, path_state_t new_state);

/**
 * @brief 点添加回调函数
 * @param point_index 点索引
 * @param point_count 总点数
 */
typedef void (*point_added_cb_t)(uint16 point_index, uint16 point_count);

/**
 * @brief 错误回调函数
 * @param error_info 错误信息
 */
typedef void (*error_cb_t)(const error_info_t* error_info);

//--------------------------------------------------------------------------------------------------
// 回调配置结构
//--------------------------------------------------------------------------------------------------

/**
 * @brief 回调函数配置
 */
typedef struct
{
    state_change_cb_t state_change_cb;    /**< 状态改变回调 */
    point_added_cb_t point_added_cb;      /**< 点添加回调 */
    error_cb_t error_cb;                  /**< 错误回调 */
} callback_config_t;

#endif /* _PATH_TYPES_H_ */
