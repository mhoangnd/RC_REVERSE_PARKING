#ifndef RPM_PID
#define RPM_PID

class PID_Controller
{
public:
PID_Controller(float kP, float kI, float kD);
float PID_Compute(float setpoint, float measurement, float dt);

private:
float kP;
float kI;
float kD;

float prevMeasurement; //for kD calculation, de/dt
float integral; //for kP calculation, error accumulation over time

bool initialized; //previous data has already existed ? 1 : 0
};

#endif