# clean old
if [ -e Chipmunk-iPhone ]; then rm -rf Chipmunk-iPhone; fi

mkdir -p Chipmunk-iPhone/constraints

# build fat library
xcodebuild -project Chipmunk.xcodeproj -sdk iphoneos4.0 -configuration Release -target ChipmunkStatic-iPhone
xcodebuild -project Chipmunk.xcodeproj -sdk iphonesimulator4.0 -configuration Debug -target ChipmunkStatic-iPhone
lipo build/Debug-iphonesimulator/libChipmunk-iPhone.a build/Release-iphoneos/libChipmunk-iPhone.a -create -output Chipmunk-iPhone/libChipmunk-iPhone.a

# copy in headers
cp ../include/chipmunk/*.h Chipmunk-iPhone/
cp ../include/chipmunk/constraints/*.h Chipmunk-iPhone/constraints

open Chipmunk-iPhone
echo "Copy Chipmunk-iPhone into your project and enjoy."
