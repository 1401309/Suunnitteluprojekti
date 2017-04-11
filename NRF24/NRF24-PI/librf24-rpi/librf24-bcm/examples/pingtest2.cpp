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
#include "./RF24.h"

const unsigned int SENDING = 1;
const unsigned int ACKNOWLEDGE = 2;
const unsigned int TEMPERATURE = 3;
const unsigned int HUMIDITY = 4;

const unsigned int SENSOR_COUNT = 2;

const uint64_t RecvPipes[SENSOR_COUNT] = { 0x0000000001LL, 0x0000000002LL };
const uint64_t SendPipes[SENSOR_COUNT] = { 0x1000000001LL, 0x1000000002LL }; 

unsigned int Temperatures[SENSOR_COUNT];
unsigned int Humidities[SENSOR_COUNT];

//
// Hardware configuration
//

// CE Pin, CSN Pin, SPI Speed

// Setup for GPIO 22 CE and GPIO 25 CSN with SPI Speed @ 1Mhz
//RF24 radio(RPI_V2_GPIO_P1_22, RPI_V2_GPIO_P1_18, BCM2835_SPI_SPEED_1MHZ);

// Setup for GPIO 22 CE and CE0 CSN with SPI Speed @ 4Mhz
//RF24 radio(RPI_V2_GPIO_P1_15, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_4MHZ); 

// Setup for GPIO 22 CE and CE1 CSN with SPI Speed @ 8Mhz
RF24 radio(RPI_V2_GPIO_P1_15, RPI_V2_GPIO_P1_26, BCM2835_SPI_SPEED_8MHZ);  


// sets the role of this unit in hardware.  Connect to GND to be the 'pong' receiver
// Leave open to be the 'ping' transmitter
const int role_pin = 7;




char message[4] = {0x00,0x00,0x00,0x00};

int main(int argc, char** argv)
{

  int state = 0;
  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  //radio.setPayloadSize(32);
  radio.setChannel(0x4c);
  radio.setPALevel(RF24_PA_LOW);

  
  radio.startListening();

  radio.printDetails();

	
	// forever loop
	while (true)
	{
		
		for(int index=0; index<RecvPipes.length; index++)
		{
			state = 0;
			radio.openReadingPipe(1,RecvPipes[index]);
			radio.startListening();
			
			if(radio.available())
			{	
				while(state < 4)
				{
					if(radio.available())
					{							
						radio.read(&message, sizeof(unsigned long));
						radio.stopListening();
						radio.openWritingPipe(SendPipes[index]);
						
						switch((unsigned int)message)
						{
							case SENDING:
								state = 1;
								radio.write(ACKNOWLEDGE, sizeof(ACKNOWLEDGE));
								break;
							case TEMPERATURE:
								state = 2;
								radio.write(ACKNOWLEDGE, sizeof(ACKNOWLEDGE));
								break;
							case HUMIDITY:
								state = 3;
								radio.write(ACKNOWLEDGE, sizeof(ACKNOWLEDGE));
								break;
							case else:
								
								switch(state)
								{
									case 2:
										memcpy(&Temperatures[index], &message, sizeof(unsigned int));
										radio.write(ACKNOWLEDGE, sizeof(ACKNOWLEDGE));
										break;
									case 3:
										memcpy(&Humidities[index], &message, sizeof(unsigned int));
										radio.write(ACKNOWLEDGE, sizeof(ACKNOWLEDGE));
										state=4; //Finish with this sensor and exit the loop
										break;
								}
						}
						
						if(state<4) //carry on listening until all data has been received
						{
							radio.openReadingPipe(1,RecvPipes[index]);
							radio.startListening();
						}	
					}
					else
					{
						//count time-out???
					}
				}
				
			}
			
			sleep(1);
		}
	}

  return 0;
}

// vim:cin:ai:sts=2 sw=2 ft=cpp
