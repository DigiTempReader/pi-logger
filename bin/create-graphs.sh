#!/bin/bash

GRAPH_DIR=$(dirname "$0")/../web
RRDFILE=$(dirname "$0")/../rrd/humiture.rrd

create_graph()
{
	SCALE=$1
	OUTFILE=$2

	rrdtool graph "${OUTFILE}" \
		--start "-${SCALE}" \
		DEF:temp="${RRDFILE}":temp:MIN \
		AREA:temp#FFFF00:"Temperature (*F)" \
		DEF:humidity="${RRDFILE}":humidity:MAX \
		LINE1:humidity#0000FF:"Relative Humidity (%)"
}

create_graph 4h "${GRAPH_DIR}/humiture_hourly.png"
create_graph 1d "${GRAPH_DIR}/humiture_daily.png"
create_graph 1w "${GRAPH_DIR}/humiture_weekly.png"
create_graph 1m "${GRAPH_DIR}/humiture_monthly.png"
create_graph 1y "${GRAPH_DIR}/humiture_yearly.png"

