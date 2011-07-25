VERS = ARGV[0]
raise("No version number!") unless VERS

def system(command)
	puts command
	Kernel.system(command)
end

DIR = "/tmp/Chipmunk-#{VERS}"

# Build Objective-Chipmunk
#system("cd ../Objective-Chipmunk; ruby BuildReleaseArchive.rb #{VERS}")

# Export, clean, and prepare
system("rm -rf #{DIR}")
system("git-export . #{DIR}")

to_remove = [
	"BuildReleaseArchive.rb",
	"TODO.txt",
	"ReleaseChecklist.txt",
	"UploadArchive.rb",
	"doxygen_generator.rb",
	"Doxyfile",
	"example",
]
system("cd #{DIR}; rm -rf #{to_remove.join(" ")}")

# Build docs
system("cd #{DIR}/doc/doc-src; ruby make_docs.rb")
system("ruby doxygen_generator.rb")
system("/Applications/Doxygen.app/Contents/Resources/doxygen Doxyfile")
system("cp -R doxygen #{DIR}/doxygen")


# Copy in Objective-Chipmunk stuff
#system("mkdir -p #{DIR}/Objective-Chipmunk/")
#system("cp -R ../Objective-Chipmunk/doxygen/html/ #{DIR}/Objective-Chipmunk/API\\ Docs");
#
#system("mkdir -p #{DIR}/Objective-Chipmunk/Objective-Chipmunk")
#system("cp -R ../Objective-Chipmunk/Objective-Chipmunk-simulator #{DIR}/Objective-Chipmunk/Objective-Chipmunk/")
#system("cp -R ../Objective-Chipmunk/Readme.rtf #{DIR}/Objective-Chipmunk/Objective-Chipmunk/")
#
#system("svn export ../iPhoneChipmunk #{DIR}/Objective-Chipmunk/iPhoneChipmunk")
#system("svn export ../iPhoneSnap #{DIR}/Objective-Chipmunk/iPhoneSnap")
#system("svn export ../SimpleObjectiveChipmunk #{DIR}/Objective-Chipmunk/SimpleObjectiveChipmunk")
#system("find #{DIR} -name '*.psd' -print0 | xargs -0 rm")

system("tar -C /tmp -czf Chipmunk-#{VERS}.tgz Chipmunk-#{VERS}/")

system("open #{DIR}")
