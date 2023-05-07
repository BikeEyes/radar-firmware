#include <Arduino.h>
#include "BluetoothSerial.h"

#define DEVICE_NAME "BikeEyes"
#define DEVICE "ESP32_Devkit"
#define LED_BUILTIN 2

char col; // For storing the data read from serial port
unsigned char buffer[8] = {};
int firstHalf = 0, secondHalf = 0;

int dst = 0; // Distance in centimeters
int last_dst = 0;
int last_dst_time = 0;

float speed = 0; // Speed in m/s

BluetoothSerial SerialBT;

void sendData(float *dst, float *speed);
void blink(int pinNumber, int delayTime = 1000, int repeatTimes = 3);

void setup()
{
        Serial.begin(9600);
        Serial2.begin(57600); // Serial is used to connect to a Radar
        SerialBT.begin(DEVICE_NAME);
        Serial.println("BikeEyes radar started");
        blink(LED_BUILTIN, 1000, 3);
}

void loop()
{
        if (Serial2.read() == 0xff)
        {
                for (int j = 0; j < 8; j++)
                {
                        col = Serial2.read();
                        buffer[j] = (char)col;
                        delay(2);
                }
                Serial2.flush();
                if (buffer[1] == 0xff)
                {
                        if (buffer[2] == 0xff)
                        {
                                firstHalf = buffer[3];
                                secondHalf = buffer[4];

                                last_dst = dst;
                                dst = (firstHalf << 8) + secondHalf;
                                int curr_time = millis();

                                int dst_offset = last_dst - dst;
                                int time_offset = curr_time - last_dst_time;

                                speed = (float)dst_offset / (float)time_offset / 1000; // convert to seconds

                                last_dst_time = curr_time;
                        }
                } // Read the obstacle distance of maximum reflection intensity

                // convert to metres and kilometres per hour

                float dst_meters = (float)dst / 100;
                float vel_kmh = (float)speed * 3.6;

                sendData(&dst_meters, &vel_kmh);
        }
}

void sendData(float *dst, float *speed)
{
        String dataString = String(*dst) + "," + String(*speed);
        Serial.println(dataString);
        SerialBT.println(dataString);
}

void blink(int pinNumber, int delayTime, int repeatTimes)
{
        for (int i = 0; i < repeatTimes; i++)
        {
                digitalWrite(pinNumber, HIGH);
                delay(delayTime);
                digitalWrite(pinNumber, LOW);
                delay(delayTime);
        }
}