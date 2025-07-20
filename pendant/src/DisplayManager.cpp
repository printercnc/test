
//DisplayManager.cpp

#include "DisplayManager.h"
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
const char axesNames[AXIS_COUNT] = {'X', 'Y', 'Z', 'A', 'C', 'E'};

// ====== PAGE_WARNING =====================
void DisplayManager::drawWarningPage() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(0, 20, "Waiting for cnc...");
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


void DisplayManager::sendHomeCommand() {
    // Ví dụ gửi lệnh homing chuẩn G-code:
    Serial1.println("G28");  // Lệnh homing - bạn có thể thay bằng lệnh phù hợp với máy của bạn
}

void DisplayManager::sendJogCommand(char axis, float dist) {
    // Ví dụ gửi lệnh jog theo trục và khoảng cách:
    Serial1.print("G91\n");  // Chế độ tương đối
    Serial1.print("G1 ");
    Serial1.print(axis);
    Serial1.print(dist);
    Serial1.println(" F1000");  // Tốc độ feed rate ví dụ
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

void DisplayManager::draw3DPage_WithConnected() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(0, 10, "3D Printer: Connected");

    // Ví dụ hiển thị nhiệt độ 2 hotend và nhiệt độ bàn in (temp)
    char buf[32];
    // Giả sử có biến nhiệt độ, thay số 200, 60, ... bằng biến thực tế của bạn
    float nozzleTemp = 200.0f;  // Nhiệt độ vòi phun
    float bedTemp = 60.0f;      // Nhiệt độ bàn in
    float progress = 45.0f;     // Tiến trình in (%)
    
    snprintf(buf, sizeof(buf), "Nozzle: %.1f C", nozzleTemp);
    u8g2.drawStr(0, 30, buf);

    snprintf(buf, sizeof(buf), "Bed: %.1f C", bedTemp);
    u8g2.drawStr(0, 45, buf);

    snprintf(buf, sizeof(buf), "Progress: %.0f%%", progress);
    u8g2.drawStr(0, 60, buf);

    u8g2.sendBuffer();
}
