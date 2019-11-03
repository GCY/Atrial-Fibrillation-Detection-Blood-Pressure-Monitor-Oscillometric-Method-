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


#include "./usb_cdc_device/usbd_usr.h"
#include "./usb_cdc_device/usbd_cdc_core.h"
#include "./usb_cdc_device/usb_conf.h"
#include "./usb_cdc_device/usbd_desc.h"
#include "./usb_cdc_device/usbd_cdc_vcp.h"

#include "I2C.h"
#include "SSD1306.h"

#include "adc.h"

#include "tiny_printf.h"

#include "pid.h"

#define __FPU_PRESENT
#define __FPU_USED

extern uint32_t SAMPLING_RATE;
extern uint32_t ADC3_value[4];
extern bool  ADC3_ready;
extern uint32_t adc_value[4];

volatile uint32_t TimingDelay;

volatile uint32_t micros = 0;

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

void MOTOR_PWM_Config(uint16_t PWM_Fren)
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
   PrescalerValue = (uint16_t) ((SystemCoreClock /2) / 1000000) - 1; // (168000000/2)/1000000 = 840

   /* Time base configuration */
   TIM_TimeBaseStructure.TIM_Period = 1000000/PWM_Fren;
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

//MOTOR_PWM輸出設置, 當PWM_Fren==0時, 輸出關閉, 當PWM_Fren!=0時, 輸出頻率PWM_Fren  HZ的方波,
void MOTOR_PWM_out(uint16_t PWM_Fren)
{

   if( PWM_Fren )
   {
      //TIM_Cmd(TIM3, DISABLE);
      TIM3->ARR = (1000000/PWM_Fren)-1; //TIM_Period
      //TIM3->CCR2 = 350000/PWM_Fren; //TIM_Pulse
      TIM3->CCR2 = 0; //TIM_Pulse
      TIM_SelectOCxM(TIM3, TIM_Channel_2, TIM_OCMode_PWM1);
      TIM_CCxCmd(TIM3, TIM_Channel_2, TIM_CCx_Enable);
      TIM3->EGR = TIM_EventSource_Update;
      TIM_Cmd(TIM3, ENABLE);	 
   }
   else
   {
      TIM_Cmd(TIM3, DISABLE);
      TIM_SelectOCxM(TIM3, TIM_Channel_2, TIM_ForcedAction_InActive);
      TIM_CCxCmd(TIM3, TIM_Channel_2, TIM_CCx_Enable);		 
   }

}

void init_I2C3(){

   GPIO_InitTypeDef GPIO_InitStructure;
   I2C_InitTypeDef I2C_InitStruct;

   RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C3, ENABLE);
   RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C3, ENABLE);
   RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C3, DISABLE);

   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC, ENABLE);

   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
   GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
   GPIO_Init(GPIOA, &GPIO_InitStructure);

   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
   GPIO_Init(GPIOC, &GPIO_InitStructure);

   GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_I2C3);
   GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_I2C3);

   // 設定 I2C1 
   I2C_InitStruct.I2C_ClockSpeed = 400000; // 設定 I2C 時鐘速度為 100kHz
   I2C_InitStruct.I2C_Mode = I2C_Mode_I2C; // I2C 模式
   I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2; // 50% duty cycle --> standard
   I2C_InitStruct.I2C_OwnAddress1 = 0x00; // own address, not relevant in master mode
   I2C_InitStruct.I2C_Ack = I2C_Ack_Enable; // disable acknowledge when reading (can be changed later on)
   I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; // 設定 I2C 地址長度為 7 bit
   I2C_Init(I2C3, &I2C_InitStruct); // 初始化 I2C1

   // 啟用 I2C1
   I2C_Cmd(I2C3, ENABLE);

}

/* Private */
#define USB_VCP_RECEIVE_BUFFER_LENGTH		128
uint8_t INT_USB_VCP_ReceiveBuffer[USB_VCP_RECEIVE_BUFFER_LENGTH];
uint32_t int_usb_vcp_buf_in, int_usb_vcp_buf_out, int_usb_vcp_buf_num;
USB_VCP_Result USB_VCP_INT_Status;
//extern LINE_CODING linecoding;
uint8_t USB_VCP_INT_Init = 0;
USB_OTG_CORE_HANDLE	USB_OTG_dev;

extern uint8_t INT_USB_VCP_ReceiveBuffer[USB_VCP_RECEIVE_BUFFER_LENGTH];

USB_VCP_Result USBVCPInit(void)
{
   USBD_Init(&USB_OTG_dev,
#ifdef USE_USB_OTG_FS
	 USB_OTG_FS_CORE_ID,
#else
	 USB_OTG_HS_CORE_ID,
#endif
	 &USR_desc, 
	 &USBD_CDC_cb, 
	 &USR_cb);   

   /* Reset buffer counters */
   int_usb_vcp_buf_in = 0;
   int_usb_vcp_buf_out = 0;
   int_usb_vcp_buf_num = 0;

   /* Initialized */
   USB_VCP_INT_Init = 1;

   return USB_VCP_OK;
}

USB_VCP_Result USB_VCP_GetStatus(void) {
   if (USB_VCP_INT_Init) {
      return USB_VCP_INT_Status;
   }
   return USB_VCP_ERROR;
}

USB_VCP_Result USB_VCP_Getc(uint8_t* c) {
   /* Any data in buffer */
   if (int_usb_vcp_buf_num > 0) {
      /* Check overflow */
      if (int_usb_vcp_buf_out >= USB_VCP_RECEIVE_BUFFER_LENGTH) {
	 int_usb_vcp_buf_out = 0;
      }
      *c = INT_USB_VCP_ReceiveBuffer[int_usb_vcp_buf_out];
      INT_USB_VCP_ReceiveBuffer[int_usb_vcp_buf_out] = 0;

      /* Set counters */
      int_usb_vcp_buf_out++;
      int_usb_vcp_buf_num--;

      /* Data OK */
      return USB_VCP_DATA_OK;
   }
   *c = 0;
   /* Data not ready */
   return USB_VCP_DATA_EMPTY;
}

USB_VCP_Result USB_VCP_Putc(volatile char c) {
   uint8_t ce = (uint8_t)c;

   /* Send data over USB */
   VCP_DataTx(&ce, 1);

   /* Return OK */
   return USB_VCP_OK;
}

USB_VCP_Result USB_VCP_Puts(char* str) {
   while (*str) {
      USB_VCP_Putc(*str++);
   }

   /* Return OK */
   return USB_VCP_OK;
}

USB_VCP_Result INT_USB_VCP_AddReceived(uint8_t c) {
   /* Still available data in buffer */
   if (int_usb_vcp_buf_num < USB_VCP_RECEIVE_BUFFER_LENGTH) {
      /* Check for overflow */
      if (int_usb_vcp_buf_in >= USB_VCP_RECEIVE_BUFFER_LENGTH) {
	 int_usb_vcp_buf_in = 0;
      }
      /* Add character to buffer */
      INT_USB_VCP_ReceiveBuffer[int_usb_vcp_buf_in] = c;
      /* Increase counters */
      int_usb_vcp_buf_in++;
      int_usb_vcp_buf_num++;

      /* Return OK */
      return USB_VCP_OK;
   }

   /* Return Buffer full */
   return USB_VCP_RECEIVE_BUFFER_FULL;
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int main(void)
{
   if(SysTick_Config(SystemCoreClock / 1000 / 1000)){
      while(1){}
   }

   USBVCPInit();
   Delay(1000);

   init_I2C3();
   Init_SSD1306();

   //Init_ADC(); //ADC3 Pooling
   ADC_4_Channel_Init(); // DMA + TIMER + FIFO, 360Hz sampling rate
   Delay(1000);
   Init_LED();
   Init_PWM();

   Init_TB6612FNG();
   Delay(10000);
   uint16_t PWM_Fren = 1000;
   MOTOR_PWM_Config(PWM_Fren);
   Delay(10000);
   MOTOR_PWM_out(PWM_Fren);
   Delay(10000);

   __IO uint32_t HSEStatus = 0;
   HSEStatus = RCC->CR & RCC_CR_HSERDY;
   if(HSEStatus){
      GPIO_SetBits(GPIOC, GPIO_Pin_12);
   }
   else{
      GPIO_ResetBits(GPIOC, GPIO_Pin_12);
   }   

   SSD1306_Fill(0x00);
   SSD1306_UpdateScreen();
   Delay(1000000);
   SSD1306_Fill(0xFF);
   SSD1306_UpdateScreen();
   Delay(1000000);
   SSD1306_Fill(0x00);
   SSD1306_UpdateScreen();  
   Delay(1000000);   

   SSD1306_GotoXY(3, 4);
   SSD1306_Puts("BPM", &Font_11x18, 0xFF);
   SSD1306_GotoXY(3, 25);
   SSD1306_Puts("20190814", &Font_11x18, 0xFF);
   SSD1306_GotoXY(3, 45);
   SSD1306_Puts("TonyGUO", &Font_11x18, 0xFF);
   SSD1306_UpdateScreen();  
   Delay(1000000);
   SSD1306_Fill(0x00);
   SSD1306_UpdateScreen();

   int id_state = 0;
   int recheck_state = 0;
   int sn = 1;  

   uint32_t last_update = micros;
   bool blue_flag = false;
   bool measure_flag = false;
   bool calibration_flag = false;

   float last_pressure = 0;
   float diff_pressure = 0;
   float pwm = 0,setpoint = 5;
   float Kp=1.5, Ki=0.07, Kd=0.003;
   float baseline = 0;
   float dc = 0;
   uint32_t init_baseline_count = 0;   
   uint32_t update_point = 0; 
   float pressure = 0;

   TIM3->CCR2 = 0;
   GPIO_ResetBits(GPIOB, GPIO_Pin_8);

   while(1){
      /*
      if(last_update - micros > 1000000){
	 int32_t dc = ((-0.00002*(adc_value[0]*adc_value[0])) + (0.1924*adc_value[0]) - 91.58);
	 if(dc < 0){
	    dc = 0;
	 }
	 unsigned char dc_str[15] = {0};
	 sprintf(dc_str,"%d mmHg",dc);
	 //sprintf(dc_str,"%d",adc_value[0]);
	 SSD1306_GotoXY(40, 25);
	 SSD1306_Puts(dc_str, &Font_11x18, 0xFF);
	 SSD1306_UpdateScreen();
	 last_update = micros;
      }*/

      /*
      unsigned char dc_str[10] = {0};
      sprintf(dc_str,"%d",adc_value[0]);
      SSD1306_GotoXY(55, 25);
      SSD1306_Puts(dc_str, &Font_11x18, 0xFF);
      SSD1306_UpdateScreen();
      */
      if(blue_flag){
	 GPIO_SetBits(GPIOC, GPIO_Pin_13);
      }
      else{
	 GPIO_ResetBits(GPIOC, GPIO_Pin_13);
      }   

      if(USB_VCP_GetStatus() == USB_VCP_CONNECTED) {
/*	 if(sn == 1){
	    uint8_t c;
	    if (USB_VCP_Getc(&c) == USB_VCP_DATA_OK) {
	       if(c == 'A' && id_state == 0){
		  ++id_state;
	       }
	       if(c == 'C' && id_state == 1){
		  ++id_state;
	       }
	       if(c == 'K' && id_state == 2){
		  ++id_state;
		  USB_VCP_Puts("ACK");
		  sn = 0;

	       }
	    }
	 }
	 else if (sn == 0){
	    uint8_t c;
	    if (USB_VCP_Getc(&c) == USB_VCP_DATA_OK) {
	       if(c == 'G' && recheck_state == 0){
		  ++recheck_state;
	       }
	       if(c == 'O' && recheck_state == 1){
		  ++recheck_state;
	       }
	       if(c == 'T' && recheck_state == 2){
		  ++recheck_state;
		  USB_VCP_Puts("GOT");
		  sn - -1;
	       }
	    }
	 }
*/
	 /* for good
	  * Kp=1.8, Ki=0.07, Kd=0.003;
	  * TIM3->CCR2 = map(pwm,1,100,180000,1000000)/1000;
	  * PID(micros,&diff_pressure, &pwm, &setpoint, Kp, Ki, Kd,1000,ON,REVERSE,1,100);
	  * setpoint = 5
	  */
	 //if(recheck_state == 3){
	 if(true){
	    uint8_t c;
	    if (USB_VCP_Getc(&c) == USB_VCP_DATA_OK) {
	       if(c == 'm' || c == 'M'){
		  last_update = micros;
		  calibration_flag = true;

		  last_pressure = 0;
		  diff_pressure = 0;
		  pwm = 0;
		  //v2 Kp=1.2, Ki=0.19, Kd=0.003;
		  //Kp=1.2, Ki=0.22, Kd=0.003; // for red version
		  //Kp=1.2, Ki=0.31, Kd=0.003; // for black version
		  Kp=1.4, Ki=0.6, Kd=0.0035; // for black version
		  baseline = 0;
		  init_baseline_count = 0;	
		  dc = 0;
		  update_point = 0;

		  //setpoint = 2.3;  // for red version
		  setpoint = 3;  // for black version
		  pressure = 0;
	       }
	    }
	    if(calibration_flag){
	       if(micros - last_update > (3*1000000)){
		  measure_flag = true;
		  TIM3->CCR2 = 350000/1000;
		  GPIO_SetBits(GPIOB, GPIO_Pin_8);

		  calibration_flag = false;

		  dc = baseline / init_baseline_count;

		  //PID(micros,&diff_pressure, &pwm, &setpoint, Kp, Ki, Kd,1000,ON,REVERSE,285000,1000000);
		  //PID(micros,&diff_pressure, &pwm, &setpoint, Kp, Ki, Kd,10000,ON,REVERSE,300000,1000000);
		  PID(micros,&diff_pressure, &pwm, &setpoint, Kp, Ki, Kd,1000,ON,REVERSE,1,100);
		  TIM3->CCR2 = 330000/1000;
	       }
	       else{
	       }
	    }
	    if(measure_flag){
	       if(micros - last_update > (33*1000000) || pressure > 180){
		  measure_flag = false;
		  TIM3->CCR2 = 0;
		  GPIO_ResetBits(GPIOB, GPIO_Pin_8);		  
	       }
	    }
	    if(ADC3_ready && (measure_flag | calibration_flag)){
	       ADC3_ready = false;
	       ++update_point;
	       unsigned char str[255];
	       //sprintf(str,"R%d,%d,%d,%d,%d,",micros, ADC3_value[0], ADC3_value[1], ADC3_value[2], ADC3_value[3]); 
	       sprintf(str,"R%d,%d,%d,%d,%d,",micros, ADC3_value[0], ADC3_value[1], ADC3_value[2], ADC3_value[3]);
	       if(calibration_flag){
		  baseline += ADC3_value[0];
		  ++init_baseline_count;
	       }
	       else if(!(update_point % 1)){
		  float value_dc = ADC3_value[0] - dc;

		  //pressure = (-0.000009f*(value_dc*value_dc))+(0.1361f*value_dc) - 0.661f; //78base
		  //pressure = (-0.000001f*(value_dc*value_dc))+(0.1333f*value_dc) - 0.3395f; //121base
		  pressure = (-0.000001f*(value_dc*value_dc))+(0.1263f*value_dc) - 1.0621f; //174base

		  diff_pressure = pressure - last_pressure;
		  //if(diff_pressure < 0){diff_pressure = 0;}
		  Computing(micros);
		  //TIM3->CCR2 = pwm/1000;
		  if(pressure > 35 || (micros - last_update > (7.5f*1000000)) ){
		     TIM3->CCR2 = map(pwm,1,100,100000,1000000)/1000;
		  }
		  else{
		     TIM3->CCR2 = 330000/1000;
		  }
		  /*
		  else if(pressure > 110  && pressure < 140){
		     TIM3->CCR2 = map(pwm,1,100,220000,1000000)/1000;
		  }
		  else if(pressure > 140  && pressure < 160){
		     TIM3->CCR2 = map(pwm,1,100,240000,1000000)/1000;
		  }
		  else if(pressure > 160  && pressure < 200){
		     TIM3->CCR2 = map(pwm,1,100,250000,1000000)/1000;
		  }			  
		  else{
		     TIM3->CCR2 = 330000/1000;
		  }*/

		  /*
		  if(diff_pressure > 5){
		     TIM3->CCR2 = 330000/1000;
		  }
		  else if(diff_pressure < 5 && diff_pressure > 3){
		     TIM3->CCR2 = 370000/1000;
		  }
		  else if(diff_pressure < 2){
		     TIM3->CCR2 = 390000/1000;
		  }*/

		  last_pressure = pressure;
		  /*
		  if(pressure > 60 && pressure < 100){
		     SetOutputLimits(275000,1000000);
		  }			  
		  if(pressure > 100 && pressure < 150){
		     SetOutputLimits(325000,1000000);
		  }	
		  else if(pressure > 150){
		     SetOutputLimits(350000,1000000);
		  }*/				  
	       }
	       USB_VCP_Puts(str);

	    }
	 }
	 blue_flag ^= true;
      }
   }

   return(0); // System will implode
}    
