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

Dir.chdir(CHIPMUNK_TMP_PATH){
	system "(cd doc-src && ruby MakeDocs.rb)"
	
	white_list = [
		"CMakeLists.txt",
		"demo",
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
