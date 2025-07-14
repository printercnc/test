

#include <cstring>  // Để dùng strlen()
#include "Keyboard.h"
#include "DisplayManager.h"
#include "MenuController.h"
#include "Status.h"
#include "encoder.h"
#include <U8g2lib.h>
#include <Arduino.h>  // quan trọng cho Arduino framework

// UART handler STM32 (ví dụ USART2)
UART_HandleTypeDef huart2;

// Timer handler để đọc Encoder
extern TIM_HandleTypeDef htim2;

// Các chân hiển thị SPI ST7920 - giữ nguyên theo phần cứng của bạn
#define PIN_CLK PA5
#define PIN_DATA PA7
#define PIN_CS PA4
#define PIN_RST U8X8_PIN_NONE

// Khởi tạo màn hình
U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, PIN_CLK, PIN_DATA, PIN_CS, PIN_RST);
DisplayManager displayManager(u8g2);

// Khởi tạo bàn phím theo chân pin (thay thế chân đúng phần cứng)
const uint8_t rowPins[4] = {PB0, PB1, PB3, PB4};
const uint8_t colPins[4] = {PA2, PA3, PB5, PA8};
Keyboard keyboard(rowPins, 4, colPins, 4);

// Menu điều khiển trạng thái giao diện
MenuController menuController;

// Biến trạng thái chung
static bool uartConnected = false;

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
bool UARTReceiveStatus(PrinterStatus* status)
{   if (status == nullptr) 
    return false;
    uint8_t buffer[128];
    int len = HAL_UART_Receive(&huart2, buffer, sizeof(buffer) - 1, 20);
    if (len > 0)
    {
        buffer[len] = 0;

float x = 0, y = 0, z = 0, e = 0;
int matched = sscanf((char*)buffer, "X:%f Y:%f Z:%f E:%f", &x, &y, &z, &e);
if (matched >= 3) {
    for (int i = 0; i < AXIS_COUNT; i++) {
                status->position[i] = 0.0f;
            }
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
    memset(&latestStatus, 0, sizeof(latestStatus)); // Khởi tạo trạng thái ban đầu

    uartConnected = false;
}

// Vòng lặp chính
void loop()
{
    // Kiểm tra UART có nhận được dữ liệu không
    if (UARTReceiveStatus(&latestStatus)) {
        uartConnected = true;
    }
    else {
        uartConnected = false;
    }

    // Đọc và xử lý bàn phím
    char key = keyboard.scanKeyboard();
    if (key != 0) {
        menuController.handleKey(key);
    }

    delay(10); // tránh chiếm CPU quá nhiều
}