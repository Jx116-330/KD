
/*********************************************************************************************************************
* TC377 Opensource Library 即（TC377 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
*
* 本文件是 TC377 开源库的一部分
*
* TC377 开源库 是免费软件
* 您可以根据自由软件基金会发布的 GPL（GNU General Public License，即 GNU通用公共许可证）的条款
* 即 GPL 的第3版（即 GPL3.0）或（您选择的）任何后来的版本，重新发布和/或修改它
*
* 本开源库的发布是希望它能发挥作用，但并未对其作任何的保证
* 甚至没有隐含的适销性或适合特定用途的保证
* 更多细节请参见 GPL
*
* 您应该在收到本开源库的同时收到一份 GPL 的副本
* 如果没有，请参阅<https://www.gnu.org/licenses/>
*
* 额外注明：
* 本开源库使用 GPL3.0 开源许可证协议 以上许可申明为译文版本
* 许可申明英文版在 libraries/doc 文件夹下的 GPL3_permission_statement.txt 文件中
* 许可证副本在 libraries 文件夹下 即该文件夹下的 LICENSE 文件
* 欢迎各位使用并传播本程序 但修改内容时必须保留逐飞科技的版权声明（即本声明）
*
* 文件名称          my_device_key
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          ADS v1.9.20
* 适用平台          TC377TP
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2022-11-03       pudding            first version
* 2023-04-28       pudding            增加中文注释说明
********************************************************************************************************************/

#include "MYKEY.h"
// 静态变量定义
static uint32               scanner_period = 0;                    // 按键扫描周期（毫秒）
static uint32               my_key_press_time[MY_KEY_NUMBER];      // 按键按下时间计数器数组
static my_key_state_enum    my_key_state[MY_KEY_NUMBER];           // 按键状态数组
static const gpio_pin_enum  my_key_index[MY_KEY_NUMBER] = MY_KEY_LIST; // 按键GPIO引脚映射表

// 全局变量定义
uint8 key_long_press_flag[MY_KEY_NUMBER] = {open_status};          // 长按标志位数组，初始为开启状态

/******************************************************************************
 * @brief : 按键状态扫描函数
 * @param : 无
 * @return: 无
 * @date  : 2025年3月19日
 * @author: sun
 * @说明  : 周期性调用此函数进行按键状态检测，支持消抖、短按和长按识别
 *          采用状态机设计，确保按键检测的稳定性和可靠性
 ******************************************************************************/
void my_key_scanner (void)
{
    uint8 i = 0;  // 循环计数器，用于遍历所有按键

    // 遍历所有按键进行状态检测
    for(i = 0; MY_KEY_NUMBER > i; i++)
    {
        // 检测当前按键是否处于按下状态（电平不等于释放电平）
        if(MY_KEY_RELEASE_LEVEL != gpio_get_level(my_key_index[i]))
        {
            // 按键按下状态处理
            my_key_press_time[i] ++;  // 增加按键按下时间计数器

            // 检查长按标志位状态
            if(key_long_press_flag[i] == close_status)
            {
                // 长按标志为关闭状态，说明按键刚被按下或已处理过长按事件
                my_key_state[i] = MY_KEY_RELEASE;  // 重置按键状态为释放
            }
            // 检测是否达到长按时间阈值
            else if((MY_KEY_LONG_PRESS_PERIOD / scanner_period <= my_key_press_time[i]) )
            {
                // 达到长按时间阈值，触发长按事件
                my_key_state[i] = MY_KEY_LONG_PRESS;   // 设置按键状态为长按
               // Key_Timer_ShortRing();                 // 触发蜂鸣器短鸣提示音
                key_long_press_flag[i] = close_status; // 设置长按标志为关闭，防止重复检测
            }
        }
        else
        {
            // 按键释放状态处理
            // 检查长按标志位是否为开启状态（表示按键刚释放）
            if(key_long_press_flag[i] == open_status)
            {
                // 检测是否为有效短按（非长按且超过消抖时间）
                if((MY_KEY_LONG_PRESS != my_key_state[i]) &&
                   (MY_KEY_MAX_SHOCK_PERIOD / scanner_period <= my_key_press_time[i]))
                {
                    // 满足短按条件：非长按状态且按下时间超过消抖时间
                    my_key_state[i] = MY_KEY_SHORT_PRESS;  // 设置按键状态为短按
                    //Key_Timer_ShortRing();                 // 触发蜂鸣器短鸣提示音
                }
                else
                {
                    // 不满足短按条件（可能是抖动或按下时间过短）
                    my_key_state[i] = MY_KEY_RELEASE;      // 设置按键状态为释放
                }
            }

            // 按键释放后的状态重置
            my_key_press_time[i] = 0;                  // 清零按键按下时间计数器
            key_long_press_flag[i] = open_status;      // 重置长按标志为开启状态，准备下一次检测
        }
    }
}

// 在主循环中检测按键事件
/******************************************************************************
 * @brief : 获取指定按键的当前状态
 * @param : key_n - 按键索引（MY_KEY_1, MY_KEY_2, MY_KEY_3, MY_KEY_4等）
 * @return: my_key_state_enum - 按键状态枚举值
 *          MY_KEY_RELEASE    - 按键释放状态
 *          MY_KEY_SHORT_PRESS - 按键短按状态
 *          MY_KEY_LONG_PRESS  - 按键长按状态
 * @date  : 2025年3月19日
 * @author: sun
 * @说明  : 该函数用于查询特定按键的当前状态，通常在事件处理中调用
 *          通过按键索引直接访问按键状态数组，返回对应的状态值
 *          调用此函数不会改变按键状态，仅用于状态查询
 ******************************************************************************/
my_key_state_enum my_key_get_state (my_key_index_enum key_n)
{
    // 通过按键索引直接返回对应的按键状态
    // key_n参数作为数组索引，从my_key_state数组中获取对应按键的状态
    return my_key_state[key_n];
}

/******************************************************************************
 * @brief : 清除指定按键的状态
 * @param : key_n - 按键索引（MY_KEY_1, MY_KEY_2, MY_KEY_3, MY_KEY_4等）
 * @return: 无
 * @date  : 2025年3月19日
 * @author: sun
 * @说明  : 该函数用于将指定按键的状态重置为释放状态
 *          通常在处理完按键事件后调用，防止按键状态被重复处理
 *          只清除状态，不影响按键扫描的时间计数器和长按标志位
 ******************************************************************************/
void my_key_clear_state (my_key_index_enum key_n)
{
    // 将指定按键的状态设置为释放状态
    // key_n参数作为数组索引，定位到my_key_state数组中的对应元素
    // MY_KEY_RELEASE表示按键处于释放/未按下状态
    my_key_state[key_n] = MY_KEY_RELEASE;
}

/******************************************************************************
 * @brief : 清除所有按键的状态
 * @param : 无
 * @return: 无
 * @date  : 2025年3月19日
 * @author: sun
 * @说明  : 该函数用于将所有按键的状态重置为释放状态
 *          通常在系统复位、模式切换或需要批量清除按键状态时调用
 *          批量操作版本，比逐个清除更高效
 ******************************************************************************/
void my_key_clear_all_state (void)
{
    // 遍历所有按键，从0到MY_KEY_NUMBER-1
    // MY_KEY_NUMBER是按键总数，在头文件中定义
    for (uint8 i = 0; i < MY_KEY_NUMBER; i++)
    {
        // 将当前按键的状态设置为释放状态
        // i作为数组索引，依次访问my_key_state数组中的每个元素
        // MY_KEY_RELEASE表示按键处于释放/未按下状态
        my_key_state[i] = MY_KEY_RELEASE;
    }
}
/******************************************************************************
 * @brief : 按键模块初始化函数
 * @param : period - 按键扫描周期（毫秒）
 * @return: 无
 * @date  : 2025年3月19日
 * @author: sun
 * @说明  : 该函数用于初始化按键模块，配置所有按键的GPIO引脚和状态
 *          必须在系统启动时调用一次，为按键扫描功能提供基础配置
 *          支持参数化扫描周期设置，提高模块的灵活性和可配置性
 ******************************************************************************/
void my_key_init (uint32 period)
{
    // 参数有效性检查：确保扫描周期大于0
    // zf_assert是逐飞科技库的断言宏，用于调试时检查条件
    // 如果period <= 0，程序会在此处停止并输出错误信息
    zf_assert(0 < period);

    // 定义循环计数器，用于遍历所有按键
    // uint8类型占用1字节，范围0-255，足够表示按键数量
    uint8 loop_temp = 0;

    // 遍历所有按键进行初始化配置
    // 循环条件：loop_temp从0开始，小于按键总数MY_KEY_NUMBER
    for(loop_temp = 0; MY_KEY_NUMBER > loop_temp; loop_temp++)
    {
        // 初始化当前按键的GPIO引脚配置
        // my_key_index[loop_temp]: 从按键映射表中获取当前按键的GPIO引脚号
        // GPI: 配置为通用输入模式
        // GPIO_HIGH: 初始电平为高电平
        // GPI_PULL_UP: 启用上拉电阻，提高抗干扰能力
        gpio_init(my_key_index[loop_temp], GPI, GPIO_HIGH, GPI_PULL_UP);

        // 初始化当前按键的状态为释放状态
        // 确保所有按键初始状态一致，避免系统启动时的误触发
        my_key_state[loop_temp] = MY_KEY_RELEASE;
    }

    // 设置按键扫描周期
    // 将传入的period参数赋值给全局静态变量scanner_period
    // 后续的按键扫描函数将使用此周期进行定时扫描
    scanner_period = period;
}

/******************************************************************************
 * @brief : 检测是否有按键被按下（任意按键）
 * @param : 无
 * @return: uint8 - 检测结果
 *          1: 有按键被按下（短按或长按）
 *          0: 没有按键被按下
 * @date  : 2025年3月19日
 * @author: sun
 * @说明  : 该函数用于快速检测系统中是否有任意按键处于按下状态
 *          提供了一种便捷的按键检测方式，无需指定具体按键
 *          适用于需要检测按键输入但不需要区分具体按键的场景
 ******************************************************************************/
uint8 My_Key_IfEnter(void)
{
    // 遍历所有按键，检查是否有按键处于按下状态
    // i: 循环计数器，从0到MY_KEY_NUMBER-1
    // MY_KEY_NUMBER: 按键总数，在头文件中定义
    for (uint8 i = 0; i < MY_KEY_NUMBER; i++)
    {
        // 检查当前按键的状态是否为非释放状态
        // my_key_get_state(): 获取指定按键的当前状态
        // (my_key_index_enum)i: 将循环计数器i强制转换为按键索引枚举类型
        // MY_KEY_RELEASE: 按键释放状态常量
        // != MY_KEY_RELEASE: 表示按键处于按下状态（短按或长按）
        if(my_key_get_state((my_key_index_enum)i) != MY_KEY_RELEASE)
            // 如果检测到有按键被按下，立即返回1
            // 采用短路返回策略，提高检测效率
            return 1;
    }

    // 遍历完所有按键后，如果没有检测到按键按下，返回0
    // 表示当前没有按键处于按下状态
    return 0;
}
