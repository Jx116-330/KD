/***
 * @file path_config.h
 * @brief 轨迹记录模块配置文件
 * @author JX116
 * @date 2026-03-21
 * @version 2.0
 *
 * @details 集中管理轨迹记录相关的配置与调参项。
 ***/

#ifndef _PATH_CONFIG_H_
#define _PATH_CONFIG_H_

// -----------------------------------------------------------------------------
// 系统配置
// -----------------------------------------------------------------------------

/** @brief 是否启用调试输出 */
#define PATH_DEBUG_ENABLED          1
/** @brief 是否启用性能统计 */
#define PATH_PERF_STATS_ENABLED     1
/** @brief 是否启用断言 */
#define PATH_ASSERT_ENABLED         1

// -----------------------------------------------------------------------------
// 轨迹记录参数
// -----------------------------------------------------------------------------

/** @brief 最大轨迹点数量 */
#define MAX_PATH_POINTS             200
/** @brief 最小记录距离，单位米 */
#define MIN_RECORD_DISTANCE         0.5f
/** @brief 最小记录时间间隔，单位毫秒 */
#define MIN_RECORD_INTERVAL_MS      100
/** @brief GPS最小有效卫星数 */
#define GPS_MIN_SATELLITES          4
/** @brief GPS最大HDOP阈值 */
#define GPS_MAX_HDOP                5.0f
/** @brief 最大记录速度，单位km/h */
#define MAX_RECORD_SPEED_KPH        50.0f

#endif
