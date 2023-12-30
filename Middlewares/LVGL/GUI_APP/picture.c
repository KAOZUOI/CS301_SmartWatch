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



/**
 * @brief       �õ�path·����,Ŀ���ļ����ܸ���
 * @param       path : ·��
 * @retval      ����Ч�ļ���
 */
uint16_t pic_get_tnum(char *path)
{
    uint8_t res;
    uint16_t rval = 0;
    DIR tdir;           /* ��ʱĿ¼ */
    FILINFO *tfileinfo; /* ��ʱ�ļ���Ϣ */
    tfileinfo = (FILINFO *)mymalloc(SRAMIN, sizeof(FILINFO));   /* �����ڴ� */
    res = f_opendir(&tdir, (const TCHAR *)path);    /* ��Ŀ¼ */

    if (res == FR_OK && tfileinfo)
    {
        while (1)       /* ��ѯ�ܵ���Ч�ļ��� */
        {
            res = f_readdir(&tdir, tfileinfo);  /* ��ȡĿ¼�µ�һ���ļ� */

            if (res != FR_OK || tfileinfo->fname[0] == 0)break; /* ������/��ĩβ��,�˳� */

            res = exfuns_file_type(tfileinfo->fname);

            if ((res & 0XF0) == 0X50)   /* ȡ����λ,�����ǲ���ͼƬ�ļ� */
            {
                rval++; /* ��Ч�ļ�������1 */
            }
        }
    }

    myfree(SRAMIN, tfileinfo);  /* �ͷ��ڴ� */
    return rval;
}

void picture_main()
{
    uint8_t res;
    DIR picdir;                 /* ͼƬĿ¼ */
    FILINFO *picfileinfo;       /* �ļ���Ϣ */
    char *pname;             /* ��·�����ļ��� */
    uint16_t totpicnum;         /* ͼƬ�ļ����� */
    uint16_t curindex;          /* ͼƬ��ǰ���� */
    uint8_t key;                /* ��ֵ */
    uint16_t temp;
    uint32_t *picoffsettbl;     /* ͼƬ�ļ�offset������ */
	
	
    norflash_init();                    /* ��ʼ��NORFLASH */
    my_mem_init(SRAMIN);                /* ��ʼ���ڲ�SRAM�ڴ�� */
		tp_dev.init();

    exfuns_init();                      /* Ϊfatfs��ر��������ڴ� */
    f_mount(fs[0], "0:", 1);            /* ����SD�� */
    f_mount(fs[1], "1:", 1);            /* ����FLASH */

    f_mount(fs[0], "0:", 1);            /* ����SD�� */
    f_mount(fs[1], "1:", 1);            /* ����FLASH */

    while (fonts_init())                /* ����ֿ� */
    {
        lcd_show_string(30, 50, 200, 16, 16, "Font Error!", RED);
        delay_ms(200);
        lcd_fill(30, 50, 240, 66, WHITE);   /* �����ʾ */
        delay_ms(200);
    }

    text_show_string(30, 50, 200, 16, "����ԭ��STM32������", 16, 0, RED);
    text_show_string(30, 70, 200, 16, "ͼƬ��ʾ ʵ��", 16, 0, RED);
    text_show_string(30, 90, 200, 16, "KEY0:NEXT KEY1:PREV", 16, 0, RED);
    text_show_string(30, 110, 200, 16, "KEY_UP:PAUSE", 16, 0, RED);
    text_show_string(30, 130, 200, 16, "����ԭ��@ALIENTEK", 16, 0, RED);


    while (f_opendir(&picdir, "0:/PICTURE"))    /* ��ͼƬ�ļ��� */
    {
        text_show_string(30, 150, 240, 16, "PICTURE�ļ��д���!", 16, 0, RED);
        delay_ms(200);
        lcd_fill(30, 150, 240, 186, WHITE);     /* �����ʾ */
        delay_ms(200);
    }

    totpicnum = pic_get_tnum("0:/PICTURE");  /* �õ�����Ч�ļ��� */

    while (totpicnum == NULL)   /* ͼƬ�ļ�Ϊ0 */
    {
        text_show_string(30, 150, 240, 16, "û��ͼƬ�ļ�!", 16, 0, RED);
        delay_ms(200);
        lcd_fill(30, 150, 240, 186, WHITE); /* �����ʾ */
        delay_ms(200);
    }

    picfileinfo = (FILINFO *)mymalloc(SRAMIN, sizeof(FILINFO)); /* �����ڴ� */
    pname = mymalloc(SRAMIN, FF_MAX_LFN * 2 + 1);   /* Ϊ��·�����ļ��������ڴ� */
    picoffsettbl = mymalloc(SRAMIN, 4 * totpicnum); /* ����4*totpicnum���ֽڵ��ڴ�,���ڴ��ͼƬ���� */

    while (!picfileinfo || !pname || !picoffsettbl) /* �ڴ������� */
    {
        text_show_string(30, 150, 240, 16, "�ڴ����ʧ��!", 16, 0, RED);
        delay_ms(200);
        lcd_fill(30, 150, 240, 186, WHITE);         /* �����ʾ */
        delay_ms(200);
    }

    /* ��¼���� */
    res = f_opendir(&picdir, "0:/PICTURE");         /* ��Ŀ¼ */

    if (res == FR_OK)
    {
        curindex = 0;   /* ��ǰ����Ϊ0 */

        while (1)       /* ȫ����ѯһ�� */
        {
            temp = picdir.dptr; /* ��¼��ǰdptrƫ�� */
            res = f_readdir(&picdir, picfileinfo);  /* ��ȡĿ¼�µ�һ���ļ� */

            if (res != FR_OK || picfileinfo->fname[0] == 0)break;   /* ������/��ĩβ��,�˳� */

            res = exfuns_file_type(picfileinfo->fname);

            if ((res & 0XF0) == 0X50)   /* ȡ����λ,�����ǲ���ͼƬ�ļ� */
            {
                picoffsettbl[curindex] = temp;  /* ��¼���� */
                curindex++;
            }
        }
    }
		
		char str_num[20];
		sprintf(str_num, "PICTURE NUMBER: %hu", totpicnum);
		text_show_string(30, 150, 240, 16, (char*)str_num, 16, 0, RED);
    text_show_string(30, 170, 240, 16, "��ʼ��ʾ...", 16, 0, RED);
    delay_ms(1500);
    piclib_init();  /* ��ʼ����ͼ */
    curindex = 0;   /* ��0��ʼ��ʾ */
    res = f_opendir(&picdir, (const TCHAR *)"0:/PICTURE");  /* ��Ŀ¼ */

    while (res == FR_OK)   /* �򿪳ɹ� */
    {
				if(curindex == 11451) break;
        dir_sdi(&picdir, picoffsettbl[curindex]);   /* �ı䵱ǰĿ¼���� */
        res = f_readdir(&picdir, picfileinfo);      /* ��ȡĿ¼�µ�һ���ļ� */

        if (res != FR_OK || picfileinfo->fname[0] == 0)break;   /* ������/��ĩβ��,�˳� */

        strcpy((char *)pname, "0:/PICTURE/");       /* ����·��(Ŀ¼) */
        strcat((char *)pname, (const char *)picfileinfo->fname);        /* ���ļ������ں��� */
        lcd_clear(BLACK);
        piclib_ai_load_picfile(pname, 0, 0, lcddev.width, lcddev.height, 1);    /* ��ʾͼƬ */
        text_show_string(2, 2, lcddev.width, 16, (char*)pname, 16, 1, RED);     /* ��ʾͼƬ���� */

        while (1)
        {
						uint8_t prekey = key;
						int flag = 0;
            key = remote_scan();      /* ɨ�谴�� */
						
						tp_dev.scan(0);

						if (tp_dev.sta & TP_PRES_DOWN)  /* ������������ */
						{
								if (tp_dev.x[0] < lcddev.width && tp_dev.y[0] < lcddev.height)
								{
										if (tp_dev.x[0] < 120)
										{
												flag = 1;
										    if (curindex)
												{
														curindex--;
												}
												else
												{
														curindex = totpicnum - 1;
												}
										}
										else 
										{
												flag = 1;
										    curindex++;
												if (curindex >= totpicnum)curindex = 0; /* ��ĩβ��ʱ��,�Զ���ͷ��ʼ */
										}
								}
						}
						else 
						{
								delay_ms(10);       /* û�а������µ�ʱ�� */
						}
						
						if(flag == 1){
							delay_ms(200);
							break;
						}
						
						if (key && key != prekey)
						{
								// lcd_show_num(86, 110, key, 3, 16, BLUE);          /* ��ʾ��ֵ */
								// lcd_show_num(86, 130, g_remote_cnt, 3, 16, BLUE); /* ��ʾ�������� */

								switch (key)
								{

										case 67:
												flag = 1;
										    curindex++;
												if (curindex >= totpicnum)curindex = 0; /* ��ĩβ��ʱ��,�Զ���ͷ��ʼ */
												break;

										case 68:
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
										case 74:
											flag = 1;
											curindex = 11451;
								}

								// lcd_fill(86, 150, 116 + 8 * 8, 170 + 16, WHITE);    /* ���֮ǰ����ʾ */
								// lcd_show_string(86, 150, 200, 16, 16, str, BLUE);   /* ��ʾSYMBOL */
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
		
		lcd_clear(WHITE);
		lcd_show_string(86, 150, 200, 16, 16, "BACK", BLUE);
		

    myfree(SRAMIN, picfileinfo);    /* �ͷ��ڴ� */
    myfree(SRAMIN, pname);          /* �ͷ��ڴ� */
    myfree(SRAMIN, picoffsettbl);   /* �ͷ��ڴ� */
}



