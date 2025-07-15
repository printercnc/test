//display manager.h


#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include "printer_status.h"
#include <U8g2lib.h>
#include <string.h> // cho memcpy


enum Page {
  PAGE_WARNING = 0,
  PAGE_G54 = 1,
  PAGE_PRINTER3D = 2,
  PAGE_PARAMETER = 3,
  PAGE_HOME_STATUS = 4,
  PAGE_COUNT = 5
};

struct Parameter {
  const char* name;
  float value;
  float minVal;
  float maxVal;
  float step;
};

#define PARAM_COUNT 3
#define AXIS_COUNT 6

class DisplayManager {
public:
    // constructor
    DisplayManager(U8G2_ST7920_128X64_F_SW_SPI& display);

    void drawHomeStatusScreen(const PrinterStatus* status);
    void drawPrinter3DPage(const PrinterStatus* status);
      void drawCNCPage_NeedConnection();
    void draw3DPage_NeedConnection();
    void drawJogPage_NeedConnection();
    void drawWarningPage();

    void sendHomeCommand();
    void sendJogCommand(char axis, float distance);

    void setJogAxis(char axis);
    char getJogAxis() const;

    void setJogDistance(float dist);
    float getJogDistance() const;

    void drawG54Page(const float offsets[AXIS_COUNT]);
    void drawG55Page(const float offsets[AXIS_COUNT]);

    void drawStatusScreen(const PrinterStatus* status);
    void drawParameterPage(int selectedParamIdx);
    void drawMachineControlPage(int selectedMenuIndex, char selectedAxis, float currentPosition, const PrinterStatus* status);

    float getParameterValue(int index) const;
    void setParameterValue(int index, float val);
    // ... khai báo thêm tương ứng

private:
    U8G2_ST7920_128X64_F_SW_SPI& u8g2;

    // Biến lưu trữ trục jog hiện tại (mặc định 'X')
    char jogAxis = 'X';

    // Bước di chuyển jog, ví dụ 0.1mm, 1mm, 10mm tùy chỉnh
    float jogDistance = 1.0f;

    // các member khác...
  float g54_offsets[AXIS_COUNT];
  float g55_offsets[AXIS_COUNT];

  const char axes[AXIS_COUNT] = {'X', 'Y', 'Z', 'A', 'C', 'E'};

  Parameter params[PARAM_COUNT] = {
    {"Step Spd", 5000, 1000, 10000, 100},
    {"Accel", 1500, 100, 3000, 50},
    {"Max Spd", 3000, 500, 5000, 100}
  };
};
 class ConnectionStatusDisplay {
public:
  ConnectionStatusDisplay(U8G2_ST7920_128X64_F_SW_SPI& u8g2);
  void draw(bool connected);              // Thêm khai báo hàm draw

private:
  U8G2_ST7920_128X64_F_SW_SPI& u8g2_ref;

  unsigned long lastToggleMS = 0;
  const unsigned long blinkInterval = 500;
   bool visible = false;
  
};

#endif