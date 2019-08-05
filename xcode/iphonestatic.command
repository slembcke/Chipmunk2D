#! /usr/bin/ruby

Dir.chdir(File.dirname($0))

require 'Tempfile'
BUILD_LOG = Tempfile.new("Chipmunk-")
BUILD_LOG_PATH = BUILD_LOG.path

def log(string)
	puts string
	open(BUILD_LOG_PATH, 'a'){|f| f.puts string}
end

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
	
	command = "xcodebuild -project #{PROJECT} -sdk #{sdk_os} -configuration #{configuration} -target #{target}"
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
