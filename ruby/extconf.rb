require 'mkmf'

system 'cp ../src/* ./'

$CFLAGS.gsub!('-fast-math')
$CFLAGS += ' -std=gnu99'
create_makefile('chipmunk')