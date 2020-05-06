#!/bin/sh
while [ 1 ]; do
  make -f tal/makefile 2>&1 | head -n 50
  cp build/tal/tal.hex /r0/pub/
  inotifywait -e MODIFY $(find tal -name "*.cc" -o -name "*.h" -o -name "*.c")
done
