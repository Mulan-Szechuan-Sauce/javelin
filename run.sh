#!/bin/sh

cat $1
./bin/scopeParser < $1 | tee /dev/tty | ./bin/javelinParser
