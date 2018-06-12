#!/bin/bash

# after tools bootstraped
# in new env chroot

GROUP_AVAILABLE=$(getent group simple)
if [ ! -z "$GROUP_AVAILABLE" ]; then
	echo group already availble
	GROUP_ID=$(echo "$GROUP_AVAILABLE" | cut -d : -f3)
	if [ "$GROUP_ID" != 250 ]; then
		echo but have wrong GID
		exit 1
	fi
else
	echo group does not exist. adding...
	groupadd --gid 250 simple
fi


USER_AVAILABLE=$(getent passwd simple)
if [ ! -z "$USER_AVAILABLE" ]; then
	echo user already availble
	USER_ID=$(echo "$USER_AVAILABLE" | cut -d : -f3)
	if [ "$USER_ID" != 250 ]; then
		echo "but have wrong GID"
		exit 1
	fi
	USER_GROUP=$(echo "$USER_AVAILABLE" | cut -d : -f4)
	if [ "$USER_GROUP" != 250 ]; then
		echo "user does not added in right group"
		exit 1
	fi
else
	echo user does not exist. adding...
	useradd -s /bin/false -d /var/tmp/simple -g simple --uid 250 -m -k /dev/null simple
fi

mkdir -vp /etc/simple
cp etc/simple/make.conf /etc/simple

install -vd  /etc/simple
install -v -m644  etc/simple/make.conf /etc/simple

install -vd  /var/cache/simple
cp      -vr  var/cache/simple/* /var/cache/simple
chown   -vR simple:simple  /var/cache/simple

install -vd  /usr/lib/simple
install -vD -m744 -o simple -g simple  usr/lib/simple/* /usr/lib/simple
chmod 644  /usr/lib/simple/libsimple

install -v -m750  usr/sbin/simple /usr/sbin
