#!/bin/bash

RPI_ROOT=/mnt/rpi-root

SCRIPT_PATH=$(pwd)
BUILD=$SCRIPT_PATH/build
SRC=$SCRIPT_PATH/src
DATA=$SCRIPT_PATH/data

#PATH du script de configuration de la wifi
CONF_WIFI=$DATA/script

#TODO config point d'accès wifi

################# Wiring PI ########################

echo "Installation de la librairie WiringPi"
#wiringPi
echo -ne "Désarchiver la librairie WiringPi ? [y-N]"
read REP
if [ "$REP" == "y" -o "$REP" == "Y" ];then
    #Copie archive source
    cp $SRC/wiringPi-*.tar.gz $RPI_ROOT/home/pi/wiringPi-*.tar.gz
    cp $DATA/installWiringPi.sh $RPI_ROOT/home/pi/
fi

echo "Installation wiringPi terminée, appuyez pour continuer"
read pause
clear

################## serveur.c ########################

echo "Compilation des fichiers sources"
cd $SRC
cp serveur.c $RPI_ROOT/home/pi
cp stream.h $RPI_ROOT/home/pi

cp $DATA/installServeur.sh $RPI_ROOT/home/pi/
cp $DATA/rc.local.zero $RPI_ROOT/etc/rc.local


