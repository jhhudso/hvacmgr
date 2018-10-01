#!/bin/bash
trap 'exit' SIGINT

test -d $HOME/dev/ || mkdir $HOME/dev/
chmod 0700 $HOME/dev/

while true; do
      socat PTY,link=$HOME/dev/hvac0,rawer,wait-slave EXEC:"ssh -p2222 at.freegeekarkansas.org -lroot ssh -lroot 192.168.1.91 'socat open:/dev/ttyUSB0,b9600,rawer -'"
done
