/**
 ****************************************************************************************************
 * @file        main.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.0
 * @date        2020-04-28
 * @brief       LVGL lv_imgbtn(ͼƬ��ť) ʵ��
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨������ԭ�� MiniSTM32 V4������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./USMART/usmart.h"
#include "./MALLOC/malloc.h"
#include "./FATFS/exfuns/exfuns.h"
#include "./TEXT/text.h"
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/KEY/key.h"
#include "./BSP/TIMER/btim.h"
#include "./BSP/TOUCH/touch.h"
#include "./BSP/REMOTE/remote.h"
#include "./BSP/SDMMC/spi_sdcard.h"
#include "./BSP/NORFLASH/norflash.h"
#include "./PICTURE/piclib.h"
#include "string.h"
#include "math.h"
#include "picture.h"
/* LVGL */
#include "lvgl.h"
#include "lv_port_indev_template.h"
#include "lv_port_disp_template.h"
#include "lvgl_demo.h"


int main(void)
{



    HAL_Init();                         /* ��ʼ��HAL�� */
    sys_stm32_clock_init(RCC_PLL_MUL9); /* ����ʱ��, 72Mhz */
    delay_init(72);                     /* ��ʱ��ʼ�� */
    usart_init(115200);                 /* ���ڳ�ʼ��Ϊ115200 */
    usmart_dev.init(72);                /* ��ʼ��USMART */
    led_init();                         /* ��ʼ��LED */
    lcd_init();                         /* ��ʼ��LCD */
    key_init();                         /* ��ʼ������ */
	  remote_init();                      /* ������ճ�ʼ�� */
    // norflash_init();                    /* ��ʼ��NORFLASH */
    // my_mem_init(SRAMIN);                /* ��ʼ���ڲ�SRAM�ڴ�� */
    // exfuns_init();                      /* Ϊfatfs��ر��������ڴ� */
		
    // lvgl_demo();                        /* ����FreeRTOS���� */
		// remote_main();
		picture_main();
}
