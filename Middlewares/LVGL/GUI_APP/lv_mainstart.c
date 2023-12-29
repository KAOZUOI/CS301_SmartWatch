#include "LVGL/GUI_APP/lv_mainstart.h"
#include "lvgl.h"
#include "./BSP/LED/led.h"
#include "./BSP/RTC/rtc.h"
#include "lv_calc.h"
#include "lv_2048.h"
#include <stdio.h>
#include <string.h>
#include <time.h>


#define scr_act_width()  lv_obj_get_width(lv_scr_act())
#define scr_act_height() lv_obj_get_height(lv_scr_act())
const char* username = "KOZO";
static lv_obj_t *btn_calc;
static lv_obj_t *btn_album;
static lv_obj_t *btn_chat;
static lv_obj_t *btn_game;

// 如果要作为切换页面的回调函数，必须将其声明为 static，并且其中的obj必须为全局变量
void lv_100ask_calc(void) {
    //clean

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

void lv_100ask_album() {
    //label
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Album");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);
}

void lv_100ask_chat() {
    //label
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Chat");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);
}

void lv_100ask_game() {
    //label
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Game");
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);
}

static void update_time(lv_timer_t *timer) {
    char tbuf[24];
    // 假设 rtc_get_time() 函数更新了 calendar 结构体
    rtc_get_time();
    sprintf(tbuf, "Time:%02d:%02d:%02d", calendar.hour, calendar.min, calendar.sec);
    // 更新标签的文本
    lv_label_set_text((lv_obj_t *)timer->user_data, tbuf);
}
static void change_color(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x00ff00), LV_STATE_PRESSED);
}

static void component_select_event_handler(lv_event_t *e) {
    lv_obj_t *btn = lv_event_get_target(e);    // 获取触发事件的按钮
    lv_obj_t *label = lv_obj_get_child(btn, 0); // 假设按钮的第一个子对象是标签
    const char *btn_label = lv_label_get_text(label); // 获取标签的文本

    if (strcmp(btn_label, "Album") == 0) {
        // 打开相册组件
        lv_obj_clean(lv_scr_act());
        btn_album = lv_btn_create(lv_scr_act());
        lv_obj_set_size(btn_album, scr_act_width() / 4, scr_act_height() / 6);
        lv_obj_align(btn_album, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_bg_color(btn_album, lv_color_hex(0xef5f60), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(btn_album, lv_color_hex(0xff0000), LV_STATE_PRESSED);

        lv_obj_t* label = lv_label_create(btn_album);
        lv_label_set_text(label, "album");
        lv_obj_set_align(label,LV_ALIGN_CENTER);
    } else if (strcmp(btn_label, "Calc") == 0) {
        // 打开计算器组件
        // clean btns
        
        lv_100ask_calc();
    } else if (strcmp(btn_label, "Chat") == 0) {
        // 打开聊天组件
        lv_obj_clean(lv_scr_act());
        btn_chat = lv_btn_create(lv_scr_act());
        lv_obj_set_size(btn_chat, scr_act_width() / 4, scr_act_height() / 6);
        lv_obj_align(btn_chat, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_bg_color(btn_chat, lv_color_hex(0xef5f60), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(btn_chat, lv_color_hex(0xff0000), LV_STATE_PRESSED);

        lv_obj_t* label = lv_label_create(btn_chat);
        lv_label_set_text(label, "chat");
        lv_obj_set_align(label,LV_ALIGN_CENTER);
        lv_obj_add_event_cb(btn_chat, change_color, LV_EVENT_CLICKED, NULL);
    } else if (strcmp(btn_label, "Game") == 0) {
        // 打开游戏组件
        lv_obj_clean(lv_scr_act());
        btn_game = lv_btn_create(lv_scr_act());
        lv_obj_set_size(btn_game, scr_act_width() / 4, scr_act_height() / 6);
        lv_obj_align(btn_game, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_bg_color(btn_game, lv_color_hex(0xef5f60), LV_STATE_DEFAULT);
        lv_obj_set_style_bg_color(btn_game, lv_color_hex(0xff0000), LV_STATE_PRESSED);

        lv_obj_t* label = lv_label_create(btn_game);
        lv_label_set_text(label, "game");
        lv_obj_set_align(label,LV_ALIGN_CENTER);
    }
}


void init_main_page() {
    lv_obj_clean(lv_scr_act());
    // 创建时间显示标签
    lv_obj_t *time_label_hour = lv_label_create(lv_scr_act());
    lv_obj_set_height(time_label_hour, LV_PCT(10));
    lv_obj_set_width(time_label_hour, LV_PCT(50));
    lv_obj_align(time_label_hour, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_set_style_text_font(time_label_hour, &lv_font_montserrat_14, 0);

    // 创建一个定时任务，每100ms调用一次 update_time 函数
    lv_timer_t *timer = lv_timer_create(update_time, 100, time_label_hour);

    // 创建日期显示标签
    char tbuf[24];
    rtc_get_time();
    sprintf((char *)tbuf, "Date:%04d-%02d-%02d", calendar.year, calendar.month, calendar.date);
    lv_obj_t *time_label_date = lv_label_create(lv_scr_act());
    lv_obj_set_height(time_label_date, LV_PCT(10));
    lv_obj_set_width(time_label_date, LV_PCT(50));
    lv_obj_align(time_label_date, LV_ALIGN_TOP_LEFT, 0, 20);
    lv_obj_set_style_text_font(time_label_date, &lv_font_montserrat_14, 0);
    lv_label_set_text(time_label_date, tbuf);


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
            lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, i*80, 0);
        }

        lv_obj_add_event_cb(btn, component_select_event_handler, LV_EVENT_CLICKED, NULL);

        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, btn_labels[i]);
        lv_obj_center(label);

        last_btn = btn; // 更新上一个按钮的引用
    }
}

static void game_2048_event_cb(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj_2048 = lv_event_get_target(e);
    lv_obj_t * label = lv_event_get_user_data(e);
    
    if(code == LV_EVENT_VALUE_CHANGED) {
        if (lv_100ask_2048_get_best_tile(obj_2048) >= 2048)
            lv_label_set_text(label, "#00b329 YOU WIN! #");
        else if(lv_100ask_2048_get_status(obj_2048))
            lv_label_set_text(label, "#ff0000 GAME OVER! #");
        else
            lv_label_set_text_fmt(label, "SCORE: #ff00ff %d #", lv_100ask_2048_get_score(obj_2048));
    }
}

static void new_game_btn_event_handler(lv_event_t * e)
{
    lv_obj_t * obj_2048 = lv_event_get_user_data(e);

    lv_100ask_2048_set_new_game(obj_2048);
}

void lv_2048(void)
{
    /*Create 2048 game*/
    lv_obj_t * obj_2048 = lv_100ask_2048_create(lv_scr_act());
    lv_obj_set_size(obj_2048, 150, 150);
    lv_obj_center(obj_2048);

    /*Information*/
    lv_obj_t * label = lv_label_create(lv_scr_act());
    lv_label_set_recolor(label, true); 
    lv_label_set_text_fmt(label, "SCORE: #ff00ff %d #", lv_100ask_2048_get_score(obj_2048));
    lv_obj_align_to(label, obj_2048, LV_ALIGN_OUT_TOP_RIGHT, 0, 0);

    lv_obj_add_event_cb(obj_2048, game_2048_event_cb, LV_EVENT_ALL, label);

    /*New Game*/
    lv_obj_t * btn = lv_btn_create(lv_scr_act());
    lv_obj_align_to(btn, obj_2048, LV_ALIGN_OUT_BOTTOM_RIGHT, 0, 5);
    lv_obj_add_event_cb(btn, new_game_btn_event_handler, LV_EVENT_CLICKED, obj_2048);

    label = lv_label_create(btn);
    lv_label_set_text(label, "New Game");
    lv_obj_center(label);

    /*Back*/
    lv_obj_t* btn_back = lv_btn_create(lv_scr_act());
    lv_obj_align_to(btn_back, obj_2048, LV_ALIGN_OUT_BOTTOM_LEFT, -70, 5);
    lv_obj_add_event_cb(btn_back, change_color, LV_EVENT_CLICKED, NULL);
    lv_obj_t* label_back = lv_label_create(btn_back);
    lv_label_set_text(label_back, "Back Home");
    lv_obj_center(label_back);
}

void lv_mainstart(void)
{

    //lv_btn_mode();
    //lv_100ask_calc();
    //init_main_page();
    lv_2048();
}
