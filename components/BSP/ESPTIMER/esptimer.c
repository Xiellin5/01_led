#include "esptimer.h"

void esptimer_callback(void *arg)
{
    LED_TOGGLE();
}

void esptimer_init(uint64_t tps)
{
    esp_timer_handle_t timer_handle;
    esp_timer_create_args_t timer_args = {
        .callback = esptimer_callback,
        .arg = NULL,
        .name = "Timer",
        .dispatch_method = ESP_TIMER_TASK,
    };
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timer_handle));
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer_handle, tps));
}