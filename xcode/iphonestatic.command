#! /bin/bash

cd "`dirname "$0"`"

# clean old
if [ -e Chipmunk-iPhone ]; then rm -rf Chipmunk-iPhone; fi

mkdir Chipmunk-iPhone

# build fat library
xcodebuild -project Chipmunk6.xcodeproj -sdk iphoneos5.0 -configuration Release -target ChipmunkStatic-iPhone && \
xcodebuild -project Chipmunk6.xcodeproj -sdk iphonesimulator5.0 -arch i386 -configuration Debug -target ChipmunkStatic-iPhone && \
lipo build/Debug-iphonesimulator/libChipmunk-iPhone.a build/Release-iphoneos/libChipmunk-iPhone.a -create -output Chipmunk-iPhone/libChipmunk-iPhone.a && \

# copy in headers
rsync -r --exclude=".*" ../include/chipmunk/ Chipmunk-iPhone/ && \

open Chipmunk-iPhone && \
echo "Copy Chipmunk-iPhone into your project and enjoy."
