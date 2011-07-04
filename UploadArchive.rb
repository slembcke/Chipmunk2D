VERS = ARGV[0]
raise("No version number!") unless VERS

def system(command)
	puts command
	Kernel.system(command)
end

DIR = "/tmp/Chipmunk-#{VERS}"

system("rsync Chipmunk-#{VERS}.tgz slembcke.net:files.slembcke.net/chipmunk/release/Chipmunk-6.x/")
system("rsync -r --exclude='doc-src' #{DIR}/doc/ slembcke.net:files.slembcke.net/chipmunk/release/Chipmunk-6.x/Chipmunk-#{VERS}-Docs")
system("rsync -r /tmp/Objective-Chipmunk-#{VERS}/html/ slembcke.net:files.slembcke.net/chipmunk/release/Chipmunk-6.x/Objective-Chipmunk-#{VERS}-Docs")

open("|ssh slembcke.net 'cd files.slembcke.net/chipmunk/release; sh'", 'w+') do|ssh|
	ssh.puts "rm ChipmunkLatest-Docs; ln -s Chipmunk-6.x/Chipmunk-#{VERS}-Docs/ ChipmunkLatest-Docs"
	ssh.puts "rm Objective-ChipmunkLatest-Docs; ln -s Chipmunk-6.x/Objective-Chipmunk-#{VERS}-Docs/ Objective-ChipmunkLatest-Docs"
	ssh.puts "rm ChipmunkLatest.tgz; ln -s Chipmunk-6.x/Chipmunk-#{VERS}.tgz ChipmunkLatest.tgz"
end
