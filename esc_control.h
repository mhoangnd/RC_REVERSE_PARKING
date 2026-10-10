#ifndef ESC_CONTROL_H
#define ESC_CONTROL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void esc_control_init(void);

// ga, lui, phanh: phan tram 0-100, da chuan hoa san tu ban vo lang/ban dap
void esc_set_throttle(uint16_t ga, uint16_t lui, uint16_t phanh);

#ifdef __cplusplus
}
#endif

#endif
