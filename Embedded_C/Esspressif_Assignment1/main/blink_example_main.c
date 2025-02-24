/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <rom/ets_sys.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"

static const char *TAG = "example";

/* Use project configuration menu (idf.py menuconfig) to choose the GPIO to blink,
   or you can edit the following line and set a number here.
*/

// Pin definitions
#define greenLED 19
#define redLED 21
#define OUTPUT_SELECT 23
#define OUTPUT_ENABLE 15
#define TSyncON 50

// Defining paramters
/*
* a -> Delay value of first pulse in TsyncON (microseconds)
* b -> Delay value of off-time between each pulse (microseconds)
* c -> Number of waveforms in data waveform cycle
* d -> Time delay at end of TsyncON before starting next TsyncON cycle
*/
const int param_a = 13 * 100;  // surname letter = m, value = 1300 microseconds
const int param_b = 1 * 100;   // surname letter = a, value = 0.1 microseconds
const int param_c = 3 + 4;          // surname letter = x, value = 7
const int param_d = 4 * 500;   // surname letter = w, value = 2000 mircoseconds

// Button values
bool buttonSelectState; // Read value for button select state
bool buttonEnableState; // Read value for button enable state
const int debounceDelay = 500; // Delay to prevent debounce causing continuous incrementation 
                               // of count value

// OUTPUT_ENABLE counter
int count = 0;

// toggle button Enable state
bool toggle = 1;

//////////////////////////////////////////////////////////
////////////// idf.py configuration output 1 /////////////
//////////////////////////////////////////////////////////

#ifdef CONFIG_BLINK_LED_STRIP

static led_strip_handle_t led_strip;

static void blink_led(void)
{
    /* If the addressable LED is enabled */
    if (toggle) {
        /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
        led_strip_set_pixel(led_strip, 0, 16, 16, 16);
        /* Refresh the strip to send data */
        led_strip_refresh(led_strip);
    } else {
        /* Set all LED off to clear all pixels */
        led_strip_clear(led_strip);
    }
}

static void configure_led(void)
{
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = BLINK_GPIO,
        .max_leds = 1, // at least one LED on board
    };
#if CONFIG_BLINK_LED_STRIP_BACKEND_RMT
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
#elif CONFIG_BLINK_LED_STRIP_BACKEND_SPI
    led_strip_spi_config_t spi_config = {
        .spi_bus = SPI2_HOST,
        .flags.with_dma = true,
    };
    ESP_ERROR_CHECK(led_strip_new_spi_device(&strip_config, &spi_config, &led_strip));
#else
#error "unsupported LED strip backend"
#endif
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

#elif CONFIG_BLINK_LED_GPIO
//////////////////////////////////////////////////////////
/////////// idf.py configuration main output /////////////
//////////////////////////////////////////////////////////
/*
static void blink_redled(void)
{
    // Set the GPIO level according to the state (LOW or HIGH)
    gpio_set_level(greenLED, toggle);
}
*/
static void dataOutputSignal()
{
    // Start TsyncON pulse
    gpio_set_level(redLED, 1); // activate red LED
    ets_delay_us(TSyncON); // Set 50 microseconds HIGH delay
    gpio_set_level(redLED, 0); // deactivate red LED
    // Start parameter A pulse
    gpio_set_level(greenLED, 1); // activate green LED
    ets_delay_us(param_a); // Parameter A pulse HIGH delay in milliseconds
    gpio_set_level(greenLED, 0); // deactivate green LED
    ets_delay_us(param_b); // Paramter B delay pulse LOW in milliseconds

    for (int index = 1; index <= param_c; index++) // increment through index from 1 until parameter C is met
    {
        gpio_set_level(greenLED, 1); // Activate green LED
        ets_delay_us(param_a + (index * 50)); // Delay next pulse by the sum of paramter A with the sum if the index*50 microseconds
        gpio_set_level(greenLED, 0); // Deactivate green LED
        ets_delay_us(param_b); // Paramter B delay pulse LOW in milliseconds
    }

    ets_delay_us(param_d); // Delay next waveform TsyncON by parameter D
}

static void altDataOutputSignal()
{
    // Start TsyncON pulse
    gpio_set_level(redLED, 1); // Activate red LED
    ets_delay_us(TSyncON); // Set 50 microseconds HIGH Delay
    gpio_set_level(redLED, 0); // Deactivate red LED

    for (int index = param_c; index >= 1; index--) // increment from paramter C until 1 is met
    {
        gpio_set_level(greenLED, 1); // Activate green LED
        ets_delay_us(param_a + (index * 50)); // Delay next pulse by the sum of paramter A with the sum if the index*50 microseconds
        gpio_set_level(greenLED, 0); // Deactivate green LED
        ets_delay_us(param_b); // Set delay of next pulse by parameter B
    }
    // Start parameter A pulse
    gpio_set_level(greenLED, 1); // Activate green LED
    ets_delay_us(param_a); // Delay parameter A in milliseconds
    gpio_set_level(greenLED, 0); // Deactivate green LED
    ets_delay_us(param_b); // Paramter B delay pulse LOW in milliseconds

    ets_delay_us(param_d); // Delay next waveform TsyncON by parameter D
}

static void configureLEDsandButtons(void)
{
    // Reset the GPIOs to initial value on reset
    gpio_reset_pin(greenLED); // Reset green LED on startup/reset
    gpio_reset_pin(redLED); // Reset red LED on startup/reset
    gpio_reset_pin(OUTPUT_ENABLE); // Reset OUTPUT_ENABLE pin on startup/reset
    gpio_reset_pin(OUTPUT_SELECT); // Reset OUTPUT_SELECT pin on startup/reset
    // Set the LEDs as outputs
    gpio_set_direction(greenLED, GPIO_MODE_OUTPUT); // greenLED = OUTPUT
    gpio_set_direction(redLED, GPIO_MODE_OUTPUT); // redLED = OUTPUT
    // Set the button read pins as inputs
    gpio_set_direction(OUTPUT_ENABLE, GPIO_MODE_INPUT); // OUTPUT_ENABLE = INPUT
    gpio_set_direction(OUTPUT_SELECT, GPIO_MODE_INPUT); // OUTPUT_SELECT = INPUT
}

#else
#error "unsupported LED type"
#endif

//////////////////////////////////////////////////////////
//////////////////// Main Function ///////////////////////
//////////////////////////////////////////////////////////

void app_main(void)
{

    /* Configure the peripheral according to the LED type */
    configureLEDsandButtons();

    while (1) 
    {
       buttonEnableState = gpio_get_level(OUTPUT_ENABLE); // read enable button
       buttonSelectState = gpio_get_level(OUTPUT_SELECT); // read select button

       // When Enable pushbutton is pressed
       if (buttonEnableState == 1)
       {
            toggle = !toggle; // change toggle to true/false
            vTaskDelay(debounceDelay / portTICK_PERIOD_MS); // Set a debounce delay so input doesn't change in the same press
       }

       // When select button is pressed 
       if (buttonSelectState == 1)
       {
            count += 1; // Increase count value
            vTaskDelay(debounceDelay / portTICK_PERIOD_MS); // Set a debounce delay so input doesn't change in the same press
       }

       // When toggle is true
       if (toggle == true)
       {
            switch (count)
            {
                case 0: // When count = 0
                    dataOutputSignal(); // Output original data waveform
                    break;
                case 1: // When count = 1
                    altDataOutputSignal(); // Output alternative data waveform
                    break;
                case 2: // When count = 2
                    count = 0; // Reset count to 0
                    break;
                default: // In the circumstance that count does not equal 0, 1 or 2
                    gpio_set_level(redLED, 0); // Set red LED LOW
                    gpio_set_level(greenLED, 0); // Set green LED LOW
            }
       }
    }
}
