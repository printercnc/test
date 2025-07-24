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
    
     bool autoPageEnabled = true;

     // Cập nhật trang tự động (autoPage, từ firmware)
      void homeStatusMenuMoveUp();
    void homeStatusMenuMoveDown();
    void setAutoPage(int page);
    void updatePage();
    int getCurrentPage() const;
     // Cách lấy giá trị userSelectedPage (nếu cần)
    int getUserSelectedPage() const;
    // Cách kiểm tra trạng thái override
    bool isUserOverrideActive() const;

    float getCurrentJogMultiplier() const;
     // Thêm hàm set/get extruderType
    void setExtruderType(uint8_t type);
    uint8_t getExtruderType() const;
    
    void setUartConnected(bool connected);
    bool isUartConnected() const ;

    char getSelectedAxis() const { return selectedAxis; }
    uint8_t getSelectedAxisIndex() const { return selectedAxisIndex; }
    void setJogMultiplierIndex(int index);
    
    int getHomeStatusMenuSelected() const { return homeStatusMenuSelected; }
     int getCurrentRow() const { return currentRow; }
    void setCurrentRow(int row) { currentRow = row; }
    
    void setHomeStatusMenuSelected(int idx) {
        if (idx >=0 && idx < homeStatusMenuCount) homeStatusMenuSelected = idx;
    }
        int getHomeStatusMenuCount() const; 
private:
    DisplayManager& displayManager;
     uint8_t extruderType = 0;    // 0 = CNC (default), 1 = 3D printer
     int autoPage = 0;           // Trang tự động
     bool uartConnected = false;

    int userSelectedPage = -1;  // Trang do người dùng chọn (-1 nghĩa chưa chọn)
    uint32_t lastUserInputTime = 0;

    static constexpr uint32_t USER_OVERRIDE_TIMEOUT = 10000; // 10 giây

    int currentPage = 0;
    char selectedAxis;
    int selectedAxisIndex;

    int currentRow = 0;
    static constexpr int HOME_ROW_INDEX = 4; // ví dụ dòng 4 là dòng HOME trên menu
    
    void drawHomeStatusPage(int selectedIdx, char jogSelectedAxis, float jogDistance);
    static constexpr int jogMultipliers[3] = {1, 10, 100}; // ví dụ cấp độ bước nhảy
    int jogMultiplierIndex;
    void autoNavigateOnConnect(); // Điều hướng tự động khi kết nối
    int parameterSelectedIdx = 0;  // Dòng đang chọn ở PAGE_PARAMETER
    int homeStatusMenuSelected = 0;
    static const int homeStatusMenuCount = 2; // Dòng đang chọn ở PAGE_HOME_STATUS
    
};

#endif