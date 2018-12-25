#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "gps.h"

const char *TAG = "main";

esp_err_t event_handler(void *ctx, system_event_t *event)
{
    return ESP_OK;
}

static void gps_debug_event(void *pvParameters)
{
    while (true)
    {
#if 1
        ESP_LOGD(TAG, "Latitude: %f", gps_info->latitude);
        ESP_LOGD(TAG, "Longtitude: %f", gps_info->longtitude);
        ESP_LOGD(TAG, "Sattelites: %d", gps_info->satellites);
        ESP_LOGD(TAG, "Speed: %f", gps_info->speed);
#endif
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    nvs_flash_init();
    gps_init();

    xTaskCreate(gps_debug_event, "gps_info_task", 2048, NULL, 12, NULL);
}
