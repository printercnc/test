//Keyboard.h

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <Arduino.h> // hoặc phù hợp môi trường của bạn

class Keyboard {
public:
    Keyboard(const uint8_t *rowPins, size_t rowCount, const uint8_t *colPins, size_t colCount);

    /**
     * Quét bàn phím 4x4, trả về ký tự tương ứng hoặc 0 nếu không có phím nào được nhấn.
     */
    char scanKeyboard();

private:
    const uint8_t* m_rowPins;
    size_t m_rowCount;

    const uint8_t* m_colPins;
    size_t m_colCount;

    // Bảng map key mặc định 4x4, có thể customize lại nếu muốn
    static const char keymap[4][4];
};

#endif // KEYBOARD_H