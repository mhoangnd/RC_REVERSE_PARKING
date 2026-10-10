#include "imu_control.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>


// =====================================================
// THU VIEN CAN CAI (Arduino IDE > Tools > Manage Libraries):
//   - "Adafruit BNO055" by Adafruit
//   - "Adafruit Unified Sensor" by Adafruit (thu vien phu, se
//     duoc cai kem tu dong khi cai Adafruit BNO055)
//
// DAY CAM:
//   BNO055 VIN -> 3.3V, GND -> GND
//   BNO055 SDA -> GPIO21 (I2C SDA mac dinh cua ESP32)
//   BNO055 SCL -> GPIO22 (I2C SCL mac dinh cua ESP32)
//   (dung chan I2C mac dinh nen KHONG can Wire.begin(sda,scl) -
//    goi Wire.begin() suong la du. 2 chan nay truoc gio chua
//    project nao khac dung toi.)
// =====================================================

#define BNO055_I2C_ADDRESS     0x28
#define BNO055_CHIP_ID         55

#define IMU_READ_INTERVAL_MS   20      // ~50Hz doc cam bien

// Nguong do lon gia toc ngang (m/s^2, DA TRU trong luc) de coi la
// va cham/rung manh bat thuong. Gia tri tam - PHAI do thuc te:
// cho xe chay binh thuong (tang/giam toc, vao cua) roi xem
// Serial in ra |a| lon nhat bao nhieu, dat nguong nay cao hon
// muc do mot chut.
#define IMPACT_ACCEL_THRESHOLD_MS2   6.0f


static Adafruit_BNO055 bno(BNO055_CHIP_ID, BNO055_I2C_ADDRESS);

static bool imuReady = false;
static uint32_t lastReadMs = 0;

static float yawDeg = 0.0f;
static float accelX = 0.0f;
static float accelY = 0.0f;
static float accelMag = 0.0f;
static bool impactFlag = false;


// =====================================================
// INIT
// =====================================================

void imu_control_init(void)
{
    Wire.begin();

    if (!bno.begin())
    {
        Serial.println(
            "LOI: khong tim thay BNO055 tren bus I2C "
            "(kiem tra day SDA=21/SCL=21, nguon 3.3V, dia chi 0x28)"
        );
        imuReady = false;
        return;
    }

    delay(200);

    // Dung thach anh ngoai cua module (chinh xac hon thach anh noi
    // cua chip) - hau het board BNO055 ban san deu co san.
    bno.setExtCrystalUse(true);

    imuReady = true;
    lastReadMs = millis();

    Serial.println(
        "BNO055 san sang (che do NDOF - fusion 9 truc: accel+gyro+mag)"
    );
}


// =====================================================
// HANDLE - doc dinh ky, khong doc moi vong loop() de khong
// chiem qua nhieu thoi gian bus I2C.
// =====================================================

void imu_control_handle(void)
{
    if (!imuReady)
    {
        return;
    }

    uint32_t now = millis();
    if (now - lastReadMs < IMU_READ_INTERVAL_MS)
    {
        return;
    }
    lastReadMs = now;


    // -------------------------------------------------
    // Goc yaw tuyet doi (0-360 do) tu bo fusion noi bo BNO055.
    // -------------------------------------------------

    imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
    yawDeg = euler.x();


    // -------------------------------------------------
    // Gia toc tuyen tinh: BNO055 DA TU TRU PHAN TRONG LUC (khac
    // voi gia toc "tho" doc truc tiep tu accelerometer, luon co
    // ~9.8 m/s^2 do trong luc lan trong do). Chi con lai phan gia
    // toc do xe THAT SU tang/giam toc/va cham gay ra.
    // -------------------------------------------------

    imu::Vector<3> lin = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
    accelX = lin.x();
    accelY = lin.y();
    accelMag = sqrtf(accelX * accelX + accelY * accelY);

    impactFlag = (accelMag > IMPACT_ACCEL_THRESHOLD_MS2);

    if (impactFlag)
    {
        Serial.print("IMU: PHAT HIEN VA CHAM / RUNG MANH! |a|=");
        Serial.print(accelMag);
        Serial.println(" m/s^2");
    }
}


// =====================================================
// GETTERS
// =====================================================

bool imu_control_is_ready(void)
{
    return imuReady;
}

float imu_control_get_yaw_deg(void)
{
    return yawDeg;
}

float imu_control_get_accel_x(void)
{
    return accelX;
}

float imu_control_get_accel_y(void)
{
    return accelY;
}

float imu_control_get_accel_magnitude(void)
{
    return accelMag;
}

bool imu_control_impact_detected(void)
{
    if (!imuReady)
    {
        return false;
    }

    return impactFlag;
}
