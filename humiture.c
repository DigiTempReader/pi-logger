/* Most of this code came from
 * https://github.com/sunfounder/SunFounder_SensorKit_for_RPi2/blob/master/C/28_humiture/humiture.c
 * Modified by Brian Masney <masneyb@onstation.org> to work against a DHT22 sensor.
 * Also added more bounds checking.
 */

#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#define MAXTIMINGS 85

int data[5];

int read_data(int gpioPin)
{
	uint8_t laststate = HIGH;
	uint8_t counter = 0;
	uint8_t j = 0, i;

	memset(data, 0, sizeof(data));

	/* pull pin down for 18 milliseconds */
	pinMode(gpioPin, OUTPUT);
	digitalWrite(gpioPin, LOW);
	delay(18);

	/* then pull it up for 40 microseconds */
	digitalWrite(gpioPin, HIGH);
	delayMicroseconds(40); 

	/* prepare to read the pin */
	pinMode(gpioPin, INPUT);

	/* detect change and read data */
	for (i=0; i < MAXTIMINGS; i++) {
		counter = 0;
		while (digitalRead(gpioPin) == laststate) {
			counter++;
			delayMicroseconds(1);
			if (counter == 255) {
				break;
			}
		}
		laststate = digitalRead(gpioPin);

		if (counter == 255) {
			break;
		}

		/* ignore first 3 transitions */
		if ((i >= 4) && (i%2 == 0)) {
			/* shove each bit into the storage bytes */
			data[j/8] <<= 1;
			if (counter > 16) {
				data[j/8] |= 1;
			}
			j++;
		}
	}

	/* check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
	   print it out if data is good */
	if ((j >= 40) && 
			(data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) ) {
		float temperature, humidity;
		humidity = (data[0] * 256 + data[1]) / 10;

		temperature = ((data[2] & 0x7F)* 256 + data[3]) / 10;
		if (data[2] & 0x80) {
			temperature *= -1;
		}

		if (humidity < 0.0 || humidity > 100.0 || temperature < -30.0 || temperature > 50.0) {
			/* Bad reading */
			return 0;
		}

		printf("Humidity: %.1f %%\nTemperature: %.1f *F\n", 
				humidity, temperature * 9.0 / 5.0 + 32);
		return 1;
	}

	/* Bad reading */
	return 0;
}

void usage() {
	printf("usage: humiture <GPIO pin #>\n");
	exit(1);
}

int main (int argc, char **argv)
{
	int gpioPin;

	if (argc != 2) {
		usage();
	}

	errno = 0;
	gpioPin = strtol(argv[1], NULL, 10);
	if (errno != 0) {
		usage();
	}

	if (wiringPiSetup () == -1) {
		return (1) ;
	}

	while (!read_data(gpioPin)) {
		/* NOOP */
	}

	return (0);
}

