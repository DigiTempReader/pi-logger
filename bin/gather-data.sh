#!/bin/bash

set -e # Exit if any commands fail
set -u # Warn about uninitialized variables

GPIO_PIN=${1:-}
SENSOR=${2:-}
TEMPERATURE_UNIT=${3:-}

if [ "${GPIO_PIN}" = "" ] || [ "${SENSOR}" = "" ] || [ "${TEMPERATURE_UNIT}" = "" ] ; then
        echo "usage: $0 <GPIO pin #> <sensor> <temperature unit>" >&2
	echo "Note: This is the GPIO pin number used by wiringPi"
        exit 1
fi


RRDFILE=$(dirname "$0")/../rrd/humiture.rrd

if [ ! -f "${RRDFILE}" ] ; then
	rrdtool create "${RRDFILE}" --start N --step 300 \
		DS:temp:GAUGE:600:U:U \
		DS:humidity:GAUGE:600:U:U \
		RRA:MIN:0.5:1:12 \
		RRA:MIN:0.5:1:288 \
		RRA:MIN:0.5:12:168 \
		RRA:MIN:0.5:12:720 \
		RRA:MIN:0.5:288:365 \
		RRA:MAX:0.5:1:12 \
		RRA:MAX:0.5:1:288 \
		RRA:MAX:0.5:12:168 \
		RRA:MAX:0.5:12:720 \
		RRA:MAX:0.5:288:365
fi

TMP=$(mktemp)
BASEDIR=$(dirname "$0")
"${BASEDIR}"/humiture "${GPIO_PIN}" "${SENSOR}" "${TEMPERATURE_UNIT}" > "${TMP}"
HUMIDITY=$(grep Humidity "${TMP}" | awk '{print $2}')
TEMPERATURE=$(grep Temperature "${TMP}" | awk '{print $2}')
rm -f "${TMP}"

rrdtool update "${RRDFILE}" "N:${TEMPERATURE}:${HUMIDITY}"
