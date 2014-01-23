#! /usr/bin/ruby

Dir.chdir(File.dirname($0))

require 'Tempfile'
BUILD_LOG = Tempfile.new("Chipmunk-")
BUILD_LOG_PATH = BUILD_LOG.path

def log(string)
	puts string
	open(BUILD_LOG_PATH, 'a'){|f| f.puts string}
end

def latest_sdk()
	sdks = `xcodebuild -showsdks`.split("\n")
	
	versions = sdks.map do|elt|
		# Match only lines with "iphoneos" in them.
		m = elt.match(/iphoneos(\d\.\d)/)
		(m ? m.captures[0] : "0.0")
	end
	
	return versions.max
end

# Or you can pick a specific version string (ex: "5.1")
IOS_SDK_VERSION = latest_sdk()

log("Building using iOS SDK #{IOS_SDK_VERSION}")

VERBOSE = (not ARGV.include?("--quiet"))

def system(command)
	log "> #{command}"
	
	result = Kernel.system(VERBOSE ? "#{command} | tee -a #{BUILD_LOG_PATH}; exit ${PIPESTATUS[0]}" : "#{command} >> #{BUILD_LOG_PATH}")
	unless $? == 0
		log "==========================================="
		log "Command failed with status #{$?}: #{command}"
		log "Build errors encountered. Aborting build script"
		log "Check the build log for more information: #{BUILD_LOG_PATH}"
		raise
	end
end

OUTPUT_DIR_NAME = "Chipmunk-iPhone"
system "rm -rf #{OUTPUT_DIR_NAME}"
system "mkdir #{OUTPUT_DIR_NAME}"

system "xcodebuild -project Chipmunk7.xcodeproj -sdk iphoneos#{IOS_SDK_VERSION} -configuration Release -target ChipmunkStatic-iPhone"
system "xcodebuild -project Chipmunk7.xcodeproj -sdk iphonesimulator#{IOS_SDK_VERSION} -arch i386 -configuration Debug -target ChipmunkStatic-iPhone"
system "lipo build/Debug-iphonesimulator/libChipmunk-iPhone.a build/Release-iphoneos/libChipmunk-iPhone.a -create -output #{OUTPUT_DIR_NAME}/libChipmunk-iPhone.a"

system "xcodebuild -project Chipmunk7.xcodeproj -sdk iphoneos#{IOS_SDK_VERSION} -configuration Release -target ObjectiveChipmunk-iPhone"
system "xcodebuild -project Chipmunk7.xcodeproj -sdk iphonesimulator#{IOS_SDK_VERSION} -arch i386 -configuration Debug -target ObjectiveChipmunk-iPhone"
system "lipo build/Debug-iphonesimulator/libObjectiveChipmunk-iPhone.a build/Release-iphoneos/libObjectiveChipmunk-iPhone.a -create -output #{OUTPUT_DIR_NAME}/libObjectiveChipmunk-iPhone.a"

system "rsync -r --exclude='.*' ../include/ #{OUTPUT_DIR_NAME}"
system "rsync -r --exclude='.*' ../objectivec/include/ #{OUTPUT_DIR_NAME}"
system "open #{OUTPUT_DIR_NAME}"

puts "Copy #{OUTPUT_DIR_NAME} into your project and enjoy."

BUILD_LOG.delete
