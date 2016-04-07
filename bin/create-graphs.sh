#!/bin/bash

GRAPH_DIR=$(dirname "$0")/../web
RRDFILE=$(dirname "$0")/../rrd/humiture.rrd

create_graph()
{
	SCALE=$1
	OUTFILE=$2

	rrdtool graph "${OUTFILE}" \
		--start "-${SCALE}" \
		DEF:tempmax="${RRDFILE}":temp:MAX \
		DEF:tempmin="${RRDFILE}":temp:MIN \
		AREA:tempmax#FF0000:"Max Temperature" \
		AREA:tempmin#FFFF00:"Min Temperature" \
		DEF:humiditymax="${RRDFILE}":humidity:MAX \
		DEF:humiditymin="${RRDFILE}":humidity:MIN \
		LINE1:humiditymax#FF00FF:"Max Relative Humidity (%)" \
		LINE1:humiditymin#0000FF:"Min Relative Humidity (%)"
}

create_graph 4h "${GRAPH_DIR}/humiture_hourly.png"
create_graph 1d "${GRAPH_DIR}/humiture_daily.png"
create_graph 1w "${GRAPH_DIR}/humiture_weekly.png"
create_graph 1m "${GRAPH_DIR}/humiture_monthly.png"
create_graph 1y "${GRAPH_DIR}/humiture_yearly.png"

