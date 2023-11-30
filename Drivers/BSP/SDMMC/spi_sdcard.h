/**
 ****************************************************************************************************
 * @file        spi_sdcard.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.1
 * @date        2020-06-02
 * @brief       SD卡 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32F103开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20200602
 * 第一次发布
 * V1.1 20221222
 * 1,新增sd_get_status函数,用于检测SD卡在位状态
 * 2,对ACMD命令添加重复多次发送检测,避免经常报错!(ACMD命令经常要发6次以上才能识别)
 *
 ****************************************************************************************************
 */

#ifndef __SPI_SDCARD_H
#define __SPI_SDCARD_H

#include "./BSP/SPI/spi.h"
#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/
/* SD卡 片选 引脚 定义 */

#define SD_CS_GPIO_PORT                 GPIOA
#define SD_CS_GPIO_PIN                  GPIO_PIN_3
#define SD_CS_GPIO_CLK_ENABLE()         do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)


/* SD卡 SPI 操作函数 宏定义
 * 大家移植的时候, 根据需要实现: spi1_read_write_byte 和 spi1_set_speed
 * 这两个函数即可, SD卡 SPI模式, 会通过这两个函数, 实现对SD卡的操作.
 */
#define sd_spi_read_write_byte(x)       spi1_read_write_byte(x)             /* SD卡 SPI读写函数 */
#define sd_spi_speed_low()              spi1_set_speed(SPI_SPEED_256)       /* SD卡 SPI低速模式 */
#define sd_spi_speed_high()             spi1_set_speed(SPI_SPEED_2)         /* SD卡 SPI高速模式 */


/******************************************************************************************/
/* SD_CS 端口定义 */
#define SD_CS(x)   do{ x ? \
                      HAL_GPIO_WritePin(SD_CS_GPIO_PORT, SD_CS_GPIO_PIN, GPIO_PIN_SET) : \
                      HAL_GPIO_WritePin(SD_CS_GPIO_PORT, SD_CS_GPIO_PIN, GPIO_PIN_RESET); \
                   }while(0)    /* SD_CS */

/******************************************************************************************/
/* SD卡 返回值定义 */
#define SD_OK           0
#define SD_ERROR        1


/* SD卡 类型定义 */
#define SD_TYPE_ERR     0X00
#define SD_TYPE_MMC     0X01
#define SD_TYPE_V1      0X02
#define SD_TYPE_V2      0X04
#define SD_TYPE_V2HC    0X06

/* SD卡 命令定义 */
#define CMD0    (0)             /* GO_IDLE_STATE */
#define CMD1    (1)             /* SEND_OP_COND (MMC) */
#define ACMD41  (0x80 + 41)     /* SEND_OP_COND (SDC) */
#define CMD8    (8)             /* SEND_IF_COND */
#define CMD9    (9)             /* SEND_CSD */
#define CMD10   (10)            /* SEND_CID */
#define CMD12   (12)            /* STOP_TRANSMISSION */
#define ACMD13  (0x80 + 13)     /* SD_STATUS (SDC) */
#define CMD16   (16)            /* SET_BLOCKLEN */
#define CMD17   (17)            /* READ_SINGLE_BLOCK */
#define CMD18   (18)            /* READ_MULTIPLE_BLOCK */
#define CMD23   (23)            /* SET_BLOCK_COUNT (MMC) */
#define ACMD23  (0x80 + 23)     /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24   (24)            /* WRITE_BLOCK */
#define CMD25   (25)            /* WRITE_MULTIPLE_BLOCK */
#define CMD32   (32)            /* ERASE_ER_BLK_START */
#define CMD33   (33)            /* ERASE_ER_BLK_END */
#define CMD38   (38)            /* ERASE */
#define CMD55   (55)            /* APP_CMD */
#define CMD58   (58)            /* READ_OCR */


/* SD卡的类型 */
extern uint8_t  sd_type;


/* 函数声明 */
static void sd_spi_init(void);                                      /* SD SPI 硬件初始化 */
static void sd_deselect(void);                                      /* SD卡取消选中 */
static uint8_t sd_select(void);                                     /* SD卡 选中 */
static uint8_t sd_wait_ready(void);                                 /* 等待SD卡准备好 */
static uint8_t sd_get_response(uint8_t response);                   /* 等待SD卡回应 */

static uint8_t sd_send_cmd(uint8_t cmd, uint32_t arg);              /* SD卡发送命令 */
static uint8_t sd_send_block(uint8_t *buf, uint8_t cmd);            /* SD卡发送一个数据块 */
static uint8_t sd_receive_data(uint8_t *buf, uint16_t len);         /* SD卡接收一次数据 */

uint8_t sd_init(void);                                              /* SD卡初始化 */
uint32_t sd_get_sector_count(void);                                 /* 获取SD卡的总扇区数(扇区数) */
uint8_t sd_get_status(void);                                        /* 获取SD卡状态 */
uint8_t sd_get_cid(uint8_t *cid_data);                              /* 获取SD卡的CID信息 */
uint8_t sd_get_csd(uint8_t *csd_data);                              /* 获取SD卡的CSD信息 */

uint8_t sd_read_disk(uint8_t *pbuf, uint32_t saddr, uint32_t cnt);  /* 读SD卡(fatfs/usb调用) */
uint8_t sd_write_disk(uint8_t *pbuf, uint32_t saddr, uint32_t cnt); /* 写SD卡(fatfs/usb调用) */

#endif


























