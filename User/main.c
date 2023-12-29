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
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/KEY/key.h"
#include "./MALLOC/malloc.h"
#include "./BSP/TIMER/btim.h"
#include "./BSP/TOUCH/touch.h"
#include "./BSP/SDMMC/spi_sdcard.h"
#include "./BSP/REMOTE/remote.h"

/* LVGL */
#include "lvgl.h"
#include "lv_port_indev_template.h"
#include "lv_port_disp_template.h"
#include "lvgl_demo.h"

/**
 * @brief       �����Ļ�������Ͻ���ʾ"RST"
 * @param       ��
 * @retval      ��
 */
void load_draw_dialog(void)
{
    lcd_clear(WHITE);                                                /* ���� */
    lcd_show_string(lcddev.width - 24, 0, 200, 16, 16, "RST", BLUE); /* ��ʾ�������� */
}

/**
 * @brief       ������
 * @param       x1,y1: �������
 * @param       x2,y2: �յ�����
 * @param       size : ������ϸ�̶�
 * @param       color: �ߵ���ɫ
 * @retval      ��
 */
void lcd_draw_bline(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t size, uint16_t color)
{
    uint16_t t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, row, col;

    if (x1 < size || x2 < size || y1 < size || y2 < size)
        return;

    delta_x = x2 - x1; /* ������������ */
    delta_y = y2 - y1;
    row = x1;
    col = y1;

    if (delta_x > 0)
    {
        incx = 1; /* ���õ������� */
    }
    else if (delta_x == 0)
    {
        incx = 0; /* ��ֱ�� */
    }
    else
    {
        incx = -1;
        delta_x = -delta_x;
    }

    if (delta_y > 0)
    {
        incy = 1;
    }
    else if (delta_y == 0)
    {
        incy = 0; /* ˮƽ�� */
    }
    else
    {
        incy = -1;
        delta_y = -delta_y;
    }

    if (delta_x > delta_y)
        distance = delta_x;       /* ѡȡ�������������� */
    else
        distance = delta_y;

    for (t = 0; t <= distance + 1; t++)     /* ������� */
    {
        lcd_fill_circle(row, col, size, color); /* ���� */
        xerr += delta_x;
        yerr += delta_y;

        if (xerr > distance)
        {
            xerr -= distance;
            row += incx;
        }

        if (yerr > distance)
        {
            yerr -= distance;
            col += incy;
        }
    }
}

/**
 * @brief       ����SD���Ķ�ȡ
 *   @note      ��secaddr��ַ��ʼ,��ȡseccnt������������
 * @param       secaddr : ������ַ
 * @param       seccnt  : ������
 * @retval      ��
 */
void sd_test_read(uint32_t secaddr, uint32_t seccnt)
{
    uint32_t i;
    uint8_t *buf;
    uint8_t sta = 0;
    buf = mymalloc(SRAMIN, seccnt * 512);       /* �����ڴ�,��SRAM�����ڴ� */
    sta = sd_read_disk(buf, secaddr, seccnt);   /* ��ȡsecaddr������ʼ������ */

    if (sta == 0)
    {
        printf("SECTOR %d DATA:\r\n", secaddr);

        for (i = 0; i < seccnt * 512; i++)
        {
            printf("%x ", buf[i]);              /* ��ӡsecaddr��ʼ���������� */
        }

        printf("\r\nDATA ENDED\r\n");
    }
    else
    {
        printf("err:%d\r\n", sta);
    }

    myfree(SRAMIN, buf);    /* �ͷ��ڴ� */
}

/**
 * @brief       ����SD����д��
 *   @note      ��secaddr��ַ��ʼ,д��seccnt������������
 *              ����!! ���дȫ��0XFF������,���������SD��.
 *
 * @param       secaddr : ������ַ
 * @param       seccnt  : ������
 * @retval      ��
 */
void sd_test_write(uint32_t secaddr, uint32_t seccnt)
{
    uint32_t i;
    uint8_t *buf;
    uint8_t sta = 0;
    buf = mymalloc(SRAMIN, seccnt * 512);       /* ��SRAM�����ڴ� */

    for (i = 0; i < seccnt * 512; i++)          /* ��ʼ��д�������,��3�ı���. */
    {
        buf[i] = i * 3;
    }

    sta = sd_write_disk(buf, secaddr, seccnt);  /* ��secaddr������ʼд��seccnt���������� */

    if (sta == 0)
    {
        printf("Write over!\r\n");
    }
    else
    {
        printf("err:%d\r\n", sta);
    }

    myfree(SRAMIN, buf);    /* �ͷ��ڴ� */
}

void sd_main(){
    uint8_t key;
    uint32_t sd_size;
    uint8_t t = 0;
    uint8_t *buf;

    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "SD TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 110, 200, 16, 16, "KEY0:Read Sector 0", RED);

    while (sd_init())   /* ��ⲻ��SD�� */
    {
        lcd_show_string(30, 130, 200, 16, 16, "SD Card Error!", RED);
        delay_ms(500);
        lcd_show_string(30, 130, 200, 16, 16, "Please Check! ", RED);
        delay_ms(500);
        LED0_TOGGLE();  /* �����˸ */
    }

    /* ���SD���ɹ� */
    lcd_show_string(30, 130, 200, 16, 16, "SD Card OK    ", BLUE);
    lcd_show_string(30, 150, 200, 16, 16, "SD Card Size:     MB", BLUE);

    sd_size = sd_get_sector_count();  /* �õ������� */
    lcd_show_num(30 + 13 * 8, 150, sd_size >> 11, 5, 16, BLUE); /* ��ʾSD������, ת����MB��λ */

    while (1)
    {
        key = key_scan(0);

        if (key == KEY0_PRES)               /* KEY0������ */
        {
            buf = mymalloc(0, 512);         /* �����ڴ� */
            key = sd_read_disk(buf, 0, 1);  /* ��ȡ0���������� */

            if (key == 0)
            {
                lcd_show_string(30, 170, 200, 16, 16, "USART1 Sending Data...", BLUE);
                printf("SECTOR 0 DATA:\r\n");

                for (sd_size = 0; sd_size < 512; sd_size++)
                {
                    printf("%x ", buf[sd_size]);    /* ��ӡ0�������� */
                }

                printf("\r\nDATA ENDED\r\n");
                lcd_show_string(30, 170, 200, 16, 16, "USART1 Send Data Over!", BLUE);
            }
            else
            {
                printf("err:%d\r\n", key);
            }

            myfree(0, buf); /* �ͷ��ڴ� */
        }

        t++;
        delay_ms(10);

        if (t == 20)
        {
            LED0_TOGGLE();  /* �����˸ */
            t = 0;
        }
    }
}

void remote_main()
{
    uint8_t key;
    uint8_t t = 0;
    char *str = 0;

    lcd_show_string(30, 50, 200, 16, 16, "STM32", RED);
    lcd_show_string(30, 70, 200, 16, 16, "REMOTE TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "ATOM@ALIENTEK", RED);
    lcd_show_string(30, 110, 200, 16, 16, "KEYVAL:", RED);
    lcd_show_string(30, 130, 200, 16, 16, "KEYCNT:", RED);
    lcd_show_string(30, 150, 200, 16, 16, "SYMBOL:", RED);

    while (1)
    {
        key = remote_scan();

        if (key)
        {
            lcd_show_num(86, 110, key, 3, 16, BLUE);          /* ��ʾ��ֵ */
            lcd_show_num(86, 130, g_remote_cnt, 3, 16, BLUE); /* ��ʾ�������� */

                        switch (key)
            {
                case 0:
                    str = "ERROR";
                    break;

                case 69:
                    str = "POWER";
                    break;

                case 70:
                    str = "UP";
                    break;

                case 64:
                    str = "PLAY";
                    break;

                case 71:
                    str = "ALIENTEK";
                    break;

                case 67:
                    str = "RIGHT";
                    break;

                case 68:
                    str = "LEFT";
                    break;

                case 7:
                    str = "VOL-";
                    break;

                case 21:
                    str = "DOWN";
                    break;

                case 9:
                    str = "VOL+";
                    break;

                case 22:
                    str = "1";
                    break;

                case 25:
                    str = "2";
                    break;

                case 13:
                    str = "3";
                    break;

                case 12:
                    str = "4";
                    break;

                case 24:
                    str = "5";
                    break;

                case 94:
                    str = "6";
                    break;

                case 8:
                    str = "7";
                    break;

                case 28:
                    str = "8";
                    break;

                case 90:
                    str = "9";
                    break;

                case 66:
                    str = "0";
                    break;

                case 74:
                    str = "DELETE";
                    break;
            }

            lcd_fill(86, 150, 116 + 8 * 8, 170 + 16, WHITE);    /* ���֮ǰ����ʾ */
            lcd_show_string(86, 150, 200, 16, 16, str, BLUE);   /* ��ʾSYMBOL */
        }
        else
        {
            delay_ms(10);
        }

        t++;

        if (t == 20)
        {
            t = 0;
            LED0_TOGGLE();  /* LED0��˸ */
        }
    }
}

int main(void)
{



    HAL_Init();                         /* ��ʼ��HAL�� */
    sys_stm32_clock_init(RCC_PLL_MUL9); /* ����ʱ��, 72Mhz */
    delay_init(72);
    usart_init(115200);
    usmart_dev.init(72);
    led_init();
    lcd_init();
    key_init();

    my_mem_init(SRAMIN);
     lvgl_demo();
}
