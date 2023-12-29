
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
#include "./BSP/RTC/rtc.h"

/* LVGL */
#include "lvgl.h"
#include "lv_port_indev_template.h"
#include "lv_port_disp_template.h"
#include "lvgl_demo.h"

int main(void) {
    HAL_Init();                         /* 初始化HAL库 */
    sys_stm32_clock_init(RCC_PLL_MUL9); /* 设置时钟, 72Mhz */
    delay_init(72);
    usart_init(115200);
    usmart_dev.init(72);
    led_init();
    lcd_init();
    key_init();
    rtc_init();

    my_mem_init(SRAMIN);
    lvgl_demo();
}
