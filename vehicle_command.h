typedef enum
{
  VEHICLE_CONTROL = 0x10,
  VEHICLE_STOP = 0x11,

  VEHICLE_RESPONSE_ID = 0x12,
}Vehicle_id;

typedef struct
{
  uint16_t speed;
  int16_t steering_angle;
  uint16_t sequence;
} __attribute__((packed)) control;

typedef enum
{
  VEHICLE_OK = 0,
  VEHICLE_INVALID_LENGTH = 1,
  VEHICLE_INVALID_VALUE = 2
}Vehicle_ack_status;

typedef struct
{
  uint8_t command_id;
  uint8_t status;
  uint16_t sequence;
} __attribute__((packed)) vehicle_response;
