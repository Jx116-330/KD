#include "myhead.h"
#define IPS200_TYPE     (IPS200_TYPE_SPI)
#pragma section all "cpu0_dsram"

int core0_main(void)
{
    clock_init();
    debug_init();
    Beep_Init();

    ips200_set_dir(IPS200_PORTAIT);
    ips200_set_color(RGB565_RED, RGB565_BLACK);
    ips200_init(IPS200_TYPE);
    ips200_clear();

    gnss_init(TAU1201);
    path_recorder_init();
    menu_init();

    cpu_wait_event_ready();

    while (TRUE)
    {
        gnss_data_parse();

        if (path_recorder_get_state() == PATH_STATE_RECORDING)
        {
            path_recorder_task();
        }

        menu_task();
    }
}

#pragma section all restore
