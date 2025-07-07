//display manager.h


#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <U8g2lib.h>
#include <string.h> // cho memcpy
#include "printer_status.h"

extern PrinterStatus printerStatus;      // extern declaration
extern const char axesNames[AXIS_COUNT]; // extern declaration

enum Page {
  PAGE_WARNING = 0,
  PAGE_G54     = 1,
  PAGE_G55     = 2,
  PAGE_MACHINE = 3,
  PAGE_PARAMETER = 4,
  PAGE_HOME_STATUS = 5,
  PAGE_COUNT      = 7,
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

    void drawG54Page(const float offsets[AXIS_COUNT]);
    void drawG55Page(const float offsets[AXIS_COUNT]);

    void drawStatusScreen(const PrinterStatus* status);
    void drawParameterPage(int selectedParamIdx);
    void drawMachineControlPage(int selectedMenuIndex, char selectedAxis, float currentPosition, const PrinterStatus* status);
    void drawHomeStatusScreen(const PrinterStatus* status);

    float getParameterValue(int index) const;
    void setParameterValue(int index, float val);
    // ... khai báo thêm tương ứng

private:
    U8G2_ST7920_128X64_F_SW_SPI& u8g2;
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