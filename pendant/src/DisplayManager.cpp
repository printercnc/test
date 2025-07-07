// DisplayManager.cpp

#include "DisplayManager.h"

#include <stdio.h>    // cho snprintf
#include "printer_status.h"
#include "PrinterI2C.h"

// STM32 HAL I2C dùng cho master
#include "stm32f1xx_hal.h"

// VÍ DỤ Class DisplayManager (dùng U8G2)
//------------------------------------------------------
#include <U8g2lib.h>
extern U8G2_ST7920_128X64_F_SW_SPI u8g2;  // hoặc extern biến được định nghĩa bên ngoài
DisplayManager::DisplayManager(U8G2_ST7920_128X64_F_SW_SPI& display)
    : u8g2(display)
{
    // (tùy ý: thêm gì cũng được)
}
const char axesNames[AXIS_COUNT] = {'X', 'Y', 'Z', 'A', 'C', 'E'};
PrinterStatus printerStatus;
// Hàm chuyển đổi float -> chuỗi với 3 số thập phân
void dtostr52_buf(char *buf, size_t size, float val) {
  int whole = (int)val;
  int frac = (int)((val - whole) * 1000);
  if (frac < 0) frac = -frac;
  snprintf(buf, size, "%d.%03d", whole, frac);
}

void ConnectionStatusDisplay::draw(bool connected) {
  if (!connected) {
    unsigned long now = millis();

    if (now - lastToggleMS > blinkInterval) {
      lastToggleMS = now;
      visible = !visible;  // Nhấp nháy bật/tắt
    }

    if (visible) {
      u8g2_ref.setFont(u8g2_font_6x12_tr);
      u8g2_ref.drawStr(0, 12, "Waiting for Marlin");
    }
    // Lưu ý: không gọi clearBuffer hoặc sendBuffer trong hàm này (giao diện chính gọi)
  } else {
    // Đã kết nối => không hiển thị cảnh báo
  }
}

void DisplayManager::drawG54Page(const float offsets[AXIS_COUNT]) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(0, 12, "G54 Offsets:");
  
  char buf[40];
  for (int i = 0; i < AXIS_COUNT; i++) {
    snprintf(buf, sizeof(buf), "%c: %.3f", axesNames[i], offsets[i]);
    u8g2.setCursor(0, 25 + i * 12);
    u8g2.print(buf);
  }

  u8g2.sendBuffer();
}

void DisplayManager::drawG55Page(const float offsets[AXIS_COUNT]) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(0, 12, "G55 Offsets:");
  
  char buf[40];
  for (int i = 0; i < AXIS_COUNT; i++) {
    snprintf(buf, sizeof(buf), "%c: %.3f", axesNames[i], offsets[i]);
    u8g2.setCursor(0, 25 + i * 12);
    u8g2.print(buf);
  }

  u8g2.sendBuffer();
}

void DisplayManager::drawStatusScreen(const PrinterStatus* status) {
  char buf[32];
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);

  snprintf(buf, sizeof(buf), "State: %u", status->state);
  u8g2.drawStr(0, 12, buf);

  for (int i = 0; i < 3; i++) {
    snprintf(buf, sizeof(buf), "%c: %.2f", 'X'+i, status->position[i]);
    u8g2.drawStr(0, 24 + i * 12, buf);
  }

  u8g2.sendBuffer();
}

void DisplayManager::drawParameterPage(int selectedParamIdx) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(0, 12, "Parameters:");

  char buf[32];
  int y = 30;
  for (int i = 0; i < PARAM_COUNT; i++) {
    if (i == selectedParamIdx) {
      u8g2.drawBox(0, y - 10, 128, 12);
      u8g2.setDrawColor(0);
    } else {
      u8g2.setDrawColor(1);
    }
    snprintf(buf, sizeof(buf), "%s: %.1f", params[i].name, params[i].value);
    u8g2.drawStr(2, y, buf);
    y += 14;
  }
  u8g2.setDrawColor(1);
  u8g2.sendBuffer();
}

// Định nghĩa hàm drawMachineControlPage (không chứa drawHomeStatusScreen)
void DisplayManager::drawMachineControlPage(int selectedMenuIndex, char selectedAxis, float currentPosition, const PrinterStatus* status) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(0, 12, "Machine Control:");

  // Hiển thị toạ độ các trục từ PrinterStatus truyền vào
  char buf[32];
  int y = 28;
  for (int i = 0; i < AXIS_COUNT; i++) {
    snprintf(buf, sizeof(buf), "%c: %.3f", axesNames[i], status ? status->position[i] : 0.0f);
    u8g2.drawStr(0, y, buf);
    y += 12;
  }

  u8g2.sendBuffer();
}
// Định nghĩa hàm drawHomeStatusScreen tách riêng, bên ngoài bất kỳ hàm nào khác
void DisplayManager::drawHomeStatusScreen(const PrinterStatus* status) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);

  char buf[16];
  for (int i = 0; i < AXIS_COUNT; i++) {
    dtostr52_buf(buf, sizeof(buf), status->position[i]);
    char line[32];
    snprintf(line, sizeof(line), "%c: %s", axesNames[i], buf);
    u8g2.setCursor(0, 25 + i * 10);
    u8g2.print(line);
  }

  const char* state_text = "Unknown";
  switch (status->state) {
    case 0: state_text = "Idle"; break;
    case 1: state_text = "Printing"; break;
  }
  u8g2.setCursor(0, 25 + AXIS_COUNT * 10);
  u8g2.print("State: ");
  u8g2.print(state_text);

  uint32_t seconds = status->elapsed_seconds;
  uint32_t hours = seconds / 3600;
  uint32_t minutes = (seconds % 3600) / 60;
  snprintf(buf, sizeof(buf), "Time: %u:%02u", hours, minutes);
  u8g2.setCursor(0, 25 + (AXIS_COUNT + 1) * 10);
  u8g2.print(buf);

  int xPos = 0;
  int yPos = 25 + (AXIS_COUNT + 2) * 10;
  for (int e = 0; e < EXTRUDERS; e++) {
    snprintf(buf, sizeof(buf), "E%d: %.1f/%.1f", e + 1, status->hotend_temps[e], status->hotend_targets[e]);
    u8g2.setCursor(xPos, yPos);
    u8g2.print(buf);
    xPos += u8g2.getStrWidth(buf);
  }
  yPos += 10;
  u8g2.setCursor(0, yPos);
  snprintf(buf, sizeof(buf), "Bed: %.1f/%.1f", status->bed_temp, status->bed_target);
  u8g2.print(buf);

  yPos += 10;
  u8g2.setCursor(0, yPos);
  snprintf(buf, sizeof(buf), "Feedrate: %d%%", status->feedrate_percentage);
  u8g2.print(buf);

  u8g2.sendBuffer();
}

float DisplayManager::getParameterValue(int index) const {
  if (index < 0 || index >= PARAM_COUNT) return 0;
  return params[index].value;
}

void DisplayManager::setParameterValue(int index, float val) {
  if (index < 0 || index >= PARAM_COUNT) return;

  if (val < params[index].minVal)
    val = params[index].minVal;
  if (val > params[index].maxVal)
    val = params[index].maxVal;

  params[index].value = val;
}

