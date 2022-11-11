/* GPIO Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

/**
 * Brief:
 * Pulse counter
 *
 * GPIO status:
 * GPIO18: output for testing
 * GPIO4:  counter input, pulled up, interrupt from rising edge.
 * GPIO0: (Boot button) - clears counter
 *
 * Test:
 * Connect GPIO18 with GPIO4
 * Generate pulses on GPIO18 to trigger interrupt on GPIO4
 *
 */

#define LEDC_OUTPUT_IO 18 // Output GPIO of a sample 1 Hz pulse generator
#define LEDC_HZ 100000

#define GPIO_INPUT_IO_0 4
#define GPIO_INPUT_IO_CLR 0
#define GPIO_INPUT_PIN_SEL ((1ULL << GPIO_INPUT_IO_0) | (1ULL << GPIO_INPUT_IO_CLR))
#define ESP_INTR_FLAG_DEFAULT 0

static xQueueHandle gpio_evt_queue = NULL;

uint32_t counts;
static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    uint32_t gpio_num = (uint32_t)arg;
    if (gpio_num == GPIO_INPUT_IO_CLR)
    {
        counts = 0;
        if (gpio_get_level(gpio_num) == 0)
        {
            // To avoid excess chatter, send the event only on the falling edge
            xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
        }
    }
    else
    {
        ++counts;
    }
}
static void gpio_task(void *arg)
{
    uint32_t gpio_num;
    for (;;)
    {
        if (xQueueReceive(gpio_evt_queue, &gpio_num, portMAX_DELAY))
        {
            if (gpio_num == GPIO_INPUT_IO_CLR)
            {
                printf("Clearing counter\n");
            }
        }
    }
}

/*
 * Configure LED PWM Controller to output sample pulses at LEDC_HZ
 */
static void ledc_init(void)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer;
    ledc_timer.speed_mode = LEDC_LOW_SPEED_MODE;
    ledc_timer.timer_num = LEDC_TIMER_1;
    ledc_timer.duty_resolution = LEDC_TIMER_9_BIT;
    ledc_timer.freq_hz = LEDC_HZ; // set output frequency at 1 Hz
    ledc_timer.clk_cfg = LEDC_AUTO_CLK;
    ledc_timer_config(&ledc_timer);

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel;
    ledc_channel.speed_mode = LEDC_LOW_SPEED_MODE;
    ledc_channel.channel = LEDC_CHANNEL_1;
    ledc_channel.timer_sel = LEDC_TIMER_1;
    ledc_channel.intr_type = LEDC_INTR_DISABLE;
    ledc_channel.gpio_num = LEDC_OUTPUT_IO;
    ledc_channel.duty = 100; // set duty cycle 100/2^resolution
    ledc_channel.hpoint = 0;
    ledc_channel_config(&ledc_channel);
}

void app_main(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_POSEDGE,
        .pin_bit_mask = GPIO_INPUT_PIN_SEL,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,
    };
    gpio_config(&io_conf);

    gpio_set_intr_type(GPIO_INPUT_IO_CLR, GPIO_INTR_ANYEDGE);

    // create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(gpio_task, "gpio_task", 2048, NULL, 10, NULL);

    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void *)GPIO_INPUT_IO_0);
    gpio_isr_handler_add(GPIO_INPUT_IO_CLR, gpio_isr_handler, (void *)GPIO_INPUT_IO_CLR);

    ledc_init(); // For testing

    while (1)
    {
        printf("counts: %d\n", counts);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}
