#include "stm32f1xx_hal.h"
#include "Keyboard.h"
#include "DisplayManager.h"
#include "PrinterI2C.h"
//#include "Encoder.h"
#include <U8g2lib.h>

// Extern phần cứng STM32
extern TIM_HandleTypeDef htim2;

#define PIN_CLK PA5   // Clock (SCLK)
#define PIN_DATA PA7  // Data (MOSI / SID)
#define PIN_CS PA4    // Chip Select
#define PIN_RST -1   // Reset (hoặc dùng U8X8_PIN_NONE nếu không có chân reset)
#define PAGE_COUNT 7

// Định nghĩa chân bàn phím (thay chân đúng phần cứng bạn)
const uint8_t rowPins[4] = {PB0, PB1, PB3, PB4};
const uint8_t colPins[4] = {PA2, PA3, PB5, PA8};
Keyboard keyboard(rowPins, 4, colPins, 4);

I2C_HandleTypeDef hi2c1;
U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, PIN_CLK, PIN_DATA, PIN_CS, PIN_RST);
DisplayManager displayManager(u8g2);
PrinterI2C printerI2c(&hi2c1, 0x42 << 1);
static Page currentPage = PAGE_WARNING;  // Mặc định trang cảnh báo lúc chưa kết nối i2c
static bool i2cConnected = false;

int selectedParamIndex = 0;  // giá trị mặc định, hoặc cập nhật logic của bạn

float g54Offsets[AXIS_COUNT] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
float g55Offsets[AXIS_COUNT] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

// Biến lưu trữ phím bấm (ví dụ)
static char lastKeyPressed = 0;

void handleKey(char key) {
    if (key == 0) return;

    // Chỉ cho phép chuyển trang khi đã kết nối I2C
    if (!i2cConnected) return;

    switch (key) {
        case '*':
            // Chỉ cho phép chuyển trang khi đã kết nối
            currentPage = (Page)((currentPage + 1) % PAGE_COUNT);
            break;
        case 'D':
            // Chỉ gửi lệnh home khi đang ở trang HOME_STATUS
            if (currentPage == PAGE_HOME_STATUS) {
                printerI2c.sendHomeCommand();
                Serial.println("Sent Home All command");
            }
            break;
        case 'X': case '1':
        case 'Y': case '2':
        case 'Z': case '3':
        case 'A':
        case 'C':
        case 'E': case '#':
            // Có thể lưu xử lý trục nhấn nếu cần, hoặc mở rộng cho PAGE_HOME_STATUS
            if (currentPage == PAGE_HOME_STATUS) {
                lastKeyPressed = key;
                // ...xử lý mở rộng cho các phím trục ở trang HOME_STATUS...
            }
            break;
        default:
            break;
    }
}

// Thêm getter cho PrinterStatus mới nhất
const PrinterStatus& getCurrentPrinterStatus() {
    return printerI2c.getLastStatus();
}

void setup() {
    HAL_Init();
    SystemClock_Config();
    u8g2.begin();
    printerI2c.begin();
    Serial.begin(115200); // Thêm dòng này để xem debug từ bàn phím
    //encoder.begin(); tạm bỏ
}

void loop() {
    // Đọc phím
    char key = keyboard.scanKeyboard();
    if (key) {
        Serial.print("Main loop got key: ");
        Serial.println(key);
        handleKey(key);
    }
    printerI2c.update();
    // Debug: In trạng thái nhận được từ Marlin
    const PrinterStatus& st = printerI2c.getLastStatus();
    Serial.print("X: "); Serial.print(st.position[0]);
    Serial.print(" Y: "); Serial.print(st.position[1]);
    Serial.print(" Z: "); Serial.print(st.position[2]);
    Serial.print(" State: "); Serial.println(st.state);

    bool newI2cConnected = printerI2c.isConnected();
    if (newI2cConnected != i2cConnected) {
        i2cConnected = newI2cConnected;
        if (i2cConnected) {
            currentPage = PAGE_MACHINE; // Khi vừa kết nối, chuyển ngay sang trang machine
        } else {
            currentPage = PAGE_WARNING;
        }
    }

    // Nếu chưa kết nối, luôn ở trang cảnh báo
    if (!i2cConnected) {
        currentPage = PAGE_WARNING;
    }

    if (currentPage == PAGE_WARNING) {
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_6x12_tr);
        u8g2.drawStr(0, 12, "Waiting for Marlin...");
        u8g2.sendBuffer();
    } else {
        switch (currentPage) {
            case PAGE_HOME_STATUS:
                displayManager.drawHomeStatusScreen(&getCurrentPrinterStatus());
                break;
            case PAGE_PARAMETER:
                displayManager.drawParameterPage(selectedParamIndex);
                break;
            case PAGE_MACHINE:
                displayManager.drawMachineControlPage(
                    0, 0, 0, &getCurrentPrinterStatus()
                );
                break;
            case PAGE_G54:
                displayManager.drawG54Page(g54Offsets);
                break;
            case PAGE_G55:
                displayManager.drawG55Page(g55Offsets);
                break;
            default:
                displayManager.drawStatusScreen(&getCurrentPrinterStatus());
                break;
        }
    }

    HAL_Delay(10);
}