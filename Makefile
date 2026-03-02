CXXFLAGS=-std=gnu++2a

(all): read_script calc_checksum

calc_checksum: calc_checksum.c

read_script: read_script.cc types.h

