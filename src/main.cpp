#include "arduinoFFT.h"

#define SAMPLES 128         
#define ANALOG 36 //ESP32 DevKitC VP port. GPIO36
#define PH1_COUNT 6 // 1900 - 200 Hz * 128 * samplingPeriod 
#define PH2_COUNT 5 // 5 or more PH1s
#define SAMPLING_FREQUENCY 6000 // Sampling rate 6kHz since 2kHz is a target
arduinoFFT FFT = arduinoFFT();
unsigned int samplingPeriod;
unsigned long microSeconds;
unsigned long phaseStart = 0;
unsigned long detectionStart = 0;

double vReal[SAMPLES]; 
double vImag[SAMPLES]; 
unsigned char phase1 = 0;
unsigned char phase2 = 0;

void resetPhaseIfNecessary()
{
  if (phaseStart > 0)
  {
    if (micros() - phaseStart > 1000000 * 8)
    {
      Serial.println("reset phases."); 
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
      Serial.println("phase1"); 
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
  Serial.begin(9600);                                           
  samplingPeriod = round(1000000 * (1.0 / SAMPLING_FREQUENCY)); 
  pinMode(2, OUTPUT);
  pinMode(ANALOG, INPUT);
}

void loop()
{
  for (int i = 0; i < SAMPLES; i++)
  {
    microSeconds = micros(); 

    vReal[i] = analogRead(ANALOG); 
    vImag[i] = 0;             
    while (micros() < (microSeconds + samplingPeriod))
    {
      //do nothing
    }
  }

  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

  double peak = FFT.MajorPeak(vReal, SAMPLES, SAMPLING_FREQUENCY);
  Serial.println(peak); 
  checkPhase1(peak);
  resetPhaseIfNecessary();
}
