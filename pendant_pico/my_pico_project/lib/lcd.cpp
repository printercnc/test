#include "lcd.hpp"
#include <cstdio>
#include <cstring>

UIManager::UIManager(LCD12864 &lcd_) :
    lcd(lcd_), currentPage(Page::Warning)
{
    // Khởi tạo trạng thái mặc định
    std::memset(&status, 0, sizeof(status));
    status.connected = false;
}

// Đổi trang
void UIManager::setPage(Page p) {
    currentPage = p;
    refresh();
}

// Cập nhật trạng thái CNC nhận từ Marlin
void UIManager::updateStatus(const CNCStatus &status_) {
    status = status_;
}

// Hiển thị lại dựa trên trang hiện tại
void UIManager::refresh() {
    lcd.clear_buffer();

    switch (currentPage) {
        case Page::Warning:
            showWarningPage();
            break;
        case Page::Machine:
            showMachinePage();
            break;
        case Page::Job:
            showJobPage();
            break;
        case Page::Parameter:
            showParameterPage();
            break;
        default:
            lcd.write_line(0, "Unknown page");
            break;
    }

    lcd.render();
}

// Chuyển trang kế tiếp
void UIManager::nextPage() {
    switch (currentPage) {
        case Page::Warning:
            // Nếu đã kết nối thì chuyển sang Machine, không thì vẫn ở Warning
            currentPage = status.connected ? Page::Machine : Page::Warning;
            break;
        case Page::Machine:
            currentPage = Page::Job;
            break;
        case Page::Job:
            currentPage = Page::Parameter;
            break;
        case Page::Parameter:
            currentPage = Page::Machine; // Vòng lại Machine
            break;
        default:
            currentPage = Page::Warning;
            break;
    }
    refresh();
}

// Chuyển trang trước
void UIManager::prevPage() {
    switch (currentPage) {
        case Page::Warning:
            currentPage = status.connected ? Page::Parameter : Page::Warning;
            break;
        case Page::Machine:
            currentPage = Page::Warning;
            break;
        case Page::Job:
            currentPage = Page::Machine;
            break;
        case Page::Parameter:
            currentPage = Page::Job;
            break;
        default:
            currentPage = Page::Warning;
            break;
    }
    refresh();
}

// Hiển thị trang Warning
void UIManager::showWarningPage() {
    lcd.write_line(0, "!!! WARNING !!!");
    if (!status.connected) {
        lcd.write_line(1, "Marlin UART NOT");
        lcd.write_line(2, "Connected");
        lcd.write_line(3, "Check cable!");
    } else {
        lcd.write_line(1, "Marlin UART OK");
        lcd.write_line(2, "Press Next...");
        lcd.write_line(3, "");
    }
}

// Hiển thị trang Machine - hiển thị tọa độ các trục
void UIManager::showMachinePage() {
    char linebuf[17];

    lcd.write_line(0, "Machine Pos:");

    snprintf(linebuf, sizeof(linebuf), " X:%+06.2f Y:%+06.2f", status.pos_x, status.pos_y);
    lcd.write_line(1, linebuf);

    snprintf(linebuf, sizeof(linebuf), " Z:%+06.2f A:%+06.2f", status.pos_z, status.pos_a);
    lcd.write_line(2, linebuf);

    snprintf(linebuf, sizeof(linebuf), " C:%+06.2f", status.pos_c);
    lcd.write_line(3, linebuf);
}

// Hiển thị trang Job - chọn trục và send lệnh
void UIManager::showJobPage() {
    lcd.write_line(0, "Job Control:");
    lcd.write_line(1, "X:Run Y:Run Z:Run");
    lcd.write_line(2, "A:Run C:Run Stop");
    lcd.write_line(3, "<Select> <Exec>");
    // Lưu ý: Thao tác chọn trục và gửi lệnh cần xử lý input, xử lý sau ở main 
}

// Hiển thị trang Parameter - vận tốc, gia tốc, step
void UIManager::showParameterPage() {
    char linebuf[17];
    lcd.write_line(0, "Param (overwrt):");

    snprintf(linebuf, sizeof(linebuf), "Accel:%6.2f", status.accel);
    lcd.write_line(1, linebuf);

        snprintf(linebuf, sizeof(linebuf), "Speed:%6.2f", status.max_speed);
    lcd.write_line(2, linebuf);

    snprintf(linebuf, sizeof(linebuf), "Step/mm:%6.2f", status.step_per_mm);
    lcd.write_line(3, linebuf);
}

// Chuyển float sang chuỗi với số chữ số thập phân cố định
void UIManager::floatToStr(char *buf, size_t bufsize, float val, int decimals) {
    char format[10];
    snprintf(format, sizeof(format), "%%.%df", decimals);
    snprintf(buf, bufsize, format, val);
}