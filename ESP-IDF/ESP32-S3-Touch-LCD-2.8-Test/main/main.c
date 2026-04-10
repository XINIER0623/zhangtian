#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "PWR_Key.h"
#include "ST7789.h"
#include "LVGL_Driver.h"
#include "ziwei_ui.h"

void app_main(void)
{
    static const char *TAG = "APP";

    ESP_LOGI(TAG, "app_main start");
    PWR_Init();
    ESP_LOGI(TAG, "power init done");
    LCD_Init();
    ESP_LOGI(TAG, "lcd init done");
    LVGL_Init();
    ESP_LOGI(TAG, "lvgl init done");
    Ziwei_App_Init();
    ESP_LOGI(TAG, "ziwei ui init done");

    while (1) {
        uint32_t delay_ms = lv_timer_handler();

        if (delay_ms == LV_NO_TIMER_READY) {
            delay_ms = 5;
        }

        if (delay_ms > 5) {
            delay_ms = 5;
        }

        if (delay_ms == 0) {
            taskYIELD();
            continue;
        }

        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}
