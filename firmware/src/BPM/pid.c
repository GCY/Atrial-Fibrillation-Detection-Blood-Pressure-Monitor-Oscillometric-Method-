#include "pid.h"

static float Kp,Ki,Kd;
static float *input,*output;
static float *setpoint;

static float integral;
static float last_input;

static float min,max;

static unsigned long last_time;
static unsigned long sample_time;

static bool on_off;
static bool direction;

void Init_PID(uint32_t micros,float *_input,float *_output,float *_setpoint,
      float _Kp,float _Ki,float _Kd,
      uint32_t _sample_time,
      bool _on_off,float _direction,
      float _min,float _max)
{
   input = _input;
   output = _output;
   setpoint = _setpoint;

   sample_time = _sample_time;

   on_off = _on_off;

   integral = 0;

   last_input = 0;

   SetOutputLimits(_min,_max);

   SetDirection(_direction);

   SetTunings(_Kp,_Ki,_Kd);

   last_time = (micros - sample_time);
}

void Computing(uint32_t micros)
{
   if(on_off == OFF){
      return ;
   }

   unsigned long now = micros;

   if((now - last_time) >= sample_time){
      float error = (*setpoint - *input);

      integral += (Ki * error);
      if(integral > max){
	 integral = max;
      }
      else if(integral < min){
	 integral = min;
      }

      float diff_input = (*input - last_input);

      *output = (Kp * error) + integral - (Kd * diff_input);

      if(*output > max){
	 *output = max;
      }
      else if(*output < min){
	 *output = min;
      }

      last_input = *input;
      last_time = now;
   }
}

void SetTunings(float _Kp,float _Ki,float _Kd)
{
   if((_Kp < 0) || (_Ki < 0) || (_Kd < 0)){
      return ;
   }

   float SampleTimeInSec = ((float)sample_time) / 1000000.0f;

   Kp = _Kp;
   Ki = _Ki * SampleTimeInSec;
   Kd = _Kd / SampleTimeInSec;

   if(direction != REVERSE){
      Kp = (0 - Kp);
      Ki = (0 - Ki);
      Kd = (0 - Kd);
   }
}

void SetSampleTime(unsigned long _sample_time)
{
   if(_sample_time > 0){
      float ratio = ((float)_sample_time / (float)sample_time);

      Ki *= ratio;
      Kd /= ratio;

      sample_time = _sample_time;
   }
}

void SetOutputLimits(float _min,float _max)
{
   if(_min > _max){
      return ;
   }

   min = _min;
   max = _max;

   if(on_off == ON){
      if(*output > max){
	 *output = max;
      }
      else if(*output < min){
	 *output = min;
      }

      if(integral > max){
	 integral = max;
      }
      else if(integral < min){
	 integral = min;
      }
   }
}

void SetOnOff(bool _on_off)
{
   bool new_state = (_on_off == ON);

   if(new_state == (!on_off)){
      integral = *output;
      last_input = *input;

      if(integral > max){
	 integral = max;
      }
      else if(integral < min){
	 integral = min;
      }
   }

   on_off = new_state;
}

void SetDirection(bool _direction)
{
   if((on_off == ON) && (_direction != direction)){
      Kp = (0 - Kp);
      Ki = (0 - Ki);
      Kd = (0 - Kd);
   }

   direction = _direction;
}
