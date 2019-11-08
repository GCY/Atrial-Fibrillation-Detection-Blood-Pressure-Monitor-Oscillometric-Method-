# Atrial-Fibrillation-Detection-Blood-Pressure-Monitor-Oscillometric-Method-
This project is electronic blood pressure monitor research platform with Oscillometric method, include SBP and DBP estimate algorithm, Atrial Fibrillation detection algorithm, calibration curve, GUI tool for fine-tune BP algorithm...etc, ofcourse, this is research use only.

![alt text](https://github.com/GCY/Atrial-Fibrillation-Detection-Blood-Pressure-Monitor-Oscillometric-Method-/blob/master/res/v2%20pcb.jpg?raw=true)  

## Features
- [x] Blood Pressure Measurment - Systolic, Diastolic, Mean
- [x] Atrial Fibrillation Detection - Irregular Pulse Peak , Irregular Heart Beats
- [x] GUI Toos - Algorithm Finetune
- [x] ADC Clibration to Mercury Manometer Pressure - Polynomial Curve Fit
- [x] RAW Data Record

## Hardware
![alt text](https://github.com/GCY/Atrial-Fibrillation-Detection-Blood-Pressure-Monitor-Oscillometric-Method-/blob/master/res/assembly%20part%201.jpg?raw=true) 

![alt text](https://github.com/GCY/Atrial-Fibrillation-Detection-Blood-Pressure-Monitor-Oscillometric-Method-/blob/master/res/assembly%20part%202.jpg?raw=true) 

![alt text](https://github.com/GCY/Atrial-Fibrillation-Detection-Blood-Pressure-Monitor-Oscillometric-Method-/blob/master/res/t-type%20and%20direct%20connector.jpg?raw=true) 

### Key Component List

- Electronics
  - TB6612FNG (for PUMP and VALVE PID control)
  - MPS-3117-006GC A2/A3(MEMS Wheatstone Bridge)
- Mechatronic
  - 3V DC Air Pump Motor(JQB032-3A or DQB030-A-3V)
  - 3V DC Electric Solenoid Valve(JQF1-3C1/DC3V Mini 10mm*15mm)
- Cuff and Tubing
  - 2mm*4mm Silicone Rubber Tubing x 2(20cm~30cm, for wrist cuff input and output)
  - 2mm*4mm Silicone Rubber Tubing x 3(5cm~10cm, for T Type connector)
  - 2mm*3mm T Type Plastic Barbed Connector
  - 2mm*3mm Straight Plastic Barbed Connector x 3
  - Wrist Air Cuff Input/Output(2 pinhole 2mm~3mm)
  
### Configure for Motor Electric Drives With Independent Power Supply - Battery2(U$19)
- J1, J2 pin1 connect to pin2 - battery2 charge
- J1, J2 pin2 connect to pin3 - motor drive with independent power supply
- JP1 pin1 connect to pin2 - motor drive with VCC
- JP1 pin2 connect to pin3 - motor drive with independent power supply

## Firmware

### Build and Burn The .elf
- 1.cd src/BPM
- 2.make
- 3.load main.elf

### Key Parameters

PWM_Freq is DC Motor PWM frequency.</br>
TIM_Prescaler = (168M/2)/1M = 84M</br>
TIM_Period = TIMER_PWM_MAX/PWM_Freq = 1000</br>

TIM = 84M / TIM_Prescaler = 1M</br>
TIM = 1M / TIM_ClockDivision = 1M</br>
1M / (TIM_Period + 1) = 1000Hz (1ms)</br>

<pre><code>
const uint16_t PWM_Freq = 1000;
</code></pre>

ARR = TIMER_PWM_MAX;</br>
CCR = 0~TIMER_PWM_MAX = 0~100% PWM.</br>
In this case, PID PWM step 1~100 mapped to 100000~1000000.</br>
TIMER_PWM_33 and TIMER_PWM_40 is 33% and 40% PWM.</br>
<pre><code>
const uint32_t PID_PWM_MIN = 1;
const uint32_t PID_PWM_MAX = 100;
const uint32_t TIMER_PWM_MIN = 100000;
const uint32_t TIMER_PWM_MAX; = 1000000;
const uint32_t TIMER_PWM_33 = 330000;
const uint32_t TIMER_PWM_40 = 400000;
</code></pre>

AC signal amplitude point of Systolic and Diastolic BP.</br>
<pre><code>
float as_am_value = 0.65f;
float ad_am_value = 0.7f;
</code></pre>

<pre><code>
float a[3] = {-0.0000012119f,0.1262915457f,-1.0620516546f};
</code></pre>

If pulse_value_N / total_pulse_value_mean > IPP_Ratio, pulse_N is irregular pulse peak, and IPP ratio range 15%~25%.</br>
If IPP number / total_pulse_number > IHB_Ratio, this measurement is irregular heart beat.</br>
If two or more IHB of the three BP measurements, AF detected.<br>
<pre><code>
const float IPP_Ratio = 0.2f;
const float IHB_Ratio = 0.2f;
</code></pre>

### Operation Flowchart
![alt text](https://github.com/GCY/Atrial-Fibrillation-Detection-Blood-Pressure-Monitor-Oscillometric-Method-/blob/master/res/operation%20flowchart.png?raw=true)  

### DC/AC Signal Process Flowchart
![alt text](https://github.com/GCY/Atrial-Fibrillation-Detection-Blood-Pressure-Monitor-Oscillometric-Method-/blob/master/res/signal%20process%20flowchart.png?raw=true)  

## Software

### Build
- Win10
  - 1.Open BPM.sln
  - 2.Rebuild
- Mac High Sierra
  - 1.make
  
### Dependence
- Win10
  - wxWidgets 3.1.2
  - VS2017 - MSVC 10.0.17763-SDK
- Mac High Sierra
  - wxWidgets 3.x
  - g++

### Mac High Sierra GUI
![alt text](https://github.com/GCY/Atrial-Fibrillation-Detection-Blood-Pressure-Monitor-Oscillometric-Method-/blob/master/res/MAC%20OS%20X.png?raw=true)  

### Win10 GUI
![alt text](https://github.com/GCY/Atrial-Fibrillation-Detection-Blood-Pressure-Monitor-Oscillometric-Method-/blob/master/res/WIN10.PNG?raw=true)  

### Operation Manual
- "Ad/Am" and "As/Am" text box is Diastolic and Systolic BP Point of AC signal amplitude.
- "Measurment" button is BP measurment after VCP connected.
- "USB Mode" button is switch from "USB Mode"(Measurment) to "Calibration Mode", and vice versa.
- "Pressurize" and "Leak" button for control mercury manometer high and low in calibration mode.

## Oscillometric Method
The two chart below show AC signal types, all of types are correct.

### Type1 AC Signal
![alt text](https://github.com/GCY/Atrial-Fibrillation-Detection-Blood-Pressure-Monitor-Oscillometric-Method-/blob/master/res/type1.png?raw=true)  

### Type2 AC Signal
![alt text](https://github.com/GCY/Atrial-Fibrillation-Detection-Blood-Pressure-Monitor-Oscillometric-Method-/blob/master/res/type2.png?raw=true)  

## Calibration ADC-DC to Mercury Manometer Pressure

## Atrial Fibrillation In Oscillometric Method Detect Algorithm

![alt text](https://github.com/GCY/Atrial-Fibrillation-Detection-Blood-Pressure-Monitor-Oscillometric-Method-/blob/master/res/Diagnostic%20accuracy%20of%20a%20new%20algorithm%20to%20detect%20atrial%20fibrillation%20in%20a%20home%20blood%20pressure%20monitor.jpg?raw=true)  

### Premature Atrial Contractions

![alt text](https://github.com/GCY/Atrial-Fibrillation-Detection-Blood-Pressure-Monitor-Oscillometric-Method-/blob/master/res/pac%20ecg.png?raw=true)  

![alt text](https://github.com/GCY/Atrial-Fibrillation-Detection-Blood-Pressure-Monitor-Oscillometric-Method-/blob/master/res/pac%20oscillometric.png?raw=true)  

### Premature Ventricular Contractions

![alt text](https://github.com/GCY/Atrial-Fibrillation-Detection-Blood-Pressure-Monitor-Oscillometric-Method-/blob/master/res/pvc%20ecg.png?raw=true)  

![alt text](https://github.com/GCY/Atrial-Fibrillation-Detection-Blood-Pressure-Monitor-Oscillometric-Method-/blob/master/res/pvc%20oscillometric.png?raw=true)  

## Reference
Kabutoya, T., Imai, Y., Hoshide, S., & Kario, K. (2017). Diagnostic accuracy of a new algorithm to detect atrial fibrillation in a home blood pressure monitor. The Journal of Clinical Hypertension, 19(11), 1143-1147.

Ogedegbe, G., & Pickering, T. (2010). Principles and techniques of blood pressure measurement. Cardiology clinics, 28(4), 571-586.

Geddes, L. A., Voelz, M., Combs, C., Reiner, D., & Babbs, C. F. (1982). Characterization of the oscillometric method for measuring indirect blood pressure. Annals of biomedical engineering, 10(6), 271-280.

C++ Program for Polynomial Fit (Least Squares) : https://www.bragitoff.com/2015/09/c-program-for-polynomial-fit-least-squares/

LICENSE
-------

MIT License

Copyright (c) 2019 Tony Guo

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
