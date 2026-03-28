/**
 * @file display_gps.c
 * @brief Cleaned module header to avoid encoding issues.
 * @author JX116
 */

#include <GPS/display_gps.h>

static int16 display_start_x = 0;

static int16 display_start_y = 0;

static int16 display_width = 0;

static int16 display_high = 0;

static int16 transition_point_num   = 0;

static screen_type_enum display_screen_type = SCREEN_IPS200_SPI;


plane_point plane_point_data[DISPLAY_POINT_MAX];

screen_point screen_point_data[DISPLAY_POINT_MAX];

gps_point gps_point_data[DISPLAY_POINT_MAX]={
    {10.0, 10.0},  /**< ���Ե�1: γ��10.0��, ����10.0�� */
    {20.0, 20.0},  /**< ���Ե�2: γ��20.0��, ����20.0�� */
    {10.0, 20.0}   /**< ���Ե�3: γ��10.0��, ����20.0�� */
};

/**
 *
 *
 */
static void spherical_to_plane(gps_point *gps_point_input)
{
    double meters_per_degree_lat = 0;  /**< γ��ÿ�ȶ�Ӧ������ */
    double meters_per_degree_lon = 0;  /**< ����ÿ�ȶ�Ӧ������ */

    if(transition_point_num > 1)
    {
        meters_per_degree_lat = (USER_PI / 180.0) * EARTH_RADIUS;

        meters_per_degree_lon = meters_per_degree_lat * cos(gps_point_input[0].lat * USER_PI / 180.0);

        for (int i = 1; i < transition_point_num; i++)
        {
            plane_point_data[i].x = (gps_point_input[i].lon - gps_point_input[0].lon) * meters_per_degree_lon;

            plane_point_data[i].y = (gps_point_input[i].lat - gps_point_input[0].lat) * meters_per_degree_lat;
        }
    }
}
/**
 *
 *
 */
static void plane_to_screen(void)
{
    double min_x = plane_point_data[0].x;  /**< ��СX���� */
    double max_x = plane_point_data[0].x;  /**< ���X���� */
    double min_y = plane_point_data[0].y;  /**< ��СY���� */
    double max_y = plane_point_data[0].y;  /**< ���Y���� */

    double width   = 0.0f;   /**< ���귶Χ���� */
    double height  = 0.0f;   /**< ���귶Χ�߶� */
    double scale_x = 0.0f;   /**< X�������ű��� */
    double scale_y = 0.0f;   /**< Y�������ű��� */
    double scale   = 0.0f;   /**< �������ű�����ȡ��Сֵ�����ݺ�ȣ�?*/

    double x_offset = 0.0;   /**< X����ƫ���������ھ��У� */
    double y_offset = 0.0;   /**< Y����ƫ���������ھ��У� */

    if(transition_point_num > 1)
    {
        for (int i = 1; i < transition_point_num; i++)
        {
            min_x = fmin(min_x, plane_point_data[i].x);
            max_x = fmax(max_x, plane_point_data[i].x);
            min_y = fmin(min_y, plane_point_data[i].y);
            max_y = fmax(max_y, plane_point_data[i].y);
        }

        if (max_x - min_x > EPSILON && max_y - min_y > EPSILON)
        {
            width = max_x - min_x;   /* ���귶Χ���� */
            height = max_y - min_y;  /* ���귶Χ�߶� */

            scale_x = (double)display_width / width;
            scale_y = (double)display_high / height;
            scale = fmin(scale_x, scale_y);  /* ȡ��Сֵ�����ݺ��?*/

            if (scale == scale_x)  /* ���X�����ȴﵽ�߽� */
            {
                y_offset = ((double)display_high - height * scale) / 2.0;
            }
            else  /* ���Y�����ȴﵽ�߽� */
            {
                x_offset = ((double)display_width - width * scale) / 2.0;
            }

            for (int i = 0; i < transition_point_num; i++)
            {
                screen_point_data[i].x = (plane_point_data[i].x - min_x) * scale + x_offset;

                screen_point_data[i].y = (double)display_high - ((plane_point_data[i].y - min_y) * scale + y_offset);
            }
        }
    }
}

/**
 *
 *
 */
void user_gps_transition(gps_point *gps_point_input, int16 point_num)
{
    zf_assert(point_num < DISPLAY_POINT_MAX);

    transition_point_num = point_num;

    memset(plane_point_data, 0, sizeof(plane_point_data));

    memset(screen_point_data, 0, sizeof(screen_point_data));

    spherical_to_plane(gps_point_input);

    plane_to_screen();
}

/**
 *
 *
 */
void user_gps_display(uint16 display_color)
{
    for(int i = 0; i < (transition_point_num - 1); i++)
    {
        switch(display_screen_type)
        {
            case SCREEN_IPS114:
            {
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
                ips200_draw_line((uint16)screen_point_data[i].x  + display_start_x,
                                 (uint16)screen_point_data[i].y  + display_start_y,
                                 (uint16)screen_point_data[i + 1].x + display_start_x,
                                 (uint16)screen_point_data[i + 1].y + display_start_y,
                                 display_color);
            }
            break;

            case SCREEN_TFT180:
            {
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
 *
 *
 */
void user_gps_display_init(screen_type_enum screen_type, int16 start_x, int16 start_y, int16 width, int16 high)
{
    display_start_x = start_x;
    display_start_y = start_y;

    display_width = width - 1;
    display_high = high - 1;

    display_screen_type = screen_type;
}


/**
 *
 *
 */
void ips200_draw_circle(uint16 x_center, uint16 y_center, uint16 radius, const uint16 color)
{
    zf_assert(x_center < ips200_width_max);
    zf_assert(y_center < ips200_height_max);
    zf_assert(radius > 0);

    int x = 0;              /**< ��ǰX���꣨��0��ʼ�� */
    int y = radius;         /**< ��ǰY���꣨�Ӱ뾶��ʼ�� */
    int d = 3 - 2 * radius; /**< Bresenham���߲�����ʼ�� */

    while (x <= y)
    {
        ips200_draw_point(x_center + x + display_start_x, y_center + y + display_start_y, color);
        ips200_draw_point(x_center + y + display_start_x, y_center + x + display_start_y, color);

        ips200_draw_point(x_center - x + display_start_x, y_center + y + display_start_y, color);
        ips200_draw_point(x_center - y + display_start_x, y_center + x + display_start_y, color);

        ips200_draw_point(x_center - x + display_start_x, y_center - y + display_start_y, color);
        ips200_draw_point(x_center - y + display_start_x, y_center - x + display_start_y, color);

        ips200_draw_point(x_center + x + display_start_x, y_center - y + display_start_y, color);
        ips200_draw_point(x_center + y + display_start_x, y_center - x + display_start_y, color);

        if (d < 0)
        {
            d += 4 * x + 6;
        }
        else
        {
            d += 4 * (x - y) + 10;
            y--;
        }
        x++;
    }
}

/**
 *
 *
 */
void gps_display(gps_point *gps_point_data, int16 point_num, uint16 x, uint16 y,
                 int16 R, const uint16 line_color, const uint16 point_color)
{
    user_gps_transition(gps_point_data, point_num);

    user_gps_display(line_color);

    ips200_draw_circle(x, y, R, point_color);
}
