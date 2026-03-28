/*********************************************************************************************************************
* File: isr_config.h
* Brief: Cleaned header comment to avoid encoding issues.
* Note: Original logic retained.
*********************************************************************************************************************/

#ifndef _isr_config_h
#define _isr_config_h







#define CCU6_0_CH0_INT_SERVICE  IfxSrc_Tos_cpu0     // ����CCU6_0 PITͨ��0�жϷ������ͣ����ж�����˭��Ӧ���� IfxSrc_Tos_cpu0 IfxSrc_Tos_cpu1 IfxSrc_Tos_dma  ��������Ϊ����ֵ
#define CCU6_0_CH0_ISR_PRIORITY 50                  // ����CCU6_0 PITͨ��0�ж����ȼ� ���ȼ���Χ1-255 Խ�����ȼ�Խ�� ��ƽʱʹ�õĵ�Ƭ����һ��

#define CCU6_0_CH1_INT_SERVICE  IfxSrc_Tos_cpu0
#define CCU6_0_CH1_ISR_PRIORITY 51

#define CCU6_1_CH0_INT_SERVICE  IfxSrc_Tos_cpu0
#define CCU6_1_CH0_ISR_PRIORITY 52

#define CCU6_1_CH1_INT_SERVICE  IfxSrc_Tos_cpu0
#define CCU6_1_CH1_ISR_PRIORITY 53



#define EXTI_CH0_CH4_INT_SERVICE IfxSrc_Tos_cpu0    // ����ERUͨ��0��ͨ��4�жϷ������ͣ����ж�����˭��Ӧ���� IfxSrc_Tos_cpu0 IfxSrc_Tos_cpu1 IfxSrc_Tos_dma  ��������Ϊ����ֵ
#define EXTI_CH0_CH4_INT_PRIO   60                  // ����ERUͨ��0��ͨ��4�ж����ȼ� ���ȼ���Χ1-255 Խ�����ȼ�Խ�� ��ƽʱʹ�õĵ�Ƭ����һ��

#define EXTI_CH1_CH5_INT_SERVICE IfxSrc_Tos_cpu0    // ����ERUͨ��1��ͨ��5�жϷ������ͣ�ͬ��
#define EXTI_CH1_CH5_INT_PRIO   61                  // ����ERUͨ��1��ͨ��5�ж����ȼ� ͬ��

#define EXTI_CH2_CH6_INT_SERVICE IfxSrc_Tos_dma     // ����ERUͨ��2��ͨ��6�жϷ������ͣ�ͬ��
#define EXTI_CH2_CH6_INT_PRIO   5                   // ����ERUͨ��2��ͨ��6�ж����ȼ� �����÷�ΧΪ0-127(DMA��Ӧ)

#define EXTI_CH3_CH7_INT_SERVICE IfxSrc_Tos_cpu0    // ����ERUͨ��3��ͨ��7�жϷ������ͣ�ͬ��
#define EXTI_CH3_CH7_INT_PRIO   62                  // ����ERUͨ��3��ͨ��7�ж����ȼ� ͬ��


#define DMA_INT_SERVICE         IfxSrc_Tos_cpu0     // ERU����DMA�жϷ������ͣ����ж�����˭��Ӧ���� IfxSrc_Tos_cpu0 IfxSrc_Tos_cpu1 IfxSrc_Tos_dma  ��������Ϊ����ֵ
#define DMA_INT_PRIO            70                  // ERU����DMA�ж����ȼ� ���ȼ���Χ1-255 Խ�����ȼ�Խ�� ��ƽʱʹ�õĵ�Ƭ����һ��


#define UART0_INT_SERVICE       IfxSrc_Tos_cpu0     // ���崮��0�жϷ������ͣ����ж�����˭��Ӧ���� IfxSrc_Tos_cpu0 IfxSrc_Tos_cpu1 IfxSrc_Tos_dma  ��������Ϊ����ֵ
#define UART0_TX_INT_PRIO       11                  // ���崮��0�����ж����ȼ� ���ȼ���Χ1-255 Խ�����ȼ�Խ�� ��ƽʱʹ�õĵ�Ƭ����һ��
#define UART0_RX_INT_PRIO       10                  // ���崮��0�����ж����ȼ� ���ȼ���Χ1-255 Խ�����ȼ�Խ�� ��ƽʱʹ�õĵ�Ƭ����һ��
#define UART0_ER_INT_PRIO       12                  // ���崮��0�����ж����ȼ� ���ȼ���Χ1-255 Խ�����ȼ�Խ�� ��ƽʱʹ�õĵ�Ƭ����һ��

#define UART1_INT_SERVICE       IfxSrc_Tos_cpu0
#define UART1_TX_INT_PRIO       13
#define UART1_RX_INT_PRIO       14
#define UART1_ER_INT_PRIO       15

#define UART2_INT_SERVICE       IfxSrc_Tos_cpu0
#define UART2_TX_INT_PRIO       16
#define UART2_RX_INT_PRIO       17
#define UART2_ER_INT_PRIO       18

#define UART3_INT_SERVICE       IfxSrc_Tos_cpu0
#define UART3_TX_INT_PRIO       19
#define UART3_RX_INT_PRIO       20
#define UART3_ER_INT_PRIO       21

#define UART4_INT_SERVICE       IfxSrc_Tos_cpu0
#define UART4_TX_INT_PRIO       22
#define UART4_RX_INT_PRIO       23
#define UART4_ER_INT_PRIO       24

#define UART5_INT_SERVICE       IfxSrc_Tos_cpu0
#define UART5_TX_INT_PRIO       25
#define UART5_RX_INT_PRIO       26
#define UART5_ER_INT_PRIO       27

#define UART6_INT_SERVICE       IfxSrc_Tos_cpu0
#define UART6_TX_INT_PRIO       28
#define UART6_RX_INT_PRIO       29
#define UART6_ER_INT_PRIO       30

#define UART8_INT_SERVICE       IfxSrc_Tos_cpu0
#define UART8_TX_INT_PRIO       31
#define UART8_RX_INT_PRIO       32
#define UART8_ER_INT_PRIO       33

#define UART9_INT_SERVICE       IfxSrc_Tos_cpu0
#define UART9_TX_INT_PRIO       34
#define UART9_RX_INT_PRIO       35
#define UART9_ER_INT_PRIO       36

#define UART10_INT_SERVICE       IfxSrc_Tos_cpu0
#define UART10_TX_INT_PRIO       37
#define UART10_RX_INT_PRIO       38
#define UART10_ER_INT_PRIO       39

#define UART11_INT_SERVICE       IfxSrc_Tos_cpu0
#define UART11_TX_INT_PRIO       40
#define UART11_RX_INT_PRIO       41
#define UART11_ER_INT_PRIO       42







#define CCU6_0_CH0_INT_VECTAB_NUM    (int)CCU6_0_CH0_INT_SERVICE      > 0 ? (int)CCU6_0_CH0_INT_SERVICE    - 1 : (int)CCU6_0_CH0_INT_SERVICE
#define CCU6_0_CH1_INT_VECTAB_NUM    (int)CCU6_0_CH1_INT_SERVICE      > 0 ? (int)CCU6_0_CH1_INT_SERVICE    - 1 : (int)CCU6_0_CH1_INT_SERVICE
#define CCU6_1_CH0_INT_VECTAB_NUM    (int)CCU6_1_CH0_INT_SERVICE      > 0 ? (int)CCU6_1_CH0_INT_SERVICE    - 1 : (int)CCU6_1_CH0_INT_SERVICE
#define CCU6_1_CH1_INT_VECTAB_NUM    (int)CCU6_1_CH1_INT_SERVICE      > 0 ? (int)CCU6_1_CH1_INT_SERVICE    - 1 : (int)CCU6_1_CH1_INT_SERVICE

#define EXTI_CH0_CH4_INT_VECTAB_NUM  (int)EXTI_CH0_CH4_INT_SERVICE    > 0 ? (int)EXTI_CH0_CH4_INT_SERVICE  - 1 : (int)EXTI_CH0_CH4_INT_SERVICE
#define EXTI_CH1_CH5_INT_VECTAB_NUM  (int)EXTI_CH1_CH5_INT_SERVICE    > 0 ? (int)EXTI_CH1_CH5_INT_SERVICE  - 1 : (int)EXTI_CH1_CH5_INT_SERVICE
#define EXTI_CH2_CH6_INT_VECTAB_NUM  (int)EXTI_CH2_CH6_INT_SERVICE    > 0 ? (int)EXTI_CH2_CH6_INT_SERVICE  - 1 : (int)EXTI_CH2_CH6_INT_SERVICE
#define EXTI_CH3_CH7_INT_VECTAB_NUM  (int)EXTI_CH3_CH7_INT_SERVICE    > 0 ? (int)EXTI_CH3_CH7_INT_SERVICE  - 1 : (int)EXTI_CH3_CH7_INT_SERVICE

#define DMA_INT_VECTAB_NUM           (int)DMA_INT_SERVICE             > 0 ? (int)DMA_INT_SERVICE           - 1 : (int)DMA_INT_SERVICE

#define UART0_INT_VECTAB_NUM         (int)UART0_INT_SERVICE           > 0 ? (int)UART0_INT_SERVICE         - 1 : (int)UART0_INT_SERVICE
#define UART1_INT_VECTAB_NUM         (int)UART1_INT_SERVICE           > 0 ? (int)UART1_INT_SERVICE         - 1 : (int)UART1_INT_SERVICE
#define UART2_INT_VECTAB_NUM         (int)UART2_INT_SERVICE           > 0 ? (int)UART2_INT_SERVICE         - 1 : (int)UART2_INT_SERVICE
#define UART3_INT_VECTAB_NUM         (int)UART3_INT_SERVICE           > 0 ? (int)UART3_INT_SERVICE         - 1 : (int)UART3_INT_SERVICE
#define UART4_INT_VECTAB_NUM         (int)UART4_INT_SERVICE           > 0 ? (int)UART4_INT_SERVICE         - 1 : (int)UART4_INT_SERVICE
#define UART5_INT_VECTAB_NUM         (int)UART5_INT_SERVICE           > 0 ? (int)UART5_INT_SERVICE         - 1 : (int)UART5_INT_SERVICE
#define UART6_INT_VECTAB_NUM         (int)UART6_INT_SERVICE           > 0 ? (int)UART6_INT_SERVICE         - 1 : (int)UART6_INT_SERVICE
#define UART8_INT_VECTAB_NUM         (int)UART8_INT_SERVICE           > 0 ? (int)UART8_INT_SERVICE         - 1 : (int)UART8_INT_SERVICE
#define UART9_INT_VECTAB_NUM         (int)UART9_INT_SERVICE           > 0 ? (int)UART9_INT_SERVICE         - 1 : (int)UART9_INT_SERVICE
#define UART10_INT_VECTAB_NUM        (int)UART10_INT_SERVICE          > 0 ? (int)UART10_INT_SERVICE        - 1 : (int)UART10_INT_SERVICE
#define UART11_INT_VECTAB_NUM        (int)UART11_INT_SERVICE          > 0 ? (int)UART11_INT_SERVICE        - 1 : (int)UART11_INT_SERVICE

#endif
