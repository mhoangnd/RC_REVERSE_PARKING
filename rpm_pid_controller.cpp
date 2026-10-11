#include "rpm_pid_controller.h"

//Constructor
PID_Controller::PID_Controller(float kP, float kI, float kD){
    this->kP = kP;
    this->kI = kI;
    this->kD = kD;
    prevMeasurement = 0.0f;
    integral = 0.0f;
    initialized = false;
}
//NOTE: chưa xử lí Integral Windup và Derivative Kick
float PID_Controller::PID_Compute(float setpoint, float measurement, float dt){
    float error = setpoint - measurement;
    float prevError = setpoint - prevMeasurement;
    if(dt<=0.0f){
        return 0.0f; //return PID output = 0 if time error occured
    }
    //P
    float P = error*kP;
    //I
    integral += error*dt;
    float I = integral*kI;
    //D
    float derivative = 0.0f;
    if(initialized){
        derivative = (error - prevError)/dt;
    }    
    float D = derivative*kD;
    
    //Save status for next calculation
    prevMeasurement = measurement;
    initialized = true;

    float PID_output = P + I + D;
    return PID_output;
}