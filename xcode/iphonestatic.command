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

PROJECT = "Chipmunk7.xcodeproj"
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

def build(target, configuration, simulator)
	sdk_os = (simulator ? "iphonesimulator" : "iphoneos")
	sdk = "#{sdk_os}#{IOS_SDK_VERSION}"
	
	command = "xcodebuild -project #{PROJECT} -sdk #{sdk} -configuration #{configuration} -target #{target}"
	system command
	
	return "build/#{configuration}-#{sdk_os}/lib#{target}.a"
end

def build_fat_lib(target, copy_list)
	iphone_lib = build(target, "Release", false)
	simulator_lib = build(target, "Debug", true)
	
	dirname = "#{target}"
	
	system "rm -rf '#{dirname}'"
	system "mkdir '#{dirname}'"
	
	system "lipo #{iphone_lib} #{simulator_lib} -create -output '#{dirname}/lib#{target}.a'"
	
	copy_list.each{|src| system "rsync -r --exclude='.*' '#{src}' '#{dirname}'"}
	
	puts "\n#{dirname}/ Succesfully built"
end


build_fat_lib( "Chipmunk-iOS", [
		"../include",
])

build_fat_lib("ObjectiveChipmunk-iOS", [
	"../include",
	"../objectivec/include",
])

BUILD_LOG.delete
