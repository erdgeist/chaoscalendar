#!/bin/sh

rm -f Makefile *.o
ruby extconf.rb
make
