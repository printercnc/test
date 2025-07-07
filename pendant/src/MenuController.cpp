#include "MenuController.h"
#include "PrinterI2C.h"


extern PrinterI2C printerI2c;  // khai báo biến toàn cục ngoài

MenuController::MenuController() 
  : currentPage(0), selectedAxis('X'), selectedAxisIndex(0), jogMultiplierIndex(0) {
}

void MenuController::handleKey(char key) {
    switch (key) {
        case '*':  // chuyển trang
            currentPage = (currentPage + 1) % PAGE_COUNT;
            Serial.print("Page changed by keyboard '*', currentPage = ");
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

        case 'D':  // Nút home all
            sendHomeCommand('D');
            Serial.println("Sent Home All command");
            break;

        case '5': // lên menu hoặc tăng giá trị
            // Thực hiện logic tăng menu hoặc giá trị
            Serial.println("Key 5 pressed: Up/Increase");
            // ...bổ sung logic tăng menu hoặc giá trị ở đây...
            break;

        case '0': // xuống menu hoặc giảm giá trị
            // Thực hiện logic giảm menu hoặc giá trị
            Serial.println("Key 0 pressed: Down/Decrease");
            // ...bổ sung logic giảm menu hoặc giá trị ở đây...
            break;

        case '8': // xác nhận/enter
            // Thực hiện logic xác nhận hoặc vào chế độ chỉnh sửa
            Serial.println("Key 8 pressed: Enter/Select");
            // ...bổ sung logic xác nhận ở đây...
            break;

        default:
            // handle những phím khác nếu có
            break;
    }
}

void MenuController::sendHomeCommand(char axis) {
    if (axis == 'D') {
        printerI2c.sendHomeCommand();
    } else {
        // Nếu muốn home từng trục, bạn có thể bổ sung lệnh tương ứng ở đây
        // printerI2c.sendHomeCommand(axis);
    }
}