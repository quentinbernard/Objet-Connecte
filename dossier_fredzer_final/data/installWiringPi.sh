#install wiringPi


#extraction archive
cd /home/pi
tar -xzf wiringPi.tar.gz
WIRING_PI_DIR=/home/pi/wiringPi-*
cd $WIRING_PI_DIR
./build

sed -i "/\/home\/pi\/installWiringPi.sh/s/^/#/g" /etc/rc.local
