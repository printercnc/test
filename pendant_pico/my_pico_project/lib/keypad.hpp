//keypad.hpp

#ifndef KEYPAD_HPP
#define KEYPAD_HPP

#include "pico/stdlib.h"

// Thư viện bàn phím ma trận 4x4

class Keypad4x4 {
public:
    // rows_pins và cols_pins là mảng 4 pin tương ứng
    Keypad4x4(uint row_pins[4], uint col_pins[4]);

    void init();
    // Trả về ký tự phím nhấn, hoặc 0 nếu không có phím nào
    char getKey();

private:
    uint rows[4];
    uint cols[4];

    // Bảng mã phím, chuẩn 4x4
    const char keymap[4][4] = {
        {'1','2','3','A'},
        {'4','5','6','B'},
        {'7','8','9','C'},
        {'*','0','#','D'}
    };
};

#endif