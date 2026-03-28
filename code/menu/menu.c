#include "menu.h"
#include "zf_device_ips200.h"
#include "zf_common_headfile.h"
#include "zf_driver_gpio.h"
#include "zf_driver_flash.h"
#include "MyKey.h"  // 【新增】引入MyKey头文件


typedef struct {
    float pid_p;
    float pid_i;
    float pid_d;
    uint8 threshold_mode;
    uint8 threshold_val;
    // 这是一个“暗号”，用来判断Flash里有没有存过数据
    // 如果读取出来不是这个数，说明是第一次运行，要加载默认值
    uint32_t magic_code;
} MyParams_t;
// 2. 定义全局变量实例
MyParams_t g_params;
// 宏定义：保存位置
#define FLASH_SECTOR    0   // 库文件要求填0
#define FLASH_PAGE      127 // 选最后一页(0-127)，防止覆盖代码
#define MAGIC_NUM       0x5A5A5A5A // 校验暗号
// 计算写入长度：(结构体大小 / 4)，如果有余数则 +1
// 因为 flash_write_page 的 len 参数单位是 uint32 (4字节)
#define SAVE_LEN        ((sizeof(MyParams_t) + 3) / 4)
// --------------------------------------------------------------------------
// 菜单项定义保持不变
// --- 功能函数区域 ---

//给断电保护里的值赋初始值
void params_set_default(void) {
    g_params.pid_p = 1.2f;        // 默认 P 设为 1.2
    g_params.pid_i = 0.01f;
    g_params.magic_code = 0xABCD1234; // 这是一个暗号，表示参数已经初始化过了
}
/**
 * @brief  通用参数调节函数
 * @param  ptr:    指向要修改的参数的指针 (例如 &g_params.pid_p)
 * @param  step:   调节的步长 (比如 0.1 或 1.0)
 * @param  min:    允许的最小值 (防止调成负数导致系统崩溃)
 * @param  max:    允许的最大值
 * @param  is_add: 1 为增加，0 为减少
 * adjust_param(参数, 步长, 最小值,最大值, 1增0减)
 */
void adjust_param(float *ptr, float step, float min, float max, uint8_t is_add) {
    if (is_add) {
        // 增加，并进行上限检查
        if (*ptr + step <= max) {
            *ptr += step;
        } else {
            *ptr = max; // 超过上限则等于上限
        }
    } else {
        // 减少，并进行下限检查
        if (*ptr - step >= min) {
            *ptr -= step;
        } else {
            *ptr = min; // 低于下限则等于下限
        }
    }
}
void action_test_inc(void) {
    adjust_param(&g_params.pid_p, 0.1f, 0.0f, 20.0f, 1);
}

// 【保存函数】：绑定到菜单项上
// --- [功能] 保存参数到 Flash ---
void Action_Save_Params(void) {
    ips200_show_string(30, 100, "Saving...");

    // 1. 打上校验标记，证明数据有效
    g_params.magic_code = MAGIC_NUM;

    // 2. 直接写入 (库函数内部会自动先擦除，不用我们管)
    // 注意：这里要把结构体指针强制转为 (uint32 *)
    flash_write_page(FLASH_SECTOR, FLASH_PAGE, (const uint32 *)&g_params, SAVE_LEN);

    system_delay_ms(500);
    ips200_show_string(30, 100, "Save Done! ");
    system_delay_ms(500);
}

// --- [功能] 上电加载参数 ---
void Init_Load_Params(void) {
    MyParams_t temp_read;

    // 1. 读取 Flash
    flash_read_page(FLASH_SECTOR, FLASH_PAGE, (uint32 *)&temp_read, SAVE_LEN);

    // 2. 检查校验码
    if (temp_read.magic_code == MAGIC_NUM) {
        // 校验正确，说明之前保存过，直接覆盖全局变量
        g_params = temp_read;
    } else {
        // 校验失败（第一次运行或Flash为空），加载默认值
        g_params.pid_p = 1.5f;
        g_params.pid_i = 0.02f;
        g_params.pid_d = 0.5f;
        g_params.threshold_mode = 0;
        g_params.threshold_val = 128;
    }
}

// --------------------------------------------------------------------------


// 1. GPS 子菜单
MenuItem gps_items[] = {
    {"1. Display Data", NULL, NULL},
    {"2. Save Point", NULL, NULL},
    {"3. Map", NULL, NULL},
    {"4. Manage Points", NULL, NULL},
    {"5. Fix Points", NULL, NULL},
};
MenuPage gps_menu = {
    "GPS",
    gps_items,
    sizeof(gps_items) / sizeof(MenuItem),
    NULL, 0
};

// 2. Camera 子菜单
MenuItem threshold_algo_items[] = {
    {"1. Manual", NULL, NULL},
    {"2. Otsu", NULL, NULL},
    {"3. Kittler", NULL, NULL},
    {"4. Sauvola", NULL, NULL},
};
MenuPage threshold_algo_menu = {
    "Threshold Mode",
    threshold_algo_items,
    sizeof(threshold_algo_items) / sizeof(MenuItem),
    NULL, 0
};
MenuItem camera_items[] = {
    {"1. Preview", NULL, NULL},
    {"2. Threshold", NULL, NULL},
    {"3. Color Preview", NULL, NULL},
    {"4. Threshold Mode", NULL, &threshold_algo_menu},
    {"5. Template Manage", NULL, NULL},
};
MenuPage camera_menu = {
    "Camera",
    camera_items,
    sizeof(camera_items) / sizeof(MenuItem),
    NULL, 0
};

// 3. PID 子菜单
MenuItem pid_items[] = {
    {"1. test", action_test_inc, NULL},
    {"2. see", NULL, NULL},
};

MenuPage pid_menu = {
    "pid_menu",
    pid_items,
    sizeof(pid_items) / sizeof(MenuItem),
    NULL, 0
};



// 主菜单
MenuItem main_items[] = {
    {"1. GPS", NULL, &gps_menu},
    {"2. Camera", NULL, &camera_menu},
    {"3. PID", NULL, &pid_menu},
    {"4. Save All",  Action_Save_Params,NULL},
};

MenuPage main_menu = {
    "xjx",
    main_items,
    sizeof(main_items) / sizeof(MenuItem),
    NULL,
    0
};

// --------------------------------------------------------------------------
// 【删除】旧的按键宏定义和静态变量 (KEY_NUMBER, KEY_LIST, key_scanner_internal等)
// 这些现在由 MyKey.c 接管
// --------------------------------------------------------------------------

static uint8 menu_needs_update = 1;
static uint16 current_font_color = RGB565_WHITE;
static int current_selection = 0;
static int last_selection = -1;
static uint8 menu_full_redraw = 1;
static menu_dynamic_draw_t menu_dynamic_draw_cb = NULL;
static uint16 menu_dynamic_x = 0;
static uint16 menu_dynamic_y = 0;
static uint16 menu_dynamic_w = 0;
static uint16 menu_dynamic_h = 0;
static uint8 menu_dynamic_clear_enable = 1;
MenuPage *current_page = &main_menu;

// 【删除】 key_scanner_internal
// 【删除】 key_get_state_internal
// 【删除】 key_clear_state_internal
// 【删除】 key_clear_all_state_internal
// 【删除】 key_init_internal

//-------------------------------------------------------------------------------------------------------------------
// 函数名称     menu_init
// 函数说明     菜单初始化
// 函数说明     void
// 返回参数     void
//-------------------------------------------------------------------------------------------------------------------
void menu_init(void)
{
    ips200_init(IPS200_TYPE_SPI);
    ips200_set_dir(IPS200_DEFAULT_DISPLAY_DIR);
    ips200_set_font(IPS200_DEFAULT_DISPLAY_FONT);
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    ips200_full(RGB565_BLACK);

    // 【修改】使用 MyKey 的初始化函数
    // 注意：这里的 10 代表扫描周期(period)，需要确保调用 my_key_scanner 的频率大致符合这个时间，
    // 或者 MyKey 内部是用这个值来计算消抖次数的。
    my_key_init(10);
    my_key_clear_all_state();

    current_page = &main_menu;
    current_page->parent = NULL;
    current_selection = 0;
    last_selection = -1;
    menu_needs_update = 1;
    menu_full_redraw = 1;
    current_font_color = RGB565_WHITE;
    menu_dynamic_draw_cb = NULL;
    menu_dynamic_x = 0;
    menu_dynamic_y = 0;
    menu_dynamic_w = 0;
    menu_dynamic_h = 0;
    menu_dynamic_clear_enable = 1;
    my_key_clear_all_state();
    // [新增] 必须加在最后：上电自动从Flash把保存的参数读回来
    Init_Load_Params();
    current_page = &main_menu;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数名称     menu_set_dynamic_draw
//-------------------------------------------------------------------------------------------------------------------
void menu_set_dynamic_draw(menu_dynamic_draw_t callback)
{
    menu_dynamic_draw_cb = callback;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数名称     menu_set_dynamic_area
//-------------------------------------------------------------------------------------------------------------------
void menu_set_dynamic_area(uint16 x, uint16 y, uint16 w, uint16 h)
{
    menu_dynamic_x = x;
    menu_dynamic_y = y;
    menu_dynamic_w = w;
    menu_dynamic_h = h;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数名称     menu_set_dynamic_clear
//-------------------------------------------------------------------------------------------------------------------
void menu_set_dynamic_clear(uint8 enable)
{
    menu_dynamic_clear_enable = enable ? 1 : 0;
}

//-------------------------------------------------------------------------------------------------------------------
// 绘图辅助函数 (ips200_fill_rect, show_string_fit) 保持不变
//-------------------------------------------------------------------------------------------------------------------
static void ips200_fill_rect(uint16 x_start, uint16 y_start, uint16 x_end, uint16 y_end, uint16 color)
{
    if (x_start >= ips200_width_max) x_start = ips200_width_max - 1;
    if (x_end >= ips200_width_max) x_end = ips200_width_max - 1;
    if (y_start >= ips200_height_max) y_start = ips200_height_max - 1;
    if (y_end >= ips200_height_max) y_end = ips200_height_max - 1;
    if (x_start > x_end) { uint16 t = x_start; x_start = x_end; x_end = t; }
    if (y_start > y_end) { uint16 t = y_start; y_start = y_end; y_end = t; }
    for (uint16 y = y_start; y <= y_end; y++) {
        ips200_draw_line(x_start, y, x_end, y, color);
    }
}

static void show_string_fit(uint16 x, uint16 y, const char *s) {
    int max_chars = (int)((ips200_width_max - 1 - x) / 8);
    if (max_chars <= 0) return;
    char buf[64];
    int i = 0;
    while (i < max_chars && s[i] != '\0' && i < (int)sizeof(buf) - 1) {
        buf[i] = s[i];
        i++;
    }
    buf[i] = '\0';
    ips200_show_string(x, y, buf);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数名称     menu_task
// 函数说明     菜单主任务
//-------------------------------------------------------------------------------------------------------------------
void menu_task(void) {
    // 【修改】调用 MyKey 的扫描函数
    // 注意：如果你的工程已经在定时器中断里调用了 my_key_scanner()，则这里不需要再调用。
    // 如果没有在中断里调用，必须保留这行代码，且确保 menu_task 被调用的频率足够快(例如10ms一次)。
    system_delay_ms(10); // 强行延时10ms，让扫描间隔符合 my_key_init(10) 的设定
    my_key_scanner();

    // 【修改】按键处理逻辑替换为 MyKey 接口

    // Up Key (原 KEY_1 -> 现 MY_KEY_1)
    if (my_key_get_state(MY_KEY_1) == MY_KEY_SHORT_PRESS) {
            my_key_clear_state(MY_KEY_1);

            // 修改逻辑：如果不是第一个，就减1；如果是第一个，就跳到最后一个
            if (current_selection > 0) {
                current_selection--;
            } else {
                // 跳到最后一个 (总数 - 1)
                current_selection = current_page->num_items - 1;
            }
            menu_needs_update = 1; // 标记需要刷新
        }

    // Down Key (原 KEY_2 -> 现 MY_KEY_2)
    if (my_key_get_state(MY_KEY_2) == MY_KEY_SHORT_PRESS) {
            my_key_clear_state(MY_KEY_2);

            // 修改逻辑：如果不是最后一个，就加1；如果是最后一个，就跳回第0个
            if (current_selection < current_page->num_items - 1) {
                current_selection++;
            } else {
                // 跳回第一个
                current_selection = 0;
            }
            menu_needs_update = 1; // 标记需要刷新
        }
    // Enter Key (原 KEY_3 -> 现 MY_KEY_3)
    if (my_key_get_state(MY_KEY_3) == MY_KEY_SHORT_PRESS) {
        my_key_clear_state(MY_KEY_3);
        MenuItem *item = &current_page->items[current_selection];
        if (item->function) {
            item->function();
            menu_full_redraw = 1;
        } else if (item->sub_page) {
            item->sub_page->parent = current_page;
            current_page = item->sub_page;
            current_selection = 0;
            menu_needs_update = 1;
            menu_full_redraw = 1;
        }
    }

    // Back Key (原 KEY_4 -> 现 MY_KEY_4)
    // 注意：原逻辑是短按返回，如果需要长按返回，请把 MY_KEY_SHORT_PRESS 改为 MY_KEY_LONG_PRESS
    if (my_key_get_state(MY_KEY_4) == MY_KEY_SHORT_PRESS) {
        my_key_clear_state(MY_KEY_4);
        if (current_page->parent) {
            current_page = current_page->parent;
            current_selection = 0;
            menu_needs_update = 1;
            menu_full_redraw = 1;
        }
    }

    // -------------------------------------------------------------------
    // 以下绘图逻辑保持原样，未修改
    // -------------------------------------------------------------------

    // Draw
    if (menu_full_redraw) {
        ips200_full(RGB565_BLACK);

        ips200_set_color(RGB565_YELLOW, RGB565_BLACK);
        show_string_fit(10, 10, current_page->title);
        ips200_draw_line(0, 30, ips200_width_max - 1, 30, RGB565_GRAY);

        for (int i = 0; i < current_page->num_items; i++) {
            uint16 y_pos = 40 + i * 20;
            if (y_pos + 16 >= ips200_height_max) break;
            if (i == current_selection) {
                ips200_fill_rect(5, y_pos - 2, ips200_width_max - 5, y_pos + 16, current_font_color);
                ips200_set_color(RGB565_BLACK, current_font_color);
                ips200_show_string(10, y_pos, current_page->items[i].name);
            } else {
                ips200_set_color(current_font_color, RGB565_BLACK);
                ips200_show_string(10, y_pos, current_page->items[i].name);
            }
        }

        ips200_set_color(RGB565_GRAY, RGB565_BLACK);
        ips200_show_string(5, ips200_height_max - 20, "K1:Up K2:Dn K3:Ok K4:Bk");

        last_selection = current_selection;
        menu_full_redraw = 0;
        menu_needs_update = 0;
    } else if (menu_needs_update) {
        int prev = last_selection;
        int curr = current_selection;
        if (prev >= 0 && prev < current_page->num_items) {
            uint16 y_prev = 40 + prev * 20;
            if (y_prev + 16 < ips200_height_max) {
                ips200_fill_rect(5, y_prev - 2, ips200_width_max - 5, y_prev + 16, RGB565_BLACK);
                ips200_set_color(current_font_color, RGB565_BLACK);
                ips200_show_string(10, y_prev, current_page->items[prev].name);
            }
        }
        if (curr >= 0 && curr < current_page->num_items) {
            uint16 y_curr = 40 + curr * 20;
            if (y_curr + 16 < ips200_height_max) {
                ips200_fill_rect(5, y_curr - 2, ips200_width_max - 5, y_curr + 16, current_font_color);
                ips200_set_color(RGB565_BLACK, current_font_color);
                ips200_show_string(10, y_curr, current_page->items[curr].name);
            }
        }
        last_selection = current_selection;
        menu_needs_update = 0;
    }

    if (menu_dynamic_draw_cb && menu_dynamic_w && menu_dynamic_h) {
        if (menu_dynamic_clear_enable) {
            uint16 x_end = (uint16)(menu_dynamic_x + menu_dynamic_w - 1);
            uint16 y_end = (uint16)(menu_dynamic_y + menu_dynamic_h - 1);
            ips200_fill_rect(menu_dynamic_x, menu_dynamic_y, x_end, y_end, RGB565_BLACK);
        }
        menu_dynamic_draw_cb(menu_dynamic_x, menu_dynamic_y, menu_dynamic_w, menu_dynamic_h);
    }
}







