#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#include <stm32f4xx.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_usart.h>
#include <stm32f4xx_exti.h>
#include <stm32f4xx_i2c.h>
#include <stm32f4xx_adc.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_dma.h>
#include <stm32f4xx_rtc.h>
#include <stm32f4xx_tim.h>

#include "define.h"

#include "I2C.h"
#include "SSD1306.h"
#include "VCP.h"

#include "FIR.h"
#include "adaptive_algorithm.h"

#include "adc.h"

#include "tiny_printf.h"

#include "pid.h"

#define __FPU_PRESENT
#define __FPU_USED

enum{
   DATA_COLLECTION_MODE = 0,
   CALIBRATION_MODE,
   MERCURY_METER_MODE,
   BPM_MODE,
   AF_MODE
};

const uint32_t SECOND = 1000000;

const uint16_t PWM_Freq = 1000;

volatile uint32_t TimingDelay;

volatile uint32_t micros = 20000000;

float as_am_value = 0.65f;
float ad_am_value = 0.7f;

float a[3] = {-0.0000012119f,0.1262915457f,-1.0620516546f};

uint32_t PRESSURE_MIN = 50;
uint32_t PRESSURE_MAX = 160;
const uint32_t STOP_PRESSURE = 180;

uint32_t last_update;
uint32_t last_screen_update;

volatile uint32_t mode = DATA_COLLECTION_MODE;
volatile bool is_pressurize = false;
volatile bool is_leak = false;

const uint32_t CALIBRATION_PRESSURIZE_TIME = 300000;	//300ms
const uint32_t CALIBRATION_LEAK_TIME = 120000;	//120ms
const uint32_t CALIBRATION_HIGH_SLOP_THRESHOLD = 1500;

bool measure_flag = false;
bool calibration_flag = false;

float last_pressure = 0;
float diff_pressure = 0;
float pwm = 0,setpoint = 3;
float Kp = 2, Ki = 0.3, Kd = 0.001;

const uint32_t PID_PWM_MIN = 1;
const uint32_t PID_PWM_MAX = 100;
const uint32_t TIMER_PWM_MIN = 100000;
const uint32_t TIMER_PWM_MAX = 1000000;
const uint32_t TIMER_PWM_33 = 330000;
const uint32_t TIMER_PWM_40 = 400000;

float baseline = 0;
float dc = 0;
uint32_t init_baseline_count = 0;  

float pressure = 0; // mmHg

float MAP;
float SBP;
float DBP;

static const uint32_t MEASURMENT_TIME = 35;
float dc_array[70];	//size = MEASURMENT_TIME * 2
float ac_array[70];	//size = MEASURMENT_TIME * 2
uint32_t index_array[70];	//size = MEASURMENT_TIME * 2
uint32_t pulse_index;

volatile uint32_t last_AF = 0;
volatile uint32_t AF_counter = 0;
const uint32_t AF_PERIOD = 8; //SENCOND
uint32_t IPP = 0;
uint32_t IHB = 0;
const float IPP_Ratio = 0.2f;
const float IHB_Ratio = 0.2f;

const unsigned int taps = 19;
/*100hz 0.7Hz~8.9Hz*/
float coeff1[19] = {-0.043047,-0.048389,-0.042549,-0.023677,0.0073069,0.046611,0.088179,0.12489,0.15008,0.15905,0.15008,0.12489,0.088179,0.046611,0.0073069,-0.023677,-0.042549,-0.048389,-0.043047};
/* 100Hz 0.5Hz low pass*/
float coeff2[19] = {0.050186,0.050989,0.051705,0.052331,0.052864,0.053303,0.053646,0.053892,0.05404,0.054089,0.05404,0.053892,0.053646,0.053303,0.052864,0.052331,0.051705,0.050989,0.050186};
float buffer1[19];
unsigned offset1;
float buffer2[19];
unsigned offset2;

FIRInfo info1;
FIRInfo info2;

uint32_t time_to_index = 0;

uint32_t detect_time = 0;

bool mercury_meter_flag = false;

void Delay(__IO uint32_t nTime)
{
   TimingDelay = nTime;
   while(TimingDelay){
   }
}

void SysTick_Handler(void)
{
   if(TimingDelay){
      --TimingDelay;
   }
   ++micros;
}

void Init_LED()
{
   //Enable the GPIOD Clock
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);


   // GPIOD Configuration
   GPIO_InitTypeDef GPIO_InitStruct;
   GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
   GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;

   //GPIO_SetBits(GPIOC, GPIO_Pin_12);
   //GPIO_SetBits(GPIOC, GPIO_Pin_13);
   GPIO_Init(GPIOC, &GPIO_InitStruct);   
}

void EXTILine0_Config(void)
{
   EXTI_InitTypeDef   EXTI_InitStructure;
   GPIO_InitTypeDef   GPIO_InitStructure;
   NVIC_InitTypeDef   NVIC_InitStructure;

   /* Enable GPIOB clock */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
   /* Enable SYSCFG clock */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

   /* Configure PB0 pin as input floating */
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
   GPIO_Init(GPIOA, &GPIO_InitStructure);

   /* Connect EXTI Line0 to PB0 pin */
   SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

   /* Configure EXTI Line0 */
   EXTI_InitStructure.EXTI_Line = EXTI_Line0;
   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;  
   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
   EXTI_Init(&EXTI_InitStructure);

   /* Enable and set EXTI Line0 Interrupt to the lowest priority */
   NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
}

void EXTILine1_Config(void)
{
   EXTI_InitTypeDef   EXTI_InitStructure;
   GPIO_InitTypeDef   GPIO_InitStructure;
   NVIC_InitTypeDef   NVIC_InitStructure;

   /* Enable GPIOB clock */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
   /* Enable SYSCFG clock */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

   /* Configure PB0 pin as input floating */
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
   GPIO_Init(GPIOA, &GPIO_InitStructure);

   /* Connect EXTI Line0 to PB0 pin */
   SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource1);

   /* Configure EXTI Line0 */
   EXTI_InitStructure.EXTI_Line = EXTI_Line1;
   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;  
   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
   EXTI_Init(&EXTI_InitStructure);

   /* Enable and set EXTI Line0 Interrupt to the lowest priority */
   NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
}

void Init_PWM()
{
   //Enable the GPIOD Clock
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);


   // GPIOD Configuration
   GPIO_InitTypeDef GPIO_InitStruct;
   GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
   GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;

   GPIO_Init(GPIOC, &GPIO_InitStruct);   

   //GPIO_SetBits(GPIOC, GPIO_Pin_8);
   GPIO_ResetBits(GPIOC, GPIO_Pin_8);
}

void Init_TB6612FNG()
{
   //Enable the GPIOD Clock
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);


   // GPIOD Configuration
   GPIO_InitTypeDef GPIO_InitStruct;
   GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
   GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;

   GPIO_Init(GPIOB, &GPIO_InitStruct); 

   GPIO_SetBits(GPIOB, GPIO_Pin_6);
   GPIO_ResetBits(GPIOB, GPIO_Pin_7);
   //GPIO_SetBits(GPIOB, GPIO_Pin_8);
   GPIO_ResetBits(GPIOB, GPIO_Pin_8);
   GPIO_ResetBits(GPIOB, GPIO_Pin_9);
   GPIO_SetBits(GPIOB, GPIO_Pin_10);
   GPIO_SetBits(GPIOB, GPIO_Pin_11);
}

void MOTOR_PWM_Config(uint16_t PWM_Freq)
{
   TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
   TIM_OCInitTypeDef  TIM_OCInitStructure;	
   GPIO_InitTypeDef GPIO_InitStructure;

   uint16_t PrescalerValue = 0;

   /* TIM3 clock enable */
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

   /* GPIOC clock enable */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);


   /* GPIOC Configuration: TIM3 CH3 (PC8) */
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
   GPIO_Init(GPIOB, &GPIO_InitStructure); 

   /* Connect TIM3 pins to AF2 */
   GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_TIM3);

   /* Compute the prescaler value */
   PrescalerValue = (uint16_t) ((SystemCoreClock /2) / 1000000) - 1; // (168000000/2)/1000000 = 84

   /* Time base configuration */
   TIM_TimeBaseStructure.TIM_Period = TIMER_PWM_MAX/PWM_Freq;
   TIM_TimeBaseStructure.TIM_Prescaler = PrescalerValue;
   TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
   TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

   /* PWM1 Mode configuration: Channel3 */
   TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
   //TIM_OCInitStructure.TIM_Pulse = 500000/PWM_Fren; //50% duty cycle
   TIM_OCInitStructure.TIM_Pulse = 0;
   TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Inactive;

   TIM_OC3Init(TIM3, &TIM_OCInitStructure);
   TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
   TIM_ARRPreloadConfig(TIM3, ENABLE);

   /* TIM3 enable counter */
   //TIM_Cmd(TIM3, ENABLE);	


}

void MOTOR_PWM_out(uint16_t PWM_Freq)
{

   if(PWM_Freq){
      //TIM_Cmd(TIM3, DISABLE);
      TIM3->ARR = (TIMER_PWM_MAX/PWM_Freq)-1; //TIM_Period
      TIM3->CCR2 = 0; //TIM_Pulse
      TIM_SelectOCxM(TIM3, TIM_Channel_2, TIM_OCMode_PWM1);
      TIM_CCxCmd(TIM3, TIM_Channel_2, TIM_CCx_Enable);
      TIM3->EGR = TIM_EventSource_Update;
      TIM_Cmd(TIM3, ENABLE);	 
   }
   else{
      TIM_Cmd(TIM3, DISABLE);
      TIM_SelectOCxM(TIM3, TIM_Channel_2, TIM_ForcedAction_InActive);
      TIM_CCxCmd(TIM3, TIM_Channel_2, TIM_CCx_Enable);		 
   }

}

void Init_Peripheral()
{
   USBVCPInit();
   Delay(10000);

   Init_LED();
   EXTILine0_Config();
   EXTILine1_Config();
   Delay(10000);

   Init_I2C3();
   Init_SSD1306();
   Delay(10000);

   //Init_ADC(); //ADC3 Pooling
   ADC_4_Channel_Init(); // DMA + TIMER + FIFO, 1000Hz sampling rate
   Delay(10000);


   Init_PWM();

   Init_TB6612FNG();
   Delay(10000);
   MOTOR_PWM_Config(PWM_Freq);
   Delay(10000);
   MOTOR_PWM_out(PWM_Freq);
   Delay(10000);

   /* if external clock is work */
   __IO uint32_t HSEStatus = 0;
   HSEStatus = RCC->CR & RCC_CR_HSERDY;
   if(HSEStatus){
      GPIO_SetBits(GPIOC, GPIO_Pin_12);
   }
   else{
      GPIO_ResetBits(GPIOC, GPIO_Pin_12);
   }     

   TIM3->CCR2 = 0;	//Motor = off
   GPIO_ResetBits(GPIOB, GPIO_Pin_8);	//VALVE = open
}

////////////////
long map(long x, long in_min, long in_max, long out_min, long out_max){
   return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void IPP_Detect()
{
   float pulse_sum = 0;
   float pulse = 0;
   for(int i = 1;i < pulse_index;++i){
      pulse_sum += (1000.0f / ((index_array[i] - index_array[i-1]) * (1000.0f/(float)SAMPLING_RATE)) * 60.0f);
   }

   if(pulse_index > 2){
      pulse = pulse_sum / (pulse_index-1);
   }   
   else{
      return;
   }

   for(int i = 1;i < pulse_index;++i){
      float ratio = fabs(1.0f - ((1000.0f / ((index_array[i] - index_array[i-1]) * (1000.0f/(float)SAMPLING_RATE)) * 60.0f) / pulse));
      if(ratio > IPP_Ratio){
	 ++IPP;
      }
   }

   if( ((float)IPP/(float)(pulse_index-1)) > IHB_Ratio ){
      ++IHB;
   }
}

void ResetMeasurementParameter()
{

   FIR_reset_buffer(&info1);
   FIR_reset_buffer(&info2);

   last_update = micros;
   calibration_flag = true;

   last_pressure = 0;
   diff_pressure = 0;
   pwm = 0;
   //Kp=1.3, Ki=0.51, Kd=0.003;
   //Kp = 2.6, Ki = 0.2, Kd = 0.002;

   if(USB_VCP_GetStatus() == USB_VCP_CONNECTED){
      //Kp = 2.6, Ki = 0.33, Kd = 0.001;
      //Kp = 2.5, Ki = 0.33, Kd = 0.0006;

      //Kp = 2.6, Ki = 0.3, Kd = 0.002;
      Kp = 2, Ki = 0.3, Kd = 0.001;
   }
   else{
      //Kp = 2.6, Ki = 0.3, Kd = 0.002;
      Kp = 2, Ki = 0.3, Kd = 0.001;
   }

   //Kp = 4, Ki=0.5, Kd=0.00000;
   //Kp = 2.6, Ki=0.33, Kd=0.001;
  /* 
      if(USB_VCP_GetStatus() == USB_VCP_CONNECTED){
	Kp = 4, Ki=0.5, Kd=0.00000;
	}
	else{
	//Kp = 2.6, Ki=0.33, Kd=0.001;
	Kp = 4, Ki=0.5, Kd=0.00000;
	}
*/

   baseline = 0;
   init_baseline_count = 0;	
   dc = 0;

   setpoint = 3; 
   pressure = 0;

   MAP = 0;
   SBP = 0;
   DBP = 0;

   for(int i = 0;i < pulse_index;++i){
      dc_array[i] = 0;
      ac_array[i] = 0;
      index_array[i] = 0;
   }
   pulse_index = 0;

   info1.taps = taps;
   info1.coeff = coeff1;
   info1.buffer = buffer1;
   info1.offset = offset1 = 0;

   info2.taps = taps;
   info2.coeff = coeff2;
   info2.buffer = buffer2;
   info2.offset = offset2 = 0;

   time_to_index = 0;

}

void Measurement(uint32_t is_usb_mode)
{


   if(calibration_flag){
      if(micros - last_update > (2*SECOND)){
	 measure_flag = true;

	 GPIO_SetBits(GPIOB, GPIO_Pin_8);

	 calibration_flag = false;

	 dc = baseline / init_baseline_count;

	 Init_PID(micros,&diff_pressure, &pwm, &setpoint, Kp, Ki, Kd,SECOND / SAMPLING_RATE,ON,REVERSE,PID_PWM_MIN,PID_PWM_MAX);
	 TIM3->CCR2 = TIMER_PWM_33/PWM_Freq;
      }
      else{
      }
   }

   if(ADC3_ready && (measure_flag | calibration_flag) && is_usb_mode == DATA_COLLECTION_MODE){
      ADC3_ready = false;
      unsigned char str[255];

      sprintf(str,"R%d,%d,%d,%d,%d,",micros, ADC3_value[0], ADC3_value[1], ADC3_value[2], ADC3_value[3]);
      if(calibration_flag){
	 baseline += FIR_filter(ADC3_value[0],&info2);
	 ++init_baseline_count;
      }
      else{
	 //float value_dc = ADC3_value[0] - dc;
	 float value_dc = FIR_filter(ADC3_value[0],&info2) - dc;

	 pressure = (a[0] * (value_dc*value_dc)) + (a[1] * value_dc) + a[2]; //174base

	 diff_pressure = pressure - last_pressure;

	 Computing(micros);

	 if(pressure > PRESSURE_MIN){
	    TIM3->CCR2 = map(pwm,PID_PWM_MIN,PID_PWM_MAX,TIMER_PWM_MIN,TIMER_PWM_MAX)/PWM_Freq;	// 1~100 = duty cycle 10%~100%
	 }
	 else{
	    TIM3->CCR2 = TIMER_PWM_33/PWM_Freq;
	 }

	 last_pressure = pressure;

      }
      USB_VCP_Puts(str);
   }
   else if(ADC3_ready && (measure_flag | calibration_flag)  && is_usb_mode == BPM_MODE){
      ADC3_ready = false;

      if(calibration_flag){
	 baseline += FIR_filter(ADC3_value[0],&info2);
	 ++init_baseline_count;
      }
      else  if(measure_flag){

	 float value = FIR_filter(ADC3_value[1],&info1);
	 float value_dc = FIR_filter(ADC3_value[0],&info2) - dc;	 

	 //pressure = (-0.000009f*(value_dc*value_dc))+(0.1361f*value_dc) - 0.661f; //78base
	 //pressure = (-0.000001f*(value_dc*value_dc))+(0.1333f*value_dc) - 0.3395f; //121base
	 pressure = (a[0] * (value_dc*value_dc)) + (a[1] * value_dc) + a[2]; //174base

	 diff_pressure = pressure - last_pressure;

	 Computing(micros);

	 if(pressure > PRESSURE_MIN){
	    TIM3->CCR2 = map(pwm,PID_PWM_MIN,PID_PWM_MAX,TIMER_PWM_MIN,TIMER_PWM_MAX)/PWM_Freq;	// 1~100 = duty cycle 10%~100%
	 }
	 else{
	    TIM3->CCR2 = TIMER_PWM_33/PWM_Freq;
	 }

	 ++time_to_index;

	 static const float CV_LIMIT = 50.0f;
	 static const float THRESHOLD_FACTOR = 3.0f;
	 float mean = CalculateMean(value);
	 float rms = CalculateRootMeanSquare(value);
	 float cv = CalculateCoefficientOfVariation(value);
	 float threshold;
	 if(cv > CV_LIMIT){
	    //threshold = rms;
	    threshold = dc;
	 }
	 else{
	    //threshold = (rms * (cv/100.0f) * THRESHOLD_FACTOR);
	    threshold = (dc * (cv/100.0f) * THRESHOLD_FACTOR);
	 }

	 bool is_peak;
	 SignalPoint result;
	 result = PeakDetect(value,time_to_index,threshold,&is_peak);
	 if(result.index != -1 && (pressure > PRESSURE_MIN && pressure < PRESSURE_MAX)){
	    if(is_peak){
	       dc_array[pulse_index] = pressure;
	       ac_array[pulse_index] = result.value;
	       index_array[pulse_index] = result.index;
	       ++pulse_index;

	       detect_time = micros;
	    }
	    else{

	    }
	 }	 

	 last_pressure = pressure;

      }

   }
   else if(ADC3_ready && is_usb_mode == AF_MODE){
      if(!measure_flag && !calibration_flag && (AF_counter > 1) && ((micros - last_AF) > (AF_PERIOD * SECOND) ) ){
	 --AF_counter;
	 ResetMeasurementParameter();
      }

      ADC3_ready = false;

      if(calibration_flag){
	 baseline += FIR_filter(ADC3_value[0],&info2);
	 ++init_baseline_count;
      }
      else if(measure_flag){

	 float value = FIR_filter(ADC3_value[1],&info1);
	 float value_dc = FIR_filter(ADC3_value[0],&info2) - dc;
	 //float value = ADC3_value[1];
	 //float value_dc = ADC3_value[0] - dc;

	 pressure = (a[0] * (value_dc*value_dc)) + (a[1] * value_dc) + a[2];

	 diff_pressure = pressure - last_pressure;

	 Computing(micros);

	 if(pressure > PRESSURE_MIN){
	    TIM3->CCR2 = map(pwm,PID_PWM_MIN,PID_PWM_MAX,TIMER_PWM_MIN,TIMER_PWM_MAX)/PWM_Freq;	// 1~100 = duty cycle 10%~100%
	 }
	 else{
	    TIM3->CCR2 = TIMER_PWM_33/PWM_Freq;
	 }

	 ++time_to_index;

	 static const float CV_LIMIT = 50.0f;
	 static const float THRESHOLD_FACTOR = 3.0f;
	 float mean = CalculateMean(value);
	 float rms = CalculateRootMeanSquare(value);
	 float cv = CalculateCoefficientOfVariation(value);
	 float threshold;
	 if(cv > CV_LIMIT){
	    threshold = dc;
	 }
	 else{
	    threshold = (dc * (cv/100.0f) * THRESHOLD_FACTOR);
	 }

	 bool is_peak;
	 SignalPoint result;
	 result = PeakDetect(value,time_to_index,threshold,&is_peak);
	 if(result.index != -1 && (pressure > PRESSURE_MIN && pressure < PRESSURE_MAX)){
	    if(is_peak){
	       dc_array[pulse_index] = pressure;
	       ac_array[pulse_index] = result.value;
	       index_array[pulse_index] = result.index;
	       ++pulse_index;

	       detect_time = micros;
	    }
	    else{

	    }
	 }	

	 last_pressure = pressure;

      }

   }

   if(measure_flag){
      if(micros - last_update > (MEASURMENT_TIME * SECOND) || pressure > STOP_PRESSURE){
	 measure_flag = false;
	 TIM3->CCR2 = 0;
	 GPIO_ResetBits(GPIOB, GPIO_Pin_8);

	 if(is_usb_mode == AF_MODE){
	    last_AF = micros;
	    IPP = 0;
	    IPP_Detect();
	 }
      }
   }	 

}

void BPCalculator()
{

   float map_ratio = 0;
   uint32_t map_index = 0;
   for(int i = 0;i < pulse_index;++i){
      float temp = ac_array[i] / dc;
      if(temp > map_ratio){
	 map_ratio = temp;
	 map_index = i;

	 MAP = dc_array[i];
      }
   }

   float dbp_error = 0;
   for(int i = map_index-1;i >= 0;--i){
      float temp = ac_array[i] / dc;
      float dbp_ratio = temp / map_ratio;
      if(fabs(dbp_ratio - ad_am_value) < dbp_error || dbp_error == 0){
	 dbp_error = fabs(dbp_ratio - ad_am_value);
	 DBP = dc_array[i];
      }
   }

   float sbp_error = 0;
   for(int i = map_index+1;i < pulse_index;++i){
      float temp = ac_array[i] / dc;
      float sbp_ratio = temp / map_ratio;
      if(fabs(sbp_ratio - as_am_value) < sbp_error || sbp_error == 0){
	 sbp_error = fabs(sbp_ratio - as_am_value);
	 SBP = dc_array[i];
      }
   }	

   float pulse_sum = 0;
   float pulse = 0;
   for(int i = 1;i < pulse_index;++i){
      pulse_sum += (1000.0f / ((index_array[i] - index_array[i-1]) * (1000.0f/(float)SAMPLING_RATE)) * 60.0f);
   }
   if(pulse_index > 2){
      pulse = pulse_sum / (pulse_index-1);
   }

   char str[255] = "";
   SSD1306_Fill(0x00);
   sprintf(str,"MAP:%dmmHg",(uint32_t)MAP);
   SSD1306_GotoXY(0, 0);
   SSD1306_Puts(str, &Font_11x18, 0xFF);	 
   sprintf(str,"SBP:%dmmHg",(uint32_t)SBP);
   SSD1306_GotoXY(0, 15);
   SSD1306_Puts(str, &Font_11x18, 0xFF);
   sprintf(str,"DBP:%dmmHg",(uint32_t)DBP);
   SSD1306_GotoXY(0, 30);
   SSD1306_Puts(str, &Font_11x18, 0xFF);		 
   sprintf(str,"PULSE:%d",(uint32_t)pulse);
   SSD1306_GotoXY(0, 45);
   SSD1306_Puts(str, &Font_11x18, 0xFF);		  
   SSD1306_UpdateScreen();

}

int main(void)
{
   if(SysTick_Config(SystemCoreClock / 1000 / 1000)){
      while(1){}
   }

   Init_Peripheral();

   SSD1306_Fill(0x00);
   SSD1306_UpdateScreen();
   Delay(SECOND);
   SSD1306_Fill(0xFF);
   SSD1306_UpdateScreen();
   Delay(SECOND);
   SSD1306_Fill(0x00);
   SSD1306_UpdateScreen();  
   Delay(SECOND);   

   SSD1306_GotoXY(3, 4);
   SSD1306_Puts("AF-BPM v2", &Font_11x18, 0xFF);
   SSD1306_GotoXY(3, 25);
   SSD1306_Puts("20190814", &Font_11x18, 0xFF);
   SSD1306_GotoXY(3, 45);
   SSD1306_Puts("TonyGUO", &Font_11x18, 0xFF);
   SSD1306_UpdateScreen();  
   Delay(SECOND);
   SSD1306_Fill(0x00);
   SSD1306_UpdateScreen();

   last_update = micros;
   last_screen_update = micros;


   while(1){

      /* USB Mode */
      if(USB_VCP_GetStatus() == USB_VCP_CONNECTED){

	 GPIO_SetBits(GPIOC, GPIO_Pin_13);

	 if(mode != DATA_COLLECTION_MODE && mode != CALIBRATION_MODE && mode != MERCURY_METER_MODE){
	    mode = DATA_COLLECTION_MODE;
	 }

	 uint8_t c;
	 if(USB_VCP_Getc(&c) == USB_VCP_DATA_OK){
	    if(c == 'm' || c == 'M'){
	       ResetMeasurementParameter();
	    }
	    else if(c == 's' || c == 'S'){
	       if(mode != DATA_COLLECTION_MODE){
		  mode = DATA_COLLECTION_MODE;
		  GPIO_ResetBits(GPIOB, GPIO_Pin_8);
	       }
	       else{
		  mode = CALIBRATION_MODE;
		  GPIO_SetBits(GPIOB, GPIO_Pin_8);
	       }
	    }
	    else if(c == 'h' || c == 'H'){
	       mercury_meter_flag ^= true;
	       if(mercury_meter_flag){
		  mode = MERCURY_METER_MODE;
	       }
	       else{
		  mode = DATA_COLLECTION_MODE;
		  GPIO_ResetBits(GPIOB, GPIO_Pin_8);		  
	       }
	    }
	    else if((mode == CALIBRATION_MODE) && (c == 'p' || c == 'P')){
	       is_pressurize = true;
	    }
	    else if((mode == CALIBRATION_MODE) && (c == 'l' || c == 'L')){
	       is_leak = true;
	    }
	 }
	 /* USB mode for algorithm finetune and RAW data collection */
	 if(mode == DATA_COLLECTION_MODE){
	    if(micros - last_screen_update > SECOND){
	       char str[255] = "";
	       sprintf(str,"%d mmHg",(uint32_t)pressure);
	       SSD1306_Fill(0x00);
	       SSD1306_GotoXY(20, 5);
	       SSD1306_Puts("USB Mode", &Font_11x18, 0xFF);	    
	       SSD1306_GotoXY(20, 35);
	       SSD1306_Puts(str, &Font_11x18, 0xFF);
	       SSD1306_UpdateScreen();
	       last_screen_update = micros;
	    }	 

	    Measurement(DATA_COLLECTION_MODE);
	 }
	 /* calibration mode with mercury manometer */
	 else if(mode == CALIBRATION_MODE){
	    if(micros - last_screen_update > SECOND){
	       char str[255] = "";
	       sprintf(str,"ADC0: %d",(uint32_t)dc);
	       SSD1306_Fill(0x00);
	       SSD1306_GotoXY(0, 5);
	       SSD1306_Puts("Calib Mode", &Font_11x18, 0xFF);	    
	       SSD1306_GotoXY(10, 35);
	       SSD1306_Puts(str, &Font_11x18, 0xFF);
	       SSD1306_UpdateScreen();
	       last_screen_update = micros;
	    }

	    if(ADC3_ready){
	       ADC3_ready = false;
	       baseline += ADC3_value[0];
	       ++init_baseline_count;	       
	    }

	    if(init_baseline_count >= SAMPLING_RATE){
	       dc = baseline / init_baseline_count;
	       baseline = 0;
	       init_baseline_count = 0;
	    }

	    if(is_pressurize){
	       if(dc < CALIBRATION_HIGH_SLOP_THRESHOLD){
		  TIM3->CCR2 = TIMER_PWM_33/PWM_Freq;
	       }
	       else{
		  TIM3->CCR2 = TIMER_PWM_40/PWM_Freq;
	       }
	       Delay(CALIBRATION_PRESSURIZE_TIME);
	       TIM3->CCR2 = 0;
	       is_pressurize = false;
	    }

	    if(is_leak){
	       GPIO_ResetBits(GPIOB, GPIO_Pin_8);
	       Delay(CALIBRATION_LEAK_TIME);
	       GPIO_SetBits(GPIOB, GPIO_Pin_8);
	       is_leak = false;
	    }
	 }
	 else if(mode == MERCURY_METER_MODE){
	    if(ADC3_ready){
	       ADC3_ready = false;
	       unsigned char str[255];

	       sprintf(str,"R%d,%d,%d,%d,%d,",micros, ADC3_value[0], ADC3_value[1], ADC3_value[2], ADC3_value[3]);	 
	       USB_VCP_Puts(str);
	    }
	 }
      }
      /* BPM Mode */
      else{

	 if(micros - detect_time < (SECOND >> 4)){
	    GPIO_SetBits(GPIOC, GPIO_Pin_13);
	 }
	 else{
	    GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	 }

	 if(mode == BPM_MODE){
	    if(micros - last_screen_update > SECOND){
	       char str[255] = "";
	       if(measure_flag || calibration_flag){
		  sprintf(str,"%d mmHg",(uint32_t)pressure);
		  SSD1306_Fill(0x00);
		  SSD1306_GotoXY(20, 5);
		  SSD1306_Puts("BPM Mode", &Font_11x18, 0xFF);	    
		  SSD1306_GotoXY(20, 27);
		  SSD1306_Puts(str, &Font_11x18, 0xFF);
		  SSD1306_UpdateScreen();
	       }
	       else{
		  BPCalculator();
	       }
	       last_screen_update = micros;
	    }	

	    Measurement(BPM_MODE);
	 }
	 else if(mode == AF_MODE){
	    if(micros - last_screen_update > SECOND){
	       char str[255] = "";
	       if(measure_flag || calibration_flag){
		  sprintf(str,"%d mmHg",(uint32_t)pressure);
		  SSD1306_Fill(0x00);
		  SSD1306_GotoXY(20, 5);
		  SSD1306_Puts("AF Mode", &Font_11x18, 0xFF);	    
		  SSD1306_GotoXY(20, 35);
		  SSD1306_Puts(str, &Font_11x18, 0xFF);
		  SSD1306_UpdateScreen();
	       }
	       else if(AF_counter > 1){
		  BPCalculator();
	       }
	       else{
		  SSD1306_Fill(0x00);
		  SSD1306_GotoXY(20, 5);
		  SSD1306_Puts("AF Result", &Font_11x18, 0xFF);	    
		  SSD1306_GotoXY(20, 27);
		  sprintf(str,"IHB: %d",IHB);
		  SSD1306_Puts(str, &Font_11x18, 0xFF);
		  if(IHB >= 2){
		     SSD1306_GotoXY(20, 45);
		     SSD1306_Puts("AF Detected", &Font_11x18, 0xFF);	     
		  }
		  else{
		     SSD1306_GotoXY(20, 45);
		     SSD1306_Puts("Safe", &Font_11x18, 0xFF);		     
		  }
		  SSD1306_UpdateScreen();	  
	       }
	       last_screen_update = micros;
	    }	

	    Measurement(AF_MODE);
	    
	 }
	 else{
	    SSD1306_Fill(0x00);
	    SSD1306_GotoXY(20, 30);
	    SSD1306_Puts("AF-BP v2", &Font_11x18, 0xFF);	    
	    SSD1306_UpdateScreen();	    
	 }

      }
   }

   return(0); // System will implode
}    

void EXTI0_IRQHandler(void)
{
   if(USB_VCP_GetStatus() != USB_VCP_CONNECTED){
      mode = BPM_MODE;
      ResetMeasurementParameter();
   }

   EXTI_ClearFlag(EXTI_Line0);
}

void EXTI1_IRQHandler(void)
{
   if(USB_VCP_GetStatus() != USB_VCP_CONNECTED){
      mode = AF_MODE;

      AF_counter = 3;
      IPP = 0;
      IHB = 0;
   }

   EXTI_ClearFlag(EXTI_Line1);
}
