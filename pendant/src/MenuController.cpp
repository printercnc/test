
//MenuController.cpp

#include "MenuController.h"
#include "DisplayManager.h"
#include <Arduino.h>

MenuController::MenuController(DisplayManager& displayMgr) : displayManager(displayMgr) {
    // Có thể khởi tạo biến member nếu muốn
    extruderType = 0;
    uartConnected = false;
    currentPage = 0;
    jogMultiplierIndex = 0;
    selectedAxis = 'X';
    selectedAxisIndex = 0;
}

// Hàm set extruder type
void MenuController::setExtruderType(uint8_t type) {
    if (extruderType != type) {
        extruderType = type;
        // Nếu cần bạn có thể quản lý logic thay đổi khi extruderType thay đổi
    }
}

// Hàm get extruder type
uint8_t MenuController::getExtruderType() const {
    return extruderType;
}

// Hàm set uart connected
void MenuController::setUartConnected(bool connected) {
    uartConnected = connected;
}
// Hàm get uart connected
bool MenuController::isUartConnected() const {
    return uartConnected;
}
void MenuController::setCurrentPage(int page) {
    if (currentPage != page) {
        currentPage = page;
        // Có thể xử lý logic khi thay đổi trang
    }
}

int MenuController::getCurrentJogMultiplier() const {
    return jogMultipliers[jogMultiplierIndex];
}

void MenuController::sendHomeCommand(char axis) {
    if (axis == 'H') {
        displayManager.sendHomeCommand();
    } else {
        displayManager.sendJogCommand(axis, jogMultipliers[jogMultiplierIndex]);
    }
}

void MenuController::handleKey(char key) {
    switch (key) {
        case '*':
            // Nếu người dùng bấm *, chuyển trang tiếp theo
            if (userSelectedPage == -1) {
                userSelectedPage = 0;
            } else {
                userSelectedPage = (userSelectedPage + 1) % PAGE_COUNT;
            }
            lastUserInputTime = millis();
            Serial1.print("User changed page to: ");
            Serial1.println(userSelectedPage);
            break;

        case '1': case 'X':  
            selectedAxis = 'X'; selectedAxisIndex = 0; 
            Serial1.println("Selected Axis: X");
            break;

        case '2': case 'Y':  
            selectedAxis = 'Y'; selectedAxisIndex = 1; 
            Serial1.println("Selected Axis: Y");
            break;

        case '3': case 'Z':  
            selectedAxis = 'Z'; selectedAxisIndex = 2; 
            Serial1.println("Selected Axis: Z");
            break;

        case 'A':            
            selectedAxis = 'A'; selectedAxisIndex = 3; 
            Serial1.println("Selected Axis: A");
            break;

        case 'C':            
            selectedAxis = 'C'; selectedAxisIndex = 4; 
            Serial1.println("Selected Axis: C");
            break;

        case '#':            
            selectedAxis = 'E'; selectedAxisIndex = 5; 
            Serial1.println("Selected Axis: E");
            break;

        case 'B':  // thay đổi cấp độ bước nhảy encoder
            jogMultiplierIndex = (jogMultiplierIndex + 1) % (sizeof(jogMultipliers) / sizeof(jogMultipliers[0]));
            Serial1.print("Jog multiplier changed to: ");
            Serial1.println(jogMultipliers[jogMultiplierIndex]);
            break;

        case '8': // Enter/Select
    Serial.println("Key 8 pressed: Enter/Select");
    switch (currentPage) {
        case 3: // Trang điều khiển jog & home
            if (currentRow == HOME_ROW_INDEX) {  // HOME_ROW_INDEX = số dòng liệt kê lệnh HOME
                displayManager.sendHomeCommand(); // gọi hàm gửi lệnh homing
            } else if (selectedAxis >= 'A' && selectedAxis <= 'Z') {
                float dist = jogMultipliers[jogMultiplierIndex];
                displayManager.sendJogCommand(selectedAxis, dist); // gửi lệnh jog bình thường
            }
            break;

                // Các trang khác xử lý nhập liệu ở đây...
            }
            break;

        case '5': // Tăng giá trị hoặc menu
            // Ví dụ dùng phím này để tăng jogDistance hoặc chọn param
            jogMultiplierIndex = (jogMultiplierIndex + 1) % (sizeof(jogMultipliers) / sizeof(jogMultipliers[0]));
            Serial1.print("Jog multiplier increased to: ");
            Serial1.println(jogMultipliers[jogMultiplierIndex]);
            break;

        case '0': // Giảm giá trị hoặc menu
            if (jogMultiplierIndex == 0)
                jogMultiplierIndex = (sizeof(jogMultipliers) / sizeof(jogMultipliers[0])) - 1;
            else
                jogMultiplierIndex--;
            Serial1.print("Jog multiplier decreased to: ");
            Serial1.println(jogMultipliers[jogMultiplierIndex]);
            break;

        // các phím khác giữ nguyên
    }
}

void MenuController::setAutoPage(int page) {
    autoPage = page;
}
void MenuController::updatePage() {
    // Nếu userSelectedPage đang override và chưa timeout
    if (userSelectedPage != -1 && (millis() - lastUserInputTime) < USER_OVERRIDE_TIMEOUT) {
        currentPage = userSelectedPage;
    } else {
        // Hết timeout hoặc chưa bao giờ override, reset override
        userSelectedPage = -1;
        currentPage = autoPage;
    }
}
int MenuController::getCurrentPage() const {
    return currentPage;
}

int MenuController::getUserSelectedPage() const {
    return userSelectedPage;
}

bool MenuController::isUserOverrideActive() const {
    return (userSelectedPage != -1) && (millis() - lastUserInputTime < USER_OVERRIDE_TIMEOUT);
}
