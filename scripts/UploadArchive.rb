VERS = ARGV[0]
raise("No version number!") unless VERS

def system(command)
	puts command
	Kernel.system(command)
end

SSH_CONNECT = "|ssh chipmunk-physics.net 'cd 'chipmunk-physics.net/release'; sh'"

def system_remote(command)
	puts "#{SSH_CONNECT} > #{command}"
	open(SSH_CONNECT, 'w+'){|ssh| ssh.puts command}
end

system("rsync '/tmp/Chipmunk-#{VERS}.tgz' 'chipmunk-physics.net:chipmunk-physics.net/release/Chipmunk-7.x/'")

DOC = "Chipmunk-7.x/Chipmunk-#{VERS}-Docs"

system("rsync -r '/tmp/Chipmunk-#{VERS}/doc/' 'chipmunk-physics.net:chipmunk-physics.net/release/#{DOC}'")
system_remote("rm 'ChipmunkLatest.tgz'; ln -s 'Chipmunk-7.x/Chipmunk-#{VERS}.tgz' 'ChipmunkLatest.tgz'")

DOC_LINK = "ChipmunkLatest-Docs"
REF_LINK = "ChipmunkLatest-API-Reference"

system_remote("rm '#{DOC_LINK}'; ln -s '#{DOC}' '#{DOC_LINK}'")
system_remote("rm '#{REF_LINK}'; ln -s '#{DOC}/API-Reference' '#{REF_LINK}'")
