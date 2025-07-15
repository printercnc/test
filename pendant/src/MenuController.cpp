

#include "MenuController.h"
#include "DisplayManager.h"
#include <Arduino.h>

MenuController::MenuController(DisplayManager& displayManagerRef)
    : displayManager(displayManagerRef), currentPage(0), selectedAxis('X'), selectedAxisIndex(0), jogMultiplierIndex(0),
      isConnected(false), machineType(0) { // Giá trị mặc định cho isConnected và machineType
}

void MenuController::handleKey(char key) {
    switch (key) {
        case '*':  // Chuyển trang
            // Cho phép chuyển trang dù có kết nối hay không
            currentPage = (currentPage + 1) % PAGE_COUNT;
            Serial.print("Page changed to: ");
            Serial.println(currentPage);
            break;

        case '1': case 'X':  
            selectedAxis = 'X'; selectedAxisIndex = 0; 
            Serial.println("Selected Axis: X");
            break;

        case '2': case 'Y':  
            selectedAxis = 'Y'; selectedAxisIndex = 1; 
            Serial.println("Selected Axis: Y");
            break;

        case '3': case 'Z':  
            selectedAxis = 'Z'; selectedAxisIndex = 2; 
            Serial.println("Selected Axis: Z");
            break;

        case 'A':            
            selectedAxis = 'A'; selectedAxisIndex = 3; 
            Serial.println("Selected Axis: A");
            break;

        case 'C':            
            selectedAxis = 'C'; selectedAxisIndex = 4; 
            Serial.println("Selected Axis: C");
            break;

        case '#':            
            selectedAxis = 'E'; selectedAxisIndex = 5; 
            Serial.println("Selected Axis: E");
            break;

        case 'B':  // thay đổi cấp độ bước nhảy encoder
            jogMultiplierIndex = (jogMultiplierIndex + 1) % (sizeof(jogMultipliers) / sizeof(jogMultipliers[0]));
            Serial.print("Jog multiplier changed to: ");
            Serial.println(jogMultipliers[jogMultiplierIndex]);
            break;

        case '8': // Enter/Select
            Serial.println("Key 8 pressed: Enter/Select");
            switch (currentPage) {
                case 3: // Trang điều khiển jog & home
                    if (selectedAxis >= 'A' && selectedAxis <= 'Z') {
                        // Ví dụ: gửi lệnh jog hoặc home
                        if (selectedAxis == 'H') { // Giả lập phím HOME
                            displayManager.sendHomeCommand();
                        } else {
                            float dist = jogMultipliers[jogMultiplierIndex];
                            displayManager.sendJogCommand(selectedAxis, dist);
                        }
                    }
                    break;

                // Các trang khác xử lý nhập liệu ở đây...

                default:
                    break;
            }
            break;

        case '5': // Tăng giá trị hoặc menu
            // Ví dụ dùng phím này để tăng jogDistance hoặc chọn param
            jogMultiplierIndex = (jogMultiplierIndex + 1) % (sizeof(jogMultipliers) / sizeof(jogMultipliers[0]));
            Serial.print("Jog multiplier increased to: ");
            Serial.println(jogMultipliers[jogMultiplierIndex]);
            break;

        case '0': // Giảm giá trị hoặc menu
            if (jogMultiplierIndex == 0)
                jogMultiplierIndex = (sizeof(jogMultipliers) / sizeof(jogMultipliers[0])) - 1;
            else
                jogMultiplierIndex--;
            Serial.print("Jog multiplier decreased to: ");
            Serial.println(jogMultipliers[jogMultiplierIndex]);
            break;

        // các phím khác giữ nguyên
    }
}

int MenuController::getCurrentPage() const {
    return currentPage;
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
void MenuController::updateConnectionStatus(bool connected) {
    isConnected = connected;
    if (isConnected) {
        autoNavigateOnConnect(); // Điều hướng tự động khi kết nối
    } else {
        currentPage = 0; // Quay lại trang 0 nếu không kết nối
        Serial.println("Disconnected. Returning to page 0.");
    }
}
void MenuController::setMachineType(int type) {
    machineType = type; // 0 = CNC, 1 = 3D Printer
    Serial.print("Machine type set to: ");
    if (machineType == 0) {
        Serial.println("CNC");
    } else if (machineType == 1) {
        Serial.println("3D Printer");
    } else {
        Serial.println("Unknown");
    }
}
void MenuController::autoNavigateOnConnect() {
    if (machineType == 0) { // CNC
        currentPage = 1; // Điều hướng đến trang 1
        Serial.println("Connected. Navigating to page 1 (CNC mode).");
    } else if (machineType == 1) { // 3D Printer
        currentPage = 2; // Điều hướng đến trang 2
        Serial.println("Connected. Navigating to page 2 (3D Printer mode).");
    }
}