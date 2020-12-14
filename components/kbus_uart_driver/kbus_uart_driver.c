#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "string.h"
#include "driver/gpio.h"

#include "kbus_uart_driver.h"

#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)
#define SERVICE_UART UART_NUM_2
#define LED_PIN (GPIO_NUM_2)

#define HERTZ(hz) ((1000/hz) / portTICK_RATE_MS)


//TODO: Add to kconfig
#define KBUS_TX_TEST_IS_ENABLED flase

static const int RX_BUF_SIZE = 1024;
static const char* TAG = "kbus_driver";
static void (*rx_handler)(uint8_t* data) = NULL;
static uint8_t rx_polling = 0;

#if KBUS_TX_TEST_IS_ENABLED
static const int TX_BUF_SIZE = 265;
static void tx_task();
#endif
static void rx_task();


void init_kbus_uart_driver(void (*rx_callback)(uint8_t* data), uint8_t rx_poll_hz) {
    //* Configuring based on i/k bus spec:
    //* http://web.archive.org/web/20070513012128/http://www.openbmw.org/bus/
    ESP_LOGI(TAG, "Initializing Kbus UART");
    const uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_EVEN,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    ESP_ERROR_CHECK(uart_param_config(SERVICE_UART, &uart_config));

    // UART2 is supposed to be on 16/17 by default; didn't seem to be the case when testing...
    ESP_ERROR_CHECK(uart_set_pin(SERVICE_UART, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    // uart_set_pin() sets GPIO_PULLUP_ONLY on RX, not TX. Need it to avoid taking control of k-bus when only listening.
    ESP_ERROR_CHECK(gpio_set_pull_mode(TXD_PIN, GPIO_PULLUP_ONLY));
    // We won't use a buffer for sending data.
    ESP_ERROR_CHECK(uart_driver_install(SERVICE_UART, RX_BUF_SIZE * 2, 0, 0, NULL, 0));
    // Make sure we're in standard uart mode
    ESP_ERROR_CHECK(uart_set_mode(SERVICE_UART, UART_MODE_UART));

    ESP_LOGI(TAG, "Creating rx task");
    xTaskCreatePinnedToCore(rx_task, "uart_rx_task", RX_BUF_SIZE*2, NULL, configMAX_PRIORITIES, NULL, 1);
#if KBUS_TX_TEST_IS_ENABLED
    ESP_LOGI(TAG, "Creating tx task");
    xTaskCreatePinnedToCore(tx_task, "uart_tx_task", TX_BUF_SIZE*2, NULL, configMAX_PRIORITIES-1, NULL, 1);
#endif

    // Setup onboard LED
    gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_PIN, 0);

    // Store RX Handler and RX polling rate
    rx_handler = rx_callback;
    rx_polling = rx_poll_hz;
}

int kbus_send_str(const char* logName, const char* str) {
    ESP_LOGD(logName, "Sending string...");
    const int len = strlen(str); //! /0 delimited char array.
    const int txBytes = uart_write_bytes(SERVICE_UART, str, len);
    ESP_LOGI(logName, "Wrote %d bytes", txBytes);
    return txBytes;
}

int kbus_send_bytes(const char* logName, const char* bytes, uint8_t numBytes) {
    ESP_LOGD(logName, "Sending raw bytes...");
    const int txBytes = uart_write_bytes(SERVICE_UART, bytes, numBytes);
    ESP_LOGI(logName, "Wrote %d bytes", txBytes);
    return txBytes;
}

#if KBUS_TX_TEST_IS_ENABLED
/**
 * @brief Task that handles incoming kbus data
 */
static void tx_task()
{
    static const char *TX_TASK_TAG = "TX_TASK";
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
    while (1) {
        kbus_send_str(TX_TASK_TAG, "Hello world");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
#endif

/**
 * @brief Task that handles incoming kbus data
 */
static void rx_task()
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    while (1) {
        // gpio_set_level(LED_PIN, 1);
        const int rxBytes = uart_read_bytes(SERVICE_UART, data, RX_BUF_SIZE, HERTZ(rx_polling));
        gpio_set_level(LED_PIN, 0);
        // vTaskDelay(200 / portTICK_RATE_MS);
        if (rxBytes > 0) {
            gpio_set_level(LED_PIN, 1);
            data[rxBytes] = 0;
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
            ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_DEBUG);
            // gpio_set_level(LED_PIN, 0);
            if(rx_handler != NULL) {
                rx_handler(data);
            }
        }
    }
    free(data);
}
