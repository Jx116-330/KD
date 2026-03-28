/**
 * @file path_recorder.c
 * @brief 路径记录模块实现文件
 * @author 贾维斯
 * @date 2026-03-21
 * @version 1.0
 *
 * @details 本模块实现GPS路径记录功能，用于迷宫导航
 *          注意：嵌入式系统中数学函数可能需要特殊处理
 */

#include "path_recorder.h"
#include <string.h>  // 添加memset等函数支持
#include <math.h>    // 数学函数，嵌入式系统中可能需要浮点支持

// 时间函数宏定义（适配TC387开源库）
#define get_current_time_ms()   system_getval_ms()

//--------------------------------------------------------------------------------------------------
// 全局变量定义
//--------------------------------------------------------------------------------------------------

/** @brief 路径数据全局变量 */
path_data_t path_data;

//--------------------------------------------------------------------------------------------------
// 静态函数声明
//--------------------------------------------------------------------------------------------------

static uint8 is_valid_gps_fix(void);
static void update_path_statistics(path_point_t* new_point);

//--------------------------------------------------------------------------------------------------
// 函数实现
//--------------------------------------------------------------------------------------------------

/**
 * @brief 路径记录模块初始化
 */
void path_recorder_init(void)
{
    // 初始化路径数据结构
    memset(&path_data, 0, sizeof(path_data_t));
    
    path_data.state = PATH_STATE_IDLE;
    path_data.point_count = 0;
    path_data.current_index = 0;
    path_data.last_record_time = 0;
    path_data.last_latitude = 0.0;
    path_data.last_longitude = 0.0;
    path_data.total_distance = 0.0f;
    path_data.total_time = 0;
    
    // 初始化路径点数组
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
    
    // 初始化完成
    // 这里可以添加Flash初始化等操作
}

/**
 * @brief 检查GPS定位是否有效
 * @return 有效返回1，无效返回0
 */
static uint8 is_valid_gps_fix(void)
{
    // 检查GPS数据是否有效
    if (gnss.state != 1)  // 定位无效
    {
        return 0;
    }
    
    // 检查卫星数量
    if (gnss.satellite_used < 4)  // 至少需要4颗卫星
    {
        return 0;
    }
    
    // 检查经纬度是否在合理范围内
    if (gnss.latitude == 0.0 || gnss.longitude == 0.0)
    {
        return 0;
    }
    
    // 检查经纬度范围（中国大致范围）
    if (gnss.latitude < 3.0 || gnss.latitude > 54.0 ||  // 中国纬度范围
        gnss.longitude < 73.0 || gnss.longitude > 136.0) // 中国经度范围
    {
        return 0;  // 超出合理范围
    }
    
    return 1;
}

/**
 * @brief 开始记录路径
 * @return 成功返回1，失败返回0
 */
uint8 path_recorder_start(void)
{
    if (path_data.state != PATH_STATE_IDLE && 
        path_data.state != PATH_STATE_PAUSED)
    {
        return 0;  // 状态不正确
    }
    
    // 检查GPS是否有效
    if (!is_valid_gps_fix())
    {
        return 0;  // GPS无效
    }
    
    if (path_data.state == PATH_STATE_IDLE)
    {
        // 新开始记录，清除旧数据
        path_recorder_clear();
        
        // 记录第一个点
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
        // 恢复记录
        path_data.state = PATH_STATE_RECORDING;
        return 1;
    }
    
    return 0;
}

/**
 * @brief 暂停记录路径
 */
void path_recorder_pause(void)
{
    if (path_data.state == PATH_STATE_RECORDING)
    {
        path_data.state = PATH_STATE_PAUSED;
    }
}

/**
 * @brief 恢复记录路径
 */
void path_recorder_resume(void)
{
    if (path_data.state == PATH_STATE_PAUSED)
    {
        path_data.state = PATH_STATE_RECORDING;
    }
}

/**
 * @brief 停止记录路径
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
 * @brief 清除所有路径数据
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
    
    // 清除路径点数据
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
 * @brief 检查是否应该记录当前点
 * @param lat 当前纬度
 * @param lon 当前经度
 * @param current_time 当前时间
 * @return 应该记录返回1，否则返回0
 */
uint8 path_recorder_should_record(float lat, float lon, uint32 current_time)
{
    // 检查时间间隔
    if (current_time - path_data.last_record_time < MIN_RECORD_INTERVAL)
    {
        return 0;
    }
    
    // 检查距离变化
    if (path_data.point_count > 0)
    {
        float distance = path_recorder_calculate_distance(
            lat, lon, 
            path_data.last_latitude, path_data.last_longitude);
        
        if (distance < MIN_RECORD_DISTANCE)
        {
            return 0;  // 移动距离太小
        }
    }
    
    return 1;
}

/**
 * @brief 添加路径点
 * @param point 路径点数据
 * @return 成功返回1，失败返回0
 */
uint8 path_recorder_add_point(path_point_t* point)
{
    if (path_data.point_count >= MAX_PATH_POINTS)
    {
        return 0;  // 路径已满
    }
    
    if (point == NULL)
    {
        return 0;  // 无效指针
    }
    
    // 复制数据到路径数组
    path_data.points[path_data.point_count] = *point;
    path_data.point_count++;
    
    // 更新统计信息
    update_path_statistics(point);
    
    return 1;
}

/**
 * @brief 更新路径统计信息
 * @param new_point 新添加的路径点
 */
static void update_path_statistics(path_point_t* new_point)
{
    if (path_data.point_count > 1)
    {
        // 计算与前一点的距离
        path_point_t* prev_point = &path_data.points[path_data.point_count - 2];
        float distance = path_recorder_calculate_distance(
            new_point->latitude, new_point->longitude,
            prev_point->latitude, prev_point->longitude);
        
        path_data.total_distance += distance;
        
        // 计算时间差
        uint32 time_diff = new_point->timestamp - prev_point->timestamp;
        path_data.total_time += time_diff;
    }
}

/**
 * @brief 路径记录主任务（需要周期性调用）
 */
void path_recorder_task(void)
{
    static uint32 last_task_time = 0;
    uint32 current_time = get_current_time_ms();
    
    // 限制任务执行频率（每100ms执行一次）
    if (current_time - last_task_time < 100)
    {
        return;
    }
    last_task_time = current_time;
    
    // 检查当前状态
    if (path_data.state != PATH_STATE_RECORDING)
    {
        return;
    }
    
    // 检查GPS是否有效
    if (!is_valid_gps_fix())
    {
        return;
    }
    
    // 检查是否应该记录当前点
    if (!path_recorder_should_record((float)gnss.latitude, (float)gnss.longitude, current_time))
    {
        return;
    }
    
    // 创建新的路径点
    path_point_t new_point;
    new_point.latitude = (float)gnss.latitude;
    new_point.longitude = (float)gnss.longitude;
    new_point.timestamp = current_time;
    new_point.speed = gnss.speed;
    new_point.direction = gnss.direction;
    new_point.satellite_count = gnss.satellite_used;
    new_point.fix_quality = gnss.state;
    
    // 添加路径点
    if (path_recorder_add_point(&new_point))
    {
        // 更新最后记录的信息
        path_data.last_latitude = (float)gnss.latitude;
        path_data.last_longitude = (float)gnss.longitude;
        path_data.last_record_time = current_time;
    }
}

/**
 * @brief 计算两点间距离（简化版，适用于短距离）
 * @param lat1 点1纬度
 * @param lon1 点1经度
 * @param lat2 点2纬度
 * @param lon2 点2经度
 * @return 距离（米）
 * 
 * @note 使用简化公式，适用于短距离计算（<10km）
 *       精度足够用于路径记录过滤
 */
float path_recorder_calculate_distance(float lat1, float lon1, float lat2, float lon2)
{
    // 简化计算：使用平面近似，适用于短距离
    const float METERS_PER_DEGREE_LAT = 111319.9f;  // 纬度每度对应的米数
    float avg_lat = (lat1 + lat2) * 0.5f;
    float METERS_PER_DEGREE_LON = 111319.9f * cosf(avg_lat * 3.14159265f / 180.0f);
    
    float dlat = lat2 - lat1;  // 纬度差（度）
    float dlon = lon2 - lon1;  // 经度差（度）
    
    // 计算距离（米） - 使用float运算
    float distance = sqrtf(dlat * dlat * METERS_PER_DEGREE_LAT * METERS_PER_DEGREE_LAT + 
                          dlon * dlon * METERS_PER_DEGREE_LON * METERS_PER_DEGREE_LON);
    
    return distance;
}

/**
 * @brief 开始路径回放
 */
void path_recorder_start_playback(void)
{
    if (path_data.point_count == 0)
    {
        return;  // 没有路径数据
    }
    
    path_data.state = PATH_STATE_PLAYBACK;
    path_data.current_index = 0;
}

/**
 * @brief 停止路径回放
 */
void path_recorder_stop_playback(void)
{
    if (path_data.state == PATH_STATE_PLAYBACK)
    {
        path_data.state = PATH_STATE_COMPLETED;
    }
}

/**
 * @brief 获取下一个回放点
 * @return 成功返回路径点指针，失败返回NULL
 */
path_point_t* path_recorder_get_next_point(void)
{
    if (path_data.state != PATH_STATE_PLAYBACK)
    {
        return NULL;
    }
    
    if (path_data.current_index >= path_data.point_count)
    {
        return NULL;  // 已到末尾
    }
    
    path_point_t* point = &path_data.points[path_data.current_index];
    path_data.current_index++;
    
    return point;
}

/**
 * @brief 获取上一个回放点
 * @return 成功返回路径点指针，失败返回NULL
 */
path_point_t* path_recorder_get_prev_point(void)
{
    if (path_data.state != PATH_STATE_PLAYBACK)
    {
        return NULL;
    }
    
    if (path_data.current_index == 0)
    {
        return NULL;  // 已在开头
    }
    
    path_data.current_index--;
    return &path_data.points[path_data.current_index];
}

/**
 * @brief 获取路径统计信息
 * @param[out] distance 总距离（米）
 * @param[out] time 总时间（秒）
 * @param[out] avg_speed 平均速度（km/h）
 */
void path_recorder_get_stats(float* distance, uint32* time, float* avg_speed)
{
    if (distance != NULL)
    {
        *distance = path_data.total_distance;
    }
    
    if (time != NULL)
    {
        *time = path_data.total_time / 1000;  // 转换为秒
    }
    
    if (avg_speed != NULL)
    {
        if (path_data.total_time > 0)
        {
            // 计算平均速度（m/s -> km/h）
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
 * @brief 保存路径到Flash（简化版，实际需要实现Flash驱动）
 * @param slot 存储槽位（0-9）
 * @return 成功返回1，失败返回0
 */
uint8 path_recorder_save_to_flash(uint8 slot)
{
    // TODO: 实现Flash存储
    // 这里需要根据具体的Flash驱动来实现
    
    // 简化实现：只返回成功
    (void)slot;  // 避免未使用参数警告
    return 1;
}

/**
 * @brief 从Flash加载路径（简化版）
 * @param slot 存储槽位（0-9）
 * @return 成功返回1，失败返回0
 */
uint8 path_recorder_load_from_flash(uint8 slot)
{
    // TODO: 实现Flash加载
    // 这里需要根据具体的Flash驱动来实现
    
    // 简化实现：只返回成功
    (void)slot;  // 避免未使用参数警告
    return 1;
}

/**
 * @brief 获取路径状态
 * @return 当前路径状态
 */
path_state_enum path_recorder_get_state(void)
{
    return path_data.state;
}

/**
 * @brief 获取当前路径点数
 * @return 路径点数
 */
uint16 path_recorder_get_point_count(void)
{
    return path_data.point_count;
}

/**
 * @brief 获取路径是否已满
 * @return 已满返回1，否则返回0
 */
uint8 path_recorder_is_full(void)
{
    return (path_data.point_count >= MAX_PATH_POINTS) ? 1 : 0;
}
