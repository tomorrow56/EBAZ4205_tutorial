#!/bin/sh

GPIO=512


if [ ! -d /sys/class/gpio/gpio$GPIO ]; then
    echo $GPIO > /sys/class/gpio/export
fi

echo out > /sys/class/gpio/gpio$GPIO/direction

while true
do
    echo 1 > /sys/class/gpio/gpio$GPIO/value
    sleep 1

    echo 0 > /sys/class/gpio/gpio$GPIO/value
    sleep 1
done

