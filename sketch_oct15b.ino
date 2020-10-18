//include libraries and declare variables

// Servo Connection

// BROWN - gnd
// red - 5v
// yellow - a6 (pwm on Sero 1)
// - A7 (servo 2)+

// MPU Connection

// VCC - 5v
// GND - GND
// SCL - B6
// SDA - B7

#include <Wire.h>
#include <Servo.h>
const int MPU_addr=0x68; //I2C address mpu
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ; //16bit integer, acceleration and gyro in x,y,z axis

float delta_t = 0.005;
float pitchAcc,rollAcc,pitch,roll,pitched;
float P_CompCoeff = 0.98;


//Initial setup

Servo servo1, servo2;

void setup()
{
  Serial.begin(57600);
  Wire.setClock(400000);
  Wire.begin();
  delay(250);
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B); //power management
  Wire.write(0x00); //wake upp microcontroller
  Wire.endTransmission(true); //end transmission if the microcontroller has been woken
  
  
  servo1.attach(8); //Attach the first servo to pin A4 on the microcontroller
  servo2.attach(9); //Attach the second servo to pin A5 on the microcontroller
  
}


//Main loop

void loop()
{
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B); //starting with register 0x3B
  Wire.endTransmission(false); // end transmission if it isn't starting att register 0x3B

  Wire.requestFrom(MPU_addr,14,true); // requesting 14 registers from microcontroller

  AcX=Wire.read()<<8|Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C(ACCEL_XOUT_L)
  AcY=Wire.read()<<8|Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E(ACCEL_YOUT_L)
  AcZ=Wire.read()<<8|Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40(ACCEL_ZOUT_L)
  Tmp=Wire.read()<<8|Wire.read();
  GyX=Wire.read()<<8|Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44(GYRO_XOUT_L)
  GyY=Wire.read()<<8|Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46(GYRO_YOUT_L)
  GyZ=Wire.read()<<8|Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48(GYRO_ZOUT_L)

//Complemantary filter
long squaresum_P=((long)GyY*GyY+(long)AcY*AcY);
long squaresum_R=((long)GyX*GyX+(long)AcX*AcX);
pitch+=((-AcY/40.8f)*(delta_t));
roll+=((-AcX/45.8f)*(delta_t)); //32.8
pitchAcc= atan((AcY/sqrt(squaresum_P))*RAD_TO_DEG);
rollAcc =atan((AcX/sqrt(squaresum_R))*RAD_TO_DEG);
pitch =(P_CompCoeff*pitch + (1.0f-P_CompCoeff)*pitchAcc);//pitch=P_CompCoeff*pitch + (1.0f-P_CompCoeff)*pitchAcc
roll =(P_CompCoeff*roll + (1.0f-P_CompCoeff)*rollAcc);

//Roll commands

if (pitch < -158)
{
  pitched = abs(pitch + 158);
  pitched = pitched - 158;
}
else if (pitch > -156)
{
  pitched = abs(156 + pitch);
  pitched = -156 - pitched;
}
//locked movement for upward direction of pitch
if (pitched < -240)
{
  pitched = -240;
}
//Servo commands, roll/pitch + nr, where nr is compensation for mounting to start horizontally
servo1.write((roll + 120));
servo2.write((pitched + 340));
}
