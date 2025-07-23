
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
    int lineHeight = 12;   // Độ cao mỗi dòng (tuỳ font + khoảng cách)
    int x = 0;
    int y = 24;
    u8g2.setCursor(x, y);
    u8g2.print("X: ");
    u8g2.print(current_position[0], 3);

    y += lineHeight;
    u8g2.setCursor(x, y);
    u8g2.print("Y: ");
    u8g2.print(current_position[1], 3);

    y += lineHeight;
    u8g2.setCursor(x, y);
    u8g2.print("Z: ");
    u8g2.print(current_position[2], 3);

    y += lineHeight;
    u8g2.setCursor(x, y);
    u8g2.print("A: ");
    u8g2.print(current_position[3], 3);

    y += lineHeight;
    u8g2.setCursor(x, y);
    u8g2.print("C: ");
    u8g2.print(current_position[4], 3);
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
        y += 14;
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
const char jogAxesList[] = {'X', 'Y', 'Z', 'A', 'C'};
constexpr int jogAxesCount = sizeof(jogAxesList) / sizeof(jogAxesList[0]);

char jogSelectedAxis = 'X';   // Trục jog đang chọn
float jogDistance = 1.0f;     // Khoảng cách di chuyển mỗi bước jog, bạn có thể thay đổi

// Hàm vẽ trang HOME_STATUS
void DisplayManager::drawHomeStatusPage(int selectedIdx) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x12_tr);
    // Vẽ tiêu đề
    u8g2.drawStr(0, 12, "Home / Jog Status:");

    int y = 30;
    for (int i = 0; i < homeStatusMenuCount; i++) {
        if (i == selectedIdx) {
            u8g2.drawBox(0, y - 10, 128, 12);
            u8g2.setDrawColor(0); // Đặt lại màu in chữ là đen
            u8g2.setCursor(2, y);
            u8g2.print(">");
             u8g2.setCursor(12, y);
            u8g2.print(homeStatusMenu[i]);
            u8g2.setDrawColor(1); // reset lại màu
        } else {
             u8g2.setCursor(12, y);
        u8g2.print(homeStatusMenu[i]);
        }
        y += 14;       
    }
    // Nếu chọn JOG-AXIS, hiện danh sách trục bên dưới để chọn
    if (selectedIdx == 1) {  // JOG-AXIS selected
        u8g2.drawStr(0, y + 10, "Select Axis:");

        int x = 0;
        int yJog = y + 25;

        for (int i = 0; i < jogAxesCount; i++) {
                char axisName[2] = {jogAxesList[i], 0};
                u8g2.drawStr(x + 2, yJog, axisName);
            x += 14;
            if (x > 110) { // xuống dòng nếu quá dài
                x = 0;
                yJog += 14;
            }
        }
       // Hiển thị info jog distance (bước di chuyển mỗi increment encoder)
char jogInfo[32];
snprintf(jogInfo, sizeof(jogInfo), "Jog Step: %.2f", jogDistance);
u8g2.drawStr(0, yJog + 20, jogInfo);
}
u8g2.sendBuffer();
}

// Hàm gọi khi người dùng bấm phím lên/xuống để chọn HOME-ALL hay JOG-AXIS
void DisplayManager::homeStatusMenuMoveUp() {
    if (homeStatusMenuSelected > 0) homeStatusMenuSelected--;
    else homeStatusMenuSelected = homeStatusMenuCount - 1; // Quay về cuối nếu ở đầu
}

void DisplayManager::homeStatusMenuMoveDown() {
    if (homeStatusMenuSelected < homeStatusMenuCount - 1) homeStatusMenuSelected++;
}

// Hàm xử lý khi người dùng nhấn phím OK/Enter trên trang HomeStatus
void DisplayManager::homeStatusMenuSelect() {
    if (homeStatusMenuSelected == 0) {
        // HOME-ALL được chọn -> gửi lệnh homing
        Serial1.println("G28");
    } else if (homeStatusMenuSelected == 1) {
         char cmdBuf[8];
        sprintf(cmdBuf, "G28 %c", jogSelectedAxis); // VD sẽ thành G28 X, G28 Y...
        Serial1.println(cmdBuf);
        // hoặc tùy ý mở rộng cho từng trường hợp
    }
}

// Hàm chọn trục jog khi người dùng nhấn phím số 1..5
void DisplayManager::selectJogAxis(char key) {
    // Nếu key là số '1'..'5', chuyển thành index trục
    if (key >= '1' && key <= '5') {
        int index = key - '1';
        if (index < jogAxesCount) {
            jogSelectedAxis = jogAxesList[index];
            // Bạn có thể reset jogDistance hoặc để nguyên tùy nhu cầu
        }
    }
}

// Hàm xử lý khi quay encoder (+step hoặc -step)
void DisplayManager::jogByStep(int steps) {
    // Tính khoảng cách di chuyển
    float dist = jogDistance * steps;
    // Gửi lệnh jog tương ứng tới Marlin
    // Ví dụ gửi lệnh jog:
    //   G91 (relative mode)
    //   G1 X... F1000 (axis và khoảng cách)
    Serial1.println("G91"); // Chế độ tương đối
    Serial1.print("G1 ");
    Serial1.print(jogSelectedAxis);
    Serial1.print(dist, 3);  // 3 chữ số thập phân
    Serial1.println(" F1000");
    Serial1.println("G90"); // Quay lại chế độ tuyệt đối nếu cần
}
void DisplayManager::setJogDistance(float dist) { jogDistance = dist; }

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
