/**
 * @file display_gps.c
 * @brief GPS坐标转换与显示模块实现文件
 * @author 用户
 * @date 2026-03-21
 * @version 1.0
 *
 * @details 该模块实现GPS坐标的转换和显示功能，包括：
 *          - 经纬度坐标到平面坐标的转换（球面到平面投影）
 *          - 平面坐标到屏幕坐标的转换（缩放和平移）
 *          - 多种屏幕类型的显示支持
 *          - 路径绘制和当前位置标记
 */

#include <GPS/display_gps.h>

/** @brief 显示区域起始X坐标 */
static int16 display_start_x = 0;

/** @brief 显示区域起始Y坐标 */
static int16 display_start_y = 0;

/** @brief 显示区域宽度 */
static int16 display_width = 0;

/** @brief 显示区域高度 */
static int16 display_high = 0;

/** @brief 当前处理的坐标点数量 */
static int16 transition_point_num   = 0;

/** @brief 当前屏幕类型，默认为IPS200 SPI屏幕 */
static screen_type_enum display_screen_type = SCREEN_IPS200_SPI;


/** @brief 平面坐标系数据数组，存储转换后的平面坐标，单位：米 */
plane_point plane_point_data[DISPLAY_POINT_MAX];

/** @brief 屏幕坐标系数据数组，存储最终显示坐标，单位：像素 */
screen_point screen_point_data[DISPLAY_POINT_MAX];

/** @brief GPS坐标数据数组，存储原始经纬度坐标，单位：度 */
gps_point gps_point_data[DISPLAY_POINT_MAX]={
    {10.0, 10.0},  /**< 测试点1: 纬度10.0°, 经度10.0° */
    {20.0, 20.0},  /**< 测试点2: 纬度20.0°, 经度20.0° */
    {10.0, 20.0}   /**< 测试点3: 纬度10.0°, 经度20.0° */
};

/**
 * @brief 经纬度球面坐标系转换为平面坐标系
 * @param gps_point_input GPS坐标数组指针
 *
 * @details 将经纬度坐标转换为以第一个点为原点的平面直角坐标系
 *          转换原理：
 *          1. 计算纬度每度对应的米数：meters_per_degree_lat = (π/180) * EARTH_RADIUS
 *          2. 计算经度每度对应的米数：meters_per_degree_lon = meters_per_degree_lat * cos(lat)
 *          3. 将经纬度差转换为米：Δx = Δlon * meters_per_degree_lon, Δy = Δlat * meters_per_degree_lat
 *
 * @note 第一个点作为原点(0,0)，其他点相对于第一个点计算
 */
static void spherical_to_plane(gps_point *gps_point_input)
{
    double meters_per_degree_lat = 0;  /**< 纬度每度对应的米数 */
    double meters_per_degree_lon = 0;  /**< 经度每度对应的米数 */

    if(transition_point_num > 1)
    {
        /* 计算纬度每度对应的米数 */
        meters_per_degree_lat = (USER_PI / 180.0) * EARTH_RADIUS;

        /* 计算经度每度对应的米数（考虑纬度对经度长度的影响） */
        meters_per_degree_lon = meters_per_degree_lat * cos(gps_point_input[0].lat * USER_PI / 180.0);

        /* 将每个点转换为平面坐标 */
        for (int i = 1; i < transition_point_num; i++)
        {
            /* 计算东方向位移（经度差转换为米） */
            plane_point_data[i].x = (gps_point_input[i].lon - gps_point_input[0].lon) * meters_per_degree_lon;

            /* 计算北方向位移（纬度差转换为米） */
            plane_point_data[i].y = (gps_point_input[i].lat - gps_point_input[0].lat) * meters_per_degree_lat;
        }
    }
}
/**
 * @brief 平面坐标系转换为屏幕坐标系
 *
 * @details 将平面坐标（单位：米）转换为屏幕坐标（单位：像素）
 *          转换步骤：
 *          1. 计算所有点的坐标范围（最小/最大X/Y）
 *          2. 计算缩放比例，保持纵横比不变
 *          3. 计算偏移量，使图形在显示区域内居中
 *          4. 反转Y轴（平面坐标系：北方向为正，屏幕坐标系：向下为正）
 *
 * @note 使用等比例缩放，保持图形不变形
 */
static void plane_to_screen(void)
{
    double min_x = plane_point_data[0].x;  /**< 最小X坐标 */
    double max_x = plane_point_data[0].x;  /**< 最大X坐标 */
    double min_y = plane_point_data[0].y;  /**< 最小Y坐标 */
    double max_y = plane_point_data[0].y;  /**< 最大Y坐标 */

    double width   = 0.0f;   /**< 坐标范围宽度 */
    double height  = 0.0f;   /**< 坐标范围高度 */
    double scale_x = 0.0f;   /**< X方向缩放比例 */
    double scale_y = 0.0f;   /**< Y方向缩放比例 */
    double scale   = 0.0f;   /**< 最终缩放比例（取较小值保持纵横比） */

    double x_offset = 0.0;   /**< X方向偏移量（用于居中） */
    double y_offset = 0.0;   /**< Y方向偏移量（用于居中） */

    if(transition_point_num > 1)
    {
        /* 步骤1：找出所有点的坐标范围 */
        for (int i = 1; i < transition_point_num; i++)
        {
            min_x = fmin(min_x, plane_point_data[i].x);
            max_x = fmax(max_x, plane_point_data[i].x);
            min_y = fmin(min_y, plane_point_data[i].y);
            max_y = fmax(max_y, plane_point_data[i].y);
        }

        /* 检查所有点是否重合（如果重合则不进行缩放） */
        if (max_x - min_x > EPSILON && max_y - min_y > EPSILON)
        {
            /* 步骤2：计算坐标范围 */
            width = max_x - min_x;   /* 坐标范围宽度 */
            height = max_y - min_y;  /* 坐标范围高度 */

            /* 步骤3：计算缩放比例 */
            scale_x = (double)display_width / width;
            scale_y = (double)display_high / height;
            scale = fmin(scale_x, scale_y);  /* 取较小值保持纵横比 */

            /* 步骤4：计算偏移量（使图形居中） */
            if (scale == scale_x)  /* 如果X方向先达到边界 */
            {
                y_offset = ((double)display_high - height * scale) / 2.0;
            }
            else  /* 如果Y方向先达到边界 */
            {
                x_offset = ((double)display_width - width * scale) / 2.0;
            }

            /* 步骤5：执行坐标转换 */
            for (int i = 0; i < transition_point_num; i++)
            {
                /* 计算X坐标：缩放 + 偏移 */
                screen_point_data[i].x = (plane_point_data[i].x - min_x) * scale + x_offset;

                /* 计算Y坐标：缩放 + 偏移 + Y轴反转 */
                /* 平面坐标系：y越大表示越北（向上为正） */
                /* 屏幕坐标系：y越大表示越南（向下为正） */
                screen_point_data[i].y = (double)display_high - ((plane_point_data[i].y - min_y) * scale + y_offset);
            }
        }
    }
}

/**
 * @brief GPS坐标转换主函数
 * @param gps_point_input GPS坐标数组指针
 * @param point_num 坐标点数量
 *
 * @details 执行完整的GPS坐标转换流程：
 *          1. 参数验证（检查点数量是否超过最大值）
 *          2. 清除之前的转换数据
 *          3. 调用 spherical_to_plane() 进行球面到平面坐标转换
 *          4. 调用 plane_to_screen() 进行平面到屏幕坐标转换
 *
 * @note 该函数是GPS显示模块的核心入口函数
 */
void user_gps_transition(gps_point *gps_point_input, int16 point_num)
{
    /* 参数验证：检查点数量是否超过最大显示数量 */
    zf_assert(point_num < DISPLAY_POINT_MAX);

    /* 设置当前处理的点数量 */
    transition_point_num = point_num;

    /* 清除之前的平面坐标系数据 */
    memset(plane_point_data, 0, sizeof(plane_point_data));

    /* 清除之前的屏幕坐标数据 */
    memset(screen_point_data, 0, sizeof(screen_point_data));

    /* 步骤1：经纬度球面坐标系转换为平面坐标系 */
    spherical_to_plane(gps_point_input);

    /* 步骤2：平面坐标系转换为屏幕显示坐标 */
    plane_to_screen();
}

/**
 * @brief GPS坐标显示函数
 * @param display_color 显示颜色（RGB565格式）
 *
 * @details 在屏幕上显示转换后的GPS坐标点，连接相邻点形成路径
 *          支持多种屏幕类型：
 *          - SCREEN_IPS114: IPS114屏幕
 *          - SCREEN_IPS200_SPI: IPS200屏幕（SPI接口）
 *          - SCREEN_IPS200_PARALLEL8: IPS200屏幕（8位并行接口）
 *          - SCREEN_TFT180: TFT180屏幕
 *
 * @note 通过绘制背景色可以实现擦除效果
 */
void user_gps_display(uint16 display_color)
{
    /* 遍历所有点，连接相邻点形成路径 */
    for(int i = 0; i < (transition_point_num - 1); i++)
    {
        /* 根据屏幕类型调用相应的绘制函数 */
        switch(display_screen_type)
        {
            case SCREEN_IPS114:
            {
                /* IPS114屏幕绘制线段 */
                ips114_draw_line((uint16)screen_point_data[i].x  + display_start_x,
                                 (uint16)screen_point_data[i].y  + display_start_y,
                                 (uint16)screen_point_data[i + 1].x + display_start_x,
                                 (uint16)screen_point_data[i + 1].y + display_start_y,
                                 display_color);
            }
            break;

            case SCREEN_IPS200_SPI:
            case SCREEN_IPS200_PARALLEL8:
            {
                /* IPS200屏幕绘制线段（支持SPI和并行接口） */
                ips200_draw_line((uint16)screen_point_data[i].x  + display_start_x,
                                 (uint16)screen_point_data[i].y  + display_start_y,
                                 (uint16)screen_point_data[i + 1].x + display_start_x,
                                 (uint16)screen_point_data[i + 1].y + display_start_y,
                                 display_color);
            }
            break;

            case SCREEN_TFT180:
            {
                /* TFT180屏幕绘制线段 */
                tft180_draw_line((uint16)screen_point_data[i].x    + display_start_x,
                                (uint16)screen_point_data[i].y    + display_start_y,
                                (uint16)screen_point_data[i+1].x  + display_start_x,
                                (uint16)screen_point_data[i+1].y  + display_start_y,
                                display_color);
            }
            break;
        }
    }
}

/**
 * @brief GPS坐标显示初始化函数
 * @param screen_type 屏幕类型（参考 screen_type_enum 枚举）
 * @param start_x 显示区域起始X坐标
 * @param start_y 显示区域起始Y坐标
 * @param width 显示区域宽度
 * @param high 显示区域高度
 *
 * @details 初始化GPS显示模块的参数：
 *          - 设置显示区域位置和大小
 *          - 设置屏幕类型
 *          - 宽度和高度减1是为了适配从0开始的坐标系统
 *
 * @note 必须在调用其他显示函数之前初始化
 */
void user_gps_display_init(screen_type_enum screen_type, int16 start_x, int16 start_y, int16 width, int16 high)
{
    /* 设置显示区域起始坐标 */
    display_start_x = start_x;
    display_start_y = start_y;

    /* 设置显示区域尺寸（减1适配从0开始的坐标系统） */
    display_width = width - 1;
    display_high = high - 1;

    /* 设置屏幕类型 */
    display_screen_type = screen_type;
}


/**
 * @brief IPS200屏幕画圆函数
 * @param x_center 圆心X坐标 [0, ips200_width_max-1]
 * @param y_center 圆心Y坐标 [0, ips200_height_max-1]
 * @param radius 圆的半径（像素）
 * @param color 颜色值（RGB565格式）
 *
 * @details 使用Bresenham算法高效绘制整圆，算法特点：
 *          1. 只计算八分之一的圆弧点
 *          2. 利用对称性绘制完整的圆
 *          3. 只使用整数运算，效率高
 *
 * @note Bresenham算法决策参数：d = 3 - 2 * radius
 */
void ips200_draw_circle(uint16 x_center, uint16 y_center, uint16 radius, const uint16 color)
{
    /* 参数有效性检查 */
    zf_assert(x_center < ips200_width_max);
    zf_assert(y_center < ips200_height_max);
    zf_assert(radius > 0);

    int x = 0;              /**< 当前X坐标（从0开始） */
    int y = radius;         /**< 当前Y坐标（从半径开始） */
    int d = 3 - 2 * radius; /**< Bresenham决策参数初始化 */

    /* Bresenham算法主循环：计算八分之一圆弧 */
    while (x <= y)
    {
        /* 利用对称性绘制八个对称点 */
        /* 第一象限 */
        ips200_draw_point(x_center + x + display_start_x, y_center + y + display_start_y, color);
        ips200_draw_point(x_center + y + display_start_x, y_center + x + display_start_y, color);

        /* 第二象限 */
        ips200_draw_point(x_center - x + display_start_x, y_center + y + display_start_y, color);
        ips200_draw_point(x_center - y + display_start_x, y_center + x + display_start_y, color);

        /* 第三象限 */
        ips200_draw_point(x_center - x + display_start_x, y_center - y + display_start_y, color);
        ips200_draw_point(x_center - y + display_start_x, y_center - x + display_start_y, color);

        /* 第四象限 */
        ips200_draw_point(x_center + x + display_start_x, y_center - y + display_start_y, color);
        ips200_draw_point(x_center + y + display_start_x, y_center - x + display_start_y, color);

        /* 更新Bresenham决策参数 */
        if (d < 0)
        {
            /* 选择E点：x增加，y不变 */
            d += 4 * x + 6;
        }
        else
        {
            /* 选择SE点：x增加，y减少 */
            d += 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

/**
 * @brief GPS坐标显示综合函数
 * @param gps_point_data GPS坐标数组指针
 * @param point_num 坐标点数量
 * @param x 当前点在屏幕上的X坐标（像素）
 * @param y 当前点在屏幕上的Y坐标（像素）
 * @param R 标示当前点的圆半径（像素）
 * @param line_color 路径线颜色（RGB565格式）
 * @param point_color 当前点圆的颜色（RGB565格式）
 *
 * @details 一站式GPS坐标显示函数，功能包括：
 *          1. 坐标转换：调用 user_gps_transition() 进行坐标转换
 *          2. 路径绘制：调用 user_gps_display() 绘制GPS路径
 *          3. 当前位置标记：调用 ips200_draw_circle() 标记当前位置
 *
 * @note 该函数适用于IPS200屏幕显示，集成了完整的GPS显示流程
 */
void gps_display(gps_point *gps_point_data, int16 point_num, uint16 x, uint16 y,
                 int16 R, const uint16 line_color, const uint16 point_color)
{
    /* 步骤1：GPS坐标转换（经纬度→平面→屏幕） */
    user_gps_transition(gps_point_data, point_num);

    /* 步骤2：绘制GPS路径（连接所有点） */
    user_gps_display(line_color);

    /* 步骤3：标记当前位置（在指定坐标绘制圆形标记） */
    ips200_draw_circle(x, y, R, point_color);
}
