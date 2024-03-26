// Include Particle Device OS APIs
#include "Particle.h"
#include "ADS1220.h"
// #include "SPI.h"

// Let Device OS manage the connection to the Particle Cloud
// SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(AUTOMATIC);
SerialLogHandler logHandler;

const std::chrono::milliseconds sleepTime = 5s;

//ADS1220 ADC
#define PGA          1                 // Programmable Gain = 1
#define VREF         2.048            // Internal reference of 2.048V
#define VFSR         VREF/PGA
#define FULL_SCALE   (((long int)1<<23)-1)
#define ADS1220_CS_PIN    D22 //D22
#define ADS1220_DRDY_PIN  D23 //D23         
#define FSR (((long int)1<<23)-1)  

volatile byte MSB;
volatile byte data;
volatile byte LSB;
//volatile char SPI_RX_Buff[3];
volatile byte *SPI_RX_Buff_Ptr;
ADS1220 ADS1220;
volatile bool drdyIntrFlag = false;
double adc0, adc1;

void setup()
{
  pinMode(ADS1220_CS_PIN, OUTPUT);
  pinMode(ADS1220_DRDY_PIN, INPUT);
  ADS1220.begin(ADS1220_CS_PIN,ADS1220_DRDY_PIN);
  ADS1220.PGA_ON();
  ADS1220.set_data_rate(DR_330SPS);
  ADS1220.set_pga_gain(PGA_GAIN_1);
  ADS1220.set_conv_mode_single_shot();
  Log.info("ADC configured");
}

float ADC(int channel)
{
  static int32_t adc_data;
  double result;
  if (channel == 1){
    Log.info("ADC Conversion Channel 1");
    adc_data=ADS1220.Read_SingleShot_SingleEnded_WaitForData(MUX_SE_CH0);
  } else if (channel == 2){
    Log.info("ADC Conversion Channel2");
    adc_data=ADS1220.Read_SingleShot_SingleEnded_WaitForData(MUX_SE_CH1);
  }
  else {
    result = 0;
  }
  result = convertToMilliV(adc_data);
  Log.info("ADC Conversion; %f", result);
  return result;
}

float convertToMilliV(int32_t i32data)
{
  return (float)((i32data*VFSR*1000)/FULL_SCALE);
}

void package() {
  adc0 = ADC(1);
  adc1 = ADC(2);
  int timestamp = Time.now();
  memset(publishData, 0, sizeof(publishData));
  JSONBufferWriter writer(publishData, sizeof(publishData) - 1);
  writer.beginObject();
    writer.name("ts").value(timestamp);
    writer.name("channel1").value(adc0);
    writer.name("channel2").value(adc1);
  writer.endObject();  
  Log.info("Message packaged: %s", publishData);
}

void loop() {
  //update the ADC and generate JSON
  package();
  delay(sleepTime);
}