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
    int getCurrentPage() const;
    int getCurrentJogMultiplier() const;
    void sendHomeCommand(char axis); // bạn implement chi tiết bên cpp
    // Cập nhật trạng thái kết nối và loại máy
    void updateConnectionStatus(bool connected);
    void setMachineType(int type); // 0 = CNC, 1 = 3D Printer
    void setCurrentPage(int page) {
        currentPage = page;
    }
    
    char getSelectedAxis() const { return selectedAxis; }
    uint8_t getSelectedAxisIndex() const { return selectedAxisIndex; }
    int getJogMultiplierIndex() const { return jogMultiplierIndex; }

private:
    DisplayManager& displayManager;
    int currentPage;
    char selectedAxis;
    int selectedAxisIndex;
    int jogMultiplierIndex;
    bool isConnected; // Trạng thái kết nối
    int machineType;  // 0 = CNC, 1 = 3D Printer

    static constexpr int jogMultipliers[3] = {1, 10, 100}; // ví dụ cấp độ bước nhảy
    void autoNavigateOnConnect(); // Điều hướng tự động khi kết nối
      
};

#endif