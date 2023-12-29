/**
 ****************************************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2020-04-28
 * @brief       LVGL lv_imgbtn(图片按钮) 实验
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台：正点原子 MiniSTM32 V4开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
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
 * @brief       清空屏幕并在右上角显示"RST"
 * @param       无
 * @retval      无
 */
void load_draw_dialog(void)
{
    lcd_clear(WHITE);                                                /* 清屏 */
    lcd_show_string(lcddev.width - 24, 0, 200, 16, 16, "RST", BLUE); /* 显示清屏区域 */
}

/**
 * @brief       画粗线
 * @param       x1,y1: 起点坐标
 * @param       x2,y2: 终点坐标
 * @param       size : 线条粗细程度
 * @param       color: 线的颜色
 * @retval      无
 */
void lcd_draw_bline(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t size, uint16_t color)
{
    uint16_t t;
    int xerr = 0, yerr = 0, delta_x, delta_y, distance;
    int incx, incy, row, col;

    if (x1 < size || x2 < size || y1 < size || y2 < size)
        return;

    delta_x = x2 - x1; /* 计算坐标增量 */
    delta_y = y2 - y1;
    row = x1;
    col = y1;

    if (delta_x > 0)
    {
        incx = 1; /* 设置单步方向 */
    }
    else if (delta_x == 0)
    {
        incx = 0; /* 垂直线 */
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
        incy = 0; /* 水平线 */
    }
    else
    {
        incy = -1;
        delta_y = -delta_y;
    }

    if (delta_x > delta_y)
        distance = delta_x;       /* 选取基本增量坐标轴 */
    else
        distance = delta_y;

    for (t = 0; t <= distance + 1; t++)     /* 画线输出 */
    {
        lcd_fill_circle(row, col, size, color); /* 画点 */
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
 * @brief       测试SD卡的读取
 *   @note      从secaddr地址开始,读取seccnt个扇区的数据
 * @param       secaddr : 扇区地址
 * @param       seccnt  : 扇区数
 * @retval      无
 */
void sd_test_read(uint32_t secaddr, uint32_t seccnt)
{
    uint32_t i;
    uint8_t *buf;
    uint8_t sta = 0;
    buf = mymalloc(SRAMIN, seccnt * 512);       /* 申请内存,从SRAM申请内存 */
    sta = sd_read_disk(buf, secaddr, seccnt);   /* 读取secaddr扇区开始的内容 */

    if (sta == 0)
    {
        printf("SECTOR %d DATA:\r\n", secaddr);

        for (i = 0; i < seccnt * 512; i++)
        {
            printf("%x ", buf[i]);              /* 打印secaddr开始的扇区数据 */
        }

        printf("\r\nDATA ENDED\r\n");
    }
    else
    {
        printf("err:%d\r\n", sta);
    }

    myfree(SRAMIN, buf);    /* 释放内存 */
}

/**
 * @brief       测试SD卡的写入
 *   @note      从secaddr地址开始,写入seccnt个扇区的数据
 *              慎用!! 最好写全是0XFF的扇区,否则可能损坏SD卡.
 *
 * @param       secaddr : 扇区地址
 * @param       seccnt  : 扇区数
 * @retval      无
 */
void sd_test_write(uint32_t secaddr, uint32_t seccnt)
{
    uint32_t i;
    uint8_t *buf;
    uint8_t sta = 0;
    buf = mymalloc(SRAMIN, seccnt * 512);       /* 从SRAM申请内存 */

    for (i = 0; i < seccnt * 512; i++)          /* 初始化写入的数据,是3的倍数. */
    {
        buf[i] = i * 3;
    }

    sta = sd_write_disk(buf, secaddr, seccnt);  /* 从secaddr扇区开始写入seccnt个扇区内容 */

    if (sta == 0)
    {
        printf("Write over!\r\n");
    }
    else
    {
        printf("err:%d\r\n", sta);
    }

    myfree(SRAMIN, buf);    /* 释放内存 */
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

    while (sd_init())   /* 检测不到SD卡 */
    {
        lcd_show_string(30, 130, 200, 16, 16, "SD Card Error!", RED);
        delay_ms(500);
        lcd_show_string(30, 130, 200, 16, 16, "Please Check! ", RED);
        delay_ms(500);
        LED0_TOGGLE();  /* 红灯闪烁 */
    }

    /* 检测SD卡成功 */
    lcd_show_string(30, 130, 200, 16, 16, "SD Card OK    ", BLUE);
    lcd_show_string(30, 150, 200, 16, 16, "SD Card Size:     MB", BLUE);

    sd_size = sd_get_sector_count();  /* 得到扇区数 */
    lcd_show_num(30 + 13 * 8, 150, sd_size >> 11, 5, 16, BLUE); /* 显示SD卡容量, 转换成MB单位 */

    while (1)
    {
        key = key_scan(0);

        if (key == KEY0_PRES)               /* KEY0按下了 */
        {
            buf = mymalloc(0, 512);         /* 申请内存 */
            key = sd_read_disk(buf, 0, 1);  /* 读取0扇区的内容 */

            if (key == 0)
            {
                lcd_show_string(30, 170, 200, 16, 16, "USART1 Sending Data...", BLUE);
                printf("SECTOR 0 DATA:\r\n");

                for (sd_size = 0; sd_size < 512; sd_size++)
                {
                    printf("%x ", buf[sd_size]);    /* 打印0扇区数据 */
                }

                printf("\r\nDATA ENDED\r\n");
                lcd_show_string(30, 170, 200, 16, 16, "USART1 Send Data Over!", BLUE);
            }
            else
            {
                printf("err:%d\r\n", key);
            }

            myfree(0, buf); /* 释放内存 */
        }

        t++;
        delay_ms(10);

        if (t == 20)
        {
            LED0_TOGGLE();  /* 红灯闪烁 */
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
            lcd_show_num(86, 110, key, 3, 16, BLUE);          /* 显示键值 */
            lcd_show_num(86, 130, g_remote_cnt, 3, 16, BLUE); /* 显示按键次数 */

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

            lcd_fill(86, 150, 116 + 8 * 8, 170 + 16, WHITE);    /* 清楚之前的显示 */
            lcd_show_string(86, 150, 200, 16, 16, str, BLUE);   /* 显示SYMBOL */
        }
        else
        {
            delay_ms(10);
        }

        t++;

        if (t == 20)
        {
            t = 0;
            LED0_TOGGLE();  /* LED0闪烁 */
        }
    }
}

int main(void)
{



    HAL_Init();                         /* 初始化HAL库 */
    sys_stm32_clock_init(RCC_PLL_MUL9); /* 设置时钟, 72Mhz */
    delay_init(72);
    usart_init(115200);
    usmart_dev.init(72);
    led_init();
    lcd_init();
    key_init();

    my_mem_init(SRAMIN);
     lvgl_demo();
}
