#ifndef UDP_H
#define UDP_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t calculateChecksum(
    uint8_t* data,
    int length
);

int16_t readInt16LE(
    uint8_t* data
);

uint16_t readUInt16LE(
    uint8_t* data
);

bool decodePacket(
    uint8_t* packet,
    int length
);

void udp_receiver_init(void);

void udp_receiver_handle(void);

bool udp_is_fresh(void);

uint16_t udp_get_speed(void);

uint16_t udp_get_lui(void);

uint16_t udp_get_phanh(void);

int16_t udp_get_steering_angle(void);

#ifdef __cplusplus
}
#endif

#endif
