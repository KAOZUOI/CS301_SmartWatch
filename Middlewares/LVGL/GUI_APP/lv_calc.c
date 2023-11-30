#include "lv_calc.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define MY_CLASS &lv_100ask_calc_class

static void lv_100ask_calc_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_100ask_calc_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj);

static void calc_btnm_changed_event_cb(lv_event_t *e);
static void lv_100ask_calc_tokenizer_init(lv_obj_t *obj, char *expr);
static lv_100ask_calc_token_t lv_100ask_calc_get_next_token(lv_obj_t *obj);
static lv_100ask_calc_token_t lv_100ask_calc_siglechar(char *curr_char);
static int lv_100ask_calc_power(int base, int exponent);
static int lv_100ask_calc_expr(lv_obj_t *obj);
static int lv_100ask_calc_term(lv_obj_t *obj);
static int lv_100ask_calc_primary(lv_obj_t *obj);
static int lv_100ask_calc_factor(lv_obj_t *obj);
static int lv_100ask_calc_tokenizer_num(char *curr_char);
static void lv_100ask_calc_accept(lv_obj_t *obj, lv_100ask_calc_token_t token);
static void lv_100ask_calc_error(lv_100ask_calc_error_t error_code ,lv_100ask_calc_error_t err);
static void lv_100ask_calc_tokenizer_next(lv_obj_t *obj);
static bool lv_100ask_calc_tokenizer_finished(lv_100ask_calc_token_t current_token, char *curr_char);
static lv_100ask_calc_token_t lv_100ask_calc_siglechar_binary(char *curr_char);
static int lv_100ask_calc_term_binary(lv_obj_t *obj);
static int lv_100ask_calc_expr_binary(lv_obj_t *obj);

float fastSqrt(float x) {
    float xhalf = 0.5f*x;
    int i = *(int*)&x; // get bits for floating VALUE 
    i = 0x5f375a86- (i>>1); // gives initial guess y0
    x = *(float*)&i; // convert bits BACK to float
    x = x*(1.5f-xhalf*x*x); // Newton step, repeating increases accuracy
    x = x*(1.5f-xhalf*x*x); // Newton step, repeating increases accuracy
    x = x*(1.5f-xhalf*x*x); // Newton step, repeating increases accuracy

    return 1/x;
}

float abso(float a) {
    return a > 0 ? a : -a;
}

void parse_equation(const char* input, equation_coeffs_t* coeffs) {
    coeffs->a = 0;
    coeffs->b = 0;
    coeffs->c = 0;
    coeffs->degree = 0;

    float temp_coeff = 0;
    int temp_degree = 0;
    int sign = 1; // process the right part of =
    char* p = (char*)input;

    while (*p) {
        if (*p == '=') {
            sign = -1;
            p++;
            continue;
        }

        if (sscanf(p, "%f", &temp_coeff) == 1) {
            temp_coeff *= sign;
            while (*p != ' ' && *p != '\0' && *p != 'x' && *p != '=') p++;

            if (*p == 'x') {
                p++;
                if (*p == '^') {
                    p++;
                    if (sscanf(p, "%d", &temp_degree) == 1) {
                        if (temp_degree == 2) {
                            coeffs->a = temp_coeff;
                            coeffs->degree = 2;
                        } else if (temp_degree == 1) {
                            coeffs->b = temp_coeff;
                            if (coeffs->degree < 2) coeffs->degree = 1;
                        } else {
                            coeffs->degree = 3;
                        }
                        while (isdigit(*p) || *p == '.') p++;
                    }
                } else {
                    coeffs->b = temp_coeff;
                    if (coeffs->degree < 2) coeffs->degree = 1;
                }
            } else {
                coeffs->c += temp_coeff;
            }
        } else if (*p == 'x') {
            p++;
            temp_coeff = sign;
            if (*p == '^') {
                p++;
                if (sscanf(p, "%d", &temp_degree) == 1 && temp_degree == 2) {
                    coeffs->a = temp_coeff;
                    coeffs->degree = 2;
                    while (isdigit(*p) || *p == '.') p++;
                }
            } else {
                coeffs->b = temp_coeff;
                if (coeffs->degree < 2) coeffs->degree = 1;
            }
        } else {
            p++;
        }
    }
}


void solve_linear_equation(const equation_coeffs_t* coeffs, char* output) {
    if (abso(coeffs->b - 0) < 0.00001) {
        sprintf(output, "error in linear equation");
        return;
    }
    float x = -coeffs->c / coeffs->b;
    sprintf(output, "x = %f", x);
}

void solve_quadratic_equation(const equation_coeffs_t* coeffs, char* output) {
    float a = coeffs->a, b = coeffs->b, c = coeffs->c;
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) {
        sprintf(output, "error in quadratic equation");
    } else if (discriminant == 0) {
        float x = -b / (2 * a);
        sprintf(output, "x = %f", x);
    } else {
        float x1 = (-b + fastSqrt(discriminant)) / (2 * a);
        float x2 = (-b - fastSqrt(discriminant)) / (2 * a);
        sprintf(output, "x = %f | x = %f", x1, x2);
    }
}

void int_to_binary_string(int value, char *buffer, int buffer_size) {
    buffer[buffer_size - 1] = '\0'; // make sure the string is null-terminated
    int index = buffer_size - 2;
    while (index >= 0) {
        if (value == 0 && index < buffer_size - 2) {
            break; // jump out of the loop if the value is zero and we're not on the first iteration
        }
        buffer[index] = (value & 1) ? '1' : '0';
        value >>= 1;
        index--;
    }

    // move the string to the beginning of the buffer
    if (index >= 0) {
        memmove(buffer, &buffer[index + 1], buffer_size - index - 1);
    }
}


static int binary_add(int a, int b) {
    int carry;
    while (b != 0) {
        carry = (a & b) << 1;
        a = a ^ b;
        b = carry;
    }
    return a;
}
static int binary_subtract(int a, int b) {
    int b_complement = binary_add(~b, 1);
    return binary_add(a, b_complement);
}
static int binary_multiply(int a, int b) {
    int result = 0;
    while (b != 0) {
        if (b & 1) {
            result = binary_add(result, a);
        }
        a <<= 1;
        b >>= 1;
    }
    return result;
}

typedef enum {
    MODE_STANDARD,
    MODE_BINARY,
    MODE_EQUATION
} calc_mode_t;

static calc_mode_t current_mode = MODE_STANDARD;





const lv_obj_class_t lv_100ask_calc_class = {
    .constructor_cb = lv_100ask_calc_constructor,
    .destructor_cb  = lv_100ask_calc_destructor,
    .width_def      = LV_DPI_DEF * 2,
    .height_def     = LV_DPI_DEF * 3,
    .instance_size  = sizeof(lv_100ask_calc_t),
    .base_class     = &lv_obj_class
};
// Key map
static const char * equation_btnm_map[] = { "D", "C", "S", "+", "\n",
                                            "0", "5", "6", "-", "\n",
                                            "1", "2", "3", "*", "\n",
                                            "^", "x", "=", "/", "" };

static const char * binary_btnm_map[] = { "0", "1", "+", "-", "\n",
                                          "=", "*", "D", "C", "" };

static const char * btnm_map[] = {  "(", ")", "C", "!", "\n",
									"X", "B", "^", "/",  "\n",
									"<", "5", ">", "*",  "\n",
									"1", "2", "3", "-",  "\n",
									"0", ".", "=", "+",  ""};

// error list
static const lv_100ask_calc_error_table_t error_table[] = {
    {.error_code = no_error,            .message = "no error"},
    {.error_code = syntax_error,        .message = "syntax error!"}
};

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * lv_100ask_calc_create(lv_obj_t * parent)
{
    LV_LOG_INFO("begin");
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

/*=====================
 * Getter functions
 *====================*/

lv_obj_t * lv_100ask_calc_get_btnm(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_100ask_calc_t * calc = (lv_100ask_calc_t *)obj;

    return calc->btnm;
}
lv_obj_t * lv_100ask_calc_get_ta_hist(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_100ask_calc_t * calc = (lv_100ask_calc_t *)obj;

    return calc->ta_hist;
}
lv_obj_t * lv_100ask_calc_get_ta_input(lv_obj_t * obj)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_100ask_calc_t * calc = (lv_100ask_calc_t *)obj;

    return calc->ta_input;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lv_100ask_calc_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    LV_TRACE_OBJ_CREATE("begin");

    lv_100ask_calc_t * calc = (lv_100ask_calc_t *)obj;

    calc->curr_char = NULL;
    calc->next_char = NULL;
    calc->current_token = TOKENIZER_ERROR;
    calc->error_code = no_error;
    calc->count = 0;

    /*set layout*/
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);

    /*Display calculation history and results*/
    calc->ta_hist = lv_textarea_create(obj);
    lv_obj_set_style_bg_color(calc->ta_hist, LV_100ASK_COLOR_BLACK, 0);
    lv_obj_set_style_text_color(calc->ta_hist, LV_100ASK_COLOR_GREEN, 0);
    lv_obj_set_style_radius(calc->ta_hist, 0, 0);
    
    lv_obj_set_size(calc->ta_hist, LV_PCT(100), LV_PCT(20));
    lv_textarea_set_cursor_click_pos(calc->ta_hist, false);
    lv_textarea_set_max_length(calc->ta_hist, LV_100ASK_CALC_HISTORY_MAX_LINE);
    lv_textarea_set_align(calc->ta_hist, LV_TEXT_ALIGN_RIGHT);
    lv_textarea_set_text(calc->ta_hist, "");
    lv_textarea_set_placeholder_text(calc->ta_hist, "CALC HISTORY\t\t");
    lv_obj_set_style_border_width(calc->ta_hist, 0, 0);

    /*Input textarea*/
    calc->ta_input = lv_textarea_create(obj);
    lv_obj_set_style_bg_color(calc->ta_input, LV_100ASK_COLOR_BLACK, 0);
    lv_obj_set_style_text_color(calc->ta_input, LV_100ASK_COLOR_GREEN, 0);
    lv_obj_set_style_radius(calc->ta_input, 0, 0);
    lv_obj_set_style_border_width(calc->ta_input, 0, 0);
    
    lv_obj_set_size(calc->ta_input, LV_PCT(100), LV_PCT(5));
    lv_textarea_set_one_line(calc->ta_input, true);
    lv_textarea_set_cursor_click_pos(calc->ta_input, false);
    lv_textarea_set_max_length(calc->ta_input, LV_100ASK_CALC_HISTORY_MAX_LINE);
    lv_textarea_set_align(calc->ta_input, LV_TEXT_ALIGN_RIGHT);
    lv_textarea_set_text(calc->ta_input, "");

    /*Calculator input panel*/
    calc->btnm = lv_btnmatrix_create(obj);
    lv_obj_set_style_radius(calc->btnm, 0, 0);
    lv_obj_set_style_border_width(calc->btnm, 0, 0);

    lv_obj_set_size(calc->btnm, LV_PCT(100), LV_PCT(73));
    lv_btnmatrix_set_map(calc->btnm, btnm_map);
    lv_obj_add_event_cb(calc->btnm, calc_btnm_changed_event_cb, LV_EVENT_VALUE_CHANGED, obj);

    LV_TRACE_OBJ_CREATE("finished");
}

static void lv_100ask_calc_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);

}

void update_textarea_view(lv_obj_t * textarea) {
    const char * txt = lv_textarea_get_text(textarea);
    int cursor_pos = lv_textarea_get_cursor_pos(textarea);

    // 获取文本的度量
    lv_point_t size;
    lv_txt_get_size(&size, txt, lv_obj_get_style_text_font(textarea, LV_PART_MAIN), 0, 0, cursor_pos, LV_TEXT_FLAG_NONE);

    // 如果光标位置超出当前视图，滚动到该位置
    lv_coord_t cur_scroll_x = lv_obj_get_scroll_x(textarea);
    lv_coord_t ta_width = lv_obj_get_width(textarea);

    if(size.x > cur_scroll_x + ta_width) {
        lv_obj_scroll_to_x(textarea, size.x - ta_width, LV_ANIM_ON);
    } else if(size.x < cur_scroll_x) {
        lv_obj_scroll_to_x(textarea, size.x, LV_ANIM_ON);
    }
}



static void calc_btnm_changed_event_cb(lv_event_t *e) {
    lv_obj_t * obj = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * user_data = lv_event_get_user_data(e);
    
    uint32_t id = lv_btnmatrix_get_selected_btn(obj);
    const char * txt = lv_btnmatrix_get_btn_text(obj, id);

    lv_100ask_calc_t * calc = (lv_100ask_calc_t *)user_data;

    if(code == LV_EVENT_VALUE_CHANGED) {
        // Perform operations
        if (strcmp(txt, "=") == 0) {
            if (current_mode != MODE_EQUATION) {
                
                char tmp_buff[32];
                int calc_results;

                // Lexical analyzer
                lv_100ask_calc_tokenizer_init(user_data, calc->calc_exp);

                // Calculates the value of the first level priority expression
                if (current_mode == MODE_BINARY) {
                    calc_results = lv_100ask_calc_expr_binary(user_data);
                }else{
                    calc_results = lv_100ask_calc_expr(user_data);  
                }


                if (calc->error_code != no_error) {
                    // Find the error code and display the corresponding message
                    for (int i = 0; i < sizeof(error_table); i++)
                    {
                        if (error_table[i].error_code == calc->error_code)
                        {
                            lv_textarea_add_text(calc->ta_hist, "\n");
                            lv_textarea_add_text(calc->ta_hist, error_table[i].message);
                            lv_textarea_add_text(calc->ta_hist, "\n");
                        }
                    }
                    calc->error_code = no_error;
                }
                else {
                    if (current_mode == MODE_BINARY) {
                        char binary_result[33]; // 32 bits + 1 null terminator
                        int_to_binary_string(calc_results, binary_result, sizeof(binary_result));
                        lv_snprintf(tmp_buff, sizeof(tmp_buff), "%s=%s\n", lv_textarea_get_text(calc->ta_input), binary_result);
                    } else {
                        lv_snprintf(tmp_buff, sizeof(tmp_buff), "%s=%d\n", lv_textarea_get_text(calc->ta_input), calc_results);
                    }
                    lv_textarea_add_text(calc->ta_hist, tmp_buff);
                    lv_textarea_set_text(calc->ta_input, tmp_buff);
                    // Empty expression
                    lv_memset_00(calc->calc_exp, sizeof(calc->calc_exp));
                    calc->count = 0;
                }
            }else {
                lv_textarea_add_text(calc->ta_input, txt);
                strcat(&calc->calc_exp[0], txt);
                calc->count++;
            }
        }
        // clear
        else if (strcmp(txt, "C") == 0) {
            lv_textarea_set_text(calc->ta_input, "");
            // Empty expression
            lv_memset_00(calc->calc_exp, sizeof(calc->calc_exp));
            calc->count = 0;
        }
        // del char
        else if (strcmp(txt, "<-") == 0) {
            lv_textarea_del_char(calc->ta_input);
            calc->calc_exp[calc->count-1] = '\0';
            calc->count--;
        }
        // change to Binary mode
        else if (strcmp(txt, "B") == 0) {
            HAL_Delay(20);
            lv_btnmatrix_set_map(calc->btnm, binary_btnm_map);
            current_mode = MODE_BINARY;
        }
        // change to Standard mode
        else if (strcmp(txt, "D") == 0) {
            lv_btnmatrix_set_map(calc->btnm, btnm_map);
            current_mode = MODE_STANDARD;
        }
        // change to Equation mode
        else if (strcmp(txt, "X") == 0) {
            
            lv_btnmatrix_set_map(calc->btnm, equation_btnm_map);
            current_mode = MODE_EQUATION;
        }
        // solve equation
        else if (strcmp(txt, "S") == 0) {
            char equation_output[128];
            equation_coeffs_t coeffs;
            parse_equation(calc->calc_exp, &coeffs); // calc->calc_exp is the equation string
            if (coeffs.degree == 1) {
                solve_linear_equation(&coeffs, equation_output);
            } else if (coeffs.degree == 2) {
                solve_quadratic_equation(&coeffs, equation_output);
            } else {
                sprintf(equation_output, "error in equation: degree = %d", coeffs.degree);
                // strcpy(equation_output, "error");
            }
            lv_textarea_set_text(calc->ta_input, equation_output);
        }
        // cursor left
        else if (strcmp(txt, "<") == 0) {
            lv_textarea_set_cursor_pos(calc->ta_input, lv_textarea_get_cursor_pos(calc->ta_input) - 1);
            update_textarea_view(calc->ta_input);
        }
        // cursor right
        else if (strcmp(txt, ">") == 0) {
            lv_textarea_set_cursor_pos(calc->ta_input, lv_textarea_get_cursor_pos(calc->ta_input) + 1);
            update_textarea_view(calc->ta_input);
        }
        // Add char
        else {
            if((calc->count == 0) && (strcmp(lv_textarea_get_text(calc->ta_input), "") == 0))
                lv_textarea_set_text(calc->ta_input, "");

            lv_textarea_add_text(calc->ta_input, txt);
            strcat(&calc->calc_exp[0], txt);
            calc->count++;
        }
    }
}

/**
 * Lexical analyzer initialization.
 * @param obj       pointer to a calc object
 * @param expr      pointer to expression
 */
static void lv_100ask_calc_tokenizer_init(lv_obj_t *obj, char *expr)
{
    lv_100ask_calc_t * calc = (lv_100ask_calc_t *)obj;

    calc->curr_char = calc->next_char = expr;
    calc->current_token = lv_100ask_calc_get_next_token(obj);

    return;
}

/**
 * Get a token.
 * @param obj       pointer to a calc object
 * @return          Token type
 */
static lv_100ask_calc_token_t lv_100ask_calc_get_next_token(lv_obj_t *obj)
{
    int i;
    lv_100ask_calc_t * calc = (lv_100ask_calc_t *)obj;

    // End of expression
    if (calc->curr_char == '\0')
        return TOKENIZER_ENDOFINPUT;
    if (current_mode == MODE_BINARY) {
        if (*calc->curr_char == '0' || *calc->curr_char == '1') {
            for (i = 0; i <= LV_100ASK_CALC_MAX_NUM_LEN && (calc->curr_char[i] == '0' || calc->curr_char[i] == '1'); i++);
            calc->next_char = calc->curr_char + i;
            return TOKENIZER_NUMBER;
        }
        else if (lv_100ask_calc_siglechar_binary(calc->curr_char)) {
            calc->next_char++;
            return lv_100ask_calc_siglechar_binary(calc->curr_char);
        }
    }else {
        if (isdigit(*calc->curr_char)) {
            // The length of the allowed number cannot be exceeded
            for (i = 0; i <= LV_100ASK_CALC_MAX_NUM_LEN; i++)
            {
                if (!isdigit(*(calc->curr_char + i)))
                {
                    calc->next_char = calc->curr_char + i;
                    return TOKENIZER_NUMBER;
                }
            }
        }
        // Delimiter
        else if (lv_100ask_calc_siglechar(calc->curr_char)) {
            calc->next_char++;
            return lv_100ask_calc_siglechar(calc->curr_char);
        }
    }

    return TOKENIZER_ERROR;
}

static lv_100ask_calc_token_t lv_100ask_calc_siglechar_binary(char *curr_char) {
    switch (*curr_char) {
        case '+':
            return TOKENIZER_PLUS;
        case '-':
            return TOKENIZER_MINUS;
        case '*':
            return TOKENIZER_ASTR;
        default:
            break;
    }
    return TOKENIZER_ERROR;
}


/**
 * Get single character token type.
 * @param curr_char       Pointer to character
 * @return                Token type
 */
static lv_100ask_calc_token_t lv_100ask_calc_siglechar(char *curr_char)
{
    switch (*curr_char)
    {
        case '+':
            return TOKENIZER_PLUS;
        case '-':
            return TOKENIZER_MINUS;
        case '*':
            return TOKENIZER_ASTR;
        case '/':
            return TOKENIZER_SLASH;
        case '(':
            return TOKENIZER_LPAREN;
        case ')':
            return TOKENIZER_RPAREN;
        case '^':
            return TOKENIZER_POWER;
        default:
            break;
    }

    return TOKENIZER_ERROR;
}



static int lv_100ask_calc_power(int base, int exponent) {
    if (exponent == 0) {
        return 1;
    } else if (exponent < 0) {
        return 0;
    }
    int result = 1;
    for (int i = 0; i < exponent; ++i) {
        result *= base;
    }
    return result;
}

static int lv_100ask_calc_expr_binary(lv_obj_t *obj) {
    lv_100ask_calc_t * calc = (lv_100ask_calc_t *)obj;
    int t1, t2 = 0;
    lv_100ask_calc_token_t op;
    t1 = lv_100ask_calc_term_binary(obj);
    op = calc->current_token;
    while (op == TOKENIZER_PLUS || op == TOKENIZER_MINUS) {
        lv_100ask_calc_tokenizer_next(obj);
        t2 = lv_100ask_calc_term_binary(obj);
        if (op == TOKENIZER_PLUS) {
            t1 = binary_add(t1, t2);
        } else if (op == TOKENIZER_MINUS) {
            t1 = binary_subtract(t1, t2);
        }
        op = calc->current_token;
    }
    return t1;
}

static int lv_100ask_calc_term_binary(lv_obj_t *obj) {
    lv_100ask_calc_t * calc = (lv_100ask_calc_t *)obj;
    int f1 = lv_100ask_calc_factor(obj);
    lv_100ask_calc_token_t op = calc->current_token;

    while (op == TOKENIZER_ASTR) {
        lv_100ask_calc_tokenizer_next(obj);
        int f2 = lv_100ask_calc_factor(obj);
        f1 = binary_multiply(f1, f2);
        op = calc->current_token;
    }

    return f1;
}
/**
 * Calculates the value of the first level priority expression.
 * @param curr_char       pointer to a calc object
 * @return                Calculation results
 */
static int lv_100ask_calc_expr(lv_obj_t *obj)
{
    lv_100ask_calc_t * calc = (lv_100ask_calc_t *)obj;
    int t1, t2 = 0;
    lv_100ask_calc_token_t op;

    // First operand
    t1 = lv_100ask_calc_term(obj);
    // Get operator
    op = calc->current_token;

    // Operators can only be plus or minus (same priority)
    while (op == TOKENIZER_PLUS || op == TOKENIZER_MINUS)
    {
        // Next token
        lv_100ask_calc_tokenizer_next(obj);

        // Second operand
        t2 = lv_100ask_calc_term(obj);
        switch ((int)op)
        {
            case TOKENIZER_PLUS:
                t1 = t1 + t2;
                break;
            case TOKENIZER_MINUS:
                t1 = t1 - t2;
                break;
        }
        op = calc->current_token;
    }

    return t1;
}


/**
 * Calculates the value of the second level priority (multiplication and division) expression.
 * @param curr_char       pointer to a calc object
 * @return                Calculation results
 */
static int lv_100ask_calc_term(lv_obj_t *obj) {
    lv_100ask_calc_t * calc = (lv_100ask_calc_t *)obj;
    int f1, f2;
    lv_100ask_calc_token_t op;

    // Get left operand (factor)
    f1 = lv_100ask_calc_primary(obj);

    // Get operator
    op = calc->current_token;

    // Operators can only be multiply or divide (same priority)
    while (op == TOKENIZER_ASTR || op == TOKENIZER_SLASH)
    {
        // Next token
        lv_100ask_calc_tokenizer_next(obj);
        
        // Get right operand (factor)
        f2 = lv_100ask_calc_primary(obj);
        switch ((int)op)
        {
            case TOKENIZER_ASTR:
                f1 = f1 * f2;
                break;
            case TOKENIZER_SLASH:
                f1 = f1 / f2;
                break;
        }
        // The value calculated above will be used as the left operand
        op = calc->current_token;
    }

    return f1;
}

static int lv_100ask_calc_primary(lv_obj_t *obj) {
    int base, exponent;
    lv_100ask_calc_t *calc = (lv_100ask_calc_t *)obj;

    base = lv_100ask_calc_factor(obj);
    while (calc->current_token == TOKENIZER_POWER) {
        lv_100ask_calc_accept(obj, TOKENIZER_POWER);
        exponent = lv_100ask_calc_primary(obj);  // 处理右结合性
        base = lv_100ask_calc_power(base, exponent);
    }

    return base;
}
/**
 * Get the value of the current factor. 
 * If the current factor (similar to m in the above formula) is an expression, 
 * perform recursive evaluation
 * @param curr_char       pointer to a calc object
 * @return                Value of factor
 */
static int lv_100ask_calc_factor(lv_obj_t *obj)
{
    int r = 0;
    lv_100ask_calc_t * calc = (lv_100ask_calc_t *)obj;

    // Type of current token
    switch (calc->current_token)
    {
        // Number (Terminator)
        case TOKENIZER_NUMBER:
            // Convert it from ASCII to numeric value
            r = lv_100ask_calc_tokenizer_num(calc->curr_char);
            // Match the current token according to syntax rules
            lv_100ask_calc_accept(obj, TOKENIZER_NUMBER);
            break;
        // left bracket
        case TOKENIZER_LPAREN:
            lv_100ask_calc_accept(obj, TOKENIZER_LPAREN);
            // Treat the value in parentheses as a new expression and calculate recursively (recursion starts with function expr())
            if (current_mode == MODE_BINARY) {
                r = lv_100ask_calc_expr_binary(obj);
            } else {
                r = lv_100ask_calc_expr(obj);
            }
            // When the expression in the bracket is processed, the next token must be the right bracket
            lv_100ask_calc_accept(obj, TOKENIZER_RPAREN);
            break;
            // Tokens other than the left parenthesis and numbers have been disposed of by the upper level
            // If there is a token, it must be an expression syntax error
        default:
            lv_100ask_calc_error(calc->error_code, syntax_error);
    }

    // Returns the value of the factor
    return r;
}


/**
 * Convert it from ASCII to numeric value.
 * @param curr_char       Pointer to character
 * @return                Result of conversion to integer
 */
static int lv_100ask_calc_tokenizer_num(char *curr_char)
{
    if (current_mode == MODE_BINARY) {
        return strtol(curr_char, NULL, 2); // to binary
    } else {
        return atoi(curr_char); // to decimal
    }
}


/**
 * Match the current token according to syntax rules.
 * @param curr_char       pointer to a calc object
 * @param token           token
 */
static void lv_100ask_calc_accept(lv_obj_t *obj, lv_100ask_calc_token_t token) {
    lv_100ask_calc_t * calc = (lv_100ask_calc_t *)obj;

    if (token != calc->current_token)
        lv_100ask_calc_error(calc->error_code, syntax_error);

    lv_100ask_calc_tokenizer_next(obj);
}



/**
 * Set error code.
 * @param error_code       Current error_code
 * @param err              Error code to be set
 */
static void lv_100ask_calc_error(lv_100ask_calc_error_t error_code, lv_100ask_calc_error_t err)
{
    error_code = err;

    LV_UNUSED(error_code);
}


/**
 * Parse next token.
 * @param error_code       pointer to a calc object
 */
static void lv_100ask_calc_tokenizer_next(lv_obj_t *obj)
{
    lv_100ask_calc_t * calc = (lv_100ask_calc_t *)obj;

    if (lv_100ask_calc_tokenizer_finished(calc->current_token, calc->curr_char))
        return;

    calc->curr_char = calc->next_char;
    calc->current_token = lv_100ask_calc_get_next_token(obj);

    return;
}


/**
 * Judge whether the token has reached the end.
 * @param current_token   curren token
 * @param curr_char       pointer to a calc object
 * @return                true:  There are no tokens to parse
 *                        false: There are tokens that need to be resolved
 */
static bool lv_100ask_calc_tokenizer_finished(lv_100ask_calc_token_t current_token, char *curr_char)
{
    return *curr_char == '\0' || current_token == TOKENIZER_ENDOFINPUT;
}

