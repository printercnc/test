#pragma once
#include <memory>
#include "lib/lcd.hpp"       // Giả định có class LCD để vẽ lên màn hình
#include "marlin_comm.hpp"   // Giao tiếp Marlin
#include <string>

// Định nghĩa kiểu InputEvent, giả định từ input_handler.hpp
struct InputEvent {
    enum Type {
        BUTTON_PRESS,
        ENCODER_ROTATE,
        // ... các loại khác
    } type;

    int code;   // Mã nút hoặc mã encoder
    int value;  // Giá trị biến động (vd: số bước encoder)
};

// Base class cho trang giao diện
class Page {
public:
    explicit Page(MarlinComm &marlin) : marlin_(marlin) {}
    virtual ~Page() {}

    // Xử lý input của người dùng (nút bấm, encoder)
    virtual void handleInput(const InputEvent &ev) = 0;

    // Cập nhật trạng thái trang giao diện (lấy dữ liệu từ marlin, trạng thái)
    virtual void update() = 0;

    // Vẽ giao diện ra LCD
    virtual void draw(LCD &lcd) = 0;

protected:
    MarlinComm &marlin_;
};



// Class WarningPage: cảnh báo hệ thống, thông báo lỗi
class WarningPage : public Page {
public:
    explicit WarningPage(MarlinComm &marlin);
    void handleInput(const InputEvent &ev) override;
    void update() override;
    void draw(LCD &lcd) override;

private:
    std::string warningMessage_;
    bool hasWarning_;
};


// Class MachinePage: trạng thái máy (nhiệt độ, trạng thái đùn, ...)
class MachinePage : public Page {
public:
    explicit MachinePage(MarlinComm &marlin);
    void handleInput(const InputEvent &ev) override;
    void update() override;
    void draw(LCD &lcd) override;

private:
    float nozzleTemp_;  // Nhiệt độ đầu phun
    float bedTemp_;     // Nhiệt độ bàn in
    std::string state_;
};


// Class JobPage: trạng thái công việc in hiện tại
class JobPage : public Page {
public:
    explicit JobPage(MarlinComm &marlin);
    void handleInput(const InputEvent &ev) override;
    void update() override;
    void draw(LCD &lcd) override;

private:
    std::string jobName_;
    int progressPercent_;
    int elapsedTime_;   // Đơn vị giây
};


// Class ParameterPage: điều chỉnh tham số máy (tốc độ, nhiệt độ,...)
class ParameterPage : public Page {
public:
    explicit ParameterPage(MarlinComm &marlin);
    void handleInput(const InputEvent &ev) override;
    void update() override;
    void draw(LCD &lcd) override;

private:
    int selectedParamIndex_;
    // ví dụ các tham số có thể chỉnh được (lưu tạm dạng string để demo)
    std::vector<std::string> paramNames_;
    std::vector<int> paramValues_;
};
