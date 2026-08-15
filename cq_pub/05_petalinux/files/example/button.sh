#!/bin/sh

GPIO=515

if [ ! -d /sys/class/gpio/gpio$GPIO ]; then
    echo $GPIO > /sys/class/gpio/export
fi

echo in > /sys/class/gpio/gpio$GPIO/direction

while true
do
    cat /sys/class/gpio/gpio$GPIO/value
    sleep 0.2
done

