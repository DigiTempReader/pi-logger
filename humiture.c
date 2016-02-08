/*
 * humiture.c - Read temperature and humidity levels from a DHT11 or
 *              DHT22 sensor.
 *
 * Copyright (C) 2015 Cavon Lee
 * https://github.com/sunfounder/SunFounder_SensorKit_for_RPi2/blob/master/C/28_humiture/humiture.c
 *
 * Copyright (C) 2016 Brian Masney <masneyb@onstation.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#define MAXTIMINGS 85

struct temperature_humidity {
	float temperature;
	float humidity;
};

float celsius_to_fahrenheit(float input)
{
	return input * 9.0 / 5.0 + 32;
}

float celsius_to_celsius(float input)
{
	return input;
}

float celsius_to_kelvin(float input)
{
	return input + 273.15;
}

void dht11_parse_data(int data[5], struct temperature_humidity *result)
{
	result->humidity = ((data[0] << 8) | data[1]) / 256;
	result->temperature = ((data[2] << 8) | data[3]) / 256;
}

void dht22_parse_data(int data[5], struct temperature_humidity *result)
{
	result->humidity = (data[0] * 256 + data[1]) / 10;

	result->temperature = ((data[2] & 0x7F) * 256 + data[3]) / 10;
	if (data[2] & 0x80)
		result->temperature *= -1;
}

int read_data(int gpioPin, void (*parser)(int data[5],
		struct temperature_humidity *result),
		float (*temperature_converter)(float input),
		char temperature_unit)
{
	int data[5];
	int checksum;
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
	for (i = 0; i < MAXTIMINGS; i++) {
		counter = 0;
		while (digitalRead(gpioPin) == laststate) {
			counter++;
			delayMicroseconds(1);
			if (counter == 255)
				break;
		}
		laststate = digitalRead(gpioPin);

		if (counter == 255)
			break;

		/* ignore first 3 transitions */
		if ((i >= 4) && (i%2 == 0)) {
			/* shove each bit into the storage bytes */
			data[j/8] <<= 1;
			if (counter > 16)
				data[j/8] |= 1;
			j++;
		}
	}

	/* check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
	 * print it out if data is good
	 */

	checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
	if (j >= 40 && data[4] == checksum) {
		struct temperature_humidity result;

		parser(data, &result);
		if (result.humidity < 0.0 ||
			result.humidity > 100.0 ||
			result.temperature < -70.0 ||
			result.temperature > 70.0) {
			/* Bad reading */
			return 0;
		}

		printf("Humidity: %.1f %%\nTemperature: %.1f *%c\n",
				result.humidity,
				temperature_converter(result.temperature),
				temperature_unit);
		return 1;
	}

	/* Bad reading */
	return 0;
}

void usage(void)
{
	printf("usage: humiture <GPIO pin #> <sensor> <temperature>\n");
	printf("\t- valid sensor values: dht11, dht22\n");
	printf("\t- valid temperature values: celsius, fahrenheit, kelvin\n");
	exit(1);
}

int main(int argc, char **argv)
{
	void (*parser)(int data[5], struct temperature_humidity *result);
	float (*temperature_converter)(float input);
	char temperature_unit;
	int gpioPin;

	if (argc != 4)
		usage();

	errno = 0;
	gpioPin = strtol(argv[1], NULL, 10);
	if (errno != 0)
		usage();

	if (strcmp(argv[2], "dht11") == 0)
		parser = &dht11_parse_data;
	else if (strcmp(argv[2], "dht22") == 0)
		parser = &dht22_parse_data;
	else
		usage();

	if (strcmp(argv[3], "celsius") == 0) {
		temperature_converter = &celsius_to_celsius;
		temperature_unit = 'C';
	} else if (strcmp(argv[3], "fahrenheit") == 0) {
		temperature_converter = &celsius_to_fahrenheit;
		temperature_unit = 'F';
	} else if (strcmp(argv[3], "kelvin") == 0) {
		temperature_converter = &celsius_to_kelvin;
		temperature_unit = 'K';
	} else
		usage();


	if (wiringPiSetup() == -1)
		return 1;

	while (!read_data(gpioPin, parser, temperature_converter,
				temperature_unit)) {
		/* NOOP */
	}

	return 0;
}

