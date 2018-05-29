#!/bin/bash

RPI_ROOT=/mnt/rpi-root

SCRIPT_PATH=$(pwd)
BUILD=$SCRIPT_PATH/build
SRC=$SCRIPT_PATH/src
DATA=$SCRIPT_PATH/data

# Suis-je root ? (uid = 0)
uid=`grep $USER /etc/passwd | awk -F: '{ print $3 }'`
if [ $uid -ne 0 ] #ou [ $(id -ru) -ne 0 ]
then
 echo "Il faut être root"
 exit 1
fi

# 2) Valider le périphérique à utiliser
dmesg | grep "sd" | tail

# afficher les derniers messages du noyau
# demande la saisie ou confirmation du périphérique à l'utilisateur

echo -ne "Utiliser $DEV ? [Y-n]"
read REP
if [ "$REP" == 'N' -o "$REP" == 'n' ]; then

 echo -ne "Périphérique à utiliser ? (/dev/...)"
 read DEV

fi


# Première partition du périphérique
DEVBOOT=${DEV}1
DEVROOT=${DEV}2


#Message de résumé : périphérique / partition
clear
echo "Utilisation de $DEVBOOT pour le boot et de $DEVROOT pour le root"
read pause

# 4) Faut-il monter ?
clear
echo "Montages actuels pour $DEV :"
df -h|grep $DEV

echo -ne "Monter $DEVBOOT en tant que $RPI_BOOT [y-N] ? "
read REP
if [ "$REP" == 'Y' -o "$REP" == 'y' ]; then

 mkdir -p $RPI_BOOT
 mount $DEVBOOT $RPI_BOOT

 # Message de résumé : df -h
 df -h|grep $DEV

fi

echo -ne "Monter $DEVROOT en tant que $RPI_ROOT [y-N] ? "
read REP
if [ "$REP" == 'Y' -o "$REP" == 'y' ]; then

 mkdir -p $RPI_ROOT
 mount $DEVROOT $RPI_ROOT

 # Message de résumé : df -h
 df -h|grep $DEV

fi

############# Configuration Wifi ################

#PATH du script de configuration de la wifi
CONF_WIFI=$SRC/routeur_wifi

#Modification du rc.local afin de lancer le script de configuration du wi-fi au démarrage
cp -R $CONF_WIFI $RPI_ROOT/home/pi
chmod +x /$RPI_ROOT/home/pi/routeur_wifi/script.sh
cp $DATA/rc.local $RPI_ROOT/etc/rc.local

################# Wiring PI ########################

echo "Installation de la librairie WiringPi"
#wiringPi
echo -ne "Désarchiver la librairie WiringPi ? [y-N]"
read REP
if [ "$REP" == "y" -o "$REP" == "Y" ];then
    #Copie archive source
    cp $SRC/wiringPi-*.tar.gz $RPI_ROOT/home/pi/wiringPi.tar.gz
    cp $DATA/installWiringPi.sh $RPI_ROOT/home/pi/
fi

echo "Installation wiringPi terminée, appuyez pour continuer"
read pause
clear

################## serveur.c/readNFC ########################

echo "Compilation des fichiers sources"
cd $SRC
cp serveur.c $RPI_ROOT/home/pi
cp stream.h $RPI_ROOT/home/pi

echo "Installation de SPI Py"
cp -r $DATA/SPI-Py $RPI_ROOT/home/pi

echo "Activation du spi"
#On dé-commente la ligne pour activer le spi
sed -i '/dtparam=spi/s/^#//g' /boot/config.txt 

cp $SRC/read.c $RPI_ROOT/home/pi

cp $DATA/installServeur.sh $RPI_ROOT/home/pi/
cp $DATA/rc.local.serveur $RPI_ROOT/etc/rc.local


