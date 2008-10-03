require 'mkmf'

system 'cp ../src/* ./'

$CFLAGS += ' -std=gnu99 -ffast-math'
create_makefile('chipmunk')