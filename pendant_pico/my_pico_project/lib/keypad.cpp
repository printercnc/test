#include "keypad.hpp"

Keypad4x4::Keypad4x4(uint row_pins[4], uint col_pins[4]) {
    for (int i=0; i<4; i++) {
        rows[i] = row_pins[i];
        cols[i] = col_pins[i];
    }
}

void Keypad4x4::init() {
    // Rows: output (set LOW to scan)
    for (int i=0; i<4; i++) {
        gpio_init(rows[i]);
        gpio_set_dir(rows[i], GPIO_OUT);
        gpio_put(rows[i], 1);
    }
    // Columns: input with pullup
    for (int i=0; i<4; i++) {
        gpio_init(cols[i]);
        gpio_set_dir(cols[i], GPIO_IN);
        gpio_pull_up(cols[i]);
    }
}

// Quét bàn phím, trả về ký tự phím nhấn, 0 nếu không có
char Keypad4x4::getKey() {
    for (int r=0; r<4; r++) {
        gpio_put(rows[r], 0);
        for (int c=0; c<4; c++) {
            if (gpio_get(cols[c]) == 0) {  // nhấn phím
                gpio_put(rows[r], 1);
                return keymap[r][c];
            }
        }
        gpio_put(rows[r], 1);
    }
    return 0;
}