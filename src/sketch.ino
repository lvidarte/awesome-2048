/**
 * Author: Leo Vidarte <http://nerdlabs.com.ar>
 *
 * This is free software:
 * you can redistribute it and/or modify it
 * under the terms of the GPL version 3
 * as published by the Free Software Foundation.
 */

#include <Colorduino.h>
#include <SoftwareSerial.h>

#define SCREEN_WIDTH  8
#define SCREEN_HEIGHT 8
#define MASK_WIDTH    B111
#define MASK_HEIGHT   B111

/**
 * Commands
 */
#define CMD_CLEAR B0000
#define CMD_FLIP  B0001
#define CMD_SET_X B1000
#define CMD_SET_Y B1001
#define CMD_SET_R B1010
#define CMD_SET_G B1011
#define CMD_SET_B B1100
#define CMD_DEBUG B1110
#define CMD_FILL  B1111

/**
 * Params
 */
#define PARAM_OBJ_LED B00
#define PARAM_OBJ_COL B01
#define PARAM_OBJ_ROW B10
#define PARAM_OBJ_ALL B11
#define PARAM_PAGE_BG B0
#define PARAM_PAGE_FG B1
#define PARAM_OFF     B0
#define PARAM_ON      B1


/**
 * Led state
 */
typedef struct {
    byte x = 0;
    byte y = 0;
    byte r = 0;
    byte g = 0;
    byte b = 0;
} Led;

Led led;

/**
 *
 */
char log_buffer[48];

byte debug = 0;


/**
 * Functions
 */
void set_x(byte param)
{
    led.x = get_pos(param, MASK_WIDTH);
    log_set('x', led.x);
}

void set_y(byte param)
{
    led.y = get_pos(param, MASK_HEIGHT);
    log_set('y', led.y);
}

void set_r(byte param)
{
    led.r = get_color(param);
    log_set('r', led.r);
}

void set_g(byte param)
{
    led.g = get_color(param);
    log_set('g', led.g);
}

void set_b(byte param)
{
    led.b = get_color(param);
    log_set('b', led.b);
}

void flip_page()
{
    Colorduino.FlipPage();
    log_text("flip_page()");
}

int get_pos(byte value, byte mask)
{
    return value & mask;
}

int get_color(byte value)
{
    return map(value, 0, 15, 0, 255);
}

void set_color_led(byte x, byte y, byte r, byte g, byte b, byte page)
{
    switch (page)
    {
        case PARAM_PAGE_BG:
            Colorduino.SetPixel(x, y, r, g, b);
            break;

        case PARAM_PAGE_FG:
            PixelRGB *p = Colorduino.GetDrawPixel(x, y);
            p->r = r;
            p->g = g;
            p->b = b;
            break;
    }
}

void set_color_row(byte y, byte r, byte g, byte b, byte page = PARAM_PAGE_BG)
{
    for (byte x = 0; x < SCREEN_WIDTH; x++)
    {
        set_color_led(x, y, r, g, b, page);
    }
}

void set_color_col(byte x, byte r, byte g, byte b, byte page = PARAM_PAGE_BG)
{
    for (byte y = 0; y < SCREEN_HEIGHT; y++)
    {
        set_color_led(x, y, r, g, b, page);
    }
}

void set_color_all(byte r, byte g, byte b, byte page = PARAM_PAGE_BG)
{
    for (byte x = 0; x < SCREEN_WIDTH; x++)
    {
        for (byte y = 0; y < SCREEN_HEIGHT; y++)
        {
            set_color_led(x, y, r, g, b, page);
        }
    }
}

void set_color(byte obj, byte r, byte g, byte b, byte page = PARAM_PAGE_BG)
{
    switch (obj)
    {
        case PARAM_OBJ_LED:
            set_color_led(led.x, led.y, r, g, b, page);
            break;
        case PARAM_OBJ_ROW:
            set_color_row(led.y, r, g, b, page);
            break;
        case PARAM_OBJ_COL:
            set_color_col(led.x, r, g, b, page);
            break;
        case PARAM_OBJ_ALL:
            set_color_all(r, g, b, page);
            break;
    }
}

void set_leds(byte command, byte param)
{
    byte obj = get_param_obj(param);
    byte page = get_param_page(param);
    byte reset = get_param_reset(param);

    switch (command)
    {
        case CMD_FILL:
            set_color(obj, led.r, led.g, led.b, page);
            break;

        case CMD_CLEAR:
            set_color(obj, 0, 0, 0, page);
            break;
    }

    log_set_leds(command, obj, page, reset);

    if (reset)
    {
        reset_matrix();
    }
}

void reset_matrix()
{
    led.x = 0;
    led.y = 0;
    led.r = 0;
    led.g = 0;
    led.b = 0;

    log_led_status();
}

void fill(byte param)
{
    set_leds(CMD_FILL, param);
}

void clear(byte param)
{
    set_leds(CMD_CLEAR, param);
}

byte get_param_obj(byte param)
{
    return param & 3;
}

byte get_param_page(byte param)
{
    return (param >> 2) & 1;
}

byte get_param_reset(byte param)
{
    return (param >> 3) & 1;
}

byte get_param_debug(byte param)
{
    return param & 1;
}

char *get_command_name(byte command)
{
    switch (command)
    {
        case CMD_FILL : return "fill" ; break;
        case CMD_CLEAR: return "clear"; break;
    }
}

char *get_obj_name(byte obj)
{
    switch (obj)
    {
        case PARAM_OBJ_LED: return "led"; break;
        case PARAM_OBJ_ROW: return "row"; break;
        case PARAM_OBJ_COL: return "col"; break;
        case PARAM_OBJ_ALL: return "all"; break;
    }
}

char *get_page_name(byte page)
{
    switch (page)
    {
        case PARAM_PAGE_BG: return "bg"; break;
        case PARAM_PAGE_FG: return "fg"; break;
    }
}

char *get_reset_name(byte reset)
{
    switch (reset)
    {
        case PARAM_OFF: return "off"; break;
        case PARAM_ON : return "on" ; break;
    }
}

void log_set(char name, byte value)
{
    if (debug)
    {
        sprintf(log_buffer, "set_%c(%d)", name, value);
        Serial.println(log_buffer);
        log_led_status();
    }
}

void log_text(char *text)
{
    if (debug)
    {
        Serial.println(text);
    }
}

void log_led_status()
{
    if (debug)
    {
        sprintf(log_buffer, "=> pos(%d, %d) rgb(%d, %d, %d)",
                led.x, led.y, led.r, led.g, led.b);
        Serial.println(log_buffer);
    }
}

void log_set_leds(byte command, byte obj, byte page, byte reset)
{
    if (debug)
    {
        char *command_name = get_command_name(command);
        char *obj_name = get_obj_name(obj);
        char *page_name = get_page_name(page);
        char *reset_name = get_reset_name(reset);

        sprintf(log_buffer, "%s(obj=%s, page=%s, reset=%s)",
                command_name, obj_name, page_name, reset_name);
        Serial.println(log_buffer);
    }
}

void set_debug(byte param)
{
    debug = get_param_debug(param);
    Serial.println(debug ? "debug on" : "debug off");
}

void setup()
{
    Serial.begin(9600);
    Colorduino.Init();
    byte whiteBalVal[3] = {36, 63, 63};
    Colorduino.SetWhiteBal(whiteBalVal);
}

void loop()
{
    if (Serial.available())
    {
        byte data = Serial.read();
        byte command = data >> 4;
        byte param = data & 15;

        switch (command)
        {
            case CMD_SET_X: set_x(param); break;
            case CMD_SET_Y: set_y(param); break;

            case CMD_SET_R: set_r(param); break;
            case CMD_SET_G: set_g(param); break;
            case CMD_SET_B: set_b(param); break;

            case CMD_FLIP : flip_page() ; break;

            case CMD_FILL : fill(param) ; break;
            case CMD_CLEAR: clear(param); break;

            case CMD_DEBUG: set_debug(param); break;

            default: log_led_status();
        }
    }
}
