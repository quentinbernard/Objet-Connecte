#! /bin/bash

CONF_WIFI=/home/pi/routeur_wifi

if [ $(id -ru) -ne 0 ]
then 
	echo "Il faut etre root"
	exit 1
fi


echo "Penser à configurer le clavier en francais par le menu preferences/RPIconfiguration/Localisation/Keyboard"

echo "Installation dnsmasq hostapd"
apt-get -y install dnsmasq python python-dev
apt-get install hostapd -yqq


echo "Configuration Wifi"

echo "denyinterfaces wlan0" >> /etc/dhcpcd.conf
cp $CONF_WIFI/interfaces /etc/network/interfaces

cp $CONF_WIFI/hostapd.conf /etc/hostapd/hostapd.conf
cp $CONF_WIFI/hostapd /etc/default/hostapd

mv /etc/dnsmasq.conf /etc/dnsmasq.conf.orig
cp $CONF_WIFI/dnsmasq.conf /etc/dnsmasq.conf

sed -i '/\/home\/pi\/routeur/s/^/#/g' /etc/rc.local

reboot
