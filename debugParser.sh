#!/bin/sh
./bin/scopeParser < $1 > /tmp/javelin.pc && gdb -ex "run" -ex "bt" bin/javelinParser < /tmp/javelin.pc | tail -n +18 | head -n -6
