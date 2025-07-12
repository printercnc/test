#ifndef PIN_HPP
#define PIN_HPP

// Khai báo chân GPIO cho toàn dự án

// UART Marlin
constexpr uint UART_TX_PIN = 0;
constexpr uint UART_RX_PIN = 1;
constexpr uint UART_ID = uart0;
constexpr uint UART_BAUDRATE = 115200;

// LCD SPI (giả sử dùng spi0)
constexpr uint PIN_LCD_SCK = 18;
constexpr uint PIN_LCD_MOSI = 19;
//constexpr uint PIN_LCD_MISO = 16;      // nếu cần đọc
constexpr uint PIN_LCD_CS = 5;
//constexpr uint PIN_LCD_DC = 17;
//constexpr uint PIN_LCD_RESET = 20;     // nếu có chân reset

// Keypad 4x4 hàng và cột
constexpr uint KEYPAD_ROWS[4] = {6, 7, 8, 9};
constexpr uint KEYPAD_COLS[4] = {10, 11, 12, 13};

// Encoder (nếu có)
constexpr uint ENCODER_PIN_A = 14;
constexpr uint ENCODER_PIN_B = 15;


#endif // PIN_HPP