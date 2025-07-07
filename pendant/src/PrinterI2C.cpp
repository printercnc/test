//printeri2c.cpp

#include "PrinterI2C.h"
#include <cstring>

#define STATUS_REQUEST_CODE 0xFE

PrinterI2C::PrinterI2C(I2C_HandleTypeDef *hi2c, uint8_t slaveAddr)
    : m_i2cHandle(hi2c), m_slaveAddress(slaveAddr)
{memset(&m_lastStatus, 0, sizeof(m_lastStatus));}

void PrinterI2C::begin() {
    // Nếu cần khởi tạo thêm I2C hoặc reset bộ đệm, delay, etc.
    // Với HAL I2C thì có thể không cần, để trống cũng được
    m_connected = false;
}

void PrinterI2C::update() {
    PrinterStatus status;
    if (requestStatus(status)) {
        m_lastStatus = status;
        m_connected = true;
    } else {
        m_connected = false;
    }
}

bool PrinterI2C::isConnected() const {
    return m_connected;
}

void PrinterI2C::serializeCommand(const PrinterCommand &cmd, uint8_t *buffer) {
    buffer[0] = cmd.command_id;
    memcpy(buffer + 1, cmd.args, sizeof(cmd.args));
}

bool PrinterI2C::sendCommand(const PrinterCommand &cmd) {
    uint8_t buf[sizeof(PrinterCommand)];
    serializeCommand(cmd, buf);
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(
        m_i2cHandle,
        (uint16_t)(m_slaveAddress << 1),  // Địa chỉ dạng 8bit (7bit <<1)
        buf,
        sizeof(buf),
        HAL_MAX_DELAY
    );
    return (status == HAL_OK);
}

bool PrinterI2C::requestStatus(PrinterStatus &status) {
    uint8_t requestCode = STATUS_REQUEST_CODE;
    HAL_StatusTypeDef ret;

    // Gửi mã yêu cầu trạng thái đến slave
    ret = HAL_I2C_Master_Transmit(
        m_i2cHandle,
        (uint16_t)(m_slaveAddress << 1),
        &requestCode, 1,
        HAL_MAX_DELAY);
    if (ret != HAL_OK) return false;

    // Đọc dữ liệu trạng thái từ slave
    uint8_t buf[sizeof(PrinterStatus)];
    ret = HAL_I2C_Master_Receive(
        m_i2cHandle,
        (uint16_t)(m_slaveAddress << 1),
        buf,
        sizeof(buf),
        HAL_MAX_DELAY);
    if (ret != HAL_OK) return false;

    // Copy dữ liệu đã đọc vào struct
    memcpy(&status, buf, sizeof(status));
    return true;
}

// Gửi lệnh Home All Axes
bool PrinterI2C::sendHomeCommand() {
    PrinterCommand cmd = {0};
    cmd.command_id = CMD_HOME;  // CMD_HOME = 0x01 theo header
    memset(cmd.args, 0, sizeof(cmd.args));
    return sendCommand(cmd);
}

// Gửi lệnh jog trục (axis - index, delta là bước di chuyển)
bool PrinterI2C::sendJogCommand(uint8_t axis, float delta) {
    PrinterCommand cmd = {0};
    cmd.command_id = CMD_PAUSE + 1; // hoặc 0x02 là jog tùy bạn định nghĩa
    // Ví dụ bạn tận dụng args để ghi trục và delta float
    // Lưu ý về endianness nếu cần
    cmd.args[0] = axis;
    memcpy(cmd.args + 1, &delta, sizeof(float));
    // Phần args còn lại để 0
    memset(cmd.args + 1 + sizeof(float), 0,
           sizeof(cmd.args) - 1 - sizeof(float));
    return sendCommand(cmd);
}