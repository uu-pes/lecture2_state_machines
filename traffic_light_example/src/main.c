#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "bsp.h"            // includes app_button.h
#include "app_timer.h"
#include "nordic_common.h"

#define RED_STATE       0 
#define YELLOW_STATE    1 
#define GREEN_STATE     2

#define GO_CMD          0 
#define STOP_CMD        1
#define NO_CMD          2

#define BUTTON_DEBOUNCE_TIME 50 

static void timers_init(void)
{
    NRF_CLOCK->LFCLKSRC            = (CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos);
    NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_LFCLKSTART    = 1;
    app_timer_init();
}

static void leds_init(void)
{
    bsp_init(BSP_INIT_LEDS, NULL);
}

static void button_event_handler(uint8_t pin_no, uint8_t button_action)
{
    if(button_action == APP_BUTTON_PUSH)
    {
        bsp_board_led_invert(1);
    }
}

static const app_button_cfg_t app_buttons[] = 
{
    {BUTTON_1, false, BUTTON_PULL, button_event_handler},
    {BUTTON_2, false, BUTTON_PULL, button_event_handler}
}; 

static void buttons_init(void)
{
    app_button_init(app_buttons, ARRAY_SIZE(app_buttons), APP_TIMER_TICKS(BUTTON_DEBOUNCE_TIME));
    app_button_enable();
}


int main(void)
{


    timers_init();
    leds_init();
    buttons_init();

    static uint8_t state = RED_STATE;
    static uint8_t cmd = NO_CMD;

    while (true)
    {
        /* look for event */ 
        /*
        if(bsp_board_button_state_get(GO_CMD))
        {
            cmd = GO_CMD;
        } 
        else if(bsp_board_button_state_get(STOP_CMD))
        {
            cmd = STOP_CMD;
        } 
        else 
        {
            cmd = NO_CMD;
        }
        */

        switch(state)
        {
            case RED_STATE:
                bsp_board_led_on(RED_STATE);

                if(cmd == GO_CMD)
                {
                    bsp_board_led_off(RED_STATE);
                    state = GREEN_STATE;
                }
            break;

            case YELLOW_STATE:
                bsp_board_led_on(YELLOW_STATE);

                /* start timer */
                nrf_delay_ms(3000);

                bsp_board_led_off(YELLOW_STATE);
                state = RED_STATE;
            break;

            case GREEN_STATE:
                bsp_board_led_on(GREEN_STATE);

                if(cmd == STOP_CMD)
                {
                    bsp_board_led_off(GREEN_STATE);
                    state = YELLOW_STATE;
                }
            break;
        }
    }
}

