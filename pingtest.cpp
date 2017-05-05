/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 
 03/17/2013 : Charles-Henri Hallard (http://hallard.me)
              Modified to use with Arduipi board http://hallard.me/arduipi
						  Changed to use modified bcm2835 and RF24 library 
 */


#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include "./RF24.h"

const short SENDING = 1;
const short ACKNOWLEDGE = 2;
const short TEMPERATURE = 3;
const short HUMIDITY = 4;

const int SENSOR_COUNT = 4;

const uint64_t RecvPipe = 0x0000000FF1LL;
const uint64_t SendPipe = 0x0000000FF2LL; 

short radio_ids[SENSOR_COUNT]={0x001, 0x002, 0x003, 0x004};
short Temperatures[SENSOR_COUNT];
short Humidities[SENSOR_COUNT];
char Times[SENSOR_COUNT][80];

void PrepareToWrite();
void PrepareToRecv();
void WriteAck(short radio_id);
int FindSensorIndex(short radio_id);
void SaveValues();

// Setup for GPIO 22 CE and CE1 CSN with SPI Speed @ 8Mhz
RF24 radio(RPI_V2_GPIO_P1_22, RPI_V2_GPIO_P1_24, BCM2835_SPI_SPEED_8MHZ);  


unsigned char messagebuf[5] = {0x00,0x00,0x00,0x00,0x00}; 
unsigned char sendbuf[5] = {0x00,0x00,0x00,0x00,0x00};

int main(int argc, char** argv)
{


	/*Temperatures[0] = 80;
	Humidities[0] = 90;
	Temperatures[1] = -30;
	Humidities[1] = 70;
	Temperatures[2] = 0;
	Humidities[2] = 20;
	Temperatures[3] = 10;
	Humidities[3] = 100;
	
	
	char * f="18:20:17 am";
	memcpy(&buf, f, 20);
	memset(Times[0], '\0', 80);
	strcpy(Times[0], buf);
	memset(Times[1], '\0', 80);
	strcpy(Times[1], buf);	
	memset(Times[2], '\0', 80);
	strcpy(Times[2], buf);
	memset(Times[3], '\0', 80);	
	strcpy(Times[3], buf);
	
	SaveValues();*/
  
	radio.begin();

	//radio.setRetries(15,15);

	radio.setPayloadSize(32);
	//radio.setChannel(0x4c);
	radio.setPALevel(RF24_PA_MAX);


	
	PrepareToRecv();
	sleep(1);
	radio.printDetails();
	
	char buf[80];
	short radio_id = 0;
	int rad_index = 0;
	signed char Temperature=0;
	unsigned char Humidity=0;
	time_t temptime;
	
	struct tm tstruct;
	
	
	// forever loop
	while (true)
	{
		if(radio.available())
		{	
			radio.read(&messagebuf, sizeof(unsigned long));
			memcpy(&radio_id, &messagebuf[0], 2);
			printf("Got message, radio id %d\n", radio_id);
			Temperature = messagebuf[2];
			Humidity = messagebuf[3];
			rad_index = FindSensorIndex(radio_id);
			if(rad_index>-1)
			{
				printf("Radio identified.\n");
				Temperatures[rad_index]=Temperature;
				Humidities[rad_index]=Humidity;
				temptime = time(0);
				tstruct = *localtime(&temptime);
				strftime(buf, sizeof(buf), "%H:%M:%S %P", &tstruct);
				//Times[rad_index] = buf;
				strcpy(Times[rad_index], buf);
				printf("Temperature: %d Humidity: %d, Time: %s\n", Temperatures[rad_index], Humidities[rad_index], Times[rad_index]);
				printf("Saving sensor data...\n");
				SaveValues();
			}
			else
			{
				printf("Radio with id %d not identified. Discarding data.\n", radio_id);
			}
		}
	}
		
  return 0;
}

int FindSensorIndex(short radio_id)
{
	int i = -1;
	for(int index = 0; index < SENSOR_COUNT; index++)
	{
		if(radio_ids[index]==radio_id)
		{
			i = index;
			break;
		}
	}
	return i;
}

void WriteAck(short radio_id)
{
	//bool sent=false;
	memcpy(&sendbuf[0], &radio_id, 2);
	memcpy(&sendbuf[2], &ACKNOWLEDGE, 2);
	sleep(1);
	bool sent=false;
	while(!sent)
	{
		sent = radio.write(&sendbuf,4);
		if(!sent)
		{
			PrepareToRecv();
			PrepareToWrite();
		}
		printf("radio.write\n");
		sleep(1);
	}
	
	printf("sent: %d,", sendbuf[0]);
	printf("%d,", sendbuf[1]);
	printf("%d,", sendbuf[2]);
	printf("%d\n", sendbuf[3]);
	//return sent;
}

void PrepareToWrite()
{
	radio.stopListening();
	sleep(1);
	radio.openWritingPipe(SendPipe);
}

void PrepareToRecv()
{
	radio.openReadingPipe(1,RecvPipe);
	sleep(1);
	radio.startListening();
}

void SaveValues()
{
	char SensorInfo[(4+20)*SENSOR_COUNT];
	memset(SensorInfo, '\0', (4+20)*SENSOR_COUNT);	
	signed char Temp;
	unsigned char Hum;
	FILE * fd = fopen("sensordata.txt", "rw+");
	
	for(int index=0;index<SENSOR_COUNT; index++)
	{	
	Temp = Temperatures[index];
	Hum = Humidities[index];
	memcpy(&SensorInfo[index*4 + index*20], &radio_ids[index], 2);
	memcpy(&SensorInfo[index*4 + index*20 + 2], &Temp, 1);
	memcpy(&SensorInfo[index*4 + index*20 + 3], &Hum, 1);
	memcpy(&SensorInfo[index*4 + index*20 + 4], &Times[index], 20);
	//printf("%s\n",Times[index]);
	}
	fwrite(&SensorInfo, 1, (4+20)*SENSOR_COUNT, fd);
	fclose(fd);
}


// vim:cin:ai:sts=2 sw=2 ft=cpp
