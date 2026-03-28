/**
 * @file path_config.h
 * @brief 路径记录模块配置头文件
 * @author 贾维斯
 * @date 2026-03-21
 * @version 2.0
 *
 * @details 集中管理所有配置参数，便于调整和优化
 */

#ifndef _PATH_CONFIG_H_
#define _PATH_CONFIG_H_

//--------------------------------------------------------------------------------------------------
// 系统配置
//--------------------------------------------------------------------------------------------------

/** @brief 启用调试输出 */
#define PATH_DEBUG_ENABLED          1

/** @brief 启用性能统计 */
#define PATH_PERF_STATS_ENABLED     1

/** @brief 启用断言检查 */
#define PATH_ASSERT_ENABLED         1

//--------------------------------------------------------------------------------------------------
// 路径记录配置
//--------------------------------------------------------------------------------------------------

/** @brief 最大路径点数（根据可用内存调整） */
#define MAX_PATH_POINTS             200

/** @brief 最小记录距离（米） */
#define MIN_RECORD_DISTANCE         0.5f

/** @brief 最小记录时间间隔（毫秒） */
#define MIN_RECORD_INTERVAL_MS      100

/** @brief GPS最小有效卫星数 */
#define GPS_MIN_SATELLITES          4

/** @brief GPS最大HDOP值（水平精度因子） */
#define GPS_MAX_HDOP                5.0f

/** @brief 最大记录速度（km/h，过滤异常值） */
#define MAX_RECORD_SPEED_KPH        50.0f

//--------------------------------------------------------------------------------------------------
// 路径过滤配置
//--------------------------------------------------------------------------------------------------

/** @brief 启用速度过滤 */
#define FILTER_BY_SPEED             1

/** @brief 启用HDOP过滤 */
#define FILTER_BY_HDOP              1

/** @brief 启用位置跳跃过滤 */
#define FILTER_POSITION_JUMP        1

/** @brief 最大位置跳跃距离（米） */
#define MAX_POSITION_JUMP_M         10.0f

//--------------------------------------------------------------------------------------------------
// 内存优化配置
//--------------------------------------------------------------------------------------------------

/** @brief 使用压缩存储（减少内存使用） */
#define USE_COMPRESSED_STORAGE      1

/** @brief 压缩精度（米） */
#define COMPRESSION_PRECISION_M     0.1f

/** @brief 启用循环缓冲区 */
#define USE_CIRCULAR_BUFFER         0

//--------------------------------------------------------------------------------------------------
// 显示配置
//--------------------------------------------------------------------------------------------------

/** @brief 默认显示区域X坐标 */
#define DISPLAY_AREA_X              10

/** @brief 默认显示区域Y坐标 */
#define DISPLAY_AREA_Y              10

/** @brief 默认显示区域宽度 */
#define DISPLAY_AREA_WIDTH          300

/** @brief 默认显示区域高度 */
#define DISPLAY_AREA_HEIGHT         220

/** @brief 显示刷新间隔（毫秒） */
#define DISPLAY_REFRESH_INTERVAL_MS 200

//--------------------------------------------------------------------------------------------------
// 颜色配置
//--------------------------------------------------------------------------------------------------

/** @brief 路径线颜色 */
#define PATH_LINE_COLOR             RGB565_BLUE

/** @brief 路径点颜色 */
#define PATH_POINT_COLOR            RGB565_RED

/** @brief 当前位置颜色 */
#define CURRENT_POS_COLOR           RGB565_GREEN

/** @brief 文本颜色 */
#define TEXT_COLOR                  RGB565_WHITE

/** @brief 背景颜色 */
#define BACKGROUND_COLOR            RGB565_BLACK

/** @brief 警告颜色 */
#define WARNING_COLOR               RGB565_YELLOW

/** @brief 错误颜色 */
#define ERROR_COLOR                 RGB565_RED

//--------------------------------------------------------------------------------------------------
// 性能配置
//--------------------------------------------------------------------------------------------------

/** @brief 任务执行间隔（毫秒） */
#define TASK_EXEC_INTERVAL_MS       10

/** @brief GPS检查间隔（毫秒） */
#define GPS_CHECK_INTERVAL_MS       100

/** @brief 统计更新间隔（毫秒） */
#define STATS_UPDATE_INTERVAL_MS    1000

//--------------------------------------------------------------------------------------------------
// 错误处理配置
//--------------------------------------------------------------------------------------------------

/** @brief 最大重试次数 */
#define MAX_RETRY_COUNT             3

/** @brief 错误恢复时间（毫秒） */
#define ERROR_RECOVERY_TIME_MS      1000

/** @brief 启用看门狗 */
#define WATCHDOG_ENABLED            1

/** @brief 看门狗超时时间（毫秒） */
#define WATCHDOG_TIMEOUT_MS         5000

//--------------------------------------------------------------------------------------------------
// 数学常量
//--------------------------------------------------------------------------------------------------

/** @brief 圆周率 */
#ifndef M_PI
#define M_PI                        3.14159265358979323846f
#endif

/** @brief 度到弧度转换系数 */
#define DEG_TO_RAD                  (M_PI / 180.0f)

/** @brief 弧度到度转换系数 */
#define RAD_TO_DEG                  (180.0f / M_PI)

/** @brief 地球半径（米） */
#define EARTH_RADIUS_M              6371000.0f

/** @brief 纬度每度对应的米数 */
#define METERS_PER_DEGREE_LAT       111319.9f

/** @brief 浮点比较精度 */
#define FLOAT_EPSILON               1e-6f

//--------------------------------------------------------------------------------------------------
// 硬件相关配置
//--------------------------------------------------------------------------------------------------

/** @brief 系统时钟频率（Hz） */
#define SYSTEM_CLOCK_HZ             200000000  // 200MHz

/** @brief 定时器分辨率（微秒） */
#define TIMER_RESOLUTION_US         10

/** @brief 启用硬件浮点单元 */
#define HW_FPU_ENABLED              1

#endif /* _PATH_CONFIG_H_ */
