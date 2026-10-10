

#ifndef HOST_VEHICLE_H
#define HOST_VEHICLE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

static void vehicle_send_frame(
    uint8_t min_id,
    const uint8_t *data,
    uint8_t len
);

void vehicle_send_control(
    uint16_t speed,
    int16_t steering_angle
);

void vehicle_send_stop(void);

void host_vehicle_handle(void);

void host_vehicle_init(void);

uint16_t vehicle_get_sequence(void);

uint8_t vehicle_get_last_status(void);

#ifdef __cplusplus
}
#endif

#endif
