#include <stdbool.h>
#include <stdint.h>
#include "nrf_delay.h"
#include "boards.h"

#define RED_STATE       0 
#define YELLOW_STATE    1 
#define GREEN_STATE     2

#define GO_CMD          0 
#define STOP_CMD        1
#define NO_CMD          2

static uint8_t state = RED_STATE;
static uint8_t cmd;

static void app_init(void)
{
    bsp_board_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS);
}

static uint8_t get_command(void)
{
    if(bsp_board_button_state_get(GO_CMD))
        {
            return GO_CMD;
        } 
        else if(bsp_board_button_state_get(STOP_CMD))
        {
            return STOP_CMD;
        } 
        else 
        {
            return NO_CMD;
        }
}

static uint8_t get_next_state(uint8_t current_state)
{
    switch(current_state)
    {
        case GREEN_STATE:
            return YELLOW_STATE;
            break;
        case YELLOW_STATE:
            return RED_STATE;
            break;
        case RED_STATE:
            return GREEN_STATE;
            break;
        default:
            return current_state;
    }
}

int main(void)
{
    app_init();

    while (true)
    {
        cmd = get_command();

        switch(state)
        {
            case RED_STATE:
                bsp_board_led_on(RED_STATE);

                if(cmd == GO_CMD)
                {
                    bsp_board_led_off(RED_STATE);
                    state = get_next_state(state);
                }
            break;

            case YELLOW_STATE:
                bsp_board_led_on(YELLOW_STATE);

                /* start timer */
                nrf_delay_ms(3000);

                bsp_board_led_off(YELLOW_STATE);
                state = get_next_state(state);
            break;

            case GREEN_STATE:
                bsp_board_led_on(GREEN_STATE);

                if(cmd == STOP_CMD)
                {
                    bsp_board_led_off(GREEN_STATE);
                    state = get_next_state(state);
                }
            break;
        }
    }
}

