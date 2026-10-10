#include "servor_control.h"

#include <ESP32Servo.h>


// =====================================================
// CHAN TIN HIEU (thay doi neu chan khac bi trung/ban)
// =====================================================

#define STEERING_SERVO_PIN     18   // servo lai
#define SPEED_SERVO_PIN        19   // ESC / servo ga (bo qua neu chua dung toi)


// =====================================================
// DAI GOC LAI HE THONG NHAN
//
// -360 -> het trai
//    0 -> giua
// +360 -> het phai
//
// (giu dung dai da kiem chung o ban STM32 truoc do,
//  neu doi lai nho sua ca ben Python/gui du lieu cho khop)
// =====================================================

#define STEERING_MIN            (-360)
#define STEERING_MAX            (360)


// =====================================================
// DAI TOC DO HE THONG NHAN: 0 -> 100
// =====================================================

#define SPEED_MIN                0
#define SPEED_MAX                100


// =====================================================
// XUNG PWM SERVO (microsecond) - chuan servo/ESC 50Hz
// =====================================================

#define PULSE_MIN_US             1000
#define PULSE_CENTER_US          1500
#define PULSE_MAX_US             2000


static Servo steeringServo;
static Servo speedServo;


// =====================================================
// MAP GIA TRI: clamp ve dai hop le truoc, roi noi suy
// tuyen tinh sang do rong xung (giong logic ben STM32)
// =====================================================

static uint16_t map_value(
    int32_t value,
    int32_t in_min,
    int32_t in_max,
    int32_t out_min,
    int32_t out_max
)
{
    if (value < in_min)
    {
        value = in_min;
    }

    if (value > in_max)
    {
        value = in_max;
    }

    return (uint16_t)(
        out_min +
        ((int32_t)(value - in_min) * (out_max - out_min)) /
        (in_max - in_min)
    );
}


// =====================================================
// INIT
// =====================================================

void servo_control_init(void)
{
    steeringServo.setPeriodHertz(50);
    steeringServo.attach(
        STEERING_SERVO_PIN,
        PULSE_MIN_US,
        PULSE_MAX_US
    );
    steeringServo.writeMicroseconds(PULSE_CENTER_US);

    speedServo.setPeriodHertz(50);
    speedServo.attach(
        SPEED_SERVO_PIN,
        PULSE_MIN_US,
        PULSE_MAX_US
    );
    speedServo.writeMicroseconds(PULSE_MIN_US);
}


// =====================================================
// SET STEERING
// =====================================================

void servo_set_steering(int16_t angle)
{
    uint16_t pulse =
        map_value(
            angle,
            STEERING_MIN,
            STEERING_MAX,
            PULSE_MIN_US,
            PULSE_MAX_US
        );

    steeringServo.writeMicroseconds(pulse);
}


// =====================================================
// SET SPEED
// =====================================================

void servo_set_speed(uint16_t speed)
{
    uint16_t pulse =
        map_value(
            speed,
            SPEED_MIN,
            SPEED_MAX,
            PULSE_MIN_US,
            PULSE_MAX_US
        );

    speedServo.writeMicroseconds(pulse);
}
