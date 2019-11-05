# Atrial-Fibrillation-Detection-Blood-Pressure-Monitor-Oscillometric-Method-
This project is electronic blood pressure monitor research platform with Oscillometric method, include SBP and DBP estimate algorithm, Atrial Fibrillation detection algorithm, calibration curve, GUI tool for fine-tune BP algorithm...etc, ofcourse, this is research use only.

![alt text](https://github.com/GCY/Atrial-Fibrillation-Detection-Blood-Pressure-Monitor-Oscillometric-Method-/blob/master/res/v2%20pcb.jpg?raw=true)  

## Hardware
![alt text](https://github.com/GCY/Atrial-Fibrillation-Detection-Blood-Pressure-Monitor-Oscillometric-Method-/blob/master/res/assembly%20part%201.jpg?raw=true) 

![alt text](https://github.com/GCY/Atrial-Fibrillation-Detection-Blood-Pressure-Monitor-Oscillometric-Method-/blob/master/res/assembly%20part%202.jpg?raw=true) 

![alt text](https://github.com/GCY/Atrial-Fibrillation-Detection-Blood-Pressure-Monitor-Oscillometric-Method-/blob/master/res/t-type%20and%20direct%20connector.jpg?raw=true) 

### Key Component List

- Electronics
  - TB6612FNG
  - MPS-3117-006GC A2/A3
- Mechatronic
  - 3V DC Air Pump Motor(JQB032-3A or DQB030-A-3V)
  - 3V DC Electric Solenoid Valve(JQF1-3C1/DC3V Mini 10mm*15mm)
- Cuff and Tubing
  - 2mm*4mm Silicone Rubber Tubing x 2(20cm~30cm, for wrist cuff input and output)
  - 2mm*4mm Silicone Rubber Tubing x 3(5cm~10cm, for T Type connector)
  - 2mm*3mm T Type Plastic Barbed Connector
  - 2mm*3mm Straight Plastic Barbed Connector x 3
  - Wrist Air Cuff Input/Output(2 pinhole 2mm~3mm)

## Firmware

## Software

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

## Oscillometric Method

![alt text](https://github.com/GCY/Atrial-Fibrillation-Detection-Blood-Pressure-Monitor-Oscillometric-Method-/blob/master/res/type1.png?raw=true)  

![alt text](https://github.com/GCY/Atrial-Fibrillation-Detection-Blood-Pressure-Monitor-Oscillometric-Method-/blob/master/res/type2.png?raw=true)  

## Calibration ADC-DC to Mercury Manometer

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
