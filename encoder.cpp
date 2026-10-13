#include <Arduino.h>
#include <stdint.h>
#include "encoder.h"

#define ENCODER_PHASE_A 16 //đọc rpm bằng kênh A
#define ENCODER_PHASE_B 17 //đọc chiều quay bằng kênh B

#define FORWARD 1
#define REVERSE -1


//PRIVATE VARIABLE
static volatile uint32_t edgeCount = 0; //đếm lần phát hiện cạnh ngắt
static uint32_t prevCount = 0;

static uint32_t lastTime = 0;
static float latestRPM = 0.0f;
static volatile int8_t direction;

//INTERRUPT SERVICE FUNCTION
static void IRAM_ATTR encoder_isr(){ //IRAM_ATTR giúp ghi vào RAM thay vì flash, tối ưu xử lý ngắt
    edgeCount++;

    if(digitalRead(ENCODER_PHASE_A) == digitalRead(ENCODER_PHASE_B)){
        direction = FORWARD;
    }
    else direction = REVERSE;
}

void encoder_init(){
    pinMode(ENCODER_PHASE_A, INPUT_PULLUP);
    pinMode(ENCODER_PHASE_B, INPUT_PULLUP);

    edgeCount = 0;
    prevCount = 0;
    latestRPM = 0.0f;
    direction = 0;
    lastTime = millis();

    attachInterrupt(digitalPinToInterrupt(ENCODER_PHASE_A), encoder_isr, CHANGE);
}

/* additional variables such as 'now' or 'currentCount' are used for capture the moment when 
this function is called, prevent ISR from intervening the calculations of this function. */
float encoder_get_rpm(){ //độ phân giải: 100ms; đĩa encoder 11 xung
    float rpm = latestRPM;
    uint32_t now = millis();
    uint32_t currentCount =  edgeCount;
    uint32_t elapsed = now - lastTime;
    if(elapsed >= 100){
        uint32_t deltaCount = currentCount - prevCount;
        rpm = (60000.0f * deltaCount)/(22.0f*elapsed);
        lastestRPM = rpm;

        //update prev variables
        prevCount = currentCount;
        lastTime = now;
    }
    return rpm;
}

int encoder_get_direction(){
    return direction;
}
