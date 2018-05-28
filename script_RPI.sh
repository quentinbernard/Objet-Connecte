#!/bin/bash

RPI_ROOT=/mnt/rpi-root

SCRIPT_PATH=$(pwd)
BUILD=$SCRIPT_PATH/build
SRC=$SCRIPT_PATH/src
DATA=$SCRIPT_PATH/data

#PATH du script de configuration de la wifi
CONF_WIFI=$DATA/script

$CONF_WIFI

echo -ne "Installer compilateur croisé ? [y-N]"
read REP
if [ "$REP" == "y" -o "$REP" == "Y" ];then
if [ ! -d $BUILD/tools-master ];then
  # copier busybox dans le rép. build depuis src
  cp $SRC/tools-master.zip $BUILD/
 
  # changer le rép. courant
  cd $BUILD

  # décompacter
  unzip tools-master.zip
  rm tools-master.zip

 else
  echo "Répertoire Tools-master déjà présent"
 fi
fi

#Variables du compilateur croisé
BIN_CC=$SCRIPT_PATH/build/tools-master/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/bin
export CCROISE=$BIN_CC/arm-linux-gnueabihf-gcc
CC_SYSROOT=$($CCROISE -print-sysroot)
PREFIX_CC=$BIN_CC/arm-linux-gnueabihf-

# Contenu du rep root sur la carte SD (rpi-root)
mkdir -p $RPI_ROOT/lib
cp $CC_SYSROOT/lib/ld-linux-armhf.so.3 $RPI_ROOT/lib
cp $CC_SYSROOT/lib/arm-linux-gnueabihf/* $RPI_ROOT/lib

echo "Compilation de la librairie WiringPi"
#Compilation wiringPi
echo -ne "Désarchiver la librairie WiringPi ? [y-N]"
read REP
if [ "$REP" == "y" -o "$REP" == "Y" ];then
    #Copie archive source
    cp $SRC/wiringPi-*.tar.gz $BUILD/wiringPi-*.tar.gz

    cd $BUILD
    #extraction archive
    tar -xf wiringPi-*.tar.gz 
    rm wiringPi-*.tar.gz
    WIRING_PI_DIR=`ls $BUILD | grep wiringPi-*`
fi

cd $BUILD/$WIRING_PI_DIR
export CC=$CCROISE
make

cp /usr/lib/libwiringPi.so $RPI_ROOT/usr/lib/


