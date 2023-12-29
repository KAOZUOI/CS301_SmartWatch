#include "LVGL/GUI_APP/lv_mainstart.h"
#include "lvgl.h"
#include "./BSP/LED/led.h"
#include "lv_calc.h"
#include <stdio.h>
#include <string.h>


#define scr_act_width()  lv_obj_get_width(lv_scr_act())
#define scr_act_height() lv_obj_get_height(lv_scr_act())

static lv_obj_t *btn_mode;          /* mode_change */

/**
 * @brief  mode change按钮
 * @param  无
 * @return 无
 */
static void lv_btn_mode(void)
{
    btn_mode = lv_btn_create(lv_scr_act());                                         /* 创建按钮 */
    lv_obj_set_size(btn_mode, scr_act_width() / 4, scr_act_height() / 6);           /* 设置按钮大小 */
    lv_obj_align(btn_mode, LV_ALIGN_CENTER, scr_act_width() / 3, 0);                /* 设置按钮位置 */
    lv_obj_set_style_bg_color(btn_mode, lv_color_hex(0xef5f60), LV_STATE_DEFAULT);  /* 设置按钮背景颜色（默认） */
    lv_obj_set_style_bg_color(btn_mode, lv_color_hex(0xff0000), LV_STATE_PRESSED);  /* 设置按钮背景颜色（按下） */

    lv_obj_t* label = lv_label_create(btn_mode);                                /* 创建加速按钮标签 */
    lv_label_set_text(label, "mode");                                            /* 设置标签文本 */
    lv_obj_set_align(label,LV_ALIGN_CENTER);                                        /* 设置标签位置 */
}
void lv_100ask_calc(void)
{
	lv_obj_t * calc = lv_100ask_calc_create(lv_scr_act());
    lv_obj_set_size(calc, scr_act_width(), scr_act_height());
    lv_obj_center(calc);
    lv_obj_clear_flag(calc, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * calc_ta_hist = lv_100ask_calc_get_ta_hist(calc);
    lv_obj_set_style_text_font(calc_ta_hist, &lv_font_montserrat_14, 0);

    lv_obj_t * calc_ta_input = lv_100ask_calc_get_ta_input(calc);
    lv_obj_set_style_text_font(calc_ta_input, &lv_font_montserrat_14, 0);

    lv_obj_t * calc_ta_btnm = lv_100ask_calc_get_btnm(calc);
    lv_obj_set_style_text_font(calc_ta_btnm, &lv_font_montserrat_14, 0);

}
/**
 * @brief  LVGL演示
 * @param  无
 * @return 无
 */
void lv_mainstart(void)
{

    //lv_btn_mode();          /* mode change按钮 */
		lv_100ask_calc();
}
