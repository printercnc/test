

#include <cstring>  // Để dùng strlen()
#include "Keyboard.h"
#include "DisplayManager.h"
#include "MenuController.h"
#include "Status.h"
#include "encoder.h"
#include <U8g2lib.h>
#include <Arduino.h>  // quan trọng cho Arduino framework
#include "printer_status.h"

// UART handler STM32 (ví dụ USART2)
UART_HandleTypeDef huart2;

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
// Biến trạng thái chung
static bool uartConnected = false;
// Biến xác định đã chuyển về page mặc định sau khi connect lần đầu chưa
static bool hasSetDefaultPage = false;

// Khởi tạo bàn phím theo chân pin (thay thế chân đúng phần cứng)
const uint8_t rowPins[4] = {PB0, PB1, PB3, PB4};
const uint8_t colPins[4] = {PA2, PA3, PB5, PA8};
Keyboard keyboard(rowPins, 4, colPins, 4);

// =================== Một số biến hỗ trợ ==================
float g54Offsets[AXIS_COUNT] = {0};
float g55Offsets[AXIS_COUNT] = {0};
uint32_t lastEncoderValue = 0;

void My_Error_Handler(void)
{
  // hiển thị hoặc debug, tạm vào vòng lặp vô hạn
  while(1) {}
}      // hàm xử lý lỗi

// Khởi tạo UART2 để giao tiếp 115200 bps
void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
      My_Error_Handler();
       return;
    }
}

// Hàm gửi chuỗi lệnh qua UART
void SendUARTCommand(const char* cmd)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)cmd, strlen(cmd), 100);
}

// Hàm nhận trạng thái từ Marlin qua UART, parse dữ liệu (ví dụ đơn giản)
bool UARTReceiveStatus(PrinterStatus* status){  

     if (!status)
    return false;
    uint8_t buffer[128];
    int len = HAL_UART_Receive(&huart2, buffer, sizeof(buffer) - 1, 20);
    if (len > 0)
    {
        buffer[len] = 0;

            int extruderType;
if (sscanf((char*)buffer, "EXTRUDER:%d", &extruderType) == 1) {
    status->extruderType = extruderType;
}

float x = status->position[0];
float y = status->position[1];
float z = status->position[2];
float e = status->position[5];
int matched = sscanf((char*)buffer, "X:%f Y:%f Z:%f E:%f", &x, &y, &z, &e);
if (matched >= 3) {
    status->position[0] = x;
    status->position[1] = y;
    status->position[2] = z;
    status->position[3] = 0.0f; // nếu không dùng
    status->position[4] = 0.0f;
    status->position[5] = e;

     return true;
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
}

 // Thêm hàm setup() và loop() bắt buộc trong Arduino framework
void setup()
{
    // Khởi tạo HAL
    HAL_Init(); 
    SystemClock_Config(); // Khởi tạo Clock (Bạn sẽ cần thực hiện hàm này ở đâu đó)   
    MX_USART2_UART_Init(); // Khởi tạo UART
    u8g2.begin();          // Quan trọng: khởi tạo màn hình
    memset(&latestStatus, 0, sizeof(latestStatus)); // Khởi tạo trạng thái ban đầu
    uartConnected = false;
    Serial.begin(115200);
    menuController.updateConnectionStatus(false); // Set loại máy dựa trên macro từ configuration.h

}

// Vòng lặp chính
void loop(){
    
     // 1. Nhận status từ UART
    if (UARTReceiveStatus(&latestStatus)) {
        menuController.updateConnectionStatus(true);
        uartConnected = true;
    } else {
        uartConnected = false;
    }

    // 2. Nếu vừa mới kết nối, setup page mặc định
    static bool lastConnected = false;
    if (uartConnected && !lastConnected) {
        // Vừa có kết nối lại
        hasSetDefaultPage = false;
    }
    lastConnected = uartConnected;

    // 3. Nếu có kết nối và chưa set page mặc định
    if (uartConnected && !hasSetDefaultPage) {
        // ---- Giả sử Marlin trả về latestStatus.extruderType
        // extruderType = 1: máy in 3d (EXTRUDER=1);  = 0: CNC (EXTRUDER=0)
        if (latestStatus.extruderType == 0) {
            menuController.setCurrentPage(1); // CNC
        } else if (latestStatus.extruderType == 1) {
            menuController.setCurrentPage(2); // 3D printer
        } else {
            menuController.setCurrentPage(0); // Hiện warning nếu không rõ trạng thái
        }
        hasSetDefaultPage = true;
    }

     // 4. Xử lý phím, cho phép chuyển trang kể cả khi mất UART
    char key = keyboard.scanKeyboard();
    if (key != 0) {
        menuController.handleKey(key);
    }
    // 5. Hiển thị trang theo currentPage
    int curPage = menuController.getCurrentPage();

    switch(curPage) {
        case 0:
            displayManager.drawWarningPage();
            break;
        case 1:
            // Trang giao diện CNC
            if (!uartConnected) {
                // Có thể hiển thị thông báo ở đầu trang CNC: "Chưa kết nối máy"
                displayManager.drawCNCPage_NeedConnection();
            } else {
                displayManager.drawG54Page(g54Offsets);
                // Bạn thêm các phần cập nhật status CNC nếu cần
            }
            break;

        case 2:
            // Trang giao diện máy in 3D
            if (!uartConnected) {
                // Tương tự, cảnh báo thiếu kết nối trong giao diện 3D
                displayManager.draw3DPage_NeedConnection();
            } else {
                displayManager.drawPrinter3DPage(&latestStatus);
                // Và các phần riêng của máy in 3D
            }
            break;

        case 3:
            // Trang Jog/Home status (cả 2 máy)
            if (!uartConnected) {
                displayManager.drawJogPage_NeedConnection();
            } else {
                displayManager.setJogAxis(menuController.getSelectedAxis());
                displayManager.setJogDistance(menuController.getCurrentJogMultiplier());
                displayManager.drawHomeStatusScreen(&latestStatus);
            }
            break;

        // Thêm các trang khác nếu có nhu cầu

        default:
            displayManager.drawWarningPage();
            break;
    }


    delay(10); // tránh chiếm CPU quá nhiều
}