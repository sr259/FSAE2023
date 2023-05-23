
#include <2JCIE-EV01.h>
#include <Parallax_H48C.h>
#include <Adafruit_MLX90614.h>
#include <SD.h> //Load SD card library
#include <SPI.h> //Load SPI Library
#include <string>
#include <cfloat>
#include <Wire.h>
#include <TCA9548.h>

#define multiplexerAdd 0x70
#define tempAdd 0x5A

//Pins
uint8_t CLK = 23;
uint8_t DIO = 22;
uint8_t CS =41;

//Calibration 
float xVal = 0;
float yVal = 0;
float zVal = 0;
float xOffset, yOffset, zOffset;
uint8_t clkVal,dioVal,csVal;
int sensorMin = 1023;      
int sensorMax = 0; 


//Files
File accelData;
File tempData;
int const chipSelect = BUILTIN_SDCARD;
unsigned long currentMillis = 0;

Parallax_H48C parallax(DIO,CLK,CS);
Adafruit_MLX90614 mlx[4]; // Array to hold the sensors
TCA9548 multi = TCA9548(multiplexerAdd);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin();
  multi.begin();
  // Initialize all sensors
  for (int i = 0; i < 4; i++) {
    multi.enableChannel(i + 1); // Channel numbers start from 1
    mlx[i].begin(0x5A, &Wire);
  }
  calibrate();

  //SD card initializing 
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  if (SD.exists("AccelerometerData.csv")){
      SD.remove("AccelerometerData.csv");
    }
  if (SD.exists("TemperatureData.csv")){
      SD.remove("TemperatureData.csv");
    }
  Serial.println("card initialized.");
}

void loop(){
  delay(500);
    currentMillis +=50;
    accelData = SD.open("AccelerometerData.csv", FILE_WRITE);
    tempData = SD.open("TemperatureData.csv", FILE_WRITE);
    if(tempData & accelData){
      float x = parallax.readXAxis()-xOffset;
      float y = parallax.readYAxis()-yOffset;
      float z = parallax.readZAxis()-zOffset+1;
      Serial.print("X:");
      Serial.print(x);
      Serial.print(" ");
      Serial.print("Y:");
      Serial.print(y);
      Serial.print(" ");
      Serial.print("Z:");
      Serial.print(z);
      Serial.println();

      for (int i = 0; i < 4; i++) {
        multi.selectChannel(i + 1);
        float temp = mlx[i].readObjectTempC();
        Serial.print("Sensor ");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(temp);
        Serial.print(" C");
        Serial.println();
        tempData.print(temp);
        tempData.print(" ");
      }

      tempData.print(currentMillis);
      tempData.println();
      tempData.close();
      
      accelData.print(x);
      accelData.print(" ");
      accelData.print(y);
      accelData.print(" ");
      accelData.print(z);
      accelData.print(" ");
      accelData.print(currentMillis);
      accelData.println();
      accelData.close();
    }
    
    else{
      Serial.println("Data File is not available.");
      return;
    }
}

//Parallax Calibration
void calibrate() {
    const int numSamples = 500;
    float xMin = FLT_MAX, xMax = -FLT_MAX, yMin = FLT_MAX, yMax = -FLT_MAX, zMin = FLT_MAX, zMax = -FLT_MAX;
    for (int i = 0; i < numSamples; i++) {
      float xVal = parallax.readXAxis();
      float yVal = parallax.readYAxis();
      float zVal = parallax.readZAxis();
      if (xVal < xMin) xMin = xVal;
      if (xVal > xMax) xMax = xVal;
      if (yVal < yMin) yMin = yVal;
      if (yVal > yMax) yMax = yVal;
      if (zVal < zMin) zMin = zVal;
      if (zVal > zMax) zMax = zVal;
      delay(10);
    }
    xOffset = (xMax + xMin) / 2;
    yOffset = (yMax + yMin) / 2;
    zOffset = (zMax + zMin) / 2;
}
