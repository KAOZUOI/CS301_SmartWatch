#include "LVGL/GUI_APP/lv_mainstart.h"
#include "lvgl.h"
#include "./BSP/LED/led.h"
#include "./BSP/RTC/rtc.h"
#include "lv_calc.h"
#include <stdio.h>
#include <string.h>
#include <time.h>


#define scr_act_width()  lv_obj_get_width(lv_scr_act())
#define scr_act_height() lv_obj_get_height(lv_scr_act())
const char* username = "KOZO";
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

static void update_time(lv_timer_t *timer) {
    char tbuf[20];
    // 假设 rtc_get_time() 函数更新了 calendar 结构体
    rtc_get_time();
    sprintf(tbuf, "Time:%02d:%02d:%02d", calendar.hour, calendar.min, calendar.sec);

    // 更新标签的文本
    lv_label_set_text((lv_obj_t *)timer->user_data, tbuf);
}

static void component_select_event_handler(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e);    // 获取触发事件的按钮
    lv_obj_t *label = lv_obj_get_child(btn, 0); // 假设按钮的第一个子对象是标签
    const char *btn_label = lv_label_get_text(label); // 获取标签的文本

    if (strcmp(btn_label, "Album") == 0) {
        // 打开相册组件
    } else if (strcmp(btn_label, "Calculator") == 0) {
        // 打开计算器组件
        lv_100ask_calc();
    }
    // 添加其他组件的条件分支
}


static void init_main_page() {
    // 创建时间显示标签
    lv_obj_t *time_label = lv_label_create(lv_scr_act());
    lv_obj_set_height(time_label, LV_PCT(10));
    lv_obj_set_width(time_label, LV_PCT(25));
    lv_obj_align(time_label, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_14, 0);

    // 创建一个定时任务，每100ms调用一次 update_time 函数
    lv_timer_t *timer = lv_timer_create(update_time, 100, time_label);

    // 添加用户名显示
    lv_obj_t *username_label = lv_label_create(lv_scr_act());
    lv_obj_set_height(username_label, LV_PCT(10)); // 设置标签高度为屏幕高度的十分之一
    lv_obj_set_width(username_label, LV_PCT(25));
    lv_label_set_text(username_label, username);
    lv_obj_set_style_text_font(username_label, &lv_font_montserrat_14, 0);
    lv_obj_align(username_label, LV_ALIGN_TOP_RIGHT, 0, 0);

    const char *btn_labels[] = {"Album", "Calc", "Chat", "Game"};
    lv_obj_t *last_btn = NULL;

    for (int i = 0; i < 4; i++) {
        lv_obj_t *btn = lv_btn_create(lv_scr_act());
        lv_obj_set_height(btn, LV_SIZE_CONTENT); // 设置按钮高度
        lv_obj_set_width(btn, LV_PCT(25));       // 设置按钮宽度为屏幕宽度的四分之一

        if (last_btn == NULL) {
            // 第一个按钮，靠屏幕左边
            lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 0, 0);
        } else {
            // 其他按钮，靠上一个按钮的右侧
            lv_obj_align_to(btn, last_btn, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
        }

        lv_obj_add_event_cb(btn, component_select_event_handler, LV_EVENT_CLICKED, NULL);

        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, btn_labels[i]);
        lv_obj_center(label);

        last_btn = btn; // 更新上一个按钮的引用
    }
}




void lv_mainstart(void)
{

    //lv_btn_mode();
    //lv_100ask_calc();
    init_main_page();
}
