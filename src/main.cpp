#include <Arduino.h>
#include "BluetoothSerial.h"

#define DEVICE_NAME "BikeEyes"
#define BUFF_SIZE 5

char col; // For storing the data read from serial port
unsigned char buff[8] = {};

float dst_buff[BUFF_SIZE] = {};
int curr_buff_idx = 0;

int firstHalf = 0, secondHalf = 0;

int dst = 0; // Distance in centimeters
int last_dst = 0;
int last_dst_time = 0;

float speed = 0; // Speed in m/s

BluetoothSerial SerialBT;

void sendData(float *dst, float *speed);
float get_dst(float *d);
float get_avg_dst();

void setup()
{
        Serial.begin(9600);
        Serial2.begin(57600); // Serial is used to connect to a Radar
        SerialBT.begin(DEVICE_NAME);
        Serial.println("Started...");

        // Init buffer with zeros
        for (int i = 0; i < BUFF_SIZE; i++)
        {
                dst_buff[i] = 0;
        }
}

void loop()
{
        // Send data only when received data
        if (Serial2.read() == 0xff)
        {
                // Read the incoming byte.
                for (int j = 0; j < 8; j++)
                {
                        col = Serial2.read();
                        buff[j] = (char)col;
                        delay(2);
                }
                Serial2.flush();
                if (buff[1] == 0xff)
                {
                        if (buff[2] == 0xff)
                        {
                                firstHalf = buff[3];
                                secondHalf = buff[4];

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

                float dst_meters = get_dst() / 100;
                float vel_kmh = (float)speed * 3.6;

                sendData(&dst_meters, &vel_kmh);
        }
}

void sendData(float *d, float *spd)
{
        String dataString = String(*d) + "," + String(*spd);
        Serial.println(dataString);
        SerialBT.println(dataString);
}

float get_dst()
{
        dst_buff[curr_buff_idx] = dst;

        curr_buff_idx++;
        if (curr_buff_idx >= BUFF_SIZE)
        {
                curr_buff_idx = 0;
        }

        return get_avg_dst();
}

float get_avg_dst()
{
        float total_dst = 0;
        for (int i = 0; i < BUFF_SIZE; i++)
        {
                total_dst += dst_buff[i];
        }

        return total_dst / BUFF_SIZE;
}