#include "udp.h"
#include "auto_control.h"        // Lien ket voi PC chay Hybrid A* + Pure Pursuit
#include "servor_control.h"
#include "esc_control.h"     // Xuat tin hieu truc tiep ra mach cau H
#include "imu_control.h"         // MOI: BNO055 - yaw + gia toc, phat hien va cham

void setup()
{
    Serial.begin(115200);

    udp_receiver_init();          // WiFi (blocking den khi ket noi) + UDP vo lang/ban dap, cong 5005
    auto_control_init();          // UDP rieng cho PC lui chuong, cong 5006 - PHAI chay SAU khi WiFi da co
    servo_control_init();
    esc_control_init();
    imu_control_init();           // MOI: I2C SDA=21/SCL=21 - xem chu thich dau imu_control.cpp
}

void loop()
{
    udp_receiver_handle();
    auto_control_handle();
    imu_control_handle();         // MOI: doc dinh ky ben trong (~50Hz), goi moi vong loop cung duoc

    static uint32_t last_time = 0;
    if (millis() - last_time >= 100)
    {
        last_time = millis();

        if (imu_control_is_ready() && imu_control_impact_detected())
        {
            // ---------------------------------------------
            // UU TIEN CAO NHAT: BNO055 vua bao gia toc bat
            // thuong (va cham / bi nhac len / rung manh) ->
            // dung xe ngay lap tuc, BO QUA moi lenh dang co
            // (ke ca lenh tay that). Day la lop an toan THEM,
            // khong thay the udp_is_fresh()/auto_control_is_active().
            // ---------------------------------------------

            servo_set_steering(0);
            esc_set_throttle(0, 0, 0);
        }
        else if (udp_is_fresh())
        {
            // ---------------------------------------------
            // Uu tien nhi: co nguoi dang cam vo lang/dap ga
            // that -> nghe theo, bat ke PC dang gui lenh
            // lui-chuong gi.
            // ---------------------------------------------

            uint16_t speed = udp_get_speed();
            int16_t angle = udp_get_steering_angle();
            uint16_t lui = udp_get_lui();
            uint16_t phanh = udp_get_phanh();

            // Xu ly toan bo tren ESP32, khong gui di dau ca.
            servo_set_steering(angle);
            esc_set_throttle(speed, lui, phanh);
        }
        else if (auto_control_is_active())
        {
            // ---------------------------------------------
            // Khong co lenh tay -> dung lenh tu he thong lui
            // chuong tu dong tren PC (da quy doi san sang don
            // vi ma servor_control/hbridge_control dang dung).
            // ---------------------------------------------

            uint16_t ga, lui, phanh;
            auto_control_get_throttle(&ga, &lui, &phanh);

            servo_set_steering(auto_control_get_steer_count());
            esc_set_throttle(ga, lui, phanh);
        }
        else
        {
            // ---------------------------------------------
            // FAILSAFE: khong nguon lenh nao con "song" (ca
            // vo lang lan PC deu mat/qua han) -> dung xe han.
            // ---------------------------------------------

            servo_set_steering(0);
            esc_set_throttle(0, 0, 0);
        }
    }
}
