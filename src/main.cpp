/*
 File/Sketch Name: AudioFrequencyDetector

 Version No.: v1.0 Created 12 December, 2019
 
 Original Author: Clyde A. Lettsome, PhD, PE, MEM
 
 Description:  This code/sketch makes displays the approximate frequency of the loudest sound detected by a sound detection module. For this project, the analog output from the 
 sound module detector sends the analog audio signal detected to A0 of the Arduino Uno. The analog signal is sampled and quantized (digitized). A Fast Fourier Transform (FFT) is
 then performed on the digitized data. The FFT converts the digital data from the approximate discrete-time domain result. The maximum frequency of the approximate discrete-time
 domain result is then determined and displayed via the Arduino IDE Serial Monitor.

 Note: The arduinoFFT.h library needs to be added to the Arduino IDE before compiling and uploading this script/sketch to an Arduino.

 License: This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License (GPL) version 3, or any later
 version of your choice, as published by the Free Software Foundation.

 Notes: Copyright (c) 2019 by C. A. Lettsome Services, LLC
 For more information visit https://clydelettsome.com/blog/2019/12/18/my-weekend-project-audio-frequency-detector-using-an-arduino/

*/

#include "arduinoFFT.h"

#define SAMPLES 128         //SAMPLES-pt FFT. Must be a base 2 number. Max 128 for Arduino Uno.
#define ANALOG 36
#define PH1_COUNT 6
#define PH2_COUNT 5
#define SAMPLING_FREQUENCY 6000//Ts = Based on Nyquist, must be 2 times the highest expected frequency.
arduinoFFT FFT = arduinoFFT();
unsigned int samplingPeriod;
unsigned long microSeconds;
unsigned long phaseStart = 0;
unsigned long detectionStart = 0;

double vReal[SAMPLES]; //create vector of size SAMPLES to hold real values
double vImag[SAMPLES]; //create vector of size SAMPLES to hold imaginary values
unsigned char phase1 = 0;
unsigned char phase2 = 0;

void resetPhaseIfNecessary()
{
  if (phaseStart > 0)
  {
    if (micros() - phaseStart > 1000000 * 8)
    {
      Serial.println("reset phases."); //Print out the most dominant frequency.
      phaseStart = 0;
      phase1 = 0;
      phase2 = 0;
    }
  }
  if (detectionStart > 0)
  {
    if (micros() - detectionStart > 1000000 * 3)
    {
      detectionStart = 0;
      Serial.println("detect reset.");
      digitalWrite(2, LOW);
    }
  }
}

void checkPhase1(double peak)
{
  if (peak > 1900 && peak < 2100)
  {
    phase1++;
    if (phase1 == 1)
    {
      phaseStart = micros();
    }
  }
  else
  {
    if (phase1 >= PH1_COUNT)
    {
      Serial.println("phase1"); //Print out the most dominant frequency.
      phase2++;
      if (phase2 > PH2_COUNT)
      {
        phase1 = 0;
        phase2 = 0;
        phaseStart = 0;
        Serial.println("detected");
        digitalWrite(2, HIGH);
        detectionStart = micros();
      }
    }
    phase1 = 0;
  }
}

void setup()
{
  Serial.begin(9600);                                           //Baud rate for the Serial Monitor
  samplingPeriod = round(1000000 * (1.0 / SAMPLING_FREQUENCY)); //Period in microseconds
  pinMode(2, OUTPUT);
  pinMode(ANALOG, INPUT);
}

void loop()
{
  /*Sample SAMPLES times*/
  for (int i = 0; i < SAMPLES; i++)
  {
    microSeconds = micros(); //Returns the number of microseconds since the Arduino board began running the current script.

    vReal[i] = analogRead(ANALOG); //Reads the value from analog pin 0 (A0), quantize it and save it as a real term.
    vImag[i] = 0;             //Makes imaginary term 0 always
    //Serial.print(vReal[i]);
    //SSerial.print(",");
    /*remaining wait time between samples if necessary*/
    while (micros() < (microSeconds + samplingPeriod))
    {
      //do nothing
    }
  }

  /*Perform FFT on samples*/
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

  /*Find peak frequency and print peak*/
  double peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
  Serial.println(peak); //Print out the most dominant frequency.
  checkPhase1(peak);
  resetPhaseIfNecessary();
}
