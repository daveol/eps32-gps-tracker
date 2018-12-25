#include "gps.h"

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include "esp_log.h"
#include "esp_intr_alloc.h"
#include "driver/uart.h"


#ifdef CONFIG_GPS_UART_NUM_0
#define GPS_UART_NUM UART_NUM_0
#define GPS_UART UART0
#elif CONFIG_GPS_UART_NUM_1
#define GPS_UART_NUM UART_NUM_1
#define GPS_UART UART1
#elif CONFIG_GPS_UART_NUM_2
#define GPS_UART_NUM UART_NUM_2
#define GPS_UART UART2
#else
#error "GPS_UART_NUM is undefined"
#endif

#ifdef CONFIG_GPS_UART_TX_GPIO
#define GPS_UART_TX CONFIG_GPS_UART_TX_GPIO
#else
#define GPS_UART_TX UART_PIN_NO_CHANGE
#endif

#ifdef CONFIG_GPS_UART_RX_GPIO
#define GPS_UART_RX CONFIG_GPS_UART_RX_GPIO
#else
#define GPS_UART_RX UART_PIN_NO_CHANGE
#endif

#define QUEUE_SIZE 20

static intr_handle_t uart_handle;
static QueueHandle_t nmea_queue = NULL;
static const char *TAG = "gps";

gps_info_t* gps_info = NULL;

static void sentenceTask(void *pvParameters)
{
    char line[128];
    while (true)
    {
        bzero(line, sizeof(line));
        if (xQueueReceive(nmea_queue, &line, portMAX_DELAY))
        {
            char prefix[5];
            bzero(prefix, 5);
            memcpy(prefix, line+1, 5);
            ESP_LOGD(TAG, "Please implement: %s", prefix);
        }
    }
}

static char h_buffer[256];
static uint8_t h_index;

static void IRAM_ATTR uart_intr_handle(void *arg)
{
    BaseType_t xHigherPrioritTaskWoken;
    uint32_t length = GPS_UART.status.rxfifo_cnt;

    while (length != 0)
    {
        h_buffer[h_index] = GPS_UART.fifo.rw_byte;

        if (h_index > 1)
        {
            if (h_buffer[h_index - 1] == '\r' && h_buffer[h_index] == '\n')
            {
                xQueueSendFromISR(nmea_queue, &h_buffer, &xHigherPrioritTaskWoken);
                bzero(&h_buffer, sizeof(h_buffer));
                h_index = 0;
            }
        }

        // sentence starts with '$' so we check for that
        if (h_buffer[0] != '$'){
            // reset if not the case
            h_index = 0;
        } else {
            // otherwise cary on
            h_index++;
        }

        length--;
    }

    ESP_ERROR_CHECK(uart_clear_intr_status(GPS_UART_NUM, UART_RXFIFO_FULL_INT_CLR | UART_RXFIFO_TOUT_INT_CLR));
}

esp_err_t gps_init()
{
    uart_config_t gps_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE};

    nmea_queue = xQueueCreate(QUEUE_SIZE, sizeof(char[128]));
    gps_info = (gps_info_t *)malloc(sizeof(gps_info_t));
    h_index = 0;

    bzero(&h_buffer, sizeof(h_buffer));
    bzero(gps_info, sizeof(gps_info_t));

    ESP_LOGD(TAG, "Registering UART for GPS, using UART: %d", GPS_UART_NUM);
    ESP_ERROR_CHECK(uart_param_config(GPS_UART_NUM, &gps_config));

#ifdef CONFIG_GPS_UART_CUSTOM
    ESP_LOGD(TAG, "Registering custom UART pins for GPS, using pins: TX=%d RX=%d", GPS_UART_TX, GPS_UART_RX);
    ESP_ERROR_CHECK(uart_set_pin(GPS_UART_NUM, GPS_UART_TX, GPS_UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
#endif

    //ESP_ERROR_CHECK(uart_isr_free(GPS_UART_NUM));
    ESP_ERROR_CHECK(uart_isr_register(GPS_UART_NUM, uart_intr_handle, NULL, ESP_INTR_FLAG_IRAM, &uart_handle));
    ESP_ERROR_CHECK(uart_enable_rx_intr(GPS_UART_NUM));

    xTaskCreate(sentenceTask, "sentenceTask", 4096, NULL, 11, NULL);

    return ESP_OK;
}