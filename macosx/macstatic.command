#! /bin/bash

cd "`dirname "$0"`"

# clean old
if [ -e Chipmunk-Mac ]; then rm -rf Chipmunk-Mac; fi

mkdir Chipmunk-Mac

# build fat library
xcodebuild -project Chipmunk6.xcodeproj -configuration Release -target ChipmunkStatic && \
xcodebuild -project Chipmunk6.xcodeproj -configuration Debug -target ChipmunkStatic && \

cp build/Debug/libChipmunk.a Chipmunk-Mac/libChipmunk-Debug.a && \
cp build/Release/libChipmunk.a Chipmunk-Mac/libChipmunk.a && \

# copy in headers
rsync -r --exclude=".*" ../include/chipmunk/ Chipmunk-Mac/ && \

echo "Copy Chipmunk-Mac into your project and enjoy."
