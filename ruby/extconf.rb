require 'mkmf'

system 'cp ../src/* ./'

$CFLAGS.gsub!('-O2', '-O3')
$CFLAGS += ' -std=gnu99'
create_makefile('chipmunk')