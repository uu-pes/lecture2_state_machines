#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "nrf_drv_clock.h"
#include "bsp.h"
#include "nrf_atfifo.h"
#include "app_button.h" 
#include "app_timer.h"

#define BUTTON_DEBOUNCE_DELAY   50
#define YELLOW_TIMEOUT          APP_TIMER_TICKS(3000)

enum app_event
{
    go = 0, 
    stop = 1, 
    timeout = 2,
    no_evt = 3
};
typedef enum app_event event_t;

enum app_light
{
    red_light = 0,
    yellow_light = 1, 
    green_light = 2
};
typedef enum app_light light_t; 

NRF_ATFIFO_DEF(event_fifo, event_t, 10);

APP_TIMER_DEF(yellow_timeout_timer);

static void button_handler(uint8_t pin_num, uint8_t btn_action)
{
    event_t evt;

    if(btn_action == APP_BUTTON_PUSH)
    {
        switch(pin_num)
        {
            case BUTTON_1:
                evt = go;
                nrf_atfifo_alloc_put(event_fifo, &evt, sizeof(event_t), NULL);
            break;

            case BUTTON_2:
                evt = stop;
                nrf_atfifo_alloc_put(event_fifo, &evt, sizeof(event_t), NULL);
            break;
        }
    }
}

static const app_button_cfg_t p_buttons[] = {
        {BUTTON_1, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_PULLUP, button_handler},
        {BUTTON_2, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_PULLUP, button_handler}
};

static void timeout_handler(void * p_context)
{
    event_t evt = timeout;
    nrf_atfifo_alloc_put(event_fifo, &evt, sizeof(event_t), NULL);
}

void init_board(void)
{
    /* Initialize the low frequency clock used by APP_TIMER */
    uint32_t err_code;
    err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_clock_lfclk_request(NULL);

    /* Initialize the event fifo */
    NRF_ATFIFO_INIT(event_fifo);

    /* Initialize the timer module */ 
    app_timer_init();
    app_timer_create(&yellow_timeout_timer, APP_TIMER_MODE_SINGLE_SHOT, timeout_handler);

    /* Initialize the LEDs */
    bsp_board_init(BSP_INIT_LEDS);

    /* Setup button interrupt handler */
    app_button_init(p_buttons, ARRAY_SIZE(p_buttons), BUTTON_DEBOUNCE_DELAY);
    app_button_enable();
}

event_t get_event(void)
{
    event_t evt = no_evt;
    /* In case the fifo is empty, nothing will be written to &evt */ 
    nrf_atfifo_get_free(event_fifo, &evt, sizeof(event_t), NULL);
    return evt; 
}

int main(void)
{
    init_board();

    event_t evt = no_evt;
    light_t curr_light = red_light; 

    while (true)
    {
        evt = get_event();

        switch(curr_light)
        {
            case red_light:
                bsp_board_led_on(red_light);

                if(evt == go)
                {
                    bsp_board_led_off(red_light);
                    curr_light = green_light;
                }
            break;

            case yellow_light:
                bsp_board_led_on(yellow_light);

                /* start timer */
                app_timer_start(yellow_timeout_timer, YELLOW_TIMEOUT, NULL);

                if(evt == timeout)
                {
                    bsp_board_led_off(yellow_light);
                    curr_light = red_light;
                }
            break;

            case green_light:
                bsp_board_led_on(green_light);

                if(evt == stop)
                {
                    bsp_board_led_off(green_light);
                    curr_light = yellow_light;
                }
            break;
        }
        __WFE();
    }
}
