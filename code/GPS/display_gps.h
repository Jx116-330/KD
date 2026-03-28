/**
 * @file display_gps.h
 * @brief Cleaned module header to avoid encoding issues.
 * @author JX116
 */

#ifndef _SHOW_GPS_H_
#define _SHOW_GPS_H_

#include "zf_common_headfile.h"

#define EARTH_RADIUS   (6371000.0)

#define EPSILON   (1e-9)

#define USER_PI     (3.1415926535898)

#define DISPLAY_POINT_MAX  (50)


/**
 */
typedef struct
{
    double lat;    /**< γ�ȣ���λ���� */
    double lon;     /**< ���ȣ���λ���� */
} gps_point;

/**
 */
typedef struct
{
    double x;    /**< ���������꣬��λ���ף�����ڵ�һ����?*/
    double y;   /**< ���������꣬��λ���ף�����ڵ�һ����?*/
} plane_point;

/**
 */
typedef struct
{
    double x;      /**< ��ĻX���꣬���ص�λ */
    double y;     /**< ��ĻY���꣬���ص�λ */
} screen_point;

/**
 */
typedef enum
{
    SCREEN_IPS114,              /**< IPS114��Ļ */
    SCREEN_IPS200_SPI,          /**< IPS200��Ļ��SPI�ӿ� */
    SCREEN_IPS200_PARALLEL8,    /**< IPS200��Ļ��8λ���нӿ� */
    SCREEN_TFT180,              /**< TFT180��Ļ */
}screen_type_enum;

extern gps_point gps_point_data[DISPLAY_POINT_MAX];

extern plane_point plane_point_data[DISPLAY_POINT_MAX];

extern screen_point screen_point_data[DISPLAY_POINT_MAX];

/**
 */
void ips200_INS_flash_draw_circle(uint16 x_center, uint16 y_center, uint16 radius, const uint16 color);

/**
 */
void user_gps_transition(gps_point *gps_point_input, int16 point_num);

/**
 */
void user_gps_display(uint16 color);

/**
 */
void user_gps_display_init(screen_type_enum screen_type, int16 start_x, int16 start_y, int16 width, int16 high);

/**
 */
void gps_display(gps_point *gps_point_data,int16 point_num,uint16 x,uint16 y,int16 R,const uint16 line_color,const uint16 point_color);

#endif
