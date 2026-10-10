#ifndef AUTO_CONTROL_H
#define AUTO_CONTROL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// =====================================================
// LIEN KET VOI PC CHAY HYBRID A* + PURE PURSUIT
// (he thong "lui chuong" tu dong) - doc lap hoan toan
// voi udp.cpp (vo lang/ban dap) ve ca cong UDP lan
// dinh dang goi tin, de khong dung cham toi duong day
// dieu khien tay dang chay on dinh.
//
// Goi tin nhi phan 10 byte, PC -> ESP32:
//   [0]   0xA5              SYNC_1
//   [1]   0x5A              SYNC_2
//   [2-3] sequence          uint16 LE, tang dan moi lan gui
//   [4-5] steer_deg * 100   int16  LE, + = trai, - = phai
//   [6-7] speed_mps * 1000  int16  LE, + = tien, - = lui
//   [8]   enable            0 = tat, 1 = bat
//   [9]   checksum          XOR byte 0..8
//
// Sequence dung de phat hien goi tin cu/trung/den sai thu tu
// (dac tinh von co cua UDP) - goi nao khong "moi hon" goi da
// nhan gan nhat se bi loai, khong duoc dem la lenh dieu khien.
// =====================================================

void auto_control_init(void);

void auto_control_handle(void);

// Song (fresh) = da nhan duoc goi tin HOP LE (dung sync/checksum/
// sequence) trong vong AUTO_FAILSAFE_MS gan day nhat, VA co si
// muon (enable=1). esp32_vehicle.ino chi nen tin lenh auto khi
// ham nay tra ve true.
bool auto_control_is_active(void);

// Gia tri goc (don vi that su: do, m/s) - chi de debug/log.
float auto_control_get_steer_deg(void);
float auto_control_get_speed_mps(void);

// Gia tri da quy doi san sang truyen thang cho servor_control.cpp /
// esc_control.cpp. An toan de goi bat ky luc nao: neu dang khong
// "active" (xem auto_control_is_active), tra ve trang thai trung
// tam/dung han (khong lam gi ca), KHONG bao gio tra ve lenh cu.
int16_t auto_control_get_steer_count(void);

void auto_control_get_throttle(
    uint16_t *ga,
    uint16_t *lui,
    uint16_t *phanh
);

#ifdef __cplusplus
}
#endif

#endif
