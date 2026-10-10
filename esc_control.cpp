#include "esc_control.h"

#include <ESP32Servo.h>


// =====================================================
// CHAN TIN HIEU ESC
// (khac GPIO18/19 dang dung cho servor_control, tranh
//  trung LEDC channel/pin voi speedServo ben do)
// =====================================================

#define ESC_PIN                  21


// =====================================================
// XUNG PWM CHUAN ESC RC: 1000-2000us, 50Hz
// 1500us = trung tam (dung yen / khong ga khong lui)
// =====================================================

#define PULSE_MIN_US              1000
#define PULSE_NEUTRAL_US          1500
#define PULSE_MAX_US              2000

// Bien do toi da tinh tu trung tam (us).
// ga=100%  -> 1500 + 500 = 2000us (het ga)
// lui=100% -> 1500 - 500 = 1000us (het lui)
#define PULSE_SWING_US            500


static Servo escServo;


static uint16_t clampPercent(uint16_t value)
{
    if (value > 100)
    {
        return 100;
    }

    return value;
}


// =====================================================
// INIT
// =====================================================

void esc_control_init(void)
{
    escServo.setPeriodHertz(50);
    escServo.attach(
        ESC_PIN,
        PULSE_MIN_US,
        PULSE_MAX_US
    );

    // Hau het ESC yeu cau nhan tin hieu NEUTRAL ngay khi
    // cap nguon trong vai giay de "arm" (nhan tin hieu hop le),
    // neu gui ga ngay se bi ESC tu choi khong chay.
    escServo.writeMicroseconds(PULSE_NEUTRAL_US);
}


// =====================================================
// SET THROTTLE
//
// ga, lui, phanh: 0-100 (%), da chuan hoa san tu
// ban vo lang/ban dap (xem scaleToPercent() ben Arduino
// doc vo lang - gia tri toi day da nam trong 0-100 roi,
// khong can hieu chinh ADC gi them o day).
//
// Uu tien: phanh > ga/lui. Neu dap phanh (bat ke ga/lui
// the nao) se luon tra ve trung tam de ngat luc keo -
// day la lua chon an toan, hoat dong voi hau het ESC pho
// thong (khong dua vao co che "phanh chu dong" rieng cua
// tung hang ESC, vi moi hang lam khac nhau).
// =====================================================

void esc_set_throttle(uint16_t ga, uint16_t lui, uint16_t phanh)
{
    ga = clampPercent(ga);
    lui = clampPercent(lui);
    phanh = clampPercent(phanh);

    uint16_t pulse;

    if (phanh > 0)
    {
        pulse = PULSE_NEUTRAL_US;
    }
    else if (ga > 0)
    {
        pulse = PULSE_NEUTRAL_US +
                ((uint32_t)ga * PULSE_SWING_US) / 100;
    }
    else if (lui > 0)
    {
        pulse = PULSE_NEUTRAL_US -
                ((uint32_t)lui * PULSE_SWING_US) / 100;
    }
    else
    {
        pulse = PULSE_NEUTRAL_US;
    }

    escServo.writeMicroseconds(pulse);
}
