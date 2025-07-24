
//display manager.h

#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

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

#define AXIS_COUNT 6
// Dữ liệu vị trí tọa độ hiện tại, dùng để hiển thị hoặc cập nhật từ UART
extern float stepperAccel[AXIS_COUNT];
extern float stepperMaxSpeed[AXIS_COUNT];
extern float current_position[AXIS_COUNT];
extern float g54Offsets[AXIS_COUNT];
extern const char axesNames[AXIS_COUNT];
extern const char* homeStatusMenu[];
extern const int homeStatusMenuCount;

class DisplayManager {
public:
    // constructor
    DisplayManager(U8G2_ST7920_128X64_F_SW_SPI& display);
    // Các hàm vẽ trang
    void drawCNCPage_NeedConnection();
    void draw3DPage_NeedConnection();
    void drawJogPage_NeedConnection();
    void drawWarningPage();
    void draw3DPage_WithConnected();

    void drawG54Page_FromMarlin_CurrentPos();
    void updateParameterValue(int paramIdx, float delta);

    void drawHomeStatusPage(int selectedIdx, int menuCount);
    void drawParameterPage(int parameterSelectedIdx);
    void drawMachineControlPage(int selectedMenuIndex, char selectedAxis, float currentPosition);

    // Chọn trục jog từ bàn phím (vd: press '1' là chọn trục X ...)
    void selectJogAxis(char axisKey);
    void setJogDistance(float dist);

    // Lấy/đặt giá trị param
    float getParameterValue(int index) const;
    void setParameterValue(int index, float val);

private:
    U8G2_ST7920_128X64_F_SW_SPI& u8g2;

    char jogAxis = 'X';
    float jogDistance = 1.0f;

    char jogSelectedAxis = 'X';

  const char axes[AXIS_COUNT] = {'X', 'Y', 'Z', 'A', 'C', 'E'};

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