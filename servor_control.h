#ifndef SERVOR_CONTROL_H
#define SERVOR_CONTROL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void servo_control_init(void);

void servo_set_steering(int16_t angle);

void servo_set_speed(uint16_t speed);

#ifdef __cplusplus
}
#endif

#endif
