#include "mraa_beaglebone_pinmap.h"
#include "stdio.h"
#include "unistd.h"
#include "stdlib.h"
#include "mraa.h"
#include "mraa_uart.h"
#include "mraa_aio.h"

// Pin mappings
int UART_DEVICE = 1;  // UART1
int UART_RX_PIN = 26; // P9_26
int UART_TX_PIN = 24; // P9_24
int AIO_PIN = 0;      // Analog input pin A0

// Thingspeak configuration
const char* THINGSPEAK_API_KEY = "YOUR_THINGSPEAK_API_KEY";
const char* THINGSPEAK_URL = "http://api.thingspeak.com/update";

// Function to read data from SDS011
float readSDS011Data(mraa_uart_context uart) {
    unsigned char buffer[10];
    float pm25, pm10;
    int bytesRead = mraa_uart_read(uart, buffer, sizeof(buffer));
    if (bytesRead == 10 && buffer[0] == 0xAA && buffer[1] == 0xC0) {
        int sum = 0;
        for (int i = 2; i < 8; i++) {
            sum += buffer[i];
        }
        if (sum % 256 == buffer[8] && buffer[9] == 0xAB) {
            pm25 = ((buffer[3] * 256) + buffer[2]) / 10.0;
            pm10 = ((buffer[5] * 256) + buffer[4]) / 10.0;
            return pm25;
        }
    }
    return -1.0; // Error reading data
}

// Function to send data to Thingspeak
void sendToThingspeak(float data) {
    char command[200];
    snprintf(command, sizeof(command), "curl -m 5 \"%s?api_key=%s&field1=%.2f\"", THINGSPEAK_URL, THINGSPEAK_API_KEY, data);
    system(command);
}

int main() {
    // Initialize UART
    mraa_uart_context uart;
    uart = mraa_uart_init(UART_DEVICE);
    if (uart == NULL) {
        fprintf(stderr, "Failed to initialize UART.\n");
        return 1;
    }
    if (mraa_uart_set_baudrate(uart, 9600) != MRAA_SUCCESS) {
        fprintf(stderr, "Failed to set UART baudrate.\n");
        return 1;
    }
    if (mraa_uart_set_mode(uart, 8, MRAA_UART_PARITY_NONE, 1) != MRAA_SUCCESS) {
        fprintf(stderr, "Failed to set UART mode.\n");
        return 1;
    }

    // Initialize analog input
    mraa_aio_context aio;
    aio = mraa_aio_init(AIO_PIN);
    if (aio == NULL) {
        fprintf(stderr, "Failed to initialize analog input.\n");
        return 1;
    }

    while (1) {
        // Read data from SDS011
        float pm25 = readSDS011Data(uart);
        if (pm25 >= 0) {
            // Send data to Thingspeak
            sendToThingspeak(pm25);
        }

        // Read analog input
        int sensorValue = mraa_aio_read(aio);
        float voltage = (sensorValue / 1023.0) * 5.0;
        // Process analog input data as needed

        usleep(5000000); // Wait for 5 seconds before reading data again
    }

    // Cleanup
    mraa_uart_stop(uart);
    mraa_aio_close(aio);

    return 0;
}
