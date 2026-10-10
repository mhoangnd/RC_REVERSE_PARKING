#include <Arduino.h>
#include <string.h>

extern "C"
{
    #include "min.h"
}

#include "vehicle_command.h"
#include "host_vehicle.h"

#define MIN_PORT 0

static struct min_context min_ctx;


/* ============================================================
 * SEQUENCE
 * ============================================================ */

static uint16_t sequence = 0;


/* ============================================================
 * RESPONSE STATUS
 * ============================================================ */

static uint8_t last_status = VEHICLE_OK;


/* ============================================================
 * SEND FRAME
 * ============================================================ */

static void vehicle_send_frame(
    uint8_t min_id,
    const uint8_t *data,
    uint8_t len
)
{
    min_send_frame(
        &min_ctx,
        min_id,
        (uint8_t *)data,
        len
    );
}


/* ============================================================
 * SEND CONTROL
 * ============================================================ */

void vehicle_send_control(
    uint16_t speed,
    int16_t steering_angle
)
{
    control command;

    command.speed = speed;

    command.steering_angle = steering_angle;

    command.sequence = sequence++;


    vehicle_send_frame(
        VEHICLE_CONTROL,
        (uint8_t *)&command,
        sizeof(command)
    );
}


/* ============================================================
 * SEND STOP
 * ============================================================ */

void vehicle_send_stop(void)
{
    vehicle_send_frame(
        VEHICLE_STOP,
        NULL,
        0
    );
}


/* ============================================================
 * MIN APPLICATION HANDLER
 *
 * MIN gọi hàm này khi nhận được frame hoàn chỉnh.
 * ============================================================ */

void min_application_handler(
    uint8_t min_id,
    uint8_t const *min_payload,
    uint8_t len_payload,
    uint8_t port
)
{
    /* --------------------------------------------------------
     * STM32 RESPONSE
     * -------------------------------------------------------- */

    if (min_id == VEHICLE_RESPONSE_ID)
    {
        vehicle_response response;


        /* Kiểm tra độ dài */

        if (len_payload != sizeof(vehicle_response))
        {
            return;
        }


        /* Copy payload */

        memcpy(
            &response,
            min_payload,
            sizeof(vehicle_response)
        );


        /* Lưu trạng thái */

        last_status = response.status;


        /* Debug */

        Serial.print("STM32 response - command: ");
        Serial.print(response.command_id);

        Serial.print(" status: ");
        Serial.print(response.status);

        Serial.print(" sequence: ");
        Serial.println(response.sequence);
    }
}


/* ============================================================
 * HANDLE
 * ============================================================ */

void host_vehicle_handle(void)
{
    uint8_t data;


    while (Serial2.available() > 0)
    {
        data = Serial2.read();

        min_poll(
            &min_ctx,
            &data,
            1
        );
    }
}


/* ============================================================
 * INIT
 * ============================================================ */

void host_vehicle_init(void)
{
    min_init_context(
        &min_ctx,
        MIN_PORT
    );


    Serial2.begin(115200);


    sequence = 0;

    last_status = VEHICLE_OK;
}


/* ============================================================
 * GET SEQUENCE
 * ============================================================ */

uint16_t vehicle_get_sequence(void)
{
    return sequence;
}


/* ============================================================
 * GET LAST STATUS
 * ============================================================ */

uint8_t vehicle_get_last_status(void)
{
    return last_status;
}
