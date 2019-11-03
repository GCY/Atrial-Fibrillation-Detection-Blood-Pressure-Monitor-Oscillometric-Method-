#ifndef __PID__
#define __PID__

#include <stdint.h>
#include <stdbool.h>

#define DIRECT true
#define REVERSE false

#define ON true
#define OFF false


void Init_PID(uint32_t,float*,float*,float*,float,float,float,unsigned long,bool,float,float,float);
void Computing(uint32_t);
void SetTunings(float,float,float);
void SetSampleTime(unsigned long);
void SetOutputLimits(float,float);
void SetOnOff(bool);
void SetDirection(bool);

#endif
