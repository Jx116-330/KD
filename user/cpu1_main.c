/*********************************************************************************************************************
* File: cpu1_main.c
* Brief: Cleaned header comment to avoid encoding issues.
* Note: Original logic retained.
*********************************************************************************************************************/

#include "zf_common_headfile.h"
#pragma section all "cpu1_dsram"





void core1_main(void)
{
    disable_Watchdog();                     // �رտ��Ź�
    interrupt_global_enable(0);             // ��ȫ���ж�




    cpu_wait_event_ready();                 // �ȴ����к��ĳ�ʼ�����?
    while (TRUE)
    {




    }
}
#pragma section all restore
