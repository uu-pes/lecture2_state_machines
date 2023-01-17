#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "bsp.h"
#include "nrf_atfifo.h"
#include "app_button.h" 
#include "app_timer.h"
#include "app_error.h"
#include "nrf_drv_clock.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define BUTTON_DEBOUNCE_DELAY       50
#define TIMEOUT_DELAY               APP_TIMER_TICKS(3000)

#define RED_LED     0
#define YELLOW_LED  1 
#define GREEN_LED   2

typedef enum { go_event = 0, stop_event = 1, timeout_event = 2, no_evt = 3} event_t;

/* Function pointer primitive */ 
typedef void (*state_func_t)( void );

struct _state 
{
    uint8_t id;
    state_func_t Do;
};
typedef struct _state state_t;

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
                evt = go_event;
                nrf_atfifo_alloc_put(event_fifo, &evt, sizeof(event_t), NULL);
                break;
            
            case BUTTON_2:
                evt = stop_event;
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
    event_t evt = timeout_event;
    nrf_atfifo_alloc_put(event_fifo, &evt, sizeof(event_t), NULL);
}

void init_board(void)
{
    /* Initialize the low frequency clock used by APP_TIMER */
    uint32_t err_code;
    err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_clock_lfclk_request(NULL);

    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();
    NRF_LOG_INFO("Logging initialized.");

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
    NRF_LOG_INFO("init_board() finished.");
}

event_t get_event(void)
{
    event_t evt = no_evt;
    /* In case the fifo is empty, nothing will be written to &evt */ 
    nrf_atfifo_get_free(event_fifo, &evt, sizeof(event_t), NULL);
    return evt; 
}

void do_state_red(void)
{
    bsp_board_leds_off();
    bsp_board_led_on(RED_LED);
}

void do_state_yellow(void)
{
    bsp_board_leds_off();
    bsp_board_led_on(YELLOW_LED);
    app_timer_start(yellow_timeout_timer, TIMEOUT_DELAY, NULL);
}

void do_state_green(void)
{
    bsp_board_leds_off();
    bsp_board_led_on(GREEN_LED);
}

const state_t state_red = {
    0, 
    do_state_red
};

const state_t state_yellow = {
    1, 
    do_state_yellow
};

const state_t state_green = {
    2, 
    do_state_green
};

const state_t state_table[3][4] = {
    /*  STATE       GO              STOP            TIMEOUT         NO-EVT */
    {/* RED */      state_green,    state_red,      state_red,      state_red},
    {/* YELLOW */   state_yellow,   state_yellow,   state_red,      state_yellow},    
    {/* GREEN */    state_green,    state_yellow,   state_green,    state_green}
};

int main(void)
{
    init_board();

    state_t current_state = state_red;
    event_t evt = no_evt;

    for(;;)
    {
        while(current_state.id == state_table[current_state.id][evt].id)
        {
            current_state.Do();
            __WFE();
            evt = get_event();
        }
        current_state = state_table[current_state.id][evt];
    }
}
