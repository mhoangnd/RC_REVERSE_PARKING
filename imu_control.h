#ifndef IMU_CONTROL_H
#define IMU_CONTROL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// =====================================================
// CAM BIEN BNO055 (I2C) - doc goc yaw tuyet doi (fusion 9 truc)
// va gia toc tuyen tinh (da tru trong luc).
//
// Day KHONG phai duong dieu khien - BNO055 khong tu dong dieu
// khien servo/motor. No chi cung cap them:
//   1. Goc yaw rieng cua xe (co the doi chieu voi yaw camera tinh
//      ra sau nay neu muon lam sensor fusion - chua lam trong
//      ban nay).
//   2. Phat hien va cham/rung manh bat thuong qua gia toc, dung
//      lam THEM 1 lop an toan trong esp32_vehicle.ino (dung xe
//      ngay neu vua va cham, bat ke dang o che do nao).
// =====================================================

void imu_control_init(void);

void imu_control_handle(void);

// True neu BNO055 khoi tao thanh cong va dang doc duoc du lieu.
// Neu false (vd day I2C long, chip hong), cac ham get* ben duoi
// tra ve 0 - esp32_vehicle.ino KHONG nen coi impact_detected() la
// tin cay khi ham nay tra ve false.
bool imu_control_is_ready(void);

// Goc yaw tuyet doi rieng cua BNO055, do (0-360). Doc lap voi yaw
// camera tinh ben PC - hien chua duoc dung de fusion, chi de
// doc/log/so sanh.
float imu_control_get_yaw_deg(void);

float imu_control_get_accel_x(void);
float imu_control_get_accel_y(void);
float imu_control_get_accel_magnitude(void);

// True dung 1 chu ky doc (~20ms) ngay khi phat hien gia toc vuot
// nguong bat thuong (va cham/nhac xe len/rung manh). KHONG bi
// "giu" trang thai - tu het true khi gia toc tro lai binh thuong
// o lan doc sau.
bool imu_control_impact_detected(void);

#ifdef __cplusplus
}
#endif

#endif
