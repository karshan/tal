#!/bin/sh
while [ 1 ]; do
  make -f tal-cube/makefile 2>&1 | head -n 50
  inotifywait -e MODIFY $(find tal-cube -name "*.cc" -o -name "*.h" -o -name "*.c")
done
