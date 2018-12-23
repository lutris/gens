#!/bin/bash

dpkg --add-architecture i386
apt-get update
apt-get install -y nasm gcc-multilib g++-multilib git
apt-get install -y libgtk2.0-dev:i386 libsdl1.2-dev:i386
autoreconf -fi
export PKG_CONFIG_PATH=/usr/lib/i386-linux-gnu/pkgconfig/
if [ -z "$TRAVIS_TAG" ]; then
    dpkg-buildpackage
else
    dpkg-buildpackage -v $TRAVIS_TAG
fi
#./configure --host=i686-linux-gnu "CFLAGS=-m32" "CXXFLAGS=-m32" "LDFLAGS=-m32"
#make
