/**
 * @file path_recorder.h
 * @brief 路径记录模块头文件
 * @author 贾维斯
 * @date 2026-03-21
 * @version 1.0
 *
 * @details 本模块提供GPS路径记录功能，包括：
 *          - 实时路径记录
 *          - 路径数据存储
 *          - 路径回放
 *          - 路径分析
 *          用于"Kart Sprint"比赛中的迷宫导航科目三
 */

#ifndef _PATH_RECORDER_H_
#define _PATH_RECORDER_H_

#include "zf_common_headfile.h"
#include "zf_device_gnss.h"
#include "zf_driver_timer.h"  // 添加timer驱动，用于system_getval_ms()

//--------------------------------------------------------------------------------------------------
// 常量定义
//--------------------------------------------------------------------------------------------------

/** @brief 最大路径点数 */
#define MAX_PATH_POINTS          200

/** @brief 最小记录距离（米） */
#define MIN_RECORD_DISTANCE      0.5

/** @brief 最小记录时间间隔（毫秒） */
#define MIN_RECORD_INTERVAL      100

/** @brief 路径状态枚举 */
typedef enum
{
    PATH_STATE_IDLE = 0,          /**< 空闲状态 */
    PATH_STATE_RECORDING,         /**< 正在记录 */
    PATH_STATE_PAUSED,            /**< 暂停记录 */
    PATH_STATE_PLAYBACK,          /**< 回放状态 */
    PATH_STATE_COMPLETED          /**< 记录完成 */
} path_state_enum;

/** @brief 路径点数据结构 */
typedef struct
{
    float latitude;               /**< 纬度（度），float精度足够（约1米精度） */
    float longitude;              /**< 经度（度），float精度足够（约1米精度） */
    uint32 timestamp;             /**< 时间戳（毫秒） */
    float speed;                  /**< 速度（km/h） */
    float direction;              /**< 方向（度） */
    uint8 satellite_count;        /**< 卫星数量 */
    uint8 fix_quality;            /**< 定位质量 */
} path_point_t;

/** @brief 路径数据结构 */
typedef struct
{
    path_point_t points[MAX_PATH_POINTS];  /**< 路径点数组 */
    uint16 point_count;                     /**< 当前路径点数 */
    uint16 current_index;                   /**< 当前索引（回放时使用） */
    path_state_enum state;                  /**< 路径状态 */
    uint32 last_record_time;                /**< 上次记录时间 */
    float last_latitude;                    /**< 上次记录的纬度 */
    float last_longitude;                   /**< 上次记录的经度 */
    float total_distance;                   /**< 总距离（米） */
    uint32 total_time;                      /**< 总时间（毫秒） */
} path_data_t;

//--------------------------------------------------------------------------------------------------
// 全局变量声明
//--------------------------------------------------------------------------------------------------

extern path_data_t path_data;

//--------------------------------------------------------------------------------------------------
// 函数声明
//--------------------------------------------------------------------------------------------------

/**
 * @brief 路径记录模块初始化
 */
void path_recorder_init(void);

/**
 * @brief 开始记录路径
 * @return 成功返回1，失败返回0
 */
uint8 path_recorder_start(void);

/**
 * @brief 暂停记录路径
 */
void path_recorder_pause(void);

/**
 * @brief 恢复记录路径
 */
void path_recorder_resume(void);

/**
 * @brief 停止记录路径
 */
void path_recorder_stop(void);

/**
 * @brief 清除所有路径数据
 */
void path_recorder_clear(void);

/**
 * @brief 路径记录主任务（需要周期性调用）
 */
void path_recorder_task(void);

/**
 * @brief 开始路径回放
 */
void path_recorder_start_playback(void);

/**
 * @brief 停止路径回放
 */
void path_recorder_stop_playback(void);

/**
 * @brief 获取下一个回放点
 * @return 成功返回路径点指针，失败返回NULL
 */
path_point_t* path_recorder_get_next_point(void);

/**
 * @brief 获取上一个回放点
 * @return 成功返回路径点指针，失败返回NULL
 */
path_point_t* path_recorder_get_prev_point(void);

/**
 * @brief 获取路径统计信息
 * @param[out] distance 总距离（米）
 * @param[out] time 总时间（秒）
 * @param[out] avg_speed 平均速度（km/h）
 */
void path_recorder_get_stats(float* distance, uint32* time, float* avg_speed);

/**
 * @brief 保存路径到Flash
 * @param slot 存储槽位（0-9）
 * @return 成功返回1，失败返回0
 */
uint8 path_recorder_save_to_flash(uint8 slot);

/**
 * @brief 从Flash加载路径
 * @param slot 存储槽位（0-9）
 * @return 成功返回1，失败返回0
 */
uint8 path_recorder_load_from_flash(uint8 slot);

/**
 * @brief 检查是否应该记录当前点
 * @param lat 当前纬度
 * @param lon 当前经度
 * @param current_time 当前时间
 * @return 应该记录返回1，否则返回0
 */
uint8 path_recorder_should_record(double lat, double lon, uint32 current_time);

/**
 * @brief 添加路径点
 * @param point 路径点数据
 * @return 成功返回1，失败返回0
 */
uint8 path_recorder_add_point(path_point_t* point);

/**
 * @brief 计算两点间距离
 * @param lat1 点1纬度
 * @param lon1 点1经度
 * @param lat2 点2纬度
 * @param lon2 点2经度
 * @return 距离（米）
 */
float path_recorder_calculate_distance(double lat1, double lon1, double lat2, double lon2);

/**
 * @brief 获取路径状态
 * @return 当前路径状态
 */
path_state_enum path_recorder_get_state(void);

/**
 * @brief 获取当前路径点数
 * @return 路径点数
 */
uint16 path_recorder_get_point_count(void);

/**
 * @brief 获取路径是否已满
 * @return 已满返回1，否则返回0
 */
uint8 path_recorder_is_full(void);

#endif /* _PATH_RECORDER_H_ */
