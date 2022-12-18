#!/bin/sh

# pin numbers
export RESET=43
export HANDSHAKE=106
export DATAREADY=140

echo $RESET > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio${RESET}/direction

echo $HANDSHAKE > /sys/class/gpio/export
echo in > /sys/class/gpio/gpio${HANDSHAKE}/direction

echo $DATAREADY > /sys/class/gpio/export
echo in > /sys/class/gpio/gpio${DATAREADY}/direction

while true; do
    # reset
    echo 0 > /sys/class/gpio/gpio${RESET}/value
    usleep 300000
    echo 1 > /sys/class/gpio/gpio${RESET}/value

    # print gpio state
    echo -n "HANDSHAKE = "
    cat /sys/class/gpio/gpio${HANDSHAKE}/value
    echo -n "DATAREADY = "
    cat /sys/class/gpio/gpio${DATAREADY}/value

    # SPI transfer
    ./spidev-xmit -d /dev/spidev1.0 -b 1600 -c 16

    # sleep 3 seconds
    usleep 3000000
done
