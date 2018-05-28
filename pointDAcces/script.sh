#! /bin/bash

if [ $(id -ru) -ne 0 ]
then 
	echo "Il faut etre root"
	exit 1
fi


echo "Penser à configurer le clavier en francais par le menu preferences/RPIconfiguration/Localisation/Keyboard"
echo "ENTREE pour continuer"
read pause

echo "Installer paquets logiciels ? (N-y)"
read rep
if [ "$rep" == "y" ]
then 
	echo "Installation apache2 libapache2-mod-php"
	export http_proxy=http://proxy.ec-lille.fr:3128
	apt-get update
	apt-get install apache2 libapache2-mod-php

	echo "Installation dnsmasq hostapd"
	apt-get install dnsmasq hostapd

	echo "Installation python3-pip"
	apt-get install python3-pip
	pip3 install pySerial

fi

echo "ENTREE pour continuer"
read pause

echo "Modif Config.txt"
echo "dtoverlay=pi3-disable-bt" >> /boot/config.txt

echo "ENTREE pour continuer"
read pause

echo "Activation serveur ssh"
update-rc.d ssh enable
service ssh start

echo "ENTREE pour continuer"
read pause

echo "Configuration Apache : rep virtuel metalo"
mkdir -p /home/pi/Desktop/www
cp 010-metalo.conf /etc/apache2/sites-available/
a2ensite 010-metalo
service apache2 restart


echo "Configuration Apache : editer /etc/php/7.0/apache2/php.ini"
echo "Ajouter : error_reporting = E_ALL"
echo "Ajouter : display_errors = On"

echo "ENTREE pour continuer"
read pause

echo "Configuration Apache : cgi"
a2enmod cgi
service apache2 restart

echo "ENTREE pour continuer"
read pause

echo "Configuration Wifi"

echo "denyinterfaces wlan0" >> /etc/dhcpcd.conf
cp interfaces /etc/network/interfaces

cp hostapd.conf /etc/hostapd/hostapd.conf
cp hostapd /etc/default/hostapd

mv /etc/dnsmasq.conf /etc/dnsmasq.conf.orig
cp dnsmasq.conf /etc/dnsmasq.conf

echo "Copie fichiers sur Bureau"
mkdir -p /home/pi/Desktop
cp Desktop.tar.gz /home/pi/Desktop
cd /home/pi/Desktop
tar -zxf Desktop.tar.gz
chown -R pi /home/pi/Desktop/www
chown -R pi "/home/pi/Desktop/TP backend"
chown -R pi /home/pi/Desktop/docs


echo "NB il faudra changer les permissions de /dev/ttyAMA0 apres avoir redemarré"
echo "avec la commande : chmod a+rw /dev/ttyAMA0"

