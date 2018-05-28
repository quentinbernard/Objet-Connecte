#!/bin/bash

RPI0_ROOT=

SCRIPT_PATH=$(pwd)
BUILD=$SCRIPT_PATH/build
SRC=$SCRIPT_PATH/src
DATA=$SCRIPT_PATH/data

WPA_CONF=$DATA/wpa_supplicant.conf
INTERFACES_CONF=$DATA/interfaces

chmod +x $WPA_CONF
chmod +x $INTERFACES_CONF

cp $WPA_CONF $RPI0_ROOT/etc/wpa_supplicant/wpa_supplicant.conf
cp $INTERFACES_CONF $RPI0_ROOT/etc/network/interfaces

