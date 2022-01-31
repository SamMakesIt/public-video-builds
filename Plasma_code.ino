#include "Wire.h" // This library allows you to communicate with I2C devices.
#include <Servo.h>
const int MPU_ADDR = 0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.
Servo myservo;

//flags to move onto next step in code
bool primeFlag = false;
bool chargeFlag = false;
bool fireFlag = false;
bool lighterFlag = false;



//Variables for gy-521
int16_t accelerometer_x, accelerometer_y, accelerometer_z; // variables for accelerometer raw data
int16_t gyro_x, gyro_y, gyro_z; // variables for gyro raw data
int16_t temperature; // variables for temperature data

#define LED_Prime 10 // Attaches LED to pin 10
#define LED_Charge 11 // Attaches LED to pin 11
#define LED_Fire 12 // Attaches LED to pin 12
#define Lighter 3 // lighter pin

char tmp_str[7]; // temporary variable used in convert function

char* convert_int16_to_str(int16_t i) { // converts int16 to string. Moreover, resulting strings will have the same length in the debug monitor.
  sprintf(tmp_str, "%6d", i);
  return tmp_str;
}

int pos = 180; //Starting position of the Servo

void setup() {
  Serial.begin(9600);
  myservo.attach(13); // Attaches Servo to pin 13
  pinMode(LED_Prime, OUTPUT);// Sets pin 10 to output data
  pinMode(LED_Charge, OUTPUT); // Sets pin 11 to output data
  pinMode(LED_Fire, OUTPUT); // Sets pin 12 to output data
  pinMode(Lighter, OUTPUT); // Sets pin 3 to output data
  digitalWrite (Lighter, LOW); //Sets lighter to off by default
  digitalWrite(LED_Prime, LOW);// Sets LED to Default of off
  digitalWrite(LED_Charge, LOW);// Sets LED to Default of off
  digitalWrite(LED_Fire, LOW);// Sets LED to Default of off
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

}
void loop() {


  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
  Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
  Wire.requestFrom(MPU_ADDR, 7 * 2, true); // request a total of 7*2=14 registers

  // "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
  accelerometer_x = Wire.read() << 8 | Wire.read(); // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
  accelerometer_y = Wire.read() << 8 | Wire.read(); // reading registers: 0x3D (ACCEL_YOUT_H) and 0x3E (ACCEL_YOUT_L)
  accelerometer_z = Wire.read() << 8 | Wire.read(); // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
  temperature = Wire.read() << 8 | Wire.read(); // reading registers: 0x41 (TEMP_OUT_H) and 0x42 (TEMP_OUT_L)
  gyro_x = Wire.read() << 8 | Wire.read(); // reading registers: 0x43 (GYRO_XOUT_H) and 0x44 (GYRO_XOUT_L)
  gyro_y = Wire.read() << 8 | Wire.read(); // reading registers: 0x45 (GYRO_YOUT_H) and 0x46 (GYRO_YOUT_L)
  gyro_z = Wire.read() << 8 | Wire.read(); // reading registers: 0x47 (GYRO_ZOUT_H) and 0x48 (GYRO_ZOUT_L)

  // print out data
  Serial.print("aX = "); Serial.print(convert_int16_to_str(accelerometer_x));
  Serial.print(" | aY = "); Serial.print(convert_int16_to_str(accelerometer_y));
  Serial.print(" | aZ = "); Serial.print(convert_int16_to_str(accelerometer_z));
  // the following equation was taken from the documentation [MPU-6000/MPU-6050 Register Map and Description, p.30]
  Serial.print(" | tmp = "); Serial.print(temperature / 340.00 + 36.53);
  Serial.print(" | gX = "); Serial.print(convert_int16_to_str(gyro_x));
  Serial.print(" | gY = "); Serial.print(convert_int16_to_str(gyro_y));
  Serial.print(" | gZ = "); Serial.print(convert_int16_to_str(gyro_z));
  Serial.println();

  //Sets LEDs to on depending on position from accelerometer
  if (accelerometer_x >= 0 && accelerometer_x <= 5000 && accelerometer_y >= 14000 && accelerometer_y <= 17000) {
    digitalWrite(LED_Prime, HIGH);
    digitalWrite(LED_Charge, LOW);
    digitalWrite(LED_Fire, LOW);
    primeFlag = true;
  }
  else {
    if (fireFlag == true)
      primeFlag = false;
  }


  if (primeFlag == true && accelerometer_x > 0 && accelerometer_x < 7000  && accelerometer_y > 9000 && accelerometer_y < 14000) {
    digitalWrite(LED_Prime, HIGH);
    digitalWrite(LED_Charge, HIGH);
    digitalWrite(LED_Fire, LOW);
    chargeFlag = true;
  }
  else {
    if (primeFlag == false)
      chargeFlag = false;
  }

  if ( chargeFlag == true && accelerometer_x > 0 && accelerometer_x < 2000 && accelerometer_y >= -15000 && accelerometer_y < -10000 ) {
    digitalWrite(LED_Prime, HIGH);
    digitalWrite(LED_Charge, HIGH);
    digitalWrite(LED_Fire, HIGH);
    fireFlag = true;
    primeFlag = false;
    chargeFlag = false;
  }



  //Powers LEDS based off of flags
  if (chargeFlag == true) {
    digitalWrite(LED_Charge, HIGH);
  }
    else {
      digitalWrite (LED_Charge, LOW);
    }
  
  //Powers LEDS based off of flags
  if (primeFlag == true) {
    digitalWrite(LED_Prime, HIGH);
  }
    else {
      digitalWrite(LED_Prime, LOW);
    }
  
  // Tells servo to 180 during the prime state, and turns Servo back to 0 once in Fire state
  if (chargeFlag == true) {
    myservo.write(0);

  }
  else {
    if (fireFlag == true) {
      myservo.write(180);
    }
  }
  //Lighter ignites during Charge stage
  if (chargeFlag == true) {
    delay (5000);
    digitalWrite (Lighter, HIGH); //Turn on lighter
    lighterFlag == true;
    delay (500);
    digitalWrite (Lighter, LOW);
  }

  else {
    if (fireFlag == true) {
      digitalWrite (Lighter, LOW);
    }
  }


  //runs loop every second
  // delay
  delay (500);
}
