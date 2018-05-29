#install serveur.c

cd /home/pi
#exe processus serveur et alarme
gcc serveur.c -o serveur -lwiringPi

#Installation de SPI Py
cd /home/pi/SPI-Py
python setup.py install

cd /home/pi
#exe lecture NFC
gcc read.c -o readNFC -lwiringPi


sed -i "/\/home\/pi\/installServeur.sh/s/^/#/g" /etc/rc.local
