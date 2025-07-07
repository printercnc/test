// PrinterI2C.h
#ifndef PRINTERI2C_H
#define PRINTERI2C_H

#include "printer_status.h"
#include "stm32f1xx_hal.h"

class PrinterI2C {
public:
   PrinterI2C(I2C_HandleTypeDef *hi2c, uint8_t slaveAddr);

    void begin();                  // Khởi tạo liên quan (nếu cần)
    void update();                 // Gửi yêu cầu trạng thái / nhận dữ liệu theo chu kỳ
    bool isConnected() const;      // Trạng thái kết nối có hợp lệ hay không

    bool sendCommand(const PrinterCommand &cmd);
    bool sendHomeCommand();
    bool sendJogCommand(uint8_t axis, float delta);

    const PrinterStatus& getLastStatus() const { return m_lastStatus; } // Truy cập trạng thái mới nhất

private:
    bool requestStatus(PrinterStatus &status);
    void serializeCommand(const PrinterCommand &cmd, uint8_t *buffer);

    I2C_HandleTypeDef* m_i2cHandle;
    uint8_t m_slaveAddress;

    PrinterStatus m_lastStatus;
    bool m_connected = false;
};

#endif