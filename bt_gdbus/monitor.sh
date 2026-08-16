#!/bin/bash

sudo dbus-monitor --system "destination='org.bluez'" "sender='org.bluez'"

