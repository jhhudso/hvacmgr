#!/bin/bash
while true; do
      socat PTY,link=$HOME/dev/vmodem0,rawer,wait-slave EXEC:"ssh -p2222 at.freegeekarkansas.org -lroot ssh -lroot 192.168.1.91 'socat open:/dev/ttyUSB0,b9600,rawer -'"
done
