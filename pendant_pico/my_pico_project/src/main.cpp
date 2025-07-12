// src/main.cpp

#include "lib/pin.hpp"
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "lib/lcd.hpp"           // Giả sử bạn có giao diện LCD
#include "lib/keypad.hpp"        // Khởi tạo keypad, encoder
//#include "src/ui_manager.hpp"
//#include "src/marlin_comm.hpp"
//#include "src/input_handler.hpp"

constexpr uint UART_ID = uart0;
constexpr uint BAUD_RATE = 115200;
constexpr uint UART_TX_PIN = 0;
constexpr uint UART_RX_PIN = 1;

int main() {
    stdio_init_all();

    // 1. Khởi tạo UART giao tiếp với Marlin
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    uart_set_hw_flow(UART_ID, false, false);
    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);

    // 2. Khởi tạo LCD (bạn tự điều chỉnh theo driver)
     lcd_init(PIN_LCD_SCK, PIN_LCD_MOSI, PIN_LCD_CS, PIN_LCD_DC);

    // 3. Khởi tạo input: keypad, encoder,...
     Keypad4x4 keypad(KEYPAD_ROWS, KEYPAD_COLS);
    keypad.init();
    encoder_init();

    // 4. Khởi tạo giao tiếp Marlin (giao tiếp uart nâng cao, parse)
    MarlinComm marlin(UART_ID);

    // 5. Khởi tạo UI manager, các trang giao diện
    UIManager uiManager(lcd, marlin);

    // Tạo các trang và đăng ký với UI manager
    uiManager.addPage(std::make_shared<WarningPage>(marlin));
    uiManager.addPage(std::make_shared<MachinePage>(marlin));
    uiManager.addPage(std::make_shared<JobPage>(marlin));
    uiManager.addPage(std::make_shared<ParameterPage>(marlin));

    uiManager.showPage(0); // Mở đầu là WarningPage

    // 6. Vòng lặp chính
    while (true) {
        // 6.1 Đọc input người dùng (nút, encoder)
        InputEvent ev;
        if (inputHandler_checkEvent(&ev)) {
            uiManager.handleInput(ev); // Xử lý input chuyển trang, chọn mục,...
        }

        // 6.2 Cập nhật giao tiếp Marlin, gửi/nhận dữ liệu
        marlin.process();
            uiManager.update();  // cập nhật trạng thái, dữ liệu mới
            uiManager.draw();    // vẽ lên màn hình

        // 6.3 Cập nhật trạng thái giao diện theo dữ liệu mới
        uiManager.update();

        // 6.4 Vẽ lại giao diện ra LCD (nếu cần)
        uiManager.draw();

        // 6.5 Delay nhẹ tránh tốn CPU, tuỳ theo tốc độ bạn muốn
        sleep_ms(10);
    }

    return 0;
}