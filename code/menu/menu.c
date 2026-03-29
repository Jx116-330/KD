/*********************************************************************************************************************
* File: menu.c
* Brief: 菜单系统实现文件。
* Author: JX116
* Note: 负责菜单绘制、编码器/按键交互、GPS子菜单状态显示与参数保存逻辑。
*********************************************************************************************************************/

#include "menu.h"
#include "zf_device_ips200.h"
#include "zf_common_headfile.h"
#include "zf_driver_gpio.h"
#include "zf_driver_flash.h"
#include "path_config.h"
#include "MyKey.h"
#include "MyEncoder.h"


/* 参数结构体：用于保存 PID 和阈值等菜单参数 */
typedef struct {
    float pid_p;
    float pid_i;
    float pid_d;
    uint8 threshold_mode;
    uint8 threshold_val;
    uint32_t magic_code;
} MyParams_t;

MyParams_t g_params;

/* Flash 参数保存位置定义 */
#define FLASH_SECTOR    0
#define FLASH_PAGE      127
#define MAGIC_NUM       0x5A5A5A5A
#define SAVE_LEN        ((sizeof(MyParams_t) + 3) / 4)

/* 菜单显示与状态控制变量 */
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
static uint8 gps_display_mode = 0;
static char gps_status_hint[64] = "";
MenuPage *current_page = NULL;

static void ips200_fill_rect(uint16 x_start, uint16 y_start, uint16 x_end, uint16 y_end, uint16 color);
static void show_string_fit(uint16 x, uint16 y, const char *s);
static void show_string_fit_width(uint16 x, uint16 y, uint16 max_width, const char *s);
static void show_string_fit_width_pad(uint16 x, uint16 y, uint16 max_width, const char *s);
static void gps_enter_view(uint8 mode);
static void gps_exit_view(void);
static void gps_draw_page_header(const char *title);
static void gps_draw_data_page(void);
static void gps_draw_map_page(void);

static uint8 gps_menu_gnss_ready(void)
{
    return ((gnss.state == 1) &&
            (gnss.satellite_used >= GPS_MIN_SATELLITES) &&
            (gnss.speed <= MAX_RECORD_SPEED_KPH) &&
            (gnss.latitude != 0.0) &&
            (gnss.longitude != 0.0));
}

/* 设置默认参数，防止首次上电或 Flash 无效时没有有效配置 */
void params_set_default(void)
{
    g_params.pid_p = 1.2f;
    g_params.pid_i = 0.01f;
    g_params.pid_d = 0.5f;
    g_params.threshold_mode = 0;
    g_params.threshold_val = 128;
    g_params.magic_code = MAGIC_NUM;
}

/* 通用参数增减函数：按步长调整，并限制上下界 */
void adjust_param(float *ptr, float step, float min, float max, uint8_t is_add)
{
    if (is_add)
    {
        if (*ptr + step <= max)
        {
            *ptr += step;
        }
        else
        {
            *ptr = max;
        }
    }
    else
    {
        if (*ptr - step >= min)
        {
            *ptr -= step;
        }
        else
        {
            *ptr = min;
        }
    }
}

void action_test_inc(void)
{
    adjust_param(&g_params.pid_p, 0.1f, 0.0f, 20.0f, 1);
}

/* 将当前菜单参数写入 Flash */
void Action_Save_Params(void)
{
    ips200_show_string(30, 100, "Saving...");
    g_params.magic_code = MAGIC_NUM;
    flash_write_page(FLASH_SECTOR, FLASH_PAGE, (const uint32 *)&g_params, SAVE_LEN);
    system_delay_ms(500);
    ips200_show_string(30, 100, "Save Done!");
    system_delay_ms(500);
}

/* 上电时从 Flash 读取参数；若无效则加载默认值 */
void Init_Load_Params(void)
{
    MyParams_t temp_read;

    flash_read_page(FLASH_SECTOR, FLASH_PAGE, (uint32 *)&temp_read, SAVE_LEN);

    if (temp_read.magic_code == MAGIC_NUM)
    {
        g_params = temp_read;
    }
    else
    {
        params_set_default();
    }
}

/* GPS 动态区域绘制：根据模式显示状态信息或轨迹图 */
static void gps_enter_view(uint8 mode)
{
    gps_display_mode = mode;
    my_key_clear_state(MY_KEY_1);
    my_key_clear_state(MY_KEY_4);
    menu_set_dynamic_draw(NULL);
    menu_set_dynamic_area(0, 0, 0, 0);
    menu_set_dynamic_clear(0);
    menu_needs_update = 1;
    menu_full_redraw = 1;
}

static void gps_exit_view(void)
{
    gps_display_mode = 0;
    menu_set_dynamic_draw(NULL);
    menu_set_dynamic_area(0, 0, 0, 0);
    menu_set_dynamic_clear(0);
    menu_needs_update = 1;
    menu_full_redraw = 1;
}

static void gps_draw_page_header(const char *title)
{
    ips200_set_color(RGB565_YELLOW, RGB565_BLACK);
    show_string_fit(10, 10, title);
    ips200_draw_line(0, 30, ips200_width_max - 1, 30, RGB565_GRAY);
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
}

static void gps_draw_data_page(void)
{
    static uint32 last_refresh_ms = 0;
    char line0[64];
    char line1[64];
    char line2[64];
    char line3[64];
    char line4[64];
    uint32 now_ms = system_getval_ms();

    if (!menu_full_redraw && (now_ms - last_refresh_ms < 200U))
    {
        return;
    }

    last_refresh_ms = now_ms;
    if (menu_full_redraw)
    {
        ips200_fill_rect(0, 32, ips200_width_max - 1, ips200_height_max - 1, RGB565_BLACK);
        gps_draw_page_header("GPS Data");
        show_string_fit_width(5, (uint16)(ips200_height_max - 20U), (uint16)(ips200_width_max - 10U), "K1 LONG Exit");
    }

    sprintf(line0, "Fix:%d Sat:%d", (int)gnss.state, (int)gnss.satellite_used);
    sprintf(line1, "Lat:%.6f", gnss.latitude);
    sprintf(line2, "Lon:%.6f", gnss.longitude);
    sprintf(line3, "Speed:%.2fkm/h", gnss.speed);
    if (gps_status_hint[0] != '\0')
    {
        sprintf(line4, "%s", gps_status_hint);
    }
    else
    {
        sprintf(line4, "Pts:%d", path_recorder_get_point_count());
    }

    show_string_fit_width_pad(10, 60, (uint16)(ips200_width_max - 20U), line0);
    show_string_fit_width_pad(10, 84, (uint16)(ips200_width_max - 20U), line1);
    show_string_fit_width_pad(10, 108, (uint16)(ips200_width_max - 20U), line2);
    show_string_fit_width_pad(10, 132, (uint16)(ips200_width_max - 20U), line3);
    show_string_fit_width_pad(10, 156, (uint16)(ips200_width_max - 20U), line4);
    menu_full_redraw = 0;
}

static void gps_draw_map_page(void)
{
    static uint32 last_refresh_ms = 0;
    uint16 map_x = 5U;
    uint16 map_y = 85U;
    uint16 map_w = (uint16)(ips200_width_max - 10U);
    uint16 map_h;
    uint32 now_ms = system_getval_ms();

    if (!menu_full_redraw && (now_ms - last_refresh_ms < 800U))
    {
        return;
    }

    last_refresh_ms = now_ms;
    if (menu_full_redraw)
    {
        ips200_fill_rect(0, 32, ips200_width_max - 1, ips200_height_max - 1, RGB565_BLACK);
        gps_draw_page_header("GPS Map");
        show_string_fit_width(5, (uint16)(ips200_height_max - 20U), (uint16)(ips200_width_max - 10U), "K1 LONG Exit");
    }

    if (gps_status_hint[0] != '\0')
    {
        show_string_fit_width_pad(10, 36, (uint16)(ips200_width_max - 20U), gps_status_hint);
    }
    else
    {
        show_string_fit_width_pad(10, 36, (uint16)(ips200_width_max - 20U), "");
    }

    if (ips200_height_max > 170U)
    {
        map_h = (uint16)(ips200_height_max - 170U);
    }
    else
    {
        map_h = (uint16)(ips200_height_max / 2U);
    }

    if (map_h < 60U)
    {
        map_h = 60U;
    }

    if ((uint32)map_y + (uint32)map_h > (uint32)(ips200_height_max - 25U))
    {
        map_h = (uint16)(ips200_height_max - map_y - 25U);
    }

    if (map_h < 60U)
    {
        map_h = 60U;
    }

    ips200_fill_rect(0, 32, ips200_width_max - 1U, (uint16)(map_y - 1U), RGB565_BLACK);
    if (gps_status_hint[0] != '\0')
    {
        show_string_fit_width_pad(10, 36, (uint16)(ips200_width_max - 20U), gps_status_hint);
    }
    else
    {
        show_string_fit_width_pad(10, 36, (uint16)(ips200_width_max - 20U), " ");
    }

    if (map_w > 2U && map_h > 2U)
    {
        ips200_fill_rect((uint16)(map_x + 1U),
                         (uint16)(map_y + 1U),
                         (uint16)(map_x + map_w - 2U),
                         (uint16)(map_y + map_h - 2U),
                         RGB565_BLACK);
    }
    path_display_set_area(map_x, map_y, map_w, map_h);
    path_display_trajectory(RGB565_BLUE);
    path_display_current_position();
    menu_full_redraw = 0;
}

/* GPS 菜单项：显示当前 GPS 状态数据 */
static void gps_action_display_data(void)
{
    gps_status_hint[0] = '\0';
    gps_enter_view(1);
}

/* GPS 菜单项：开始或停止轨迹记录 */
static void gps_action_toggle_record(void)
{
    path_state_enum state = path_recorder_get_state();

    if (state == PATH_STATE_RECORDING)
    {
        path_recorder_stop();
        sprintf(gps_status_hint, "Record stopped");
    }
    else
    {
        if (gps_menu_gnss_ready())
        {
            if (path_recorder_start())
            {
                sprintf(gps_status_hint, "Record started");
            }
            else
            {
                sprintf(gps_status_hint, "Start failed");
            }
        }
        else
        {
            sprintf(gps_status_hint, "GPS not ready");
        }
    }

    gps_display_mode = 0;
    menu_full_redraw = 1;
}

/* GPS 菜单项：进入轨迹显示模式 */
static void gps_action_map(void)
{
    sprintf(gps_status_hint, "Track from recorded points");
    path_display_init();
    gps_enter_view(2);
}

/* GPS 菜单项：手动保存当前 GPS 点 */
static void gps_action_save_current_point(void)
{
    path_point_t point;

    if (gps_menu_gnss_ready())
    {
        point.latitude = (float)gnss.latitude;
        point.longitude = (float)gnss.longitude;
        point.timestamp = system_getval_ms();
        point.speed = gnss.speed;
        point.direction = gnss.direction;
        point.satellite_count = gnss.satellite_used;
        point.fix_quality = gnss.state;

        if (path_recorder_add_point(&point))
        {
            sprintf(gps_status_hint, "Current point saved");
        }
        else
        {
            sprintf(gps_status_hint, "Point save failed");
        }
    }
    else
    {
        sprintf(gps_status_hint, "GPS not ready");
    }

    gps_display_mode = 0;
    menu_full_redraw = 1;
}

/* GPS 菜单项：把当前点加入轨迹并立即进入地图显示 */
static void gps_action_current_map(void)
{
    path_point_t point;

    if (gps_menu_gnss_ready())
    {
        point.latitude = (float)gnss.latitude;
        point.longitude = (float)gnss.longitude;
        point.timestamp = system_getval_ms();
        point.speed = gnss.speed;
        point.direction = gnss.direction;
        point.satellite_count = gnss.satellite_used;
        point.fix_quality = gnss.state;

        if (path_recorder_add_point(&point))
        {
            sprintf(gps_status_hint, "Point added to map");
        }
        else
        {
            sprintf(gps_status_hint, "Map add failed");
        }
    }
    else
    {
        sprintf(gps_status_hint, "GPS not ready");
    }

    path_display_init();
    gps_enter_view(2);
}

/* GPS 菜单项：清空当前记录点 */
static void gps_action_clear_points(void)
{
    path_recorder_clear();
    sprintf(gps_status_hint, "Points cleared");
    gps_display_mode = 0;
    menu_full_redraw = 1;
}

/* GPS 菜单项：保存当前路径到 Flash 槽位 */
static void gps_action_save_path(void)
{
    if (path_recorder_get_point_count() > 0)
    {
        if (path_recorder_save_to_flash(0))
        {
            sprintf(gps_status_hint, "Path saved slot 0");
        }
        else
        {
            sprintf(gps_status_hint, "Path save failed");
        }
    }
    else
    {
        sprintf(gps_status_hint, "No path to save");
    }
    gps_display_mode = 0;
    menu_full_redraw = 1;
}

/* GPS 菜单项：从 Flash 槽位加载轨迹并进入地图显示 */
static void gps_action_load_path(void)
{
    if (path_recorder_load_from_flash(0))
    {
        sprintf(gps_status_hint, "Path loaded slot 0");
        path_display_init();
        gps_enter_view(2);
    }
    else
    {
        sprintf(gps_status_hint, "Load path failed");
        gps_display_mode = 0;
        menu_full_redraw = 1;
    }
}

MenuItem gps_items[] = {
    {"1. Display Data", gps_action_display_data, NULL},
    {"2. Start/Stop Rec", gps_action_toggle_record, NULL},
    {"3. Save Curr Point", gps_action_save_current_point, NULL},
    {"4. Current Map", gps_action_current_map, NULL},
    {"5. Map", gps_action_map, NULL},
    {"6. Clear Points", gps_action_clear_points, NULL},
    {"7. Save Path", gps_action_save_path, NULL},
    {"8. Load Path", gps_action_load_path, NULL},
};

MenuPage gps_menu = {
    "GPS",
    gps_items,
    sizeof(gps_items) / sizeof(MenuItem),
    NULL,
    0
};

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
    NULL,
    0
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
    NULL,
    0
};

MenuItem pid_items[] = {
    {"1. test", action_test_inc, NULL},
    {"2. see", NULL, NULL},
};

MenuPage pid_menu = {
    "PID",
    pid_items,
    sizeof(pid_items) / sizeof(MenuItem),
    NULL,
    0
};

MenuItem main_items[] = {
    {"1. GPS", NULL, &gps_menu},
    {"2. Camera", NULL, &camera_menu},
    {"3. PID", NULL, &pid_menu},
    {"4. Save All", Action_Save_Params, NULL},
};

MenuPage main_menu = {
    "xjx",
    main_items,
    sizeof(main_items) / sizeof(MenuItem),
    NULL,
    0
};

static void ips200_fill_rect(uint16 x_start, uint16 y_start, uint16 x_end, uint16 y_end, uint16 color)
{
    if (x_start >= ips200_width_max) x_start = ips200_width_max - 1;
    if (x_end >= ips200_width_max) x_end = ips200_width_max - 1;
    if (y_start >= ips200_height_max) y_start = ips200_height_max - 1;
    if (y_end >= ips200_height_max) y_end = ips200_height_max - 1;
    if (x_start > x_end) { uint16 t = x_start; x_start = x_end; x_end = t; }
    if (y_start > y_end) { uint16 t = y_start; y_start = y_end; y_end = t; }

    for (uint16 y = y_start; y <= y_end; y++)
    {
        ips200_draw_line(x_start, y, x_end, y, color);
    }
}

static void show_string_fit(uint16 x, uint16 y, const char *s)
{
    int max_chars = (int)((ips200_width_max - 1 - x) / 8);
    if (max_chars <= 0) return;

    char buf[64];
    int i = 0;
    while (i < max_chars && s[i] != '\0' && i < (int)sizeof(buf) - 1)
    {
        buf[i] = s[i];
        i++;
    }
    buf[i] = '\0';
    ips200_show_string(x, y, buf);
}

static void show_string_fit_width(uint16 x, uint16 y, uint16 max_width, const char *s)
{
    int max_chars;
    char buf[64];
    int i = 0;

    if (0U == max_width)
    {
        return;
    }

    max_chars = (int)(max_width / 8U);
    if (max_chars <= 0)
    {
        return;
    }

    while (i < max_chars && s[i] != '\0' && i < (int)sizeof(buf) - 1)
    {
        buf[i] = s[i];
        i++;
    }
    buf[i] = '\0';
    ips200_show_string(x, y, buf);
}

static void show_string_fit_width_pad(uint16 x, uint16 y, uint16 max_width, const char *s)
{
    int max_chars;
    char buf[64];
    int i = 0;

    if (0U == max_width)
    {
        return;
    }

    max_chars = (int)(max_width / 8U);
    if (max_chars <= 0)
    {
        return;
    }

    while (i < max_chars && s[i] != '\0' && i < (int)sizeof(buf) - 1)
    {
        buf[i] = s[i];
        i++;
    }

    while (i < max_chars && i < (int)sizeof(buf) - 1)
    {
        buf[i] = ' ';
        i++;
    }

    buf[i] = '\0';
    ips200_show_string(x, y, buf);
}

/* 菜单系统初始化：初始化屏幕、按键、编码器、参数与主菜单状态 */
void menu_init(void)
{
    ips200_init(IPS200_TYPE_SPI);
    ips200_set_dir(IPS200_DEFAULT_DISPLAY_DIR);
    ips200_set_font(IPS200_DEFAULT_DISPLAY_FONT);
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    ips200_full(RGB565_BLACK);

    my_key_init(10);
    my_key_clear_all_state();
    MyEncoder_Init();

    Init_Load_Params();

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
}

void menu_set_dynamic_draw(menu_dynamic_draw_t callback)
{
    menu_dynamic_draw_cb = callback;
}

void menu_set_dynamic_area(uint16 x, uint16 y, uint16 w, uint16 h)
{
    menu_dynamic_x = x;
    menu_dynamic_y = y;
    menu_dynamic_w = w;
    menu_dynamic_h = h;
}

void menu_set_dynamic_clear(uint8 enable)
{
    menu_dynamic_clear_enable = enable ? 1 : 0;
}

/* 菜单主任务：处理编码器/按键输入、菜单跳转与界面刷新 */
void menu_task(void)
{
    system_delay_ms(10);
    my_key_scanner();
    Get_Switch_Num();

    if (current_page == NULL || current_page->num_items <= 0)
    {
        return;
    }

    if (gps_display_mode != 0)
    {
        if (my_key_get_state(MY_KEY_1) == MY_KEY_LONG_PRESS)
        {
            my_key_clear_state(MY_KEY_1);
            gps_exit_view();
            return;
        }

        if (1 == gps_display_mode)
        {
            gps_draw_data_page();
        }
        else if (2 == gps_display_mode)
        {
            gps_draw_map_page();
        }
        return;
    }

    if (If_Switch_Encoder_Change())
    {
        current_selection -= switch_encoder_change_num;

        while (current_selection >= current_page->num_items)
        {
            current_selection -= current_page->num_items;
        }

        while (current_selection < 0)
        {
            current_selection += current_page->num_items;
        }

        menu_needs_update = 1;
    }

    if (my_key_get_state(MY_KEY_1) == MY_KEY_LONG_PRESS)
    {
        my_key_clear_state(MY_KEY_1);
        if (current_page->parent)
        {
            current_page = current_page->parent;
            current_selection = 0;
            gps_exit_view();
            gps_status_hint[0] = '\0';
            menu_set_dynamic_draw(NULL);
            menu_needs_update = 1;
            menu_full_redraw = 1;
        }
        return;
    }

    if (my_key_get_state(MY_KEY_1) == MY_KEY_SHORT_PRESS)
    {
        my_key_clear_state(MY_KEY_1);
        MenuItem *item = &current_page->items[current_selection];

        if (item->function)
        {
            item->function();
            if (gps_display_mode != 0)
            {
                if (1 == gps_display_mode)
                {
                    gps_draw_data_page();
                }
                else if (2 == gps_display_mode)
                {
                    gps_draw_map_page();
                }
                return;
            }
            menu_full_redraw = 1;
        }
        else if (item->sub_page)
        {
            if (item->sub_page == &gps_menu)
            {
                gps_display_mode = 0;
                gps_status_hint[0] = '\0';
                menu_set_dynamic_area(0, 0, 0, 0);
                menu_set_dynamic_draw(NULL);
                menu_set_dynamic_clear(0);
            }
            item->sub_page->parent = current_page;
            current_page = item->sub_page;
            current_selection = 0;
            menu_needs_update = 1;
            menu_full_redraw = 1;
        }
    }

    if (menu_full_redraw)
    {
        ips200_full(RGB565_BLACK);

        ips200_set_color(RGB565_YELLOW, RGB565_BLACK);
        show_string_fit(10, 10, current_page->title);
        ips200_draw_line(0, 30, ips200_width_max - 1, 30, RGB565_GRAY);

        for (int i = 0; i < current_page->num_items; i++)
        {
            uint16 y_pos = 40 + i * 20;
            if (y_pos + 16 >= ips200_height_max) break;

            if (i == current_selection)
            {
                ips200_fill_rect(5, y_pos - 2, ips200_width_max - 5, y_pos + 16, current_font_color);
                ips200_set_color(RGB565_BLACK, current_font_color);
                ips200_show_string(10, y_pos, current_page->items[i].name);
            }
            else
            {
                ips200_set_color(current_font_color, RGB565_BLACK);
                ips200_show_string(10, y_pos, current_page->items[i].name);
            }
        }

        ips200_set_color(RGB565_GRAY, RGB565_BLACK);
        ips200_show_string(5, ips200_height_max - 20, "ENC:Move K1:OK/LONG:BK");

        last_selection = current_selection;
        menu_full_redraw = 0;
        menu_needs_update = 0;
    }
    else if (menu_needs_update)
    {
        int prev = last_selection;
        int curr = current_selection;

        if (prev >= 0 && prev < current_page->num_items)
        {
            uint16 y_prev = 40 + prev * 20;
            if (y_prev + 16 < ips200_height_max)
            {
                ips200_fill_rect(5, y_prev - 2, ips200_width_max - 5, y_prev + 16, RGB565_BLACK);
                ips200_set_color(current_font_color, RGB565_BLACK);
                ips200_show_string(10, y_prev, current_page->items[prev].name);
            }
        }

        if (curr >= 0 && curr < current_page->num_items)
        {
            uint16 y_curr = 40 + curr * 20;
            if (y_curr + 16 < ips200_height_max)
            {
                ips200_fill_rect(5, y_curr - 2, ips200_width_max - 5, y_curr + 16, current_font_color);
                ips200_set_color(RGB565_BLACK, current_font_color);
                ips200_show_string(10, y_curr, current_page->items[curr].name);
            }
        }

        last_selection = current_selection;
        menu_needs_update = 0;
    }

    if (menu_dynamic_draw_cb && menu_dynamic_w && menu_dynamic_h)
    {
        if (menu_dynamic_clear_enable)
        {
            uint16 x_end = (uint16)(menu_dynamic_x + menu_dynamic_w - 1);
            uint16 y_end = (uint16)(menu_dynamic_y + menu_dynamic_h - 1);
            ips200_fill_rect(menu_dynamic_x, menu_dynamic_y, x_end, y_end, RGB565_BLACK);
        }
        menu_dynamic_draw_cb(menu_dynamic_x, menu_dynamic_y, menu_dynamic_w, menu_dynamic_h);
    }
}
