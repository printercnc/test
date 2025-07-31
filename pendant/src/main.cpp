
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
static unsigned long lastCheckTime = 0;
static const unsigned long checkInterval = 1000;
static bool waitingAck = false;

static unsigned long lastReplyTime = 0;
static const unsigned long connTimeout = 3000; // 3 giây timeout

// chế độ quản lý gửi parameter
static bool parameterModeActive = false;
static int currentParamIndex = 0;
static int parameterSelectedIdx = 0;
const char* parameterCommands[] = {
    "M201 X\r\n",
    "M201 Y\r\n",
    "M201 Z\r\n",
    "M201 A\r\n",
    "M201 C\r\n",
    "M201 E\r\n"
};
const int parameterCommandsCount = sizeof(parameterCommands) / sizeof(parameterCommands[0]);

void sendCommand(const char* cmd) {
    Serial1.print(cmd);
    waitingAck = true;
}

// Gửi lệnh M114 định kỳ, mỗi giây 1 lần nếu không đang chờ ACK
void updatePositionRequest() {
    if (!waitingAck && (millis() - lastCheckTime >= checkInterval)) {
        sendCommand("M114\r\n");
        lastCheckTime = millis();
    }
}

// Gửi chuỗi lệnh parameter khi vào trang parameter
void updateParameterCommands() {
    if (!parameterModeActive) return;
    if (waitingAck) return; // cứ chờ phản hồi rồi mới gửi tiếp

    if (currentParamIndex < parameterCommandsCount) {
        sendCommand(parameterCommands[currentParamIndex]);
        currentParamIndex++;
    } else {
        // Đã gửi hết lệnh param => tắt chế độ này
        parameterModeActive = false;
        currentParamIndex = 0;
    }
}

void onEnterParameterPage() {  // gọi khi user vào trang param
    parameterModeActive = true;
    currentParamIndex = 0;
    waitingAck = false; // reset chờ ack
}

void onExitParameterPage() {
    parameterModeActive = false;
    currentParamIndex = 0;
    waitingAck = false;
}

// Hàm tiện ích lấy float sau marker trong chuỗi
float parseFloatAfterMark(const String &line, const char* mark) {
    int idx = line.indexOf(mark);
    if (idx < 0) return NAN;
    idx += strlen(mark);
    int endIdx = idx;

    // Tìm vị trí kết thúc number (số hoặc dấu âm, chấm, số)
    while (endIdx < line.length() && 
           (isDigit(line[endIdx]) || line[endIdx] == '.' || line[endIdx] == '-' || line[endIdx] == '+')) {
        endIdx++;
    }
    String numStr = line.substring(idx, endIdx);
    return numStr.toFloat();
}
bool uartConnected = false;
void setup()
{
    // Khởi tạo HAL
   // HAL_Init(); 
   // SystemClock_Config(); // Khởi tạo Clock (Bạn sẽ cần thực hiện hàm này ở đâu đó)
    Serial1.begin(115200);   // UART 0, thường để debug serial console máy tính
    u8g2.begin();
    menuController.setExtruderType(0);  // Ví dụ đặt là CNC
    menuController.setUartConnected(false);
    uartConnected = true;
}

// Vòng lặp chính
void loop() {
    // Đọc dữ liệu UART từ Serial nếu có
    while (Serial1.available()) {
    String line = Serial1.readStringUntil('\n'); 
    line.trim();
    if (line.length() == 0) continue;

      Serial.println("[RX]" + line); // Debug nhận

       lastReplyTime = millis();
        uartConnected = true;

        // Kiểm tra loại phản hồi, bật tắt cờ chờ ACK
        if (line.equalsIgnoreCase("ok")) {
            waitingAck = false;
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

    if (line.indexOf("extruder=0") >= 0) {
        menuController.setExtruderType(0);
    } else if (line.indexOf("extruder=1") >= 0) {
        menuController.setExtruderType(1);
    }
}   
// Gọi hàm gửi lệnh định kỳ M114 (nếu không chờ ACK)
    updatePositionRequest();
// Gọi hàm gửi tuần tự lệnh parameter khi vào trang parameter
    updateParameterCommands();

    // Kiểm tra timeout kết nối UART, nếu quá 3s không nhận dữ liệu => mất kết nối
    if (millis() - lastReplyTime > connTimeout) {
        uartConnected = false;
        for (int i=0; i < AXIS_COUNT; i++) current_position[i] = 0.0f;  
    }
    
    menuController.setUartConnected(uartConnected); // Tự động setup trang autoPage
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
          displayManager.drawParameterPage(parameterSelectedIdx);
            break;
        case 4: // PAGE_HOME_STATUS
       displayManager.drawHomeStatusPage(
    menuController.getHomeStatusMenuSelected(),
    menuController.getSelectedAxis(),
    menuController.getCurrentJogMultiplier()      // hoặc jogDistance nếu có biến
);
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

