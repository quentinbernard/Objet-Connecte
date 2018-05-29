#!/bin/bash

SCRIPT_PATH=$(pwd)

SPI=$SCRIPT_PATH/SPI-Py


# 1) Suis-je root ? (uid = 0)
uid=`grep $USER /etc/passwd | awk -F: '{ print $3 }'`
if [ $uid -ne 0 ] #ou [ $(id -ru) -ne 0 ]
then
 echo "Il faut être root"
 exit 1
fi

echo "Installation de python"
apt-get install python
apt-get install python-dev

echo "Installation de SPI Py"
cd $SPI
python setup.py install

echo "Activation du spi"
#On dé-commente la ligne pour activer le spi
sed -i '/dtparam=spi/s/^#//g' /boot/config.txt 



gcc read.c -o read.exe -lwiringPi

