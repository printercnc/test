
//MenuController.cpp

#include "MenuController.h"
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
uint8_t MenuController::getExtruderType() const {return extruderType;}

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
// -- các hàm dưới đây nằm NGOÀI handleKey, không lồng vào -- 
void MenuController::homeStatusMenuMoveUp() {
    homeStatusMenuSelected = (homeStatusMenuSelected - 1 + homeStatusMenuCount) % homeStatusMenuCount;
}
void MenuController::homeStatusMenuMoveDown() {
    homeStatusMenuSelected = (homeStatusMenuSelected + 1) % homeStatusMenuCount;
}

int MenuController::getHomeStatusMenuCount() const { return homeStatusMenuCount;}
 
const int CNC_PAGES[] = {1, 3, 4};        // PAGE_G54, PAGE_PARAMETER, PAGE_HOME_STATUS
const int PRINTER3D_PAGES[] = {2, 3, 4};  // PAGE_PRINTER3D, PAGE_PARAMETER, PAGE_HOME_STATUS
const int CNC_PAGES_COUNT = sizeof(CNC_PAGES)/sizeof(CNC_PAGES[0]);
const int PRINTER3D_PAGES_COUNT = sizeof(PRINTER3D_PAGES)/sizeof(PRINTER3D_PAGES[0]);
//----------------------------------
void MenuController::handleKey(char key) {
    switch (key) {
        case '*':
    if (userSelectedPage == -1) {
        userSelectedPage = (extruderType == 0) ? CNC_PAGES[0] : PRINTER3D_PAGES[0];
    } else {
        userSelectedPage = getNextUserPage(userSelectedPage, extruderType);
    }
    lastUserInputTime = millis();
    if (uartConnected) {
        Serial1.print("User changed page to: ");
        Serial1.println(userSelectedPage);
    }
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
            Serial1.println(jogMultipliers[jogMultiplierIndex], 2); // in với 2 số thập phân
            break;

        case '8':
    if (currentPage == PAGE_HOME_STATUS) {
        if (homeStatusMenuSelected == 0) {
            Serial1.println("G28");
            // Tự động chuyển page
            userSelectedPage = (extruderType == 0) ? 1 : 2;
            lastUserInputTime = millis();
        } else if (homeStatusMenuSelected == 1) {
            Serial1.println("G91");
            Serial1.print("G1 ");
            Serial1.print(selectedAxis);
            Serial1.print(getCurrentJogMultiplier());
            Serial1.println(" F1000");
            Serial1.println("G90");
        }
    }
    break;

         case '0':
            if (currentPage == PAGE_HOME_STATUS) {
            Serial1.println("Pressed 0 Move DOWN");
            homeStatusMenuMoveDown();
            Serial1.print("homeStatusMenuSelected: ");
            Serial1.println(homeStatusMenuSelected);
            }else if (currentPage == PAGE_PARAMETER) {
            parameterMenuMoveDown();
            Serial1.print("Parameter selected: ");
            Serial1.println(parameterSelectedIdx );
        }
            break;

         case 'D':
            if (currentPage == PAGE_HOME_STATUS) {
            autoPageEnabled = true;
            userSelectedPage = -1; // reset override
            Serial1.println("Auto page enabled");
            return; // trả về ngay, không disable autoPageEnabled nữa
            }
            break;

        default:
            break;
    }

    // Nếu key không phải D ở trang PAGE_HOME_STATUS thì tắt autoPageEnabled
    if (currentPage == PAGE_HOME_STATUS && key != 'D') {
        autoPageEnabled = false;
    }
}

void MenuController::setAutoPage(int page) {
    autoPage = page;
}
void MenuController::updatePage() {
    if (!autoPageEnabled) {
        // Auto page off, chỉ dùng userSelectedPage nếu có, hoặc giữ currentPage
        if (userSelectedPage != -1) {
            currentPage = userSelectedPage;
        }
        // Nếu userSelectedPage == -1 thì giữ nguyên currentPage, không chuyển tự động
    } else {
        if (userSelectedPage != -1 && (millis() - lastUserInputTime) < USER_OVERRIDE_TIMEOUT) {
            currentPage = userSelectedPage;
        } else {
            userSelectedPage = -1;
            currentPage = autoPage;
        }
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
float MenuController::getCurrentJogMultiplier() const {
    return jogMultipliers[jogMultiplierIndex];
}
void MenuController::parameterMenuMoveUp() {
    parameterSelectedIdx  = (parameterSelectedIdx  - 1 + paramCount) % paramCount;
}
void MenuController::parameterMenuMoveDown() {
    parameterSelectedIdx  = (parameterSelectedIdx  + 1) % paramCount;
}
int MenuController::getNextUserPage(int current, uint8_t extruderType) {
    int idx = -1;
    if (extruderType == 0) {
        for (int i = 0; i < CNC_PAGES_COUNT; i++)
            if (CNC_PAGES[i] == current) { idx = i; break; }
        int nextIdx = (idx < 0) ? 0 : (idx+1)%CNC_PAGES_COUNT;
        return CNC_PAGES[nextIdx];
    } else {
        for (int i = 0; i < PRINTER3D_PAGES_COUNT; i++)
            if (PRINTER3D_PAGES[i] == current) { idx = i; break; }
        int nextIdx = (idx < 0) ? 0 : (idx+1)%PRINTER3D_PAGES_COUNT;
        return PRINTER3D_PAGES[nextIdx];
    }
}