#include "ui_pages.hpp"
#include <cstdio>

//
// WarningPage
//
WarningPage::WarningPage(MarlinComm &marlin)
    : Page(marlin), warningMessage_(""), hasWarning_(false) {}

void WarningPage::handleInput(const InputEvent &ev) {
    // Với trang cảnh báo, có thể bạn chỉ muốn nút “xác nhận” hoặc reset warning
    if (ev.type == InputEvent::BUTTON_PRESS) {
        if (ev.code == /* nút xác nhận, ví dụ 0*/) {
            hasWarning_ = false;
            warningMessage_.clear();
        }
    }
}

void WarningPage::update() {
    // Cập nhật trạng thái warning từ Marlin, ví dụ:
    // Giả định marlin_.getWarning() trả về empty string nếu không có cảnh báo
    std::string warning = marlin_.getWarning();
    if (!warning.empty()) {
        warningMessage_ = warning;
        hasWarning_ = true;
    }
}

void WarningPage::draw(LCD &lcd) {
    lcd.clear();
    if (hasWarning_) {
        lcd.print(0, 0, "WARNING:");
        lcd.print(0, 1, warningMessage_.c_str());
    } else {
        lcd.print(0, 0, "No Warnings");
    }
    lcd.refresh();
}

//
// MachinePage
//
MachinePage::MachinePage(MarlinComm &marlin)
    : Page(marlin), nozzleTemp_(0), bedTemp_(0), state_("Idle") {}

void MachinePage::handleInput(const InputEvent &ev) {
    // Có thể xử lý thay đổi trang hoặc mở menu ở đây
    (void)ev; // tránh cảnh báo chưa dùng nếu bạn chưa dùng đến
}

void MachinePage::update() {
    nozzleTemp_ = marlin_.getNozzleTemperature();
    bedTemp_ = marlin_.getBedTemperature();
    state_ = marlin_.getMachineState();
}

void MachinePage::draw(LCD &lcd) {
    lcd.clear();
    char buf[32];
    snprintf(buf, sizeof(buf), "Nozzle: %.1f C", nozzleTemp_);
    lcd.print(0, 0, buf);
    snprintf(buf, sizeof(buf), "Bed: %.1f C", bedTemp_);
    lcd.print(0, 1, buf);
    lcd.print(0, 2, ("State: " + state_).c_str());
    lcd.refresh();
}

//
// JobPage
//
JobPage::JobPage(MarlinComm &marlin)
    : Page(marlin), jobName_("None"), progressPercent_(0), elapsedTime_(0) {}

void JobPage::handleInput(const InputEvent &ev) {
    // Xử lý input điều khiển job (ví dụ pause, stop, ...)
    (void)ev;
}

void JobPage::update() {
    jobName_ = marlin_.getCurrentJobName();
    progressPercent_ = marlin_.getJobProgress();
    elapsedTime_ = marlin_.getJobElapsedTime();
}

void JobPage::draw(LCD &lcd) {
    lcd.clear();
    lcd.print(0, 0, ("Job: " + jobName_).c_str());
    char buf[32];
    snprintf(buf, sizeof(buf), "Progress: %d%%", progressPercent_);
    lcd.print(0, 1, buf);
        int minutes = elapsedTime_ / 60;
    int seconds = elapsedTime_ % 60;
    snprintf(buf, sizeof(buf), "Time: %02d:%02d", minutes, seconds);
    lcd.print(0, 2, buf);
    lcd.refresh();
}