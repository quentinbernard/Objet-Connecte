#install client.c

cd /home/pi
gcc client.c -o client -lwiringPi

sed -i "/sudo \/home\/pi\/installClient.sh/s/^/#/g" /etc/rc.local
