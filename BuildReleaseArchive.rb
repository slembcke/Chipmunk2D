VERS = ARGV[0]
raise("No version number!") unless VERS

def system(command)
	puts command
	Kernel.system(command)
end

DIR = "/tmp/Chipmunk-#{VERS}"

# Build Objective-Chipmunk
system("cd ../Objective-Chipmunk; ruby BuildReleaseArchive.rb #{VERS}")

# Export, clean, and prepare
system("rm -rf #{DIR}")
system("svn export . #{DIR}")
system("cd #{DIR}; rm BuildReleaseArchive.rb TODO.txt ReleaseChecklist.txt UploadArchive.rb")
system("mkdir -p #{DIR}/Objective-Chipmunk/")

# Build docs
system("cd #{DIR}/doc/doc-src; ruby make_docs.rb")

# Copy in Objective-Chipmunk stuff
system("cp -R ../Objective-Chipmunk/doxygen/html/ #{DIR}/Objective-Chipmunk/API\\ Docs");

system("mkdir -p #{DIR}/Objective-Chipmunk/Objective-Chipmunk")
system("cp -R ../Objective-Chipmunk/Objective-Chipmunk-simulator #{DIR}/Objective-Chipmunk/Objective-Chipmunk/")
system("cp -R ../Objective-Chipmunk/Readme.rtf #{DIR}/Objective-Chipmunk/Objective-Chipmunk/")

system("svn export ../iPhoneChipmunk #{DIR}/Objective-Chipmunk/iPhoneChipmunk")
system("svn export ../iPhoneSnap #{DIR}/Objective-Chipmunk/iPhoneSnap")
system("svn export ../SimpleObjectiveChipmunk #{DIR}/Objective-Chipmunk/SimpleObjectiveChipmunk")

system("tar -C /tmp -czf Chipmunk-#{VERS}.tgz Chipmunk-#{VERS}/")

system("open #{DIR}")
