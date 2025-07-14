

#include "DisplayManager.h"
#include "printer_status.h"
#include "Status.h"
#include <stdio.h>    // snprintf

extern U8G2_ST7920_128X64_F_SW_SPI u8g2;  // extern display instance từ main hoặc nơi khác
extern UART_HandleTypeDef huart2;
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

const char axesNames[AXIS_COUNT] = {'X', 'Y', 'Z', 'A', 'C', 'E'};
PrinterStatus printerStatus;

// ====== PAGE_WARNING =====================
void DisplayManager::drawWarningPage() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(0, 20, "Waiting for Marlin...");
  u8g2.sendBuffer();
}

// ====== PAGE_G54 =====================
void DisplayManager::drawG54Page(const float offsets[AXIS_COUNT]) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(0, 12, "G54 Offsets:");
  
  char buf[40];
  // Hiển thị chỉ X, Y, Z, A, C (5 trục)
  for (int i = 0; i < 5; i++) {
    snprintf(buf, sizeof(buf), "%c: %.3f", axesNames[i], offsets[i]);
    u8g2.setCursor(0, 25 + i * 12);
    u8g2.print(buf);
  }
  u8g2.sendBuffer();
}

// ====== PAGE_PRINTER3D - Hiển thị trạng thái trục máy 3D =====================
void DisplayManager::drawPrinter3DPage(const PrinterStatus* status) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(0, 12, "3D Printer Status:");

  char buf[32];
// Hiện thực các trục X,Y,Z,A,C,E đầy đủ
for (int i = 0; i < AXIS_COUNT; i++) {
  snprintf(buf, sizeof(buf), "%c: %.3f", axesNames[i], status ? status->position[i] : 0.0f);
  u8g2.drawStr(0, 25 + i * 12, buf);
}

u8g2.sendBuffer();
// Hiển thị trạng thái in
char statusBuf[32];
snprintf(statusBuf, sizeof(statusBuf), "State: %u", status ? status->state : 0);
u8g2.drawStr(0, 25 + AXIS_COUNT * 12, statusBuf);

// Ví dụ hiển thị nhiệt độ Extruder & Bed (nếu có trong PrinterStatus)
if(status) {
  int yPos = 25 + (AXIS_COUNT + 1) * 12;
  for (int e = 0; e < EXTRUDERS; e++) {
    snprintf(buf, sizeof(buf), "E%d: %.1f/%.1f", e + 1, status->hotend_temps[e], status->hotend_targets[e]);
    u8g2.drawStr(0, yPos, buf);
    yPos += 12;
  }
  snprintf(buf, sizeof(buf), "Bed: %.1f/%.1f", status->bed_temp, status->bed_target);
  u8g2.drawStr(0, yPos, buf);
  yPos += 12;

  // Feedrate
  snprintf(buf, sizeof(buf), "Feedrate: %d%%", status->feedrate_percentage);
  u8g2.drawStr(0, yPos, buf);
}
u8g2.sendBuffer();
}

// ====== PAGE_PARAMETER =====================
void DisplayManager::drawParameterPage(int selectedParamIdx) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(0, 12, "Parameters:");

  char buf[32];
  int y = 30;
  for (int i = 0; i < PARAM_COUNT; i++) {
    if (i == selectedParamIdx) {
      u8g2.drawBox(0, y - 10, 128, 12);
      u8g2.setDrawColor(0);  // In đảo màu chữ nền đen
    } else {
      u8g2.setDrawColor(1);  // Màu chữ bình thường
    }
    snprintf(buf, sizeof(buf), "%s: %.1f", params[i].name, params[i].value);
    u8g2.drawStr(2, y, buf);
    y += 14;
  }
  u8g2.setDrawColor(1); // Reset màu chữ
  u8g2.sendBuffer();
}

// ====== PAGE_HOME_STATUS =====================
void DisplayManager::drawHomeStatusScreen(const PrinterStatus* status) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);
// Hiển thị title
    u8g2.drawStr(0, 12, "Home & Jog Control");

  char buf[32];

    // Hiển thị nút HOME (giả lập kiểu button)
    u8g2.drawFrame(0, 16, 40, 14);
    u8g2.drawStr(8, 28, "HOME");

    // Hiển thị trục jog hiện tại
    snprintf(buf, sizeof(buf), "Jog Axis: %c", jogAxis);
    u8g2.drawStr(50, 28, buf);

  if(status) {
  // Tìm index trục jog trong mảng axesNames
        int idx = -1;
        for (int i = 0; i < AXIS_COUNT; i++) {
            if (axesNames[i] == jogAxis) {
                idx = i;
                break;
            }
        }

        if (idx >= 0) {
            char posBuf[32];
            snprintf(posBuf, sizeof(posBuf), "Pos: %.3f", status->position[idx]);
            u8g2.drawStr(0, 44, posBuf);
        }
    }
    // Hiển thị bước jog hiện tại
    snprintf(buf, sizeof(buf), "Jog Step: %.3f", jogDistance);
    u8g2.drawStr(80, 44, buf);

    // Hiển thị hướng để jog (+/-)
    u8g2.drawStr(0, 60, "Turn encoder to jog");
    u8g2.drawStr(0, 74, "Press HOME to home");

    // Gửi buffer ra màn hình
    u8g2.sendBuffer();
}

   void DisplayManager::sendHomeCommand() {
    const char* cmd = "G28\n";
    // giả sử có huart2 extern hoặc truyền handler
    HAL_UART_Transmit(&huart2, (uint8_t*)cmd, strlen(cmd), 100);
}

void DisplayManager::sendJogCommand(char axis, float distance) {
    char cmd[32];
    // Lệnh G91 cho chế độ relatve, rồi di chuyển và về lại absolutes
    snprintf(cmd, sizeof(cmd), "G91\nG0 %c%.3f\nG90\n", axis, distance);
    HAL_UART_Transmit(&huart2, (uint8_t*)cmd, strlen(cmd), 100);
}

void DisplayManager::setJogAxis(char axis) {
    jogAxis = axis;
}

char DisplayManager::getJogAxis() const {
    return jogAxis;
}

void DisplayManager::setJogDistance(float dist) {
    jogDistance = dist;
}

float DisplayManager::getJogDistance() const {
    return jogDistance;
} 