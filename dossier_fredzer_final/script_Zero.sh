#!/bin/bash

# paramètres par défaut
DEV=/dev/sdc
ZERO_BOOT=/media/eleve/boot
ZERO_ROOT=/media/eleve/rootfs

SCRIPT_PATH=$(pwd)
BUILD=$SCRIPT_PATH/build
SRC=$SCRIPT_PATH/src
DATA=$SCRIPT_PATH/data

############Préparation#############################

# 1) Suis-je root ? (uid = 0)
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

echo -ne "Monter $DEVBOOT en tant que $ZERO_BOOT [y-N] ? "
read REP
if [ "$REP" == 'Y' -o "$REP" == 'y' ]; then

 mkdir -p $ZERO_BOOT
 mount $DEVBOOT $ZERO_BOOT

 # Message de résumé : df -h
 df -h|grep $DEV

fi

echo -ne "Monter $DEVROOT en tant que $ZERO_ROOT [y-N] ? "
read REP
if [ "$REP" == 'Y' -o "$REP" == 'y' ]; then

 mkdir -p $ZERO_ROOT
 mount $DEVROOT $ZERO_ROOT

 # Message de résumé : df -h
 df -h|grep $DEV

fi

############## Point D'Accès Wifi ##################

#Fichiers de configuration du point d'accès
WPA_CONF=$DATA/wpa_supplicant.conf
INTERFACES_CONF=$DATA/interfaces

echo "Configuration connexion au point d'accès..."
chmod a+x $WPA_CONF
chmod a+x $INTERFACES_CONF

cp $WPA_CONF $ZERO_ROOT/etc/wpa_supplicant/wpa_supplicant.conf
cp $INTERFACES_CONF $ZERO_ROOT/etc/network/interfaces

echo "Configuration point d'accès Wifi terminée, appuyez pour continuer"
read pause
clear

################# Wiring PI ########################

echo "Installation de la librairie WiringPi"
#wiringPi
#Copie archive source
cp $SRC/wiringPi-*.tar.gz $ZERO_ROOT/home/pi/wiringPi.tar.gz
cp $DATA/installWiringPi.sh $ZERO_ROOT/home/pi/

echo "Installation wiringPi terminée, appuyez pour continuer"
read pause
clear

################## Client.c ########################

echo "Compilation des fichiers sources"
cd $SRC
cp client.c $ZERO_ROOT/home/pi
cp stream.h $ZERO_ROOT/home/pi

cp $DATA/installClient.sh $ZERO_ROOT/home/pi/
cp $DATA/rc.local.zero $ZERO_ROOT/etc/rc.local


