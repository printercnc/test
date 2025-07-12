

#ifndef LCD_HPP
#define LCD_HPP

#include <cstddef>  // cho size_t
#include <cstdio>   // cho snprintf
#include <cstring>  // cho memset
// Enum xác định các trang hiện có
enum class Page {
    Warning,
    Machine,
    Job,
    Parameter,
    None
};

// Giả định đây là interface (class) LCD12864 bạn đã có từ đâu đó
class LCD12864 {
public:
    void clear_buffer();
    void write_line(int line, const char *text);
    void render();
    // Có thể có thêm các hàm khác...
}

// Cấu trúc dữ liệu để giữ trạng thái CNC (phụ thuộc bạn truyền dữ liệu từ Marlin)
struct CNCStatus {
    bool connected;      // Có kết nối UART với Marlin không
    float pos_x;
    float pos_y;
    float pos_z;
    float pos_a; // trục A (bù hoặc quay thêm)
    float pos_b; // trục B (nếu dùng, có thể bỏ)
    float pos_c; // trục C (6 trục máy của bạn)
    
    // Các tham số máy (gia tốc, max speed...) ví dụ kiểu float đơn giản
    float accel;
    float max_speed;
    float step_per_mm;
};
// UIManager quản lý giao diện LCD
class UIManager {
public:
    UIManager(LCD12864 &lcd);

    void setPage(Page p);
    Page getPage() const;

    void updateStatus(const CNCStatus &status);
    void refresh();

    void nextPage();
    void prevPage();

private:
    LCD12864 &lcd;
    Page currentPage;
    CNCStatus status;

    void showWarningPage();
    void showMachinePage();
    void showJobPage();
    void showParameterPage();

    static void floatToStr(char *buf, size_t bufsize, float val, int decimals);
};

#endif // LCD_HPP