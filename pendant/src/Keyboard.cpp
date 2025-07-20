
//Keyboard.h

#include "Keyboard.h"
#include <Arduino.h> // cho pinMode, digitalRead, digitalWrite, delay

// B���ng map key mặc định (4x4)
const char Keyboard::keymap[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

Keyboard::Keyboard(const uint8_t *rowPins, size_t rowCount, const uint8_t *colPins, size_t colCount)
    : m_rowPins(rowPins), m_rowCount(rowCount), m_colPins(colPins), m_colCount(colCount) {
    // Có thể cấu hình chân lúc tạo đối tượng ở đây nếu bạn muốn
    for (size_t i = 0; i < m_rowCount; ++i) {
        pinMode(m_rowPins[i], INPUT_PULLUP);
    }
    for (size_t i = 0; i < m_colCount; ++i) {
        pinMode(m_colPins[i], INPUT_PULLUP); // Đảm bảo kéo lên khi không scan
    }
}

char Keyboard::scanKeyboard() {
    for (size_t c = 0; c < m_colCount; c++) {
        // Đặt cột này xuống LOW, các cột khác vẫn PULLUP
        pinMode(m_colPins[c], OUTPUT);
        digitalWrite(m_colPins[c], LOW);

        for (size_t r = 0; r < m_rowCount; r++) {
            if (digitalRead(m_rowPins[r]) == LOW) {
                delay(20); // debounce nhẹ
                // Chờ nhả phím
                while (digitalRead(m_rowPins[r]) == LOW);
                pinMode(m_colPins[c], INPUT_PULLUP); // Trả lại trạng thái PULLUP
                // Debug: in ra phím nhận được
                Serial.print("Key pressed: ");
                Serial.println(keymap[r][c]);
                return keymap[r][c];
            }
        }
        pinMode(m_colPins[c], INPUT_PULLUP); // Trả lại trạng thái PULLUP
    }
    return 0;
}