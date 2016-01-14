#!/bin/bash

RRDFILE=$(dirname "$0")/../rrd/basement.rrd

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
dirname "$0"/humiture > "${TMP}"
HUMIDITY=$(grep Humidity "${TMP}" | awk '{print $2}')
TEMPERATURE=$(grep Temperature "${TMP}" | awk '{print $2}')
rm -f "${TMP}"

rrdtool update "${RRDFILE}" "N:${TEMPERATURE}:${HUMIDITY}"
