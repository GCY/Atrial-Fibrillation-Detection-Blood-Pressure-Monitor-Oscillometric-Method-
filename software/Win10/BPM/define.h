#ifndef __DEFINE__
#define __DEFINE__

#include <stdint.h>

//const uint32_t SAMPLING_RATE = 1000;
const uint32_t SAMPLING_RATE = 100;
//const uint32_t SAMPLING_RATE = 360;

typedef struct
{
   float value;
   int32_t index;
}SignalPoint;


#endif
