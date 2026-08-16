#!/bin/sh

if [ $# -ne 1 ]; then
    echo "Usage: $0 <bitstream.bin>"
    exit 1
fi

BINFILE="$1"
BASENAME="$(basename "$BINFILE")"

if [ ! -f "$BINFILE" ]; then
    echo "Error: file not found: $BINFILE"
    exit 1
fi

mkdir -p /lib/firmware/

cp "$BINFILE" "/lib/firmware/$BASENAME" || exit 1

echo 0 > /sys/class/fpga_manager/fpga0/flags || exit 1

echo "$BASENAME" > /sys/class/fpga_manager/fpga0/firmware || exit 1

STATE=$(cat /sys/class/fpga_manager/fpga0/state)

echo "FPGA state: $STATE"

if [ "$STATE" = "operating" ]; then
    echo "FPGA programming successful."
else
    echo "FPGA programming may have failed."
    exit 1
fi

