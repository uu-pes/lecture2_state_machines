#include "pico/stdlib.h"

typedef enum _state {red_state, yellow_state, green_state} state_t; 
typedef enum _cmd {go_cmd, stop_cmd, no_cmd} cmd_t; 

static state_t state = red_state;
static cmd_t cmd;

const static uint led_red = 0; 
const static uint led_yellow = 1; 
const static uint led_green = 2; 

const static uint go_btn = 20; 
const static uint stop_btn = 21; 

static void app_init(void)
{
    /* Setup LEDs */
    gpio_init(led_red); 
    gpio_init(led_yellow); 
    gpio_init(led_green);  
    gpio_set_dir(led_red, GPIO_OUT);
    gpio_set_dir(led_yellow, GPIO_OUT);
    gpio_set_dir(led_green, GPIO_OUT);
    
    /* Setup buttons */
    gpio_init(go_btn); 
    gpio_init(stop_btn); 
    gpio_set_dir(go_btn, GPIO_IN); 
    gpio_set_dir(stop_btn, GPIO_IN); 
}

static cmd_t get_command(void)
{
    if(!gpio_get(go_btn))
    {
        return go_cmd;
    } 
    else if(!gpio_get(stop_btn))
    {
        return stop_cmd;
    } 
    else 
    {
        return no_cmd;
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
            case red_state:
                gpio_put(led_red, 1);

                if(cmd == go_cmd)
                {
                    gpio_put(led_red, 0);
                    state = green_state;
                }
            break;

            case yellow_state:
                gpio_put(led_yellow, 1);

                /* start timer */
                sleep_ms(3000);

                gpio_put(led_yellow, 0);
                state = red_state;
            break;

            case green_state:
                gpio_put(led_green, 1);

                if(cmd == stop_cmd)
                {
                    gpio_put(led_green, 0);
                    state = yellow_state;
                }
            break;
        }
    }
}

