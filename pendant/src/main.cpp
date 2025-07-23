
//Main.cpp

#include <cstring>  // Để dùng strlen()
#include "Keyboard.h"
#include "DisplayManager.h"
#include "MenuController.h"
#include "encoder.h"
#include <U8g2lib.h>
#include <Arduino.h>  // quan trọng cho Arduino framework 

// Timer handler để đọc Encoder
extern TIM_HandleTypeDef htim2;

// Các chân hiển thị SPI ST7920 - giữ nguyên theo phần cứng của bạn
#define PIN_CLK  PA5
#define PIN_DATA PA7
#define PIN_CS   PA4
#define PIN_RST  U8X8_PIN_NONE

// Khởi tạo màn hình
U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, PIN_CLK, PIN_DATA, PIN_CS, PIN_RST);
DisplayManager displayManager(u8g2);
// Menu điều khiển trạng thái giao diện
MenuController menuController(displayManager);

// Khởi tạo bàn phím theo chân pin (thay thế chân đúng phần cứng)
const uint8_t rowPins[4] = {PB0, PB1, PB3, PB4};
const uint8_t colPins[4] = {PB5, PB6, PB7, PA8};
Keyboard keyboard(rowPins, 4, colPins, 4);

// =================== Một số biến hỗ trợ ==================
uint32_t lastEncoderValue = 0;
static uint32_t lastCheckTime = 0;
static uint32_t lastReplyTime = 0;
static const uint32_t connTimeout = 3000; // timeout 3 giây hoặc tùy bạn đặt
bool uartConnected = false;
static int selectedParamIdx = 0;

// Trả về NAN nếu không tìm thấy mark hoặc không có số float sau dấu mark.
float parseFloatAfterMark(const String& line, const String& mark) {
    int idx = line.indexOf(mark);
    if (idx < 0) return NAN; // Không tìm thấy

    idx += mark.length(); // tới ngay sau "X:"
    // Bỏ qua phần trắng hoặc ký tự không cần thiết
    while (idx < line.length() && (line[idx] == ' ' || line[idx] == '=')) idx++;

    // Đặt vị trí kết thúc: tìm dấu cách tiếp theo hoặc hết dòng
    int endIdx = idx;
    while (endIdx < line.length() && (isDigit(line[endIdx]) || line[endIdx] == '.' || line[endIdx] == '-')) endIdx++;

    String numberStr = line.substring(idx, endIdx);
    if (numberStr.length() == 0) return NAN; // Không có số
    return numberStr.toFloat();
}
// =================== Hàm chính ==================
 // Thêm hàm setup() và loop() bắt buộc trong Arduino framework
void setup()
{
    // Khởi tạo HAL
    HAL_Init(); 
    SystemClock_Config(); // Khởi tạo Clock (Bạn sẽ cần thực hiện hàm này ở đâu đó)
    Serial1.begin(115200);   // UART 0, thường để debug serial console máy tính
    u8g2.begin();
    menuController.setExtruderType(0);  // Ví dụ đặt là CNC
    menuController.setUartConnected(false);
    uartConnected = true;
}

uint8_t extruderType = 0; // 0: CNC, 1: 3D printer
uint8_t exType = menuController.getExtruderType();

// Vòng lặp chính
void loop() {
    // Đọc dữ liệu UART từ Serial nếu có
    Serial1.setTimeout(100); // timeout cho readStringUntil
    while (Serial1.available()) {
    String line = Serial1.readStringUntil('\n'); 
    line.trim();
    if (line.length() == 0) continue;

    Serial1.println("[DBG] Marlin sent: " + line);

    if (line.indexOf("extruder=0") >= 0) {
        menuController.setExtruderType(0);
    } else if (line.indexOf("extruder=1") >= 0) {
        menuController.setExtruderType(1);
    }

    if (line.indexOf("X:") >= 0) {
        float parsed;

        parsed = parseFloatAfterMark(line, "X:");
        if (!isnan(parsed)) current_position[0] = parsed;

        parsed = parseFloatAfterMark(line, "Y:");
        if (!isnan(parsed)) current_position[1] = parsed;

        parsed = parseFloatAfterMark(line, "Z:");
        if (!isnan(parsed)) current_position[2] = parsed;

        parsed = parseFloatAfterMark(line, "A:");
        if (!isnan(parsed)) current_position[3] = parsed;

        parsed = parseFloatAfterMark(line, "C:");
        if (!isnan(parsed)) current_position[4] = parsed;
    }

    uartConnected = true;
    lastReplyTime = millis();
}

     // Gửi lệnh M105 (hoặc lệnh khác nếu cần) mỗi giây để kiểm tra tình trạng kết nối
    if (millis() - lastCheckTime > 1000) {
        Serial1.print("M114\r\n"); // Đảm bảo có \r\n
          lastCheckTime = millis();
    }
    
    // Kiểm tra timeout kết nối UART, nếu quá 3s không nhận dữ liệu => mất kết nối
    if (millis() - lastReplyTime > connTimeout) {
        uartConnected = false;
        for (int i=0; i < AXIS_COUNT; i++) current_position[i] = 0.0f;  
    }
    // Tự động setup trang autoPage
    menuController.setUartConnected(uartConnected); 
    int autoPage;
    if (!uartConnected) {
        autoPage = 0;
    } else {
        int extType = menuController.getExtruderType();
        if (extType == 0) autoPage = 1;
        else if (extType == 1) autoPage = 2;
        else autoPage = 0;
    }
    menuController.setAutoPage(autoPage);
    menuController.updatePage();
    int page = menuController.getCurrentPage();
    if (page == PAGE_HOME_STATUS) {
        displayManager.drawHomeStatusPage(menuController.getHomeStatusMenuSelected());
    }

     // Đọc bàn phím (nếu có) và gọi handleKey    
    char key = keyboard.scanKeyboard();
    if (key != 0) {
        menuController.handleKey(key);
    }
    
    switch(menuController.getCurrentPage()) {
        case 0:
            displayManager.drawWarningPage();
            break;
        case 1:           
        displayManager.drawG54Page_FromMarlin_CurrentPos(); 
            break;

        case 2:
            if (!menuController.isUartConnected()) {
                displayManager.draw3DPage_NeedConnection();
            } else {
                displayManager.draw3DPage_WithConnected(); // Bạn cần tạo hàm hiển thị trạng thái 3D printer
            }
            break;

        case 3:// PAGE_PARAMETER
           displayManager.drawParameterPage(selectedParamIdx);
            break;
        case 4: // PAGE_HOME_STATUS
        displayManager.drawHomeStatusPage(displayManager.getHomeStatusMenuSelected());
    break;
          
        default:
          
        break;
        }
    delay(10); // tránh chiếm CPU quá nhiều
}
// Bạn nhớ bổ sung hàm Error_Handler() nếu chưa có
#ifdef Error_Handler
#undef Error_Handler
#endif
void Error_Handler()
{
    // Xử lý lỗi tùy ý, ví dụ treo hệ thống hoặc báo lỗi qua LED
    while (1)
    {
        // blink led hoặc đơn giản lặp vô hạn
    }
}

