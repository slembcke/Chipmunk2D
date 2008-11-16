require 'mkmf'

$CFLAGS += ' -std=gnu99 -ffast-math -I../src -I../src/constraints -Wall -Werror'
$LDFLAGS += ' libChipmunk.a'
create_makefile('chipmunk')