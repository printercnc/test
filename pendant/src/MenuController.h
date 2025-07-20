//menu controller keyboard.h

#ifndef MENU_CONTROLLER_H
#define MENU_CONTROLLER_H

#include <Arduino.h>
#include "DisplayManager.h"
#define PAGE_COUNT 5

class MenuController {
public:
    MenuController(DisplayManager& displayManagerRef);
     void handleKey(char key);
     void setCurrentPage(int page);
    int getCurrentPage() const;
     // Cập nhật trang tự động (autoPage, từ firmware)
    void setAutoPage(int page);
     // Cách lấy giá trị userSelectedPage (nếu cần)
    int getUserSelectedPage() const;

    // Cách kiểm tra trạng thái override
    bool isUserOverrideActive() const;

    int getCurrentJogMultiplier() const;
    void sendHomeCommand(char axis); // bạn implement chi tiết bên cpp

     // Thêm hàm set/get extruderType
    void setExtruderType(uint8_t type);
    uint8_t getExtruderType() const;
    
    void setUartConnected(bool connected);
    bool isUartConnected() const ;

    char getSelectedAxis() const { return selectedAxis; }
    uint8_t getSelectedAxisIndex() const { return selectedAxisIndex; }
    int getJogMultiplierIndex() const { return jogMultiplierIndex; }

     int getCurrentRow() const { return currentRow; }
    void setCurrentRow(int row) { currentRow = row; }

    void updatePage();

private:
    DisplayManager& displayManager;
     uint8_t extruderType = 0;    // 0 = CNC (default), 1 = 3D printer
     int autoPage = 0;           // Trang tự động
    int userSelectedPage = -1;  // Trang do người dùng chọn (-1 nghĩa chưa chọn)
    uint32_t lastUserInputTime = 0;

    static constexpr uint32_t USER_OVERRIDE_TIMEOUT = 10000; // 10 giây

    bool uartConnected = false;
    int currentPage = 0;
    char selectedAxis;
    int selectedAxisIndex;
    int jogMultiplierIndex;

    int currentRow = 0;
    static constexpr int HOME_ROW_INDEX = 4; // ví dụ dòng 4 là dòng HOME trên menu

    static constexpr int jogMultipliers[3] = {1, 10, 100}; // ví dụ cấp độ bước nhảy
    void autoNavigateOnConnect(); // Điều hướng tự động khi kết nối
    
};

#endif