#!/bin/bash

#SCRIPT.SH: Déploiement d'un OS sur un périphérique

# paramètres par défaut
DEV=/dev/sdc
RPIBOOT=/mnt/rpi-boot
RPIROOT=/mnt/rpi-root

SCRIPT_PATH=$(pwd)
BUILD=$SCRIPT_PATH/build
SRC=$SCRIPT_PATH/src
DATA=$SCRIPT_PATH/data

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

 echo -n "Périphérique à utiliser ? (/dev/...)"
 read DEV

fi


# Première partition du périphérique
DEVBOOT=${DEV}1
DEVROOT=${DEV}2


#Message de résumé : périphérique / partition
clear
echo "Utilisation de $DEVBOOT pour le boot et de $DEVROOT pour le root"
read pause


# 3) Faut-il repartitionner ?

echo -ne "Partitionner $DEV ? [y-N]"
read REP
if [ "$REP" == 'Y' -o "$REP" == 'y' ]; then

 # SI OUI : partitionner, formater
 # NB : on ne peut pas partitionner un périhpérique des partitions montées
 if [ ! -z "$( df -h | grep $DEV | cut -d' ' -f1)" ]; then
  #Démontage
  df -h | grep $DEV | cut -d' ' -f1 |xargs umount
 fi

 fdisk $DEV

 echo "Début du formatage..."
 read pause
 mkfs.vfat $DEVBOOT
 mkfs.ext4 $DEVROOT
 echo "formatage terminé"
	
fi

# 4) Faut-il monter ?
clear
echo "Montages actuels pour $DEV :"
df -h|grep $DEV

echo -ne "Monter $DEVBOOT [y-N] ? "
read REP
if [ "$REP" == 'Y' -o "$REP" == 'y' ]; then

 mkdir -p $RPIBOOT
 mount $DEVBOOT $RPIBOOT


 # Message de résumé : df -h
 df -h|grep $DEV

fi

echo -ne "Monter $DEVROOT [y-N] ? "
read REP
if [ "$REP" == 'Y' -o "$REP" == 'y' ]; then

 mkdir -p $RPIROOT
 mount $DEVROOT $RPIROOT

 # Message de résumé : df -h
 df -h|grep $DEV

fi


#Recopier le contenu du boot raspbian sur RPIBOOT
#NB: root=/dev/mmcblk0p2 dans fichier cmdline.txt
#NB: enlever quiet et splash pour voir les messages des pilotes
echo -ne "Copier dossier /boot [y-N] ? "
read REP
if [ "$REP" == 'Y' -o "$REP" == 'y' ]; then
 cp -r $DATA/bootSave/* $RPIBOOT
fi

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
mkdir -p $RPIROOT/lib
cp $CC_SYSROOT/lib/ld-linux-armhf.so.3 $RPIROOT/lib
cp $CC_SYSROOT/lib/arm-linux-gnueabihf/* $RPIROOT/lib

##################################################
####################BusyBox#######################
##################################################
echo -ne "Installer et configurer BusyBox ? [y-N]"
read REP
if [ "$REP" == "y" -o "$REP" == "Y" ];then

 if [ ! -d $BUILD/busybox* ];then
  # copier busybox dans le rép. build depuis src
  cp $SRC/busybox* $BUILD/busybox.tar.bz2
 
  # changer le rép. courant
  cd $BUILD

  # décompacter
  #bunzip2 -d busybox.tar.bz2
  tar -xjvf busybox.tar.bz2
  rm busybox.tar.bz2

  cd busybox*
 else
  echo "Répertoire BusyBox déjà présent"
  cd $BUILD/busybox*
 fi
 cd $BUILD/busybox*
 # rép. créé contient un nom de version
 BUSYBOX=$(pwd)

 apt-get install libncurses5-dev


 # Opportunité de sauvegarder le fichier .config dans le dossier data pour le réutiliser
 echo -ne "Configurer BusyBox ? [y-N]"
 read REP
 if [ "$REP" == "y" -o "$REP" == "Y" ];then
  make menuconfig
 fi
 # compilation en dynamique
 # 2 erreurs :
 # syncfs -> Coreutils > disbale option "Enable -d and -f flags..."
 #util-linux/lib.a(nsenter.o) : Dans la fonction « nsenter_main » :
 #nsenter.c:(.text.nsenter_main+0x188) : référence indéfinie vers « setns »
 # setns -> Linux System Utilities -> disbale nsenter
 
 # make
 make -j9 CROSS_COMPILE=$PREFIX_CC

 # make install SUR POINT DE MONTAGE
 make CONFIG_PREFIX=$RPIROOT CROSS_COMPILE=$PREFIX_CC install
 
 #installation dynamique
 #ldd $RPIROOT/bin/busybox
 echo "Copie des dépendances déjà fait"

fi

##################################################
##################  Périphériques ################
##################################################
echo -ne "Installer les périphériques ? [y-N]"
read REP
if [ "$REP" == "y" -o "$REP" == "Y" ];then

 #Cf; article RPI From Scratch 2
 mkdir -p $RPIROOT/dev
 mkdir -p $RPIROOT/proc
 mkdir -p $RPIROOT/sys

 # /etc/init.d/rcs
 echo "Création du fichier rcS..."
 mkdir -p $RPIROOT/etc/init.d

 #On vide le fichier
 > $RPIROOT/etc/init.d/rcS

 echo "#! /bin/sh" > $RPIROOT/etc/init.d/rcS
 echo "echo 'montage de proc et sys'" >> $RPIROOT/etc/init.d/rcS
 echo "mount none /proc -t proc" >> $RPIROOT/etc/init.d/rcS
 echo "mount none /sys -t sysfs" >> $RPIROOT/etc/init.d/rcS
 echo "echo 'remontage /'" >> $RPIROOT/etc/init.d/rcS
 echo "mount / -o remount,rw" >> $RPIROOT/etc/init.d/rcS
 echo "mount /dev/mmcblk0p1 /boot" >> $RPIROOT/etc/init.d/rcS
 echo "echo 'montage /boot'" >> $RPIROOT/etc/init.d/rcS
 echo "echo 'Fichiers dev'" >>$RPIROOT/etc/init.d/rcS
 echo "/sbin/mdev -s" >> $RPIROOT/etc/init.d/rcS
 echo "echo /sbin/mdev >/proc/sys/kernel/hotplug" >> $RPIROOT/etc/init.d/rcS
 # => clavier français
 echo "echo Prise en charge du clavier FR" >> $RPIROOT/etc/init.d/rcS
 cp $DATA/azerty.kmap $RPIROOT/etc/french.kmap
 echo "loadkmap < /etc/french.kmap" >> $RPIROOT/etc/init.d/rcS
 # => configuration réseau
 echo "/etc/init.d/rc.network &" >> $RPIROOT/etc/init.d/rcS
 #Fichier rc.network
 cp $DATA/rc.network $RPIROOT/etc/init.d/
 chmod a+x $RPIROOT/etc/init.d/rc.network

 #tests telnetd et httpd
 echo "/etc/init.d/rc.services &" >> $RPIROOT/etc/init.d/rcS
 cp $DATA/rc.services $RPIROOT/etc/init.d/
 chmod a+x $RPIROOT/etc/init.d/rc.services

 chmod a+x $RPIROOT/etc/init.d/rcS

fi

################################################
################# Ncurses ######################
################################################
echo -ne "Installer et recompiler la librairie Ncurses ? [y-N]"
read REP
if [ "$REP" == "y" -o "$REP" == "Y" ];then
 echo -ne "Installer les sources ? [y-N]"
 read REP
 if [ "$REP" == "y" -o "$REP" == "Y" ];then

    #Copie archive source
    cp $SRC/ncurses-*.tar.gz $BUILD/ncurses-*.tar.gz

    cd $BUILD
    #extraction archive
    tar -xf ncurses-*.tar.gz 
    rm ncurses-*.tar.gz
    NCURSES_DIR=`ls $BUILD | grep ncurses-*`
  
  
 fi

 #Variables d'env: 
 #BUILD_CC chemin compilateur natif gcc
 #CC chemin compilateur croisé

 #on exécute le compilateur croisé avec l'option -v :
 #$CC -v --build=x86_64-build_unknown-linux-gnu --host=x86_64-build_unknown-linux-gnu --target=arm-linux-gnuebihf
 #si l'on faisait ça avec le compilateur hôte:
 #--build=x86_64-linux-gnu --host=x86_64-linux-gnu --target=x86_64-linux-gnu

 #compilation de la librairie
 #pas de sudo supplémentaire sinon perte des variables d'env
 #./configure --with-shared --prefix= (..)/target --host=x86_64-build_unknown-linux-gnu

 cd $BUILD/$NCURSES_DIR
 export BUILD_CC=/usr/bin/gcc
 export CC=$CCROISE

 ./configure --prefix=$BUILD/target_ncurses --with-shared --host=x86_64-build_unknown-linux-gnu
 make -j9
 make install

 echo -ne "Compiler les exemples ncurses ? [y-N]"
 read REP
 if [ "$REP" == "y" -o "$REP" == "Y" ];then
  #exemples ncurses
  if [ ! -d $BUILD/examples_ncurses ];then
  cp $SRC/ncurses_programs.tar.gz $BUILD/ncurses_programs.tar.gz
  cd $BUILD/
  tar -xf ncurses_programs.tar.gz
  rm ncurses_programs.tar.gz
  mv ncurses_programs* examples_ncurses

  sed -i 's/CC=gcc/CC=$(CCROISE) -I..\/..\/target_ncurses\/include -I..\/..\/target_ncurses\/include\/ncurses -L..\/..\/target_ncurses\/lib/' $BUILD/examples_ncurses/JustForFun/Makefile
  fi

  cd $BUILD/examples_ncurses/JustForFun

  echo "Compilation des exemples ncurses..."
  export CCROISE=$BIN_CC/arm-linux-gnueabihf-gcc
  make
  echo "Compilation terminée"
  read pause
 fi

 clear
 echo "Copie des librairies partagées..."
 cd $BUILD/target_ncurses/lib/ 
 find ./ -type f -name "*.so*" -exec cp {} $RPIROOT/lib \;
 mkdir -p $RPIROOT/ncurses_examples
 cp -r $BUILD/examples_ncurses/demo/exe/* $RPIROOT/ncurses_examples
 mkdir -p $RPIROOT/usr/share
 cp -r /usr/share/terminfo $RPIROOT/usr/share/
 echo "echo export TERMINFO=/usr/share/terminfo >> /etc/profile" >> $RPIROOT/etc/init.d/rcS
 echo "echo export TERM=linux2.2 >> /etc/profile" >> $RPIROOT/etc/init.d/rcS
 echo "Librairies partagées copiées"
fi

################################################
############# Librairies images ################
################################################
echo -ne "Recompiler les librairies images ? [y-N]"
read REP
if [ "$REP" == "y" -o "$REP" == "Y" ];then
 echo -ne "Désarchiver les librairies ? [y-N]"
 read REP
 if [ "$REP" == "y" -o "$REP" == "Y" ];then
  mkdir -p $BUILD/libs
  cp $SRC/libs/*.tar.gz $BUILD/libs/
  cd $BUILD/libs

  tar -xf jpeg*.tar.gz
  tar -xf libpng*.tar.gz
  tar -xf libungif*.tar.gz
  tar -xf zlib*.tar.gz

  rm -rf *.tar.gz
 fi

 #recompilation libz
 cd $BUILD/libs/zlib*
 export CC=$CCROISE
 ./configure --prefix=$BUILD/libs/
 make -j9
 make install

 read pause
 clear
 
 #recompilation libpng
 cd $BUILD/libs/libpng*
 export LIBRARY_PATH=$BUILD/libs/lib
 export LDFLAGS=-L$BUILD/libs/lib
 export CFLAGS=-I$BUILD/libs/include
 ./configure --prefix=$BUILD/libs/ --host=x86_64-build_unknown-linux-gnu 
 make install

 read pause
 clear

 #recompilation libjpeg
 cd $BUILD/libs/jpeg*
 ./configure --prefix=$BUILD/libs/ --host=x86_64-build_unknown-linux-gnu --enable-shared
 make install

 read pause
 clear

 #recompilation libungif
 cd $BUILD/libs/libungif*
 ./configure --prefix=$BUILD/libs/ --host=x86_64-build_unknown-linux-gnu --enable-shared
 make install

 read pause
 clear

fi

################################################
#################### FBV #######################
################################################
echo -ne "Créer les périphériques du FrameBuffer ? [y-N]"
read REP
if [ "$REP" == "y" -o "$REP" == "Y" ];then
 cd $RPIROOT/dev
 /sbin/MAKEDEV fb &
 
 if [ ! -d $BUILD/fbv ];then
  cp $SRC/fbv* $BUILD/fbv.tar.gz

  cd $BUILD
  tar -xf fbv.tar.gz
  rm fbv.tar.gz

  mv $BUILD/fbv* $BUILD/fbv
 else
  echo "Dossier fbv existe deja"
 fi
 cd $BUILD/fbv

 echo -ne "Modifier le Makefile de fbv ? [y-N]"
 read REP
 if [ "$REP" == "y" -o "$REP" == "Y" ];then
  vi $BUILD/fbv/Makefile
 fi

 #echo vérif. présence des libs
 ./configure --without-bmp

 make clean
 make
 
 cp fbv $RPIROOT/bin

 mkdir -p $RPIROOT/images_fbv
 cp $DATA/pingouins/* $RPIROOT/images_fbv

 
fi


################################################
#################### GPIO #######################
################################################



echo -ne "Installer et recompiler la librairie GPIO ? [y-N]"
read REP
if [ "$REP" == "y" -o "$REP" == "Y" ];then
 echo -ne "Installer les sources ? [y-N]"
 read REP
 if [ "$REP" == "y" -o "$REP" == "Y" ];then

    #Copie archive source
    cp $SRC/wiringPi-*.tar.gz $BUILD/wiringPi-*.tar.gz

    cd $BUILD
    #extraction archive
    tar -xf wiringPi-*.tar.gz 
    rm wiringPi-*.tar.gz
    WIRINGPI_DIR=`ls $BUILD | grep wiringPi-*`
  
  
 fi

mkdir -p $BUILD/target_wp/bin
mkdir -p $BUILD/target_wp/lib
mkdir -p $BUILD/target_wp/include

cp $DATA/wiringPi/Makefile $BUILD/$WIRINGPI_DIR/wiringPi/
cp $DATA/devLib/Makefile $BUILD/$WIRINGPI_DIR/devLib/
cp $DATA/gpio/Makefile $BUILD/$WIRINGPI_DIR/gpio/

cd $BUILD/$WIRINGPI_DIR
./build

 cp -r $BUILD/target_wp $RPIROOT/
 cp $RPIROOT/target_wp/* $RPIROOT/lib
 ln -sf $RPIROOT/target_wp/lib/libwiringPi.so.2.44 $RPIROOT/lib/libwiringPi.so
 ln -sf $RPIROOT/target_wp/lib/libwiringPiDev.so.2.44 $RPIROOT/lib/libwiringPiDev.so

cp $DATA/gpio_examples/* $BUILD/$WIRINGPI_DIR/examples
cd $BUILD/$WIRINGPI_DIR/examples
make blink
cp $BUILD/$WIRINGPI_DIR/examples/blink $RPIROOT/bin

fi


