/**
 * @file path_config.h
 * @brief Cleaned module header to avoid encoding issues.
 * @author JX116
 */

#ifndef _PATH_CONFIG_H_
#define _PATH_CONFIG_H_

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

#define PATH_DEBUG_ENABLED          1

#define PATH_PERF_STATS_ENABLED     1

#define PATH_ASSERT_ENABLED         1

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

#define MAX_PATH_POINTS             200

#define MIN_RECORD_DISTANCE         0.5f

#define MIN_RECORD_INTERVAL_MS      100

#define GPS_MIN_SATELLITES          4

#define GPS_MAX_HDOP                5.0f

#define MAX_RECORD_SPEED_KPH        50.0f

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

#define FILTER_BY_SPEED             1

#define FILTER_BY_HDOP              1

#define FILTER_POSITION_JUMP        1

#define MAX_POSITION_JUMP_M         10.0f

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

#define USE_COMPRESSED_STORAGE      1

#define COMPRESSION_PRECISION_M     0.1f

#define USE_CIRCULAR_BUFFER         0

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

#define DISPLAY_AREA_X              10

#define DISPLAY_AREA_Y              10

#define DISPLAY_AREA_WIDTH          300

#define DISPLAY_AREA_HEIGHT         220

#define DISPLAY_REFRESH_INTERVAL_MS 200

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

#define PATH_LINE_COLOR             RGB565_BLUE

#define PATH_POINT_COLOR            RGB565_RED

#define CURRENT_POS_COLOR           RGB565_GREEN

#define TEXT_COLOR                  RGB565_WHITE

#define BACKGROUND_COLOR            RGB565_BLACK

#define WARNING_COLOR               RGB565_YELLOW

#define ERROR_COLOR                 RGB565_RED

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

#define TASK_EXEC_INTERVAL_MS       10

#define GPS_CHECK_INTERVAL_MS       100

#define STATS_UPDATE_INTERVAL_MS    1000

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

#define MAX_RETRY_COUNT             3

#define ERROR_RECOVERY_TIME_MS      1000

#define WATCHDOG_ENABLED            1

#define WATCHDOG_TIMEOUT_MS         5000

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

#ifndef M_PI
#define M_PI                        3.14159265358979323846f
#endif

#define DEG_TO_RAD                  (M_PI / 180.0f)

#define RAD_TO_DEG                  (180.0f / M_PI)

#define EARTH_RADIUS_M              6371000.0f

#define METERS_PER_DEGREE_LAT       111319.9f

#define FLOAT_EPSILON               1e-6f

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

#define SYSTEM_CLOCK_HZ             200000000  // 200MHz

#define TIMER_RESOLUTION_US         10

#define HW_FPU_ENABLED              1

#endif /* _PATH_CONFIG_H_ */
