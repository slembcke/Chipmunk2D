#! /usr/bin/ruby

Dir.chdir(File.dirname($0))

require 'Tempfile'
BUILD_LOG = Tempfile.new("Chipmunk-")
BUILD_LOG_PATH = BUILD_LOG.path

def log(string)
	puts string
	open(BUILD_LOG_PATH, 'a'){|f| f.puts string}
end

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

OUTPUT_DIR_NAME = "Chipmunk-Mac"
system "rm -rf #{OUTPUT_DIR_NAME}"
system "mkdir #{OUTPUT_DIR_NAME}"

system "xcodebuild -project Chipmunk7.xcodeproj -configuration Release -target ChipmunkStatic"
system "xcodebuild -project Chipmunk7.xcodeproj -configuration Debug -target ChipmunkStatic"

system "cp build/Debug/libChipmunk.a #{OUTPUT_DIR_NAME}/libChipmunk-Debug.a"
system "cp build/Release/libChipmunk.a #{OUTPUT_DIR_NAME}/libChipmunk.a"

system "rsync -r --exclude='.*' ../include/chipmunk/ #{OUTPUT_DIR_NAME}"
system "open #{OUTPUT_DIR_NAME}"

puts "Copy #{OUTPUT_DIR_NAME} into your project and enjoy."

BUILD_LOG.delete
