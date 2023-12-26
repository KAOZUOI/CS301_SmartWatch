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
/* LVGL */
#include "lvgl.h"
#include "lv_port_indev_template.h"
#include "lv_port_disp_template.h"
#include "lvgl_demo.h"



/**
 * @brief       得到path路径下,目标文件的总个数
 * @param       path : 路径
 * @retval      总有效文件数
 */
uint16_t pic_get_tnum(char *path)
{
    uint8_t res;
    uint16_t rval = 0;
    DIR tdir;           /* 临时目录 */
    FILINFO *tfileinfo; /* 临时文件信息 */
    tfileinfo = (FILINFO *)mymalloc(SRAMIN, sizeof(FILINFO));   /* 申请内存 */
    res = f_opendir(&tdir, (const TCHAR *)path);    /* 打开目录 */

    if (res == FR_OK && tfileinfo)
    {
        while (1)       /* 查询总的有效文件数 */
        {
            res = f_readdir(&tdir, tfileinfo);  /* 读取目录下的一个文件 */

            if (res != FR_OK || tfileinfo->fname[0] == 0)break; /* 错误了/到末尾了,退出 */

            res = exfuns_file_type(tfileinfo->fname);

            if ((res & 0XF0) == 0X50)   /* 取高四位,看看是不是图片文件 */
            {
                rval++; /* 有效文件数增加1 */
            }
        }
    }

    myfree(SRAMIN, tfileinfo);  /* 释放内存 */
    return rval;
}

void picture_main()
{
    uint8_t res;
    DIR picdir;                 /* 图片目录 */
    FILINFO *picfileinfo;       /* 文件信息 */
    char *pname;             /* 带路径的文件名 */
    uint16_t totpicnum;         /* 图片文件总数 */
    uint16_t curindex;          /* 图片当前索引 */
    uint8_t key;                /* 键值 */
    uint8_t pause = 0;          /* 暂停标记 */
    uint8_t t;
    uint16_t temp;
    uint32_t *picoffsettbl;     /* 图片文件offset索引表 */
	
	
    norflash_init();                    /* 初始化NORFLASH */
    my_mem_init(SRAMIN);                /* 初始化内部SRAM内存池 */

    exfuns_init();                      /* 为fatfs相关变量申请内存 */
    f_mount(fs[0], "0:", 1);            /* 挂载SD卡 */
    f_mount(fs[1], "1:", 1);            /* 挂载FLASH */

    f_mount(fs[0], "0:", 1);            /* 挂载SD卡 */
    f_mount(fs[1], "1:", 1);            /* 挂载FLASH */

    while (fonts_init())                /* 检查字库 */
    {
        lcd_show_string(30, 50, 200, 16, 16, "Font Error!", RED);
        delay_ms(200);
        lcd_fill(30, 50, 240, 66, WHITE);   /* 清除显示 */
        delay_ms(200);
    }

    text_show_string(30, 50, 200, 16, "正点原子STM32开发板", 16, 0, RED);
    text_show_string(30, 70, 200, 16, "图片显示 实验", 16, 0, RED);
    text_show_string(30, 90, 200, 16, "KEY0:NEXT KEY1:PREV", 16, 0, RED);
    text_show_string(30, 110, 200, 16, "KEY_UP:PAUSE", 16, 0, RED);
    text_show_string(30, 130, 200, 16, "正点原子@ALIENTEK", 16, 0, RED);


    while (f_opendir(&picdir, "0:/PICTURE"))    /* 打开图片文件夹 */
    {
        text_show_string(30, 150, 240, 16, "PICTURE文件夹错误!", 16, 0, RED);
        delay_ms(200);
        lcd_fill(30, 150, 240, 186, WHITE);     /* 清除显示 */
        delay_ms(200);
    }

    totpicnum = pic_get_tnum("0:/PICTURE");  /* 得到总有效文件数 */

    while (totpicnum == NULL)   /* 图片文件为0 */
    {
        text_show_string(30, 150, 240, 16, "没有图片文件!", 16, 0, RED);
        delay_ms(200);
        lcd_fill(30, 150, 240, 186, WHITE); /* 清除显示 */
        delay_ms(200);
    }

    picfileinfo = (FILINFO *)mymalloc(SRAMIN, sizeof(FILINFO)); /* 申请内存 */
    pname = mymalloc(SRAMIN, FF_MAX_LFN * 2 + 1);   /* 为带路径的文件名分配内存 */
    picoffsettbl = mymalloc(SRAMIN, 4 * totpicnum); /* 申请4*totpicnum个字节的内存,用于存放图片索引 */

    while (!picfileinfo || !pname || !picoffsettbl) /* 内存分配出错 */
    {
        text_show_string(30, 150, 240, 16, "内存分配失败!", 16, 0, RED);
        delay_ms(200);
        lcd_fill(30, 150, 240, 186, WHITE);         /* 清除显示 */
        delay_ms(200);
    }

    /* 记录索引 */
    res = f_opendir(&picdir, "0:/PICTURE");         /* 打开目录 */

    if (res == FR_OK)
    {
        curindex = 0;   /* 当前索引为0 */

        while (1)       /* 全部查询一遍 */
        {
            temp = picdir.dptr; /* 记录当前dptr偏移 */
            res = f_readdir(&picdir, picfileinfo);  /* 读取目录下的一个文件 */

            if (res != FR_OK || picfileinfo->fname[0] == 0)break;   /* 错误了/到末尾了,退出 */

            res = exfuns_file_type(picfileinfo->fname);

            if ((res & 0XF0) == 0X50)   /* 取高四位,看看是不是图片文件 */
            {
                picoffsettbl[curindex] = temp;  /* 记录索引 */
                curindex++;
            }
        }
    }
		
		char str_num[20];
		sprintf(str_num, "PICTURE NUMBER: %hu", totpicnum);
		text_show_string(30, 150, 240, 16, (char*)str_num, 16, 0, RED);
    text_show_string(30, 170, 240, 16, "开始显示...", 16, 0, RED);
    delay_ms(1500);
    piclib_init();  /* 初始化画图 */
    curindex = 0;   /* 从0开始显示 */
    res = f_opendir(&picdir, (const TCHAR *)"0:/PICTURE");  /* 打开目录 */

    while (res == FR_OK)   /* 打开成功 */
    {
        dir_sdi(&picdir, picoffsettbl[curindex]);   /* 改变当前目录索引 */
        res = f_readdir(&picdir, picfileinfo);      /* 读取目录下的一个文件 */

        if (res != FR_OK || picfileinfo->fname[0] == 0)break;   /* 错误了/到末尾了,退出 */

        strcpy((char *)pname, "0:/PICTURE/");       /* 复制路径(目录) */
        strcat((char *)pname, (const char *)picfileinfo->fname);        /* 将文件名接在后面 */
        lcd_clear(BLACK);
        piclib_ai_load_picfile(pname, 0, 0, lcddev.width, lcddev.height, 1);    /* 显示图片 */
        text_show_string(2, 2, lcddev.width, 16, (char*)pname, 16, 1, RED);     /* 显示图片名字 */

        while (1)
        {
						uint8_t prekey = key;
						int flag = 0;
						char *str = 0;
            key = remote_scan();      /* 扫描按键 */
						
						if (key && key != prekey)
						{
								// lcd_show_num(86, 110, key, 3, 16, BLUE);          /* 显示键值 */
								// lcd_show_num(86, 130, g_remote_cnt, 3, 16, BLUE); /* 显示按键次数 */

								switch (key)
								{

										case 67:
												str = "RIGHT";
												flag = 1;
										    curindex++;
												if (curindex >= totpicnum)curindex = 0; /* 到末尾的时候,自动从头开始 */
												break;

										case 68:
												str = "LEFT";
												flag = 1;
										    if (curindex)
												{
														curindex--;
												}
												else
												{
														curindex = totpicnum - 1;
												}
												break;
								}

								// lcd_fill(86, 150, 116 + 8 * 8, 170 + 16, WHITE);    /* 清楚之前的显示 */
								// lcd_show_string(86, 150, 200, 16, 16, str, BLUE);   /* 显示SYMBOL */
						}
						else
						{
								delay_ms(10);
						}
						
						if(flag == 1){
							delay_ms(200);
							break;
						}
				
        }

    }

    myfree(SRAMIN, picfileinfo);    /* 释放内存 */
    myfree(SRAMIN, pname);          /* 释放内存 */
    myfree(SRAMIN, picoffsettbl);   /* 释放内存 */
}


int main(void)
{



    HAL_Init();                         /* 初始化HAL库 */
    sys_stm32_clock_init(RCC_PLL_MUL9); /* 设置时钟, 72Mhz */
    delay_init(72);                     /* 延时初始化 */
    usart_init(115200);                 /* 串口初始化为115200 */
    usmart_dev.init(72);                /* 初始化USMART */
    led_init();                         /* 初始化LED */
    lcd_init();                         /* 初始化LCD */
    key_init();                         /* 初始化按键 */
	  remote_init();                      /* 红外接收初始化 */
    // norflash_init();                    /* 初始化NORFLASH */
    // my_mem_init(SRAMIN);                /* 初始化内部SRAM内存池 */
    // exfuns_init();                      /* 为fatfs相关变量申请内存 */
		
    lvgl_demo();                        /* 运行FreeRTOS例程 */
		// remote_main();
		picture_main();
}
