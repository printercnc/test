// printer_status.h

#ifndef PRINTER_STATUS_H
#define PRINTER_STATUS_H
#pragma once
#include <stdint.h>

#define AXIS_COUNT 6
#define EXTRUDERS 2

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)
typedef struct {
    float position[AXIS_COUNT];
    float hotend_temps[EXTRUDERS];
    float hotend_targets[EXTRUDERS];
    float bed_temp;
    float bed_target;
    uint8_t state;
    uint8_t extruderType;     // Thêm trường này: 0 = CNC, 1 = 3D printer
    uint32_t elapsed_seconds;
    uint8_t feedrate_percentage;
    uint8_t reserved[5];      // Giảm còn 5 (do phía trên thêm 1 byte extruderType)
} PrinterStatus;

/**
 * Struct lệnh gửi từ STM32 -> Marlin
 * Ví dụ lệnh Home, Pause, Resume, Stop, ...
 */
typedef struct {
    uint8_t command_id;  // Loại lệnh, ví dụ CMD_HOME
    uint8_t args[7];     // Dữ liệu mở rộng (padding tổng đủ 8 bytes cho dễ giao tiếp và căn hàng)
} PrinterCommand;
#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif // PRINTER_STATUS_H