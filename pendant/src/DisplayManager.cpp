
//DisplayManager.cpp

#include "DisplayManager.h"
#
#include <stdio.h>    // snprintf
extern U8G2_ST7920_128X64_F_SW_SPI u8g2;  // extern display instance từ main hoặc nơi khác
#include <Arduino.h> // Để sử dụng Serial
DisplayManager::DisplayManager(U8G2_ST7920_128X64_F_SW_SPI& display)
    : u8g2(display)
{ }

// Hàm chuyển float -> chuỗi dạng "123.456" có 3 số thập phân
void dtostr52_buf(char *buf, size_t size, float val) {
  int whole = (int)val;
  int frac = (int)((val - whole) * 1000);
  if (frac < 0) frac = -frac;
  snprintf(buf, size, "%d.%03d", whole, frac);
}

float stepperAccel[AXIS_COUNT];  // max acceleration
float stepperMaxSpeed[AXIS_COUNT]; // max speed
float current_position[AXIS_COUNT] = {0};
float g54Offsets[AXIS_COUNT] = {0};
const char axesNames[AXIS_COUNT] = {'X', 'Y', 'Z', 'A', 'C', 'E'};

// ====== PAGE_WARNING =====================
void DisplayManager::drawWarningPage() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(0, 20, "Waiting for cnc...");
  u8g2.sendBuffer();
}

// ====== PAGE_G54 =====================
void DisplayManager::drawG54Page_FromMarlin_CurrentPos() {
    u8g2.clearBuffer();

    // Tiêu đề
    u8g2.setFont(u8g2_font_6x12_tr); // Chọn font phù hợp với màn hình
    u8g2.setCursor(0, 12);
    u8g2.print("G54 Offset (Marlin):");

    // Hiển thị từng trục, căn chỉnh dòng cho đẹp
    int lineHeight = 10;   // Độ cao mỗi dòng (tuỳ font + khoảng cách)
    int x = 0;
    int y = 22;
    for(int i=0; i<5; i++) {
        u8g2.setCursor(x, y + i*lineHeight);
        u8g2.print(axes[i]);
        u8g2.print(": ");
        u8g2.print(current_position[i], 3);
    }

    u8g2.sendBuffer();
}
// ====== PAGE_PARAMETER =====================
void DisplayManager::drawParameterPage(int selectedParamIdx) {
  u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x12_tr);
    u8g2.drawStr(0, 12, "Stepper Parameters:");

         char buf[32];                // KHai báo buf ở đây
         int y = 30;                  // Khai báo y tại đây
  int totalParams = AXIS_COUNT * 2; // mỗi trục 2 param: accel và speed
    for (int i = 0; i < totalParams; i++) {
        if (i == selectedParamIdx) {
            u8g2.drawBox(0, y - 10, 128, 12);
            u8g2.setDrawColor(0);  // In đảo màu chữ nền đen
        } else {
            u8g2.setDrawColor(1);  // Màu chữ bình thường
        }
    int axisIndex = i / 2;
        bool isAccel = (i % 2 == 0);
        if (isAccel) {
            snprintf(buf, sizeof(buf), "%c Accel: %.1f", axesNames[axisIndex], stepperAccel[axisIndex]);
        } else {
            snprintf(buf, sizeof(buf), "%c MaxSpd: %.1f", axesNames[axisIndex], stepperMaxSpeed[axisIndex]);
        }

        u8g2.drawStr(2, y, buf);
        y += 12;
    }
    u8g2.setDrawColor(1); // Reset màu chữ
    u8g2.sendBuffer();
}
void DisplayManager::updateParameterValue(int paramIdx, float delta) {
    int axisIndex = paramIdx / 2;
    bool isAccel = (paramIdx % 2 == 0);

    if (isAccel) {
        stepperAccel[axisIndex] += delta;
        if (stepperAccel[axisIndex] < 0) stepperAccel[axisIndex] = 0;

        // Gửi lệnh cập nhật acceleration cho trục axisIndex tới Marlin
        // Ví dụ gửi: M204 T<value> hoặc tương tự, bạn tùy chỉnh theo firmware
        char cmd[32];
        snprintf(cmd, sizeof(cmd), "M204 %c%.1f", axesNames[axisIndex], stepperAccel[axisIndex]);
        Serial1.println(cmd);

    } else {
        stepperMaxSpeed[axisIndex] += delta;
        if (stepperMaxSpeed[axisIndex] < 0) stepperMaxSpeed[axisIndex] = 0;

        // Gửi lệnh cập nhật max speed cho trục axisIndex tới Marlin
        // Ví dụ: M203 X10.0 Y20.0 Z30.0 (cập nhật từng trục trên trục)
        // Ở đây ta gửi đơn cho trục này:
        char cmd[32];
        snprintf(cmd, sizeof(cmd), "M203 %c%.1f", axesNames[axisIndex], stepperMaxSpeed[axisIndex]);
        Serial1.println(cmd);
    }
}

// ====== PAGE_HOME_STATUS =====================
const char* homeStatusMenu[] = {"HOME-ALL", "JOG-AXIS"};
const int homeStatusMenuCount = sizeof(homeStatusMenu)/sizeof(homeStatusMenu[0]);

// Các trục để chọn jog, có thể mở rộng nếu cần
const char jogAxesList[] = {'X', 'Y', 'Z', 'A', 'C', 'E'};
constexpr int jogAxesCount = sizeof(jogAxesList) / sizeof(jogAxesList[0]);

char jogSelectedAxis = 'X';   // Trục jog đang chọn
float jogDistance = 1.0f;     // Khoảng cách di chuyển mỗi bước jog, bạn có thể thay đổi

// Hàm vẽ trang HOME_STATUS
void DisplayManager::drawHomeStatusPage(int selectedIdx, char jogSelectedAxis, float jogMultiplier) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_5x8_tr);
    // Vẽ tiêu đề
    u8g2.drawStr(0, 11, "Home / Jog Status:");
    int lineHeight = 9;          // Chiều cao mỗi dòng (có thể thử 8 hoặc 9)
    int menuStartY = 20;         // Tọa độ y bắt đầu menu
    for (int i = 0; i < homeStatusMenuCount; i++) {
        int y = menuStartY + i * lineHeight;
        if (i == selectedIdx) {
             u8g2.drawBox(0, y - 7, 128, lineHeight);
            u8g2.setDrawColor(0);
        }
        u8g2.drawStr(12, y, homeStatusMenu[i]);
        u8g2.setDrawColor(1);
    }
    // Nếu chọn JOG-AXIS, hiện danh sách trục bên dưới để chọn
    if (selectedIdx == 1) {  // JOG-AXIS selected
        int axisStartY = menuStartY + homeStatusMenuCount * lineHeight + 3;
        u8g2.setFont(u8g2_font_5x8_tr); // Font nhỏ cho phần trục
        u8g2.drawStr(0, axisStartY, "Select Axis:");

        int x = 0;
        int axisY = axisStartY + 10;   // Dòng kế tiếp
        for (int i = 0; i < jogAxesCount; i++) {
    char axisName[2] = {jogAxesList[i], 0};
    // **** Đổi: dùng menuController.getSelectedAxis()! ***
    if(jogSelectedAxis == jogAxesList[i]) {
        u8g2.setDrawColor(1);
        u8g2.drawBox(x-1, axisY-7, 10, 9); // bôi font
        u8g2.setDrawColor(0);
        u8g2.drawStr(x, axisY, axisName);
        u8g2.setDrawColor(1);
    } else {
        u8g2.drawStr(x, axisY, axisName);
    }
    x += 12;   // Khoảng cách giữa các trục
            // Nếu quá chiều rộng, xuống dòng tiếp (cho chắc chắn không tràn màn)
            if (x > 120) { // 5 trục thì không cần, nhưng để an toàn nếu sau này mở rộng
                x = 0;
                axisY += 10;
            }
        }
        axisY += 12;
       char floatBuf[10];
dtostrf(jogMultiplier, 4, 2, floatBuf);  // width=4, decimals=2

char stepStr[24];
snprintf(stepStr, sizeof(stepStr), "Step: %s mm", floatBuf);

u8g2.drawStr(0, axisY, stepStr);
    }

    u8g2.sendBuffer();
}

void DisplayManager::drawCNCPage_NeedConnection() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 10, "CNC: No Connection");
    u8g2.sendBuffer();
}

void DisplayManager::draw3DPage_NeedConnection() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 10, "3D Printer: No Conn.");
    u8g2.sendBuffer();
}

void DisplayManager::drawJogPage_NeedConnection() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 10, "Jog: No Connection");
    u8g2.sendBuffer();
}
// ====== PAGE_PRINTER3D - Hiển thị trạng thái trục máy 3D =====================
void DisplayManager::draw3DPage_WithConnected() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 10, "3D Printer: Connected");
    // Ví dụ hiển thị nhiệt độ 2 hotend và nhiệt độ bàn in (temp)
    char buf[32];
    // Giả sử có biến nhiệt độ, thay số 200, 60, ... bằng biến thực tế của bạn
    float nozzleTemp = 200.0f;  // Nhiệt độ vòi phun
    //float bedTemp = 60.0f;      // Nhiệt độ bàn in
    float progress = 45.0f;     // Tiến trình in (%)
    snprintf(buf, sizeof(buf), "Nozzle: %.1f C", nozzleTemp);
    u8g2.drawStr(0, 30, buf);
    //snprintf(buf, sizeof(buf), "Bed: %.1f C", bedTemp);
    //u8g2.drawStr(0, 45, buf);
    snprintf(buf, sizeof(buf), "Progress: %.0f%%", progress);
    u8g2.drawStr(0, 60, buf);
    u8g2.sendBuffer();
}
