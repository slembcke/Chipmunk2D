VERS = ARGV[0]
raise("No version number!") unless VERS

DOXYGEN_PATH = "/Applications/Doxygen.app/Contents/Resources/doxygen"

def system(command)
	puts command
	Kernel.system(command)
end

# Build mainline Chipmunk
CHIPMUNK_DIRNAME = "Chipmunk-#{VERS}"
CHIPMUNK_TMP_PATH = "/tmp/#{CHIPMUNK_DIRNAME}"

system "rm -rf '#{CHIPMUNK_TMP_PATH}'"
system "(git archive --prefix='#{CHIPMUNK_DIRNAME}/' HEAD | tar -xC /tmp)"

Dir.chdir("#{CHIPMUNK_TMP_PATH}/doc-src"){
	system "ruby doxygen_generator.rb"
	system "'#{DOXYGEN_PATH}' Doxyfile"
}

exit

Dir.chdir(CHIPMUNK_TMP_PATH){
	system "(cd doc-src && ruby MakeDocs.rb)"
	
	white_list = [
		"CMakeLists.txt",
		"Demo",
		"doc",
		"extract_protos.rb",
		"include",
		"LICENSE.txt",
		"xcode",
		"msvc",
		"README.textile",
		"src",
		"objectivec",
		"VERSION.txt",
	]
	Dir.glob("*"){|filename| system "rm -rf #{filename}" unless white_list.include?(filename)}
}

system "tar -czf '#{CHIPMUNK_TMP_PATH}.tgz' -C '#{File.dirname(CHIPMUNK_TMP_PATH)}' '#{File.basename(CHIPMUNK_TMP_PATH)}'"

## Build Chipmunk Pro variants
#CHIPMUNK_PRO_TMP_PATH = "/tmp/ChipmunkPro-TMP"
#
#system "rm -rf #{CHIPMUNK_PRO_TMP_PATH}"
#system "git archive --prefix=ChipmunkPro-TMP/ HEAD | tar -xC /tmp"
#
#def build_variant(name, white_list)
#	variant_path = "/tmp/#{name}"
#	system "mkdir #{variant_path}"
#	
#	Dir.glob("*"){|filename| system "cp -r #{filename} #{variant_path}" if white_list.include?(filename)}
#	
#	system "tar -czf #{variant_path}.tgz -C #{File.dirname(variant_path)} #{File.basename(variant_path)}"
#	
#	return variant_path
#end
#
#Dir.chdir(CHIPMUNK_PRO_TMP_PATH){
#	system "cp -r #{CHIPMUNK_TMP_PATH}/ Chipmunk/"
#	
#	system "cd Chipmunk && ruby doxygen_generator.rb"
#	system "#{DOXYGEN_PATH} Doxyfile"
#	system "./iphonestatic.command"
#	
#	build_variant("ChipmunkPro-#{VERS}", [
#		"API-Reference",
#		"AutoGeometry",
#		"BenchmarkApp",
#		"Chipmunk",
#		"ChipmunkPro-iPhone",
#		"ChipmunkPro.xcodeproj",
#		"Experimental",
#		"HastySpace",
#		"iphonestatic.command",
#		"macstatic.command",
#		"Objective-Chipmunk",
#		"README.html",
#		"UnitTests",
#	])
#	
#	build_variant("ChipmunkPro-Trial-#{VERS}", [
#		"API-Reference",
#		"Chipmunk",
#		"ChipmunkPro-iPhone-Trial",
#		"README.html",
#	])
#}
