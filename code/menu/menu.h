/*********************************************************************************************************************
* File: menu.h
* Brief: 菜单系统头文件。
* Author: JX116
* Note: 定义菜单项、菜单页、动态绘制回调以及菜单基础接口。
*********************************************************************************************************************/

#ifndef _menu_h_
#define _menu_h_

#include "zf_common_typedef.h"

typedef struct MenuPage MenuPage;

/* 单个菜单项：名称、执行函数、子页面 */
typedef struct {
    const char *name;
    void (*function)(void);
    MenuPage *sub_page;
} MenuItem;

/* 菜单页结构：标题、条目、数量、父页面、当前索引 */
struct MenuPage {
    const char *title;
    MenuItem *items;
    int num_items;
    MenuPage *parent;
    int current_index;
};

/* 动态绘制回调：用于在菜单区域外显示状态信息或图形 */
typedef void (*menu_dynamic_draw_t)(uint16 x, uint16 y, uint16 w, uint16 h);

/* 菜单主任务：处理输入与界面刷新 */
void    menu_task                       (void);
/* 菜单初始化 */
void    menu_init                       (void);
/* 设置动态绘制回调 */
void    menu_set_dynamic_draw           (menu_dynamic_draw_t callback);
/* 设置动态绘制区域 */
void    menu_set_dynamic_area           (uint16 x, uint16 y, uint16 w, uint16 h);
/* 设置动态区域是否自动清空 */
void    menu_set_dynamic_clear          (uint8 enable);

#endif
