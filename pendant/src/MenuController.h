//menu controller keyboard.h

#ifndef MENU_CONTROLLER_H
#define MENU_CONTROLLER_H

#include <Arduino.h>

#define PAGE_COUNT 5

class MenuController {
public:
    MenuController();

    void handleKey(char key);

    uint8_t getCurrentPage() const { return currentPage; }
    char getSelectedAxis() const { return selectedAxis; }
    uint8_t getSelectedAxisIndex() const { return selectedAxisIndex; }
    int getJogMultiplierIndex() const { return jogMultiplierIndex; }

private:
    uint8_t currentPage;
    char selectedAxis;
    uint8_t selectedAxisIndex;
    int jogMultiplierIndex;

    static constexpr int jogMultipliers[3] = {1, 10, 100}; // ví dụ cấp độ bước nhảy

    void sendHomeCommand(char axis); // bạn implement chi tiết bên cpp
};

#endif