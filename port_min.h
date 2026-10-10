#ifndef PORT_MIN_H
#define PORT_MIN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void min_tx_start(uint8_t port);
void min_tx_finished(uint8_t port);
void min_tx_byte(uint8_t port, uint8_t byte);

uint16_t min_tx_space(uint8_t port);

uint32_t min_time_ms(void);

#ifdef __cplusplus
}
#endif

#endif
