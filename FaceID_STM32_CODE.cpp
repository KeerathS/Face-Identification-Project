#include "mbed.h"
#include "LCD_DISCO_F429ZI.h"
#include "fonts.h"
#include <cstring>

// Create an LCD instance for the STM32F429 Discovery board
LCD_DISCO_F429ZI lcd;

// Use alternate UART pins: PB_10 = TX, PB_11 = RX, using USART3 at 115200 baud
BufferedSerial serial(PB_10, PB_11, 115200);

// Buffer for incoming serial data
#define BUFFER_SIZE 64
char msgBuffer[BUFFER_SIZE] = {0};
int bufIndex = 0;

// LED on PG_13 to indicate successful reception of "Hello"
DigitalOut led(PG_13);

// User button (using BUTTON1 alias, predefined for the board)
DigitalIn userButton(BUTTON1);

// FSM states
enum State {
    WAITING_FOR_DATA,
    DISPLAYING_DATA,
    CLEARING_DISPLAY
};

/// @brief Displays the given message on the LCD by wrapping text into multiple lines.
/// @param message The null-terminated string to display.
void displayWrappedText(const char *message) {
    // Clear LCD and print a header on line 1
    lcd.Clear(LCD_COLOR_WHITE);
    lcd.DisplayStringAt(0, LINE(1), (uint8_t *)"Connected!", CENTER_MODE);
    
    // For this example, we assume Font16 is used and that gives about 30 characters per line.
    const int maxCharsPerLine = 30;
    int len = strlen(message);
    int line = 3;  // Start displaying from LINE(3)
    
    // Loop through the message and display each chunk on a new line.
    for (int i = 0; i < len; i += maxCharsPerLine) {
        char buffer[maxCharsPerLine + 1];
        memset(buffer, 0, sizeof(buffer));
        // Copy up to maxCharsPerLine characters into buffer
        strncpy(buffer, message + i, maxCharsPerLine);
        // Display this line (centered horizontally)
        lcd.DisplayStringAt(0, LINE(line++), (uint8_t *)buffer, CENTER_MODE);
    }
    printf("%s\n", message);
    ThisThread::sleep_for(2500ms);
    
}

/// @brief Clears the LCD and resets the display state.
void clearDisplay() {
    lcd.Clear(LCD_COLOR_WHITE);
    lcd.DisplayStringAt(0, LINE(2), (uint8_t *)"SEARCHING FOR FACE", CENTER_MODE);
    lcd.DisplayStringAt(0, LINE(4), (uint8_t *)"USER...", CENTER_MODE);
}

int main() {
    // Initialize LCD: clear screen and display waiting message on line 2
    lcd.Clear(LCD_COLOR_WHITE);
    lcd.SetFont(&Font16);
    lcd.SetTextColor(LCD_COLOR_DARKBLUE);
    lcd.SetBackColor(LCD_COLOR_WHITE);
    lcd.DisplayStringAt(0, LINE(2), (uint8_t *)"START UP", CENTER_MODE);
    lcd.DisplayStringAt(0, LINE(4), (uint8_t *)"Waiting for data...", CENTER_MODE);

    // Ensure the LED is off
    led = 0;

    // Initialize FSM state
    State currentState = WAITING_FOR_DATA;

    while (true) {
        switch (currentState) {
            case WAITING_FOR_DATA: {
                // Check for incoming serial data
                if (serial.readable()) {
                    char c;
                    if (serial.read(&c, 1) == 1) {
                        // Append the character to msgBuffer if there's space
                        if (bufIndex < BUFFER_SIZE - 1) {
                            msgBuffer[bufIndex++] = c;
                        }

                        // If we detect a newline or carriage return, or buffer is nearly full, process the message.
                        if (c == '\n' || c == '\r' || bufIndex >= BUFFER_SIZE - 1) {
                            msgBuffer[bufIndex] = '\0'; // Null-terminate the string

                            // Remove trailing newline or carriage return if present
                            if (bufIndex > 0 && (msgBuffer[bufIndex - 1] == '\n' || msgBuffer[bufIndex - 1] == '\r')) {
                                msgBuffer[bufIndex - 1] = '\0';
                            }

                            // Transition to DISPLAYING_DATA state
                            currentState = DISPLAYING_DATA;
                        }
                    }
                }
                break;
            }

            case DISPLAYING_DATA: {
                // Display the wrapped text on the LCD
                displayWrappedText(msgBuffer);

                // For example, if the message is exactly "Hello", light the LED.
                if (strcmp(msgBuffer, "Hello") == 0) {
                    led = 1;
                } else {
                    led = 0;
                }

                // Wait for the user button to be pressed
                if (userButton.read() == 0) { // Assuming active-low button
                    // Transition to CLEARING_DISPLAY state
                    currentState = CLEARING_DISPLAY;
                    ThisThread::sleep_for(200ms); // Debounce delay
                }
                break;
            }

            case CLEARING_DISPLAY: {
                // Clear the display and reset the state
                clearDisplay();
                led = 0; // Turn off the LED

                // Reset the buffer and index to prepare for new messages
                memset(msgBuffer, 0, sizeof(msgBuffer));
                bufIndex = 0;

                // Transition back to WAITING_FOR_DATA state
                currentState = WAITING_FOR_DATA;

                // Clear any remaining data in the UART buffer
                while (serial.readable()) {
                    char c;
                    serial.read(&c, 1); // Discard leftover characters
                }
                break;
            }
        }

        // Small delay to prevent busy-waiting
        ThisThread::sleep_for(50ms);
    }
}